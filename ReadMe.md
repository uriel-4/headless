# headless — C++ headers auto generator

C/C++ is probably the only language that asks you to write header files. Even though it can be helpful to write definitions separately, programs that require tinkering, with many development iterations, usually don't benefit much from such system.

There is also an option to abstain from header files as much as you can, but you would eventually hit some limitations: slow recompile time, impossible cyclic dependencies, and hot reloading source code.

I was researching myself other languages for a game development, but realised that powerful C++ ecosystem is probably the only option. What was stopping me every time is the itch of writing those header files and forgetting to update definitions in both files. Also, don't forget to add your new `.cpp` file to a list of sources every time you create one.

With that said, solution to such problem ideally should be inside a compiler, language server of IDE and other tooling. C++ has many different compilers, and different toolchains that usually come from vendors, when it comes to compiling for their systems. Pushing a change here is too much.

There are some solutions that generate headers, it's usually hacking your way with special macros (makeheaders: _but it still makes you writing functions outside your classes_), or even creating a new file extension that IDE wouldn't comprehend (lazycpp).

But C++ already has a way to avoid writing definitions twice! You can just write all the source code in `.h` or `.hpp`, it just sucks and always sucked. What if we had a tool that divides such files into header and source files?

This is what `headless` is.

### Proposed pipeline

Here is how I use `headless` in my projects:
- The source code is inside `src/` as `.hpp` files
- `headless` generates `.cpp` and `.hpp` files inside `gsrc/` (_gitignored_) as preprocess command in CMake
- `headless` also generates `gsrc/sources.txt` file with paths to all `.cpp` files listed, to include as sources for CMake

You can try a sample project here: https://github.com/uriel-4/headless-example

But essentially `headless` is just a tool that divides `.hpp` files into `.cpp` and `.hpp`, and you can use it as you like. 

## Status
- [ ] Tests
  - [ ] a 
- [ ] Incremental generation
- [ ] Solve cyclic dependencies
- [ ] Windows support
- [ ] macOS support

### Installing
I prefer including dependencies inside a project and building them from source, instead of making everyone who onboards to install yet another tool on their system. You can do that with `headless`, too.

You can add `headless` as submodule:
```bash
git submodule add https://github.com/uriel-4/headless lib/headless
```
And include its CMake into yours:
```cmake
add_project(headless lib/headless)
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

### Setting up proposed pipeline
// TODO

### General Usage

// TODO


