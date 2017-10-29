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
      {Types::Int, Types::Double, Types::Class, Types::Long},
      {Types::Int, Types::Double, Types::Class, Types::Long});

  REQUIRE(t1.numLocals() == 6);
  REQUIRE(t1.numStack() == 6);

  std::vector<Type> ExpandedTypes =
      {Types::Int, Types::Double, Types::Top, Types::Class,
       Types::Long, Types::Top};
  REQUIRE(t1.locals() == ExpandedTypes);
}

TEST_CASE("Pop matching list", "[Verifier][StackFrame]") {
  StackFrame t1({}, {});

  // Can't pop if stack is empty
  REQUIRE(!t1.popMatchingList({Types::Int}));
  // However can pop empty list from empty stack
  REQUIRE(t1.popMatchingList({}));

  StackFrame t2({}, {Types::Int, Types::Long, Types::Class, Types::Double});
  // Double pops two stack slots
  REQUIRE(t2.numStack() == 6);
  REQUIRE(t2.popMatchingList({Types::Double}));

  // Can't pop wrong type
  REQUIRE(t2.numStack() == 4);
  REQUIRE(!t2.popMatchingList({Types::Int}));
  REQUIRE(!t2.popMatchingList({Types::TwoWord}));
  REQUIRE(!t2.popMatchingList({Types::Long}));

  // This is atomic operation - pop only happens if it can happen completely.
  // In this example it fails because of the TwoWord type at the end.
  REQUIRE(t2.numStack() == 4);
  REQUIRE(!t2.popMatchingList({Types::Reference, Types::TwoWord, Types::TwoWord}));

  // Can pop subtypes
  REQUIRE(t2.numStack() == 4);
  REQUIRE(t2.popMatchingList({Types::Reference, Types::Long, Types::OneWord}));

  REQUIRE(t2.numStack() == 0);
}

TEST_CASE("Push", "[Verifier][StackFrame]") {
  StackFrame t({}, {});

  t.pushList({Types::Int});
  REQUIRE(t.numStack() == 1);

  t.pushList(
      {Types::Class, Types::Double,
       Types::UninitializedOffset(10), Types::UninitializedOffset(5)});
  REQUIRE(t.numStack() == 6); // two slots for double

  REQUIRE(t.popMatchingList(
      {Types::UninitializedOffset(), Types::UninitializedOffset(10),
       Types::Double, Types::Class, Types::Int}));
}

TEST_CASE("Type transition", "[Verifier][StackFrame]") {
  StackFrame t1({}, {Types::Double, Types::Int, Types::Class});
  REQUIRE(t1.numStack() == 4);

  // Unable to perform transition
  REQUIRE(!t1.doTypeTransition({Types::Int, Types::Int}, Types::Int));
  REQUIRE(t1.numStack() == 4);

  // This is atomic operation
  REQUIRE(!t1.doTypeTransition({Types::Class, Types::Int, Types::Long}, Types::Double));
  REQUIRE(t1.numStack() == 4);

  REQUIRE(t1.doTypeTransition({Types::Class, Types::Int}, Types::Double));
  REQUIRE(t1.numStack() == 4);

  REQUIRE(t1.doTypeTransition({Types::Double, Types::Double}, Types::Int));
  REQUIRE(t1.numStack() == 1);

  REQUIRE(t1.popMatchingList({Types::Int}));
  REQUIRE(t1.numStack() == 0);
}

TEST_CASE("Uninitialized this", "[Verifier][StackFrame]") {
  StackFrame t1({Types::Int, Types::UninitializedThis}, {});
  REQUIRE(t1.flagThisUninit());

  StackFrame t2({Types::Int, Types::Uninitialized}, {});
  REQUIRE(!t2.flagThisUninit());
}

TEST_CASE("Parse field descriptor", "[Verifier][StackFrame]") {
  REQUIRE(StackFrame::parseFieldDescriptor("B") == Types::Byte);
  REQUIRE(StackFrame::parseFieldDescriptor("C") == Types::Char);
  REQUIRE(StackFrame::parseFieldDescriptor("D") == Types::Double);
  REQUIRE(StackFrame::parseFieldDescriptor("F") == Types::Float);
  REQUIRE(StackFrame::parseFieldDescriptor("I") == Types::Int);
  REQUIRE(StackFrame::parseFieldDescriptor("J") == Types::Long);
  REQUIRE(StackFrame::parseFieldDescriptor("S") == Types::Short);
  REQUIRE(StackFrame::parseFieldDescriptor("Z") == Types::Boolean);

  REQUIRE(StackFrame::parseFieldDescriptor("Lclass;") == Types::Class);
  REQUIRE(StackFrame::parseFieldDescriptor("[Lclass;") == Types::Array);

  std::size_t Pos = 0;
  REQUIRE(StackFrame::parseFieldDescriptor("[Lclass;asdfasdf", &Pos) == Types::Array);
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
  Type RetT = Types::Void;
  std::vector<Type> RawTypes;

  {
    std::tie(RetT, RawTypes) =
        StackFrame::parseMethodDescriptor("([Ljava/lang/String;)I");
    const StackFrame t(RawTypes, {});

    REQUIRE(t.numLocals() == 1);
    REQUIRE(t.numStack() == 0);
    REQUIRE(t.locals()[0] == Types::Array);
    REQUIRE(RetT == Types::Int);
  }

  {
    std::tie(RetT, RawTypes) =
        StackFrame::parseMethodDescriptor(
            "(IDLjava/lang/Thread;)Ljava/lang/Object;");
    const StackFrame t1(RawTypes, {});

    // parseDescriptor returns raw types. I.e double represented as a single
    // entry.
    REQUIRE(RawTypes.size() == 3);
    std::vector<Type> ExpectedTypes = {Types::Int, Types::Double, Types::Class};
    REQUIRE(RawTypes == ExpectedTypes);

    // Check that stack frame had expanded two-word types accordingly
    REQUIRE(t1.numLocals() == 4);
    REQUIRE(t1.numStack() == 0);
    std::vector<Type> ExpandedTypes =
        {Types::Int, Types::Double, Types::Top, Types::Class};
    REQUIRE(t1.locals() == ExpandedTypes);
    REQUIRE(RetT == Types::Class);
  }

  {
    std::tie(RetT, RawTypes) =
        StackFrame::parseMethodDescriptor(
            "()V");
    REQUIRE(RawTypes.empty());
    REQUIRE(RetT == Types::Void);
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

TEST_CASE("Set local", "[Verifier][StackFrame]") {
  StackFrame t({Types::Int, Types::Reference}, {});

  REQUIRE(!t.flagThisUninit());

  t.setLocal(0, Types::UninitializedThis);
  REQUIRE(t.flagThisUninit());

  t.setLocal(1, Types::UninitializedThis);
  REQUIRE(t.flagThisUninit());

  t.setLocal(0, Types::Double);
  REQUIRE(!t.flagThisUninit());
  REQUIRE(t.getLocal(1) == Types::Top);
}
