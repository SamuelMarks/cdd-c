# Exhaustive Expansion Plan for `cdd-c` (CST Weaver for MSVC Migration)

To achieve PR-ready, pre-processor-intertwined code that upstream maintainers will accept, `cdd-c` must evolve from a standard parser/generator into an **Advanced Concrete Syntax Tree (CST) Weaver**. Standard AST parsers discard whitespace, comments, and macro expansions, which destroys the original formatting. To generate acceptable patches, `cdd-c` must preserve the exact byte-for-byte layout of unmodified code while cleanly injecting MSVC/Windows compatibility layers.

Here is the exhaustive roadmap for extending `cdd-c` to perfectly support the `auto-win-msvc` project and handle the extreme complexities of legacy C codebases.

## 1. High-Fidelity Concrete Syntax Tree (CST) & Core Engine
* **[x] Continuous Token Stream Attachment:** Modify the parser to keep a continuous, doubly-linked stream of original tokens (including all whitespace, newlines, comments, and line continuations `\`) attached to the AST nodes.
* **[x] Non-Destructive Rewriting (The "Printer"):** Implement an emitter that traverses the original token stream, writing exactly the original text unless an AST node is explicitly flagged as mutated.
* **[x] Source Map & Location Tracking:** Track original file, line, and column numbers for every token, even after macro expansion, to generate accurate compiler diagnostics and `diff` patches.
* **[x] Trigraph & Digraph Preservation:** Ensure legacy character sequences (e.g., `??!`, `<:`) are parsed and restored exactly as they were authored.

## 2. Advanced Preprocessor & Macro Handling
* **[x] Macro Invocation vs. Expansion Tracking:** The CST must represent a macro call (e.g., `MY_MACRO(x)`) as a distinct manipulable node, while also holding its expanded AST representation underneath. Edits should happen on the macro *invocation* when possible, not the expanded output.
* **[x] `#include` Graph Analysis:** Parse `#include` directives as manipulable nodes. Resolve local vs. system includes (`""` vs `<>`) to determine where POSIX headers are entering the translation unit.
* **[x] Conditional Compilation Awareness:** Parse `#if`, `#ifdef`, `#elif`, `#else`, `#endif` as structural nodes. The parser must optionally parse *all* branches of an `#ifdef` (multiplexing the AST) to ensure transformations don't break code for other platforms.
* **[x] Token Pasting (`##`) and Stringification (`#`):** Accurately model and rewrite macros that use complex C preprocessor features.

## 3. Semantic Analysis, Type Inference, & Data Flow
* **[x] Cross-Platform Type Resolution (LP64 vs. LLP64):** Identify variables typed as `long`. In POSIX (LP64), `long` is 64-bit; in MSVC (LLP64), it is 32-bit. Detect size-dependent usage and inject `#ifdef _MSC_VER` mappings to `long long` or `int64_t`.
* **[x] Buffer Size Inference for CRT Secure Functions:** Implement data-flow analysis to infer the size of arrays or buffers (e.g., finding the `sizeof(dest)` when transforming `strcpy(dest, src)` into `strcpy_s(dest, size, src)`).
* **[x] Handle vs. Descriptor Disambiguation:** Differentiate `int` used as an integer from `int` used as a POSIX file descriptor (fd) or POSIX socket descriptor. This is required for mapping fds to Win32 `HANDLE`s or sockets to WinSock `SOCKET`s.
* **[x] Scope Awareness:** Detect local vs. global variable usage to determine the safest place to inject polyfill initializations (e.g., calling `WSAStartup()` in `main()`).

## 4. Compiler Extension & Dialect Translation (GCC/Clang -> MSVC)
* **[x] `__attribute__` to `__declspec` / `#pragma` Translation:**
  * `__attribute__((packed))` -> Wrap structure in `#pragma pack(push, 1)` and `#pragma pack(pop)`.
  * `__attribute__((visibility("default")))` -> `__declspec(dllexport)` / `dllimport`.
  * `__attribute__((unused))` -> `(void)var;` or `UNREFERENCED_PARAMETER(var);`.
  * `__attribute__((noreturn))` -> `__declspec(noreturn)`.
  * `__attribute__((format(printf, ...)))` -> `_Printf_format_string_` (SAL annotations).
* **[x] Built-in Functions (`__builtin_*`):**
  * `__builtin_expect(expr, c)` -> `(expr)` (strip it, as MSVC lacks exact `__builtin_expect`).
  * `__builtin_popcount` -> `__popcnt` / `<intrin.h>`.
  * `__builtin_clz` -> `_BitScanReverse`.
  * `__builtin_ctz` -> `_BitScanForward`.
* **[x] Inline Assembly Translation:** Detect GCC/Clang style `__asm__ volatile ("...")` and either flag for manual review or wrap in an `#ifdef _MSC_VER` with a stub, as MSVC x64 does not support inline assembly.
* **[x] Keyword Mapping:** Detect `inline`, `__inline__` and normalize to `__inline` for MSVC compatibility in strict C89 mode.

## 5. C Standard Adaptation (C99/C11 -> MSVC-Compatible C)
* [x] **Variable Length Array (VLA) Translation:** Detect VLAs (e.g., `int arr[n];`). Transform the AST to allocate via `_alloca()` or `malloc()/free()`: `int *arr = (int *)_alloca(n * sizeof(int));`.
* [x] **Declaration Hoisting (Mixed Code and Declarations):** For older MSVC compatibility (strict C89), identify variables declared in the middle of a block and rewrite the AST to hoist their declarations to the top of the block.
* [x] **Designated Initializers:** Identify C99 designated initializers (e.g., `struct S s = { .x = 1 };`) and translate them to ordered initialization or runtime assignment if targeting older MSVC versions.
* **[x] `complex.h` Workarounds:** Detect C99 complex types (`_Complex`, `double complex`) and map them to MSVC's `<complex>` (C++) or equivalent struct representations.

## 6. POSIX to MSVC/Win32 AST Transformations
* **[x] Argument Reordering & Injection (Annex K):** Automatically transform unsafe standard functions to their MSVC `_s` equivalents (e.g., `sprintf` -> `sprintf_s`, `fopen` -> `fopen_s`). Synthesize the required size parameters cleanly.
* **[x] Format String Parser & `<inttypes.h>` Injection:** Parse literal format strings in `printf`/`scanf`.
  * Transform `%zu` to `%Iu` or inject standard `<inttypes.h>` macros (e.g., `"Value: %" PRId64 "\n"`) using string literal concatenation.
* **[x] File I/O Binary Mode Enforcement:** Detect `fopen(path, "r")` and `open(path, O_RDONLY)` and optionally inject `O_BINARY` or `"rb"` if the file is known to be a binary format (avoids CR/LF translation corruption on Windows).
* **[x] `errno` Translation:** Map POSIX `errno` checks to `GetLastError()` checks where direct CRT equivalents do not exist, mapping POSIX error codes (e.g., `EAGAIN`, `EINTR`) to Win32 error codes (e.g., `WSAEWOULDBLOCK`).
* **[x] Standard Type Replacement:** Swap standard C types for Windows types in function signatures or variable declarations (e.g., `pthread_t` -> `HANDLE`, `pid_t` -> `DWORD`), wrapping the declaration in an `#ifdef _MSC_VER` block.

## 7. The Weaver Engine: Safe Injection & Patching
* [x] The `#ifdef` Wrapper API: High-level API to cleanly wrap an existing AST node, statement, or contiguous set of statements in conditional compilation blocks without messing up indentation.
  * *Input:* Target Node, Condition (`_MSC_VER`), True-Block AST, False-Block AST.
  * *Output:* Woven token stream with correct local indentation applied to the injected macros.
* [x] **Header Weaving API:** API to safely append MSVC headers adjacent to existing POSIX headers.
  * Handles injecting `#define WIN32_LEAN_AND_MEAN` and `#define NOMINMAX` *before* `<windows.h>`.
  * Orders `<winsock2.h>` explicitly *before* `<windows.h>` to prevent macro clashes.
* [x] **Patch / Diff Emitter:** Instead of just overwriting the file, output a standard Unified Diff (`.patch` file) that maintainers can easily review and apply.
* [x] **Interactive Review Mode:** A CLI flag to pause and prompt the user for confirmation when a complex transformation (like a VLA to `_alloca` translation) is identified.

## 8. Build System Translation & Emitters
* [x] **CMake Parser/Generator:** Since `cdd-c` focuses on C, it needs an auxiliary module to parse basic `CMakeLists.txt` structures to append MSVC-specific `target_compile_options` (e.g., `/W4`, `/WX`, `/D_CRT_SECURE_NO_WARNINGS`) and link standard Windows libraries (`ws2_32.lib`, `Bcrypt.lib`).
* [x] **Autotools / Makefile Scraper:** Extract source files, include directories, and compile definitions from legacy `Makefile` or `configure.ac` scripts to bootstrap modern `CMakeLists.txt` files automatically.
* [x] **Vcpkg Integration Generator:** Scan the required `#include` tree to automatically generate a `vcpkg.json` manifest identifying dependencies (e.g., `pthreads`, `dirent`, `zlib`).

## 9. Current Status & Future Plans

**Current Status (`auto-win-msvc` integration):**
- The `auto-win-msvc` monorepo has been successfully scaffolded into 18 distinct, modular CMake projects.
- Simple functions with direct MSVC equivalents (e.g., `open` -> `_open`) are fully mapped via macros.
- Complex POSIX APIs (e.g., `mmap`, `pthreads`, `dirent`) are scaffolded as `ENOSYS` stubs with target Win32 APIs documented in `mappings.json`.

**Future Plans:**
- **AI-Driven Iteration:** Iteratively implement all stubbed polyfills using native Win32 APIs across the 18 modules, maintaining strict C89 compliance.
- **cdd-c CST Engine Overhaul:** Complete the transition to the full CST Weaver engine described in this document.
- **Automated Upstream PRs:** Use `cdd-c` to clone top open-source C repositories, automatically weave MSVC support, verify the build via GitHub Actions, and generate PRs.

## 10. Immediate Action Items for `cdd-c`:
1. [x] **Parser Token Attachment:** Validate the current parsing engine's ability to retain whitespace/comments. If it drops them, switch the backend parser immediately to a CST-friendly one (e.g., adapting an existing library like `tree-sitter-c` with a token preservation layer, or extending a Clang-based LibTooling approach).
2. [x] **Weaver Proof-of-Concept:** Implement the `Weaver Engine` primitive to successfully wrap a single `printf("hello")` into an `#ifdef _MSC_VER` block and write it back to disk perfectly formatted.
3. [x] **Macro AST Overlay:** Implement the architecture for dual-representing macro invocations (the raw text tokens vs. the expanded syntax tree) so `#define` logic isn't destroyed during rewrites.
