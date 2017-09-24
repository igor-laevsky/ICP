cmake_minimum_required(VERSION 3.8)
project(ICP)

set(CMAKE_CXX_STANDARD 17)

set(CXX_FLAGS "-Wall -Werror")
set(LINK_FLAGS "-static -static-libgcc -static-libstdc++")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}${CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}${LINKER_FLAGS}")

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
        src/JavaTypes/ConstantPoolRecords.h src/Utils/Utf8String.h src/JavaTypes/JavaMethod.cpp src/JavaTypes/JavaMethod.h)

set (TEST_FILES
        tests/ClassFileReader/ClassFileReaderTests.cpp
        tests/JavaTypes/ConstantPoolTests.cpp)

add_executable(ICP
        src/main.cpp
        ${SOURCE_FILES})

add_executable(ICP_unit_tests
        tests/tests_main.cpp
        ${SOURCE_FILES}
        ${TEST_FILES})
