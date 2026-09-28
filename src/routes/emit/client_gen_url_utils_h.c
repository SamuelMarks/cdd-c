/**
 * @file client_gen_url_utils_h.c
 * @brief url_utils.h emission for OpenAPI client generator.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "routes/emit/client_gen_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Emit url_utils.h for installable package.
 */
cdd_c_error_t client_gen_emit_url_utils_h(const char *dir_name) {
  char hpath[512];
  FILE *uh = NULL;

  CDD_SNPRINTF(hpath, sizeof(hpath), "%s/src/url_utils.h", dir_name);
#if defined(_MSC_VER)
  if (fopen_s(&uh, hpath, "w") != 0)
    uh = NULL;
#else
  uh = fopen(hpath, "w");
#endif
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 54) {
    fclose(uh);
    uh = NULL;
  }
#endif
  if (uh) {
    fputs("/**\n"
          " * @file url.h\n"
          " * @brief Utilities for URL encoding and Query String "
          "construction.\n"
          " *\n"
          " * Provides functionality to:\n"
          " * - Percent-encode strings according to RFC 3986 (safe for Path "
          "and Query).\n"
          " * - Manage a collection of Query Parameters.\n"
          " * - Serialize parameters into a valid query string (e.g., "
          "\"?key=val&k2=v2\").\n"
          " *\n"
          " * @author Samuel Marks\n"
          " */\n"
          "\n",
          uh);
    fputs("#ifndef C_CDD_URL_UTILS_H\n"
          "#define C_CDD_URL_UTILS_H\n"
          "\n"
          "#ifdef __cplusplus\n"
          "extern \"C\" {\n"
          "#endif /* __cplusplus */\n"
          "\n"
          "/* clang-format "
          "off */\n"
          "\n"
          "#include <stddef.h>\n"
          "/* clang-format "
          "on */\n"
          "\n"
          "/**\n"
          " * @brief Represents a single key-value query parameter.\n"
          " */\n"
          "struct UrlQueryParam {\n"
          "  char *key;            /**< The parameter key (unencoded) */\n",
          uh);
    fputs("  char *value;          /**< The parameter value (raw or "
          "pre-encoded) */\n"
          "  int value_is_encoded; /**< 1 if value is already percent-encoded "
          "*/\n"
          "};\n"
          "\n"
          "/**\n"
          " * @brief Container for a list of query parameters.\n"
          " */\n"
          "struct UrlQueryParams {\n"
          "  struct UrlQueryParam *params; /**< Dynamic array of parameters "
          "*/\n"
          "  size_t count;                 /**< Number of items used */\n",
          uh);
    fputs("  size_t capacity;              /**< Current allocated capacity */\n"
          "};\n"
          "\n"
          "/**\n"
          " * @brief Supported value types for object-style query parameters.\n"
          " */\n"
          "enum OpenAPI_KVType {\n"
          "  OA_KV_STRING = 0, /**< String value */\n"
          "  OA_KV_INTEGER,    /**< Integer value */\n"
          "  OA_KV_NUMBER,     /**< Floating-point value */\n"
          "  OA_KV_BOOLEAN     /**< Boolean value (0/1) */\n"
          "};\n"
          "\n"
          "/**\n",
          uh);
    fputs(" * @brief Strongly typed key/value pair for object-style query "
          "parameters.\n"
          " *\n"
          " * Used when serializing `style=form` (object) and "
          "`style=deepObject`.\n"
          " */\n"
          "struct OpenAPI_KV {\n"
          "  const char *key;          /**< Parameter key */\n"
          "  enum OpenAPI_KVType type; /**< Value type */\n"
          "  /** @brief union data */\n"
          "  union {\n"
          "    const char *s; /**< String value */\n",
          uh);
    fputs("    int i;         /**< Integer value */\n"
          "    double n;      /**< Number value */\n"
          "    int b;         /**< Boolean value (0/1) */\n"
          "    /** @brief KV value */\n"
          "  } value;\n"
          "};\n"
          "\n"
          "/**\n"
          " * @param[out] _out_val Pointer to store the result\n"
          " * @brief Join object-style key/value pairs into a form-encoded "
          "value string.\n"
          " *\n",
          uh);
    fputs(" * Produces a single string suitable for use as the value of a "
          "form-style\n"
          " * parameter when explode=false (or space/pipe-delimited object "
          "styles).\n"
          " * Keys and values are percent-encoded using form rules; the "
          "delimiter is\n"
          " * inserted as-is between tokens.\n"
          " *\n"
          " * @param[in] kvs The key/value array.\n"
          " * @param[in] n Number of entries in kvs.\n",
          uh);
    fputs(" * @param[in] delim Delimiter string to insert between tokens "
          "(e.g., \",\",\n"
          " *                  \"%20\", \"%7C\").\n"
          " * @param[in] allow_reserved If non-zero, preserve reserved "
          "characters in\n"
          " *                           values (except form delimiters).\n"
          " * @return Newly allocated string containing the joined value, or "
          "NULL on\n"
          " * allocation failure.\n"
          " */\n",
          uh);
    fputs("extern  cdd_c_error_t openapi_kv_join_form(const struct "
          "OpenAPI_KV *kvs,\n"
          "                                             size_t n, const char "
          "*delim,\n"
          "                                             int allow_reserved,\n"
          "                                             char **_out_val);\n"
          "\n"
          "/**\n"
          " * @param[out] _out_val Pointer to store the result\n",
          uh);
    fputs(" * @brief Percent-encode a string for use in a URL.\n"
          " *\n"
          " * Conforms to RFC 3986. Encodes all characters except:\n"
          " * ALPHA, DIGIT, \"-\", \".\", \"_\", \"~\".\n"
          " * Spaces are encoded as \"%20\".\n"
          " *\n"
          " * @param[in] str The null-terminated string to encode.\n"
          " * @return A newly allocated string containing the encoded result, "
          "or NULL on\n"
          " * error/allocation failure.\n"
          " */\n",
          uh);
    fputs("extern  cdd_c_error_t url_encode(const char *str, char "
          "**_out_val);\n"
          "\n"
          "/**\n"
          " * @param[out] _out_val Pointer to store the result\n"
          " * @brief Percent-encode a string while allowing reserved "
          "characters.\n"
          " *\n"
          " * Encodes all characters except RFC 3986 unreserved and reserved "
          "sets.\n"
          " * Preserves existing percent-encoded triples (\"%HH\") verbatim.\n",
          uh);
    fputs(" * Spaces are encoded as \"%20\".\n"
          " *\n"
          " * @param[in] str The null-terminated string to encode.\n"
          " * @return A newly allocated string containing the encoded result, "
          "or NULL on\n"
          " * error/allocation failure.\n"
          " */\n"
          "extern  cdd_c_error_t url_encode_allow_reserved(const char "
          "*str,\n"
          "                                                  char "
          "**_out_val);\n"
          "\n"
          "/**\n",
          uh);
    fputs(" * @param[out] _out_val Pointer to store the result\n"
          " * @brief Percent-encode a string for "
          "application/x-www-form-urlencoded.\n"
          " *\n"
          " * Encodes all characters except: ALPHA, DIGIT, \"-\", \".\", "
          "\"_\", \"*\".\n"
          " * Spaces are encoded as \"+\".\n"
          " *\n"
          " * @param[in] str The null-terminated string to encode.\n"
          " * @return A newly allocated string containing the encoded result, ",
          uh);
    fputs("or NULL on\n"
          " * error/allocation failure.\n"
          " */\n"
          "extern  cdd_c_error_t url_encode_form(const char *str, char "
          "**_out_val);\n"
          "\n"
          "/**\n"
          " * @param[out] _out_val Pointer to store the result\n"
          " * @brief Percent-encode a string for "
          "application/x-www-form-urlencoded while\n"
          " * allowing reserved characters (except delimiters).\n"
          " *\n",
          uh);
    fputs(" * Preserves RFC3986 reserved characters except for '&', '=' and "
          "'+', which are\n"
          " * always encoded to avoid breaking form key/value delimiters. "
          "Spaces are\n"
          " * encoded as \"+\" and existing percent-encoded triples are "
          "preserved.\n"
          " *\n"
          " * @param[in] str The null-terminated string to encode.\n"
          " * @return A newly allocated string containing the encoded result, "
          "or NULL on\n",
          uh);
    fputs(" * error/allocation failure.\n"
          " */\n"
          "extern  /**\n"
          "                     * @brief Executes the url encode form allow "
          "reserved\n"
          "                     * operation.\n"
          "                     */\n"
          "    int\n"
          "    url_encode_form_allow_reserved(const char *str, char "
          "**_out_val);\n"
          "\n"
          "/**\n"
          " * @brief Initialize a query parameters container.\n"
          " *\n",
          uh);
    fputs(" * @param[out] qp The structure to initialize.\n"
          " * @return 0 on success, EINVAL if qp is NULL.\n"
          " */\n"
          "extern  cdd_c_error_t url_query_init(struct UrlQueryParams "
          "*qp);\n"
          "\n"
          "/**\n"
          " * @brief Free resources associated with a query parameters "
          "container.\n"
          " * Frees all key/value strings and the internal array.\n"
          " *\n"
          " * @param[in] qp The structure to free. Safe to pass NULL.\n"
          " */\n",
          uh);
    fputs("extern  void url_query_free(struct UrlQueryParams *qp);\n"
          "\n"
          "/**\n"
          " * @brief Add a key-value pair to the query container.\n"
          " *\n"
          " * @param[in] qp The container.\n"
          " * @param[in] key The parameter key (will be copied).\n"
          " * @param[in] value The parameter value (will be copied).\n"
          " * @return 0 on success, ENOMEM on allocation failure, EINVAL on "
          "invalid args.\n"
          " */\n",
          uh);
    fputs("extern  cdd_c_error_t url_query_add(struct UrlQueryParams *qp,\n"
          "                                      const char *key, const char "
          "*value);\n"
          "\n"
          "/**\n"
          " * @brief Add a key-value pair where the value is already "
          "percent-encoded.\n"
          " *\n"
          " * The value will be copied as-is and will not be encoded again "
          "during\n"
          " * url_query_build(). Use this for OpenAPI styles that require "
          "reserved\n",
          uh);
    fputs(" * delimiters (e.g. comma for form-style explode=false).\n"
          " *\n"
          " * @param[in] qp The container.\n"
          " * @param[in] key The parameter key (will be copied and encoded on "
          "build).\n"
          " * @param[in] value The parameter value (already encoded, will be "
          "copied).\n"
          " * @return 0 on success, ENOMEM on allocation failure, EINVAL on "
          "invalid args.\n"
          " */\n",
          uh);
    fputs("extern  cdd_c_error_t url_query_add_encoded(struct "
          "UrlQueryParams *qp,\n"
          "                                              const char *key,\n"
          "                                              const char *value);\n"
          "\n"
          "/**\n"
          " * @brief Build the final query string starting with '?'.\n"
          " *\n"
          " * Iterates through parameters, URL-encodes keys and values, and "
          "joins them\n"
          " * with '&'.\n",
          uh);
    fputs(" * Example output: \"?q=hello%20world&page=1\"\n"
          " *\n"
          " * @param[in] qp The container describing the parameters.\n"
          " * @param[out] out_str Pointer to a char* where the result will be "
          "allocated.\n"
          " *                     If count is 0, allocates an empty string "
          "\"\".\n"
          " * @return 0 on success, ENOMEM on allocation failure.\n"
          " */\n"
          "extern  cdd_c_error_t url_query_build(const struct ",
          uh);
    fputs("UrlQueryParams *qp,\n"
          "                                        char **out_str);\n"
          "\n"
          "/**\n"
          " * @brief Build a application/x-www-form-urlencoded body string.\n"
          " *\n"
          " * Uses form encoding (space -> \"+\") and does not prefix with "
          "'?'.\n"
          " *\n"
          " * @param[in] qp The container describing the parameters.\n"
          " * @param[out] out_str Pointer to a char* where the result will be "
          "allocated.\n",
          uh);
    fputs(" *                     If count is 0, allocates an empty string "
          "\"\".\n"
          " * @return 0 on success, ENOMEM on allocation failure.\n"
          " */\n"
          "extern  cdd_c_error_t url_query_build_form(const struct "
          "UrlQueryParams *qp,\n"
          "                                             char **out_str);\n"
          "\n"
          "#ifdef __cplusplus\n"
          "}\n"
          "#endif /* __cplusplus */\n"
          "\n"
          "#endif /* C_CDD_URL_UTILS_H */\n"
          "\n",
          uh);
    fclose(uh);
    fclose(uh);
  }

  return CDD_C_SUCCESS;
}
