//
// Implementation for the bytecode representation
//

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
    PARSE_OP(aload_0);
    PARSE_OP(invokespecial);
    PARSE_OP(java_return);
    PARSE_OP(ireturn);
    PARSE_OP(iconst_0);
    default:
      throw UnknownBytecode();
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