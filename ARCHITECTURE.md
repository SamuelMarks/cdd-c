Architecture
============

`cdd-c` is built as a modular library (`c_cdd`) with a CLI wrapper. It avoids using heavy compiler frontends like
`libclang` in favor of a bespoke, lightweight tokenizer and CST (Concrete Syntax Tree) parser targeted specifically at
C89/C23 constructs relevant to data modeling and API logic.

## Core Modules

### 1. Parsing Layer

* **Tokenizer (`tokenizer.c`):** Implements ISO C Translation Phases 1-3. It handles trigraphs, line splicing (`\`), and
  standard C tokens. It is robust enough to handle C23 features like digit separators (`1'000`).
* **CST Parser (`cst_parser.c`):** Groups tokens into semantic nodes (Functions, Structs, Enums). Unlike an AST, it
  retains position data for precise text patching.
* **Doc Parser (`doc_parser.c`):** extracts annotations (`@route`, `@param`) from comment blocks.

### 2. Analysis & Refactoring

* **Analysis (`analysis.c`):** A heuristic engine that detects allocation patterns (`malloc`, `asprintf`) and verifies
  if the result is checked against `NULL` or error codes before use.
* **Refactor Orchestrator (`refactor_orchestrator.c`):** Builds a dependency graph of functions. If a deep calle changes
  signature (e.g., `void` to `int` for error propagation), the orchestrator propagates this change up the graph to
  callers.
* **Text Patcher (`text_patcher.c`):** Applies non-destructive edits to the source code based on token indices.

### 3. Code Generation

* **Schema Codegen (`schema_codegen.c`):** Generates C structs, JSON serializers (`parson` based), and memory management
  functions (`_cleanup`, `_deepcopy`) from internal `StructFields` representations.
* **Client Gen (`openapi_client_gen.c`):** Generates networking code. It abstracts the HTTP layer, allowing the
  generated code to compile against **libcurl** (Linux/macOS) or **WinHTTP/WinInet** (Windows) without code changes.

## Transport Abstraction Layer

To ensure portability, generated clients interact with an **Abstract Network Interface (ANI)** defined in
`http_types.h`.

```mermaid
%%{init: {'theme': 'base', 'fontFamily': 'Google Sans Normal'}}%%
classDiagram
  class HttpClient {
    +char* base_url
    +HttpConfig config
    +send(ctx, req, res)
  }

  class HttpRequest {
    +char* url
    +HttpMethod method
    +HttpHeaders headers
    +void* body
  }

  class HttpResponse {
    +int status_code
    +void* body
    +size_t body_len
  }

  class TransportFactory {
    <<Interface>>
  }

  class LibCurlBackend {
    +http_curl_send()
    +http_curl_context_init()
  }

  class WinHttpBackend {
    +http_winhttp_send()
    +http_winhttp_context_init()
  }

  HttpClient ..> HttpRequest: Creates
  HttpClient ..> HttpResponse: Receives
  HttpClient --> TransportFactory: Uses logic
  TransportFactory <|-- LibCurlBackend: #ifdef UNIX
  TransportFactory <|-- WinHttpBackend: #ifdef WIN32

%% Styling Fixes:
%% 1. Properties MUST be separated by commas.
%% 2. No spaces allowed after commas or inside values.
%% 3. Removed specific fonts from classDef to avoid quote parsing bugs.
%% 4. Replaced 'stroke:none' with 'stroke-width:0px' (safer for all parsers).
classDef styleAbstract fill:#4285f4,color:#ffffff,stroke-width:0px
classDef styleImpl fill:#f9ab00,color:#20344b,stroke-width:0px
classDef styleStruct fill:#ffffff,color:#20344b,stroke:#4285f4

class TransportFactory styleAbstract
class LibCurlBackend, WinHttpBackend styleImpl
class HttpClient, HttpRequest, HttpResponse styleStruct
```

## Security & Mapping

The architecture includes a dedicated **C Type Mapper** (`c_mapping.c`) which bridges the gap between C types and
OpenAPI types:

| C Type           | OpenAPI Type | Notes                          |
|:-----------------|:-------------|:-------------------------------|
| `int`, `short`   | `integer`    | format: `int32`                |
| `long`, `size_t` | `integer`    | format: `int64`                |
| `char *`         | `string`     |                                |
| `bool`           | `boolean`    | Requires `stdbool.h` or C23    |
| `struct X`       | `object`     | `$ref: #/components/schemas/X` |
| `int []`         | `array`      | items: `integer`               |

This mapping is bidirectional, allowing `c2openapi` to read C types and `openapi2client` to generate C types.
