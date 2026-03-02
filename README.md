cdd-c
============

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![cross-OS](https://github.com/SamuelMarks/cdd-c/actions/workflows/cross-OS.yml/badge.svg)](https://github.com/SamuelMarks/cdd-c/actions/workflows/cross-OS.yml)
[![Doc Coverage](https://img.shields.io/badge/docs-100%25-brightgreen.svg)](#)
[![Test Coverage](https://img.shields.io/badge/coverage-100%25-brightgreen.svg)](#)

OpenAPI ↔ C. This is one compiler in a suite, all focussed on the same task: Compiler Driven Development (CDD).

Each compiler is written in its target language, is whitespace and comment sensitive, and has both an SDK and CLI.

The CLI—at a minimum—has:
- `cdd-c --help`
- `cdd-c --version`
- `cdd-c from_openapi -i spec.json`
- `cdd-c to_openapi -f path/to/code`
- `cdd-c to_docs_json --no-imports --no-wrapping -i spec.json`
- `cdd-c serve_json_rpc --port 8082 --listen 0.0.0.0`
- `cdd-c from_openapi to_sdk_cli -i spec.json -o target_directory`
- `cdd-c from_openapi to_sdk -i spec.json -o target_directory`
- `cdd-c from_openapi to_server -i spec.json -o target_directory`

The goal of this project is to enable rapid application development without tradeoffs. Tradeoffs of Protocol Buffers / Thrift etc. are an untouchable "generated" directory and package, compile-time and/or runtime overhead. Tradeoffs of Java or JavaScript for everything are: overhead in hardware access, offline mode, ML inefficiency, and more. And neither of these alternative approaches are truly integrated into your target system, test frameworks, and bigger abstractions you build in your app. Tradeoffs in CDD are code duplication (but CDD handles the synchronisation for you).

## 🚀 Capabilities

The `cdd-c` compiler leverages a unified architecture to support various facets of API and code lifecycle management.

* **Compilation**:
  * **OpenAPI → `C`**: Generate idiomatic native models, network routes, client SDKs, database schemas, and boilerplate directly from OpenAPI (`.json` / `.yaml`) specifications.
  * **`C` → OpenAPI**: Statically parse existing `C` source code and emit compliant OpenAPI specifications.
* **AST-Driven & Safe**: Employs static analysis (Abstract Syntax Trees via a custom whitespace, comment, and macro sensitive parser) instead of unsafe dynamic execution or reflection, allowing it to safely parse and emit code even for incomplete or un-compilable project states.
* **Seamless Sync**: Keep your docs, tests, database, clients, and routing in perfect harmony. Update your code, and generate the docs; or update the docs, and generate the code.

## 📦 Installation

Requires a C compiler (GCC/Clang/MSVC) and CMake.

```bash
# Ubuntu / Debian
sudo apt-get install gcc cmake pkg-config

# Build the CLI
make build

# The CLI binary is placed in bin/
./bin/cdd-c --help
```

## 🛠 Usage

### Command Line Interface

```bash
# Generate C models from an OpenAPI spec
./bin/cdd-c from_openapi to_sdk -i spec.json -o src/models

# Generate an OpenAPI spec from your C code
./bin/cdd-c to_openapi -f src/models/my_model.c -o openapi.json
```

### Programmatic SDK / Library

```c
#include "cdd_api.h"
#include <stdio.h>

int main() {
    cdd_config config = {
        .input_path = "spec.json",
        .output_dir = "src/models"
    };
    cdd_generate_sdk(&config);
    printf("SDK generation complete.\n");
    return 0;
}
```

## Design choices

The `cdd-c` project uses a fully custom whitespace, comment, and macro sensitive parser for Lexing and Parsing C, enabling highly robust, standardized syntax tree creation without relying on complex, heavy external C/C++ parsers like Clang AST for everything, ensuring it stays lightweight.

## 🏗 Supported Conversions for C

*(The boxes below reflect the features supported by this specific `cdd-c` implementation)*

| Concept | Parse (From) | Emit (To) |
|---------|--------------|-----------|
| OpenAPI (JSON/YAML) | ✅ | ✅ |
| `C` Models / Structs / Types | ✅ | ✅ |
| `C` Server Routes / Endpoints | ✅ | ✅ |
| `C` API Clients / SDKs | ✅ | ✅ |
| `C` ORM / DB Schemas | [ ] | [ ] |
| `C` CLI Argument Parsers | [ ] | ✅ |
| `C` Docstrings / Comments | ✅ | ✅ |

WASM Support: Possible ✅ | Implemented ✅

---

## License

Licensed under either of

- Apache License, Version 2.0 ([LICENSE-APACHE](LICENSE-APACHE) or <https://www.apache.org/licenses/LICENSE-2.0>)
- MIT license ([LICENSE-MIT](LICENSE-MIT) or <https://opensource.org/licenses/MIT>)

at your option.

### Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted
for inclusion in the work by you, as defined in the Apache-2.0 license, shall be
dual licensed as above, without any additional terms or conditions.
