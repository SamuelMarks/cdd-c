cdd-c
=====

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![CI/CD](https://github.com/offscale/cdd-c/workflows/ci/badge.svg)](https://github.com/offscale/cdd-c/actions)

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

This project is built using CMake and requires a C89 compatible compiler (e.g., GCC, Clang, MSVC).

```bash
git clone https://github.com/offscale/cdd-c.git
cd cdd-c
cmake -B build
cmake --build build --config Release
```

After building, the `cdd-c` executable will be available in `build/bin/` (or `build\bin\Release` on Windows).

## 🛠 Usage

### Command Line Interface

Generate an OpenAPI specification from your C source code:
```bash
./build/bin/cdd-c to_openapi -f src/ -o openapi.json
```

Generate a C client SDK from an OpenAPI specification:
```bash
./build/bin/cdd-c from_openapi -i openapi.json
```

Generate a JSON structure with language-specific code examples from an OpenAPI specification:
```bash
./build/bin/cdd-c to_docs_json --no-imports --no-wrapping -i openapi.json
```

### Programmatic SDK / Library

You can also use `cdd-c` programmatically by including it as a library in your C projects:

```c
#include "c_cddConfig.h"
#include "routes/parse/cli.h"
#include "openapi/parse/openapi.h"

int main(int argc, char **argv) {
    /* Initialize an OpenAPI Spec from C code programmatically */
    char *c2_args[] = {"c2openapi", "src/", "openapi.json"};
    return c2openapi_cli_main(3, c2_args);
}
```

## Design choices

`cdd-c` is written in strict, pure C89 (ANSI C) to guarantee maximum portability across embedded systems, legacy compilers, and modern platforms. It uses a custom lexical analyzer and parser rather than relying on heavy compiler frameworks (like Clang/LLVM) to ensure minimal dependencies and high execution speed. Its AST approach preserves whitespace and comment formatting to enable non-destructive generation without disrupting handwritten additions or file layout.

## 🏗 Supported Conversions for C

*(The boxes below reflect the features supported by this specific `cdd-c` implementation)*

| Concept | Parse (From) | Emit (To) |
|---------|--------------|-----------|
| OpenAPI (JSON/YAML) | [✅] | [✅] |
| `C` Models / Structs / Types | [✅] | [✅] |
| `C` Server Routes / Endpoints | [✅] | [✅] |
| `C` API Clients / SDKs | [ ] | [✅] |
| `C` ORM / DB Schemas | [ ] | [ ] |
| `C` CLI Argument Parsers | [ ] | [ ] |
| `C` Docstrings / Comments | [✅] | [✅] |

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
