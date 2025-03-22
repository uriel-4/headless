cmake_minimum_required(VERSION 3.16)
project(headless)

# Find LLVM and Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "LLVM version: ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM include directory: ${LLVM_INCLUDE_DIRS}")
message(STATUS "Clang include directory: ${CLANG_INCLUDE_DIRS}")

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

add_subdirectory(lib/cxxopts)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(SOURCES
    src/main.cpp
    src/Extractor.cpp
)

add_executable(headless ${SOURCES})

llvm_map_components_to_libnames(LLVM_LIBS support core)
target_link_libraries(headless PRIVATE ${LLVM_LIBS} clangTooling clangFrontend clangAST clangLex clangBasic cxxopts)
