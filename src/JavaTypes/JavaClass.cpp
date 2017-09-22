//
// Created by Igor on 15.09.2017.
//

#include "JavaClass.h"

#include <ostream>

using namespace JavaTypes;

bool JavaClass::verify(std::string &ErrorMessage) const {
  // User is supposed to provide valid constant pool, hence this is assert
  // rather than a normal check.
  assert(CP->verify(ErrorMessage));

  if (getAccessFlags() != (AccessFlags::ACC_PUBLIC | AccessFlags::ACC_SUPER)) {
    ErrorMessage = "Wrong class access flags";
    return false;
  }

  return true;
}

void JavaClass::print(std::ostream &Out) const {
  Out << "Class name: " << getClassName() << "\n";
  if (getSuperClassName())
    Out << "Super class: " << *getSuperClassName() << "\n";

  Out << "Constant pool: \n";
  getConstantPool().print(Out);
}
