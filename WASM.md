# WebAssembly (WASM) Support

The `cdd-c` project supports compilation to WebAssembly using **WASI SDK** (`wasi-sdk`, targeting `wasm32-wasi` / `wasi_snapshot_preview1`) as well as **Emscripten**.

## Why WebAssembly?

Building `cdd-c` into WASM allows it to be integrated into:

- A unified CLI (`cdd-c.wasm`) for all `cdd-*` projects that runs on any system via WASI runtimes (`wasmtime`, `wasmer`, Node.js `node:wasi`, or browser WASI shims) without requiring C compilers, PHP, or Python.
- A Model Context Protocol (MCP) server over standard input/output (`cdd-c.wasm mcp`) inside sandboxed WebAssembly environments.
- A unified web interface that runs natively within the browser, providing instant C parsing, OpenAPI/Arazzo codegen, and AST refactoring without server round-trips.

## Building for WASM (`wasi-sdk`)

Running `make build_wasm` automatically downloads `wasi-sdk` (if not already present in `./wasi-sdk`) and configures an incremental CMake build in `build_wasm/`:

```bash
make build_wasm
```

This produces `bin/cdd-c.wasm` (and `build_wasm/bin/cdd-c.wasm`).

## Running `bin/cdd-c.wasm`

### Via Node.js (`node:wasi`)

```javascript
const fs = require("fs");
const { WASI } = require("wasi");

const wasi = new WASI({
  version: "preview1",
  args: ["cdd-c", "--version"],
  preopens: { ".": "." },
});

const wasm = new WebAssembly.Module(fs.readFileSync("./bin/cdd-c.wasm"));
const instance = new WebAssembly.Instance(wasm, wasi.getImportObject());
wasi.start(instance);
```

### Via Wasmtime

```bash
wasmtime run --dir=. bin/cdd-c.wasm --help
```
