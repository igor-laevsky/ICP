//
// JavaMethod class implementation
//

#include "JavaMethod.h"

#include <ostream>

using namespace JavaTypes;
using namespace Bytecode;

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
    Out << "  " << getBciForInst(Instr) << ": ";
    Instr.print(Out);
  }
}

BciType JavaMethod::getBciForInst(const Instruction &Inst) const {
  BciType cur_bci = 0;

  // This should be optimized
  for (const auto &CurInst: *this) {
    if (CurInst == Inst)
      return cur_bci;
    cur_bci += CurInst.getLength();
  }

  assert(false); // trying to get bci for the non existent instruction.
  return 0;
}

const Instruction &
JavaMethod::getInstrAtBci(BciType Bci) const {
  BciType cur_bci = 0;

  // This should be optimized
  for (const auto &Inst: *this) {
    if (cur_bci == Bci)
      return Inst;
    cur_bci += Inst.getLength();
  }

  assert(false); // trying to get instruction at non existent bci
  return *this->begin();
}
