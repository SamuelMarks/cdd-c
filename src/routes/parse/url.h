/**
 * @file url.h
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

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Represents a single key-value query parameter.
 */
struct UrlQueryParam {
  char *key;            /**< The parameter key (unencoded) */
  char *value;          /**< The parameter value (raw or pre-encoded) */
  int value_is_encoded; /**< 1 if value is already percent-encoded */
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
 * @brief Supported value types for object-style query parameters.
 */
enum OpenAPI_KVType {
  OA_KV_STRING = 0, /**< String value */
  OA_KV_INTEGER,    /**< Integer value */
  OA_KV_NUMBER,     /**< Floating-point value */
  OA_KV_BOOLEAN     /**< Boolean value (0/1) */
};

/**
 * @brief Strongly typed key/value pair for object-style query parameters.
 *
 * Used when serializing `style=form` (object) and `style=deepObject`.
 */
struct OpenAPI_KV {
  const char *key;          /**< Parameter key */
  enum OpenAPI_KVType type; /**< Value type */
  /** @brief union data */
  union {
    const char *s; /**< String value */
    int i;         /**< Integer value */
    double n;      /**< Number value */
    int b;         /**< Boolean value (0/1) */
    /** @brief KV value */
  } value;
};

/**
 * @brief Join object-style key/value pairs into a form-encoded value string.
 *
 * Produces a single string suitable for use as the value of a form-style
 * parameter when explode=false (or space/pipe-delimited object styles).
 * Keys and values are percent-encoded using form rules; the delimiter is
 * inserted as-is between tokens.
 *
 *                  "%20", "%7C").
 *                           values (except form delimiters).
 * allocation failure.
 */
extern C_CDD_EXPORT enum cdd_c_error
openapi_kv_join_form(const struct OpenAPI_KV *kvs, size_t n, const char *delim,
                     int allow_reserved, char **_out_val);

/**
 * @brief Percent-encode a string for use in a URL.
 *
 * Conforms to RFC 3986. Encodes all characters except:
 * ALPHA, DIGIT, "-", ".", "_", "~".
 * Spaces are encoded as "%20".
 *
 * error/allocation failure.
 */
extern C_CDD_EXPORT enum cdd_c_error url_encode(const char *str,
                                                char **_out_val);

/**
 * @brief Percent-encode a string while allowing reserved characters.
 *
 * Encodes all characters except RFC 3986 unreserved and reserved sets.
 * Preserves existing percent-encoded triples ("%HH") verbatim.
 * Spaces are encoded as "%20".
 *
 * error/allocation failure.
 */
extern C_CDD_EXPORT enum cdd_c_error url_encode_allow_reserved(const char *str,
                                                               char **_out_val);

/**
 * @brief Percent-encode a string for application/x-www-form-urlencoded.
 *
 * Encodes all characters except: ALPHA, DIGIT, "-", ".", "_", "*".
 * Spaces are encoded as "+".
 *
 * error/allocation failure.
 */
extern C_CDD_EXPORT enum cdd_c_error url_encode_form(const char *str,
                                                     char **_out_val);

/**
 * @brief Percent-encode a string for application/x-www-form-urlencoded while
 * allowing reserved characters (except delimiters).
 *
 * Preserves RFC3986 reserved characters except for '&', '=' and '+', which are
 * always encoded to avoid breaking form key/value delimiters. Spaces are
 * encoded as "+" and existing percent-encoded triples are preserved.
 *
 * error/allocation failure.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the url encode form allow reserved
                     * operation.
                     */
    enum cdd_c_error
    url_encode_form_allow_reserved(const char *str, char **_out_val);

/**
 * @brief Initialize a query parameters container.
 *
 */
extern C_CDD_EXPORT enum cdd_c_error url_query_init(struct UrlQueryParams *qp);

/**
 * @brief Free resources associated with a query parameters container.
 * Frees all key/value strings and the internal array.
 *
 */
extern C_CDD_EXPORT void url_query_free(struct UrlQueryParams *qp);

/**
 * @brief Add a key-value pair to the query container.
 *
 */
extern C_CDD_EXPORT enum cdd_c_error
url_query_add(struct UrlQueryParams *qp, const char *key, const char *value);

/**
 * @brief Add a key-value pair where the value is already percent-encoded.
 *
 * The value will be copied as-is and will not be encoded again during
 * url_query_build(). Use this for OpenAPI styles that require reserved
 * delimiters (e.g. comma for form-style explode=false).
 *
 */
extern C_CDD_EXPORT enum cdd_c_error
url_query_add_encoded(struct UrlQueryParams *qp, const char *key,
                      const char *value);

/**
 * @brief Build the final query string starting with '?'.
 *
 * Iterates through parameters, URL-encodes keys and values, and joins them
 * with '&'.
 * Example output: "?q=hello%20world&page=1"
 *
 *                     If count is 0, allocates an empty string "".
 */
extern C_CDD_EXPORT enum cdd_c_error
url_query_build(const struct UrlQueryParams *qp, char **out_str);

/**
 * @brief Build a application/x-www-form-urlencoded body string.
 *
 * Uses form encoding (space -> "+") and does not prefix with '?'.
 *
 *                     If count is 0, allocates an empty string "".
 */
extern C_CDD_EXPORT enum cdd_c_error
url_query_build_form(const struct UrlQueryParams *qp, char **out_str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_URL_UTILS_H */
