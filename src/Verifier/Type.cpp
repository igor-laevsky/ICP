//
// Implementation of the Type class
//

#include "Type.h"

#include <cassert>

using namespace Verifier;

Type Type::Top(Type::TagType::TOP);
Type Type::OneWord(Type::TagType::ONE_WORD);
Type Type::TwoWord(Type::TagType::TWO_WORD);

Type Type::Int(Type::TagType::INT);
Type Type::Float(Type::TagType::FLOAT);
Type Type::Long(Type::TagType::LONG);
Type Type::Double(Type::TagType::DOUBLE);

Type Type::Reference(Type::TagType::REFERENCE);
Type Type::Uninitialized(Type::TagType::UNINITIALIZED);
Type Type::UninitializedThis(Type::TagType::UNINITIALIZED_THIS);

Type Type::UninitializedOffset() {
  static auto Ret = Type(Type::TagType::UNINITIALIZED_OFFSET);
  return Ret;
}

Type Type::UninitializedOffset(uint32_t Offset) {
  return Type(Type::TagType::UNINITIALIZED_OFFSET, Offset);
}

Type Type::Class(Type::TagType::CLASS);
Type Type::Array(Type::TagType::ARRAY);
Type Type::Null(Type::TagType::NULL_TAG);


bool Type::isAssignable(Type From, Type To) {
  // Can assign to the same type
  if (From == To)
    return true;

  // Top can only be assigned to itself
  if (From == Type::Top)
    return false;

  // Everything can be assigned to top
  if (To == Type::Top)
    return true;

  if (From == Type::OneWord || From == Type::TwoWord)
    return To == Type::Top;

  // Recursion here is not necessary, however it provides more clear
  // implementation. If performance would ever become an issue this should be
  // optimized.
  if (From == Type::Int || From == Type::Float || From == Type::Reference)
    return isAssignable(Type::OneWord, To);

  if (From == Type::Long || From == Type::Double)
    return isAssignable(Type::TwoWord, To);

  if (From == Type::Uninitialized)
    return isAssignable(Type::Reference, To);

  if (From == Type::UninitializedThis || From == Type::UninitializedOffset())
    return isAssignable(Type::Uninitialized, To);

  if (From == Type::Class || From == Type::Array)
    return isAssignable(Type::Reference, To);

  if (From == Type::Null)
    return isAssignable(Type::Class, To) || isAssignable(Type::Array, To);

  assert(false); // All types should be covered
  return false;
}

std::size_t Type::sizeOf(Type T) {
  if (isAssignable(T, Type::OneWord))
    return 1;
  else if (isAssignable(T, Type::TwoWord))
    return 2;
  assert(false); // should cover all types
  return 0;
}
