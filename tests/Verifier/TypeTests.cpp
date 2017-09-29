//
//
//

#include "catch.hpp"

#include "Verifier/Type.h"

using namespace Verifier;

TEST_CASE("Exact relation", "[Verifier][Types]") {
  Type t1 = Type::Top;
  Type t2 = Type::Uninitialized;
  Type t3 = Type::UninitializedOffset(10);
  Type t4 = Type::Reference;

  REQUIRE(t1 == Type::Top);
  REQUIRE(t2 == Type::Uninitialized);

  REQUIRE(t3 == Type::UninitializedOffset(10));
  REQUIRE(t3 != Type::UninitializedOffset(5));
  REQUIRE(Type::UninitializedOffset(5) == Type::UninitializedOffset(5));

  // Empty brackets match all offsets
  REQUIRE(t3 == Type::UninitializedOffset());
  REQUIRE(Type::UninitializedOffset(5) == Type::UninitializedOffset());
  REQUIRE(Type::UninitializedOffset() == Type::UninitializedOffset());
  REQUIRE(Type::Uninitialized != Type::UninitializedOffset());

  REQUIRE(Type::Array != Type::Class);
  REQUIRE(Type::Class == Type::Class);
  REQUIRE(Type::Null != Type::Class);

  // Subtypes do not match exactly
  REQUIRE(t2 != t1);
  REQUIRE(t4 != t1);
}

TEST_CASE("Subtype relation", "[Verifier][Types]") {
  REQUIRE(Type::isAssignable(Type::OneWord, Type::Top));
  REQUIRE(Type::isAssignable(Type::TwoWord, Type::Top));

  REQUIRE(Type::isAssignable(Type::Int, Type::OneWord));
  REQUIRE(Type::isAssignable(Type::Float, Type::OneWord));
  REQUIRE(Type::isAssignable(Type::Long, Type::TwoWord));
  REQUIRE(Type::isAssignable(Type::Double, Type::TwoWord));

  REQUIRE(Type::isAssignable(Type::Reference, Type::OneWord));

  REQUIRE(Type::isAssignable(Type::Uninitialized, Type::Reference));
  REQUIRE(Type::isAssignable(Type::UninitializedThis, Type::Uninitialized));
  REQUIRE(Type::isAssignable(Type::UninitializedOffset(), Type::Uninitialized));
  REQUIRE(Type::isAssignable(Type::UninitializedOffset(5), Type::Uninitialized));

  REQUIRE(Type::isAssignable(Type::Class, Type::Reference));
  REQUIRE(Type::isAssignable(Type::Class, Type::OneWord));

  REQUIRE(Type::isAssignable(Type::Array, Type::Reference));
  REQUIRE(!Type::isAssignable(Type::Array, Type::Class));

  REQUIRE(Type::isAssignable(Type::Null, Type::Array));
  REQUIRE(Type::isAssignable(Type::Null, Type::Class));
  REQUIRE(Type::isAssignable(Type::Null, Type::Reference));
  REQUIRE(!Type::isAssignable(Type::Null, Type::Uninitialized));

  // Some transitive checks
  REQUIRE(Type::isAssignable(Type::UninitializedOffset(5), Type::Top));
  REQUIRE(Type::isAssignable(Type::UninitializedThis, Type::Reference));

  // Some negative checks
  REQUIRE(!Type::isAssignable(Type::Int, Type::Reference));
  REQUIRE(!Type::isAssignable(Type::Float, Type::Int));
  REQUIRE(!Type::isAssignable(Type::Top, Type::Reference));
  REQUIRE(!Type::isAssignable(Type::UninitializedOffset(), Type::Float));
}