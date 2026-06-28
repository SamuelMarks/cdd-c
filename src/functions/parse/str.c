/**
 * @file str.c
 * @brief Implementation of shared string utilities.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
#else
/* Linux/Unix */
#include <errno.h>
#endif

#include "functions/parse/str.h"

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_strdup_fail = 0;
#endif
/* clang-format on */

/**
 * @brief Executes the c cdd strdup operation.
 */
enum cdd_c_error c_cdd_strdup(const char *s, char **out_s) {
  if (s == NULL) {
    *out_s = NULL;
    return CDD_C_SUCCESS;
  }
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_strdup_fail;
    if (g_cdd_strdup_fail && --g_cdd_strdup_fail == 0) {
      *out_s = NULL;
      return CDD_C_ERROR_MEMORY;
    }
  }
#endif
#ifdef _WIN32
  *out_s = _strdup(s);

#else
  *out_s = strdup(s);
#endif
  return *out_s ? 0 : 12;
}

/**
 * @brief Executes the c cdd str starts with operation.
 */
enum cdd_c_error c_cdd_str_starts_with(const char *str, const char *prefix,
                                       int *out_b) {
  size_t i;
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
 * @brief Executes the c cdd str equal operation.
 */
enum cdd_c_error c_cdd_str_equal(const char *a, const char *b, int *out_b) {
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
 * @brief Executes the c cdd str iequal operation.
 */
enum cdd_c_error c_cdd_str_iequal(const char *a, const char *b, int *out_b) {
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
 * @brief Executes the c cdd str after last operation.
 */
enum cdd_c_error c_cdd_str_after_last(const char *str, const int delimiter,
                                      const char **out_s) {
  const char *last_occurrence;
  if (str == NULL) {
    *out_s = "";
    return CDD_C_SUCCESS;
  }
  last_occurrence = strrchr(str, delimiter);
  *out_s = last_occurrence ? last_occurrence + 1 : str;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the c cdd ref is type operation.
 */
enum cdd_c_error c_cdd_ref_is_type(const char *ref, const char *type,
                                   int *out_b) {
  const char *extracted = NULL;
  if (ref == NULL || type == NULL) {
    *out_b = false;
    return CDD_C_SUCCESS;
  }
  c_cdd_str_after_last(ref, '/', &extracted);
  return c_cdd_str_equal(extracted, type, out_b);
}

/**
 * @brief Executes the c cdd str trim trailing whitespace operation.
 */
void c_cdd_str_trim_trailing_whitespace(char *str) {
  size_t len;
  if (str == NULL)
    return;
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
}

/**
 * @brief Executes the c cdd destringize operation.
 */
#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_str_unquote_malloc_fail = 0;
#endif

enum cdd_c_error c_cdd_destringize(const char *quoted, char **out_s) {
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

#ifdef CDD_BUILD_TESTS
  if (g_str_unquote_malloc_fail) {
    out = NULL;
  } else {
#endif
    out = (char *)malloc(len + 1);
#ifdef CDD_BUILD_TESTS
  }
#endif
  if (!out) {
    *out_s = NULL;
    return CDD_C_SUCCESS;
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
 * @brief Executes the c cdd stricmp operation.
 *
 */
enum cdd_c_error c_cdd_stricmp(const char *a, const char *b, int *out_diff) {
  int diff;
  if (!out_diff)
    return 22; /* EINVAL */
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
