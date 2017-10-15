//
// Definitions for the bytecode instructions
//

#ifndef ICP_BYTECODEINSTRUCTIONS_H
#define ICP_BYTECODEINSTRUCTIONS_H

#include "Bytecode.h"

namespace Bytecode {

// Utility class for instruction consisting of a single byte.
template<class ConcreteType>
class NoIndex: public VisitableInstruction<ConcreteType> {
  using VisitableInstruction<ConcreteType>::VisitableInstruction;

public:
  static constexpr uint8_t Length = 1;
};

// Utility class for three byte instructions. First byte is opcode,
// second two represent constant pool index.
template<class ConcreteType>
class SingleIndex: public VisitableInstruction<ConcreteType> {
public:
  static constexpr uint8_t Length = 3;

public:
  auto getIdx() const {
    return Idx;
  }

private:
  SingleIndex(ContainerIterator It, BciType bci):
      VisitableInstruction<ConcreteType>(It, bci),
      Idx((*(It + 1) << 8) | *(It + 2)) {
    ;
  }

  // Allow calling constructor from the create function
  template<class InstructionType>
  friend std::unique_ptr<Instruction> Instruction::create(
      const Container &Bytecodes, ContainerIterator &It);

private:
  const IdxType Idx;
};

class aload_0 final: public NoIndex<aload_0> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0x2a;

  void print(std::ostream &Out) const override {
    Out << "aload_0\n";
  }
};

class invokespecial final: public SingleIndex<invokespecial> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0xb7;

  void print(std::ostream &Out) const override {
    Out << "invokespecial #" << getIdx() << "\n";
  }
};

class java_return final: public NoIndex<java_return> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0xb1;

  void print(std::ostream &Out) const override {
    Out << "return\n";
  }
};

class iconst_0 final: public NoIndex<iconst_0> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0x03;

  void print(std::ostream &Out) const override {
    Out << "iconst_0\n";
  }
};

class ireturn final: public NoIndex<ireturn> {
  using NoIndex<ireturn>::NoIndex;

public:
  static constexpr uint8_t OpCode = 0xac;

  void print(std::ostream &Out) const override {
    Out << "ireturn\n";
  }
};

}

#endif //ICP_BYTECODEINSTRUCTIONS_H
