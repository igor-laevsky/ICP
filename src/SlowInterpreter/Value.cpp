///
/// Implementation for the dynamic java types.
/// The mapping between static types and dynamic types is clumsy and inefficient.
/// This can be simplified with a bit of preprocessor magic.
///

#include "Value.h"

using namespace SlowInterpreter;
using namespace JavaTypes;

// Static types ans runtime types are tightly coupled. Good or bad, but we need
// them to have the same size.
static_assert(Types::sizeInBytes(Types::Int) == sizeof(JavaInt));
static_assert(Types::sizeInBytes(Types::Byte) == sizeof(JavaByte));
static_assert(Types::sizeInBytes(Types::Char) == sizeof(JavaChar));
static_assert(Types::sizeInBytes(Types::Short) == sizeof(JavaShort));
static_assert(Types::sizeInBytes(Types::Boolean) == sizeof(JavaBool));
static_assert(Types::sizeInBytes(Types::Float) == sizeof(JavaFloat));
static_assert(Types::sizeInBytes(Types::Long) == sizeof(JavaLong));
static_assert(Types::sizeInBytes(Types::Double) == sizeof(JavaDouble));

Value Value::fromMemory(const Type &T, const uint8_t *Mem) {

  // Clumsy
  if (T == Types::Int)
    return Value::create<JavaInt>(*(reinterpret_cast<const JavaInt *>(Mem)));
  if (T == Types::Byte)
    return Value::create<JavaByte>(*(reinterpret_cast<const JavaByte *>(Mem)));
  if (T == Types::Char)
    return Value::create<JavaChar>(*(reinterpret_cast<const JavaChar *>(Mem)));
  if (T == Types::Short)
    return Value::create<JavaShort>(*(reinterpret_cast<const JavaShort *>(Mem)));
  if (T == Types::Boolean)
    return Value::create<JavaBool>(*(reinterpret_cast<const JavaBool *>(Mem)));

  if (T == Types::Float)
    return Value::create<JavaFloat>(*(reinterpret_cast<const JavaFloat *>(Mem)));
  if (T == Types::Long)
    return Value::create<JavaLong>(*(reinterpret_cast<const JavaLong *>(Mem)));
  if (T == Types::Double)
    return Value::create<JavaDouble>(*(reinterpret_cast<const JavaDouble *>(Mem)));

  if (Types::isAssignable(T, Types::Reference))
    return Value::create<JavaRef>(*(reinterpret_cast<const JavaRef *>(Mem)));

  assert(false); // Unrecognized type
  return {};
}

void Value::toMemory(
    uint8_t *Mem, const Value &V, const JavaTypes::Type &T) {

  // Clumsy
  if (T == Types::Int)
    *(reinterpret_cast<JavaInt*>(Mem)) = V.getAs<JavaInt>();
  else if (T == Types::Byte)
    *(reinterpret_cast<JavaByte*>(Mem)) = V.getAs<JavaByte>();
  else if (T == Types::Char)
    *(reinterpret_cast<JavaChar*>(Mem)) = V.getAs<JavaChar>();
  else if (T == Types::Short)
    *(reinterpret_cast<JavaShort*>(Mem)) = V.getAs<JavaShort>();
  else if (T == Types::Boolean)
    *(reinterpret_cast<JavaBool*>(Mem)) = V.getAs<JavaBool>();

  else if (T == Types::Float)
    *(reinterpret_cast<JavaFloat*>(Mem)) = V.getAs<JavaFloat>();
  else if (T == Types::Long)
    *(reinterpret_cast<JavaLong*>(Mem)) = V.getAs<JavaLong>();
  else if (T == Types::Double)
    *(reinterpret_cast<JavaDouble*>(Mem)) = V.getAs<JavaDouble>();

  else if (Types::isAssignable(T, Types::Reference))
    *(reinterpret_cast<JavaRef*>(Mem)) = V.getAs<JavaRef>();

  else
    assert(false); // Unrecognized type
}
