//
// Tests for the JavaMethod class
//

#include "catch.hpp"

#include "Utils/TestUtils.h"

#include "JavaTypes/JavaMethod.h"
#include "JavaTypes/ConstantPool.h"
#include "JavaTypes/ConstantPoolRecords.h"
#include "Bytecode/Bytecode.h"
#include "Bytecode/Instructions.h"

using namespace JavaTypes;
using namespace Bytecode;

TEST_CASE("Basic method interface", "[JavaMethod]") {
  auto Method = TestUtils::createTrivialMethod();

  // Check BCI access
  const auto &Aload = Method->getInstrAtBci(0);
  const auto &Invoke = Method->getInstrAtBci(1);
  const auto &Ret = Method->getInstrAtBci(4);
  REQUIRE_THROWS_AS(Method->getInstrAtBci(100), JavaMethod::WrongBci);
  REQUIRE_THROWS_AS(Method->getInstrAtBci(2), JavaMethod::WrongBci);

  // Check that instructions are parsed correctly
  REQUIRE(Aload.isA<aload_0>());
  REQUIRE(Invoke.isA<invokespecial>());
  REQUIRE(Invoke.getAs<invokespecial>().getIdx() == 1);
  REQUIRE(Ret.isA<java_return>());

  // Check that code iterator works
  std::vector<std::reference_wrapper<const Instruction>> Instrs;
  for (const auto &Instr: *Method)
    Instrs.emplace_back(Instr);

  std::vector<std::reference_wrapper<const Instruction>> InstrsExpected =
      {Aload, Invoke, Ret};
  REQUIRE(Instrs == InstrsExpected);
}

TEST_CASE("Unknown bytecode error", "[JavaMethod]") {
  REQUIRE_THROWS_AS(
      TestUtils::createMethod({0x00}), UnknownBytecode);
}

TEST_CASE("Bytecode parsing error", "[JavaMethod]") {
  REQUIRE_THROWS_AS(
      TestUtils::createMethod({0xb7, 0x00}), BytecodeParsingError);

}
