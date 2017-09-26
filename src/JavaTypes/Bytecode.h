//
// This file defines internal bytecode representation.
//

#ifndef ICP_BYTECODE_H
#define ICP_BYTECODE_H

#include <cstdint>
#include <ostream>
#include <memory>
#include <cassert>
#include <vector>

namespace JavaTypes::Bytecode {

using BciType = uint32_t;
using Container = const std::vector<uint8_t>;
using ContainerIterator = Container::const_iterator;

// Exceptions
class UnexpectedBytecodeOperation: public std::exception {};
class BytecodeParsingError: public std::exception {};
class UnknownBytecode: public std::exception {};

// Represents single bytecode instruction. Supposed to be created from the
// byte array using 'Instruction::create' or 'parseInstruction' functions.
class Instruction {
public:
  // These constants are used in 'create' function and are expected to be
  // found in each subclass.

  // Instruction bytecode
  static constexpr uint8_t OpCode = 0x00;

  // Number of bytes used to store this instruction including one byte for the
  // opcode.
  static constexpr uint8_t Length = 0;

public:
  // No copies
  Instruction(const Instruction&) = delete;
  Instruction &operator=(const Instruction &) = delete;

  BciType getBci() {
    return bci;
  }

  // Useful dynamic type cast functions
  template<class RetType>
  bool isA() const {
    return dynamic_cast<const RetType*>(this) != nullptr;
  }

  template<class RetType>
  const RetType &getAs() const {
    const RetType *Res = dynamic_cast<const RetType*>(this);
    if (Res == nullptr)
      throw UnexpectedBytecodeOperation();
    return *Res;
  }

  template<class RetType>
  const RetType *getAsOrNull() const {
    return dynamic_cast<const RetType*>(this);
  }

  // Print information about this instruction.
  // This is intended as a debug output and should not be relied on for
  // correctness.
  virtual void print(std::ostream &Out) = 0;

  // Creates instruction and advances the iterator.
  // \param It Iterator pointing to the beginning of the instruction
  // \returns New instruction.
  // \throws BytecodeParsingError if length of the container was less than
  // instruction length.
  template<class InstructionType>
  static std::unique_ptr<Instruction> create(
      Container &Bytecodes, ContainerIterator &It);

protected:
  // This is supposed to be called only from 'create' function
  Instruction(ContainerIterator It, BciType bci):
      bci(bci) {
    (void)It; // This parameter is used in the inherited classes
  }

private:
  const BciType bci;
};

// Creates instruction and advances iterator.
// \returns New instruction.
// \throws UndefinedBytecode if opcode was not recognized.
// \throws BytecodeParsingError if length of the container was less than
// instruction length.
std::unique_ptr<Instruction> parseInstruction(
    Container &Bytecodes, ContainerIterator &It);

template<class InstructionType>
std::unique_ptr<Instruction>
Instruction::create(Container &Bytecodes, ContainerIterator &It) {
  // Check that we can parse this instruction
  if (std::distance(It, Bytecodes.end()) < InstructionType::Length)
    throw BytecodeParsingError();
  assert(*It == InstructionType::OpCode);

  // Compute bci
  BciType bci = static_cast<BciType>(std::distance(Bytecodes.begin(), It));

  // Create instruction
  auto Res = std::unique_ptr<InstructionType>(new InstructionType(It, bci));

  // Advance iterator
  It += InstructionType::Length;

  return Res;
}

}

#endif //ICP_BYTECODE_H
