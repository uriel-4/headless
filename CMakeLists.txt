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

if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)

    add_executable(headless src/main.cpp)
    target_compile_options(headless PRIVATE -Oz -fdata-sections -ffunction-sections)
    target_link_options(headless PRIVATE -Wl,-dead_strip)
    set_target_properties(headless PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)

    target_link_libraries(headless PRIVATE clangTooling cxxopts)

else()

    if (NOT DEFINED HEADLESS_SYNC_FROM)
        set(HEADLESS_SYNC_FROM "${CMAKE_SOURCE_DIR}/src")
    endif()
    if (NOT DEFINED HEADLESS_SYNC_TO)
        set(HEADLESS_SYNC_TO "${CMAKE_SOURCE_DIR}/gsrc")
    endif()

    add_executable(headless_tool src/main.cpp)
    target_link_libraries(headless_tool PRIVATE clangTooling cxxopts)

    add_custom_command(
        OUTPUT ${HEADLESS_SYNC_TO}/sources.cmake
        COMMAND headless_tool --from=${HEADLESS_SYNC_FROM} --to=${HEADLESS_SYNC_TO} -ig
        DEPENDS headless_tool
        COMMENT "Generating source files with headless"
    )

    add_custom_target(run_headless
        DEPENDS ${HEADLESS_SYNC_TO}/sources.cmake
    )

endif()

