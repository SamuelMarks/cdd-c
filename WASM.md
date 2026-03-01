# WebAssembly (WASM) Support in `cdd-c`

The `cdd-c` compiler fully supports being compiled to a standalone WebAssembly (WASM) module via Emscripten.

This enables you to:
1. Run the CDD compiler entirely in the browser for web interfaces.
2. Execute the CLI via Node.js/Deno on systems without a native compiler toolchain.
3. Integrate `cdd-c` into the unified `cdd-*` cross-language orchestrator without requiring complex native distribution layers.

## Building the WASM Target

To build `cdd-c` for WASM, ensure you have the [Emscripten SDK (emsdk)](https://emscripten.org/docs/getting_started/downloads.html) installed in the parent directory (`../emsdk`).

Then, run:
```sh
make build_wasm
```
*(Or use `make.bat build_wasm` on Windows)*

This will execute `emcmake cmake` and output `cdd-c.js` and `cdd-c.wasm` into the `build_wasm/bin/` directory.

### Build Details
- Tests are inherently disabled for WASM target compilation.
- The `libcurl` dependency is built natively via Emscripten using `-DHTTP_ONLY=ON` and `OpenSSL` support is explicitly bypassed to ensure a portable, statically pure WASM binary.
