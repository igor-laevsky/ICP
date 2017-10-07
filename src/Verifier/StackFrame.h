//
//
//

#ifndef ICP_STACKFRAME_H
#define ICP_STACKFRAME_H

#include <vector>
#include <stack>
#include <algorithm>
#include <cassert>

#include "Type.h"

namespace Verifier {

// This class more or less directly reflects the term
// "frame(Locals, OperandStack, Flags)" from the JVM specification.
// Internally two word types are encoded as pairs of (Actual type, Type::Top)
// This is strictly internal format and all input data is expected to be in a
// regular form (i.e two-word types are stored as a single element)
class StackFrame {
public:
  StackFrame(const std::vector<Type> &Locals, const std::vector<Type> &Stack)
  {
    this->Locals = expandTypes(Locals);
    this->Stack = expandTypes(Stack);
    computeFlags();
    assert(verifyTypeEncoding());
  }

  auto numLocals() const { return Locals.size(); }

  auto numStack() const { return Stack.size(); }

  const auto &locals() const { return Locals; }

  // It's possible to pop list of types if each corresponding stack slot is
  // assignable to the given type.
  // Function has no effect if not all types can be popped.
  // First type in the list popped first. No special encoding of two-word types
  // is expected (i.e they are stored as a single element).
  // \returns true if all types were poped, false otherwise.
  bool popMatchingList(const std::vector<Type> &Types);

private:
  // Computes uninitializedThis flag
  void computeFlags();

  // Expands each TwoWord type into the two consequential slots:
  //   (Actual type, Type::Top)
  // \returns New vector with expanded types.
  static std::vector<Type> expandTypes(const std::vector<Type> &Src);

  // There is no need for user to access the stack directly, hence it's a
  // private functions.
  const auto &stack() const { return Stack; }
  auto &stack() { return Stack; }

  // This is low level method which doesn't check if this operation is
  // logically valid.
  void pop() {
    assert(!stack().empty());
    stack().pop_back();
  }

  void push(const Type &T) {
    stack().push_back(T);
  }

  // Private non const accessor for locals.
  auto &locals() { return Locals; }

  // Checks that all two word types are correctly encoded.
  bool verifyTypeEncoding();

private:
  std::vector<Type> Locals;
  std::vector<Type> Stack;
  bool flagThisUninit; // true if any of the locals is uninitializedThis
};

}

#endif //ICP_STACKFRAME_H
