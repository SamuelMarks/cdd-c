# Compliance

`cdd-c` is designed with strict adherence to portability, standardization, and security practices.

## Language Standards

*   **C89 / ANSI C Compatibility:** The core library and generated SDKs are strictly written in C89/C90 (ISO/IEC 9899:1990). This ensures maximum portability across embedded systems, legacy toolchains, and modern compilers alike.
*   **No Variable Length Arrays (VLAs):** VLAs (introduced in C99) are strictly avoided to prevent stack-smashing vulnerabilities and ensure compatibility with MSVC.
*   **Strict Typing & Aliasing:** The codebase enforces strict pointer aliasing rules (`-fstrict-aliasing`) and compiles cleanly under `-Wall -Wextra -pedantic -Wshadow`.

## Security

*   **Memory Safety Analysis:** `cdd-c` includes built-in auditing tools to statically detect unchecked heap allocations (e.g., `malloc` without `NULL` checks).
*   **Safe String Handling:** Avoids standard `<string.h>` buffer-overflow-prone functions (like `strcpy`, `sprintf`) in favor of length-bounded or dynamically allocating alternatives (like `asprintf` or `snprintf` polyfills from `c89stringutils`), and non-allocating string views via `c-str-span`.
*   **Secure Transport:**
    *   **Linux / macOS:** Uses `libcurl` compiled against OpenSSL/BoringSSL for robust TLS 1.2+ and certificate validation.
    *   **Windows:** Uses native `WinHTTP` with secure flags enabled, leveraging the OS-level certificate store and Schannel provider, avoiding the need to ship separate crypto binaries.

## OpenAPI Compliance

*   **OpenAPI v3.x:** Focuses on generating and consuming OpenAPI 3.2.0 specifications.
*   **Validation:** Generated specs enforce structural compliance (e.g., path templates matching parameters, required `info` and `paths` blocks, valid HTTP status codes).
*   **Type Resolution:** Accurately resolves `$ref` components, handles inline schemas by promoting them to components, and maps JSON schema types directly to safe C struct representations.

## Licensing

`cdd-c` is dual-licensed under:
*   Apache License, Version 2.0 (`LICENSE-APACHE`)
*   MIT License (`LICENSE-MIT`)

Users may choose either license when consuming or contributing to the project.
