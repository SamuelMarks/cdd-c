/**
 * @file client_sig_media.c
 * @brief Media type detection and categorization for client signature
 * generation.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <stddef.h>
#include <string.h>

#include "cdd_c_error.h"
#include "functions/emit/client_sig.h"
#include "functions/emit/client_sig_media.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
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
extern C_CDD_EXPORT int g_cdd_fail_find_media_type;
#endif

/**
 * @brief Computes length of media type up to optional semicolon parameter.
 */
cdd_c_error_t media_type_base_len(const char *media_type, size_t *_out_val) {
  size_t i = 0;
  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
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
  while (media_type[i] && media_type[i] != ';') {
    ++i;
  }
  *_out_val = i;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if media type starts with a specified prefix.
 */
cdd_c_error_t media_type_has_prefix(const char *media_type, const char *prefix,
                                    int *out) {
  size_t i;
  size_t pre_len;
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
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
 */
cdd_c_error_t media_type_has_suffix(const char *media_type, const char *suffix,
                                    int *out) {
  size_t i;
  size_t len = 0;
  size_t suf_len;
  size_t start;
  cdd_c_error_t rc;
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
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
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
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
 */
cdd_c_error_t media_type_ieq(const char *media_type, const char *expected,
                             int *out) {
  size_t i;
  size_t len = 0;
  size_t exp_len;
  cdd_c_error_t rc;
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_ieq) {
    if (--g_cdd_fail_media_type_ieq == 0) {
      return CDD_C_ERROR_UNKNOWN;
    }
  }
#endif
  if (!media_type || !expected) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  rc = media_type_base_len(media_type, &len);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
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
 */
cdd_c_error_t media_type_is_json(const char *media_type, int *out) {
  int is_app_json = 0;
  int has_plus_json = 0;
  cdd_c_error_t rc;
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_is_json) {
    g_cdd_fail_media_type_is_json = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *out = 0;
  if (!media_type) {
    return CDD_C_SUCCESS;
  }
  rc = media_type_ieq(media_type, "application/json", &is_app_json);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (is_app_json) {
    *out = 1;
    return CDD_C_SUCCESS;
  }
  rc = media_type_has_suffix(media_type, "+json", &has_plus_json);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  *out = has_plus_json;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a media type represents URL-encoded form data.
 */
cdd_c_error_t media_type_is_form(const char *media_type, int *out) {
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
 */
cdd_c_error_t media_type_is_text_plain(const char *media_type, int *out) {
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
 */
cdd_c_error_t media_type_is_multipart(const char *media_type, int *out) {
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
 */
cdd_c_error_t media_type_is_multipart_form(const char *media_type, int *out) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_is_multipart_form) {
    if (--g_cdd_fail_media_type_is_multipart_form == 0) {
      return CDD_C_ERROR_UNKNOWN;
    }
  }
#endif
  return media_type_ieq(media_type, "multipart/form-data", out);
}

/**
 * @brief Finds a media type definition by name in an array.
 */
cdd_c_error_t find_media_type(const struct OpenAPI_MediaType *mts, size_t n,
                              const char *name,
                              const struct OpenAPI_MediaType **_out_val) {
  size_t i;
  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_find_media_type) {
    g_cdd_fail_find_media_type = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!mts || !name) {
    return CDD_C_SUCCESS;
  }
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
 */
cdd_c_error_t media_type_is_textual(const char *media_type, int *out) {
  int check = 0;
  cdd_c_error_t rc;
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_is_textual) {
    g_cdd_fail_media_type_is_textual = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *out = 0;
  if (!media_type) {
    return CDD_C_SUCCESS;
  }
  rc = media_type_is_text_plain(media_type, &check);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (check) {
    *out = 1;
    return CDD_C_SUCCESS;
  }
  rc = media_type_has_prefix(media_type, "text/", &check);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (check) {
    *out = 1;
    return CDD_C_SUCCESS;
  }
  rc = media_type_ieq(media_type, "application/xml", &check);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (check) {
    *out = 1;
    return CDD_C_SUCCESS;
  }
  rc = media_type_has_suffix(media_type, "+xml", &check);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  *out = check;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a media type represents binary content.
 */
cdd_c_error_t media_type_is_binary(const char *media_type, int *out) {
  int check = 0;
  cdd_c_error_t rc;
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_media_type_is_binary) {
    g_cdd_fail_media_type_is_binary = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *out = 0;
  if (!media_type) {
    return CDD_C_SUCCESS;
  }
  rc = media_type_is_json(media_type, &check);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (check) {
    return CDD_C_SUCCESS;
  }
  rc = media_type_is_form(media_type, &check);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (check) {
    return CDD_C_SUCCESS;
  }
  rc = media_type_is_multipart(media_type, &check);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (check) {
    return CDD_C_SUCCESS;
  }
  rc = media_type_is_textual(media_type, &check);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (check) {
    return CDD_C_SUCCESS;
  }
  *out = 1;
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
/**
 * @brief Test helper to compute base length of media type.
 */
cdd_c_error_t cdd_test_sig_media_type_base_len(const char *media_type,
                                               size_t *out_val) {
  return media_type_base_len(media_type, out_val);
}

/**
 * @brief Test helper to check if media type has prefix.
 */
cdd_c_error_t cdd_test_sig_media_type_has_prefix(const char *media_type,
                                                 const char *prefix, int *out) {
  return media_type_has_prefix(media_type, prefix, out);
}

/**
 * @brief Test helper to check if media type has suffix.
 */
cdd_c_error_t cdd_test_sig_media_type_has_suffix(const char *media_type,
                                                 const char *suffix, int *out) {
  return media_type_has_suffix(media_type, suffix, out);
}

/**
 * @brief Test helper for media type equality.
 */
cdd_c_error_t cdd_test_sig_media_type_ieq(const char *media_type,
                                          const char *expected, int *out) {
  return media_type_ieq(media_type, expected, out);
}

/**
 * @brief Test helper to check if media type is JSON.
 */
cdd_c_error_t cdd_test_sig_media_type_is_json(const char *media_type,
                                              int *out) {
  return media_type_is_json(media_type, out);
}

/**
 * @brief Test helper to check if media type is form.
 */
cdd_c_error_t cdd_test_sig_media_type_is_form(const char *media_type,
                                              int *out) {
  return media_type_is_form(media_type, out);
}

/**
 * @brief Test helper to check if media type is text/plain.
 */
cdd_c_error_t cdd_test_sig_media_type_is_text_plain(const char *media_type,
                                                    int *out) {
  return media_type_is_text_plain(media_type, out);
}

/**
 * @brief Test helper to check if media type is multipart.
 */
cdd_c_error_t cdd_test_sig_media_type_is_multipart(const char *media_type,
                                                   int *out) {
  return media_type_is_multipart(media_type, out);
}

/**
 * @brief Test helper to check if media type is multipart/form-data.
 */
cdd_c_error_t cdd_test_sig_media_type_is_multipart_form(const char *media_type,
                                                        int *out) {
  return media_type_is_multipart_form(media_type, out);
}

/**
 * @brief Test helper to find media type by name.
 */
cdd_c_error_t
cdd_test_sig_find_media_type(const struct OpenAPI_MediaType *mts, size_t n,
                             const char *name,
                             const struct OpenAPI_MediaType **out_val) {
  return find_media_type(mts, n, name, out_val);
}

/**
 * @brief Test helper to check if media type is textual.
 */
cdd_c_error_t cdd_test_sig_media_type_is_textual(const char *media_type,
                                                 int *out) {
  return media_type_is_textual(media_type, out);
}

/**
 * @brief Test helper to check if media type is binary.
 */
cdd_c_error_t cdd_test_sig_media_type_is_binary(const char *media_type,
                                                int *out) {
  return media_type_is_binary(media_type, out);
}
#endif
