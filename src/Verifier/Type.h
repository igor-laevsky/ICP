//
// Representation of the verification type system.
//

#ifndef ICP_TYPE_H
#define ICP_TYPE_H

#include <optional>
#include <cassert>

namespace Verifier {

// Class representing verifier data type.
// Direct construction is forbidden. In order to refer to the specific types
// user is supposed to use static variables from the 'Types' struct.
// This is intended to be a small immutable value-like class
// (i.e pass by value is fine).
class Type final {
public:
  constexpr bool operator==(const Type &Rhs) const noexcept {
    if (Tag != Rhs.Tag)
      return false;

    // This is to support wildcard matching of parametrized types
    if (!Data || !Rhs.Data)
      return true;

    return Data == Rhs.Data;
  }

  constexpr bool operator!=(const Type &Rhs) const noexcept {
    return !(*this == Rhs);
  }

  // Explicitly state that moves and copies are allowed.
  Type(const Type&) = default;
  Type &operator=(const Type &) = default;
  Type(Type&&) = default;
  Type &operator=(Type &&) = default;

private:
  enum class TagType {
    TOP = 0,
    ONE_WORD,
    TWO_WORD,
    INT,
    BYTE,
    CHAR,
    SHORT,
    BOOLEAN,
    FLOAT,
    LONG,
    DOUBLE,
    REFERENCE,
    UNINITIALIZED,
    UNINITIALIZED_THIS,
    UNINITIALIZED_OFFSET,
    ARRAY,
    CLASS,
    NULL_TAG,
    VOID
  };

  TagType Tag;

  // Once array, class and null type are implemented this will be replaced
  // with std::variant.
  using DataType = uint32_t;

  std::optional<DataType> Data;

private:
  explicit constexpr Type(TagType Tag) noexcept:
      Tag(Tag),
      Data() {
    ;
  }

  constexpr Type(TagType Tag, DataType Data) noexcept:
      Tag(Tag),
      Data(Data) {
    ;
  }

  friend struct Types;
};

// Definitions for the all available types.
// Types with parameters are constructed via static functions. Functions without
// parameters return type which exactly matches same type with any parameter.
// I.e UninitializedOffset() will exactly match UninitializedOffset(10).
// Which represents uninitializedOffset(_) from the JVM specification.
// This struct also contains utility functions to work on 'Type' objects, which
// is done mainly for the sake of uniformity in the used code.

// Implementation note:
// Goal here is to make all this types constexpr, so they can be used in a
// constexpr expressions.
// Two important things are happening here:
//  1. These definitions are moved out of the 'Type' class. It is not allowed
//     to use incomplete types in a constexpr initializer, hence we first
//     complete the 'Type', then define all the variables.
//  2. 'struct' is used instead of 'namespace'. We want to keep 'Type'
//      constructor private. However we need to somehow construct this objects
//      out-of-line. Hence we used 'struct' which is a friend of 'Type' class.
// TODO: Add parameters for the class and array types
struct Types final {
  // This is a null object. It doesn't reflect any real world type and should
  // be used to indicate that type is unknown.
  static constexpr Type Void{Type::TagType::VOID};

  static constexpr Type Top{Type::TagType::TOP};
  static constexpr Type OneWord{Type::TagType::ONE_WORD};
  static constexpr Type TwoWord{Type::TagType::TWO_WORD};

  static constexpr Type Int{Type::TagType::INT};
  static constexpr Type Byte{Type::TagType::BYTE};
  static constexpr Type Char{Type::TagType::CHAR};
  static constexpr Type Short{Type::TagType::SHORT};
  static constexpr Type Boolean{Type::TagType::BOOLEAN};

  static constexpr Type Float{Type::TagType::FLOAT};
  static constexpr Type Long{Type::TagType::LONG};
  static constexpr Type Double{Type::TagType::DOUBLE};

  static constexpr Type Reference{Type::TagType::REFERENCE};
  static constexpr Type Uninitialized{Type::TagType::UNINITIALIZED};
  static constexpr Type UninitializedThis{Type::TagType::UNINITIALIZED_THIS};

  static constexpr Type UninitializedOffset() {
    return Type(Type::TagType::UNINITIALIZED_OFFSET);
  }

  static constexpr Type UninitializedOffset(uint32_t Offset) {
    return Type(Type::TagType::UNINITIALIZED_OFFSET, Offset);
  }

  static constexpr Type Class{Type::TagType::CLASS};
  static constexpr Type Array{Type::TagType::ARRAY};
  static constexpr Type Null{Type::TagType::NULL_TAG};


  //  Returns 1 for OneWord types and 2 for TwoWord types.
  static constexpr std::size_t sizeOf(const Type &T) noexcept;

  // This function mirrors same function from the JVm specification.
  // Essentially it represents subtyping relation on the verifier types.
  // See jvms 4.10.1.2 for clear visualization of the type system.
  static constexpr bool isAssignable(const Type &From, const Type &To) noexcept;

  // Convert type to verification type.
  // This means converting  byte, char, short, and boolean into integer type.
  static constexpr Type toVerificationType(const Type &From) noexcept;


  // This struct is used as a namespace, it's not allowed to construct it.
  Types() = delete;
};

//
// Implementation of the utility functions.
//

constexpr bool Types::isAssignable(const Type &From, const Type &To) noexcept {
  // Can assign to the same type
  if (From == To)
    return true;

  // Top can only be assigned to itself
  if (From == Types::Top)
    return false;

  // Everything can be assigned to top
  if (To == Types::Top)
    return true;

  if (From == Types::OneWord || From == Types::TwoWord)
    return To == Types::Top;

  // Recursion here is not necessary, however it provides more clear
  // implementation. If performance would ever become an issue this should be
  // optimized.
  if (From == Types::Int || From == Types::Float || From == Types::Reference)
    return isAssignable(Types::OneWord, To);

  if (From == Types::Long || From == Types::Double)
    return isAssignable(Types::TwoWord, To);

  if (From == Types::Uninitialized)
    return isAssignable(Types::Reference, To);

  if (From == Types::UninitializedThis || From == Types::UninitializedOffset())
    return isAssignable(Types::Uninitialized, To);

  if (From == Types::Class || From == Types::Array)
    return isAssignable(Types::Reference, To);

  if (From == Types::Char || From == Types::Short ||
      From == Types::Byte || From == Types::Boolean)
    return isAssignable(Types::Int, To);

  if (From == Types::Null)
    return isAssignable(Types::Class, To) || isAssignable(Types::Array, To);

  // TODO: assert from constexpr function is not possible due to the bug in MinGW
  //assert(false); // All types should be covered
  return false;
}

constexpr std::size_t Types::sizeOf(const Type &T) noexcept {
  if (T == Types::Top)
    return 1;
  else if (isAssignable(T, Types::OneWord))
    return 1;
  else if (isAssignable(T, Types::TwoWord))
    return 2;

  // TODO: assert from constexpr function is not possible due to the bug in MinGW
  //assert(false); // should cover all types
  return 0;
}

constexpr Type Types::toVerificationType(const Type &From) noexcept {
  assert(From != Types::Void);

  if (From == Types::Char || From == Types::Short ||
      From == Types::Byte || From == Types::Boolean)
    return Types::Int;

  return From;
}

}

#endif //ICP_TYPE_H
