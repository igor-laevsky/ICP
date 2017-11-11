
#ifndef ICP_LEXER_H
#define ICP_LEXER_H

#include <optional>
#include <cassert>

namespace CD {

class Token {
public:
  static const Token LBrace;
  static const Token RBrace;
  static const Token Comma;
  static const Token Colon;
  static const Token Sharp;

  static Token String();
  static Token String(std::string Data);

  static Token Keyword();
  static Token Keyword(std::string Data);

  static Token Num();
  static Token Num(std::string Data);

  static Token Id();
  static Token Id(std::string Data);

public:
  enum Type {
    L_BRACE = 0,
    R_BRACE,
    COMMA,
    COLON,
    SHARP,
    STRING,
    KEYWORD,
    NUM,
    ID,

    FIRST = L_BRACE,
    LAST = ID
  };

  static constexpr const char *getPattern(Type T) {
    switch (T) {
      case L_BRACE: return "{";
      case R_BRACE: return "{";
      case COMMA: return ",";
      case COLON: return ":";
      case SHARP: return "#";
      case STRING: return "\"\\w+\"";
      case KEYWORD: return "class|constant_pool|method";
      case NUM: return "\\d+";
      case ID: return "\\w+";

      default: assert(false);
    }
    return "error";
  }

  bool operator==(const Token &Rhs) const noexcept {
    if (T != Rhs.T)
      return false;

    // Wildcard matching
    if (!Data || !Rhs.Data)
      return true;

    return Data == Rhs.Data;
  }

  bool operator!=(const Token &Rhs) const noexcept {
    return !(*this == Rhs);
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
  const Type T;
  std::optional<std::string> Data;
};


}

#endif //ICP_LEXER_H
