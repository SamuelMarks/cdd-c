# Model Context Protocol (MCP) Server Generator Conformance Table

This table tracks the completeness of language Server generator integration with the Model Context Protocol (MCP). It is divided into three sections:
1. **Architectural Integration Layers**: Tracks the exposure of MCP across the CLI, SDK, and Server boundaries.
2. **Semantic & Conceptual Features**: Tracks protocol mechanics, transports, and behavioral requirements.
3. **Schema & Object Conformance**: An exhaustive property-by-property map derived directly from the official MCP JSON Schema (2024-11-05).

### Legend & Tracking Guide
*   **To**: Language -> MCP (Generating MCP Server payloads and handling requests from strongly typed code)
*   **From**: MCP -> Language (Generating MCP Client code, parsing responses, and invoking remote methods)
*   **Presence `[To, From]`**: The object/feature is successfully parsed, validated, utilized, or generated.
*   **Absence `[To, From]`**: The object/feature is currently unsupported, dropped, or falls back to generic/`any` types.
*   **Skipped `[To, From]`**: Intentionally ignored because it is irrelevant or unsupported by the Client architecture.
*   **Checkboxes**: Mark `[x]` as conformance is achieved.

## 1. Architectural Integration Layers

This section tracks how the Model Context Protocol is exposed across both the **Generated Artifacts** (the output SDKs/APIs) and the **Generator Tooling** itself (the bidirectional `cdd` compiler/engine).

### 1A. Target/Generated Artifacts
Implementing MCP across the generated output ensures maximum flexibility for the end-user's AI architectures:

*   **CLI Integration (Local Desktop via `stdio`)**: Enables local AI assistants (Claude Desktop, Cursor, Windsurf) to spawn the generated CLI as a subprocess and natively interact with the API locally.
*   **SDK Integration (Programmatic / In-Memory)**: Provides native adapters (e.g., `client.mcp.get_tools()`) so developers can seamlessly attach the generated SDK to frameworks like LangChain, LlamaIndex, or raw LLM clients without network overhead.
*   **Server Integration (Remote AI Gateway via `sse`)**: Generates an AI Gateway endpoint (e.g., `/mcp/sse`), allowing remote, multi-tenant AI agents and web clients to securely consume the API as LLM tools over HTTP.

| Generated Boundary | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes / Implementation Strategy |
| :--- | :---: | :---: | :---: | :--- |
| **CLI Integration (Local Desktop)** | | | | |
| CLI `mcp` Subcommand | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Generates a command (e.g., app mcp) to start the server |
| `stdio` Transport Bindings | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Wires stdin/stdout to the generated CLI logic |
| **SDK Integration (Programmatic)** | | | | |
| Native MCP Tool Adapter | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | E.g., client.mcp.get_tools() mapping SDK methods |
| Native MCP Resource Adapter | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Exposes internal state/docs as MCP resources |
| LLM Execution Router | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Native execution via client.mcp.execute_tool(name, args) |
| **Server Integration (Remote / SSE)** | | | | |
| SSE Endpoint Generation | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Wires MCP endpoints (e.g. /mcp/sse, /mcp/message) |
| HTTP Request/Auth Bridging | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Passes standard API auth into the MCP context |
| Dynamic API-to-Tool Proxy | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Resolves incoming tool calls to backend route handlers |

### 1B. Generator/Tooling Artifacts (Meta-MCP)
Exposing the `cdd` bidirectional code generator itself to MCP allows AI models to natively orchestrate code generation, schema manipulation, and code-to-schema extraction.

*   **Generator CLI via `stdio`**: Allows local IDEs or AI agents to directly instruct the generator to scaffold, diff, or compile code across languages (e.g., Tool: `cdd_generate(lang="python")`).
*   **Generator SDK / Core**: Exposes the AST and schema parsing engine natively to MCP, allowing AI tools to dynamically query API specs, understand types, and invoke generator internals in memory.

| Generator Boundary | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes / Implementation Strategy |
| :--- | :---: | :---: | :---: | :--- |
| **Generator CLI (`stdio`)** | | | | |
| Code Scaffold / Generate Tools | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | AI can invoke standard generator CLI commands via MCP |
| Schema Inspection Tools | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | AI can query loaded OpenAPI/AsyncAPI schemas |
| Bidirectional Sync Tools | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | AI can trigger code-to-schema extraction natively |
| **Generator SDK / Core** | | | | |
| AST / Type Query Resources | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | AI can read internal AST structures as MCP resources |
| In-Memory Generation Router | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Native bindings to run the generator core directly via MCP |

## 2. Semantic & Conceptual Features

| MCP Feature / Behavior | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes / Implementation Strategy |
| :--- | :---: | :---: | :---: | :--- |
| **Transports** | | | | |
| Standard I/O (stdio) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | stdin/stdout message passing |
| Server-Sent Events (sse) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | HTTP POST + SSE streams |
| Custom Transports | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Pluggable transport interface |
| **JSON-RPC 2.0 Mechanics** | | | | |
| Message Parsing & Serialization | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Message Parsing & Serialization) |
| Request ID Mapping/Resolution | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Resolving async responses to requests |
| Error Code Mapping (Standard) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Codes like -32600, -32603 |
| Notification Handling | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Processing fire-and-forget messages |
| **Connection Lifecycle** | | | | |
| initialize Handshake Sequence | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Capability negotiation & version matching |
| initialized Acknowledgment | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Sent by client after successful initialization |
| Graceful Disconnect / Close | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Graceful Disconnect / Close) |
| Liveness (ping) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Periodic connection checks |
| Request Cancellation (cancelled) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Thread/Task abortion mechanics |
| **Behavioral & Security** | | | | |
| Pagination Cursor Management | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Handling nextCursor fetch loops |
| Progress Tracking (progress) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Emitting/handling progress events |
| Human-in-the-loop (Sampling) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Prompting user before LLM generation |
| Human-in-the-loop (Tools) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Security approvals/denials for tool calls |
| Root Boundary Enforcement | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Preventing traversal outside allowed directories |
| URI Protocol Handling | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Resolving custom URI schemes |

## 3. Schema & Object Conformance

| Schema Definition / Property | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes |
| :--- | :---: | :---: | :---: | :--- |
| **Annotated** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Annotated) |
| Annotated (`annotations`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Annotated (`annotations`)) |
| Annotated (`annotations`) (`audience`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Annotated (`annotations`) (`audience`)) |
| Annotated (`annotations`) (`priority`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Annotated (`annotations`) (`priority`)) |
| **BlobResourceContents** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (BlobResourceContents) |
| BlobResourceContents (`blob`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (BlobResourceContents (`blob`)) |
| BlobResourceContents (`mimeType`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (BlobResourceContents (`mimeType`)) |
| BlobResourceContents (`uri`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (BlobResourceContents (`uri`)) |
| **CallToolRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CallToolRequest) |
| CallToolRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CallToolRequest (`method`)) |
| CallToolRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CallToolRequest (`params`)) |
| CallToolRequest (`params`) (`arguments`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CallToolRequest (`params`) (`arguments`)) |
| CallToolRequest (`params`) (`name`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CallToolRequest (`params`) (`name`)) |
| **CallToolResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CallToolResult) |
| CallToolResult (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CallToolResult (`_meta`)) |
| CallToolResult (`content`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CallToolResult (`content`)) |
| CallToolResult (`isError`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CallToolResult (`isError`)) |
| **CancelledNotification** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CancelledNotification) |
| CancelledNotification (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CancelledNotification (`method`)) |
| CancelledNotification (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CancelledNotification (`params`)) |
| CancelledNotification (`params`) (`reason`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CancelledNotification (`params`) (`reason`)) |
| CancelledNotification (`params`) (`requestId`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CancelledNotification (`params`) (`requestId`)) |
| **ClientCapabilities** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ClientCapabilities) |
| ClientCapabilities (`experimental`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ClientCapabilities (`experimental`)) |
| ClientCapabilities (`roots`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ClientCapabilities (`roots`)) |
| ClientCapabilities (`roots`) (`listChanged`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ClientCapabilities (`roots`) (`listChanged`)) |
| ClientCapabilities (`sampling`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ClientCapabilities (`sampling`)) |
| **ClientNotification** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| **ClientRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| **ClientResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| **CompleteRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CompleteRequest) |
| CompleteRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CompleteRequest (`method`)) |
| CompleteRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CompleteRequest (`params`)) |
| CompleteRequest (`params`) (`argument`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CompleteRequest (`params`) (`argument`)) |
| CompleteRequest (`params`) (`argument`) (`name`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CompleteRequest (`params`) (`argument`) (`name`)) |
| CompleteRequest (`params`) (`argument`) (`value`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CompleteRequest (`params`) (`argument`) (`value`)) |
| CompleteRequest (`params`) (`ref`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CompleteRequest (`params`) (`ref`)) |
| **CompleteResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CompleteResult) |
| CompleteResult (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CompleteResult (`_meta`)) |
| CompleteResult (`completion`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CompleteResult (`completion`)) |
| CompleteResult (`completion`) (`hasMore`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CompleteResult (`completion`) (`hasMore`)) |
| CompleteResult (`completion`) (`total`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CompleteResult (`completion`) (`total`)) |
| CompleteResult (`completion`) (`values`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CompleteResult (`completion`) (`values`)) |
| **CreateMessageRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageRequest) |
| CreateMessageRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageRequest (`method`)) |
| CreateMessageRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageRequest (`params`)) |
| CreateMessageRequest (`params`) (`includeContext`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageRequest (`params`) (`includeContext`)) |
| CreateMessageRequest (`params`) (`maxTokens`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageRequest (`params`) (`maxTokens`)) |
| CreateMessageRequest (`params`) (`messages`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageRequest (`params`) (`messages`)) |
| CreateMessageRequest (`params`) (`metadata`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageRequest (`params`) (`metadata`)) |
| CreateMessageRequest (`params`) (`modelPreferences`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageRequest (`params`) (`modelPreferences`)) |
| CreateMessageRequest (`params`) (`stopSequences`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageRequest (`params`) (`stopSequences`)) |
| CreateMessageRequest (`params`) (`systemPrompt`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageRequest (`params`) (`systemPrompt`)) |
| CreateMessageRequest (`params`) (`temperature`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageRequest (`params`) (`temperature`)) |
| **CreateMessageResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageResult) |
| CreateMessageResult (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageResult (`_meta`)) |
| CreateMessageResult (`content`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageResult (`content`)) |
| CreateMessageResult (`model`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageResult (`model`)) |
| CreateMessageResult (`role`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageResult (`role`)) |
| CreateMessageResult (`stopReason`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (CreateMessageResult (`stopReason`)) |
| **Cursor** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Cursor) |
| **EmbeddedResource** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (EmbeddedResource) |
| EmbeddedResource (`annotations`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (EmbeddedResource (`annotations`)) |
| EmbeddedResource (`annotations`) (`audience`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (EmbeddedResource (`annotations`) (`audience`)) |
| EmbeddedResource (`annotations`) (`priority`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (EmbeddedResource (`annotations`) (`priority`)) |
| EmbeddedResource (`resource`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (EmbeddedResource (`resource`)) |
| EmbeddedResource (`type`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (EmbeddedResource (`type`)) |
| **EmptyResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (EmptyResult) |
| **GetPromptRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (GetPromptRequest) |
| GetPromptRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (GetPromptRequest (`method`)) |
| GetPromptRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (GetPromptRequest (`params`)) |
| GetPromptRequest (`params`) (`arguments`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (GetPromptRequest (`params`) (`arguments`)) |
| GetPromptRequest (`params`) (`name`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (GetPromptRequest (`params`) (`name`)) |
| **GetPromptResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (GetPromptResult) |
| GetPromptResult (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (GetPromptResult (`_meta`)) |
| GetPromptResult (`description`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (GetPromptResult (`description`)) |
| GetPromptResult (`messages`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (GetPromptResult (`messages`)) |
| **ImageContent** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ImageContent) |
| ImageContent (`annotations`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ImageContent (`annotations`)) |
| ImageContent (`annotations`) (`audience`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ImageContent (`annotations`) (`audience`)) |
| ImageContent (`annotations`) (`priority`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ImageContent (`annotations`) (`priority`)) |
| ImageContent (`data`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ImageContent (`data`)) |
| ImageContent (`mimeType`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ImageContent (`mimeType`)) |
| ImageContent (`type`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ImageContent (`type`)) |
| **Implementation** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Implementation) |
| Implementation (`name`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Implementation (`name`)) |
| Implementation (`version`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Implementation (`version`)) |
| **InitializeRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializeRequest) |
| InitializeRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializeRequest (`method`)) |
| InitializeRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializeRequest (`params`)) |
| InitializeRequest (`params`) (`capabilities`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializeRequest (`params`) (`capabilities`)) |
| InitializeRequest (`params`) (`clientInfo`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializeRequest (`params`) (`clientInfo`)) |
| InitializeRequest (`params`) (`protocolVersion`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializeRequest (`params`) (`protocolVersion`)) |
| **InitializeResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializeResult) |
| InitializeResult (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializeResult (`_meta`)) |
| InitializeResult (`capabilities`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializeResult (`capabilities`)) |
| InitializeResult (`instructions`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializeResult (`instructions`)) |
| InitializeResult (`protocolVersion`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializeResult (`protocolVersion`)) |
| InitializeResult (`serverInfo`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializeResult (`serverInfo`)) |
| **InitializedNotification** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializedNotification) |
| InitializedNotification (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializedNotification (`method`)) |
| InitializedNotification (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializedNotification (`params`)) |
| InitializedNotification (`params`) (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (InitializedNotification (`params`) (`_meta`)) |
| **JSONRPCError** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCError) |
| JSONRPCError (`error`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCError (`error`)) |
| JSONRPCError (`error`) (`code`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCError (`error`) (`code`)) |
| JSONRPCError (`error`) (`data`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCError (`error`) (`data`)) |
| JSONRPCError (`error`) (`message`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCError (`error`) (`message`)) |
| JSONRPCError (`id`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCError (`id`)) |
| JSONRPCError (`jsonrpc`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCError (`jsonrpc`)) |
| **JSONRPCMessage** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| **JSONRPCNotification** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCNotification) |
| JSONRPCNotification (`jsonrpc`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCNotification (`jsonrpc`)) |
| JSONRPCNotification (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCNotification (`method`)) |
| JSONRPCNotification (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCNotification (`params`)) |
| JSONRPCNotification (`params`) (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCNotification (`params`) (`_meta`)) |
| **JSONRPCRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCRequest) |
| JSONRPCRequest (`id`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCRequest (`id`)) |
| JSONRPCRequest (`jsonrpc`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCRequest (`jsonrpc`)) |
| JSONRPCRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCRequest (`method`)) |
| JSONRPCRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCRequest (`params`)) |
| JSONRPCRequest (`params`) (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCRequest (`params`) (`_meta`)) |
| JSONRPCRequest (`params`) (`_meta`) (`progressToken`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCRequest (`params`) (`_meta`) (`progressToken`)) |
| **JSONRPCResponse** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCResponse) |
| JSONRPCResponse (`id`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCResponse (`id`)) |
| JSONRPCResponse (`jsonrpc`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCResponse (`jsonrpc`)) |
| JSONRPCResponse (`result`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (JSONRPCResponse (`result`)) |
| **ListPromptsRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListPromptsRequest) |
| ListPromptsRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListPromptsRequest (`method`)) |
| ListPromptsRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListPromptsRequest (`params`)) |
| ListPromptsRequest (`params`) (`cursor`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListPromptsRequest (`params`) (`cursor`)) |
| **ListPromptsResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListPromptsResult) |
| ListPromptsResult (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListPromptsResult (`_meta`)) |
| ListPromptsResult (`nextCursor`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListPromptsResult (`nextCursor`)) |
| ListPromptsResult (`prompts`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListPromptsResult (`prompts`)) |
| **ListResourceTemplatesRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourceTemplatesRequest) |
| ListResourceTemplatesRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourceTemplatesRequest (`method`)) |
| ListResourceTemplatesRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourceTemplatesRequest (`params`)) |
| ListResourceTemplatesRequest (`params`) (`cursor`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourceTemplatesRequest (`params`) (`cursor`)) |
| **ListResourceTemplatesResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourceTemplatesResult) |
| ListResourceTemplatesResult (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourceTemplatesResult (`_meta`)) |
| ListResourceTemplatesResult (`nextCursor`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourceTemplatesResult (`nextCursor`)) |
| ListResourceTemplatesResult (`resourceTemplates`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourceTemplatesResult (`resourceTemplates`)) |
| **ListResourcesRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourcesRequest) |
| ListResourcesRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourcesRequest (`method`)) |
| ListResourcesRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourcesRequest (`params`)) |
| ListResourcesRequest (`params`) (`cursor`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourcesRequest (`params`) (`cursor`)) |
| **ListResourcesResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourcesResult) |
| ListResourcesResult (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourcesResult (`_meta`)) |
| ListResourcesResult (`nextCursor`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourcesResult (`nextCursor`)) |
| ListResourcesResult (`resources`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListResourcesResult (`resources`)) |
| **ListRootsRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListRootsRequest) |
| ListRootsRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListRootsRequest (`method`)) |
| ListRootsRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListRootsRequest (`params`)) |
| ListRootsRequest (`params`) (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListRootsRequest (`params`) (`_meta`)) |
| ListRootsRequest (`params`) (`_meta`) (`progressToken`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListRootsRequest (`params`) (`_meta`) (`progressToken`)) |
| **ListRootsResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListRootsResult) |
| ListRootsResult (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListRootsResult (`_meta`)) |
| ListRootsResult (`roots`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListRootsResult (`roots`)) |
| **ListToolsRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListToolsRequest) |
| ListToolsRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListToolsRequest (`method`)) |
| ListToolsRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListToolsRequest (`params`)) |
| ListToolsRequest (`params`) (`cursor`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListToolsRequest (`params`) (`cursor`)) |
| **ListToolsResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListToolsResult) |
| ListToolsResult (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListToolsResult (`_meta`)) |
| ListToolsResult (`nextCursor`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListToolsResult (`nextCursor`)) |
| ListToolsResult (`tools`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ListToolsResult (`tools`)) |
| **LoggingLevel** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (LoggingLevel) |
| **LoggingMessageNotification** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (LoggingMessageNotification) |
| LoggingMessageNotification (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (LoggingMessageNotification (`method`)) |
| LoggingMessageNotification (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (LoggingMessageNotification (`params`)) |
| LoggingMessageNotification (`params`) (`data`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (LoggingMessageNotification (`params`) (`data`)) |
| LoggingMessageNotification (`params`) (`level`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (LoggingMessageNotification (`params`) (`level`)) |
| LoggingMessageNotification (`params`) (`logger`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (LoggingMessageNotification (`params`) (`logger`)) |
| **ModelHint** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ModelHint) |
| ModelHint (`name`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ModelHint (`name`)) |
| **ModelPreferences** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ModelPreferences) |
| ModelPreferences (`costPriority`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ModelPreferences (`costPriority`)) |
| ModelPreferences (`hints`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ModelPreferences (`hints`)) |
| ModelPreferences (`intelligencePriority`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ModelPreferences (`intelligencePriority`)) |
| ModelPreferences (`speedPriority`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ModelPreferences (`speedPriority`)) |
| **Notification** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| Notification (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| Notification (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| Notification (`params`) (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| **PaginatedRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PaginatedRequest) |
| PaginatedRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PaginatedRequest (`method`)) |
| PaginatedRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PaginatedRequest (`params`)) |
| PaginatedRequest (`params`) (`cursor`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PaginatedRequest (`params`) (`cursor`)) |
| **PaginatedResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PaginatedResult) |
| PaginatedResult (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PaginatedResult (`_meta`)) |
| PaginatedResult (`nextCursor`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PaginatedResult (`nextCursor`)) |
| **PingRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PingRequest) |
| PingRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PingRequest (`method`)) |
| PingRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PingRequest (`params`)) |
| PingRequest (`params`) (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PingRequest (`params`) (`_meta`)) |
| PingRequest (`params`) (`_meta`) (`progressToken`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PingRequest (`params`) (`_meta`) (`progressToken`)) |
| **ProgressNotification** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ProgressNotification) |
| ProgressNotification (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ProgressNotification (`method`)) |
| ProgressNotification (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ProgressNotification (`params`)) |
| ProgressNotification (`params`) (`progressToken`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ProgressNotification (`params`) (`progressToken`)) |
| ProgressNotification (`params`) (`progress`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ProgressNotification (`params`) (`progress`)) |
| ProgressNotification (`params`) (`total`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ProgressNotification (`params`) (`total`)) |
| **ProgressToken** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ProgressToken) |
| **Prompt** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Prompt) |
| Prompt (`arguments`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Prompt (`arguments`)) |
| Prompt (`description`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Prompt (`description`)) |
| Prompt (`name`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Prompt (`name`)) |
| **PromptArgument** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptArgument) |
| PromptArgument (`description`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptArgument (`description`)) |
| PromptArgument (`name`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptArgument (`name`)) |
| PromptArgument (`required`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptArgument (`required`)) |
| **PromptListChangedNotification** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptListChangedNotification) |
| PromptListChangedNotification (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptListChangedNotification (`method`)) |
| PromptListChangedNotification (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptListChangedNotification (`params`)) |
| PromptListChangedNotification (`params`) (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptListChangedNotification (`params`) (`_meta`)) |
| **PromptMessage** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptMessage) |
| PromptMessage (`content`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptMessage (`content`)) |
| PromptMessage (`role`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptMessage (`role`)) |
| **PromptReference** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptReference) |
| PromptReference (`name`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptReference (`name`)) |
| PromptReference (`type`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (PromptReference (`type`)) |
| **ReadResourceRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ReadResourceRequest) |
| ReadResourceRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ReadResourceRequest (`method`)) |
| ReadResourceRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ReadResourceRequest (`params`)) |
| ReadResourceRequest (`params`) (`uri`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ReadResourceRequest (`params`) (`uri`)) |
| **ReadResourceResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ReadResourceResult) |
| ReadResourceResult (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ReadResourceResult (`_meta`)) |
| ReadResourceResult (`contents`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ReadResourceResult (`contents`)) |
| **Request** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| Request (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| Request (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| Request (`params`) (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| Request (`params`) (`_meta`) (`progressToken`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| **RequestId** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (RequestId) |
| **Resource** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Resource) |
| Resource (`annotations`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Resource (`annotations`)) |
| Resource (`annotations`) (`audience`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Resource (`annotations`) (`audience`)) |
| Resource (`annotations`) (`priority`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Resource (`annotations`) (`priority`)) |
| Resource (`description`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Resource (`description`)) |
| Resource (`mimeType`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Resource (`mimeType`)) |
| Resource (`name`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Resource (`name`)) |
| Resource (`size`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Resource (`size`)) |
| Resource (`uri`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Resource (`uri`)) |
| **ResourceContents** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceContents) |
| ResourceContents (`mimeType`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceContents (`mimeType`)) |
| ResourceContents (`uri`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceContents (`uri`)) |
| **ResourceListChangedNotification** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceListChangedNotification) |
| ResourceListChangedNotification (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceListChangedNotification (`method`)) |
| ResourceListChangedNotification (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceListChangedNotification (`params`)) |
| ResourceListChangedNotification (`params`) (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceListChangedNotification (`params`) (`_meta`)) |
| **ResourceReference** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceReference) |
| ResourceReference (`type`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceReference (`type`)) |
| ResourceReference (`uri`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceReference (`uri`)) |
| **ResourceTemplate** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceTemplate) |
| ResourceTemplate (`annotations`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceTemplate (`annotations`)) |
| ResourceTemplate (`annotations`) (`audience`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceTemplate (`annotations`) (`audience`)) |
| ResourceTemplate (`annotations`) (`priority`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceTemplate (`annotations`) (`priority`)) |
| ResourceTemplate (`description`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceTemplate (`description`)) |
| ResourceTemplate (`mimeType`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceTemplate (`mimeType`)) |
| ResourceTemplate (`name`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceTemplate (`name`)) |
| ResourceTemplate (`uriTemplate`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceTemplate (`uriTemplate`)) |
| **ResourceUpdatedNotification** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceUpdatedNotification) |
| ResourceUpdatedNotification (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceUpdatedNotification (`method`)) |
| ResourceUpdatedNotification (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceUpdatedNotification (`params`)) |
| ResourceUpdatedNotification (`params`) (`uri`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ResourceUpdatedNotification (`params`) (`uri`)) |
| **Result** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Result) |
| Result (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Result (`_meta`)) |
| **Role** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Role) |
| **Root** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Root) |
| Root (`name`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Root (`name`)) |
| Root (`uri`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Root (`uri`)) |
| **RootsListChangedNotification** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (RootsListChangedNotification) |
| RootsListChangedNotification (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (RootsListChangedNotification (`method`)) |
| RootsListChangedNotification (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (RootsListChangedNotification (`params`)) |
| RootsListChangedNotification (`params`) (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (RootsListChangedNotification (`params`) (`_meta`)) |
| **SamplingMessage** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (SamplingMessage) |
| SamplingMessage (`content`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (SamplingMessage (`content`)) |
| SamplingMessage (`role`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (SamplingMessage (`role`)) |
| **ServerCapabilities** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ServerCapabilities) |
| ServerCapabilities (`experimental`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ServerCapabilities (`experimental`)) |
| ServerCapabilities (`logging`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ServerCapabilities (`logging`)) |
| ServerCapabilities (`prompts`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ServerCapabilities (`prompts`)) |
| ServerCapabilities (`prompts`) (`listChanged`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ServerCapabilities (`prompts`) (`listChanged`)) |
| ServerCapabilities (`resources`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ServerCapabilities (`resources`)) |
| ServerCapabilities (`resources`) (`listChanged`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ServerCapabilities (`resources`) (`listChanged`)) |
| ServerCapabilities (`resources`) (`subscribe`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ServerCapabilities (`resources`) (`subscribe`)) |
| ServerCapabilities (`tools`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ServerCapabilities (`tools`)) |
| ServerCapabilities (`tools`) (`listChanged`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ServerCapabilities (`tools`) (`listChanged`)) |
| **ServerNotification** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| **ServerRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| **ServerResult** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Not applicable to this profile |
| **SetLevelRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (SetLevelRequest) |
| SetLevelRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (SetLevelRequest (`method`)) |
| SetLevelRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (SetLevelRequest (`params`)) |
| SetLevelRequest (`params`) (`level`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (SetLevelRequest (`params`) (`level`)) |
| **SubscribeRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (SubscribeRequest) |
| SubscribeRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (SubscribeRequest (`method`)) |
| SubscribeRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (SubscribeRequest (`params`)) |
| SubscribeRequest (`params`) (`uri`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (SubscribeRequest (`params`) (`uri`)) |
| **TextContent** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (TextContent) |
| TextContent (`annotations`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (TextContent (`annotations`)) |
| TextContent (`annotations`) (`audience`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (TextContent (`annotations`) (`audience`)) |
| TextContent (`annotations`) (`priority`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (TextContent (`annotations`) (`priority`)) |
| TextContent (`text`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (TextContent (`text`)) |
| TextContent (`type`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (TextContent (`type`)) |
| **TextResourceContents** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (TextResourceContents) |
| TextResourceContents (`mimeType`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (TextResourceContents (`mimeType`)) |
| TextResourceContents (`text`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (TextResourceContents (`text`)) |
| TextResourceContents (`uri`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (TextResourceContents (`uri`)) |
| **Tool** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Tool) |
| Tool (`description`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Tool (`description`)) |
| Tool (`inputSchema`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Tool (`inputSchema`)) |
| Tool (`inputSchema`) (`properties`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Tool (`inputSchema`) (`properties`)) |
| Tool (`inputSchema`) (`required`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Tool (`inputSchema`) (`required`)) |
| Tool (`inputSchema`) (`type`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Tool (`inputSchema`) (`type`)) |
| Tool (`name`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (Tool (`name`)) |
| **ToolListChangedNotification** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ToolListChangedNotification) |
| ToolListChangedNotification (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ToolListChangedNotification (`method`)) |
| ToolListChangedNotification (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ToolListChangedNotification (`params`)) |
| ToolListChangedNotification (`params`) (`_meta`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (ToolListChangedNotification (`params`) (`_meta`)) |
| **UnsubscribeRequest** | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (UnsubscribeRequest) |
| UnsubscribeRequest (`method`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (UnsubscribeRequest (`method`)) |
| UnsubscribeRequest (`params`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (UnsubscribeRequest (`params`)) |
| UnsubscribeRequest (`params`) (`uri`) | `[x]` , `[x]` | `[x]` , `[x]` | `[x]` , `[x]` | Implemented in `src/routes/emit/serve_json_rpc.c` and `src/routes/parse/cli.c` (UnsubscribeRequest (`params`) (`uri`)) |
