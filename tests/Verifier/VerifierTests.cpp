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
    JavaMethod::AccessFlags Flags,
    uint16_t MaxStack, uint16_t MaxLocals,
    ConstantPool::IndexType NameIdx, ConstantPool::IndexType DescriptorIdx,
    const std::vector<uint8_t> &Bytecode,
    JavaMethod::StackMapTableType &&StackMapTable) {

  std::vector<std::unique_ptr<JavaTypes::JavaMethod>> Methods;
  Methods.push_back(TestUtils::createMethod(
      MaxStack,
      MaxLocals,
      NameIdx,
      DescriptorIdx,
      Bytecode,
      std::move(StackMapTable),
      Flags
  ));

  auto TrivialClass = TestUtils::createClass(std::move(Methods));
  Verifier::verify(*TrivialClass);
}

TEST_CASE("Basic verification", "[Verifier]") {
  REQUIRE_NOTHROW(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC,
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
      JavaMethod::AccessFlags::ACC_PUBLIC,
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
  ), VerificationError);

  // Wrong return type
  REQUIRE_THROWS_AS(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC,
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
      JavaMethod::AccessFlags::ACC_PUBLIC,
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
      JavaMethod::AccessFlags::ACC_PUBLIC,
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

TEST_CASE("aload_0", "[Verifier]") {
  // Return array
  REQUIRE_NOTHROW(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC_STATIC,
      2, // Max stack
      2, // Max locals
      1, // trivial_method
      7, // ([Ljava/lang/String;)V
      {
          Bytecode::aload_0::OpCode,
          Bytecode::java_return::OpCode,
      },
      {} // No stack map
  ));

  // Return object
  REQUIRE_NOTHROW(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC_STATIC,
      2, // Max stack
      2, // Max locals
      1, // trivial_method
      8, // (Ljava/lang/Object;)V
      {
          Bytecode::aload_0::OpCode,
          Bytecode::java_return::OpCode,
      },
      {} // No stack map
  ));

  // Try to load integer
  REQUIRE_THROWS_AS(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC_STATIC,
      2, // Max stack
      2, // Max locals
      1, // trivial_method
      9, // (I)V
      {
          Bytecode::aload_0::OpCode,
          Bytecode::java_return::OpCode,
      },
      {} // No stack map
  ), VerificationError);

  // Try to load when no locals are present
  REQUIRE_THROWS_AS(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC_STATIC,
      2, // Max stack
      2, // Max locals
      1, // trivial_method
      5, // ()V
      {
          Bytecode::aload_0::OpCode,
          Bytecode::java_return::OpCode,
      },
      {} // No stack map
  ), VerificationError);
}
