///
/// Test for the CD parser
///

#include "catch.hpp"

#include "CD/Parser.h"
#include "Verifier/Verifier.h"
#include "SlowInterpreter/SlowInterpreter.h"
#include "Runtime/Value.h"
#include "Runtime/ClassManager.h"
#include "JavaTypes/JavaClass.h"
#include "JavaTypes/JavaMethod.h"
#include "Utils/BinaryFiles.h"

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
  REQUIRE_THROWS_AS(parseFromFile("tests/CD/Incomplete.cd"), ParserError);
}

TEST_CASE("Empty", "[CD][Parser]") {
  REQUIRE(parseFromFile("tests/CD/Empty.cd"));
}

TEST_CASE("EmptyMethods", "[CD][Parser]") {
  auto C = parseFromFile("tests/CD/EmptyMethods.cd");

  REQUIRE(C);

  REQUIRE(C->hasSuper());
  REQUIRE(C->getSuperClassName() == "java/lang/Object");
  REQUIRE(C->getClassName() == "Simple");

  REQUIRE(C->getMethod("main"));
  REQUIRE(C->getMethod("<init>"));
}

TEST_CASE("CD parser for the simple case", "[CD][Parser]") {
  auto C = parseFromFile("tests/CD/Simple.cd");

  REQUIRE(C);

  REQUIRE(C->hasSuper());
  REQUIRE(C->getSuperClassName() == "java/lang/Object");
  REQUIRE(C->getClassName() == "Simple");

  REQUIRE(C->getMethod("main"));
  REQUIRE(C->getMethod("<init>"));

  REQUIRE_NOTHROW(Verifier::verify(*C));

  Runtime::ClassManager CM;

  auto Res = SlowInterpreter::interpret(*C->getMethod("main"), {}, CM);
  REQUIRE(Res.getAs<Runtime::JavaInt>() == 0);
}

TEST_CASE("Fields", "[CD][Parser]") {
  auto C = parseFromFile("tests/CD/Fields.cd");

  REQUIRE(C);
  REQUIRE(C->hasSuper());
  REQUIRE(C->methods().empty());

  // Check parsed fields
  // Don't bother with full JavaField objects
  std::vector<std::tuple<const char*, const char*, JavaField::AccessFlags>>
      Fields =
  {
      {"I", "F1", JavaField::AccessFlags::ACC_PUBLIC_STATIC},
      {"D", "F2",
          JavaField::AccessFlags::ACC_PUBLIC | JavaField::AccessFlags::ACC_FINAL},
      {"Ljava/lang/SomeClass;", "Ref", JavaField::AccessFlags::ACC_PRIVATE}
  };

  REQUIRE(C->fields().size() == 3);
  for (std::size_t i = 0; i < C->fields().size(); ++i) {
    REQUIRE(C->fields()[i].getDescriptor() == std::get<0>(Fields[i]));
    REQUIRE(C->fields()[i].getName() == std::get<1>(Fields[i]));
    REQUIRE(C->fields()[i].getFlags() == std::get<2>(Fields[i]));
  }
}

TEST_CASE("FieldRef", "[CD][Parser]") {
  REQUIRE(parseFromFile("tests/CD/FieldRef.cd"));
}

TEST_CASE("is8bit is16bit utils", "[CD][Utils][Parser]") {
  REQUIRE(Utils::isUint8<uint32_t>(0));
  REQUIRE(Utils::isUint16<uint32_t>(0));
  REQUIRE_FALSE(Utils::isUint8<uint32_t>(-1));
  REQUIRE_FALSE(Utils::isUint16<uint32_t>(-1));
  REQUIRE(Utils::isUint8<uint32_t>(255));
  REQUIRE(Utils::isUint16<uint32_t>(255));
  REQUIRE_FALSE(Utils::isUint8<uint32_t>(256));
  REQUIRE(Utils::isUint16<uint32_t>(256));
  REQUIRE_FALSE(Utils::isUint8<uint32_t>(-1));
  REQUIRE_FALSE(Utils::isUint16<uint32_t>(-1));
  REQUIRE_FALSE(Utils::isUint8<uint32_t>(-130));
  REQUIRE_FALSE(Utils::isUint16<uint32_t>(-130));
  REQUIRE_FALSE(Utils::isUint8<uint32_t>(-128));
  REQUIRE_FALSE(Utils::isUint16<uint32_t>(-128));
}