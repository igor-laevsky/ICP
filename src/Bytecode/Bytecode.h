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

#include "Bytecode/BytecodeFwd.h"
#include "Bytecode/InstructionVisitor.h"

namespace Bytecode {

// Exceptions
class UnexpectedBytecodeOperation: public std::exception {};
class BytecodeParsingError: public std::exception {};
class UnknownBytecode: public std::runtime_error {
  using runtime_error::runtime_error;
};

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

  // Human readable name of this instruction
  static constexpr const char *Name = "";

public:
  virtual ~Instruction() = default;

  // No copies
  Instruction(const Instruction&) = delete;
  Instruction &operator=(const Instruction &) = delete;

  // Support visitor pattern
  virtual void accept(InstructionVisitor &V) const = 0;

  // Return true if this instruction has type 'RetType'
  template<class RetType>
  bool isA() const {
    return dynamic_cast<const RetType*>(this) != nullptr;
  }

  // Cast instruction to the type 'RetType' or throw an exception.
  // \throws UnexpectedBytecodeOperation If this instruction is of the wrong type.
  template<class RetType>
  const RetType &getAs() const {
    const RetType *Res = dynamic_cast<const RetType*>(this);
    if (Res == nullptr)
      throw UnexpectedBytecodeOperation();
    return *Res;
  }

  // Cast instruction to the type 'RetType' or return null if it is impossible.
  template<class RetType>
  const RetType *getAsOrNull() const {
    return dynamic_cast<const RetType*>(this);
  }

  // Get length of this inistruction. So far this is the only common field for
  // the all instructions.
  virtual uint8_t getLength() const = 0;

  // Print information about this instruction.
  // This is intended as a debug output and should not be relied on for
  // correctness.
  virtual void print(std::ostream &Out) const = 0;

  // Creates instruction and advances the iterator.
  // \param It Iterator pointing to the beginning of the instruction
  // \returns New instruction.
  // \throws BytecodeParsingError if length of the container was less than
  // instruction length.
  template<class InstructionType>
  static std::unique_ptr<Instruction> create(
      const Container &Bytecodes, ContainerIterator &It);

  // Directly create an instruction. Arg1 is unused for single index
  // instructions.
  template<class InstructionType>
  static std::unique_ptr<Instruction> create(IdxType Arg1 = 0);

protected:
  Instruction() = default;
};

// This class is used as a base in CRTP to simplify visitor implementation
template<class ConcreteType>
class VisitableInstruction: public Instruction {
public:
  // Support instruction visitor
  void accept(InstructionVisitor &V) const override {
    V.visit(static_cast<const ConcreteType&>(*this));
  }

  // Also implement getLength. It might not be a perfect place for this
  // but adding another inheritance level seems to be redundant.
  uint8_t getLength() const override { return ConcreteType::Length; }

protected:
  VisitableInstruction() = default;
};

template<class InstructionType>
std::unique_ptr<Instruction>
Instruction::create(const Container &Bytecodes, ContainerIterator &It) {
  // Check that we can parse this instruction
  if (std::distance(It, Bytecodes.end()) < InstructionType::Length)
    throw BytecodeParsingError();
  assert(*It == InstructionType::OpCode);

  // Create instruction
  auto Res = std::unique_ptr<InstructionType>(new InstructionType(It));

  // Advance iterator
  It += InstructionType::Length;

  return Res;
}

template<class InstructionType>
std::unique_ptr<Instruction> Instruction::create(IdxType Arg1/* = 0*/) {
  Container Bytecode;

  if constexpr (InstructionType::Length == 1) {
    Bytecode = {InstructionType::OpCode};
  }
  else if constexpr (InstructionType::Length == 3) {
    Bytecode = {InstructionType::OpCode,
                static_cast<uint8_t>((Arg1 & 0xFF00) >> 8),
                static_cast<uint8_t>(Arg1 & 0x00FF)};
  }
  else {
    assert(false); // Unhandled instruction length
  }

  return
      std::unique_ptr<InstructionType>(new InstructionType(Bytecode.cbegin()));
}

// Parses all instructions from the specified container.
// \throws UndefinedBytecode if opcode was not recognized.
// \throws BytecodeParsingError if length of the container was less than
// instruction length.
std::vector<std::unique_ptr<Instruction>> parseInstructions(
    const Container &Bytecodes);

// Creates instruction and advances the iterator.
// \returns New instruction.
// \throws UndefinedBytecode if opcode was not recognized.
// \throws BytecodeParsingError if length of the container was less than
// instruction length.
std::unique_ptr<Instruction> parseInstruction(
    const Container &Bytecodes, ContainerIterator &It);

// Parses single instruction from it's string representation.
// Receives string which names the opcode and it's indexes.
// \returns New bytecode
// \throws UndefinedBytecode if opcode was not recognized.
std::unique_ptr<Instruction> parseFromString(
    const std::string_view OpCodeStr, IdxType Idx = 0);

}

#endif //ICP_BYTECODE_H
