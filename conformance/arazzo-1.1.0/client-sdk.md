# Arazzo 1.1.0 Conformance Table: Client SDK (Workflow Orchestration & Chained Methods)

This table tracks the completeness of language integration with Arazzo Specification 1.1.0 for generating and extracting Client SDK workflow orchestrators.

### Legend & Tracking Guide
*   **To**: Language -> Arazzo (Generating the Arazzo specification from strongly typed SDK workflow definitions or code)
*   **From**: Arazzo -> Language (Generating strongly typed workflow classes, chained SDK methods, assertions, and state machines from Arazzo)
*   **Presence `[To, From]`**: The object is successfully parsed, validated, utilized, or generated.
*   **Absence `[To, From]`**: The object is currently unsupported, dropped, or falls back to generic/`any` types.
*   **Skipped `[To, From]`**: Intentionally ignored because it is irrelevant or unsupported by the Client architecture.
*   **Checkboxes**: Mark `[x]` as conformance is achieved.

| Arazzo 1.1.0 Object / Feature | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes / Implementation Strategy |
| :--- | :---: | :---: | :---: | :--- |
| **Arazzo Specification Object (Root)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Root workflow orchestrator class or module wrapper |
| **Arazzo Specification Object (`arazzo`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Semantic version validation (ensuring 1.1.x compatibility) |
| **Arazzo Specification Object (`$self`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Canonical base URI resolution for relative and remote references |
| **Arazzo Specification Object (`info`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Client SDK workflow suite metadata, module docstrings |
| **Arazzo Specification Object (`sourceDescriptions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Binding underlying API clients (OpenAPI, AsyncAPI, external Arazzo) |
| **Arazzo Specification Object (`workflows`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Array of executable workflow methods or classes |
| **Arazzo Specification Object (`components`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Shared parameters, inputs, and action handler registry |
| **Info Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Workflow package documentation and metadata header |
| **Info Object (`title`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | SDK workflow suite title |
| **Info Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Short module summary docstring |
| **Info Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Extended Markdown documentation rendered into SDK docs |
| **Info Object (`version`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Workflow bundle version string |
| **Source Description Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | API client dependency and source spec linkage |
| **Source Description Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Unique source identifier mapped to client instance property |
| **Source Description Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Source location URI / relative spec file resolver |
| **Source Description Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Client protocol type (`openapi`, `asyncapi`, `arazzo`) |
| **Workflow Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Executable workflow function / orchestrator method |
| **Workflow Object (`workflowId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Generated method name (e.g., `client.workflows.userPurchase(...)`) |
| **Workflow Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Method docstring summary |
| **Workflow Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Detailed method Javadoc/docstring description |
| **Workflow Object (`inputs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Strongly typed method input arguments / DTO via JSON Schema 2020-12 |
| **Workflow Object (`dependsOn`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Prerequisite workflow invocation check / DAG dependency ordering |
| **Workflow Object (`steps`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Sequential or DAG execution pipeline of API calls |
| **Workflow Object (`successActions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Workflow-level success branch handlers / event hooks |
| **Workflow Object (`failureActions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Workflow-level error handling, fallback routing, and retries |
| **Workflow Object (`outputs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Strongly typed return type definition and output value mapping |
| **Workflow Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Common parameter defaults injected across all steps |
| **Step Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Individual operation call or sub-workflow invocation step |
| **Step Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Step execution comment or logging label |
| **Step Object (`stepId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Unique step identifier used for referencing intermediate outputs |
| **Step Object (`operationId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Direct invocation of target SDK client method by operationId |
| **Step Object (`operationPath`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Invocation via JSON pointer path to OpenAPI operation |
| **Step Object (`channelPath`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Invocation via AsyncAPI channel path for event streaming |
| **Step Object (`workflowId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Composition via invoking sibling or external workflow |
| **Step Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parameter mapping (path, query, header, cookie) for this step |
| **Step Object (`requestBody`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Serialization of request payload with dynamic expression replacements |
| **Step Object (`successCriteria`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Step validation assertions (status code, payload conditions) |
| **Step Object (`onSuccess`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Step-specific transition routing upon assertion success |
| **Step Object (`onFailure`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Step-specific recovery, retry policies, or error branching |
| **Step Object (`outputs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Extraction of step response values into workflow state context |
| **Step Object (`timeout`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Per-step HTTP/async client timeout override in milliseconds |
| **Step Object (`correlationId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | AsyncAPI message correlation tracking header or token |
| **Step Object (`action`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Async message flow intent (`send` or `receive`) |
| **Step Object (`dependsOn`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Step-level prerequisite join point for asynchronous or out-of-order steps |
| **Parameter Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Operation argument binding |
| **Parameter Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Target parameter name |
| **Parameter Object (`in`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parameter location (`path`, `query`, `querystring`, `header`, `cookie`) |
| **Parameter Object (`value`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Runtime expression resolution, constant value, or selector |
| **Success Action Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Positive branch state transition handler |
| **Success Action Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Action name identifier |
| **Success Action Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Action flow primitive (`end` to terminate or `goto` to branch) |
| **Success Action Object (`workflowId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Target workflow to chain upon success |
| **Success Action Object (`stepId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Target step within current workflow to jump to |
| **Success Action Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Input parameters forwarded to chained target workflow |
| **Success Action Object (`criteria`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Guard conditions required before triggering this action |
| **Failure Action Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Error recovery and retry strategy handler |
| **Failure Action Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Failure action identifier |
| **Failure Action Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Action flow primitive (`end`, `retry`, or `goto`) |
| **Failure Action Object (`workflowId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Chained recovery workflow identifier |
| **Failure Action Object (`stepId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Fallback step identifier |
| **Failure Action Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parameters passed to recovery workflow |
| **Failure Action Object (`retryAfter`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Delay before retry attempt (honoring HTTP Retry-After if present) |
| **Failure Action Object (`retryLimit`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Maximum retry count before propagating step failure |
| **Failure Action Object (`criteria`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Condition guards matching specific failure modes |
| **Components Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Container for reusable workflow definitions |
| **Components Object (`inputs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable JSON Schema DTO definitions |
| **Components Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable parameter templates |
| **Components Object (`successActions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable success handlers |
| **Components Object (`failureActions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable failure and retry handlers |
| **Reusable Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reference pointer to components registry |
| **Reusable Object (`reference`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Pointer expression (e.g. `$components.parameters.page`) |
| **Reusable Object (`value`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Context-specific parameter value override |
| **Criterion Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Assertion evaluator for success criteria and action guards |
| **Criterion Object (`context`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Evaluation root context (e.g. `$response.body`, `$statusCode`) |
| **Criterion Object (`condition`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Condition expression (simple comparison, regex, JSONPath, XPath) |
| **Criterion Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Condition engine type (`simple`, `regex`, `jsonpath`, `xpath`) |
| **Expression Type Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Explicit expression dialect and version specification |
| **Expression Type Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Selector dialect (`jsonpath`, `xpath`, `jsonpointer`) |
| **Expression Type Object (`version`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Specific standard version (e.g. `rfc9535`, `xpath-31`, `rfc6901`) |
| **Selector Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Structured data extractor for dynamic output variables |
| **Selector Object (`context`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Source data context expression (e.g. `$response.body`) |
| **Selector Object (`selector`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Query expression string (e.g. `$.items[0].id`) |
| **Selector Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Query dialect (`jsonpath`, `xpath`, `jsonpointer`) |
| **Request Body Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dynamic request body payload constructor |
| **Request Body Object (`contentType`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Explicit request Content-Type header |
| **Request Body Object (`payload`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Base payload template (raw JSON, YAML, or object) |
| **Request Body Object (`replacements`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Targeted in-place payload replacements prior to transmission |
| **Payload Replacement Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Field-level injection into payload |
| **Payload Replacement Object (`target`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Target location pointer (JSON Pointer, JSONPath, XPath) |
| **Payload Replacement Object (`targetSelectorType`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Explicit replacement dialect specification |
| **Payload Replacement Object (`value`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Value expression to inject at target location |
| **Specification Extensions (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Vendor extension passthrough and metadata hooks |
| **Runtime Expressions (`$url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Resolved request/operation URL |
| **Runtime Expressions (`$method`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Operation HTTP verb |
| **Runtime Expressions (`$statusCode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Response HTTP status code integer |
| **Runtime Expressions (`$request.header`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Outgoing request header values |
| **Runtime Expressions (`$request.query`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Outgoing query parameter values |
| **Runtime Expressions (`$request.path`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Outgoing path parameter values |
| **Runtime Expressions (`$request.body`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Outgoing request body evaluation |
| **Runtime Expressions (`$response.header`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Incoming response headers |
| **Runtime Expressions (`$response.body`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Incoming response body evaluation |
| **Runtime Expressions (`$message.header`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | AsyncAPI message header evaluation |
| **Runtime Expressions (`$message.payload`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | AsyncAPI message payload evaluation |
| **Runtime Expressions (`$inputs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Workflow input parameter references |
| **Runtime Expressions (`$outputs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Workflow output parameter references |
| **Runtime Expressions (`$steps`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Downstream step access to previous step outputs (`$steps.<id>.outputs.<k>`) |
| **Runtime Expressions (`$workflows`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Cross-workflow output references |
| **Runtime Expressions (`$sourceDescriptions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Resolving source description endpoints and metadata |
| **Runtime Expressions (`$components`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable component references |
| **Runtime Expressions (`$self`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Self-referential document URI |
