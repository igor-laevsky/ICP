///
/// Tests for the ClassManager class
///

#include "catch.hpp"

#include "Runtime/ClassManager.h"
#include "Runtime/Objects.h"
#include "Verifier/Verifier.h"
#include "JavaTypes/JavaClass.h"

using namespace Runtime;

TEST_CASE("Class manager basic linking and initialization", "[Runtime][ClassManager]") {
  ClassManager CM;

  // Load the classes
  const auto &C1 = CM.getClass("tests/SlowInterpreter/get_put_static", getTestLoader());
  REQUIRE(C1.getClassName() == "PutGetStatic");
  const auto &C2 = CM.getClass("examples/Fields");
  REQUIRE(C2.getClassName() == "Fields");

  // Different references for the different classes
  auto &CObj = CM.getClassObject(C1);
  auto &CObj1 = CM.getClassObject(C2);
  REQUIRE(&CObj != &CObj1);

  auto &CObj_ = CM.getClassObject(C1);
  auto &CObj1_ = CM.getClassObject(C2);
  REQUIRE(&CObj_ != &CObj1_);

  // Always return same reference to the same class
  REQUIRE(&CObj == &CObj_);
  REQUIRE(&CObj1 == &CObj1_);

  // Single static instance everywhere
  CObj.setField("F1", Value::create<JavaInt>(5));
  REQUIRE(CObj_.getField("F1").getAs<JavaInt>() == 5);
}

TEST_CASE("Class manager bootstrap class loader", "[Runtime][ClassManager]") {
  // For now bootstrap class loader works in an ad-hoc manner searching classes
  // in the cwd.

  ClassManager CM;

  REQUIRE_THROWS_AS(CM.getClass("NotFound"), ClassNotFoundException);
  auto &C = CM.getClass("examples/Simple");
  auto &C1 = CM.getClass("examples/Simple");
  REQUIRE(&C == &C1);

  REQUIRE(CM.getDefLoader(C1) == getBootstrapLoader());
}

TEST_CASE("Class manager test class loader", "[Runtime][ClassManager]") {
  // Test class loader is used to load CD test files.

  ClassManager CM;

  REQUIRE_THROWS_AS(CM.getClass("NotFound", getTestLoader()), ClassNotFoundException);
  auto &C = CM.getClass("tests/SlowInterpreter/get_put_static", getTestLoader());
  auto &C1 = CM.getClass("tests/SlowInterpreter/get_put_static", getTestLoader());
  auto &C2 = CM.getClass("examples/Simple");
  REQUIRE(&C == &C1);
  REQUIRE(&C != &C2);

  REQUIRE(CM.getDefLoader(C1) == getTestLoader());
  REQUIRE(CM.getDefLoader(C2) == getBootstrapLoader());
}

TEST_CASE("Class manager verification failure", "[Runtime][ClassManager]") {
  ClassManager CM;

  // Load class
  const auto &C = CM.getClass("tests/Verifier/to_many_locals", getTestLoader());

  // Try to link and initialize but fail in the process
  REQUIRE_THROWS_AS(CM.getClassObject(C), Verifier::VerificationError);
}

TEST_CASE("Class manager correct preparation", "[Runtime][ClassManager]") {
  ClassManager CM;
  const auto &C = CM.getClass("tests/SlowInterpreter/get_put_static", getTestLoader());
  const auto &O = CM.getClassObject(C);

  REQUIRE(O.getField("F1").getAs<JavaInt>() == 0);
  REQUIRE(O.getField("F2").getAs<JavaDouble>() == 0);
}

//TEST_CASE("Class manager correct initialization", "[Runtime][ClassManager]") {
//  ClassManager CM;
//  const auto &C = CM.getClass("examples/Branches");
//  const auto &O = CM.getClassObject(C);
//
//  REQUIRE(O.getField("a").getAs<JavaInt>() == 1);
//  REQUIRE(O.getField("b").getAs<JavaInt>() == 2);
//  REQUIRE(O.getField("c").getAs<JavaInt>() == 3);
//}
