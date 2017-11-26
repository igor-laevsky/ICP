//
// Various verifier tests
//

#include "catch.hpp"

#include "Utils/TestUtils.h"
#include "Verifier/Verifier.h"
#include "JavaTypes/ConstantPool.h"
#include "CD/Parser.h"
#include "Bytecode/Instructions.h"

using Catch::Matchers::Equals;

using namespace Verifier;
using namespace JavaTypes;
using namespace TestUtils;

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

static void testWithMethodFile(const std::string &FileName) {
  auto NewClass = CD::parseFromFile("tests/Verifier/" + FileName);
  Verifier::verify(*NewClass);
}

TEST_CASE("Basic verification", "[Verifier]") {
  REQUIRE_NOTHROW(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC,
      1, // Max stack
      1, // Max locals
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

TEST_CASE("invokespecial", "[Verifier]") {
  // Trivial init method
  REQUIRE_NOTHROW(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC,
      1, // Max stack
      1, // Max locals
      11, // <init>
      5, // ()V
      {
          Bytecode::aload_0::OpCode,
          Bytecode::invokespecial::OpCode, 0x00, 14, // java/lang/Object."<init>":()V
          Bytecode::java_return::OpCode,
      },
      {} // No stack map
  ));

  // No uninitializedArg on the stack
  REQUIRE_THROWS_AS(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC,
      1, // Max stack
      1, // Max locals
      11, // <init>
      5, // ()V
      {
          Bytecode::invokespecial::OpCode, 0x00, 14, // java/lang/Object."<init>":()V
          Bytecode::java_return::OpCode,
      },
      {} // No stack map
  ), VerificationError);

  // Init method with arguments
  REQUIRE_NOTHROW(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC,
      3, // Max stack
      2, // Max locals
      11, // <init>
      8, // (Ljava/lang/Object;)V
      {
          Bytecode::aload_0::OpCode,
          Bytecode::aload::OpCode, 0x00, 1,
          Bytecode::iconst_0::OpCode,
          Bytecode::invokespecial::OpCode, 0x00, 17, // java/lang/Object.<init>:(Ljava/lang/Object;I)V
          Bytecode::java_return::OpCode,
      },
      {} // No stack map
  ));

  // Init method with arguments in the wrong order
  REQUIRE_THROWS_AS(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC,
      3, // Max stack
      2, // Max locals
      11, // <init>
      8, // (Ljava/lang/Object;)V
      {
          Bytecode::aload_0::OpCode,
          Bytecode::iconst_0::OpCode,
          Bytecode::aload::OpCode, 0x00, 1,
          Bytecode::invokespecial::OpCode, 0x00, 17, // java/lang/Object.<init>:(Ljava/lang/Object;I)V
          Bytecode::java_return::OpCode,
      },
      {} // No stack map
  ), VerificationError);

  // Init method with non void return type
  REQUIRE_THROWS_AS(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC,
      1, // Max stack
      1, // Max locals
      11, // <init>
      5, // ()V
      {
          Bytecode::invokespecial::OpCode, 0x00, 20, // java/lang/Object.<init>:()I
          Bytecode::java_return::OpCode,
      },
      {} // No stack map
  ), VerificationError);

  // Returning before complete initialization
  REQUIRE_THROWS_AS(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC,
      1, // Max stack
      1, // Max locals
      11, // <init>
      5, // ()V
      {
          Bytecode::java_return::OpCode,
      },
      {} // No stack map
  ), VerificationError);

  // Not an init method
  REQUIRE_THROWS_AS(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC,
      1, // Max stack
      1, // Max locals
      1, // trivial_method
      5, // ()V
      {
          Bytecode::aload_0::OpCode,
          Bytecode::invokespecial::OpCode, 0x00, 14, // java/lang/Object."<init>":()V
          Bytecode::java_return::OpCode,
      },
      {} // No stack map
  ), VerificationError);
}

TEST_CASE("aload", "[Verifier]") {
  REQUIRE_NOTHROW(testWithMethodFile("aload.cd"));

  // Local index is out of bounds
  REQUIRE_THROWS_AS(testWithMethod(
      JavaMethod::AccessFlags::ACC_PUBLIC,
      1, // Max stack
      1, // Max locals
      11, // <init>
      5, // ()V
      {
          Bytecode::aload::OpCode, 0x00, 10,
          Bytecode::invokespecial::OpCode, 0x00,
          14, // java/lang/Object."<init>":()V
          Bytecode::java_return::OpCode,
      },
      {} // No stack map
  ), VerificationError);
}

TEST_CASE("too many locals", "[Verifier]") {
//  REQUIRE_THROWS_AS(
//      testWithMethodFile("to_many_locals.cd"), VerificationError);

  REQUIRE_THROWS_MATCHES(
      testWithMethodFile("to_many_locals.cd"),
      VerificationError, ExEquals("Exceeded maximum number of locals"));

}
