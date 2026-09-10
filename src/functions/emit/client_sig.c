/**
 * @file client_sig.c
 * @brief Implementation of Client Signature Generation.
 *
 * Updated to support Grouped naming convention (Resource_Prefix_OpId).
 * Appends standard `struct ApiError **api_error` argument to all operations.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/safe_crt.h"
#include "functions/emit/client_sig.h"
#include "functions/parse/str.h"
#include "win_compat_sym.h"
/* clang-format on */

/** @brief CHECK_IO definition */
#ifdef CDD_BUILD_TESTS
extern int g_fail_io_after;
extern int g_io_calls;
extern C_CDD_EXPORT int g_cdd_fail_media_type_base_len;
extern C_CDD_EXPORT int g_cdd_fail_media_type_has_suffix;
extern C_CDD_EXPORT int g_cdd_fail_media_type_ieq;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_json;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_text_plain;
extern C_CDD_EXPORT int g_cdd_fail_media_type_has_prefix;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_form;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_multipart;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_textual;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_binary;
extern C_CDD_EXPORT int g_cdd_fail_media_type_is_multipart_form;
extern C_CDD_EXPORT int g_cdd_fail_qs_json_array_item_type;
extern C_CDD_EXPORT int g_cdd_fail_qs_json_array_item_ref;
extern C_CDD_EXPORT int g_cdd_fail_qs_json_prim;
extern C_CDD_EXPORT int g_cdd_fail_qs_raw;
extern C_CDD_EXPORT int g_cdd_fail_qs_form_obj;
extern C_CDD_EXPORT int g_cdd_fail_qs_json_ref;
extern C_CDD_EXPORT int g_cdd_fail_map_array_item_type;
extern C_CDD_EXPORT int g_cdd_fail_sanitize_ident;
extern C_CDD_EXPORT int g_cdd_fail_map_type_to_c_arg;
extern C_CDD_EXPORT int g_cdd_fail_is_primitive_type;
extern C_CDD_EXPORT int g_cdd_fail_param_is_object_kv;
extern C_CDD_EXPORT int g_cdd_fail_find_media_type;
extern C_CDD_EXPORT int g_cdd_fail_header_name_is_content_type;
extern C_CDD_EXPORT int g_cdd_fail_c_cdd_str_iequal;
extern C_CDD_EXPORT int g_cdd_fail_multipart_header_param_name;
extern C_CDD_EXPORT int g_cdd_fail_response_is_binary_success;
extern C_CDD_EXPORT int g_cdd_fail_get_success_schema;
extern C_CDD_EXPORT int g_cdd_fail_get_success_response;
extern C_CDD_EXPORT int g_cdd_fail_schema_has_inline;
extern C_CDD_EXPORT int g_cdd_fail_map_type_to_c_out;
extern C_CDD_EXPORT int g_cdd_fail_map_array_item_type_out;

static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)))
#endif
    ;
static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...) {
  int ret;
  va_list args;
  if (g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after)
    return -1;
  va_start(args, format);
  ret = vfprintf(stream, format, args);
  va_end(args);
  return ret;
}
#define fprintf test_cdd_fprintf_hook
#endif

#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return CDD_C_ERROR_IO;                                                   \
  } while (0)

/**
 * @brief Maps an OpenAPI type string to a C function argument type.
 *
 * @param[in] oa_type OpenAPI type name.
 * @param[out] _out_val Pointer to receive the C argument type string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
static cdd_c_error_t map_type_to_c_arg(const char *oa_type,
                                       const char **_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_map_type_to_c_arg) {
    g_cdd_fail_map_type_to_c_arg = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!oa_type) {
    *_out_val = "const void *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "integer") == 0) {
    *_out_val = "int ";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "string") == 0) {
    *_out_val = "const char *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "boolean") == 0) {
    *_out_val = "int ";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "number") == 0) {
    *_out_val = "double ";
    return CDD_C_SUCCESS;
  }
  *_out_val = "const void *";
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if an OpenAPI type is a primitive type.
 *
 * @param[in] oa_type OpenAPI type name.
 * @param[out] out Pointer to receive 1 if primitive, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
static cdd_c_error_t is_primitive_type(const char *oa_type, int *out) {
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_is_primitive_type) {
    g_cdd_fail_is_primitive_type = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!oa_type) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  *out = (strcmp(oa_type, "integer") == 0 || strcmp(oa_type, "string") == 0 ||
          strcmp(oa_type, "boolean") == 0 || strcmp(oa_type, "number") == 0);
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a parameter represents an object Key-Value structure.
 *
 * @param[in] p Parameter definition.
 * @param[out] out Pointer to receive 1 if object KV, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
static cdd_c_error_t param_is_object_kv(const struct OpenAPI_Parameter *p,
                                        int *out) {
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_param_is_object_kv) {
    g_cdd_fail_param_is_object_kv = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!p) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  *out = 0;
  if (!p->is_array && p->type && strcmp(p->type, "object") == 0) {
    *out = (p->in == OA_PARAM_IN_QUERY || p->in == OA_PARAM_IN_PATH ||
            p->in == OA_PARAM_IN_HEADER || p->in == OA_PARAM_IN_COOKIE);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Computes length of media type up to optional semicolon parameter.
 *
 * @param[in] media_type Media type string.
 * @param[out] _out_val Pointer to receive base length.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
static cdd_c_error_t media_type_base_len(const char *media_type,
                                         size_t *_out_val) {
  size_t i = 0;
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_base_len) {
    g_cdd_fail_media_type_base_len = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!media_type) {
    *_out_val = 0;
    return CDD_C_SUCCESS;
  }
  while (media_type[i] && media_type[i] != ';')
    ++i;
  *_out_val = i;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if media type starts with a specified prefix.
 *
 * @param[in] media_type Media type string.
 * @param[in] prefix Prefix string to test.
 * @param[out] out Pointer to receive 1 if prefix matches, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
static cdd_c_error_t media_type_has_prefix(const char *media_type,
                                           const char *prefix, int *out) {
  size_t i;
  size_t pre_len;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_has_prefix) {
    g_cdd_fail_media_type_has_prefix = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!media_type || !prefix) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  pre_len = strlen(prefix);
  for (i = 0; i < pre_len; ++i) {
    char a = media_type[i];
    char b = prefix[i];
    if (a != b) {
      *out = 0;
      return CDD_C_SUCCESS;
    }
  }
  *out = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if base media type ends with a specified suffix.
 *
 * @param[in] media_type Media type string.
 * @param[in] suffix Suffix string to test.
 * @param[out] out Pointer to receive 1 if suffix matches, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t media_type_has_suffix(const char *media_type,
                                           const char *suffix, int *out) {
  size_t i;
  size_t len = 0;
  size_t suf_len;
  size_t start;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_has_suffix) {
    g_cdd_fail_media_type_has_suffix = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!media_type || !suffix) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  rc = media_type_base_len(media_type, &len);
  if (rc != CDD_C_SUCCESS)
    return rc;
  suf_len = strlen(suffix);
  if (len < suf_len) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  start = len - suf_len;
  for (i = 0; i < suf_len; ++i) {
    char a = media_type[start + i];
    char b = suffix[i];
    if (a != b) {
      *out = 0;
      return CDD_C_SUCCESS;
    }
  }
  *out = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Compares media type base portion with an expected string.
 *
 * @param[in] media_type Media type string.
 * @param[in] expected Expected media type string.
 * @param[out] out Pointer to receive 1 if equal, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t media_type_ieq(const char *media_type,
                                    const char *expected, int *out) {
  size_t i;
  size_t len = 0;
  size_t exp_len;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_ieq) {
    if (--g_cdd_fail_media_type_ieq == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!media_type || !expected) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  rc = media_type_base_len(media_type, &len);
  if (rc != CDD_C_SUCCESS)
    return rc;
  exp_len = strlen(expected);
  if (len != exp_len) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < len; ++i) {
    char a = media_type[i];
    char b = expected[i];
    if (a != b) {
      *out = 0;
      return CDD_C_SUCCESS;
    }
  }
  *out = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a media type represents JSON.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if JSON, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t media_type_is_json(const char *media_type, int *out) {
  int is_app_json = 0;
  int has_plus_json = 0;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_is_json) {
    g_cdd_fail_media_type_is_json = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *out = 0;
  if (!media_type)
    return CDD_C_SUCCESS;
  rc = media_type_ieq(media_type, "application/json", &is_app_json);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (is_app_json) {
    *out = 1;
    return CDD_C_SUCCESS;
  }
  rc = media_type_has_suffix(media_type, "+json", &has_plus_json);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out = has_plus_json;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a media type represents URL-encoded form data.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if form data, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t media_type_is_form(const char *media_type, int *out) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_is_form) {
    g_cdd_fail_media_type_is_form = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  return media_type_ieq(media_type, "application/x-www-form-urlencoded", out);
}

/**
 * @brief Checks if a media type is text/plain.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if text/plain, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t media_type_is_text_plain(const char *media_type,
                                              int *out) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_is_text_plain) {
    g_cdd_fail_media_type_is_text_plain = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  return media_type_ieq(media_type, "text/plain", out);
}

/**
 * @brief Checks if a media type is multipart.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if multipart, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t media_type_is_multipart(const char *media_type, int *out) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_is_multipart) {
    g_cdd_fail_media_type_is_multipart = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  return media_type_has_prefix(media_type, "multipart/", out);
}

/**
 * @brief Checks if a media type is multipart/form-data.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if multipart/form-data, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t media_type_is_multipart_form(const char *media_type,
                                                  int *out) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_is_multipart_form) {
    if (--g_cdd_fail_media_type_is_multipart_form == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
#endif
  return media_type_ieq(media_type, "multipart/form-data", out);
}

/**
 * @brief Finds a media type definition by name in an array.
 *
 * @param[in] mts Array of OpenAPI_MediaType structures.
 * @param[in] n Array size.
 * @param[in] name Media type name to find.
 * @param[out] _out_val Pointer to receive matched media type pointer.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
static cdd_c_error_t
find_media_type(const struct OpenAPI_MediaType *mts, size_t n, const char *name,
                const struct OpenAPI_MediaType **_out_val) {
  size_t i;
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_find_media_type) {
    g_cdd_fail_find_media_type = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!mts || !name)
    return CDD_C_SUCCESS;
  for (i = 0; i < n; ++i) {
    if (mts[i].name && strcmp(mts[i].name, name) == 0) {
      *_out_val = &mts[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a media type represents textual content.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if textual, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t media_type_is_textual(const char *media_type, int *out) {
  int check = 0;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_is_textual) {
    g_cdd_fail_media_type_is_textual = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *out = 0;
  if (!media_type)
    return CDD_C_SUCCESS;
  rc = media_type_is_text_plain(media_type, &check);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (check) {
    *out = 1;
    return CDD_C_SUCCESS;
  }
  rc = media_type_has_prefix(media_type, "text/", &check);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (check) {
    *out = 1;
    return CDD_C_SUCCESS;
  }
  rc = media_type_ieq(media_type, "application/xml", &check);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (check) {
    *out = 1;
    return CDD_C_SUCCESS;
  }
  rc = media_type_has_suffix(media_type, "+xml", &check);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out = check;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a media type represents binary content.
 *
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if binary, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t media_type_is_binary(const char *media_type, int *out) {
  int check = 0;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_is_binary) {
    g_cdd_fail_media_type_is_binary = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *out = 0;
  if (!media_type)
    return CDD_C_SUCCESS;
  rc = media_type_is_json(media_type, &check);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (check)
    return CDD_C_SUCCESS;
  rc = media_type_is_form(media_type, &check);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (check)
    return CDD_C_SUCCESS;
  rc = media_type_is_multipart(media_type, &check);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (check)
    return CDD_C_SUCCESS;
  rc = media_type_is_textual(media_type, &check);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (check)
    return CDD_C_SUCCESS;
  *out = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a querystring parameter is a form object.
 *
 * @param[in] p Parameter definition.
 * @param[out] out Pointer to receive 1 if form object, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
static cdd_c_error_t
querystring_param_is_form_object(const struct OpenAPI_Parameter *p, int *out) {
  int is_json = 0;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_qs_form_obj) {
    g_cdd_fail_qs_form_obj = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!p) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  if (p->content_type) {
    rc = media_type_is_json(p->content_type, &is_json);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_json) {
      *out = 0;
      return CDD_C_SUCCESS;
    }
  }
  *out = (p->schema.ref_name != NULL);
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a querystring parameter is a JSON schema reference.
 *
 * @param[in] p Parameter definition.
 * @param[out] out Pointer to receive 1 if JSON ref, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
static cdd_c_error_t
querystring_param_is_json_ref(const struct OpenAPI_Parameter *p, int *out) {
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_qs_json_ref) {
    g_cdd_fail_qs_json_ref = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!p) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  if (p->schema.is_array || (p->type && strcmp(p->type, "array") == 0)) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  *out = (p->schema.ref_name != NULL);
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines the JSON primitive type of a querystring parameter.
 *
 * @param[in] p Parameter definition.
 * @param[out] _out_val Pointer to receive primitive type string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
querystring_param_json_primitive_type(const struct OpenAPI_Parameter *p,
                                      const char **_out_val) {
  const char *type = NULL;
  int is_json = 0;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_qs_json_prim) {
    g_cdd_fail_qs_json_prim = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!p)
    return CDD_C_SUCCESS;

  if (p->in != OA_PARAM_IN_QUERYSTRING)
    return CDD_C_SUCCESS;

  rc = media_type_is_json(p->content_type, &is_json);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!is_json)
    return CDD_C_SUCCESS;

  if (p->schema.is_array || (p->type && strcmp(p->type, "array") == 0))
    return CDD_C_SUCCESS;

  if (p->schema.inline_type)
    type = p->schema.inline_type;
  else if (p->type)
    type = p->type;

  if (!type)
    return CDD_C_SUCCESS;

  if (strcmp(type, "string") == 0 || strcmp(type, "integer") == 0 ||
      strcmp(type, "number") == 0 || strcmp(type, "boolean") == 0) {
    *_out_val = type;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines the JSON array item type of a querystring parameter.
 *
 * @param[in] p Parameter definition.
 * @param[out] _out_val Pointer to receive item type string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
querystring_param_json_array_item_type(const struct OpenAPI_Parameter *p,
                                       const char **_out_val) {
  const char *item_type = NULL;
  int is_json = 0;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_qs_json_array_item_type) {
    g_cdd_fail_qs_json_array_item_type = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!p)
    return CDD_C_SUCCESS;

  if (p->in != OA_PARAM_IN_QUERYSTRING)
    return CDD_C_SUCCESS;

  rc = media_type_is_json(p->content_type, &is_json);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!is_json)
    return CDD_C_SUCCESS;

  if (!(p->schema.is_array || (p->type && strcmp(p->type, "array") == 0) ||
        p->is_array))
    return CDD_C_SUCCESS;

  if (p->schema.inline_type)
    item_type = p->schema.inline_type;
  else if (p->items_type)
    item_type = p->items_type;

  if (!item_type)
    return CDD_C_SUCCESS;

  if (strcmp(item_type, "string") == 0 || strcmp(item_type, "integer") == 0 ||
      strcmp(item_type, "number") == 0 || strcmp(item_type, "boolean") == 0) {
    *_out_val = item_type;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines the JSON array item reference of a querystring parameter.
 *
 * @param[in] p Parameter definition.
 * @param[out] _out_val Pointer to receive item reference string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
querystring_param_json_array_item_ref(const struct OpenAPI_Parameter *p,
                                      const char **_out_val) {
  const char *item_type = NULL;
  int is_json = 0;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_qs_json_array_item_ref) {
    g_cdd_fail_qs_json_array_item_ref = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!p)
    return CDD_C_SUCCESS;

  if (p->in != OA_PARAM_IN_QUERYSTRING)
    return CDD_C_SUCCESS;

  rc = media_type_is_json(p->content_type, &is_json);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!is_json)
    return CDD_C_SUCCESS;

  if (!(p->schema.is_array || (p->type && strcmp(p->type, "array") == 0) ||
        p->is_array))
    return CDD_C_SUCCESS;

  if (p->schema.inline_type)
    item_type = p->schema.inline_type;
  else if (p->items_type)
    item_type = p->items_type;

  if (!item_type)
    return CDD_C_SUCCESS;

  if (strcmp(item_type, "string") == 0 || strcmp(item_type, "integer") == 0 ||
      strcmp(item_type, "number") == 0 || strcmp(item_type, "boolean") == 0)
    return CDD_C_SUCCESS;

  if (strcmp(item_type, "object") == 0)
    return CDD_C_SUCCESS;

  *_out_val = item_type;
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines raw primitive type for non-JSON/form querystring
 * parameters.
 *
 * @param[in] p Parameter definition.
 * @param[out] _out_val Pointer to receive primitive type string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
querystring_param_raw_primitive_type(const struct OpenAPI_Parameter *p,
                                     const char **_out_val) {
  const char *type = NULL;
  int is_json = 0;
  int is_form = 0;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_qs_raw) {
    g_cdd_fail_qs_raw = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!p)
    return CDD_C_SUCCESS;

  if (p->in != OA_PARAM_IN_QUERYSTRING || !p->content_type)
    return CDD_C_SUCCESS;

  rc = media_type_is_json(p->content_type, &is_json);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (is_json)
    return CDD_C_SUCCESS;

  rc = media_type_is_form(p->content_type, &is_form);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (is_form)
    return CDD_C_SUCCESS;

  if (p->schema.inline_type)
    type = p->schema.inline_type;
  else if (p->type)
    type = p->type;

  if (!type) {
    *_out_val = "string";
    return CDD_C_SUCCESS;
  }
  if (strcmp(type, "string") == 0 || strcmp(type, "integer") == 0 ||
      strcmp(type, "number") == 0 || strcmp(type, "boolean") == 0) {
    *_out_val = type;
    return CDD_C_SUCCESS;
  }
  *_out_val = "string";
  return CDD_C_SUCCESS;
}

/**
 * @brief Maps an OpenAPI array item type to a C pointer type.
 *
 * @param[in] oa_type OpenAPI array item type.
 * @param[out] _out_val Pointer to receive C pointer type string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
static cdd_c_error_t map_array_item_type(const char *oa_type,
                                         const char **_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_map_array_item_type) {
    g_cdd_fail_map_array_item_type = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!oa_type) {
    *_out_val = "const void *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "integer") == 0) {
    *_out_val = "const int *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "boolean") == 0) {
    *_out_val = "const int *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "string") == 0) {
    *_out_val = "const char **";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "number") == 0) {
    *_out_val = "const double *";
    return CDD_C_SUCCESS;
  }
  *_out_val = "const void *";
  return CDD_C_SUCCESS;
}

/**
 * @brief Sanitizes an identifier to be a valid C identifier.
 *
 * @param[out] out Output buffer.
 * @param[in] outsz Size of output buffer.
 * @param[in] in Input string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL/empty.
 */
static cdd_c_error_t sanitize_ident(char *out, size_t outsz, const char *in) {
  size_t i = 0;
  size_t j = 0;
  if (!out || outsz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_sanitize_ident) {
    g_cdd_fail_sanitize_ident = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  out[0] = '\0';
  if (!in)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; in[i] && j + 1 < outsz; ++i) {
    const unsigned char c = (unsigned char)in[i];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9')) {
      out[j++] = (char)c;
    } else {
      out[j++] = '_';
    }
  }
  out[j] = '\0';
  if (j > 0 && out[0] >= '0' && out[0] <= '9') {
    if (j + 1 < outsz) {
      memmove(out + 1, out, j + 1);
      out[0] = '_';
    } else {
      out[0] = '_';
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates a sanitized multipart header parameter name.
 *
 * @param[out] out Destination buffer.
 * @param[in] outsz Destination buffer size.
 * @param[in] field Multipart field name.
 * @param[in] header Header name.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t multipart_header_param_name(char *out, size_t outsz,
                                                 const char *field,
                                                 const char *header) {
  char hdr_sanitized[128];
  cdd_c_error_t rc;
  if (!out || outsz == 0 || !field || !header)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_multipart_header_param_name) {
    g_cdd_fail_multipart_header_param_name = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  out[0] = '\0';
  rc = sanitize_ident(hdr_sanitized, sizeof(hdr_sanitized), header);
  if (rc != CDD_C_SUCCESS)
    return rc;
  CDD_SNPRINTF(out, outsz, "%s_hdr_%s", field, hdr_sanitized);
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a header name is Content-Type (case-insensitive).
 *
 * @param[in] name Header name string.
 * @param[out] out Pointer to receive 1 if Content-Type, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
static cdd_c_error_t header_name_is_content_type(const char *name, int *out) {
  int iequal = 0;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_header_name_is_content_type) {
    g_cdd_fail_header_name_is_content_type = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *out = 0;
  if (!name)
    return CDD_C_SUCCESS;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_c_cdd_str_iequal) {
    g_cdd_fail_c_cdd_str_iequal = 0;
    rc = CDD_C_ERROR_UNKNOWN;
  } else
#endif
    rc = c_cdd_str_iequal(name, "Content-Type", &iequal);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out = iequal;
  return CDD_C_SUCCESS;
}

/**
 * @brief Maps an OpenAPI type to a C output pointer parameter type.
 *
 * @param[in] oa_type OpenAPI type name.
 * @param[out] _out_val Pointer to receive C type string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
static cdd_c_error_t map_type_to_c_out(const char *oa_type,
                                       const char **_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_map_type_to_c_out) {
    g_cdd_fail_map_type_to_c_out = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!oa_type) {
    *_out_val = "void *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "integer") == 0) {
    *_out_val = "int *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "boolean") == 0) {
    *_out_val = "int *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "string") == 0) {
    *_out_val = "char **";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "number") == 0) {
    *_out_val = "double *";
    return CDD_C_SUCCESS;
  }
  *_out_val = "void *";
  return CDD_C_SUCCESS;
}

/**
 * @brief Maps an OpenAPI array item type to a C array output pointer parameter
 * type.
 *
 * @param[in] oa_type OpenAPI array item type name.
 * @param[out] _out_val Pointer to receive C type string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
static cdd_c_error_t map_array_item_type_out(const char *oa_type,
                                             const char **_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_map_array_item_type_out) {
    g_cdd_fail_map_array_item_type_out = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!oa_type) {
    *_out_val = "void **";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "integer") == 0) {
    *_out_val = "int **";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "boolean") == 0) {
    *_out_val = "int **";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "string") == 0) {
    *_out_val = "char ***";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "number") == 0) {
    *_out_val = "double **";
    return CDD_C_SUCCESS;
  }
  *_out_val = "void **";
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a schema reference contains an inline type definition.
 *
 * @param[in] schema Schema reference structure.
 * @param[out] out Pointer to receive 1 if inline type exists, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out is
 * NULL.
 */
static cdd_c_error_t schema_has_inline(const struct OpenAPI_SchemaRef *schema,
                                       int *out) {
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_schema_has_inline) {
    if (--g_cdd_fail_schema_has_inline == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!schema) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  *out = (schema->inline_type != NULL);
  return CDD_C_SUCCESS;
}

/**
 * @brief Selects the primary success response from an operation.
 *
 * @param[in] op Operation definition.
 * @param[out] _out_val Pointer to receive selected response.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
static cdd_c_error_t
get_success_response(const struct OpenAPI_Operation *op,
                     const struct OpenAPI_Response **_out_val) {
  const struct OpenAPI_Response *default_resp = NULL;
  size_t i;
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_get_success_response) {
    g_cdd_fail_get_success_response = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!op)
    return CDD_C_SUCCESS;

  for (i = 0; i < op->n_responses; ++i) {
    const struct OpenAPI_Response *resp = &op->responses[i];
    const char *c = resp->code;
    if (!c)
      continue;
    if (strcmp(c, "default") == 0) {
      default_resp = resp;
      continue;
    }
    if (strlen(c) == 3 && c[0] == '2' && c[1] == 'X' && c[2] == 'X') {
      *_out_val = resp;
      return CDD_C_SUCCESS;
    }
    if (c[0] == '2') {
      *_out_val = resp;
      return CDD_C_SUCCESS;
    }
  }
  *_out_val = default_resp;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if the success response of an operation is binary.
 *
 * @param[in] op Operation definition.
 * @param[out] out Pointer to receive 1 if binary success, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
response_is_binary_success(const struct OpenAPI_Operation *op, int *out) {
  const struct OpenAPI_Response *resp = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_response_is_binary_success) {
    g_cdd_fail_response_is_binary_success = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *out = 0;
  if (!op)
    return CDD_C_SUCCESS;
  rc = get_success_response(op, &resp);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!resp || !resp->content_type)
    return CDD_C_SUCCESS;
  return media_type_is_binary(resp->content_type, out);
}

/**
 * @brief Selects the success schema reference from an operation.
 *
 * @param[in] op Operation definition.
 * @param[out] _out_val Pointer to receive selected schema reference pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
get_success_schema(const struct OpenAPI_Operation *op,
                   const struct OpenAPI_SchemaRef **_out_val) {
  const struct OpenAPI_Response *default_resp = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_get_success_schema) {
    g_cdd_fail_get_success_schema = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!op)
    return CDD_C_SUCCESS;

  for (i = 0; i < op->n_responses; ++i) {
    const char *c = op->responses[i].code;
    int has_inline = 0;
    if (!c)
      continue;
    if (strcmp(c, "default") == 0) {
      default_resp = &op->responses[i];
      continue;
    }
    rc = schema_has_inline(&op->responses[i].schema, &has_inline);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (strlen(c) == 3 && c[0] == '2' && c[1] == 'X' && c[2] == 'X') {
      if (op->responses[i].schema.ref_name || has_inline ||
          op->responses[i].schema.is_array) {
        *_out_val = &op->responses[i].schema;
        return CDD_C_SUCCESS;
      }
      continue;
    }
    if (c[0] == '2') {
      if (op->responses[i].schema.ref_name || has_inline ||
          op->responses[i].schema.is_array) {
        *_out_val = &op->responses[i].schema;
        return CDD_C_SUCCESS;
      }
    }
  }

  if (default_resp) {
    int has_inline = 0;
    rc = schema_has_inline(&default_resp->schema, &has_inline);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (default_resp->schema.ref_name || has_inline ||
        default_resp->schema.is_array) {
      *_out_val = &default_resp->schema;
      return CDD_C_SUCCESS;
    }
  }

  *_out_val = &op->req_body;
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for codegen client write signature.
 *
 * @param[in] fp File stream to write to.
 * @param[in] op OpenAPI Operation definition.
 * @param[in] config Codegen signature configuration.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t
codegen_client_write_signature(FILE *fp, const struct OpenAPI_Operation *op,
                               const struct CodegenSigConfig *config) {
  const char *ctx_type;
  const char *prefix;
  const char *func_name;
  const char *group;
  const struct OpenAPI_SchemaRef *success_schema = NULL;
  int success_is_binary = 0;
  size_t i;
  cdd_c_error_t rc;

  if (!fp || !op)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  ctx_type =
      (config && config->ctx_type) ? config->ctx_type : "struct HttpClient *";
  prefix = (config && config->prefix) ? config->prefix : "";
  group = (config && config->group_name) ? config->group_name : NULL;
  func_name = op->operation_id ? op->operation_id : "unnamed_op";

  /* Construct function name: [Group_][Prefix][OpName] */
  CHECK_IO(fprintf(fp, "int "));
  if (group && *group) {
    CHECK_IO(fprintf(fp, "%s_", group));
  }
  CHECK_IO(fprintf(fp, "%s%s(%sctx", prefix, func_name, ctx_type));

  /* 1. Parameters */
  for (i = 0; i < op->n_parameters; ++i) {
    const struct OpenAPI_Parameter *p = &op->parameters[i];
    if (p->in == OA_PARAM_IN_QUERYSTRING) {
      const char *qs_json_item = NULL;
      const char *qs_json_obj = NULL;
      const char *qs_json_prim = NULL;
      const char *qs_raw = NULL;
      int is_form_obj = 0;
      int is_json_ref_val = 0;

      rc = querystring_param_json_array_item_type(p, &qs_json_item);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = querystring_param_json_array_item_ref(p, &qs_json_obj);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = querystring_param_json_primitive_type(p, &qs_json_prim);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = querystring_param_raw_primitive_type(p, &qs_raw);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = querystring_param_is_form_object(p, &is_form_obj);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = querystring_param_is_json_ref(p, &is_json_ref_val);
      if (rc != CDD_C_SUCCESS)
        return rc;

      if (is_form_obj) {
        CHECK_IO(fprintf(fp, ", const struct OpenAPI_KV *%s, size_t %s_len",
                         p->name, p->name));
      } else if (is_json_ref_val) {
        CHECK_IO(
            fprintf(fp, ", const struct %s *%s", p->schema.ref_name, p->name));
      } else if (qs_json_obj) {
        CHECK_IO(fprintf(fp, ", const struct %s **%s, size_t %s_len",
                         qs_json_obj, p->name, p->name));
      } else if (qs_json_item) {
        const char *c_type = NULL;
        rc = map_array_item_type(qs_json_item, &c_type);
        if (rc != CDD_C_SUCCESS)
          return rc;
        CHECK_IO(
            fprintf(fp, ", %s%s, size_t %s_len", c_type, p->name, p->name));
      } else if (qs_json_prim) {
        const char *c_type = NULL;
        rc = map_type_to_c_arg(qs_json_prim, &c_type);
        if (rc != CDD_C_SUCCESS)
          return rc;
        CHECK_IO(fprintf(fp, ", %s%s", c_type, p->name));
      } else if (qs_raw) {
        const char *c_type = NULL;
        rc = map_type_to_c_arg(qs_raw, &c_type);
        if (rc != CDD_C_SUCCESS)
          return rc;
        CHECK_IO(fprintf(fp, ", %s%s", c_type, p->name));
      } else {
        CHECK_IO(fprintf(fp, ", const char *%s", p->name));
      }
      continue;
    }
    {
      int is_p_json = 0;
      if (p->content_type) {
        rc = media_type_is_json(p->content_type, &is_p_json);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (is_p_json) {
        const char *ref_name = p->schema.ref_name;
        int is_prim = 0;
        if (p->type) {
          rc = is_primitive_type(p->type, &is_prim);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        if (!ref_name && p->type && !is_prim &&
            strcmp(p->type, "object") != 0 && strcmp(p->type, "array") != 0) {
          ref_name = p->type;
        }
        if (p->is_array) {
          const char *item_type =
              p->items_type ? p->items_type : p->schema.inline_type;
          int item_is_prim = 0;
          if (item_type) {
            rc = is_primitive_type(item_type, &item_is_prim);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          if (item_type && item_is_prim) {
            const char *c_type = NULL;
            rc = map_array_item_type(item_type, &c_type);
            if (rc != CDD_C_SUCCESS)
              return rc;
            CHECK_IO(
                fprintf(fp, ", %s%s, size_t %s_len", c_type, p->name, p->name));
          } else if (item_type && strcmp(item_type, "object") != 0) {
            CHECK_IO(fprintf(fp, ", const struct %s **%s, size_t %s_len",
                             item_type, p->name, p->name));
          } else {
            CHECK_IO(fprintf(fp, ", const void *%s, size_t %s_len", p->name,
                             p->name));
          }
        } else if (ref_name) {
          CHECK_IO(fprintf(fp, ", const struct %s *%s", ref_name, p->name));
        } else if (p->type && strcmp(p->type, "object") == 0) {
          CHECK_IO(fprintf(fp, ", const struct OpenAPI_KV *%s, size_t %s_len",
                           p->name, p->name));
        } else {
          const char *prim = p->type ? p->type : p->schema.inline_type;
          const char *c_type = NULL;
          rc = map_type_to_c_arg(prim ? prim : "string", &c_type);
          if (rc != CDD_C_SUCCESS)
            return rc;
          CHECK_IO(fprintf(fp, ", %s%s", c_type, p->name));
        }
        continue;
      }
    }
    {
      int is_obj_kv = 0;
      rc = param_is_object_kv(p, &is_obj_kv);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (is_obj_kv) {
        CHECK_IO(fprintf(fp, ", const struct OpenAPI_KV *%s, size_t %s_len",
                         p->name, p->name));
      } else if (p->is_array) {
        const char *c_type = NULL;
        rc = map_array_item_type(p->items_type, &c_type);
        if (rc != CDD_C_SUCCESS)
          return rc;
        CHECK_IO(
            fprintf(fp, ", %s%s, size_t %s_len", c_type, p->name, p->name));
      } else {
        const char *c_type = NULL;
        rc = map_type_to_c_arg(p->type, &c_type);
        if (rc != CDD_C_SUCCESS)
          return rc;
        CHECK_IO(fprintf(fp, ", %s%s", c_type, p->name));
      }
    }
  }

  /* 2. Request Body */
  if (op->req_body.content_type) {
    int is_bin = 0;
    int is_mp = 0;
    int is_mp_form = 0;
    int is_txt = 0;

    rc = media_type_is_binary(op->req_body.content_type, &is_bin);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = media_type_is_multipart(op->req_body.content_type, &is_mp);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = media_type_is_multipart_form(op->req_body.content_type, &is_mp_form);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = media_type_is_textual(op->req_body.content_type, &is_txt);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (is_bin || (is_mp && !is_mp_form)) {
      CHECK_IO(fprintf(fp, ", const unsigned char *body, size_t body_len"));
    } else if (is_txt) {
      CHECK_IO(fprintf(fp, ", const char *req_body"));
    } else if (op->req_body.ref_name) {
      if (op->req_body.is_array) {
        if (strcmp(op->req_body.ref_name, "string") == 0) {
          CHECK_IO(fprintf(fp, ", const char **body, size_t body_len"));
        } else if (strcmp(op->req_body.ref_name, "integer") == 0) {
          CHECK_IO(fprintf(fp, ", const int *body, size_t body_len"));
        } else {
          CHECK_IO(fprintf(fp, ", struct %s **body, size_t body_len",
                           op->req_body.ref_name));
        }
      } else {
        CHECK_IO(
            fprintf(fp, ", const struct %s *req_body", op->req_body.ref_name));
      }
    } else if (op->req_body.inline_type) {
      if (op->req_body.is_array) {
        const char *item_type = op->req_body.inline_type;
        const char *c_type = NULL;
        rc = map_array_item_type(item_type, &c_type);
        if (rc != CDD_C_SUCCESS)
          return rc;
        CHECK_IO(fprintf(fp, ", %sbody, size_t body_len", c_type));
      } else {
        const char *c_type = NULL;
        rc = map_type_to_c_arg(op->req_body.inline_type, &c_type);
        if (rc != CDD_C_SUCCESS)
          return rc;
        CHECK_IO(fprintf(fp, ", %sreq_body", c_type));
      }
    }
  }

  /* 2b. Multipart per-part encoding headers */
  if (op->req_body.content_type) {
    int is_mp_form = 0;
    rc = media_type_is_multipart_form(op->req_body.content_type, &is_mp_form);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_mp_form) {
      const struct OpenAPI_MediaType *mt = NULL;
      rc = find_media_type(op->req_body_media_types, op->n_req_body_media_types,
                           "multipart/form-data", &mt);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (mt && mt->encoding && mt->n_encoding > 0) {
        size_t e;
        for (e = 0; e < mt->n_encoding; ++e) {
          const struct OpenAPI_Encoding *enc = &mt->encoding[e];
          size_t h;
          if (!enc->name || !enc->headers || enc->n_headers == 0)
            continue;
          for (h = 0; h < enc->n_headers; ++h) {
            const struct OpenAPI_Header *hdr = &enc->headers[h];
            const char *hdr_type = hdr->type ? hdr->type : "string";
            int hdr_is_array =
                hdr->is_array ||
                ((hdr_type != NULL) && strcmp(hdr_type, "array") == 0);
            char param_name[256];
            int is_ct = 0;
            if (!hdr->name)
              continue;
            rc = header_name_is_content_type(hdr->name, &is_ct);
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (is_ct)
              continue;
            rc = multipart_header_param_name(param_name, sizeof(param_name),
                                             enc->name, hdr->name);
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (hdr_is_array) {
              const char *item_type =
                  hdr->items_type ? hdr->items_type : "string";
              const char *c_type = NULL;
              rc = map_array_item_type(item_type, &c_type);
              if (rc != CDD_C_SUCCESS)
                return rc;
              CHECK_IO(fprintf(fp, ", %s%s, size_t %s_len", c_type, param_name,
                               param_name));
            } else if (strcmp(hdr_type, "object") == 0) {
              CHECK_IO(fprintf(fp,
                               ", const struct OpenAPI_KV *%s, size_t %s_len",
                               param_name, param_name));
            } else {
              const char *c_type = NULL;
              rc = map_type_to_c_arg(hdr_type, &c_type);
              if (rc != CDD_C_SUCCESS)
                return rc;
              CHECK_IO(fprintf(fp, ", %s%s", c_type, param_name));
            }
          }
        }
      }
    }
  }

  /* 3. Success Output */
  rc = response_is_binary_success(op, &success_is_binary);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = get_success_schema(op, &success_schema);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (success_is_binary)
    success_schema = NULL;

  if (success_schema) {
    int schema_inline = 0;
    rc = schema_has_inline(success_schema, &schema_inline);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (success_schema->ref_name || schema_inline || success_schema->is_array) {
      if (success_schema->is_array) {
        if (success_schema->ref_name) {
          if (strcmp(success_schema->ref_name, "string") == 0) {
            CHECK_IO(fprintf(fp, ", char ***out, size_t *out_len"));
          } else if (strcmp(success_schema->ref_name, "integer") == 0) {
            CHECK_IO(fprintf(fp, ", int **out, size_t *out_len"));
          } else {
            CHECK_IO(fprintf(fp, ", struct %s ***out, size_t *out_len",
                             success_schema->ref_name));
          }
        } else if (success_schema->inline_type) {
          const char *out_type = NULL;
          rc = map_array_item_type_out(success_schema->inline_type, &out_type);
          if (rc != CDD_C_SUCCESS)
            return rc;
          CHECK_IO(fprintf(fp, ", %sout, size_t *out_len", out_type));
        }
      } else if (success_schema->ref_name) {
        CHECK_IO(fprintf(fp, ", struct %s **out", success_schema->ref_name));
      } else if (success_schema->inline_type) {
        const char *out_type = NULL;
        rc = map_type_to_c_out(success_schema->inline_type, &out_type);
        if (rc != CDD_C_SUCCESS)
          return rc;
        CHECK_IO(fprintf(fp, ", %sout", out_type));
      }
    }
  } else if (success_is_binary) {
    CHECK_IO(fprintf(fp, ", unsigned char **out, size_t *out_len"));
  }

  /* 4. Global Error Output */
  CHECK_IO(fprintf(fp, ", struct ApiError **api_error"));
  CHECK_IO(fprintf(fp, ")"));

  if (config && config->include_semicolon) {
    CHECK_IO(fprintf(fp, ";\n"));
  } else {
    CHECK_IO(fprintf(fp, " {\n"));
  }

  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
cdd_c_error_t cdd_test_sig_map_type_to_c_arg(const char *oa_type,
                                             const char **out_val) {
  return map_type_to_c_arg(oa_type, out_val);
}

cdd_c_error_t cdd_test_sig_is_primitive_type(const char *oa_type, int *out) {
  return is_primitive_type(oa_type, out);
}

cdd_c_error_t cdd_test_sig_param_is_object_kv(const struct OpenAPI_Parameter *p,
                                              int *out) {
  return param_is_object_kv(p, out);
}

cdd_c_error_t cdd_test_sig_media_type_base_len(const char *media_type,
                                               size_t *out_val) {
  return media_type_base_len(media_type, out_val);
}

cdd_c_error_t cdd_test_sig_media_type_has_prefix(const char *media_type,
                                                 const char *prefix, int *out) {
  return media_type_has_prefix(media_type, prefix, out);
}

cdd_c_error_t cdd_test_sig_media_type_has_suffix(const char *media_type,
                                                 const char *suffix, int *out) {
  return media_type_has_suffix(media_type, suffix, out);
}

cdd_c_error_t cdd_test_sig_media_type_ieq(const char *media_type,
                                          const char *expected, int *out) {
  return media_type_ieq(media_type, expected, out);
}

cdd_c_error_t cdd_test_sig_media_type_is_json(const char *media_type,
                                              int *out) {
  return media_type_is_json(media_type, out);
}

cdd_c_error_t cdd_test_sig_media_type_is_form(const char *media_type,
                                              int *out) {
  return media_type_is_form(media_type, out);
}

cdd_c_error_t cdd_test_sig_media_type_is_text_plain(const char *media_type,
                                                    int *out) {
  return media_type_is_text_plain(media_type, out);
}

cdd_c_error_t cdd_test_sig_media_type_is_multipart(const char *media_type,
                                                   int *out) {
  return media_type_is_multipart(media_type, out);
}

cdd_c_error_t cdd_test_sig_media_type_is_multipart_form(const char *media_type,
                                                        int *out) {
  return media_type_is_multipart_form(media_type, out);
}

cdd_c_error_t
cdd_test_sig_find_media_type(const struct OpenAPI_MediaType *mts, size_t n,
                             const char *name,
                             const struct OpenAPI_MediaType **out_val) {
  return find_media_type(mts, n, name, out_val);
}

cdd_c_error_t cdd_test_sig_media_type_is_textual(const char *media_type,
                                                 int *out) {
  return media_type_is_textual(media_type, out);
}

cdd_c_error_t cdd_test_sig_media_type_is_binary(const char *media_type,
                                                int *out) {
  return media_type_is_binary(media_type, out);
}

cdd_c_error_t
cdd_test_sig_querystring_param_is_form_object(const struct OpenAPI_Parameter *p,
                                              int *out) {
  return querystring_param_is_form_object(p, out);
}

cdd_c_error_t
cdd_test_sig_querystring_param_is_json_ref(const struct OpenAPI_Parameter *p,
                                           int *out) {
  return querystring_param_is_json_ref(p, out);
}

cdd_c_error_t cdd_test_sig_querystring_param_json_primitive_type(
    const struct OpenAPI_Parameter *p, const char **out_val) {
  return querystring_param_json_primitive_type(p, out_val);
}

cdd_c_error_t cdd_test_sig_querystring_param_json_array_item_type(
    const struct OpenAPI_Parameter *p, const char **out_val) {
  return querystring_param_json_array_item_type(p, out_val);
}

cdd_c_error_t cdd_test_sig_querystring_param_json_array_item_ref(
    const struct OpenAPI_Parameter *p, const char **out_val) {
  return querystring_param_json_array_item_ref(p, out_val);
}

cdd_c_error_t cdd_test_sig_querystring_param_raw_primitive_type(
    const struct OpenAPI_Parameter *p, const char **out_val) {
  return querystring_param_raw_primitive_type(p, out_val);
}

cdd_c_error_t cdd_test_sig_map_array_item_type(const char *oa_type,
                                               const char **out_val) {
  return map_array_item_type(oa_type, out_val);
}

cdd_c_error_t cdd_test_sig_sanitize_ident(char *out, size_t outsz,
                                          const char *in) {
  return sanitize_ident(out, outsz, in);
}

cdd_c_error_t cdd_test_sig_multipart_header_param_name(char *out, size_t outsz,
                                                       const char *field,
                                                       const char *header) {
  return multipart_header_param_name(out, outsz, field, header);
}

cdd_c_error_t cdd_test_sig_header_name_is_content_type(const char *name,
                                                       int *out) {
  return header_name_is_content_type(name, out);
}

cdd_c_error_t cdd_test_sig_map_type_to_c_out(const char *oa_type,
                                             const char **out_val) {
  return map_type_to_c_out(oa_type, out_val);
}

cdd_c_error_t cdd_test_sig_map_array_item_type_out(const char *oa_type,
                                                   const char **out_val) {
  return map_array_item_type_out(oa_type, out_val);
}

cdd_c_error_t
cdd_test_sig_schema_has_inline(const struct OpenAPI_SchemaRef *schema,
                               int *out) {
  return schema_has_inline(schema, out);
}

cdd_c_error_t
cdd_test_sig_get_success_response(const struct OpenAPI_Operation *op,
                                  const struct OpenAPI_Response **out_val) {
  return get_success_response(op, out_val);
}

cdd_c_error_t
cdd_test_sig_response_is_binary_success(const struct OpenAPI_Operation *op,
                                        int *out) {
  return response_is_binary_success(op, out);
}

cdd_c_error_t
cdd_test_sig_get_success_schema(const struct OpenAPI_Operation *op,
                                const struct OpenAPI_SchemaRef **out_val) {
  return get_success_schema(op, out_val);
}
#endif
