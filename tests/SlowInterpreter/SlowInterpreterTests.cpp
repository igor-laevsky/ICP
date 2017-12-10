//
// Tests for the slow interpreter
//

#include "catch.hpp"

#include <any>

#include "SlowInterpreter/SlowInterpreter.h"
#include "SlowInterpreter/Value.h"
#include "JavaTypes/ConstantPool.h"
#include "Utils/TestUtils.h"
#include "Bytecode/Instructions.h"

using namespace JavaTypes;
using namespace SlowInterpreter;

// Returns true if interpreter interpreted given method and returned 'ExpectedResult'
template<typename ResT>
static bool testWithMethod(
    ResT ExpectedResult,
    const std::vector<Value>& InputArgs,
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

  auto Res = SlowInterpreter::interpret(*Method, InputArgs);
  return Res.getAs<ResT>() == ExpectedResult;
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
