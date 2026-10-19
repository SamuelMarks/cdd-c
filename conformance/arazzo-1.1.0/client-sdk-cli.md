# Arazzo 1.1.0 Conformance Table: Client SDK CLI (CLI Workflow Runner & Tooling)

This table tracks the completeness of language integration with Arazzo Specification 1.1.0 for generating and executing Command-Line Interface (CLI) workflow runners.

### Legend & Tracking Guide
*   **To**: Language -> Arazzo (Generating the Arazzo specification from CLI command structures or workflow manifests)
*   **From**: Arazzo -> Language (Generating CLI subcommands, input flag parsers, assertion evaluators, and terminal reporters from Arazzo)
*   **Presence `[To, From]`**: The object is successfully parsed, validated, utilized, or generated.
*   **Absence `[To, From]`**: The object is currently unsupported, dropped, or falls back to generic/`any` types.
*   **Skipped `[To, From]`**: Intentionally ignored because it is irrelevant or unsupported by the CLI architecture.
*   **Checkboxes**: Mark `[x]` as conformance is achieved.

| Arazzo 1.1.0 Object / Feature | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes / Implementation Strategy |
| :--- | :---: | :---: | :---: | :--- |
| **Arazzo Specification Object (Root)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Top-level CLI workflow root command group (e.g. `cli workflow ...`) |
| **Arazzo Specification Object (`arazzo`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Validates specification version compatibility before execution |
| **Arazzo Specification Object (`$self`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Base URI resolver for remote workflow manifests and imports |
| **Arazzo Specification Object (`info`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | CLI root `--help` description and `--version` display |
| **Arazzo Specification Object (`sourceDescriptions`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Resolving target API specs or server connections for the CLI session |
| **Arazzo Specification Object (`workflows`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | List of runnable workflow subcommands |
| **Arazzo Specification Object (`components`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Shared parameter flags and action definitions across CLI commands |
| **Info Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Workflow suite documentation block |
| **Info Object (`title`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Displayed in CLI help header |
| **Info Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Short summary for command list description |
| **Info Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Formatted `--help` man page text |
| **Info Object (`version`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Displayed via `cli workflow --version` |
| **Source Description Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | CLI target API specification loader |
| **Source Description Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Named target environment / API configuration key |
| **Source Description Object (`url`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Local file path or HTTP URL to API definition |
| **Source Description Object (`type`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Parser selection (`openapi`, `asyncapi`, `arazzo`) |
| **Workflow Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Direct CLI command (e.g. `cli workflow run <id>`) |
| **Workflow Object (`workflowId`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | CLI subcommand name / execution positional argument |
| **Workflow Object (`summary`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Subcommand summary in CLI help output |
| **Workflow Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Extended command description in `--help` |
| **Workflow Object (`inputs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Generated CLI flags (e.g. `--username <val>`, `--input-file <json>`) |
| **Workflow Object (`dependsOn`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Automatic execution of prerequisite workflows or DAG validation |
| **Workflow Object (`steps`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Step-by-step progress spinner and execution logger |
| **Workflow Object (`successActions`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Terminal notifications and next-step execution hooks |
| **Workflow Object (`failureActions`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | CLI error formatting, retry spinners, and non-zero exit codes |
| **Workflow Object (`outputs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Formatted stdout output (`--format json\|yaml\|table`) |
| **Workflow Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Global CLI flags applied to all workflow steps |
| **Step Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Step execution lifecycle and console progress reporting |
| **Step Object (`description`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Displayed in console output as step title / spinner label |
| **Step Object (`stepId`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Step reference key for debug logs and `--step <id>` selective execution |
| **Step Object (`operationId`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Dispatched operation invoked via CLI HTTP client |
| **Step Object (`operationPath`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | JSON Pointer resolution of target operation |
| **Step Object (`channelPath`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Event stream connection to target AsyncAPI channel |
| **Step Object (`workflowId`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Nested sub-workflow execution with indented CLI progress |
| **Step Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Step parameter injection from flags and prior step outputs |
| **Step Object (`requestBody`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Payload serialization and expression substitution |
| **Step Object (`successCriteria`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Step assertions with colored pass/fail terminal output |
| **Step Object (`onSuccess`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Conditional branch execution upon step passing |
| **Step Object (`onFailure`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Retry prompt, error summary, or exit with status code |
| **Step Object (`outputs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Captured output variables printed in verbose mode (`-v`) |
| **Step Object (`timeout`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | CLI request timeout limit in milliseconds |
| **Step Object (`correlationId`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Async message correlation ID logged to console |
| **Step Object (`action`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Async action type (`send` or `receive`) indicator |
| **Step Object (`dependsOn`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Step-level prerequisite join point coordinating async operations before executing step |
| **Parameter Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Command parameter mapping |
| **Parameter Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Parameter flag name |
| **Parameter Object (`in`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Target location (`path`, `query`, `querystring`, `header`, `cookie`) |
| **Parameter Object (`value`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Expression evaluation or literal CLI value |
| **Success Action Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Success flow transition handler |
| **Success Action Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Action name printed in verbose execution logs |
| **Success Action Object (`type`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Flow action (`end` cleanly or `goto` target step) |
| **Success Action Object (`workflowId`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Chained workflow invocation |
| **Success Action Object (`stepId`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Jump to step within current workflow |
| **Success Action Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Passed parameters for chained workflow |
| **Success Action Object (`criteria`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Guard conditions for executing action |
| **Failure Action Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Failure recovery and retry handler |
| **Failure Action Object (`name`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Failure action identifier |
| **Failure Action Object (`type`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Failure action (`end` with error, `retry`, or `goto`) |
| **Failure Action Object (`workflowId`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Chained failure workflow |
| **Failure Action Object (`stepId`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Fallback step |
| **Failure Action Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Failure parameters |
| **Failure Action Object (`retryAfter`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Delay with countdown spinner before retry |
| **Failure Action Object (`retryLimit`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Maximum retry attempts displayed in CLI |
| **Failure Action Object (`criteria`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Specific error conditions triggering this action |
| **Components Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Reusable workflow definitions registry |
| **Components Object (`inputs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Reusable CLI flag definitions |
| **Components Object (`parameters`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Reusable parameter templates |
| **Components Object (`successActions`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Shared success actions |
| **Components Object (`failureActions`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Shared failure actions |
| **Reusable Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Reusable object reference resolver |
| **Reusable Object (`reference`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Target component reference expression |
| **Reusable Object (`value`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Value override for referenced parameter |
| **Criterion Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Step assertion engine |
| **Criterion Object (`context`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Source data context evaluated from runtime state |
| **Criterion Object (`condition`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Expression evaluated against context |
| **Criterion Object (`type`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Engine type (`simple`, `regex`, `jsonpath`, `xpath`) |
| **Expression Type Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Query dialect specification |
| **Expression Type Object (`type`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Dialect (`jsonpath`, `xpath`, `jsonpointer`) |
| **Expression Type Object (`version`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Dialect version specification |
| **Selector Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Data extraction query for CLI outputs |
| **Selector Object (`context`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Target payload context |
| **Selector Object (`selector`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Query string |
| **Selector Object (`type`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Selector dialect |
| **Request Body Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Request payload construction |
| **Request Body Object (`contentType`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Explicit Content-Type header |
| **Request Body Object (`payload`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Payload template (or loaded via `--data-file`) |
| **Request Body Object (`replacements`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | In-place payload modification |
| **Payload Replacement Object** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Payload replacement rule |
| **Payload Replacement Object (`target`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Target property path |
| **Payload Replacement Object (`targetSelectorType`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Target selector dialect |
| **Payload Replacement Object (`value`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Injected replacement value |
| **Specification Extensions (`^x-`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Custom CLI flags and extension hooks |
| **Runtime Expressions (`$url`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Evaluates to target request URL |
| **Runtime Expressions (`$method`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Evaluates to HTTP method |
| **Runtime Expressions (`$statusCode`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Evaluates to response status code |
| **Runtime Expressions (`$request.header`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Outgoing header evaluation |
| **Runtime Expressions (`$request.query`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Outgoing query evaluation |
| **Runtime Expressions (`$request.path`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Outgoing path parameter evaluation |
| **Runtime Expressions (`$request.body`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Outgoing body evaluation |
| **Runtime Expressions (`$response.header`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Incoming response header evaluation |
| **Runtime Expressions (`$response.body`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Incoming response payload evaluation |
| **Runtime Expressions (`$message.header`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Async message header evaluation |
| **Runtime Expressions (`$message.payload`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Async message payload evaluation |
| **Runtime Expressions (`$inputs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | CLI argument / flag value resolution |
| **Runtime Expressions (`$outputs`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Workflow output formatting |
| **Runtime Expressions (`$steps`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Prior step output variable resolution |
| **Runtime Expressions (`$workflows`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | External workflow output resolution |
| **Runtime Expressions (`$sourceDescriptions`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Source URL / host resolution |
| **Runtime Expressions (`$components`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Reusable component dereferencing |
| **Runtime Expressions (`$self`)** | `[x]` , `[x]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Document URI resolution |
