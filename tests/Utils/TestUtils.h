//
// Those are helper function for other tests.
//

#ifndef ICP_TESTUTILS_H
#define ICP_TESTUTILS_H

#include <memory>
#include <vector>
#include <exception>

#include "catch.hpp"

#include "Bytecode/Bytecode.h"
#include "JavaTypes/JavaClass.h"
#include "JavaTypes/JavaMethod.h"
#include "JavaTypes/ConstantPool.h"

#include <iostream>

namespace TestUtils {

/// Matcher for the exceptions with description
class ExEquals : public Catch::MatcherBase<std::runtime_error> {
  const char *Target;

public:
    explicit ExEquals(const char *Target) noexcept:
        Target(Target) {
      ;
    }

    bool match (const std::runtime_error &Ex) const override {
      return std::string(Ex.what()) == Target;
    }

    std::string describe() const override {
        std::ostringstream ss;
        ss << "is exuals " << Target;
        return ss.str();
    }
};

/// Return static constant pool with the variety of records suitable for
/// testing. See sources for the list of avaliable records.
JavaTypes::ConstantPool &getEternalConstantPool();

/// Method and class creation utility functions.
/// Deprecated. Prefer using CD description language.
std::unique_ptr<JavaTypes::JavaMethod> createMethod(
    const std::vector<uint8_t> &Bytecode);

std::unique_ptr<JavaTypes::JavaMethod> createMethod(
    uint16_t MaxStack, uint16_t MaxLocals,
    JavaTypes::ConstantPool::IndexType NameIdx,
    JavaTypes::ConstantPool::IndexType DescriptorIdx,
    const std::vector<uint8_t> &Bytecode,
    JavaTypes::JavaMethod::AccessFlags Flags = JavaTypes::JavaMethod::AccessFlags::ACC_PUBLIC);

std::unique_ptr<JavaTypes::JavaMethod> createTrivialMethod();

std::unique_ptr<JavaTypes::JavaClass> createClass(
    std::vector<std::unique_ptr<JavaTypes::JavaMethod>> &&Methods);

// Vector with all available bytecode instructions with no specific order
//std::vector<std::unique_ptr<Bytecode::Instruction>> allBytecodeInstructions();

}

#endif //ICP_TESTUTILS_H
