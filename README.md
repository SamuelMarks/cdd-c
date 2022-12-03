c-cdd
=====

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Linux, Windows, macOS](https://github.com/SamuelMarks/cdd-c/actions/workflows/linux-Windows-macOS.yml/badge.svg)](https://github.com/SamuelMarks/cdd-c/actions/workflows/linux-Windows-macOS.yml)

Frontend for C, concentrating on: generation from code; single-file analysis; modification; and emission (to C code).

Use-cases this is designed to support:

  - Generate/update `free` calling `int cleanup_StructName(struct StructName*)` functions;
  - With `fmt` of JSON, INI, YAML, TOML, &etc.:
    - Generate/update parsers  from `const char*` to `struct`: `const int StructName_<fmt>_parse(struct StructName*, const char*)` function;
    - Generate/update emitters from `struct` to `char*`: `const int StructName_<fmt>_emit(const struct StructName*, char*)` function;
  - Generate Google Cloud client library for C (with [Google Cloud API Discovery Service](https://developers.google.com/discovery/v1/reference) as input);
  - Generate arbitrary C client libraries (with [OpenAPI](https://spec.openapis.org/oas/v3.1.0) as input).

## Design

This C compiler has a very unusual design, the macro and C language are treated as one. The foci are:

  - location start/end of function, `struct`, and feature macros (e.g., `#ifdef JSON_EMIT`, `#endif /* JSON_EMIT */`);
  - `struct` fields.

…which enable a number of use-cases to be simply developed, e.g., see the list above.

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
