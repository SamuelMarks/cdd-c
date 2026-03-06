# OpenAPI 3.2.0 Conformance Table: Client SDK (HTTP Client + Mocks + Tests)

This table tracks the completeness of language integration with OpenAPI 3.2.0 for Client SDK generation and extraction.

### Legend & Tracking Guide
*   **To**: Language -> OpenAPI (Generating the OpenAPI document from strongly typed client SDKs)
*   **From**: OpenAPI -> Language (Generating HTTP Client code, interfaces, and methods from the OpenAPI document)
*   **Presence `[To, From]`**: The object is successfully parsed, validated, utilized, or generated.
*   **Absence `[To, From]`**: The object is currently unsupported, dropped, or falls back to generic/`any` types.
*   **Skipped `[To, From]`**: Intentionally ignored because it is irrelevant or unsupported by the Client architecture.
*   **Checkboxes**: Mark `[x]` as conformance is achieved.

| OpenAPI 3.2.0 Object / Feature | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes / Implementation Strategy |
| :--- | :---: | :---: | :---: | :--- |
| **OpenAPI Object (Root)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Root generation / parsing |
| **OpenAPI Object (`openapi`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`$self`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Base URI resolution for internal and external references |
| **OpenAPI Object (`info`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`jsonSchemaDialect`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Recognizing custom dialect validation rules locally |
| **OpenAPI Object (`servers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`paths`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`webhooks`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Generating local event/webhook parsing utilities |
| **OpenAPI Object (`components`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`security`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`tags`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OpenAPI Object (`externalDocs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | SDK Header metadata, docstrings, package descriptions |
| **Info Object (`title`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Short package description in package.json/pom.xml |
| **Info Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`termsOfService`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`contact`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`license`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Info Object (`version`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Contact Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Package maintainer info in manifest |
| **Contact Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Contact Object (`url`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Contact Object (`email`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **License Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Package license generation |
| **License Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **License Object (`identifier`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Package license SPDX field |
| **License Object (`url`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Default Base URL configuration in Client builder |
| **Server Object (`url`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Generated as enum/constants for environment selection |
| **Server Object (`variables`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Variable Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | SDK builder/constructor parameters (e.g. `region`, `env`) |
| **Server Variable Object (`enum`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Variable Object (`default`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Server Variable Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Container for reusable types, interfaces, classes |
| **Components Object (`schemas`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`responses`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`examples`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`requestBodies`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`headers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`securitySchemes`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`links`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`callbacks`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Components Object (`pathItems`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Reusable SDK resource group resolution |
| **Components Object (`mediaTypes`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Reusable payload serialization definitions |
| **Paths Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Mapped to top-level client namespaces or groups |
| **Paths Object (`/{path}`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Grouping related operations under a single resource |
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
| **Path Item Object (`query`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | QUERY HTTP method generation |
| **Path Item Object (`additionalOperations`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Custom HTTP method generation |
| **Path Item Object (`servers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Path Item Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Operation Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Specific Client methods (e.g., `client.users.get(id)`) |
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
| **External Documentation Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Rendered into method/class Javadoc or IDE docstrings |
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
| **Request Body Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Typed request payload object/class argument |
| **Request Body Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Request Body Object (`content`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Request Body Object (`required`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Automatically setting `Content-Type` / `Accept` headers |
| **Media Type Object (`schema`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`itemSchema`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Strong typing for array items in sequential media types |
| **Media Type Object (`example`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`examples`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`encoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`prefixEncoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Media Type Object (`itemEncoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | URL-encoding logic for complex queries, multipart builders |
| **Encoding Object (`contentType`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`headers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`encoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`prefixEncoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`itemEncoding`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`style`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`explode`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Encoding Object (`allowReserved`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Responses Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Return type branching (Success types vs. Error throwing) |
| **Responses Object (`default`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Responses Object (`HTTP Status Code`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Response Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Strongly typed response payload class wrapper |
| **Response Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Javadoc/Docstring for specific response branches |
| **Response Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Response Object (`headers`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Response Object (`content`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Response Object (`links`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Callback Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Often skipped in synchronous SDKs |
| **Callback Object (`{expression}`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Mock HTTP client generation & unit test fixtures |
| **Example Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`dataValue`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`serializedValue`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`externalValue`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Example Object (`value`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Fluent method chaining helpers (e.g., `resp.getAuthor()`) |
| **Link Object (`operationRef`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`operationId`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`requestBody`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Link Object (`server`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Exposing typed headers on the Return/Response object |
| **Header Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`required`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`deprecated`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`example`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`examples`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`style`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`explode`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`schema`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Header Object (`content`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | SDK Namespace grouping (e.g., `client.billing.*`) |
| **Tag Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object (`externalDocs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Tag Object (`parent`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Nested client namespaces (e.g., `client.billing.invoices`) |
| **Tag Object (`kind`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Organizing generated classes by kind |
| **Reference Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Reference Object (`$ref`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Internal/External type resolution |
| **Reference Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Reference Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object (`discriminator`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object (`xml`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object (`externalDocs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Schema Object (`example`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Discriminator Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Client-side deserialization factories (Polymorphic JSON decoding) |
| **Discriminator Object (`propertyName`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Discriminator Object (`mapping`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Discriminator Object (`defaultMapping`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Default fallback for polymorphic deserialization |
| **XML Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | XML DOM mapping / Often skipped if JSON-only |
| **XML Object (`nodeType`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Precise XML DOM mapping controls |
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
| **OAuth Flows Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Helpers for token exchange requests |
| **OAuth Flows Object (`implicit`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flows Object (`password`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flows Object (`clientCredentials`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flows Object (`authorizationCode`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flows Object (`deviceAuthorization`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Support for the Device Authorization grant flow |
| **OAuth Flow Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | URL discovery for token exchange |
| **OAuth Flow Object (`authorizationUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flow Object (`deviceAuthorizationUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | SDK method for initiating device flow authentication |
| **OAuth Flow Object (`tokenUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flow Object (`refreshUrl`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **OAuth Flow Object (`scopes`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
| **Security Requirement Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Automatic attachment of required auth headers per method |
| **Security Requirement Object (`{name}`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | TODO |
