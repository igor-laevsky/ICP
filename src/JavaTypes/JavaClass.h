//
// This file contains definition of the JavaClass class.
// This class is intended to contain all information about single java class:
// i.e constant pool, methods, fields and so on. Essentially it represents
// parsed class file.
//

#ifndef ICP_JAVACLASS_H
#define ICP_JAVACLASS_H

#include "ConstantPool.h"
#include "ConstantPoolRecords.h"
#include "JavaMethod.h"
#include "JavaField.h"

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

  // Allow using AccessFlags as a bitfield.
  friend constexpr AccessFlags operator|(AccessFlags Lhs, AccessFlags Rhs) {
    return static_cast<AccessFlags>(
        static_cast<uint16_t>(Lhs) | static_cast<uint16_t>(Rhs));
  }

  // This structure is intended to simplify passing constructor arguments.
  // It doesn't have any additional semantic meaning.
  struct ClassParameters {
    const ConstantPoolRecords::ClassInfo *ClassName = nullptr;
    const ConstantPoolRecords::ClassInfo *SuperClass = nullptr;

    AccessFlags Flags = AccessFlags::ACC_NONE;

    std::unique_ptr<ConstantPool> CP = nullptr;

    std::vector<std::unique_ptr<JavaMethod>> Methods;

    std::vector<JavaField> Fields;
  };

public:
  // Rvalue parameter is to emphasize the fact that we will transfer
  // ownership of the unique_ptr's (i.e ConstantPool), which makes
  // ClassParameters structure invalid after this constructor was executed.
  explicit JavaClass(ClassParameters &&Params);

  // No copies
  JavaClass(const JavaClass&) = delete;
  JavaClass &operator=(const JavaClass &) = delete;

  AccessFlags getAccessFlags() const {
    return Flags;
  }

  const ConstantPool &getConstantPool() const {
    return *CP;
  }

  const Utf8String &getClassName() const {
    assert(ClassName != nullptr);
    return ClassName->getName();
  }

  // Checks if this class has a super class
  bool hasSuper() const {
    return SuperClass != nullptr;
  }

  const std::vector<std::unique_ptr<JavaMethod>> &methods() const {
    return Methods;
  }

  const std::vector<JavaField> &fields() const { return Fields; }

  // Finds method by name or return null if nothing found
  // It's a trivial getter. Fll resolutin logic is located inside ClassObject
  // and InstanceObject.
  const JavaMethod *getMethod(const Utf8String &Name) const;

  // Only valid to call when there is super class
  const Utf8String &getSuperClassName() const {
    assert(hasSuper());
    return SuperClass->getName();
  }

  void print(std::ostream &Out) const;

private:
  const ConstantPoolRecords::ClassInfo *const ClassName;
  // Null when no super class is present
  const ConstantPoolRecords::ClassInfo *const SuperClass;

  const AccessFlags Flags;

  const std::unique_ptr<ConstantPool> CP;

  const std::vector<std::unique_ptr<JavaMethod>> Methods;

  std::vector<JavaField> Fields;
};

}

#endif //ICP_JAVACLASS_H
