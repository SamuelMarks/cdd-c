cdd-c
=====
[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Doc Coverage](https://img.shields.io/badge/Doc%20Coverage-100.0%25-brightgreen.svg)](#)
[![Test Coverage](https://img.shields.io/badge/Test%20Coverage-100.0%25-brightgreen.svg)](#)
[![interactive WASM web demo](https://img.shields.io/badge/interactive-WASM_web_demo-blue.svg)](https://offscale.io/wasm_web_demo)
[![CI](https://github.com/SamuelMarks/cdd-c/actions/workflows/ci.yml/badge.svg)](https://github.com/SamuelMarks/cdd-c/actions)

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
- `cdd-c serve_json_rpc --port 8080 --listen 127.0.0.1`

## SDK Example

```c
#include "cdd_api.h"
#include "cdd_c_error.h"
#include <stdio.h>

int main() {
    cdd_c_error_t rc;
    cdd_config config = {
        .input_path = "spec.json",
        .output_dir = "src/models"
    };

    rc = cdd_generate_sdk(&config);
    if (rc != CDD_C_SUCCESS) {
        fprintf(stderr, "SDK generation failed with error code: %d\n", rc);
        return rc;
    }

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
- **Universal FFI Bindings**: Using the `bind` command, leverage a normalized Intermediate Representation (IR) to statically analyze C code and generate native bindings for 40+ languages (Python, Rust, C#, etc.) without writing manual SWIG rules.
- **Model Context Protocol (MCP)**: Run `cdd-c serve_json_rpc` to expose an MCP-compliant server via stdio or HTTP, allowing LLMs to seamlessly interface with your C codebase, understand types, and interact with the compiler.
- **AST-Driven & Safe**: Employs static analysis instead of unsafe dynamic execution or reflection, allowing it to safely parse and emit code even for incomplete or un-compilable project states.
- **Seamless Sync**: Keep your docs, tests, database, clients, and routing in perfect harmony. Update your code, and generate the docs; or update the docs, and generate the code.

**Uncommon Features:**

`cdd-c` is strictly a **superset** of the standard `cdd-${LANGUAGE}` features. In addition to standard OpenAPI code generation, it features a native, lossless C Concrete Syntax Tree (CST) and Abstract Syntax Tree (AST) engine targeting strict ISO C90 compliance.

> 📖 **Deep-Dive Reference**: For an exhaustive technical breakdown of every transformer phase, language emitter, AST query API, and the Model Context Protocol, see [**CAPABILITIES.md**](CAPABILITIES.md).

- **Lossless AST Manipulation (Zero-Destruction Rule):** Unlike standard preprocessors or regex-based rewriters, `cdd-c` preserves 100% of "trivia"—leading/trailing whitespace, exact indentation, single-line comments (`//`), block comments (`/* ... */`), and Doxygen docstrings. When nodes are transformed, their trivia remains anchored to the syntax tree.
- **Source-to-Source AST Transformation Suite (`transformer` & `standardize-gnu`):**
  - **`gnu_standardizer`**: Methodically lowers non-standard GNU C extensions across 8 phases (including `__int128`, `typeof`, Variable Length Arrays, zero-length struct arrays, `__complex__`, vector types, statement expressions `({ ... })`, case ranges `case 1 ... 5:`, omitted conditionals `x ? : y`, `cleanup` attribute unwinding, and bitwise `__builtin_*` intrinsics) into strictly compliant ISO C90 (C89) or C99 code.
  - **`safe_crt`**: Automatically scans and rewrites vulnerable legacy C runtime calls (`strcpy`, `strcat`, `sprintf`, `gets`, `memcpy`) to bounds-checked ISO/IEC TR 24731-1 Safe CRT equivalents (`strcpy_s`, `sprintf_s`, `memcpy_s`) with static buffer capacity inference.
  - **`msvc_port`**: Transpiles POSIX/GNU primitives (`strcasecmp` -> `_stricmp`, `strdup` -> `_strdup`, `ssize_t` -> `SSIZE_T`, `off_t` -> `_off_t`) and injects required MSVC headers (`BaseTsd.h`).
  - **`extern_c`**: Detects unprotected C declarations and surgically wraps headers with `#ifdef __cplusplus \n extern "C" { \n #endif` guards.
  - **`error_percolator`**: Refactors legacy functions returning raw data or `void` into robust error-handling architectures returning `cdd_c_error_t`, promoting original return types to typed out-pointers and rewriting all caller sites.
  - **`macros`**: Expands function-like macros and macro operators (`#` stringification, `##` token concatenation) directly into canonical AST nodes for downstream inspection without external preprocessors.
- **Universal Multi-Language FFI Generator (`bind`):**
  - Generates native, idiomatic bindings for **42 programming languages and targets** (Rust, Python, Go, TypeScript, C#, Java, Swift, Kotlin, Zig, Odin, Dart, Ruby, PHP, Lua, Haskell, OCaml, Elixir, Erlang, WebAssembly, and more) from C headers without writing manual SWIG rules.
  - Features an format-specifier analyzer (`cdd_ffi_variadic`) that parses `printf`/`scanf` format strings to synthesize strongly typed trampoline functions for languages without native C variadic support.
  - Includes a human-readable, lossless **FFI Intermediate Representation DSL** with left-to-right type composition and first-class memory data-flow intents (`[in]`, `[out]`, `[inout]`, `len=target`). See [**DSL.md**](DSL.md).
- **Native Model Context Protocol (MCP) & AI Gateway:**
  - **Meta-MCP Server (`mcp`)**: Runs an MCP-compliant JSON-RPC server via stdio, exposing compiler tools (`cdd_generate`, `cdd_inspect`, `cdd_sync`) directly to local AI assistants (Claude Desktop, Cursor, Windsurf).
  - **Generated Client MCP Adapters**: Automatically equips generated C client SDKs with native MCP adapters, allowing LLMs to discover and call OpenAPI endpoints as tools (`tools/list`, `tools/call`, `resources/list`, `prompts/list`). See [**mcp-1.0.0/**](mcp-1.0.0/).
- **Built-in Static Security & Memory Safety Auditing (`audit`):** Scans C source trees to detect unchecked dynamic allocations (`malloc`, `calloc`, `realloc`, `strdup`), missing NULL pointer guards, unsafe CRT functions, and dangling pointer escapes, emitting structured JSON reports.
- **Bidirectional Schema & Database Migration Suite:**
  - **`code2schema` & `schema2code`**: Convert bidirectionally between C struct headers and JSON Schema definitions (OpenAPI 3.2.0 / draft 2020-12).
  - **`to_docs_json`**: Generates full documentation matrices with runnable, self-contained C client snippets for every HTTP verb.
  - **`client_gui_gen`**: Generates native Material 3 / C-based desktop OAuth2 authentication dialogs and views.
  - **Database Migrations & `c-orm`**: Manages forward and rollback migrations (`cdd-c migrate up|down|create`), CI test database setup (`setup_test_db`, `db reset`, `seed`), and live schema extraction (`schema dump`) for PostgreSQL, SQLite3, and MySQL/MariaDB. See [**USAGE.md**](USAGE.md).
- **Cross-Platform Scaffolding & WebAssembly Runtime:**
  - Automatically generates platform-aware `CMakeLists.txt` build definitions (WinHTTP vs libcurl).
  - Compiles cleanly to WebAssembly via Emscripten for zero-dependency execution in web browsers and Node.js. See [**WASM.md**](WASM.md).

---

## CLI Options

```text
Usage: cdd-c [OPTIONS] <COMMAND>

Commands:
  from_openapi to_sdk -i <spec.json> [-o <dir>] [--no-github-actions] [--no-installable-package] [--tests]
  from_openapi to_sdk --input-dir <specs_dir> [-o <dir>] [--no-github-actions] [--no-installable-package] [--tests]
  from_openapi to_sdk_cli -i <spec.json> [-o <dir>] [--no-github-actions] [--no-installable-package] [--tests]
  from_openapi to_sdk_cli --input-dir <specs_dir> [-o <dir>] [--no-github-actions] [--no-installable-package] [--tests]
  from_openapi to_server -i <spec.json> [-o <dir>] [--no-github-actions] [--no-installable-package] [--tests]
  from_openapi to_server --input-dir <specs_dir> [-o <dir>] [--no-github-actions] [--no-installable-package] [--tests]
      Generate code from an OpenAPI specification.
  to_openapi -i <dir> [-o <out.json>]
      Generate an OpenAPI specification from source code.
  to_docs_json [--no-imports] [--no-wrapping] -i|--input <spec.json> [-o <docs.json>]
      Generate JSON documentation with code snippets for an OpenAPI specification.
  bind [OPTIONS]
      Generate SWIG-like FFI bindings for 42 languages.
  serve_json_rpc [-p|--port <int>] [-l|--listen <address>]
      Expose CLI interface as a JSON-RPC server.
  mcp
      Expose CLI interface as an MCP server via stdio.

Language-Specific Commands:
  audit <directory>
      Scan directory for memory safety issues.
  c2openapi <dir> <out.json>
      Generate OpenAPI spec from C source code.
  standardize-gnu [OPTIONS] <files...>
      Standardize GNU C extensions to ISO C (C89/C99).
  transformer <toolname> [--audit|--fix] [--dry-run] <files...>
      Run syntax tree transformations (extern_c, msvc_port, gnu_standardizer, error_percolator, safe_crt).
  code2schema <header.h> <schema.json>
      Convert C header to JSON Schema.
  generate_build_system <type> <out_dir> <name> [test_file]
      Generate build system files.
  schema2code <schema.json> <out_dir>
      Generate C code from JSON schema.
```

### `from_openapi`

Generate C SDK, Server, and optionally CLI from OpenAPI spec.

```text
Usage: cdd-c from_openapi [to_sdk|to_sdk_cli|to_server] [args]

Commands:
  to_sdk         Generate a Client SDK from an OpenAPI specification.
  to_sdk_cli     Generate a Client SDK CLI from an OpenAPI specification.
  to_server      Generate a Server stub from an OpenAPI specification.

Options:
  -i, --input <spec.json>   Input OpenAPI spec file
  --input-dir <specs_dir>   Input directory containing OpenAPI specs
  -o, --output <dir>        Output directory
  --no-github-actions       Do not generate GitHub Actions CI workflow
  --no-installable-package  Do not generate build system files (e.g. CMakeLists.txt)
  --tests                   Generate composable tests and mocks
```

#### `from_openapi to_sdk`

```text
Usage: cdd-c from_openapi [to_sdk|to_sdk_cli|to_server] [args]

Commands:
  to_sdk         Generate a Client SDK from an OpenAPI specification.
  to_sdk_cli     Generate a Client SDK CLI from an OpenAPI specification.
  to_server      Generate a Server stub from an OpenAPI specification.

Options:
  -i, --input <spec.json>   Input OpenAPI spec file
  --input-dir <specs_dir>   Input directory containing OpenAPI specs
  -o, --output <dir>        Output directory
  --no-github-actions       Do not generate GitHub Actions CI workflow
  --no-installable-package  Do not generate build system files (e.g. CMakeLists.txt)
  --tests                   Generate composable tests and mocks
```

#### `from_openapi to_sdk_cli`

```text
Usage: cdd-c from_openapi [to_sdk|to_sdk_cli|to_server] [args]

Commands:
  to_sdk         Generate a Client SDK from an OpenAPI specification.
  to_sdk_cli     Generate a Client SDK CLI from an OpenAPI specification.
  to_server      Generate a Server stub from an OpenAPI specification.

Options:
  -i, --input <spec.json>   Input OpenAPI spec file
  --input-dir <specs_dir>   Input directory containing OpenAPI specs
  -o, --output <dir>        Output directory
  --no-github-actions       Do not generate GitHub Actions CI workflow
  --no-installable-package  Do not generate build system files (e.g. CMakeLists.txt)
  --tests                   Generate composable tests and mocks
```

#### `from_openapi to_server`

```text
Usage: cdd-c from_openapi [to_sdk|to_sdk_cli|to_server] [args]

Commands:
  to_sdk         Generate a Client SDK from an OpenAPI specification.
  to_sdk_cli     Generate a Client SDK CLI from an OpenAPI specification.
  to_server      Generate a Server stub from an OpenAPI specification.

Options:
  -i, --input <spec.json>   Input OpenAPI spec file
  --input-dir <specs_dir>   Input directory containing OpenAPI specs
  -o, --output <dir>        Output directory
  --no-github-actions       Do not generate GitHub Actions CI workflow
  --no-installable-package  Do not generate build system files (e.g. CMakeLists.txt)
  --tests                   Generate composable tests and mocks
```

### `to_openapi`

Generate OpenAPI spec from C source code.

```text
Usage: cdd-c to_openapi [args]

Options:
  -i, --input <dir>       Input directory containing C source code
  -o, --output <out.json> Output OpenAPI spec file (default: openapi.json)
```

### `to_docs_json`

Generate JSON code examples for doc sites.

```text
Usage: cdd-c to_docs_json [--no-imports] [--no-wrapping] -i|--input <spec.json> [-o <docs.json>]
```

### `bind`

Generate SWIG-like FFI bindings for 40 languages.

```text
Usage: cdd-c bind [OPTIONS]

Options:
  -i, --input <file|dir>    Input C header/source or directory
  -o, --output-dir <dir>    Output directory for bindings
  -l, --lang <langs>        Comma-separated list of languages or '*' (e.g., python,rust)
  -n, --lib-name <name>     Name of the shared library (e.g., sqlite3)
  -m, --module-name <name>  Name of the generated namespace/module
  --skip-static             Skip static inline functions
  --opaque-pointers         Treat unknown structs as void* (opaque)
  --generate-tests          Generate basic sanity-check tests
  -h, --help                Show this help message
```

### `serve_json_rpc`

Expose CLI interface as a JSON-RPC server.

```text
Usage: cdd-c serve_json_rpc [-p|--port <int>] [-l|--listen <address>]
```

### `mcp`

Expose CLI interface as an MCP server via stdio.

```text
Usage: cdd-c mcp
```

### `audit`

Scan directory for memory safety issues.

```text
Usage: cdd-c audit <directory>
```

### `c2openapi`

Generate OpenAPI spec from C source code.

```text
Usage: c2openapi [--base <openapi.json>] [--self <uri>] [--dialect <uri>] <src_dir> <out.json>
```

### `transformer`

Run syntax tree transformations.

```text
Usage: cdd-c transformer <toolname> [--audit | --fix] [--dry-run] <files...>
Tools:
  extern_c
  msvc_port
  gnu_standardizer
  error_percolator
  safe_crt
```

### `code2schema`

Convert C header to JSON Schema.

```text
Usage: cdd-c code2schema <header.h> <schema.json>
```

### `generate_build_system`

Generate build system files.

```text
Usage: generate_build_system <type> <out_dir> <name> [test]
```

### `schema2code`

Generate C code from JSON schema.

```text
Usage: cdd-c schema2code <schema.json> <out_dir> [--guard-enum=...] [--guard-json=...] [--guard-utils=...]
```

## Extensive Features & Functionality

`cdd-c` is not just a standard parser; it is a full-fledged **Compiler Driven Development (CDD)** suite tailored specifically for `C` (strictly targeting ISO C90 compliance). It deeply understands C down to its comments and whitespace, treating codebase refactoring, code generation, and API alignment as first-class, lossless operations.

> 📚 For comprehensive documentation of all compiler subsystems, lowering rules, and architecture, refer to:
> - [**CAPABILITIES.md**](CAPABILITIES.md) — Comprehensive capabilities guide covering all transformers, FFI emitters, and MCP integration.
> - [**DSL.md**](DSL.md) — Specification and grammar for the CDD FFI IR Domain Specific Language.
> - [**ARCHITECTURE.md**](ARCHITECTURE.md) — High-level compiler design, Frontend $\to$ IR $\to$ Backend pipelines.
> - [**COMPLIANCE.md**](COMPLIANCE.md) — OpenAPI 3.2.0 and Swagger 2.0 specification support matrix.
> - [**USAGE.md**](USAGE.md) — Database migrations, CI/CD database provisioning, and `c-orm` schema extraction.
> - [**WASM.md**](WASM.md) — WebAssembly compilation with Emscripten for web browser and Node.js environments.
> - [**mcp-1.0.0/**](mcp-1.0.0/) — Model Context Protocol conformance and integration specs.

### Lossless AST Manipulation (Zero-Destruction Rule)
Unlike standard preprocessors or regex-based refactoring tools, `cdd-c` uses a custom, highly robust **Concrete Syntax Tree (CST)** and **Abstract Syntax Tree (AST)** pipeline.
- **Trivia Preservation:** It perfectly captures and preserves "trivia"—inline comments, block comments, and exact whitespace (indentation, newlines).
- **Format-Safe Rewrites:** When you mutate a tree (for instance, converting a GNU compiler extension into a standard C89 equivalent), `cdd-c` applies the change surgically. Your code does not get butchered, formatted entirely differently, or stripped of its documentation. The generated ISO C code is meant for human developers, not just compilers.

### AST Lowering & Source Refactoring Suite
Run source-to-source refactoring transformations via `cdd-c transformer <toolname>` or `cdd-c standardize-gnu` with `--audit`, `--fix`, or `--dry-run`:
- **`gnu_standardizer`**: 8-phase lowering of GNU C extensions (`__int128`, `typeof`, VLAs, zero-length arrays, `__complex__`, SIMD vectors, statement expressions, case ranges, `__attribute__`, `__builtin_*`, comma-swallowing macros, and inline assembly) to standard ISO C. See [GNU Transformer README](src/transformers/gnu_standardizer/README.md).
- **`safe_crt`**: Automated migration from legacy unbounded CRT calls (`strcpy`, `sprintf`, `memcpy`) to bounds-checked ISO/IEC TR 24731-1 Safe CRT functions.
- **`msvc_port`**: Cross-compiler porting from POSIX/GNU primitives to Microsoft Visual C++ equivalents.
- **`extern_c`**: Automated injection of C++ `extern "C"` declaration guards.
- **`error_percolator`**: Architectural refactoring from value-returning functions to `cdd_c_error_t` return conventions with typed out-pointers and call-site rewrites.

### Multi-Directional Code Generation
`cdd-c` acts as the master sync tool between your abstractions:
- **API to Native C (SDK & Server):** Generate production-ready, memory-safe C clients, routing servers, and CLI argument parsers straight from OpenAPI 3.2.0 JSON/YAML.
- **Native C to OpenAPI:** Write your C functions, document them with standard block comments, and let `cdd-c` statically analyze the types and routes to reverse-engineer a compliant OpenAPI spec.
- **SQL / JSON Schema Sync:** Seamlessly sync SQL DDL statements to C ORM layers, or JSON schemas directly into C structs and validation logic (`code2schema` / `schema2code`).

### Universal FFI Bindings & IR DSL
`cdd-c` features a universal Foreign Function Interface (FFI) Intermediate Representation (IR). Instead of writing manual SWIG bindings or using fragile regex replacements:
- The `cdd-c bind` command statically analyzes your C codebase and generates seamless native bindings for **42 languages and platforms** (Rust, Python, Go, TypeScript, C#, Java, Swift, Kotlin, Zig, Odin, Dart, Ruby, PHP, Lua, Haskell, OCaml, WebAssembly, etc.).
- Intelligently maps complex C concepts like structs, enums, format-specifier variadic trampolines (`cdd_ffi_variadic`), function pointers, and opaque pointers.
- Provides a human-readable, lossless **FFI IR DSL** with left-to-right typing and explicit memory flow intents. See [**DSL.md**](DSL.md).

### Model Context Protocol (MCP) & AI Integration
- **Meta-MCP Server (`cdd-c mcp`)**: Exposes compiler capabilities over stdio to AI desktop assistants (Claude Desktop, Cursor, Windsurf) for autonomous code scaffolding, schema inspection, and bidirectional synchronization.
- **Generated Client MCP Adapters**: Generated C client SDKs automatically include native MCP tools and resource handlers, enabling LLMs to call backend APIs directly. See [**mcp-1.0.0/**](mcp-1.0.0/).

### Built-in Security & Auditing
- **Memory Safety Audits:** Utilize the built-in `audit` command to run static analysis directly over your C directories. Detect dangerous CRT functions (`strcpy`, `sprintf`), dangling pointer risks, unchecked `malloc`/`calloc`/`realloc`/`strdup` returns, and missing initializations before they hit production. Emits structured JSON reports.

### Database Migrations & c-orm Integration
- Integrated database migration manager (`cdd-c migrate up|down|create`), CI test database setup (`setup_test_db`, `db reset`, `seed`), and active schema dumper (`schema dump`) supporting dynamic drivers for PostgreSQL (`libpq`), SQLite3, and MySQL/MariaDB. See [**USAGE.md**](USAGE.md).

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
