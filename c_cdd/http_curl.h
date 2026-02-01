/**
 * @file http_curl.h
 * @brief Libcurl implementation of the Abstract Network Interface (ANI).
 *
 * Provides functions to instantiate a transport context backed by libcurl.
 * Handles the mapping between the generic `HttpClient`/`HttpRequest` structures
 * and the specific `CURL` handle options.
 *
 * Includes support for:
 * - Reference-counted global initialization.
 * - Configuration application (Timeouts, SSL, Proxy).
 * - Comprehensive error code mapping.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_HTTP_CURL_H
#define C_CDD_HTTP_CURL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "http_types.h"

/**
 * @brief Initialize the global curl environment safely.
 *
 * Uses an internal reference counter to ensure `curl_global_init` is called
 * called exactly once on the first invocation, and `curl_global_cleanup` only
 * on the last. This allows multiple independent clients to initialize/cleanup
 * without race conditions (assuming single-threaded startup or external
 * locking, as C89 lacks mutexes).
 *
 * @return 0 on success, EIO on failure.
 */
extern C_CDD_EXPORT int http_curl_global_init(void);

/**
 * @brief Decrement the global initialization reference count.
 *
 * If the count reaches zero, `curl_global_cleanup` is invoked.
 */
extern C_CDD_EXPORT void http_curl_global_cleanup(void);

/**
 * @brief Create a new Curl-backed transport context.
 * Internal allocates a CURL handle for reuse.
 *
 * @param[out] ctx Double pointer to receive the allocated context.
 * @return 0 on success, ENOMEM on allocation failure, EIO on Curl init failure.
 */
extern C_CDD_EXPORT int
http_curl_context_init(struct HttpTransportContext **ctx);

/**
 * @brief Free the transport context.
 * Cleans up the internal CURL handle and any cached configuration.
 *
 * @param[in] ctx The context to free. Safe to pass NULL.
 */
extern C_CDD_EXPORT void
http_curl_context_free(struct HttpTransportContext *ctx);

/**
 * @brief Apply configuration settings to the CURL handle.
 *
 * Maps abstract `HttpConfig` settings (timeout, verify peer, proxy, user-agent)
 * to specific `curl_easy_setopt` calls. This should be called after
 * context_init and before sending requests if custom settings are required.
 *
 * @param[in] ctx The transport context.
 * @param[in] config The configuration structure to apply.
 * @return 0 on success, EINVAL if inputs invalid, or EIO if setopt fails.
 */
extern C_CDD_EXPORT int http_curl_config_apply(struct HttpTransportContext *ctx,
                                               const struct HttpConfig *config);

/**
 * @brief The send implementation for libcurl.
 * Matches `http_send_fn` signature.
 *
 * Performs the HTTP request using the stored CURL handle.
 * 1. Resets reusable handle state (except sticky options like connection
 * cache).
 * 2. Sets Method, URL, Body, and Headers from `req`.
 * 3. Executes request (`curl_easy_perform`).
 * 4. Captures Response Code and Body.
 * 5. Maps CURL errors to system `errno` codes.
 *
 * @param[in] ctx The transport context.
 * @param[in] req The request to send.
 * @param[out] res Double pointer to receive the newly allocated response
 * object.
 * @return 0 on success, or a mapped error code (e.g. ETIMEDOUT, ECONNREFUSED)
 * on failure.
 */
extern C_CDD_EXPORT int http_curl_send(struct HttpTransportContext *ctx,
                                       const struct HttpRequest *req,
                                       struct HttpResponse **res);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_HTTP_CURL_H */
