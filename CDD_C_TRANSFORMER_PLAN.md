# cdd-c Transformer & Fixers Plan

This document outlines the architectural roadmap and implementation steps for evolving `cdd-c` into a robust, lossless Abstract/Concrete Syntax Tree (CST) transformation engine. 

The core constraint: **Transformations MUST be macro, comment, and whitespace sensitive.** Standard compiler ASTs are insufficient. `cdd-c` will implement a Lossless Syntax Tree (CST) where `Unparse(Parse(source)) == source` is guaranteed byte-for-byte.

## Phase 1: Lossless Lexer & CST Foundations
*The goal is to parse C source code without losing a single character of trivia or expanding a single macro.*

- [ ] **Data Structures: Tokens & Trivia**
  - [ ] Define the `cdd_token_t` struct (type, text span, line, column, offset).
  - [ ] Define the `cdd_trivia_t` struct to classify non-semantic characters (Whitespace, Newline, LineComment, BlockComment).
  - [ ] Implement trivia attachment rules (e.g., leading trivia attaches to the next token, trailing trivia attaches to the previous token).
- [ ] **The Lexer (`src/classes/parse/cdd_lexer.c`)**
  - [ ] Tokenize standard C89/C99/C11/C23 keywords and symbols.
  - [ ] Implement rigorous comment parsing (both `//` and `/* ... */`), ensuring no internal whitespace or formatting is lost.
  - [ ] Tokenize preprocessor directives (`#include`, `#define`, `#ifdef`, `#pragma`, etc.) as distinct tokens rather than executing them.
- [ ] **Data Structures: The Concrete Syntax Tree (CST)**
  - [ ] Design the CST Node hierarchy (`cdd_cst_node_t`) encompassing Declarations, Statements, Expressions, and Translation Units.
  - [ ] Ensure every CST Node maintains pointers to its constituent Tokens (and thus, their Trivia).
- [ ] **The Lossless Parser (`src/classes/parse/cdd_cst_parser.c`)**
  - [ ] Implement a recursive descent parser that constructs the CST from the lexed tokens.
  - [ ] Handle object-like macros as opaque identifiers.
  - [ ] Handle function-like macros as opaque function calls.
  - [ ] Handle unexpanded preprocessor conditional blocks (`#ifdef ... #else ... #endif`) as high-level AST wrapper nodes to allow parsing multiple code paths natively.
- [ ] **The Emitter (`src/classes/emit/cdd_cst_emit.c`)**
  - [ ] Implement a strict CST walker that un-parses the tree back into text.
  - [ ] Create an integrity test suite (`Unparse(Parse(file)) == file`) and test it against massive C files (e.g., `sqlite3.c`, `stb_image.h`) to verify zero data loss.

## Phase 2: CST Transformation Primitives
*The engine that makes surgical edits to the CST.*

- [ ] **Traversal & Querying**
  - [ ] Implement a CST Visitor pattern (pre-order and post-order traversal).
  - [ ] Implement node query functions (e.g., `find_nodes_by_type`, `find_function_calls_named`).
- [ ] **Safe Mutation API**
  - [ ] Implement `cst_node_replace(old_node, new_node)`: Swaps nodes while preserving the original leading/trailing trivia of the replaced node.
  - [ ] Implement `cst_node_insert_before(target, new_node)` and `cst_node_insert_after(target, new_node)`.
  - [ ] Implement `cst_node_delete(target)`: Safely removes a node and cleans up orphaned adjacent whitespace/newlines.
- [ ] **Trivia Synthesis & Formatting**
  - [ ] Implement a file-level indentation detector (analyzes if the file uses spaces vs. tabs, and the indent width).
  - [ ] Implement trivia generators for newly synthesized nodes so injected code naturally matches the surrounding block's indentation.

## Phase 3: The Fixers (Transformation Passes)
*Specific modules targeting the required architectural standardizations.*

### 3.1: Extern C Fixer (`cdd_transform_extern_c`)
- [ ] Scan CST for existing `#ifdef __cplusplus` guards to prevent duplicate wrapping.
- [ ] Analyze the Translation Unit to locate the optimal insertion boundary (typically after file-level license comments and `#include` directives, but before the first actual C struct/function declaration).
- [ ] Synthesize the top `#ifdef __cplusplus \n extern "C" { \n #endif` nodes and insert.
- [ ] Synthesize the bottom `#ifdef __cplusplus \n } \n #endif` nodes and insert before EOF.
- [ ] Handle edge cases: placing guards inside of header include guards (`#ifndef HEADER_H`).

### 3.2: MSVC Port Fixer (`cdd_transform_msvc`)
- [ ] Identify POSIX/GNU specific includes (e.g., `<unistd.h>`, `<sys/time.h>`).
- [ ] Conditionally strip or wrap unsupported includes, or inject `#include "win_compat_sym.h"`.
- [ ] Traverse the CST for known POSIX identifier tokens:
  - [ ] Replace `strcasecmp` with `_stricmp`.
  - [ ] Replace `strncasecmp` with `_strnicmp`.
  - [ ] Replace `strdup` with `_strdup`.
  - [ ] Replace `ssize_t` with `SSIZE_T` (or `intptr_t`).
- [ ] Detect `__builtin_expect` and translate to standard mechanisms or custom macros.
- [ ] Ensure trivia around swapped tokens is 100% preserved.

### 3.3: GNU-ism Standardizer (`cdd_transform_gnu`)
- [ ] Locate and parse `__attribute__((...))` tokens.
- [ ] Translate `__attribute__((unused))` to `(void)var;` at the start of the function body or C23 `[[maybe_unused]]`.
- [ ] Translate `__attribute__((packed))` to MSVC/Standard `#pragma pack(push, 1)` and `#pragma pack(pop)` wrappers.
- [ ] Translate `__attribute__((noreturn))` to standard C `_Noreturn`.
- [ ] Locate GNU Statement Expressions `({ ... })`.
- [ ] Unroll Statement Expressions by extracting the inner block into an inline function or preceding scope block.
- [ ] Locate Variable Length Arrays (VLAs) and rewrite them to use `malloc`/`free` or `alloca`.

### 3.4: Error Percolation Fixer (`cdd_transform_percolate_errors`)
- [ ] Traverse function definitions to identify targets returning non-error types (`void`, pointers, etc.).
- [ ] **Signature Rewrite:** Change the return type token to `int` (or a standardized `cdd_result_t`).
- [ ] **Parameter Append:** Append a new parameter node (`OriginalType* out_result`) to the signature, preserving multi-line parameter formatting.
- [ ] **Allocation Detection:** Scan function bodies for `malloc`, `calloc`, and `realloc` calls.
- [ ] **Check Injection:** Synthesize conditional blocks `if (!ptr) { return ENOMEM; }` and insert them immediately after allocations, inheriting exact local indentation.
- [ ] **Return Rewrite:** Rewrite `return <expr>;` statements to `*out_result = <expr>; return EXIT_SUCCESS;`.
- [ ] **Call Site Updates:** Traverse the translation unit to locate all invocations of the modified functions.
- [ ] **Call Site Rewrite:** Update arguments to pass `&local_var` for the new `out` parameter, and wrap the call in a return code check.
- [ ] **Memory Leak Prevention:** For functions with early error returns, implement basic control flow analysis to synthesize `goto cleanup;` blocks if other allocations occurred prior to the failure.

## Phase 4: SDK, CLI Integration & Testing
*Making the tools accessible and ensuring absolute reliability.*

- [ ] **SDK / C API Interface (`include/cdd_c_backend_interface.h`)**
  - [ ] Define configurations structs for transformations (e.g., specifying indentation width if detection fails).
  - [ ] Expose `cdd_transform_extern_c()`.
  - [ ] Expose `cdd_transform_msvc()`.
  - [ ] Expose `cdd_transform_gnu()`.
  - [ ] Expose `cdd_transform_percolate_errors()`.
- [ ] **CLI Subcommand Routing (`src/classes/parse/cli_parser.c` & `bin_cdd.c`)**
  - [ ] Wire up `cdd-c fix-extern-c <files...>`.
  - [ ] Wire up `cdd-c port-msvc <files...>`.
  - [ ] Wire up `cdd-c standardize-gnu <files...>`.
  - [ ] Wire up `cdd-c percolate-errors <files...>`.
  - [ ] Implement global formatting options (`--indent=4`, `--use-tabs`, `--in-place`, `--out-dir`).
- [ ] **Test Suite (`src/tests/`)**
  - [ ] **Lexer & Parser Tests:** Verify trivia is correctly attached to CST nodes.
  - [ ] **Integrity Tests:** Run the `Unparse(Parse(file))` loop over complex real-world files to guarantee zero data loss.
  - [ ] **Fixer Integration Tests:** Create fixture directories (`test_data/before` and `test_data/expected_after`).
  - [ ] Test `extern_c` edge cases (multiple includes, existing guards).
  - [ ] Test `msvc_port` POSIX replacements with complex inline comments `strcasecmp /* comment */ (a, b)`.
  - [ ] Test `percolate_errors` on complex control flows (nested loops, multiple allocations).
- [ ] **CI/CD Integration**
  - [ ] Automate the transformation test suite in GitHub Actions.
  - [ ] Verify that `cdd-c` can successfully run its own fixers on its own codebase without breaking compilation.