//
// Implementation for the bytecode representation
//

#include "Bytecode.h"
#include "BytecodeInstructions.h"

using namespace JavaTypes::Bytecode;
using namespace JavaTypes::Bytecode::Instructions;

std::unique_ptr<Instruction> JavaTypes::Bytecode::parseInstruction(
  Container &Bytecodes, ContainerIterator &It) {

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
    default:
      throw UnknownBytecode();
  }

#undef PARSE_OP
}
