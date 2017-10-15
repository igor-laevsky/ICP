//
// Tests for the JavaMethod class
//

#include "catch.hpp"

#include "JavaTypes/JavaMethod.h"
#include "JavaTypes/ConstantPool.h"
#include "JavaTypes/ConstantPoolRecords.h"
#include "Bytecode/Bytecode.h"
#include "Bytecode/Instructions.h"

using namespace JavaTypes;
using namespace Bytecode;

TEST_CASE("Basic method interface", "[JavaMethod]") {
  // Create constant pool records for the method name and descriptor.
  // We don't need ConstantPool for such simple records.
  auto Name = std::make_unique<ConstantPoolRecords::Utf8>("method_name");
  auto Descriptor = std::make_unique<ConstantPoolRecords::Utf8>(
      "method_descriptor");

  JavaMethod::MethodConstructorParameters Params;

  Params.Flags = JavaMethod::AccessFlags::ACC_PUBLIC;

  Params.Name = Name.get();
  Params.Descriptor = Descriptor.get();

  Params.MaxLocals = 0;
  Params.MaxStack = 0;
  Params.Code = {
      0x2a,             // aload_0
      0xb7, 0x00, 0x01, // invokespecial #1
      0xb1};            // return

  auto Method = std::make_unique<const JavaMethod>(std::move(Params));

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
  auto Name = std::make_unique<ConstantPoolRecords::Utf8>("method_name");
  auto Descriptor = std::make_unique<ConstantPoolRecords::Utf8>(
      "method_descriptor");

  JavaMethod::MethodConstructorParameters Params = {
      JavaMethod::AccessFlags::ACC_PUBLIC,
      Name.get(), Descriptor.get(),
      0, 0,
      {0x00}
  };
  REQUIRE_THROWS_AS(
      std::make_unique<JavaMethod>(std::move(Params)), UnknownBytecode);
}

TEST_CASE("Bytecode parsing error", "[JavaMethod]") {
  auto Name = std::make_unique<ConstantPoolRecords::Utf8>("method_name");
  auto Descriptor = std::make_unique<ConstantPoolRecords::Utf8>(
      "method_descriptor");

  JavaMethod::MethodConstructorParameters Params = {
      JavaMethod::AccessFlags::ACC_PUBLIC,
      Name.get(), Descriptor.get(),
      0, 0,
      {0xb7, 0x00}
  };
  REQUIRE_THROWS_AS(
      std::make_unique<JavaMethod>(std::move(Params)), BytecodeParsingError);
}
