/**
 * @file url_utils.h
 * @brief Utilities for URL encoding and Query String construction.
 *
 * Provides functionality to:
 * - Percent-encode strings according to RFC 3986 (safe for Path and Query).
 * - Manage a collection of Query Parameters.
 * - Serialize parameters into a valid query string (e.g., "?key=val&k2=v2").
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_URL_UTILS_H
#define C_CDD_URL_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include <stddef.h>

/**
 * @brief Represents a single key-value query parameter.
 */
struct UrlQueryParam {
  char *key;   /**< The parameter key (unencoded) */
  char *value; /**< The parameter value (unencoded) */
};

/**
 * @brief Container for a list of query parameters.
 */
struct UrlQueryParams {
  struct UrlQueryParam *params; /**< Dynamic array of parameters */
  size_t count;                 /**< Number of items used */
  size_t capacity;              /**< Current allocated capacity */
};

/**
 * @brief Percent-encode a string for use in a URL.
 *
 * Conforms to RFC 3986. Encodes all characters except:
 * ALPHA, DIGIT, "-", ".", "_", "~".
 * Spaces are encoded as "%20".
 *
 * @param[in] str The null-terminated string to encode.
 * @return A newly allocated string containing the encoded result, or NULL on
 * error/allocation failure.
 */
extern C_CDD_EXPORT char *url_encode(const char *str);

/**
 * @brief Initialize a query parameters container.
 *
 * @param[out] qp The structure to initialize.
 * @return 0 on success, EINVAL if qp is NULL.
 */
extern C_CDD_EXPORT int url_query_init(struct UrlQueryParams *qp);

/**
 * @brief Free resources associated with a query parameters container.
 * Frees all key/value strings and the internal array.
 *
 * @param[in] qp The structure to free. Safe to pass NULL.
 */
extern C_CDD_EXPORT void url_query_free(struct UrlQueryParams *qp);

/**
 * @brief Add a key-value pair to the query container.
 *
 * @param[in] qp The container.
 * @param[in] key The parameter key (will be copied).
 * @param[in] value The parameter value (will be copied).
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on invalid args.
 */
extern C_CDD_EXPORT int url_query_add(struct UrlQueryParams *qp,
                                      const char *key, const char *value);

/**
 * @brief Build the final query string starting with '?'.
 *
 * Iterates through parameters, URL-encodes keys and values, and joins them
 * with '&'.
 * Example output: "?q=hello%20world&page=1"
 *
 * @param[in] qp The container describing the parameters.
 * @param[out] out_str Pointer to a char* where the result will be allocated.
 *                     If count is 0, allocates an empty string "".
 * @return 0 on success, ENOMEM on allocation failure.
 */
extern C_CDD_EXPORT int url_query_build(const struct UrlQueryParams *qp,
                                        char **out_str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_URL_UTILS_H */
