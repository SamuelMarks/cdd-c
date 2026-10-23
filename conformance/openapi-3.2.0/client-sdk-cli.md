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
| **OpenAPI Object (Root)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Root CLI definition |
| **OpenAPI Object (`openapi`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Version metadata preserved in `@openapi` docstring |
| **OpenAPI Object (`$self`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Base URI resolution for internal and external references |
| **OpenAPI Object (`info`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | CLI `--help` header and `--version` output |
| **OpenAPI Object (`jsonSchemaDialect`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Skipped or used for advanced flag validation |
| **OpenAPI Object (`servers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `--base-url` / `--server` CLI flag defaults and `@server` tags |
| **OpenAPI Object (`paths`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Mapped to CLI subcommands and `dispatch_cli` router |
| **OpenAPI Object (`webhooks`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Skipped (CLIs typically don't expose webhook listeners) |
| **OpenAPI Object (`components`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Shared model structs, flag parsers, and `@component_*` tags |
| **OpenAPI Object (`security`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `--token` / `--api-key` CLI flags and `@security` tags |
| **OpenAPI Object (`tags`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Subcommand grouping in `--help` and `@tag_meta` tags |
| **OpenAPI Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Displayed in `--help` footer and `@externalDocs` tag |
| **Info Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | CLI --help text, --version command |
| **Info Object (`title`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | CLI banner title and `@title` tag |
| **Info Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Short summary for CLI global help text |
| **Info Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Extended description in `--help` and `@description` tag |
| **Info Object (`termsOfService`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@termsOfService` Doxygen tag |
| **Info Object (`contact`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@contact` Doxygen tag |
| **Info Object (`license`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@license` Doxygen tag |
| **Info Object (`version`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Printed on `--version` and stored in `@version` |
| **Contact Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Appended to global help or skipped |
| **Contact Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@contact_name` Doxygen tag |
| **Contact Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@contact_url` Doxygen tag |
| **Contact Object (`email`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@contact_email` Doxygen tag |
| **License Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Appended to global help or skipped |
| **License Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@license_name` Doxygen tag |
| **License Object (`identifier`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | SPDX license identifier extraction |
| **License Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@license_url` Doxygen tag |
| **Server Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Global --server or --host flag mapping |
| **Server Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Default server URL in CLI client initialization |
| **Server Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Server description in `@server` tag |
| **Server Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Unique name used as CLI alias for a host environment |
| **Server Object (`variables`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Server variable defaults and `@server_variable` tags |
| **Server Variable Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Individual host template CLI flags |
| **Server Variable Object (`enum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Allowed server variable values |
| **Server Variable Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Default value for host template substitution |
| **Server Variable Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Help text for server variable |
| **Components Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable flag groups or interactive prompt states |
| **Components Object (`schemas`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Emitted as C89 model structs for JSON payload flags |
| **Components Object (`responses`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@component_response` |
| **Components Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Resolved into CLI flags and `@component_parameter` |
| **Components Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Displayed in subcommand help and `@component_example` |
| **Components Object (`requestBodies`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Resolved into `--body` / `--data` flags and `@component_requestBody` |
| **Components Object (`headers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@component_header` |
| **Components Object (`securitySchemes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Mapped to `--token` / `--api-key` flags and `@securityScheme` |
| **Components Object (`links`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@component_link` |
| **Components Object (`callbacks`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@component_callback` |
| **Components Object (`pathItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable subcommand groupings |
| **Components Object (`mediaTypes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable payload flag definitions |
| **Paths Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Structural mapping to CLI namespaces |
| **Paths Object (`/{path}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Route path template and `@route` annotation |
| **Path Item Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Subcommand grouping |
| **Path Item Object (`$ref`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dereferenced from `components.pathItems` and `@path_ref` |
| **Path Item Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@path_summary` annotation |
| **Path Item Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@path_description` annotation |
| **Path Item Object (`get`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `GET` operation subcommand |
| **Path Item Object (`put`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `PUT` operation subcommand |
| **Path Item Object (`post`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `POST` operation subcommand |
| **Path Item Object (`delete`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `DELETE` operation subcommand |
| **Path Item Object (`options`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `OPTIONS` operation subcommand |
| **Path Item Object (`head`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `HEAD` operation subcommand |
| **Path Item Object (`patch`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `PATCH` operation subcommand |
| **Path Item Object (`trace`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `TRACE` operation subcommand |
| **Path Item Object (`query`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | QUERY HTTP method subcommand |
| **Path Item Object (`additionalOperations`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Map of custom HTTP methods to subcommands |
| **Path Item Object (`servers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Path-level server override (`@path_servers`) |
| **Path Item Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Shared path-level CLI flags (`@path_parameters`) |
| **Operation Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | The execution targets of subcommands (e.g., cli users get) |
| **Operation Object (`tags`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Subcommand category in `--help` and `@tag` |
| **Operation Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | One-line subcommand summary in `--help` |
| **Operation Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Detailed subcommand help text |
| **Operation Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@operation_externalDocs` annotation |
| **Operation Object (`operationId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Primary subcommand name (`@operationId`) |
| **Operation Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parsed from `--<param> <value>` CLI arguments |
| **Operation Object (`requestBody`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parsed from `--body <json>` CLI argument |
| **Operation Object (`responses`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Formatted JSON stdout output and exit code |
| **Operation Object (`callbacks`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@operation_callbacks` |
| **Operation Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Deprecation warning in `--help` and `@deprecated` |
| **Operation Object (`security`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Per-command auth token forwarding (`@operation_security`) |
| **Operation Object (`servers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Per-command server URL override (`@operation_servers`) |
| **External Documentation Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Added to See also: in subcommand help |
| **External Documentation Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | External docs description text |
| **External Documentation Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | External docs link URL |
| **Parameter Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | CLI option flag definition (`--<name>`) |
| **Parameter Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | CLI flag name and `@param` annotation |
| **Parameter Object (`in`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Forwarded to SDK `path`, `query`, `querystring`, `header`, or `cookie` |
| **Parameter Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Flag description in `--help` |
| **Parameter Object (`required`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Enforced required CLI flag validation before dispatch |
| **Parameter Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Marked deprecated in flag help (`@param_meta`) |
| **Parameter Object (`allowEmptyValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Allows empty string flag value (`@param_meta`) |
| **Parameter Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Shown in flag help example (`@param_meta`) |
| **Parameter Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved in `@param_meta` |
| **Parameter Object (`style`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parameter serialization style in underlying SDK call |
| **Parameter Object (`explode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Repeated/exploded flag serialization |
| **Parameter Object (`allowReserved`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reserved character handling in underlying SDK call |
| **Parameter Object (`schema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | CLI argument string-to-type conversion (`std::stoll`, `std::stod`, bool) |
| **Parameter Object (`content`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Complex JSON parameter flag parsing |
| **Request Body Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Handled via file ingest (-F @data.json), STDIN pipe, or nested flags |
| **Request Body Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `--body` flag help text |
| **Request Body Object (`content`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Deserialized via `from_json` into typed request struct |
| **Request Body Object (`required`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Enforces `--body` presence when required |
| **Media Type Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Inferred based on payload flag logic |
| **Media Type Object (`schema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Target struct schema for `--body` parsing |
| **Media Type Object (`itemSchema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Validation for individual items in a CLI array flag |
| **Media Type Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Example payload in subcommand `--help` |
| **Media Type Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Named payload examples |
| **Media Type Object (`encoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Multipart/form-data encoding rules |
| **Media Type Object (`prefixEncoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Positional prefix encoding metadata |
| **Media Type Object (`itemEncoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Streaming item encoding metadata |
| **Encoding Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Internal CLI form-data builder logic |
| **Encoding Object (`contentType`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Per-part content type |
| **Encoding Object (`headers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Per-part headers |
| **Encoding Object (`encoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Nested encoding map |
| **Encoding Object (`prefixEncoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Nested prefix encoding array |
| **Encoding Object (`itemEncoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Nested item encoding |
| **Encoding Object (`style`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Property serialization style |
| **Encoding Object (`explode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Property explode flag |
| **Encoding Object (`allowReserved`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Allow reserved characters |
| **Responses Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Determines Exit Codes (0 vs 1, etc.) |
| **Responses Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Default error output formatting |
| **Responses Object (`HTTP Status Code`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Status code handling and `@response` annotations |
| **Response Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Stdout formatting (Table format, JSON, YAML, --raw) |
| **Response Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Subcommand exit status short description |
| **Response Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Response description in help text |
| **Response Object (`headers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@response_headers` |
| **Response Object (`content`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Formatted to stdout and preserved via `@response_content` |
| **Response Object (`links`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@response_links` |
| **Callback Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Skipped (CLI is generally stateless) |
| **Callback Object (`{expression}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Callback URL runtime expression key |
| **Example Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Injected into subcommand --help 'Examples' block |
| **Example Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Example summary |
| **Example Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Example description |
| **Example Object (`dataValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAS 3.2 structured example value |
| **Example Object (`serializedValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAS 3.2 serialized wire value |
| **Example Object (`externalValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | External example URL |
| **Example Object (`value`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Embedded example value |
| **Link Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Skipped |
| **Link Object (`operationRef`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Target operation reference |
| **Link Object (`operationId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Target subcommand operationId |
| **Link Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Linked parameter expressions |
| **Link Object (`requestBody`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Linked request body expression |
| **Link Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Link description |
| **Link Object (`server`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Linked server override |
| **Header Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Can optionally be printed with -v (verbose) flags |
| **Header Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header description |
| **Header Object (`required`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Required header flag |
| **Header Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Deprecated header flag |
| **Header Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header example |
| **Header Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header examples map |
| **Header Object (`style`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header serialization style |
| **Header Object (`explode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header explode flag |
| **Header Object (`schema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header schema |
| **Header Object (`content`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header content map |
| **Tag Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | High-level CLI command groups (e.g. cli [tag] [operation]) |
| **Tag Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Tag group name |
| **Tag Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Short summary for command group |
| **Tag Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Detailed command group description |
| **Tag Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Tag external docs |
| **Tag Object (`parent`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Nested subcommand groups |
| **Tag Object (`kind`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Subcommand grouping logic (e.g. nav vs hidden) |
| **Reference Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Resolved for CLI flags and preserved in AST |
| **Reference Object (`$ref`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Internal resolution to flatten flags/commands |
| **Reference Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reference summary override |
| **Reference Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reference description override |
| **Schema Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | JSON Schema validation and struct flag binding |
| **Schema Object (`discriminator`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Polymorphic payload dispatch (`@discriminator`) |
| **Schema Object (`xml`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@xml` annotations |
| **Schema Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@schema_externalDocs` |
| **Schema Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Used in CLI help examples and tests |
| **Discriminator Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Mutually exclusive flag groups based on type |
| **Discriminator Object (`propertyName`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Discriminator JSON key name |
| **Discriminator Object (`mapping`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Value to schema `$ref` map |
| **Discriminator Object (`defaultMapping`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Fallback CLI flag group when type is omitted |
| **XML Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Skipped |
| **XML Object (`nodeType`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Skipped (CLI XML is rare) |
| **XML Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML tag/attribute name |
| **XML Object (`namespace`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML namespace URI |
| **XML Object (`prefix`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML prefix |
| **XML Object (`attribute`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML attribute flag |
| **XML Object (`wrapped`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML wrapped array flag |
| **Security Scheme Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Global `--token` / `--api-key` CLI flags |
| **Security Scheme Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `apiKey`, `http`, `mutualTLS`, `oauth2`, `openIdConnect` |
| **Security Scheme Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Auth flag description in `--help` |
| **Security Scheme Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header/query/cookie parameter name |
| **Security Scheme Object (`in`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `header`, `query`, or `cookie` |
| **Security Scheme Object (`scheme`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | HTTP auth scheme (`bearer`, `basic`) |
| **Security Scheme Object (`bearerFormat`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Bearer token format (`JWT`) |
| **Security Scheme Object (`flows`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAuth2 flows configuration |
| **Security Scheme Object (`openIdConnectUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OIDC discovery URL |
| **Security Scheme Object (`oauth2MetadataUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAuth2 metadata discovery |
| **Security Scheme Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Deprecated security scheme flag |
| **OAuth Flows Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | CLI token manager / local keychain integration |
| **OAuth Flows Object (`implicit`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implicit flow metadata |
| **OAuth Flows Object (`password`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Password grant flow |
| **OAuth Flows Object (`clientCredentials`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Client credentials grant flow |
| **OAuth Flows Object (`authorizationCode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Authorization code grant flow |
| **OAuth Flows Object (`deviceAuthorization`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Support for the Device Authorization grant flow |
| **OAuth Flow Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Flow routing logic for CLI login |
| **OAuth Flow Object (`authorizationUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Authorization endpoint URL |
| **OAuth Flow Object (`deviceAuthorizationUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Triggers CLI device auth prompt |
| **OAuth Flow Object (`tokenUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Token exchange URL |
| **OAuth Flow Object (`refreshUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Refresh token URL |
| **OAuth Flow Object (`scopes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAuth2 scope map |
| **Security Requirement Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Asserting required auth exists before command execution |
| **Security Requirement Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Security scheme name and required scopes |
