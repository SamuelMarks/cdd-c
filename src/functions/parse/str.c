/**
 * @file str_utils.c
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

char *c_cdd_strdup(const char *const s) {
  if (s == NULL)
    return NULL;
#ifdef _WIN32
  return _strdup(s);
#else
  return strdup(s);
#endif
}

bool c_cdd_str_starts_with(const char *const str, const char *const prefix) {
  size_t i;
  if (str == NULL || prefix == NULL)
    return false;
  for (i = 0; prefix[i] != '\0'; i++) {
    if (str[i] != prefix[i])
      return false;
  }
  return true;
}

bool c_cdd_str_equal(const char *a, const char *b) {
  if (a == b)
    return true; /* Handles both NULL case */
  if (a == NULL || b == NULL)
    return false; /* One is NULL, other is not */
  return strcmp(a, b) == 0;
}

bool c_cdd_str_iequal(const char *a, const char *b) {
  if (a == b)
    return true; /* Handles both NULL case */
  if (a == NULL || b == NULL)
    return false; /* One is NULL, other is not */
  while (*a && *b) {
    if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
      return false;
    ++a;
    ++b;
  }
  return *a == '\0' && *b == '\0';
}

const char *c_cdd_str_after_last(const char *const str, const int delimiter) {
  const char *last_occurrence;
  if (str == NULL)
    return "";
  last_occurrence = strrchr(str, delimiter);
  return last_occurrence ? last_occurrence + 1 : str;
}

bool c_cdd_ref_is_type(const char *const ref, const char *const type) {
  const char *extracted;
  if (ref == NULL || type == NULL)
    return false;
  extracted = c_cdd_str_after_last(ref, '/');
  return strcmp(extracted, type) == 0;
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

char *c_cdd_destringize(const char *const quoted) {
  size_t len, i, j;
  char *out;
  const char *inner;

  if (!quoted)
    return NULL;

  len = strlen(quoted);
  if (len < 2)
    return NULL; /* Malformed literal */

  /* Simple check for "..." structure */
  /* Handles optional prefix L"..." */
  if (quoted[0] == 'L' && (len > 2 && quoted[1] == '"')) {
    inner = quoted + 2;
    len -= 3; /* L, ", and trailing " */
  } else if (quoted[0] == '"') {
    inner = quoted + 1;
    len -= 2; /* ", and trailing " */
  } else {
    return NULL; /* Not a standard double-quoted string literal */
  }

  /* Allocation bound: destringized length <= inner length */
  out = (char *)malloc(len + 1);
  if (!out)
    return NULL;

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
          /* Copy other escapes literally as per spec phase logic? */
          /* 6.10.9: "replacing each escape sequence \" by a double-quote, and
           * replacing each escape sequence \\ by a single backslash" */
          /* It does not mention other escapes. They should remain literally in
           * the output token stream? */
          /* Standard says: The result is a sequence of preprocessing tokens. */
          /* Here we just perform the specified mapping. */
          out[j++] = inner[i];
        }
      } else {
        out[j++] = inner[i]; /* Trailing backslash? */
      }
    } else {
      out[j++] = inner[i];
    }
  }
  out[j] = '\0';
  return out;
}
