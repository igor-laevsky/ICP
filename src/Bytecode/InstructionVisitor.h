
#ifndef ICP_INSTRUCTIONVISITOR_H
#define ICP_INSTRUCTIONVISITOR_H

#include <cassert>

#include "BytecodeFwd.h"

namespace Bytecode {

class InstructionVisitor {
public:
#define DEF_VISIT(Instr) \
  virtual void visit(const Instr &) {\
    assert(false && "unimplemented"); \
  }

#define HANDLE_INSTR(ClassName) DEF_VISIT(ClassName)
#define HANDLE_WRAPPER(ClassName) DEF_VISIT(ClassName)
#include "Instructions.inc"

  // Just to be safe
  virtual ~InstructionVisitor() = default;
};

}

#endif //ICP_INSTRUCTIONVISITOR_H
