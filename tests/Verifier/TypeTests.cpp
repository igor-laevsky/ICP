//
//
//

#include "catch.hpp"

#include "Verifier/Type.h"

using namespace Verifier;

TEST_CASE("Exact relation", "[Verifier][Types]") {
  Type t1 = Types::Top;
  Type t2 = Types::Uninitialized;
  Type t3 = Types::UninitializedOffset(10);
  Type t4 = Types::Reference;

  REQUIRE(t1 == Types::Top);
  REQUIRE(t2 == Types::Uninitialized);

  REQUIRE(t3 == Types::UninitializedOffset(10));
  REQUIRE(t3 != Types::UninitializedOffset(5));
  REQUIRE(Types::UninitializedOffset(5) == Types::UninitializedOffset(5));

  // Empty brackets match all offsets
  REQUIRE(t3 == Types::UninitializedOffset());
  REQUIRE(Types::UninitializedOffset(5) == Types::UninitializedOffset());
  REQUIRE(Types::UninitializedOffset() == Types::UninitializedOffset());
  REQUIRE(Types::Uninitialized != Types::UninitializedOffset());

  REQUIRE(Types::Array != Types::Class);
  REQUIRE(Types::Class == Types::Class);
  REQUIRE(Types::Null != Types::Class);

  // Subtypes do not match exactly
  REQUIRE(t2 != t1);
  REQUIRE(t4 != t1);
}

TEST_CASE("Subtype relation", "[Verifier][Types]") {
  REQUIRE(Types::isAssignable(Types::OneWord, Types::Top));
  REQUIRE(Types::isAssignable(Types::TwoWord, Types::Top));

  REQUIRE(Types::isAssignable(Types::Int, Types::OneWord));
  REQUIRE(Types::isAssignable(Types::Float, Types::OneWord));
  REQUIRE(Types::isAssignable(Types::Long, Types::TwoWord));
  REQUIRE(Types::isAssignable(Types::Double, Types::TwoWord));

  REQUIRE(Types::isAssignable(Types::Reference, Types::OneWord));

  REQUIRE(Types::isAssignable(Types::Uninitialized, Types::Reference));
  REQUIRE(Types::isAssignable(Types::UninitializedThis, Types::Uninitialized));
  REQUIRE(Types::isAssignable(Types::UninitializedOffset(), Types::Uninitialized));
  REQUIRE(Types::isAssignable(Types::UninitializedOffset(5), Types::Uninitialized));

  REQUIRE(Types::isAssignable(Types::Class, Types::Reference));
  REQUIRE(Types::isAssignable(Types::Class, Types::OneWord));

  REQUIRE(Types::isAssignable(Types::Array, Types::Reference));
  REQUIRE(!Types::isAssignable(Types::Array, Types::Class));

  REQUIRE(Types::isAssignable(Types::Null, Types::Array));
  REQUIRE(Types::isAssignable(Types::Null, Types::Class));
  REQUIRE(Types::isAssignable(Types::Null, Types::Reference));
  REQUIRE(!Types::isAssignable(Types::Null, Types::Uninitialized));

  REQUIRE(Types::isAssignable(Types::Short, Types::OneWord));
  REQUIRE(Types::isAssignable(Types::Boolean, Types::Int));
  REQUIRE(!Types::isAssignable(Types::Char, Types::TwoWord));
  REQUIRE(!Types::isAssignable(Types::Byte, Types::Reference));

  // Some transitive checks
  REQUIRE(Types::isAssignable(Types::UninitializedOffset(5), Types::Top));
  REQUIRE(Types::isAssignable(Types::UninitializedThis, Types::Reference));

  // Some negative checks
  REQUIRE(!Types::isAssignable(Types::Int, Types::Reference));
  REQUIRE(!Types::isAssignable(Types::Float, Types::Int));
  REQUIRE(!Types::isAssignable(Types::Top, Types::Reference));
  REQUIRE(!Types::isAssignable(Types::UninitializedOffset(), Types::Float));
}

TEST_CASE("Type size", "[Verifier][Types]") {
  REQUIRE(Types::sizeOf(Types::Top) == 1);
  REQUIRE(Types::sizeOf(Types::OneWord) == 1);
  REQUIRE(Types::sizeOf(Types::Int) == 1);
  REQUIRE(Types::sizeOf(Types::Class) == 1);
  REQUIRE(Types::sizeOf(Types::Null) == 1);
  REQUIRE(Types::sizeOf(Types::Byte) == 1);
  REQUIRE(Types::sizeOf(Types::TwoWord) == 2);
  REQUIRE(Types::sizeOf(Types::Double) == 2);
  REQUIRE(Types::sizeOf(Types::Long) == 2);
}

TEST_CASE("Convert to verifier type", "[Verifier][Types]") {
  REQUIRE(Types::toVerificationType(Types::Int) == Types::Int);
  REQUIRE(Types::toVerificationType(Types::Float) == Types::Float);
  REQUIRE(Types::toVerificationType(Types::Byte) == Types::Int);
  REQUIRE(Types::toVerificationType(Types::Char) == Types::Int);
  REQUIRE(Types::toVerificationType(Types::Boolean) == Types::Int);
  REQUIRE(Types::toVerificationType(Types::Short) == Types::Int);
}