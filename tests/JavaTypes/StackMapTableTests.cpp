///
/// Tests for the StackMapTable class
///

#include "catch.hpp"

#include "JavaTypes/Type.h"
#include "JavaTypes/StackFrame.h"
#include "JavaTypes/StackMapTable.h"

using namespace JavaTypes;

TEST_CASE("Basic stack map table construction", "[StackMapTable]") {
  StackMapTableBuilder Builder;

  Builder.addAppend(2, {Types::Int});
  Builder.addAppend(4, {Types::Double, Types::Int});
  Builder.addSame(8);
  Builder.addFull(9, {Types::Long}, {Types::Double, Types::Int});

  std::vector<Type> LocalTypes;
  Type ReturnType = Types::Void;
  std::tie(ReturnType, LocalTypes) = Type::parseMethodDescriptor("(J)V");
  StackMapTable Table = Builder.createTable(LocalTypes);

  // Check iteration order
  auto It = Table.begin();
  REQUIRE(It.getBci() == 2);
  const auto &f1 = *It++;
  REQUIRE(It.getBci() == 4);
  const auto &f2 = *It++;
  REQUIRE(It.getBci() == 8);
  const auto &f3 = *It++;
  REQUIRE(It.getBci() == 9);
  const auto &f4 = *It++;
  REQUIRE(It == Table.end());

  // Check that we created correct frames
  REQUIRE(f1.getLocal(2) == Types::Int);

  REQUIRE(f2.getLocal(3) == Types::Double);
  REQUIRE(f2.getLocal(5) == Types::Int);

  REQUIRE(f3 == f2);
  REQUIRE(f4 == StackFrame({Types::Long}, {Types::Double, Types::Int}));
}
