//
//
//

#include "StackFrame.h"

void Verifier::StackFrame::computeFlags() {
  // Look for uninitializedThis in locals
  flagThisUninit =
      std::find(
          Locals.begin(), Locals.end(),
          Type::UninitializedThis) != Locals.end();
}

void Verifier::StackFrame::expandTypes() {
  auto GenericExpand = [&] (const auto &Src) {
    std::decay_t<decltype(Src)> Ret;

    Ret.reserve(Src.size());
    for (const auto &T: Src) {
      Ret.push_back(T);
      if (Type::isAssignable(T, Type::TwoWord))
        Ret.push_back(Type::Top);
    }

    return Ret;
  };

  locals() = GenericExpand(locals());
  stack() = GenericExpand(stack());
}
