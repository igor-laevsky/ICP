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
    return NameRef->getValue();
  }

  const Utf8String &getDescriptor() const {
    return DescriptorRef->getValue();
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

  const Utf8String &getName() const {
    return Name->getValue();
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
    return *ClassRef;
  }

  const NameAndType &getNameAndType() const {
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
