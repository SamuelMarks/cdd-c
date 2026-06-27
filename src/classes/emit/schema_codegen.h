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
#include "parson.h"
/* clang-format on */

/**
 * @brief Entry point for generating C code from a JSON schema file.
 * Requires a schema file path and an output basename.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector. argv[0] is schema file, argv[1] is output
 * basename.
 * @return 0 on success, non-zero error code (errno or EXIT_FAILURE) on failure.
 */
/* Forward declarations for testing */
struct CodegenConfig;
extern C_CDD_EXPORT int generate_header(const char *prefix,
                                        const char *basename,
                                        JSON_Object *schemas_obj,
                                        const struct CodegenConfig *config);
extern C_CDD_EXPORT int generate_source(const char *prefix,
                                        const char *basename,
                                        JSON_Object *schemas_obj,
                                        const struct CodegenConfig *config);

extern C_CDD_EXPORT int schema2code_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SCHEMA_CODEGEN_H */
