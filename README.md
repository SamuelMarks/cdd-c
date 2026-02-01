cdd-c
=====

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Linux, Windows, macOS](https://github.com/SamuelMarks/cdd-c/actions/workflows/linux-Windows-macOS.yml/badge.svg)](https://github.com/SamuelMarks/cdd-c/actions/workflows/linux-Windows-macOS.yml)
[![C89](https://img.shields.io/badge/C-89-blue)](https://en.wikipedia.org/wiki/C89_(C_version))

**cdd-c** is a production-ready Code-Driven-Development toolchain for C. Its primary capability is generating complete, type-safe, portable C89 HTTP client libraries (SDKs) from OpenAPI 3.0+ specifications.

Stop writing boilerplate HTTP code. Generate it. Bidirectionally.

## Key Features

### 🚀 OpenAPI to C SDK
Turn any `openapi.json` into a compilation-ready C library.
-   **Transport Agnostic (ANI):** The generated code uses an Abstract Network Interface. It compiles against **libcurl** (Linux/macOS) or **WinHTTP/WinInet** (Windows) natively.
-   **Type-Safe:** Request bodies, query parameters, and response models are generated as strongly-typed C structs.
-   **Dependencies:** The generated code only requires `parson` (JSON) and your OS's HTTP stack.
-   **Features:**
    -   JSON serialization/deserialization.
    -   `multipart/form-data` and `application/x-www-form-urlencoded` support.
    -   Standardized `ApiError` structures (RFC 7807).
    -   Authentication (Bearer Token, API Key).
    -   Auto-generated `CMakeLists.txt` for immediate integration.

### 🛠️ Refactoring & Safety Tools
Includes legacy tools for analyzing and upgrading C codebases:
-   **Audit:** Scans projects for unchecked `malloc` usage.
-   **Refactor:** Automatically rewrites `void func()` signatures to `int func()` to propagate error codes up the stack.

## Installation

### Prerequisites
*   CMake 3.10+
*   C Compiler (GCC, Clang, MSVC)
*   **Dependencies (fetched automatically via CMake/Git):**
    *   `c89stringutils`
    *   `parson`
    *   `c-str-span`

### Build from Source

```bash
git clone https://github.com/SamuelMarks/cdd-c.git
cd cdd-c
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
# Optional: Install globally
# sudo cmake --install build
```

## Quick Usage

### Generate a Client SDK

```bash
# 1. Download your spec
curl https://petstore.swagger.io/v2/swagger.json -o petstore.json

# 2. Generate the C library
./build/c_cdd_cli openapi2client petstore.json petstore_sdk

# 3. Build your new SDK
cd petstore_sdk_build
cmake -S . -B build
cmake --build build
```

See the [SDK Tutorial](docs/tutorial_sdk.md) for a step-by-step integration guide.

## CLI Reference

### `openapi2client`
**Usage:** `openapi2client <spec.json> <out_basename>`
Generates a header (`.h`), source (`.c`), and build script (`CMakeLists.txt`) for an OpenAPI document.

### `audit`
**Usage:** `audit <directory>`
Performs static analysis to find memory safety violations (unchecked pointers).

### `fix`
**Usage:** `fix <directory> --in-place`
Applies AST-based refactoring to injecting safety checks and standardize error handling logic.

### `code2schema`
**Usage:** `code2schema <header.h> <schema.json>`
Extracts C struct definitions into JSON Schema format.

## Architecture

The toolchain operates as a pipeline:
1.  **Tokenizer/CST:** Parses C code or JSON specs into structured trees.
2.  **Analysis/Loader:** Validates types, detects allocation sites, or loads OpenAPI definitions.
3.  **Generator/Rewriter:** 
    *   *For SDKs:* Emits C implementation strings via `codegen_*` modules.
    *   *For Refactoring:* Patches existing source text via `text_patcher`.

## Documentation

*   [Tutorial: Generating an SDK](docs/tutorial_sdk.md)
*   [API Reference](docs/api.md)

## License

Licensed under either of Apache License, Version 2.0 or MIT license at your option.
```

`docs/tutorial_sdk.md`:

```md
# Tutorial: Generating a C Client SDK

This guide walks you through generating a portable C89 client library from an OpenAPI specification using `cdd-c`.

## 1. Prerequisites

Ensure you have built the `c_cdd_cli` tool:

```bash
git clone https://github.com/SamuelMarks/cdd-c
cd cdd-c
cmake -S . -B build
cmake --build build
```

The executable will be located at `build/c_cdd_cli` (or `build/Debug/c_cdd_cli.exe` on Windows).

## 2. Obtain an OpenAPI Specification

You can use any valid OpenAPI 3.0+ JSON file. For this tutorial, we will use a minimal Petstore definition. Save the following content as `petstore.json`:

```json
{
  "openapi": "3.0.0",
  "info": {
    "title": "Minimal Petstore",
    "version": "1.0.0"
  },
  "servers": [
    { "url": "https://api.example.com/v1" }
  ],
  "paths": {
    "/pets/{petId}": {
      "get": {
        "operationId": "getPet",
        "parameters": [
          {
            "name": "petId",
            "in": "path",
            "required": true,
            "schema": { "type": "integer" }
          }
        ],
        "responses": {
          "200": {
            "description": "Found",
            "content": {
              "application/json": {
                "schema": { "$ref": "#/components/schemas/Pet" }
              }
            }
          }
        }
      }
    }
  },
  "components": {
    "schemas": {
      "Pet": {
        "type": "object",
        "properties": {
          "id": { "type": "integer" },
          "name": { "type": "string" }
        }
      }
    },
    "securitySchemes": {
      "BearerAuth": {
        "type": "http",
        "scheme": "bearer"
      }
    }
  }
}
```

## 3. Generate the SDK

Run the `openapi2client` command. This reads the spec and produces the source code.

```bash
# Syntax: openapi2client <spec_file> <output_base_name>
./build/c_cdd_cli openapi2client petstore.json petstore_client
```

This will generate the following files:
*   `petstore_client.h`: Function prototypes and structs (`struct Pet`).
*   `petstore_client.c`: Implementation of the HTTP logic.
*   `CMakeLists.txt`: A ready-to-use build script for the library.

It also relies on internal runtime files (like `http_curl.c`) which the generator assumes are present or linked. 

## 4. Build the SDK

The generated `CMakeLists.txt` automatically handles dependencies (like libcurl or WinHTTP). 

Create a build directory for your new library:

```bash
mkdir sdk_build
cd sdk_build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

This produces a static library (e.g., `libpetstore_client.a` or `petstore_client.lib`).

## 5. Using the Library

Now you can write a C program that consumes your new SDK.

**`main.c`:**

```c
#include <stdio.h>
#include <stdlib.h>
#include "petstore_client.h"

int main(void) {
    struct HttpClient client;
    struct Pet *my_pet = NULL;
    struct ApiError *err = NULL;
    int rc;

    /* 1. Initialize the client */
    /* Pass NULL as base_url to use the default from the Spec */
    rc = api_init(&client, "https://api.example.com/v1");
    if (rc != 0) {
        fprintf(stderr, "Failed to init client: %d\n", rc);
        return 1;
    }

    /* 2. Configure Authentication (if needed) */
    /* Checks if ctx->security exists logic from codegen_security */
    // client.security.bearer_token = "my-secret-token"; 

    /* 3. Call the API */
    /* int api_getPet(ctx, petId, out_obj, error_obj) */
    rc = api_getPet(&client, 123, &my_pet, &err);

    if (rc == 0) {
        printf("Success! Pet Name: %s\n", my_pet->name);
        /* 4. Cleanup Response */
        Pet_cleanup(my_pet);
    } else {
        printf("Error Code: %d\n", rc);
        if (err) {
            printf("API Error Details: %s\n", err->detail);
            ApiError_cleanup(err);
        }
    }

    /* 5. Cleanup Client */
    api_cleanup(&client);
    return rc == 0 ? 0 : 1;
}
```

## 6. Advanced Configuration

### Windows vs Linux
The generated code uses preprocessor macros to select the backend:
*   **Linux/macOS:** Uses `libcurl`. Ensure `libcurl4-openssl-dev` is installed.
*   **Windows:** Automatically uses `WinHTTP` or `WinInet` if compiled with MSVC. No external DLLs required usually.

### Customizing HTTP behavior
You can modify the `client.config` struct before making calls:
```c
client.config.timeout_ms = 5000;
client.config.verify_peer = 0; /* Disable SSL verification (Debug only!) */
client.config.user_agent = "MyCustomBot/1.0";
```

### Automatic Retries
The SDK includes a retry loop for 5xx errors and network failures:
```c
client.config.retry_count = 3; /* Retry up to 3 times */
```

## Troubleshooting

*   **Header not found:** Ensure `target_include_directories` in your consuming CMake file points to where `petstore_client.h` is located.
*   **Linker errors:** Ensure you link against the generated library target *and* its dependencies (`curl`, `ssl`, `parson`). If using the generated CMakeLists, linking against `petstore_client` handles this transitively.
