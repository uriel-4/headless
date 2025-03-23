# (indev) headless — Automatic C++ Header Generator

`headless` — is a tool that splits code files into corresponding `.hpp` and `.cpp` files, eliminating the need to manually duplicate function declarations and definitions.

## Why?
C/C++ is one of the few languages that requires manually maintaining header files. While separating declarations and definitions may be helpful for writing libraries, but I believe not really pleasant for rapid prototyping and workflow that requires multiple iterations.

There are other approaches, like [lazycpp](https://github.com/mjspncr/lzz3) that makes you write code in .lzz files with custom macros, or [makeheaders](https://github.com/bjconlan/makeheaders) that gives an option to write in one file, but still twice declaration and definition in different places. These seem like a hacky way to do that, and IDE/language servers/debuggers are usually not happy about it.

### The Idea
C++ already allows writing everything in only `.hpp` files! It's just compilers usually hate it. Impossible circular dependencies, slow recompilation time and no hot reloading (was my case), usually make you write separate source and header files anyway.

`headless` fixes that by automatically splitting these files into proper `.hpp` and `.cpp` files in a separate folder.

### Features
- Write full definitions in `.hpp` files
- Automatically generates `.hpp` + `.cpp` pairs
- Compatible with IDEs, language servers, and existing toolchains
- Generates #line directives for debuggers
- Supports preprocessor directives and complex C++ features
- Optional generation of `sources.cmake` for CMake

## Proposed pipeline

- The source code is inside `src/` as `.hpp` files
- `headless` generates `.cpp` and `.hpp` files inside `gsrc/` (_gitignored_) as preprocess command in CMake
- `headless` also generates `gsrc/sources.cmake` file with paths to all `.cpp` files listed, to include as sources for CMake

### Status
- [x] [passing values of static/global variables](https://github.com/uriel-4/headless/tree/dev/test/static)
- [x] passing functions bodies
  - [x] except [inline](https://github.com/uriel-4/headless/tree/dev/test/inline), constexpr
  - [x] [operators](https://github.com/uriel-4/headless/tree/dev/test/operator)
  - [x] [keep annotations](https://github.com/uriel-4/headless/tree/dev/test/annotation)
- [x] [namespace support](https://github.com/uriel-4/headless/tree/dev/test/namespace)
- [x] [#ifdef support](https://github.com/uriel-4/headless/tree/dev/test/ifdef)
- [x] [generation of #line directives for debugger](https://github.com/uriel-4/headless/tree/dev/test/lines)
- [ ] review explicit templates
- [ ] CMake integration
- [x] Linux support
- [x] macOS support
- [ ] Windows support

#### Installing as CLI
You can compile `headless` and use it with command line interface:
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
