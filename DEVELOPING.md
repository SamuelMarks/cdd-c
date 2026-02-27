# Developing `cdd-c`

This guide explains how to set up, build, and test `cdd-c` locally.

## Prerequisites

To build the toolchain and run its tests, you will need:
*   **CMake:** Minimum version 3.11.
*   **A C Compiler:** GCC, Clang, or MSVC (Visual Studio 2022 recommended on Windows).
*   **vcpkg:** The C++ package manager is used to automatically fetch dependencies like `parson` (JSON parser).
*   **Git**

## Setup and Build

1.  **Install vcpkg:**
    ```bash
    git clone https://github.com/microsoft/vcpkg.git -b 2024.04.26  # Or a compatible baseline
    cd vcpkg
    # On Windows:
    .\bootstrap-vcpkg.bat
    # On Linux/macOS:
    ./bootstrap-vcpkg.sh
    cd ..
    ```

2.  **Clone `cdd-c`:**
    ```bash
    git clone https://github.com/SamuelMarks/cdd-c.git
    cd cdd-c
    ```

3.  **Configure the Build with CMake:**
    The project requires specific CMake arguments to find the `vcpkg` toolchain and enable testing.

    *   **Windows:**
        ```cmd
        cmake -DCMAKE_BUILD_TYPE="Debug" -DBUILD_TESTING=ON -DC_CDD_BUILD_TESTING=ON -DCMAKE_TOOLCHAIN_FILE="..\vcpkg\scripts\buildsystems\vcpkg.cmake" -S . -B "build"
        ```
    *   **Linux / macOS:**
        ```bash
        cmake -DCMAKE_BUILD_TYPE="Debug" -DBUILD_TESTING=ON -DC_CDD_BUILD_TESTING=ON -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" -S . -B "build"
        ```

4.  **Compile the Code:**
    ```bash
    cmake --build "build" --parallel
    ```

## Testing

`cdd-c` uses the `greatest.h` testing framework internally (downloaded automatically via CMake).

Run tests using CTest:
```bash
cd build
ctest -C Debug --verbose
```
Or run the test binary directly:
```bash
./bin/test_c_cdd
```

## Adding Code

*   **Structure:**
    *   `src/classes/` contains parsers and emitters for objects (structs, schemas).
    *   `src/functions/` contains function-level parsers, code generators, and network abstraction layers.
    *   `src/openapi/` contains the OpenAPI specific logic.
    *   `src/tests/` contains all unit and integration tests.
*   **CMake:** Ensure you add new `.c` and `.h` files to `src/CMakeLists.txt` or `src/tests/CMakeLists.txt` and run `cmake -B build` to refresh the build files.
*   **Code Style:**
    *   Adhere to C89 standards (no `//` comments, declare variables at the top of scopes).
    *   Run `clang-format` if available before committing.
    *   Avoid dynamic allocations where `c-str-span` (string views) can be used.

## Submitting a PR

1.  Create a feature branch.
2.  Write tests for any new parsers or emitters (add headers to `test_c_cdd.c`).
3.  Ensure the build completes and `ctest` passes locally.
4.  Commit changes and push to GitHub. GitHub Actions will verify your code across Linux, Windows, and macOS.
