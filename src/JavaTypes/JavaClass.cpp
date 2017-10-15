//
// Implementation of the JavaClass
//

#include "JavaClass.h"

#include <ostream>

using namespace JavaTypes;

bool JavaClass::verify(std::string &ErrorMessage) const {
  // User is supposed to provide valid constant pool, hence this is assert
  // rather than a normal check.
  assert(CP != nullptr && CP->verify(ErrorMessage));

  if (getAccessFlags() != (AccessFlags::ACC_PUBLIC | AccessFlags::ACC_SUPER)) {
    ErrorMessage = "Unsupported class access flags";
    return false;
  }

  // TODO: Verify that all methods not null and have correct owner

  return true;
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
