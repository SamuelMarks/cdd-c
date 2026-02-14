cdd-c
=====

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Linux, Windows, macOS](https://github.com/SamuelMarks/cdd-c/actions/workflows/linux-Windows-macOS.yml/badge.svg)](https://github.com/SamuelMarks/cdd-c/actions/workflows/linux-Windows-macOS.yml)
[`#rewriteInC`](https://rewriteInC.io)
![coverage](reports/test_coverage.svg)
[![C89](https://img.shields.io/badge/C-89-blue)](https://en.wikipedia.org/wiki/C89_(C_version))

**cdd-c** is a comprehensive C frontend toolchain designed for interoperability, code generation, and static safety analysis. It acts as a bridge between C code and modern API specifications, focusing on:
1.  **Reverse Engineering**: Generating OpenAPI v3.2 specifications from annotated C source code.
2.  **SDK Generation**: Generating type-safe C client libraries from OpenAPI specs strings.
3.  **Refactoring**: Auditing and patching C code for memory safety (unchecked allocations).

OpenAPI 3.2 I/O support includes parameter styles (form/simple/matrix/label/spaceDelimited/pipeDelimited/deepObject/cookie), `allowEmptyValue`, content-based parameters (including parameter/header Media Type Objects with encoding/examples/schema, plus JSON content serialization for query/header and URL-encoding for query values), parameter/header schema `$ref` (including array items, plus non-component/external refs), multi-document component `$ref` resolution via `$self`/retrieval base when using the document registry, component schema `$id`-based `$ref` resolution, component schema `$anchor`/`$dynamicAnchor` fragment resolution for `$ref`/`$dynamicRef`, schema-only document round-tripping, inline primitive/array/object request/response schemas (inline objects are promoted to component schemas for strong typing), inline schema `enum` (any JSON type)/`default` and `type` arrays (union types, including `null`), boolean schemas (`schema: true|false`), inline schema `const` (including array items `const`/`default`), deprecated `example`, plus JSON Schema `examples` arrays (including `items.examples`), inline schema numeric/string/array constraints (`minimum`/`maximum`/`exclusiveMinimum`/`exclusiveMaximum`, `minLength`/`maxLength`/`pattern`, `minItems`/`maxItems`/`uniqueItems`) including array item constraints, inline schema annotations (`description`, `deprecated`, `readOnly`, `writeOnly`) and `$ref` summary/description overrides, inline schema `format` and `contentMediaType`/`contentEncoding`, inline schema extra keyword passthrough for non-component schemas (including `x-` extensions, `$schema`/`$id`, `oneOf`/`anyOf`/`allOf`, `contentSchema`, and other custom keys) plus typed `externalDocs`, `discriminator`, and `xml` metadata, spec extension passthrough on non-schema objects (info/contact/license, tags, servers, paths, webhooks, components, operations, responses **objects**, parameters, headers, responses, request bodies (including operation-level requestBody objects), callbacks, links, security schemes, externalDocs, and security requirement objects), multi-content request/response maps, Media Type `encoding`, `prefixEncoding`, and `itemEncoding` for form/multipart, sequential `itemSchema` mapping to array types for SDK generation, response headers, response links, callback objects, requestBody component `$ref` resolution, `components.requestBodies`, `components.mediaTypes`, `components.pathItems`, `components.links`, `components.callbacks`, `components.examples`, Path Item/Callback `$ref` resolution to components (paths/webhooks/callbacks), `additionalOperations`, content `$ref` to Media Type Objects, Media Type/Parameter/Header/Response/Request examples, component schema `allOf`/`anyOf`/`oneOf` preserved verbatim in output (raw override) while codegen merges `allOf` and emits tagged unions for `oneOf`/`anyOf` (object refs + primitive + inline object/array variants, with inline objects promoted to components) using discriminator/property heuristics, component schema property `type` arrays (including array items) preserved and mapped to the primary non-null type for code generation, schema annotations (`description`, `format`, `deprecated`, `readOnly`, `writeOnly`), array constraints (`minItems`, `maxItems`, `uniqueItems`), JSON Schema keyword passthrough (`additionalProperties`, `patternProperties`, `$defs`, `not`, `if`/`then`/`else`, and other custom keys), raw component schemas for non-object/boolean/array definitions, components enum schemas (string) and required property lists, and security schemes including `mutualTLS`, OAuth2 flows, and `deprecated`. SDK generation supports array query serialization for `form` (default `explode=true` and explicit `explode=false` CSV), `spaceDelimited`, and `pipeDelimited` styles, `allowReserved` query encoding, querystring parameters for `application/x-www-form-urlencoded` (typed `struct OpenAPI_KV`) and `application/json` (component schema to JSON, including inline object schemas promoted to components, plus inline primitive/array JSON serialization and JSON arrays of component objects), object query serialization for `form` (explode true/false), `deepObject`, `spaceDelimited`, and `pipeDelimited` using `struct OpenAPI_KV`, object path serialization for `simple`/`label`/`matrix` (explode true/false) using `struct OpenAPI_KV`, object header serialization for `simple` (explode true/false), numeric parameters (`number`) for path/query/header/cookie (including arrays), cookie parameters (including `explode=false` arrays, object cookies via `struct OpenAPI_KV`, and form-style percent-encoding via `allowReserved`), simple header arrays, form-urlencoded and multipart/form-data request bodies (primitive fields + arrays, plus object fields serialized as JSON by default and honoring `encoding.style`/`explode` for form-urlencoded objects, including `encoding.contentType` and `encoding.headers` with contentType lists picking the first entry for codegen), `text/plain` request bodies, raw `text/*` and `application/*+xml` request/response bodies (string pass-through), raw non-JSON binary request/response bodies (Content-Type preserved), raw `application/octet-stream` request/response bodies, inline primitive/array/object JSON request and response bodies (inline objects promoted to components), matrix/label path serialization, apiKey security injection (header/query/cookie), HTTP bearer/basic auth injection (including OAuth2/OpenID bearer tokens), additionalOperations for custom HTTP methods mapped to supported verbs (including `CONNECT`), and response selection for `default` and `xXX` status ranges, plus raw `text/plain` string responses; SDK initialization defaults to the first `servers[]` entry when `base_url` is `NULL` or empty, or to `/` when no servers are defined, and operation/path-level `servers` override the base URL when present.

OpenAPI compliance guards include path template validation (all `{param}` segments must have matching `in: path` parameters, and those parameters must be `required: true`), detection of ambiguous templated paths (same shape, different `{param}` names), `paths` keys starting with `/`, component key regex validation (`^[a-zA-Z0-9._-]+$`), required parameter `name`/`in` fields with duplicate (`name` + `in`) detection, parameter `style` validation by location (plus deepObject/spaceDelimited/pipeDelimited type checks), querystring parameters limited to a single occurrence and forbidden alongside `in: query` parameters (including callbacks), Header Objects restricted to `style: simple`, Media Type / Encoding Objects forbidding `encoding` alongside `prefixEncoding` or `itemEncoding`, unique tag names and `operationId` values (including callbacks), Tag Object `name` required with parent validation (parent must exist, no cycles), ExternalDocs `url` required, Request Body `content` required, Response `description` required, operations requiring a non-empty `responses` object with valid status code keys (`default`, a 3-digit code, or `1XX`-`5XX`), additionalOperations forbidding standard HTTP methods, Link Objects requiring exactly one of `operationId` or `operationRef`, root-level presence of at least one of `paths`, `components`, or `webhooks`, root `openapi` (or Swagger `swagger`) required with `info.title` and `info.version`, `openapi` versions must be 3.x when present, Example Objects require mutually exclusive `value` vs `dataValue` and `serializedValue` vs `externalValue`, Parameter/Header/Media Type Objects must not include both `example` and `examples`, Server Variable `default` required (and enforced to be in `enum` when present), server URL template variables must be defined and unique, server `name` values must be unique within a `servers` array when provided, `allowEmptyValue` restricted to `in: query` parameters, Server `url` forbidding `?` or `#` and required for explicit entries, License `name` required with `identifier`/`url` mutually exclusive, and media type selection that prefers the most specific content key (e.g. `text/plain` over `text/*`) while recognizing JSON types (including `+json`) even when media type parameters are present.

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

You annotate your C functions with `@route`/`@webhook`, `@param`, and `@return`.

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

You can also provide richer metadata:

- `@description <text>`
- `@operationId <id>`
- `@tag <name>` / `@tags <name1, name2>`
- `@tagMeta <name> [summary:<text>] [description:<text>] [parent:<name>] [kind:<value>] [externalDocs:<url>] [externalDocsDescription:<text>]`
- `@webhook <VERB> <PATH>`
- `@deprecated [true|false]`
- `@externalDocs <url> [description]`
- `@security <scheme> [scope1, scope2]`
- `@securityScheme <name> [type:<apiKey|http|mutualTLS|oauth2|openIdConnect>]`
- `@server <url> [name=<name>] [description=<text>]`
- `@requestBody [required|required:true|required:false] [contentType:<media/type>] <description>`
- `@return <status> [contentType:<media/type>] <description>`
- `@responseHeader <status> <name> [type:<string|integer|number|boolean>] [format:<fmt>] [contentType:<media/type>] [required|required:true|required:false] [example:<json>] <description>`
- `@link <status> <name> [operationId:<id> | operationRef:<uri>] [parameters:<json>] [requestBody:<json>] [summary:<text>] [description:<text>] <description>`
- `@param` flags: `[in:<path|query|header|cookie|querystring>] [required]`
  `[style:<form|simple|matrix|label|spaceDelimited|pipeDelimited|deepObject|cookie>]`
  `[explode:true|false] [allowReserved:true|false] [allowEmptyValue:true|false] [contentType:<media/type>] [format:<fmt>]`

Notes:
- Custom HTTP methods in `@route` (e.g., `COPY`) are emitted under
  Path Item `additionalOperations`.
- Operation tags are emitted as top-level Tag Objects in the generated spec.
- `@tagMeta` enriches top-level Tag Objects with summary/description/parent/kind/externalDocs.
- Repeating `@return` for the same status with different `contentType` values
  emits multi-content responses.
- Querystring parameters with `contentType:application/json` now use the C
  type mapping to populate the parameter schema (including `$ref` for structs).
- Querystring parameters with non-JSON content types (for example
  `text/plain` or `application/jsonpath`) are serialized as raw values and
  URL-encoded in generated clients, preserving primitive C types where
  possible.
- Pass `--self <uri>` to emit a `$self` URI in generated OpenAPI.
- Standalone doc blocks (no `@route`) can define root metadata: `@server` for
  root `servers`, `@externalDocs` for root `externalDocs`, `@security` for root
  `security`, and `@securityScheme` for `components.securitySchemes`.
- Security requirements may reference `components.securitySchemes` via URI
  (including `$self`-qualified URIs).
- `$self` values that are relative paths are honored when matching
  component `$ref` URIs.

### 2. The Generated Specification (OpenAPI v3.2)

Running `c_cdd_cli c2openapi . openapi.json` produces:

```json
{
  "openapi": "3.2.0",
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
