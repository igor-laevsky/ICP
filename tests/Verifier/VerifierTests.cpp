//
// Various verifier tests
//

#include <Bytecode/Instructions.h>
#include "catch.hpp"

#include "Utils/TestUtils.h"
#include "Verifier/Verifier.h"
#include "JavaTypes/ConstantPool.h"

using namespace Verifier;
using namespace JavaTypes;

// Helper function which creates class with single method and virifies it.
static void testWithMethod(
    uint16_t MaxStack, uint16_t MaxLocals,
    ConstantPool::IndexType NameIdx, ConstantPool::IndexType DescriptorIdx,
    const std::vector<uint8_t> &Bytecode,
    JavaTypes::JavaMethod::StackMapTableType &&StackMapTable) {

  // Stack overflow
  std::vector<std::unique_ptr<JavaTypes::JavaMethod>> Methods;
  Methods.push_back(TestUtils::createMethod(
      MaxStack,
      MaxLocals,
      NameIdx,
      DescriptorIdx,
      Bytecode,
      std::move(StackMapTable)
  ));

  auto TrivialClass = TestUtils::createClass(std::move(Methods));
  Verifier::verify(*TrivialClass);
}

TEST_CASE("Basic verification", "[Verifier]") {
  REQUIRE_NOTHROW(testWithMethod(
      1, // Max stack
      0, // Max locals
      1, // trivial_method
      2, // ()I
      {
          0x3, // iconst_0
          0xac // ireturn
      },
      {} // No stack map
  ));
}

TEST_CASE("iconst", "[Verifier]") {
  // Stack overflow
  REQUIRE_THROWS_AS(testWithMethod(
      2, // Max stack
      2, // Max locals
      1, // trivial_method
      2, // ()I
      {
          0x3, // iconst_0
          0x3, // iconst_0
          0x3, // iconst_0
      },
      {} // No stack map
  ), VerificationError);
}

TEST_CASE("ireturn", "[Verifier]") {
  // Void return type
  REQUIRE_THROWS_AS(testWithMethod(
      2, // Max stack
      2, // Max locals
      1, // trivial_method
      5, // ()V
      {
          Bytecode::iconst_0::OpCode,
          Bytecode::ireturn::OpCode,
      },
      {} // No stack map
  ), VerificationError);

  // Wrong return type
  REQUIRE_THROWS_AS(testWithMethod(
      2, // Max stack
      2, // Max locals
      1, // trivial_method
      6, // ()J
      {
          Bytecode::iconst_0::OpCode,
          Bytecode::ireturn::OpCode,
      },
      {} // No stack map
  ), VerificationError);

  // Stack is empty
  REQUIRE_THROWS_AS(testWithMethod(
      2, // Max stack
      2, // Max locals
      1, // trivial_method
      2, // ()I
      {
          Bytecode::ireturn::OpCode,
      },
      {} // No stack map
  ), VerificationError);

  // All good
  REQUIRE_NOTHROW(testWithMethod(
      2, // Max stack
      2, // Max locals
      1, // trivial_method
      2, // ()I
      {
          Bytecode::iconst_0::OpCode,
          Bytecode::ireturn::OpCode,
      },
      {} // No stack map
  ));
}
