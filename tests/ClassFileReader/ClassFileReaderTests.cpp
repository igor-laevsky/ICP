//
// Tests for the class file reader
//

#include "catch.hpp"

#include "SlowInterpreter/SlowInterpreter.h"
#include "ClassFileReader/ClassFileReader.h"
#include "Verifier/Verifier.h"

TEST_CASE("Throw exception if file not found", "[ClassFileReader]") {
  REQUIRE_THROWS_AS(ClassFileReader::loadClassFromFile("wrong name"),
    ClassFileReader::FileNotFound);
}

TEST_CASE("Read verify and interpret simple class", "[ClassFileReader]") {
  auto NewClass = ClassFileReader::loadClassFromFile("./assets/Simple.class");
  assert(NewClass != nullptr);

  Verifier::verify(*NewClass);

  auto Method = NewClass->getMethod("main");
  REQUIRE(Method != nullptr);

  auto Ret = SlowInterpreter::interpret(*Method, {});
  REQUIRE(Ret.has_value());

  REQUIRE_NOTHROW(std::any_cast<SlowInterpreter::JavaInt>(Ret));
}
