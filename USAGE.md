# CDD-C Usage & c-orm Integration

`cdd-c` is both a command-line compiler and a programmatic C89 library (`libc_cdd`) for bidirectional compilation between C (C89), [OpenAPI 3.2.0](https://spec.openapis.org/oas/v3.2.0.html), [Swagger 2.0](https://spec.openapis.org/oas/v2.0.html), and [Arazzo 1.1.0](https://spec.openapis.org/arazzo/v1.1.0.html), as well as 42-language FFI binding generation, CST refactoring, and `c-orm` database schema integration.

## Core CLI Workflows

```bash
# 1. Compile OpenAPI 3.2.0 / Swagger 2.0 to C Client SDK, CLI, or Server
cdd-c from_openapi to_sdk -i openapi.json -o ./sdk --tests
cdd-c from_openapi to_sdk_cli -i openapi.json -o ./cli --tests
cdd-c from_openapi to_server -i openapi.json -o ./server --tests

# 2. Extract OpenAPI 3.2.0 from C source/header files
cdd-c to_openapi -i ./src -o openapi.json

# 3. Compile and extract Arazzo 1.1.0 multi-step workflows
cdd-c from_arazzo to_sdk -i workflow.arazzo.json -o ./workflows
cdd-c from_arazzo to_sdk_cli -i workflow.arazzo.json -o ./workflow-cli
cdd-c from_arazzo to_server -i workflow.arazzo.json -o ./workflow-server
cdd-c to_arazzo -i ./workflows -o extracted.arazzo.json

# 4. Generate documentation code snippets JSON
cdd-c to_docs_json --no-imports --no-wrapping -i openapi.json -o docs.json

# 5. Generate 42-language FFI bindings from C headers
cdd-c bind -i include/my_lib.h -o ./bindings -l python,rust,go,typescript -n my_lib -m MyLib

# 6. Run lossless CST transformers and GNU-to-C89 lowering
cdd-c standardize-gnu --target-c89 --fix src/*.c
cdd-c transformer safe_crt --audit src/*.c

# 7. Expose cdd-c over MCP (stdio) or JSON-RPC 2.0 (HTTP/TCP)
cdd-c mcp
cdd-c serve_json_rpc --port 8080 --listen 127.0.0.1
```

---

## Database Migration Management & `c-orm` Integration

`cdd-c` provides a complete database migration management system out of the box to securely prepare target development schemas prior to `c-orm` code generation.

The code generator strictly introspects database schema artifacts locally to ensure C type safety, which mandates a live running database equipped with identical schema profiles.

## Configuration

To establish the environment variable configuration globally across CLI tools:

```bash
export DATABASE_URL="postgresql://user:password@localhost:5432/my_dev_db"
```

## Creating & Running Migrations

```bash
# Generate a new boilerplate .sql file stamped by timestamp inside ./migrations/
cdd-c migrate create init_users

# Once written, apply all pending forward chronological UP migrations
cdd-c migrate up

# Rollback the last applied migration natively
cdd-c migrate down
```

## CI/CD Pipeline Operations

For headless systems such as GitHub Actions, `cdd-c` integrates seamlessly to scaffold dynamic integration testing environments.

Point `DATABASE_URL` momentarily to the administrative `postgres` DB to establish the target.

```bash
# Administrative context
export DATABASE_URL="postgresql://postgres:pass@localhost:5432/postgres"

# Establish target database (if it doesn't already exist)
cdd-c setup_test_db my_ci_test_db

# Pivot context towards the created database
export DATABASE_URL="postgresql://postgres:pass@localhost:5432/my_ci_test_db"

# Force clear all schemas & execute all UP migrations entirely
cdd-c db reset

# Execute a direct initialization payload of default parameters/entities
cdd-c seed seeds.sql
```

## Exposing the Schema to c-orm

To perform programmatic extraction, run the schema dumper. This guarantees your `c-orm` generators operate off the pristine schema state.

```bash
# Extract the active schema structure utilizing pg_dump wrapper
cdd-c schema dump schema.sql
```

Now, `c-orm` can aggressively compile structural mapping layers across all verified SQLite/PostgreSQL/MySQL models.
