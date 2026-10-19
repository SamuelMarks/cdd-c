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
extern C_CDD_EXPORT cdd_c_error_t openapi_client_generate(
    const struct OpenAPI_Spec *spec, const struct OpenApiClientConfig *config);

/**
 * @brief Finds a server variable by name in an OpenAPI Server object.
 *
 * @param[in] srv The OpenAPI Server structure.
 * @param[in] name The variable name to find.
 * @param[out] _out_val Output pointer to the found variable, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_server_variable(const struct OpenAPI_Server *srv, const char *name,
                     const struct OpenAPI_ServerVariable **_out_val);

/**
 * @brief Renders a Server URL substituting default values for template
 * variables.
 *
 * @param[in] srv The OpenAPI Server structure.
 * @param[out] _out_val Newly allocated rendered URL string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
render_server_url_default(const struct OpenAPI_Server *srv, char **_out_val);

/**
 * @brief Escapes backslashes and quotes in a string for C string literal
 * emission.
 *
 * @param[in] s Input string.
 * @param[out] _out_val Newly allocated escaped string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t escape_c_string_literal(const char *s,
                                                          char **_out_val);

/**
 * @brief Selects the effective OpenAPI Server for an operation (operation-level
 * or path-level).
 *
 * @param[in] path Enclosing OpenAPI Path item.
 * @param[in] op OpenAPI Operation structure.
 * @param[out] _out_val Output pointer to the selected Server, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t select_operation_server(
    const struct OpenAPI_Path *path, const struct OpenAPI_Operation *op,
    struct OpenAPI_Server **_out_val);

/**
 * @brief Builds a C string literal expression for a base URL.
 *
 * @param[in] url Raw URL string.
 * @param[out] _out_val Newly allocated C string literal expression.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t build_base_url_literal(const char *url,
                                                         char **_out_val);

/**
 * @brief Generates an uppercase C header include guard from a base name.
 *
 * @param[in] base Base filename string.
 * @param[out] _out_val Newly allocated include guard string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t generate_guard(const char *base,
                                                 char **_out_val);

/**
 * @brief Derives the default model header filename from a base name.
 *
 * @param[in] base Base filename string.
 * @param[out] _out_val Newly allocated model header filename.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t derive_model_header(const char *base,
                                                      char **_out_val);

/**
 * @brief Sanitizes an OpenAPI tag name into a valid C identifier segment.
 *
 * @param[in] tag Raw tag string.
 * @param[out] _out_val Newly allocated sanitized tag string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t sanitize_tag(const char *tag,
                                               char **_out_val);

/**
 * @brief Checks if two OpenAPI parameters share the same name and location.
 *
 * @param[in] a First OpenAPI parameter.
 * @param[in] b Second OpenAPI parameter.
 * @return Non-zero if matching, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t param_keys_match(
    const struct OpenAPI_Parameter *a, const struct OpenAPI_Parameter *b);

/**
 * @brief Merges path-level and operation-level parameters into an effective
 * parameter list.
 *
 * @param[in] path Enclosing OpenAPI Path item.
 * @param[in] op OpenAPI Operation structure.
 * @param[out] out_params Output array of merged parameters.
 * @param[out] out_count Output count of merged parameters.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t build_effective_parameters(
    const struct OpenAPI_Path *path, const struct OpenAPI_Operation *op,
    struct OpenAPI_Parameter **out_params, size_t *out_count);

/**
 * @brief Converts an OpenAPI HTTP verb enum into its uppercase string literal.
 *
 * @param[in] verb HTTP verb enum.
 * @param[out] _out_val Pointer to receive the static uppercase verb string.
 * @return CDD_C_SUCCESS on success or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t verb_to_string(enum OpenAPI_Verb verb,
                                                 const char **_out_val);

/**
 * @brief Writes a Doxygen docblock for an OpenAPI operation.
 *
 * @param[in,out] fp Output file stream.
 * @param[in] path Enclosing OpenAPI path item.
 * @param[in] op OpenAPI operation descriptor.
 * @return CDD_C_SUCCESS on success or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_docblock(FILE *fp, const struct OpenAPI_Path *path,
               const struct OpenAPI_Operation *op);

/**
 * @brief Writes the C header preamble for a generated OpenAPI client header.
 *
 * @param[in,out] fp Output file stream.
 * @param[in] guard Include guard macro name.
 * @param[in] model_decl Optional model header filename to include.
 * @return CDD_C_SUCCESS on success or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t write_header_preamble(FILE *fp,
                                                        const char *guard,
                                                        const char *model_decl);

/**
 * @brief Writes the C source preamble for a generated OpenAPI client
 * implementation.
 *
 * @param[in,out] fp Output file stream.
 * @param[in] header_name Header filename to include.
 * @return CDD_C_SUCCESS on success or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_source_preamble(FILE *fp, const char *header_name);

/**
 * @brief Writes client lifecycle (`_init`, `_cleanup`, `ApiError`) functions.
 *
 * @param[in,out] h Output header file stream.
 * @param[in,out] c Output source file stream.
 * @param[in] prefix Function name prefix.
 * @param[in] spec Parsed OpenAPI specification.
 * @return CDD_C_SUCCESS on success or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t write_lifecycle_funcs(
    FILE *h, FILE *c, const char *prefix, const struct OpenAPI_Spec *spec);

/**
 * @brief Emits the declaration and implementation for a single OpenAPI
 * operation.
 *
 * @param[in,out] hfile Output header file stream.
 * @param[in,out] cfile Output source file stream.
 * @param[in] path Enclosing OpenAPI path item.
 * @param[in] op OpenAPI operation descriptor.
 * @param[in] spec Parsed OpenAPI specification.
 * @param[in] config Client generator configuration.
 * @param[in] prefix Function name prefix.
 * @return CDD_C_SUCCESS on success or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t emit_operation(
    FILE *hfile, FILE *cfile, const struct OpenAPI_Path *path,
    const struct OpenAPI_Operation *op, const struct OpenAPI_Spec *spec,
    const struct OpenApiClientConfig *config, const char *prefix);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_OPENAPI_CLIENT_GEN_H */
