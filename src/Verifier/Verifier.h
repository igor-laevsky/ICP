//
// This methods perform verification of the java classes according with the
// JVM specification.
//

#ifndef ICP_VERIFIER_H
#define ICP_VERIFIER_H

#include "JavaTypes/JavaTypesFwd.h"

#include <stdexcept>

namespace Verifier {

class VerificationError: public std::runtime_error {
  using std::runtime_error::runtime_error;
};


// Verifies single method.
// \throws VerificationError In case of any verification errors.
void verifyMethod(const JavaTypes::JavaMethod &Method);

// Perform class verification.
// \throws VerificationError In case of any verification errors.
void verify(const JavaTypes::JavaClass &Class);

}


#endif //ICP_VERIFIER_H
