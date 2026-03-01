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

#include <c_cdd_export.h>

/**
 * @brief Entry point for generating C code from a JSON schema file.
 * Requires a schema file path and an output basename.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector. argv[0] is schema file, argv[1] is output
 * basename.
 * @return 0 on success, non-zero error code (errno or EXIT_FAILURE) on failure.
 */
extern C_CDD_EXPORT int schema2code_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SCHEMA_CODEGEN_H */
