//
// Tests for the slow interpreter
//

#include "catch.hpp"

#include "SlowInterpreter/SlowInterpreter.h"
#include "Verifier/Verifier.h"
#include "Runtime/Value.h"
#include "Runtime/ClassManager.h"
#include "CD/Parser.h"

#include <iostream>

using namespace JavaTypes;
using namespace SlowInterpreter;
using namespace Runtime;

template<typename ResT>
static ResT testWithMethod(
    const JavaClass &Class,
    const Utf8String &Name,
    const std::vector<Value>& InputArgs) {

  auto Method = Class.getMethod(Name);
  assert(Method != nullptr);

#ifndef NDEBUG
  Verifier::verifyMethod(*Method);
#endif

  auto Res = SlowInterpreter::interpret(*Method, InputArgs);
  return Res.getAs<ResT>();
}

TEST_CASE("interpret iconst with ireturn", "[SlowInterpreter]") {
  auto Class =
      CD::parseFromFile("tests/SlowInterpreter/iconst_ireturn.cd");

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      *Class, "test1", {}) == 0);

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      *Class, "test2", {}) == 0);

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      *Class, "test3", {}) == 1);
}

TEST_CASE("interpret dconst with dreturn", "[SlowInterpreter]") {
  auto Class =
      CD::parseFromFile("tests/SlowInterpreter/dconst_dreturn.cd");

  REQUIRE(testWithMethod<Runtime::JavaDouble>(
      *Class, "test1", {}) == 0);

  REQUIRE(testWithMethod<Runtime::JavaDouble>(
      *Class, "test2", {}) == 1);

  REQUIRE(testWithMethod<Runtime::JavaDouble>(
      *Class, "test3", {}) == 1);

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      *Class, "test4", {}) == 1);
}

TEST_CASE("interpret get put static", "[SlowInterpreter]") {
  auto Class =
      CD::parseFromFile("tests/SlowInterpreter/get_put_static.cd");

  getClassManager().registerClass(
      ClassObject::create(*Class)->getAs<ClassObject>());

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      *Class, "test1", {}) == 0);

  REQUIRE(testWithMethod<Runtime::JavaDouble>(
      *Class, "test2", {}) == 0);

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      *Class, "test3", {}) == 1);

  REQUIRE(testWithMethod<Runtime::JavaDouble>(
      *Class, "test4", {}) == 1);
}
