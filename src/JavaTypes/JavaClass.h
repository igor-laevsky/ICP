//
// This file contains definition of the JavaClass class.
// This class is intended to contain all information about single java class:
// i.e constant pool, methods, fields and so on. Essentially it represents
// parsed class file.
//

#ifndef IJVM_JAVACLASS_H
#define IJVM_JAVACLASS_H

#include <optional>

#include "ConstantPool.h"
#include "ConstantPoolRecords.h"

namespace JavaTypes {

class JavaClass final {
public:
  enum class AccessFlags: uint16_t {
    ACC_NONE = 0x0000,
    ACC_PUBLIC = 0x0001,
    ACC_FINAL = 0x0010,
    ACC_SUPER = 0x0020,
    ACC_INTERFACE = 0x0200,
    ACC_ABSTRACT = 0x0400,
    ACC_SYNTHETIC = 0x1000,
    ACC_ANNOTATION = 0x2000,
    ACC_ENUM = 0x4000
  };

  // This structure is intended to simplify passing constructor arguments.
  // It doesn't have any additional semantic meaning.
  struct ClassParameters {
    const ConstantPoolRecords::ClassInfo *ClassName = nullptr;
    const ConstantPoolRecords::ClassInfo *SuperClass = nullptr;
    AccessFlags Flags = AccessFlags::ACC_NONE;
    std::unique_ptr<ConstantPool> CP = nullptr;
  };

public:
  // Rvalue parameter is to emphasize the fact that we will transfer
  // ownership of the internal unique_ptr (ConstantPool), which makes
  // ClassParameters structure invalid after this constructor was executed.
  explicit JavaClass(ClassParameters &&Params):
      ClassName(*Params.ClassName),
      SuperClass(Params.SuperClass),
      Flags(Params.Flags),
      CP(std::move(Params.CP))
  {
    // It is responsibility of the user to provide valid constant pool.
    assert(CP != nullptr && CP->verify());
  }

  AccessFlags getAccessFlags() const {
    return Flags;
  }

  const ConstantPool &getConstantPool() const {
    return *CP;
  }

  const Utf8String &getClassName() const {
    return ClassName.getName();
  }

  // \returns nullptr if class doesn't have super class, pointer to the
  // class name otherwise.
  const Utf8String *getSuperClassName() const {
    if (SuperClass == nullptr)
      return nullptr;
    return &SuperClass->getName();
  }

  // Verifies the class. Includes verification of the constant pool and all
  // class methods.
  // \returns true in case is class is valid, false otherwise.
  bool verify(std::string &ErrorMessage) const;

  void print(std::ostream &Out) const;

private:
  const ConstantPoolRecords::ClassInfo &ClassName;
  // Maybe null if this is an object class
  const ConstantPoolRecords::ClassInfo *SuperClass;

  const AccessFlags Flags;

  const std::unique_ptr<ConstantPool> CP;
};

// Allows using AccessFlags as a bitfield.
constexpr JavaClass::AccessFlags operator|(
    JavaClass::AccessFlags Lhs, JavaClass::AccessFlags Rhs) {
  return static_cast<JavaClass::AccessFlags>(
      static_cast<uint16_t>(Lhs) | static_cast<uint16_t>(Rhs));
}

}

#endif //IJVM_JAVACLASS_H
