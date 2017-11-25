///
/// Test for the CD parser
///

#include "catch.hpp"

#include "CD/Parser.h"
#include "Verifier/Verifier.h"
#include "SlowInterpreter/SlowInterpreter.h"
#include "JavaTypes/JavaClass.h"
#include "JavaTypes/JavaMethod.h"

using namespace CD;
using namespace JavaTypes;

TEST_CASE("String input", "[CD][Parser]") {
  // Empty input
  REQUIRE_THROWS_AS(parseFromString(""), ParserError);

  // No constant pool
  REQUIRE_THROWS_AS(parseFromString("class {}"), ParserError);

  // No name
  REQUIRE_THROWS_AS(parseFromString("class { constant_pool {} }"), ParserError);

  // No super
  REQUIRE_THROWS_AS(parseFromString(
    "class {\n"
    "  constant_pool {\n"
    "    1: ClassInfo \"Simple\"\n"
    "    2: ClassInfo \"java/lang/Object\"\n"
    "  }\n"
    "\n"
    "  Name: #1\n"
    "}"), ParserError);

  // Empty class
  REQUIRE(parseFromString(
    "class {\n"
    "  constant_pool {\n"
    "    1: ClassInfo \"Simple\"\n"
    "    2: ClassInfo \"java/lang/Object\"\n"
    "  }\n"
    "\n"
    "  Name: #1\n"
    "  Super: #2\n"
    "}"));
}

TEST_CASE("Incomplete", "[CD][Parser]") {
  REQUIRE_THROWS_AS(parseFromFile("CD/Incomplete.cd"), ParserError);
}

TEST_CASE("Empty", "[CD][Parser]") {
  REQUIRE(parseFromFile("CD/Empty.cd"));
}

TEST_CASE("EmptyMethods", "[CD][Parser]") {
  auto C = parseFromFile("CD/EmptyMethods.cd");

  REQUIRE(C);

  REQUIRE(C->hasSuper());
  REQUIRE(C->getSuperClassName() == "java/lang/Object");
  REQUIRE(C->getClassName() == "Simple");

  REQUIRE(C->getMethod("main"));
  REQUIRE(C->getMethod("<init>"));
}

TEST_CASE("Simple", "[CD][Parser]") {
  auto C = parseFromFile("CD/Simple.cd");

  REQUIRE(C);

  REQUIRE(C->hasSuper());
  REQUIRE(C->getSuperClassName() == "java/lang/Object");
  REQUIRE(C->getClassName() == "Simple");

  REQUIRE(C->getMethod("main"));
  REQUIRE(C->getMethod("<init>"));

  REQUIRE_NOTHROW(Verifier::verify(*C));

  auto Res = SlowInterpreter::interpret(*C->getMethod("main"), {});
  REQUIRE(std::any_cast<SlowInterpreter::JavaInt>(Res) == 0);
}
