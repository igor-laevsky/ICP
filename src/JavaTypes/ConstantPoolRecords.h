//
// Contains definitions of the various constant pool records.
//

#ifndef ICP_CONSTANTPOOLRECORDS_H
#define ICP_CONSTANTPOOLRECORDS_H

#include "ConstantPool.h"

#include "Utils/Utf8String.h"

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
  NameAndType(ConstantPool::CellReference<Utf8> NewNameRef,
              ConstantPool::CellReference<Utf8> NewDescriptorRef):
      NameRef(NewNameRef), DescriptorRef(NewDescriptorRef) {
    ;
  }

  const Utf8String &getName() const {
    assert(isValid());
    return static_cast<Utf8*>(NameRef.get())->getValue();
  }

  const Utf8String &getDescriptor() const {
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
  ConstantPool::CellReference<Utf8> NameRef, DescriptorRef;
};

class ClassInfo final: public Record {
public:
  explicit ClassInfo(ConstantPool::CellReference<Utf8> NewName):
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
    return Name != nullptr &&
        dynamic_cast<Utf8*>(static_cast<Record*>(Name.get()));
  }

  void print(std::ostream &Out) const override {
    Out << "ClassInfo\t" << getName() << "\n";
  }

private:
  ConstantPool::CellReference<Utf8> Name;
};

// Common implementation of the field, method and interface ref fields
class RefRecord : public Record {
public:
  RefRecord(ConstantPool::CellReference<ClassInfo> NewClassRef,
            ConstantPool::CellReference<NameAndType> NewNameAndTypeRef) :
      ClassRef(NewClassRef), NameAndTypeRef(NewNameAndTypeRef) {
    ;
  }

  const ClassInfo &getClass() const {
    assert(isValid());
    return *ClassRef;
  }

  const NameAndType &getNameAndType() const {
    assert(isValid());
    return *NameAndTypeRef;
  }

  const Utf8String &getClassName() const {
    return getClass().getName();
  }

  const Utf8String &getName() const {
    return getNameAndType().getName();
  }

  const Utf8String &getDescriptor() const {
    return getNameAndType().getDescriptor();
  }

  bool isValid() const override {
    return ClassRef != nullptr && dynamic_cast<ClassInfo *>(ClassRef.get()) &&
           NameAndTypeRef != nullptr &&
           dynamic_cast<NameAndType *>(NameAndTypeRef.get());
  }

private:
  ConstantPool::CellReference<ClassInfo> ClassRef;
  ConstantPool::CellReference<NameAndType> NameAndTypeRef;
};

class MethodRef final: public RefRecord {
public:
  MethodRef(ConstantPool::CellReference<ClassInfo> ClassRef,
            ConstantPool::CellReference<NameAndType> NameAndTypeRef) :
      RefRecord(ClassRef, NameAndTypeRef) {
    ;
  }

  void print(std::ostream &Out) const override {
    Out << "MethodRef\t" << getClass().getName() << " " <<
        getNameAndType().getName() << " " << getNameAndType().getDescriptor()
        << "\n";
  }

  // TODO: Add descriptor verification
};

class FieldRef final: public RefRecord {
public:
  FieldRef(ConstantPool::CellReference<ClassInfo> ClassRef,
           ConstantPool::CellReference<NameAndType> NameAndTypeRef) :
      RefRecord(ClassRef, NameAndTypeRef) {
    ;
  }

  void print(std::ostream &Out) const override {
    Out << "FieldRef\t" << getClass().getName() << " " <<
        getNameAndType().getName() << " " << getNameAndType().getDescriptor()
        << "\n";
  }

  // TODO: Add descriptor verification
};

}
}

#endif //ICP_CONSTANTPOOLRECORDS_H
