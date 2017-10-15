//
// Verifier implementation
//

#include "Verifier.h"

#include "StackFrame.h"
#include "Bytecode/InstructionVisitor.h"

using namespace JavaTypes;
using namespace Bytecode;
using namespace Verifier;

namespace {

// This visitor is intended to be called on all instructions of the method
// in order of their appearance. Caller is responsible to supply
// correct stack frames when necessary.
class MethodVerifier: public InstructionVisitor {
public:
  MethodVerifier(
    const ConstantPool &CP,
    const JavaMethod &Method,
    StackFrame InitialStackFrame):
      CP(CP),
      Method(Method),
      CurrentFrame(std::move(InitialStackFrame)) {
    ;
  }

private:
  const ConstantPool &CP;
  const JavaMethod &Method;
  StackFrame CurrentFrame;
};

}

void Verifier::verify(const JavaTypes::JavaClass &Class) {
  (void)Class;
}
