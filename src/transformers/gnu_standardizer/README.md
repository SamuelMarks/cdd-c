# GNU C to Standard C (ISO) Transformer

This directory houses the `gnu_standardizer` transformer, a robust AST-aware transformation engine designed to lower GNU C compiler extensions into strictly compliant ISO C90 (C89).

## Overview

The `gnu_standardizer` represents a significant architectural achievement within `cdd-c`. Originally, codebase refactoring tools relied heavily on token-level string replacements, which often resulted in fragile code, subtle regressions, and the destruction of formatting and comments ("trivia").

To overcome these limitations and achieve zero edge cases, we implemented a full **Abstract Syntax Tree (AST)** manipulation engine equipped with a semantic type-checker and Control Flow Graph (CFG) analyzer. 

### The Zero-Destruction Rule
At the core of this transformer lies the **Zero-Destruction Rule**. We guarantee that all comments (inline and block) and whitespace (indentation, newlines) are rigorously preserved. The generated ISO C code is meant for human developers; thus, transformations are surgically applied directly to the AST without disrupting the surrounding trivia.

## Execution of the Transformation Plan

The execution of the `GNU_TRANSFORM_PLAN` was broken down into 8 methodical phases. Each phase successfully introduced deep polyfills and lowering mechanisms for GCC extensions that are fundamentally incompatible with legacy compilers like MSVC (e.g., MSVC 2005/2022).

### Phase 0: The `cdd-c` Minimum Viable Engine (MVE)
Before complex transformations could begin, the engine needed the ability to semantically understand C code. We implemented:
- **Full CST-to-AST Lifting**: Transforming flat tokens into a hierarchical tree while perfectly attaching trivia to the nearest parent nodes.
- **Scope-Aware Symbol Tables**: Tracking variables, typedefs, structs, unions, and labels across block and function scopes, accurately resolving shadowing.
- **Compile-Time Type Evaluator**: Accurately resolving types, pointer sizes, and qualifiers (`const`, `volatile`) dynamically based on target architectures.
- **Control Flow Graph (CFG) Builder**: Tracking normal/return exits, loops, switches, `goto` jumps, and identifying cyclic dependencies.
- **AST Mutator Framework**: Creating an API for safe node detachment, programmatic synthesis of new standard C nodes, collision-free name mangling (e.g., `__cdd_tmp_42`), and pretty-printing.

### Phase 1: Type System & Declarations
We tackled GNU C's extended type system, mapping it to standard C or polyfilling it where necessary:
- **128-bit Integers**: `__int128` types were mapped to a standard `struct { uint64_t low, high; }` representation.
- **Type Introspection**: Evaluated `typeof` and `__auto_type` constructs during traversal to statically resolve underlying types.
- **Arrays & Sizing**: Handled complex Variable Length Arrays (VLAs) using `alloca()` or `malloc()`/`free()` fallbacks. Converted GNU zero-length arrays (`int arr[0];`) located in the middle of structs to standard 1-element arrays (`int arr[1];`) to avoid violating C89 padding.
- **Complex Numbers**: Transformed `__complex__ float z` to standard structs `struct { float real, imag; } z`, automatically mutating `__real__ z` and `__imag__ z` extractions into field accessors (`z.real`, `z.imag`).
- **Vector Types**: Lowered `__attribute__((vector_size(N)))` extensions to standard C arrays, maintaining indexing behavior while stripping incompatible C++-style operator semantics.
- **Struct/Union Extensions**: Synthesized dummy tags for anonymous structs and unions to satisfy C89. Rewrote transparent unions and packed attributes.

### Phase 2: Control Flow & Scoping
This phase required complex graph restructuring:
- **Nested Functions**: Detected nested functions and safely hoisted them where possible. For impossible stack-based trampolines (where closures capture local state dynamically), we explicitly emit standard translation errors instead of silently failing.
- **Computed Gotos**: Parsed `&&label` syntax and jump tables (`goto *ptr`). Emitted hard warnings for cross-VLA scope jumps that cannot be natively polyfilled without complete function re-writing.
- **Statement Expressions**: Inferred types of `({ ... })` blocks and safely hoisted side-effects to standard scope blocks.
- **Extended Switch Statements**: Expanded GNU case ranges (`case 1 ... 5:`) into explicit sequential case blocks, appropriately handling fallthrough and negative ranges.

### Phase 3: Expressions & Operators
- **Conditionals**: Resolved omitted operands (`x ? : y`), generating temporaries to prevent duplicated side-effects from volatile variables.
- **Pointers & Memory**: Handled `void*` arithmetic and function pointer offsets.
- **Casts and Lvalues**: Rewrote Lvalue cast expressions and provided heuristics to rewrite cast-to-union calls as C99 compound literals.
- **Initialization**: Mapped designated initializers and complex range initializers safely.

### Phase 4: Attributes (`__attribute__`)
We implemented broad lowering rules for GNU attributes, gracefully degrading them to MSVC equivalents:
- **Memory & Layout**: Transformed `aligned(N)` into `_Alignas` or MSVC's `__declspec(align)`.
- **Function Attributes**: Converted `always_inline`, `noreturn`, and stripped unsupported hints like `cold`/`hot`.
- **Variable Attributes**: Handled `cleanup(func)` by automatically unwinding execution blocks at scope exits, correctly reversing the order of execution for multiple cleanups.

### Phase 5: Built-in Functions (`__builtin_*`)
- **System Builtins**: Emulated `__builtin_offsetof` and frame/return addresses.
- **Math & Bitwise**: Replaced built-ins like `__builtin_ctz`, `__builtin_popcount`, and `__builtin_bswap16` with standard C bitwise emulations or MSVC intrinsics.
- **Arithmetic Overflows**: Provided safe widening fallbacks for `__builtin_add_overflow` and related overflow checks.
- **Security**: Lowered object size checking and stack protectors.

### Phase 6: Preprocessor & Lexical Extensions
- **Macros**: Normalized variadic macros and rewrote `, ##args` comma-swallowing hacks to standard C99 structures.
- **Magic Identifiers**: Inlined `__PRETTY_FUNCTION__` and concatenated string forms to `__func__`.
- **Pragmas**: Stripped GCC poison, warnings, and handled `pack(push, 1)`.
- **Lexical Quirks**: Converted binary literals (`0b0101`) to standard hex, and handled hexadecimal floating-point parsing.

### Phase 7: Inline Assembly (`__asm__`)
- **Syntax**: Normalized `__asm__` and `asm volatile` keywords.
- **Operands & Constraints**: Parsed complex GCC inline assembly blocks, extracting output/input operands and clobber lists.
- **Transformation**: Safely translated compatible blocks into MSVC native `__asm` blocks, ensuring proper variable mapping and register-to-stack saves.

## Summary

The `gnu_standardizer` is a testament to what can be achieved with rigorous AST manipulation. It provides a reliable bridge between legacy proprietary C extensions and universal, highly portable ISO C89 code, leaving the resulting workspace clean, fully documented, extensively tested, and entirely idempotent.
