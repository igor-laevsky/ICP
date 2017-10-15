//
// Tests for the bytecode handling infrastructure
//

#include "catch.hpp"

#include "Bytecode/Bytecode.h"
#include "Bytecode/BytecodeInstructions.h"
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

  void visit(const iconst_0 &) override {
    assert(false);
  }

  void visit(const ireturn &) override {
    assert(false);
  }

  bool seenEverything() const {
    return seenAload && seenInvoke && seenRet;
  }

private:
  bool seenAload = false;
  bool seenInvoke = false;
  bool seenRet = false;
};

}

TEST_CASE("Basic", "[Bytecode][Visitor]") {
  const std::vector<uint8_t> Bytes =
      {0x2a,             // aload_0
       0xb7, 0x00, 0x01, // invokespecial #1
       0xb1 };           // return

  std::vector<std::unique_ptr<Instruction>> Insts;

  auto It = Bytes.begin();
  while (It != Bytes.end()) {
    Insts.push_back(parseInstruction(Bytes, It));
  }

  TestVisitor V;
  for (const auto &I: Insts) {
    I->accept(V);
  }
}
