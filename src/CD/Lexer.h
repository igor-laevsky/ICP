///
/// Lexer for the CD language.
///

#ifndef ICP_LEXER_H
#define ICP_LEXER_H

#include <optional>
#include <cassert>
#include <vector>
#include <ostream>

namespace CD {

class Token final {
public:
  static const Token LBrace;
  static const Token RBrace;
  static const Token LSBrace;
  static const Token RSBrace;
  static const Token Comma;
  static const Token Colon;
  static const Token Sharp;
  static const Token Dog;

  // Token which matches all strings independent from their 'Data'.
  // Same for similar functions defined below.
  static Token String();
  static Token String(std::string Data);

  static Token Keyword();
  static Token Keyword(std::string Data);

  static Token Num();
  static Token Num(std::string Data);

  static Token Id();
  static Token Id(std::string Data);

public:
  bool operator==(const Token &Rhs) const noexcept {
    if (T != Rhs.T)
      return false;

    // Wildcard matching
    if (!Data || !Rhs.Data)
      return true;

    return Data == Rhs.Data;
  }

  friend std::ostream& operator<<(std::ostream &Out, const Token &Tok) {
    Out << to_string(Tok);
    return Out;
  }

  friend std::string to_string(const Token &Tok) {
    std::string Res = std::to_string(Tok.T) + " (";

    if (Tok.Data)
      Res += *Tok.Data;
    else
      Res += "none";
    Res += ")";
    return Res;
  }

  bool operator!=(const Token &Rhs) const noexcept {
    return !(*this == Rhs);
  }

  const std::string &getData() const {
    assert(Data);
    return *Data;
  }

  void swap(Token &Other) {
    using std::swap;
    swap(T, Other.T);
    swap(Data, Other.Data);
  }

private:
  enum Type {
    L_BRACE = 0,
    R_BRACE,
    L_SBRACE,
    R_SBRACE,
    COMMA,
    COLON,
    SHARP,
    DOG,
    STRING,
    KEYWORD,
    NUM,
    ID,

    FIRST = L_BRACE,
    LAST = ID + 1
  };

  static constexpr const char *getPattern(Type T) {
    switch (T) {
      case L_BRACE: return "\\{";
      case R_BRACE: return "\\}";
      case L_SBRACE: return "\\[";
      case R_SBRACE: return "\\]";
      case COMMA: return ",";
      case COLON: return ":";
      case SHARP: return "#";
      case DOG: return "@";
      case STRING: return "\"[a-zA-Z0-9_<>/()\\[\\];]+\"";
      case KEYWORD: return "class|constant_pool|method|bytecode|auto|fields|stackmap";
      case NUM: return "\\d+\\b";
      case ID: return "[a-zA-Z0-9_]+\\b";

      default: assert(false);
    }

    return "error";
  }

  // This is intended to be used only from the Lexer constructor
  static Token create(Type T, const std::string &Data) {
    switch (T) {
      case L_BRACE: return LBrace;
      case R_BRACE: return RBrace;
      case L_SBRACE: return LSBrace;
      case R_SBRACE: return RSBrace;
      case COMMA: return Comma;
      case COLON: return Colon;
      case SHARP: return Sharp;
      case DOG: return Dog;
      case STRING:
        // Cut quotes from the match
        return String(Data.substr(1, Data.length() - 2));
      case KEYWORD: return Keyword(Data);
      case NUM: return Num(Data);
      case ID: return Id(Data);
      case LAST: assert(false);
    }

    assert(false);
    return Token(LAST);
  }

private:
  explicit constexpr Token(Type T) noexcept:
      T(T) {
    ;
  }

  Token(Type T, std::string Data) noexcept:
      T(T) {

    // Treat empty string as an absence of data
    if (!Data.empty())
      this->Data = std::move(Data);
  }

private:
  Type T;
  std::optional<std::string> Data;

  friend class Lexer;
};

// Lexer returns references to tokens and owns them. This means that user
// should ensure that lifetime of the Lexer is greater than the lifetime of it's
// tokens.
class Lexer final {
public:
  class LexerError: public std::runtime_error {
    using std::runtime_error::runtime_error;
  };

public:
  // \throws LexerError if unable to lex the output
  // \throws std::regex_error If token patterns are invalid
  explicit Lexer(std::string &&Input);

  Lexer(const Lexer &) = delete;
  Lexer &operator=(const Lexer &) = delete;

  // Return true if there are any tokens left in the stream.
  bool hasNext() const;

  // Checks if next token in the stream matches 'Tok'.
  // Doesn't extract it.
  bool isNext(const Token &Tok) const;

  // Extracts next token from the stream.
  const Token &consume();

  // If next token is equal to Tok - extracts it and returns.
  // Otherwise return null.
  const Token *consume(const Token &Tok);

private:
  const auto &tokens() const { return Tokens; }
  auto &tokens() { return Tokens; }

  auto numTokens() const { return NumTokens; }

private:
  std::vector<Token> Tokens;
  std::size_t NumTokens = 0;
};

}

#endif //ICP_LEXER_H
