/**
 * @file url.c
 * @brief Implementation of URL parsing.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "functions/parse/str.h" /* For c_cdd_strdup helpers */
#include "routes/parse/url.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
#undef malloc
#define malloc(sz) C_CDD_MALLOC(sz)
#undef realloc
#define realloc(ptr, sz) C_CDD_REALLOC(ptr, sz)
#undef calloc
#define calloc(n, sz) C_CDD_CALLOC(n, sz)
#endif

/* Standard definitions for C89 compatibility */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define sprintf_s_chk(buf, size, fmt, arg) sprintf_s(buf, size, fmt, arg)
#else
/* Naive fallback for non-MSVC C89 */
/** @brief sprintf_s_chk macro */
#define sprintf_s_chk(buf, size, fmt, arg) CDD_SNPRINTF(buf, size, fmt, arg)
#endif

/**
 * @brief Check if a character is unreserved per RFC 3986 Section 2.3.
 *
 * Unreserved characters: ALPHA, DIGIT, "-", ".", "_", "~".
 */
static cdd_c_error_t is_unreserved(unsigned char c) {
  if (isalnum(c))
    return CDD_C_ERROR_UNKNOWN;
  if (c == '-' || c == '.' || c == '_' || c == '~')
    return CDD_C_ERROR_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Check if a character is reserved per RFC 3986 Section 2.2.
 */
static cdd_c_error_t is_reserved(unsigned char c) {
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
    return CDD_C_ERROR_UNKNOWN;
  default:
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Checks if hex.
 */
static cdd_c_error_t is_hex(unsigned char c) { return isxdigit(c) ? 1 : 0; }

/**
 * @brief Checks if pct encoded.
 */
static cdd_c_error_t is_pct_encoded(const char *p) {
  if (!p)
    return CDD_C_SUCCESS;
  return (p[0] == '%' && is_hex((unsigned char)p[1]) &&
          is_hex((unsigned char)p[2]));
}

/**
 * @brief Convert a nibble to hexagonal character.
 */
static cdd_c_error_t to_hex(char code, char *_out_val) {
  static const char hex[] = "0123456789ABCDEF";
  {
    *_out_val = hex[code & 15];
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Checks if unreserved form.
 */
static cdd_c_error_t is_unreserved_form(unsigned char c) {
  if (isalnum(c))
    return CDD_C_ERROR_UNKNOWN;
  if (c == '-' || c == '.' || c == '_' || c == '*')
    return CDD_C_ERROR_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Percent-encodes a string according to RFC 3986 unreserved character
 * rules.
 */
cdd_c_error_t url_encode(const char *str, char **_out_val) {
  char hex_hi;
  char hex_lo;
  const char *p;
  char *enc = NULL;
  char *e;
  size_t needed_len = 0;

  if (!str) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  /* Pass 1: Calculate required length */
  for (p = str; *p; p++) {
    if (is_unreserved((unsigned char)*p)) {
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
    if (is_unreserved(c)) {
      *e++ = *p;
    } else {
      *e++ = '%';
      to_hex((char)(unsigned char)((unsigned char)c >> 4), &hex_hi);
      *e++ = hex_hi;
      to_hex((char)(unsigned char)((unsigned char)c & 15), &hex_lo);
      *e++ = hex_lo;
    }
  }
  *e = '\0';

  {
    *_out_val = enc;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Percent-encodes a string while preserving RFC 3986 reserved characters
 * and %HH triples.
 */
cdd_c_error_t url_encode_allow_reserved(const char *str, char **_out_val) {
  char hex_hi;
  char hex_lo;
  const char *p;
  char *enc = NULL;
  char *e;
  size_t needed_len = 0;

  if (!str) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  /* Pass 1: Calculate required length */
  for (p = str; *p; p++) {
    if (*p == '%' && is_pct_encoded(p)) {
      needed_len += 3;
      p += 2;
      continue;
    }
    if (is_unreserved((unsigned char)*p) || is_reserved((unsigned char)*p)) {
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
    if (*p == '%' && is_pct_encoded(p)) {
      *e++ = *p++;
      *e++ = *p++;
      *e++ = *p;
      continue;
    }
    if (is_unreserved(c) || is_reserved(c)) {
      *e++ = *p;
    } else {
      *e++ = '%';
      to_hex((char)(unsigned char)((unsigned char)c >> 4), &hex_hi);
      *e++ = hex_hi;
      to_hex((char)(unsigned char)((unsigned char)c & 15), &hex_lo);
      *e++ = hex_lo;
    }
  }
  *e = '\0';
  {
    *_out_val = enc;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Percent-encodes a string for application/x-www-form-urlencoded (spaces
 * as +).
 */
cdd_c_error_t url_encode_form(const char *str, char **_out_val) {
  char hex_hi;
  char hex_lo;
  const char *p;
  char *enc = NULL;
  char *e;
  size_t needed_len = 0;

  if (!str) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (c == ' ') {
      needed_len++;
    } else if (is_unreserved_form(c)) {
      needed_len++;
    } else {
      needed_len += 3;
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
    } else if (is_unreserved_form(c)) {
      *e++ = *p;
    } else {
      *e++ = '%';
      to_hex((char)(unsigned char)((unsigned char)c >> 4), &hex_hi);
      *e++ = hex_hi;
      to_hex((char)(unsigned char)((unsigned char)c & 15), &hex_lo);
      *e++ = hex_lo;
    }
  }
  *e = '\0';
  {
    *_out_val = enc;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Form-encodes a string while preserving non-delimiter reserved
 * characters.
 */
cdd_c_error_t url_encode_form_allow_reserved(const char *str, char **_out_val) {
  char hex_hi;
  char hex_lo;
  const char *p;
  char *enc = NULL;
  char *e;
  size_t needed_len = 0;

  if (!str) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

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
      } else {
        needed_len++;
      }
    } else {
      needed_len += 3;
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
    } else if (is_pct_encoded(p)) {
      *e++ = *p++;
      *e++ = *p++;
      *e++ = *p;
    } else if (is_unreserved_form(c) || is_reserved(c)) {
      if (c == '&' || c == '=' || c == '+') {
        *e++ = '%';
        to_hex((char)(unsigned char)((unsigned char)c >> 4), &hex_hi);
        *e++ = hex_hi;
        to_hex((char)(unsigned char)((unsigned char)c & 15), &hex_lo);
        *e++ = hex_lo;
      } else {
        *e++ = *p;
      }
    } else {
      *e++ = '%';
      to_hex((char)(unsigned char)((unsigned char)c >> 4), &hex_hi);
      *e++ = hex_hi;
      to_hex((char)(unsigned char)((unsigned char)c & 15), &hex_lo);
      *e++ = hex_lo;
    }
  }
  *e = '\0';
  {
    *_out_val = enc;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Initializes a UrlQueryParams container to empty state.
 */
cdd_c_error_t url_query_init(struct UrlQueryParams *qp) {
  if (!qp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  qp->params = NULL;
  qp->count = 0;
  qp->capacity = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Releases all key/value strings and storage in a UrlQueryParams
 * container.
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
 * @brief Appends a raw key/value parameter pair to a UrlQueryParams container.
 */
cdd_c_error_t url_query_add(struct UrlQueryParams *qp, const char *key,
                            const char *value) {
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

  c_cdd_strdup(key, &qp->params[qp->count].key);
  if (!qp->params[qp->count].key)
    return CDD_C_ERROR_MEMORY;

  c_cdd_strdup(value, &qp->params[qp->count].value);
  if (!qp->params[qp->count].value) {
    free(qp->params[qp->count].key);
    return CDD_C_ERROR_MEMORY;
  }
  qp->params[qp->count].value_is_encoded = 0;

  qp->count++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends a pre-encoded value parameter pair to a UrlQueryParams
 * container.
 */
cdd_c_error_t url_query_add_encoded(struct UrlQueryParams *qp, const char *key,
                                    const char *value) {
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

  c_cdd_strdup(key, &qp->params[qp->count].key);
  if (!qp->params[qp->count].key)
    return CDD_C_ERROR_MEMORY;

  c_cdd_strdup(value, &qp->params[qp->count].value);
  if (!qp->params[qp->count].value) {
    free(qp->params[qp->count].key);
    return CDD_C_ERROR_MEMORY;
  }
  qp->params[qp->count].value_is_encoded = 1;

  qp->count++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Serializes a UrlQueryParams container into a ?-prefixed RFC 3986 query
 * string.
 */
cdd_c_error_t url_query_build(const struct UrlQueryParams *qp, char **out_str) {
  size_t i;
  size_t total_len = 0;
  char *buf = NULL;
  char *ptr = NULL;

  if (!qp || !out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (qp->count == 0) {
    c_cdd_strdup("", out_str);
    return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }

  /* 1. Calculate Total Length */
  /* Format: ?key=encoded_val&key2=encoded_val2 */
  /* Overhead: '?' (1) + ('=' + '&') * count */
  /* Actually separators are N-1 '&', 1 '?', N '='. So roughly N + N + 1. */

  total_len = 1; /* '?' */

  for (i = 0; i < qp->count; ++i) {
    char *e_key = NULL;
    char *e_val = NULL;
    const char *raw_val = qp->params[i].value;

    url_encode(qp->params[i].key, &e_key);
    if (qp->params[i].value_is_encoded) {
      c_cdd_strdup(raw_val, &e_val);
    } else {
      url_encode(raw_val, &e_val);
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

  /* 2. Allocate */
  buf = (char *)malloc(total_len + 1);
  if (!buf) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  /* 3. Build */
  ptr = buf;
  *ptr++ = '?';

  for (i = 0; i < qp->count; ++i) {
    char *e_key = NULL;
    size_t kl, vl;
    char *e_val = NULL;
    const char *raw_val = qp->params[i].value;

    url_encode(qp->params[i].key, &e_key);
    if (qp->params[i].value_is_encoded) {
      c_cdd_strdup(raw_val, &e_val);
    } else {
      url_encode(raw_val, &e_val);
    }

    if (!e_key || !e_val) {
      if (e_key)
        free(e_key);
      if (e_val)
        free(e_val);
      free(buf);
      return CDD_C_ERROR_MEMORY;
    }

    /* Copy Key */
    kl = strlen(e_key);
    memcpy(ptr, e_key, kl);
    ptr += kl;

    /* Copy Eq */
    *ptr++ = '=';

    /* Copy Value */
    {
      vl = strlen(e_val);
      memcpy(ptr, e_val, vl);
      ptr += vl;
    }

    /* Copy Sep */
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
 * @brief Serializes a UrlQueryParams container into an
 * application/x-www-form-urlencoded body.
 */
cdd_c_error_t url_query_build_form(const struct UrlQueryParams *qp,
                                   char **out_str) {
  size_t i;
  size_t total_len = 0;
  char *buf;
  char *ptr;

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
    url_encode_form(qp->params[i].key, &e_key);
    if (!e_key) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    if (qp->params[i].value_is_encoded) {
      c_cdd_strdup(qp->params[i].value, &e_val);
    } else {
      url_encode_form(qp->params[i].value, &e_val);
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
    url_encode_form(qp->params[i].key, &e_key);
    if (!e_key) {
      free(buf);
      return CDD_C_ERROR_MEMORY;
    }
    if (qp->params[i].value_is_encoded) {
      c_cdd_strdup(qp->params[i].value, &e_val);
    } else {
      url_encode_form(qp->params[i].value, &e_val);
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
 * @brief Appends a null-terminated string to a dynamically resized character
 * buffer.
 */
static cdd_c_error_t append_str(char **buf, size_t *len, size_t *cap,
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
 * @brief Formats a typed OpenAPI_KV value into a string representation.
 */
static cdd_c_error_t kv_value_to_string(const struct OpenAPI_KV *kv, char *buf,
                                        size_t buf_len, const char **_out_val) {
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
    {
      *_out_val = buf;
      return CDD_C_SUCCESS;
    }
  case OA_KV_NUMBER:
    if (!buf || buf_len == 0) {
      *_out_val = NULL;
      return CDD_C_SUCCESS;
    }
    sprintf_s_chk(buf, buf_len, "%g", kv->value.n);
    {
      *_out_val = buf;
      return CDD_C_SUCCESS;
    }
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
 * @brief Joins an array of OpenAPI_KV pairs using a delimiter and form
 * encoding.
 */
cdd_c_error_t openapi_kv_join_form(const struct OpenAPI_KV *kvs, size_t n,
                                   const char *delim, int allow_reserved,
                                   char **_out_val) {
  size_t i;
  char *buf = NULL;
  size_t len = 0;
  size_t cap = 0;
  char num_buf[64];
  char *enc_key = NULL;
  char *enc_val = NULL;
  cdd_c_error_t (*enc_fn)(const char *, char **) =
      allow_reserved ? url_encode_form_allow_reserved : url_encode_form;

  if (!delim)
    delim = ",";

  if (!kvs || n == 0) {
    buf = (char *)calloc(1, 1);
    if (!buf)
      return CDD_C_ERROR_MEMORY;
    {
      *_out_val = buf;
      return CDD_C_SUCCESS;
    }
  }

  for (i = 0; i < n; ++i) {
    const char *raw_val = NULL;
    if (!kvs[i].key)
      continue;
    kv_value_to_string(&kvs[i], num_buf, sizeof(num_buf), &raw_val);
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
    }
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
  }

  if (!buf) {
    buf = (char *)calloc(1, 1);
    if (!buf)
      return CDD_C_ERROR_MEMORY;
  }
  {
    *_out_val = buf;
    return CDD_C_SUCCESS;
  }

oom:
  if (enc_key)
    free(enc_key);
  if (enc_val)
    free(enc_val);
  if (buf)
    free(buf);
  *_out_val = NULL;
  return CDD_C_ERROR_MEMORY;
}

#ifdef CDD_BUILD_TESTS

C_CDD_EXPORT cdd_c_error_t is_pct_encoded_test(const char *p);
C_CDD_EXPORT cdd_c_error_t kv_value_to_string_test(const struct OpenAPI_KV *kv,
                                                   char *buf, size_t buf_len,
                                                   const char **_out_val);
C_CDD_EXPORT cdd_c_error_t append_str_test(char **buf, size_t *len, size_t *cap,
                                           const char *s);

C_CDD_EXPORT cdd_c_error_t is_pct_encoded_test(const char *p) {
  return is_pct_encoded(p);
}
C_CDD_EXPORT cdd_c_error_t kv_value_to_string_test(const struct OpenAPI_KV *kv,
                                                   char *buf, size_t buf_len,
                                                   const char **_out_val) {
  return kv_value_to_string(kv, buf, buf_len, _out_val);
}
C_CDD_EXPORT cdd_c_error_t append_str_test(char **buf, size_t *len, size_t *cap,
                                           const char *s) {
  return append_str(buf, len, cap, s);
}
#endif
