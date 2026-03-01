# cdd-c Usage

## Generating a Native C Client from OpenAPI

```sh
# Generate the C headers, source logic, and HTTP Curl wrappers for the provided OpenAPI specification
cdd-c from_openapi -i spec.json
```

## Extracting OpenAPI from Existing C Code

```sh
# Statically parse a local C project structure into an OpenAPI specification. 
# Automatically extracts `struct` definitions, `typedefs`, and routing hooks.
cdd-c to_openapi -f path/to/code -o out.json
```

## Generating Docs

```sh
# Emit language-specific snippets optimized for documentation site inclusion
cdd-c to_docs_json --no-imports --no-wrapping -i spec.json
```