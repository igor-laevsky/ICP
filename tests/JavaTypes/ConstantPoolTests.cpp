#include "catch.hpp"

#include "JavaTypes/ConstantPool.h"
#include "JavaTypes/ConstantPoolRecords.h"

using namespace JavaTypes;
using namespace JavaTypes::ConstantPoolRecords;

TEST_CASE("Basic constant pool construction", "[ConstantPool]") {
  ConstantPoolBuilder Builder(2);
  REQUIRE(Builder.isValid());

  Builder.set(1, std::make_unique<ConstantPoolRecords::ClassInfo>(
      Builder.getCellReference<ConstantPoolRecords::Utf8>(2)));
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

TEST_CASE("Constant pool type safety", "[ConstantPool]") {
  ConstantPoolBuilder Builder(2);

  (void)Builder.getCellReference<ConstantPoolRecords::Utf8>(1);
  // Matching types.
  REQUIRE_NOTHROW(Builder.getCellReference<ConstantPoolRecords::Utf8>(1));
  // Not matching types.
  REQUIRE_THROWS(Builder.getCellReference<ConstantPoolRecords::ClassInfo>(1));

  // Matching types.
  REQUIRE_NOTHROW(Builder.getCellReference<ConstantPoolRecords::ClassInfo>(2));
  // Non matching types.
  REQUIRE_THROWS(Builder.getCellReference<ConstantPoolRecords::NameAndType>(2));

  // Can't save Utf8 into ClassInfo.
  REQUIRE_THROWS(
      Builder.set(2, std::make_unique<ConstantPoolRecords::Utf8>("test")));

  // But can set ClassInfo into ClassInfo.
  REQUIRE_NOTHROW(Builder.set(2, std::make_unique<ConstantPoolRecords::ClassInfo>(
      Builder.getCellReference<ConstantPoolRecords::Utf8>(1))));

  // And can set Utf8 into Utf8.
  REQUIRE_NOTHROW(
      Builder.set(1, std::make_unique<ConstantPoolRecords::Utf8>("asdf")));
}
