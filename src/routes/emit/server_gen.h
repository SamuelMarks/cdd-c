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

C_CDD_EXPORT /**
              * @brief Executes the openapi server generate operation.
              */
    cdd_c_error_t
    openapi_server_generate(const struct OpenAPI_Spec *spec,
                            const struct OpenApiClientConfig *config);

C_CDD_EXPORT /**
              * @brief Losslessly synchronizes a server C source file with an
              * OpenAPI specification.
              */
    cdd_c_error_t
    openapi_server_sync_file(const char *filename,
                             const struct OpenAPI_Spec *spec,
                             const struct OpenApiClientConfig *config);

C_CDD_EXPORT /**
              * @brief Generates server handler signature, body stub, and
              * Doxygen docblock for an operation.
              */
    cdd_c_error_t
    generate_server_handler_stub(const struct OpenAPI_Operation *op,
                                 char **out_sig, char **out_body,
                                 char **out_doc);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CDD_SERVER_GEN_H */
