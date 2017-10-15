//
// Implementation of the JavaClass
//

#include "JavaClass.h"

#include <ostream>

using namespace JavaTypes;

JavaClass::JavaClass(JavaClass::ClassParameters &&Params):
  ClassName(Params.ClassName),
  SuperClass(Params.SuperClass),
  Flags(Params.Flags),
  CP(std::move(Params.CP)),
  Methods(std::move(Params.Methods))
{
  assert(Params.ClassName != nullptr);

  // It is responsibility of the user to provide valid constant pool.
  assert(CP != nullptr && CP->verify());

  // Set up correct owner for the class methods
  for (auto &Method: getMethods()) {
    assert(Method != nullptr);
    Method->setOwner(*this);
  }

  // Other access flags are not yet supported
  assert(
      getAccessFlags() == (AccessFlags::ACC_PUBLIC | AccessFlags::ACC_SUPER));
}

void JavaClass::print(std::ostream &Out) const {
  Out << "Class name: " << getClassName() << "\n";
  if (hasSuper())
    Out << "Super class: " << getSuperClassName() << "\n";

  Out << "Constant pool: \n";
  getConstantPool().print(Out);

  Out << "Methods:\n";
  for (const auto &Method: getMethods())
    Method->print(Out);
}
