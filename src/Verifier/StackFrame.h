//
//
//

#ifndef ICP_STACKFRAME_H
#define ICP_STACKFRAME_H

#include <vector>
#include <stack>
#include <algorithm>

#include "Type.h"

namespace Verifier {

// This class more or less directly reflects the term
// "frame(Locals, OperandStack, Flags)" from the JVM specification.
class StackFrame {
public:

  StackFrame(std::vector<Type> Locals, std::vector<Type> Stack):
      Locals(std::move(Locals)),
      Stack(std::move(Stack)) {
    computeFlags();
    expandTypes();
  }

  auto numLocals() const { return Locals.size(); }

  auto numStack() const { return Stack.size(); }

  const auto &locals() const { return Locals; }

private:
  // Computes uninitializedThis flag
  void computeFlags();

  // Expands TwoWord types into the two consequential slots.
  void expandTypes();

  // There is no need for user to access the stack directly, hence it's a private
  // function.
  const auto &stack() const { return Stack; }
  auto &stack() { return Stack; }

  // Private non const accessor for locals
  auto &locals() { return Locals; }

private:
  std::vector<Type> Locals;
  std::vector<Type> Stack;
  bool flagThisUninit; // true if any of the locals is uninitializedThis
};

}


#endif //ICP_STACKFRAME_H
