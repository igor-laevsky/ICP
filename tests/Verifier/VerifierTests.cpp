//
// Various verifier tests. Earlier tests should be ported to the new CD
// description language.
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
    const std::vector<uint8_t> &Bytecode) {


  std::vector<std::unique_ptr<JavaTypes::JavaMethod>> Methods;
  Methods.push_back(TestUtils::createMethod(
      MaxStack,
      MaxLocals,
      NameIdx,
      DescriptorIdx,
      Bytecode,
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
      }
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
      }
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
      }
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
      }
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
      }
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
      }
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
      }
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
      }
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
      }
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
      }
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
      }
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
      }
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
      }
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
      }
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
      }
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
      }
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
      }
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
      }
  ), VerificationError);
}

TEST_CASE("too many locals", "[Verifier]") {
  REQUIRE_THROWS_AS(
      testWithMethodFile("to_many_locals.cd"), VerificationError);
}

TEST_CASE("verify get_put_static", "[Verifier][getput]") {
  auto C = CD::parseFromFile("tests/Verifier/get_put_static.cd");

  const auto *m = C->getMethod("ok");
  REQUIRE(m);
  REQUIRE_NOTHROW(verifyMethod(*m));

  m = C->getMethod("wrong_type");
  REQUIRE(m);
  REQUIRE_THROWS_MATCHES(
      verifyMethod(*m),
      VerificationError,
      ExEquals("Incompatible type in put static instruction"));

  m = C->getMethod("wrong_type2");
  REQUIRE(m);
  REQUIRE_THROWS_MATCHES(
      verifyMethod(*m),
      VerificationError,
      ExEquals("Expected integer type to be on the stack"));

  m = C->getMethod("wrong_idx");
  REQUIRE(m);
  REQUIRE_THROWS_MATCHES(
      verifyMethod(*m),
      VerificationError,
      ExEquals("Incorrect CP index"));

  m = C->getMethod("wrong_idx2");
  REQUIRE(m);
  REQUIRE_THROWS_MATCHES(
      verifyMethod(*m),
      VerificationError,
      ExEquals("Incorrect CP index"));
}

TEST_CASE("verifier dconst_dreturn", "[Verifier][dconst_dreturn]") {
  auto C = CD::parseFromFile("tests/Verifier/dconst_dreturn.cd");

  const auto *m = C->getMethod("test1");
  REQUIRE(m);
  REQUIRE_NOTHROW(verifyMethod(*m));

  m = C->getMethod("test2");
  REQUIRE(m);
  REQUIRE_THROWS_MATCHES(
      verifyMethod(*m),
      VerificationError,
      ExEquals("Expected double type to be on the stack"));

  m = C->getMethod("test3");
  REQUIRE(m);
  REQUIRE_THROWS_MATCHES(
      verifyMethod(*m),
      VerificationError,
      ExEquals("Return type should be double"));

  m = C->getMethod("test4");
  REQUIRE(m);
  REQUIRE_NOTHROW(verifyMethod(*m));

  m = C->getMethod("test5");
  REQUIRE(m);
  REQUIRE_NOTHROW(verifyMethod(*m));
}

//TEST_CASE("verifier if_icmp", "[Verifier][if_icmp]") {
//  auto C = CD::parseFromFile("tests/Verifier/if_icmp.cd");
//
//  for (const auto &method: C->methods()) {
//    assert(method.get() != nullptr); // should never happen
//    const JavaMethod &M = *method.get();
//
//    if (starts_with(M.getName(), "ok")) {
//      REQUIRE_NOTHROW(verifyMethod(M));
//    } else if (starts_with(M.getName(), "wrong")) {
//      REQUIRE_THROWS_AS(verifyMethod(M), VerificationError);
//    } else {
//      assert(false); // Method name should start with either 'ok' or 'wrong'
//    }
//  }
//}
