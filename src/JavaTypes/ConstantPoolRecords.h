//
// Contains definitions of the various constant pool records.
//

#ifndef IJVM_CONSTANTPOOLRECORDS_H
#define IJVM_CONSTANTPOOLRECORDS_H

#include <string>

#include "ConstantPool.h"

namespace JavaTypes {
namespace ConstantPoolRecords {

class StringRecord final: public Record {
public:
  // TODO: This should support unicode
  explicit StringRecord(const std::string &&NewValue):
      Value(NewValue) {
    ;
  }

  const std::string &getValue() const {
    return Value;
  }

  bool isValid() const override {
    return true;
  }

private:
  const std::string Value;
};

class MethodRef final: public Record {
public:
  MethodRef(ConstantPool::CellReference NewClassRef,
            ConstantPool::CellReference NewNameAndTypeRef):
      ClassRef(NewClassRef), NameAndTypeRef(NewNameAndTypeRef) {
    ;
  }

  bool isValid() const override {
    // TODO: Fix this
    return true;
  }

private:
  const ConstantPool::CellReference ClassRef;
  const ConstantPool::CellReference NameAndTypeRef;
};

class ClassInfo final: public Record {
public:
  explicit ClassInfo(ConstantPool::CellReference NewName):
      Name(NewName) {
    ;
  }

  const std::string &getName() const {
    assert(isValid());
    return static_cast<StringRecord*>(Name.get())->getValue();
  }

  bool isValid() const override {
    return Name != nullptr && dynamic_cast<StringRecord*>(Name.get());
  }

private:
  const ConstantPool::CellReference Name;
};

}
}

#endif //IJVM_CONSTANTPOOLRECORDS_H
