///
/// Tests for the JavaFields class
///

#include "catch.hpp"

#include "JavaTypes/JavaField.h"
#include "Utils/TestUtils.h"
#include "JavaTypes/Type.h"

using namespace JavaTypes;
using namespace TestUtils;

TEST_CASE("Int field", "[JavaField]") {
  JavaField f(
      // I
      getEternalConstantPool().getAs<ConstantPoolRecords::Utf8>(21),
      // F1
      getEternalConstantPool().getAs<ConstantPoolRecords::Utf8>(22),
      JavaField::AccessFlags::ACC_PUBLIC);

  REQUIRE(f.getName() == "F1");
  REQUIRE(f.getDescriptor() == "I");
  REQUIRE(f.getType() == Types::Int);
  REQUIRE(f.getSize() == 4);
}

TEST_CASE("Double field", "[JavaField]") {
  JavaField f(
      // D
      getEternalConstantPool().getAs<ConstantPoolRecords::Utf8>(23),
      // F2
      getEternalConstantPool().getAs<ConstantPoolRecords::Utf8>(24),
      JavaField::AccessFlags::ACC_PUBLIC);

  REQUIRE(f.getName() == "F2");
  REQUIRE(f.getDescriptor() == "D");
  REQUIRE(f.getType() == Types::Double);
  REQUIRE(f.getSize() == 8);
}

TEST_CASE("Reference field", "[JavaField]") {
  JavaField f(
      // LFields;
      getEternalConstantPool().getAs<ConstantPoolRecords::Utf8>(25),
      // Ref
      getEternalConstantPool().getAs<ConstantPoolRecords::Utf8>(26),
      JavaField::AccessFlags::ACC_PUBLIC_STATIC);

  REQUIRE(f.getName() == "Ref");
  REQUIRE(f.getDescriptor() == "LFields;");
  REQUIRE(f.getType() == Types::Class);
  REQUIRE(f.getSize() == 4);
}

TEST_CASE("Incorrect descriptor", "[JavaField]") {
  REQUIRE_THROWS_AS(
      JavaField(
        // LFields
        getEternalConstantPool().getAs<ConstantPoolRecords::Utf8>(27),
        // Ref
        getEternalConstantPool().getAs<ConstantPoolRecords::Utf8>(26),
        JavaField::AccessFlags::ACC_PUBLIC_STATIC),
      Type::ParsingError);
}
