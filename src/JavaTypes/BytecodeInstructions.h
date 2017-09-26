//
// Definitions for the bytecode instructions
//

#ifndef ICP_BYTECODEINSTRUCTIONS_H
#define ICP_BYTECODEINSTRUCTIONS_H

#include "Bytecode.h"
#include "ConstantPool.h"

namespace JavaTypes::Bytecode::Instructions {

class aload_0 final: public Instruction {
public:
  static constexpr uint8_t Length = 1;
  static constexpr uint8_t OpCode = 0x2a;

public:
  void print(std::ostream &Out) override {
    Out << "aload_0\n";
  }

protected:
  // Simply inherit the constructor
  using Instruction::Instruction;
};

class invokespecial final: public Instruction {
public:
  static constexpr uint8_t Length = 3;
  static constexpr uint8_t OpCode = 0xb7;

public:

  ConstantPool::IndexType getIdx() {
    return (*(getIt() + 1) << 8) | *(getIt() + 2);
  }

  void print(std::ostream &Out) override {
    Out << "invokespecial #" << getIdx() << "\n";
  }

protected:
  using Instruction::Instruction;
};

class java_return final: public Instruction {
public:
  static constexpr uint8_t Length = 1;
  static constexpr uint8_t OpCode = 0xb1;

public:
  void print(std::ostream &Out) override {
    Out << "return\n";
  }

protected:
  using Instruction::Instruction;
};

}


#endif //ICP_BYTECODEINSTRUCTIONS_H
