//
// Implementation of the java class file parser
//

#include <istream>
#include <fstream>
#include <sstream>

#include "ClassFileReader.h"
#include "Utils/BinaryFiles.h"

using namespace ClassFileReader;
using namespace Utils;

std::unique_ptr<JavaTypes::JavaClass> ClassFileReader::loadClassFromFile(
    const std::string &FileName) {
  std::ifstream file(FileName, std::ios_base::in | std::ios_base::binary);

  if (!file)
    throw FileNotFound();

  return loadClassFromStream(file);
}

std::unique_ptr<JavaTypes::JavaClass> ClassFileReader::loadClassFromStream(
    std::istream& Input) {
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

  uint16_t constant_pool_count = 0;
  try {
    constant_pool_count = BigEndianReading::readHalf(Input);
  } catch (ReadError &) {
    throw FormatError("Unable to read constant pool count");
  }

  (void)constant_pool_count;
  return nullptr;
}
