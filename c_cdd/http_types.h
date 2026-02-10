/**
 * @file http_types.h
 * @brief Abstract Network Interface (ANI) definitions.
 *
 * Defines core structures for HTTP communication, configuration (retries),
 * and Multipart/Form-Data support.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_HTTP_TYPES_H
#define C_CDD_HTTP_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>

#include "c_cdd_export.h"

/**
 * @brief HTTP Method verbs.
 */
enum HttpMethod {
  HTTP_GET,
  HTTP_POST,
  HTTP_PUT,
  HTTP_DELETE,
  HTTP_PATCH,
  HTTP_HEAD,
  HTTP_OPTIONS,
  HTTP_TRACE,
  HTTP_QUERY,
  HTTP_CONNECT
};

/**
 * @brief Retry Policy Enum.
 */
enum HttpRetryPolicy {
  HTTP_RETRY_NONE,       /**< No retries (default) */
  HTTP_RETRY_LINEAR,     /**< Fixed delay between retries */
  HTTP_RETRY_EXPONENTIAL /**< Exponential backoff */
};

/**
 * @brief Represents a single HTTP header.
 */
struct HttpHeader {
  char *key;   /**< Header name (allocated) */
  char *value; /**< Header value (allocated) */
};

/**
 * @brief Container for HTTP headers.
 */
struct HttpHeaders {
  struct HttpHeader *headers; /**< Dynamic array of headers */
  size_t count;               /**< Number of headers used */
  size_t capacity;            /**< Current capacity */
};

/**
 * @brief Represents a single part in a Multipart request.
 */
struct HttpPart {
  char *name;         /**< Form field name */
  char *filename;     /**< Filename (optional, implies file upload) */
  char *content_type; /**< Content-Type of the part (e.g. "application/json"),
                         optional */
  void *data;         /**< Pointer to data buffer */
  size_t data_len;    /**< Length of data buffer */
};

/**
 * @brief Container for Multipart parts.
 */
struct HttpParts {
  struct HttpPart *parts; /**< Dynamic array of parts */
  size_t count;           /**< Number of parts used */
  size_t capacity;        /**< Current capacity */
};

/**
 * @brief Represents an outgoing HTTP request.
 */
struct HttpRequest {
  char *url;                  /**< Full destination URL */
  enum HttpMethod method;     /**< HTTP Verb */
  struct HttpHeaders headers; /**< Request headers */
  void *body; /**< Raw body payload (mutually exclusive with parts generally,
                 but flattened parts end up here) */
  size_t body_len;        /**< Length of raw body */
  struct HttpParts parts; /**< Multipart segments (if any) */
};

/**
 * @brief Represents an incoming HTTP response.
 */
struct HttpResponse {
  int status_code;            /**< HTTP Status Code (e.g. 200, 404) */
  struct HttpHeaders headers; /**< Response Headers */
  void *body;                 /**< Response Body Payload */
  size_t body_len;            /**< Length of Response Body */
};

/**
 * @brief Configuration settings for the HTTP Client.
 */
struct HttpConfig {
  long timeout_ms;  /**< Timeout in milliseconds */
  int verify_peer;  /**< 1 to verify SSL peer, 0 to ignore */
  int verify_host;  /**< 1 to verify SSL host, 0 to ignore */
  char *user_agent; /**< Custom User-Agent string */
  char *proxy_url;  /**< Proxy URL (e.g. "http://10.0.0.1:8080") */
  int retry_count;  /**< Max retries on failure (default 0) */
  enum HttpRetryPolicy retry_policy; /**< Backoff strategy */
};

struct HttpTransportContext;

/**
 * @brief Function pointer signature for the transport send method.
 */
typedef int (*http_send_fn)(struct HttpTransportContext *ctx,
                            const struct HttpRequest *req,
                            struct HttpResponse **res);

/**
 * @brief High-level client context.
 */
struct HttpClient {
  struct HttpTransportContext
      *transport;    /**< Backend-specific context (CURL*, HINTERNET, etc.) */
  http_send_fn send; /**< Function pointer to execute requests */
  char *base_url;    /**< Base URL for API calls */
  struct HttpConfig config; /**< Client configuration */
};

/* --- Lifecycle Management --- */

extern C_CDD_EXPORT int http_headers_init(struct HttpHeaders *headers);
extern C_CDD_EXPORT void http_headers_free(struct HttpHeaders *headers);
extern C_CDD_EXPORT int http_headers_add(struct HttpHeaders *headers,
                                         const char *key, const char *value);

extern C_CDD_EXPORT int http_config_init(struct HttpConfig *config);
extern C_CDD_EXPORT void http_config_free(struct HttpConfig *config);

extern C_CDD_EXPORT int http_client_init(struct HttpClient *client);
extern C_CDD_EXPORT void http_client_free(struct HttpClient *client);

extern C_CDD_EXPORT int http_request_init(struct HttpRequest *req);
extern C_CDD_EXPORT void http_request_free(struct HttpRequest *req);
extern C_CDD_EXPORT int http_request_set_auth_bearer(struct HttpRequest *req,
                                                     const char *token);

extern C_CDD_EXPORT int http_response_init(struct HttpResponse *res);
extern C_CDD_EXPORT void http_response_free(struct HttpResponse *res);
extern C_CDD_EXPORT int
http_response_save_to_file(const struct HttpResponse *res, const char *path);

/* --- Multipart Management --- */

/**
 * @brief Initialize parts container.
 * @param[out] parts Container to init.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int http_parts_init(struct HttpParts *parts);

/**
 * @brief Free parts container contents.
 * @param[in] parts Container to clean.
 */
extern C_CDD_EXPORT void http_parts_free(struct HttpParts *parts);

/**
 * @brief Add a part to the request.
 *
 * @param[in,out] req The request object.
 * @param[in] name Field name.
 * @param[in] filename Optional filename (NULL for text fields).
 * @param[in] content_type Optional MIME type (NULL autodetects/defaults).
 * @param[in] data Pointer to payload.
 * @param[in] data_len Length of payload.
 * @return 0 on success, ENOMEM on failure.
 */
extern C_CDD_EXPORT int
http_request_add_part(struct HttpRequest *req, const char *name,
                      const char *filename, const char *content_type,
                      const void *data, size_t data_len);

/**
 * @brief Flatten parts into a single multipart/form-data body buffer.
 *
 * Generates the boundary, headers, and payload for all parts, concatenates them
 * into `req->body`, and sets the `Content-Type` header on `req`.
 * Used by transport layers that lack native multipart support or for
 * consistency.
 *
 * @param[in,out] req The request structure.
 * @return 0 on success, ENOMEM on allocation failure.
 */
extern C_CDD_EXPORT int http_request_flatten_parts(struct HttpRequest *req);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_HTTP_TYPES_H */
