# Decouple Plan: Invert Dependencies from cdd-c to Frameworks

This document outlines the step-by-step checklist to remove dependencies on `c-orm`, `c-rest-framework`, and `c-abstract-http` from `cdd-c`, and invert the architecture so those three frameworks depend on `cdd-c` instead. All steps strictly adhere to the requested quality guidelines.

## Phase 1: Clean Up `cdd-c` (Dependency Removal & Compliance)
- [x] Remove `FetchContent_Declare` and `FetchContent_Populate`/`FetchContent_MakeAvailable` calls for `c-orm`, `c-rest-framework`, and `c-abstract-http` from `cdd-c/CMakeLists.txt`.
- [x] Verify `cdd-c/CMakeLists.txt` uses `FetchContent_MakeAvailable` for any remaining dependencies (like `c-fs`), pulling the `master` branch.
- [x] Strip out `#include` directives referencing `c-orm`, `c-rest-framework`, or `c-abstract-http` across all `cdd-c` source files.
- [x] Refactor internal logic in `cdd-c` to safely remove any types or functions previously provided by those frameworks.
- [x] Ensure strict C89 compliance: convert all `//` comments to `/* */` and move variable declarations to the top of their scopes.
- [x] Wrap all public headers with exactly one `#ifdef __cplusplus` / `#endif /* __cplusplus */` block.
- [x] Wrap `#include` blocks with exactly one `/* clang-format off */` / `/* clang-format on */` per file.
- [x] Guard POSIX/C99 headers (e.g., `<stdint.h>`, `<stdbool.h>`, `<unistd.h>`) against older MSVC versions.
- [x] Eliminate `#include <windows.h>`, replacing with forward declarations or specific headers (e.g., `<winsock2.h>`).
- [x] Implement MSVC "Safe CRT" variants (e.g., `strcpy_s`, `sprintf_s`) behind `#if defined(_MSC_VER)`, falling back to standard C89 functions for GCC/Clang/MinGW.
- [x] Abstract printf format specifiers into macros (e.g., `#define NUM_FORMAT "%I64d"` vs `"%lld"`).
- [x] Verify CMake supports toggling: CRT Linkage (/MT vs /MD), Charsets (UNICODE/ANSI), Threading, LTO, and MSVC Runtime Checks (/RTC1, /RTCs, /RTCu).
- [x] Guarantee exactly 100% test coverage and 100% Doxygen-style documentation coverage.
- [x] Identify and fix any local CI/build/test failures in `cdd-c` without running `git push`.
- [x] Format `cdd-c` code by running: `fd -eh -ec -x clang-format -i --style=LLVM`

## Phase 2: Update `c-abstract-http`
- [x] Add `FetchContent_Declare` for `cdd-c` pointing to the `master` branch in `c-abstract-http/CMakeLists.txt`.
- [x] Use `FetchContent_MakeAvailable(cdd_c)`.
- [x] Refactor codebase to integrate and utilize `cdd-c` functionalities.
- [x] Apply C89 strict compliance (no `//` comments, top-of-scope declarations).
- [x] Add C++ header guards and `clang-format off/on` blocks to headers.
- [x] Add POSIX header guards and avoid `#include <windows.h>`.
- [x] Implement MSVC Safe CRT functions and abstract format specifiers.
- [x] Verify CMake supports toggling CRT Linkage, Charsets, Threading, LTO, and MSVC Runtime Checks.
- [x] Guarantee exactly 100% test and documentation coverage.
- [x] Format `c-abstract-http` code by running: `fd -eh -ec -x clang-format -i --style=LLVM`

## Phase 3: Update `c-rest-framework`
- [x] Add `FetchContent_Declare` for `cdd-c` pointing to the `master` branch in `c-rest-framework/CMakeLists.txt`.
- [x] Use `FetchContent_MakeAvailable(cdd_c)`.
- [x] Refactor codebase to integrate and utilize `cdd-c` functionalities.
- [x] Apply C89 strict compliance (no `//` comments, top-of-scope declarations).
- [x] Add C++ header guards and `clang-format off/on` blocks to headers.
- [x] Add POSIX header guards and avoid `#include <windows.h>`.
- [x] Implement MSVC Safe CRT functions and abstract format specifiers.
- [x] Verify CMake supports toggling CRT Linkage, Charsets, Threading, LTO, and MSVC Runtime Checks.
- [x] Guarantee exactly 100% test and documentation coverage.
- [x] Format `c-rest-framework` code by running: `fd -eh -ec -x clang-format -i --style=LLVM`

## Phase 4: Update `c-orm`
- [x] Add `FetchContent_Declare` for `cdd-c` pointing to the `master` branch in `c-orm/CMakeLists.txt`.
- [x] Use `FetchContent_MakeAvailable(cdd_c)`.
- [x] Refactor codebase to integrate and utilize `cdd-c` functionalities.
- [x] Apply C89 strict compliance (no `//` comments, top-of-scope declarations).
- [x] Add C++ header guards and `clang-format off/on` blocks to headers.
- [x] Add POSIX header guards and avoid `#include <windows.h>`.
- [x] Implement MSVC Safe CRT functions and abstract format specifiers.
- [x] Verify CMake supports toggling CRT Linkage, Charsets, Threading, LTO, and MSVC Runtime Checks.
- [x] Guarantee exactly 100% test and documentation coverage.
- [x] Format `c-orm` code by running: `fd -eh -ec -x clang-format -i --style=LLVM`

## Phase 5: Final Validation
- [x] If `vcpkg` is needed, verify it is utilized exactly via `DCMAKE_TOOLCHAIN_FILE="C:\%HOMEPATH%\repos\vcpkg\scripts\buildsystems\vcpkg.cmake"`.
- [x] Build all 4 projects locally under various configurations (MSVC, MinGW, Clang) to ensure compilable, clean states.
- [x] Run all local test suites to confirm 100% coverage and passing tests.
- [x] Validate no `git push` commands were executed throughout the process.
