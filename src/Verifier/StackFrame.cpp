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
          Types::UninitializedThis) != Locals.end();
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

Type StackFrame::parseFieldDescriptor(
    const std::string &Desc, std::size_t *LastPos) {

  if (Desc.empty())
    throw ParsingError("Field descriptor is empty");

  if (LastPos != nullptr)
    *LastPos = 1;

  switch (Desc[0]) {
    case 'B': return Types::Byte;
    case 'C': return Types::Char;
    case 'D': return Types::Double;
    case 'F': return Types::Float;
    case 'I': return Types::Int;
    case 'J': return Types::Long;
    case 'S': return Types::Short;
    case 'Z': return Types::Boolean;

    case 'L': {
      auto End = Desc.find(';');
      if (End == std::string::npos)
        throw ParsingError("Reference type in a wrong format");

      if (LastPos != nullptr)
        *LastPos = End + 1;
      return Types::Class;
    }

    case '[': {
      try {
        (void)parseFieldDescriptor(Desc.substr(1), LastPos);
        if (LastPos != nullptr)
          *LastPos += 1;
      } catch (std::exception &) {
        // This will also catch cases when substr(1) had throw and exception
        throw ParsingError("Array type in a wrong format");
      }
      return Types::Array;
    }

    default:
      throw ParsingError("Unrecognized field descriptor");
  }
}

std::pair<Type, std::vector<Type>>
StackFrame::parseMethodDescriptor(const std::string &Desc) {
  if (Desc.empty())
    throw ParsingError("Empty descriptor");

  if (Desc[0] != '(')
    throw ParsingError("Expected to find l-brace");

  auto RBracePos = Desc.find(')');
  if (RBracePos == std::string::npos)
    throw ParsingError("Expected to find r-brace");
  if (RBracePos == Desc.size() - 1)
    throw ParsingError("Expected to find return type descriptor");

  std::string ArgsDesc = Desc.substr(1, RBracePos - 1);
  std::string RetDesc = Desc.substr(RBracePos + 1);
  assert(!RetDesc.empty());

  // Parse argument types
  std::vector<Type> ArgTypes;
  std::size_t ArgsPos = 0;
  while (ArgsPos < ArgsDesc.length()) {
    std::size_t ArgEnd;
    auto NewType = parseFieldDescriptor(ArgsDesc.substr(ArgsPos), &ArgEnd);

    ArgTypes.push_back(NewType);
    ArgsPos += ArgEnd;
  }

  // Parse return type
  auto RetType = Types::Void;
  if (RetDesc != "V") {
    std::size_t RetEnd;
    RetType = parseFieldDescriptor(RetDesc, &RetEnd);
    if (RetEnd != RetDesc.length())
      throw ParsingError("Can't parse tail of the descriptor");
  }

  return std::pair(RetType, ArgTypes);
}

void StackFrame::setLocal(uint32_t Idx, Type T) {
  assert(Idx < locals().size());
  locals()[Idx] = T;

  if (Types::sizeOf(T) == 2) {
    assert(Idx + 1 < locals().size());
    locals()[Idx + 1] = Types::Top;
  }

  computeFlags();
  assert(verifyTypeEncoding());
}
