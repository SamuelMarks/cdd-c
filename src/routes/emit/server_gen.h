/**
 * @file server_gen.h
 * @brief Core server gen definitions and implementation.
 */
#ifndef CDD_SERVER_GEN_H
#define CDD_SERVER_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cddConfig.h"
#include "openapi/parse/openapi.h"
#include "cdd_c_error.h"
#include "routes/emit/client_gen.h" /* For OpenApiClientConfig */
/* clang-format on */

/**
 * @brief Generates a standalone C HTTP server implementation from an OpenAPI
 * specification.
 *
 * @param[in] spec The parsed OpenAPI specification.
 * @param[in] config Client/Server generator configuration.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t openapi_server_generate(
    const struct OpenAPI_Spec *spec, const struct OpenApiClientConfig *config);

/**
 * @brief Losslessly synchronizes a server C source file with an
 * OpenAPI specification.
 *
 * @param[in] filename Path to the C source file to synchronize.
 * @param[in] spec The parsed OpenAPI specification.
 * @param[in] config Client/Server generator configuration.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
openapi_server_sync_file(const char *filename, const struct OpenAPI_Spec *spec,
                         const struct OpenApiClientConfig *config);

/**
 * @brief Generates server handler signature, body stub, and
 * Doxygen docblock for an operation.
 *
 * @param[in] op The OpenAPI operation structure.
 * @param[out] out_sig Output allocated C function signature string.
 * @param[out] out_body Output allocated C function body stub string.
 * @param[out] out_doc Output allocated Doxygen comment block string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
generate_server_handler_stub(const struct OpenAPI_Operation *op, char **out_sig,
                             char **out_body, char **out_doc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_SERVER_GEN_H */
