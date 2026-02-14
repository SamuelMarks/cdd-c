Usage
=====

The `c_cdd_cli` binary serves as the entry point for multiple modes of operation including auditing, schema extraction,
and code generation.

## Command Reference

### 1. Audit Project

Scans a directory for memory safety issues, specifically focusing on unchecked allocations (e.g., `malloc` results used
without `NULL` checks) and functions that return raw pointers.

```bash
c_cdd_cli audit <source_directory>
```

**Output:** Prints a summary of scanned files and a JSON object containing specific violations (file, line number,
variable name).

### 2. C to OpenAPI (c2openapi)

Generates an OpenAPI v3.2 JSON specification by analyzing C source code, function signatures, and Doxygen-style
comments.

```bash
c_cdd_cli c2openapi <source_directory> <output_spec.json>
```

Optional base spec (for root metadata/components reuse):

```bash
c_cdd_cli c2openapi --base <base_openapi.json> <source_directory> <output_spec.json>
```

Optional `$self` URI (can be combined with `--base`):

```bash
c_cdd_cli c2openapi --self <uri> <source_directory> <output_spec.json>
```

Optional `jsonSchemaDialect` (can be combined with `--base`):

```bash
c_cdd_cli c2openapi --dialect <uri> <source_directory> <output_spec.json>
```

**Annotations:**
To expose a C function to the OpenAPI generator, add documentation comments:

```c
/**
 * @route GET /users/{id}
 * @summary Get a user by ID
 * @param id [in:path] The user ID
 */
int api_get_user(int id, struct User **out);
```

Additional annotations supported:

- `@description <text>` for an operation description.
- `@operationId <id>` to override the default operationId.
- `@tag <name>` or `@tags <name1, name2>` to set operation tags.
- `@tagMeta <name> [summary:<text>] [description:<text>] [parent:<name>] [kind:<value>] [externalDocs:<url>] [externalDocsDescription:<text>]` to enrich top-level Tag Objects.
- `@webhook <VERB> <PATH>` to emit an operation under top-level `webhooks`.
- `@deprecated [true|false]` to mark an operation deprecated.
- `@externalDocs <url> [description]` to attach external docs metadata.
- `@security <scheme> [scope1, scope2]` to set operation security.
- `@securityScheme <name> [type:<apiKey|http|mutualTLS|oauth2|openIdConnect>]` to define security schemes in `components.securitySchemes`.
- `@server <url> [name=<name>] [description=<text>]` to add operation servers.
- `@serverVar <name> [default:<value>] [enum:<v1,v2,...>] [description:<text>]` to define server variables for the most recent `@server` in the same doc block.
- `@infoTitle <text>` to set root `info.title`.
- `@infoVersion <text>` to set root `info.version`.
- `@infoSummary <text>` to set root `info.summary`.
- `@infoDescription <text>` to set root `info.description`.
- `@termsOfService <url>` to set root `info.termsOfService`.
- `@contact [name:<text>] [url:<url>] [email:<email>]` to set root `info.contact`.
- `@license [name:<text>] [identifier:<spdx>] [url:<url>]` to set root `info.license`.
- `@requestBody [required|required:true|required:false] [contentType:<media/type>] [example:<json>] <description>` to describe request bodies.
- `@return <status> [contentType:<media/type>] [summary:<text>] [example:<json>] <description>` to describe response content.
- `@responseHeader <status> <name> [type:<string|integer|number|boolean>] [format:<fmt>] [contentType:<media/type>] [required|required:true|required:false] [example:<json>] <description>` to describe response headers.
- `@link <status> <name> [operationId:<id> | operationRef:<uri>] [parameters:<json>] [requestBody:<json>] [summary:<text>] [description:<text>] [serverUrl:<url>] [serverName:<name>] [serverDescription:<text>] <description>` to describe response links.
- `@param` flags: `[in:<path|query|header|cookie|querystring>] [required]`
  `[style:<form|simple|matrix|label|spaceDelimited|pipeDelimited|deepObject|cookie>]`
  `[explode:true|false] [allowReserved:true|false] [allowEmptyValue:true|false] [contentType:<media/type>] [format:<fmt>] [deprecated:true|false] [example:<json>]`

**Notes:**
- Generated specs default to OpenAPI `3.2.0`.
- If no `@return` is provided and no output parameter is detected, `c2openapi`
  emits a default `200` response with description `Success`.
- Custom HTTP methods in `@route` (for example `COPY`) are emitted under
  Path Item `additionalOperations`.
- Operation tags are emitted as top-level Tag Objects in generated specs.
- `@tagMeta` enriches top-level Tag Objects with summary/description/parent/kind/externalDocs.
- Repeating `@return` for the same status with different `contentType` values
  emits multi-content responses.
- Repeating `@requestBody` with different `contentType` values emits a
  multi-content request body.
- Omitting `contentType` on `@requestBody` defaults to `application/json`.
- Querystring parameters with `contentType:application/json` use the C type
  mapping to populate the parameter schema (including `$ref` for structs) and
  SDK generation supports inline JSON primitives/arrays for querystring values.
- Querystring parameters with non-JSON content types (for example
  `text/plain` or `application/jsonpath`) are serialized as raw values and
  URL-encoded in generated clients, with primitive C types preserved where
  possible.
- The loader/writer round-trips `$self`, `jsonSchemaDialect`, root `externalDocs`, and top-level `tags` metadata, resolves `$self`-qualified component `$ref` URIs (e.g. `https://host/spec.json#/components/schemas/Pet`) when loading, resolves component schemas by `$id` and `$anchor`/`$dynamicAnchor` when `$ref`/`$dynamicRef` targets those identifiers, and supports multi-document component `$ref` resolution using `openapi_doc_registry_*` with `openapi_load_from_json_with_context`.
- The loader accepts Schema-only documents (JSON Schema at the root); it marks `spec.is_schema_document` and stores the serialized root schema in `spec.schema_root_json`, using `$id` (or the retrieval URI) as the base URI for registry lookups. The writer now re-emits `schema_root_json` when `spec.is_schema_document` is set.
- Standalone doc blocks (no `@route`) can define root metadata: `@infoTitle`, `@infoVersion`, `@infoSummary`, `@infoDescription`, `@termsOfService`, `@contact`, `@license`, `@server` for root `servers`, `@serverVar` for root server variables, `@externalDocs` for root `externalDocs`, `@security` for root `security`, and `@securityScheme` for `components.securitySchemes`.
- Supported operation fields include `summary`, `description`, `deprecated`, `externalDocs`, and the HTTP `QUERY` method in addition to the standard verbs.
- The loader/writer round-trips `info` metadata (title/summary/description/terms/contact/license), root `servers`, `components.securitySchemes` (apiKey/http/openIdConnect + oauth2 metadata URL), root/operation `security` requirements, request body `description`/`required`, and response `description`/content media type.
- When `--base` is provided, the base spec must be OpenAPI 3.x JSON (not YAML). Its
  root metadata (`info`, `$self`, `jsonSchemaDialect`, `externalDocs`, `servers`, `tags`, `security`) and `components.*`
  are preserved and merged with generated operations and schemas.
- Response headers are emitted from `@responseHeader` directives as Header Objects under the matching response code.
- Media Type, Parameter, Header, Response, and Request Body examples (`example`, `examples`, `dataValue`, `serializedValue`, `externalValue`) plus `components.examples` are preserved.
- Parameter metadata `description`, `deprecated`, `allowReserved`, `allowEmptyValue`, and `format` are preserved in both directions; `@param` descriptions flow into generated OpenAPI.
- Parameter styles cover `form`, `simple`, `matrix`, `label`, `spaceDelimited`, `pipeDelimited`, `deepObject`, and `cookie`. Content-based parameters are parsed/emitted for any location.
- OpenAPI compliance guards: parameters/headers MUST use either `schema` or `content` (not both), and parameter/header `content` MUST contain exactly one media type; parameter `name`/`in` are required; parameter lists MUST NOT contain duplicates (`name` + `in`); parameter `style` is validated by location (with deepObject/spaceDelimited/pipeDelimited type checks); querystring parameters are limited to a single occurrence and are forbidden alongside `in: query` parameters (including callbacks); header parameters named `Accept`, `Content-Type`, or `Authorization` are ignored; response/encoding headers named `Content-Type` are ignored; Header Objects only allow `style: simple`; Media Type / Encoding Objects MUST NOT combine `encoding` with `prefixEncoding` or `itemEncoding`; `paths` keys must start with `/`; component keys must match `^[a-zA-Z0-9._-]+$`; tag names and `operationId` values must be unique (including callbacks); path templates must have matching `in: path` parameters and those parameters must be `required: true`; operations MUST define a non-empty `responses` object with valid status code keys (`default`, a 3-digit code, or `1XX`-`5XX`); additionalOperations MUST NOT include standard HTTP methods; Link Objects must specify exactly one of `operationId` or `operationRef`; explicit Server entries must define `url` and server URL variables must be defined and unique; server `name` values must be unique within a `servers` array when provided; security schemes enforce required fields (`apiKey` requires `name`+`in`, `http` requires `scheme`, `openIdConnect` requires `openIdConnectUrl`, `oauth2` requires non-empty `flows` with required URLs and `scopes`); root objects must include at least one of `paths`, `components`, or `webhooks`; root `openapi` (or Swagger `swagger`) must be present with `info.title` and `info.version`; `openapi` versions must be 3.x when present; Example Objects must not mix `value` with `dataValue`, or `serializedValue` with `externalValue`; and Parameter/Header/Media Type Objects must not include both `example` and `examples`.
- SDK generation now supports array query serialization for `form` (default `explode=true` and explicit `explode=false` CSV), plus `spaceDelimited` and `pipeDelimited` array styles, `allowReserved` query encoding, querystring JSON primitives/arrays for `application/json`, object query serialization for `form` (explode true/false) and `deepObject` using `struct OpenAPI_KV`, object path serialization for `simple`/`label`/`matrix` (explode true/false) using `struct OpenAPI_KV`, object header serialization for `simple` (explode true/false), cookie parameters (including `explode=false` arrays, object cookies via `struct OpenAPI_KV`, and form-style percent-encoding via `allowReserved`), simple header arrays, form-urlencoded and multipart/form-data request bodies (primitive fields + arrays, including `encoding.contentType`), sequential `itemSchema` request/response bodies treated as arrays (non-streaming fallback), `text/plain` request bodies, raw `application/octet-stream` request/response bodies, apiKey security injection (header/query/cookie) and HTTP basic auth injection, inline primitive/array/object JSON request and response bodies (inline objects promoted to components), and response selection for `default` and `xXX` status ranges.
- SDK initialization uses the first `servers[]` entry (including default server
  variables) as a fallback base URL when `base_url` is `NULL` or empty, or `/`
  when no servers are defined.
- Inline primitive, array, or object response/request schemas (e.g. `type: string`, `type: array` + `items`, `type: object` with `properties`) are parsed and emitted; inline objects are promoted to component schemas for strong typing.
- Media type selection for request/response content prefers the most specific key (e.g. `text/plain` over `text/*`) and recognizes JSON types even when media type parameters are present.
- Inline schema `enum` (any JSON type)/`default` and `type` arrays (union types, including `null`) are preserved on request/response/parameter/header schemas, along with boolean schemas (`schema: true|false`).
- Inline schema `const`, deprecated `example`, JSON Schema `examples` arrays (including `items.examples`), numeric/string/array constraints (`minimum`/`maximum`/`exclusiveMinimum`/`exclusiveMaximum`, `minLength`/`maxLength`/`pattern`, `minItems`/`maxItems`/`uniqueItems`) including array item constraints, schema annotations (`description`, `deprecated`, `readOnly`, `writeOnly`), and `$ref` summary/description overrides are preserved on request/response/parameter/header schemas.
- Media Type Encoding supports `encoding`, `prefixEncoding`, and `itemEncoding`, including nested Encoding Objects.
- Parameter/Header `content` preserves full Media Type Objects (schema/encoding/examples) when round-tripping specs.
- JSON Schema constraints `minimum`/`maximum`, numeric `exclusiveMinimum`/`exclusiveMaximum`, `minLength`/`maxLength`, `pattern`, `default`, `minItems`/`maxItems`, and `uniqueItems` round-trip between OpenAPI and generated StructFields; schema annotations (`description`, `format`, `deprecated`, `readOnly`, `writeOnly`) are preserved as well.
- JSON Schema keywords `additionalProperties`, `patternProperties`, `$defs`, `not`, `if`/`then`/`else`, and other custom keys are preserved on component and property schemas during loader/writer round-trips.
- Inline schema extra keywords (including `x-` extensions, `$schema`/`$id`, `oneOf`/`anyOf`/`allOf`, `contentSchema`, and other custom keys) are preserved for parameters/headers/request/response media types, including array items `const`/`default`, alongside typed `externalDocs`, `discriminator`, and `xml` metadata.
- Spec extensions (`x-` fields) on non-schema objects are preserved (info/contact/license, tags, servers, paths, webhooks, components, operations, responses **objects**, parameters, headers, responses, request bodies (including operation-level requestBody objects), callbacks, links, security schemes, security requirement objects, externalDocs).
- Component schema `allOf`/`anyOf`/`oneOf` are preserved verbatim in `components.schemas` on write (raw override); codegen merges `allOf` and emits tagged unions for `oneOf`/`anyOf` (object refs + primitive + inline object/array variants, with inline objects promoted to components) using discriminator/property heuristics.
- Path Item `summary`/`description` and path-level `parameters` are preserved, and path-level parameters are merged into generated SDK signatures and request builders.
- Path Item `$ref` is resolved to `components.pathItems` for paths/webhooks/callbacks (summary/description overrides preserved), and path-/operation-level `servers` arrays are parsed and emitted to preserve overrides.
- `in: querystring` parameters are supported and treated as pre-serialized query strings; when present they take precedence over per-parameter query serialization.
- Server Variables (`servers[].variables`) are parsed and emitted, including `default`, `enum`, and `description`.
- Top-level `webhooks` are parsed and emitted using the same Path Item structure as `paths`; webhook keys are treated as opaque names (no path-template validation).
- OAuth2 flow definitions under `components.securitySchemes` are parsed and emitted (implicit, password, client credentials, authorization code, device authorization).
- Non-component/external schema `$ref` values (including array item `$ref`) are preserved during round-trips.
- Component schemas that are non-object/boolean/array are preserved as raw JSON and re-emitted.

### 3. C Header to JSON Schema (code2schema)

Extracts `struct` and `enum` definitions from a C header file and converts them into JSON Schema definitions.

```bash
c_cdd_cli code2schema <input_header.h> <output_schema.json>
```

---

## Data Flow Diagram

The following diagram illustrates how the `c2openapi` command processes source code into a specification.

```mermaid
%%{init: {'theme': 'base', 'fontFamily': 'Google Sans Normal'}}%%
flowchart LR
    classDef head font-family: 'Google Sans Medium', fill: #4285f4, color: #ffffff, stroke: none;
    classDef process font-family: 'Roboto Mono Normal', fill: #57caff, color: #20344b, stroke: none;
    classDef file fill: #ffd427, color: #20344b, stroke: #f9ab00, stroke-dasharray: 5 5;
    classDef final fill: #34a853, color: #ffffff, stroke: none;
    Src["Source.c / .h"]:::file
    Inspector["C Inspector"]:::head

    subgraph Parsing Step
        Tokenize["Tokenizer"]:::process
        MatchTypes["Type Matcher"]:::process
        DocParse["Doc Parser"]:::process
    end

    subgraph Mapping Step
        OpBuild["Op Builder"]:::process
        SchemaReg["Schema Registry"]:::process
        TypeMap["C -> OA Type Mapper"]:::process
    end

    Output["openapi.json"]:::final
    Src --> Inspector
    Inspector --> Tokenize
    Tokenize --> MatchTypes & DocParse
    MatchTypes --> SchemaReg
    DocParse --> OpBuild
    TypeMap --> OpBuild
    SchemaReg --> Output
    OpBuild --> Output
    linkStyle default stroke: #20344b, stroke-width: 2px;
```

## Generated Code Usage

When generating a client SDK (via `openapi2client` logic), the output includes:

1. **Transport abstraction**: `struct HttpClient` initialized with `_init()`.
2. **Helpers**: `_to_json`, `_from_json`, `_cleanup` for all models.
3. **Operations**: `api_verb_resource(ctx, params...)`.
4. **Serialization support**: form-urlencoded request bodies and matrix/label
   path parameter styles in generated clients.

### Standard Lifecycle

```c
struct HttpClient client;
struct User *u = NULL;
struct ApiError *err = NULL;

/* 0. Initialize (Selects Curl or WinHTTP automatically) */
api_init(&client, "https://api.example.com");

/* 1. Call Operation */
if (api_get_user(&client, 1, &u, &err) != 0) {
    printf("Error: %s\n", err->title);
    ApiError_cleanup(err);
} else {
    printf("User: %s\n", u->name);
    User_cleanup(u);
}

/* 2. Cleanup */
api_cleanup(&client);
```
