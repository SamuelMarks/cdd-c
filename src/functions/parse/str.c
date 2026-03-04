/**
 * @file str.c
 * @brief Implementation of shared string utilities.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
#else
/* Linux/Unix */
#include <sys/errno.h>
#endif

#include "functions/parse/str.h"

int c_cdd_strdup(const char *const s, char **out_s) {
  if (s == NULL) {
    *out_s = NULL;
    return 0;
  }
#ifdef _WIN32
  *out_s = _strdup(s);
#else
  *out_s = strdup(s);
#endif
  return *out_s ? 0 : 12;
}

int c_cdd_str_starts_with(const char *const str, const char *const prefix,
                          bool *out_b) {
  size_t i;
  if (str == NULL || prefix == NULL) {
    *out_b = false;
    return 0;
  }
  for (i = 0; prefix[i] != '\0'; i++) {
    if (str[i] != prefix[i]) {
      *out_b = false;
      return 0;
    }
  }
  *out_b = true;
  return 0;
}

int c_cdd_str_equal(const char *a, const char *b, bool *out_b) {
  if (a == b) {
    *out_b = true;
    return 0;
  }
  if (a == NULL || b == NULL) {
    *out_b = false;
    return 0;
  }
  *out_b = (strcmp(a, b) == 0);
  return 0;
}

int c_cdd_str_iequal(const char *a, const char *b, bool *out_b) {
  if (a == b) {
    *out_b = true;
    return 0;
  }
  if (a == NULL || b == NULL) {
    *out_b = false;
    return 0;
  }
  while (*a && *b) {
    if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
      *out_b = false;
      return 0;
    }
    ++a;
    ++b;
  }
  *out_b = (*a == '\0' && *b == '\0');
  return 0;
}

int c_cdd_str_after_last(const char *const str, const int delimiter,
                         const char **out_s) {
  const char *last_occurrence;
  if (str == NULL) {
    *out_s = "";
    return 0;
  }
  last_occurrence = strrchr(str, delimiter);
  *out_s = last_occurrence ? last_occurrence + 1 : str;
  return 0;
}

int c_cdd_ref_is_type(const char *const ref, const char *const type,
                      bool *out_b) {
  const char *extracted = NULL;
  if (ref == NULL || type == NULL) {
    *out_b = false;
    return 0;
  }
  c_cdd_str_after_last(ref, '/', &extracted);
  return c_cdd_str_equal(extracted, type, out_b);
}

void c_cdd_str_trim_trailing_whitespace(char *const str) {
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

int c_cdd_destringize(const char *const quoted, char **out_s) {
  size_t len, i, j;
  char *out;
  const char *inner;

  if (!quoted) {
    *out_s = NULL;
    return 0;
  }

  len = strlen(quoted);
  if (len < 2) {
    *out_s = NULL;
    return 0;
  }

  if (quoted[0] == 'L' && (len > 2 && quoted[1] == '"')) {
    inner = quoted + 2;
    len -= 3;
  } else if (quoted[0] == '"') {
    inner = quoted + 1;
    len -= 2;
  } else {
    *out_s = NULL;
    return 0;
  }

  out = (char *)malloc(len + 1);
  if (!out) {
    *out_s = NULL;
    return 0;
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
  return 0;
}
