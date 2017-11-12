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

TEST_CASE("Simple tokens", "[CD]") {
  Lexer lex(",:{}#");

  REQUIRE(lex.hasNext());
  REQUIRE(lex.isNext(Token::Comma));
  auto t = lex.consume(Token::Comma);
  REQUIRE(t.has_value());
  REQUIRE(*t == Token::Comma);

  REQUIRE(lex.hasNext());
  REQUIRE(!lex.isNext(Token::Comma));
  REQUIRE(!lex.isNext(Token::Keyword()));
  REQUIRE(!lex.consume(Token::Num()).has_value());
  REQUIRE(lex.isNext(Token::Colon));
  REQUIRE(lex.getNext() == Token::Colon);

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::LBrace);

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::RBrace);

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::Sharp);

  REQUIRE(!lex.hasNext());
  t = lex.consume(Token::Id());
  REQUIRE(!t.has_value());
}

TEST_CASE("Complex tokens", "[CD]") {
  Lexer lex("class aload_0 method \"string\" 123 0ident");

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::Keyword("class"));

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::Id("aload_0"));

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::Keyword());

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::String("string"));

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::Num("123"));

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::Id("0ident"));

  REQUIRE(!lex.hasNext());
}

TEST_CASE("Multiline", "[CD]") {
  Lexer lex(
      "class \n"
      "  {\n"
        "  bytecode {\r\n"
          "    aload_0\r\n"
        "  }\n"
      "  }\n");

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::Keyword("class"));

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::LBrace);

  REQUIRE(lex.hasNext());
  auto t = lex.consume(Token::Keyword());
  REQUIRE(t.has_value());
  REQUIRE(*t == Token::Keyword("bytecode"));

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::LBrace);

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::Id("aload_0"));

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::RBrace);

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::RBrace);

  REQUIRE(!lex.hasNext());
}

TEST_CASE("Comments", "[CD]") {
  Lexer lex(
      "\\\\ comment \n"
      "class \n"
      "{\n"
        "aload_0 \\\\another comment\r\n"
        "  \\\\ valid in the comment: \"\"\n"
      "}\n");

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::Keyword("class"));

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::LBrace);

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::Id("aload_0"));

  REQUIRE(lex.hasNext());
  REQUIRE(lex.getNext() == Token::RBrace);

  REQUIRE(!lex.hasNext());
}

TEST_CASE("Lexer error", "[CD]") {
  REQUIRE_THROWS_AS(
      Lexer("\"asd"), Lexer::LexerError);

  // empty strings are specifically disallowed
  REQUIRE_THROWS_AS(
      Lexer("\"\""), Lexer::LexerError);
}