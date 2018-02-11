//
// Tests for the bytecode handling infrastructure
//

#include "catch.hpp"

#include "Bytecode/Bytecode.h"
#include "Bytecode/Instructions.h"
#include "Bytecode/InstructionVisitor.h"

using namespace Bytecode;

namespace {

class TestVisitor: public InstructionVisitor {
public:
  void visit(const aload_0 &) override {
    seenAload = true;
  }

  void visit(const invokespecial &) override {
    seenInvoke = true;
  }

  void visit(const java_return &) override {
    seenRet = true;
  }

  void visit(const ireturn &) override {
    assert(false);
  }

  void visit(const aload &) override {
   assert(false);
  }

  void visit(const putstatic &) override {
    assert(false);
  }

  void visit(const getstatic &) override {
    assert(false);
  }

  void visit(const iconst_val &) override {
    seenIconstVal = true;
  }

  void visit(const dconst_val &) override {
    seenDconstVal = true;
  }

  bool seenEverything() const {
    return seenAload && seenInvoke && seenRet &&
        seenIconstVal && seenIconst1 &&
        seenDconstVal && !seenDconst0;
  }

private:
  bool seenAload = false;
  bool seenInvoke = false;
  bool seenRet = false;
  bool seenIconstVal = false;
  bool seenIconst1 = false;
  bool seenDconstVal = false;
  bool seenDconst0 = false;
};

}

TEST_CASE("Basic bytecode visitor", "[Bytecode][Visitor]") {
  const std::vector<uint8_t> Bytes =
      {0x2a,             // aload_0
       0xb7, 0x00, 0x01, // invokespecial #1
       iconst_0::OpCode,
       iconst_1::OpCode,
       dconst_1::OpCode,
       0xb1 };           // return

  auto Insts = parseInstructions(Bytes);

  TestVisitor V;
  for (const auto &I: Insts) {
    I->accept(V);
  }
}
