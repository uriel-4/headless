# (beta) headless — Automatic C++ Header Generator

`headless` — is a tool that splits code files into `.hpp` and `.cpp` files, eliminating the need to manually write header+source in separate files.

## Why?
C/C++ is one of the few languages that requires manually maintaining header files. It is great for a documentation, when writing libraries for others; But it might feel like a burden, when your workflow involves many iterations, rapid prototyping, especially if you come from another language.

There are other approaches, like [lazycpp](https://github.com/mjspncr/lzz3) that creates a new file extension with its own macro system, or [makeheaders](https://github.com/bjconlan/makeheaders) that generates header from macros, but still asks you to write declaration and implementation twice. These seem like a hacky way to do that, and IDE/language servers/debuggers are usually not happy about it.

### The Idea
C++ already allows writing everything in only `.hpp` files! It's just compilers usually hate it. Impossible circular dependencies, slow recompilation time and no hot reloading (was my case), usually makes you write separate source and header files anyway.

`headless` fixes that by automatically splitting these files into proper `.hpp` and `.cpp` files in a separate folder.

### Features
- Write full definitions in `.hpp` files
- Automatically generates `.hpp` + `.cpp` pairs
- Compatible with IDEs, language servers, and existing toolchains
- Generates `#line` directives for debuggers
- Uses CLang parser, for supporting many cases
- Generates list of sources in `sources.cmake` for CMake

## Proposed pipeline

- The source code is inside `src/` as `.hpp` files
- `headless` generates `.cpp` and `.hpp` files inside `gsrc/` (_gitignored_) as preprocess command in CMake
- `headless` also generates `gsrc/sources.cmake` file with paths to all `.cpp` files listed, to include as sources for CMake

## Installing

Binaries for Linux and macOS (arm64) are in [Releases](https://github.com/uriel-4/headless/releases). For proposed pipeline, put a binary in `$PATH` directory. 

#### Building

For building install `LLVM 19` library. Then do:
```bash
git clone --recursive https://github.com/uriel-4/headless
cd headless
cmake -B build
cmake --build build
build/headless --test=./test
```
You can then put `build/headless` file to `$PATH` directory.

## Integration with CMake

Example of `CMakeLists.txt` is below. You can see full example of integrating `headless` in this repository: https://github.com/uriel-4/headless-example
```cmake
set(SRC_DIR src)
set(GEN_SRC_DIR gsrc)
execute_process(
    COMMAND headless -iglw --from=${SRC_DIR} --to=${GEN_SRC_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_custom_target(headless
    COMMAND headless -iglw --from=${SRC_DIR} --to=${GEN_SRC_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
include(${GEN_SRC_DIR}/sources.cmake)

add_executable(your_program ${SOURCES})
add_dependencies(your_program headless)
```
This runs `headless` both before CMake configuration and building. `-i` stands for incremental, meaning that `headless` will process a file, only if its last modified date was updated.

_(coming soon)_ Example of integrating `headless` into Hot Reload feature.

### Status
- [x] [passing values of static/global variables](https://github.com/uriel-4/headless/tree/dev/test/static)
  - [ ] [support complex types](https://github.com/uriel-4/headless/tree/dev/test/-static_extra1)
- [x] passing functions bodies
  - [x] except [inline](https://github.com/uriel-4/headless/tree/dev/test/inline), constexpr
  - [x] [operators](https://github.com/uriel-4/headless/tree/dev/test/operator)
  - [x] [keep annotations](https://github.com/uriel-4/headless/tree/dev/test/annotation)
- [x] [namespace support](https://github.com/uriel-4/headless/tree/dev/test/namespace)
- [x] [#ifdef support](https://github.com/uriel-4/headless/tree/dev/test/ifdef)
- [x] [generation of #line directives for debugger](https://github.com/uriel-4/headless/tree/dev/test/lines)
  - [ ] add #line also when copying sources
  - [ ] use #define magic to use less memory defining #line?
- [ ] review explicit templates
- [ ] CMake integration
- [x] Linux support
- [x] macOS support
- [ ] Windows support

