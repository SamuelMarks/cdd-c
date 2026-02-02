cdd-c
=====

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Linux, Windows, macOS](https://github.com/SamuelMarks/cdd-c/actions/workflows/linux-Windows-macOS.yml/badge.svg)](https://github.com/SamuelMarks/cdd-c/actions/workflows/linux-Windows-macOS.yml)
[`#rewriteInC`](https://rewriteInC.io)
![coverage](reports/test_coverage.svg)
[![C89](https://img.shields.io/badge/C-89-blue)](https://en.wikipedia.org/wiki/C89_(C_version))

**cdd-c** is a comprehensive C frontend toolchain designed for interoperability, code generation, and static safety analysis. It acts as a bridge between C code and modern API specifications, focusing on:
1.  **Reverse Engineering**: Generating OpenAPI v3 specifications from annotated C source code.
2.  **SDK Generation**: Generating type-safe C client libraries from OpenAPI specs strings.
3.  **Refactoring**: Auditing and patching C code for memory safety (unchecked allocations).

## Workflow

```mermaid
%%{init: {'theme': 'base', 'fontFamily': 'Google Sans Normal'}}%%
graph TD
    classDef head font-family: 'Google Sans Medium', fill:#4285f4, color:#ffffff, stroke:none;
    classDef subhead font-family: 'Roboto Mono Normal', fill:#57caff, color:#20344b, stroke:none;
    classDef body font-family: 'Google Sans Normal', fill:#ffffff, color:#20344b, stroke:#4285f4;
    classDef output fill:#34a853, color:#ffffff, stroke:none;

    InputCode[/"C Source / Header"/]:::body
    InputSpec[/"OpenAPI Specification"/]:::body
    
    Core[("cdd-c Toolchain")]:::head
    
    Parser["CST & Doc Parser"]:::subhead
    GenClient["Client Generator"]:::subhead
    GenSpec["Spec Generator"]:::subhead
    
    OutJSON[/"openapi.json"/]:::output
    OutSDK[/"C Client SDK"/]:::output

    InputCode --> Core
    InputSpec --> Core
    
    Core --> Parser
    Parser --> GenSpec
    GenSpec --> OutJSON
    
    Core --> GenClient
    GenClient --> OutSDK

    linkStyle default stroke:#20344b,stroke-width:2px;
```

## Example: C to OpenAPI to Client SDK

**cdd-c** allows you to maintain your API definition strictly within your C code using Doxygen-style annotations, generate a standard OpenAPI JSON file, and then generate a consumer SDK.

### 1. The Input (Server Implementation)

You annotate your C functions with `@route`, `@param`, and `@return`.

```c
/* server.c */

struct Pet {
  int id;
  char *name;
};

/**
 * @route GET /pets/{petId}
 * @summary Find pet by ID
 * @param petId [in:path] ID of pet to return
 * @return 200 Success
 */
int api_get_pet(int petId, struct Pet **out) {
  /* Implementation logic... */
  return 0;
}
```

### 2. The Generated Specification (OpenAPI v3)

Running `c_cdd_cli c2openapi . openapi.json` produces:

```json
{
  "openapi": "3.0.0",
  "paths": {
    "/pets/{petId}": {
      "get": {
        "operationId": "api_get_pet",
        "summary": "Find pet by ID",
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
            "description": "Success",
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
    }
  }
}
```

### 3. The Generated Client SDK

Running the client generator produces a C89 compatible library handling serialization, networking (Curl/WinHTTP), and error handling.

```c
/* main_client.c */
#include "generated_client.h"

int main(void) {
  struct HttpClient client;
  struct Pet *my_pet = NULL;
  struct ApiError *err = NULL;
  int rc;

  /* Initialize (Selects Libcurl or WinHTTP automatically) */
  api_init(&client, "https://api.petstore.com");

  /* Call the typed API */
  rc = api_get_pet(&client, 12, &my_pet, &err);

  if (rc != 0) {
    if (err) {
      printf("API Error: %s\n", err->detail);
      ApiError_cleanup(err);
    } else {
      printf("Transport Error: %d\n", rc);
    }
  } else {
    printf("Got Pet: %s (ID: %d)\n", my_pet->name, my_pet->id);
    Pet_cleanup(my_pet); /* Auto-generated cleanup */
  }

  api_cleanup(&client);
  return 0;
}
```

## Quick Start

### Dependencies
*   CMake 3.11+
*   C Compiler (GCC, Clang, MSVC)
*   **Libraries:** `parson`, `c89stringutils`, `c-str-span` (fetched automatically or via vcpkg).

### Building and testing

```sh
$ git clone "https://github.com/offscale/vcpkg" -b "project0"
# Windows:
$ vcpkg\bootstrap-vcpkg.bat
# Non-Windows:
$ ./vcpkg/bootstrap-vcpkg.sh
# Both Windows and non-Windows:
$ git clone "https://github.com/SamuelMarks/cdd-c" && cd "cdd-c"  # Or your fork of this repo
# Windows
$ cmake -DCMAKE_BUILD_TYPE="Debug" -DBUILD_TESTING=ON -DC_CDD_BUILD_TESTING=ON -DCMAKE_TOOLCHAIN_FILE="..\vcpkg\scripts\buildsystems\vcpkg.cmake" -S . -B "build"
# Non-Windows
$ cmake -DCMAKE_BUILD_TYPE='Debug' -DBUILD_TESTING=ON -DC_CDD_BUILD_TESTING=ON -DCMAKE_TOOLCHAIN_FILE='../vcpkg/scripts/buildsystems/vcpkg.cmake' -S . -B 'build'
# Both Windows and non-Windows:
$ cmake --build "build"
# Test
$ cd "build" && ctest -C "Debug" --verbose
```

More docs: [USAGE](./USAGE.md); [ARCHITECTURE](./ARCHITECTURE.md).
