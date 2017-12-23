
#ifndef ICP_INSTRUCTIONVISITOR_H
#define ICP_INSTRUCTIONVISITOR_H

#include <cassert>

#include "InstructionsFwd.h"

namespace Bytecode {

class InstructionVisitor {
public:
#define DEF_VISIT(Instr) \
  virtual void visit(const Instr &) {\
    assert(false && "unimplemented"); \
  }
#define DECL_VISIT(Instr) virtual void visit(const Instr &)

  DEF_VISIT(aload);
  DEF_VISIT(aload_0);
  DEF_VISIT(invokespecial);
  DEF_VISIT(java_return);
  DEF_VISIT(ireturn);
  DEF_VISIT(dreturn);
  DEF_VISIT(putstatic);
  DEF_VISIT(getstatic);

  DEF_VISIT(iconst_val);
  DECL_VISIT(iconst_m1);
  DECL_VISIT(iconst_0);
  DECL_VISIT(iconst_1);
  DECL_VISIT(iconst_2);
  DECL_VISIT(iconst_3);
  DECL_VISIT(iconst_4);
  DECL_VISIT(iconst_5);

  DEF_VISIT(dconst_val);
  DECL_VISIT(dconst_0);
  DECL_VISIT(dconst_1);

  DEF_VISIT(if_icmp_op);
  DECL_VISIT(if_icmpeq);
  DECL_VISIT(if_icmpne);
  DECL_VISIT(if_icmplt);
  DECL_VISIT(if_icmpge);
  DECL_VISIT(if_icmpgt);
  DECL_VISIT(if_icmple);

#undef DEF_VISIT
#undef DECL_VISIT

  // Just to be safe
  virtual ~InstructionVisitor() = default;
};

}

#endif //ICP_INSTRUCTIONVISITOR_H
