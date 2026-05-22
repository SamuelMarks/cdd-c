cdd-c
=====
[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![interactive WASM web demo](https://img.shields.io/badge/interactive-WASM_web_demo-blue.svg)](https://offscale.io/wasm_web_demo)
[![CI](https://github.com/SamuelMarks/cdd-c/actions/workflows/ci.yml/badge.svg)](https://github.com/SamuelMarks/cdd-c/actions)
[![Test Coverage](https://img.shields.io/badge/test_coverage-100%25-brightgreen.svg)](#)
[![Doc Coverage](https://img.shields.io/badge/doc_coverage-100%25-brightgreen.svg)](#)

----

OpenAPI ↔ C. This is one compiler in a suite, all focussed on the same task: Compiler Driven Development (CDD).

Each compiler is written in its target language, is whitespace and comment sensitive, and has both an SDK and CLI.

The core philosophy of Compiler Driven Development (CDD) is synchronization without compromise. Where traditional generators silo your API boundaries into read-only files, this compiler natively merges changes into your codebase via a robust, [whitespace and comment aware] Abstract Syntax Tree (AST) driven parser & emitter. It bridges the gap between design and implementation, allowing you to seamlessly generate SDKs from a spec or extract a spec from existing code. By keeping your APIs, SDKs, and tests in continuous, automated alignment, it drastically improves both delivery speed and software reliability.

The CLI—at a minimum—has:

- `cdd-c --help`
- `cdd-c --version`
- `cdd-c from_openapi to_sdk_cli -i spec.json`
- `cdd-c from_openapi to_sdk -i spec.json`
- `cdd-c from_openapi to_server -i spec.json`
- `cdd-c to_openapi -f path/to/code`
- `cdd-c to_docs_json --no-imports --no-wrapping -i spec.json`
- `cdd-c serve_json_rpc --port 8080 --listen 0.0.0.0`

## SDK Example

```c
#include "cdd_api.h"
#include <stdio.h>

int main() {
    cdd_config config = {
        .input_path = "spec.json",
        .output_dir = "src/models"
    };
    cdd_generate_sdk(&config);
    printf("SDK generation complete.\n");
    return 0;
}
```

## Installation

```bash
cmake -B build && cmake --build build
```

## Development

You can use standard tooling commands or the included cross-platform Makefiles to fetch dependencies, build, and test:

```bash
cmake -B build && cmake --build build
ctest --test-dir build
# or
make deps
make build
make test
# or on Windows
.\make.bat deps
.\make.bat build
.\make.bat test
```

See [PUBLISH.md](PUBLISH.md) for packaging and releasing.

## Features

The `cdd-c` compiler leverages a unified architecture to support various facets of API and code lifecycle management. For a deep dive into the compiler's design, see [ARCHITECTURE.md](ARCHITECTURE.md).

- **Compilation**:
    - **OpenAPI → `C`**: Generate idiomatic native models, network routes, client SDKs, and boilerplate directly from OpenAPI (`.json` / `.yaml`) specifications.
    - **`C` → OpenAPI**: Statically parse existing `C` source code and emit compliant OpenAPI specifications.
- **AST-Driven & Safe**: Employs static analysis instead of unsafe dynamic execution or reflection, allowing it to safely parse and emit code even for incomplete or un-compilable project states.
- **Seamless Sync**: Keep your docs, tests, database, clients, and routing in perfect harmony. Update your code, and generate the docs; or update the docs, and generate the code.

**Uncommon Features:**

`cdd-c` is strictly a **superset** of the standard `cdd-${LANGUAGE}` features. In addition to standard OpenAPI code generation, it features a native, lossless C AST/CST framework. This enables powerful code transformations (like standardizing GNU extensions, porting to MSVC, and safe CRT rewriting) and memory safety auditing natively.

- **Lossless AST Manipulation (Zero-Destruction Rule):** Perfectly captures and preserves "trivia"—inline comments, block comments, and exact whitespace (indentation, newlines).
- **Built-in Security & Auditing:** Utilize the built-in `audit` command to run static analysis directly over your C directories. Detect dangerous CRT functions (`strcpy`, `sprintf`), dangling pointer risks, and missing initializations before they hit production.
- **Safe CRT Transformation:** Automatically enforce safety by running the `safe_crt` transformer, which intelligently rewrites legacy string and memory operations to use bounds-checked equivalents (e.g., `strcpy_s`, `sprintf_s`).

---

## CLI Options

Usage: `cdd-c <command> [args]`

### `from_openapi`

Generate C SDK, Server, and optionally CLI from OpenAPI spec.

```shell
cdd-c from_openapi to_sdk -i <spec.json> [-o <dir>]
cdd-c from_openapi to_sdk --input-dir <specs_dir> [-o <dir>]
cdd-c from_openapi to_sdk_cli -i <spec.json> [-o <dir>]
cdd-c from_openapi to_sdk_cli --input-dir <specs_dir> [-o <dir>]
cdd-c from_openapi to_server -i <spec.json> [-o <dir>]
cdd-c from_openapi to_server --input-dir <specs_dir> [-o <dir>]
```

### `to_openapi`

Generate OpenAPI spec from C source code.

```shell
cdd-c to_openapi -i <dir> [-o <out.json>]
```

### `to_docs_json`

Generate JSON code examples for doc sites.

```shell
cdd-c to_docs_json [--no-imports] [--no-wrapping] -i|--input <spec.json>
```

### `audit`

Scan directory for memory safety issues.

```shell
cdd-c audit <directory>
```

### `c2openapi`

Generate OpenAPI spec from C source code.

```shell
cdd-c c2openapi <dir> <out.json>
```

### `transformer`

Run syntax tree transformations.

```shell
cdd-c transformer <toolname> [--audit|--fix] [--dry-run] <files...>
```

### `code2schema`

Convert C header to JSON Schema.

```shell
cdd-c code2schema <header.h> <schema.json>
```

### `generate_build_system`

Generate build system files.

```shell
cdd-c generate_build_system <type> <out_dir> <name> [test_file]
```

### `schema2code`

Generate C code from JSON schema.

```shell
cdd-c schema2code <schema.json> <out_dir>
```

## Extensive Features & Functionality

`cdd-c` is not just a standard parser; it is a full-fledged **Compiler Driven Development (CDD)** suite tailored specifically for `C` (strictly targeting ISO C90 compliance). It deeply understands C down to its comments and whitespace, treating codebase refactoring, code generation, and API alignment as first-class, lossless operations.

### 1. Lossless AST Manipulation (Zero-Destruction Rule)
Unlike standard preprocessors or regex-based refactoring tools, `cdd-c` uses a custom, highly robust **Concrete Syntax Tree (CST)** and **Abstract Syntax Tree (AST)** pipeline.
- **Trivia Preservation:** It perfectly captures and preserves "trivia"—inline comments, block comments, and exact whitespace (indentation, newlines).
- **Format-Safe Rewrites:** When you mutate a tree (for instance, converting a GNU compiler extension into a standard C89 equivalent), `cdd-c` applies the change surgically. Your code does not get butchered, formatted entirely differently, or stripped of its documentation. The generated ISO C code is meant for human developers, not just compilers.

### 2. Multi-Directional Code Generation
`cdd-c` acts as the master sync tool between your abstractions:
- **API to Native C (SDK & Server):** Generate production-ready, memory-safe C clients, routing servers, and CLI argument parsers straight from OpenAPI JSON/YAML.
- **Native C to OpenAPI:** Write your C functions, document them with standard block comments, and let `cdd-c` statically analyze the types and routes to reverse-engineer a perfectly compliant OpenAPI 3.x spec.
- **SQL / JSON Schema Sync:** Seamlessly sync SQL DDL statements to C ORM layers, or JSON schemas directly into C structs and validation logic.

### 3. Built-in Security & Auditing
- **Memory Safety Audits:** Utilize the built-in `audit` command to run static analysis directly over your C directories. Detect dangerous CRT functions (`strcpy`, `sprintf`), dangling pointer risks, and missing initializations before they hit production.
- **Safe CRT Transformation:** Automatically enforce safety by running the `safe_crt` transformer, which intelligently rewrites legacy string and memory operations to use bounds-checked equivalents (e.g., `strcpy_s`, `sprintf_s`).

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
