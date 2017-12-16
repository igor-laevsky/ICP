//
// Tests for the slow interpreter
//

#include "catch.hpp"

#include "SlowInterpreter/SlowInterpreter.h"
#include "Runtime/Value.h"
#include "CD/Parser.h"

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

  auto Res = SlowInterpreter::interpret(*Method, InputArgs);
  return Res.getAs<ResT>();
}

TEST_CASE("iconst with ireturn", "[SlowInterpreter]") {
  auto Class =
      CD::parseFromFile("tests/SlowInterpreter/iconst_ireturn.cd");

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      *Class, "test1", {}) == 0);

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      *Class, "test2", {}) == 0);

  REQUIRE(testWithMethod<Runtime::JavaInt>(
      *Class, "test3", {}) == 1);
}
