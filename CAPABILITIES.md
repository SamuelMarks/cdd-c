# CDD-C Capabilities & Advanced Architecture

<!-- BADGES_START -->

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Doc Coverage](https://img.shields.io/badge/doc_coverage-100%25-brightgreen.svg)](#)
[![Test Coverage](https://img.shields.io/badge/test_coverage-100.0%25-brightgreen.svg)](#)
[![interactive WASM web demo](https://img.shields.io/badge/interactive-WASM_web_demo-blue.svg)](https://offscale.io/wasm_web_demo)

<!-- BADGES_END -->

The **cdd-c** compiler is a comprehensive, ISO C90-compliant **Compiler Driven Development (CDD)** platform and Abstract Syntax Tree (AST) manipulation engine. While standard code generators treat target languages as passive output destinations, `cdd-c` understands C source code symmetrically down to its lexical tokens, comments, and whitespace.

This document provides a deep dive into the specialized capabilities and architectural subsystems that make `cdd-c` a superset of standard language compilers.

---

## Table of Contents

1. [Architectural Philosophy & Trivia Preservation](#1-architectural-philosophy--trivia-preservation)
2. [Concrete Syntax Tree (CST) & Semantic Lifting](#2-concrete-syntax-tree-cst--semantic-lifting)
3. [The AST Transformation Engine](#3-the-ast-transformation-engine)
   - [GNU C to ISO C90 Standardizer (`gnu_standardizer`)](#gnu-c-to-iso-c90-standardizer-gnu_standardizer)
   - [Safe CRT Transformer (`safe_crt`)](#safe-crt-transformer-safe_crt)
   - [MSVC Porting Engine (`msvc_port`)](#msvc-porting-engine-msvc_port)
   - [C++ Wrapper (`extern_c`)](#c-wrapper-extern_c)
   - [Error Handling Percolator (`error_percolator`)](#error-handling-percolator-error_percolator)
   - [Macro AST Expansion Pass (`macros`)](#macro-ast-expansion-pass-macros)
4. [Universal Multi-Language FFI Generator (`cdd-c bind`)](#4-universal-multi-language-ffi-generator-cdd-c-bind)
   - [Normalized Intermediate Representation (IR)](#normalized-intermediate-representation-ir)
   - [42 Supported Target Languages](#42-supported-target-languages)
   - [Printf/Scanf Variadic Trampoline Synthesis](#printfscanf-variadic-trampoline-synthesis)
   - [Lossless IR Domain Specific Language (`DSL.md`)](#lossless-ir-domain-specific-language-dslmd)
5. [Model Context Protocol (MCP) & AI Tooling](#5-model-context-protocol-mcp--ai-tooling)
   - [Meta-MCP Server (`cdd-c mcp`)](#meta-mcp-server-cdd-c-mcp)
   - [Generated Client MCP Adapters](#generated-client-mcp-adapters)
   - [JSON-RPC 2.0 Server (`cdd-c serve_json_rpc`)](#json-rpc-20-server-cdd-c-serve_json_rpc)
6. [Static Security & Memory Safety Auditing (`cdd-c audit`)](#6-static-security--memory-safety-auditing-cdd-c-audit)
7. [Bidirectional Schema, Database & UI Interoperability](#7-bidirectional-schema-database--ui-interoperability)
   - [Bidirectional Struct $\leftrightarrow$ JSON Schema (`code2schema` / `schema2code`)](#bidirectional-struct--json-schema-code2schema--schema2code)
   - [Documentation Code Matrix (`to_docs_json`)](#documentation-code-matrix-to_docs_json)
   - [Material 3 Client GUI Generator (`client_gui_gen`)](#material-3-client-gui-generator-client_gui_gen)
   - [Database Migration Suite & `c-orm`](#database-migration-suite--c-orm)
8. [Cross-Platform Build Scaffolding & WebAssembly Runtime](#8-cross-platform-build-scaffolding--webassembly-runtime)

---

## 1. Architectural Philosophy & Trivia Preservation

In typical compilers, the lexer discards whitespace and comments to construct an Abstract Syntax Tree (AST) suited solely for binary code emission. When such tools format or transpile code, original human documentation and layout choices are permanently lost.

`cdd-c` enforces the **Zero-Destruction Rule**:
- **Lossless Roundtripping**: Every single whitespace character, newline, horizontal tab, single-line comment (`//`), multi-line comment (`/* ... */`), and Doxygen tag is parsed as first-class **trivia**.
- **Trivia Anchoring**: Trivia tokens are deterministically anchored to the nearest syntax nodes they precede or succeed. When a subtree is inspected, transformed, or hoisted, its accompanying trivia travels with it.
- **Human-Centric Emission**: Code emitted by `cdd-c` is meant for human maintainers, passing strict static analysis while preserving existing codebase formatting.

---

## 2. Concrete Syntax Tree (CST) & Semantic Lifting

`cdd-c` implements a two-tier syntactic pipeline:

```
Source Code (.c/.h) ──► Lexer ──► CST (Concrete Syntax Tree) ──► Semantic Lifting ──► AST + Symbol Table + CFG
                                      │                                                     │
                                      ▼                                                     ▼
                              Lossless Emitter ◄──────────────────────────────────── Transformations
```

1. **Concrete Syntax Tree (CST)**:
   - Full representation of the token stream including punctuation, delimiters, and trivia.
   - Built via `cdd_cst_builder_t` (`classes/parse/cdd_cst_builder.h`).
   - Queryable with predicate matchers like `cdd_cst_find_nodes_by_type` and `cdd_cst_find_function_calls_named`.
   - Node-level mutation framework via `cdd_cst_replace_node_preserve_trivia` and `cdd_cst_insert_node_after`.
2. **Semantic Lifting**:
   - **Scope-Aware Symbol Table**: Tracks variables, typedefs, structs, unions, enumerations, and labels across global, file, function, and block scopes, handling lexical shadowing accurately.
   - **Compile-Time Type Evaluator**: Dynamically determines size, alignment, pointer indirection, and qualifier composition (`const`, `volatile`) targeting C89 and C99 models.
   - **Control Flow Graph (CFG)**: Maps control edges for branches, loops, switches, `break`, `continue`, and `goto` jumps to verify dead code paths and safety guarantees.

---

## 3. The AST Transformation Engine

`cdd-c` provides a suite of source-to-source AST transformers executable via the CLI:

```bash
# General syntax:
cdd-c transformer <toolname> [--audit | --fix] [--dry-run] <files...>

# Specialized GNU C standardization:
cdd-c standardize-gnu [--target-c89 | --target-c99] [--fallback-alloca] [--audit | --fix | --dry-run] <files...>
```

All transformers support three execution modes:
- `--audit`: Non-zero exit code if files require transformation; prints required diffs.
- `--fix`: Applies surgical transformations in-place, rewriting files without altering unrelated trivia.
- `--dry-run`: Simulates transformations to standard output without touching files on disk.

### GNU C to ISO C90 Standardizer (`gnu_standardizer`)

The `gnu_standardizer` engine lowers proprietary GNU C compiler extensions into strictly compliant ISO C90 (C89) or C99 code. It operates across 8 methodical phases:

| Phase | Category | Extensions Lowered | Lowering Strategy |
| :--- | :--- | :--- | :--- |
| **Phase 0** | **Engine Foundation** | CST-to-AST lifting, symbol resolution, CFG construction | Allocates scoped symbol tables and prepares AST mutator arenas. |
| **Phase 1** | **Type System** | `__int128`, `typeof`, `__auto_type`, VLAs, zero-length arrays, `__complex__`, vector types | • Maps `__int128` to `struct { uint64_t low, high; }` with portable math helpers (`int128_math.h`).<br>• Resolves `typeof` statically at compile time.<br>• Transforms VLAs into `alloca()` or `malloc()`/`free()`.<br>• Rewrites struct `arr[0]` to `arr[1]` for C89 padding conformance.<br>• Lowers `__complex__` to `.real` / `.imag` struct representations.<br>• Converts `vector_size` to standard typed arrays. |
| **Phase 2** | **Control Flow** | Nested functions, computed `goto` (`&&label`), statement expressions (`({ ... })`), case ranges (`case 1 ... 5:`) | • Emits trampoline warnings for dynamic closures; hoists non-capturing nested functions.<br>• Expands case ranges into explicit sequential fallthrough blocks.<br>• Hoists statement expressions to parent block scopes with collision-free temporaries (`__cdd_tmp_N`). |
| **Phase 3** | **Expressions** | Binary conditionals (`x ? : y`), `void*` arithmetic, Lvalue casts, designated initializers | • Synthesizes temporary bindings for omitted ternary operands to prevent side-effect re-evaluation.<br>• Inserts explicit byte casts for pointer math.<br>• Rewrites designated initializers into ANSI C sequential initialization sequences. |
| **Phase 4** | **Attributes** | `__attribute__((aligned, always_inline, noreturn, cleanup, packed))` | • Translates alignment into MSVC `__declspec(align)` or `_Alignas`.<br>• Degrades non-portable inline hints gracefully.<br>• Automatically unwinds `cleanup(func)` handlers in reverse registration order at all scope exits and return points. |
| **Phase 5** | **Builtins** | `__builtin_offsetof`, `__builtin_ctz`, `__builtin_popcount`, `__builtin_bswap*`, `__builtin_add_overflow` | • Emulates GCC bitwise intrinsics via pure ANSI C bitwise operations or MSVC `_BitScanForward` / `__popcnt`.<br>• Implements safe integer overflow checking via widening arithmetic checks. |
| **Phase 6** | **Preprocessor** | Variadic macros, `, ##__VA_ARGS__`, `__PRETTY_FUNCTION__`, binary literals (`0b0101`) | • Rewrites comma-swallowing hacks to C99 standard macro conventions.<br>• Inlines `__PRETTY_FUNCTION__` / `__FUNCTION__` to `__func__`.<br>• Lowers binary integer literals to hexadecimal notation. |
| **Phase 7** | **Inline Assembly** | GCC `__asm__` operands, input/output constraints, clobbers | • Parses inline assembly operand lists and maps compatible registers into MSVC native `__asm` blocks. |

For deep implementation details on the lowering phases, see [`src/transformers/gnu_standardizer/README.md`](src/transformers/gnu_standardizer/README.md).

### Safe CRT Transformer (`safe_crt`)

Automates migration from vulnerable, unconstrained C runtime string and memory primitives to ISO/IEC TR 24731-1 bounds-checked functions:

```c
/* Legacy C Input */
strcpy(dest, src);
sprintf(buf, "%s: %d", name, count);
memcpy(out, in, n);

/* Transformed Safe CRT Output */
strcpy_s(dest, sizeof(dest), src);
sprintf_s(buf, sizeof(buf), "%s: %d", name, count);
memcpy_s(out, sizeof(out), in, n);
```

- **Capacity Inference**: Statically resolves destination buffer sizes from array bounds, struct definitions, or preceding allocation calls.
- **Cross-Platform Compatibility**: Bundles [`include/c_cdd/safe_crt.h`](include/c_cdd/safe_crt.h) and [`include/c_cdd/safe_crt_msvc.h`](include/c_cdd/safe_crt_msvc.h) to polyfill Safe CRT semantics seamlessly across Clang, GCC, and MSVC.

### MSVC Porting Engine (`msvc_port`)

Transpiles POSIX and GNU C conventions into Microsoft Visual C++ equivalents:
- Rewrites `strcasecmp` to `_stricmp` and `strncasecmp` to `_strnicmp`.
- Replaces POSIX `strdup` with MSVC `_strdup`.
- Converts POSIX `ssize_t` to Windows `SSIZE_T` and injects `#include <BaseTsd.h>`.
- Maps `off_t` to `_off_t`.
- Converts `__builtin_expect` hints into conditional branching polyfills.

### C++ Wrapper (`extern_c`)

Automates header refactoring for seamless C++ interoperability:
- Parses the CST to detect whether C declarations (`CDD_CST_DECLARATION`, `CDD_CST_FUNCTION_DEFINITION`) exist without proper guards.
- Surgically injects `#ifdef __cplusplus \n extern "C" { \n #endif` and `#ifdef __cplusplus \n } \n #endif` blocks.
- Avoids redundant wrapping if existing guards or C++ preprocessor conditions are already present.

### Error Handling Percolator (`error_percolator`)

Refactors legacy procedural C code into robust, production-grade error-percolating architectures:
- Scans functions for internal memory allocations or fallible operations.
- Mutates function signatures from returning raw values or `void` to returning `cdd_c_error_t`.
- Promotes the original return value to a typed out-parameter pointer (`type** out_val`).
- Automatically rewrites all return expressions (`return val;` -> `*out_val = val; return CDD_C_SUCCESS;`) and adjusts caller call sites across the entire CST.

### Macro AST Expansion Pass (`macros`)

Selectively unrolls function-like macros and macro operators (`#` stringification and `##` token pasting) directly in the AST, allowing subsequent FFI and OpenAPI extraction engines to inspect the expanded semantic structure without running a destructive external C preprocessor.

---

## 4. Universal Multi-Language FFI Generator (`cdd-c bind`)

The `bind` command replaces manual SWIG interface files, ctypes boilerplate, and complex binding scripts by parsing C headers directly into a normalized **FFI Intermediate Representation (IR)**.

```bash
cdd-c bind -i include/my_lib.h -o bindings/ -l python,rust,csharp --lib-name my_lib
```

```
C Headers (.h) ──► FFI Extractor ──► FFI IR (cdd_ffi_ir_t) ──► 42 Emitters ──► Native Language Bindings
```

### Normalized Intermediate Representation (IR)

The FFI IR (`include/ffi/cdd_ffi_ir.h`) structures the API into:
- **Structs & Enums**: Field layouts, alignments, signedness, integer values, and documentation.
- **Function Signatures**: Return types, parameters, directionality intents (`in`, `out`, `inout`), and variadic conventions.
- **Opaque Pointers**: Unknown structs cleanly mapped to type-safe opaque handles.
- **Trivia & Docstrings**: Preserved and emitted as native docstrings in the target languages (e.g., Python docstrings, Rust `///`, Javadoc `/** */`).

### 42 Supported Target Languages

The FFI backend contains 42 dedicated, handwritten code generators (`src/functions/ffi/`):

| Target Family | Languages Supported | Generation Pattern |
| :--- | :--- | :--- |
| **Systems & Native** | C++, Rust, Zig, Odin, D, Nim, V (`vlang`), WebAssembly, Ada, Fortran | Direct ABI-compatible extern blocks, idiomatic RAII wrappers, and memory-safe abstractions. |
| **Managed & OOP** | C#, Java, Kotlin, Scala, Groovy, Delphi, Objective-C, Swift | P/Invoke (`DllImport`), JNI/JNA bindings, Swift `@_silgen_name` wrappers, and native classes. |
| **Dynamic & Scripting** | Python, Ruby, Perl, PHP, Lua, Tcl, R, MATLAB, Julia | `ctypes`/`cffi` modules, Ruby FFI gems, Perl XS wrappers, PHP FFI extensions, LuaJIT FFI tables. |
| **Functional & Logic** | Haskell, OCaml, Elixir, Erlang, Clojure, Common Lisp, Racket, Scheme, F# | Foreign Function Interfaces, Erlang NIFs, OCaml `external` primitives, Haskell `foreign import`. |
| **Web & Node.js** | TypeScript, Node.js N-API | Native N-API C++ addons, strongly typed `.d.ts` definitions, and WebAssembly glue code. |
| **Data & Tooling** | Intermediate Representation JSON (`ir_dump.json`) | Normalized machine-readable JSON representation for custom toolchains and AST consumers. |

### Printf/Scanf Variadic Trampoline Synthesis

C variadic functions (`...`) are notoriously problematic across FFI boundaries. `cdd-c` integrates an format analyzer (`include/ffi/cdd_ffi_variadic.h`):
- Heuristically parses format specifier strings (e.g., `%s`, `%d`, `%lf`, `%p`) at compile or code-generation time.
- Synthesizes strongly typed trampoline functions (`cdd_ffi_vararg_trampoline`) for target languages that do not natively support C ABI variadics.

### Lossless IR Domain Specific Language (`DSL.md`)

To bridge human-readable API declarations and machine generation, `cdd-c` provides an ergonomic, left-to-right interface definition language with lossless roundtripping.

```c
namespace AudioEngine {
    /* Process incoming buffer and write resulting frame size */
    func process_pcm([in] *const u8 buffer, [out] *mut size_t out_len);
}
```

- **Left-to-Right Type Reading**: Replaces C's inside-out declaration syntax with intuitive types (`*mut`, `*const`, `[N]`).
- **Data Flow Intent Annotations**: Expresses memory semantics (`[in]`, `[out]`, `[inout]`, `len=target`).
- **Fenced Native C**: Fences raw C bodies via `%{ ... }%` for lexer safety.
- **Full Roundtripping**: Generate bindings directly from DSL text via `cdd_generate_bindings_from_dsl`.

For full grammar specifications and examples, see [`DSL.md`](DSL.md).

---

## 5. Model Context Protocol (MCP) & AI Tooling

`cdd-c` implements the **Model Context Protocol (MCP)** specification across both the **compiler tooling** itself and the **generated artifacts**.

For complete compliance tables and schemas, consult [`mcp-1.0.0/`](mcp-1.0.0/).

```
                 ┌──────────────────────────────────────────────────────────┐
                 │                   LLM Client / IDE                       │
                 │         (Claude Desktop, Cursor, Windsurf)               │
                 └────────────────────────────┬─────────────────────────────┘
                                              │ stdio / JSON-RPC
                      ┌───────────────────────┴────────────────────────┐
                      ▼                                                ▼
     ┌─────────────────────────────────┐              ┌─────────────────────────────────┐
     │      Meta-MCP Compiler CLI      │              │      Generated C Client SDK     │
     │          (`cdd-c mcp`)          │              │    (`client_gen_mcp.c`)         │
     ├─────────────────────────────────┤              ├─────────────────────────────────┤
     │ • cdd_generate (Code Scaffold)  │              │ • tools/list, tools/call        │
     │ • cdd_inspect  (Schema Query)   │              │ • resources/list, resources/read│
     │ • cdd_sync     (Bi-Sync Engine) │              │ • prompts/list, prompts/get     │
     └─────────────────────────────────┘              └─────────────────────────────────┘
```

### Meta-MCP Server (`cdd-c mcp`)

Running `cdd-c mcp` starts a stdio-based JSON-RPC server conforming to the MCP specification:
- Exposes tools to AI assistants:
  - `cdd_generate`: Scaffolds C SDKs, server stubs, and build configurations.
  - `cdd_inspect`: Statically introspects C types, header declarations, and schemas.
  - `cdd_sync`: Synchronizes OpenAPI specifications with C codebases bidirectionally.

### Generated Client MCP Adapters

When generating a client SDK from an OpenAPI spec, `cdd-c` automatically emits native MCP adapters (`src/routes/emit/client_gen_mcp.c`):
- Every OpenAPI operation is automatically exposed as an MCP tool (`tools/list`, `tools/call`).
- Endpoints can be called directly by LLM agents via standard JSON-RPC payloads.
- Includes adapters for `resources/list`, `resources/read`, `prompts/list`, and `ping`.

### JSON-RPC 2.0 Server (`cdd-c serve_json_rpc`)

Exposes the entire compiler CLI surface over TCP sockets or HTTP:

```bash
cdd-c serve_json_rpc --port 8080 --listen 127.0.0.1
```

Enables remote orchestration from IDE plugins, web interfaces, and CI/CD automation systems.

---

## 6. Static Security & Memory Safety Auditing (`cdd-c audit`)

The built-in `audit` command executes deep static analysis directly over C project directories:

```bash
cdd-c audit /path/to/c/project
```

### Audit Capabilities
- **Unchecked Allocations**: Tracks all allocation calls (`malloc`, `calloc`, `realloc`, `strdup`) and flags cases where the return pointer is dereferenced or returned without a verified `NULL` check.
- **Dangerous CRT Functions**: Detects usage of legacy CRT routines vulnerable to buffer overflows (`strcpy`, `strcat`, `sprintf`, `gets`).
- **Uninitialized Pointers & Dangling References**: Flags pointer declarations that remain uninitialized or escape their lexical scope.
- **Structured JSON Reporting**: Emits structured violation reports including precise file path, line number, column offset, and offending variable/allocator name.

---

## 7. Bidirectional Schema, Database & UI Interoperability

### Bidirectional Struct $\leftrightarrow$ JSON Schema (`code2schema` / `schema2code`)

- **`code2schema <header.h> <schema.json>`**: Statically analyzes C header struct declarations and emits compliant JSON Schema definitions (supporting arrays, nested structs, primitive mappings, and docstrings).
- **`schema2code <schema.json> <out_dir>`**: Consumes JSON Schema files and generates C struct headers, Parson-based serialization/deserialization routines, and bounds-validation guards.

### Documentation Code Matrix (`to_docs_json`)

Generates comprehensive documentation matrices from OpenAPI specs:

```bash
cdd-c to_docs_json -i spec.json -o docs.json
```

Emits self-contained, syntax-highlighted, executable C code snippets for every endpoint and HTTP verb (`GET`, `POST`, `PUT`, `DELETE`, `PATCH`, `OPTIONS`, `HEAD`), complete with mock payloads and error-checking boilerplate.

### Material 3 Client GUI Generator (`client_gui_gen`)

When generating client SDKs, `cdd-c` synthesizes a native Material 3 / C-based desktop GUI module (`src/routes/emit/client_gui_gen.c`):
- Scaffolds an OAuth2 authentication management dialog (`cmp_oauth2_view_render`).
- Implements the OAuth2 password grant flow (`execute_password_grant`).
- Generates component trees with column layouts, text fields (`m3_text_field_init`), and action buttons (`m3_button_init`).

### Database Migration Suite & `c-orm`

`cdd-c` provides database migration tooling and driver introspection:
- **Driver Introspection**: Detects dynamic or linked drivers for PostgreSQL (`libpq`), SQLite3, and MySQL/MariaDB at startup (`cdd-c --version`).
- **Migration Commands**:
  - `cdd-c migrate create <name>`: Scaffolds timestamped SQL migration files.
  - `cdd-c migrate up` / `cdd-c migrate down`: Applies or rolls back schema migrations.
  - `cdd-c setup_test_db <name>`, `db reset`, `seed <file.sql>`: Scaffolds isolated CI testing databases.
  - `cdd-c schema dump <out.sql>`: Extracts the live schema for `c-orm` structural mapping.

For practical database workflows, see [`USAGE.md`](USAGE.md).

---

## 8. Cross-Platform Build Scaffolding & WebAssembly Runtime

### Automated Build Scaffolding (`generate_build_system`)

Scaffolds cross-platform `CMakeLists.txt` build definitions:
- Detects the runtime target platform: configures WinHTTP on Windows vs libcurl on POSIX systems.
- Configures sanitizers, warning flags, export headers, and optional test harnesses (`AcquireGoogleTest.cmake`).

### WebAssembly Engine (`WASM.md`)

The entire compiler compiles cleanly to WebAssembly using Emscripten:
- **Zero-Dependency CLI**: Run `cdd-c` on any machine via Node.js or browser without native build tools or C compilers.
- **In-Browser Demo**: Enables live, instant API generation and code refactoring in web applications.

For build and integration instructions, see [`WASM.md`](WASM.md).

---

## Summary of Specialized Tooling

| Subsystem | Primary Command | Key Architectural Features |
| :--- | :--- | :--- |
| **AST Transformers** | `cdd-c transformer`, `standardize-gnu` | 8-phase GNU lowering, Safe CRT rewriting, MSVC porting, `extern "C"` injection, Error percolator. |
| **Universal FFI** | `cdd-c bind` | Normalized IR, 42 languages, printf variadic trampolines, lossless DSL (`DSL.md`). |
| **Model Context Protocol** | `cdd-c mcp` | AI code scaffolding via stdio; client SDKs emitted as native MCP tools. |
| **Security Auditing** | `cdd-c audit` | Static analysis for unchecked allocations, missing NULL guards, and unsafe CRT calls. |
| **Schema Interop** | `code2schema`, `schema2code` | Symmetric C struct $\leftrightarrow$ JSON Schema conversion with validation logic. |
| **Developer Docs** | `cdd-c to_docs_json` | Generates self-contained C snippet catalogs for developer portals. |
| **Database & ORM** | `cdd-c migrate`, `schema dump` | PostgreSQL, SQLite, and MySQL dynamic drivers for `c-orm`. |
| **WebAssembly** | `make build_wasm` | Fully portable browser and Node.js execution via Emscripten. |
