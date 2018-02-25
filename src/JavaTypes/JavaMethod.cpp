//
// JavaMethod class implementation
//

#include "JavaMethod.h"

#include <ostream>

using namespace JavaTypes;
using namespace Bytecode;

// Helper for the name and descriptor initializers.
static const ConstantPoolRecords::Utf8 &
checkRecord(const ConstantPoolRecords::Utf8 *Rec) {
  assert(Rec != nullptr); // can never be null
  return *Rec;
}

JavaMethod::JavaMethod(JavaMethod::MethodConstructorParameters &&Params) :
    Owner(nullptr),
    Flags(Params.Flags),
    Name(checkRecord(Params.Name)),
    Descriptor(checkRecord(Params.Descriptor)),
    MaxStack(Params.MaxStack),
    MaxLocals(Params.MaxLocals),
    CodeOwner(std::move(Params.Code)),
    StackMapBuilder(std::move(Params.StackMapBuilder))
{
  // Fill code viewer
  BciType cur_bci = 0;
  for (auto &Inst: CodeOwner) {
    assert(Inst != nullptr);
    this->Code.insert_back(cur_bci, Inst.get());
    cur_bci += Inst->getLength();
  }

  // Other flags are not supported currently
  assert(
      getAccessFlags() == AccessFlags::ACC_PUBLIC ||
      getAccessFlags() == AccessFlags::ACC_STATIC ||
      getAccessFlags() == AccessFlags::ACC_PUBLIC_STATIC);
}

void JavaMethod::print(std::ostream &Out) const {
  Out << getName() << " " << getDescriptor() << "\n";
  Out << "MaxStack: " << getMaxStack() << " MaxLocals: " << getMaxLocals() << "\n";
  Out << "Code:\n";

  for (auto It = this->begin(), End = this->end(); It != End; ++It) {
    Out << "  " << It.getBci() << ": ";
    (*It)->print(Out);
  }
}

JavaMethod::CodeIterator
JavaMethod::getCodeIterAtBci(Bytecode::BciType Bci) const {
  BciType cur_bci = 0;

  // This should be optimized
  for (auto It = begin(), End = end(); It != End; ++It) {
    if (cur_bci == Bci)
      return It;
    cur_bci += (*It)->getLength();
  }

  assert(false); // trying to get instruction at non existent bci
  return this->end();
}
