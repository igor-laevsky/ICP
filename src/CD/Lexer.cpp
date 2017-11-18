//
// Implementation of the CD lexer
//

#include "Lexer.h"

#include <regex>
#include <iostream>

using namespace CD;
using namespace std::string_literals;

const Token Token::LBrace{L_BRACE};
const Token Token::RBrace{R_BRACE};
const Token Token::Comma{COMMA};
const Token Token::Colon{COLON};
const Token Token::Sharp{SHARP};

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
  std::regex Spaces("^\\s*(\\\\.*[\n|\r\n|\r]\\s*)*");

  // Match tokens until the end of string.
  // We can modify 'Input' because it's an rvalue.
  while (!Input.empty()) {
    std::smatch Matches;

    // Skip whitespaces and comments
    Input = std::regex_replace(Input, Spaces, "");

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
    throw LexerError("Unable to lex the whole string");

  // Reverse token list so that we can pop_back to advance to the next token
  std::reverse(Tokens.begin(), Tokens.end());
}

bool Lexer::hasNext() const {
  return !tokens().empty();
}

Token Lexer::getNext() {
  assert(hasNext());
  Token Ret = tokens().back();
  tokens().pop_back();
  return Ret;
}

std::optional<Token> Lexer::consume(const Token &Tok) {
  if (!hasNext() || !isNext(Tok))
    return std::nullopt;

  return getNext();
}

bool Lexer::isNext(const Token &Tok) const {
  if (!hasNext())
    return false;
  return tokens().back() == Tok;
}
