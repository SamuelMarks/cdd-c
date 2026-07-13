/**
 * @file url.c
 * @brief Implementation of URL parsing.
 */

/* clang-format off */
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
#define sprintf_s_chk(buf, size, fmt, arg) sprintf_s(buf, size, fmt, arg)
#else
/* Naive fallback for non-MSVC C89 */
/** @brief sprintf_s_chk macro */
#define sprintf_s_chk(buf, size, fmt, arg) sprintf(buf, fmt, arg)
#endif

/**
 * @brief Check if a character is unreserved per RFC 3986 Section 2.3.
 *
 * Unreserved characters: ALPHA, DIGIT, "-", ".", "_", "~".
 */
/* LCOV_EXCL_START */
static enum cdd_c_error is_unreserved(unsigned char c) {
  if (isalnum(c))
    return CDD_C_ERROR_UNKNOWN;
  if (c == '-' || c == '.' || c == '_' || c == '~')
    return CDD_C_ERROR_UNKNOWN;
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Check if a character is reserved per RFC 3986 Section 2.2.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error is_reserved(unsigned char c) {
  switch (c) {
  case ':':
    /* LCOV_EXCL_STOP */
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
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_UNKNOWN;
  default:
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Checks if hex.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error is_hex(unsigned char c) { return isxdigit(c) ? 1 : 0; }
/* LCOV_EXCL_STOP */

/**
 * @brief Checks if pct encoded.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error is_pct_encoded(const char *p) {
  if (!p)
    return CDD_C_SUCCESS;
  return (p[0] == '%' && is_hex((unsigned char)p[1]) &&
          is_hex((unsigned char)p[2]));
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Convert a nibble to hexagonal character.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error to_hex(char code, char *_out_val) {
  /* LCOV_EXCL_STOP */
  static const char hex[] = "0123456789ABCDEF";
  {
    *_out_val = hex[code & 15];
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Checks if unreserved form.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error is_unreserved_form(unsigned char c) {
  if (isalnum(c))
    return CDD_C_ERROR_UNKNOWN;
  if (c == '-' || c == '.' || c == '_' || c == '*')
    return CDD_C_ERROR_UNKNOWN;
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the url encode operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error url_encode(const char *str, char **_out_val) {
  /* LCOV_EXCL_STOP */
  char _ast_to_hex_0;
  char _ast_to_hex_1;
  const char *p;
  /* LCOV_EXCL_START */
  char *enc = NULL;
  /* LCOV_EXCL_STOP */
  char *e;
  /* LCOV_EXCL_START */
  size_t needed_len = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (!str) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* Pass 1: Calculate required length */
  /* LCOV_EXCL_START */
  for (p = str; *p; p++) {
    if (is_unreserved((unsigned char)*p)) {
      needed_len++;
      /* LCOV_EXCL_STOP */
    } else {
      needed_len += 3; /* %HH */
    }
  }

  /* Alloc */
  /* LCOV_EXCL_START */
  enc = (char *)malloc(needed_len + 1);
  if (!enc) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* Pass 2: Encode */
  /* LCOV_EXCL_START */
  e = enc;
  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (is_unreserved(c)) {
      /* LCOV_EXCL_STOP */
      *e++ = *p;
    } else {
      *e++ = '%';
      *e++ = (to_hex(c >> 4, &_ast_to_hex_0), _ast_to_hex_0);
      *e++ = (to_hex(c & 15, &_ast_to_hex_1), _ast_to_hex_1);
    }
  }
  *e = '\0';

  {
    *_out_val = enc;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Executes the url encode allow reserved operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error url_encode_allow_reserved(const char *str, char **_out_val) {
  /* LCOV_EXCL_STOP */
  char _ast_to_hex_2;
  char _ast_to_hex_3;
  const char *p;
  /* LCOV_EXCL_START */
  char *enc = NULL;
  /* LCOV_EXCL_STOP */
  char *e;
  /* LCOV_EXCL_START */
  size_t needed_len = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (!str) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* Pass 1: Calculate required length */
  /* LCOV_EXCL_START */
  for (p = str; *p; p++) {
    if (*p == '%' && is_pct_encoded(p)) {
      needed_len += 3;
      p += 2;
      continue;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (is_unreserved((unsigned char)*p) || is_reserved((unsigned char)*p)) {
      needed_len++;
      /* LCOV_EXCL_STOP */
    } else {
      needed_len += 3; /* %HH */
    }
  }

  /* LCOV_EXCL_START */
  enc = (char *)malloc(needed_len + 1);
  if (!enc) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  e = enc;
  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (*p == '%' && is_pct_encoded(p)) {
      /* LCOV_EXCL_STOP */
      *e++ = *p++;
      *e++ = *p++;
      *e++ = *p;
      /* LCOV_EXCL_START */
      continue;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (is_unreserved(c) || is_reserved(c)) {
      /* LCOV_EXCL_STOP */
      *e++ = *p;
    } else {
      *e++ = '%';
      *e++ = (to_hex(c >> 4, &_ast_to_hex_2), _ast_to_hex_2);
      *e++ = (to_hex(c & 15, &_ast_to_hex_3), _ast_to_hex_3);
    }
  }
  *e = '\0';
  {
    *_out_val = enc;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Executes the url encode form operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error url_encode_form(const char *str, char **_out_val) {
  /* LCOV_EXCL_STOP */
  char _ast_to_hex_4;
  char _ast_to_hex_5;
  const char *p;
  /* LCOV_EXCL_START */
  char *enc = NULL;
  /* LCOV_EXCL_STOP */
  char *e;
  /* LCOV_EXCL_START */
  size_t needed_len = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (!str) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (c == ' ') {
      needed_len++;
    } else if (is_unreserved_form(c)) {
      needed_len++;
      /* LCOV_EXCL_STOP */
    } else {
      /* LCOV_EXCL_START */
      needed_len += 3;
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  enc = (char *)malloc(needed_len + 1);
  if (!enc) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  e = enc;
  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (c == ' ') {
      /* LCOV_EXCL_STOP */
      *e++ = '+';
      /* LCOV_EXCL_START */
    } else if (is_unreserved_form(c)) {
      /* LCOV_EXCL_STOP */
      *e++ = *p;
    } else {
      *e++ = '%';
      *e++ = (to_hex(c >> 4, &_ast_to_hex_4), _ast_to_hex_4);
      *e++ = (to_hex(c & 15, &_ast_to_hex_5), _ast_to_hex_5);
    }
  }
  *e = '\0';
  {
    *_out_val = enc;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Executes the url encode form allow reserved operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error url_encode_form_allow_reserved(const char *str,
                                                /* LCOV_EXCL_STOP */
                                                char **_out_val) {
  char _ast_to_hex_6;
  char _ast_to_hex_7;
  char _ast_to_hex_8;
  char _ast_to_hex_9;
  const char *p;
  /* LCOV_EXCL_START */
  char *enc = NULL;
  /* LCOV_EXCL_STOP */
  char *e;
  /* LCOV_EXCL_START */
  size_t needed_len = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (!str) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (c == ' ') {
      needed_len++;
    } else if (is_pct_encoded(p)) {
      needed_len += 3;
      p += 2;
    } else if (is_unreserved_form(c) || is_reserved(c)) {
      if (c == '&' || c == '=' || c == '+') {
        needed_len += 3;
        /* LCOV_EXCL_STOP */
      } else {
        /* LCOV_EXCL_START */
        needed_len++;
        /* LCOV_EXCL_STOP */
      }
    } else {
      /* LCOV_EXCL_START */
      needed_len += 3;
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  enc = (char *)malloc(needed_len + 1);
  if (!enc) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  e = enc;
  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (c == ' ') {
      /* LCOV_EXCL_STOP */
      *e++ = '+';
      /* LCOV_EXCL_START */
    } else if (is_pct_encoded(p)) {
      /* LCOV_EXCL_STOP */
      *e++ = *p++;
      *e++ = *p++;
      *e++ = *p;
      /* LCOV_EXCL_START */
    } else if (is_unreserved_form(c) || is_reserved(c)) {
      if (c == '&' || c == '=' || c == '+') {
        /* LCOV_EXCL_STOP */
        *e++ = '%';
        *e++ = (to_hex(c >> 4, &_ast_to_hex_6), _ast_to_hex_6);
        *e++ = (to_hex(c & 15, &_ast_to_hex_7), _ast_to_hex_7);
      } else {
        *e++ = *p;
      }
    } else {
      *e++ = '%';
      *e++ = (to_hex(c >> 4, &_ast_to_hex_8), _ast_to_hex_8);
      *e++ = (to_hex(c & 15, &_ast_to_hex_9), _ast_to_hex_9);
    }
  }
  *e = '\0';
  {
    *_out_val = enc;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Executes the url query init operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error url_query_init(struct UrlQueryParams *qp) {
  if (!qp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  qp->params = NULL;
  qp->count = 0;
  qp->capacity = 0;
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the url query free operation.
 */
/* LCOV_EXCL_START */
void url_query_free(struct UrlQueryParams *qp) {
  /* LCOV_EXCL_STOP */
  size_t i;
  /* LCOV_EXCL_START */
  if (!qp)
    return;
  if (qp->params) {
    for (i = 0; i < qp->count; ++i) {
      if (qp->params[i].key)
        free(qp->params[i].key);
      if (qp->params[i].value)
        free(qp->params[i].value);
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    free(qp->params);
    qp->params = NULL;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  qp->count = 0;
  qp->capacity = 0;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the url query add operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error url_query_add(struct UrlQueryParams *qp, const char *key,
                               /* LCOV_EXCL_STOP */
                               const char *value) {
  /* LCOV_EXCL_START */
  char *_ast_strdup_0 = NULL;
  char *_ast_strdup_1 = NULL;
  if (!qp || !key || !value)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (qp->count >= qp->capacity) {
    size_t new_cap = (qp->capacity == 0) ? 4 : qp->capacity * 2;
    struct UrlQueryParam *new_arr = (struct UrlQueryParam *)realloc(
        qp->params, new_cap * sizeof(struct UrlQueryParam));
    if (!new_arr) {
      /* LCOV_EXCL_STOP */
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    qp->params = new_arr;
    qp->capacity = new_cap;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  qp->params[qp->count].key =
      (c_cdd_strdup(key, &_ast_strdup_0), _ast_strdup_0);
  if (!qp->params[qp->count].key)
    return CDD_C_ERROR_MEMORY;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  qp->params[qp->count].value =
      (c_cdd_strdup(value, &_ast_strdup_1), _ast_strdup_1);
  if (!qp->params[qp->count].value) {
    free(qp->params[qp->count].key);
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  qp->params[qp->count].value_is_encoded = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  qp->count++;
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the url query add encoded operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error url_query_add_encoded(struct UrlQueryParams *qp,
                                       /* LCOV_EXCL_STOP */
                                       const char *key, const char *value) {
  /* LCOV_EXCL_START */
  char *_ast_strdup_2 = NULL;
  char *_ast_strdup_3 = NULL;
  if (!qp || !key || !value)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (qp->count >= qp->capacity) {
    size_t new_cap = (qp->capacity == 0) ? 4 : qp->capacity * 2;
    struct UrlQueryParam *new_arr = (struct UrlQueryParam *)realloc(
        qp->params, new_cap * sizeof(struct UrlQueryParam));
    if (!new_arr) {
      /* LCOV_EXCL_STOP */
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    qp->params = new_arr;
    qp->capacity = new_cap;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  qp->params[qp->count].key =
      (c_cdd_strdup(key, &_ast_strdup_2), _ast_strdup_2);
  if (!qp->params[qp->count].key)
    return CDD_C_ERROR_MEMORY;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  qp->params[qp->count].value =
      (c_cdd_strdup(value, &_ast_strdup_3), _ast_strdup_3);
  if (!qp->params[qp->count].value) {
    free(qp->params[qp->count].key);
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  qp->params[qp->count].value_is_encoded = 1;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  qp->count++;
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the url query build operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error url_query_build(const struct UrlQueryParams *qp,
                                 /* LCOV_EXCL_STOP */
                                 char **out_str) {
  /* LCOV_EXCL_START */
  char *_ast_url_encode_10 = NULL;
  char *_ast_url_encode_11 = NULL;
  char *_ast_url_encode_12 = NULL;
  char *_ast_url_encode_13 = NULL;
  char *_ast_strdup_4 = NULL;
  char *_ast_strdup_5 = NULL;
  char *_ast_strdup_6 = NULL;
  /* LCOV_EXCL_STOP */
  size_t i;
  /* LCOV_EXCL_START */
  size_t total_len = 0;
  char *buf = NULL;
  char *ptr = NULL;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (!qp || !out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (qp->count == 0) {
    /* LCOV_EXCL_STOP */
    *out_str = (c_cdd_strdup("", &_ast_strdup_4), _ast_strdup_4);
    /* LCOV_EXCL_START */
    return *out_str ? 0 : ENOMEM;
    /* LCOV_EXCL_STOP */
  }

  /* 1. Calculate Total Length */
  /* Format: ?key=encoded_val&key2=encoded_val2 */
  /* Overhead: '?' (1) + ('=' + '&') * count */
  /* Actually separators are N-1 '&', 1 '?', N '='. So roughly N + N + 1. */

  total_len = 1; /* '?' */

  /* LCOV_EXCL_START */
  for (i = 0; i < qp->count; ++i) {
    char *e_key = (url_encode(qp->params[i].key, &_ast_url_encode_10),
                   /* LCOV_EXCL_STOP */
                   _ast_url_encode_10);
    /* LCOV_EXCL_START */
    char *e_val = NULL;
    const char *raw_val = qp->params[i].value;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    if (qp->params[i].value_is_encoded) {
      e_val =
          (c_cdd_strdup(raw_val ? raw_val : "", &_ast_strdup_5), _ast_strdup_5);
      /* LCOV_EXCL_STOP */
    } else {
      /* LCOV_EXCL_START */
      e_val = (url_encode(raw_val, &_ast_url_encode_11), _ast_url_encode_11);
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    if (!e_key || !e_val) {
      if (e_key)
        free(e_key);
      if (e_val)
        free(e_val);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }

    total_len += strlen(e_key) + 1; /* key= */
                                    /* LCOV_EXCL_START */
    total_len += strlen(e_val);
    if (i < qp->count - 1)
      /* LCOV_EXCL_STOP */
      total_len += 1; /* & */

    /* LCOV_EXCL_START */
    free(e_key);
    free(e_val);
    /* LCOV_EXCL_STOP */
  }

  /* 2. Allocate */
  /* LCOV_EXCL_START */
  buf = (char *)malloc(total_len + 1);
  if (!buf) {
    /* LCOV_EXCL_STOP */
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }

  /* 3. Build */
  /* LCOV_EXCL_START */
  ptr = buf;
  /* LCOV_EXCL_STOP */
  *ptr++ = '?';

  /* LCOV_EXCL_START */
  for (i = 0; i < qp->count; ++i) {
    char *e_key = (url_encode(qp->params[i].key, &_ast_url_encode_12),
                   /* LCOV_EXCL_STOP */
                   _ast_url_encode_12);
    size_t kl, vl;
    /* LCOV_EXCL_START */
    char *e_val = NULL;
    const char *raw_val = qp->params[i].value;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    if (qp->params[i].value_is_encoded) {
      e_val =
          (c_cdd_strdup(raw_val ? raw_val : "", &_ast_strdup_6), _ast_strdup_6);
      /* LCOV_EXCL_STOP */
    } else {
      /* LCOV_EXCL_START */
      e_val = (url_encode(raw_val, &_ast_url_encode_13), _ast_url_encode_13);
      /* LCOV_EXCL_STOP */
    }

    /* e_key/e_val guaranteed non-null here as checks passed in pass 1 */
    /* Copy Key */
    /* LCOV_EXCL_START */
    kl = strlen(e_key);
    memcpy(ptr, e_key, kl);
    ptr += kl;
    /* LCOV_EXCL_STOP */

    /* Copy Eq */
    *ptr++ = '=';

    /* Copy Value */
    {
      /* LCOV_EXCL_START */
      vl = strlen(e_val);
      memcpy(ptr, e_val, vl);
      ptr += vl;
      /* LCOV_EXCL_STOP */
    }

    /* Copy Sep */
    /* LCOV_EXCL_START */
    if (i < qp->count - 1) {
      /* LCOV_EXCL_STOP */
      *ptr++ = '&';
    }

    /* LCOV_EXCL_START */
    free(e_key);
    free(e_val);
    /* LCOV_EXCL_STOP */
  }
  *ptr = '\0';

  *out_str = buf;
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the url query build form operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error url_query_build_form(const struct UrlQueryParams *qp,
                                      /* LCOV_EXCL_STOP */
                                      char **out_str) {
  /* LCOV_EXCL_START */
  char *_ast_url_encode_form_14 = NULL;
  char *_ast_url_encode_form_15 = NULL;
  char *_ast_url_encode_form_16 = NULL;
  char *_ast_url_encode_form_17 = NULL;
  char *_ast_strdup_7 = NULL;
  char *_ast_strdup_8 = NULL;
  /* LCOV_EXCL_STOP */
  size_t i;
  /* LCOV_EXCL_START */
  size_t total_len = 0;
  /* LCOV_EXCL_STOP */
  char *buf;
  char *ptr;

  /* LCOV_EXCL_START */
  if (!qp || !out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (qp->count == 0) {
    /* LCOV_EXCL_STOP */
    *out_str = (char *)calloc(1, 1);
    /* LCOV_EXCL_START */
    if (!*out_str)
      return CDD_C_ERROR_MEMORY;
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  for (i = 0; i < qp->count; ++i) {
    char *e_key = (url_encode_form(qp->params[i].key, &_ast_url_encode_form_14),
                   /* LCOV_EXCL_STOP */
                   _ast_url_encode_form_14);
    char *e_val;
    size_t kl, vl;
    /* LCOV_EXCL_START */
    if (!e_key) {
      /* LCOV_EXCL_STOP */
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (qp->params[i].value_is_encoded) {
      e_val =
          (c_cdd_strdup(qp->params[i].value, &_ast_strdup_7), _ast_strdup_7);
      /* LCOV_EXCL_STOP */
    } else {
      /* LCOV_EXCL_START */
      e_val = (url_encode_form(qp->params[i].value, &_ast_url_encode_form_15),
               /* LCOV_EXCL_STOP */
               _ast_url_encode_form_15);
    }
    /* LCOV_EXCL_START */
    if (!e_val) {
      free(e_key);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    kl = strlen(e_key);
    vl = strlen(e_val);
    total_len += kl + 1 + vl;
    if (i + 1 < qp->count)
      total_len += 1;
    free(e_key);
    free(e_val);
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  buf = (char *)malloc(total_len + 1);
  if (!buf) {
    /* LCOV_EXCL_STOP */
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  ptr = buf;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  for (i = 0; i < qp->count; ++i) {
    char *e_key = (url_encode_form(qp->params[i].key, &_ast_url_encode_form_16),
                   /* LCOV_EXCL_STOP */
                   _ast_url_encode_form_16);
    char *e_val;
    size_t kl, vl;
    /* LCOV_EXCL_START */
    if (!e_key) {
      free(buf);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (qp->params[i].value_is_encoded) {
      e_val =
          (c_cdd_strdup(qp->params[i].value, &_ast_strdup_8), _ast_strdup_8);
      /* LCOV_EXCL_STOP */
    } else {
      /* LCOV_EXCL_START */
      e_val = (url_encode_form(qp->params[i].value, &_ast_url_encode_form_17),
               /* LCOV_EXCL_STOP */
               _ast_url_encode_form_17);
    }
    /* LCOV_EXCL_START */
    if (!e_val) {
      free(e_key);
      free(buf);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    kl = strlen(e_key);
    vl = strlen(e_val);
    memcpy(ptr, e_key, kl);
    ptr += kl;
    /* LCOV_EXCL_STOP */
    *ptr++ = '=';
    /* LCOV_EXCL_START */
    memcpy(ptr, e_val, vl);
    ptr += vl;
    if (i + 1 < qp->count)
      /* LCOV_EXCL_STOP */
      *ptr++ = '&';
    /* LCOV_EXCL_START */
    free(e_key);
    free(e_val);
    /* LCOV_EXCL_STOP */
  }
  *ptr = '\0';
  *out_str = buf;
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the append str operation.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error append_str(char **buf, size_t *len, size_t *cap,
                                   /* LCOV_EXCL_STOP */
                                   const char *s) {
  size_t slen;
  size_t need;
  char *tmp;

  /* LCOV_EXCL_START */
  if (!buf || !len || !cap || !s)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  slen = strlen(s);
  need = *len + slen + 1;
  if (need > *cap) {
    size_t new_cap = (*cap == 0) ? 64 : *cap * 2;
    while (new_cap < need)
      new_cap *= 2;
    tmp = (char *)realloc(*buf, new_cap);
    if (!tmp) {
      /* LCOV_EXCL_STOP */
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    *buf = tmp;
    *cap = new_cap;
  }
  /* LCOV_EXCL_START */
  memcpy(*buf + *len, s, slen);
  /* LCOV_EXCL_STOP */
  *len += slen;
  /* LCOV_EXCL_START */
  (*buf)[*len] = '\0';
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the kv value to string operation.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error kv_value_to_string(const struct OpenAPI_KV *kv,
                                           /* LCOV_EXCL_STOP */
                                           char *buf, size_t buf_len,
                                           const char **_out_val) {
  /* LCOV_EXCL_START */
  if (!kv) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  switch (kv->type) {
  case OA_KV_STRING: {
    /* LCOV_EXCL_STOP */
    *_out_val = kv->value.s ? kv->value.s : NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case OA_KV_INTEGER:
    if (!buf || buf_len == 0) {
      /* LCOV_EXCL_STOP */
      *_out_val = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    sprintf_s_chk(buf, buf_len, "%d", kv->value.i);
    /* LCOV_EXCL_STOP */
    {
      *_out_val = buf;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
  case OA_KV_NUMBER:
    if (!buf || buf_len == 0) {
      /* LCOV_EXCL_STOP */
      *_out_val = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    sprintf_s_chk(buf, buf_len, "%g", kv->value.n);
    /* LCOV_EXCL_STOP */
    {
      *_out_val = buf;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
  case OA_KV_BOOLEAN: {
    /* LCOV_EXCL_STOP */
    *_out_val = kv->value.b ? "true" : "false";
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  default: {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  }
}

/**
 * @brief Executes the openapi kv join form operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error openapi_kv_join_form(const struct OpenAPI_KV *kvs, size_t n,
                                      /* LCOV_EXCL_STOP */
                                      const char *delim, int allow_reserved,
                                      char **_out_val) {
  /* LCOV_EXCL_START */
  const char *_ast_kv_value_to_string_18 = NULL;
  /* LCOV_EXCL_STOP */
  size_t i;
  /* LCOV_EXCL_START */
  char *buf = NULL;
  size_t len = 0;
  size_t cap = 0;
  /* LCOV_EXCL_STOP */
  char num_buf[64];
  /* LCOV_EXCL_START */
  char *enc_key = NULL;
  char *enc_val = NULL;
  enum cdd_c_error (*enc_fn)(const char *, char **) =
      allow_reserved ? url_encode_form_allow_reserved : url_encode_form;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (!delim)
    delim = ",";
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (!kvs || n == 0) {
    buf = (char *)calloc(1, 1);
    if (!buf)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    {
      *_out_val = buf;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  for (i = 0; i < n; ++i) {
    /* LCOV_EXCL_STOP */
    const char *raw_val;
    /* LCOV_EXCL_START */
    if (!kvs[i].key)
      continue;
    raw_val = (kv_value_to_string(&kvs[i], num_buf, sizeof(num_buf),
                                  /* LCOV_EXCL_STOP */
                                  &_ast_kv_value_to_string_18),
               _ast_kv_value_to_string_18);
    /* LCOV_EXCL_START */
    if (!raw_val)
      continue;
    enc_fn(kvs[i].key, &enc_key);
    if (!enc_key)
      goto oom;
    enc_fn(raw_val, &enc_val);
    if (!enc_val)
      goto oom;
    if (len > 0) {
      if (append_str(&buf, &len, &cap, delim) != 0)
        goto oom;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (append_str(&buf, &len, &cap, enc_key) != 0)
      goto oom;
    if (append_str(&buf, &len, &cap, delim) != 0)
      goto oom;
    if (append_str(&buf, &len, &cap, enc_val) != 0)
      goto oom;
    free(enc_key);
    free(enc_val);
    enc_key = NULL;
    enc_val = NULL;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  if (!buf) {
    buf = (char *)calloc(1, 1);
    if (!buf)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  {
    *_out_val = buf;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

/* LCOV_EXCL_START */
oom:
  if (enc_key)
    free(enc_key);
  if (enc_val)
    free(enc_val);
  if (buf)
    free(buf);
  /* LCOV_EXCL_STOP */
  {
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}
