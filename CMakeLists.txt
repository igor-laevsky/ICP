cmake_minimum_required(VERSION 3.8)
project(IJVM)

set(CMAKE_CXX_STANDARD 17)

include_directories( ./src )

add_executable(IJVM src/main.cpp src/JavaTypes/JavaClass.cpp src/ClassFileReader/ClassFileReader.cpp src/Utils/BinaryFiles.h src/Utils/BinaryFiles.cpp)