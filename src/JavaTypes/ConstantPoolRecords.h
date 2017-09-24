//
// Contains definitions of the various constant pool records.
//

#ifndef IJVM_CONSTANTPOOLRECORDS_H
#define IJVM_CONSTANTPOOLRECORDS_H

#include "ConstantPool.h"

#include <string>

#include "Utils/Utf8String.h"

using namespace Utils;

namespace JavaTypes {
namespace ConstantPoolRecords {

class Utf8 final: public Record {
public:
  explicit Utf8(Utf8String NewValue):
      Value(std::move(NewValue)) {
    ;
  }

  const Utf8String &getValue() const {
    return Value;
  }

  bool isValid() const override {
    return true;
  }

  void print(std::ostream &Out) const override {
    Out << "Utf8\t" << getValue() << "\n";
  }

private:
  const Utf8String Value;
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
    // TODO: Check that names are in a correct form.
    return NameRef != nullptr && dynamic_cast<Utf8*>(NameRef.get()) &&
      DescriptorRef != nullptr && dynamic_cast<Utf8*>(DescriptorRef.get());
  }

  void print(std::ostream &Out) const override {
    Out << "NameAndType\t" << getName() << " " << getDescriptor() << "\n";
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

  const Utf8 &getUtf8() const {
    assert(isValid());
    return *static_cast<Utf8*>(Name.get());
  }

  const Utf8String &getName() const {
    assert(isValid());
    return static_cast<Utf8*>(Name.get())->getValue();
  }

  bool isValid() const override {
    // TODO: Check that name is in a correct form.
    return Name != nullptr && dynamic_cast<Utf8*>(Name.get());
  }

  void print(std::ostream &Out) const override {
    Out << "ClassInfo\t" << getName() << "\n";
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

  const ClassInfo &getClass() const {
    assert(isValid());
    return *static_cast<ClassInfo*>(ClassRef.get());
  }

  const NameAndType &getNameAndType() const {
    assert(isValid());
    return *static_cast<NameAndType*>(NameAndTypeRef.get());
  }

  bool isValid() const override {
    // TODO: Check that name is in a correct form.
    return ClassRef != nullptr && dynamic_cast<ClassInfo*>(ClassRef.get()) &&
      NameAndTypeRef != nullptr && dynamic_cast<NameAndType*>(NameAndTypeRef.get());
  }

  void print(std::ostream &Out) const override {
    Out << "MethodRef\t" << getClass().getName() << " " <<
      getNameAndType().getName() << " " << getNameAndType().getDescriptor() << "\n";
  }

private:
  const ConstantPool::CellReference ClassRef, NameAndTypeRef;
};

}
}

#endif //IJVM_CONSTANTPOOLRECORDS_H
