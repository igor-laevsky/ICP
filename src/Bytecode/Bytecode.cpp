///
/// Implementation for the bytecode parsing functions.
///

#include "Bytecode.h"
#include "Instructions.h"

using namespace Bytecode;

std::unique_ptr<Instruction> Bytecode::parseInstruction(
  const Container &Bytecodes, ContainerIterator &It) {

  // There should be at least one byte so that we can read OpCode
  // Rest of the checks will happen inside 'create' function.
  if (It == Bytecodes.end())
    throw BytecodeParsingError();

#define PARSE_OP(OpType) \
    case OpType::OpCode: \
      return Instruction::create<OpType>(Bytecodes, It);

  switch (*It) {
#define HANDLE_INSTR_ALL(ClassName) PARSE_OP(ClassName)
#include "Instructions.inc"

    default:
      throw UnknownBytecode(std::to_string(*It));
  }

#undef PARSE_OP
}

std::vector<std::unique_ptr<Instruction>> Bytecode::parseInstructions(
    const Container &Bytecodes) {

  std::vector<std::unique_ptr<Instruction>> Ret;

  auto It = Bytecodes.begin();
  while (It != Bytecodes.end()) {
    Ret.push_back(parseInstruction(Bytecodes, It));
  }

  return Ret;
}

std::unique_ptr<Instruction> Bytecode::parseFromString(
    std::string_view OpCodeStr, IdxType Idx /*= 0*/) {

#define PARSE_OP(OpType) \
  if (OpCodeStr == OpType::Name) \
    return Instruction::create<OpType>(Idx);

#define HANDLE_INSTR_ALL(ClassName) PARSE_OP(ClassName)
#include "Instructions.inc"

#undef PARSE_OP

  // Unable to find correct instruction for string
  throw UnknownBytecode(OpCodeStr.data());
}
