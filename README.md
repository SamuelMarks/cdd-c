# cdd-c

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![CI](https://github.com/SamuelMarks/cdd-c/actions/workflows/ci.yml/badge.svg)](https://github.com/SamuelMarks/cdd-c/actions/workflows/ci.yml)
[![Doc Coverage](https://img.shields.io/badge/docs-100%25-brightgreen.svg)](#)
[![Test Coverage](https://img.shields.io/badge/coverage-100%25-brightgreen.svg)](#)

OpenAPI ↔ C. This is one compiler in a suite, all focussed on the same task: Compiler Driven Development (CDD).

Each compiler is written in its target language, is whitespace and comment sensitive, and has both an SDK and CLI.

The CLI—at a minimum—has:

- `cdd-c --help`
- `cdd-c --version`
- `cdd-c from_openapi to_sdk_cli -i spec.json`
- `cdd-c from_openapi to_sdk -i spec.json`
- `cdd-c from_openapi to_server -i spec.json`
- `cdd-c to_openapi -f path/to/code`
- `cdd-c to_docs_json --no-imports --no-wrapping -i spec.json`
- `cdd-c serve_json_rpc --port 8080 --listen 0.0.0.0`

The goal of this project is to enable rapid application development without tradeoffs. Tradeoffs of Protocol Buffers / Thrift etc. are an untouchable "generated" directory and package, compile-time and/or runtime overhead. Tradeoffs of Java or JavaScript for everything are: overhead in hardware access, offline mode, ML inefficiency, and more. And neither of these alternative approaches are truly integrated into your target system, test frameworks, and bigger abstractions you build in your app. Tradeoffs in CDD are code duplication (but CDD handles the synchronisation for you).

## 🚀 Capabilities

The `cdd-c` compiler leverages a unified architecture to support various facets of API and code lifecycle management.

- **Compilation**:
    - **OpenAPI → `C`**: Generate idiomatic native models, network routes, client SDKs, and boilerplate directly from OpenAPI (`.json` / `.yaml`) specifications.
    - **`C` → OpenAPI**: Statically parse existing `C` source code and emit compliant OpenAPI specifications.
- **AST-Driven & Safe**: Employs static analysis instead of unsafe dynamic execution or reflection, allowing it to safely parse and emit code even for incomplete or un-compilable project states.
- **Seamless Sync**: Keep your docs, tests, database, clients, and routing in perfect harmony. Update your code, and generate the docs; or update the docs, and generate the code.

## 📦 Installation & Build

### Native Tooling

```bash
cmake -B build && cmake --build build
ctest --test-dir build
```

### Makefile / make.bat

You can also use the included cross-platform Makefiles to fetch dependencies, build, and test:

```bash
# Install dependencies
make deps

# Build the project
make build

# Run tests
make test
```

## 🛠 Usage

### Command Line Interface

```bash
# Generate C models from an OpenAPI spec
cdd-c from_openapi to_sdk -i spec.json -o src/models

# Generate an OpenAPI spec from your C code
cdd-c to_openapi -f src/models -o openapi.json
```

### Programmatic SDK / Library

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

## 🏗 Supported Conversions for C

*(The boxes below reflect the features supported by this specific `cdd-c` implementation)*

| Features | Parse (From) | Emit (To) |
| --- | --- | --- |
| OpenAPI 3.2.0 | ✅ | ✅ |
| API Client SDK | ✅ | ✅ |
| API Client CLI | ✅ | ✅ |
| Server Routes / Endpoints | ✅ | ✅ |
| ORM / DB Schema | [ ] | [ ] |
| Mocks + Tests | [ ] | [ ] |
| Model Context Protocol (MCP) | [ ] | [ ] |

### Uncommon Features

`cdd-c` is strictly a **superset** of the standard `cdd-${LANGUAGE}` features. In addition to standard OpenAPI code generation, it features a native, lossless C AST/CST framework. This enables powerful code transformations (like standardizing GNU extensions, porting to MSVC, and safe CRT rewriting) and memory safety auditing natively.

- **Lossless AST Manipulation (Zero-Destruction Rule):** Perfectly captures and preserves "trivia"—inline comments, block comments, and exact whitespace (indentation, newlines).
- **Built-in Security & Auditing:** Utilize the built-in `audit` command to run static analysis directly over your C directories. Detect dangerous CRT functions (`strcpy`, `sprintf`), dangling pointer risks, and missing initializations before they hit production.
- **Safe CRT Transformation:** Automatically enforce safety by running the `safe_crt` transformer, which intelligently rewrites legacy string and memory operations to use bounds-checked equivalents (e.g., `strcpy_s`, `sprintf_s`).

---

## CLI Help

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

### `sql2c`

Generate C code (c-orm compatible) from SQL DDL.

```shell
cdd-c sql2c <schema.sql> <out_dir>
```

### `jsonschema2tests`

Generate C tests from JSON schema.

```shell
cdd-c jsonschema2tests <schema.json> <header_to_test.h> <out.h>
```

### `migrate`

Manage database migrations.

```shell
cdd-c migrate <up|down|create> [args...]
```

### `db`

Drop and recreate the database schema, then run UP migrations.

```shell
cdd-c db reset
```

### `schema`

Dump the current database schema state.

```shell
cdd-c schema dump [schema.sql]
```

### `seed`

Seed the database with test data.

```shell
cdd-c seed [seeds.sql]
```

### `setup_test_db`

Setup a test database dynamically in CI mode.

```shell
cdd-c setup_test_db [db_name]
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
