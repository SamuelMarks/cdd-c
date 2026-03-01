# Usage Reference

> **Purpose of this file (`USAGE.md`)**: To document the command-line interface, configuration flags, and primary use cases for the `cdd-c` tool.

The `cdd-c` CLI acts as the main interface for bidirectional transformations.

## Core Commands

### `cdd-c to_openapi`
Scans a C source directory, parses structures, enums, and annotated routes, and emits a compliant OpenAPI 3.2.0 specification.

```bash
cdd-c to_openapi -f src/routes/ -o spec/openapi.json
```

### `cdd-c from_openapi`
Reads an OpenAPI specification (JSON or YAML) and generates compliant, idiomatic C code (client SDKs, server mock routes, and data models).

```bash
cdd-c from_openapi -i spec/openapi.json
```
*(Files generated will include `generated_client.c` and `generated_client.h`)*

### `cdd-c audit`
Scans a specified project directory for memory safety violations, schema inconsistencies, and C89 compliance checks.

```bash
cdd-c audit src/
```

### `cdd-c to_docs_json`
Generates pure JSON code examples and schemas optimized for static documentation site ingestion.

```bash
cdd-c to_docs_json --no-imports -i spec/openapi.json
```

### `cdd-c c2openapi`
An explicit lower-level variant of `to_openapi`.

```bash
cdd-c c2openapi <source_directory> <output_openapi.json>
```

### `cdd-c jsonschema2tests`
Generates comprehensive C test suites from a given JSON schema payload, ensuring your C implementation accurately reflects the data model constraints.

```bash
cdd-c jsonschema2tests schema.json header_to_test.h out_test.h
```