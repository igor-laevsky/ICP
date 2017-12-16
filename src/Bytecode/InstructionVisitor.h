
#ifndef ICP_INSTRUCTIONVISITOR_H
#define ICP_INSTRUCTIONVISITOR_H

#include <cassert>

#include "InstructionsFwd.h"

namespace Bytecode {

class InstructionVisitor {
public:
  virtual void visit(const aload &) = 0;
  virtual void visit(const aload_0 &) = 0;
  virtual void visit(const invokespecial &) = 0;
  virtual void visit(const java_return &) = 0;
  virtual void visit(const ireturn &) = 0;
  virtual void visit(const putstatic &) = 0;
  virtual void visit(const getstatic &) = 0;

  virtual void visit(const iconst_val &) {
    assert(false); // unimplemented
  }
  virtual void visit(const iconst_0 &I);
  virtual void visit(const iconst_1 &I);

  virtual void visit(const dconst_val &) {
    assert(false); // unimplemented
  }
  virtual void visit(const dconst_0 &I);
  virtual void visit(const dconst_1 &I);

  // Just to be safe
  virtual ~InstructionVisitor() = default;
};

}

#endif //ICP_INSTRUCTIONVISITOR_H
