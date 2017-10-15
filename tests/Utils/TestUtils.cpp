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
  ConstantPoolBuilder Builder(4);

  Builder.set(1, std::make_unique<ConstantPoolRecords::Utf8>("trivial_method"));
  Builder.set(2, std::make_unique<ConstantPoolRecords::Utf8>("()V"));
  Builder.set(3, std::make_unique<ConstantPoolRecords::Utf8>("trivial_class"));
  Builder.set(4, std::make_unique<ConstantPoolRecords::ClassInfo>(
      Builder.getCellReference(3)));

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
    const std::vector<uint8_t> &Bytecode) {

  const auto &Name =
      getEternalConstantPool().getAs<ConstantPoolRecords::Utf8>(1);
  const auto &Descriptor =
      getEternalConstantPool().getAs<ConstantPoolRecords::Utf8>(2);

  JavaMethod::MethodConstructorParameters Params;

  Params.Flags = JavaMethod::AccessFlags::ACC_PUBLIC;

  Params.Name = &Name;
  Params.Descriptor = &Descriptor;

  Params.MaxLocals = 0;
  Params.MaxStack = 0;
  Params.Code = Bytecode;

  return std::make_unique<JavaMethod>(std::move(Params));
}

std::unique_ptr<JavaMethod> TestUtils::createTrivialMethod() {
  return createMethod(trivialBytecodePlain());
}

std::unique_ptr<JavaClass> TestUtils::createTrivialClass() {
  // These two are using two different constant pools. Should be fine for
  // testing purposes.
  auto CP = createConstantPool();

  std::vector<std::unique_ptr<JavaMethod>> Methods;
  Methods.push_back(createTrivialMethod());

  JavaClass::ClassParameters Params;

  Params.CP = std::move(CP);
  Params.SuperClass = nullptr;
  Params.ClassName = &Params.CP->getAs<ConstantPoolRecords::ClassInfo>(4);
  Params.Flags =
      JavaClass::AccessFlags::ACC_PUBLIC | JavaClass::AccessFlags::ACC_SUPER;
  Params.Methods = std::move(Methods);

  return std::make_unique<JavaClass>(std::move(Params));
}
