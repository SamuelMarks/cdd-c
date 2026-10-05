/**
 * @file url_query.c
 * @brief Implementation of Query serialization for RFC 3986 URLs.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "url_utils.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "functions/parse/str.h"
#include "routes/parse/url.h"
/* clang-format on */

/* Standard definitions for C89 compatibility */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
/** @brief sprintf_s_chk macro for MSVC */
#define sprintf_s_chk(buf, size, fmt, arg) sprintf_s(buf, size, fmt, arg)
#else
/* Naive fallback for non-MSVC C89 */
/** @brief sprintf_s_chk macro for non-MSVC fallback */
#define sprintf_s_chk(buf, size, fmt, arg)                                     \
  CDD_SNPRINTF(buf, sizeof(buf), fmt, arg)
#endif

/**
 * @brief Executes the url query init operation.
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
cdd_c_error_t url_query_add(struct UrlQueryParams *qp, const char *key,
                            const char *value) {
  char *_ast_strdup_0 = NULL;
  char *_ast_strdup_1 = NULL;
  cdd_c_error_t rc;

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
cdd_c_error_t url_query_add_encoded(struct UrlQueryParams *qp, const char *key,
                                    const char *value) {
  char *_ast_strdup_2 = NULL;
  char *_ast_strdup_3 = NULL;
  cdd_c_error_t rc;

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
cdd_c_error_t url_query_build(const struct UrlQueryParams *qp, char **out_str) {
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
  cdd_c_error_t rc;

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

  buf = (char *)(size_t)malloc(total_len + 1);
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
cdd_c_error_t url_query_build_form(const struct UrlQueryParams *qp,
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
  cdd_c_error_t rc;

  if (!qp || !out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (qp->count == 0) {
    *out_str = (char *)(size_t)calloc(1, 1);
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

  buf = (char *)(size_t)malloc(total_len + 1);
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
    tmp = (char *)(size_t)realloc(*buf, new_cap);
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
cdd_c_error_t openapi_kv_join_form(const struct OpenAPI_KV *kvs, size_t n,
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
  cdd_c_error_t (*enc_fn)(const char *, char **) =
      allow_reserved ? url_encode_form_allow_reserved : url_encode_form;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  cdd_c_error_t enc_rc;
  const char *raw_val;

  if (!delim)
    delim = ",";

  if (!kvs || n == 0) {
    buf = (char *)(size_t)calloc(1, 1);
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
    buf = (char *)(size_t)calloc(1, 1);
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
