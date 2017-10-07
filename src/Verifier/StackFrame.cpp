//
//
//

#include "StackFrame.h"

using namespace Verifier;

void StackFrame::computeFlags() {
  // Look for uninitializedThis in locals
  flagThisUninit =
      std::find(
          Locals.begin(), Locals.end(),
          Type::UninitializedThis) != Locals.end();
}

std::vector<Type> StackFrame::expandTypes(const std::vector<Type> &Src) {
  std::vector<Type> Ret;

  Ret.reserve(Src.size());
  for (const auto &T: Src) {
    Ret.push_back(T);
    if (Type::isAssignable(T, Type::TwoWord))
      Ret.push_back(Type::Top);
  }

  return Ret;
}

bool StackFrame::popMatchingList(const std::vector<Type> &Types) {
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

  return true;
}
