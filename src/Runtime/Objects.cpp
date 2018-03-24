///
/// Implementation for the runtime java object representation
///

#include "Objects.h"

#include "JavaTypes/JavaClass.h"

using namespace Runtime;
using namespace JavaTypes;

ClassObject::ClassObject(const JavaTypes::JavaClass &Class):
  Class(Class) {

  // Compute total size of the object static fields.
  // Don't care about alignment for now.
  std::size_t ObjectSize = 0;
  for (const auto &Field: Class.fields()) {
    if (Field.isStatic())
      ObjectSize += Field.getSize();
  }

  Fields.resize(ObjectSize);
}

std::pair<const JavaField*, std::size_t>
ClassObject::findFieldAndOffset(const Utf8String &Name) const {
  std::size_t CurrentOffset = 0;
  const JavaField *Found = nullptr;

  for (const auto &Field: getClass().fields()) {
    if (Field.getName() == Name) {
      assert(Field.isStatic());
      Found = &Field;
      break;
    }

    if (Field.isStatic())
      CurrentOffset += Field.getSize();
  }
  if (!Found)
    throw UnrecognizedField();

  assert(CurrentOffset + Found->getSize() <= fields().size());
  return {Found, CurrentOffset};
}

Value ClassObject::getField(const Utf8String &Name) const {
  std::size_t Offset = 0;
  const JavaField *Field = nullptr;

  std::tie(Field, Offset) = findFieldAndOffset(Name);
  return Value::fromMemory(Field->getType(), fields().data() + Offset);
}

void ClassObject::setField(const Utf8String &Name, const Value &V) {
  std::size_t Offset = 0;
  const JavaField *Field = nullptr;

  std::tie(Field, Offset) = findFieldAndOffset(Name);
  Value::toMemory(fields().data() + Offset, V, Field->getType());
}

const JavaMethod *ClassObject::getMethod(const Utf8String &Name) const {
  return getClass().getMethod(Name);
}
