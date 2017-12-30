///
/// Tests for the StackMapTable class
///

// #include "catch.hpp"

// #include "JavaTypes/Type.h"
// #include "Verifier/StackFrame.h"

// using namespace JavaTypes;
// using namespace Verifier;

// TEST_CASE("basic stack map table construction", "[StackMapTable]") {
//   StackMapTableBuilder Builder({Types::Long}, {});

//   Builder.addAppend(2, {Types::Int});
//   Builder.addAppend(4, {Types::Double, Types::Int});
//   Builder.addSame(8);
//   Builder.addFull(9, {Types::Long}, {Types::Double, Types::Int});

//   StackMapTable Table = Builder.getTable();

//   REQUIRE(Table.hasBci(2));
//   REQUIRE(Table.hasBci(4));
//   REQUIRE(Table.hasBci(8));
//   REQUIRE(!Table.hasBci(0));
//   REQUIRE(!Table.hasBci(100000));

//   auto f1 = Table.getAtBci(2);
//   auto f2 = Table.getAtBci(4);
//   auto f3 = Table.getAtBci(8);
//   auto f4 = Table.getAtBci(9);

//   REQUIRE(f1.getLocal(2) == Types::Int);
//   REQUIRE(f2.getLocal(5) == Types::Double);
//   REQUIRE(f2.getLocal(6) == Types::Int);
//   REQUIRE(f3 == f2);
//   REQUIRE(f4 == StackFrame({Types::Long}, {Types::Double, Types::Int}));
// }
