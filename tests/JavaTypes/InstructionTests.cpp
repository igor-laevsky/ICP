//
// Tests for the bytecode handling infrastructure
//

#include "catch.hpp"

#include "Bytecode/Bytecode.h"
#include "Bytecode/Instructions.h"

using namespace Bytecode;

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
    REQUIRE(Aload->isA<aload_0>());
    REQUIRE(Invoke->isA<invokespecial>());
    REQUIRE(Ret->isA<java_return>());

    REQUIRE_FALSE(Aload->isA<invokespecial>());
    REQUIRE_FALSE(Aload->isA<java_return>());
  }

  SECTION("getAs") {
    REQUIRE_NOTHROW(Aload->getAs<aload_0>());
    REQUIRE_THROWS_AS(
        Aload->getAs<invokespecial>(),
        UnexpectedBytecodeOperation);
  }

  SECTION("getAsOtNull") {
    REQUIRE(Aload->getAsOrNull<aload_0>() != nullptr);
    REQUIRE(Aload->getAsOrNull<invokespecial>() == nullptr);
  }

  SECTION("bci") {
    REQUIRE(Aload->getBci() == 0);
    REQUIRE(Invoke->getBci() == 1);
    REQUIRE(Ret->getBci() == 4);
  }
}

TEST_CASE("Plain create", "[Bytecode]") {
  auto Instr = Instruction::create<aload_0>();
  REQUIRE(Instr->isA<aload_0>());

  auto Instr2 = Instruction::create<invokespecial>(5);
  REQUIRE(Instr2->isA<invokespecial>());
  REQUIRE(Instr2->getAs<invokespecial>().getIdx() == 5);
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

TEST_CASE("Parse from string") {
  auto aload0 = parseFromString("aload_0");
  REQUIRE(aload0);
  REQUIRE(aload0->isA<aload_0>());

  auto aloadInst = parseFromString("aload", 1);
  REQUIRE(aloadInst);
  REQUIRE(aloadInst->isA<aload>());
  REQUIRE(aloadInst->getAs<aload>().getIdx() == 1);

  auto invoke = parseFromString("invokespecial", 1);
  REQUIRE(invoke);
  REQUIRE(invoke->isA<invokespecial>());
  REQUIRE(invoke->getAs<invokespecial>().getIdx() == 1);
}
