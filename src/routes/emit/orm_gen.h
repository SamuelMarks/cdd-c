#ifndef C_CDD_ROUTES_EMIT_ORM_GEN_H
#define C_CDD_ROUTES_EMIT_ORM_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "openapi/parse/openapi.h"
#include "routes/emit/client_gen.h" /* For config */
/* clang-format on */

/**
 * @brief Emit c-orm compatible database schema models and C structures.
 *
 * @param[in] spec The parsed OpenAPI specification.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the openapi orm generate operation.
                     */
    int
    openapi_orm_generate(const struct OpenAPI_Spec *spec,
                         const struct OpenApiClientConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ROUTES_EMIT_ORM_GEN_H */
