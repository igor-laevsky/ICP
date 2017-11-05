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
// Internally two word types are encoded as pairs of (Actual type, Types::Top)
// This is strictly internal format and all input data is expected to be in a
// regular form (i.e two-word types are stored as a single element)
class StackFrame final {
public:
  // Used to indicate parsing error in parseFieldDescriptor and
  // parseMethodDescriptor.
  class ParsingError: public std::runtime_error {
    using std::runtime_error::runtime_error;
  };

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

  // Locals accessors
  const auto &locals() const { return Locals; }
  Type getLocal(std::size_t Idx) const {
    assert(Idx < locals().size());
    return locals()[Idx];
  }
  void setLocal(std::size_t Idx, Type T);

  // Returns true if locals have uninitialized this flag
  bool flagThisUninit() const { return Flags; }

  // It's possible to pop list of types if each corresponding stack slot is
  // assignable to the given type.
  // Function has no effect if not all types can be popped.
  // First type in the list is popped first. No special encoding of two-word
  // types is expected (i.e they are stored as a single element).
  // \returns true if all types were popped, false otherwise.
  bool popMatchingList(const std::vector<Type> &Types);

  // Push types onto the frame stack.
  void pushList(const std::vector<Type> &Types);

  // Pops 'ToPop' types then pushes 'ToPush' type.
  // This is primitive for modeling instruction behaviour which takes some
  // operands from stack and pushes the result back.
  // If some types are incompatible whole function is a no-op.
  // \returns true if type transition was successfull, false otherwise.
  bool doTypeTransition(
      const std::vector<Type> &ToPop, Type ToPush);

  // Parses field descriptor. Note that it returns pure non-verifier type, i.e
  // short, byte, char and boolean are represented as themselves, not as integers.
  // For the format info see:
  // https://docs.oracle.com/javase/specs/jvms/se8/html/jvms-4.html#jvms-4.3.2
  // \param LastPos If not null will contain index of the first not parsed
  // character.
  // \returns Parsed type.
  // \throws ParsingError In case of any errors.
  static Type parseFieldDescriptor(
      const std::string &Desc, std::size_t *LastPos = nullptr);

  // Parses method descriptor. Note that it returns pure non-verifier type, i.e
  // short, byte, char and boolean are represented as themselves, not as integers.
  // For the format info see:
  // https://docs.oracle.com/javase/specs/jvms/se8/html/jvms-4.html#jvms-4.3.3
  // \returns Pair where first element is the return type and
  // second element is vector of the argument types. For void functions return
  // type is Types::Top.
  // \throws ParsingError In case of any errors.
  static std::pair<Type, std::vector<Type>>
  parseMethodDescriptor(const std::string &Desc);

private:
  // Computes uninitializedThis flag
  void computeFlags();

  // Expands each TwoWord type into the two consequential slots:
  //   (Actual type, Types::Top)
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
  bool Flags; // true if any of the locals is uninitializedThis
};

}

#endif //ICP_STACKFRAME_H
