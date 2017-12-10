///
/// Implementation for the dynamic java types.
/// The mapping between static types and dynamic types is clumsy and inefficient.
/// This can be simplified with a bit of preprocessor magic.
///

#include "Value.h"

#include <cstring>
#include <sstream>

using namespace Runtime;
using namespace JavaTypes;

// Static types and runtime types are tightly coupled. Good or bad, but we need
// them to have the same size.
static_assert(Types::sizeInBytes(Types::Int) == sizeof(JavaInt));
static_assert(Types::sizeInBytes(Types::Byte) == sizeof(JavaByte));
static_assert(Types::sizeInBytes(Types::Char) == sizeof(JavaChar));
static_assert(Types::sizeInBytes(Types::Short) == sizeof(JavaShort));
static_assert(Types::sizeInBytes(Types::Boolean) == sizeof(JavaBool));
static_assert(Types::sizeInBytes(Types::Float) == sizeof(JavaFloat));
static_assert(Types::sizeInBytes(Types::Long) == sizeof(JavaLong));
static_assert(Types::sizeInBytes(Types::Double) == sizeof(JavaDouble));
static_assert(Types::sizeInBytes(Types::Reference) == sizeof(JavaRef));

// Helper for the 'fromMemory' function
template<class RuntimeT>
static Value CreateAndCopy(const Type &T, const uint8_t *Mem) {
  RuntimeT Ret{};
  assert(sizeof(Ret) == Types::sizeInBytes(T));
  std::memcpy(&Ret, Mem, sizeof(Ret));
  return Value::create<RuntimeT>(Ret);
}

Value Value::fromMemory(const Type &T, const uint8_t *Mem) {

  if (T == Types::Int)
    return CreateAndCopy<JavaInt>(T, Mem);
  if (T == Types::Byte)
    return CreateAndCopy<JavaByte>(T, Mem);
  if (T == Types::Char)
    return CreateAndCopy<JavaChar>(T, Mem);
  if (T == Types::Short)
    return CreateAndCopy<JavaShort>(T, Mem);
  if (T == Types::Boolean)
    return CreateAndCopy<JavaBool>(T, Mem);

  if (T == Types::Float)
    return CreateAndCopy<JavaFloat>(T, Mem);
  if (T == Types::Long)
    return CreateAndCopy<JavaLong>(T, Mem);
  if (T == Types::Double)
    return CreateAndCopy<JavaDouble>(T, Mem);

  if (Types::isAssignable(T, Types::Reference))
    return CreateAndCopy<JavaRef>(T, Mem);

  assert(false); // Unrecognized type
  return {};
}

// Helper for the 'toMemory' function
template<class RuntimeT>
static void TypedCopy(uint8_t *Mem, const Value &V, const JavaTypes::Type &T) {
  auto TypedVal = V.getAs<RuntimeT>();
  assert(sizeof(TypedVal) == Types::sizeInBytes(T));
  std::memcpy(Mem, &TypedVal, sizeof(TypedVal));
}

void Value::toMemory(
    uint8_t *Mem, const Value &V, const JavaTypes::Type &T) {

  // This can be done simpler with variant visitor but we will sacrifice
  // type safety.

  if (T == Types::Int)
    TypedCopy<JavaInt>(Mem, V, T);
  else if (T == Types::Byte)
    TypedCopy<JavaByte>(Mem, V, T);
  else if (T == Types::Char)
    TypedCopy<JavaChar>(Mem, V, T);
  else if (T == Types::Short)
    TypedCopy<JavaShort>(Mem, V, T);
  else if (T == Types::Boolean)
    TypedCopy<JavaBool>(Mem, V, T);

  else if (T == Types::Float)
    TypedCopy<JavaFloat>(Mem, V, T);
  else if (T == Types::Long)
    TypedCopy<JavaLong>(Mem, V, T);
  else if (T == Types::Double)
    TypedCopy<JavaDouble>(Mem, V, T);

  else if (Types::isAssignable(T, Types::Reference))
    TypedCopy<JavaRef>(Mem, V, T);

  else
    assert(false); // Unrecognized type
}
