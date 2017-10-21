//
// Various verifier tests
//

#include "catch.hpp"

#include "Utils/TestUtils.h"

#include "Verifier/Verifier.h"

TEST_CASE("Basic verification", "[Verifier]") {
  std::vector<std::unique_ptr<JavaTypes::JavaMethod>> Methods;
  Methods.push_back(TestUtils::createMethod({0x3, 0xac}));


  auto TrivialClass = TestUtils::createClass(std::move(Methods));

  REQUIRE_NOTHROW(Verifier::verify(*TrivialClass));
}
