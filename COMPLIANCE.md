# Compliance

> **Purpose of this file (`COMPLIANCE.md`)**: To outline the exact specifications, standards, and metrics the project strictly adheres to, and report on the current compliance status.

## OpenAPI 3.2.0 Specification Compliance
`cdd-c` acts as a reference implementation for generating and validating the **OpenAPI 3.2.0** specification.

The project currently fully supports:
- Root level `paths` and `webhooks` definition.
- Advanced `components` including schemas, responses, parameters, examples, requestBodies, headers, securitySchemes, links, callbacks, and pathItems.
- Multi-document and dynamic `$ref` traversal.
- Precise encoding definitions for Media Types.
- OAuth2 Security Scheme Flows.

*Note: The actual specifications are maintained in this repository as `3.2.0.md` and `3.1.1.md` for local test validation and regression checking.*

## Strict C89 (ANSI C) Compliance
To ensure absolute portability across embedded controllers, IoT applications, and major enterprise backends, the codebase enforces strict C89 constraints.

- Variables are exclusively declared at the start of scope blocks.
- Uses `/* ... */` comment blocks natively (with parser-awareness for modern `//` styles in user code).
- Tested simultaneously against:
  - MSVC
  - GCC
  - Clang

## Quality Assurance Metrics
- **Test Coverage:** The repository enforces a 100% test coverage rule. The CI/CD pipeline validates every code path, edge case, and failure state using automated tests.
- **Documentation Coverage:** The codebase requires a full Doxygen-style docstring block ensuring 100% doc coverage.