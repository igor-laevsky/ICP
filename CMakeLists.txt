cmake_minimum_required(VERSION 3.8)
project(ICP)

set(CMAKE_CXX_STANDARD 17)

set(CXX_FLAGS "-Wall -Werror -Wextra -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}${CXX_FLAGS}")

include_directories( ./src )
include_directories( ./vendor )
include_directories( ./tests )

set(SOURCE_FILES
        src/ClassFileReader/ClassFileReader.cpp
        src/ClassFileReader/ClassFileReader.h
        src/Utils/BinaryFiles.cpp
        src/Utils/BinaryFiles.h
        src/Utils/Iterators.h
        src/Utils/Utf8String.h
        src/JavaTypes/JavaClass.cpp
        src/JavaTypes/JavaClass.h
        src/JavaTypes/ConstantPool.cpp
        src/JavaTypes/ConstantPool.h
        src/JavaTypes/ConstantPoolRecords.h
        src/JavaTypes/JavaMethod.cpp
        src/JavaTypes/JavaMethod.h
        src/Bytecode/Bytecode.cpp
        src/Bytecode/Bytecode.h
        src/Bytecode/Instructions.h
        src/JavaTypes/Type.h
        src/JavaTypes/Type.cpp
        src/Verifier/StackFrame.cpp
        src/Verifier/StackFrame.h
        src/Bytecode/InstructionVisitor.cpp
        src/Bytecode/InstructionVisitor.h
        src/Bytecode/InstructionsFwd.h
        src/Verifier/Verifier.cpp
        src/Verifier/Verifier.h
        src/JavaTypes/JavaTypesFwd.h
        src/Verifier/VerifierFwd.h
        src/SlowInterpreter/SlowInterpreter.h
        src/SlowInterpreter/SlowInterpreter.cpp
        src/CD/Lexer.cpp
        src/CD/Lexer.h
        src/CD/Parser.cpp
        src/CD/Parser.h
        src/JavaTypes/JavaField.cpp
        src/JavaTypes/JavaField.h src/SlowInterpreter/Value.h src/SlowInterpreter/Value.cpp)

set (TEST_FILES
        tests/ClassFileReader/ClassFileReaderTests.cpp
        tests/JavaTypes/ConstantPoolTests.cpp
        tests/JavaTypes/InstructionTests.cpp
        tests/JavaTypes/JavaMethodTests.cpp
        tests/Utils/IteratorsTests.cpp
        tests/Verifier/TypeTests.cpp
        tests/Verifier/StackFrameTests.cpp
        tests/JavaTypes/InstructionVisitorTests.cpp
        tests/Verifier/VerifierTests.cpp
        tests/Utils/TestUtils.cpp
        tests/Utils/TestUtils.h
        tests/SlowInterpreter/SlowInterpreterTests.cpp
        tests/CD/LexerTests.cpp
        tests/CD/ParserTests.cpp
        tests/JavaTypes/JavaFieldTests.cpp tests/SlowInterpreter/ValueTests.cpp)

add_library(ICP_LIB ${SOURCE_FILES})

add_executable(ICP src/main.cpp)
target_link_libraries(ICP ICP_LIB)

add_executable(ICP_unit_tests tests/tests_main.cpp ${TEST_FILES})
target_link_libraries(ICP_unit_tests ICP_LIB)
