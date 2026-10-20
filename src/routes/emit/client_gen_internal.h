/**
 * @file client_gen_internal.h
 * @brief Internal declarations for OpenAPI client generator modularization.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_ROUTES_EMIT_CLIENT_GEN_INTERNAL_H
#define C_CDD_ROUTES_EMIT_CLIENT_GEN_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd/code_writer.h"
#include "c_cdd/context.h"
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "routes/emit/client_gen.h"
#include <stdio.h>
/* clang-format on */

/** @brief fprintf macro alias for stream hook */
#define fprintf cdd_io_fprintf_hook

/**
 * @def CHECK_IO_CLEANUP
 * @brief CHECK_IO_CLEANUP macro
 */
#define CHECK_IO_CLEANUP(x)                                                    \
  for (; (x) < 0;) {                                                           \
    rc = CDD_C_ERROR_IO;                                                       \
    fprintf(stderr, "goto cleanup at %s:%d\n", __FILE__, __LINE__);            \
    goto cleanup;                                                              \
  }

/**
 * @def CHECK_IO
 * @brief CHECK_IO macro
 */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

/**
 * @brief Emit OpenAPI models into models header and source.
 *
 * @param[in,out] mhfile Output models header file stream.
 * @param[in,out] mcfile Output models source file stream.
 * @param[in] model_guard Include guard macro for models header.
 * @param[in] mh_name Models header filename.
 * @param[in] spec Parsed OpenAPI specification.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_gen_emit_models(FILE *mhfile, FILE *mcfile, const char *model_guard,
                       const char *mh_name, const struct OpenAPI_Spec *spec);

/**
 * @brief Emit MCP adapters into client header and source.
 *
 * @param[in,out] hfile Output client header file stream.
 * @param[in,out] cfile Output client source file stream.
 * @param[in] guard Include guard macro for client header.
 * @param[in] prefix Function name prefix.
 * @param[in] spec Parsed OpenAPI specification.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_gen_emit_mcp(FILE *hfile, FILE *cfile, const char *guard,
                    const char *prefix, const struct OpenAPI_Spec *spec);

/**
 * @brief Emit url_utils.h for installable package.
 *
 * @param[in] dir_name Output directory path.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_gen_emit_url_utils_h(const char *dir_name);

/**
 * @brief Emit url_utils.c (part 1) for installable package.
 *
 * @param[in,out] uc Output url_utils.c file stream.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t client_gen_emit_url_utils_c1(FILE *uc);

/**
 * @brief Emit url_utils.c (part 2) for installable package.
 *
 * @param[in,out] uc Output url_utils.c file stream.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t client_gen_emit_url_utils_c2(FILE *uc);

/**
 * @brief Emit test_sdk.c mocks for testing client.
 *
 * @param[in] dir_name Output directory path.
 * @param[in] spec Parsed OpenAPI specification.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_gen_emit_mocks(const char *dir_name, const struct OpenAPI_Spec *spec);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ROUTES_EMIT_CLIENT_GEN_INTERNAL_H */
