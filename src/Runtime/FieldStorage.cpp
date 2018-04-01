///
/// Implementation for the FieldStorageClass
///

#include "FieldStorage.h"

#include "JavaTypes/JavaClass.h"
#include "JavaTypes/JavaField.h"

using namespace Runtime;
using namespace JavaTypes;

bool FieldStorage::shouldManage(const JavaTypes::JavaField &F) const {
  switch (Kind) {
  case STATIC: return F.isStatic();
  case INSTANCE: return !F.isStatic();
  }

  assert(false); // unknown kind
  return false;
}

FieldStorage::FieldStorage(const JavaTypes::JavaClass &Class, bool is_static):
  Class(Class),
  Kind(is_static ? STATIC : INSTANCE) {
  // Compute total size of the object fields.
  // Don't care about alignment for now.
  std::size_t ObjectSize = 0;
  for (const auto &Field: Class.fields()) {
    if (shouldManage(Field))
      ObjectSize += Field.getSize();
  }

  Fields.resize(ObjectSize);
}

std::pair<const JavaField*, std::size_t>
FieldStorage::findFieldAndOffset(const Utf8String &Name) const {
  std::size_t CurrentOffset = 0;
  const JavaField *Found = nullptr;

  for (const auto &Field: Class.fields()) {
    if (Field.getName() == Name) {
      assert(shouldManage(Field));
      Found = &Field;
      break;
    }

    if (shouldManage(Field))
      CurrentOffset += Field.getSize();
  }
  if (!Found)
    throw UnrecognizedField();

  assert(CurrentOffset + Found->getSize() <= Fields.size());
  return {Found, CurrentOffset};
}

Value FieldStorage::getField(const Utf8String &Name) const {
  std::size_t Offset = 0;
  const JavaField *Field = nullptr;

  std::tie(Field, Offset) = findFieldAndOffset(Name);
  return Value::fromMemory(Field->getType(), Fields.data() + Offset);
}

void FieldStorage::setField(const Utf8String &Name, const Value &V) {
  std::size_t Offset = 0;
  const JavaField *Field = nullptr;

  std::tie(Field, Offset) = findFieldAndOffset(Name);
  Value::toMemory(Fields.data() + Offset, V, Field->getType());
}
