/**
 * @file http_winhttp.h
 * @brief WinHTTP implementation of the Abstract Network Interface.
 *
 * Provides functions to instantiate a transport context backed by the Windows
 * HTTP Services (WinHTTP) API. This backend handles:
 * - Session caching.
 * - Secure Sockets Layer (SSL) configuration mapping.
 * - Proxy settings and timeouts.
 * - Automatic Unicode conversions.
 *
 * @note This module is only available on Windows platforms.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_HTTP_WINHTTP_H
#define C_CDD_HTTP_WINHTTP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "functions/parse_http_types.h"

/**
 * @brief Initialize the global WinHTTP environment.
 * Usually a no-op as WinHTTP initializes lazily, but kept for interface
 * consistency.
 *
 * @return 0 on success.
 */
extern C_CDD_EXPORT int http_winhttp_global_init(void);

/**
 * @brief Cleanup global WinHTTP environment.
 * No-op for WinHTTP.
 */
extern C_CDD_EXPORT void http_winhttp_global_cleanup(void);

/**
 * @brief Create a new WinHTTP-backed transport context.
 * Opens a WinHTTP Session handle using `WinHttpOpen`.
 *
 * @param[out] ctx Double pointer to receive the allocated context.
 * @return 0 on success, ENOMEM on allocation failure, or EIO on WinHTTP
 * failure.
 */
extern C_CDD_EXPORT int
http_winhttp_context_init(struct HttpTransportContext **ctx);

/**
 * @brief Free the transport context.
 * Closes the internal WinHTTP Session handle.
 *
 * @param[in] ctx The context to free. Satisfies NULL-safety.
 */
extern C_CDD_EXPORT void
http_winhttp_context_free(struct HttpTransportContext *ctx);

/**
 * @brief Apply configuration settings to the WinHTTP session.
 *
 * Updates the session handle with:
 * - Timeouts (Resolve, Connect, Send, Receive) via `WinHttpSetTimeouts`.
 * - Proxy settings via `WinHttpSetOption`.
 * - Caches Security flags (SSL verification) internally to be applied
 *   during `send` (since they apply to Request handles, not Session handles).
 *
 * @param[in] ctx The transport context.
 * @param[in] config The configuration to apply.
 * @return 0 on success, EIO if WinHTTP calls fail.
 */
extern C_CDD_EXPORT int
http_winhttp_config_apply(struct HttpTransportContext *ctx,
                          const struct HttpConfig *config);

/**
 * @brief The send implementation for WinHTTP.
 * Matches generic `http_send_fn` signature.
 *
 * Workflow:
 * 1. URL Cracking (Host/Port/Path separation).
 * 2. UTF-8 to UTF-16 conversions for all strings.
 * 3. Handle hierarchy creation: Session -> Connect -> Request.
 * 4. Application of cached security flags (Ignore Cert Errors).
 * 5. Header aggregation.
 * 6. Synchronous transmission.
 * 7. Response reading and transcoding (UTF-16 -> UTF-8).
 *
 * @param[in] ctx The transport context.
 * @param[in] req The request to send.
 * @param[out] res Double pointer to receive the allocated response object.
 * @return 0 on success, error code (EIO/EINVAL/ENOMEM) on failure.
 */
extern C_CDD_EXPORT int http_winhttp_send(struct HttpTransportContext *ctx,
                                          const struct HttpRequest *req,
                                          struct HttpResponse **res);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_HTTP_WINHTTP_H */
