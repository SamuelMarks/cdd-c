# OpenAPI 3.2.0 Conformance Table: Client SDK CLI (CLI Tooling & Tests)

This table tracks the completeness of language integration with OpenAPI 3.2.0 for generating Command-Line Interfaces (CLIs) wrapper tools, and vice-versa.

### Legend & Tracking Guide
*   **To**: Language -> OpenAPI (Generating the OpenAPI document from declarative CLI structures)
*   **From**: OpenAPI -> Language (Generating CLI routing, flag parsing, and formatting from OpenAPI)
*   **Presence `[To, From]`**: The object is successfully parsed, validated, utilized, or generated.
*   **Absence `[To, From]`**: The object is currently unsupported, dropped, or falls back to generic/`any` types.
*   **Skipped `[To, From]`**: Intentionally ignored because it is irrelevant or unsupported by the CLI environment.
*   **Checkboxes**: Mark `[x]` as conformance is achieved.

| OpenAPI 3.2.0 Object / Feature | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes / Implementation Strategy |
| :--- | :---: | :---: | :---: | :--- |
| **OpenAPI Object (Root)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Root CLI definition |
| **OpenAPI Object (`openapi`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`$self`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Base URI resolution for internal and external references |
| **OpenAPI Object (`info`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`jsonSchemaDialect`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Skipped or used for advanced flag validation |
| **OpenAPI Object (`servers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`paths`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`webhooks`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Skipped (CLIs typically don't expose webhook listeners) |
| **OpenAPI Object (`components`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`security`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`tags`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`externalDocs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | CLI `--help` text, `--version` command |
| **Info Object (`title`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Short summary for CLI global help text |
| **Info Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`termsOfService`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`contact`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`license`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`version`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Contact Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Appended to global help or skipped |
| **Contact Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Contact Object (`url`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Contact Object (`email`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **License Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Appended to global help or skipped |
| **License Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **License Object (`identifier`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | SPDX license identifier extraction |
| **License Object (`url`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Global `--server` or `--host` flag mapping |
| **Server Object (`url`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Unique name used as CLI alias for a host environment |
| **Server Object (`variables`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Variable Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Individual host template CLI flags |
| **Server Variable Object (`enum`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Variable Object (`default`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Variable Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Reusable flag groups or interactive prompt states |
| **Components Object (`schemas`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`responses`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`examples`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`requestBodies`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`headers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`securitySchemes`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`links`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`callbacks`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`pathItems`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Reusable subcommand groupings |
| **Components Object (`mediaTypes`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Reusable payload flag definitions |
| **Paths Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Structural mapping to CLI namespaces |
| **Paths Object (`/{path}`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Subcommand grouping |
| **Path Item Object (`$ref`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`get`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`put`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`post`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`delete`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`options`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`head`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`patch`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`trace`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`query`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | QUERY HTTP method subcommand |
| **Path Item Object (`additionalOperations`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Map of custom HTTP methods to subcommands |
| **Path Item Object (`servers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | The execution targets of subcommands (e.g., `cli users get`) |
| **Operation Object (`tags`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object (`externalDocs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object (`operationId`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object (`requestBody`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object (`responses`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object (`callbacks`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object (`deprecated`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object (`security`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object (`servers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **External Documentation Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Added to `See also:` in subcommand help |
| **External Documentation Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **External Documentation Object (`url`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object (`in`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object (`required`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object (`deprecated`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object (`allowEmptyValue`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object (`example`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object (`examples`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object (`style`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object (`explode`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object (`allowReserved`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object (`schema`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Parameter Object (`content`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Request Body Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Handled via file ingest (`-F @data.json`), STDIN pipe, or nested flags |
| **Request Body Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Request Body Object (`content`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Request Body Object (`required`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Inferred based on payload flag logic |
| **Media Type Object (`schema`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`itemSchema`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Validation for individual items in a CLI array flag |
| **Media Type Object (`example`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`examples`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`encoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`prefixEncoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`itemEncoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Internal CLI form-data builder logic |
| **Encoding Object (`contentType`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`headers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`encoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`prefixEncoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`itemEncoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`style`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`explode`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`allowReserved`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Responses Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Determines Exit Codes (`0` vs `1`, etc.) |
| **Responses Object (`default`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Responses Object (`HTTP Status Code`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Response Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Stdout formatting (Table format, JSON, YAML, `--raw`) |
| **Response Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Subcommand exit status short description |
| **Response Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Response Object (`headers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Response Object (`content`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Response Object (`links`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Callback Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Skipped (CLI is generally stateless) |
| **Callback Object (`{expression}`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Injected into subcommand `--help` 'Examples' block |
| **Example Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`dataValue`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`serializedValue`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`externalValue`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`value`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Skipped |
| **Link Object (`operationRef`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`operationId`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`requestBody`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`server`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Can optionally be printed with `-v` (verbose) flags |
| **Header Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`required`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`deprecated`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`example`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`examples`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`style`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`explode`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`schema`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`content`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | High-level CLI command groups (e.g. `cli [tag] [operation]`) |
| **Tag Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object (`externalDocs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object (`parent`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Nested subcommand groups |
| **Tag Object (`kind`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Subcommand grouping logic (e.g. `nav` vs `hidden`) |
| **Reference Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Reference Object (`$ref`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Internal resolution to flatten flags/commands |
| **Reference Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Reference Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object (`discriminator`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object (`xml`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object (`externalDocs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object (`example`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Discriminator Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Mutually exclusive flag groups based on type |
| **Discriminator Object (`propertyName`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Discriminator Object (`mapping`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Discriminator Object (`defaultMapping`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Fallback CLI flag group when type is omitted |
| **XML Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Skipped |
| **XML Object (`nodeType`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Skipped (CLI XML is rare) |
| **XML Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **XML Object (`namespace`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **XML Object (`prefix`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **XML Object (`attribute`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **XML Object (`wrapped`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Security Scheme Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Security Scheme Object (`type`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Security Scheme Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Security Scheme Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Security Scheme Object (`in`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Security Scheme Object (`scheme`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Security Scheme Object (`bearerFormat`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Security Scheme Object (`flows`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Security Scheme Object (`openIdConnectUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Security Scheme Object (`oauth2MetadataUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | OAuth2 metadata discovery |
| **Security Scheme Object (`deprecated`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flows Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | CLI token manager / local keychain integration |
| **OAuth Flows Object (`implicit`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flows Object (`password`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flows Object (`clientCredentials`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flows Object (`authorizationCode`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flows Object (`deviceAuthorization`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Support for the Device Authorization grant flow |
| **OAuth Flow Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Flow routing logic for CLI login |
| **OAuth Flow Object (`authorizationUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flow Object (`deviceAuthorizationUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Triggers CLI device auth prompt |
| **OAuth Flow Object (`tokenUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flow Object (`refreshUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flow Object (`scopes`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Security Requirement Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Asserting required auth exists before command execution |
| **Security Requirement Object (`{name}`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
