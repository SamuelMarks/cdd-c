/**
 * @file str_utils.c
 * @brief Implementation of shared string utilities.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
#else
/* Linux/Unix */
#include <sys/errno.h>
#endif

#include "str_utils.h"

char *c_cdd_strdup(const char *const s) {
  if (s == NULL)
    return NULL;
  return strdup(s);
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

bool c_cdd_str_equal(const char *const a, const char *const b) {
  if (a == b)
    return true; /* Handles both NULL case */
  if (a == NULL || b == NULL)
    return false; /* One is NULL, other is not */
  return strcmp(a, b) == 0;
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
