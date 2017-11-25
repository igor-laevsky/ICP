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
    MaxLocals(Params.MaxLocals),
    Code(std::move(Params.Code)),
    StackMapTable(std::move(Params.StackMapTable))
{
  assert(Name != nullptr);
  assert(Descriptor != nullptr);

  // Null check all instructions
  assert(std::all_of(
      Code.begin(), Code.end(),
      [](const auto &Inst) { return Inst != nullptr; }));

  // Other flags are not supported currently
  assert(
      getAccessFlags() == AccessFlags::ACC_PUBLIC ||
      getAccessFlags() == (AccessFlags::ACC_PUBLIC | AccessFlags::ACC_STATIC));
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
