/**
 * @file cli_internal.h
 * @brief Internal declarations for CLI routes parsing.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_ROUTES_PARSE_CLI_INTERNAL_H
#define C_CDD_ROUTES_PARSE_CLI_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include <stddef.h>

#include "c_cdd/memory.h"
#include "cdd_c_error.h"
#include "openapi/parse/openapi.h"
#include "routes/parse/cli.h"
/* clang-format on */

struct CstNode;
struct TokenList;

/**
 * @brief Sets a dynamically allocated string if destination pointer is NULL.
 *
 * @param[in,out] dst Destination string pointer.
 * @param[in] src Source string to duplicate if dst is NULL.
 * @return CDD_C_SUCCESS on success or error code.
 */
cdd_c_error_t set_str_if_missing(char **dst, const char *src);

/**
 * @brief Finds a security scheme by name in an OpenAPI specification.
 *
 * @param[in] spec Target OpenAPI specification.
 * @param[in] name Name of security scheme to find.
 * @param[out] out_val Pointer to receive matched scheme pointer.
 * @return CDD_C_SUCCESS on success or CDD_C_ERROR_UNKNOWN if not found.
 */
cdd_c_error_t
spec_find_security_scheme(struct OpenAPI_Spec *spec, const char *name,
                          struct OpenAPI_SecurityScheme **out_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
 * @brief Test helper for c2openapi_infer_client_route.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_c2openapi_infer_client_route(
    const struct CstNode *func_node, const struct TokenList *tokens,
    char **out_route);

/**
 * @brief Test helper for c2openapi_scan_server_routes.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_c2openapi_scan_server_routes(
    const struct TokenList *tokens, struct OpenAPI_Spec *spec);

/**
 * @brief Test helper for c2openapi_scan_gui_views.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_c2openapi_scan_gui_views(
    const struct TokenList *tokens, struct OpenAPI_Spec *spec);

#endif /* C_CDD_ROUTES_PARSE_CLI_INTERNAL_H */
