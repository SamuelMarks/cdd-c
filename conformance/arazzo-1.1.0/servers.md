# Arazzo 1.1.0 Conformance Table: Servers (Workflow Engine, Mock Runner & E2E Test Orchestrator)

This table tracks the completeness of language integration with Arazzo Specification 1.1.0 for server-side workflow execution engines, mock server orchestration, and automated integration test runners.

### Legend & Tracking Guide
*   **To**: Language -> Arazzo (Generating the Arazzo specification from server route tests, integration specs, or workflow DAG definitions)
*   **From**: Arazzo -> Language (Generating server-side test suites, synthetic workflow drivers, mock server orchestrators, and event dispatchers from Arazzo)
*   **Presence `[To, From]`**: The object is successfully parsed, validated, utilized, or generated.
*   **Absence `[To, From]`**: The object is currently unsupported, dropped, or falls back to generic/`any` types.
*   **Skipped `[To, From]`**: Intentionally ignored because it is irrelevant or unsupported by the server architecture.
*   **Checkboxes**: Mark `[x]` as conformance is achieved.

| Arazzo 1.1.0 Object / Feature | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes / Implementation Strategy |
| :--- | :---: | :---: | :---: | :--- |
| **Arazzo Specification Object (Root)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Server-side test harness or workflow execution engine entrypoint |
| **Arazzo Specification Object (`arazzo`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Engine capability checking for 1.1.x features (AsyncAPI, $message, querystring) |
| **Arazzo Specification Object (`$self`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Resolving relative workflow documents in multi-service test suites |
| **Arazzo Specification Object (`info`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test suite metadata and reporting summary headers |
| **Arazzo Specification Object (`sourceDescriptions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Service discovery mapping (HTTP endpoints, message brokers, Kafka/RabbitMQ) |
| **Arazzo Specification Object (`workflows`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Generated end-to-end integration test scenarios or stateful mock handlers |
| **Arazzo Specification Object (`components`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test fixture registry and shared action definitions |
| **Info Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test suite report header metadata |
| **Info Object (`title`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test suite name in JUnit/HTML test reports |
| **Info Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test suite overview summary |
| **Info Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Detailed test scenario documentation |
| **Info Object (`version`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test suite artifact version |
| **Source Description Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Target service endpoint or event broker configuration |
| **Source Description Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Service identifier mapped to environment connection strings |
| **Source Description Object (`url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Service base URL or schema specification URL |
| **Source Description Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Protocol adapter selection (`openapi` REST client, `asyncapi` broker client) |
| **Workflow Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | End-to-end integration test case or orchestrator DAG |
| **Workflow Object (`workflowId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Generated test case name (e.g. `test_user_purchase_flow`) |
| **Workflow Object (`summary`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test case summary in test results |
| **Workflow Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Extended test case documentation |
| **Workflow Object (`inputs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test seed data fixtures generated from JSON Schema 2020-12 |
| **Workflow Object (`dependsOn`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test suite topological execution ordering / dependency validation |
| **Workflow Object (`steps`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Sequential test step pipeline with state propagation |
| **Workflow Object (`successActions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Global teardown hooks or transition to subsequent integration scenario |
| **Workflow Object (`failureActions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Diagnostic dump, database snapshot, and retry harness |
| **Workflow Object (`outputs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Exported test artifacts and generated relational entity IDs |
| **Workflow Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Suite-wide environment headers (e.g. `Authorization`, `X-Tenant-ID`) |
| **Step Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Individual HTTP request dispatch or event publish/subscribe step |
| **Step Object (`description`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test step label in CI reports |
| **Step Object (`stepId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | State context key for step responses |
| **Step Object (`operationId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Server route handler invoked directly or via HTTP client |
| **Step Object (`operationPath`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | JSON Pointer to OpenAPI operation definition |
| **Step Object (`channelPath`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Message channel target on event broker |
| **Step Object (`workflowId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Sub-scenario invocation |
| **Step Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parameter mapping for request dispatch |
| **Step Object (`requestBody`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Request payload construction with database entity references |
| **Step Object (`successCriteria`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test assertions verifying HTTP status code and response payloads |
| **Step Object (`onSuccess`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Flow continuation upon assertion pass |
| **Step Object (`onFailure`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Error recovery routing or failure assertion generation |
| **Step Object (`outputs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Capturing response values into test run context |
| **Step Object (`timeout`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test step timeout / async message wait limit |
| **Step Object (`correlationId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Message correlation tracking across async event broker |
| **Step Object (`action`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Async message action (`send` to publish, `receive` to assert subscription) |
| **Step Object (`dependsOn`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Step-level prerequisite join point ensuring async broker operations complete before step execution |
| **Parameter Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Request parameter binding |
| **Parameter Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Target parameter key |
| **Parameter Object (`in`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parameter location (`path`, `query`, `querystring`, `header`, `cookie`) |
| **Parameter Object (`value`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dynamic runtime value or constant |
| **Success Action Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Branching transition handler |
| **Success Action Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Transition identifier |
| **Success Action Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | State transition (`end` or `goto`) |
| **Success Action Object (`workflowId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Next workflow invocation |
| **Success Action Object (`stepId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Jump to step |
| **Success Action Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Forwarded parameters |
| **Success Action Object (`criteria`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Guard conditions |
| **Failure Action Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Error recovery policy |
| **Failure Action Object (`name`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Policy name |
| **Failure Action Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Action type (`end`, `retry`, `goto`) |
| **Failure Action Object (`workflowId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Recovery scenario |
| **Failure Action Object (`stepId`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Fallback step |
| **Failure Action Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parameters passed to recovery |
| **Failure Action Object (`retryAfter`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test retry backoff delay |
| **Failure Action Object (`retryLimit`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Max retry attempts before failing test suite |
| **Failure Action Object (`criteria`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Error criteria filter |
| **Components Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable test fixtures and action templates |
| **Components Object (`inputs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Shared seed data schemas |
| **Components Object (`parameters`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Shared test parameters |
| **Components Object (`successActions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Shared success routines |
| **Components Object (`failureActions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Shared failure handlers |
| **Reusable Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Component reference |
| **Reusable Object (`reference`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reference pointer |
| **Reusable Object (`value`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Parameter override |
| **Criterion Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test assertion assertion condition |
| **Criterion Object (`context`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test context evaluation root |
| **Criterion Object (`condition`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Assertion expression |
| **Criterion Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Assertion dialect (`simple`, `regex`, `jsonpath`, `xpath`) |
| **Expression Type Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Selector dialect and versioning |
| **Expression Type Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dialect type |
| **Expression Type Object (`version`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dialect version |
| **Selector Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dynamic response extractor for test state |
| **Selector Object (`context`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Response payload context |
| **Selector Object (`selector`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Extraction query |
| **Selector Object (`type`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Extraction query type |
| **Request Body Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test payload factory |
| **Request Body Object (`contentType`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Content-Type header |
| **Request Body Object (`payload`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Seed payload data |
| **Request Body Object (`replacements`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dynamic field replacement |
| **Payload Replacement Object** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | In-place payload modification |
| **Payload Replacement Object (`target`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Target property path |
| **Payload Replacement Object (`targetSelectorType`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Target property dialect |
| **Payload Replacement Object (`value`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Dynamic replacement value |
| **Specification Extensions (`^x-`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Custom server test annotations and harness extensions |
| **Runtime Expressions (`$url`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Evaluates to dispatched request URL |
| **Runtime Expressions (`$method`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Evaluates to HTTP method |
| **Runtime Expressions (`$statusCode`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Evaluates to response status code for assertions |
| **Runtime Expressions (`$request.header`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Outgoing request header inspection |
| **Runtime Expressions (`$request.query`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Outgoing query string inspection |
| **Runtime Expressions (`$request.path`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Outgoing path parameter inspection |
| **Runtime Expressions (`$request.body`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Outgoing request body inspection |
| **Runtime Expressions (`$response.header`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Response header assertion evaluation |
| **Runtime Expressions (`$response.body`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Response payload assertion evaluation |
| **Runtime Expressions (`$message.header`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Async message header assertion evaluation |
| **Runtime Expressions (`$message.payload`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Async message payload assertion evaluation |
| **Runtime Expressions (`$inputs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Test suite input fixtures |
| **Runtime Expressions (`$outputs`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Exported test artifacts |
| **Runtime Expressions (`$steps`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Cross-step state passing |
| **Runtime Expressions (`$workflows`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Cross-workflow state passing |
| **Runtime Expressions (`$sourceDescriptions`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Service discovery resolution |
| **Runtime Expressions (`$components`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Reusable component lookup |
| **Runtime Expressions (`$self`)** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Self document identity resolution |
