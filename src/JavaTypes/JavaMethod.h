//
// This class represents java method.
//

#ifndef ICP_JAVAMETHOD_H
#define ICP_JAVAMETHOD_H

#include "ConstantPool.h"
#include "ConstantPoolRecords.h"
#include "Bytecode/Bytecode.h"
#include "Utils/Iterators.h"
#include "Verifier/StackFrame.h"

namespace JavaTypes {

class JavaClass;

class JavaMethod final {
public:
  using CodeOwnerType =
    std::vector<std::unique_ptr<Bytecode::Instruction>>;
  using CodeIterator = Utils::SmartPtrIterator<CodeOwnerType::const_iterator>;

  using StackMapTableType =
    std::vector<std::pair<Bytecode::BciType, Verifier::StackFrame>>;

  enum class AccessFlags: uint16_t {
    ACC_NONE = 0x0000,
    ACC_PUBLIC = 0x0001,
    ACC_PRIVATE = 0x0002,
    ACC_PROTECTED = 0x0004,
    ACC_STATIC = 0x0008,
    ACC_FINAL = 0x0010,
    ACC_SYNCHRONIZED = 0x0020,
    ACC_BRIDGE = 0x0040,
    ACC_VARARGS = 0x0080,
    ACC_NATIVE = 0x0100,
    ACC_ABSTRACT = 0x0400,
    ACC_STRICT = 0x0800,
    ACC_SYNTHETIC = 0x1000,

    // Shortcuts for the widely used flags
    ACC_PUBLIC_STATIC = ACC_PUBLIC | ACC_STATIC
  };

  // Allows using AccessFlags as a bitfield.
  friend constexpr AccessFlags operator|(AccessFlags Lhs, AccessFlags Rhs) {
    return static_cast<AccessFlags>(
        static_cast<uint16_t>(Lhs) | static_cast<uint16_t>(Rhs));
  }
  friend constexpr bool operator&(AccessFlags Lhs, AccessFlags Rhs) {
    return static_cast<bool>(
        static_cast<uint16_t>(Lhs) & static_cast<uint16_t>(Rhs));
  }


  // This is only to simplify constructor parameter list. No additional
  // semantic meaning is implied.
  struct MethodConstructorParameters {
    AccessFlags Flags = AccessFlags::ACC_NONE;

    const ConstantPoolRecords::Utf8 *Name = nullptr;
    const ConstantPoolRecords::Utf8 *Descriptor = nullptr;

    uint16_t MaxStack = 0;
    uint16_t MaxLocals = 0;

    CodeOwnerType Code; // This is plain unparsed bytecode

    StackMapTableType StackMapTable;
  };

public:
  explicit JavaMethod(MethodConstructorParameters &&Params);

  // No copies
  JavaMethod(const JavaMethod&) = delete;
  JavaMethod &operator=(const JavaMethod &) = delete;

  const Utf8String &getName() const {
    return Name.getValue();
  }
  const Utf8String &getDescriptor() const {
    return Descriptor.getValue();
  }

  uint16_t getMaxStack() const { return MaxStack; }
  uint16_t getMaxLocals() const { return MaxLocals; }
  AccessFlags getAccessFlags() const { return Flags; }

  const JavaClass &getOwner() const {
    // Never request owner before it was assigned
    assert(Owner != nullptr);
    return *Owner;
  }
  void setOwner(const JavaClass &NewOwner) {
    // Only set owner once
    assert(Owner == nullptr);
    Owner = &NewOwner;
  }

  const auto &getStackMapTable() const { return StackMapTable; }

  // Get bci for the specified instruction
  Bytecode::BciType getBciForInst(const Bytecode::Instruction &Inst) const;

  // Get instruction at specified bci.
  // \throws WrongBci if no such instruction is found.
  const Bytecode::Instruction &getInstrAtBci(Bytecode::BciType Bci) const;

  // Get code iterator for the given bci.
  // \throws WrongBci if no such instruction is found.
  CodeIterator getCodeIterAtBci(Bytecode::BciType Bci) const;

  // Support ranged-for iteration over instructions.
  CodeIterator begin() const { return CodeIterator(Code.cbegin()); }
  CodeIterator end() const { return CodeIterator(Code.cend()); }

  Bytecode::BciType numInstructions() const {
    return static_cast<Bytecode::BciType>(Code.size());
  }

  void print(std::ostream &Out) const;

private:
  const JavaClass *Owner;

  const AccessFlags Flags;

  const ConstantPoolRecords::Utf8 &Name;
  const ConstantPoolRecords::Utf8 &Descriptor;

  const uint16_t MaxStack;
  const uint16_t MaxLocals;

  CodeOwnerType Code;

  StackMapTableType StackMapTable;
};

}

#endif //ICP_JAVAMETHOD_H
