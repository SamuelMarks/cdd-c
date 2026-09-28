/**
 * @file sync_gen.h
 * @brief Internal generator routines for API sync.
 *
 * @author Samuel Marks
 */

#ifndef CDD_ROUTES_PARSE_SYNC_GEN_H
#define CDD_ROUTES_PARSE_SYNC_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <stdio.h>

#include "cdd_c_error.h"
#include "openapi/parse/openapi.h"
#include "routes/parse/sync.h"
/* clang-format on */

/**
 * @brief Creates a temporary file in memory or system temp.
 *
 * @param[out] out_file Pointer to receive the opened temporary file.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t make_cdd_tmpfile(FILE **out_file);

/**
 * @brief Generate expected signature string for an OpenAPI operation.
 *
 * @param[in] op Operation specification.
 * @param[in] cfg Synchronization configuration.
 * @param[out] _out_val Pointer to receive allocated signature string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t generate_expected_sig(const struct OpenAPI_Operation *op,
                                    const struct ApiSyncConfig *cfg,
                                    char **_out_val);

/**
 * @brief Generate Query parameters block for an OpenAPI operation.
 *
 * @param[in] op Operation specification.
 * @param[out] _out_val Pointer to receive allocated query block string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t generate_expected_query(const struct OpenAPI_Operation *op,
                                      char **_out_val);

/**
 * @brief Generate Header parameters block line by line.
 *
 * @param[in] p Parameter specification.
 * @param[out] _out_val Pointer to receive allocated header line string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t generate_expected_header_line(const struct OpenAPI_Parameter *p,
                                            char **_out_val);

/**
 * @brief Generate URL builder logic for an OpenAPI operation.
 *
 * @param[in] path Path template string.
 * @param[in] op Operation specification.
 * @param[in] cfg Synchronization configuration.
 * @param[out] _out_val Pointer to receive allocated URL builder string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t generate_expected_url(const char *path,
                                    const struct OpenAPI_Operation *op,
                                    const struct ApiSyncConfig *cfg,
                                    char **_out_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CDD_ROUTES_PARSE_SYNC_GEN_H */
