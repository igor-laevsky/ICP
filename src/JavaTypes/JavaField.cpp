///
/// JavaField class implementation
///

#include "JavaField.h"

using namespace JavaTypes;

JavaField::JavaField(
    const ConstantPoolRecords::Utf8 &Descr,
    const ConstantPoolRecords::Utf8 &Name,
    JavaField::AccessFlags Flags):
  Name(Name),
  Descr(Descr),
  Flags(Flags)
{
  assert(Flags != AccessFlags::ACC_NONE); // Flags should be specified

  T = Type::parseFieldDescriptor(Descr.getValue());
}

const std::string &JavaField::getName() const {
  return Name.getValue();
}

const std::string &JavaField::getDescriptor() const {
  return Descr.getValue();
}

JavaTypes::Type JavaField::getType() const {
  return T;
}

std::size_t JavaField::getSize() const {
  return Types::sizeInBytes(T);
}
