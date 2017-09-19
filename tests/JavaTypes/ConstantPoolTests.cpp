#include "catch.hpp"

#include "JavaTypes/ConstantPool.h"
#include "JavaTypes/ConstantPoolRecords.h"

using namespace JavaTypes;
using namespace JavaTypes::ConstantPoolRecords;

TEST_CASE("Basic constant pool construction", "[ConstantPool]") {
  ConstantPoolBuilder Builder(2);
  REQUIRE(Builder.isValid());

  Builder.set(0, std::make_unique<ConstantPoolRecords::ClassInfo>(
      Builder.getCellReference(1)));
  REQUIRE(Builder.isValid());

  Builder.set(1, std::make_unique<ConstantPoolRecords::StringRecord>("test"));
  REQUIRE(Builder.isValid());

  std::unique_ptr<ConstantPool> CP = Builder.createConstantPool();
  REQUIRE_FALSE(Builder.isValid());

  const auto *CI = CP->getAsOrNull<ClassInfo>(0);
  REQUIRE(CI != nullptr);
  REQUIRE(CI->isValid());
  const auto *StrRec = CP->getAsOrNull<StringRecord>(1);
  REQUIRE(StrRec != nullptr);
  REQUIRE(StrRec->isValid());

  REQUIRE(StrRec->getValue() == "test");
  REQUIRE(CI->getName() == "test");
}
