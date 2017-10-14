
#ifndef ICP_INSTRUCTIONVISITOR_H
#define ICP_INSTRUCTIONVISITOR_H

#include <cassert>

#include "JavaTypes/BytecodeInstructionsFwd.h"

namespace JavaTypes::Bytecode {

class InstructionVisitor {
public:
  virtual void visit(const JavaTypes::Bytecode::aload_0 &) = 0;
  virtual void visit(const JavaTypes::Bytecode::invokespecial &) = 0;
  virtual void visit(const JavaTypes::Bytecode::java_return &) = 0;
  virtual void visit(const JavaTypes::Bytecode::iconst_0 &) = 0;
  virtual void visit(const JavaTypes::Bytecode::ireturn &) = 0;
};

}


#endif //ICP_INSTRUCTIONVISITOR_H
