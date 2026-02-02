/**
 * @file openapi_client_gen.h
 * @brief Orchestrator for generating C API Client libraries from OpenAPI specs.
 *
 * Drives the generation of Header and Implementation files.
 * Includes definitions for the standard `ApiError` structure (RFC 7807).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_OPENAPI_CLIENT_GEN_H
#define C_CDD_OPENAPI_CLIENT_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "openapi_loader.h"

/**
 * @brief Configuration configuration for the Client Generator.
 */
struct OpenApiClientConfig {
  /**
   * @brief The base name for output files.
   * e.g., "petstore" -> generates "petstore.h" and "petstore.c".
   */
  const char *filename_base;

  /**
   * @brief Prefix to prepend to generated C function names.
   * e.g., "api_" -> "api_get_pet".
   */
  const char *func_prefix;

  /**
   * @brief Name of the header file containing data models (structs).
   * Used to start the generated header with `#include "models.h"`.
   * If NULL, defaults to `<filename_base>_models.h`.
   */
  const char *model_header;

  /**
   * @brief Macro to safeguard header file (include guard).
   * e.g., "PETSTORE_CLIENT_H". If NULL, derived from filename_base.
   */
  const char *header_guard;

  /**
   * @brief Global namespace prefix for function grouping.
   * If provided, prepended to the Resource group (e.g. "Foo").
   * Result: `Foo_Pet_api_get`.
   */
  const char *namespace_prefix;
};

/**
 * @brief Generate the C Client Library (Header and Source).
 *
 * Performs the following steps:
 * 1. Opens `<basename>.h` and `<basename>.c` for writing.
 * 2. Writes Preamble (Includes, Type Definitions).
 *    - Injects `struct ApiError` definition for global error handling.
 * 3. Iterates through all Paths and Operations in the Spec.
 * 4. For each operation:
 *    - Generates a Doxygen-style docblock in the header.
 *    - Resolves naming convention (Namespace + Tag + Prefix).
 *    - Generates a function prototype in the header.
 *    - Generates the function definition and body in the source.
 * 5. Writes the `_init`, `_cleanup`, and `ApiError` lifecycle functions.
 *
 * @param[in] spec The parsed OpenAPI specification.
 * @param[in] config Configuration options for generation.
 * @return 0 on success, error code (EINVAL, ENOMEM, EIO) on failure.
 */
extern C_CDD_EXPORT int
openapi_client_generate(const struct OpenAPI_Spec *spec,
                        const struct OpenApiClientConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_OPENAPI_CLIENT_GEN_H */
