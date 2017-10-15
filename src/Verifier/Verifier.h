//
// This methods perform verification of the java classes according with the
// JVM specification.
//

#ifndef ICP_VERIFIER_H
#define ICP_VERIFIER_H

#include <stdexcept>
#include "JavaTypes/JavaTypesFwd.h"

namespace Verifier {

class VerificationError: public std::runtime_error {
  using std::runtime_error::runtime_error;
};

// Perform class verification.
// \throws VerificationError In case of any verification errors.
void verify(const JavaTypes::JavaClass &Class);

}


#endif //ICP_VERIFIER_H
