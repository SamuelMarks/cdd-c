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
| **Swagger Object (Root)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`swagger`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`info`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`host`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`basePath`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`schemes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`consumes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`produces`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`paths`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`definitions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`responses`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`securityDefinitions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`security`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`tags`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Swagger Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Info Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Info Object (`title`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Info Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Info Object (`termsOfService`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Info Object (`contact`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Info Object (`license`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Info Object (`version`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Info Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Contact Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Contact Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Contact Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Contact Object (`email`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Contact Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **License Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **License Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **License Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **License Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Paths Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Paths Object (`/{path}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Paths Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Path Item Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Path Item Object (`$ref`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Path Item Object (`get`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Path Item Object (`put`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Path Item Object (`post`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Path Item Object (`delete`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Path Item Object (`options`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Path Item Object (`head`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Path Item Object (`patch`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Path Item Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Path Item Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object (`tags`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object (`operationId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object (`consumes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object (`produces`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object (`responses`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object (`schemes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object (`security`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Operation Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **External Documentation Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **External Documentation Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **External Documentation Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **External Documentation Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`in`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`required`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`schema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`format`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`allowEmptyValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`items`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`collectionFormat`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`maximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`exclusiveMaximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`minimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`exclusiveMinimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`maxLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`minLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`pattern`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`maxItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`minItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`uniqueItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`enum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`multipleOf`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameter Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`format`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`items`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`collectionFormat`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`maximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`exclusiveMaximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`minimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`exclusiveMinimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`maxLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`minLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`pattern`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`maxItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`minItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`uniqueItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`enum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`multipleOf`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Items Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Responses Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Responses Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Responses Object (`{HTTP Status Code}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Responses Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Response Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Response Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Response Object (`schema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Response Object (`headers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Response Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Response Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Headers Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Headers Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Example Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Example Object (`{mime type}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`format`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`items`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`collectionFormat`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`maximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`exclusiveMaximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`minimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`exclusiveMinimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`maxLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`minLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`pattern`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`maxItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`minItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`uniqueItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`enum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`multipleOf`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Header Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Tag Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Tag Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Tag Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Tag Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Tag Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Reference Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Reference Object (`$ref`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`$ref`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`format`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`title`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`multipleOf`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`maximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`exclusiveMaximum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`minimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`exclusiveMinimum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`maxLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`minLength`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`pattern`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`maxItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`minItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`uniqueItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`maxProperties`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`minProperties`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`required`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`enum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`items`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`allOf`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`properties`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`additionalProperties`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`discriminator`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`readOnly`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`xml`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Schema Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **XML Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **XML Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **XML Object (`namespace`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **XML Object (`prefix`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **XML Object (`attribute`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **XML Object (`wrapped`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **XML Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Definitions Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Definitions Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameters Definitions Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Parameters Definitions Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Responses Definitions Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Responses Definitions Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Definitions Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Definitions Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Scheme Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Scheme Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Scheme Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Scheme Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Scheme Object (`in`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Scheme Object (`flow`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Scheme Object (`authorizationUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Scheme Object (`tokenUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Scheme Object (`scopes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Scheme Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Scopes Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Scopes Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Scopes Object (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Requirement Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
| **Security Requirement Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | TODO |
