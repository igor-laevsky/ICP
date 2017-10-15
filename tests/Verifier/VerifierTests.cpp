//
// Various verifier tests
//

#include "catch.hpp"

#include "Utils/TestUtils.h"

#include "Verifier/Verifier.h"

TEST_CASE("Basic verification", "[Verifier]") {
  auto TrivialClass = TestUtils::createTrivialClass();
  REQUIRE_NOTHROW(Verifier::verify(*TrivialClass));
}
