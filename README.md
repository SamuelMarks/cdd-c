c-cdd
=====

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Frontend for C, concentrating on: generation from code; single-file analysis; modification; and emission (prettyprinting).

Use-cases this is designed to support:

  - Generate/update `free` calling `cleanup_StructName` functions;
  - Generate/update parsers taking `struct` as input and producing `const struct StructName StructName_<fmt>_parse(const char*)` function;
  - Generate/update emitters taking `struct` as input and producing `const char* StructName_<fmt>_emit(const struct StructName*)` function;
    - fmt: could be JSON, INI, YAML, TOML, &etc.
  - Generate Google Cloud client library for C (with Google Cloud Discovery API as input);
  - Generate arbitrary client libraries (with OpenAPI as input).

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
