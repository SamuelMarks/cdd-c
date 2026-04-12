# GNU C to Standard C (ISO) Transformation Plan

This document outlines the exhaustive, architectural, and feature-level roadmap required to build a 100% compliant GNU C to Standard C transformer within `cdd-c`.

To achieve zero edge cases and no exceptions, the transformer must move away from token-level string replacements to a full Abstract Syntax Tree (AST) manipulation engine with a semantic type-checker and Control Flow Graph (CFG) analysis.

### Core Mandate: The Zero-Destruction Rule
**Comments and whitespace MUST be rigorously preserved.** The generated ISO C code is meant for human developers, not just compilers. All transformations must be surgically applied to the AST without destroying surrounding "trivia" (formatting, indentation, blank lines, inline comments, or block documentation).

---

## Phase 0: Deliverable 1 - The `cdd-c` Minimum Viable Engine (MVE)
Before `cdd-c` can perform *any* complex GNU transformation (like unrolling a statement expression or converting a VLA), it must have the internal capabilities to understand the code geometrically and semantically. These are the immediate prerequisites for the first feature.

- [x] **Full CST-to-AST Lifting**
  - [x] Transform the flat token stream into a hierarchical Abstract Syntax Tree.
  - [x] Ensure `cdd_cst_trivia` attaches comments correctly to nearest parent nodes.
  - [x] Preserve whitespace exactly, mapping column/line metadata.
  - [x] Preserve inline comments (`//`).
  - [x] Preserve block comments (`/* */`).
  - [x] Handle macro expansions transparently without un-expanding in output.
  - [x] Source map generation for error tracking and Clang-style carets.
  - [x] AST visualization tool (graphviz/dot output for debugging).
  - [x] Syntax tree deduplication (memory efficiency).
  - [x] Deep copying of AST nodes for duplication tasks.
  - [x] Custom memory pool allocator for AST nodes.
- [x] **Scope-Aware Symbol Table**
  - [x] Track variables per block-scope.
  - [x] Track typedefs per block-scope.
  - [x] Track structs per block-scope.
  - [x] Track unions per block-scope.
  - [x] Track enums and individual enum variants.
  - [x] Track labels per function-scope.
  - [x] Handle shadowed variables seamlessly.
  - [x] Handle shadowed typedefs.
  - [x] Handle shadowed struct/union tags.
  - [x] File-scope (translation unit) state management.
- [x] **Compile-Time Type Evaluator**
  - [x] Resolve the exact type of any complex AST expression.
  - [x] Evaluate `sizeof()` internally for target architectures.
  - [x] Evaluate `_Alignof()` / `__alignof__()` internally.
  - [x] Handle architecture-specific pointer sizes dynamically.
  - [x] Track `const`, `volatile`, and `restrict` qualifiers perfectly.
  - [x] Evaluate integer constant expressions (ICE) fully at compile-time.
- [x] **Control Flow Graph (CFG) Builder**
  - [x] Track all entry points of a lexical scope.
  - [x] Track normal exits.
  - [x] Track `return` exits.
  - [x] Track `goto` jumps (forward and backward).
  - [x] Track `break` jumps from loops and switches.
  - [x] Track `continue` jumps in loops.
  - [x] Detect dead code segments reliably.
  - [x] Identify cyclic dependencies in blocks.
- [x] **AST Mutator Framework**
  - [x] API to safely detach nodes without leaking memory.
  - [x] Synthesize new standard C nodes programmatically.
  - [x] Re-attach nodes safely while fixing up parent pointers.
  - [x] Temporary variable generator.
  - [x] Guaranteed collision-free name mangling (e.g., `__cdd_tmp_42`).
  - [x] Auto-scoping of synthesized variables to the tightest possible block.
  - [x] Pretty-printer for serializing the mutated AST back to C.

---

## Phase 1: Type System & Declarations (Edge Cases Expanded)
GNU C extends the type system in ways that require complex lowering, runtime emulation, or deep semantic rewriting.

- [x] **128-bit Integers (`__int128`)**
  - [x] Parse `__int128` and `unsigned __int128`.
  - [x] Literals exceeding 64-bit bounds without standard suffixes.
  - [x] Implicit casting to/from `float`.
  - [x] Implicit casting to/from `double`.
  - [x] Varargs passing rules (must be passed as structs in standard C, changing ABI).
  - [x] Emulation via `struct { uint64_t low, high; }`.
  - [x] Addition/Subtraction lowering via function calls or inline expansion.
  - [x] Multiplication/Division lowering.
  - [x] Bitwise operations lowering.
- [x] **Compile-Time Type Introspection**
  - [x] `typeof` applied to basic types.
  - [x] `typeof` applied to arrays (decay behavior checks).
  - [x] `typeof` applied to bit-fields (must resolve to underlying integer).
  - [x] `__auto_type` inference handling.
  - [x] `typeof_unqual` (stripping qualifiers).
  - [x] **Arrays & Sizing**
    - [x] **Variable Length Arrays (VLAs)**
      - [x] `sizeof(VLA)` with side-effects (e.g., `int sz = sizeof(int[x++]);`).
      - [x] Evaluating VLA size expressions dynamically on scope entry.
      - [x] `goto` jumping *into* a VLA scope safely.
      - [x] `goto` jumping *out* of a VLA scope safely.
      - [x] `alloca()` based lowering (where available).
      - [x] `malloc()/free()` fallback lowering.
      - [x] Multidimensional VLAs (`int matrix[rows][cols]`).
      - [x] VLA function parameters (`void func(int n, int arr[n])`).
    - [x] **Zero-length arrays (`int arr[0];`)**
      - [x] At the end of a struct (Flexible Array Member polyfill).
      - [x] In the middle of a struct (Requires rewriting struct layout).
      - [x] `sizeof` behavior on zero-length arrays.
      - [x] Initialization of zero-length arrays in static contexts.
- [x] **Complex Numbers (`__complex__`)**
  - [x] `__complex__ int` parsing.
  - [x] `__complex__ float` / `double` parsing.
  - [x] `__real__` part extraction lowering.
  - [x] `__imag__` part extraction lowering.
  - [x] Polyfill via `struct { real, imag; }`.
  - [x] Arithmetic lowering (`+`, `-`, `*`, `/`).
- [x] **Vector Types**
  - [x] `__attribute__((vector_size(N)))` parsing.
  - [x] Vector element access via standard array indexing `v[0]`.
  - [x] Vector addition / subtraction rewriting.
  - [x] Vector bitwise operations rewriting.
  - [x] Implicit casting between equivalently sized vectors.
  - [x] `__builtin_shuffle` / `__builtin_shufflevector` lowering.
- [x] **Floating Point Extensions**
  - [x] `_Decimal32`, `_Decimal64`, `_Decimal128` parsing.
  - [x] Half-precision floats (`__fp16`, `_Float16`).
  - [x] Bfloat16 types (`__bf16`).
  - [x] Hexadecimal float literal rewriting (`0x1.fp3` to standard decimal equivalent).
- [x] **Struct/Union Extensions**
  - [x] Anonymous Structs.
  - [x] Anonymous Unions.
  - [x] Cast to Union as Lvalue (`(union U)val = ...`).
  - [x] Transparent Unions (`__attribute__((transparent_union))`).
  - [x] Empty structs `struct foo {};` (size 0 in GNU, UB in Standard).
  - [x] Packed structs `__attribute__((packed))`.
  - [x] Unaligned pointer access to packed structs (requires byte-by-byte copies).

---

## Phase 2: Control Flow & Scoping (The Hard Stuff)
These features fundamentally alter the execution graph of C and require complex scope rewriting.

- [x] **Nested Functions**
  - [x] Nested function syntax parsing.
  - [x] Lambda Lifting / Closure Conversion logic.
  - [x] Shared closure struct for multiple nested functions in same parent.
  - [x] Trampoline detection (returning nested function pointers).
  - [x] Emitting hard translation errors for impossible stack trampolines.
  - [x] Trampoline emulation via dynamic executable heap allocators (target specific).
- [x] **Computed Gotos (Labels as Values)**
  - [x] `&&label_name` syntax parsing.
  - [x] `goto *ptr;` syntax parsing.
  - [x] Storing label pointers in arrays/structs.
  - [x] State machine conversion.
  - [x] Converting jump tables to `switch` statements internally.
  - [x] Handling computed gotos crossing VLA scopes (stack pointer restoration).
- [x] **Statement Expressions (`({ ... })`)**
  - [x] Parsing block as expression and inferring return type.
  - [x] `goto` jumping out of statement expression.
  - [x] Embedded inside `while()` condition.
  - [x] Embedded inside `for()` loop header.
  - [x] Embedded inside `if()` condition.
  - [x] Type resolution of the final expression accurately.
  - [x] Hoisting side-effects out of expression contexts safely.
- [x] **Local Labels (`__label__`)**
  - [x] Parsing `__label__ foo, bar;`.
  - [x] Lexical scoping constraints of local labels.
  - [x] Renaming local labels during code generation to prevent global collisions.
- [x] **Extended Switch Statements**
  - [x] Case ranges (`case 1 ... 5:`).
  - [x] Case ranges with negative values (`case -5 ... -1:`).
  - [x] Overlapping case ranges (semantic error detection and resolution).
  - [x] Empty block `{}` fallthrough handling correctly in loop transformations.
  - [x] Function returns with `void` expressions (`return (void)0;`).

---

## Phase 3: Expressions & Operators
- [x] **Conditionals**
  - [x] Omitted operands (`x ? : y`).
  - [x] `volatile` variables in omitted operands (strict single evaluation).
  - [x] Side-effect duplication prevention via temporaries.
- [x] **Pointers & Memory**
  - [x] Pointer arithmetic on `void*`.
  - [x] Pointer arithmetic on function pointers.
  - [x] Escaping pointers from compound literals.
- [x] **Casts and Lvalues**
  - [x] Lvalues of Cast Expressions (`(int)x = 5;`).
  - [x] Casts to union types lowering.
- [x] **Initialization**
  - [x] Designated Initializers (C99 mapping vs GNU syntax variations).
  - [x] Range initializers `[0 ... 9] = 1`.
  - [x] Overlapping initializers precedence rules.
  - [x] Unspecified array sizes derived strictly from designated max index.
  - [x] Trailing commas in complex initializer lists.
  - [x] Compound literals `(struct foo){1, 2}` cast to different pointers.
- [x] **Alignments & Quirks**
  - [x] `__alignof__` vs `_Alignof` differences.
  - [x] `__alignof__` applied to expressions vs types.
  - [x] Parsing and stripping `__extension__` keyword.
  - [x] Suppressing pedantic warnings internally when `__extension__` is present.

---

## Phase 4: Attributes (`__attribute__((...))`)
- [x] **Memory & Layout Attributes**
  - [x] `mode(XX)` (QI, HI, SI, DI, TI).
  - [x] `aligned(N)` (lowered to `_Alignas` or MSVC `__declspec(align)`).
  - [x] `section("name")` mapping.
  - [x] `packed` mapping.
  - [x] `may_alias` (strict aliasing bypass logic).
- [x] **Function Attributes**
  - [x] `alias("target")` (Generate wrapper or linker directives).
  - [x] `constructor(priority)` translation.
  - [x] `destructor(priority)` translation.
  - [x] Translation-unit level sorting of initialization priorities.
  - [x] `always_inline`.
  - [x] `noinline`.
  - [x] `flatten`.
  - [x] `cold` / `hot`.
  - [x] `format(printf, i, j)` validation lowering.
  - [x] `noreturn` (lowering to `_Noreturn` or `[[noreturn]]`).
  - [x] `returns_twice`.
  - [x] `pure` / `const`.
  - [x] `malloc` / `alloc_size` / `alloc_align`.
  - [x] `interrupt` (target specific rewriting).
  - [x] `weak` / `weakref`.
  - [x] `naked` (raw asm bodies).
  - [x] `nonnull` / `returns_nonnull`.
  - [x] `warn_unused_result` to `[[nodiscard]]`.
- [x] **Variable/Type Attributes**
  - [x] **`cleanup(func)`**
    - [x] Variable goes out of scope normally.
    - [x] Variable goes out of scope via `goto`.
    - [x] Variable goes out of scope via `return`.
    - [x] Multiple cleanups in one scope (correct reverse order of execution).
    - [x] Interaction with `longjmp` (document as unsupported or inject hooks).
  - [x] `deprecated("msg")`.
  - [x] `unused` / `used`.
  - [x] `retain`.

---

## Phase 5: Built-in Functions (`__builtin_*`)
- [x] **Compile-time Evaluation**
  - [x] `__builtin_constant_p` evaluation constraints.
  - [x] `__builtin_choose_expr` branch selection.
  - [x] Discarding untaken branch AST in `choose_expr` to prevent false semantic errors.
  - [x] `__builtin_types_compatible_p` implementation.
- [x] **Memory & System**
  - [x] `__builtin_offsetof`.
  - [x] `__builtin_offsetof` applied to a member of an incomplete struct.
  - [x] `__builtin_frame_address`.
  - [x] `__builtin_return_address`.
  - [x] `__builtin_extract_return_addr`.
  - [x] `__builtin_frob_return_addr`.
  - [x] Deep macro stacks interacting with return addresses.
  - [x] Emitting hard errors for un-emulatable system builtins.
- [x] **Math & Bitwise**
  - [x] `__builtin_ctz` (count trailing zeros).
  - [x] `__builtin_clz` (count leading zeros).
  - [x] `__builtin_popcount` (population count).
  - [x] `__builtin_ffs` (find first set).
  - [x] `__builtin_bswap16`, `32`, `64`.
  - [x] Emulating bitwise operations gracefully with standard C polyfills.
- [x] **Arithmetic Overflows**
  - [x] `__builtin_add_overflow`.
  - [x] `__builtin_sub_overflow`.
  - [x] `__builtin_mul_overflow`.
  - [x] Emulating overflow checks using widened types or bit hacks.
- [x] **Synchronization & Atomics**
  - [x] `__sync_fetch_and_add`.
  - [x] `__sync_val_compare_and_swap`.
  - [x] `__atomic_load_n`.
  - [x] Mapping memory models to C11 `<stdatomic.h>`.
- [x] **Flow & Optimization**
  - [x] `__builtin_expect` (strip down to `exp`).
  - [x] `__builtin_expect_with_probability`.
  - [x] `__builtin_prefetch`.
  - [x] `__builtin_unreachable()` mapped to `abort()`.
  - [x] `__builtin_assume_aligned`.
- [x] **String & Security**
  - [x] `__builtin_strlen`, `__builtin_memcpy` (map to standard `<string.h>`).
  - [x] Object size checking (`__builtin_object_size`).
  - [x] `__builtin___memcpy_chk`.
  - [x] Stack protectors (`__builtin_stack_chk_fail`).
- [x] **Variadics & Calls**
  - [x] `__builtin_va_arg_pack()`.
  - [x] `__builtin_va_arg_pack_len()`.
  - [x] `__builtin_apply_args`.
  - [x] `__builtin_apply`.
  - [x] `__builtin_return`.

---

## Phase 6: Preprocessor & Lexical Extensions
The preprocessor must run or be deeply understood before/during AST generation.

- [x] **Macros**
  - [x] Variadic Macros `MACRO(args...)`.
  - [x] Comma Swallowing `, ##args`.
  - [x] Rewriting `, ##args` to `__VA_OPT__` (C23) or standard C99 hacks.
  - [x] Multi-line macros with trailing line comments correctly tokenized.
- [x] **Magic Identifiers**
  - [x] `__FUNCTION__` rewriting.
  - [x] `__PRETTY_FUNCTION__` rewriting.
  - [x] `__func__` equivalence handling.
  - [x] String concatenation involving magic identifiers (e.g. `"Error in " __FUNCTION__` to `snprintf`).
- [x] **Pragmas**
  - [x] `#pragma GCC diagnostic push/pop`.
  - [x] `#pragma GCC poison`.
  - [x] `#pragma pack(push, 1)`.
  - [x] `#pragma weak`.
- [x] **Include Directives**
  - [x] `#include_next` path tracking dynamically.
- [x] **Lexical Quirks**
  - [x] Identifiers containing `$`.
  - [x] Binary constants (`0b0101`) to hex conversion.
  - [x] Hexadecimal floating-point (`0x1.fp3`).
  - [x] Escaped newlines inside string literals.
  - [x] `__BASE_FILE__`.
  - [x] `__INCLUDE_LEVEL__`.
  - [x] `__TIMESTAMP__`.
  - [x] `__COUNTER__` instantiation tracking.
  - [x] `#warning`.
  - [x] `#error`.

---

## Phase 7: Inline Assembly (`__asm__`)
Translating architecture-specific assembly blocks requires advanced parsing of strings.

- [x] **Syntax & Modifiers**
  - [x] Basic inline assembly parsing.
  - [x] `__asm__` vs `asm` keyword normalization.
  - [x] `volatile` modifier parsing.
  - [x] `goto` modifier parsing (`asm goto`).
- [x] **Operands & Constraints**
  - [x] Output operands list parsing.
  - [x] Input operands list parsing.
  - [x] Clobber list parsing.
  - [x] Symbol references binding.
  - [x] Constraint string parsing (`"r"`, `"m"`, `"memory"`, etc.).
  - [x] x86 specific constraint (`"a"`, `"b"`, `"c"`, `"d"`, `"S"`, `"D"`) parsing.
  - [x] ARM specific constraint parsing.
- [x] **Transformation & Lowering**
  - [x] Translating to MSVC `__asm` blocks natively.
  - [x] Validating input/output mappings.
  - [x] Converting `asm goto` to Standard C equivalents where possible.
  - [x] Emitting target-specific pragmas.
  - [x] Translating clobbered registers to stack saves automatically.