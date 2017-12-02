///
/// JavaField class implementation
///

#include "JavaField.h"

using namespace JavaTypes;

JavaField::JavaField(
    const JavaTypes::ConstantPoolRecords::Utf8 &Descr,
    const JavaTypes::ConstantPoolRecords::Utf8 &Name,
    JavaTypes::JavaField::AccessFlags Flags):
  Name(Name),
  Descr(Descr),
  Flags(Flags)
{
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
  return Types::sizeOf(T) * sizeof(uint32_t);
}
