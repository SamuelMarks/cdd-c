/**
 * @file str.c
 * @brief Implementation of shared string utilities.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/memory.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
/** @brief Alias `strdup` to `_strdup` on MSVC. */
#define strdup _strdup
#endif
#endif
#else
/* Linux/Unix */
#include <errno.h>
#endif

#include "c_cdd/context.h"
#include "functions/parse/str.h"
/* clang-format on */

/**
 * @brief Allocates a duplicate of a null-terminated string.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
cdd_c_error_t c_cdd_strdup(const char *s, char **out_s) {
  const cdd_context_t *ctx = NULL;
  if (s == NULL) {
    *out_s = NULL;
    return CDD_C_SUCCESS;
  }
  cdd_context_get_global(&ctx);
  return ctx->allocator.strdup_fn(s, out_s);
}

/**
 * @brief Checks whether a string begins with a specified prefix.
 *
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t c_cdd_str_starts_with(const char *str, const char *prefix,
                                    int *out_b) {
  size_t i;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_str_starts_with;
  if (g_cdd_fail_str_starts_with && --g_cdd_fail_str_starts_with == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (str == NULL || prefix == NULL) {
    *out_b = false;
    return CDD_C_SUCCESS;
  }
  for (i = 0; prefix[i] != '\0'; i++) {
    if (str[i] != prefix[i]) {
      *out_b = false;
      return CDD_C_SUCCESS;
    }
  }
  *out_b = true;
  return CDD_C_SUCCESS;
}

/**
 * @brief Compares two null-terminated strings for exact equality.
 *
 * @return `CDD_C_SUCCESS` on success.
 */
cdd_c_error_t c_cdd_str_equal(const char *a, const char *b, int *out_b) {
  if (a == b) {
    *out_b = true;
    return CDD_C_SUCCESS;
  }
  if (a == NULL || b == NULL) {
    *out_b = false;
    return CDD_C_SUCCESS;
  }
  *out_b = (strcmp(a, b) == 0);
  return CDD_C_SUCCESS;
}

/**
 * @brief Compares two null-terminated strings for case-insensitive equality.
 *
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t c_cdd_str_iequal(const char *a, const char *b, int *out_b) {
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_str_iequal;
    if (g_cdd_fail_str_iequal && --g_cdd_fail_str_iequal == 0)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (a == b) {
    *out_b = true;
    return CDD_C_SUCCESS;
  }
  if (a == NULL || b == NULL) {
    *out_b = false;
    return CDD_C_SUCCESS;
  }
  while (*a && *b) {
    if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
      *out_b = false;
      return CDD_C_SUCCESS;
    }
    ++a;
    ++b;
  }
  *out_b = (*a == '\0' && *b == '\0');
  return CDD_C_SUCCESS;
}

/**
 * @brief Returns a pointer to the substring following the last occurrence of a
 * delimiter.
 *
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t c_cdd_str_after_last(const char *str, int delimiter,
                                   const char **out_s) {
  const char *last_occurrence;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_str_after_last;
  if (g_cdd_fail_str_after_last && --g_cdd_fail_str_after_last == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (str == NULL) {
    *out_s = "";
    return CDD_C_SUCCESS;
  }
  last_occurrence = strrchr(str, delimiter);
  *out_s = last_occurrence ? last_occurrence + 1 : str;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks whether a JSON reference points to a named schema component.
 *
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t c_cdd_ref_is_type(const char *ref, const char *type, int *out_b) {
  const char *extracted = NULL;
  if (ref == NULL || type == NULL) {
    *out_b = false;
    return CDD_C_SUCCESS;
  }
  {
    cdd_c_error_t rc_st = c_cdd_str_after_last(ref, '/', &extracted);
    if (rc_st != CDD_C_SUCCESS)
      return rc_st;
  }
  return c_cdd_str_equal(extracted, type, out_b);
}

/**
 * @brief Trims trailing ASCII whitespace characters from a string in place.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * NULL.
 */
cdd_c_error_t c_cdd_str_trim_trailing_whitespace(char *str) {
  size_t len;
  if (str == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  len = strlen(str);
  while (len > 0) {
    const char c = str[len - 1];
    if (isspace((unsigned char)c)) {
      str[len - 1] = '\0';
      len--;
    } else {
      break;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Unescapes a C string literal token into its raw character sequence.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
cdd_c_error_t c_cdd_destringize(const char *quoted, char **out_s) {
  size_t len, i, j;
  char *out;
  const char *inner;

  if (!quoted) {
    *out_s = NULL;
    return CDD_C_SUCCESS;
  }

  len = strlen(quoted);
  if (len < 2) {
    *out_s = NULL;
    return CDD_C_SUCCESS;
  }

  if (quoted[0] == 'L' && (len > 2 && quoted[1] == '"')) {
    inner = quoted + 2;
    len -= 3;
  } else if (quoted[0] == '"') {
    inner = quoted + 1;
    len -= 2;
  } else {
    *out_s = NULL;
    return CDD_C_SUCCESS;
  }

  out = (char *)C_CDD_MALLOC(len + 1);
  if (!out) {
    *out_s = NULL;
    return CDD_C_ERROR_MEMORY;
  }

  for (i = 0, j = 0; i < len; i++) {
    if (inner[i] == '\\') {
      if (i + 1 < len) {
        if (inner[i + 1] == '"') {
          out[j++] = '"';
          i++;
        } else if (inner[i + 1] == '\\') {
          out[j++] = '\\';
          i++;
        } else {
          out[j++] = inner[i];
        }
      } else {
        out[j++] = inner[i];
      }
    } else {
      out[j++] = inner[i];
    }
  }
  out[j] = '\0';
  *out_s = out;
  return CDD_C_SUCCESS;
}

/**
 * @brief Performs a case-insensitive comparison of two strings.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * `out_diff` is NULL.
 */
cdd_c_error_t c_cdd_stricmp(const char *a, const char *b, int *out_diff) {
  int diff;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_stricmp;
  if (g_cdd_fail_stricmp && --g_cdd_fail_stricmp == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!out_diff)
    return CDD_C_ERROR_INVALID_ARGUMENT; /* EINVAL */
  if (a == b) {
    *out_diff = 0;
    return CDD_C_SUCCESS;
  }
  if (!a || !b) {
    *out_diff = a ? 1 : -1;
    return CDD_C_SUCCESS;
  }
  while (*a && *b) {
    diff = tolower((unsigned char)*a) - tolower((unsigned char)*b);
    if (diff != 0) {
      *out_diff = diff;
      return CDD_C_SUCCESS;
    }
    ++a;
    ++b;
  }
  *out_diff = tolower((unsigned char)*a) - tolower((unsigned char)*b);
  return CDD_C_SUCCESS;
}
