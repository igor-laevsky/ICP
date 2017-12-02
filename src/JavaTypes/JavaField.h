///
/// JavaField class describes field of the underlying java class.
/// Note that this is only a description, actual values should be handled
/// via other means.
///

#ifndef ICP_JAVAFIELD_H
#define ICP_JAVAFIELD_H

#include "Type.h"
#include "ConstantPool.h"
#include "ConstantPoolRecords.h"

namespace JavaTypes {

class JavaField final {
public:
  enum class AccessFlags: uint16_t {
    ACC_PUBLIC = 0x0001,
    ACC_PRIVATE = 0x0002,
    ACC_PROTECTED = 0x0004,
    ACC_STATIC = 0x0008,
    ACC_FINAL = 0x0010,
    ACC_VOLATILE = 0x0040,
    ACC_TRANSIENT = 0x0080,
    ACC_SYNTHETIC = 0x1000,
    ACC_ENUM = 0x4000,

    ACC_PUBLIC_STATIC = ACC_PUBLIC | ACC_STATIC
  };

  // Allows using AccessFlags as a bitfield.
  friend constexpr AccessFlags operator|(AccessFlags Lhs, AccessFlags Rhs) {
    return static_cast<AccessFlags>(
        static_cast<uint16_t>(Lhs) | static_cast<uint16_t>(Rhs));
  }

public:
  /// \throws Type::ParsingError if unable to parse field descriptor
  JavaField(
      const ConstantPoolRecords::Utf8 &Descr,
      const ConstantPoolRecords::Utf8 &Name,
      AccessFlags Flags);

  // This is expensive to copy so fobid it completely
  JavaField(const JavaField &) = delete;
  JavaField &operator=(const JavaField &) = delete;
  
  const std::string &getName() const;
  const std::string &getDescriptor() const;

  Type getType() const;

  // Return size of this fields in bytes
  std::size_t getSize() const;

private:
  const ConstantPoolRecords::Utf8 &Name;
  const ConstantPoolRecords::Utf8 &Descr;
  AccessFlags Flags;

  Type T = Types::Void;
};

}

#endif //ICP_JAVAFIELD_H
