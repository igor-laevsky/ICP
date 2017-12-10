///
/// Tests for the ClassManager class
///

#include "catch.hpp"

#include "Runtime/ClassManager.h"
#include "Runtime/Objects.h"
#include "CD/Parser.h"

using namespace Runtime;

TEST_CASE("Class manager static class", "[Runtime][ClassManager]") {
  auto C1 = CD::parseFromFile("tests/SlowInterpreter/get_put_static.cd");
  auto C2 = CD::parseFromFile("tests/verifier/aload.cd");

  getClassManager().registerClass(ClassObject::create(*C1)->getAs<ClassObject>());
  getClassManager().registerClass(ClassObject::create(*C2)->getAs<ClassObject>());

  // Different references for the different classes
  auto &CObj = getClassManager().getClassObject("PutGetStatic");
  auto &CObj1 = getClassManager().getClassObject("Simple");
  REQUIRE(&CObj != &CObj1);

  auto &CObj_ = getClassManager().getClassObject("PutGetStatic");
  auto &CObj1_ = getClassManager().getClassObject("Simple");
  REQUIRE(&CObj_ != &CObj1_);

  // Always return same reference to the same class
  REQUIRE(&CObj == &CObj_);
  REQUIRE(&CObj1 == &CObj1_);

  // Single static instance everywhere
  CObj.setField("F1", Value::create<JavaInt>(0));
  REQUIRE(CObj_.getField("F1").getAs<JavaInt>() == 0);
}
