#include "catch.hpp"

#include "JavaTypes/ConstantPool.h"
#include "JavaTypes/ConstantPoolRecords.h"

using namespace JavaTypes;
using namespace JavaTypes::ConstantPoolRecords;

TEST_CASE("Basic constant pool construction", "[ConstantPool]") {
  ConstantPoolBuilder Builder(2);
  REQUIRE(Builder.isValid());

  Builder.set(1, std::make_unique<ConstantPoolRecords::ClassInfo>(
      Builder.getCellReference(2)));
  REQUIRE(Builder.isValid());

  Builder.set(2, std::make_unique<ConstantPoolRecords::Utf8>("test"));
  REQUIRE(Builder.isValid());

  std::unique_ptr<ConstantPool> CP = Builder.createConstantPool();
  REQUIRE_FALSE(Builder.isValid());

  const auto *CI = CP->getAsOrNull<ClassInfo>(1);
  REQUIRE(CI != nullptr);
  REQUIRE(CI->isValid());
  const auto *StrRec = CP->getAsOrNull<Utf8>(2);
  REQUIRE(StrRec != nullptr);
  REQUIRE(StrRec->isValid());

  REQUIRE(StrRec->getValue() == "test");
  REQUIRE(CI->getName() == "test");

  std::string Error;
  bool IsValid = CP->verify(Error);
  REQUIRE(IsValid);
}

TEST_CASE("Constant pool with wrong record type", "[ConstantPool]") {
  ConstantPoolBuilder Builder(2);
  REQUIRE(Builder.isValid());

  Builder.set(1, std::make_unique<ConstantPoolRecords::ClassInfo>(
      Builder.getCellReference(1)));
  REQUIRE(Builder.isValid());

  Builder.set(2, std::make_unique<ConstantPoolRecords::ClassInfo>(
      Builder.getCellReference(2)));
  REQUIRE(Builder.isValid());

  std::unique_ptr<ConstantPool> CP = Builder.createConstantPool();
  REQUIRE_FALSE(Builder.isValid());

  const auto *CI = CP->getAsOrNull<ClassInfo>(1);
  REQUIRE(CI != nullptr);
  REQUIRE_FALSE(CI->isValid());
  const auto *StrRec = CP->getAsOrNull<Utf8>(2);
  REQUIRE(StrRec == nullptr);
  const auto *Rec2 = CP->getAsOrNull<ClassInfo>(2);
  REQUIRE(Rec2 != nullptr);
  REQUIRE_FALSE(Rec2->isValid());

  std::string Error;
  bool IsValid = CP->verify(Error);
  REQUIRE_FALSE(IsValid);
  REQUIRE(Error == "Invalid record at index 1");
}
