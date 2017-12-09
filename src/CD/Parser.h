///
/// Parser for the class file description language. This is a DSL which
/// is intended as a textual representation of the java class files.
/// It's primary purpose is testing.
///

#ifndef ICP_PARSER_H
#define ICP_PARSER_H

#include <stdexcept>
#include <istream>
#include <memory>

#include "JavaTypes/JavaTypesFwd.h"
#include "JavaTypes/JavaClass.h"

namespace CD {

class FileNotFound: public std::exception {};
class ParserError: public std::runtime_error {
  using std::runtime_error::runtime_error;
};

/// Parse input in the form of the class description language and create java
/// class. Throws exception on error. Never returns null.
/// Following three functions exhibit same behaviour. They can be overloads but
/// have different names for the sake of clarity.
///
/// \return Freshly created java class which is never null.
/// \throws ParserError, LexerError, FileNotFound or other standard library
///         exception
std::unique_ptr<JavaTypes::JavaClass> parseFromFile(
    const std::string &FileName);

std::unique_ptr<JavaTypes::JavaClass> parseFromString(
    const std::string &Input);

std::unique_ptr<JavaTypes::JavaClass> parseFromStream(
    std::istream &InputStream);

}

#endif //ICP_PARSER_H
