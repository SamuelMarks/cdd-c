# OpenAPI 3.2.0 Conformance Table: Servers (ORM + REST API + Mocks + Tests)

This table tracks the completeness of language integration with OpenAPI 3.2.0 for server-side generation and extraction.

### Legend & Tracking Guide
*   **To**: Language -> OpenAPI (Generating the OpenAPI document from code/types/decorators)
*   **From**: OpenAPI -> Language (Generating server stubs/types/ORM models from the OpenAPI document)
*   **Presence `[To, From]`**: The object is successfully parsed, validated, utilized, or generated.
*   **Absence `[To, From]`**: The object is currently unsupported, dropped, or falls back to generic/`any` types.
*   **Skipped `[To, From]`**: Intentionally ignored because it is irrelevant or unsupported by the server architecture.
*   **Checkboxes**: Mark `[x]` as conformance is achieved.

| OpenAPI 3.2.0 Object / Feature | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes / Implementation Strategy |
| :--- | :---: | :---: | :---: | :--- |
| **OpenAPI Object (Root)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Root document initialization |
| **OpenAPI Object (`openapi`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Version validation and `@openapi` docstring annotation |
| **OpenAPI Object (`$self`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Base URI resolution for internal and external references |
| **OpenAPI Object (`info`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Generated C server Doxygen block and `openapi.json` metadata |
| **OpenAPI Object (`jsonSchemaDialect`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Validating schemas against explicit JSON Schema drafts |
| **OpenAPI Object (`servers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Default bind host/port and `@server` Doxygen annotations |
| **OpenAPI Object (`paths`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `c-abstract-http / generated C server router` route registration and virtual handler hooks |
| **OpenAPI Object (`webhooks`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Independent webhook routing / Event subscription dispatch |
| **OpenAPI Object (`components`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | C structs, c-orm SQL DDL/DAOs, and mock stubs, and `@component_*` metadata |
| **OpenAPI Object (`security`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Global auth validation middleware (`--enforce-auth`) and `@security` |
| **OpenAPI Object (`tags`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Handler organization and `@tag_meta` Doxygen tags |
| **OpenAPI Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@externalDocs` Doxygen tag on server class |
| **Info Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Global API Metadata (Title, Version, Description) |
| **Info Object (`title`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@title` Doxygen annotation in C server docstrings |
| **Info Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Global API short summary docstring |
| **Info Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@description` Doxygen annotation in C server docstrings |
| **Info Object (`termsOfService`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@termsOfService` Doxygen annotation |
| **Info Object (`contact`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@contact` Doxygen annotation |
| **Info Object (`license`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@license` Doxygen annotation |
| **Info Object (`version`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@version` Doxygen annotation |
| **Contact Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Included in generated HTML docs/metadata |
| **Contact Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@contact_name` Doxygen tag |
| **Contact Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@contact_url` Doxygen tag |
| **Contact Object (`email`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@contact_email` Doxygen tag |
| **License Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Included in generated HTML docs/metadata |
| **License Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@license_name` Doxygen tag |
| **License Object (`identifier`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Server metadata / openapi.json generation |
| **License Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@license_url` Doxygen tag |
| **Server Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Base URL generation, Router namespace/prefix mounting |
| **Server Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Route prefix mounting and `@server` URL |
| **Server Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@server` description metadata |
| **Server Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Server/Router instance naming |
| **Server Object (`variables`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dynamic route prefix variables (`@server_variable`) |
| **Server Variable Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dynamic route prefix validation / Enum injection |
| **Server Variable Object (`enum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Allowed server variable enum values |
| **Server Variable Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Default route variable value |
| **Server Variable Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Server variable description |
| **Components Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Container for reusable ORM models, DTOs, Handlers |
| **Components Object (`schemas`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | C structs (`src/classes/emit/struct.c`), SQL DDL/c-orm tables (`src/classes/emit/sql.c`), and mock server handlers |
| **Components Object (`responses`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable response serializers and `@component_response` |
| **Components Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable route parameter extractors and `@component_parameter` |
| **Components Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Mock response fixtures and `@component_example` |
| **Components Object (`requestBodies`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable request payload parsers and `@component_requestBody` |
| **Components Object (`headers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable response header setters and `@component_header` |
| **Components Object (`securitySchemes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Auth middleware + embedded OAuth2/OIDC IdP (`@securityScheme`) |
| **Components Object (`links`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@component_link` |
| **Components Object (`callbacks`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Outgoing callback dispatchers and `@component_callback` |
| **Components Object (`pathItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable router mounting blocks |
| **Components Object (`mediaTypes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable content negotiation blocks |
| **Paths Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Core router mounting and path mapping |
| **Paths Object (`/{path}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Converted from `{param}` to `:param` regex routes and `@route` tags |
| **Path Item Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Grouping endpoints by URI / Route-level parameters |
| **Path Item Object (`$ref`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dereferenced from `components.pathItems` and `@path_ref` |
| **Path Item Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@path_summary` Doxygen annotation |
| **Path Item Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@path_description` Doxygen annotation |
| **Path Item Object (`get`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `HTTP GET` route handler registration |
| **Path Item Object (`put`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `HTTP PUT` route handler registration |
| **Path Item Object (`post`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `HTTP POST` route handler registration |
| **Path Item Object (`delete`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `HTTP DELETE` route handler registration |
| **Path Item Object (`options`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `HTTP OPTIONS` route and CORS preflight handler |
| **Path Item Object (`head`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `HTTP GET` / HEAD route handler |
| **Path Item Object (`patch`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `HTTP PATCH` route handler registration |
| **Path Item Object (`trace`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Custom method route handler |
| **Path Item Object (`query`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | QUERY HTTP method handler binding |
| **Path Item Object (`additionalOperations`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Custom HTTP method handler bindings |
| **Path Item Object (`servers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Path-level server metadata (`@path_servers`) |
| **Path Item Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Shared path-level parameter extraction (`@path_parameters`) |
| **Operation Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Individual Controller/Handler method bindings |
| **Operation Object (`tags`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@tag` Doxygen annotations on server handlers |
| **Operation Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@brief` Doxygen annotation on server handlers |
| **Operation Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Detailed Doxygen comment on server handlers |
| **Operation Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@operation_externalDocs` Doxygen tag |
| **Operation Object (`operationId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Handler method name and `@operationId` tag |
| **Operation Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Extracted from `req.path_params`, `req.params`, `req.headers` |
| **Operation Object (`requestBody`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parsed via `from_json(req.body)` and validated via RFC 7807 middleware |
| **Operation Object (`responses`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `res.status` and `res.set_content(to_json(...), ...)` |
| **Operation Object (`callbacks`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dispatched via `/_mock/trigger-webhook` and `@operation_callbacks` |
| **Operation Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `[[deprecated]]` attribute and `@deprecated` tag |
| **Operation Object (`security`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Per-route auth check (`401`/`403`) and `@operation_security` |
| **Operation Object (`servers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Per-route server metadata (`@operation_servers`) |
| **External Documentation Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Javadoc/Docstring generation or extraction |
| **External Documentation Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | External docs description |
| **External Documentation Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | External docs URL |
| **Parameter Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Route parameter extraction, type conversion, and validation |
| **Parameter Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parameter lookup key and C89 handler argument name |
| **Parameter Object (`in`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Extracted from `path`, `query`, `querystring`, `header`, or `cookie` |
| **Parameter Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@param` docstring description |
| **Parameter Object (`required`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Enforced in request validation middleware (`400 Bad Request`) |
| **Parameter Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved in `@param_meta` |
| **Parameter Object (`allowEmptyValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved in `@param_meta` |
| **Parameter Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Used in generated server tests and `@param_meta` |
| **Parameter Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved in `@param_meta` |
| **Parameter Object (`style`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parameter deserialization style |
| **Parameter Object (`explode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Multi-value parameter parsing |
| **Parameter Object (`allowReserved`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reserved character decoding |
| **Parameter Object (`schema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Strong C89 parameter type and constraint validation |
| **Parameter Object (`content`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | JSON-encoded parameter parsing |
| **Request Body Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Payload parsing, ORM hydration, and validation limits |
| **Request Body Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@requestBody` description |
| **Request Body Object (`content`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Content-Type validation (`415 Unsupported Media Type`) and JSON parsing |
| **Request Body Object (`required`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Rejects empty body with RFC 7807 `400 Bad Request` when required |
| **Media Type Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Content negotiation & Content-Type specific routing |
| **Media Type Object (`schema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Schema constraint validation (`minLength`, `minimum`, `enum`, `required`) |
| **Media Type Object (`itemSchema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Validation middleware for sequential payload elements |
| **Media Type Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Default mock response payload when DAO is not bound |
| **Media Type Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Named mock response fixtures |
| **Media Type Object (`encoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Multipart/urlencoded field decoding metadata |
| **Media Type Object (`prefixEncoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Positional prefix encoding metadata |
| **Media Type Object (`itemEncoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Sequential item encoding metadata |
| **Encoding Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Handling multipart/form-data boundary & URL-encoded arrays |
| **Encoding Object (`contentType`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Multipart part content type |
| **Encoding Object (`headers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Multipart part headers |
| **Encoding Object (`encoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Nested encoding map |
| **Encoding Object (`prefixEncoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Nested prefix encoding array |
| **Encoding Object (`itemEncoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Nested item encoding |
| **Encoding Object (`style`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Form property style |
| **Encoding Object (`explode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Form property explode flag |
| **Encoding Object (`allowReserved`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Form property allowReserved flag |
| **Responses Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Status code mapping (200, 4xx) & default fallback handlers |
| **Responses Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Default response status and `@response default` |
| **Responses Object (`HTTP Status Code`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Explicit HTTP status code response emission (`@response <code>`) |
| **Response Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Outgoing DTO serialization & header formatting |
| **Response Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Docstring generation for handler responses |
| **Response Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@response` description text |
| **Response Object (`headers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Outgoing response headers and `@response_headers` |
| **Response Object (`content`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Outgoing `res.set_content` media type and `@response_content` |
| **Response Object (`links`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@response_links` |
| **Callback Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Async out-of-band webhook dispatchers |
| **Callback Object (`{expression}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Runtime callback target URL expression |
| **Example Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Generating mock server endpoints / Test fixtures |
| **Example Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Example summary |
| **Example Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Example description |
| **Example Object (`dataValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAS 3.2 structured example payload |
| **Example Object (`serializedValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAS 3.2 wire-serialized example payload |
| **Example Object (`externalValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | External example URI |
| **Example Object (`value`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Embedded example value |
| **Link Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | HATEOAS / Relation mapping generation for APIs |
| **Link Object (`operationRef`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Linked operation reference |
| **Link Object (`operationId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Linked operationId |
| **Link Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Linked parameter map |
| **Link Object (`requestBody`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Linked request body |
| **Link Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Link description |
| **Link Object (`server`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Linked server override |
| **Header Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Setting strongly-typed response headers (e.g. X-RateLimit) |
| **Header Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header description |
| **Header Object (`required`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Required header flag |
| **Header Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Deprecated header flag |
| **Header Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header example value |
| **Header Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header examples map |
| **Header Object (`style`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header serialization style |
| **Header Object (`explode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header explode flag |
| **Header Object (`schema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header value schema |
| **Header Object (`content`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header media type content |
| **Tag Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Controller grouping, Namespace generation |
| **Tag Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Tag name |
| **Tag Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Tag short summary |
| **Tag Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Tag description |
| **Tag Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Tag external documentation |
| **Tag Object (`parent`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Controller inheritance / nested routers |
| **Tag Object (`kind`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Controller categorization metadata |
| **Reference Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Component dereferencing and AST `$ref` preservation |
| **Reference Object (`$ref`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Internal component resolution & circular dependency cycles |
| **Reference Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reference summary override |
| **Reference Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reference description override |
| **Schema Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | C89 structs, SQL DDL columns/constraints, Faker generators |
| **Schema Object (`discriminator`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Polymorphic `discriminated union` deserialization and `@discriminator` |
| **Schema Object (`xml`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@xml` annotations |
| **Schema Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@schema_externalDocs` |
| **Schema Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Used in Faker fallback seeding and `@example` |
| **Discriminator Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Polymorphism / Single Table Inheritance ORM parsing |
| **Discriminator Object (`propertyName`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Discriminator JSON property name |
| **Discriminator Object (`mapping`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Discriminator value to schema `$ref` map |
| **Discriminator Object (`defaultMapping`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Fallback STI mapping in ORM when missing type |
| **XML Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML DOM serialization/deserialization |
| **XML Object (`nodeType`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML deserialization binding rules (attribute/element) |
| **XML Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML element/attribute name |
| **XML Object (`namespace`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML namespace URI |
| **XML Object (`prefix`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML namespace prefix |
| **XML Object (`attribute`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML attribute flag |
| **XML Object (`wrapped`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML wrapped array flag |
| **Security Scheme Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Auth middleware + embedded OAuth2/OIDC Identity Provider |
| **Security Scheme Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `apiKey`, `http`, `mutualTLS`, `oauth2`, `openIdConnect` |
| **Security Scheme Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Security scheme description |
| **Security Scheme Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header, query, or cookie name for `apiKey` validation |
| **Security Scheme Object (`in`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `header`, `query`, or `cookie` extractor |
| **Security Scheme Object (`scheme`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `Authorization: Bearer` / `Basic` validation |
| **Security Scheme Object (`bearerFormat`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | JWT token format issued by embedded IdP |
| **Security Scheme Object (`flows`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Embedded `/oauth/authorize` and `/oauth/token` endpoints |
| **Security Scheme Object (`openIdConnectUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Embedded `/.well-known/openid-configuration` & `/.well-known/jwks.json` |
| **Security Scheme Object (`oauth2MetadataUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAuth2 metadata discovery |
| **Security Scheme Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Deprecated security scheme flag |
| **OAuth Flows Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Registering endpoints for supported grant types |
| **OAuth Flows Object (`implicit`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implicit grant handling on `/oauth/authorize` |
| **OAuth Flows Object (`password`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `grant_type=password` handling on `/oauth/token` |
| **OAuth Flows Object (`clientCredentials`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `grant_type=client_credentials` handling on `/oauth/token` |
| **OAuth Flows Object (`authorizationCode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `grant_type=authorization_code` handling on `/oauth/token` |
| **OAuth Flows Object (`deviceAuthorization`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Support for the Device Authorization grant flow |
| **OAuth Flow Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Extracting flow metadata (auth URL, token URL, scopes) |
| **OAuth Flow Object (`authorizationUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Authorization endpoint URL |
| **OAuth Flow Object (`deviceAuthorizationUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Routing for issuing device codes |
| **OAuth Flow Object (`tokenUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Token endpoint URL |
| **OAuth Flow Object (`refreshUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Refresh token endpoint URL |
| **OAuth Flow Object (`scopes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Validated scopes map |
| **Security Requirement Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Enforcing endpoint-level combinations (AND/OR auth logic) |
| **Security Requirement Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Scheme name and required scope enforcement (`403 Forbidden`) |
