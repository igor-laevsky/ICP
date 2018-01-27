///
/// Stack map table implementation.
///

#include "StackMapTable.h"

using namespace JavaTypes;

void StackMapTableBuilder::addAppend(
    Bytecode::BciType Idx, std::vector<Type> &&Locals) {

  assert(checkBciMonotonic(Idx));
  actions().emplace_back(Idx,
      Action::APPEND, std::move(Locals), std::vector<Type>());
}

void StackMapTableBuilder::addSame(Bytecode::BciType Idx) {
  assert(checkBciMonotonic(Idx));
  actions().emplace_back(Idx,
      Action::SAME, std::vector<Type>(), std::vector<Type>());
}

void StackMapTableBuilder::addFull(
    Bytecode::BciType Idx,
    std::vector<Type> &&Locals, std::vector<Type> &&Stack) {

  assert(checkBciMonotonic(Idx));
  actions().emplace_back(Idx,
      Action::FULL, std::move(Locals), std::move(Stack));
}

bool StackMapTableBuilder::checkBciMonotonic(Bytecode::BciType Idx) {
  if (actions().empty())
    return true;

  auto last_index = actions().back().Bci;
  return Idx > last_index;
}

void StackMapTableBuilder::transformFrame(RawFrame &Frame, const Action &Act) {
  switch (Act.FrameType) {
  case Action::SAME:
    // Noting to be done.
    return;
  case Action::FULL:
    Frame.first = Act.Locals;
    Frame.second = Act.Stack;
    return;
  case Action::APPEND:
    Frame.first.insert(Frame.first.end(), Act.Locals.begin(), Act.Locals.end());
    Frame.second.clear();
    return;
  }

  assert(false); // all actions should be implemented
}

StackMapTable StackMapTableBuilder::createTable(
    const std::vector<Type> &InitialLocals) const {

  StackMapTable::Container specific_frames;
  specific_frames.reserve(actions().size() + 1);

  // Initial stack frame.
  specific_frames.emplace_back(0, StackFrame(InitialLocals, std::vector<Type>()));

  // Fill in all other stack frames.
  RawFrame cur_frame_raw{InitialLocals, {}};

  for (const auto &act: actions()) {
    transformFrame(cur_frame_raw, act);
    specific_frames.emplace_back(
        act.Bci, StackFrame(cur_frame_raw.first, cur_frame_raw.second));
  }

  return {std::move(specific_frames)};
}
