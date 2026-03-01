# Usage

To use `cdd-c`, you first need an input source: either an OpenAPI spec or an existing C codebase.

## Parsing C Code (C to OpenAPI)

When converting your native C structures, headers, and comments into an OpenAPI JSON spec:

```bash
# Output OpenAPI
./bin/cdd-c to_openapi -f src/routes.c -o build/openapi.json
```

## Emitting C Code (OpenAPI to C)

When converting an OpenAPI spec into native C code (like an API Client/SDK or Server Stub):

```bash
# Generate SDK client code
./bin/cdd-c from_openapi to_sdk -i swagger.yaml -o generated_sdk/

# Generate an interactive CLI to wrap the SDK
./bin/cdd-c from_openapi to_sdk_cli -i swagger.yaml -o generated_cli/

# Generate a server route skeleton
./bin/cdd-c from_openapi to_server -i swagger.yaml -o generated_server/
```

### JSON RPC Server

The tool also operates as a long-running JSON RPC server to offer compiler services remotely:

```bash
./bin/cdd-c serve_json_rpc --port 8082 --listen 0.0.0.0
```

### Documentation Snippets

Generate documentation configuration `docs.json`:

```bash
./bin/cdd-c to_docs_json --no-imports --no-wrapping -i swagger.yaml -o docs.json
```
