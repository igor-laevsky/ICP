//
// Tests for the bytecode handling infrastructure
//

#include "catch.hpp"

#include "JavaTypes/Bytecode.h"
#include "JavaTypes/BytecodeInstructions.h"

using namespace JavaTypes::Bytecode;

TEST_CASE("Common instruction interface", "[Bytecode]") {
  const std::vector<uint8_t> Bytes =
      {0x2a,             // aload_0
       0xb7, 0x00, 0x01, // invokespecial #1
       0xb1 };           // return
  auto It = Bytes.begin();
  std::unique_ptr<Instruction> Aload = parseInstruction(Bytes, It);
  std::unique_ptr<Instruction> Invoke = parseInstruction(Bytes, It);
  std::unique_ptr<Instruction> Ret = parseInstruction(Bytes, It);
  REQUIRE(It == Bytes.end());

  SECTION("isA") {
    REQUIRE(Aload->isA<Instructions::aload_0>());
    REQUIRE(Invoke->isA<Instructions::invokespecial>());
    REQUIRE(Ret->isA<Instructions::java_return>());

    REQUIRE_FALSE(Aload->isA<Instructions::invokespecial>());
    REQUIRE_FALSE(Aload->isA<Instructions::java_return>());
  }

  SECTION("getAs") {
    REQUIRE_NOTHROW(Aload->getAs<Instructions::aload_0>());
    REQUIRE_THROWS_AS(
        Aload->getAs<Instructions::invokespecial>(),
        UnexpectedBytecodeOperation);

  }

  SECTION("getAsOtNull") {
    REQUIRE(Aload->getAsOrNull<Instructions::aload_0>() != nullptr);
    REQUIRE(Aload->getAsOrNull<Instructions::invokespecial>() == nullptr);
  }

  SECTION("bci") {
    REQUIRE(Aload->getBci() == 0);
    REQUIRE(Invoke->getBci() == 1);
    REQUIRE(Ret->getBci() == 4);
  }
}

TEST_CASE("Undefined bytecode", "[Bytecode]") {
  const std::vector<uint8_t> Bytes = {0x00};
  auto It = Bytes.begin();
  REQUIRE_THROWS_AS(parseInstruction(Bytes, It), UnknownBytecode);
}

TEST_CASE("Empty bytecode", "[Bytecode]") {
  const std::vector<uint8_t> Bytes = {};
  auto It = Bytes.begin();
  REQUIRE_THROWS_AS(parseInstruction(Bytes, It), BytecodeParsingError);
}
