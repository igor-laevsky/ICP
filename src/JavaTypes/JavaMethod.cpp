//
// JavaMethod class implementation
//

#include "JavaMethod.h"

#include <ostream>

using namespace JavaTypes;

JavaMethod::JavaMethod(JavaMethod::MethodConstructorParameters &&Params) :
    Owner(nullptr),
    Flags(Params.Flags),
    Name(Params.Name),
    Descriptor(Params.Descriptor),
    MaxStack(Params.MaxStack),
    MaxLocals(Params.MaxLocals)
{
  assert(Name != nullptr);
  assert(Descriptor != nullptr);

  auto CodeIt = Params.Code.cbegin();

  while (CodeIt != Params.Code.end())
    Code.push_back(Bytecode::parseInstruction(Params.Code, CodeIt));
}

bool JavaMethod::verify(std::string &ErrorMessage) const {
  if (Flags != AccessFlags::ACC_PUBLIC &&
      Flags != (AccessFlags::ACC_PUBLIC | AccessFlags::ACC_STATIC)) {
    ErrorMessage = "Unsupported access flags";
    return false;
  }

  return true;
}

void JavaMethod::print(std::ostream &Out) const {
  Out << getName() << " " << getDescriptor() << "\n";
  Out << "MaxStack: " << getMaxStack() << " MaxLocals: " << getMaxLocals() << "\n";
  Out << "Code:\n";

  for (const auto &Instr: *this) {
    Out << "  " << Instr.getBci() << ": ";
    Instr.print(Out);
  }
}

const Bytecode::Instruction &
JavaMethod::getInstrAtBci(Bytecode::BciType Bci) const {
  // This is can be more efficient, but it doesn't matter for now
  for (const auto &Instr: *this) {
    if (Instr.getBci() == Bci)
      return Instr;
  }

  throw WrongBci();
}
