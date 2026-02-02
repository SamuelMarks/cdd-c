Usage
=====

The `c_cdd_cli` binary serves as the entry point for multiple modes of operation including auditing, schema extraction,
and code generation.

## Command Reference

### 1. Audit Project

Scans a directory for memory safety issues, specifically focusing on unchecked allocations (e.g., `malloc` results used
without `NULL` checks) and functions that return raw pointers.

```bash
c_cdd_cli audit <source_directory>
```

**Output:** Prints a summary of scanned files and a JSON object containing specific violations (file, line number,
variable name).

### 2. C to OpenAPI (c2openapi)

Generates an OpenAPI v3.0 JSON specification by analyzing C source code, function signatures, and Doxygen-style
comments.

```bash
c_cdd_cli c2openapi <source_directory> <output_spec.json>
```

**Annotations:**
To expose a C function to the OpenAPI generator, add documentation comments:

```c
/**
 * @route GET /users/{id}
 * @summary Get a user by ID
 * @param id [in:path] The user ID
 */
int api_get_user(int id, struct User **out);
```

### 3. C Header to JSON Schema (code2schema)

Extracts `struct` and `enum` definitions from a C header file and converts them into JSON Schema definitions.

```bash
c_cdd_cli code2schema <input_header.h> <output_schema.json>
```

---

## Data Flow Diagram

The following diagram illustrates how the `c2openapi` command processes source code into a specification.

```mermaid
%%{init: {'theme': 'base', 'fontFamily': 'Google Sans Normal'}}%%
flowchart LR
    classDef head font-family: 'Google Sans Medium', fill: #4285f4, color: #ffffff, stroke: none;
    classDef process font-family: 'Roboto Mono Normal', fill: #57caff, color: #20344b, stroke: none;
    classDef file fill: #ffd427, color: #20344b, stroke: #f9ab00, stroke-dasharray: 5 5;
    classDef final fill: #34a853, color: #ffffff, stroke: none;
    Src["Source.c / .h"]:::file
    Inspector["C Inspector"]:::head

    subgraph Parsing Step
        Tokenize["Tokenizer"]:::process
        MatchTypes["Type Matcher"]:::process
        DocParse["Doc Parser"]:::process
    end

    subgraph Mapping Step
        OpBuild["Op Builder"]:::process
        SchemaReg["Schema Registry"]:::process
        TypeMap["C -> OA Type Mapper"]:::process
    end

    Output["openapi.json"]:::final
    Src --> Inspector
    Inspector --> Tokenize
    Tokenize --> MatchTypes & DocParse
    MatchTypes --> SchemaReg
    DocParse --> OpBuild
    TypeMap --> OpBuild
    SchemaReg --> Output
    OpBuild --> Output
    linkStyle default stroke: #20344b, stroke-width: 2px;
```

## Generated Code Usage

When generating a client SDK (via `openapi2client` logic), the output includes:

1. **Transport abstraction**: `struct HttpClient` initialized with `_init()`.
2. **Helpers**: `_to_json`, `_from_json`, `_cleanup` for all models.
3. **Operations**: `api_verb_resource(ctx, params...)`.

### Standard Lifecycle

```c
struct HttpClient client;
struct User *u = NULL;
struct ApiError *err = NULL;

/* 0. Initialize (Selects Curl or WinHTTP automatically) */
api_init(&client, "https://api.example.com");

/* 1. Call Operation */
if (api_get_user(&client, 1, &u, &err) != 0) {
    printf("Error: %s\n", err->title);
    ApiError_cleanup(err);
} else {
    printf("User: %s\n", u->name);
    User_cleanup(u);
}

/* 2. Cleanup */
api_cleanup(&client);
```
