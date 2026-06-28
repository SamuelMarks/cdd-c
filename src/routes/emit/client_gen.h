/**
 * @file client_gen.h
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

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

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
   * Used to start the generated header with `\#include "models.h"`.
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

  /**
   * @brief If non-zero, prevents generating GitHub Actions CI workflow files.
   */
  int no_github_actions;

  /**
   * @brief If non-zero, prevents generating installable package build systems
   * (e.g. CMakeLists.txt).
   */
  int no_installable_package;

  /**
   * @brief If non-zero, generates composable tests and mocks.
   */
  int create_tests_and_mocks;
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
extern C_CDD_EXPORT enum cdd_c_error
openapi_client_generate(const struct OpenAPI_Spec *spec,
                        const struct OpenApiClientConfig *config);

extern C_CDD_EXPORT enum cdd_c_error
find_server_variable(const struct OpenAPI_Server *srv, const char *name,
                     const struct OpenAPI_ServerVariable **_out_val);
extern C_CDD_EXPORT enum cdd_c_error
render_server_url_default(const struct OpenAPI_Server *srv, char **_out_val);
extern C_CDD_EXPORT enum cdd_c_error escape_c_string_literal(const char *s,
                                                             char **_out_val);
extern C_CDD_EXPORT enum cdd_c_error
select_operation_server(const struct OpenAPI_Path *path,
                        const struct OpenAPI_Operation *op,
                        struct OpenAPI_Server **_out_val);
extern C_CDD_EXPORT enum cdd_c_error build_base_url_literal(const char *url,
                                                            char **_out_val);
extern C_CDD_EXPORT enum cdd_c_error generate_guard(const char *base,
                                                    char **_out_val);
extern C_CDD_EXPORT enum cdd_c_error derive_model_header(const char *base,
                                                         char **_out_val);
extern C_CDD_EXPORT enum cdd_c_error sanitize_tag(const char *tag,
                                                  char **_out_val);
extern C_CDD_EXPORT enum cdd_c_error
param_keys_match(const struct OpenAPI_Parameter *a,
                 const struct OpenAPI_Parameter *b);
extern C_CDD_EXPORT enum cdd_c_error build_effective_parameters(
    const struct OpenAPI_Path *path, const struct OpenAPI_Operation *op,
    struct OpenAPI_Parameter **out_params, size_t *out_count);
extern C_CDD_EXPORT enum cdd_c_error verb_to_string(enum OpenAPI_Verb verb,
                                                    char **_out_val);
extern C_CDD_EXPORT enum cdd_c_error
write_docblock(FILE *fp, const struct OpenAPI_Path *path,
               const struct OpenAPI_Operation *op);
extern C_CDD_EXPORT enum cdd_c_error
write_header_preamble(FILE *fp, const char *guard, const char *model_decl);
extern C_CDD_EXPORT enum cdd_c_error
write_source_preamble(FILE *fp, const char *header_name);
extern C_CDD_EXPORT enum cdd_c_error
write_lifecycle_funcs(FILE *h, FILE *c, const char *prefix,
                      const struct OpenAPI_Spec *spec);

extern C_CDD_EXPORT enum cdd_c_error
emit_operation(FILE *hfile, FILE *cfile, const struct OpenAPI_Path *path,
               const struct OpenAPI_Operation *op,
               const struct OpenAPI_Spec *spec,
               const struct OpenApiClientConfig *config, const char *prefix);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_OPENAPI_CLIENT_GEN_H */
