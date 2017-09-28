//
// Representation of the verification type
//

#ifndef ICP_TYPE_H
#define ICP_TYPE_H

#include <optional>

namespace Verifier {

// Class representing verifier data type.
// User is supposed to use static variables defined below to refer to the
// specific types. Direct construction is forbidden.
// Types with parameters are constructed via static functions. Function without
// parameter means type which exactly matches same type with any parameter.
// I.e UninitializedOffset() will match UninitializedOffset(10).
// It represents uninitializedOffset(_) from the JVM specification.
// This is intended to be a small immutable value-like class.
// TODO: Add class, array and null types.
class Type {
public:
  static Type Top;

  static Type OneWord;
  static Type TwoWord;

  static Type Int;
  static Type Float;
  static Type Long;
  static Type Double;

  static Type Reference;
  static Type Uninitialized;
  static Type UninitializedThis;

  static Type UninitializedOffset();
  static Type UninitializedOffset(uint32_t Offset);

public:
  // This objects are often copied.
  Type(const Type &) = default;
  // However assignment is not allowed since they are immutable.
  Type &operator=(const Type &) = delete;

  bool operator==(const Type &Rhs) const {
    if (Tag != Rhs.Tag)
      return false;

    // This is to support wildcard matching of parametrized types
    if (!Data || !Rhs.Data)
      return true;

    return Data == Rhs.Data;
  }

  bool operator!=(const Type &Rhs) const {
    return !(*this == Rhs);
  }

  // This function mirrors same function from the JVm specification.
  // Essentially it represents subtyping relation on the verifier types.
  // See jvms 4.10.1.2 for clear visualization of the type system.
  static bool isAssignable(Type From, Type To);

private:
  // Once array, class and null type are implemented this will be replaced
  // with std::variant.
  using DataType = uint32_t;

  enum class TagType {
    TOP = 0,
    ONE_WORD,
    TWO_WORD,
    INT,
    FLOAT,
    LONG,
    DOUBLE,
    REFERENCE,
    UNINITIALIZED,
    UNINITIALIZED_THIS,
    UNINITIALIZED_OFFSET
  };
  const TagType Tag;

  const std::optional<DataType> Data;

private:
  explicit Type(TagType Tag):
      Tag(Tag),
      Data() {
    ;
  }

  Type(TagType Tag, DataType Data):
      Tag(Tag),
      Data(Data) {
    ;
  }
};

}

#endif //ICP_TYPE_H
