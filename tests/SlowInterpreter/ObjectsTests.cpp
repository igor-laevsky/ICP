///
/// Tests for the java object runtime representation
///

#include "catch.hpp"

#include "SlowInterpreter/Objects.h"
#include "SlowInterpreter/Value.h"
#include "CD/Parser.h"

using namespace SlowInterpreter;

TEST_CASE("Class objects static fields", "[Interpreter][Value]") {
  auto C = CD::parseFromFile("tests/SlowInterpreter/get_put_static.cd");

  // Create class with static fields
  JavaRef ClassRef = ClassObject::create(*C);

  // Ensure polymorphic behavior
  REQUIRE(ClassRef->isA<ClassObject>());
  REQUIRE(ClassRef->getAsOrNull<ClassObject>());
  REQUIRE_NOTHROW(ClassRef->getAs<ClassObject>());

  auto &Class = ClassRef->getAs<ClassObject>();

  // Ensure that we are able to get declared fields and only them
  Value F1 = Class.getField("F1");
  Value F2 = Class.getField("F2");
  Value F3 = Class.getField("F3");
  Value Ref = Class.getField("Ref");
  REQUIRE_THROWS_AS(Class.getField("asd"), ClassObject::UnrecognizedField);

  // Ensure that by default memory is zero initialized and correctly typed
  REQUIRE(F1.isA<JavaInt>());
  REQUIRE(F1.getAs<JavaInt>() == 0);

  REQUIRE(F2.isA<JavaDouble>());
  REQUIRE(F2.getAs<JavaDouble>() == 0);

  REQUIRE(F3.isA<JavaInt>());
  REQUIRE(F3.getAs<JavaInt>() == 0);

  REQUIRE(Ref.isA<JavaRef>());
  REQUIRE(Ref.getAs<JavaRef>() == nullptr);

  // Store value to the field. Everything decays into JavaInt.
  Class.setField("F1", Value::create<JavaChar>(10));
  Class.setField("F2", Value::create<JavaDouble>(20));
  Class.setField("F3", Value::create<JavaShort>(30));

  // Check that we have stored correct values
  REQUIRE(Class.getField("F1").getAs<JavaInt>() == 10);
  REQUIRE(Class.getField("F2").getAs<JavaDouble>() == 20);
  REQUIRE(Class.getField("F3").getAs<JavaShort>() == 30);
}
