cdd-c
============

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![CI/CD](https://github.com/offscale/cdd-c/workflows/CI/badge.svg)](https://github.com/offscale/cdd-c/actions)
<!-- REPLACE WITH separate test and doc coverage badges that you generate in pre-commit hook -->

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

Requires CMake and a C90 compiler.

```bash
mkdir build && cd build
cmake ..
make -j4
sudo make install
```

## 🛠 Usage

### Command Line Interface

<!-- INSTRUCTION TO LLM: Provide 1-2 idiomatic CLI examples using the `cdd-c` placeholder. Ensure paths reflect standard `C` project structures. -->

### Programmatic SDK / Library

<!-- INSTRUCTION TO LLM: Provide a small code snippet in `C` demonstrating how to invoke the compiler as a library, using the `c`. -->

## Design choices


This project is written in strict C90 to ensure maximum portability across embedded systems, legacy environments, and modern platforms alike. It avoids large heavy-weight frameworks in favor of tightly-scoped, bespoke parsing and emitting logic. 

It features a custom C lexer and parser (AST generator) built specifically to extract OpenAPI semantics directly from C source code (structs, enums, docstrings, and custom annotations like `@route` and `@webhook`). This allows it to operate completely statically without needing to execute or even fully compile the target C code. It uses `parson` for lightweight JSON manipulation and `libcurl` for testing generated client SDKs.


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