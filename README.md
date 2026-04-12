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
- `cdd-c from_openapi -i spec.json`
- `cdd-c to_openapi -f path/to/code`
- `cdd-c to_docs_json --no-imports --no-wrapping -i spec.json`
- `cdd-c serve_json_rpc --port 8082 --listen 0.0.0.0`
- `cdd-c from_openapi to_sdk_cli -i spec.json -o target_directory`
- `cdd-c from_openapi to_sdk -i spec.json -o target_directory`
- `cdd-c from_openapi to_server -i spec.json -o target_directory`
- `cdd-c transformer extern_c --audit src/my_file.c`
- `cdd-c transformer msvc_port --fix src/my_file.c`

The goal of this project is to enable rapid application development without tradeoffs. Tradeoffs of Protocol Buffers / Thrift etc. are an untouchable "generated" directory and package, compile-time and/or runtime overhead. Tradeoffs of Java or JavaScript for everything are: overhead in hardware access, offline mode, ML inefficiency, and more. And neither of these alternative approaches are truly integrated into your target system, test frameworks, and bigger abstractions you build in your app. Tradeoffs in CDD are code duplication (but CDD handles the synchronisation for you).

## 🚀 Capabilities

The `cdd-c` compiler leverages a unified architecture to support various facets of API and code lifecycle management.

- **Compilation**:
  - **OpenAPI → `C`**: Generate idiomatic native models, network routes, client SDKs, database schemas, and boilerplate directly from OpenAPI (`.json` / `.yaml`) specifications.
  - **`C` → OpenAPI**: Statically parse existing `C` source code and emit compliant OpenAPI specifications.
  - **`C` → `C` (CST Transforms)**: Natively parse, analyze, and safely refactor `C` syntax (e.g. standardizing GNU extensions, porting to MSVC, percolating errors) with byte-for-byte lossless formatting.
- **AST-Driven & Safe**: Employs static analysis (Abstract Syntax Trees via a custom whitespace, comment, and macro sensitive parser) instead of unsafe dynamic execution or reflection, allowing it to safely parse and emit code even for incomplete or un-compilable project states.
- **Seamless Sync**: Keep your docs, tests, database, clients, and routing in perfect harmony. Update your code, and generate the docs; or update the docs, and generate the code.

## 📦 Installation

Requires a C compiler (GCC/Clang/MSVC) and CMake.

```bash
# Ubuntu / Debian
sudo apt-get install gcc cmake pkg-config

# Build the CLI
make build

# The CLI binary is placed in bin/
./bin/cdd-c --help
```

## 🛠 Usage

### Command Line Interface

```bash
# Generate C models from an OpenAPI spec
./bin/cdd-c from_openapi to_sdk -i spec.json -o src/models

# Generate an OpenAPI spec from your C code
./bin/cdd-c to_openapi -f src/models/my_model.c -o openapi.json
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

## Design choices

The `cdd-c` project uses a fully custom whitespace, comment, and macro sensitive parser for Lexing and Parsing C, enabling highly robust, standardized syntax tree creation without relying on complex, heavy external C/C++ parsers like Clang AST for everything, ensuring it stays lightweight.

## 🏗 Supported Conversions for C

_(The boxes below reflect the features supported by this specific `cdd-c` implementation)_

| Concept                       | Parse (From) | Emit (To) |
| ----------------------------- | ------------ | --------- |
| OpenAPI (JSON/YAML)           | ✅           | ✅        |
| `C` Models / Structs / Types  | ✅           | ✅        |
| `C` Server Routes / Endpoints | ✅           | ✅        |
| `C` API Clients / SDKs        | ✅           | ✅        |
| `C` ORM / DB Schemas          | [ ]          | ✅        |
| `C` CLI Argument Parsers      | ✅           | ✅        |
| `C` Docstrings / Comments     | ✅           | ✅        |
| `C` AST Transformation (CST)  | ✅           | ✅        |

WASM Support: Possible ✅ | Implemented ✅

---

## CLI Help

```
$ ./build_cmake/bin/cdd-c --help
Usage: ./build_cmake/bin/cdd-c <command> [args]

Commands:
  from_openapi to_sdk -i <spec.json> [-o <dir>]
  from_openapi to_sdk --input-dir <specs_dir> [-o <dir>]
  from_openapi to_sdk_cli -i <spec.json> [-o <dir>]
  from_openapi to_sdk_cli --input-dir <specs_dir> [-o <dir>]
  from_openapi to_server -i <spec.json> [-o <dir>]
  from_openapi to_server --input-dir <specs_dir> [-o <dir>]
      Generate C SDK, Server, and optionally CLI from OpenAPI spec.
  to_openapi -i <dir> [-o <out.json>]
      Generate OpenAPI spec from C source code.
  to_docs_json [--no-imports] [--no-wrapping] -i|--input <spec.json>
      Generate JSON code examples for doc sites.
  audit <directory>
      Scan directory for memory safety issues.
  c2openapi <dir> <out.json>
      Generate OpenAPI spec from C source code.
  transformer <toolname> [--audit|--fix] [--dry-run] <files...>
      Run syntax tree transformations.
  code2schema <header.h> <schema.json>
      Convert C header to JSON Schema.
  generate_build_system <type> <out_dir> <name> [test_file]
      Generate build system files.
  schema2code <schema.json> <out_dir>
      Generate C code from JSON schema.
  sql2c <schema.sql> <out_dir>
      Generate C code (c-orm compatible) from SQL DDL.
  jsonschema2tests <schema.json> <header_to_test.h> <out.h>
      Generate C tests from JSON schema.
  migrate <up|down|create> [args...]
      Manage database migrations.
  db reset
      Drop and recreate the database schema, then run UP migrations.
  schema dump [schema.sql]
      Dump the current database schema state.
  seed [seeds.sql]
      Seed the database with test data.
  setup_test_db [db_name]
      Setup a test database dynamically in CI mode.
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

## 🛠 Extending `cdd-c` with Transformers

The true power of `cdd-c` is its extensibility. You can write your own standalone, AST-aware transformers that modify C codebases directly while perfectly preserving all formatting, macros, and comments. 

### What is a Transformer?
A transformer takes a parsed `cdd_cst_tree_t`, analyzes its geometrical and semantic nodes, and mutates the tree in place. It powers the `cdd-c transformer <toolname> --fix <files...>` CLI command.

### Built-in Transformers
`cdd-c` ships with several highly complex transformers ready out of the box:
- **`gnu_standardizer`**: An exhaustive engine that lowers complex GNU C extensions (like `__int128`, `__complex__`, statement expressions, computed gotos, and VLAs) into strictly compliant ISO C89 code.
- **`msvc_port`**: Automatically patches syntax, header guards, and type declarations to compile cleanly under Microsoft Visual C++ (MSVC).
- **`extern_c`**: Intelligently wraps headers in `extern "C" { ... }` blocks for flawless C++ interoperability.
- **`error_percolator`**: Analyzes function return types and automatically propagates nested error enums and exit codes up the call stack.
- **`safe_crt`**: Upgrades standard library calls to their bounds-checked, secure MSVC equivalents.

### How to Write Your Own Transformer

Want to enforce a new coding standard, deprecate an old API project-wide, or implement a custom language feature? You can plug directly into the `cdd-c` mutator framework.

1. **Create the File:** Create a new directory and source file under `src/transformers/my_tool/my_tool.c`.
2. **Include the Headers:** Include the core AST and mutator API headers (`classes/parse/cdd_cst_query.h`, `classes/parse/cdd_cst_mutate.h`).
3. **Define the Signature:** Implement the standard transformer prototype:
   ```c
   int cdd_transform_my_tool(cdd_cst_tree_t *tree, const cdd_transform_config_t *config);
   ```
4. **Mutate the Tree:** Use `cdd_cst_find_nodes_by_type` to locate your target expressions. You can deeply inspect the `cdd_cst_node_t` structures, check their lexical scopes, or evaluate their compile-time types. Use `cdd_cst_node_replace` or `cdd_cst_node_insert_after` to safely alter the tree.
5. **Register the Tool:** Add your prototype to `include/cdd_cst_transform.h` and map it to the CLI in `src/routes/parse/cli_cst.c`.
6. **Compile & Run:** Your tool is now available natively via:
   ```bash
   cdd-c transformer my_tool --fix src/**/*.c
   ```

Because `cdd-c` handles all the Lexing, CST generation, trivia management, and string emitting, your custom transformer only needs to focus on the raw logic of the C tree.

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
