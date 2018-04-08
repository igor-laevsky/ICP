///
/// Implementation of the java class file parser.
///

#include "ClassFileReader.h"

#include "Utils/BinaryFiles.h"
#include "JavaTypes/ConstantPool.h"
#include "JavaTypes/ConstantPoolRecords.h"
#include "JavaTypes/JavaMethod.h"
#include "Bytecode/Bytecode.h"

#include <istream>
#include <iostream>
#include <fstream>

using namespace std::string_literals;

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

// Parses single constant pool record and adds it to the Builder.
// \throws ConstantPoolBuilder::IncompatibleCellType
static void parseConstantPoolRecord(
    ConstantPoolBuilder &Builder, ConstantPool::IndexType CurIdx,
    std::istream& Input) {

  auto CheckIndex =
      [&](ConstantPool::IndexType ToCheck) {
        if (!Builder.isValidIndex(ToCheck))
          throw FormatError(
              "Incorrect index " + std::to_string(ToCheck) +
              " found at " + std::to_string(CurIdx));
      };

  uint8_t tag = BigEndianReading::readByte(Input);

  switch (static_cast<ConstantPoolTags>(tag)) {
    case ConstantPoolTags::CONSTANT_Class: {
      uint16_t name_index = BigEndianReading::readHalf(Input);
      CheckIndex(name_index);

      const auto &NameRef = Builder.getCellReference<Utf8>(name_index);
      Builder.create<ConstantPoolRecords::ClassInfo>(CurIdx, NameRef);
      break;
    }

    case ConstantPoolTags::CONSTANT_Methodref: {
      uint16_t class_index = BigEndianReading::readHalf(Input);
      CheckIndex(class_index);

      uint16_t name_and_type_index = BigEndianReading::readHalf(Input);
      CheckIndex(name_and_type_index);

      const auto &ClassRef =
          Builder.getCellReference<ConstantPoolRecords::ClassInfo>(class_index);
      const auto &NameAndTypeRef =
          Builder.getCellReference<ConstantPoolRecords::NameAndType>(name_and_type_index);
      Builder.create<ConstantPoolRecords::MethodRef>(
          CurIdx, ClassRef, NameAndTypeRef);
      break;
    }

    case ConstantPoolTags::CONSTANT_Fieldref: {
      uint16_t class_index = BigEndianReading::readHalf(Input);
      CheckIndex(class_index);

      uint16_t name_and_type_index = BigEndianReading::readHalf(Input);
      CheckIndex(name_and_type_index);

      const auto &ClassRef =
          Builder.getCellReference<ConstantPoolRecords::ClassInfo>(class_index);
      const auto &NameAndTypeRef =
          Builder.getCellReference<ConstantPoolRecords::NameAndType>(name_and_type_index);
      Builder.create<ConstantPoolRecords::FieldRef>(
          CurIdx, ClassRef, NameAndTypeRef);
      break;
    }

    case ConstantPoolTags::CONSTANT_Utf8: {
      uint16_t length = BigEndianReading::readHalf(Input);
      std::string name;

      for (uint16_t byte_idx = 0; byte_idx < length; ++byte_idx) {
        uint8_t byte = BigEndianReading::readByte(Input);

        // Specification requirements
        if (byte == 0 || byte >= 0xf0)
          throw FormatError("Unexpected string byte at " + std::to_string(CurIdx));

        // For now support only regular ASCII codes
        if (byte > 0x7f)
          throw FormatError("Unicode is not fully supported at " +
                                std::to_string(CurIdx));

        name += static_cast<char>(byte);
      }

      Builder.create<ConstantPoolRecords::Utf8>(CurIdx, name);
      break;
    }

    case ConstantPoolTags::CONSTANT_NameAndType: {
      uint16_t name_index = BigEndianReading::readHalf(Input);
      CheckIndex(name_index);

      uint16_t descriptor_index = BigEndianReading::readHalf(Input);
      CheckIndex(descriptor_index);

      const auto &NameRef =
          Builder.getCellReference<ConstantPoolRecords::Utf8>(name_index);
      const auto &DescriptorRef =
          Builder.getCellReference<ConstantPoolRecords::Utf8>(descriptor_index);
      Builder.create<ConstantPoolRecords::NameAndType>(
          CurIdx, NameRef, DescriptorRef);
      break;
    }

    default:
      throw FormatError("Unsupported constant pool tag " +
                            std::to_string(tag));
  }
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

  try {
    for (ConstantPool::IndexType i = 1; i <= ConstantPoolSize; ++i)
      parseConstantPoolRecord(Builder, i, Input);
  } catch (const ConstantPoolBuilder::IncompatibleCellType &e) {
    throw FormatError("Constant pool parsing error: "s + e.what());
  }

  return Builder.createConstantPool();
}

namespace {

// Reads class file attributes. Throws ReadError on errors.
class AttributeIterator {
public:
  explicit AttributeIterator(const ConstantPool &CP, std::istream &Input):
      CP(CP), Input(Input) {
    NumAttrs = BigEndianReading::readHalf(Input);

    // Read first attribute if possible
    if (!empty())
      readAttr();
  }

  // Return true if all attributes were parsed.
  bool empty() const { return NumAttrs == 0; }

  // Get name of the current attribute
  const Utf8String &getName() const { assert(CurName); return *CurName; };

  // Assuming current input position is at the beginning of the attribute,
  // read information about this attribute. Fails assertion if no attributes
  // were left.
  void next() {
    assert(!empty()); // too much attributes

    --NumAttrs;
    if (!empty())
      readAttr();
  }

  // Skips this attribute base on it's size. User should explicitly call 'next'
  // if he want's to visit next attribute.
  void skip() {
    assert(!empty());
    Input.seekg(CurSize, std::ios_base::cur);
  }

private:
  // Read attribute information, but don't advance attribute count.
  void readAttr() {
    assert(!empty());

    const auto &NameRec =
        readConstantPoolRecord<ConstantPoolRecords::Utf8>(
            Input, CP, "attribute_name");
    CurName = &NameRec.getValue();
    CurSize = BigEndianReading::readWord(Input);
  }

private:
  const ConstantPool &CP;
  std::istream &Input;
  uint16_t NumAttrs = 0;

  const Utf8String *CurName = nullptr;
  uint32_t CurSize = 0;
};

}

static Type parseVerificationTypeInfo(std::istream &Input) {
  const uint8_t tag = BigEndianReading::readByte(Input);

  switch (tag) {
  case 0: return Types::Top;
  case 1: return Types::Int;
  case 2: return Types::Float;
  case 5: return Types::Null;
  case 6: return Types::UninitializedThis;
  case 7: {
    (void)BigEndianReading::readHalf(Input);
    return Types::Class;
  }
  case 8: {
    const uint16_t offset = BigEndianReading::readHalf(Input);
    return Types::UninitializedOffset(offset);
  }
  case 4: return Types::Long;
  case 3: return Types::Double;
  default: throw FormatError("Unrecognized verification type tag");
  }

  assert(false); // never happens
  return Types::Void;
}

// Parses stack map table and saves it into the 'Params' structure.
// \throws ReadError or FormatError.
static StackMapTableBuilder parseStackMapTable(std::istream &Input) {
  const uint16_t number_of_entries = BigEndianReading::readHalf(Input);

  StackMapTableBuilder Ret;
  // Specification is terribly thoughtful
  auto cur_bci = static_cast<Bytecode::BciType>(-1);

  for (int i = 0; i < number_of_entries; ++i) {
    const uint8_t frame_type = BigEndianReading::readByte(Input);

    if (frame_type <= 63) {
      cur_bci += frame_type + 1;
      Ret.addSame(cur_bci);

    } else if (frame_type >= 252 && frame_type <= 254) {
      const uint16_t offset_delta = BigEndianReading::readHalf(Input);
      const uint8_t k = frame_type - 251;

      std::vector<Type> new_locals;
      new_locals.reserve(k);
      for (uint8_t local_idx = 0; local_idx < k; ++local_idx) {
        new_locals.push_back(parseVerificationTypeInfo(Input));
      }

      cur_bci += offset_delta + 1;
      Ret.addAppend(cur_bci, std::move(new_locals));

    } else {
      throw FormatError("Unrecognized stack map frame type");
    }
  }

  return Ret;
}

// Expects current input position to be set up right after 'Code' attribute
// tag. Saves result in the 'Params' structure.
// \throws ReadError or FormatError.
static void parseMethodCode(
    JavaMethod::MethodConstructorParameters &Params,
    const ConstantPool &CP, std::istream &Input) {

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

  AttributeIterator AttrIt(CP, Input);
  for (; !AttrIt.empty(); AttrIt.next()) {
    if (AttrIt.getName() == "StackMapTable") {
      Params.StackMapBuilder = parseStackMapTable(Input);
    } else {
      AttrIt.skip();
    }
  }

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
  AttributeIterator AttrIt(CP, Input);
  for (; !AttrIt.empty(); AttrIt.next())
    AttrIt.skip();

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

  bool seen_code = false;

  AttributeIterator AttrIt(CP, Input);
  for (; !AttrIt.empty(); AttrIt.next()) {
    if (AttrIt.getName() == "Code") {
      parseMethodCode(Params, CP, Input);
      seen_code = true;
    } else {
      AttrIt.skip();
    }
  }

  if (!seen_code)
    throw FormatError("Couldn't find method code attribute");

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
  } catch (Bytecode::BytecodeParsingError &e) {
    throw FormatError("Unable to parse method's bytecode: "s + e.what());
  } catch (Bytecode::UnknownBytecode &e) {
    throw FormatError("Encountered unknown bytecode: "s + e.what());
  }

  return std::make_unique<JavaClass>(std::move(ClassParams));
}
