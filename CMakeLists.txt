cmake_minimum_required(VERSION 3.8)
project(ICP)

set(CMAKE_CXX_STANDARD 17)

set(CXX_FLAGS "-Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}${CXX_FLAGS}")

include_directories( ./src )
include_directories( ./vendor )
include_directories( ./tests )

set(SOURCE_FILES
        src/JavaTypes/JavaClass.cpp
        src/JavaTypes/JavaClass.h
        src/ClassFileReader/ClassFileReader.cpp
        src/ClassFileReader/ClassFileReader.h
        src/Utils/BinaryFiles.cpp
        src/Utils/BinaryFiles.h src/JavaTypes/ConstantPool.cpp
        src/JavaTypes/ConstantPool.h
        src/JavaTypes/ConstantPoolRecords.h
        src/Utils/Utf8String.h
        src/JavaTypes/JavaMethod.cpp
        src/JavaTypes/JavaMethod.h
        src/JavaTypes/Bytecode.cpp
        src/JavaTypes/Bytecode.h
        src/JavaTypes/BytecodeInstructions.h)

set (TEST_FILES
        tests/ClassFileReader/ClassFileReaderTests.cpp
        tests/JavaTypes/ConstantPoolTests.cpp
        tests/JavaTypes/InstructionTests.cpp tests/JavaTypes/JavaMethodTests.cpp)

add_library(ICP_LIB ${SOURCE_FILES} src/Utils/Iterators.h)

add_executable(ICP src/main.cpp ${SOURCE_FILES})
target_link_libraries(ICP ICP_LIB)

add_executable(ICP_unit_tests tests/tests_main.cpp ${TEST_FILES})
target_link_libraries(ICP_unit_tests ICP_LIB)
