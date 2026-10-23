# Swagger 2.0 Conformance Table: Servers (ORM + REST API + Mocks + Tests)

This table tracks the completeness of language integration with Swagger 2.0 for server-side generation and extraction.

### Legend & Tracking Guide
*   **To**: Language -> OpenAPI (Generating the Swagger document from code/types/decorators)
*   **From**: OpenAPI -> Language (Generating server stubs/types/ORM models from the Swagger document)
*   **Presence `[To, From]`**: The object is successfully parsed, validated, utilized, or generated.
*   **Absence `[To, From]`**: The object is currently unsupported, dropped, or falls back to generic/`any` types.
*   **Skipped `[To, From]`**: Intentionally ignored because it is irrelevant or unsupported by the architecture.
*   **Checkboxes**: Mark `[x]` as conformance is achieved.

| Swagger 2.0 Object / Feature | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes / Implementation Strategy |
| :--- | :---: | :---: | :---: | :--- |
| **Swagger Object (Root)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`swagger`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`info`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`host`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`basePath`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`schemes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`consumes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`produces`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`paths`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`definitions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`responses`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`securityDefinitions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`security`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`tags`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Swagger Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Info Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Info Object (`title`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Info Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Info Object (`termsOfService`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Info Object (`contact`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Info Object (`license`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Info Object (`version`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Info Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Contact Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Contact Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Contact Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Contact Object (`email`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Contact Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **License Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **License Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **License Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **License Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Paths Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Paths Object (`/{path}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Paths Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Path Item Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Path Item Object (`$ref`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Path Item Object (`get`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Path Item Object (`put`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Path Item Object (`post`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Path Item Object (`delete`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Path Item Object (`options`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Path Item Object (`head`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Path Item Object (`patch`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Path Item Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Path Item Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Operation Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Operation Object (`tags`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Operation Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Operation Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Operation Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Operation Object (`operationId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Operation Object (`consumes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Operation Object (`produces`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Operation Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Operation Object (`responses`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Operation Object (`schemes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dropped / Not converted to OAS3 |
| **Operation Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Operation Object (`security`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Operation Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **External Documentation Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **External Documentation Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **External Documentation Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **External Documentation Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`in`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`required`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`schema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`format`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`allowEmptyValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`items`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`collectionFormat`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dropped / Not converted to OAS3 |
| **Parameter Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`maximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`exclusiveMaximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`minimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`exclusiveMinimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`maxLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`minLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`pattern`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`maxItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`minItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`uniqueItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`enum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`multipleOf`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameter Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`format`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`items`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`collectionFormat`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dropped / Not converted to OAS3 |
| **Items Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`maximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`exclusiveMaximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`minimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`exclusiveMinimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`maxLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`minLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`pattern`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`maxItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`minItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`uniqueItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`enum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`multipleOf`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Items Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Responses Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Responses Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Responses Object (`{HTTP Status Code}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Responses Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Response Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Response Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Response Object (`schema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Response Object (`headers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Response Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Response Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Headers Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Headers Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Example Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Example Object (`{mime type}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`format`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`items`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`collectionFormat`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dropped / Not converted to OAS3 |
| **Header Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`maximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`exclusiveMaximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`minimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`exclusiveMinimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`maxLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`minLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`pattern`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`maxItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`minItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`uniqueItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`enum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`multipleOf`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Header Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Tag Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Tag Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Tag Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Tag Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Tag Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Reference Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Reference Object (`$ref`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`$ref`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`format`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`title`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`multipleOf`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`maximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`exclusiveMaximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`minimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`exclusiveMinimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`maxLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`minLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`pattern`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`maxItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`minItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`uniqueItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`maxProperties`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`minProperties`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`required`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`enum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`items`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`allOf`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`properties`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`additionalProperties`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`discriminator`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`readOnly`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`xml`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Schema Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **XML Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **XML Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **XML Object (`namespace`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **XML Object (`prefix`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **XML Object (`attribute`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **XML Object (`wrapped`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **XML Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Definitions Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Definitions Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameters Definitions Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Parameters Definitions Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Responses Definitions Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Responses Definitions Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Definitions Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Definitions Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Scheme Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Scheme Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Scheme Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Scheme Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Scheme Object (`in`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Scheme Object (`flow`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Scheme Object (`authorizationUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Scheme Object (`tokenUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Scheme Object (`scopes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Scheme Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Scopes Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Scopes Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Scopes Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Requirement Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
| **Security Requirement Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Upgraded to OAS 3.2.0 via openapi_load_from_json / Swagger 2.0 to OAS 3.2.0 upgrader in src/openapi/parse/openapi.c |
