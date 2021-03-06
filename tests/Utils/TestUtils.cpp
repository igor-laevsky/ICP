//
// Implementation of the utilities for tests.
// This doesn't test anything.
//

#include "TestUtils.h"

using namespace TestUtils;
using namespace JavaTypes;

static std::vector<uint8_t> trivialBytecodePlain() {
  return
      {0x2a,             // aload_0
       0xb7, 0x00, 0x01, // invokespecial #1
       0xb1 };           // return
}

static std::unique_ptr<ConstantPool> createConstantPool() {
  ConstantPoolBuilder Builder(27);

  Builder.set(1, std::make_unique<ConstantPoolRecords::Utf8>("trivial_method"));
  Builder.set(2, std::make_unique<ConstantPoolRecords::Utf8>("()I"));
  Builder.set(3, std::make_unique<ConstantPoolRecords::Utf8>("trivial_class"));
  Builder.set(4, std::make_unique<ConstantPoolRecords::ClassInfo>(
      Builder.getCellReference<ConstantPoolRecords::Utf8>(3)));


  Builder.set(5, std::make_unique<ConstantPoolRecords::Utf8>("()V"));
  Builder.set(6, std::make_unique<ConstantPoolRecords::Utf8>("()J"));

  Builder.set(7, std::make_unique<ConstantPoolRecords::Utf8>("([Ljava/lang/String;)V"));
  Builder.set(8, std::make_unique<ConstantPoolRecords::Utf8>("(Ljava/lang/Object;)V"));
  Builder.set(9, std::make_unique<ConstantPoolRecords::Utf8>("(I)V"));

  Builder.set(10, std::make_unique<ConstantPoolRecords::Utf8>("java/lang/Object"));
  Builder.set(12, std::make_unique<ConstantPoolRecords::ClassInfo>(
      Builder.getCellReference<ConstantPoolRecords::Utf8>(10)));

  // NameAndType "<init>":()V
  Builder.set(11, std::make_unique<ConstantPoolRecords::Utf8>("<init>"));
  Builder.set(13, std::make_unique<ConstantPoolRecords::NameAndType>(
      Builder.getCellReference<ConstantPoolRecords::Utf8>(11),
      Builder.getCellReference<ConstantPoolRecords::Utf8>(5)));
  // MethodRef java/lang/Object."<init>":()V
  Builder.set(14, std::make_unique<ConstantPoolRecords::MethodRef>(
      Builder.getCellReference<ConstantPoolRecords::ClassInfo>(12),
      Builder.getCellReference<ConstantPoolRecords::NameAndType>(13)));

  // NameAndType <init>:(Ljava/lang/Object;I)V
  Builder.set(15,
      std::make_unique<ConstantPoolRecords::Utf8>("(Ljava/lang/String;I)V"));
  Builder.set(16, std::make_unique<ConstantPoolRecords::NameAndType>(
      Builder.getCellReference<ConstantPoolRecords::Utf8>(11),
      Builder.getCellReference<ConstantPoolRecords::Utf8>(15)));
  // MethodRef java/lang/Object.<init>:(Ljava/lang/Object;I)V
  Builder.set(17, std::make_unique<ConstantPoolRecords::MethodRef>(
      Builder.getCellReference<ConstantPoolRecords::ClassInfo>(12),
      Builder.getCellReference<ConstantPoolRecords::NameAndType>(16)));

  // NameAndType <init>:(Ljava/lang/Object;)V
  Builder.set(18, std::make_unique<ConstantPoolRecords::NameAndType>(
      Builder.getCellReference<ConstantPoolRecords::Utf8>(11),
      Builder.getCellReference<ConstantPoolRecords::Utf8>(8)));

  // NameAndType <init>:()I
  Builder.set(19, std::make_unique<ConstantPoolRecords::NameAndType>(
      Builder.getCellReference<ConstantPoolRecords::Utf8>(11),
      Builder.getCellReference<ConstantPoolRecords::Utf8>(2)));
  // MethodRef java/lang/Object.<init>:()I
  Builder.set(20, std::make_unique<ConstantPoolRecords::MethodRef>(
      Builder.getCellReference<ConstantPoolRecords::ClassInfo>(12),
      Builder.getCellReference<ConstantPoolRecords::NameAndType>(19)));

  Builder.set(21, std::make_unique<ConstantPoolRecords::Utf8>(
      "I"));
  Builder.set(22, std::make_unique<ConstantPoolRecords::Utf8>(
      "F1"));

  Builder.set(23, std::make_unique<ConstantPoolRecords::Utf8>(
      "D"));
  Builder.set(24, std::make_unique<ConstantPoolRecords::Utf8>(
      "F2"));

  Builder.set(25, std::make_unique<ConstantPoolRecords::Utf8>(
      "LFields;"));
  Builder.set(26, std::make_unique<ConstantPoolRecords::Utf8>(
      "Ref"));

  Builder.set(27, std::make_unique<ConstantPoolRecords::Utf8>(
      "LFields"));

  return Builder.createConstantPool();
}

// Eternal constant pool to simplify memory management
ConstantPool &TestUtils::getEternalConstantPool() {
  static auto Ret = createConstantPool();
  return *Ret;
}

std::unique_ptr<JavaTypes::JavaMethod> TestUtils::createMethod(
    uint16_t MaxStack, uint16_t MaxLocals,
    ConstantPool::IndexType NameIdx,
    ConstantPool::IndexType DescriptorIdx,
    const std::vector<uint8_t> &Bytecode,
    JavaMethod::AccessFlags Flags /*= ACC_PUBLIC */) {

  const auto &Name =
      getEternalConstantPool().getAs<ConstantPoolRecords::Utf8>(NameIdx);
  const auto &Descriptor =
      getEternalConstantPool().getAs<ConstantPoolRecords::Utf8>(DescriptorIdx);

  JavaMethod::MethodConstructorParameters Params;

  Params.Flags = Flags;

  Params.Name = &Name;
  Params.Descriptor = &Descriptor;

  Params.MaxLocals = MaxLocals;
  Params.MaxStack = MaxStack;
  Params.Code = Bytecode::parseInstructions(Bytecode);

  return std::make_unique<JavaMethod>(std::move(Params));
}

std::unique_ptr<JavaTypes::JavaMethod> TestUtils::createMethod(
    const std::vector<uint8_t> &Bytecode) {
  return createMethod(
      10, 10, 1, 2,
      Bytecode, JavaMethod::AccessFlags::ACC_PUBLIC);
}

std::unique_ptr<JavaMethod> TestUtils::createTrivialMethod() {
  return createMethod(trivialBytecodePlain());
}

std::unique_ptr<JavaTypes::JavaClass> TestUtils::createClass(
    std::vector<std::unique_ptr<JavaTypes::JavaMethod>> &&Methods) {

  auto CP = createConstantPool();

  JavaClass::ClassParameters Params;

  Params.CP = std::move(CP);
  Params.SuperClass = nullptr;
  Params.ClassName = &Params.CP->getAs<ConstantPoolRecords::ClassInfo>(4);
  Params.Flags =
      JavaClass::AccessFlags::ACC_PUBLIC | JavaClass::AccessFlags::ACC_SUPER;
  Params.Methods = std::move(Methods);

  return std::make_unique<JavaClass>(std::move(Params));

}
