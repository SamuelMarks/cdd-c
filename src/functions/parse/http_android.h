/**
 * @file http_android.h
 * @brief Android JNI implementation of the Abstract Network Interface.
 *
 * Provides functions to instantiate a transport context backed by the Android
 * Java framework (`java.net.HttpURLConnection`) via JNI.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_HTTP_ANDROID_H
#define C_CDD_HTTP_ANDROID_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "functions/parse/http_types.h"

/**
 * @brief Initialize the global Android JNI environment.
 *
 * Configures the JVM handle for subsequent calls.
 *
 * @return 0 on success.
 */
extern C_CDD_EXPORT int http_android_global_init(void);

/**
 * @brief Cleanup global Android environment.
 *
 * Releases any globally cached Java classes or references.
 */
extern C_CDD_EXPORT void http_android_global_cleanup(void);

/**
 * @brief Create a new Android-backed transport context.
 *
 * @param[out] ctx Double pointer to receive the allocated context.
 * @return 0 on success, ENOMEM on allocation failure.
 */
extern C_CDD_EXPORT int
http_android_context_init(struct HttpTransportContext **ctx);

/**
 * @brief Free the transport context.
 *
 * @param[in] ctx The context to free. Satisfies NULL-safety.
 */
extern C_CDD_EXPORT void
http_android_context_free(struct HttpTransportContext *ctx);

/**
 * @brief Apply configuration settings to the Android context.
 *
 * @param[in] ctx The transport context.
 * @param[in] config The configuration to apply.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
http_android_config_apply(struct HttpTransportContext *ctx,
                          const struct HttpConfig *config);

/**
 * @brief The send implementation for Android.
 * Matches generic `http_send_fn` signature.
 *
 * @param[in] ctx The transport context.
 * @param[in] req The request to send.
 * @param[out] res Double pointer to receive the allocated response object.
 * @return 0 on success, error code (EIO/EINVAL/ENOMEM) on failure.
 */
extern C_CDD_EXPORT int http_android_send(struct HttpTransportContext *ctx,
                                          const struct HttpRequest *req,
                                          struct HttpResponse **res);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_HTTP_ANDROID_H */