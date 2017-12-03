///
/// Tests for the intepreter values
///

#include "catch.hpp"

#include "SlowInterpreter/Value.h"
#include "CD/Parser.h"
#include "JavaTypes/Type.h"

using namespace SlowInterpreter;
using namespace JavaTypes;

TEST_CASE("Values for pods", "[Interpreter][Value]") {
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

TEST_CASE("Construct value from memory", "[Interpreter][Value]") {
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

TEST_CASE("Value assignment", "[Interpreter][Value]") {
  Value T1 = Value::create<JavaInt>(10);
  Value T2 = Value::create<JavaInt>(20);

  T1 = T2;

  REQUIRE(T1 == T2);
}

TEST_CASE("Save value to memory", "[Interpreter][Value]") {
  Value T1 = Value::create<JavaChar>(10);
  Value T2 = Value::create<JavaInt>(20);
  Value T3 = Value::create<JavaDouble>(20);

  uint8_t T1Mem[2] = {0};
  uint8_t T2Mem[4] = {0};
  uint8_t T3Mem[8] = {0};

  T1.toMemory(T1Mem, T1, Types::Char);
  T2.toMemory(T2Mem, T2, Types::Int);
  T3.toMemory(T3Mem, T3, Types::Double);

  Value NewT1 = Value::fromMemory(Types::Char, T1Mem);
  Value NewT2 = Value::fromMemory(Types::Int, T2Mem);
  Value NewT3 = Value::fromMemory(Types::Double, T3Mem);

  REQUIRE(T1 == NewT1);
  REQUIRE(T2 == NewT2);
  REQUIRE(T3 == NewT3);

  // Copy constructor should also work
  Value A(NewT3);
  REQUIRE(A == NewT3);
}

//TEST_CASE("Class objects static fields", "[Interpreter][Value]") {
//  auto C = CD::parseFromFile("tests/SlowInterpreter/get_put_static.cd");
//
//  JavaRef Class = ClassObject::create(*C);
//
//  Value F1 = Class->getField("F1");
//  Value F2 = Class->getField("F2");
//  Value F3 = Class->getField("F3");
//  Value Ref = Class->getField("Ref");
//  REQUIRE_THROWS(Class->getField("asd"), ClassObject::UnrecognizedField);
//
//  REQUIRE(F1.isA<JavaInt>());
//  REQUIRE(F1.getAs<JavaInt>() == 0);
//
//  REQUIRE(F2.isA<JavaDouble>());
//  REQUIRE(F2.getAs<JavaDouble>() == 0);
//
//  REQUIRE(F3.isA<JavaInt>());
//  REQUIRE(F3.getAs<JavaInt>() == 0);
//
//  REQUIRE(Ref.isA<JavaRef>());
//  REQUIRE(Ref.getAs<JavaRef>() == nullptr);
//
//  // Store value to the field. Everything decays into JavaInt.
//  Class->setField("F1", Value::create<JavaChar>(10));
//  Class->setField("F2", Value::create<JavaDouble>(20));
//  Class->setField("F3", Value::create<JavaShort>(20));
//
//  // Check that we stored correct values
//  REQUIRE(Class->getField("F1").getAs<JavaInt> == 10);
//  REQUIRE(Class->getField("F2").getAs<JavaDouble> == 20);
//  REQUIRE(Class->getField("F3").getAs<JavaInt> == 30);
//}
