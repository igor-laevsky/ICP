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
    PARSE_OP(aload);
    PARSE_OP(aload_0);
    PARSE_OP(invokespecial);
    PARSE_OP(java_return);
    PARSE_OP(ireturn);
    PARSE_OP(dreturn);
    PARSE_OP(iconst_m1);
    PARSE_OP(iconst_0);
    PARSE_OP(iconst_1);
    PARSE_OP(iconst_2);
    PARSE_OP(iconst_3);
    PARSE_OP(iconst_4);
    PARSE_OP(iconst_5);
    PARSE_OP(putstatic);
    PARSE_OP(getstatic);
    PARSE_OP(dconst_0);
    PARSE_OP(dconst_1);

    PARSE_OP(if_icmpeq);
    PARSE_OP(if_icmpne);
    PARSE_OP(if_icmplt);
    PARSE_OP(if_icmpge);
    PARSE_OP(if_icmpgt);
    PARSE_OP(if_icmple);
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

std::unique_ptr<Instruction> Bytecode::parseFromString(
    const std::string_view OpCodeStr, IdxType Idx /*= 0*/) {

#define PARSE_OP(OpType) \
  if (OpCodeStr == OpType::Name) \
    return Instruction::create<OpType>(Idx);

  PARSE_OP(aload);
  PARSE_OP(aload_0);
  PARSE_OP(invokespecial);
  PARSE_OP(java_return);
  PARSE_OP(ireturn);
  PARSE_OP(dreturn);
  PARSE_OP(iconst_m1);
  PARSE_OP(iconst_0);
  PARSE_OP(iconst_1);
  PARSE_OP(iconst_2);
  PARSE_OP(iconst_3);
  PARSE_OP(iconst_4);
  PARSE_OP(iconst_5);
  PARSE_OP(putstatic);
  PARSE_OP(getstatic);
  PARSE_OP(dconst_0);
  PARSE_OP(dconst_1);

  PARSE_OP(if_icmpeq);
  PARSE_OP(if_icmpne);
  PARSE_OP(if_icmplt);
  PARSE_OP(if_icmpge);
  PARSE_OP(if_icmpgt);
  PARSE_OP(if_icmple);

#undef PARSE_OP

  throw UnknownBytecode(); // unable to find correct instruction for string
}
