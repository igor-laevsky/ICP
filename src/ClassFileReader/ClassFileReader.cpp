//
// Implementation of the java class file parser
//

#include <istream>
#include <iostream>
#include <fstream>

#include "ClassFileReader.h"
#include "Utils/BinaryFiles.h"
#include "JavaTypes/ConstantPool.h"
#include "JavaTypes/ConstantPoolRecords.h"

using namespace ClassFileReader;
using namespace Utils;
using namespace JavaTypes;
using namespace JavaTypes::ConstantPoolRecords;

namespace {

enum class ConstantPoolTags {
  CONSTANT_Class = 7,
  CONSTANT_Fieldref = 9,
  CONSTANT_Methodref = 10,
  CONSTANT_InterfaceMethodref = 11,
  CONSTANT_String8 = 8,
  CONSTANT_Integer = 3,
  CONSTANT_Float = 4,
  CONSTANT_Long = 5,
  CONSTANT_Double = 6,
  CONSTANT_NameAndType = 12,
  CONSTANT_Utf8 = 1,
  CONSTANT_MethodHandle = 15,
  CONSTANT_MethodType = 16,
  CONSTANT_InvokeDynamic = 18
};

}

// Helper function for the class file reader. Parses 'ConstantPoolSize' records
// from the input stream.
// \returns Unique pointer to the valid constant pool.
// \throws FormatError or ReadError is case of any input problems.
static std::unique_ptr<ConstantPool>
parseConstantPool(std::istream& Input, ConstantPool::SizeType ConstantPoolSize) {
  ConstantPoolBuilder Builder(ConstantPoolSize);

  auto CheckIndex =
      [&](ConstantPool::IndexType ToCheck, ConstantPool::IndexType At) {
        if (!(ToCheck >= 1 && ToCheck <= ConstantPoolSize))
          throw FormatError(
              "Incorrect index " + std::to_string(ToCheck) +
              " found at " + std::to_string(At));
      };

  for (ConstantPool::IndexType i = 1; i <= ConstantPoolSize; ++i) {
    uint8_t tag = BigEndianReading::readByte(Input);

    switch (static_cast<ConstantPoolTags>(tag)) {
      case ConstantPoolTags::CONSTANT_Class: {
        uint16_t name_index = BigEndianReading::readHalf(Input);
        CheckIndex(name_index, i);

        ConstantPool::CellReference NameRef =
            Builder.getCellReference(name_index);
        Builder.set(i, std::make_unique<ConstantPoolRecords::ClassInfo>(NameRef));
        break;
      }

      case ConstantPoolTags::CONSTANT_Methodref: {
        uint16_t class_index = BigEndianReading::readHalf(Input);
        CheckIndex(class_index, i);

        uint16_t name_and_type_index = BigEndianReading::readHalf(Input);
        CheckIndex(name_and_type_index, i);

        ConstantPool::CellReference
            ClassRef = Builder.getCellReference(class_index),
            NameAndTypeRef = Builder.getCellReference(name_and_type_index);
        Builder.set(i,
                    std::make_unique<ConstantPoolRecords::MethodRef>(
                        ClassRef, NameAndTypeRef));
        break;
      }

      case ConstantPoolTags::CONSTANT_Utf8: {
        uint16_t length = BigEndianReading::readHalf(Input);
        std::string name;

        for (uint16_t byte_idx = 0; byte_idx < length; ++byte_idx) {
          uint8_t byte = BigEndianReading::readByte(Input);

          // Specification requirements
          if (byte == 0 || (byte >= 0xf0 && byte <= 0xff))
            throw FormatError("Unexpected string byte at " + std::to_string(i));

          // For now support only regular ASCII codes
          if (byte > 0x7f)
            throw FormatError("Unicode is not fully supported at " +
                                  std::to_string(i));

          name += static_cast<char>(byte);
        }

        Builder.set(i, std::make_unique<ConstantPoolRecords::Utf8>(name));
        break;
      }

      case ConstantPoolTags::CONSTANT_NameAndType: {
        uint16_t name_index = BigEndianReading::readHalf(Input);
        CheckIndex(name_index, i);

        uint16_t descriptor_index = BigEndianReading::readHalf(Input);
        CheckIndex(descriptor_index, i);

        ConstantPool::CellReference
            NameRef = Builder.getCellReference(name_index),
            DescriptorRef = Builder.getCellReference(descriptor_index);
        Builder.set(i,
                    std::make_unique<ConstantPoolRecords::NameAndType>(
                        NameRef, DescriptorRef));

        break;
      }

      default:
        throw FormatError("Unsupported constant pull tag " +
                              std::to_string(tag));
    }
  }

  std::unique_ptr<ConstantPool> CP = Builder.createConstantPool();
  std::string ErrorMessage;
  if (!CP->verify(ErrorMessage))
    throw FormatError(
        "Failed constant pool verification with message: " + ErrorMessage);
  return CP;
}

std::unique_ptr<JavaTypes::JavaClass> ClassFileReader::loadClassFromFile(
    const std::string &FileName) {
  std::ifstream file(FileName, std::ios_base::in | std::ios_base::binary);

  if (!file)
    throw FileNotFound();

  return loadClassFromStream(file);
}

std::unique_ptr<JavaTypes::JavaClass> ClassFileReader::loadClassFromStream(
    std::istream &Input) {
  JavaClass::ClassParameters ClassParams;

  // Read basic constants
  //
  try {
    uint32_t magic = BigEndianReading::readWord(Input);
    if (magic != 0xCAFEBABE)
      throw FormatError("Magic word in a wrong format");
  } catch (ReadError &) {
    throw FormatError("Unable to read magic word.");
  }

  try {
    uint16_t minor_version = BigEndianReading::readHalf(Input);
    uint16_t major_version = BigEndianReading::readHalf(Input);
    if (major_version != 52 || minor_version != 0)
      throw FormatError("Unsupported class file version");
  } catch (ReadError &) {
    throw FormatError("Unable to read class file version.");
  }

  // Parse constant pool
  //
  try {
    uint16_t constant_pool_count = 0;
    constant_pool_count = BigEndianReading::readHalf(Input);
    // I guess extra one is added to account for the weird representation of
    // the long numbers.
    constant_pool_count -= 1;

    ClassParams.CP = parseConstantPool(Input, constant_pool_count);
  } catch (ReadError &) {
    throw FormatError("Unable to fully read constant pull");
  }
  assert(ClassParams.CP != nullptr);

  // Access flags
  //
  try {
    uint16_t access_flags = BigEndianReading::readHalf(Input);
    ClassParams.Flags = static_cast<JavaClass::AccessFlags>(access_flags);
  } catch (ReadError &) {
    throw FormatError("Unable to read access flags");
  }

  // This class and super class
  //
  try {
    auto GetClassInfoFromIndex =
        [&](ConstantPool::IndexType Idx, const std::string &FieldName) {
          if (!ClassParams.CP->isValidIndex(Idx))
            throw FormatError("Invalid constant pool index for the " + FieldName);
          const auto *Res = ClassParams.CP->
              getAsOrNull<ConstantPoolRecords::ClassInfo>(Idx);
          if (Res == nullptr)
            throw FormatError("Expected ClassInfo at index " + FieldName);
          return Res;
        };

    uint16_t this_class = BigEndianReading::readHalf(Input);
    ClassParams.ClassName = GetClassInfoFromIndex(this_class, "this_class");
    assert(ClassParams.ClassName != nullptr);

    uint16_t super_class = BigEndianReading::readHalf(Input);
    if (super_class != 0) {
      const auto *SuperCI = GetClassInfoFromIndex(super_class, "super_class");
      assert(SuperCI != nullptr);
      ClassParams.SuperClass = SuperCI;
    }
  } catch (ReadError &) {
    throw FormatError("Unable to read this or super class indexes");
  }

  return std::make_unique<JavaClass>(std::move(ClassParams));
}
