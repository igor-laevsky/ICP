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
#include "JavaTypes/JavaMethod.h"
#include "Bytecode/Bytecode.h"

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

// Helper function for the class file reader.
// Checks that 'Idx' is a valid index to the constant pool, checks
// that record at that index has type 'RecordType' and returns it. Throws
// exception otherwise.
// \returns Constant pool record.
// \throws ReadError or FormatError accordingly.
template<class RecordType>
static const RecordType &readConstantPoolRecord(
    uint16_t Idx, const ConstantPool &CP, const std::string &FieldName) {
  if (!CP.isValidIndex(Idx))
    throw FormatError("Invalid constant pool index for the " + FieldName +
                      " " + std::to_string(Idx));

  const auto *Res = CP.getAsOrNull<RecordType>(Idx);
  if (Res == nullptr)
    throw FormatError("Unexpected record type at " + FieldName +
                      " index " + std::to_string(Idx));

  return *Res;
}

// Same as above but reads 2 byte index from the input stream.
template<class RecordType>
static const RecordType &readConstantPoolRecord(
    std::istream &Input, const ConstantPool &CP, const std::string &FieldName) {
  uint16_t Idx = BigEndianReading::readHalf(Input);

  return readConstantPoolRecord<RecordType>(Idx, CP, FieldName);
}

// Creates and parses constant pool from the class file.
// \returns Unique pointer to the valid constant pool.
// \throws FormatError or ReadError is case of any input problems.
static std::unique_ptr<ConstantPool>
parseConstantPool(std::istream& Input) {
  const uint16_t constant_pool_count = BigEndianReading::readHalf(Input);
  // I guess extra one is added to account for the weird representation of
  // the long numbers.
  const uint16_t ConstantPoolSize =
      constant_pool_count - static_cast<uint16_t>(1);

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

        const auto &NameRef = Builder.getCellReference<Utf8>(name_index);
        Builder.set(i, std::make_unique<ConstantPoolRecords::ClassInfo>(NameRef));
        break;
      }

      case ConstantPoolTags::CONSTANT_Methodref: {
        uint16_t class_index = BigEndianReading::readHalf(Input);
        CheckIndex(class_index, i);

        uint16_t name_and_type_index = BigEndianReading::readHalf(Input);
        CheckIndex(name_and_type_index, i);

        const auto &ClassRef =
            Builder.getCellReference<ConstantPoolRecords::ClassInfo>(class_index);
        const auto &NameAndTypeRef =
            Builder.getCellReference<ConstantPoolRecords::NameAndType>(name_and_type_index);
        Builder.set(i,
                    std::make_unique<ConstantPoolRecords::MethodRef>(
                        ClassRef, NameAndTypeRef));
        break;
      }

      case ConstantPoolTags::CONSTANT_Fieldref: {
        uint16_t class_index = BigEndianReading::readHalf(Input);
        CheckIndex(class_index, i);

        uint16_t name_and_type_index = BigEndianReading::readHalf(Input);
        CheckIndex(name_and_type_index, i);

        const auto &ClassRef =
            Builder.getCellReference<ConstantPoolRecords::ClassInfo>(class_index);
        const auto &NameAndTypeRef =
            Builder.getCellReference<ConstantPoolRecords::NameAndType>(name_and_type_index);
        Builder.set(i,
                    std::make_unique<ConstantPoolRecords::FieldRef>(
                        ClassRef, NameAndTypeRef));
        break;
      }

      case ConstantPoolTags::CONSTANT_Utf8: {
        uint16_t length = BigEndianReading::readHalf(Input);
        std::string name;

        for (uint16_t byte_idx = 0; byte_idx < length; ++byte_idx) {
          uint8_t byte = BigEndianReading::readByte(Input);

          // Specification requirements
          if (byte == 0 || byte >= 0xf0)
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

        const auto &NameRef =
            Builder.getCellReference<ConstantPoolRecords::Utf8>(name_index);
        const auto &DescriptorRef =
            Builder.getCellReference<ConstantPoolRecords::Utf8>(descriptor_index);
        Builder.set(i,
                    std::make_unique<ConstantPoolRecords::NameAndType>(
                        NameRef, DescriptorRef));

        break;
      }

      default:
        throw FormatError("Unsupported constant pool tag " +
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

// Skips single attribute.
// \throws FormatError or ReadError in case of any errors.
static void skipAttribute(std::istream &Input) {
  BigEndianReading::readHalf(Input); // name_index
  const uint32_t attribute_length = BigEndianReading::readWord(Input);

  Input.seekg(attribute_length, std::ios_base::cur);

  if (Input.fail())
    throw FormatError("Unable to skip attribute");
}

// Skips attributes until attribute with specific name is found.
// Input position is set right after the name of the target attribute.
// \returns Number of skipped attributes.
// \throws FormatError in case if end of file was reached before target attribute.
// \throws ReadError In case of a reading error.
static uint16_t skipAttributesUntil(
    const std::string &TargetAttrName, std::istream &Input,
    const ConstantPool &CP) {

  uint16_t NumSkipped = 0;

  while (Input) {
    const auto &Name =
        readConstantPoolRecord<ConstantPoolRecords::Utf8>(
            Input, CP, "attribute_name");
    if (Name.getValue() == TargetAttrName)
      return NumSkipped;

    const uint32_t attribute_length = BigEndianReading::readWord(Input);
    Input.seekg(attribute_length, std::ios_base::cur);

    ++NumSkipped;
  }

  // Reached the end of the file and no attribute was found
  throw FormatError("Unable to find attribute " + TargetAttrName);
}

// Parses stack map table and saves it into the 'Params' structure.
// \throws ReadError or FormatError.
//static void parseStackMapTable(
//    JavaMethod::MethodConstructorParameters &Params,
//    std::istream &Input) {
//
//
//
//}

// Expects current input position to be set up right after 'Code' attribute
// tag. Saves result in the 'Params' structure.
// \throws ReadError or FormatError.
static void parseMethodCode(
    JavaMethod::MethodConstructorParameters &Params,
    std::istream &Input) {
  BigEndianReading::readWord(Input); // attribute_length

  Params.MaxStack = BigEndianReading::readHalf(Input);
  Params.MaxLocals = BigEndianReading::readHalf(Input);

  std::vector<uint8_t> Bytecode;
  const uint32_t code_length = BigEndianReading::readWord(Input);
  Bytecode.resize(code_length);
  Input.read(reinterpret_cast<char*>(Bytecode.data()), code_length);
  if (Input.fail())
    throw FormatError("Failed to read method code");
  Params.Code = Bytecode::parseInstructions(Bytecode);

  // Skip exception table for now
  const uint16_t exception_table_length = BigEndianReading::readHalf(Input);
  const uint16_t exception_table_entry_size = 8;
  Input.seekg(exception_table_length * exception_table_entry_size,
              std::ios_base::cur);
  if (Input.fail())
    throw FormatError("Failed to read exception table from code attribute");

  // Skip all attributes for now
  const uint16_t attributes_count = BigEndianReading::readHalf(Input);
  for (uint16_t i = 0; i < attributes_count; ++i)
    skipAttribute(Input);

  // TODO: Check that attribute_length is consistent with the actual
  // amount of data.
}

// Parse single field description.
// \returns JavaField by value with the hope that it's going to be moved to
// the fields vector.
// \throws FormatError or ReadError accordingly.
static JavaField parseField(
    std::istream &Input, const ConstantPool &CP) {
  const uint16_t access_flags = BigEndianReading::readHalf(Input);
  auto Flags = static_cast<JavaField::AccessFlags>(access_flags);
  // This doesn't follow JVM specification but this is what javac
  // generates on practice.
  if (Flags == JavaField::ACC_NONE)
    Flags = JavaField::ACC_PUBLIC;

  const auto &Name =
      readConstantPoolRecord<ConstantPoolRecords::Utf8>(
          Input, CP, "field_name_index");
  const auto &Descriptor =
      readConstantPoolRecord<ConstantPoolRecords::Utf8>(
          Input, CP, "field_descriptor_index");

  // Skip all the attributes for now
  const uint16_t attributes_count = BigEndianReading::readHalf(Input);
  for (uint16_t idx = 0; idx < attributes_count; ++idx)
    skipAttribute(Input);

  return JavaField(Descriptor, Name, Flags);
}

// Helper function for the class file parser. Parses single method.
// \returns Structure with all method parameters. Method is not constructed
// inside of this function in order to avoid copying. Instead user is expected
// to call constructor himself.
// \throws FormatError or ReadError accordingly.
static std::unique_ptr<JavaMethod> parseMethod(
    std::istream &Input, const ConstantPool &CP) {
  JavaMethod::MethodConstructorParameters Params;

  const uint16_t access_flags = BigEndianReading::readHalf(Input);
  Params.Flags = static_cast<JavaMethod::AccessFlags>(access_flags);

  const auto &Name =
      readConstantPoolRecord<ConstantPoolRecords::Utf8>(
          Input, CP, "method_name_index");
  Params.Name = &Name;

  const auto &Descriptor =
      readConstantPoolRecord<ConstantPoolRecords::Utf8>(
          Input, CP, "method_descriptor_index");
  Params.Descriptor = &Descriptor;

  uint16_t attributes_count = BigEndianReading::readHalf(Input);

  // Search for code
  attributes_count -= skipAttributesUntil("Code", Input, CP);

  // Parse method code
  parseMethodCode(Params, Input);
  attributes_count -= 1;

  // Skip the rest of the attributes
  for (uint16_t i = 0; i < attributes_count; ++i)
    skipAttribute(Input);

  return std::make_unique<JavaMethod>(std::move(Params));
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
    const uint32_t magic = BigEndianReading::readWord(Input);
    if (magic != 0xCAFEBABE)
      throw FormatError("Magic word in a wrong format");
  } catch (ReadError &) {
    throw FormatError("Unable to read magic word");
  }

  try {
    const uint16_t minor_version = BigEndianReading::readHalf(Input);
    const uint16_t major_version = BigEndianReading::readHalf(Input);
    if (major_version != 52 || minor_version != 0)
      throw FormatError("Unsupported class file version");
  } catch (ReadError &) {
    throw FormatError("Unable to read class file version");
  }

  // Parse constant pool
  //
  try {
    ClassParams.CP = parseConstantPool(Input);
  } catch (ReadError &) {
    throw FormatError("Unable to fully read constant pull");
  }
  assert(ClassParams.CP != nullptr);

  // Access flags
  //
  try {
    const uint16_t access_flags = BigEndianReading::readHalf(Input);
    ClassParams.Flags = static_cast<JavaClass::AccessFlags>(access_flags);
  } catch (ReadError &) {
    throw FormatError("Unable to read access flags");
  }

  // This class and super class
  //
  try {
    const auto &ThisClass =
        readConstantPoolRecord<ConstantPoolRecords::ClassInfo>(
            Input, *ClassParams.CP, "this_class");
    ClassParams.ClassName = &ThisClass;

    const uint16_t super_class = BigEndianReading::readHalf(Input);
    if (super_class != 0) {
      const auto &SuperClass =
          readConstantPoolRecord<ConstantPoolRecords::ClassInfo>(
              super_class, *ClassParams.CP, "super_class");
      ClassParams.SuperClass = &SuperClass;
    }
  } catch (ReadError &) {
    throw FormatError("Unable to read this or super class indexes");
  }

  // Interfaces
  //
  try {
    const uint16_t interfaces_count = BigEndianReading::readHalf(Input);
    if (interfaces_count != 0)
      throw FormatError("Interface inheritance is not supported yet");
  } catch (ReadError &) {
    throw FormatError("Can't read interface_count");
  }

  // Fields
  //
  try {
    const uint16_t fields_count = BigEndianReading::readHalf(Input);

    ClassParams.Fields.reserve(fields_count);
    for (uint16_t idx = 0; idx < fields_count; ++idx) {
      ClassParams.Fields.push_back(parseField(Input, *ClassParams.CP));
    }
  } catch (ReadError &) {
    throw FormatError("Can't read fields_count");
  }

  // Methods
  //
  try {
    const uint16_t methods_count = BigEndianReading::readHalf(Input);

    ClassParams.Methods.reserve(methods_count);
    for (uint16_t method_idx = 0; method_idx < methods_count; ++method_idx)
      ClassParams.Methods.push_back(parseMethod(Input, *ClassParams.CP));
  } catch (ReadError &) {
    throw FormatError("Unable to read class methods");
  } catch (Bytecode::BytecodeParsingError &) {
    throw FormatError("Unable to parse method's bytecode");
  } catch (Bytecode::UnknownBytecode &) {
    throw FormatError("Encountered unknown bytecode");
  }

  return std::make_unique<JavaClass>(std::move(ClassParams));
}
