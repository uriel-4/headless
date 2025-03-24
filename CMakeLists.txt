cmake_minimum_required(VERSION 3.16)
project(headless)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM and Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "LLVM version: ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM include directory: ${LLVM_INCLUDE_DIRS}")
message(STATUS "Clang include directory: ${CLANG_INCLUDE_DIRS}")

include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})

add_subdirectory(lib/cxxopts)

add_executable(headless src/main.cpp)
target_link_libraries(headless PRIVATE clangTooling cxxopts)