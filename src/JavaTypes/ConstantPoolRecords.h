//
// Contains definitions of the various constant pool records.
//

#ifndef IJVM_CONSTANTPOOLRECORDS_H
#define IJVM_CONSTANTPOOLRECORDS_H

#include <string>

#include "ConstantPool.h"

namespace JavaTypes {
namespace ConstantPoolRecords {

class Utf8 final: public Record {
public:
  // TODO: This should support unicode
  explicit Utf8(const std::string &NewValue):
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

class NameAndType final: public Record {
public:
  NameAndType(ConstantPool::CellReference NewNameRef,
              ConstantPool::CellReference NewDescriptorRef):
      NameRef(NewNameRef), DescriptorRef(NewDescriptorRef) {
    ;
  }

  const std::string &getName() const {
    assert(isValid());
    return static_cast<Utf8*>(NameRef.get())->getValue();
  }

  const std::string &getDescriptor() const {
    assert(isValid());
    return static_cast<Utf8*>(DescriptorRef.get())->getValue();
  }

  bool isValid() const override {
    // This should also check that name and descriptor are presented in
    // a correct form. However I decided to leave this check for now.
    return NameRef != nullptr && dynamic_cast<Utf8*>(NameRef.get()) &&
      DescriptorRef != nullptr && dynamic_cast<Utf8*>(DescriptorRef.get());
  }

private:
  const ConstantPool::CellReference NameRef, DescriptorRef;
};

class ClassInfo final: public Record {
public:
  explicit ClassInfo(ConstantPool::CellReference NewName):
      Name(NewName) {
    ;
  }

  const std::string &getName() const {
    assert(isValid());
    return static_cast<Utf8*>(Name.get())->getValue();
  }

  bool isValid() const override {
    return Name != nullptr && dynamic_cast<Utf8*>(Name.get());
  }

private:
  const ConstantPool::CellReference Name;
};

class MethodRef final: public Record {
public:
  MethodRef(ConstantPool::CellReference NewClassRef,
            ConstantPool::CellReference NewNameAndTypeRef):
      ClassRef(NewClassRef), NameAndTypeRef(NewNameAndTypeRef) {
    ;
  }

  bool isValid() const override {
    // This should also check that class is a normal class and that
    // name and type points to a method, not a field.
    return ClassRef != nullptr && dynamic_cast<ClassInfo*>(ClassRef.get()) &&
           NameAndTypeRef != nullptr && dynamic_cast<NameAndType*>(NameAndTypeRef.get());
  }

private:
  const ConstantPool::CellReference ClassRef, NameAndTypeRef;
};

}
}

#endif //IJVM_CONSTANTPOOLRECORDS_H
