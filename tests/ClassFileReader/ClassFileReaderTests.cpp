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

TEST_CASE("Read verify and interpret simple class", "[ClassFileReader]") {
  Runtime::ClassManager CM;
  auto &NewClass = CM.getClass("./examples/Simple");

  Verifier::verify(NewClass);

  auto *Method = NewClass.getMethod("main");
  REQUIRE(Method != nullptr);

  auto Ret = SlowInterpreter::interpret(*Method, {}, CM);
  REQUIRE(Ret.getAs<Runtime::JavaInt>() == 0);
}

TEST_CASE("Read verify and interpret fields class", "[ClassFileReader]") {
  Runtime::ClassManager CM;
  auto &NewClass = CM.getClass("./examples/Fields");

  Verifier::verify(NewClass);

  auto *Method = NewClass.getMethod("main");
  REQUIRE(Method != nullptr);
  auto Ret = SlowInterpreter::interpret(*Method, {}, CM);

  REQUIRE(Ret.getAs<Runtime::JavaInt>() == 1);
}

TEST_CASE("Read verify and interpret branches class", "[ClassFileReader]") {
  Runtime::ClassManager CM;
  auto &NewClass = CM.getClass("./examples/Branches");

  Verifier::verify(NewClass);

  auto *Method = NewClass.getMethod("main");
  REQUIRE(Method != nullptr);
  auto Ret = SlowInterpreter::interpret(*Method, {}, CM);

  REQUIRE(Ret.getAs<Runtime::JavaInt>() == 1);
}

TEST_CASE("Read verify and interpret loop class", "[ClassFileReader]") {
  Runtime::ClassManager CM;
  auto &NewClass = CM.getClass("./examples/Loop");

  Verifier::verify(NewClass);

  auto *Method = NewClass.getMethod("main");
  REQUIRE(Method != nullptr);
  auto Ret = SlowInterpreter::interpret(*Method, {}, CM);

  REQUIRE(Ret.getAs<Runtime::JavaInt>() == 6);
}
