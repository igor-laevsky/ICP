//
// Main interface for the class file loading
//

#ifndef IJVM_CLASSFILEREADER_H
#define IJVM_CLASSFILEREADER_H

#include <memory>
#include <string>

#include "JavaTypes/JavaClass.h"

namespace ClassFileReader {

// Exception thrown by class file parser.
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

#endif //IJVM_CLASSFILEREADER_H
