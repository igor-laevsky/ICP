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

  void print(std::ostream &Out) const override {
    Out << ConcreteType::Name << "\n";
  }
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

  void print(std::ostream &Out) const override {
    Out << ConcreteType::Name << " #" << getIdx() << "\n";
  }

private:
  SingleIndex(ContainerIterator It, BciType bci):
      VisitableInstruction<ConcreteType>(It, bci),
      Idx((*(It + 1) << 8) | *(It + 2)) {
    ;
  }

  // Allow calling constructor from the Instruction::create functions
  friend class Instruction;

private:
  const IdxType Idx;
};

class aload_0 final: public NoIndex<aload_0> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0x2a;
  static constexpr const char *Name = "aload_0";
};

class aload final: public SingleIndex<aload> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0x2b;
  static constexpr const char *Name = "aload";
};

class invokespecial final: public SingleIndex<invokespecial> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0xb7;
  static constexpr const char *Name = "invokespecial";
};

class java_return final: public NoIndex<java_return> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0xb1;
  static constexpr const char *Name = "return";
};

class iconst_0 final: public NoIndex<iconst_0> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0x03;
  static constexpr const char *Name = "iconst_0";
};

class dconst_0 final: public NoIndex<dconst_0> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0x0e;
  static constexpr const char *Name = "dconst_0";
};

class dconst_1 final: public NoIndex<dconst_1> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0x0f;
  static constexpr const char *Name = "dconst_1";
};

class ireturn final: public NoIndex<ireturn> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0xac;
  static constexpr const char *Name = "ireturn";
};

class getstatic final: public SingleIndex<getstatic> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0xb2;
  static constexpr const char *Name = "getstatic";
};

class putstatic final: public SingleIndex<putstatic> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0xb3;
  static constexpr const char *Name = "putstatic";
};

}

#endif //ICP_BYTECODEINSTRUCTIONS_H
