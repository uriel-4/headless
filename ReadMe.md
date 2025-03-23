# headless — Automatic C++ Header Generator

`headless` is a tool that splits .hpp files into corresponding .h and .cpp files, eliminating the need to manually duplicate function declarations and definitions.

## Why?
C/C++ is one of the few modern languages that still requires manually maintaining header files. While separating declarations and definitions has benefits, it often slows down development — especially in iterative workflows like game development or rapid prototyping.

Maintaining headers can be error-prone and tedious:
- Forgetting to update declarations
- Creating duplicate boilerplate
- Manually adding new `.cpp` files to build scripts

Other approaches exist (e.g., macro-based generators like [makeheaders](https://github.com/bjconlan/makeheaders) or alternative preprocessors like [lazycpp](https://github.com/mjspncr/lzz3)), but they often require non-standard extensions or hacks incompatible with IDEs or language servers.

### The Idea
C++ already allows writing everything in a single .hpp file — it's just not scalable. headless fixes that by automatically splitting these files into proper .h and .cpp files, without changing the way you write C++.

### Features
- Write full definitions in `.hpp` files
- Automatically generates `.hpp` + `.cpp` pairs
- Compatible with IDEs, language servers, and existing toolchains
- Supports preprocessor directives and complex C++ features
- Optional generation of `sources.cmake` for CMake

## Proposed pipeline

Here is how I use `headless` in my projects:
- The source code is inside `src/` as `.hpp` files
- `headless` generates `.cpp` and `.hpp` files inside `gsrc/` (_gitignored_) as preprocess command in CMake
- `headless` also generates `gsrc/sources.txt` file with paths to all `.cpp` files listed, to include as sources for CMake

You can try a sample project here: https://github.com/uriel-4/headless-example

But essentially `headless` is just a tool that divides `.hpp` files into `.cpp` and `.hpp`, and you can use it as you like.

### Status
- [x] passing functions bodies
  - [x] except inline, constexpr
  - [x] operators
  - [x] pass annotations
- [x] namespace support
- [x] #ifdef support
- [x] Linux support
- [x] macOS support
- [ ] Windows support
- [ ] templates

### Installing
You can add `headless` as submodule to your project:
```bash
git submodule add https://github.com/uriel-4/headless lib/headless
git submodule update --init --recursive
```
And include its CMake into yours:
```cmake
set(HEADLESS_SYNC_FROM src/)
set(HEADLESS_SYNC_TO gsrc/)
add_subdirectory(lib/headless)
include(gsrc/sources.cmake)
```

#### Installing as CLI
You can still compile `headless` and use it with command line interface:
```bash
git clone https://github.com/uriel-4/headless
cd headless
mkdir build
cd build
cmake ..
make -j12
./headless --help
```

#### Tests

You can test `headless` against `test/` folder:
```bash
./headless --test=./test
```