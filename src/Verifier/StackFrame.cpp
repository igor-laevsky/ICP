//
//
//

#include "StackFrame.h"

using namespace Verifier;

void StackFrame::computeFlags() {
  // Look for uninitializedThis in locals
  Flags =
      std::find(
          Locals.begin(), Locals.end(),
          Type::UninitializedThis) != Locals.end();
}

std::vector<Type> StackFrame::expandTypes(const std::vector<Type> &Src) {
  std::vector<Type> Ret;

  Ret.reserve(Src.size());
  for (const auto &T: Src) {
    Ret.push_back(T);
    if (Type::sizeOf(T) == 2)
      Ret.push_back(Type::Top);
  }

  return Ret;
}

bool StackFrame::popMatchingList(const std::vector<Type> &Types) {
  assert(verifyTypeEncoding());

  if (stack().empty())
    return Types.empty();

  auto IsTwoWordType = [&] (auto Idx) {
    return Idx >= 1 &&
        stack()[Idx] == Type::Top &&
        Type::sizeOf(stack()[Idx - 1]) == 2;
  };

  // First check that we can pop all the types
  int TopIdx = static_cast<int>(stack().size()) - 1;
  for (const auto &T: Types) {
    // Not enough types to pop
    if (TopIdx < 0)
      return false;

    // This is two word type
    if (IsTwoWordType(TopIdx)) {
      --TopIdx;
      assert(TopIdx >= 0);
    }

    if (!Type::isAssignable(stack()[TopIdx], T))
      return false;
    --TopIdx;
  }

  // Actually pop all the types. No additional checks are needed.
  for (std::size_t i = 0; i < Types.size(); ++i) {
    // Additional pop for two word types
    if (IsTwoWordType(stack().size() - 1))
      pop();
    pop();
  }

  assert(verifyTypeEncoding());
  return true;
}

bool StackFrame::verifyTypeEncoding() {
  auto CheckVector = [&] (const auto &Vec) {
    for (std::size_t i = 0; i < Vec.size(); ++i) {
      if (Type::sizeOf(Vec[i]) == 2)
        return i <= Vec.size() - 2 && Vec[i + 1] == Type::Top;
    }

    return true;
  };

  return CheckVector(locals()) && CheckVector(stack());
}

void StackFrame::pushList(const std::vector<Type> &Types) {
  const auto ExpandedTypes = expandTypes(Types);

  for (const auto &T: ExpandedTypes)
    push(T);

  assert(verifyTypeEncoding());
}

bool StackFrame::doTypeTransition(
    const std::vector<Type> &ToPop, Type ToPush) {

  // Pop operands
  if (!popMatchingList(ToPop))
    return false;

  // Push the result
  push(ToPush);
  if (Type::sizeOf(ToPush) == 2)
    push(Type::Top);

  assert(verifyTypeEncoding());
  return true;
}
