//
// Representation of the verification type system.
//

#ifndef ICP_TYPE_H
#define ICP_TYPE_H

#include <optional>

namespace Verifier {

// Class representing verifier data type.
// User is supposed to use static variables defined below to refer to the
// specific types. Direct construction is forbidden.
// Types with parameters are constructed via static functions. Function without
// parameter returns type which exactly matches same type with any parameter.
// I.e UninitializedOffset() will match UninitializedOffset(10).
// It represents uninitializedOffset(_) from the JVM specification.
// This is intended to be a small immutable value-like class.
// TODO: Add parameters for the class and array types
class Type {
public:
  // This is a null object. It doesn't reflect any real world type and should
  // be used to indicate that type is unknown.
  static const Type Void;

  static const Type Top;

  static const Type OneWord;
  static const Type TwoWord;

  static const Type Int;
  static const Type Byte;
  static const Type Char;
  static const Type Short;
  static const Type Boolean;

  static const Type Float;
  static const Type Long;
  static const Type Double;

  static const Type Reference;
  static const Type Uninitialized;
  static const Type UninitializedThis;

  static Type UninitializedOffset();
  static Type UninitializedOffset(uint32_t Offset);

  static const Type Class;
  static const Type Array;
  static const Type Null;

public:
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

  //  Returns 1 for OneWord types and 2 for TwoWord types.
  static std::size_t sizeOf(Type T);

  // This function mirrors same function from the JVm specification.
  // Essentially it represents subtyping relation on the verifier types.
  // See jvms 4.10.1.2 for clear visualization of the type system.
  static bool isAssignable(Type From, Type To);

  // Convert type to verification type.
  // This means converting  byte, char, short, and boolean into integer type.
  static Type toVerificationType(Type From);

private:
  // Once array, class and null type are implemented this will be replaced
  // with std::variant.
  using DataType = uint32_t;

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

  std::optional<DataType> Data;

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
