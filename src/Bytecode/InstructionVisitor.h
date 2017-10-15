
#ifndef ICP_INSTRUCTIONVISITOR_H
#define ICP_INSTRUCTIONVISITOR_H

#include <cassert>

#include "InstructionsFwd.h"

namespace Bytecode {

class InstructionVisitor {
public:
  virtual void visit(const aload_0 &) = 0;
  virtual void visit(const invokespecial &) = 0;
  virtual void visit(const java_return &) = 0;
  virtual void visit(const iconst_0 &) = 0;
  virtual void visit(const ireturn &) = 0;
};

}


#endif //ICP_INSTRUCTIONVISITOR_H
