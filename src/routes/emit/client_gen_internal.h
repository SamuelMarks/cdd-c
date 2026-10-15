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
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "routes/emit/client_gen.h"
#include <stdio.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
#define fprintf cdd_io_fprintf_hook
#define fputs cdd_io_fputs_hook
#endif /* CDD_BUILD_TESTS */

/**
 * @def CHECK_IO_CLEANUP
 * @brief CHECK_IO_CLEANUP macro
 */
#define CHECK_IO_CLEANUP(x)                                                    \
  for (; (x) < 0;) {                                                           \
    rc = CDD_C_SUCCESS;                                                        \
    fprintf(stderr, "goto cleanup at %s:%d\n", __FILE__, __LINE__);            \
    goto cleanup;                                                              \
  }

/**
 * @def CHECK_IO
 * @brief CHECK_IO macro
 */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return 0

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

/**
 * @brief Emit OpenAPI models into models header and source.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_gen_emit_models(FILE *mhfile, FILE *mcfile, const char *model_guard,
                       const char *mh_name, const struct OpenAPI_Spec *spec);

/**
 * @brief Emit MCP adapters into client header and source.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_gen_emit_mcp(FILE *hfile, FILE *cfile, const char *guard,
                    const char *prefix, const struct OpenAPI_Spec *spec);

/**
 * @brief Emit url_utils.h for installable package.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_gen_emit_url_utils_h(const char *dir_name);

/**
 * @brief Emit url_utils.c (part 1) for installable package.
 */
extern C_CDD_EXPORT cdd_c_error_t client_gen_emit_url_utils_c1(FILE *uc);

/**
 * @brief Emit url_utils.c (part 2) for installable package.
 */
extern C_CDD_EXPORT cdd_c_error_t client_gen_emit_url_utils_c2(FILE *uc);

/**
 * @brief Emit test_sdk.c mocks for testing client.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_gen_emit_mocks(const char *dir_name, const struct OpenAPI_Spec *spec);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ROUTES_EMIT_CLIENT_GEN_INTERNAL_H */
