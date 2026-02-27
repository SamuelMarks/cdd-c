/**
 * @file http_wininet.h
 * @brief WinInet implementation of the Abstract Network Interface.
 *
 * Provides functions to instantiate a transport context backed by the Microsoft
 * Windows Internet (WinInet) API. This backend handles:
 * - Request/Response lifecycle via HINTERNET handles.
 * - SSL/TLS configuration including certificate verification skipping.
 * - Connection persistence management (via InternetOpen handles).
 *
 * @note This module is only available on Windows platforms.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_HTTP_WININET_H
#define C_CDD_HTTP_WININET_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "functions/parse_http_types.h"

/**
 * @brief Initialize the global WinInet environment.
 *
 * WinInet is largely initialized per-call or implicitly by the DLL,
 * but this hook allows for future global configuration if needed.
 *
 * @return 0 on success.
 */
extern C_CDD_EXPORT int http_wininet_global_init(void);

/**
 * @brief Cleanup global WinInet environment.
 * No-op for WinInet.
 */
extern C_CDD_EXPORT void http_wininet_global_cleanup(void);

/**
 * @brief Create a new WinInet-backed transport context.
 * Opens a root HINTERNET handle using `InternetOpen`.
 *
 * @param[out] ctx Double pointer to receive the allocated context.
 * @return 0 on success, ENOMEM on allocation failure, or EIO on WinInet
 * failure.
 */
extern C_CDD_EXPORT int
http_wininet_context_init(struct HttpTransportContext **ctx);

/**
 * @brief Free the transport context.
 * Closes the root HINTERNET handle.
 *
 * @param[in] ctx The context to free. Satisfies NULL-safety.
 */
extern C_CDD_EXPORT void
http_wininet_context_free(struct HttpTransportContext *ctx);

/**
 * @brief Apply configuration settings to the WinInet context.
 *
 * - Sets timeouts via `InternetSetOption`.
 * - Caches verification flags to be applied to specific requests.
 *
 * @param[in] ctx The transport context.
 * @param[in] config The configuration to apply.
 * @return 0 on success, EIO if WinInet calls fail.
 */
extern C_CDD_EXPORT int
http_wininet_config_apply(struct HttpTransportContext *ctx,
                          const struct HttpConfig *config);

/**
 * @brief The send implementation for WinInet.
 * Matches generic `http_send_fn` signature.
 *
 * Workflow:
 * 1. Parse URL elements (Host, Path, Scheme).
 * 2. `InternetConnect` into the target host.
 * 3. `HttpOpenRequest` for the specific resource.
 * 4. Apply headers and Send Request.
 * 5. Read response stream.
 *
 * @param[in] ctx The transport context.
 * @param[in] req The request to send.
 * @param[out] res Double pointer to receive the allocated response object.
 * @return 0 on success, error code (EIO/EINVAL/ENOMEM) on failure.
 */
extern C_CDD_EXPORT int http_wininet_send(struct HttpTransportContext *ctx,
                                          const struct HttpRequest *req,
                                          struct HttpResponse **res);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_HTTP_WININET_H */
