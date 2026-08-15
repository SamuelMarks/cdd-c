/**
 * @file url_utils.c
 * @brief Implementation of RFC 3986 URL encoding and Query serialization.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/memory.h"
#include "url_utils.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/str.h" /* For c_cdd_strdup helpers */
#include "routes/parse/url.h"
#include "c_cdd/log.h"
/* clang-format on */

/* Standard definitions for C89 compatibility */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
/** @brief sprintf_s_chk macro for MSVC */
#define sprintf_s_chk(buf, size, fmt, arg) sprintf_s(buf, size, fmt, arg)
#else
/* Naive fallback for non-MSVC C89 */
/** @brief sprintf_s_chk macro for non-MSVC fallback */
#define sprintf_s_chk(buf, size, fmt, arg) sprintf(buf, fmt, arg)
#endif

/**
 * @brief Check if a character is unreserved per RFC 3986 Section 2.3.
 *
 * Unreserved characters: ALPHA, DIGIT, "-", ".", "_", "~".
 *
 * @param[in] c Character to check.
 * @param[out] out 1 if unreserved, 0 otherwise.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t is_unreserved(unsigned char c, int *out) {
  if (isalnum(c)) {
    *out = 1;
  } else if (c == '-' || c == '.' || c == '_' || c == '~') {
    *out = 1;
  } else {
    *out = 0;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Check if a character is reserved per RFC 3986 Section 2.2.
 *
 * @param[in] c Character to check.
 * @param[out] out 1 if reserved, 0 otherwise.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t is_reserved(unsigned char c, int *out) {
  switch (c) {
  case ':':
  case '/':
  case '?':
  case '#':
  case '[':
  case ']':
  case '@':
  case '!':
  case '$':
  case '&':
  case '\'':
  case '(':
  case ')':
  case '*':
  case '+':
  case ',':
  case ';':
  case '=':
    *out = 1;
    break;
  default:
    *out = 0;
    break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Check if a character is a hexadecimal digit.
 *
 * @param[in] c Character to check.
 * @param[out] out 1 if hex, 0 otherwise.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t is_hex(unsigned char c, int *out) {
  *out = isxdigit(c) ? 1 : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Check if the string starting at p is a valid percent-encoded sequence.
 *
 * @param[in] p String pointer.
 * @param[out] out 1 if percent-encoded, 0 otherwise.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t is_pct_encoded(const char *p, int *out) {
  int h1;
  int h2;
  if (!p) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  {
    cdd_c_error_t rc = is_hex((unsigned char)p[1], &h1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  {
    cdd_c_error_t rc = is_hex((unsigned char)p[2], &h2);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  *out = (p[0] == '%' && h1 && h2) ? 1 : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Convert a nibble to hexagonal character.
 *
 * @param[in] code Nibble to convert.
 * @param[out] _out_val Output character.
 * @return Error code.
 */
static cdd_c_error_t to_hex(char code, char *_out_val) {
  static const char hex[] = "0123456789ABCDEF";
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = hex[code & 15];
  return CDD_C_SUCCESS;
}

/**
 * @brief Check if a character is unreserved for form encoding.
 *
 * @param[in] c Character to check.
 * @param[out] out 1 if unreserved, 0 otherwise.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t is_unreserved_form(unsigned char c, int *out) {
  if (isalnum(c)) {
    *out = 1;
  } else if (c == '-' || c == '.' || c == '_' || c == '*') {
    *out = 1;
  } else {
    *out = 0;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the url encode operation.
 */
cdd_c_error_t url_encode(const char *str, char **_out_val) {
  char _ast_to_hex_0;
  char _ast_to_hex_1;
  const char *p;
  char *enc = NULL;
  char *e;
  size_t needed_len = 0;
  int unres;
  cdd_c_error_t rc;

  if (!str) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  /* Pass 1: Calculate required length */
  for (p = str; *p; p++) {
    rc = is_unreserved((unsigned char)*p, &unres);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (unres) {
      needed_len++;
    } else {
      needed_len += 3; /* %HH */
    }
  }

  /* Alloc */
  enc = (char *)C_CDD_MALLOC(needed_len + 1);
  if (!enc) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }

  /* Pass 2: Encode */
  e = enc;
  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    rc = is_unreserved(c, &unres);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(enc);
      return rc;
    }
    if (unres) {
      *e++ = *p;
    } else {
      *e++ = '%';
      rc = to_hex(c >> 4, &_ast_to_hex_0);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(enc);
        return rc;
      }
      *e++ = _ast_to_hex_0;

      rc = to_hex(c & 15, &_ast_to_hex_1);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(enc);
        return rc;
      }
      *e++ = _ast_to_hex_1;
    }
  }
  *e = '\0';
  *_out_val = enc;
  return CDD_C_SUCCESS;
}
