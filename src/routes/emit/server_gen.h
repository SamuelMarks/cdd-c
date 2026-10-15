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

C_CDD_EXPORT cdd_c_error_t openapi_server_generate(
    const struct OpenAPI_Spec *spec, const struct OpenApiClientConfig *config);

/**
 * @brief Losslessly synchronizes a server C source file with an
 * OpenAPI specification.
 */
C_CDD_EXPORT cdd_c_error_t
openapi_server_sync_file(const char *filename, const struct OpenAPI_Spec *spec,
                         const struct OpenApiClientConfig *config);

/**
 * @brief Generates server handler signature, body stub, and
 * Doxygen docblock for an operation.
 */
C_CDD_EXPORT cdd_c_error_t
generate_server_handler_stub(const struct OpenAPI_Operation *op, char **out_sig,
                             char **out_body, char **out_doc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_SERVER_GEN_H */
