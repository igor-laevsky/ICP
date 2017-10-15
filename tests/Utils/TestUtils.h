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

namespace TestUtils {

std::vector<std::unique_ptr<Bytecode::Instruction>> createTrivialBytecode();

std::unique_ptr<JavaTypes::JavaMethod> createMethod(
    const std::vector<uint8_t> &Bytecode);

std::unique_ptr<JavaTypes::JavaMethod> createTrivialMethod();

std::unique_ptr<JavaTypes::JavaClass> createTrivialClass();

// Vector with all available bytecode instructions with no specific order
//std::vector<std::unique_ptr<Bytecode::Instruction>> allBytecodeInstructions();

}

#endif //ICP_TESTUTILS_H
