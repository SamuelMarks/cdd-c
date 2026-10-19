/**
 * @file schema_codegen.h
 * @brief Logic for generating C headers and sources from JSON Schemas.
 * @author Samuel Marks
 */

#ifndef SCHEMA_CODEGEN_H
#define SCHEMA_CODEGEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include <c_cdd_export.h>
#include "cdd_c_error.h"
#include "parson.h"
/* clang-format on */

/* Forward declarations for testing */
struct CodegenConfig;

/**
 * @brief Generates a C header file (`<prefix>.h`) for the given JSON Schemas.
 *
 * @param[in] prefix Output path prefix (without `.h` extension).
 * @param[in] basename Base filename used for include guards.
 * @param[in] schemas_obj Parson `JSON_Object` mapping schema names to schemas.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
generate_header(const char *prefix, const char *basename,
                JSON_Object *schemas_obj, const struct CodegenConfig *config);

/**
 * @brief Generates a C source file (`<prefix>.c`) for the given JSON Schemas.
 *
 * @param[in] prefix Output path prefix (without `.c` extension).
 * @param[in] basename Base filename used to `#include "<basename>.h"`.
 * @param[in] schemas_obj Parson `JSON_Object` mapping schema names to schemas.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
generate_source(const char *prefix, const char *basename,
                JSON_Object *schemas_obj, const struct CodegenConfig *config);

/**
 * @brief Entry point for generating C code from a JSON schema file.
 * Requires a schema file path and an output basename.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector. argv[0] is schema file, argv[1] is output
 * basename.
 * @return 0 on success, non-zero error code (errno or EXIT_FAILURE) on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t schema2code_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SCHEMA_CODEGEN_H */
