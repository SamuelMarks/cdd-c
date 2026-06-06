# Model Context Protocol (MCP) CLI Conformance Table

This table tracks the completeness of language CLI integration with the Model Context Protocol (MCP). It is divided into three sections:
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
| CLI `mcp` Subcommand | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Generates a command (e.g., `app mcp`) to start the server |
| `stdio` Transport Bindings | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Wires stdin/stdout to the generated CLI logic |
| **SDK Integration (Programmatic)** | | | | |
| Native MCP Tool Adapter | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | E.g., `client.mcp.get_tools()` mapping SDK methods |
| Native MCP Resource Adapter | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Exposes internal state/docs as MCP resources |
| LLM Execution Router | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Native execution via `client.mcp.execute_tool(name, args)` |
| **Server Integration (Remote / SSE)** | | | | |
| SSE Endpoint Generation | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Wires MCP endpoints (e.g. `/mcp/sse`, `/mcp/message`) |
| HTTP Request/Auth Bridging | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Passes standard API auth into the MCP context |
| Dynamic API-to-Tool Proxy | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Resolves incoming tool calls to backend route handlers |

### 1B. Generator/Tooling Artifacts (Meta-MCP)
Exposing the `cdd` bidirectional code generator itself to MCP allows AI models to natively orchestrate code generation, schema manipulation, and code-to-schema extraction.

*   **Generator CLI via `stdio`**: Allows local IDEs or AI agents to directly instruct the generator to scaffold, diff, or compile code across languages (e.g., Tool: `cdd_generate(lang="python")`).
*   **Generator SDK / Core**: Exposes the AST and schema parsing engine natively to MCP, allowing AI tools to dynamically query API specs, understand types, and invoke generator internals in memory.

| Generator Boundary | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes / Implementation Strategy |
| :--- | :---: | :---: | :---: | :--- |
| **Generator CLI (`stdio`)** | | | | |
| Code Scaffold / Generate Tools | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | AI can invoke standard generator CLI commands via MCP |
| Schema Inspection Tools | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | AI can query loaded OpenAPI/AsyncAPI schemas |
| Bidirectional Sync Tools | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | AI can trigger code-to-schema extraction natively |
| **Generator SDK / Core** | | | | |
| AST / Type Query Resources | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | AI can read internal AST structures as MCP resources |
| In-Memory Generation Router | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Native bindings to run the generator core directly via MCP |

## 2. Semantic & Conceptual Features

| MCP Feature / Behavior | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes / Implementation Strategy |
| :--- | :---: | :---: | :---: | :--- |
| **Transports** | | | | |
| Standard I/O (stdio) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | stdin/stdout message passing |
| Server-Sent Events (sse) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | HTTP POST + SSE streams |
| Custom Transports | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | Pluggable transport interface |
| **JSON-RPC 2.0 Mechanics** | | | | |
| Message Parsing & Serialization | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | |
| Request ID Mapping/Resolution | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Resolving async responses to requests |
| Error Code Mapping (Standard) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Codes like -32600, -32603 |
| Notification Handling | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Processing fire-and-forget messages |
| **Connection Lifecycle** | | | | |
| initialize Handshake Sequence | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Capability negotiation & version matching |
| initialized Acknowledgment | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Sent by client after successful initialization |
| Graceful Disconnect / Close | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| Liveness (ping) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Periodic connection checks |
| Request Cancellation (cancelled)| `[x]` , `[ ]` | `[ ]` , `[ ]` | `[x]` , `[ ]` | Thread/Task abortion mechanics |
| **Behavioral & Security** | | | | |
| Pagination Cursor Management | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Handling nextCursor fetch loops |
| Progress Tracking (progress) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | Emitting/handling progress events |
| Human-in-the-loop (Sampling) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Prompting user before LLM generation |
| Human-in-the-loop (Tools) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Security approvals/denials for tool calls |
| Root Boundary Enforcement | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Preventing traversal outside allowed directories |
| URI Protocol Handling | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | Resolving custom URI schemes |

## 3. Schema & Object Conformance

| Schema Definition / Property | Presence `[To, From]` | Absence `[To, From]` | Skipped `[To, From]` | Notes |
| :--- | :---: | :---: | :---: | :--- |
| **Annotated** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Annotated (`annotations`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Annotated (`annotations`) (`audience`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Annotated (`annotations`) (`priority`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **BlobResourceContents** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| BlobResourceContents (`blob`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| BlobResourceContents (`mimeType`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| BlobResourceContents (`uri`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **CallToolRequest** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| CallToolRequest (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| CallToolRequest (`params`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| CallToolRequest (`params`) (`arguments`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| CallToolRequest (`params`) (`name`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **CallToolResult** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| CallToolResult (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| CallToolResult (`content`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| CallToolResult (`isError`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **CancelledNotification** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| CancelledNotification (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| CancelledNotification (`params`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| CancelledNotification (`params`) (`reason`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| CancelledNotification (`params`) (`requestId`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ClientCapabilities** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ClientCapabilities (`experimental`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ClientCapabilities (`roots`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ClientCapabilities (`roots`) (`listChanged`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ClientCapabilities (`sampling`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ClientNotification** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ClientRequest** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ClientResult** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **CompleteRequest** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CompleteRequest (`method`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CompleteRequest (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CompleteRequest (`params`) (`argument`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CompleteRequest (`params`) (`argument`) (`name`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CompleteRequest (`params`) (`argument`) (`value`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CompleteRequest (`params`) (`ref`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **CompleteResult** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CompleteResult (`_meta`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CompleteResult (`completion`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CompleteResult (`completion`) (`hasMore`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CompleteResult (`completion`) (`total`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CompleteResult (`completion`) (`values`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **CreateMessageRequest** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageRequest (`method`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageRequest (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageRequest (`params`) (`includeContext`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageRequest (`params`) (`maxTokens`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageRequest (`params`) (`messages`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageRequest (`params`) (`metadata`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageRequest (`params`) (`modelPreferences`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageRequest (`params`) (`stopSequences`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageRequest (`params`) (`systemPrompt`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageRequest (`params`) (`temperature`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **CreateMessageResult** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageResult (`_meta`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageResult (`content`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageResult (`model`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageResult (`role`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| CreateMessageResult (`stopReason`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **Cursor** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **EmbeddedResource** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| EmbeddedResource (`annotations`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| EmbeddedResource (`annotations`) (`audience`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| EmbeddedResource (`annotations`) (`priority`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| EmbeddedResource (`resource`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| EmbeddedResource (`type`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **EmptyResult** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **GetPromptRequest** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| GetPromptRequest (`method`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| GetPromptRequest (`params`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| GetPromptRequest (`params`) (`arguments`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| GetPromptRequest (`params`) (`name`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **GetPromptResult** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| GetPromptResult (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| GetPromptResult (`description`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| GetPromptResult (`messages`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ImageContent** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ImageContent (`annotations`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ImageContent (`annotations`) (`audience`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ImageContent (`annotations`) (`priority`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ImageContent (`data`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ImageContent (`mimeType`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ImageContent (`type`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **Implementation** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| Implementation (`name`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| Implementation (`version`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **InitializeRequest** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| InitializeRequest (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| InitializeRequest (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| InitializeRequest (`params`) (`capabilities`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| InitializeRequest (`params`) (`clientInfo`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| InitializeRequest (`params`) (`protocolVersion`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **InitializeResult** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| InitializeResult (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| InitializeResult (`capabilities`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| InitializeResult (`instructions`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| InitializeResult (`protocolVersion`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| InitializeResult (`serverInfo`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **InitializedNotification** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| InitializedNotification (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| InitializedNotification (`params`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| InitializedNotification (`params`) (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **JSONRPCError** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCError (`error`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCError (`error`) (`code`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCError (`error`) (`data`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCError (`error`) (`message`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCError (`id`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCError (`jsonrpc`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **JSONRPCMessage** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **JSONRPCNotification** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCNotification (`jsonrpc`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCNotification (`method`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCNotification (`params`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCNotification (`params`) (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **JSONRPCRequest** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCRequest (`id`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCRequest (`jsonrpc`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCRequest (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCRequest (`params`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCRequest (`params`) (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCRequest (`params`) (`_meta`) (`progressToken`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **JSONRPCResponse** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCResponse (`id`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCResponse (`jsonrpc`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| JSONRPCResponse (`result`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ListPromptsRequest** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListPromptsRequest (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListPromptsRequest (`params`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListPromptsRequest (`params`) (`cursor`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ListPromptsResult** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListPromptsResult (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListPromptsResult (`nextCursor`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListPromptsResult (`prompts`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ListResourceTemplatesRequest** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ListResourceTemplatesRequest (`method`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ListResourceTemplatesRequest (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ListResourceTemplatesRequest (`params`) (`cursor`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **ListResourceTemplatesResult** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ListResourceTemplatesResult (`_meta`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ListResourceTemplatesResult (`nextCursor`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ListResourceTemplatesResult (`resourceTemplates`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **ListResourcesRequest** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListResourcesRequest (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListResourcesRequest (`params`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListResourcesRequest (`params`) (`cursor`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ListResourcesResult** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListResourcesResult (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListResourcesResult (`nextCursor`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListResourcesResult (`resources`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ListRootsRequest** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListRootsRequest (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListRootsRequest (`params`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListRootsRequest (`params`) (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListRootsRequest (`params`) (`_meta`) (`progressToken`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ListRootsResult** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListRootsResult (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListRootsResult (`roots`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ListToolsRequest** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListToolsRequest (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListToolsRequest (`params`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListToolsRequest (`params`) (`cursor`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ListToolsResult** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListToolsResult (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListToolsResult (`nextCursor`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ListToolsResult (`tools`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **LoggingLevel** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **LoggingMessageNotification** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| LoggingMessageNotification (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| LoggingMessageNotification (`params`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| LoggingMessageNotification (`params`) (`data`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| LoggingMessageNotification (`params`) (`level`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| LoggingMessageNotification (`params`) (`logger`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ModelHint** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ModelHint (`name`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ModelPreferences** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ModelPreferences (`costPriority`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ModelPreferences (`hints`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ModelPreferences (`intelligencePriority`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ModelPreferences (`speedPriority`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **Notification** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Notification (`method`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Notification (`params`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Notification (`params`) (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **PaginatedRequest** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | |
| PaginatedRequest (`method`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | |
| PaginatedRequest (`params`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | |
| PaginatedRequest (`params`) (`cursor`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | |
| **PaginatedResult** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | |
| PaginatedResult (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | |
| PaginatedResult (`nextCursor`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[x]` , `[ ]` | |
| **PingRequest** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| PingRequest (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| PingRequest (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| PingRequest (`params`) (`_meta`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| PingRequest (`params`) (`_meta`) (`progressToken`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ProgressNotification** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ProgressNotification (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ProgressNotification (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ProgressNotification (`params`) (`progressToken`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ProgressNotification (`params`) (`progress`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ProgressNotification (`params`) (`total`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **ProgressToken** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **Prompt** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Prompt (`arguments`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Prompt (`description`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Prompt (`name`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **PromptArgument** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| PromptArgument (`description`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| PromptArgument (`name`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| PromptArgument (`required`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **PromptListChangedNotification** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| PromptListChangedNotification (`method`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| PromptListChangedNotification (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| PromptListChangedNotification (`params`) (`_meta`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **PromptMessage** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| PromptMessage (`content`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| PromptMessage (`role`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **PromptReference** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| PromptReference (`name`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| PromptReference (`type`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **ReadResourceRequest** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ReadResourceRequest (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ReadResourceRequest (`params`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ReadResourceRequest (`params`) (`uri`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ReadResourceResult** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ReadResourceResult (`_meta`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ReadResourceResult (`contents`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **Request** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| Request (`method`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| Request (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| Request (`params`) (`_meta`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| Request (`params`) (`_meta`) (`progressToken`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **RequestId** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **Resource** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Resource (`annotations`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| Resource (`annotations`) (`audience`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| Resource (`annotations`) (`priority`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| Resource (`description`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| Resource (`mimeType`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Resource (`name`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Resource (`size`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| Resource (`uri`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ResourceContents** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceContents (`mimeType`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceContents (`uri`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **ResourceListChangedNotification** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceListChangedNotification (`method`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceListChangedNotification (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceListChangedNotification (`params`) (`_meta`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **ResourceReference** | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceReference (`type`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceReference (`uri`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **ResourceTemplate** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceTemplate (`annotations`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceTemplate (`annotations`) (`audience`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceTemplate (`annotations`) (`priority`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceTemplate (`description`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceTemplate (`mimeType`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceTemplate (`name`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceTemplate (`uriTemplate`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **ResourceUpdatedNotification** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceUpdatedNotification (`method`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceUpdatedNotification (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ResourceUpdatedNotification (`params`) (`uri`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **Result** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| Result (`_meta`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **Role** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **Root** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Root (`name`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Root (`uri`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **RootsListChangedNotification** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| RootsListChangedNotification (`method`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| RootsListChangedNotification (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| RootsListChangedNotification (`params`) (`_meta`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **SamplingMessage** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| SamplingMessage (`content`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| SamplingMessage (`role`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **ServerCapabilities** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ServerCapabilities (`experimental`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ServerCapabilities (`logging`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ServerCapabilities (`prompts`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ServerCapabilities (`prompts`) (`listChanged`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ServerCapabilities (`resources`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ServerCapabilities (`resources`) (`listChanged`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ServerCapabilities (`resources`) (`subscribe`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ServerCapabilities (`tools`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| ServerCapabilities (`tools`) (`listChanged`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ServerNotification** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **ServerRequest** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **ServerResult** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **SetLevelRequest** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| SetLevelRequest (`method`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| SetLevelRequest (`params`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| SetLevelRequest (`params`) (`level`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **SubscribeRequest** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| SubscribeRequest (`method`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| SubscribeRequest (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| SubscribeRequest (`params`) (`uri`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **TextContent** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| TextContent (`annotations`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| TextContent (`annotations`) (`audience`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| TextContent (`annotations`) (`priority`) | `[ ]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| TextContent (`text`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| TextContent (`type`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **TextResourceContents** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| TextResourceContents (`mimeType`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| TextResourceContents (`text`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| TextResourceContents (`uri`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **Tool** | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Tool (`description`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Tool (`inputSchema`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Tool (`inputSchema`) (`properties`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Tool (`inputSchema`) (`required`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Tool (`inputSchema`) (`type`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| Tool (`name`) | `[x]` , `[ ]` | `[x]` , `[ ]` | `[ ]` , `[ ]` | |
| **ToolListChangedNotification** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ToolListChangedNotification (`method`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ToolListChangedNotification (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| ToolListChangedNotification (`params`) (`_meta`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| **UnsubscribeRequest** | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| UnsubscribeRequest (`method`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| UnsubscribeRequest (`params`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
| UnsubscribeRequest (`params`) (`uri`) | `[x]` , `[ ]` | `[ ]` , `[ ]` | `[ ]` , `[ ]` | |
