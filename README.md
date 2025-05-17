c-cdd
=====

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Linux, Windows, macOS](https://github.com/SamuelMarks/cdd-c/actions/workflows/linux-Windows-macOS.yml/badge.svg)](https://github.com/SamuelMarks/cdd-c/actions/workflows/linux-Windows-macOS.yml)
[`#rewriteInC`](https://rewriteInC.io)

Frontend for C, concentrating on: generation from code; single-file analysis; modification; and emission (to C code).

Use-cases:

- Generate/update `free` calling `int StructName_cleanup(struct StructName*)` functions, e.g.:
    - `struct Foo { struct Bar *bar;}; struct Bar { int a; };` will generate:
    - `int Bar_cleanup(struct Bar*);` &
    - `int Foo_cleanup(struct Foo*);` (which internally will call `Bar_cleanup`)
- With `fmt` of JSON, INI, YAML, TOML, &etc.:
    - Generate/update parsers from `const char*` to `struct`:
      `const int StructName_<fmt>_parse(struct StructName*, const char*)` function;
    - Generate/update emitters from `struct` to `char*`:
      `const int StructName_<fmt>_emit(const struct StructName*, char*)` function;
- And helper functions inspired by Haskell typeclasses & Rust traits like:
    - `bool StructName_eq(struct StructName*, struct StructName*);`
    - `int StructName_default(struct StructName*);`
    - `int StructName_deepcopy(const struct StructName*, struct StructName*);`
    - `int StructName_display(struct StructName*, FILE*);`
    - `int StructName_debug(struct StructName*, FILE*);`
- Generate Google Cloud client library for C (
  with [Google Cloud API Discovery Service](https://developers.google.com/discovery/v1/reference) as input);
- Generate arbitrary C client libraries (with [OpenAPI](https://spec.openapis.org/oas/v3.1.0) as input);
- Generating `#pragma` for every function to expose them for calling from,
  e.g., [assembly](https://www.ibm.com/docs/en/zos/2.5.0?topic=programs-calling-c-code-from-assembler-c-example);
- Prepare C SDK to be wrapped in higher-level language SDKs ([SWIG](https://en.wikipedia.org/wiki/SWIG) style)

## Why

There are four reasons to write in C:
0. Maintain existing codebase
1. Write low-level code (e.g., kernels, drivers, microcontrollers, compilers, assembly helpers)
2. Write performant code (e.g., databases, filesystems, math libraries)
3. Write interoperable code.

The focus of this venture is the final point… **interoperability**:

```mermaid
graph LR
%% Central node C
C["C<br/><img src='https://upload.wikimedia.org/wikipedia/commons/1/19/C_Logo.svg' width='40'/>"]

%% Languages connected to C (distance 1)
Cpp["C++<br/><img src='https://upload.wikimedia.org/wikipedia/commons/1/18/ISO_C%2B%2B_Logo.svg' width='40'/>"]
Csharp["C#<br/><img src='https://upload.wikimedia.org/wikipedia/commons/4/4f/Csharp_Logo.png' width='40'/>"]
ObjC["Objective-C<br/><img src='https://upload.wikimedia.org/wikipedia/commons/d/d7/Objective-C_logo.svg' width='40'/>"]
Java["Java<br/><img src='https://upload.wikimedia.org/wikipedia/en/3/30/Java_programming_language_logo.svg' width='40'/>"]
Python["Python<br/><img src='https://upload.wikimedia.org/wikipedia/commons/c/c3/Python-logo-notext.svg' width='40'/>"]
Go["Go<br/><img src='https://upload.wikimedia.org/wikipedia/commons/0/05/Go_Logo_Blue.svg' width='40'/>"]
Rust["Rust<br/><img src='https://upload.wikimedia.org/wikipedia/commons/d/d5/Rust_programming_language_black_logo.svg' width='40'/>"]
JavaScript["JavaScript<br/><img src='https://upload.wikimedia.org/wikipedia/commons/6/6a/JavaScript-logo.png' width='40'/>"]
Perl["Perl<br/><img src='https://upload.wikimedia.org/wikipedia/commons/9/9e/Perl_language_logo.svg' width='40'/>"]
PHP["PHP<br/><img src='https://upload.wikimedia.org/wikipedia/commons/2/27/PHP-logo.svg' width='40'/>"]
Swift["Swift<br/><img src='https://upload.wikimedia.org/wikipedia/commons/9/9d/Swift_logo.svg' width='40'/>"]
Kotlin["Kotlin<br/><img src='https://upload.wikimedia.org/wikipedia/commons/7/74/Kotlin_Icon.png' width='40'/>"]
Lua["Lua<br/><img src='https://upload.wikimedia.org/wikipedia/commons/c/cf/Lua-Logo.svg' width='40'/>"]
Dlang["D<br/><img src='https://upload.wikimedia.org/wikipedia/commons/e/e1/D_Programming_Language_logo.svg' width='40'/>"]
Scala["Scala<br/><img src='https://upload.wikimedia.org/wikipedia/commons/3/39/Scala-full-color.svg' width='40'/>"]
Bash["Bash<br/><img src='https://upload.wikimedia.org/wikipedia/commons/4/44/Gnu-bash-logo.svg' width='40'/>"]
Tcl["Tcl<br/><img src='https://upload.wikimedia.org/wikipedia/commons/0/00/Tcl_logo.svg' width='40'/>"]
Ruby["Ruby<br/><img src='https://upload.wikimedia.org/wikipedia/commons/7/73/Ruby_logo.svg' width='40'/>"]
Fortran["Fortran<br/><img src='https://upload.wikimedia.org/wikipedia/commons/3/31/Fortran_logo.svg' width='40'/>"]
Ada["Ada<br/><img src='https://upload.wikimedia.org/wikipedia/commons/e/e0/Ada_programming_language_logo.svg' width='40'/>"]
Haskell["Haskell<br/><img src='https://upload.wikimedia.org/wikipedia/commons/1/1c/Haskell-Logo.svg' width='40'/>"]
Elixir["Elixir<br/><img src='https://upload.wikimedia.org/wikipedia/commons/4/44/Elixir_logo.png' width='40'/>"]
Julia["Julia<br/><img src='https://upload.wikimedia.org/wikipedia/commons/1/1f/Julia_Programming_Language_Logo.svg' width='40'/>"]
Groovy["Groovy<br/><img src='https://upload.wikimedia.org/wikipedia/commons/3/35/Groovy_Logo_2018.svg' width='40'/>"]
Crystal["Crystal<br/><img src='https://upload.wikimedia.org/wikipedia/commons/6/64/Crystal_Programming_Language_Logo.svg' width='40'/>"]
Nim["Nim<br/><img src='https://upload.wikimedia.org/wikipedia/commons/6/6a/Nim_Logo.svg' width='40'/>"]
OCaml["OCaml<br/><img src='https://upload.wikimedia.org/wikipedia/commons/5/5a/OCaml_logo.svg' width='40'/>"]
V["V<br/><img src='https://upload.wikimedia.org/wikipedia/commons/8/85/V_Programming_Language_Icon.svg' width='40'/>"]
Eiffel["Eiffel<br/><img src='https://upload.wikimedia.org/wikipedia/commons/2/23/Eiffel_Logo.svg' width='40'/>"]
Fsharp["F#<br/><img src='https://upload.wikimedia.org/wikipedia/commons/5/5e/F_Sharp_logo.svg' width='40'/>"]
Typescript["TypeScript<br/><img src='https://upload.wikimedia.org/wikipedia/commons/f/f5/Typescript.svg' width='40'/>"]
Cobol["COBOL<br/><img src='https://upload.wikimedia.org/wikipedia/commons/9/9a/COBOL_Logo.svg' width='40'/>"]
SQL["SQL<br/><img src='https://upload.wikimedia.org/wikipedia/commons/8/87/Sql_data_base_with_logo.png' width='40'/>"]
Matlab["MATLAB<br/><img src='https://upload.wikimedia.org/wikipedia/commons/2/21/Matlab_Logo.png' width='40'/>"]
Lisp["Lisp<br/><img src='https://upload.wikimedia.org/wikipedia/commons/6/6a/Common_Lisp_Logo.svg' width='40'/>"]
Prolog["Prolog<br/><img src='https://upload.wikimedia.org/wikipedia/commons/9/9f/Prolog_logo.svg' width='40'/>"]
Dart["Dart<br/><img src='https://upload.wikimedia.org/wikipedia/commons/7/7e/Dart-logo.png' width='40'/>"]
Erlang["Erlang<br/><img src='https://upload.wikimedia.org/wikipedia/commons/1/11/Erlang_logo.svg' width='40'/>"]

%% Connections to C
C --> Cpp
C --> Csharp
C --> ObjC
C --> Java
C --> Python
C --> Go
C --> Rust
C --> JavaScript
C --> Perl
C --> PHP
C --> Swift
C --> Kotlin
C --> Lua
C --> Dlang
C --> Scala
C --> Bash
C --> Tcl
C --> Ruby
C --> Fortran
C --> Ada
C --> Haskell
C --> Elixir
C --> Julia
C --> Groovy
C --> Crystal
C --> Nim
C --> OCaml
C --> V
C --> Eiffel
C --> Fsharp
C --> Typescript
C --> Cobol
C --> SQL
C --> Matlab
C --> Lisp
C --> Prolog
C --> Dart
C --> Erlang
```

## Design

This C compiler has a very unusual design, the macro and C language are treated as one. The foci are:

- location start/end of function, `struct`, and feature macros (e.g., `#ifdef JSON_EMIT` then `#endif /* JSON_EMIT */`);
- `struct` fields.

…which enable a number of use-cases to be simply developed, e.g., see the list above.

### Drawbacks

- Macros aren't evaluated, which means a simple `#define LBRACE {` will break cdd-c.
- Like [`m4`](https://en.wikipedia.org/wiki/M4_(computer_language)), CMake, and other tools; cdd-c must be run before
  your code is built.

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

## Command-Line Interface (CLI)

### `c_cdd_cli <command> [args]`

    code2schema <header.h> <schema.json>
    jsonschema2tests <schema.json> <test.c>
    schema2code <schema.json> <basename>
    sync_code <header.h> <impl.c>

### `code2schema`

    Usage: code2schema <header.h> <schema.json>

Generates JSON Schema from `struct`s located in specified C/C++ header or source file.

### `jsonschema2tests`

    Usage: jsonschema2tests <schema.json> <test.c>

Generates [`greatest.h`](https://github.com/silentbicycle/greatest/blob/release/greatest.h) based tests for the generated code.

### `schema2code`

    Usage: schema2code <schema.json> <basename>

Generates `struct`s from JSON Schema to specified C/C++ basename (that will generate header [basename.h] and source
files [basename.c] for).

### `sync_code`

    Usage: sync_code <header.h> <impl.c>

Update existing source code based on [potentially] handwritten changes to the code, e.g., changing the `struct` fields
by hand should update all the `to_json`, `from_json`, `eq`, `to_str`, and `from_str` functions.

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
