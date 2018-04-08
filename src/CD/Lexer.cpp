///
/// Implementation of the CD lexer.
///

#include "Lexer.h"

#include <regex>
#include <iostream>

using namespace CD;
using namespace std::string_literals;

const Token Token::LBrace{L_BRACE};
const Token Token::RBrace{R_BRACE};
const Token Token::LSBrace{L_SBRACE};
const Token Token::RSBrace{R_SBRACE};
const Token Token::Comma{COMMA};
const Token Token::Colon{COLON};
const Token Token::Sharp{SHARP};
const Token Token::Dog{DOG};

Token Token::String() {
  static Token Ret(STRING);
  return Ret;
}
Token Token::String(std::string Data) {
  return Token(STRING, std::move(Data));
}

Token Token::Keyword() {
  static Token Ret(KEYWORD);
  return Ret;
}
Token Token::Keyword(std::string Data) {
  return Token(KEYWORD, std::move(Data));
}

Token Token::Num() {
  static Token Ret(NUM);
  return Ret;
}
Token Token::Num(std::string Data) {
  return Token(NUM, std::move(Data));
}

Token Token::Id() {
  static Token Ret(ID);
  return Ret;
}
Token Token::Id(std::string Data) {
  return Token(ID, std::move(Data));
}

// Trims spaces and comments from the beginning of the string. Stops at the next
// character after the first new line. Hand rolled solution is much faster than
// the regexp based one.
static std::string trimSpacesAndComments(const std::string &Input) {
  std::size_t new_start = 0;
  bool in_comment = false;

  while (new_start < Input.size()) {
    // Check new line
    if (Input.substr(new_start, 2) == "\r\n") {
      new_start += 2;
      break;
    }
    if (Input[new_start] == '\n' || Input[new_start] == '\r') {
      ++new_start;
      break;
    }

    // Skip comments
    if (in_comment) {
      ++new_start;
      continue;
    }
    if (Input.substr(new_start, 2) == "//") {
      in_comment = true;
      new_start += 2;
      continue;
    }

    // Skip whitespaces
    if (std::isblank(Input[new_start])) {
      ++new_start;
      continue;
    }

    // Nothing to skip
    break;
  }
  return Input.substr(new_start, std::string::npos);
}

Lexer::Lexer(std::string &&Input) {
  // Merge all patterns into single regular expression in the form of
  // "^(token1)|(token2)|..."
  std::string PatternStr = "^";
  for (int TokT = Token::FIRST; TokT != Token::LAST; ++TokT) {
    PatternStr += "("s +
        Token::getPattern(static_cast<Token::Type>(TokT)) + ")|";
  }
  PatternStr.pop_back();

  std::regex Pattern(PatternStr);

  // Match tokens until the end of string.
  // TODO: This whole process creates fresh std::string each time it needs a
  // substring. Although this doesn't show as a performance problem but
  // allocation rate be improved using string_view or similar instrument.
  std::size_t cur_line = 1;
  while (!Input.empty()) {
    std::smatch Matches;

    // Skip spaces while counting new lines
    auto old_len = Input.length();
    Input = trimSpacesAndComments(Input);
    if (old_len != Input.length()) {
      cur_line++;
      continue;
    }

    // Parse token
    std::regex_search(Input, Matches, Pattern,
        std::regex_constants::match_continuous);
    if (Matches.empty())
      break; // no token found
    // Should have single sub-match for each token type
    assert(Matches.size() == Token::LAST + 1);

    // Create and record the new token
    bool FoundToken = false;
    for (int TokT = Token::FIRST; TokT != Token::LAST; ++TokT) {
      // Plus one because zero sub match represents the whole match
      if (Matches[TokT + 1].matched) {
        Tokens.push_back(Token::create(
            static_cast<Token::Type>(TokT), Matches[TokT + 1].str()));
        FoundToken = true;
        break;
      }
    }
    assert(FoundToken); // Should be able to find token

    // Consume this token
    Input = Matches.suffix().str();
  }

  if (!Input.empty())
    throw LexerError(
        "Unable to lex the whole string: " + std::to_string(cur_line));

  // Reverse the list to simplify common operations
  std::reverse(Tokens.begin(), Tokens.end());
  NumTokens = Tokens.size();
}

bool Lexer::hasNext() const {
  return NumTokens >= 1;
}

const Token &Lexer::consume() {
  assert(hasNext());
  --NumTokens;
  return tokens()[numTokens()];
}

bool Lexer::isNext(const Token &Tok) const {
  if (!hasNext())
    return false;
  return tokens()[numTokens() - 1] == Tok;
}

const Token *Lexer::consume(const Token &Tok) {
  if (!hasNext() || !isNext(Tok))
    return nullptr;

  return &consume();
}
