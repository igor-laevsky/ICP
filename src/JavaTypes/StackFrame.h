//
//
//

#ifndef ICP_STACKFRAME_H
#define ICP_STACKFRAME_H

#include <vector>
#include <stack>
#include <algorithm>
#include <cassert>

#include "JavaTypes/Type.h"

namespace JavaTypes {

// This class more or less directly reflects the term
// "frame(Locals, OperandStack, Flags)" from the JVM specification.
// Internally two word types are encoded as pairs of (Actual type, Types::Top)
// This is strictly internal format and all input data is expected to be in a
// regular form (i.e two-word types are stored as a single element)
class StackFrame final {
public:
  StackFrame(
      const std::vector<JavaTypes::Type> &Locals,
      const std::vector<JavaTypes::Type> &Stack)
  {
    this->Locals = expandTypes(Locals);
    this->Stack = expandTypes(Stack);
    computeFlags();
    assert(verifyTypeEncoding());
  }

  StackFrame(const StackFrame &) = default;
  StackFrame &operator=(const StackFrame &)  = default;

  StackFrame(StackFrame &&) = default;
  StackFrame &operator=(StackFrame &&)  = default;

  auto numLocals() const { return Locals.size(); }

  auto numStack() const { return Stack.size(); }

  // Locals accessors
  const auto &locals() const { return this->Locals; }
  JavaTypes::Type getLocal(std::size_t Idx) const {
    assert(Idx < locals().size());
    return locals()[Idx];
  }
  void setLocal(std::size_t Idx, JavaTypes::Type T);

  // Returns true if locals have uninitialized this flag
  bool flagThisUninit() const { return Flags; }

  // It's possible to pop list of types if each corresponding stack slot is
  // assignable to the given type.
  // Function has no effect if not all types can be popped.
  // First type in the list is popped first. No special encoding of two-word
  // types is expected (i.e they are stored as a single element).
  // \returns true if all types were popped, false otherwise.
  bool popMatchingList(const std::vector<JavaTypes::Type> &Types);

  // Push types onto the frame stack.
  void pushList(const std::vector<JavaTypes::Type> &Types);

  // Pops 'ToPop' types then pushes 'ToPush' type.
  // This is primitive for modeling instruction behaviour which takes some
  // operands from stack and pushes the result back.
  // If some types are incompatible whole function is a no-op.
  // \returns true if type transition was successfull, false otherwise.
  bool doTypeTransition(
      const std::vector<JavaTypes::Type> &ToPop, JavaTypes::Type ToPush);

private:
  // Computes uninitializedThis flag
  void computeFlags();

  // Expands each TwoWord type into the two consequential slots:
  //   (Actual type, Types::Top)
  // \returns New vector with expanded types.
  static std::vector<JavaTypes::Type> expandTypes(
      const std::vector<JavaTypes::Type> &Src);

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

  void push(const JavaTypes::Type &T) {
    stack().push_back(T);
  }

  // Private non const accessor for locals.
  auto &locals() { return Locals; }

  // Checks that all two word types are correctly encoded.
  bool verifyTypeEncoding();

private:
  std::vector<JavaTypes::Type> Locals;
  std::vector<JavaTypes::Type> Stack;
  bool Flags; // true if any of the locals is uninitializedThis
};

}

#endif //ICP_STACKFRAME_H
