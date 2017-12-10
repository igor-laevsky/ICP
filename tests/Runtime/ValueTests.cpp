///
/// Tests for the intepreter values
///

#include "catch.hpp"

#include "Runtime/Value.h"
#include "CD/Parser.h"
#include "JavaTypes/Type.h"

using namespace Runtime;
using namespace JavaTypes;

TEST_CASE("Values for pods", "[Runtime][Value]") {
  // Stack values decay into JavaInt
  std::vector<Value> Stack(3);

  Stack[0] = Value::create<JavaChar>(10);
  Stack[1] = Value::create<JavaInt>(20);
  Stack[2] = Value::create<JavaDouble>(30.0);

  // JavaChar should be promoted into it's stack type (JavaInt)
  REQUIRE(Stack[0].getAsOrNull<JavaChar>());
  REQUIRE(Stack[0].getAs<JavaInt>() == 10);
  REQUIRE(Stack[1].getAs<JavaInt>() == 20);
  REQUIRE(Stack[2].getAs<JavaDouble>() == 30);

  // All small types should be promoted
  REQUIRE(Stack[0].isA<JavaChar>());
  REQUIRE(Stack[0].isA<JavaShort>());
  REQUIRE(Stack[0].isA<JavaBool>());
  REQUIRE(Stack[0].isA<JavaByte>());
  REQUIRE(Stack[0].isA<JavaInt>());

  // But never promote too much
  REQUIRE(!Stack[0].isA<JavaFloat>());
  REQUIRE(!Stack[1].isA<JavaRef>());
  REQUIRE(!Stack[2].isA<JavaChar>());
  REQUIRE_THROWS_AS(Stack[0].getAs<JavaDouble>(), Value::BadAccess);
  REQUIRE_THROWS_AS(Stack[2].getAs<JavaInt>(), Value::BadAccess);

  // Compile errors. For the reference. No idea how to check in the test.
  //Stack[3] = Value::create(10); // type deduction should be disabled
  //Stack[3] = Value::create<bool>(false); // only allow java types
}

TEST_CASE("Construct value from memory", "[Runtime][Value]") {
  const uint8_t PlainMemory[8] = {0};

  Value T1 = Value::fromMemory(Types::Char, PlainMemory);
  REQUIRE(T1.isA<JavaInt>());
  REQUIRE(T1.getAs<JavaInt>() == 0);


  Value T2 = Value::fromMemory(Types::Short, PlainMemory);
  REQUIRE(T2.isA<JavaInt>());
  REQUIRE(T2.getAs<JavaInt>() == 0);

  Value T3 = Value::fromMemory(Types::Int, PlainMemory);
  REQUIRE(T3.isA<JavaInt>());
  REQUIRE(T3.getAs<JavaInt>() == 0);

  Value T4 = Value::fromMemory(Types::Double, PlainMemory);
  REQUIRE(T4.isA<JavaDouble>());
  REQUIRE(T4.getAs<JavaDouble>() == 0);

  REQUIRE_THROWS_AS(T4.getAs<JavaInt>(), Value::BadAccess);
}

TEST_CASE("Value copy", "[Runtime][Value]") {
  Value T1 = Value::create<JavaInt>(10);
  Value T2 = Value::create<JavaInt>(20);

  T1 = T2;
  REQUIRE(T1 == T2);

  // Copy constructor should also work
  Value A(T2);
  REQUIRE(A == T2);
}

TEST_CASE("Save value to memory", "[Runtime][Value]") {
  Value T1 = Value::create<JavaChar>(10);
  Value T2 = Value::create<JavaInt>(20);
  Value T3 = Value::create<JavaDouble>(20);
  Value Ref = Value::create<JavaRef>(reinterpret_cast<JavaRef>(10));

  uint8_t T1Mem[2] = {0};
  uint8_t T2Mem[4] = {0};
  uint8_t T3Mem[8] = {0};
  uint8_t RefMem[sizeof(JavaRef)] = {0};

  T1.toMemory(T1Mem, T1, Types::Char);
  T2.toMemory(T2Mem, T2, Types::Int);
  T3.toMemory(T3Mem, T3, Types::Double);
  Ref.toMemory(RefMem, Ref, Types::Reference);

  Value NewT1 = Value::fromMemory(Types::Char, T1Mem);
  Value NewT2 = Value::fromMemory(Types::Int, T2Mem);
  Value NewT3 = Value::fromMemory(Types::Double, T3Mem);
  Value NewRef = Value::fromMemory(Types::Reference, RefMem);

  REQUIRE(T1 == NewT1);
  REQUIRE(T2 == NewT2);
  REQUIRE(T3 == NewT3);
  REQUIRE(Ref == NewRef);
}
