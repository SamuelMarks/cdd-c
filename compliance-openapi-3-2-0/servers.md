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
| **OpenAPI Object (Root)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Root document initialization |
| **OpenAPI Object (`openapi`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`$self`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Base URI resolution for internal and external references |
| **OpenAPI Object (`info`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`jsonSchemaDialect`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Validating schemas against explicit JSON Schema drafts |
| **OpenAPI Object (`servers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`paths`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`webhooks`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Independent webhook routing / Event subscription dispatch |
| **OpenAPI Object (`components`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`security`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`tags`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`externalDocs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Global API Metadata (Title, Version, Description) |
| **Info Object (`title`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Global API short summary docstring |
| **Info Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`termsOfService`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`contact`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`license`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`version`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Contact Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Included in generated HTML docs/metadata |
| **Contact Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Contact Object (`url`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Contact Object (`email`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **License Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Included in generated HTML docs/metadata |
| **License Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **License Object (`identifier`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Server metadata / openapi.json generation |
| **License Object (`url`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Base URL generation, Router namespace/prefix mounting |
| **Server Object (`url`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Server/Router instance naming |
| **Server Object (`variables`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Variable Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Dynamic route prefix validation / Enum injection |
| **Server Variable Object (`enum`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Variable Object (`default`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Variable Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Container for reusable ORM models, DTOs, Handlers |
| **Components Object (`schemas`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`responses`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`examples`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`requestBodies`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`headers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`securitySchemes`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`links`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`callbacks`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`pathItems`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Reusable router mounting blocks |
| **Components Object (`mediaTypes`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Reusable content negotiation blocks |
| **Paths Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Core router mounting and path mapping |
| **Paths Object (`/{path}`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Grouping endpoints by URI / Route-level parameters |
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
| **Path Item Object (`query`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | QUERY HTTP method handler binding |
| **Path Item Object (`additionalOperations`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Custom HTTP method handler bindings |
| **Path Item Object (`servers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Individual Controller/Handler method bindings |
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
| **External Documentation Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Javadoc/Docstring generation or extraction |
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
| **Request Body Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Payload parsing, ORM hydration, and validation limits |
| **Request Body Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Request Body Object (`content`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Request Body Object (`required`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Content negotiation & Content-Type specific routing |
| **Media Type Object (`schema`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`itemSchema`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Validation middleware for sequential payload elements |
| **Media Type Object (`example`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`examples`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`encoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`prefixEncoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`itemEncoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Handling multipart/form-data boundary & URL-encoded arrays |
| **Encoding Object (`contentType`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`headers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`encoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`prefixEncoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`itemEncoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`style`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`explode`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`allowReserved`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Responses Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Status code mapping (`200`, `4xx`) & `default` fallback handlers |
| **Responses Object (`default`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Responses Object (`HTTP Status Code`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Response Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Outgoing DTO serialization & header formatting |
| **Response Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Docstring generation for handler responses |
| **Response Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Response Object (`headers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Response Object (`content`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Response Object (`links`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Callback Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Async out-of-band webhook dispatchers |
| **Callback Object (`{expression}`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Generating mock server endpoints / Test fixtures |
| **Example Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`dataValue`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`serializedValue`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`externalValue`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`value`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | HATEOAS / Relation mapping generation for APIs |
| **Link Object (`operationRef`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`operationId`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`requestBody`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`server`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Setting strongly-typed response headers (e.g. `X-RateLimit`) |
| **Header Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`required`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`deprecated`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`example`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`examples`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`style`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`explode`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`schema`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`content`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Controller grouping, Namespace generation |
| **Tag Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object (`externalDocs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object (`parent`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Controller inheritance / nested routers |
| **Tag Object (`kind`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Controller categorization metadata |
| **Reference Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Reference Object (`$ref`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Internal component resolution & circular dependency cycles |
| **Reference Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Reference Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object (`discriminator`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object (`xml`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object (`externalDocs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object (`example`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Discriminator Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Polymorphism / Single Table Inheritance ORM parsing |
| **Discriminator Object (`propertyName`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Discriminator Object (`mapping`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Discriminator Object (`defaultMapping`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Fallback STI mapping in ORM when missing type |
| **XML Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | XML DOM serialization/deserialization |
| **XML Object (`nodeType`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | XML deserialization binding rules (`attribute`/`element`) |
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
| **OAuth Flows Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Registering endpoints for supported grant types |
| **OAuth Flows Object (`implicit`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flows Object (`password`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flows Object (`clientCredentials`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flows Object (`authorizationCode`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flows Object (`deviceAuthorization`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Support for the Device Authorization grant flow |
| **OAuth Flow Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Extracting flow metadata (auth URL, token URL, scopes) |
| **OAuth Flow Object (`authorizationUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flow Object (`deviceAuthorizationUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Routing for issuing device codes |
| **OAuth Flow Object (`tokenUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flow Object (`refreshUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flow Object (`scopes`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Security Requirement Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Enforcing endpoint-level combinations (AND/OR auth logic) |
| **Security Requirement Object (`{name}`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
