//
// JavaMethod class implementation
//

#include "JavaMethod.h"

#include <ostream>

using namespace JavaTypes;

bool JavaMethod::verify(std::string &ErrorMessage) const {
  if (Flags != AccessFlags::ACC_PUBLIC &&
      Flags != (AccessFlags::ACC_PUBLIC | AccessFlags::ACC_STATIC)) {
    ErrorMessage = "Unsupported access flags";
    return false;
  }

  return true;
}

void JavaMethod::print(std::ostream &Out) const {
  Out << getName() << " " << getDescriptor() << "\n";
  Out << "MaxStack: " << getMaxStack() << " MaxLocals: " << getMaxLocals() << "\n";
  Out << "Code size: " << getCode().size() << "\n";
}
