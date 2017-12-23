//
// Intentionally left empty
//

#include "InstructionVisitor.h"

#include "Bytecode/Instructions.h"

using namespace Bytecode;

#define DEF_FWD_VISIT(TargetType, SourceType) \
void InstructionVisitor::visit(const SourceType &I) {\
  visit(TargetType(I));\
}

DEF_FWD_VISIT(iconst_val, iconst_m1)
DEF_FWD_VISIT(iconst_val, iconst_0)
DEF_FWD_VISIT(iconst_val, iconst_1)
DEF_FWD_VISIT(iconst_val, iconst_2)
DEF_FWD_VISIT(iconst_val, iconst_3)
DEF_FWD_VISIT(iconst_val, iconst_4)
DEF_FWD_VISIT(iconst_val, iconst_5)

DEF_FWD_VISIT(dconst_val, dconst_0)
DEF_FWD_VISIT(dconst_val, dconst_1)

DEF_FWD_VISIT(if_icmp_op, if_icmpeq)
DEF_FWD_VISIT(if_icmp_op, if_icmpne)
DEF_FWD_VISIT(if_icmp_op, if_icmplt)
DEF_FWD_VISIT(if_icmp_op, if_icmpge)
DEF_FWD_VISIT(if_icmp_op, if_icmpgt)
DEF_FWD_VISIT(if_icmp_op, if_icmple)

#undef DEF_VISIT
