//
// Tests for the CD lexer
//

#include "catch.hpp"

#include "CD/Lexer.h"

using namespace CD;

TEST_CASE("Tokens", "[CD]") {
  REQUIRE(Token::LBrace == Token::LBrace);
  REQUIRE(Token::RBrace != Token::LBrace);

  REQUIRE(Token::Keyword() == Token::Keyword("Hello"));
  REQUIRE(Token::Keyword("Hello") == Token::Keyword("Hello"));
  REQUIRE(Token::Keyword("Noo") != Token::Keyword("Hello"));

  REQUIRE(Token::Num() != Token::Id());
}
