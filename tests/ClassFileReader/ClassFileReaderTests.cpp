//
// Tests for the class file reader
//

#include "catch.hpp"

#include "SlowInterpreter/SlowInterpreter.h"
#include "Runtime/Value.h"
#include "Runtime/ClassManager.h"
#include "Runtime/Objects.h"
#include "ClassFileReader/ClassFileReader.h"
#include "Verifier/Verifier.h"

#include <iostream>

TEST_CASE("Throw exception if file not found", "[ClassFileReader]") {
  REQUIRE_THROWS_AS(ClassFileReader::loadClassFromFile("wrong name"),
    ClassFileReader::FileNotFound);
}

TEST_CASE("Read verify and interpret simple class", "[ClassFileReader]") {
  auto NewClass = ClassFileReader::loadClassFromFile("./examples/Simple.class");
  assert(NewClass != nullptr);

  Verifier::verify(*NewClass);

  auto Method = NewClass->getMethod("main");
  REQUIRE(Method != nullptr);

  auto Ret = SlowInterpreter::interpret(*Method, {});

  REQUIRE(Ret.getAs<Runtime::JavaInt>() == 0);
}

TEST_CASE("Read verify and interpret fields class", "[ClassFileReader]") {
  auto NewClass = ClassFileReader::loadClassFromFile("./examples/Fields.class");
  assert(NewClass != nullptr);

  Runtime::getClassManager().registerClass(
      Runtime::ClassObject::create(*NewClass)->getAs<Runtime::ClassObject>());

  Verifier::verify(*NewClass);

  auto Method = NewClass->getMethod("main");
  REQUIRE(Method != nullptr);
  auto Ret = SlowInterpreter::interpret(*Method, {});

  REQUIRE(Ret.getAs<Runtime::JavaInt>() == 1);

  Runtime::getClassManager().reset();
}

TEST_CASE("Read verify and interpret branches class", "[ClassFileReader]") {
  auto NewClass = ClassFileReader::loadClassFromFile("./examples/Branches.class");
  assert(NewClass != nullptr);

  Runtime::getClassManager().registerClass(
      Runtime::ClassObject::create(*NewClass)->getAs<Runtime::ClassObject>());

  Verifier::verify(*NewClass);

  // TODO: This should go away with normal class manager
  auto ClInit = NewClass->getMethod("<clinit>");
  REQUIRE(ClInit != nullptr);
  SlowInterpreter::interpret(*ClInit, {});

  auto Method = NewClass->getMethod("main");
  REQUIRE(Method != nullptr);
  auto Ret = SlowInterpreter::interpret(*Method, {});

  REQUIRE(Ret.getAs<Runtime::JavaInt>() == 1);

  Runtime::getClassManager().reset();
}

TEST_CASE("Read verify and interpret loop class", "[ClassFileReader]") {
  auto NewClass = ClassFileReader::loadClassFromFile("./examples/Loop.class");
  assert(NewClass != nullptr);

  Runtime::getClassManager().registerClass(
      Runtime::ClassObject::create(*NewClass)->getAs<Runtime::ClassObject>());

  Verifier::verify(*NewClass);

  // TODO: This should go away with normal class manager
  auto ClInit = NewClass->getMethod("<clinit>");
  REQUIRE(ClInit != nullptr);
  SlowInterpreter::interpret(*ClInit, {});

  auto Method = NewClass->getMethod("main");
  REQUIRE(Method != nullptr);
  auto Ret = SlowInterpreter::interpret(*Method, {});

  REQUIRE(Ret.getAs<Runtime::JavaInt>() == 6);

  Runtime::getClassManager().reset();
}
