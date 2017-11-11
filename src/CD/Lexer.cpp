//
// Implementation of the CD lexer
//

#include "Lexer.h"

using namespace CD;

const Token Token::LBrace{L_BRACE};
const Token Token::RBrace{R_BRACE};
const Token Token::Comma{COMMA};
const Token Token::Colon{COLON};
const Token Token::Sharp{SHARP};

Token Token::String() {
  return Token(STRING);
}
Token Token::String(std::string Data) {
  return Token(STRING, std::move(Data));
}

Token Token::Keyword() {
  return Token(KEYWORD);
}
Token Token::Keyword(std::string Data) {
  return Token(KEYWORD, std::move(Data));
}

Token Token::Num() {
  return Token(NUM);
}
Token Token::Num(std::string Data) {
  return Token(NUM, std::move(Data));
}

Token Token::Id() {
  return Token(ID);
}
Token Token::Id(std::string Data) {
  return Token(ID, std::move(Data));
}
