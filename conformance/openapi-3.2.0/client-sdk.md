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
| **OpenAPI Object (Root)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Root generation / parsing |
| **OpenAPI Object (`openapi`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Semantic version validation and `@openapi` docstring round-tripping |
| **OpenAPI Object (`$self`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Base URI resolution for internal and external references |
| **OpenAPI Object (`info`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Client class Doxygen header block and `@title`/`@version` annotations |
| **OpenAPI Object (`jsonSchemaDialect`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Recognizing custom dialect validation rules locally |
| **OpenAPI Object (`servers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Default `base_url` member initialization and `@server` annotations |
| **OpenAPI Object (`paths`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Strongly-typed `ApiClient` HTTP methods and `IClient`/`MockClient` mocks |
| **OpenAPI Object (`webhooks`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Generating local event/webhook parsing utilities |
| **OpenAPI Object (`components`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Emitted as reusable C89 structs, parameter types, and `@component_*` metadata |
| **OpenAPI Object (`security`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Global auth token/API key member injection and `@security` tags |
| **OpenAPI Object (`tags`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Client method grouping and `@tag_meta` Doxygen tags |
| **OpenAPI Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Doxygen `@externalDocs` annotations on client class |
| **Info Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | SDK Header metadata, docstrings, package descriptions |
| **Info Object (`title`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@title` Doxygen tag on `ApiClient` class |
| **Info Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Short package description in package.json/pom.xml |
| **Info Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@description` Doxygen block on `ApiClient` class |
| **Info Object (`termsOfService`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@termsOfService` Doxygen tag |
| **Info Object (`contact`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@contact` Doxygen annotation |
| **Info Object (`license`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@license` Doxygen annotation |
| **Info Object (`version`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@version` Doxygen annotation and CMake project version |
| **Contact Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Package maintainer info in manifest |
| **Contact Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@contact_name` Doxygen tag |
| **Contact Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@contact_url` Doxygen tag |
| **Contact Object (`email`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@contact_email` Doxygen tag |
| **License Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Package license generation |
| **License Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@license_name` Doxygen tag |
| **License Object (`identifier`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Package license SPDX field |
| **License Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@license_url` Doxygen tag |
| **Server Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Default Base URL configuration in Client builder |
| **Server Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Default `base_url` constructor argument and `@server` URL |
| **Server Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@server` description metadata |
| **Server Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Generated as enum/constants for environment selection |
| **Server Object (`variables`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | URL template variable substitution helpers and `@server_variable` |
| **Server Variable Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | SDK builder/constructor parameters (e.g. region, env) |
| **Server Variable Object (`enum`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Allowed server variable enum values |
| **Server Variable Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Default fallback value in URL template resolution |
| **Server Variable Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Server variable docstring metadata |
| **Components Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Container for reusable types, interfaces, classes |
| **Components Object (`schemas`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Emitted as C8923 `struct` models with `to_json`/`from_json` |
| **Components Object (`responses`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Resolved on operations and preserved via `@component_response` |
| **Components Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Resolved on operations and preserved via `@component_parameter` |
| **Components Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Used in test generation and preserved via `@component_example` |
| **Components Object (`requestBodies`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Resolved into typed request body parameters (`@component_requestBody`) |
| **Components Object (`headers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@component_header` |
| **Components Object (`securitySchemes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Auth setter methods (`set_bearer_token`, `set_api_key`) and `@securityScheme` |
| **Components Object (`links`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@component_link` |
| **Components Object (`callbacks`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@component_callback` |
| **Components Object (`pathItems`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable SDK resource group resolution |
| **Components Object (`mediaTypes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable payload serialization definitions |
| **Paths Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Mapped to top-level client namespaces or groups |
| **Paths Object (`/{path}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | URL path interpolation and `@route` Doxygen tags |
| **Path Item Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Grouping related operations under a single resource |
| **Path Item Object (`$ref`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dereferenced against `components.pathItems` and `@path_ref` |
| **Path Item Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@path_summary` Doxygen annotation |
| **Path Item Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@path_description` Doxygen annotation |
| **Path Item Object (`get`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `GET` HTTP request method via `libcurl` |
| **Path Item Object (`put`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `PUT` HTTP request method via `libcurl` |
| **Path Item Object (`post`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `POST` HTTP request method via `libcurl` |
| **Path Item Object (`delete`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `DELETE` HTTP request method via `libcurl` |
| **Path Item Object (`options`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `OPTIONS` HTTP request method via `libcurl` |
| **Path Item Object (`head`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `HEAD` HTTP request method via `libcurl` |
| **Path Item Object (`patch`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `PATCH` HTTP request method via `libcurl` |
| **Path Item Object (`trace`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `TRACE` HTTP request method via `libcurl` |
| **Path Item Object (`query`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | QUERY HTTP method generation |
| **Path Item Object (`additionalOperations`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Custom HTTP method generation |
| **Path Item Object (`servers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Path-level server override and `@path_servers` |
| **Path Item Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Inherited by child operations and `@path_parameters` |
| **Operation Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Specific Client methods (e.g., client.users.get(id)) |
| **Operation Object (`tags`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@tag` Doxygen annotations on client methods |
| **Operation Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@brief` Doxygen annotation on client methods |
| **Operation Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Detailed Doxygen comment body on client methods |
| **Operation Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@operation_externalDocs` Doxygen tag |
| **Operation Object (`operationId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | C89 method identifier and `@operationId` tag |
| **Operation Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Typed C89 function arguments and `@param` tags |
| **Operation Object (`requestBody`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Typed `body` function parameter and `@requestBody` tag |
| **Operation Object (`responses`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Return type resolution and `@response` tags |
| **Operation Object (`callbacks`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@operation_callbacks` |
| **Operation Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `[[deprecated]]` C89 attribute and `@deprecated` tag |
| **Operation Object (`security`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Per-operation auth header attachment and `@operation_security` |
| **Operation Object (`servers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Per-operation base URL override and `@operation_servers` |
| **External Documentation Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Rendered into method/class Javadoc or IDE docstrings |
| **External Documentation Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | External docs description in Doxygen tag |
| **External Documentation Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | External docs URL in Doxygen tag |
| **Parameter Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Typed function arguments and URL/header/cookie builders |
| **Parameter Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Sanitized C89 parameter identifier and wire name |
| **Parameter Object (`in`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dispatched to `path`, `query`, `querystring`, `header`, or `cookie` |
| **Parameter Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@param` description text |
| **Parameter Object (`required`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Non-optional `T` vs `optional pointer/flag fields<T>` parameter type |
| **Parameter Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved in `@param_meta` |
| **Parameter Object (`allowEmptyValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved in `@param_meta` |
| **Parameter Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Used in generated SDK unit tests and `@param_meta` |
| **Parameter Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved in `@param_meta` |
| **Parameter Object (`style`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parameter serialization style (`form`, `simple`, `matrix`, `label`, `spaceDelimited`, `pipeDelimited`, `deepObject`) |
| **Parameter Object (`explode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Array/object explode serialization flag |
| **Parameter Object (`allowReserved`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reserved character encoding bypass in query builder |
| **Parameter Object (`schema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Mapped to strong C8923 parameter type |
| **Parameter Object (`content`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | JSON-serialized parameter content map |
| **Request Body Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Typed request payload object/class argument |
| **Request Body Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@requestBody` description metadata |
| **Request Body Object (`content`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Content-Type header selection and payload serialization |
| **Request Body Object (`required`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Value vs `optional pointer/flag fields` request body argument |
| **Media Type Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Automatically setting Content-Type / Accept headers |
| **Media Type Object (`schema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Mapped to C89 request/response struct type |
| **Media Type Object (`itemSchema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Strong typing for array items in sequential media types |
| **Media Type Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Mock client fixture generation and metadata |
| **Media Type Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Named example fixtures |
| **Media Type Object (`encoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Property encoding map for `multipart` and `urlencoded` bodies |
| **Media Type Object (`prefixEncoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Positional array prefix encoding rules |
| **Media Type Object (`itemEncoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Per-item streaming/multipart encoding rules |
| **Encoding Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | URL-encoding logic for complex queries, multipart builders |
| **Encoding Object (`contentType`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Per-part `Content-Type` header in multipart/form-data |
| **Encoding Object (`headers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Custom multipart section headers |
| **Encoding Object (`encoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Nested property encoding rules |
| **Encoding Object (`prefixEncoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Nested positional prefix encoding rules |
| **Encoding Object (`itemEncoding`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Nested item encoding rules |
| **Encoding Object (`style`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Form property serialization style |
| **Encoding Object (`explode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Form property explode flag |
| **Encoding Object (`allowReserved`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Unescaped reserved characters in form encoding |
| **Responses Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Return type branching (Success types vs. Error throwing) |
| **Responses Object (`default`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Fallback response model and `@response default` |
| **Responses Object (`HTTP Status Code`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Status-specific deserialization and `@response <code>` |
| **Response Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Strongly typed response payload class wrapper |
| **Response Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Javadoc/Docstring for specific response branches |
| **Response Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `@return` / `@response` description |
| **Response Object (`headers`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Response header metadata (`@response_headers`) |
| **Response Object (`content`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `simdjson` response payload deserialization (`@response_content`) |
| **Response Object (`links`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Response link metadata (`@response_links`) |
| **Callback Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Often skipped in synchronous SDKs |
| **Callback Object (`{expression}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Runtime callback URL expression key |
| **Example Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Mock HTTP client generation & unit test fixtures |
| **Example Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Example summary metadata |
| **Example Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Example description metadata |
| **Example Object (`dataValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAS 3.2 structured data example value |
| **Example Object (`serializedValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAS 3.2 wire-serialized example value |
| **Example Object (`externalValue`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | External example URI reference |
| **Example Object (`value`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Legacy embedded example value |
| **Link Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Fluent method chaining helpers (e.g., resp.getAuthor()) |
| **Link Object (`operationRef`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Relative/absolute operation URI reference |
| **Link Object (`operationId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Target linked `operationId` |
| **Link Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parameter expression map for linked call |
| **Link Object (`requestBody`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Request body expression for linked call |
| **Link Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Link documentation |
| **Link Object (`server`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Target server override for linked operation |
| **Header Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Exposing typed headers on the Return/Response object |
| **Header Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header description docstring |
| **Header Object (`required`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Required header validation flag |
| **Header Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Deprecated header flag |
| **Header Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header example value |
| **Header Object (`examples`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header named examples map |
| **Header Object (`style`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `simple` header serialization style |
| **Header Object (`explode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header explode serialization flag |
| **Header Object (`schema`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header value JSON Schema |
| **Header Object (`content`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Media-type encoded header content |
| **Tag Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | SDK Namespace grouping (e.g., client.billing.*) |
| **Tag Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Tag identifier |
| **Tag Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAS 3.2 short tag summary |
| **Tag Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Tag documentation |
| **Tag Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Tag-level external documentation |
| **Tag Object (`parent`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Nested client namespaces (e.g., client.billing.invoices) |
| **Tag Object (`kind`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Organizing generated classes by kind |
| **Reference Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `$ref` resolution and lossless AST preservation |
| **Reference Object (`$ref`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Internal/External type resolution |
| **Reference Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reference override summary |
| **Reference Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reference override description |
| **Schema Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | JSON Schema Draft 2020-12 to C8923 `struct` / `discriminated union` |
| **Schema Object (`discriminator`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Polymorphic `discriminated union` dispatch and `@discriminator` tag |
| **Schema Object (`xml`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@xml` struct/field annotations |
| **Schema Object (`externalDocs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@schema_externalDocs` |
| **Schema Object (`example`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preserved via `@example` and used in mock data generation |
| **Discriminator Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Client-side deserialization factories (Polymorphic JSON decoding) |
| **Discriminator Object (`propertyName`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | JSON discriminator tag field name |
| **Discriminator Object (`mapping`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Tag value to schema `$ref` mapping table |
| **Discriminator Object (`defaultMapping`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Default fallback for polymorphic deserialization |
| **XML Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML DOM mapping / Often skipped if JSON-only |
| **XML Object (`nodeType`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Precise XML DOM mapping controls |
| **XML Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML element/attribute name override |
| **XML Object (`namespace`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML namespace URI |
| **XML Object (`prefix`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML namespace prefix |
| **XML Object (`attribute`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Legacy boolean attribute flag |
| **XML Object (`wrapped`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | XML array wrapper element flag |
| **Security Scheme Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Client authentication configuration and header/query injection |
| **Security Scheme Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | `apiKey`, `http`, `mutualTLS`, `oauth2`, `openIdConnect` |
| **Security Scheme Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Security scheme documentation |
| **Security Scheme Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Header, query, or cookie parameter name for `apiKey` |
| **Security Scheme Object (`in`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | API key location (`header`, `query`, `cookie`) |
| **Security Scheme Object (`scheme`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | HTTP auth scheme (`bearer`, `basic`) |
| **Security Scheme Object (`bearerFormat`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Bearer token format hint (e.g. `JWT`) |
| **Security Scheme Object (`flows`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAuth2 flow configurations |
| **Security Scheme Object (`openIdConnectUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OpenID Connect discovery URL |
| **Security Scheme Object (`oauth2MetadataUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAuth2 metadata discovery |
| **Security Scheme Object (`deprecated`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | OAS 3.2 deprecated security scheme flag |
| **OAuth Flows Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Helpers for token exchange requests |
| **OAuth Flows Object (`implicit`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implicit OAuth2 flow metadata |
| **OAuth Flows Object (`password`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Resource Owner Password Credentials flow helper |
| **OAuth Flows Object (`clientCredentials`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Client Credentials token exchange helper |
| **OAuth Flows Object (`authorizationCode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Authorization Code flow helper |
| **OAuth Flows Object (`deviceAuthorization`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Support for the Device Authorization grant flow |
| **OAuth Flow Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | URL discovery for token exchange |
| **OAuth Flow Object (`authorizationUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Authorization endpoint URL |
| **OAuth Flow Object (`deviceAuthorizationUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | SDK method for initiating device flow authentication |
| **OAuth Flow Object (`tokenUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Token endpoint URL |
| **OAuth Flow Object (`refreshUrl`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Token refresh endpoint URL |
| **OAuth Flow Object (`scopes`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Available OAuth2 scopes map |
| **Security Requirement Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Automatic attachment of required auth headers per method |
| **Security Requirement Object (`{name}`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Security scheme name to required scopes list |
