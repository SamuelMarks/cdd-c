# CDD-C Usage & c-orm Integration

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

## General CLI Operations

Beyond database and ORM management, `cdd-c` operates as a robust multi-tool for code and specification generation.

### Generating Code from OpenAPI
Generate a fully compliant `c-rest-framework` HTTP Server, a Client SDK, or an interactive CLI from a `spec.json` OpenAPI definition:
```bash
# Generate C SDK models and API wrappers
cdd-c from_openapi to_sdk -i spec.json -o src/models

# Generate an HTTP Server (utilizing c-rest-framework)
cdd-c from_openapi to_server -i spec.json -o src/server

# Generate a Client SDK with an interactive CLI harness
cdd-c from_openapi to_sdk_cli -i spec.json -o src/cli
```

### Emitting OpenAPI from C Code
Run a static analysis extraction across your C source headers and implementation files to reverse-engineer and output an OpenAPI specification:
```bash
# Scrape the directory to produce openapi.json
cdd-c to_openapi -i src/models -o openapi.json
```

### JSON Schema & Testing
Convert pure JSON Schemas to C code, generate unit tests based on schema boundaries, or convert existing structs to Schemas:
```bash
# JSON Schema to C Structs
cdd-c schema2code schema.json out_dir/

# C Header Structs to JSON Schema
cdd-c code2schema header.h schema.json

# Generate unit tests verifying header struct layouts against JSON Schema boundaries
cdd-c jsonschema2tests schema.json header_to_test.h out_test.h
```

### SQL DDL Parsing & C-ORM Generation
Instead of introspecting a live database, you can parse static `.sql` files with `CREATE TABLE` instructions to generate strictly typed `c-orm` C models directly:
```bash
cdd-c sql2c schema.sql src/models
```

### Memory Safety Auditing
Run the built-in memory auditor across a C codebase to locate un-freed allocations or potentially unsafe patterns before compilation:
```bash
cdd-c audit src/
```

### Build System Scaffold
Automatically scaffold `CMakeLists.txt` or other target build files for your auto-generated codebase:
```bash
cdd-c generate_build_system cmake out_dir my_project
```

### Generating API Documentation JSON Examples
If you need raw JSON snippets to embed in external tools or static documentation generators:
```bash
cdd-c to_docs_json --no-imports --no-wrapping -i spec.json
```

### JSON-RPC Server
Launch a local language-server-like interface for your editor or CI pipeline:
```bash
cdd-c serve_json_rpc --port 8082 --listen 0.0.0.0
```
