//
// Tests for the slow interpreter
//

#include "catch.hpp"

#include <any>

#include "SlowInterpreter/SlowInterpreter.h"
#include "JavaTypes/ConstantPool.h"
#include "Utils/TestUtils.h"
#include "Bytecode/Instructions.h"

using namespace JavaTypes;

// Returns true if interpreter interpreted given method and returned 'ExpectedResult'
template<typename ResT>
static bool testWithMethod(
    ResT ExpectedResult,
    const std::vector<std::any>& InputArgs,
    JavaMethod::AccessFlags Flags,
    uint16_t MaxStack, uint16_t MaxLocals,
    ConstantPool::IndexType NameIdx, ConstantPool::IndexType DescriptorIdx,
    const std::vector<uint8_t> &Bytecode,
    JavaTypes::JavaMethod::StackMapTableType &&StackMapTable) {

  auto Method = TestUtils::createMethod(
      MaxStack,
      MaxLocals,
      NameIdx,
      DescriptorIdx,
      Bytecode,
      std::move(StackMapTable),
      Flags);

  std::any Res = SlowInterpreter::interpret(*Method, InputArgs);
  return std::any_cast<ResT>(Res) == ExpectedResult;
}

TEST_CASE("iconst with ireturn", "[SlowInterpreter]") {
  REQUIRE(testWithMethod<SlowInterpreter::JavaInt>(
      0, // Expected result
      {},// Input args
      JavaMethod::AccessFlags::ACC_PUBLIC,
      2, // Max stack
      2, // Max locals
      1, // trivial_method
      5, // ()V
      {
          Bytecode::iconst_0::OpCode,
          Bytecode::ireturn::OpCode,
      },
      {} // No stack map
  ));

  REQUIRE(testWithMethod<SlowInterpreter::JavaInt>(
      0, // Expected result
      {},// Input args
      JavaMethod::AccessFlags::ACC_PUBLIC,
      2, // Max stack
      2, // Max locals
      1, // trivial_method
      5, // ()V
      {
          Bytecode::iconst_0::OpCode,
          Bytecode::iconst_0::OpCode,
          Bytecode::ireturn::OpCode,
      },
      {} // No stack map
  ));
}
