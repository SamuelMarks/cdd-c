# AST-Based C Return Type Refactoring Tool

This directory contains a standalone `tree-sitter` powered script for performing deep, semantic, abstract syntax tree (AST) refactoring on legacy or strict ISO C90 codebases.

## Motivation

When refactoring a large C codebase to strictly enforce exit-code returns (`int`) for all functions, developers face several challenges that simple `sed` or regular expression scripts cannot handle:
1.  **Out-Parameter Conversion:** A function like `char* c_cdd_strdup(const char* s)` must become `int c_cdd_strdup(const char* s, char** out_s)`.
2.  **Inline Boolean Logic:** If the original function was used in a condition (`if (c_cdd_str_starts_with(a, "prefix"))`), modifying the signature completely breaks the logic.
3.  **ISO C90 Strict Compliance:** C90 outlaws inline variable declarations (such as GCC's statement expressions `({ ... })`) inside logic blocks.
4.  **Preprocessor Macros:** `#if defined(_MSC_VER)` macros heavily fragment syntax, confusing most parsers.

## How it Works

The `refactor_returns.py` script systematically solves these issues via the following phases:

1.  **Preprocessor Masking (`mask_msc_ver`)**
    *   Finds troublesome un-evaluated `#ifdef` / `#else` branching and masks them out with spaces. This preserves exact byte offsets so that tree-sitter can cleanly parse the file as valid AST without losing index precision.
2.  **Function Identification (Pass 1)**
    *   Walks the entire repository AST looking for function declarations and definitions.
    *   Filters out any function returning `int`, `void`, or math primitives.
    *   Records the required non-standard return types (struct pointers, enums, JSON types, etc.) in memory.
3.  **Signature & Return Normalization (Pass 2)**
    *   Updates the signatures from `Type func(...)` to `int func(..., Type *_out_val)`.
    *   Walks inside the function block and dynamically replaces `return EXPR;` with `{ *_out_val = EXPR; return 0; }`.
4.  **Call-Site Rewriting & C90 Hoisting (Pass 3)**
    *   Finds every usage of the refactored functions.
    *   Generates a new, unique out-parameter variable (e.g., `_ast_func_0`).
    *   Replaces the inline call using the C comma operator: `func(args)` -> `(func(args, &_ast_func_0), _ast_func_0)`.
    *   Hoists the declaration of `_ast_func_0` to the very top of the immediate enclosing block `{ ... }`, perfectly satisfying ISO C90 compiler rules without breaking `if/else` conditions.

## Requirements

The script requires Python 3 and the tree-sitter bindings for C.

```bash
pip install tree-sitter tree-sitter-c
```

## Usage

```bash
cd scripts/ast_refactor
./refactor_returns.py
```

*Note: As with all massive automated refactoring tools, this should be executed on a clean git working tree so that changes can be easily reviewed or discarded if any manual touch-ups are needed (e.g. system callbacks).*
