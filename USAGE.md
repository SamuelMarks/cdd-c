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

## Serde-Style C89 Reflection & Declarative CLI (`structopt` / `clap`) Codegen

Annotate any C89 `struct`, `enum`, or tagged `union` in a header file with `/* @cdd ... */` or `/* @cdd cli(...) */` directives and run `cdd-c sync` (or `cdd-c code2schema` / `cdd-c schema2code`) to generate JSON, form, schema, and command-line parsers:

```c
/**
 * @brief Deployment CLI & JSON configuration
 * @cdd rename_all = "camelCase", deny_unknown_fields
 * @cdd cli(name = "deploy-ctl", version = "1.0.0", env_prefix = "DEPLOY_")
 */
struct DeployConfig {
  /* @cdd rename = "target-host", alias = "host" */
  /* @cdd cli(short = 'H', long = "host", env, required, help = "Target hostname") */
  char *target_host;

  /* @cdd skip_serializing_if = "zero" */
  /* @cdd cli(short = 'v', long = "verbose", action = "count", help = "Verbosity level") */
  int verbose;

  /* @cdd cli(long = "color", negatable, default = "1", help = "Enable colored output") */
  int color;

  /* @cdd cli(short = 't', long = "tag", value_delimiter = ',', help = "Deployment tags") */
  size_t n_tags;
  char **tags;

  /* @cdd flatten */
  struct AuthOpts *auth;
};
```

Running `cdd-c sync deploy.h deploy.c` generates direct, zero-overhead C89 functions and additive RTTI descriptors:
- `DeployConfig_to_json` / `DeployConfig_from_json` (zero-dependency RFC 8259 cursor/buffer codecs)
- `DeployConfig_from_cli` / `DeployConfig_parse_cli_into` / `DeployConfig_to_cli_argv` (recursively handling `/* @cdd flatten */` structs and `/* @cdd cli(subcommand) */` tagged unions)
- `DeployConfig_print_help` / `DeployConfig_print_version` / `DeployConfig_print_completions`
- `DeployConfig_to_form_urlencoded` / `DeployConfig_from_form_urlencoded` (with `+` and `%XX` percent-decoding and nested object composition)
- `DeployConfig_default` / `DeployConfig_deepcopy` / `DeployConfig_eq` / `DeployConfig_cleanup` / `DeployConfig_debug` (for both structs and tagged unions, recursively delegating to child `<Type>_cleanup`, `<Type>_deepcopy`, `<Type>_eq`, and `<Type>_debug`)
- `DeployConfig_reflect` (`struct cdd_type_info`, `struct cdd_field_info`, `cdd_reflect_visit_fields`, `cdd_reflect_visit_fields_mut`, `cdd_reflect_get_*` / `cdd_reflect_set_*`, `cdd_reflect_array_len` / `cdd_reflect_array_at`)
- C11/C23 `_Generic` dispatch macros (`CDD_TO_JSON`, `CDD_FROM_JSON`, `CDD_FROM_CLI`, `CDD_TO_CLI_ARGV`, `CDD_TO_FORM`, `CDD_FROM_FORM`, `CDD_EQ`, `CDD_DEEPCOPY`, `CDD_DEFAULT`, `CDD_CLEANUP`, `CDD_DEBUG`, `CDD_REFLECT`)

### End-to-End Lifecycle, CLI, Form, & Zero-Dependency JSON Example

```c
#include "deploy.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  struct DeployConfig *cfg = NULL;
  struct DeployConfig *copy = NULL;
  char *json = NULL;
  char *form = NULL;
  int is_eq = 0;
  cdd_c_error_t rc;

  /* 1. Parse CLI argv + environment variables + defaults */
  rc = DeployConfig_from_cli(argc, argv, &cfg);
  if (rc != CDD_C_SUCCESS) return EXIT_FAILURE;

  /* 2. Deep-copy and verify structural equality (recursively checks AuthOpts) */
  rc = DeployConfig_deepcopy(cfg, &copy);
  if (rc != CDD_C_SUCCESS) {
    rc = DeployConfig_cleanup(cfg);
    return rc != CDD_C_SUCCESS ? (int)rc : EXIT_FAILURE;
  }
  rc = DeployConfig_eq(cfg, copy, &is_eq);
  if (rc != CDD_C_SUCCESS || !is_eq) {
    rc = DeployConfig_cleanup(copy);
    if (rc != CDD_C_SUCCESS) return (int)rc;
    rc = DeployConfig_cleanup(cfg);
    return rc != CDD_C_SUCCESS ? (int)rc : EXIT_FAILURE;
  }

  /* 3. Emit zero-dependency RFC 8259 JSON and URL-encoded form payload */
  rc = DeployConfig_to_json(cfg, &json);
  if (rc != CDD_C_SUCCESS) {
    rc = DeployConfig_cleanup(copy);
    if (rc != CDD_C_SUCCESS) return (int)rc;
    rc = DeployConfig_cleanup(cfg);
    return rc != CDD_C_SUCCESS ? (int)rc : EXIT_FAILURE;
  }
  rc = DeployConfig_to_form_urlencoded(cfg, &form);
  if (rc != CDD_C_SUCCESS) {
    free(json);
    rc = DeployConfig_cleanup(copy);
    if (rc != CDD_C_SUCCESS) return (int)rc;
    rc = DeployConfig_cleanup(cfg);
    return rc != CDD_C_SUCCESS ? (int)rc : EXIT_FAILURE;
  }

  printf("JSON: %s\nForm: %s\n", json, form);
  free(form);
  free(json);

  /* 4. Recursive cleanup (automatically calls AuthOpts_cleanup(cfg->auth) and frees cfg) */
  rc = DeployConfig_cleanup(copy);
  if (rc != CDD_C_SUCCESS) return EXIT_FAILURE;
  rc = DeployConfig_cleanup(cfg);
  if (rc != CDD_C_SUCCESS) return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
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
