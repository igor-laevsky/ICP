//
// Implementation of the utilities for tests.
// This doesn't test anything.
//

#include "TestUtils.h"

#include "Bytecode/Bytecode.h"

#include "JavaTypes/ConstantPool.h"
#include "JavaTypes/JavaClass.h"
#include "JavaTypes/JavaMethod.h"

using namespace TestUtils;
using namespace JavaTypes;

static std::vector<uint8_t> trivialBytecodePlain() {
  return
      {0x2a,             // aload_0
       0xb7, 0x00, 0x01, // invokespecial #1
       0xb1 };           // return
}

static std::unique_ptr<ConstantPool> createConstantPool() {
  ConstantPoolBuilder Builder(9);

  Builder.set(1, std::make_unique<ConstantPoolRecords::Utf8>("trivial_method"));
  Builder.set(2, std::make_unique<ConstantPoolRecords::Utf8>("()I"));
  Builder.set(3, std::make_unique<ConstantPoolRecords::Utf8>("trivial_class"));
  Builder.set(4, std::make_unique<ConstantPoolRecords::ClassInfo>(
      Builder.getCellReference(3)));


  Builder.set(5, std::make_unique<ConstantPoolRecords::Utf8>("()V"));
  Builder.set(6, std::make_unique<ConstantPoolRecords::Utf8>("()J"));

  Builder.set(7, std::make_unique<ConstantPoolRecords::Utf8>("([Ljava/lang/String;)V"));
  Builder.set(8, std::make_unique<ConstantPoolRecords::Utf8>("(Ljava/lang/Object;)V"));
  Builder.set(9, std::make_unique<ConstantPoolRecords::Utf8>("(I)V"));

  auto CP = Builder.createConstantPool();
  assert(CP->verify());
  return CP;
}

// Eternal constant pool to simplify memory management
static ConstantPool &getEternalConstantPool() {
  static auto Ret = createConstantPool();
  return *Ret;
}

std::vector<std::unique_ptr<Bytecode::Instruction>>
TestUtils::createTrivialBytecode() {
  return Bytecode::parseInstructions(trivialBytecodePlain());
}

std::unique_ptr<JavaTypes::JavaMethod> TestUtils::createMethod(
    uint16_t MaxStack, uint16_t MaxLocals,
    ConstantPool::IndexType NameIdx,
    ConstantPool::IndexType DescriptorIdx,
    const std::vector<uint8_t> &Bytecode,
    JavaMethod::StackMapTableType &&StackMapTable,
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
  Params.Code = Bytecode;
  Params.StackMapTable = std::move(StackMapTable);

  return std::make_unique<JavaMethod>(std::move(Params));
}

std::unique_ptr<JavaTypes::JavaMethod> TestUtils::createMethod(
    const std::vector<uint8_t> &Bytecode) {
  JavaMethod::StackMapTableType T = {
      {0, Verifier::StackFrame({}, {})}
  };
  return createMethod(
      10, 10, 1, 2,
      Bytecode, std::move(T));
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

std::unique_ptr<JavaClass> TestUtils::createTrivialClass() {
  std::vector<std::unique_ptr<JavaTypes::JavaMethod>> Methods;

  Methods.push_back(createTrivialMethod());
  return createClass(std::move(Methods));
}
