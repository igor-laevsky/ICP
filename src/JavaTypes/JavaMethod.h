//
// This class represents java method.
//

#ifndef IJVM_JAVAMETHOD_H
#define IJVM_JAVAMETHOD_H

#include <string>

#include "ConstantPool.h"
#include "ConstantPoolRecords.h"

namespace JavaTypes {

class JavaClass;

class JavaMethod final {
public:
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
    ACC_SYNTHETIC = 0x1000
  };

  // This is only to simplify constructor parameter list. No additional
  // semantic meaning is implied.
  struct MethodConstructorParameters {
    AccessFlags Flags = AccessFlags::ACC_NONE;

    const ConstantPoolRecords::Utf8 *Name = nullptr;
    const ConstantPoolRecords::Utf8 *Descriptor = nullptr;

    uint16_t MaxStack = 0;
    uint16_t MaxLocals = 0;

    std::vector<uint8_t> Code;
  };

public:
  explicit JavaMethod(MethodConstructorParameters &&Params):
      Owner(nullptr),
      Flags(Params.Flags),
      Name(Params.Name),
      Descriptor(Params.Descriptor),
      MaxStack(Params.MaxStack),
      MaxLocals(Params.MaxLocals),
      Code(std::move(Params.Code))
  {
    assert(Name != nullptr);
    assert(Descriptor != nullptr);
  }

  // No copies
  JavaMethod(const JavaMethod&) = delete;
  JavaMethod &operator=(const JavaMethod &) = delete;

  const Utf8String &getName() const {
    assert(Name != nullptr);
    return Name->getValue();
  }
  const Utf8String &getDescriptor() const {
    assert(Descriptor != nullptr);
    return Descriptor->getValue();
  }
  uint16_t getMaxStack() const {
    return MaxStack;
  }
  uint16_t getMaxLocals() const {
    return MaxLocals;
  }

  const std::vector<uint8_t> &getCode() const {
    return Code;
  }

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

  bool verify(std::string &ErrorMessage) const;

  void print(std::ostream &Out) const;

private:
  const JavaClass *Owner;

  const AccessFlags Flags;

  const ConstantPoolRecords::Utf8 *const Name;
  const ConstantPoolRecords::Utf8 *const Descriptor;

  const uint16_t MaxStack;
  const uint16_t MaxLocals;

  const std::vector<uint8_t> Code;
};

// Allows using AccessFlags as a bitfield.
constexpr JavaMethod::AccessFlags operator|(
    JavaMethod::AccessFlags Lhs, JavaMethod::AccessFlags Rhs) {
  return static_cast<JavaMethod::AccessFlags>(
      static_cast<uint16_t>(Lhs) | static_cast<uint16_t>(Rhs));
}

}

#endif //IJVM_JAVAMETHOD_H
