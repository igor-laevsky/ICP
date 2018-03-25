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
    const std::vector<Value>& InputArgs,
    ClassManager &CM) {

  auto Method = Class.getMethod(Name);
  assert(Method != nullptr);

#ifndef NDEBUG
  Verifier::verifyMethod(*Method);
#endif

  auto Res = SlowInterpreter::interpret(*Method, InputArgs, CM);
  return Res.getAs<ResT>();
}

template<class ResT>
static bool runAutoTest(
    const std::string &FileName, const std::vector<Value> &InputArgs,
    bool Debug = false) {

  ClassManager CM;
  const auto &Class = CM.getClass("tests/SlowInterpreter/" + FileName, getTestLoader());

  for (const auto &method: Class.methods()) {
    if (method->getName() == "<init>")
      continue;

    ResT res = SlowInterpreter::interpret(*method, InputArgs, CM, Debug).getAs<ResT>();
    if (res != 0) {
      std::cerr << "Wrong interpreter result: " << res <<
                   " for " << method->getName() << "\n";
      return false;
    }
  }

  return true;
}

TEST_CASE("interpret iconst with ireturn", "[SlowInterpreter]") {
  ClassManager CM;
  const auto &Class =
      CM.getClass("tests/SlowInterpreter/iconst_ireturn", getTestLoader());

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      Class, "test1", {}, CM) == 0);

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      Class, "test2", {}, CM) == 0);

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      Class, "test3", {}, CM) == 1);

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      Class, "test4", {}, CM) == 5);

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      Class, "test5", {}, CM) == -1);
}

TEST_CASE("interpret dconst with dreturn", "[SlowInterpreter]") {
  ClassManager CM;
  const auto &Class =
      CM.getClass("tests/SlowInterpreter/dconst_dreturn", getTestLoader());

  REQUIRE(testWithMethod<Runtime::JavaDouble>(
      Class, "test1", {}, CM) == 0);

  REQUIRE(testWithMethod<Runtime::JavaDouble>(
      Class, "test2", {}, CM) == 1);

  REQUIRE(testWithMethod<Runtime::JavaDouble>(
      Class, "test3", {}, CM) == 1);

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      Class, "test4", {}, CM) == 1);
}

TEST_CASE("interpret get put static", "[SlowInterpreter]") {
  ClassManager CM;
  const auto &Class =
      CM.getClass("tests/SlowInterpreter/get_put_static", getTestLoader());

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      Class, "test1", {}, CM) == 0);

  REQUIRE(testWithMethod<Runtime::JavaDouble>(
      Class, "test2", {}, CM) == 0);

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      Class, "test3", {}, CM) == 1);

  REQUIRE(testWithMethod<Runtime::JavaDouble>(
      Class, "test4", {}, CM) == 1);
}

TEST_CASE("interpret comparisons", "[SlowInterpreter][comparisons]") {
  REQUIRE(runAutoTest<Runtime::JavaInt>("ifcmp", {}));
}

TEST_CASE("interpret iload istore", "[SlowInterpreter][iload_istore]") {
  REQUIRE(runAutoTest<Runtime::JavaInt>("iload_istore",
      {Value::create<JavaInt>(5), Value::create<JavaInt>(0)}));
}

TEST_CASE("interpret iinc", "[SlowInterpreter][iinc]") {
  REQUIRE(runAutoTest<Runtime::JavaInt>("iinc",
      {Value::create<JavaInt>(-5), Value::create<JavaInt>(-12)}));
}

TEST_CASE("interpret goto", "[SlowInterpreter][goto]") {
  REQUIRE(runAutoTest<Runtime::JavaInt>("goto",
      {Value::create<JavaInt>(-5), Value::create<JavaInt>(-12)}));
}

TEST_CASE("interpret iadd", "[SlowInterpreter][iadd]") {
  REQUIRE(runAutoTest<Runtime::JavaInt>("iadd",
      {Value::create<JavaInt>(-5), Value::create<JavaInt>(5)}));
}

//TEST_CASE("interpret new", "[SlowInterpreter][new]") {
//  REQUIRE(runAutoTest<Runtime::JavaInt>("new",
//      {Value::create<JavaInt>(-5), Value::create<JavaInt>(5)}));
//}
