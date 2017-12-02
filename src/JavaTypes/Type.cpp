//
// Intended to be empty. Checks successful compilation of the related header.
//

#include "Type.h"

#include <vector>

using namespace JavaTypes;

Type Type::parseFieldDescriptor(
    const std::string &Desc, std::size_t *LastPos) {

  if (Desc.empty())
    throw ParsingError("Field descriptor is empty");

  if (LastPos != nullptr)
    *LastPos = 1;

  switch (Desc[0]) {
    case 'B': return Types::Byte;
    case 'C': return Types::Char;
    case 'D': return Types::Double;
    case 'F': return Types::Float;
    case 'I': return Types::Int;
    case 'J': return Types::Long;
    case 'S': return Types::Short;
    case 'Z': return Types::Boolean;

    case 'L': {
      auto End = Desc.find(';');
      if (End == std::string::npos)
        throw ParsingError("Reference type in a wrong format");

      if (LastPos != nullptr)
        *LastPos = End + 1;
      return Types::Class;
    }

    case '[': {
      try {
        (void)parseFieldDescriptor(Desc.substr(1), LastPos);
        if (LastPos != nullptr)
          *LastPos += 1;
      } catch (std::exception &) {
        // This will also catch cases when substr(1) had throw and exception
        throw ParsingError("Array type in a wrong format");
      }
      return Types::Array;
    }

    default:
      throw ParsingError("Unrecognized field descriptor");
  }
}

std::pair<Type, std::vector<Type>>
Type::parseMethodDescriptor(const std::string &Desc) {
  if (Desc.empty())
    throw ParsingError("Empty descriptor");

  if (Desc[0] != '(')
    throw ParsingError("Expected to find l-brace");

  auto RBracePos = Desc.find(')');
  if (RBracePos == std::string::npos)
    throw ParsingError("Expected to find r-brace");
  if (RBracePos == Desc.size() - 1)
    throw ParsingError("Expected to find return type descriptor");

  std::string ArgsDesc = Desc.substr(1, RBracePos - 1);
  std::string RetDesc = Desc.substr(RBracePos + 1);
  assert(!RetDesc.empty());

  // Parse argument types
  std::vector<Type> ArgTypes;
  std::size_t ArgsPos = 0;
  while (ArgsPos < ArgsDesc.length()) {
    std::size_t ArgEnd;
    auto NewType = parseFieldDescriptor(ArgsDesc.substr(ArgsPos), &ArgEnd);

    ArgTypes.push_back(NewType);
    ArgsPos += ArgEnd;
  }

  // Parse return type
  auto RetType = Types::Void;
  if (RetDesc != "V") {
    std::size_t RetEnd;
    RetType = parseFieldDescriptor(RetDesc, &RetEnd);
    if (RetEnd != RetDesc.length())
      throw ParsingError("Can't parse tail of the descriptor");
  }

  return std::pair(RetType, ArgTypes);
}
