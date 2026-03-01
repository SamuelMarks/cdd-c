# WebAssembly (WASM) Support

The `cdd-c` project fully supports compilation to WebAssembly using Emscripten.

## Why WebAssembly?

Building `cdd-c` into WASM allows it to be integrated into:
- A unified CLI for all `cdd-*` projects that can run on any system without requiring C compilers, Node.js, PHP, or Python.
- A unified web interface that runs natively within the browser, providing instant parsing and generation without round-trips to a server.

## Building for WASM

To build the WASM target, you need the Emscripten SDK (`emsdk`). Assuming it is installed in a directory above the current one (`../emsdk`), you can build it via:

```bash
# Setup emsdk
source ../emsdk/emsdk_env.sh

# Run the build command
make build_wasm
```

This will create a `build_wasm` directory containing the `cdd-c.js` and `cdd-c.wasm` artifacts, which can then be embedded into your JS environments.

## Integration

The emitted `cdd-c.js` module can be loaded in browsers or Node.js via standard Emscripten APIs:

```javascript
const cddModule = require('./build_wasm/cdd-c.js');

cddModule().then((instance) => {
  // Use instance to call exported C functions
});
```
