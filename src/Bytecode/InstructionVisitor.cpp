//
// Intentionally left empty
//

#include "InstructionVisitor.h"

#include "Bytecode/Instructions.h"

using namespace Bytecode;

void InstructionVisitor::visit(const iconst_m1 &I) {
  visit(iconst_val(I));
}
void InstructionVisitor::visit(const iconst_0 &I)  {
  visit(iconst_val(I));
}
void InstructionVisitor::visit(const iconst_1 &I) {
  visit(iconst_val(I));
}
void InstructionVisitor::visit(const iconst_2 &I) {
  visit(iconst_val(I));
}
void InstructionVisitor::visit(const iconst_3 &I) {
  visit(iconst_val(I));
}
void InstructionVisitor::visit(const iconst_4 &I) {
  visit(iconst_val(I));
}
void InstructionVisitor::visit(const iconst_5 &I) {
  visit(iconst_val(I));
}

void InstructionVisitor::visit(const dconst_0 &I) {
  visit(dconst_val(I));
}
void InstructionVisitor::visit(const dconst_1 &I) {
  visit(dconst_val(I));
}
