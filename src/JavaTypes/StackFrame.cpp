//
//
//

#include "StackFrame.h"

using namespace JavaTypes;

void StackFrame::computeFlags() {
  // Look for uninitializedThis in locals
  Flags =
      std::find(
          Locals.begin(), Locals.end(),
          JavaTypes::Types::UninitializedThis) != Locals.end();
}

std::vector<Type> StackFrame::expandTypes(const std::vector<Type> &Src) {
  std::vector<Type> Ret;

  Ret.reserve(Src.size());
  for (const auto &T: Src) {
    Ret.push_back(T);
    if (Types::sizeOf(T) == 2)
      Ret.push_back(Types::Top);
  }

  return Ret;
}

bool StackFrame::popMatchingList(const std::vector<Type> &Types) {
  if (stack().empty())
    return Types.empty();

  auto IsTwoWordType = [&] (auto Idx) {
    return Idx >= 1 &&
        stack()[Idx] == Types::Top &&
        Types::sizeOf(stack()[Idx - 1]) == 2;
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

    if (!Types::isAssignable(stack()[TopIdx], T))
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
      if (Types::sizeOf(Vec[i]) == 2)
        return i <= Vec.size() - 2 && Vec[i + 1] == Types::Top;
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
  if (Types::sizeOf(ToPush) == 2)
    push(Types::Top);

  assert(verifyTypeEncoding());
  return true;
}

void StackFrame::setLocal(std::size_t Idx, Type T) {
  assert(Idx < locals().size());
  locals()[Idx] = T;

  if (Types::sizeOf(T) == 2) {
    assert(Idx + 1 < locals().size());
    locals()[Idx + 1] = Types::Top;
  }

  computeFlags();
  assert(verifyTypeEncoding());
}

bool StackFrame::isAssignable(const StackFrame &From, const StackFrame &To) {
  // Early return on trivial case.
  if (From == To)
    return true;

  // Stack sized should match.
  if (From.numStack() != To.numStack())
    return false;

  // Stacks should be assignable.
  auto to_stack = To.stack().begin();
  for (const auto &from_stack: From.stack()) {
    if (!Types::isAssignable(from_stack, *to_stack))
      return false;
    ++to_stack;
  }

  // Locals should be assignable but can have different lengths.
  // Unexistent elements are considered as Top type according with the spec.
  auto to_locals = To.locals().begin();
  auto to_locals_end = To.locals().end();
  auto from_locals = From.locals().begin();
  auto from_locals_end = From.locals().end();

  while (to_locals != to_locals_end || from_locals != from_locals_end) {
    Type to_type = to_locals == to_locals_end ? Types::Top : *to_locals;
    Type from_type = from_locals == from_locals_end ? Types::Top : *from_locals;

    if (!Types::isAssignable(from_type, to_type))
      return false;

    if (to_locals != to_locals_end) ++to_locals;
    if (from_locals != from_locals_end) ++from_locals;
  }

  // All checks have passed.
  return true;
}

bool StackFrame::transformInto(const StackFrame &NextFrame) {
  // Early return on trivial case.
  if (*this == NextFrame)
    return true;

  if (!StackFrame::isAssignable(*this, NextFrame))
    return false;

  // Frames are assignable, do the transformation.
  locals() = NextFrame.locals();
  stack() = NextFrame.stack();
  computeFlags();
  assert(verifyTypeEncoding());

  return true;
}