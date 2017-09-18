//
// Tests for the class file reader
//

#include "catch.hpp"

#include "ClassFileReader/ClassFileReader.h"

TEST_CASE("Throw exception if file not found", "[ClassFileReader]") {
  REQUIRE_THROWS_AS(ClassFileReader::loadClassFromFile("wrong name"),
    ClassFileReader::FileNotFound);
}
