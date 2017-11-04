//
// Those are helper function for other tests.
//

#ifndef ICP_TESTUTILS_H
#define ICP_TESTUTILS_H

#include <memory>
#include <vector>

#include "Bytecode/Bytecode.h"
#include "JavaTypes/JavaClass.h"
#include "JavaTypes/JavaMethod.h"
#include "JavaTypes/ConstantPool.h"

namespace TestUtils {

std::vector<std::unique_ptr<Bytecode::Instruction>> createTrivialBytecode();

std::unique_ptr<JavaTypes::JavaMethod> createMethod(
    const std::vector<uint8_t> &Bytecode);

std::unique_ptr<JavaTypes::JavaMethod> createMethod(
    uint16_t MaxStack, uint16_t MaxLocals,
    JavaTypes::ConstantPool::IndexType NameIdx,
    JavaTypes::ConstantPool::IndexType DescriptorIdx,
    const std::vector<uint8_t> &Bytecode,
    JavaTypes::JavaMethod::StackMapTableType &&StackMapTable,
    JavaTypes::JavaMethod::AccessFlags Flags = JavaTypes::JavaMethod::AccessFlags::ACC_PUBLIC);

std::unique_ptr<JavaTypes::JavaMethod> createTrivialMethod();

std::unique_ptr<JavaTypes::JavaClass> createClass(
    std::vector<std::unique_ptr<JavaTypes::JavaMethod>> &&Methods);

std::unique_ptr<JavaTypes::JavaClass> createTrivialClass();

// Vector with all available bytecode instructions with no specific order
//std::vector<std::unique_ptr<Bytecode::Instruction>> allBytecodeInstructions();

}

#endif //ICP_TESTUTILS_H
