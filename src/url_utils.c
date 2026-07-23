/**
 * @file url_utils.c
 * @brief Implementation of RFC 3986 URL encoding and Query serialization.
 *
 * @author Samuel Marks
 */

/* clang-format off */
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
static enum cdd_c_error is_unreserved(unsigned char c, int *out) {
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
static enum cdd_c_error is_reserved(unsigned char c, int *out) {
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
static enum cdd_c_error is_hex(unsigned char c, int *out) {
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
static enum cdd_c_error is_pct_encoded(const char *p, int *out) {
  int h1;
  int h2;
  if (!p) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  is_hex((unsigned char)p[1], &h1);
  is_hex((unsigned char)p[2], &h2);
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
static enum cdd_c_error to_hex(char code, char *_out_val) {
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
static enum cdd_c_error is_unreserved_form(unsigned char c, int *out) {
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
enum cdd_c_error url_encode(const char *str, char **_out_val) {
  char _ast_to_hex_0;
  char _ast_to_hex_1;
  const char *p;
  char *enc = NULL;
  char *e;
  size_t needed_len = 0;
  int unres;
  enum cdd_c_error rc;

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
  enc = (char *)malloc(needed_len + 1);
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
      free(enc);
      return rc;
    }
    if (unres) {
      *e++ = *p;
    } else {
      *e++ = '%';
      rc = to_hex(c >> 4, &_ast_to_hex_0);
      if (rc != CDD_C_SUCCESS) {
        free(enc);
        return rc;
      }
      *e++ = _ast_to_hex_0;

      rc = to_hex(c & 15, &_ast_to_hex_1);
      if (rc != CDD_C_SUCCESS) {
        free(enc);
        return rc;
      }
      *e++ = _ast_to_hex_1;
    }
  }
  *e = '\0';

  *_out_val = enc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the url encode allow reserved operation.
 */
enum cdd_c_error url_encode_allow_reserved(const char *str, char **_out_val) {
  char _ast_to_hex_2;
  char _ast_to_hex_3;
  const char *p;
  char *enc = NULL;
  char *e;
  size_t needed_len = 0;
  int pct_enc;
  int unres;
  int res;
  enum cdd_c_error rc;

  if (!str) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  /* Pass 1: Calculate required length */
  for (p = str; *p; p++) {
    if (*p == '%') {
      rc = is_pct_encoded(p, &pct_enc);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (pct_enc) {
        needed_len += 3;
        p += 2;
        continue;
      }
    }
    rc = is_unreserved((unsigned char)*p, &unres);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = is_reserved((unsigned char)*p, &res);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (unres || res) {
      needed_len++;
    } else {
      needed_len += 3; /* %HH */
    }
  }

  enc = (char *)malloc(needed_len + 1);
  if (!enc) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }

  e = enc;
  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (*p == '%') {
      rc = is_pct_encoded(p, &pct_enc);
      if (rc != CDD_C_SUCCESS) {
        free(enc);
        return rc;
      }
      if (pct_enc) {
        *e++ = *p++;
        *e++ = *p++;
        *e++ = *p;
        continue;
      }
    }
    rc = is_unreserved(c, &unres);
    if (rc != CDD_C_SUCCESS) {
      free(enc);
      return rc;
    }
    rc = is_reserved(c, &res);
    if (rc != CDD_C_SUCCESS) {
      free(enc);
      return rc;
    }

    if (unres || res) {
      *e++ = *p;
    } else {
      *e++ = '%';
      rc = to_hex(c >> 4, &_ast_to_hex_2);
      if (rc != CDD_C_SUCCESS) {
        free(enc);
        return rc;
      }
      *e++ = _ast_to_hex_2;
      rc = to_hex(c & 15, &_ast_to_hex_3);
      if (rc != CDD_C_SUCCESS) {
        free(enc);
        return rc;
      }
      *e++ = _ast_to_hex_3;
    }
  }
  *e = '\0';
  *_out_val = enc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the url encode form operation.
 */
enum cdd_c_error url_encode_form(const char *str, char **_out_val) {
  char _ast_to_hex_4;
  char _ast_to_hex_5;
  const char *p;
  char *enc = NULL;
  char *e;
  size_t needed_len = 0;
  int unres;
  enum cdd_c_error rc;

  if (!str) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (c == ' ') {
      needed_len++;
    } else {
      rc = is_unreserved_form(c, &unres);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (unres) {
        needed_len++;
      } else {
        needed_len += 3;
      }
    }
  }

  enc = (char *)malloc(needed_len + 1);
  if (!enc) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }

  e = enc;
  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (c == ' ') {
      *e++ = '+';
    } else {
      rc = is_unreserved_form(c, &unres);
      if (rc != CDD_C_SUCCESS) {
        free(enc);
        return rc;
      }
      if (unres) {
        *e++ = *p;
      } else {
        *e++ = '%';
        rc = to_hex(c >> 4, &_ast_to_hex_4);
        if (rc != CDD_C_SUCCESS) {
          free(enc);
          return rc;
        }
        *e++ = _ast_to_hex_4;
        rc = to_hex(c & 15, &_ast_to_hex_5);
        if (rc != CDD_C_SUCCESS) {
          free(enc);
          return rc;
        }
        *e++ = _ast_to_hex_5;
      }
    }
  }
  *e = '\0';
  *_out_val = enc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the url encode form allow reserved operation.
 */
enum cdd_c_error url_encode_form_allow_reserved(const char *str,
                                                char **_out_val) {
  char _ast_to_hex_6;
  char _ast_to_hex_7;
  char _ast_to_hex_8;
  char _ast_to_hex_9;
  const char *p;
  char *enc = NULL;
  char *e;
  size_t needed_len = 0;
  int pct_enc;
  int unres;
  int res;
  enum cdd_c_error rc;

  if (!str) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (c == ' ') {
      needed_len++;
    } else {
      rc = is_pct_encoded(p, &pct_enc);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (pct_enc) {
        needed_len += 3;
        p += 2;
      } else {
        rc = is_unreserved_form(c, &unres);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = is_reserved(c, &res);
        if (rc != CDD_C_SUCCESS)
          return rc;

        if (unres || res) {
          if (c == '&' || c == '=' || c == '+') {
            needed_len += 3;
          } else {
            needed_len++;
          }
        } else {
          needed_len += 3;
        }
      }
    }
  }

  enc = (char *)malloc(needed_len + 1);
  if (!enc) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }

  e = enc;
  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (c == ' ') {
      *e++ = '+';
    } else {
      rc = is_pct_encoded(p, &pct_enc);
      if (rc != CDD_C_SUCCESS) {
        free(enc);
        return rc;
      }
      if (pct_enc) {
        *e++ = *p++;
        *e++ = *p++;
        *e++ = *p;
      } else {
        rc = is_unreserved_form(c, &unres);
        if (rc != CDD_C_SUCCESS) {
          free(enc);
          return rc;
        }
        rc = is_reserved(c, &res);
        if (rc != CDD_C_SUCCESS) {
          free(enc);
          return rc;
        }

        if (unres || res) {
          if (c == '&' || c == '=' || c == '+') {
            *e++ = '%';
            rc = to_hex(c >> 4, &_ast_to_hex_6);
            if (rc != CDD_C_SUCCESS) {
              free(enc);
              return rc;
            }
            *e++ = _ast_to_hex_6;
            rc = to_hex(c & 15, &_ast_to_hex_7);
            if (rc != CDD_C_SUCCESS) {
              free(enc);
              return rc;
            }
            *e++ = _ast_to_hex_7;
          } else {
            *e++ = *p;
          }
        } else {
          *e++ = '%';
          rc = to_hex(c >> 4, &_ast_to_hex_8);
          if (rc != CDD_C_SUCCESS) {
            free(enc);
            return rc;
          }
          *e++ = _ast_to_hex_8;
          rc = to_hex(c & 15, &_ast_to_hex_9);
          if (rc != CDD_C_SUCCESS) {
            free(enc);
            return rc;
          }
          *e++ = _ast_to_hex_9;
        }
      }
    }
  }
  *e = '\0';
  *_out_val = enc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the url query init operation.
 */
enum cdd_c_error url_query_init(struct UrlQueryParams *qp) {
  if (!qp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  qp->params = NULL;
  qp->count = 0;
  qp->capacity = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the url query free operation.
 */
void url_query_free(struct UrlQueryParams *qp) {
  size_t i;
  if (!qp)
    return;
  if (qp->params) {
    for (i = 0; i < qp->count; ++i) {
      if (qp->params[i].key)
        free(qp->params[i].key);
      if (qp->params[i].value)
        free(qp->params[i].value);
    }
    free(qp->params);
    qp->params = NULL;
  }
  qp->count = 0;
  qp->capacity = 0;
}

/**
 * @brief Executes the url query add operation.
 */
enum cdd_c_error url_query_add(struct UrlQueryParams *qp, const char *key,
                               const char *value) {
  char *_ast_strdup_0 = NULL;
  char *_ast_strdup_1 = NULL;
  enum cdd_c_error rc;

  if (!qp || !key || !value)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (qp->count >= qp->capacity) {
    size_t new_cap = (qp->capacity == 0) ? 4 : qp->capacity * 2;
    struct UrlQueryParam *new_arr = (struct UrlQueryParam *)realloc(
        qp->params, new_cap * sizeof(struct UrlQueryParam));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    qp->params = new_arr;
    qp->capacity = new_cap;
  }

  rc = c_cdd_strdup(key, &_ast_strdup_0);
  if (rc != CDD_C_SUCCESS)
    return rc;
  qp->params[qp->count].key = _ast_strdup_0;
  if (!qp->params[qp->count].key)
    return CDD_C_ERROR_MEMORY;

  rc = c_cdd_strdup(value, &_ast_strdup_1);
  if (rc != CDD_C_SUCCESS) {
    free(qp->params[qp->count].key);
    return rc;
  }
  qp->params[qp->count].value = _ast_strdup_1;
  if (!qp->params[qp->count].value) {
    free(qp->params[qp->count].key);
    return CDD_C_ERROR_MEMORY;
  }
  qp->params[qp->count].value_is_encoded = 0;

  qp->count++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the url query add encoded operation.
 */
enum cdd_c_error url_query_add_encoded(struct UrlQueryParams *qp,
                                       const char *key, const char *value) {
  char *_ast_strdup_2 = NULL;
  char *_ast_strdup_3 = NULL;
  enum cdd_c_error rc;

  if (!qp || !key || !value)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (qp->count >= qp->capacity) {
    size_t new_cap = (qp->capacity == 0) ? 4 : qp->capacity * 2;
    struct UrlQueryParam *new_arr = (struct UrlQueryParam *)realloc(
        qp->params, new_cap * sizeof(struct UrlQueryParam));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    qp->params = new_arr;
    qp->capacity = new_cap;
  }

  rc = c_cdd_strdup(key, &_ast_strdup_2);
  if (rc != CDD_C_SUCCESS)
    return rc;
  qp->params[qp->count].key = _ast_strdup_2;
  if (!qp->params[qp->count].key)
    return CDD_C_ERROR_MEMORY;

  rc = c_cdd_strdup(value, &_ast_strdup_3);
  if (rc != CDD_C_SUCCESS) {
    free(qp->params[qp->count].key);
    return rc;
  }
  qp->params[qp->count].value = _ast_strdup_3;
  if (!qp->params[qp->count].value) {
    free(qp->params[qp->count].key);
    return CDD_C_ERROR_MEMORY;
  }
  qp->params[qp->count].value_is_encoded = 1;

  qp->count++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the url query build operation.
 */
enum cdd_c_error url_query_build(const struct UrlQueryParams *qp,
                                 char **out_str) {
  char *_ast_url_encode_10 = NULL;
  char *_ast_url_encode_11 = NULL;
  char *_ast_url_encode_12 = NULL;
  char *_ast_url_encode_13 = NULL;
  char *_ast_strdup_4 = NULL;
  char *_ast_strdup_5 = NULL;
  char *_ast_strdup_6 = NULL;
  size_t i;
  size_t total_len = 0;
  char *buf = NULL;
  char *ptr = NULL;
  enum cdd_c_error rc;

  if (!qp || !out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (qp->count == 0) {
    rc = c_cdd_strdup("", &_ast_strdup_4);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_str = _ast_strdup_4;
    return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }

  total_len = 1; /* '?' */

  for (i = 0; i < qp->count; ++i) {
    char *e_key = NULL;
    char *e_val = NULL;
    const char *raw_val = qp->params[i].value;

    rc = url_encode(qp->params[i].key, &_ast_url_encode_10);
    if (rc != CDD_C_SUCCESS)
      return rc;
    e_key = _ast_url_encode_10;

    if (qp->params[i].value_is_encoded) {
      rc = c_cdd_strdup(raw_val ? raw_val : "", &_ast_strdup_5);
      if (rc != CDD_C_SUCCESS) {
        free(e_key);
        return rc;
      }
      e_val = _ast_strdup_5;
    } else {
      rc = url_encode(raw_val, &_ast_url_encode_11);
      if (rc != CDD_C_SUCCESS) {
        free(e_key);
        return rc;
      }
      e_val = _ast_url_encode_11;
    }

    if (!e_key || !e_val) {
      if (e_key)
        free(e_key);
      if (e_val)
        free(e_val);
      return CDD_C_ERROR_MEMORY;
    }

    total_len += strlen(e_key) + 1; /* key= */
    total_len += strlen(e_val);
    if (i < qp->count - 1)
      total_len += 1; /* & */

    free(e_key);
    free(e_val);
  }

  buf = (char *)malloc(total_len + 1);
  if (!buf) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  ptr = buf;
  *ptr++ = '?';

  for (i = 0; i < qp->count; ++i) {
    char *e_key = NULL;
    char *e_val = NULL;
    size_t kl, vl;
    const char *raw_val = qp->params[i].value;

    rc = url_encode(qp->params[i].key, &_ast_url_encode_12);
    if (rc != CDD_C_SUCCESS) {
      free(buf);
      return rc;
    }
    e_key = _ast_url_encode_12;

    if (qp->params[i].value_is_encoded) {
      rc = c_cdd_strdup(raw_val ? raw_val : "", &_ast_strdup_6);
      if (rc != CDD_C_SUCCESS) {
        free(e_key);
        free(buf);
        return rc;
      }
      e_val = _ast_strdup_6;
    } else {
      rc = url_encode(raw_val, &_ast_url_encode_13);
      if (rc != CDD_C_SUCCESS) {
        free(e_key);
        free(buf);
        return rc;
      }
      e_val = _ast_url_encode_13;
    }

    kl = strlen(e_key);
    memcpy(ptr, e_key, kl);
    ptr += kl;

    *ptr++ = '=';

    vl = strlen(e_val);
    memcpy(ptr, e_val, vl);
    ptr += vl;

    if (i < qp->count - 1) {
      *ptr++ = '&';
    }

    free(e_key);
    free(e_val);
  }
  *ptr = '\0';

  *out_str = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the url query build form operation.
 */
enum cdd_c_error url_query_build_form(const struct UrlQueryParams *qp,
                                      char **out_str) {
  char *_ast_url_encode_form_14 = NULL;
  char *_ast_url_encode_form_15 = NULL;
  char *_ast_url_encode_form_16 = NULL;
  char *_ast_url_encode_form_17 = NULL;
  char *_ast_strdup_7 = NULL;
  char *_ast_strdup_8 = NULL;
  size_t i;
  size_t total_len = 0;
  char *buf;
  char *ptr;
  enum cdd_c_error rc;

  if (!qp || !out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (qp->count == 0) {
    *out_str = (char *)calloc(1, 1);
    if (!*out_str)
      return CDD_C_ERROR_MEMORY;
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < qp->count; ++i) {
    char *e_key = NULL;
    char *e_val = NULL;
    size_t kl, vl;

    rc = url_encode_form(qp->params[i].key, &_ast_url_encode_form_14);
    if (rc != CDD_C_SUCCESS)
      return rc;
    e_key = _ast_url_encode_form_14;

    if (!e_key) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }

    if (qp->params[i].value_is_encoded) {
      rc = c_cdd_strdup(qp->params[i].value, &_ast_strdup_7);
      if (rc != CDD_C_SUCCESS) {
        free(e_key);
        return rc;
      }
      e_val = _ast_strdup_7;
    } else {
      rc = url_encode_form(qp->params[i].value, &_ast_url_encode_form_15);
      if (rc != CDD_C_SUCCESS) {
        free(e_key);
        return rc;
      }
      e_val = _ast_url_encode_form_15;
    }
    if (!e_val) {
      free(e_key);
      return CDD_C_ERROR_MEMORY;
    }
    kl = strlen(e_key);
    vl = strlen(e_val);
    total_len += kl + 1 + vl;
    if (i + 1 < qp->count)
      total_len += 1;
    free(e_key);
    free(e_val);
  }

  buf = (char *)malloc(total_len + 1);
  if (!buf) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  ptr = buf;

  for (i = 0; i < qp->count; ++i) {
    char *e_key = NULL;
    char *e_val = NULL;
    size_t kl, vl;

    rc = url_encode_form(qp->params[i].key, &_ast_url_encode_form_16);
    if (rc != CDD_C_SUCCESS) {
      free(buf);
      return rc;
    }
    e_key = _ast_url_encode_form_16;

    if (!e_key) {
      free(buf);
      return CDD_C_ERROR_MEMORY;
    }

    if (qp->params[i].value_is_encoded) {
      rc = c_cdd_strdup(qp->params[i].value, &_ast_strdup_8);
      if (rc != CDD_C_SUCCESS) {
        free(e_key);
        free(buf);
        return rc;
      }
      e_val = _ast_strdup_8;
    } else {
      rc = url_encode_form(qp->params[i].value, &_ast_url_encode_form_17);
      if (rc != CDD_C_SUCCESS) {
        free(e_key);
        free(buf);
        return rc;
      }
      e_val = _ast_url_encode_form_17;
    }
    if (!e_val) {
      free(e_key);
      free(buf);
      return CDD_C_ERROR_MEMORY;
    }
    kl = strlen(e_key);
    vl = strlen(e_val);
    memcpy(ptr, e_key, kl);
    ptr += kl;
    *ptr++ = '=';
    memcpy(ptr, e_val, vl);
    ptr += vl;
    if (i + 1 < qp->count)
      *ptr++ = '&';
    free(e_key);
    free(e_val);
  }
  *ptr = '\0';
  *out_str = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Append string to a dynamic buffer.
 *
 * @param[in,out] buf Pointer to the buffer.
 * @param[in,out] len Pointer to the current length.
 * @param[in,out] cap Pointer to the current capacity.
 * @param[in] s String to append.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
static enum cdd_c_error append_str(char **buf, size_t *len, size_t *cap,
                                   const char *s) {
  size_t slen;
  size_t need;
  char *tmp;

  if (!buf || !len || !cap || !s)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  slen = strlen(s);
  need = *len + slen + 1;
  if (need > *cap) {
    size_t new_cap = (*cap == 0) ? 64 : *cap * 2;
    while (new_cap < need)
      new_cap *= 2;
    tmp = (char *)realloc(*buf, new_cap);
    if (!tmp) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    *buf = tmp;
    *cap = new_cap;
  }
  memcpy(*buf + *len, s, slen);
  *len += slen;
  (*buf)[*len] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Convert KV value to a string literal or formatted string.
 *
 * @param[in] kv The OpenAPI_KV to process.
 * @param[in,out] buf Buffer for numeric conversion.
 * @param[in] buf_len Size of buffer.
 * @param[out] _out_val The resulting string pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
static enum cdd_c_error kv_value_to_string(const struct OpenAPI_KV *kv,
                                           char *buf, size_t buf_len,
                                           const char **_out_val) {
  if (!kv) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  switch (kv->type) {
  case OA_KV_STRING: {
    *_out_val = kv->value.s ? kv->value.s : NULL;
    return CDD_C_SUCCESS;
  }
  case OA_KV_INTEGER:
    if (!buf || buf_len == 0) {
      *_out_val = NULL;
      return CDD_C_SUCCESS;
    }
    sprintf_s_chk(buf, buf_len, "%d", kv->value.i);
    *_out_val = buf;
    return CDD_C_SUCCESS;
  case OA_KV_NUMBER:
    if (!buf || buf_len == 0) {
      *_out_val = NULL;
      return CDD_C_SUCCESS;
    }
    sprintf_s_chk(buf, buf_len, "%g", kv->value.n);
    *_out_val = buf;
    return CDD_C_SUCCESS;
  case OA_KV_BOOLEAN: {
    *_out_val = kv->value.b ? "true" : "false";
    return CDD_C_SUCCESS;
  }
  default: {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  }
}

/**
 * @brief Executes the openapi kv join form operation.
 */
enum cdd_c_error openapi_kv_join_form(const struct OpenAPI_KV *kvs, size_t n,
                                      const char *delim, int allow_reserved,
                                      char **_out_val) {
  const char *_ast_kv_value_to_string_18 = NULL;
  size_t i;
  char *buf = NULL;
  size_t len = 0;
  size_t cap = 0;
  char num_buf[64];
  char *enc_key = NULL;
  char *enc_val = NULL;
  enum cdd_c_error (*enc_fn)(const char *, char **) =
      allow_reserved ? url_encode_form_allow_reserved : url_encode_form;
  enum cdd_c_error rc = CDD_C_SUCCESS;
  enum cdd_c_error enc_rc;
  const char *raw_val;

  if (!delim)
    delim = ",";

  if (!kvs || n == 0) {
    buf = (char *)calloc(1, 1);
    if (!buf)
      return CDD_C_ERROR_MEMORY;
    *_out_val = buf;
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < n; ++i) {
    if (!kvs[i].key)
      continue;

    rc = kv_value_to_string(&kvs[i], num_buf, sizeof(num_buf),
                            &_ast_kv_value_to_string_18);
    if (rc != CDD_C_SUCCESS)
      goto oom;
    raw_val = _ast_kv_value_to_string_18;

    if (!raw_val)
      continue;

    enc_rc = enc_fn(kvs[i].key, &enc_key);
    if (enc_rc != CDD_C_SUCCESS) {
      rc = enc_rc;
      goto oom;
    }

    if (!enc_key)
      goto oom;

    enc_rc = enc_fn(raw_val, &enc_val);
    if (enc_rc != CDD_C_SUCCESS) {
      rc = enc_rc;
      goto oom;
    }

    if (!enc_val)
      goto oom;

    if (len > 0) {
      rc = append_str(&buf, &len, &cap, delim);
      if (rc != CDD_C_SUCCESS)
        goto oom;
    }

    rc = append_str(&buf, &len, &cap, enc_key);
    if (rc != CDD_C_SUCCESS)
      goto oom;
    rc = append_str(&buf, &len, &cap, delim);
    if (rc != CDD_C_SUCCESS)
      goto oom;
    rc = append_str(&buf, &len, &cap, enc_val);
    if (rc != CDD_C_SUCCESS)
      goto oom;

    free(enc_key);
    free(enc_val);
    enc_key = NULL;
    enc_val = NULL;
  }

  if (!buf) {
    buf = (char *)calloc(1, 1);
    if (!buf)
      return CDD_C_ERROR_MEMORY;
  }

  *_out_val = buf;
  return CDD_C_SUCCESS;

oom:
  if (enc_key)
    free(enc_key);
  if (enc_val)
    free(enc_val);
  if (buf)
    free(buf);
  *_out_val = NULL;
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  return CDD_C_ERROR_MEMORY;
}
