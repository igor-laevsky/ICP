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

// Utility class which wraps number of instructions with values encoded into
// their opcodes and provides uniform access to them.
template<class... Ts>
class ValueInstWrapper {
private:
  // Better move this into utility module but so far it's the only place
  // where it's needed.
  template<class Arg, class... Args>
  static constexpr bool is_valid_inst =
    std::disjunction<std::is_same<Arg, Args>...>::value;

public:
  template<
      class InstT,
      class X = std::enable_if_t<is_valid_inst<InstT, Ts...>>>
  constexpr ValueInstWrapper(const InstT& Inst) noexcept:
      Inst(Inst),
      Val(InstT::Val) {
    ;
  }

  constexpr int8_t getVal() const { return Val; }
  constexpr const Instruction &getInst() const { return Inst; }

private:
  const Instruction &Inst;
  int8_t Val = 0;
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
  static constexpr const uint8_t Val = 0;
};

class iconst_1 final: public NoIndex<iconst_1> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0x04;
  static constexpr const char *Name = "iconst_1";
  static constexpr const uint8_t Val = 1;
};

class iconst_val final: public ValueInstWrapper<iconst_0, iconst_1> {
  using ValueInstWrapper::ValueInstWrapper;
};

class dconst_0 final: public NoIndex<dconst_0> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0x0e;
  static constexpr const char *Name = "dconst_0";
  static constexpr const uint8_t Val = 0;
};

class dconst_1 final: public NoIndex<dconst_1> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0x0f;
  static constexpr const char *Name = "dconst_1";
  static constexpr const uint8_t Val = 1;
};

class dconst_val final: public ValueInstWrapper<dconst_0, dconst_1> {
  using ValueInstWrapper::ValueInstWrapper;
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
