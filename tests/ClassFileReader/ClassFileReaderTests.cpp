//
// Tests for the class file reader
//

#include "catch.hpp"

#include "SlowInterpreter/SlowInterpreter.h"
#include "Runtime/Value.h"
#include "Runtime/ClassManager.h"
#include "Runtime/Objects.h"
#include "Runtime/ClassManager.h"
#include "ClassFileReader/ClassFileReader.h"
#include "Verifier/Verifier.h"

#include <iostream>

TEST_CASE("Throw exception if file not found", "[ClassFileReader]") {
  REQUIRE_THROWS_AS(ClassFileReader::loadClassFromFile("wrong name"),
    ClassFileReader::FileNotFound);
}

template<class ResT = Runtime::JavaInt>
ResT testSingleClass(const std::string &ClassName) {
  Runtime::ClassManager CM;
  auto &NewClass = CM.getClass(ClassName, Runtime::getBootstrapLoader());

  Verifier::verify(NewClass);

  auto *Method = NewClass.getMethod("main");
  REQUIRE(Method != nullptr);

  auto Ret = SlowInterpreter::interpret(*Method, {}, CM);
  return Ret.getAs<Runtime::JavaInt>();
}

TEST_CASE("Read verify and interpret simple class", "[ClassFileReader]") {
  REQUIRE(testSingleClass("./examples/Simple") == 0);
}

TEST_CASE("Read verify and interpret fields class", "[ClassFileReader]") {
  REQUIRE(testSingleClass("./examples/Fields") == 1);
}

TEST_CASE("Read verify and interpret branches class", "[ClassFileReader]") {
  REQUIRE(testSingleClass("./examples/Branches") == 1);
}

TEST_CASE("Read verify and interpret loop class", "[ClassFileReader]") {
  REQUIRE(testSingleClass("./examples/Loop") == 6);
}

TEST_CASE("Read verify and interpret simple new class", "[ClassFileReader]") {
  REQUIRE(testSingleClass("./examples/SimpleNew") == 50);
}
