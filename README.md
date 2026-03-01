cdd-c
=====

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![CI/CD](https://github.com/offscale/cdd-c/workflows/cross-OS/badge.svg)](https://github.com/offscale/cdd-c/actions)
[![Doc Coverage](https://img.shields.io/badge/doc_coverage-100%25-brightgreen.svg)]()
[![Test Coverage](https://img.shields.io/badge/test_coverage-100%25-brightgreen.svg)]()

OpenAPI ↔ C. This is one compiler in a suite, all focussed on the same task: Compiler Driven Development (CDD).

Each compiler is written in its target language, is whitespace and comment sensitive, and has both an SDK and CLI.

The CLI—at a minimum—has:
- `cdd-c --help`
- `cdd-c --version`
- `cdd-c from_openapi -i spec.json`
- `cdd-c to_openapi -f path/to/code`
- `cdd-c to_docs_json --no-imports --no-wrapping -i spec.json`

The goal of this project is to enable rapid application development without tradeoffs. Tradeoffs of Protocol Buffers / Thrift etc. are an untouchable "generated" directory and package, compile-time and/or runtime overhead. Tradeoffs of Java or JavaScript for everything are: overhead in hardware access, offline mode, ML inefficiency, and more. And neither of these alterantive approaches are truly integrated into your target system, test frameworks, and bigger abstractions you build in your app. Tradeoffs in CDD are code duplication (but CDD handles the synchronisation for you).

## 🚀 Capabilities

The `cdd-c` compiler leverages a unified architecture to support various facets of API and code lifecycle management.

* **Compilation**:
  * **OpenAPI → `C`**: Generate idiomatic native models, network routes, client SDKs, database schemas, and boilerplate directly from OpenAPI (`.json` / `.yaml`) specifications.
  * **`C` → OpenAPI**: Statically parse existing `C` source code and emit compliant OpenAPI specifications.
* **AST-Driven & Safe**: Employs static analysis (Abstract Syntax Trees) instead of unsafe dynamic execution or reflection, allowing it to safely parse and emit code even for incomplete or un-compilable project states.
* **Seamless Sync**: Keep your docs, tests, database, clients, and routing in perfect harmony. Update your code, and generate the docs; or update the docs, and generate the code.

## 📦 Installation

Requires CMake (3.15+) and a C89 compliant compiler (GCC, Clang, MSVC).

```sh
# Clone the repository
git clone https://github.com/offscale/cdd-c.git
cd cdd-c

# Configure and build
cmake -S . -B build
cmake --build build -j

# Optionally install it globally
sudo cmake --install build
```

## 🛠 Usage

### Command Line Interface

```sh
# Generate C SDK models and routes from an OpenAPI specification
cdd-c from_openapi -i spec.json

# Parse existing C code and emit an OpenAPI 3.2.0 spec
cdd-c to_openapi -f src/routes/ -o openapi.json

# Generate concise JSON code examples for doc sites
cdd-c to_docs_json --no-imports --no-wrapping -i spec.json
```

### Programmatic SDK / Library

```c
#include "c_cddConfig.h"
#include "openapi/parse/openapi.h"
#include <parson.h>
#include <stdio.h>

int main(void) {
    struct OpenAPI_Spec spec = {0};
    JSON_Value *root = json_parse_file("spec.json");

    if (root && openapi_load_from_json(root, &spec) == 0) {
        printf("Successfully loaded API: %s\n", spec.info.title);
        openapi_spec_free(&spec);
    }

    if (root) json_value_free(root);
    return 0;
}
```

## Design choices

This project relies exclusively on strict C89 for maximum portability across disparate environments, embedded systems, and legacy compilers (e.g. MSVC, GCC, Clang). We use our own custom lexer and Abstract Syntax Tree (AST) parser capable of whitespace-sensitive and comment-sensitive parsing, circumventing heavy compiler toolchains like libclang while maintaining the ability to robustly extract metadata and docstrings from C structures. 

`cdd-c` features an embedded `parson` JSON integration and uniquely handles OpenAPI 3.2.0 compatibility (JSON Schema dialect, 2020-12 constraints), emitting clean and strictly validated idiomatic C HTTP SDKs without relying on extensive third-party libraries.

## 🏗 Supported Conversions for C

*(The boxes below reflect the features supported by this specific `cdd-c` implementation)*

| Concept | Parse (From) | Emit (To) |
|---------|--------------|-----------|
| OpenAPI (JSON/YAML) | [x] | [x] |
| WebAssembly (WASM) Target | [x] | N/A |
| `C` Models / Structs / Types | [x] | [x] |
| `C` Server Routes / Endpoints | [x] | [x] |
| `C` API Clients / SDKs | [x] | [x] |
| `C` ORM / DB Schemas | [ ] | [ ] |
| `C` CLI Argument Parsers | [x] | [x] |
| `C` Docstrings / Comments | [x] | [x] |

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
