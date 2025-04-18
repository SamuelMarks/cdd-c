c-cdd
=====

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Linux, Windows, macOS](https://github.com/SamuelMarks/cdd-c/actions/workflows/linux-Windows-macOS.yml/badge.svg)](https://github.com/SamuelMarks/cdd-c/actions/workflows/linux-Windows-macOS.yml)

Frontend for C, concentrating on: generation from code; single-file analysis; modification; and emission (to C code).

Use-cases this is designed to support:

  - Generate/update `free` calling `int cleanup_StructName(struct StructName*)` functions, e.g.:
      - `struct Foo { struct Bar *bar;}; struct Bar { int a; };` will generate:
      - `int cleanup_Bar(struct Bar*);` &
      - `int cleanup_Foo(struct Foo*);` (which internally will call `cleanup_Bar`)
  - With `fmt` of JSON, INI, YAML, TOML, &etc.:
    - Generate/update parsers  from `const char*` to `struct`: `const int StructName_<fmt>_parse(struct StructName*, const char*)` function;
    - Generate/update emitters from `struct` to `char*`: `const int StructName_<fmt>_emit(const struct StructName*, char*)` function;
  - Generate Google Cloud client library for C (with [Google Cloud API Discovery Service](https://developers.google.com/discovery/v1/reference) as input);
  - Generate arbitrary C client libraries (with [OpenAPI](https://spec.openapis.org/oas/v3.1.0) as input);
  - Generating `#pragma` for every function to expose them for calling from, e.g., [assembly](https://www.ibm.com/docs/en/zos/2.5.0?topic=programs-calling-c-code-from-assembler-c-example);
  - Prepare C SDK to be wrapped in higher-level language SDKs ([SWIG](https://en.wikipedia.org/wiki/SWIG) style)

## Design

This C compiler has a very unusual design, the macro and C language are treated as one. The foci are:

  - location start/end of function, `struct`, and feature macros (e.g., `#ifdef JSON_EMIT` then `#endif /* JSON_EMIT */`);
  - `struct` fields.

…which enable a number of use-cases to be simply developed, e.g., see the list above.

### Drawbacks

  - Macros aren't evaluated, which means a simple `#define LBRACE {` will break cdd-c.
  - Like [`m4`](https://en.wikipedia.org/wiki/M4_(computer_language)), CMake, and other tools; cdd-c must be run before your code is built.

### Development guide

Install: CMake ; C compiler toolchain ; git. Then:

```sh
$ git clone "https://github.com/offscale/vcpkg" -b "project0"
# Windows:
$ vcpkg\bootstrap-vcpkg.bat
# Non-Windows:
$ ./vcpkg/bootstrap-vcpkg.sh
# Both Windows and non-Windows:
$ git clone "https://github.com/SamuelMarks/cdd-c" && cd "cdd-c"  # Or your fork of this repo
# Windows
$ cmake -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_TOOLCHAIN_FILE="..\vcpkg\scripts\buildsystems\vcpkg.cmake" -S . -B "build"
# Non-Windows
$ cmake -DCMAKE_BUILD_TYPE='Debug' -DCMAKE_TOOLCHAIN_FILE='../vcpkg/scripts/buildsystems/vcpkg.cmake' -S . -B 'build'
# Both Windows and non-Windows:
$ cmake --build "build"
# Test
$ cd "build" && ctest -C "Debug" --verbose
```

---

## License

Licensed under either of

- Apache License, Version 2.0 ([LICENSE-APACHE](LICENSE-APACHE) or <https://www.apache.org/licenses/LICENSE-2.0>)
- MIT license ([LICENSE-MIT](LICENSE-MIT) or <https://opensource.org/licenses/MIT>)

at your option.

### Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted
for inclusion in the work by you, as defined in the Apache-2.0 license, shall be
dual licensed as above, without any additional terms or conditions.

#### pre-commit

This repository uses `clang-format` to maintain source code formatted in LLVM style.
Before committing for the first time, please install `pre-commit` on your system and then
execute the following command to install pre-commit hooks:
{{{
pre-commit install
}}}
