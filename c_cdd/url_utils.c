/**
 * @file url_utils.c
 * @brief Implementation of RFC 3986 URL encoding and Query serialization.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "str_utils.h" /* For c_cdd_strdup helpers */
#include "url_utils.h"

/* Standard definitions for C89 compatibility */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define sprintf_s_chk(buf, size, fmt, arg) sprintf_s(buf, size, fmt, arg)
#else
/* Naive fallback for non-MSVC C89 */
#define sprintf_s_chk(buf, size, fmt, arg) sprintf(buf, fmt, arg)
#endif

/**
 * @brief Check if a character is unreserved per RFC 3986 Section 2.3.
 *
 * Unreserved characters: ALPHA, DIGIT, "-", ".", "_", "~".
 */
static int is_unreserved(unsigned char c) {
  if (isalnum(c))
    return 1;
  if (c == '-' || c == '.' || c == '_' || c == '~')
    return 1;
  return 0;
}

/**
 * @brief Check if a character is reserved per RFC 3986 Section 2.2.
 */
static int is_reserved(unsigned char c) {
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
    return 1;
  default:
    return 0;
  }
}

static int is_hex(unsigned char c) { return isxdigit(c) ? 1 : 0; }

static int is_pct_encoded(const char *p) {
  if (!p)
    return 0;
  return (p[0] == '%' && is_hex((unsigned char)p[1]) &&
          is_hex((unsigned char)p[2]));
}

/**
 * @brief Convert a nibble to hexagonal character.
 */
static char to_hex(char code) {
  static const char hex[] = "0123456789ABCDEF";
  return hex[code & 15];
}

static int is_unreserved_form(unsigned char c) {
  if (isalnum(c))
    return 1;
  if (c == '-' || c == '.' || c == '_' || c == '*')
    return 1;
  return 0;
}

char *url_encode(const char *str) {
  const char *p;
  char *enc = NULL;
  char *e;
  size_t needed_len = 0;

  if (!str)
    return NULL;

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
  if (!enc)
    return NULL;

  /* Pass 2: Encode */
  e = enc;
  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (is_unreserved(c)) {
      *e++ = *p;
    } else {
      *e++ = '%';
      *e++ = to_hex(c >> 4);
      *e++ = to_hex(c & 15);
    }
  }
  *e = '\0';

  return enc;
}

char *url_encode_allow_reserved(const char *str) {
  const char *p;
  char *enc = NULL;
  char *e;
  size_t needed_len = 0;

  if (!str)
    return NULL;

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
  if (!enc)
    return NULL;

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
      *e++ = to_hex(c >> 4);
      *e++ = to_hex(c & 15);
    }
  }
  *e = '\0';
  return enc;
}

char *url_encode_form(const char *str) {
  const char *p;
  char *enc = NULL;
  char *e;
  size_t needed_len = 0;

  if (!str)
    return NULL;

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
  if (!enc)
    return NULL;

  e = enc;
  for (p = str; *p; p++) {
    unsigned char c = (unsigned char)*p;
    if (c == ' ') {
      *e++ = '+';
    } else if (is_unreserved_form(c)) {
      *e++ = *p;
    } else {
      *e++ = '%';
      *e++ = to_hex(c >> 4);
      *e++ = to_hex(c & 15);
    }
  }
  *e = '\0';
  return enc;
}

char *url_encode_form_allow_reserved(const char *str) {
  const char *p;
  char *enc = NULL;
  char *e;
  size_t needed_len = 0;

  if (!str)
    return NULL;

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
  if (!enc)
    return NULL;

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
        *e++ = to_hex(c >> 4);
        *e++ = to_hex(c & 15);
      } else {
        *e++ = *p;
      }
    } else {
      *e++ = '%';
      *e++ = to_hex(c >> 4);
      *e++ = to_hex(c & 15);
    }
  }
  *e = '\0';
  return enc;
}

int url_query_init(struct UrlQueryParams *const qp) {
  if (!qp)
    return EINVAL;
  qp->params = NULL;
  qp->count = 0;
  qp->capacity = 0;
  return 0;
}

void url_query_free(struct UrlQueryParams *const qp) {
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

int url_query_add(struct UrlQueryParams *const qp, const char *const key,
                  const char *const value) {
  if (!qp || !key || !value)
    return EINVAL;

  if (qp->count >= qp->capacity) {
    size_t new_cap = (qp->capacity == 0) ? 4 : qp->capacity * 2;
    struct UrlQueryParam *new_arr = (struct UrlQueryParam *)realloc(
        qp->params, new_cap * sizeof(struct UrlQueryParam));
    if (!new_arr)
      return ENOMEM;
    qp->params = new_arr;
    qp->capacity = new_cap;
  }

  qp->params[qp->count].key = c_cdd_strdup(key);
  if (!qp->params[qp->count].key)
    return ENOMEM;

  qp->params[qp->count].value = c_cdd_strdup(value);
  if (!qp->params[qp->count].value) {
    free(qp->params[qp->count].key);
    return ENOMEM;
  }
  qp->params[qp->count].value_is_encoded = 0;

  qp->count++;
  return 0;
}

int url_query_add_encoded(struct UrlQueryParams *const qp,
                          const char *const key, const char *const value) {
  if (!qp || !key || !value)
    return EINVAL;

  if (qp->count >= qp->capacity) {
    size_t new_cap = (qp->capacity == 0) ? 4 : qp->capacity * 2;
    struct UrlQueryParam *new_arr = (struct UrlQueryParam *)realloc(
        qp->params, new_cap * sizeof(struct UrlQueryParam));
    if (!new_arr)
      return ENOMEM;
    qp->params = new_arr;
    qp->capacity = new_cap;
  }

  qp->params[qp->count].key = c_cdd_strdup(key);
  if (!qp->params[qp->count].key)
    return ENOMEM;

  qp->params[qp->count].value = c_cdd_strdup(value);
  if (!qp->params[qp->count].value) {
    free(qp->params[qp->count].key);
    return ENOMEM;
  }
  qp->params[qp->count].value_is_encoded = 1;

  qp->count++;
  return 0;
}

int url_query_build(const struct UrlQueryParams *const qp, char **out_str) {
  size_t i;
  size_t total_len = 0;
  char *buf = NULL;
  char *ptr = NULL;

  if (!qp || !out_str)
    return EINVAL;

  if (qp->count == 0) {
    *out_str = c_cdd_strdup("");
    return *out_str ? 0 : ENOMEM;
  }

  /* 1. Calculate Total Length */
  /* Format: ?key=encoded_val&key2=encoded_val2 */
  /* Overhead: '?' (1) + ('=' + '&') * count */
  /* Actually separators are N-1 '&', 1 '?', N '='. So roughly N + N + 1. */

  total_len = 1; /* '?' */

  for (i = 0; i < qp->count; ++i) {
    char *e_key = url_encode(qp->params[i].key);
    char *e_val = NULL;
    const char *raw_val = qp->params[i].value;

    if (qp->params[i].value_is_encoded) {
      e_val = c_cdd_strdup(raw_val ? raw_val : "");
    } else {
      e_val = url_encode(raw_val);
    }

    if (!e_key || !e_val) {
      if (e_key)
        free(e_key);
      if (e_val)
        free(e_val);
      return ENOMEM;
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
  if (!buf)
    return ENOMEM;

  /* 3. Build */
  ptr = buf;
  *ptr++ = '?';

  for (i = 0; i < qp->count; ++i) {
    char *e_key = url_encode(qp->params[i].key);
    char *e_val = NULL;
    const char *raw_val = qp->params[i].value;

    if (qp->params[i].value_is_encoded) {
      e_val = c_cdd_strdup(raw_val ? raw_val : "");
    } else {
      e_val = url_encode(raw_val);
    }

    /* e_key/e_val guaranteed non-null here as checks passed in pass 1 */
    /* Copy Key */
    size_t kl = strlen(e_key);
    memcpy(ptr, e_key, kl);
    ptr += kl;

    /* Copy Eq */
    *ptr++ = '=';

    /* Copy Value */
    {
      size_t vl = strlen(e_val);
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
  return 0;
}

int url_query_build_form(const struct UrlQueryParams *const qp,
                         char **out_str) {
  size_t i;
  size_t total_len = 0;
  char *buf;
  char *ptr;

  if (!qp || !out_str)
    return EINVAL;

  if (qp->count == 0) {
    *out_str = (char *)calloc(1, 1);
    if (!*out_str)
      return ENOMEM;
    return 0;
  }

  for (i = 0; i < qp->count; ++i) {
    char *e_key = url_encode_form(qp->params[i].key);
    char *e_val;
    size_t kl, vl;
    if (!e_key)
      return ENOMEM;
    if (qp->params[i].value_is_encoded) {
      e_val = c_cdd_strdup(qp->params[i].value);
    } else {
      e_val = url_encode_form(qp->params[i].value);
    }
    if (!e_val) {
      free(e_key);
      return ENOMEM;
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
  if (!buf)
    return ENOMEM;
  ptr = buf;

  for (i = 0; i < qp->count; ++i) {
    char *e_key = url_encode_form(qp->params[i].key);
    char *e_val;
    size_t kl, vl;
    if (!e_key) {
      free(buf);
      return ENOMEM;
    }
    if (qp->params[i].value_is_encoded) {
      e_val = c_cdd_strdup(qp->params[i].value);
    } else {
      e_val = url_encode_form(qp->params[i].value);
    }
    if (!e_val) {
      free(e_key);
      free(buf);
      return ENOMEM;
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
  return 0;
}
