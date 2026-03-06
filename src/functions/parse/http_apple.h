/**
 * @file http_apple.h
 * @brief Apple CFNetwork implementation of the Abstract Network Interface.
 *
 * Implements the HTTP transport using Apple's CFNetwork framework.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_HTTP_APPLE_H
#define C_CDD_HTTP_APPLE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "functions/parse/http_types.h"

/**
 * @brief Initialize global Apple networking environment.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int http_apple_global_init(void);

/**
 * @brief Cleanup global Apple networking environment.
 */
extern C_CDD_EXPORT void http_apple_global_cleanup(void);

/**
 * @brief Create a new Apple-backed transport context.
 *
 * @param[out] ctx Double pointer to receive the allocated context.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int http_apple_context_init(struct HttpTransportContext **ctx);

/**
 * @brief Free the transport context.
 *
 * @param[in] ctx The context to free. Satisfies NULL-safety.
 */
extern C_CDD_EXPORT void http_apple_context_free(struct HttpTransportContext *ctx);

/**
 * @brief Apply configuration settings to the Apple session.
 *
 * @param[in] ctx The transport context.
 * @param[in] config The configuration to apply.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int http_apple_config_apply(struct HttpTransportContext *ctx,
                                                const struct HttpConfig *config);

/**
 * @brief The send implementation for Apple.
 *
 * @param[in] ctx The transport context.
 * @param[in] req The request to send.
 * @param[out] res Double pointer to receive the allocated response object.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int http_apple_send(struct HttpTransportContext *ctx,
                                        const struct HttpRequest *req,
                                        struct HttpResponse **res);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_HTTP_APPLE_H */
