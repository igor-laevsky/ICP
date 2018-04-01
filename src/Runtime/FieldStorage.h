///
/// Utility class to common out field handling for ClassObject and InstanceObject.
///

#ifndef ICP_FIELDSTORAGE_H
#define ICP_FIELDSTORAGE_H

#include "Runtime/Value.h"
#include "JavaTypes/JavaTypesFwd.h"
#include "Utils/Utf8String.h"

namespace Runtime {

class UnrecognizedField: public std::exception { };

class FieldStorage {
public:
  FieldStorage(const JavaTypes::JavaClass &Class, bool is_static);

  /// \throws UnrecognizedField If no field was found.
  Value getField(const Utf8String &Name) const;

  /// \throws UnrecognizedField If no field was found.
  void setField(const Utf8String &Name, const Value &V);

  /// \throws UnrecognizedField If no field was found.
  std::pair<const JavaTypes::JavaField*, std::size_t>
  findFieldAndOffset(const Utf8String &Name) const;

private:
  bool shouldManage(const JavaTypes::JavaField &F) const;

  const JavaTypes::JavaClass &Class;
  enum FeildsKind {
    STATIC, INSTANCE
  } Kind;
  std::vector<uint8_t> Fields;

};

}

#endif //ICP_FIELDSTORAGE_H
