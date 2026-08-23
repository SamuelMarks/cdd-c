#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
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

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CDD_SERVER_GEN_H */

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
