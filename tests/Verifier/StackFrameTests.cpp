//
//
//

#include "catch.hpp"

#include <optional>

#include "Verifier/StackFrame.h"
#include "Verifier/Type.h"

using namespace Verifier;

TEST_CASE("Two word extension", "[Verifier][StackFrame]") {
  // It is responsibility of the StackMap to expand two-word data types

  const StackFrame t1(
      {Type::Int, Type::Double, Type::Class, Type::Long},
      {Type::Int, Type::Double, Type::Class, Type::Long});

  REQUIRE(t1.numLocals() == 6);
  REQUIRE(t1.numStack() == 6);

  std::vector<Type> ExpandedTypes =
      {Type::Int, Type::Double, Type::Top, Type::Class,
       Type::Long, Type::Top};
  REQUIRE(t1.locals() == ExpandedTypes);
}

TEST_CASE("Pop matching list", "[Verifier][StackFrame]") {
  StackFrame t1({}, {});

  // Can't pop if stack is empty
  REQUIRE(!t1.popMatchingList({Type::Int}));
  // However can pop empty list from empty stack
  REQUIRE(t1.popMatchingList({}));

  StackFrame t2({}, {Type::Int, Type::Long, Type::Class, Type::Double});
  // Double pops two stack slots
  REQUIRE(t2.numStack() == 6);
  REQUIRE(t2.popMatchingList({Type::Double}));

  // Can't pop wrong type
  REQUIRE(t2.numStack() == 4);
  REQUIRE(!t2.popMatchingList({Type::Int}));
  REQUIRE(!t2.popMatchingList({Type::TwoWord}));
  REQUIRE(!t2.popMatchingList({Type::Long}));

  // This is atomic operation - pop only happens if it can happen completely.
  // In this example it fails because of the TwoWord type at the end.
  REQUIRE(t2.numStack() == 4);
  REQUIRE(!t2.popMatchingList({Type::Reference, Type::TwoWord, Type::TwoWord}));

  // Can pop subtypes
  REQUIRE(t2.numStack() == 4);
  REQUIRE(t2.popMatchingList({Type::Reference, Type::Long, Type::OneWord}));

  REQUIRE(t2.numStack() == 0);
}

TEST_CASE("Push", "[Verifier][StackFrame]") {
  StackFrame t({}, {});

  t.pushList({Type::Int});
  REQUIRE(t.numStack() == 1);

  t.pushList(
      {Type::Class, Type::Double,
       Type::UninitializedOffset(10), Type::UninitializedOffset(5)});
  REQUIRE(t.numStack() == 6); // two slots for double

  REQUIRE(t.popMatchingList(
      {Type::UninitializedOffset(), Type::UninitializedOffset(10),
       Type::Double, Type::Class, Type::Int}));
}

TEST_CASE("Type transition", "[Verifier][StackFrame]") {
  StackFrame t1({}, {Type::Double, Type::Int, Type::Class});
  REQUIRE(t1.numStack() == 4);

  // Unable to perform transition
  REQUIRE(!t1.doTypeTransition({Type::Int, Type::Int}, Type::Int));
  REQUIRE(t1.numStack() == 4);

  // This is atomic operation
  REQUIRE(!t1.doTypeTransition({Type::Class, Type::Int, Type::Long}, Type::Double));
  REQUIRE(t1.numStack() == 4);

  REQUIRE(t1.doTypeTransition({Type::Class, Type::Int}, Type::Double));
  REQUIRE(t1.numStack() == 4);

  REQUIRE(t1.doTypeTransition({Type::Double, Type::Double}, Type::Int));
  REQUIRE(t1.numStack() == 1);

  REQUIRE(t1.popMatchingList({Type::Int}));
  REQUIRE(t1.numStack() == 0);
}

TEST_CASE("Uninitialized this", "[Verifier][StackFrame]") {
  StackFrame t1({Type::Int, Type::UninitializedThis}, {});
  REQUIRE(t1.flagThisUninit());

  StackFrame t2({Type::Int, Type::Uninitialized}, {});
  REQUIRE(!t2.flagThisUninit());
}

TEST_CASE("Parse field descriptor", "[Verifier][StackFrame]") {
  REQUIRE(StackFrame::parseFieldDescriptor("B") == Type::Byte);
  REQUIRE(StackFrame::parseFieldDescriptor("C") == Type::Char);
  REQUIRE(StackFrame::parseFieldDescriptor("D") == Type::Double);
  REQUIRE(StackFrame::parseFieldDescriptor("F") == Type::Float);
  REQUIRE(StackFrame::parseFieldDescriptor("I") == Type::Int);
  REQUIRE(StackFrame::parseFieldDescriptor("J") == Type::Long);
  REQUIRE(StackFrame::parseFieldDescriptor("S") == Type::Short);
  REQUIRE(StackFrame::parseFieldDescriptor("Z") == Type::Boolean);

  REQUIRE(StackFrame::parseFieldDescriptor("Lclass;") == Type::Class);
  REQUIRE(StackFrame::parseFieldDescriptor("[Lclass;") == Type::Array);

  std::size_t Pos = 0;
  REQUIRE(StackFrame::parseFieldDescriptor("[Lclass;asdfasdf", &Pos) == Type::Array);
  REQUIRE(Pos == 8);

  REQUIRE_THROWS_AS(StackFrame::parseFieldDescriptor("Lclass"),
                    StackFrame::ParsingError);
  REQUIRE_THROWS_AS(StackFrame::parseFieldDescriptor("["),
                    StackFrame::ParsingError);
  REQUIRE_THROWS_AS(StackFrame::parseFieldDescriptor("[wrong"),
                    StackFrame::ParsingError);
  REQUIRE_THROWS_AS(StackFrame::parseFieldDescriptor("[Lwrong"),
                    StackFrame::ParsingError);
  REQUIRE_THROWS_AS(StackFrame::parseFieldDescriptor(""),
                    StackFrame::ParsingError);
  REQUIRE_THROWS_AS(StackFrame::parseFieldDescriptor("U"),
                    StackFrame::ParsingError);
}

TEST_CASE("Parse method descriptor", "[Verifier][StackFrame]") {
  Type RetT = Type::Void;
  std::vector<Type> RawTypes;

  {
    std::tie(RetT, RawTypes) =
        StackFrame::parseMethodDescriptor("([Ljava/lang/String;)I");
    const StackFrame t(RawTypes, {});

    REQUIRE(t.numLocals() == 1);
    REQUIRE(t.numStack() == 0);
    REQUIRE(t.locals()[0] == Type::Array);
    REQUIRE(RetT == Type::Int);
  }

  {
    std::tie(RetT, RawTypes) =
        StackFrame::parseMethodDescriptor(
            "(IDLjava/lang/Thread;)Ljava/lang/Object;");
    const StackFrame t1(RawTypes, {});

    // parseDescriptor returns raw types. I.e double represented as a single
    // entry.
    REQUIRE(RawTypes.size() == 3);
    std::vector<Type> ExpectedTypes = {Type::Int, Type::Double, Type::Class};
    REQUIRE(RawTypes == ExpectedTypes);

    // Check that stack frame had expanded two-word types accordingly
    REQUIRE(t1.numLocals() == 4);
    REQUIRE(t1.numStack() == 0);
    std::vector<Type> ExpandedTypes =
        {Type::Int, Type::Double, Type::Top, Type::Class};
    REQUIRE(t1.locals() == ExpandedTypes);
    REQUIRE(RetT == Type::Class);
  }

  {
    std::tie(RetT, RawTypes) =
        StackFrame::parseMethodDescriptor(
            "()V");
    REQUIRE(RawTypes.empty());
    REQUIRE(RetT == Type::Void);
  }

  REQUIRE_THROWS_AS(
      StackFrame::parseMethodDescriptor("wrong descriptor"),
      StackFrame::ParsingError);
  REQUIRE_THROWS_AS(
      StackFrame::parseMethodDescriptor(""),
      StackFrame::ParsingError);
  REQUIRE_THROWS_AS(
      StackFrame::parseMethodDescriptor("(Lclass"),
      StackFrame::ParsingError);
  REQUIRE_THROWS_AS(
      StackFrame::parseMethodDescriptor("(I;"),
      StackFrame::ParsingError);
  REQUIRE_THROWS_AS(
      StackFrame::parseMethodDescriptor("(I"),
      StackFrame::ParsingError);
  REQUIRE_THROWS_AS(
      StackFrame::parseMethodDescriptor("(I)"),
      StackFrame::ParsingError);
  REQUIRE_THROWS_AS(
      StackFrame::parseMethodDescriptor("(I)Vbla-bla"),
      StackFrame::ParsingError);
  REQUIRE_THROWS_AS(
      StackFrame::parseMethodDescriptor("(I)Dbla-bla"),
      StackFrame::ParsingError);
  REQUIRE_THROWS_AS(
      StackFrame::parseMethodDescriptor("(U)V"),
      StackFrame::ParsingError);
}
