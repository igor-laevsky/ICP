///
/// Main interface for the class file loading
///

#ifndef ICP_CLASSFILEREADER_H
#define ICP_CLASSFILEREADER_H

#include "JavaTypes/JavaClass.h"

#include <memory>
#include <string>

namespace ClassFileReader {

// Exceptions thrown by the class file parser.
class FileNotFound: public std::exception {};
class FormatError: public std::runtime_error {
  using std::runtime_error::runtime_error;
};

// Parses class from file or throws an exception.
// \returns Unique pointer for the parsed class file
// \throws FileNotFound In case of file open error.
// \throws FormatError In case of any parsing problems.
std::unique_ptr<JavaTypes::JavaClass> loadClassFromFile(
    const std::string &FileName);

// Same as loadClassFromFile, but gathers information from the input stream.
// \returns Unique pointer for the parsed class file.
// \throws FormatError In case of any parsing problems.
std::unique_ptr<JavaTypes::JavaClass> loadClassFromStream(std::istream &Input);

}

#endif //ICP_CLASSFILEREADER_H
