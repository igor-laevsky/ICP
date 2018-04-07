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
  StackFrame &operator=(const StackFrame &) = default;
  StackFrame(StackFrame &&) = default;
  StackFrame &operator=(StackFrame &&)  = default;

  void resizeLocals(std::size_t NewSize) {
    if (NewSize <= locals().size())
      return;

    locals().insert(locals().end(), NewSize - locals().size(), Types::Top);
  }

  bool operator==(const StackFrame &Other) const {
    return stack() == Other.stack() && locals() == Other.locals();
  }
  bool operator!=(const StackFrame &Other) const { return !(*this == Other); }

  // Returns true if locals have uninitialized this flag
  bool flagThisUninit() const { return Flags; }


  // Methods to work with locals
  //

  auto numLocals() const { return Locals.size(); }

  JavaTypes::Type getLocal(std::size_t Idx) const {
    assert(Idx < locals().size());
    return locals()[Idx];
  }

  void setLocal(std::size_t Idx, JavaTypes::Type T);

  void substituteLocals(const Type &From, const Type &To);

  // Methods to work with stack
  //

  std::size_t numStack() const { return Stack.size(); }
  bool emptyStack() const { return Stack.empty(); }

  Type topStack() const;

  void substituteStack(const Type &From, const Type &To);

  bool stackContains(const Type &T) const;

  // Complex methods handling various type transitions in the verifier
  //

  // Return none if unable to pop the type.
  // Otherwise return type which was actually popped.
  std::optional<Type> popMatchingType(const JavaTypes::Type &T);

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

  // Checks if we can assign this stack from into the 'NextFrame'.
  static bool isAssignable(const StackFrame &From, const StackFrame &To);

  // Checks if this frame is assignable into the 'NextFrame' and if this is true
  // transforms this frame into 'NextFrame'. Guarantees that no changes were made
  // if frames were not assignable.
  // \return true on success, false otherwise.
  bool transformInto(const StackFrame &NextFrame);

private:
  // Computes uninitializedThis flag
  void computeFlags();

  // Expands each TwoWord type into the two consequential slots:
  //   (Actual type, Types::Top)
  // \returns New vector with expanded types.
  static std::vector<JavaTypes::Type> expandTypes(
      const std::vector<JavaTypes::Type> &Src);

  // Private accessors for the locals and stack.
  // This class internally maintains "expanded type" encoding so the user should
  // not be exposed to this plain containers.
  const std::vector<Type> &locals() const { return Locals; }
  std::vector<Type> &locals() { return Locals; }
  const std::vector<Type> &stack() const { return Stack; }
  std::vector<Type> &stack() { return Stack; }

  // Unconditionally pop from the stack. Preserves type encoding (i.e pops two
  // elements for the two word type).
  void pop();

  // Unconditionally push to the stack. Preserves type encoding (i.e pops two
  // elements for the two word type).
  void push(const JavaTypes::Type &T);

  // Checks that all two word types are correctly encoded.
  bool verifyTypeEncoding();

  bool isTwoWordType(std::size_t Idx) const;

private:
  std::vector<Type> Locals;
  std::vector<Type> Stack;
  bool Flags; // true if any of the locals is uninitializedThis
};

}

#endif //ICP_STACKFRAME_H
