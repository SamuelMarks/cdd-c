/**
 * @file json_dom.c
 * @brief Zero-dependency C89 JSON DOM parser, mutator, and serializer
 * implementation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/json_dom.h"
#include "c_cdd/memory.h"
/* clang-format on */

/**
 * @brief Adds two 128-bit unsigned integers.
 *
 * @param[in] a First operand.
 * @param[in] b Second operand.
 * @param[out] out Pointer to result.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_u128_add(cdd_uint128_t a, cdd_uint128_t b,
                                       cdd_uint128_t *out) {
  out->low = a.low + b.low;
  out->high = a.high + b.high + (out->low < a.low ? 1u : 0u);
  return CDD_C_SUCCESS;
}

/**
 * @brief Subtracts two 128-bit unsigned integers.
 *
 * @param[in] a First operand.
 * @param[in] b Second operand.
 * @param[out] out Pointer to result.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_u128_sub(cdd_uint128_t a, cdd_uint128_t b,
                                       cdd_uint128_t *out) {
  out->low = a.low - b.low;
  out->high = a.high - b.high - (a.low < b.low ? 1u : 0u);
  return CDD_C_SUCCESS;
}

/**
 * @brief Multiplies two 128-bit unsigned integers.
 *
 * @param[in] a First operand.
 * @param[in] b Second operand.
 * @param[out] out Pointer to result.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_u128_mul(cdd_uint128_t a, cdd_uint128_t b,
                                       cdd_uint128_t *out) {
  uint64_t a_lo = a.low & 0xFFFFFFFFu;
  uint64_t a_hi = a.low >> 32;
  uint64_t b_lo = b.low & 0xFFFFFFFFu;
  uint64_t p00 = a_lo * b_lo;
  uint64_t p10 = a_hi * b_lo;
  uint64_t mid = p10 + (p00 >> 32);
  out->low = (mid << 32) | (p00 & 0xFFFFFFFFu);
  out->high = a.high * b.low + (mid >> 32);
  return CDD_C_SUCCESS;
}

/**
 * @brief Divides a 128-bit unsigned integer by 10, returning quotient and
 * remainder.
 *
 * @param[in] num Dividend.
 * @param[out] quot Quotient.
 * @param[out] rem Remainder (0..9).
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t
cdd_json_u128_divmod10(cdd_uint128_t num, cdd_uint128_t *quot, uint64_t *rem) {
  cdd_uint128_t q;
  uint64_t r = 0u;
  int i;
  q.low = 0u;
  q.high = 0u;
  for (i = 127; i >= 0; --i) {
    uint64_t bit;
    r <<= 1u;
    if (i >= 64) {
      bit = (num.high >> (unsigned int)(i - 64)) & 1u;
    } else {
      bit = (num.low >> (unsigned int)i) & 1u;
    }
    r |= bit;
    if (r >= 10u) {
      r -= 10u;
      if (i >= 64) {
        q.high |= (((uint64_t)1u) << (unsigned int)(i - 64));
      } else {
        q.low |= (((uint64_t)1u) << (unsigned int)i);
      }
    }
  }
  *quot = q;
  *rem = r;
  return CDD_C_SUCCESS;
}

/**
 * @brief Converts a 128-bit signed integer to double.
 *
 * @param[in] val 128-bit signed integer.
 * @param[out] out Output double.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_i128_to_double(cdd_int128_t val, double *out) {
  uint64_t hi = (uint64_t)val.high;
  uint64_t lo = val.low;
  int sign = 0;
  double d;
  double p2_32 = 4294967296.0;
  double p2_64 = p2_32 * p2_32;
  if (val.high < 0) {
    sign = 1;
    lo = ~lo;
    hi = ~hi;
    if (++lo == 0u)
      hi++;
  }
  d = (double)hi * p2_64 + (double)lo;
  *out = sign ? -d : d;
  return CDD_C_SUCCESS;
}

/**
 * @brief Converts a double to a 128-bit signed integer.
 *
 * @param[in] val Input double.
 * @param[out] out Output 128-bit signed integer.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_double_to_i128(double val, cdd_int128_t *out) {
  int sign = 0;
  double p2_32 = 4294967296.0;
  double p2_64 = p2_32 * p2_32;
  uint64_t hi;
  uint64_t lo;
  cdd_uint128_t u;
  if (val < 0.0) {
    sign = 1;
    val = -val;
  }
  hi = (uint64_t)(val / p2_64);
  lo = (uint64_t)(val - ((double)hi * p2_64));
  u.high = hi;
  u.low = lo;
  if (sign) {
    cdd_uint128_t zero;
    cdd_uint128_t neg;
    zero.low = 0u;
    zero.high = 0u;
    g_cdd_last_alloc_err = cdd_json_u128_sub(zero, u, &neg);
    out->low = neg.low;
    out->high = (int64_t)neg.high;
  } else {
    out->low = u.low;
    out->high = (int64_t)u.high;
  }
  return CDD_C_SUCCESS;
}

/** @brief Maximum nesting depth for JSON parsing. */
#define CDD_JSON_MAX_DEPTH 2048

/** @brief Initial capacity for object/array tables. */
#define CDD_JSON_INIT_CAP 16

/** @brief Active custom malloc function pointer (NULL uses default). */
static JSON_Malloc_Function g_json_malloc = NULL;
/** @brief Active custom free function pointer (NULL uses default). */
static JSON_Free_Function g_json_free = NULL;

/**
 * @brief Allocates `sz` bytes via custom allocator or `malloc`.
 *
 * @param[in] sz Number of bytes to allocate.
 * @param[out] out_ptr Output allocated pointer.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_alloc_fn(size_t sz, void **out_ptr) {
  *out_ptr = g_json_malloc ? g_json_malloc(sz) : malloc(sz);
  return *out_ptr ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Allocates `sz` bytes via custom allocator or `C_CDD_MALLOC`.
 *
 * @param[in] sz Number of bytes to allocate.
 * @param[out] out_ptr Output allocated pointer.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_ctx_alloc_fn(size_t sz, void **out_ptr) {
  *out_ptr = g_json_malloc ? g_json_malloc(sz) : C_CDD_MALLOC(sz);
  return *out_ptr ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/** @brief Allocates `sz` bytes via `cdd_json_alloc_fn`. */
#define cdd_json_alloc(sz)                                                     \
  (g_json_tmp_ptr = NULL,                                                      \
   g_cdd_last_alloc_err = cdd_json_alloc_fn((sz), &g_json_tmp_ptr),            \
   g_json_tmp_ptr)

/** @brief Allocates `sz` bytes via `cdd_json_ctx_alloc_fn`. */
#define cdd_json_ctx_alloc(sz)                                                 \
  (g_json_tmp_ptr = NULL,                                                      \
   g_cdd_last_alloc_err = cdd_json_ctx_alloc_fn((sz), &g_json_tmp_ptr),        \
   g_json_tmp_ptr)

/** @brief Temporary pointer holder for allocation macros. */
static void *g_json_tmp_ptr = NULL;

/**
 * @brief Frees `p` via custom deallocator or `free`.
 *
 * @param[in] p Pointer to free.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_dealloc(void *p) {
  if (g_json_free)
    g_json_free(p);
  else
    free(p);
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees `p` via custom deallocator or `C_CDD_FREE`.
 *
 * @param[in] p Pointer to free.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_ctx_dealloc(void *p) {
  if (g_json_free)
    g_json_free(p);
  else
    C_CDD_FREE(p);
  return CDD_C_SUCCESS;
}

/**
 * @brief Growable string buffer for JSON serialization.
 */
struct cdd_json_ser_buf {
  /** @brief Heap-allocated character buffer. */
  char *data;
  /** @brief Current string length. */
  size_t len;
  /** @brief Allocated buffer capacity. */
  size_t cap;
  /** @brief Optional direct FILE* output stream. */
  FILE *fp;
  /** @brief Sticky error status (`CDD_C_SUCCESS` if no error occurred). */
  cdd_c_error_t err;
};

/**
 * @brief Allocates a duplicate of a byte buffer of length n using
 * g_json_malloc.
 *
 * @param[in] str Source string bytes.
 * @param[in] n Number of bytes to copy.
 * @param[out] out Output heap string (NUL-terminated at index n).
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_strndup(const char *str, size_t n, char **out) {
  char *copy;
  copy = (char *)cdd_json_alloc(n + 1);
  if (!copy)
    return CDD_C_ERROR_MEMORY;
  if (n > 0)
    memcpy(copy, str, n);
  copy[n] = "\0"[0];
  *out = copy;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_set_allocation_functions(JSON_Malloc_Function malloc_fun,
                                                JSON_Free_Function free_fun) {
  g_json_malloc = (malloc_fun && malloc_fun != malloc) ? malloc_fun : NULL;
  g_json_free = (free_fun && free_fun != free) ? free_fun : NULL;
  return CDD_C_SUCCESS;
}

JSON_Status json_set_allocation_functions(JSON_Malloc_Function malloc_fun,
                                          JSON_Free_Function free_fun) {
  g_cdd_last_alloc_err =
      cdd_json_set_allocation_functions(malloc_fun, free_fun);
  return JSONSuccess;
}

cdd_c_error_t cdd_json_value_init_object(JSON_Value **out) {
  JSON_Value *val;
  JSON_Object *obj;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  val = (JSON_Value *)cdd_json_alloc(sizeof(JSON_Value));
  if (!val)
    return CDD_C_ERROR_MEMORY;
  obj = (JSON_Object *)cdd_json_alloc(sizeof(JSON_Object));
  if (!obj) {
    g_cdd_last_alloc_err = cdd_json_dealloc(val);
    return CDD_C_ERROR_MEMORY;
  }
  memset(val, 0, sizeof(JSON_Value));
  memset(obj, 0, sizeof(JSON_Object));
  val->type = JSONObject;
  val->object_val = obj;
  obj->wrapping_value = val;
  *out = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_init_array(JSON_Value **out) {
  JSON_Value *val;
  JSON_Array *arr;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  val = (JSON_Value *)cdd_json_alloc(sizeof(JSON_Value));
  if (!val)
    return CDD_C_ERROR_MEMORY;
  arr = (JSON_Array *)cdd_json_alloc(sizeof(JSON_Array));
  if (!arr) {
    g_cdd_last_alloc_err = cdd_json_dealloc(val);
    return CDD_C_ERROR_MEMORY;
  }
  memset(val, 0, sizeof(JSON_Value));
  memset(arr, 0, sizeof(JSON_Array));
  val->type = JSONArray;
  val->array_val = arr;
  arr->wrapping_value = val;
  *out = val;
  return CDD_C_SUCCESS;
}

/**
 * @brief Initializes a JSONString value taking ownership of an allocated
 * string with explicit byte length.
 *
 * @param[in] owned_str Heap-allocated string.
 * @param[in] str_len Explicit byte length of owned_str.
 * @param[out] out Output pointer receiving the new JSON_Value.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_value_init_string_no_copy_len(char *owned_str,
                                                            size_t str_len,
                                                            JSON_Value **out) {
  JSON_Value *val;
  val = (JSON_Value *)cdd_json_alloc(sizeof(JSON_Value));
  if (!val) {
    g_cdd_last_alloc_err = cdd_json_dealloc(owned_str);
    return CDD_C_ERROR_MEMORY;
  }
  memset(val, 0, sizeof(JSON_Value));
  val->type = JSONString;
  val->string_val = owned_str;
  val->string_len = str_len;
  *out = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_init_string_with_len(const char *str, size_t len,
                                                  JSON_Value **out) {
  char *copy = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_strndup(str, len, &copy);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_init_string_no_copy_len(copy, len, out);
}

cdd_c_error_t cdd_json_value_init_string(const char *str, JSON_Value **out) {
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return cdd_json_value_init_string_with_len(str, strlen(str), out);
}

cdd_c_error_t cdd_json_value_get_string_len(const JSON_Value *val,
                                            size_t *out_len) {
  if (!val || !out_len || val->type != JSONString)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_len = val->string_len;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_init_number(double number, JSON_Value **out) {
  JSON_Value *val;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (number != number || (number * 0.0 != 0.0))
    return CDD_C_ERROR_INVALID_ARGUMENT;
  val = (JSON_Value *)cdd_json_alloc(sizeof(JSON_Value));
  if (!val)
    return CDD_C_ERROR_MEMORY;
  memset(val, 0, sizeof(JSON_Value));
  val->type = JSONNumber;
  val->number_val = number;
  val->num_kind = 0;
  *out = val;
  return CDD_C_SUCCESS;
}

/**
 * @brief Formats an unsigned 64-bit integer into a decimal string buffer.
 *
 * @param[in] uval Unsigned 64-bit integer.
 * @param[in] is_neg Non-zero to prefix with `-`.
 * @param[out] buf Output buffer (at least 32 bytes).
 * @param[out] out_len Output length of the string.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_format_u64(uint64_t uval, int is_neg, char *buf,
                                         size_t *out_len) {
  char rev[32];
  size_t rlen = 0;
  size_t pos = 0;
  if (uval == 0) {
    rev[rlen++] = '0';
  } else {
    while (uval > 0) {
      rev[rlen++] = (char)('0' + (int)(uval % 10u));
      uval /= 10u;
    }
  }
  if (is_neg)
    buf[pos++] = '-';
  while (rlen > 0) {
    buf[pos++] = rev[--rlen];
  }
  buf[pos] = '\0';
  *out_len = pos;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_init_int64(int64_t val, JSON_Value **out) {
  JSON_Value *jv = NULL;
  char nbuf[32];
  size_t nlen = 0;
  uint64_t mag;
  int is_neg = (val < 0);
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (is_neg) {
    mag = (uint64_t)(-(val + 1)) + 1u;
  } else {
    mag = (uint64_t)val;
  }
  g_cdd_last_alloc_err = cdd_json_format_u64(mag, is_neg, nbuf, &nlen);
  rc = cdd_json_value_init_number((double)val, &jv);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_strndup(nbuf, nlen, &jv->raw_num_str);
  if (rc != CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = json_value_free(jv);
    return rc;
  }
  jv->int64_val = val;
  jv->uint64_val = is_neg ? 0u : (uint64_t)val;
  jv->int128_val.low = (uint64_t)val;
  jv->int128_val.high = is_neg ? -1 : 0;
  if (!is_neg) {
    jv->uint128_val.low = (uint64_t)val;
    jv->uint128_val.high = 0u;
  }
  jv->num_kind = 1;
  *out = jv;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_init_uint64(uint64_t val, JSON_Value **out) {
  JSON_Value *jv = NULL;
  char nbuf[32];
  size_t nlen = 0;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  g_cdd_last_alloc_err = cdd_json_format_u64(val, 0, nbuf, &nlen);
  rc = cdd_json_value_init_number((double)val, &jv);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_strndup(nbuf, nlen, &jv->raw_num_str);
  if (rc != CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = json_value_free(jv);
    return rc;
  }
  jv->uint64_val = val;
  jv->int64_val = (int64_t)val;
  jv->uint128_val.low = val;
  jv->uint128_val.high = 0u;
  jv->int128_val.low = val;
  jv->int128_val.high = 0;
  jv->num_kind = 2;
  *out = jv;
  return CDD_C_SUCCESS;
}

/**
 * @brief Formats an unsigned 128-bit integer into a decimal string buffer.
 *
 * @param[in] uval Unsigned 128-bit integer.
 * @param[in] is_neg Non-zero to prefix with `-`.
 * @param[out] buf Output buffer (at least 48 bytes).
 * @param[out] out_len Output length of the string.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_format_u128(cdd_uint128_t uval, int is_neg,
                                          char *buf, size_t *out_len) {
  char rev[48];
  size_t rlen = 0;
  size_t pos = 0;
  if (uval.high == 0u && uval.low == 0u) {
    rev[rlen++] = '0';
  } else {
    while (uval.high != 0u || uval.low != 0u) {
      cdd_uint128_t q;
      uint64_t r = 0u;
      g_cdd_last_alloc_err = cdd_json_u128_divmod10(uval, &q, &r);
      rev[rlen++] = (char)('0' + (int)r);
      uval = q;
    }
  }
  if (is_neg)
    buf[pos++] = '-';
  while (rlen > 0) {
    buf[pos++] = rev[--rlen];
  }
  buf[pos] = '\0';
  *out_len = pos;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_init_int128(cdd_int128_t val, JSON_Value **out) {
  JSON_Value *jv = NULL;
  char nbuf[48];
  size_t nlen = 0;
  cdd_uint128_t mag;
  double dval = 0.0;
  int is_neg = (val.high < 0);
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (is_neg) {
    cdd_uint128_t zero;
    cdd_uint128_t tmp;
    zero.low = 0u;
    zero.high = 0u;
    tmp.low = val.low;
    tmp.high = (uint64_t)val.high;
    g_cdd_last_alloc_err = cdd_json_u128_sub(zero, tmp, &mag);
  } else {
    mag.low = val.low;
    mag.high = (uint64_t)val.high;
  }
  g_cdd_last_alloc_err = cdd_json_format_u128(mag, is_neg, nbuf, &nlen);
  g_cdd_last_alloc_err = cdd_json_i128_to_double(val, &dval);
  rc = cdd_json_value_init_number(dval, &jv);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_strndup(nbuf, nlen, &jv->raw_num_str);
  if (rc != CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = json_value_free(jv);
    return rc;
  }
  jv->int128_val = val;
  if (!is_neg) {
    jv->uint128_val = mag;
  }
  if ((val.high == 0 && (val.low & (((uint64_t)1u) << 63)) == 0) ||
      (val.high == -1 && (val.low & (((uint64_t)1u) << 63)) != 0)) {
    jv->int64_val = (int64_t)val.low;
    jv->uint64_val = is_neg ? 0u : val.low;
    jv->num_kind = 1;
  } else if (val.high == 0) {
    jv->uint64_val = val.low;
    jv->int64_val = (int64_t)val.low;
    jv->num_kind = 2;
  } else {
    jv->num_kind = 3;
  }
  *out = jv;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_init_uint128(cdd_uint128_t val, JSON_Value **out) {
  JSON_Value *jv = NULL;
  char nbuf[48];
  size_t nlen = 0;
  double dval = 0.0;
  double p2_32 = 4294967296.0;
  double p2_64 = p2_32 * p2_32;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  g_cdd_last_alloc_err = cdd_json_format_u128(val, 0, nbuf, &nlen);
  dval = (double)val.high * p2_64 + (double)val.low;
  rc = cdd_json_value_init_number(dval, &jv);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_strndup(nbuf, nlen, &jv->raw_num_str);
  if (rc != CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = json_value_free(jv);
    return rc;
  }
  jv->uint128_val = val;
  jv->int128_val.low = val.low;
  jv->int128_val.high = (int64_t)val.high;
  if (val.high == 0) {
    jv->uint64_val = val.low;
    jv->int64_val = (int64_t)val.low;
    jv->num_kind = (val.low <= 9223372036854775807u) ? 1 : 2;
  } else {
    jv->num_kind = 4;
  }
  *out = jv;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_get_int64(const JSON_Value *val, int64_t *out) {
  if (!val || !out || val->type != JSONNumber)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (val->num_kind == 1) {
    *out = val->int64_val;
    return CDD_C_SUCCESS;
  }
  if (val->num_kind == 2) {
    if (val->uint64_val > 9223372036854775807u)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    *out = (int64_t)val->uint64_val;
    return CDD_C_SUCCESS;
  }
  if (val->num_kind == 3 || val->num_kind == 4)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = (int64_t)val->number_val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_get_uint64(const JSON_Value *val, uint64_t *out) {
  if (!val || !out || val->type != JSONNumber)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (val->num_kind == 2) {
    *out = val->uint64_val;
    return CDD_C_SUCCESS;
  }
  if (val->num_kind == 1) {
    if (val->int64_val < 0)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    *out = (uint64_t)val->int64_val;
    return CDD_C_SUCCESS;
  }
  if (val->num_kind == 3 || val->num_kind == 4)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (val->number_val < 0.0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = (uint64_t)val->number_val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_get_int128(const JSON_Value *val,
                                        cdd_int128_t *out) {
  if (!val || !out || val->type != JSONNumber)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (val->num_kind == 3) {
    *out = val->int128_val;
    return CDD_C_SUCCESS;
  }
  if (val->num_kind == 1) {
    out->low = (uint64_t)val->int64_val;
    out->high = (val->int64_val < 0) ? -1 : 0;
    return CDD_C_SUCCESS;
  }
  if (val->num_kind == 2) {
    out->low = val->uint64_val;
    out->high = 0;
    return CDD_C_SUCCESS;
  }
  if (val->num_kind == 4) {
    if ((val->uint128_val.high & (((uint64_t)1u) << 63)) != 0)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    out->low = val->uint128_val.low;
    out->high = (int64_t)val->uint128_val.high;
    return CDD_C_SUCCESS;
  }
  return cdd_json_double_to_i128(val->number_val, out);
}

cdd_c_error_t cdd_json_value_get_uint128(const JSON_Value *val,
                                         cdd_uint128_t *out) {
  cdd_int128_t s128;
  if (!val || !out || val->type != JSONNumber)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (val->num_kind == 4) {
    *out = val->uint128_val;
    return CDD_C_SUCCESS;
  }
  if (val->num_kind == 2) {
    out->low = val->uint64_val;
    out->high = 0u;
    return CDD_C_SUCCESS;
  }
  if (val->num_kind == 1) {
    if (val->int64_val < 0)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    out->low = (uint64_t)val->int64_val;
    out->high = 0u;
    return CDD_C_SUCCESS;
  }
  if (val->num_kind == 3) {
    if (val->int128_val.high < 0)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    out->low = val->int128_val.low;
    out->high = (uint64_t)val->int128_val.high;
    return CDD_C_SUCCESS;
  }
  if (val->number_val < 0.0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  g_cdd_last_alloc_err = cdd_json_double_to_i128(val->number_val, &s128);
  out->low = s128.low;
  out->high = (uint64_t)s128.high;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_init_boolean(int boolean, JSON_Value **out) {
  JSON_Value *val;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  val = (JSON_Value *)cdd_json_alloc(sizeof(JSON_Value));
  if (!val)
    return CDD_C_ERROR_MEMORY;
  memset(val, 0, sizeof(JSON_Value));
  val->type = JSONBoolean;
  val->boolean_val = boolean ? 1 : 0;
  *out = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_init_null(JSON_Value **out) {
  JSON_Value *val;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  val = (JSON_Value *)cdd_json_alloc(sizeof(JSON_Value));
  if (!val)
    return CDD_C_ERROR_MEMORY;
  memset(val, 0, sizeof(JSON_Value));
  val->type = JSONNull;
  *out = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_free(JSON_Value *val) {
  size_t i;
  if (!val)
    return CDD_C_SUCCESS;
  if (val->type == JSONString) {
    g_cdd_last_alloc_err = cdd_json_dealloc(val->string_val);
  } else if (val->type == JSONNumber) {
    if (val->raw_num_str)
      g_cdd_last_alloc_err = cdd_json_dealloc(val->raw_num_str);
  } else if (val->type == JSONObject) {
    JSON_Object *obj = val->object_val;
    for (i = 0; i < obj->count; ++i) {
      g_cdd_last_alloc_err = cdd_json_dealloc(obj->names[i]);
      g_cdd_last_alloc_err = cdd_json_value_free(obj->values[i]);
    }
    if (obj->names) {
      g_cdd_last_alloc_err = cdd_json_dealloc((void *)obj->names);
      g_cdd_last_alloc_err = cdd_json_dealloc((void *)obj->name_lens);
      g_cdd_last_alloc_err = cdd_json_dealloc((void *)obj->values);
    }
    if (obj->hash_buckets)
      g_cdd_last_alloc_err = cdd_json_dealloc((void *)obj->hash_buckets);
    g_cdd_last_alloc_err = cdd_json_dealloc(obj);
  } else if (val->type == JSONArray) {
    JSON_Array *arr = val->array_val;
    for (i = 0; i < arr->count; ++i) {
      g_cdd_last_alloc_err = cdd_json_value_free(arr->items[i]);
    }
    if (arr->items)
      g_cdd_last_alloc_err = cdd_json_dealloc((void *)arr->items);
    g_cdd_last_alloc_err = cdd_json_dealloc(arr);
  }
  g_cdd_last_alloc_err = cdd_json_dealloc(val);
  return CDD_C_SUCCESS;
}

cdd_c_error_t json_value_free(JSON_Value *val) {
  return cdd_json_value_free(val);
}

cdd_c_error_t cdd_json_free_serialized_string(char *str) {
  if (str)
    g_cdd_last_alloc_err = cdd_json_dealloc(str);
  return CDD_C_SUCCESS;
}

cdd_c_error_t json_free_serialized_string(char *str) {
  return cdd_json_free_serialized_string(str);
}

cdd_c_error_t cdd_json_value_get_type(const JSON_Value *val,
                                      JSON_Value_Type *out_type) {
  if (!val || !out_type)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_type = val->type;
  return CDD_C_SUCCESS;
}

JSON_Value_Type json_value_get_type(const JSON_Value *val) {
  return val ? val->type : JSONError;
}

cdd_c_error_t cdd_json_value_get_boolean(const JSON_Value *val, int *out_bool) {
  if (!val || !out_bool || val->type != JSONBoolean)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_bool = val->boolean_val ? 1 : 0;
  return CDD_C_SUCCESS;
}

cdd_json_bool_t json_value_get_boolean(const JSON_Value *val) {
  if (!val || val->type != JSONBoolean)
    return CDD_JSON_ERR_BOOL;
  return val->boolean_val ? CDD_JSON_TRUE : CDD_JSON_FALSE;
}

cdd_c_error_t cdd_json_value_get_number(const JSON_Value *val,
                                        double *out_num) {
  if (!val || !out_num || val->type != JSONNumber)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_num = val->number_val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_get_object(const JSON_Value *val,
                                        JSON_Object **out_obj) {
  if (!out_obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_obj = NULL;
  if (!val || val->type != JSONObject)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_obj = val->object_val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_get_array(const JSON_Value *val,
                                       JSON_Array **out_arr) {
  if (!out_arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_arr = NULL;
  if (!val || val->type != JSONArray)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_arr = val->array_val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_get_string(const JSON_Value *val,
                                        const char **out_str) {
  if (!out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_str = NULL;
  if (!val || val->type != JSONString)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_str = val->string_val;
  return CDD_C_SUCCESS;
}

/**
 * @brief Computes a 32-bit FNV-1a hash of a key buffer.
 *
 * @param[in] key Key bytes.
 * @param[in] len Byte length of key.
 * @param[out] out_hash Output hash value.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_calc_hash(const char *key, size_t len,
                                        size_t *out_hash) {
  size_t h = 2166136261u;
  size_t i;
  for (i = 0; i < len; ++i) {
    h ^= (size_t)(unsigned char)key[i];
    h *= 16777619u;
  }
  *out_hash = h;
  return CDD_C_SUCCESS;
}

/**
 * @brief Rebuilds the open-addressing hash index of a JSON_Object.
 *
 * @param[in,out] obj Object whose hash index should be rebuilt.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_object_rebuild_hash(JSON_Object *obj) {
  size_t i;
  size_t target_cap = 16;
  if (obj->count < 8) {
    return CDD_C_SUCCESS;
  }
  while (target_cap < obj->count * 2) {
    target_cap *= 2;
  }
  if (target_cap > obj->hash_cap) {
    size_t *new_buckets = (size_t *)cdd_json_alloc(target_cap * sizeof(size_t));
    if (!new_buckets) {
      if (obj->hash_buckets) {
        g_cdd_last_alloc_err = cdd_json_dealloc((void *)obj->hash_buckets);
        obj->hash_buckets = NULL;
      }
      obj->hash_cap = 0;
      return CDD_C_SUCCESS;
    }
    if (obj->hash_buckets)
      g_cdd_last_alloc_err = cdd_json_dealloc((void *)obj->hash_buckets);
    obj->hash_buckets = new_buckets;
    obj->hash_cap = target_cap;
  }
  memset(obj->hash_buckets, 0, obj->hash_cap * sizeof(size_t));
  for (i = 0; i < obj->count; ++i) {
    size_t klen = obj->name_lens[i];
    size_t h = 0;
    size_t slot;
    g_cdd_last_alloc_err = cdd_json_calc_hash(obj->names[i], klen, &h);
    slot = h & (obj->hash_cap - 1);
    while (obj->hash_buckets[slot] != 0) {
      slot = (slot + 1) & (obj->hash_cap - 1);
    }
    obj->hash_buckets[slot] = i + 1;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Looks up a child value by key in a JSON_Object using an explicit key
 * length.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key string.
 * @param[in] name_len Length of key string.
 * @param[out] out Output child JSON_Value.
 * @return CDD_C_SUCCESS if found, or CDD_C_ERROR_NOT_FOUND.
 */
static cdd_c_error_t cdd_json_object_getn_value(const JSON_Object *obj,
                                                const char *name,
                                                size_t name_len,
                                                JSON_Value **out) {
  size_t i;
  *out = NULL;
  if (obj->count >= 8 && obj->hash_buckets) {
    size_t h = 0;
    size_t slot;
    g_cdd_last_alloc_err = cdd_json_calc_hash(name, name_len, &h);
    slot = h & (obj->hash_cap - 1);
    while (obj->hash_buckets[slot] != 0) {
      size_t idx = obj->hash_buckets[slot] - 1;
      size_t klen = obj->name_lens[idx];
      if (klen == name_len && memcmp(obj->names[idx], name, name_len) == 0) {
        *out = obj->values[idx];
        return CDD_C_SUCCESS;
      }
      slot = (slot + 1) & (obj->hash_cap - 1);
    }
    return CDD_C_ERROR_NOT_FOUND;
  }
  for (i = 0; i < obj->count; ++i) {
    size_t klen = obj->name_lens[i];
    if (klen == name_len && memcmp(obj->names[i], name, name_len) == 0) {
      *out = obj->values[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

cdd_c_error_t cdd_json_object_get_value(const JSON_Object *obj,
                                        const char *name, JSON_Value **out) {
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return cdd_json_object_getn_value(obj, name, strlen(name), out);
}

cdd_c_error_t cdd_json_object_get_string(const JSON_Object *obj,
                                         const char *name, const char **out) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  rc = cdd_json_object_get_value(obj, name, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (v->type != JSONString)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = v->string_val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_object_get_object(const JSON_Object *obj,
                                         const char *name, JSON_Object **out) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  rc = cdd_json_object_get_value(obj, name, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (v->type != JSONObject)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = v->object_val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_object_get_array(const JSON_Object *obj,
                                        const char *name, JSON_Array **out) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  rc = cdd_json_object_get_value(obj, name, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (v->type != JSONArray)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = v->array_val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_object_get_boolean(const JSON_Object *obj,
                                          const char *name, int *out_bool) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out_bool)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_object_get_value(obj, name, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_get_boolean(v, out_bool);
}

cdd_c_error_t cdd_json_object_get_number(const JSON_Object *obj,
                                         const char *name, double *out_num) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out_num)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_object_get_value(obj, name, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_get_number(v, out_num);
}

cdd_c_error_t cdd_json_object_get_int64(const JSON_Object *obj,
                                        const char *name, int64_t *out) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_object_get_value(obj, name, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_get_int64(v, out);
}

cdd_c_error_t cdd_json_object_get_uint64(const JSON_Object *obj,
                                         const char *name, uint64_t *out) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_object_get_value(obj, name, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_get_uint64(v, out);
}

cdd_c_error_t cdd_json_object_get_int128(const JSON_Object *obj,
                                         const char *name, cdd_int128_t *out) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_object_get_value(obj, name, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_get_int128(v, out);
}

cdd_c_error_t cdd_json_object_get_uint128(const JSON_Object *obj,
                                          const char *name,
                                          cdd_uint128_t *out) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_object_get_value(obj, name, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_get_uint128(v, out);
}

cdd_c_error_t cdd_json_object_get_count(const JSON_Object *obj,
                                        size_t *out_count) {
  if (!obj || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_count = obj->count;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_object_get_name(const JSON_Object *obj, size_t idx,
                                       const char **out_name) {
  if (!obj || !out_name || idx >= obj->count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_name = obj->names[idx];
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_object_get_value_at(const JSON_Object *obj, size_t idx,
                                           JSON_Value **out_val) {
  if (!obj || !out_val || idx >= obj->count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_val = obj->values[idx];
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_object_dotget_value(const JSON_Object *obj,
                                           const char *name, JSON_Value **out) {
  const char *dot;
  JSON_Value *next_val = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  dot = strchr(name, 46);
  if (!dot)
    return cdd_json_object_get_value(obj, name, out);
  rc = cdd_json_object_getn_value(obj, name, (size_t)(dot - name), &next_val);
  if (rc != CDD_C_SUCCESS || next_val->type != JSONObject)
    return CDD_C_ERROR_NOT_FOUND;
  return cdd_json_object_dotget_value(next_val->object_val, dot + 1, out);
}

cdd_c_error_t cdd_json_object_dothas_value(const JSON_Object *obj,
                                           const char *name, int *out_has) {
  JSON_Value *v = NULL;
  if (!obj || !name || !out_has)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has =
      (cdd_json_object_dotget_value(obj, name, &v) == CDD_C_SUCCESS) ? 1 : 0;
  return CDD_C_SUCCESS;
}

cdd_json_bool_t json_object_dothas_value(const JSON_Object *obj,
                                         const char *name) {
  int has = 0;
  if (cdd_json_object_dothas_value(obj, name, &has) != CDD_C_SUCCESS)
    return CDD_JSON_FALSE;
  return has ? CDD_JSON_TRUE : CDD_JSON_FALSE;
}

cdd_json_bool_t json_object_get_boolean(const JSON_Object *obj,
                                        const char *name) {
  JSON_Value *v = NULL;
  if (cdd_json_object_get_value(obj, name, &v) != CDD_C_SUCCESS)
    return CDD_JSON_ERR_BOOL;
  return json_value_get_boolean(v);
}

cdd_c_error_t cdd_json_object_has_value(const JSON_Object *obj,
                                        const char *name, int *out_has) {
  JSON_Value *v = NULL;
  if (!obj || !name || !out_has)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has =
      (cdd_json_object_get_value(obj, name, &v) == CDD_C_SUCCESS) ? 1 : 0;
  return CDD_C_SUCCESS;
}

cdd_json_bool_t json_object_has_value(const JSON_Object *obj,
                                      const char *name) {
  JSON_Value *v = NULL;
  return (cdd_json_object_get_value(obj, name, &v) == CDD_C_SUCCESS)
             ? CDD_JSON_TRUE
             : CDD_JSON_FALSE;
}

cdd_c_error_t cdd_json_object_has_value_of_type(const JSON_Object *obj,
                                                const char *name,
                                                JSON_Value_Type type,
                                                int *out_has) {
  JSON_Value *v = NULL;
  if (!obj || !name || !out_has)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (cdd_json_object_get_value(obj, name, &v) != CDD_C_SUCCESS) {
    *out_has = 0;
  } else {
    *out_has = (v->type == type) ? 1 : 0;
  }
  return CDD_C_SUCCESS;
}

cdd_json_bool_t json_object_has_value_of_type(const JSON_Object *obj,
                                              const char *name,
                                              JSON_Value_Type type) {
  JSON_Value *v = NULL;
  if (cdd_json_object_get_value(obj, name, &v) != CDD_C_SUCCESS)
    return CDD_JSON_FALSE;
  return (v->type == type) ? CDD_JSON_TRUE : CDD_JSON_FALSE;
}

cdd_c_error_t cdd_json_array_get_count(const JSON_Array *arr,
                                       size_t *out_count) {
  if (!arr || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_count = arr->count;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_array_get_value(const JSON_Array *arr, size_t idx,
                                       JSON_Value **out_val) {
  if (!arr || !out_val || idx >= arr->count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_val = arr->items[idx];
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_array_get_string(const JSON_Array *arr, size_t idx,
                                        const char **out_str) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_str = NULL;
  rc = cdd_json_array_get_value(arr, idx, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (v->type != JSONString)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_str = v->string_val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_array_get_object(const JSON_Array *arr, size_t idx,
                                        JSON_Object **out_obj) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out_obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_obj = NULL;
  rc = cdd_json_array_get_value(arr, idx, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (v->type != JSONObject)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_obj = v->object_val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_array_get_array(const JSON_Array *arr, size_t idx,
                                       JSON_Array **out_arr) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out_arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_arr = NULL;
  rc = cdd_json_array_get_value(arr, idx, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (v->type != JSONArray)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_arr = v->array_val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_array_get_number(const JSON_Array *arr, size_t idx,
                                        double *out_num) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out_num)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_array_get_value(arr, idx, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_get_number(v, out_num);
}

cdd_c_error_t cdd_json_array_get_boolean(const JSON_Array *arr, size_t idx,
                                         int *out_bool) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out_bool)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_array_get_value(arr, idx, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_get_boolean(v, out_bool);
}

cdd_json_bool_t json_array_get_boolean(const JSON_Array *arr, size_t idx) {
  if (!arr || idx >= arr->count)
    return CDD_JSON_ERR_BOOL;
  return json_value_get_boolean(arr->items[idx]);
}

cdd_c_error_t cdd_json_array_get_int64(const JSON_Array *arr, size_t idx,
                                       int64_t *out) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_array_get_value(arr, idx, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_get_int64(v, out);
}

cdd_c_error_t cdd_json_array_get_uint64(const JSON_Array *arr, size_t idx,
                                        uint64_t *out) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_array_get_value(arr, idx, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_get_uint64(v, out);
}

cdd_c_error_t cdd_json_array_get_int128(const JSON_Array *arr, size_t idx,
                                        cdd_int128_t *out) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_array_get_value(arr, idx, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_get_int128(v, out);
}

cdd_c_error_t cdd_json_array_get_uint128(const JSON_Array *arr, size_t idx,
                                         cdd_uint128_t *out) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_array_get_value(arr, idx, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_get_uint128(v, out);
}

/**
 * @brief Expands a JSON_Object internal arrays to new_cap.
 *
 * @param[in,out] obj Object to resize.
 * @param[in] new_cap Target capacity.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_object_resize(JSON_Object *obj, size_t new_cap) {
  char **new_names;
  size_t *new_lens;
  JSON_Value **new_vals;
  new_names = (char **)cdd_json_alloc(new_cap * sizeof(char *));
  if (!new_names)
    return CDD_C_ERROR_MEMORY;
  new_lens = (size_t *)cdd_json_alloc(new_cap * sizeof(size_t));
  if (!new_lens) {
    g_cdd_last_alloc_err = cdd_json_dealloc((void *)new_names);
    return CDD_C_ERROR_MEMORY;
  }
  new_vals = (JSON_Value **)cdd_json_alloc(new_cap * sizeof(JSON_Value *));
  if (!new_vals) {
    g_cdd_last_alloc_err = cdd_json_dealloc((void *)new_names);
    g_cdd_last_alloc_err = cdd_json_dealloc((void *)new_lens);
    return CDD_C_ERROR_MEMORY;
  }
  if (obj->count > 0) {
    memcpy((void *)new_names, (const void *)obj->names,
           obj->count * sizeof(char *));
    memcpy((void *)new_lens, (const void *)obj->name_lens,
           obj->count * sizeof(size_t));
    memcpy((void *)new_vals, (const void *)obj->values,
           obj->count * sizeof(JSON_Value *));
    g_cdd_last_alloc_err = cdd_json_dealloc((void *)obj->names);
    g_cdd_last_alloc_err = cdd_json_dealloc((void *)obj->name_lens);
    g_cdd_last_alloc_err = cdd_json_dealloc((void *)obj->values);
  }
  obj->names = new_names;
  obj->name_lens = new_lens;
  obj->values = new_vals;
  obj->capacity = new_cap;
  return CDD_C_SUCCESS;
}

/**
 * @brief Sets a key-value pair in a JSON_Object with explicit key length.
 *
 * @param[in,out] obj Target object.
 * @param[in] name Key string.
 * @param[in] name_len Key length.
 * @param[in] val Child value.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_object_setn_value(JSON_Object *obj,
                                                const char *name,
                                                size_t name_len,
                                                JSON_Value *val) {
  size_t i;
  char *key_copy = NULL;
  cdd_c_error_t rc;
  if (val->parent != NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (obj->count >= 8 && obj->hash_buckets) {
    size_t h = 0;
    size_t slot;
    g_cdd_last_alloc_err = cdd_json_calc_hash(name, name_len, &h);
    slot = h & (obj->hash_cap - 1);
    while (obj->hash_buckets[slot] != 0) {
      size_t idx = obj->hash_buckets[slot] - 1;
      size_t klen = obj->name_lens[idx];
      if (klen == name_len && memcmp(obj->names[idx], name, name_len) == 0) {
        g_cdd_last_alloc_err = cdd_json_value_free(obj->values[idx]);
        val->parent = obj->wrapping_value;
        obj->values[idx] = val;
        return CDD_C_SUCCESS;
      }
      slot = (slot + 1) & (obj->hash_cap - 1);
    }
  } else {
    for (i = 0; i < obj->count; ++i) {
      size_t klen = obj->name_lens[i];
      if (klen == name_len && memcmp(obj->names[i], name, name_len) == 0) {
        g_cdd_last_alloc_err = cdd_json_value_free(obj->values[i]);
        val->parent = obj->wrapping_value;
        obj->values[i] = val;
        return CDD_C_SUCCESS;
      }
    }
  }
  if (obj->count >= obj->capacity) {
    size_t ncap =
        (obj->capacity == 0) ? CDD_JSON_INIT_CAP : (obj->capacity * 2);
    rc = cdd_json_object_resize(obj, ncap);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  rc = cdd_json_strndup(name, name_len, &key_copy);
  if (rc != CDD_C_SUCCESS)
    return rc;
  obj->names[obj->count] = key_copy;
  obj->name_lens[obj->count] = name_len;
  val->parent = obj->wrapping_value;
  obj->values[obj->count] = val;
  obj->count++;
  if (obj->count >= 8) {
    if (obj->hash_buckets && obj->hash_cap >= obj->count * 2) {
      size_t h = 0;
      size_t slot;
      g_cdd_last_alloc_err = cdd_json_calc_hash(name, name_len, &h);
      slot = h & (obj->hash_cap - 1);
      while (obj->hash_buckets[slot] != 0) {
        slot = (slot + 1) & (obj->hash_cap - 1);
      }
      obj->hash_buckets[slot] = obj->count;
    } else {
      g_cdd_last_alloc_err = cdd_json_object_rebuild_hash(obj);
    }
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_object_set_value(JSON_Object *obj, const char *name,
                                        JSON_Value *val) {
  if (!obj || !name || !val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return cdd_json_object_setn_value(obj, name, strlen(name), val);
}

JSON_Status json_object_set_value(JSON_Object *obj, const char *name,
                                  JSON_Value *val) {
  return (cdd_json_object_set_value(obj, name, val) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

/**
 * @brief Sets `name` to newly allocated `val` on `obj`, freeing `val` on
 * failure.
 *
 * @param[in,out] obj Target JSON_Object (non-NULL).
 * @param[in] name Property name (non-NULL).
 * @param[in] val Newly initialized JSON_Value.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t
cdd_json_object_set_owned(JSON_Object *obj, const char *name, JSON_Value *val) {
  cdd_c_error_t rc;
  if (!val)
    return CDD_C_ERROR_MEMORY;
  rc = cdd_json_object_setn_value(obj, name, strlen(name), val);
  if (rc != CDD_C_SUCCESS)
    g_cdd_last_alloc_err = cdd_json_value_free(val);
  return rc;
}

cdd_c_error_t cdd_json_object_set_string(JSON_Object *obj, const char *name,
                                         const char *str) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name || !str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_string(str, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_object_set_owned(obj, name, v) : rc;
}

JSON_Status json_object_set_string(JSON_Object *obj, const char *name,
                                   const char *str) {
  return (cdd_json_object_set_string(obj, name, str) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_object_set_number(JSON_Object *obj, const char *name,
                                         double number) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_number(number, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_object_set_owned(obj, name, v) : rc;
}

cdd_c_error_t cdd_json_object_set_int64(JSON_Object *obj, const char *name,
                                        int64_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_int64(val, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_object_set_owned(obj, name, v) : rc;
}

cdd_c_error_t cdd_json_object_set_uint64(JSON_Object *obj, const char *name,
                                         uint64_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_uint64(val, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_object_set_owned(obj, name, v) : rc;
}

cdd_c_error_t cdd_json_object_set_int128(JSON_Object *obj, const char *name,
                                         cdd_int128_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_int128(val, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_object_set_owned(obj, name, v) : rc;
}

cdd_c_error_t cdd_json_object_set_uint128(JSON_Object *obj, const char *name,
                                          cdd_uint128_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_uint128(val, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_object_set_owned(obj, name, v) : rc;
}

JSON_Status json_object_set_number(JSON_Object *obj, const char *name,
                                   double number) {
  return (cdd_json_object_set_number(obj, name, number) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_object_set_boolean(JSON_Object *obj, const char *name,
                                          int boolean) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_boolean(boolean, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_object_set_owned(obj, name, v) : rc;
}

JSON_Status json_object_set_boolean(JSON_Object *obj, const char *name,
                                    int boolean) {
  return (cdd_json_object_set_boolean(obj, name, boolean) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_object_set_null(JSON_Object *obj, const char *name) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_null(&v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_object_set_owned(obj, name, v) : rc;
}

JSON_Status json_object_set_null(JSON_Object *obj, const char *name) {
  return (cdd_json_object_set_null(obj, name) == CDD_C_SUCCESS) ? JSONSuccess
                                                                : JSONFailure;
}

cdd_c_error_t cdd_json_object_dotset_value(JSON_Object *obj, const char *name,
                                           JSON_Value *val) {
  const char *dot;
  JSON_Value *sub_val = NULL;
  JSON_Object *sub_obj = NULL;
  size_t prefix_len;
  cdd_c_error_t rc;
  if (!obj || !name || !val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  dot = strchr(name, 46);
  if (!dot)
    return cdd_json_object_set_value(obj, name, val);
  prefix_len = (size_t)(dot - name);
  if (cdd_json_object_getn_value(obj, name, prefix_len, &sub_val) ==
          CDD_C_SUCCESS &&
      sub_val->type == JSONObject) {
    sub_obj = sub_val->object_val;
  } else {
    JSON_Value *new_obj_val = NULL;
    rc = cdd_json_value_init_object(&new_obj_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_object_setn_value(obj, name, prefix_len, new_obj_val);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(new_obj_val);
      return rc;
    }
    sub_obj = new_obj_val->object_val;
  }
  return cdd_json_object_dotset_value(sub_obj, dot + 1, val);
}

JSON_Status json_object_dotset_value(JSON_Object *obj, const char *name,
                                     JSON_Value *val) {
  return (cdd_json_object_dotset_value(obj, name, val) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

/**
 * @brief Dot-sets `name` to newly allocated `val` on `obj`, freeing `val` on
 * failure.
 *
 * @param[in,out] obj Target JSON_Object (non-NULL).
 * @param[in] name Dot-separated property path (non-NULL).
 * @param[in] val Newly initialized JSON_Value.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_object_dotset_owned(JSON_Object *obj,
                                                  const char *name,
                                                  JSON_Value *val) {
  cdd_c_error_t rc = cdd_json_object_dotset_value(obj, name, val);
  if (rc != CDD_C_SUCCESS)
    g_cdd_last_alloc_err = cdd_json_value_free(val);
  return rc;
}

cdd_c_error_t cdd_json_object_dotset_string(JSON_Object *obj, const char *name,
                                            const char *str) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name || !str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_string(str, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_object_dotset_owned(obj, name, v)
                               : rc;
}

JSON_Status json_object_dotset_string(JSON_Object *obj, const char *name,
                                      const char *str) {
  return (cdd_json_object_dotset_string(obj, name, str) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_object_dotset_number(JSON_Object *obj, const char *name,
                                            double number) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_number(number, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_object_dotset_owned(obj, name, v)
                               : rc;
}

JSON_Status json_object_dotset_number(JSON_Object *obj, const char *name,
                                      double number) {
  return (cdd_json_object_dotset_number(obj, name, number) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_object_dotset_boolean(JSON_Object *obj, const char *name,
                                             int boolean) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_boolean(boolean, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_object_dotset_owned(obj, name, v)
                               : rc;
}

JSON_Status json_object_dotset_boolean(JSON_Object *obj, const char *name,
                                       int boolean) {
  return (cdd_json_object_dotset_boolean(obj, name, boolean) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_object_dotset_null(JSON_Object *obj, const char *name) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_null(&v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_object_dotset_owned(obj, name, v)
                               : rc;
}

JSON_Status json_object_dotset_null(JSON_Object *obj, const char *name) {
  return (cdd_json_object_dotset_null(obj, name) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_object_remove(JSON_Object *obj, const char *name) {
  size_t i, j;
  size_t name_len;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  name_len = strlen(name);
  for (i = 0; i < obj->count; ++i) {
    size_t klen = obj->name_lens[i];
    if (klen == name_len && memcmp(obj->names[i], name, name_len) == 0) {
      g_cdd_last_alloc_err = cdd_json_dealloc(obj->names[i]);
      g_cdd_last_alloc_err = cdd_json_value_free(obj->values[i]);
      for (j = i; j + 1 < obj->count; ++j) {
        obj->names[j] = obj->names[j + 1];
        obj->name_lens[j] = obj->name_lens[j + 1];
        obj->values[j] = obj->values[j + 1];
      }
      obj->count--;
      return cdd_json_object_rebuild_hash(obj);
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

JSON_Status json_object_remove(JSON_Object *obj, const char *name) {
  return (cdd_json_object_remove(obj, name) == CDD_C_SUCCESS) ? JSONSuccess
                                                              : JSONFailure;
}

cdd_c_error_t cdd_json_object_dotremove(JSON_Object *obj, const char *name) {
  const char *dot;
  JSON_Value *sub_val = NULL;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  dot = strchr(name, 46);
  if (!dot)
    return cdd_json_object_remove(obj, name);
  if (cdd_json_object_getn_value(obj, name, (size_t)(dot - name), &sub_val) !=
          CDD_C_SUCCESS ||
      sub_val->type != JSONObject)
    return CDD_C_ERROR_NOT_FOUND;
  return cdd_json_object_dotremove(sub_val->object_val, dot + 1);
}

JSON_Status json_object_dotremove(JSON_Object *obj, const char *name) {
  return (cdd_json_object_dotremove(obj, name) == CDD_C_SUCCESS) ? JSONSuccess
                                                                 : JSONFailure;
}

cdd_c_error_t cdd_json_object_clear(JSON_Object *obj) {
  size_t i;
  if (!obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < obj->count; ++i) {
    g_cdd_last_alloc_err = cdd_json_dealloc(obj->names[i]);
    g_cdd_last_alloc_err = cdd_json_value_free(obj->values[i]);
  }
  obj->count = 0;
  if (obj->hash_buckets) {
    memset(obj->hash_buckets, 0, obj->hash_cap * sizeof(size_t));
  }
  return CDD_C_SUCCESS;
}

JSON_Status json_object_clear(JSON_Object *obj) {
  return (cdd_json_object_clear(obj) == CDD_C_SUCCESS) ? JSONSuccess
                                                       : JSONFailure;
}

/**
 * @brief Expands a JSON_Array internal items buffer to new_cap.
 *
 * @param[in,out] arr Array to resize.
 * @param[in] new_cap Target capacity.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_array_resize(JSON_Array *arr, size_t new_cap) {
  JSON_Value **new_items;
  new_items = (JSON_Value **)cdd_json_alloc(new_cap * sizeof(JSON_Value *));
  if (!new_items)
    return CDD_C_ERROR_MEMORY;
  if (arr->count > 0) {
    memcpy((void *)new_items, (const void *)arr->items,
           arr->count * sizeof(JSON_Value *));
    g_cdd_last_alloc_err = cdd_json_dealloc((void *)arr->items);
  }
  arr->items = new_items;
  arr->capacity = new_cap;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_array_append_value(JSON_Array *arr, JSON_Value *val) {
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return cdd_json_array_insert_value(arr, arr->count, val);
}

JSON_Status json_array_append_value(JSON_Array *arr, JSON_Value *val) {
  return (cdd_json_array_append_value(arr, val) == CDD_C_SUCCESS) ? JSONSuccess
                                                                  : JSONFailure;
}

/**
 * @brief Appends a newly allocated `val` to `arr`, freeing `val` if resizing
 * fails.
 *
 * @param[in,out] arr Target JSON_Array (non-NULL).
 * @param[in] val Newly initialized JSON_Value (non-NULL, parent == NULL).
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_array_append_owned(JSON_Array *arr,
                                                 JSON_Value *val) {
  cdd_c_error_t rc = cdd_json_array_insert_value(arr, arr->count, val);
  if (rc != CDD_C_SUCCESS)
    g_cdd_last_alloc_err = cdd_json_value_free(val);
  return rc;
}

cdd_c_error_t cdd_json_array_append_int64(JSON_Array *arr, int64_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_int64(val, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_array_append_owned(arr, v) : rc;
}

cdd_c_error_t cdd_json_array_append_uint64(JSON_Array *arr, uint64_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_uint64(val, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_array_append_owned(arr, v) : rc;
}

cdd_c_error_t cdd_json_array_append_int128(JSON_Array *arr, cdd_int128_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_int128(val, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_array_append_owned(arr, v) : rc;
}

cdd_c_error_t cdd_json_array_append_uint128(JSON_Array *arr,
                                            cdd_uint128_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_uint128(val, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_array_append_owned(arr, v) : rc;
}

cdd_c_error_t cdd_json_array_append_string(JSON_Array *arr, const char *str) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr || !str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_string(str, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_array_append_owned(arr, v) : rc;
}

JSON_Status json_array_append_string(JSON_Array *arr, const char *str) {
  return (cdd_json_array_append_string(arr, str) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_array_append_number(JSON_Array *arr, double number) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_number(number, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_array_append_owned(arr, v) : rc;
}

JSON_Status json_array_append_number(JSON_Array *arr, double number) {
  return (cdd_json_array_append_number(arr, number) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_array_append_boolean(JSON_Array *arr, int boolean) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_boolean(boolean, &v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_array_append_owned(arr, v) : rc;
}

JSON_Status json_array_append_boolean(JSON_Array *arr, int boolean) {
  return (cdd_json_array_append_boolean(arr, boolean) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_array_append_null(JSON_Array *arr) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_null(&v);
  return (rc == CDD_C_SUCCESS) ? cdd_json_array_append_owned(arr, v) : rc;
}

JSON_Status json_array_append_null(JSON_Array *arr) {
  return (cdd_json_array_append_null(arr) == CDD_C_SUCCESS) ? JSONSuccess
                                                            : JSONFailure;
}

cdd_c_error_t cdd_json_array_replace_value(JSON_Array *arr, size_t i,
                                           JSON_Value *val) {
  if (!arr || i >= arr->count || !val || val->parent != NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  g_cdd_last_alloc_err = cdd_json_value_free(arr->items[i]);
  val->parent = arr->wrapping_value;
  arr->items[i] = val;
  return CDD_C_SUCCESS;
}

JSON_Status json_array_replace_value(JSON_Array *arr, size_t i,
                                     JSON_Value *val) {
  return (cdd_json_array_replace_value(arr, i, val) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_array_replace_string(JSON_Array *arr, size_t i,
                                            const char *str) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr || !str || i >= arr->count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_string(str, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_array_replace_value(arr, i, v);
}

JSON_Status json_array_replace_string(JSON_Array *arr, size_t i,
                                      const char *str) {
  return (cdd_json_array_replace_string(arr, i, str) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_array_replace_number(JSON_Array *arr, size_t i,
                                            double number) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr || i >= arr->count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_number(number, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_array_replace_value(arr, i, v);
}

JSON_Status json_array_replace_number(JSON_Array *arr, size_t i,
                                      double number) {
  return (cdd_json_array_replace_number(arr, i, number) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_array_replace_boolean(JSON_Array *arr, size_t i,
                                             int boolean) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr || i >= arr->count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_boolean(boolean, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_array_replace_value(arr, i, v);
}

JSON_Status json_array_replace_boolean(JSON_Array *arr, size_t i, int boolean) {
  return (cdd_json_array_replace_boolean(arr, i, boolean) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_array_replace_null(JSON_Array *arr, size_t i) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr || i >= arr->count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_null(&v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_array_replace_value(arr, i, v);
}

JSON_Status json_array_replace_null(JSON_Array *arr, size_t i) {
  return (cdd_json_array_replace_null(arr, i) == CDD_C_SUCCESS) ? JSONSuccess
                                                                : JSONFailure;
}

cdd_c_error_t cdd_json_array_insert_value(JSON_Array *arr, size_t idx,
                                          JSON_Value *val) {
  size_t j;
  cdd_c_error_t rc;
  if (!arr || idx > arr->count || !val || val->parent != NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (arr->count >= arr->capacity) {
    size_t ncap =
        (arr->capacity == 0) ? CDD_JSON_INIT_CAP : (arr->capacity * 2);
    rc = cdd_json_array_resize(arr, ncap);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  for (j = arr->count; j > idx; --j) {
    arr->items[j] = arr->items[j - 1];
  }
  val->parent = arr->wrapping_value;
  arr->items[idx] = val;
  arr->count++;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_array_remove(JSON_Array *arr, size_t idx) {
  size_t j;
  if (!arr || idx >= arr->count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  g_cdd_last_alloc_err = cdd_json_value_free(arr->items[idx]);
  for (j = idx; j + 1 < arr->count; ++j) {
    arr->items[j] = arr->items[j + 1];
  }
  arr->count--;
  return CDD_C_SUCCESS;
}

JSON_Status json_array_remove(JSON_Array *arr, size_t idx) {
  return (cdd_json_array_remove(arr, idx) == CDD_C_SUCCESS) ? JSONSuccess
                                                            : JSONFailure;
}

cdd_c_error_t cdd_json_array_clear(JSON_Array *arr) {
  size_t i;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < arr->count; ++i) {
    g_cdd_last_alloc_err = cdd_json_value_free(arr->items[i]);
  }
  arr->count = 0;
  return CDD_C_SUCCESS;
}

JSON_Status json_array_clear(JSON_Array *arr) {
  return (cdd_json_array_clear(arr) == CDD_C_SUCCESS) ? JSONSuccess
                                                      : JSONFailure;
}

/**
 * @brief Internal recursive helper for cdd_json_value_equals with depth bound.
 *
 * @param[in] a First value.
 * @param[in] b Second value.
 * @param[in] depth Current recursion depth.
 * @param[out] out_eq Output 1 if equal, 0 otherwise.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_value_equals_depth(const JSON_Value *a,
                                                 const JSON_Value *b,
                                                 size_t depth, int *out_eq) {
  size_t i;
  if (!out_eq)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_eq = 0;
  if (depth > CDD_JSON_MAX_DEPTH)
    return CDD_C_ERROR_PARSE;
  if (a == b) {
    *out_eq = (a != NULL) ? 1 : 0;
    return CDD_C_SUCCESS;
  }
  if (!b || a->type != b->type)
    return CDD_C_SUCCESS;
  switch (a->type) {
  case JSONNull:
    *out_eq = 1;
    return CDD_C_SUCCESS;
  case JSONBoolean:
    *out_eq = ((a->boolean_val ? 1 : 0) == (b->boolean_val ? 1 : 0)) ? 1 : 0;
    return CDD_C_SUCCESS;
  case JSONNumber:
    if (a->raw_num_str && b->raw_num_str &&
        strcmp(a->raw_num_str, b->raw_num_str) == 0) {
      *out_eq = 1;
    } else {
      *out_eq = (a->number_val == b->number_val) ? 1 : 0;
    }
    return CDD_C_SUCCESS;
  case JSONString:
    if (a->string_len == b->string_len &&
        memcmp(a->string_val, b->string_val, a->string_len) == 0) {
      *out_eq = 1;
    }
    return CDD_C_SUCCESS;
  case JSONArray:
    if (a->array_val->count != b->array_val->count)
      return CDD_C_SUCCESS;
    for (i = 0; i < a->array_val->count; ++i) {
      int sub_eq = 0;
      cdd_c_error_t rc = cdd_json_value_equals_depth(
          a->array_val->items[i], b->array_val->items[i], depth + 1, &sub_eq);
      if (rc != CDD_C_SUCCESS || !sub_eq)
        return rc;
    }
    *out_eq = 1;
    return CDD_C_SUCCESS;
  case JSONObject:
    if (a->object_val->count != b->object_val->count)
      return CDD_C_SUCCESS;
    for (i = 0; i < a->object_val->count; ++i) {
      JSON_Value *b_child = NULL;
      int sub_eq = 0;
      size_t klen = a->object_val->name_lens[i];
      if (cdd_json_object_getn_value(b->object_val, a->object_val->names[i],
                                     klen, &b_child) != CDD_C_SUCCESS)
        return CDD_C_SUCCESS;
      if (cdd_json_value_equals_depth(a->object_val->values[i], b_child,
                                      depth + 1, &sub_eq) != CDD_C_SUCCESS ||
          !sub_eq)
        return CDD_C_SUCCESS;
    }
    *out_eq = 1;
    return CDD_C_SUCCESS;
  default:
    return CDD_C_SUCCESS;
  }
}

cdd_c_error_t cdd_json_value_equals(const JSON_Value *a, const JSON_Value *b,
                                    int *out_eq) {
  return cdd_json_value_equals_depth(a, b, 0, out_eq);
}

cdd_json_bool_t json_value_equals(const JSON_Value *a, const JSON_Value *b) {
  int eq = 0;
  if (cdd_json_value_equals(a, b, &eq) != CDD_C_SUCCESS)
    return CDD_JSON_FALSE;
  return eq ? CDD_JSON_TRUE : CDD_JSON_FALSE;
}

cdd_c_error_t cdd_json_validate(const JSON_Value *schema,
                                const JSON_Value *value) {
  size_t i;
  if (!schema || !value)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (schema->type == JSONNull)
    return CDD_C_SUCCESS;
  if (schema->type != value->type)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (schema->type == JSONArray) {
    if (schema->array_val->count == 0)
      return CDD_C_SUCCESS;
    for (i = 0; i < value->array_val->count; ++i) {
      cdd_c_error_t rc = cdd_json_validate(schema->array_val->items[0],
                                           value->array_val->items[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return CDD_C_SUCCESS;
  }
  if (schema->type == JSONObject) {
    for (i = 0; i < schema->object_val->count; ++i) {
      JSON_Value *v_child = NULL;
      cdd_c_error_t rc;
      size_t klen = schema->object_val->name_lens[i];
      rc = cdd_json_object_getn_value(
          value->object_val, schema->object_val->names[i], klen, &v_child);
      if (rc != CDD_C_SUCCESS)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      rc = cdd_json_validate(schema->object_val->values[i], v_child);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

JSON_Status json_validate(const JSON_Value *schema, const JSON_Value *value) {
  return (cdd_json_validate(schema, value) == CDD_C_SUCCESS) ? JSONSuccess
                                                             : JSONFailure;
}

/**
 * @brief Internal recursive helper for cdd_json_value_deep_copy with depth
 * bound.
 *
 * @param[in] val Source JSON_Value.
 * @param[in] depth Current recursion depth.
 * @param[out] out Output deep-copied JSON_Value.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_value_deep_copy_depth(const JSON_Value *val,
                                                    size_t depth,
                                                    JSON_Value **out) {
  size_t i;
  JSON_Value *copy = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (depth > CDD_JSON_MAX_DEPTH)
    return CDD_C_ERROR_PARSE;
  switch (val->type) {
  case JSONNull:
    return cdd_json_value_init_null(out);
  case JSONBoolean:
    return cdd_json_value_init_boolean(val->boolean_val, out);
  case JSONNumber:
    rc = cdd_json_value_init_number(val->number_val, &copy);
    if (rc != CDD_C_SUCCESS)
      return rc;
    copy->int64_val = val->int64_val;
    copy->uint64_val = val->uint64_val;
    copy->int128_val = val->int128_val;
    copy->uint128_val = val->uint128_val;
    copy->num_kind = val->num_kind;
    if (val->raw_num_str) {
      rc = cdd_json_strndup(val->raw_num_str, strlen(val->raw_num_str),
                            &copy->raw_num_str);
      if (rc != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_value_free(copy);
        return rc;
      }
    }
    *out = copy;
    return CDD_C_SUCCESS;
  case JSONString:
    return cdd_json_value_init_string_with_len(val->string_val, val->string_len,
                                               out);
  case JSONArray:
    rc = cdd_json_value_init_array(&copy);
    if (rc != CDD_C_SUCCESS)
      return rc;
    for (i = 0; i < val->array_val->count; ++i) {
      JSON_Value *elem_copy = NULL;
      rc = cdd_json_value_deep_copy_depth(val->array_val->items[i], depth + 1,
                                          &elem_copy);
      if (rc != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_value_free(copy);
        return rc;
      }
      rc = cdd_json_array_append_value(copy->array_val, elem_copy);
      if (rc != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_value_free(elem_copy);
        g_cdd_last_alloc_err = cdd_json_value_free(copy);
        return rc;
      }
    }
    *out = copy;
    return CDD_C_SUCCESS;
  case JSONObject:
    rc = cdd_json_value_init_object(&copy);
    if (rc != CDD_C_SUCCESS)
      return rc;
    for (i = 0; i < val->object_val->count; ++i) {
      JSON_Value *child_copy = NULL;
      size_t klen = val->object_val->name_lens[i];
      rc = cdd_json_value_deep_copy_depth(val->object_val->values[i], depth + 1,
                                          &child_copy);
      if (rc != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_value_free(copy);
        return rc;
      }
      rc = cdd_json_object_setn_value(
          copy->object_val, val->object_val->names[i], klen, child_copy);
      if (rc != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_value_free(child_copy);
        g_cdd_last_alloc_err = cdd_json_value_free(copy);
        return rc;
      }
    }
    *out = copy;
    return CDD_C_SUCCESS;
  default:
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
}

cdd_c_error_t cdd_json_value_deep_copy(const JSON_Value *val,
                                       JSON_Value **out) {
  return cdd_json_value_deep_copy_depth(val, 0, out);
}

/* --- RFC 8259 Recursive-Descent Parser --- */

/**
 * @brief Skips ASCII whitespace in a JSON cursor.
 *
 * @param[in,out] p Cursor pointer.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_skip_ws_cursor(const char **p) {
  while (**p == 32 || **p == 9 || **p == 10 || **p == 13)
    (*p)++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses 4 hex digits into an unsigned int.
 *
 * @param[in] s Pointer to 4 hex characters.
 * @param[out] out Parsed 16-bit value.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_PARSE.
 */
static cdd_c_error_t cdd_json_parse_hex4(const char *s, unsigned int *out) {
  unsigned int v = 0;
  int i;
  for (i = 0; i < 4; ++i) {
    char c = s[i];
    v <<= 4;
    if (c >= 48 && c <= 57)
      v |= (unsigned int)(c - 48);
    else if (c >= 97 && c <= 102)
      v |= (unsigned int)(c - 97 + 10);
    else if (c >= 65 && c <= 70)
      v |= (unsigned int)(c - 65 + 10);
    else
      return CDD_C_ERROR_PARSE;
  }
  *out = v;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses and unescapes a JSON string literal at *p, returning explicit
 * byte length.
 *
 * @param[in,out] p Cursor positioned at opening quote.
 * @param[out] out Heap-allocated unescaped string.
 * @param[out] out_len Explicit byte length of unescaped string.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_parse_raw_string_len(const char **p, char **out,
                                                   size_t *out_len) {
  const char *scan;
  size_t max_len;
  size_t oi = 0;
  char *res;
  (*p)++;
  scan = *p;
  while (*scan && *scan != 34) {
    if ((unsigned char)*scan < 0x20)
      return CDD_C_ERROR_PARSE;
    if (*scan == 92) {
      scan++;
      if (!*scan)
        return CDD_C_ERROR_PARSE;
    }
    scan++;
  }
  if (*scan != 34)
    return CDD_C_ERROR_PARSE;
  max_len = (size_t)(scan - *p);
  res = (char *)cdd_json_alloc(max_len + 1);
  if (!res)
    return CDD_C_ERROR_MEMORY;
  while (**p != 34) {
    if (**p == 92) {
      char esc;
      (*p)++;
      esc = **p;
      if (esc == 34 || esc == 92 || esc == 47) {
        res[oi++] = esc;
        (*p)++;
      } else if (esc == 98) {
        res[oi++] = 8;
        (*p)++;
      } else if (esc == 102) {
        res[oi++] = 12;
        (*p)++;
      } else if (esc == 110) {
        res[oi++] = 10;
        (*p)++;
      } else if (esc == 114) {
        res[oi++] = 13;
        (*p)++;
      } else if (esc == 116) {
        res[oi++] = 9;
        (*p)++;
      } else if (esc == 117) {
        unsigned int cp = 0;
        (*p)++;
        if (cdd_json_parse_hex4(*p, &cp) != CDD_C_SUCCESS) {
          g_cdd_last_alloc_err = cdd_json_dealloc(res);
          return CDD_C_ERROR_PARSE;
        }
        *p += 4;
        if (cp >= 0xD800 && cp <= 0xDBFF) {
          unsigned int low = 0;
          if ((*p)[0] != 92 || (*p)[1] != 117 ||
              cdd_json_parse_hex4(*p + 2, &low) != CDD_C_SUCCESS ||
              low < 0xDC00 || low > 0xDFFF) {
            g_cdd_last_alloc_err = cdd_json_dealloc(res);
            return CDD_C_ERROR_PARSE;
          }
          cp = 0x10000 + (((cp - 0xD800) << 10) | (low - 0xDC00));
          *p += 6;
        } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
          g_cdd_last_alloc_err = cdd_json_dealloc(res);
          return CDD_C_ERROR_PARSE;
        }
        if (cp <= 0x7F) {
          res[oi++] = (char)cp;
        } else if (cp <= 0x7FF) {
          res[oi++] = (char)(0xC0 | ((cp >> 6) & 0x1F));
          res[oi++] = (char)(0x80 | (cp & 0x3F));
        } else if (cp <= 0xFFFF) {
          res[oi++] = (char)(0xE0 | ((cp >> 12) & 0x0F));
          res[oi++] = (char)(0x80 | ((cp >> 6) & 0x3F));
          res[oi++] = (char)(0x80 | (cp & 0x3F));
        } else {
          res[oi++] = (char)(0xF0 | ((cp >> 18) & 0x07));
          res[oi++] = (char)(0x80 | ((cp >> 12) & 0x3F));
          res[oi++] = (char)(0x80 | ((cp >> 6) & 0x3F));
          res[oi++] = (char)(0x80 | (cp & 0x3F));
        }
      } else {
        g_cdd_last_alloc_err = cdd_json_dealloc(res);
        return CDD_C_ERROR_PARSE;
      }
    } else {
      res[oi++] = *(*p)++;
    }
  }
  (*p)++;
  res[oi] = 0;
  *out = res;
  *out_len = oi;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t
cdd_json_parse_value(const char **p, size_t depth,
                     const struct cdd_json_parse_options *opts,
                     JSON_Value **out);

/**
 * @brief Parses a JSON object starting at "{".
 *
 * @param[in,out] p Cursor.
 * @param[in] depth Current nesting depth.
 * @param[in] opts Optional parse options.
 * @param[out] out Parsed JSON_Value.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t
cdd_json_parse_object(const char **p, size_t depth,
                      const struct cdd_json_parse_options *opts,
                      JSON_Value **out) {
  JSON_Value *val = NULL;
  cdd_c_error_t rc;
  (*p)++;
  g_cdd_last_alloc_err = cdd_json_skip_ws_cursor(p);
  rc = cdd_json_value_init_object(&val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (**p == 125) {
    (*p)++;
    *out = val;
    return CDD_C_SUCCESS;
  }
  for (;;) {
    char *key = NULL;
    size_t key_len = 0;
    JSON_Value *child = NULL;
    g_cdd_last_alloc_err = cdd_json_skip_ws_cursor(p);
    if (**p == 125) {
      if (opts && !opts->allow_trailing_commas) {
        g_cdd_last_alloc_err = cdd_json_value_free(val);
        return CDD_C_ERROR_PARSE;
      }
      (*p)++;
      *out = val;
      return CDD_C_SUCCESS;
    }
    if (**p != 34) {
      g_cdd_last_alloc_err = cdd_json_value_free(val);
      return CDD_C_ERROR_PARSE;
    }
    rc = cdd_json_parse_raw_string_len(p, &key, &key_len);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(val);
      return rc;
    }
    if (opts && opts->reject_duplicate_keys) {
      JSON_Value *existing = NULL;
      if (cdd_json_object_getn_value(val->object_val, key, key_len,
                                     &existing) == CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_dealloc(key);
        g_cdd_last_alloc_err = cdd_json_value_free(val);
        return CDD_C_ERROR_PARSE;
      }
    }
    g_cdd_last_alloc_err = cdd_json_skip_ws_cursor(p);
    if (**p != 58) {
      g_cdd_last_alloc_err = cdd_json_dealloc(key);
      g_cdd_last_alloc_err = cdd_json_value_free(val);
      return CDD_C_ERROR_PARSE;
    }
    (*p)++;
    rc = cdd_json_parse_value(p, depth + 1, opts, &child);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_dealloc(key);
      g_cdd_last_alloc_err = cdd_json_value_free(val);
      return rc;
    }
    rc = cdd_json_object_setn_value(val->object_val, key, key_len, child);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_dealloc(key);
      g_cdd_last_alloc_err = cdd_json_value_free(child);
      g_cdd_last_alloc_err = cdd_json_value_free(val);
      return rc;
    }
    g_cdd_last_alloc_err = cdd_json_dealloc(key);
    g_cdd_last_alloc_err = cdd_json_skip_ws_cursor(p);
    if (**p == 44) {
      (*p)++;
    } else if (**p == 125) {
      (*p)++;
      *out = val;
      return CDD_C_SUCCESS;
    } else {
      g_cdd_last_alloc_err = cdd_json_value_free(val);
      return CDD_C_ERROR_PARSE;
    }
  }
}

/**
 * @brief Parses a JSON array starting at "[".
 *
 * @param[in,out] p Cursor.
 * @param[in] depth Current nesting depth.
 * @param[in] opts Optional parse options.
 * @param[out] out Parsed JSON_Value.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t
cdd_json_parse_array(const char **p, size_t depth,
                     const struct cdd_json_parse_options *opts,
                     JSON_Value **out) {
  JSON_Value *val = NULL;
  cdd_c_error_t rc;
  (*p)++;
  g_cdd_last_alloc_err = cdd_json_skip_ws_cursor(p);
  rc = cdd_json_value_init_array(&val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (**p == 93) {
    (*p)++;
    *out = val;
    return CDD_C_SUCCESS;
  }
  for (;;) {
    JSON_Value *item = NULL;
    g_cdd_last_alloc_err = cdd_json_skip_ws_cursor(p);
    if (**p == 93) {
      if (opts && !opts->allow_trailing_commas) {
        g_cdd_last_alloc_err = cdd_json_value_free(val);
        return CDD_C_ERROR_PARSE;
      }
      (*p)++;
      *out = val;
      return CDD_C_SUCCESS;
    }
    rc = cdd_json_parse_value(p, depth + 1, opts, &item);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(val);
      return rc;
    }
    rc = cdd_json_array_append_value(val->array_val, item);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(item);
      g_cdd_last_alloc_err = cdd_json_value_free(val);
      return rc;
    }
    g_cdd_last_alloc_err = cdd_json_skip_ws_cursor(p);
    if (**p == 44) {
      (*p)++;
    } else if (**p == 93) {
      (*p)++;
      *out = val;
      return CDD_C_SUCCESS;
    } else {
      g_cdd_last_alloc_err = cdd_json_value_free(val);
      return CDD_C_ERROR_PARSE;
    }
  }
}

/**
 * @brief Scales a double value by 10^exp10 in a locale-independent manner.
 *
 * @param[in] base Base mantissa value.
 * @param[in] exp10 Base-10 exponent.
 * @param[out] out_val Scaled double value.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_scale_pow10(double base, int exp10,
                                          double *out_val) {
  static const double k_pow10_pos[9] = {1e1,  1e2,  1e4,   1e8,  1e16,
                                        1e32, 1e64, 1e128, 1e256};
  double v = base;
  int e = exp10;
  int idx = 0;
  if (v == 0.0 || e == 0) {
    *out_val = v;
    return CDD_C_SUCCESS;
  }
  if (e > 308) {
    *out_val = v * 1e308;
    return CDD_C_SUCCESS;
  }
  if (e < -324) {
    *out_val = 0.0;
    return CDD_C_SUCCESS;
  }
  if (e > 0) {
    unsigned int ue = (unsigned int)e;
    while (ue > 0) {
      if (ue & 1u)
        v *= k_pow10_pos[idx];
      ue >>= 1u;
      idx++;
    }
  } else {
    unsigned int ue = (unsigned int)(-e);
    while (ue > 0) {
      if (ue & 1u)
        v /= k_pow10_pos[idx];
      ue >>= 1u;
      idx++;
    }
  }
  *out_val = v;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a JSON number at *p, preserving exact 64-bit and 128-bit
 * integer values and enforcing strict RFC 8259 grammar.
 *
 * @param[in,out] p Cursor.
 * @param[out] out Parsed JSON_Value.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_parse_number(const char **p, JSON_Value **out) {
  const char *start = *p;
  const char *s = *p;
  const char *endp;
  int is_neg = 0;
  int is_int = 1;
  uint64_t uacc = 0;
  int overflow64 = 0;
  cdd_uint128_t u128;
  int overflow128 = 0;
  cdd_uint128_t u128_ten;
  cdd_uint128_t u128_max_div10;
  uint64_t mant = 0;
  int mant_digits = 0;
  int exp_adj = 0;
  int exp_val = 0;
  int exp_neg = 0;
  double num = 0.0;
  JSON_Value *jv = NULL;
  cdd_c_error_t rc;

  u128.low = 0u;
  u128.high = 0u;
  u128_ten.low = 10u;
  u128_ten.high = 0u;
  /* (2^128 - 1) / 10 = 0x19999999999999999999999999999999 */
  u128_max_div10.high = 0x1999999999999999u;
  u128_max_div10.low = 0x9999999999999999u;

  if (*s == '-') {
    is_neg = 1;
    s++;
  }
  if (*s < '0' || *s > '9')
    return CDD_C_ERROR_PARSE;
  if (*s == '0' && s[1] >= '0' && s[1] <= '9')
    return CDD_C_ERROR_PARSE;

  while (*s >= '0' && *s <= '9') {
    uint64_t digit = (uint64_t)(*s - '0');
    if (uacc > (((uint64_t)-1) - digit) / 10u) {
      overflow64 = 1;
    } else {
      uacc = uacc * 10u + digit;
    }
    if (!overflow128) {
      if (u128.high > u128_max_div10.high ||
          (u128.high == u128_max_div10.high &&
           u128.low * 10u + digit < u128.low)) {
        overflow128 = 1;
      } else {
        cdd_uint128_t prod;
        cdd_uint128_t d128;
        d128.low = digit;
        d128.high = 0u;
        g_cdd_last_alloc_err = cdd_json_u128_mul(u128, u128_ten, &prod);
        g_cdd_last_alloc_err = cdd_json_u128_add(prod, d128, &u128);
      }
    }
    if (mant_digits < 19) {
      if (mant != 0 || digit != 0) {
        mant = mant * 10u + digit;
        mant_digits++;
      }
    } else {
      exp_adj++;
    }
    s++;
  }

  if (*s == '.') {
    is_int = 0;
    s++;
    if (*s < '0' || *s > '9')
      return CDD_C_ERROR_PARSE;
    while (*s >= '0' && *s <= '9') {
      uint64_t digit = (uint64_t)(*s - '0');
      if (mant_digits < 19) {
        if (mant != 0 || digit != 0) {
          mant = mant * 10u + digit;
          mant_digits++;
          exp_adj--;
        } else {
          exp_adj--;
        }
      }
      s++;
    }
  }

  if (*s == 'e' || *s == 'E') {
    is_int = 0;
    s++;
    if (*s == '+' || *s == '-') {
      if (*s == '-')
        exp_neg = 1;
      s++;
    }
    if (*s < '0' || *s > '9')
      return CDD_C_ERROR_PARSE;
    while (*s >= '0' && *s <= '9') {
      int digit = (int)(*s - '0');
      if (exp_val < 10000) {
        exp_val = exp_val * 10 + digit;
      }
      s++;
    }
    if (exp_neg)
      exp_val = -exp_val;
  }

  endp = s;
  g_cdd_last_alloc_err =
      cdd_json_scale_pow10((double)mant, exp_val + exp_adj, &num);
  if (is_neg)
    num = -num;

  *p = endp;
  rc = cdd_json_value_init_number(num, &jv);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (is_int && !overflow128) {
    size_t tok_len = (size_t)(endp - start);
    rc = cdd_json_strndup(start, tok_len, &jv->raw_num_str);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(jv);
      return rc;
    }
    if (!overflow64) {
      if (is_neg) {
        uint64_t max_neg = ((uint64_t)1u) << 63;
        if (uacc <= max_neg) {
          jv->int64_val =
              (uacc == max_neg) ? (-9223372036854775807 - 1) : -(int64_t)uacc;
          jv->int128_val.low = (uint64_t)jv->int64_val;
          jv->int128_val.high = -1;
          jv->num_kind = 1;
        } else {
          cdd_uint128_t zero128;
          cdd_uint128_t neg128;
          zero128.low = 0u;
          zero128.high = 0u;
          g_cdd_last_alloc_err = cdd_json_u128_sub(zero128, u128, &neg128);
          jv->int128_val.low = neg128.low;
          jv->int128_val.high = (int64_t)neg128.high;
          jv->num_kind = 3;
        }
      } else {
        jv->uint64_val = uacc;
        jv->uint128_val = u128;
        jv->int128_val.low = u128.low;
        jv->int128_val.high = 0;
        if (uacc <= 9223372036854775807u) {
          jv->int64_val = (int64_t)uacc;
          jv->num_kind = 1;
        } else {
          jv->num_kind = 2;
        }
      }
    } else {
      if (is_neg) {
        uint64_t max_neg_hi = ((uint64_t)1u) << 63;
        if (u128.high < max_neg_hi ||
            (u128.high == max_neg_hi && u128.low == 0u)) {
          cdd_uint128_t zero128;
          cdd_uint128_t neg128;
          zero128.low = 0u;
          zero128.high = 0u;
          g_cdd_last_alloc_err = cdd_json_u128_sub(zero128, u128, &neg128);
          jv->int128_val.low = neg128.low;
          jv->int128_val.high = (int64_t)neg128.high;
          jv->num_kind = 3;
        }
      } else {
        jv->uint128_val = u128;
        if ((u128.high & (((uint64_t)1u) << 63)) == 0) {
          jv->int128_val.low = u128.low;
          jv->int128_val.high = (int64_t)u128.high;
          jv->num_kind = 3;
        } else {
          jv->num_kind = 4;
        }
      }
    }
  }
  *out = jv;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses any JSON value at *p.
 *
 * @param[in,out] p Cursor.
 * @param[in] depth Current nesting depth.
 * @param[in] opts Optional parse options.
 * @param[out] out Parsed JSON_Value.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t
cdd_json_parse_value(const char **p, size_t depth,
                     const struct cdd_json_parse_options *opts,
                     JSON_Value **out) {
  cdd_c_error_t rc;
  size_t limit =
      (opts && opts->max_depth > 0) ? opts->max_depth : CDD_JSON_MAX_DEPTH;
  if (depth > limit)
    return CDD_C_ERROR_PARSE;
  g_cdd_last_alloc_err = cdd_json_skip_ws_cursor(p);
  switch (**p) {
  case 123:
    return cdd_json_parse_object(p, depth, opts, out);
  case 91:
    return cdd_json_parse_array(p, depth, opts, out);
  case 34: {
    char *s = NULL;
    size_t slen = 0;
    rc = cdd_json_parse_raw_string_len(p, &s, &slen);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_json_value_init_string_no_copy_len(s, slen, out);
  }
  case 116:
    if (strncmp(*p, "true", 4) == 0) {
      *p += 4;
      return cdd_json_value_init_boolean(1, out);
    }
    return CDD_C_ERROR_PARSE;
  case 102:
    if (strncmp(*p, "false", 5) == 0) {
      *p += 5;
      return cdd_json_value_init_boolean(0, out);
    }
    return CDD_C_ERROR_PARSE;
  case 110:
    if (strncmp(*p, "null", 4) == 0) {
      *p += 4;
      return cdd_json_value_init_null(out);
    }
    return CDD_C_ERROR_PARSE;
  case 45:
  case 48:
  case 49:
  case 50:
  case 51:
  case 52:
  case 53:
  case 54:
  case 55:
  case 56:
  case 57:
    return cdd_json_parse_number(p, out);
  default:
    return CDD_C_ERROR_PARSE;
  }
}

/**
 * @brief Populates line, column, and byte offset diagnostics for a parse
 * failure cursor position.
 *
 * @param[in] start Beginning of the JSON input string.
 * @param[in] cur Position where parsing failed.
 * @param[out] err_out Output diagnostics struct.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t
cdd_json_fill_parse_error(const char *start, const char *cur,
                          struct cdd_json_parse_error *err_out) {
  const char *p;
  size_t line = 1;
  size_t col = 1;
  if (!err_out)
    return CDD_C_SUCCESS;
  for (p = start; p < cur; ++p) {
    if (*p == 10) {
      line++;
      col = 1;
    } else {
      col++;
    }
  }
  err_out->line = line;
  err_out->column = col;
  err_out->byte_offset = (size_t)(cur - start);
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_parse_string_ex(const char *str, JSON_Value **out,
                                       struct cdd_json_parse_error *err_out) {
  const char *p;
  JSON_Value *val = NULL;
  cdd_c_error_t rc;
  if (err_out) {
    err_out->line = 0;
    err_out->column = 0;
    err_out->byte_offset = 0;
  }
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  p = str;
  if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB &&
      (unsigned char)p[2] == 0xBF) {
    p += 3;
  }
  rc = cdd_json_parse_value(&p, 0, NULL, &val);
  if (rc != CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = cdd_json_fill_parse_error(str, p, err_out);
    return rc;
  }
  g_cdd_last_alloc_err = cdd_json_skip_ws_cursor(&p);
  if (*p != 0) {
    g_cdd_last_alloc_err = cdd_json_value_free(val);
    g_cdd_last_alloc_err = cdd_json_fill_parse_error(str, p, err_out);
    return CDD_C_ERROR_PARSE;
  }

  *out = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_parse_string(const char *str, JSON_Value **out) {
  return cdd_json_parse_string_ex(str, out, NULL);
}

cdd_c_error_t cdd_json_parse_buffer(const char *buf, size_t len,
                                    JSON_Value **out) {
  char *copy = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!buf)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_strndup(buf, len, &copy);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_parse_string(copy, out);
  g_cdd_last_alloc_err = cdd_json_dealloc(copy);
  return rc;
}

/**
 * @brief Strips C/C++ comments from a JSON string.
 *
 * @param[in] str Input JSON text.
 * @param[out] out_cleaned Newly allocated cleaned string.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_preprocess_source(const char *str,
                                                char **out_cleaned) {
  size_t len, i = 0, oi = 0;
  char *cleaned;
  int in_str = 0;
  *out_cleaned = NULL;
  len = strlen(str);
  cleaned = (char *)cdd_json_alloc(len + 1);
  if (!cleaned)
    return CDD_C_ERROR_MEMORY;
  while (i < len) {
    if (in_str) {
      if (str[i] == 92 && i + 1 < len) {
        cleaned[oi++] = str[i++];
        cleaned[oi++] = str[i++];
        continue;
      }
      if (str[i] == 34)
        in_str = 0;
      cleaned[oi++] = str[i++];
    } else {
      if (str[i] == 34) {
        in_str = 1;
        cleaned[oi++] = str[i++];
      } else if (str[i] == 47 && i + 1 < len && str[i + 1] == 47) {
        i += 2;
        while (i < len && str[i] != 10)
          i++;
      } else if (str[i] == 47 && i + 1 < len && str[i + 1] == 42) {
        i += 2;
        while (i + 1 < len && !(str[i] == 42 && str[i + 1] == 47)) {
          if (str[i] == 10)
            cleaned[oi++] = 10;
          i++;
        }
        if (i + 1 >= len) {
          g_cdd_last_alloc_err = cdd_json_dealloc(cleaned);
          return CDD_C_ERROR_PARSE;
        }
        i += 2;
      } else {
        cleaned[oi++] = str[i++];
      }
    }
  }
  cleaned[oi] = 0;
  *out_cleaned = cleaned;
  return CDD_C_SUCCESS;
}

cdd_c_error_t
cdd_json_parse_string_with_options(const char *str,
                                   const struct cdd_json_parse_options *options,
                                   JSON_Value **out) {
  char *cleaned = NULL;
  const char *src;
  const char *p;
  JSON_Value *val = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!options)
    return cdd_json_parse_string(str, out);
  if (options->allow_comments) {
    rc = cdd_json_preprocess_source(str, &cleaned);
    if (rc != CDD_C_SUCCESS)
      return rc;
    src = cleaned;
  } else {
    src = str;
  }
  p = src;
  if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB &&
      (unsigned char)p[2] == 0xBF) {
    p += 3;
  }
  rc = cdd_json_parse_value(&p, 0, options, &val);
  if (rc == CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = cdd_json_skip_ws_cursor(&p);
    if (*p != 0) {
      g_cdd_last_alloc_err = cdd_json_value_free(val);
      val = NULL;
      rc = CDD_C_ERROR_PARSE;
    }
  }
  if (cleaned)
    g_cdd_last_alloc_err = cdd_json_dealloc(cleaned);
  if (rc == CDD_C_SUCCESS)
    *out = val;
  return rc;
}

cdd_c_error_t cdd_json_parse_string_with_comments(const char *str,
                                                  JSON_Value **out) {
  struct cdd_json_parse_options opts;
  opts.allow_comments = 1;
  opts.allow_trailing_commas = 1;
  opts.reject_duplicate_keys = 0;
  opts.max_depth = 0;
  return cdd_json_parse_string_with_options(str, &opts, out);
}

cdd_c_error_t cdd_json_parse_file(const char *filename, JSON_Value **out) {
  FILE *fp = NULL;
  long fsize;
  size_t read_bytes;
  char *buf = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!filename)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#if defined(_MSC_VER)
  if (fopen_s(&fp, filename, "rb") != 0 || !fp)
    return CDD_C_ERROR_IO;
#else
  fp = fopen(filename, "rb");
  if (!fp)
    return CDD_C_ERROR_IO;
#endif
  fseek(fp, 0, SEEK_END);
  fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  buf = (char *)cdd_json_alloc((size_t)fsize + 1);
  if (!buf) {
    fclose(fp);
    return CDD_C_ERROR_MEMORY;
  }
  read_bytes = fread(buf, 1, (size_t)fsize, fp);
  fclose(fp);
  buf[read_bytes] = 0;
  rc = cdd_json_parse_string(buf, out);
  g_cdd_last_alloc_err = cdd_json_dealloc(buf);
  return rc;
}

/* --- Serializer --- */

/**
 * @brief Appends n bytes to a serialization buffer using g_json_malloc.
 *
 * @param[in,out] b Buffer.
 * @param[in] s Source bytes.
 * @param[in] n Byte length.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_ser_append(struct cdd_json_ser_buf *b,
                                         const char *s, size_t n) {
  if (b->err != CDD_C_SUCCESS)
    return b->err;
  if (b->fp) {
    if (fwrite(s, 1, n, b->fp) != n) {
      b->err = CDD_C_ERROR_IO;
      return b->err;
    }
    b->len += n;
    return CDD_C_SUCCESS;
  }
  if (b->len + n + 1 > b->cap) {
    size_t ncap = b->cap * 2 + n + 256;
    char *nbuf = (char *)cdd_json_alloc(ncap);
    if (!nbuf) {
      b->err = CDD_C_ERROR_MEMORY;
      return b->err;
    }
    if (b->len > 0) {
      memcpy(nbuf, b->data, b->len);
      g_cdd_last_alloc_err = cdd_json_dealloc(b->data);
    }
    b->data = nbuf;
    b->cap = ncap;
  }
  memcpy(b->data + b->len, s, n);
  b->len += n;
  b->data[b->len] = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Decodes the next UTF-16 code unit from a UTF-8 byte sequence for
 * RFC 8785 §3.2.3 key ordering.
 *
 * @param[in] s UTF-8 string bytes.
 * @param[in] len Total byte length of `s`.
 * @param[in,out] pos Current byte offset in `s`.
 * @param[in,out] pending_low Pending low surrogate (0 if none).
 * @param[out] out_unit Next 16-bit UTF-16 code unit.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_next_utf16_unit(const char *s, size_t len,
                                              size_t *pos,
                                              unsigned int *pending_low,
                                              unsigned int *out_unit) {
  unsigned char b0;
  unsigned int cp;
  if (*pending_low != 0) {
    *out_unit = *pending_low;
    *pending_low = 0;
    return CDD_C_SUCCESS;
  }
  b0 = (unsigned char)s[*pos];
  if (b0 < 0x80) {
    *out_unit = b0;
    (*pos) += 1;
    return CDD_C_SUCCESS;
  }
  if ((b0 & 0xE0) == 0xC0 && *pos + 1 < len) {
    cp = ((unsigned int)(b0 & 0x1F) << 6) |
         (unsigned int)((unsigned char)s[*pos + 1] & 0x3F);
    *out_unit = cp;
    (*pos) += 2;
    return CDD_C_SUCCESS;
  }
  if ((b0 & 0xF0) == 0xE0 && *pos + 2 < len) {
    cp = ((unsigned int)(b0 & 0x0F) << 12) |
         ((unsigned int)((unsigned char)s[*pos + 1] & 0x3F) << 6) |
         (unsigned int)((unsigned char)s[*pos + 2] & 0x3F);
    *out_unit = cp;
    (*pos) += 3;
    return CDD_C_SUCCESS;
  }
  if ((b0 & 0xF8) == 0xF0 && *pos + 3 < len) {
    cp = ((unsigned int)(b0 & 0x07) << 18) |
         ((unsigned int)((unsigned char)s[*pos + 1] & 0x3F) << 12) |
         ((unsigned int)((unsigned char)s[*pos + 2] & 0x3F) << 6) |
         (unsigned int)((unsigned char)s[*pos + 3] & 0x3F);
    (*pos) += 4;
    cp -= 0x10000;
    *out_unit = 0xD800u + (cp >> 10);
    *pending_low = 0xDC00u + (cp & 0x3FFu);
    return CDD_C_SUCCESS;
  }
  *out_unit = b0;
  (*pos) += 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Compares two UTF-8 strings by their UTF-16 code unit sequences per
 * RFC 8785 §3.2.3.
 *
 * @param[in] a First string.
 * @param[in] la Byte length of `a`.
 * @param[in] b Second string.
 * @param[in] lb Byte length of `b`.
 * @param[out] out_cmp Negative if `a < b`, 0 if `a == b`, positive if `a > b`.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_cmp_utf16_keys(const char *a, size_t la,
                                             const char *b, size_t lb,
                                             int *out_cmp) {
  size_t pa = 0, pb = 0;
  unsigned int low_a = 0, low_b = 0;
  for (;;) {
    unsigned int ua = 0, ub = 0;
    if (pa >= la && low_a == 0) {
      *out_cmp = -1;
      return CDD_C_SUCCESS;
    }
    if (pb >= lb && low_b == 0) {
      *out_cmp = 1;
      return CDD_C_SUCCESS;
    }
    g_cdd_last_alloc_err = cdd_json_next_utf16_unit(a, la, &pa, &low_a, &ua);
    g_cdd_last_alloc_err = cdd_json_next_utf16_unit(b, lb, &pb, &low_b, &ub);
    if (ua != ub) {
      *out_cmp = (ua < ub) ? -1 : 1;
      return CDD_C_SUCCESS;
    }
  }
}

/**
 * @brief Formats a `JSONNumber` value according to RFC 8785 §3.2.2.3
 * (ECMAScript Number::toString canonicalization).
 *
 * @param[in] val JSONNumber value.
 * @param[out] nbuf Output character buffer (at least 64 bytes).
 * @param[in] nbuf_sz Size of `nbuf`.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_format_canonical_number(const JSON_Value *val,
                                                      char *nbuf,
                                                      size_t nbuf_sz) {
  char *e_ptr;
  size_t len;
  uint64_t max_safe = (((uint64_t)1u) << 53) - 1u;
  int64_t max_safe_s = (int64_t)max_safe;
  nbuf[nbuf_sz - 1] = '\0';
  if (val->number_val == 0.0) {
    nbuf[0] = '0';
    nbuf[1] = '\0';
    return CDD_C_SUCCESS;
  }
  if (val->num_kind == 1 && val->int64_val >= -max_safe_s &&
      val->int64_val <= max_safe_s) {
    uint64_t uval = (val->int64_val < 0)
                        ? (uint64_t)(-(val->int64_val + 1)) + 1u
                        : (uint64_t)val->int64_val;
    return cdd_json_format_u64(uval, val->int64_val < 0, nbuf, &len);
  }
#if defined(_MSC_VER)
  sprintf_s(nbuf, nbuf_sz, "%.15g", val->number_val);
#else
  sprintf(nbuf, "%.15g", val->number_val);
#endif
  if (strtod(nbuf, NULL) != val->number_val) {
#if defined(_MSC_VER)
    sprintf_s(nbuf, nbuf_sz, "%.17g", val->number_val);
#else
    sprintf(nbuf, "%.17g", val->number_val);
#endif
  }
  e_ptr = strchr(nbuf, 'e');
  if (e_ptr) {
    char *exp_p = e_ptr + 2;
    while (*exp_p == '0') {
      memmove(exp_p, exp_p + 1, strlen(exp_p));
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends an indentation prefix of 4 * level spaces.
 *
 * @param[in,out] b Buffer.
 * @param[in] level Indentation depth.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_ser_indent(struct cdd_json_ser_buf *b,
                                         size_t level) {
  size_t i;
  for (i = 0; i < level; ++i)
    g_cdd_last_alloc_err = cdd_json_ser_append(b, "    ", 4);
  return b->err;
}

/**
 * @brief Appends an escaped JSON string literal of explicit byte length
 * (including quotes) to b.
 *
 * @param[in,out] b Buffer.
 * @param[in] s String bytes to escape.
 * @param[in] len Explicit byte length of s.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_ser_string_len(struct cdd_json_ser_buf *b,
                                             const char *s, size_t len) {
  size_t i;
  static const char k_quote[1] = {34};
  static const char k_esc_quote[2] = {92, 34};
  static const char k_esc_bs[2] = {92, 92};
  static const char k_esc_slash[2] = {92, 47};
  static const char k_esc_b[2] = {92, 98};
  static const char k_esc_f[2] = {92, 102};
  static const char k_esc_n[2] = {92, 110};
  static const char k_esc_r[2] = {92, 114};
  static const char k_esc_t[2] = {92, 116};
  g_cdd_last_alloc_err = cdd_json_ser_append(b, k_quote, 1);
  for (i = 0; i < len; ++i) {
    unsigned char c = (unsigned char)s[i];
    if (c == 34)
      g_cdd_last_alloc_err = cdd_json_ser_append(b, k_esc_quote, 2);
    else if (c == 92)
      g_cdd_last_alloc_err = cdd_json_ser_append(b, k_esc_bs, 2);
    else if (c == 47)
      g_cdd_last_alloc_err = cdd_json_ser_append(b, k_esc_slash, 2);
    else if (c == 8)
      g_cdd_last_alloc_err = cdd_json_ser_append(b, k_esc_b, 2);
    else if (c == 12)
      g_cdd_last_alloc_err = cdd_json_ser_append(b, k_esc_f, 2);
    else if (c == 10)
      g_cdd_last_alloc_err = cdd_json_ser_append(b, k_esc_n, 2);
    else if (c == 13)
      g_cdd_last_alloc_err = cdd_json_ser_append(b, k_esc_r, 2);
    else if (c == 9)
      g_cdd_last_alloc_err = cdd_json_ser_append(b, k_esc_t, 2);
    else if (c < 0x20) {
      static const char k_hex_digits[] = "0123456789abcdef";
      char uhex[6];
      uhex[0] = 92;
      uhex[1] = 117;
      uhex[2] = '0';
      uhex[3] = '0';
      uhex[4] = k_hex_digits[(c >> 4) & 0x0F];
      uhex[5] = k_hex_digits[c & 0x0F];
      g_cdd_last_alloc_err = cdd_json_ser_append(b, uhex, 6);
    } else {
      char ch = (char)c;
      g_cdd_last_alloc_err = cdd_json_ser_append(b, &ch, 1);
    }
  }
  return cdd_json_ser_append(b, k_quote, 1);
}

/**
 * @brief Recursively serializes a JSON_Value into b.
 *
 * @param[in] val Value to serialize.
 * @param[in,out] b Output buffer.
 * @param[in] pretty Non-zero for pretty printing.
 * @param[in] canonical Non-zero for RFC 8785 lexicographical key sorting.
 * @param[in] level Current indentation/nesting level.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_ser_value(const JSON_Value *val,
                                        struct cdd_json_ser_buf *b, int pretty,
                                        int canonical, size_t level) {
  size_t i;
  static const char k_arr_pretty_open[2] = {91, 10};
  static const char k_obj_pretty_open[2] = {123, 10};
  static const char k_comma_nl[2] = {44, 10};
  static const char k_nl[1] = {10};
  if (level > CDD_JSON_MAX_DEPTH) {
    b->err = CDD_C_ERROR_PARSE;
    return b->err;
  }
  switch (val->type) {
  case JSONNull:
    return cdd_json_ser_append(b, "null", 4);
  case JSONBoolean:
    return val->boolean_val ? cdd_json_ser_append(b, "true", 4)
                            : cdd_json_ser_append(b, "false", 5);
  case JSONNumber: {
    char nbuf[64];
    if (canonical) {
      g_cdd_last_alloc_err =
          cdd_json_format_canonical_number(val, nbuf, sizeof(nbuf));
      return cdd_json_ser_append(b, nbuf, strlen(nbuf));
    }
    if (val->raw_num_str) {
      return cdd_json_ser_append(b, val->raw_num_str, strlen(val->raw_num_str));
    }
#if defined(_MSC_VER)
    sprintf_s(nbuf, sizeof(nbuf), "%1.17g", val->number_val);
#else
    sprintf(nbuf, "%1.17g", val->number_val);
#endif
    return cdd_json_ser_append(b, nbuf, strlen(nbuf));
  }
  case JSONString: {
    return cdd_json_ser_string_len(b, val->string_val, val->string_len);
  }
  case JSONArray: {
    const JSON_Array *arr = val->array_val;
    if (arr->count == 0)
      return cdd_json_ser_append(b, "[]", 2);
    g_cdd_last_alloc_err = cdd_json_ser_append(
        b, pretty ? k_arr_pretty_open : "[", pretty ? 2 : 1);
    for (i = 0; i < arr->count; ++i) {
      if (pretty)
        g_cdd_last_alloc_err = cdd_json_ser_indent(b, level + 1);
      g_cdd_last_alloc_err =
          cdd_json_ser_value(arr->items[i], b, pretty, canonical, level + 1);
      if (i + 1 < arr->count)
        g_cdd_last_alloc_err =
            cdd_json_ser_append(b, pretty ? k_comma_nl : ",", pretty ? 2 : 1);
    }
    if (pretty) {
      g_cdd_last_alloc_err = cdd_json_ser_append(b, k_nl, 1);
      g_cdd_last_alloc_err = cdd_json_ser_indent(b, level);
    }
    return cdd_json_ser_append(b, "]", 1);
  }
  case JSONObject: {
    const JSON_Object *obj = val->object_val;
    size_t *order = NULL;
    if (obj->count == 0)
      return cdd_json_ser_append(b, "{}", 2);
    if (canonical && obj->count > 1) {
      size_t a_idx, gap;
      order = (size_t *)cdd_json_alloc(obj->count * sizeof(size_t));
      if (!order) {
        b->err = CDD_C_ERROR_MEMORY;
        return b->err;
      }
      for (a_idx = 0; a_idx < obj->count; ++a_idx)
        order[a_idx] = a_idx;
      for (gap = obj->count / 2; gap > 0; gap /= 2) {
        for (a_idx = gap; a_idx < obj->count; ++a_idx) {
          size_t temp = order[a_idx];
          size_t tl = obj->name_lens[temp];
          size_t j = a_idx;
          while (j >= gap) {
            size_t prev = order[j - gap];
            size_t pl = obj->name_lens[prev];
            int cmp = 0;
            g_cdd_last_alloc_err = cdd_json_cmp_utf16_keys(
                obj->names[prev], pl, obj->names[temp], tl, &cmp);
            if (cmp > 0) {
              order[j] = prev;
              j -= gap;
            } else {
              break;
            }
          }
          order[j] = temp;
        }
      }
    }
    g_cdd_last_alloc_err = cdd_json_ser_append(
        b, pretty ? k_obj_pretty_open : "{", pretty ? 2 : 1);
    for (i = 0; i < obj->count; ++i) {
      size_t idx = order ? order[i] : i;
      size_t klen = obj->name_lens[idx];
      if (pretty)
        g_cdd_last_alloc_err = cdd_json_ser_indent(b, level + 1);
      g_cdd_last_alloc_err = cdd_json_ser_string_len(b, obj->names[idx], klen);
      g_cdd_last_alloc_err =
          cdd_json_ser_append(b, pretty ? ": " : ":", pretty ? 2 : 1);
      g_cdd_last_alloc_err =
          cdd_json_ser_value(obj->values[idx], b, pretty, canonical, level + 1);
      if (i + 1 < obj->count)
        g_cdd_last_alloc_err =
            cdd_json_ser_append(b, pretty ? k_comma_nl : ",", pretty ? 2 : 1);
    }
    if (order)
      g_cdd_last_alloc_err = cdd_json_dealloc(order);
    if (pretty) {
      g_cdd_last_alloc_err = cdd_json_ser_append(b, k_nl, 1);
      g_cdd_last_alloc_err = cdd_json_ser_indent(b, level);
    }
    return cdd_json_ser_append(b, "}", 1);
  }
  default:
    b->err = CDD_C_ERROR_INVALID_ARGUMENT;
    return b->err;
  }
}

/**
 * @brief Internal serializer entry point allocating and returning a string.
 *
 * @param[in] val Root JSON_Value.
 * @param[in] pretty Non-zero for pretty output.
 * @param[in] canonical Non-zero for RFC 8785 canonical output.
 * @param[out] out Output string pointer.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_serialize_internal(const JSON_Value *val,
                                                 int pretty, int canonical,
                                                 char **out) {
  struct cdd_json_ser_buf b;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  b.data = NULL;
  b.len = 0;
  b.cap = 0;
  b.fp = NULL;
  b.err = CDD_C_SUCCESS;
  rc = cdd_json_ser_value(val, &b, pretty, canonical, 0);
  if (rc != CDD_C_SUCCESS) {
    if (b.data)
      g_cdd_last_alloc_err = cdd_json_dealloc(b.data);
    return rc;
  }
  *out = b.data;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_serialize_to_string(const JSON_Value *val, char **out) {
  return cdd_json_serialize_internal(val, 0, 0, out);
}

cdd_c_error_t cdd_json_serialize_to_string_pretty(const JSON_Value *val,
                                                  char **out) {
  return cdd_json_serialize_internal(val, 1, 0, out);
}

cdd_c_error_t cdd_json_serialize_to_string_canonical(const JSON_Value *val,
                                                     char **out) {
  return cdd_json_serialize_internal(val, 0, 1, out);
}

cdd_c_error_t cdd_json_serialization_size(const JSON_Value *val, int pretty,
                                          size_t *out_size) {
  char *s = NULL;
  cdd_c_error_t rc;
  if (!val || !out_size)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_size = 0;
  rc = cdd_json_serialize_internal(val, pretty ? 1 : 0, 0, &s);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out_size = strlen(s) + 1;
  g_cdd_last_alloc_err = cdd_json_dealloc(s);
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_serialize_to_buffer(const JSON_Value *val, char *buf,
                                           size_t buf_size, int pretty) {
  char *s = NULL;
  size_t len;
  cdd_c_error_t rc;
  if (!val || !buf || buf_size == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_serialize_internal(val, pretty ? 1 : 0, 0, &s);
  if (rc != CDD_C_SUCCESS)
    return rc;
  len = strlen(s);
  if (len + 1 > buf_size) {
    g_cdd_last_alloc_err = cdd_json_dealloc(s);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  memcpy(buf, s, len + 1);
  g_cdd_last_alloc_err = cdd_json_dealloc(s);
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_serialize_to_fp(const JSON_Value *val, FILE *fp,
                                       int pretty) {
  struct cdd_json_ser_buf b;
  if (!val || !fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  b.data = NULL;
  b.len = 0;
  b.cap = 0;
  b.fp = fp;
  b.err = CDD_C_SUCCESS;
  return cdd_json_ser_value(val, &b, pretty ? 1 : 0, 0, 0);
}

cdd_c_error_t cdd_json_serialize_to_file_pretty(const JSON_Value *val,
                                                const char *filename) {
  FILE *fp = NULL;
  cdd_c_error_t rc;
  if (!val || !filename)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#if defined(_MSC_VER)
  if (fopen_s(&fp, filename, "wb") != 0 || !fp)
    return CDD_C_ERROR_IO;
#else
  fp = fopen(filename, "wb");
  if (!fp)
    return CDD_C_ERROR_IO;
#endif
  rc = cdd_json_serialize_to_fp(val, fp, 1);
  fclose(fp);
  return rc;
}

JSON_Status json_serialize_to_file_pretty(const JSON_Value *val,
                                          const char *filename) {
  return (cdd_json_serialize_to_file_pretty(val, filename) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_ndjson_parse_line(const char *line, JSON_Value **out) {
  size_t len;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!line)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  len = strlen(line);
  while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
    --len;
  if (len == 0)
    return CDD_C_ERROR_PARSE;
  return cdd_json_parse_buffer(line, len, out);
}

cdd_c_error_t cdd_json_ndjson_parse_string(const char *ndjson_str,
                                           JSON_Value **out_array) {
  JSON_Value *arr_val = NULL;
  const char *p;
  cdd_c_error_t rc;
  if (!out_array)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_array = NULL;
  if (!ndjson_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_array(&arr_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  p = ndjson_str;
  while (*p != '\0') {
    const char *nl = strchr(p, '\n');
    size_t line_len = nl ? (size_t)(nl - p) : strlen(p);
    size_t s = 0;
    while (line_len > 0 && p[line_len - 1] == '\r')
      --line_len;
    while (s < line_len && (p[s] == ' ' || p[s] == '\t'))
      ++s;
    if (s < line_len) {
      JSON_Value *item = NULL;
      rc = cdd_json_parse_buffer(p + s, line_len - s, &item);
      if (rc != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_value_free(arr_val);
        return rc;
      }
      rc = cdd_json_array_append_value(arr_val->array_val, item);
      if (rc != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_value_free(item);
        g_cdd_last_alloc_err = cdd_json_value_free(arr_val);
        return rc;
      }
    }
    if (!nl)
      break;
    p = nl + 1;
  }
  *out_array = arr_val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_ndjson_serialize_array(const JSON_Array *arr,
                                              char **out_ndjson) {
  struct cdd_json_ser_buf b;
  size_t i;
  if (!out_ndjson)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_ndjson = NULL;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  b.data = NULL;
  b.len = 0;
  b.cap = 0;
  b.fp = NULL;
  b.err = CDD_C_SUCCESS;
  if (arr->count == 0) {
    return cdd_json_strndup("", 0, out_ndjson);
  }
  for (i = 0; i < arr->count; ++i) {
    g_cdd_last_alloc_err = cdd_json_ser_value(arr->items[i], &b, 0, 0, 0);
    g_cdd_last_alloc_err = cdd_json_ser_append(&b, "\n", 1);
  }
  if (b.err != CDD_C_SUCCESS) {
    if (b.data)
      g_cdd_last_alloc_err = cdd_json_dealloc(b.data);
    return b.err;
  }
  *out_ndjson = b.data;
  return CDD_C_SUCCESS;
}

/* --- RFC 6901 JSON Pointer, RFC 7396 Merge Patch & RFC 6902 JSON Patch --- */

cdd_c_error_t cdd_json_pointer_unescape(const char *in, size_t len,
                                        char **out_token) {
  size_t i, j = 0;
  char *out;
  if (!out_token)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_token = NULL;
  if (!in)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  out = (char *)cdd_json_ctx_alloc(len + 1);
  if (!out)
    return CDD_C_ERROR_MEMORY;
  for (i = 0; i < len; ++i) {
    if (in[i] == '~' && i + 1 < len) {
      if (in[i + 1] == '0') {
        out[j++] = '~';
        i++;
        continue;
      }
      if (in[i + 1] == '1') {
        out[j++] = '/';
        i++;
        continue;
      }
    }
    out[j++] = in[i];
  }
  out[j] = '\0';
  *out_token = out;
  return CDD_C_SUCCESS;
}

/**
 * @brief Validates that every `~` in `in[0..len-1]` is followed by `0` or `1`
 * per RFC 6901, then unescapes the token.
 *
 * @param[in] in Pointer segment.
 * @param[in] len Segment byte length.
 * @param[out] out_token Newly allocated unescaped token.
 * @return CDD_C_SUCCESS, CDD_C_ERROR_INVALID_ARGUMENT, or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t
cdd_json_pointer_unescape_strict(const char *in, size_t len, char **out_token) {
  size_t k;
  for (k = 0; k < len; ++k) {
    if (in[k] == '~') {
      if (k + 1 >= len || (in[k + 1] != '0' && in[k + 1] != '1'))
        return CDD_C_ERROR_INVALID_ARGUMENT;
      ++k;
    }
  }
  return cdd_json_pointer_unescape(in, len, out_token);
}

/**
 * @brief Frees a token allocated by cdd_json_pointer_unescape.
 *
 * @param[in] tok Token buffer to free.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_pointer_free_token(char *tok) {
  g_cdd_last_alloc_err = cdd_json_ctx_dealloc(tok);
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_pointer_escape(const char *token, char *out_buf,
                                      size_t buf_sz) {
  size_t i = 0;
  size_t pos = 0;
  if (!token || !out_buf || buf_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  while (token[i] != '\0') {
    if (token[i] == '~') {
      if (pos + 2 >= buf_sz)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      out_buf[pos++] = '~';
      out_buf[pos++] = '0';
    } else if (token[i] == '/') {
      if (pos + 2 >= buf_sz)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      out_buf[pos++] = '~';
      out_buf[pos++] = '1';
    } else {
      if (pos + 1 >= buf_sz)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      out_buf[pos++] = token[i];
    }
    ++i;
  }
  out_buf[pos] = '\0';
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_pointer_get(const JSON_Value *root, const char *pointer,
                                   const JSON_Value **out_val) {
  const JSON_Value *cur;
  const char *p;
  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_val = NULL;
  if (!root || !pointer)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  p = pointer;
  if (*p == '#')
    p++;
  if (*p == '\0') {
    *out_val = root;
    return CDD_C_SUCCESS;
  }
  if (*p != '/')
    return CDD_C_ERROR_INVALID_ARGUMENT;
  cur = root;
  while (*p == '/') {
    const char *slash;
    size_t seg_len;
    char *token = NULL;
    cdd_c_error_t rc;
    p++;
    slash = strchr(p, '/');
    seg_len = slash ? (size_t)(slash - p) : strlen(p);
    rc = cdd_json_pointer_unescape_strict(p, seg_len, &token);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (cur->type == JSONObject) {
      JSON_Value *child = NULL;
      if (cdd_json_object_get_value(cur->object_val, token, &child) !=
          CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_pointer_free_token(token);
        return CDD_C_ERROR_NOT_FOUND;
      }
      cur = child;
    } else if (cur->type == JSONArray) {
      char *endp = NULL;
      unsigned long idx;
      if (token[0] == '\0' || (token[0] == '0' && token[1] != '\0')) {
        g_cdd_last_alloc_err = cdd_json_pointer_free_token(token);
        return CDD_C_ERROR_NOT_FOUND;
      }
      idx = strtoul(token, &endp, 10);
      if (*endp != '\0' || idx >= cur->array_val->count) {
        g_cdd_last_alloc_err = cdd_json_pointer_free_token(token);
        return CDD_C_ERROR_NOT_FOUND;
      }
      cur = cur->array_val->items[idx];
    } else {
      g_cdd_last_alloc_err = cdd_json_pointer_free_token(token);
      return CDD_C_ERROR_NOT_FOUND;
    }
    g_cdd_last_alloc_err = cdd_json_pointer_free_token(token);
    p += seg_len;
  }
  *out_val = cur;
  return CDD_C_SUCCESS;
}

/**
 * @brief Looks up a property `kstart[0..klen-1]` on an object element.
 *
 * @param[in] elem Candidate JSON_Value element.
 * @param[in] kstart Property name start.
 * @param[in] klen Property name length.
 * @param[out] out_fval Resolved property value (or NULL if missing).
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_filter_lookup_prop(const JSON_Value *elem,
                                                 const char *kstart,
                                                 size_t klen,
                                                 const JSON_Value **out_fval) {
  size_t ki;
  *out_fval = NULL;
  if (elem->type != JSONObject)
    return CDD_C_SUCCESS;
  for (ki = 0; ki < elem->object_val->count; ++ki) {
    size_t nlen = elem->object_val->name_lens[ki];
    if (nlen == klen &&
        memcmp(elem->object_val->names[ki], kstart, klen) == 0) {
      *out_fval = elem->object_val->values[ki];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a single- or double-quoted JSONPath string literal, handling
 * `\'`, `\"`, `\\`, `\n`, `\r`, `\t`, `\b`, `\f`, and `\/` escapes.
 *
 * @param[in,out] cursor Pointer to the opening quote (`'` or `"`). Advanced
 * past the closing quote on success.
 * @param[out] out_str Newly allocated unescaped string via
 * `cdd_json_ctx_alloc`.
 * @param[out] out_len Output byte length.
 * @return CDD_C_SUCCESS, CDD_C_ERROR_INVALID_ARGUMENT, or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_parse_jp_quoted(const char **cursor,
                                              char **out_str, size_t *out_len) {
  const char *p = *cursor;
  char quote;
  const char *scan;
  size_t max_len = 0;
  char *buf;
  size_t oi = 0;
  quote = *p++;
  scan = p;
  while (*scan != '\0' && *scan != quote) {
    if (*scan == '\\') {
      ++scan;
      if (*scan == '\0')
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    ++scan;
    ++max_len;
  }
  if (*scan != quote)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  buf = (char *)cdd_json_ctx_alloc(max_len + 1);
  if (!buf)
    return CDD_C_ERROR_MEMORY;
  while (*p != quote) {
    if (*p == '\\') {
      ++p;
      if (*p == 'n')
        buf[oi++] = '\n';
      else if (*p == 'r')
        buf[oi++] = '\r';
      else if (*p == 't')
        buf[oi++] = '\t';
      else if (*p == 'b')
        buf[oi++] = '\b';
      else if (*p == 'f')
        buf[oi++] = '\f';
      else
        buf[oi++] = *p;
      ++p;
    } else {
      buf[oi++] = *p++;
    }
  }
  buf[oi] = '\0';
  *cursor = p + 1;
  *out_str = buf;
  *out_len = oi;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if character `ch` matches a bracketed character class
 * `[...]` at `pat[0..c_end-1]`.
 *
 * @param[in] pat Inside of `[...]` (after `[`).
 * @param[in] c_end Length of class content before `]`.
 * @param[in] ch Character to test.
 * @param[out] out_matched 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_iregexp_class(const char *pat, size_t c_end,
                                            unsigned char ch,
                                            int *out_matched) {
  size_t i = 0;
  int neg = 0;
  int found = 0;
  if (pat[0] == '^') {
    neg = 1;
    ++i;
  }
  while (i < c_end) {
    unsigned char c1 = (unsigned char)pat[i++];
    if (c1 == '\\')
      c1 = (unsigned char)pat[i++];
    if (i + 1 < c_end && pat[i] == '-') {
      unsigned char c2 = (unsigned char)pat[i + 1];
      i += 2;
      if (c2 == '\\')
        c2 = (unsigned char)pat[i++];
      if (ch >= c1 && ch <= c2)
        found = 1;
    } else {
      if (ch == c1)
        found = 1;
    }
  }
  *out_matched = neg ? !found : found;
  return CDD_C_SUCCESS;
}

/**
 * @brief Matches a single I-Regexp atom against `ch`.
 *
 * @param[in] atom_p Atom start in pattern.
 * @param[in] atom_len Atom byte length.
 * @param[in] ch Input character.
 * @param[out] out_matched 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_iregexp_atom(const char *atom_p, size_t atom_len,
                                           unsigned char ch, int *out_matched) {
  *out_matched = 0;
  if (atom_p[0] == '.') {
    *out_matched = (ch != '\n' && ch != '\r') ? 1 : 0;
    return CDD_C_SUCCESS;
  }
  if (atom_p[0] == '\\') {
    char esc = atom_p[1];
    if (esc == 'd' || esc == 'D') {
      int is_d = (ch >= '0' && ch <= '9');
      *out_matched = (esc == 'd') ? is_d : !is_d;
    } else if (esc == 's' || esc == 'S') {
      int is_s = (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
      *out_matched = (esc == 's') ? is_s : !is_s;
    } else if (esc == 'w' || esc == 'W') {
      int is_w = ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                  (ch >= '0' && ch <= '9') || ch == '_');
      *out_matched = (esc == 'w') ? is_w : !is_w;
    } else if (esc == 'n')
      *out_matched = (ch == '\n');
    else if (esc == 'r')
      *out_matched = (ch == '\r');
    else if (esc == 't')
      *out_matched = (ch == '\t');
    else
      *out_matched = (ch == (unsigned char)esc);
    return CDD_C_SUCCESS;
  }
  if (atom_p[0] == '[') {
    return cdd_json_iregexp_class(atom_p + 1, atom_len - 2, ch, out_matched);
  }
  *out_matched = (ch == (unsigned char)atom_p[0]) ? 1 : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Validates the syntax of an I-Regexp pattern (checking balanced
 * `[...]` and trailing `\`).
 *
 * @param[in] pat Pattern string.
 * @param[in] pat_len Pattern byte length.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_INVALID_ARGUMENT.
 */
static cdd_c_error_t cdd_json_iregexp_validate(const char *pat,
                                               size_t pat_len) {
  size_t i = 0;
  while (i < pat_len) {
    if (pat[i] == '\\') {
      if (i + 1 >= pat_len)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      i += 2;
    } else if (pat[i] == '[') {
      ++i;
      if (pat[i] == '^')
        ++i;
      if (pat[i] == ']')
        ++i;
      while (i < pat_len && pat[i] != ']') {
        if (pat[i] == '\\') {
          if (i + 1 >= pat_len)
            return CDD_C_ERROR_INVALID_ARGUMENT;
          i += 2;
        } else {
          ++i;
        }
      }
      if (i >= pat_len)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      ++i;
    } else {
      ++i;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates a validated I-Regexp pattern branch (without top-level `|`)
 * against `str[str_pos..str_len]`.
 *
 * @param[in] pat Pattern branch string.
 * @param[in] pat_len Length of pattern branch string.
 * @param[in] str Input string to match against.
 * @param[in] str_len Length of input string.
 * @param[in] str_pos Starting byte offset in `str`.
 * @param[in] anchor_end Non-zero if match must reach `str_len`.
 * @param[out] out_matched Receives 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_iregexp_branch(const char *pat, size_t pat_len,
                                             const char *str, size_t str_len,
                                             size_t str_pos, int anchor_end,
                                             int *out_matched) {
  size_t atom_len = 0;
  char quant = 0;
  size_t next_pat;
  *out_matched = 0;
  if (pat_len == 0) {
    *out_matched = (!anchor_end || str_pos == str_len) ? 1 : 0;
    return CDD_C_SUCCESS;
  }
  if (pat[0] == '^') {
    if (str_pos != 0)
      return CDD_C_SUCCESS;
    return cdd_json_iregexp_branch(pat + 1, pat_len - 1, str, str_len, str_pos,
                                   anchor_end, out_matched);
  }
  if (pat[0] == '$' && pat_len == 1) {
    *out_matched = (str_pos == str_len) ? 1 : 0;
    return CDD_C_SUCCESS;
  }
  if (pat[0] == '[') {
    size_t k = 1;
    if (pat[k] == '^')
      ++k;
    if (pat[k] == ']')
      ++k;
    while (pat[k] != ']') {
      if (pat[k] == '\\')
        ++k;
      ++k;
    }
    atom_len = k + 1;
  } else if (pat[0] == '\\') {
    atom_len = 2;
  } else {
    atom_len = 1;
  }
  next_pat = atom_len;
  if (next_pat < pat_len &&
      (pat[next_pat] == '*' || pat[next_pat] == '+' || pat[next_pat] == '?')) {
    quant = pat[next_pat];
    ++next_pat;
  }
  if (quant == '*') {
    size_t cur = str_pos;
    for (;;) {
      int m = 0;
      g_cdd_last_alloc_err =
          cdd_json_iregexp_branch(pat + next_pat, pat_len - next_pat, str,
                                  str_len, cur, anchor_end, &m);
      if (m) {
        *out_matched = 1;
        return CDD_C_SUCCESS;
      }
      if (cur >= str_len)
        break;
      g_cdd_last_alloc_err =
          cdd_json_iregexp_atom(pat, atom_len, (unsigned char)str[cur], &m);
      if (!m)
        break;
      ++cur;
    }
    return CDD_C_SUCCESS;
  }
  if (quant == '+') {
    size_t cur = str_pos;
    int m_atom = 0;
    if (cur >= str_len)
      return CDD_C_SUCCESS;
    g_cdd_last_alloc_err =
        cdd_json_iregexp_atom(pat, atom_len, (unsigned char)str[cur], &m_atom);
    if (!m_atom)
      return CDD_C_SUCCESS;
    ++cur;
    for (;;) {
      int m = 0;
      g_cdd_last_alloc_err =
          cdd_json_iregexp_branch(pat + next_pat, pat_len - next_pat, str,
                                  str_len, cur, anchor_end, &m);
      if (m) {
        *out_matched = 1;
        return CDD_C_SUCCESS;
      }
      if (cur >= str_len)
        break;
      g_cdd_last_alloc_err =
          cdd_json_iregexp_atom(pat, atom_len, (unsigned char)str[cur], &m);
      if (!m)
        break;
      ++cur;
    }
    return CDD_C_SUCCESS;
  }
  if (quant == '?') {
    int m = 0;
    if (str_pos < str_len) {
      int m_atom = 0;
      g_cdd_last_alloc_err = cdd_json_iregexp_atom(
          pat, atom_len, (unsigned char)str[str_pos], &m_atom);
      if (m_atom) {
        g_cdd_last_alloc_err =
            cdd_json_iregexp_branch(pat + next_pat, pat_len - next_pat, str,
                                    str_len, str_pos + 1, anchor_end, &m);
        if (m) {
          *out_matched = 1;
          return CDD_C_SUCCESS;
        }
      }
    }
    return cdd_json_iregexp_branch(pat + next_pat, pat_len - next_pat, str,
                                   str_len, str_pos, anchor_end, out_matched);
  }
  if (str_pos < str_len) {
    int m_atom = 0;
    g_cdd_last_alloc_err = cdd_json_iregexp_atom(
        pat, atom_len, (unsigned char)str[str_pos], &m_atom);
    if (m_atom) {
      return cdd_json_iregexp_branch(pat + next_pat, pat_len - next_pat, str,
                                     str_len, str_pos + 1, anchor_end,
                                     out_matched);
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates an I-Regexp pattern (supporting `|` alternation) against
 * `str[str_pos..str_len]`.
 *
 * @param[in] pat Pattern string.
 * @param[in] pat_len Length of pattern string.
 * @param[in] str Input string to match against.
 * @param[in] str_len Length of input string.
 * @param[in] str_pos Starting byte offset in `str`.
 * @param[in] anchor_end Non-zero if match must reach `str_len`.
 * @param[out] out_matched Receives 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS on valid evaluation, or error code on invalid pattern.
 */
static cdd_c_error_t cdd_json_iregexp_exec(const char *pat, size_t pat_len,
                                           const char *str, size_t str_len,
                                           size_t str_pos, int anchor_end,
                                           int *out_matched) {
  size_t i = 0;
  size_t seg_start = 0;
  int in_brackets = 0;
  cdd_c_error_t rc;
  *out_matched = 0;
  rc = cdd_json_iregexp_validate(pat, pat_len);
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (i = 0; i <= pat_len; ++i) {
    if (i < pat_len && pat[i] == '\\') {
      ++i;
      continue;
    }
    if (i < pat_len && pat[i] == '[')
      in_brackets = 1;
    else if (i < pat_len && pat[i] == ']')
      in_brackets = 0;
    if (i == pat_len || (!in_brackets && pat[i] == '|')) {
      int m = 0;
      g_cdd_last_alloc_err =
          cdd_json_iregexp_branch(pat + seg_start, i - seg_start, str, str_len,
                                  str_pos, anchor_end, &m);
      if (m) {
        *out_matched = 1;
        return CDD_C_SUCCESS;
      }
      seg_start = i + 1;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Skips spaces and horizontal tabs in a filter expression cursor.
 *
 * @param[in,out] cursor Filter cursor.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_filter_skip_ws(const char **cursor) {
  while (**cursor == ' ' || **cursor == '\t')
    (*cursor)++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves a relative (`@...`) or root (`$...`) path inside a filter
 * expression.
 *
 * @param[in] base Base node (`elem` for `@`, `root` for `$`; NULL for syntax
 * validation).
 * @param[in,out] cursor Cursor pointing at `.` or `[` after `@` or `$`.
 * @param[out] out_fval Resolved JSON_Value (or NULL if not found).
 * @return CDD_C_SUCCESS, CDD_C_ERROR_INVALID_ARGUMENT, or
 * CDD_C_ERROR_NOT_SUPPORTED.
 */
static cdd_c_error_t cdd_json_filter_resolve_path(const JSON_Value *base,
                                                  const char **cursor,
                                                  const JSON_Value **out_fval) {
  const char *p = *cursor;
  const JSON_Value *cur = base;
  int steps = 0;
  *out_fval = NULL;
  while (*p == '.' || *p == '[') {
    if (*p == '.') {
      const char *kstart;
      size_t klen = 0;
      ++p;
      kstart = p;
      while (!strchr(".[]() \t=!<>|&,", *p)) {
        ++p;
        ++klen;
      }
      if (klen == 0 || *p == '(')
        return CDD_C_ERROR_NOT_SUPPORTED;
      if (cur) {
        const JSON_Value *next = NULL;
        g_cdd_last_alloc_err =
            cdd_json_filter_lookup_prop(cur, kstart, klen, &next);
        cur = next;
      }
      ++steps;
    } else {
      ++p;
      g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
      if (*p == 39 || *p == '"') {
        char *key = NULL;
        size_t klen = 0;
        cdd_c_error_t rc = cdd_json_parse_jp_quoted(&p, &key, &klen);
        if (rc != CDD_C_SUCCESS)
          return rc;
        g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
        if (*p != ']') {
          g_cdd_last_alloc_err = cdd_json_pointer_free_token(key);
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
        ++p;
        if (cur) {
          const JSON_Value *next = NULL;
          g_cdd_last_alloc_err =
              cdd_json_filter_lookup_prop(cur, key, klen, &next);
          cur = next;
        }
        g_cdd_last_alloc_err = cdd_json_pointer_free_token(key);
        ++steps;
      } else if (*p == '-' || (*p >= '0' && *p <= '9')) {
        char *endp = NULL;
        long idx = strtol(p, &endp, 10);
        if (endp == p)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        p = endp;
        g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
        if (*p != ']')
          return CDD_C_ERROR_INVALID_ARGUMENT;
        ++p;
        if (cur) {
          if (cur->type == JSONArray) {
            long cnt = (long)cur->array_val->count;
            long eff = idx < 0 ? cnt + idx : idx;
            cur = (eff >= 0 && eff < cnt) ? cur->array_val->items[(size_t)eff]
                                          : NULL;
          } else {
            cur = NULL;
          }
        }
        ++steps;
      } else {
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
  }
  if (steps == 0)
    return CDD_C_ERROR_NOT_SUPPORTED;
  *cursor = p;
  *out_fval = cur;
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates relational operator `op` (1: ==, 2: !=, 3: <=, 4: <, 5: >=,
 * 6: >) on two doubles.
 *
 * @param[in] op Operator code (1..6).
 * @param[in] lhs Left operand.
 * @param[in] rhs Right operand.
 * @param[out] out_res 1 if true, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_cmp_double_op(int op, double lhs, double rhs,
                                            int *out_res) {
  if (op == 1)
    *out_res = (lhs == rhs);
  else if (op == 2)
    *out_res = (lhs != rhs);
  else if (op == 3)
    *out_res = (lhs <= rhs);
  else if (op == 4)
    *out_res = (lhs < rhs);
  else if (op == 5)
    *out_res = (lhs >= rhs);
  else
    *out_res = (lhs > rhs);
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates a single atomic filter predicate against `elem` (or
 * validates syntax when `elem` is NULL).
 *
 * @param[in] root Root document node for `$` references.
 * @param[in] elem Candidate JSON_Value element (NULL for syntax-only pass).
 * @param[in,out] cursor Filter expression cursor.
 * @param[out] out_res Output 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS, CDD_C_ERROR_INVALID_ARGUMENT, or
 * CDD_C_ERROR_NOT_SUPPORTED.
 */
static cdd_c_error_t cdd_json_eval_filter_atom(const JSON_Value *root,
                                               const JSON_Value *elem,
                                               const char **cursor,
                                               int *out_res) {
  const char *p = *cursor;
  const JSON_Value *fval = NULL;
  int fn_kind = 0; /* 0:none, 1:length/count, 2:match, 3:search, 4:value */
  int op = 0;      /* 0:existence, 1:==, 2:!=, 3:<=, 4:<, 5:>=, 6:> */
  JSON_Value_Type rhs_type = JSONError;
  char *rhs_str_owned = NULL;
  const char *rhs_str = NULL;
  size_t rhs_len = 0;
  double rhs_num = 0.0;
  int rhs_bool = 0;
  cdd_c_error_t rc;

  *out_res = 0;
  g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);

  if (strncmp(p, "length(@", 8) == 0 || strncmp(p, "count(@", 7) == 0 ||
      strncmp(p, "value(@", 7) == 0) {
    fn_kind = (p[0] == 'v') ? 4 : 1;
    p += (p[0] == 'l') ? 8 : 7;
    rc = cdd_json_filter_resolve_path(elem, &p, &fval);
    if (rc != CDD_C_SUCCESS)
      return (rc == CDD_C_ERROR_NOT_SUPPORTED) ? CDD_C_ERROR_INVALID_ARGUMENT
                                               : rc;
    g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
    if (*p != ')')
      return CDD_C_ERROR_INVALID_ARGUMENT;
    ++p;
  } else if (strncmp(p, "match(@", 7) == 0 || strncmp(p, "search(@", 8) == 0) {
    char *pat_owned = NULL;
    size_t pat_len = 0;
    fn_kind = (p[0] == 'm') ? 2 : 3;
    p += (fn_kind == 2) ? 7 : 8;
    rc = cdd_json_filter_resolve_path(elem, &p, &fval);
    if (rc != CDD_C_SUCCESS)
      return (rc == CDD_C_ERROR_NOT_SUPPORTED) ? CDD_C_ERROR_INVALID_ARGUMENT
                                               : rc;
    g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
    if (*p != ',')
      return CDD_C_ERROR_INVALID_ARGUMENT;
    ++p;
    g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
    if (*p != 39 && *p != '"')
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = cdd_json_parse_jp_quoted(&p, &pat_owned, &pat_len);
    if (rc != CDD_C_SUCCESS)
      return rc;
    g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
    if (*p != ')') {
      g_cdd_last_alloc_err = cdd_json_pointer_free_token(pat_owned);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    ++p;
    *cursor = p;
    if (!elem) {
      int dummy_m = 0;
      rc = cdd_json_iregexp_exec(pat_owned, pat_len, "", 0, 0, 1, &dummy_m);
      g_cdd_last_alloc_err = cdd_json_pointer_free_token(pat_owned);
      return rc;
    }
    if (fval && fval->type == JSONString) {
      size_t flen = fval->string_len;
      if (fn_kind == 2) {
        g_cdd_last_alloc_err = cdd_json_iregexp_exec(
            pat_owned, pat_len, fval->string_val, flen, 0, 1, out_res);
      } else {
        size_t si;
        if (pat_len == 0) {
          *out_res = 1;
        } else {
          for (si = 0; si <= flen; ++si) {
            int m = 0;
            g_cdd_last_alloc_err = cdd_json_iregexp_exec(
                pat_owned, pat_len, fval->string_val, flen, si, 0, &m);
            if (m) {
              *out_res = 1;
              break;
            }
          }
        }
      }
    }
    g_cdd_last_alloc_err = cdd_json_pointer_free_token(pat_owned);
    return CDD_C_SUCCESS;
  } else if (p[0] == '@' && (p[1] == '.' || p[1] == '[')) {
    ++p;
    rc = cdd_json_filter_resolve_path(elem, &p, &fval);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    return CDD_C_ERROR_NOT_SUPPORTED;
  }

  g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
  if ((fn_kind == 0 || fn_kind == 4) &&
      (*p == ')' || *p == ']' || strncmp(p, "&&", 2) == 0 ||
       strncmp(p, "||", 2) == 0)) {
    *cursor = p;
    if (elem) {
      *out_res = (fval != NULL) ? 1 : 0;
    }
    return CDD_C_SUCCESS;
  }
  if (strncmp(p, "==", 2) == 0) {
    op = 1;
    p += 2;
  } else if (strncmp(p, "!=", 2) == 0) {
    op = 2;
    p += 2;
  } else if (strncmp(p, "<=", 2) == 0) {
    op = 3;
    p += 2;
  } else if (*p == '<') {
    op = 4;
    p += 1;
  } else if (strncmp(p, ">=", 2) == 0) {
    op = 5;
    p += 2;
  } else if (*p == '>') {
    op = 6;
    p += 1;
  } else {
    return CDD_C_ERROR_NOT_SUPPORTED;
  }
  g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
  if (*p == 39 || *p == '"') {
    rc = cdd_json_parse_jp_quoted(&p, &rhs_str_owned, &rhs_len);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rhs_type = JSONString;
    rhs_str = rhs_str_owned;
  } else if (*p == '-' || (*p >= '0' && *p <= '9')) {
    char *endp = NULL;
    rhs_num = strtod(p, &endp);
    if (endp == p)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rhs_type = JSONNumber;
    p = endp;
  } else if (strncmp(p, "true", 4) == 0) {
    rhs_type = JSONBoolean;
    rhs_bool = 1;
    p += 4;
  } else if (strncmp(p, "false", 5) == 0) {
    rhs_type = JSONBoolean;
    rhs_bool = 0;
    p += 5;
  } else if (strncmp(p, "null", 4) == 0) {
    rhs_type = JSONNull;
    p += 4;
  } else if (p[0] == '$' && (p[1] == '.' || p[1] == '[')) {
    const JSON_Value *rval = NULL;
    ++p;
    rc = cdd_json_filter_resolve_path(root, &p, &rval);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (rval) {
      rhs_type = rval->type;
      if (rval->type == JSONString) {
        rhs_str = rval->string_val;
        rhs_len = rval->string_len;
      } else if (rval->type == JSONNumber) {
        rhs_num = rval->number_val;
      } else if (rval->type == JSONBoolean) {
        rhs_bool = rval->boolean_val != 0;
      }
    }
  } else {
    return CDD_C_ERROR_NOT_SUPPORTED;
  }
  *cursor = p;
  if (!elem) {
    if (rhs_str_owned)
      g_cdd_last_alloc_err = cdd_json_pointer_free_token(rhs_str_owned);
    return CDD_C_SUCCESS;
  }

  {
    int matched = 0;
    if (fval) {
      if (fn_kind == 1) {
        if (rhs_type == JSONNumber) {
          if (fval->type == JSONString)
            g_cdd_last_alloc_err = cdd_json_cmp_double_op(
                op, (double)fval->string_len, rhs_num, &matched);
          else if (fval->type == JSONArray)
            g_cdd_last_alloc_err = cdd_json_cmp_double_op(
                op, (double)fval->array_val->count, rhs_num, &matched);
          else if (fval->type == JSONObject)
            g_cdd_last_alloc_err = cdd_json_cmp_double_op(
                op, (double)fval->object_val->count, rhs_num, &matched);
        }
      } else if (rhs_type == JSONString && fval->type == JSONString) {
        int eq = (fval->string_len == rhs_len &&
                  memcmp(fval->string_val, rhs_str, rhs_len) == 0);
        matched = (op == 1) ? eq : !eq;
      } else if (rhs_type == JSONNumber && fval->type == JSONNumber) {
        g_cdd_last_alloc_err =
            cdd_json_cmp_double_op(op, fval->number_val, rhs_num, &matched);
      } else if (rhs_type == JSONBoolean && fval->type == JSONBoolean) {
        int eq = ((fval->boolean_val != 0) == rhs_bool);
        matched = (op == 1) ? eq : !eq;
      } else if (rhs_type == JSONNull) {
        int eq = (fval->type == JSONNull);
        matched = (op == 1) ? eq : !eq;
      }
    }
    *out_res = matched ? 1 : 0;
  }
  if (rhs_str_owned)
    g_cdd_last_alloc_err = cdd_json_pointer_free_token(rhs_str_owned);
  return CDD_C_SUCCESS;
}

static cdd_c_error_t cdd_json_eval_filter_expr(const JSON_Value *root,
                                               const JSON_Value *elem,
                                               const char **cursor,
                                               int *out_res);

/**
 * @brief Evaluates a unary or parenthesized filter factor.
 *
 * @param[in] root Root document node.
 * @param[in] elem Candidate JSON_Value element (or NULL for syntax check).
 * @param[in,out] cursor Filter expression cursor.
 * @param[out] out_res Output 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_eval_filter_unary(const JSON_Value *root,
                                                const JSON_Value *elem,
                                                const char **cursor,
                                                int *out_res) {
  const char *p = *cursor;
  int neg = 0;
  int sub = 0;
  cdd_c_error_t rc;
  g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
  while (*p == '!') {
    neg = !neg;
    ++p;
    g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
  }
  if (*p == '(') {
    ++p;
    rc = cdd_json_eval_filter_expr(root, elem, &p, &sub);
    if (rc != CDD_C_SUCCESS)
      return rc;
    g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
    if (*p != ')')
      return CDD_C_ERROR_INVALID_ARGUMENT;
    ++p;
  } else {
    rc = cdd_json_eval_filter_atom(root, elem, &p, &sub);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  *cursor = p;
  *out_res = neg ? !sub : sub;
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates a compound RFC 9535 filter expression (`&&`, `||`) against
 * `elem`.
 *
 * @param[in] root Root document node.
 * @param[in] elem Candidate JSON_Value element (or NULL for syntax check).
 * @param[in,out] cursor Filter expression cursor.
 * @param[out] out_res Output 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_eval_filter_expr(const JSON_Value *root,
                                               const JSON_Value *elem,
                                               const char **cursor,
                                               int *out_res) {
  const char *p = *cursor;
  int lhs = 0;
  cdd_c_error_t rc;
  rc = cdd_json_eval_filter_unary(root, elem, &p, &lhs);
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (;;) {
    int is_and = 0;
    int rhs = 0;
    g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
    if (strncmp(p, "&&", 2) == 0) {
      is_and = 1;
      p += 2;
    } else if (strncmp(p, "||", 2) == 0) {
      is_and = 0;
      p += 2;
    } else {
      break;
    }
    rc = cdd_json_eval_filter_unary(root, elem, &p, &rhs);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_and)
      lhs = (lhs && rhs) ? 1 : 0;
    else
      lhs = (lhs || rhs) ? 1 : 0;
  }
  *cursor = p;
  *out_res = lhs;
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates an RFC 9535 filter selector `[?(...)]` against a JSONArray,
 * advancing `*cursor` past the closing `]`.
 *
 * @param[in] root Root JSON_Value document.
 * @param[in] arr_val Current JSON_Value (must be JSONArray to match).
 * @param[in,out] cursor Pointer to the character after `?` in the JSONPath.
 * @param[out] out_matched Output pointer receiving the first matched element.
 * @return CDD_C_SUCCESS on match, CDD_C_ERROR_NOT_FOUND if no element matches,
 * CDD_C_ERROR_NOT_SUPPORTED for unsupported filter syntax, or
 * CDD_C_ERROR_INVALID_ARGUMENT if malformed.
 */
static cdd_c_error_t
cdd_json_eval_filter_selector(const JSON_Value *root, const JSON_Value *arr_val,
                              const char **cursor,
                              const JSON_Value **out_matched) {
  const char *expr_start = *cursor;
  const char *p = expr_start;
  int dummy = 0;
  size_t i;
  cdd_c_error_t rc;

  rc = cdd_json_eval_filter_expr(root, NULL, &p, &dummy);
  if (rc != CDD_C_SUCCESS)
    return rc;
  g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
  if (*p != ']')
    return CDD_C_ERROR_NOT_SUPPORTED;
  *cursor = p + 1;

  if (arr_val->type != JSONArray)
    return CDD_C_ERROR_NOT_FOUND;

  for (i = 0; i < arr_val->array_val->count; ++i) {
    const JSON_Value *elem = arr_val->array_val->items[i];
    const char *ep = expr_start;
    int matched = 0;
    g_cdd_last_alloc_err = cdd_json_eval_filter_expr(root, elem, &ep, &matched);
    if (matched) {
      *out_matched = elem;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/**
 * @brief Gets the number of immediate children of `node` (0 for primitives).
 *
 * @param[in] node JSON_Value node.
 * @param[out] out_cnt Number of children.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_node_child_count(const JSON_Value *node,
                                               size_t *out_cnt) {
  if (node->type == JSONObject)
    *out_cnt = node->object_val->count;
  else if (node->type == JSONArray)
    *out_cnt = node->array_val->count;
  else
    *out_cnt = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Gets the i-th immediate child of `node` (JSONObject or JSONArray).
 *
 * @param[in] node JSON_Value container.
 * @param[in] i Child index.
 * @param[out] out_child Child JSON_Value pointer.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_node_child_at(const JSON_Value *node, size_t i,
                                            const JSON_Value **out_child) {
  *out_child = (node->type == JSONObject) ? node->object_val->values[i]
                                          : node->array_val->items[i];
  return CDD_C_SUCCESS;
}

/**
 * @brief Recursively searches a JSON_Value tree for the first node matching a
 * key name (or first child if `key` is `"*"`).
 *
 * @param[in] node Current node.
 * @param[in] key Key name or `"*"` for wildcard.
 * @param[out] out_found Output pointer receiving the first matched node.
 * @return CDD_C_SUCCESS if found, or CDD_C_ERROR_NOT_FOUND.
 */
static cdd_c_error_t
cdd_json_find_recursive_first(const JSON_Value *node, const char *key,
                              const JSON_Value **out_found) {
  size_t i, cnt = 0;
  g_cdd_last_alloc_err = cdd_json_node_child_count(node, &cnt);
  if (strcmp(key, "*") == 0) {
    if (cnt > 0)
      return cdd_json_node_child_at(node, 0, out_found);
    return CDD_C_ERROR_NOT_FOUND;
  }
  if (node->type == JSONObject) {
    JSON_Value *child = NULL;
    if (cdd_json_object_get_value(node->object_val, key, &child) ==
        CDD_C_SUCCESS) {
      *out_found = child;
      return CDD_C_SUCCESS;
    }
  }
  for (i = 0; i < cnt; ++i) {
    const JSON_Value *ch = NULL;
    g_cdd_last_alloc_err = cdd_json_node_child_at(node, i, &ch);
    if (cdd_json_find_recursive_first(ch, key, out_found) == CDD_C_SUCCESS)
      return CDD_C_SUCCESS;
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/**
 * @brief Appends a deep copy of `node` into `arr`.
 *
 * @param[in,out] arr Target JSON_Array.
 * @param[in] node Node to copy and append.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_nodelist_append(JSON_Array *arr,
                                              const JSON_Value *node) {
  JSON_Value *copy = NULL;
  cdd_c_error_t rc = cdd_json_value_deep_copy(node, &copy);
  return (rc == CDD_C_SUCCESS) ? cdd_json_array_append_owned(arr, copy) : rc;
}

/**
 * @brief Appends a deep copy of `node` into `arr`, updating sticky `*io_rc`.
 *
 * @param[in,out] arr Target JSON_Array.
 * @param[in] node Node to copy and append.
 * @param[in,out] io_rc Sticky error status.
 * @return Sticky error status.
 */
static cdd_c_error_t cdd_json_nodelist_append_sticky(JSON_Array *arr,
                                                     const JSON_Value *node,
                                                     cdd_c_error_t *io_rc) {
  if (*io_rc == CDD_C_SUCCESS)
    *io_rc = cdd_json_nodelist_append(arr, node);
  return *io_rc;
}

/**
 * @brief Appends all immediate children of `node` (if JSONObject or JSONArray)
 * to `out_arr`, setting `*io_rc` on failure.
 *
 * @param[in] node Node whose children should be appended.
 * @param[in,out] out_arr Destination array.
 * @param[in,out] io_rc Sticky error status.
 * @return Sticky error status.
 */
static cdd_c_error_t cdd_json_nodelist_append_children(const JSON_Value *node,
                                                       JSON_Array *out_arr,
                                                       cdd_c_error_t *io_rc) {
  size_t i, cnt = 0;
  g_cdd_last_alloc_err = cdd_json_node_child_count(node, &cnt);
  for (i = 0; i < cnt; ++i) {
    const JSON_Value *ch = NULL;
    g_cdd_last_alloc_err = cdd_json_node_child_at(node, i, &ch);
    g_cdd_last_alloc_err = cdd_json_nodelist_append_sticky(out_arr, ch, io_rc);
  }
  return *io_rc;
}

/**
 * @brief Recursively collects all descendants matching `key[0..key_len-1]` (or
 * all children if `key_len == 1 && key[0] == '*'`) into `out_arr`.
 *
 * @param[in] node Current node.
 * @param[in] key Key bytes or `"*"`.
 * @param[in] key_len Byte length of `key`.
 * @param[in,out] out_arr Output JSON_Array receiving deep copies.
 * @param[in,out] io_rc Sticky error status.
 * @return Sticky error status.
 */
static cdd_c_error_t cdd_json_collect_recursive(const JSON_Value *node,
                                                const char *key, size_t key_len,
                                                JSON_Array *out_arr,
                                                cdd_c_error_t *io_rc) {
  size_t i, cnt = 0;
  if (!key) {
    g_cdd_last_alloc_err =
        cdd_json_nodelist_append_children(node, out_arr, io_rc);
  } else if (node->type == JSONObject) {
    JSON_Value *child = NULL;
    if (cdd_json_object_getn_value(node->object_val, key, key_len, &child) ==
        CDD_C_SUCCESS) {
      g_cdd_last_alloc_err =
          cdd_json_nodelist_append_sticky(out_arr, child, io_rc);
    }
  }
  g_cdd_last_alloc_err = cdd_json_node_child_count(node, &cnt);
  for (i = 0; i < cnt; ++i) {
    const JSON_Value *ch = NULL;
    g_cdd_last_alloc_err = cdd_json_node_child_at(node, i, &ch);
    g_cdd_last_alloc_err =
        cdd_json_collect_recursive(ch, key, key_len, out_arr, io_rc);
  }
  return *io_rc;
}

/**
 * @brief Parses `[start]:end:step` after the first `:` in an RFC 9535 slice
 * selector.
 *
 * @param[in,out] cursor Pointer to character immediately following the first
 * `:`.
 * @param[out] out_has_end 1 if `end` was specified, 0 otherwise.
 * @param[out] out_end Parsed `end` index.
 * @param[out] out_step Parsed `step` (> 0).
 * @return CDD_C_SUCCESS or CDD_C_ERROR_INVALID_ARGUMENT.
 */
static cdd_c_error_t cdd_json_parse_slice_rest(const char **cursor,
                                               int *out_has_end, long *out_end,
                                               long *out_step) {
  const char *p = *cursor;
  char *endp = NULL;
  *out_has_end = 0;
  *out_end = 0;
  *out_step = 1;
  if (!strchr("]:,", *p)) {
    *out_end = strtol(p, &endp, 10);
    if (endp == p)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    *out_has_end = 1;
    p = endp;
  }
  if (*p == ':') {
    ++p;
    if (!strchr("],", *p)) {
      *out_step = strtol(p, &endp, 10);
      if (endp == p || *out_step <= 0)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      p = endp;
    }
  }
  *cursor = p;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_jsonpath_get(const JSON_Value *root,
                                    const char *jsonpath,
                                    const JSON_Value **out_val) {
  const char *p;
  const JSON_Value *cur;

  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_val = NULL;
  if (!root || !jsonpath || jsonpath[0] != '$')
    return CDD_C_ERROR_INVALID_ARGUMENT;

  p = jsonpath + 1;
  cur = root;

  while (*p != '\0') {
    if (*p == '.') {
      const char *start;
      size_t len = 0;
      char *key = NULL;
      JSON_Value *child = NULL;
      int is_rec = 0;

      ++p;
      if (*p == '.') {
        is_rec = 1;
        ++p;
      }
      if (*p == '*') {
        ++p;
        if (is_rec) {
          const JSON_Value *found = NULL;
          if (cdd_json_find_recursive_first(cur, "*", &found) != CDD_C_SUCCESS)
            return CDD_C_ERROR_NOT_FOUND;
          cur = found;
          continue;
        }
        if (cur->type == JSONObject) {
          cur =
              (cur->object_val->count > 0) ? cur->object_val->values[0] : NULL;
        } else if (cur->type == JSONArray) {
          cur = (cur->array_val->count > 0) ? cur->array_val->items[0] : NULL;
        } else {
          cur = NULL;
        }
        if (!cur)
          return CDD_C_ERROR_NOT_FOUND;
        continue;
      }

      start = p;
      while (*p != '\0' && *p != '.' && *p != '[') {
        ++p;
        ++len;
      }
      if (len == 0)
        return CDD_C_ERROR_INVALID_ARGUMENT;

      key = (char *)cdd_json_ctx_alloc(len + 1);
      if (!key)
        return CDD_C_ERROR_MEMORY;
      memcpy(key, start, len);
      key[len] = '\0';
      if (is_rec) {
        const JSON_Value *found = NULL;
        if (cdd_json_find_recursive_first(cur, key, &found) != CDD_C_SUCCESS) {
          g_cdd_last_alloc_err = cdd_json_pointer_free_token(key);
          return CDD_C_ERROR_NOT_FOUND;
        }
        g_cdd_last_alloc_err = cdd_json_pointer_free_token(key);
        cur = found;
        continue;
      }
      if (cur->type != JSONObject) {
        g_cdd_last_alloc_err = cdd_json_pointer_free_token(key);
        return CDD_C_ERROR_NOT_FOUND;
      }
      if (cdd_json_object_get_value(cur->object_val, key, &child) !=
          CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_pointer_free_token(key);
        return CDD_C_ERROR_NOT_FOUND;
      }
      g_cdd_last_alloc_err = cdd_json_pointer_free_token(key);
      cur = child;
    } else if (*p == '[') {
      ++p;
      if (*p == '?') {
        const JSON_Value *matched = NULL;
        cdd_c_error_t rc;
        ++p;
        rc = cdd_json_eval_filter_selector(root, cur, &p, &matched);
        if (rc != CDD_C_SUCCESS)
          return rc;
        cur = matched;
      } else if (strncmp(p, "*]", 2) == 0) {
        p += 2;
        if (cur->type == JSONArray) {
          cur = (cur->array_val->count > 0) ? cur->array_val->items[0] : NULL;
        } else if (cur->type == JSONObject) {
          cur =
              (cur->object_val->count > 0) ? cur->object_val->values[0] : NULL;
        } else {
          cur = NULL;
        }
        if (!cur)
          return CDD_C_ERROR_NOT_FOUND;
      } else if (*p == 39 || *p == '"') {
        size_t klen = 0;
        char *key = NULL;
        JSON_Value *child = NULL;
        cdd_c_error_t rc = cdd_json_parse_jp_quoted(&p, &key, &klen);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (*p != ']') {
          g_cdd_last_alloc_err = cdd_json_pointer_free_token(key);
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
        ++p;
        if (cur->type != JSONObject) {
          g_cdd_last_alloc_err = cdd_json_pointer_free_token(key);
          return CDD_C_ERROR_NOT_FOUND;
        }
        if (cdd_json_object_getn_value(cur->object_val, key, klen, &child) !=
            CDD_C_SUCCESS) {
          g_cdd_last_alloc_err = cdd_json_pointer_free_token(key);
          return CDD_C_ERROR_NOT_FOUND;
        }
        g_cdd_last_alloc_err = cdd_json_pointer_free_token(key);
        cur = child;
      } else {
        char *endp = NULL;
        int has_start = 0;
        long start_idx = 0;
        if (*p != ':') {
          start_idx = strtol(p, &endp, 10);
          if (endp == p)
            return CDD_C_ERROR_INVALID_ARGUMENT;
          has_start = 1;
          p = endp;
        }
        if (*p == ']') {
          size_t arr_cnt;
          size_t eff_idx;
          if (cur->type != JSONArray)
            return CDD_C_ERROR_NOT_FOUND;
          arr_cnt = cur->array_val->count;
          if (start_idx < 0) {
            long pos_idx = (long)arr_cnt + start_idx;
            if (pos_idx < 0)
              return CDD_C_ERROR_NOT_FOUND;
            eff_idx = (size_t)pos_idx;
          } else {
            eff_idx = (size_t)start_idx;
          }
          if (eff_idx >= arr_cnt)
            return CDD_C_ERROR_NOT_FOUND;
          cur = cur->array_val->items[eff_idx];
          ++p;
        } else if (*p == ':') {
          int has_end = 0;
          long end_idx = 0;
          long step = 1;
          long arr_cnt;
          long s, e;
          ++p;
          if (cdd_json_parse_slice_rest(&p, &has_end, &end_idx, &step) !=
                  CDD_C_SUCCESS ||
              *p != ']') {
            return CDD_C_ERROR_INVALID_ARGUMENT;
          }
          ++p;
          if (cur->type != JSONArray)
            return CDD_C_ERROR_NOT_FOUND;
          arr_cnt = (long)cur->array_val->count;
          s = has_start ? (start_idx < 0 ? arr_cnt + start_idx : start_idx) : 0;
          e = has_end ? (end_idx < 0 ? arr_cnt + end_idx : end_idx) : arr_cnt;
          if (s < 0)
            s = 0;
          if (e > arr_cnt)
            e = arr_cnt;
          if (s >= e)
            return CDD_C_ERROR_NOT_FOUND;
          cur = cur->array_val->items[(size_t)s];
        } else {
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
      }
    } else {
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }

  *out_val = cur;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_jsonpath_query(const JSON_Value *root,
                                      const char *jsonpath,
                                      JSON_Value **out_array) {
  const char *p;
  JSON_Value *cur_list = NULL;
  cdd_c_error_t rc;

  if (!out_array)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_array = NULL;
  if (!root || !jsonpath || jsonpath[0] != '$')
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_json_value_init_array(&cur_list);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_nodelist_append(cur_list->array_val, root);
  if (rc != CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = cdd_json_value_free(cur_list);
    return rc;
  }

  p = jsonpath + 1;
  while (*p != '\0') {
    JSON_Array next_arr;
    size_t ni;
    memset(&next_arr, 0, sizeof(next_arr));
    next_arr.wrapping_value = cur_list;

    if (*p == '.') {
      int is_rec = 0;
      const char *start;
      size_t len = 0;
      ++p;
      if (*p == '.') {
        is_rec = 1;
        ++p;
      }
      if (*p == '*') {
        ++p;
        for (ni = 0; ni < cur_list->array_val->count; ++ni) {
          const JSON_Value *node = cur_list->array_val->items[ni];
          if (is_rec)
            g_cdd_last_alloc_err =
                cdd_json_collect_recursive(node, NULL, 0, &next_arr, &rc);
          else
            g_cdd_last_alloc_err =
                cdd_json_nodelist_append_children(node, &next_arr, &rc);
        }
      } else {
        start = p;
        while (*p != '\0' && *p != '.' && *p != '[') {
          ++p;
          ++len;
        }
        if (len == 0) {
          rc = CDD_C_ERROR_INVALID_ARGUMENT;
        } else {
          for (ni = 0; ni < cur_list->array_val->count; ++ni) {
            const JSON_Value *node = cur_list->array_val->items[ni];
            if (is_rec) {
              g_cdd_last_alloc_err =
                  cdd_json_collect_recursive(node, start, len, &next_arr, &rc);
            } else {
              const JSON_Value *child = NULL;
              g_cdd_last_alloc_err =
                  cdd_json_filter_lookup_prop(node, start, len, &child);
              if (child)
                g_cdd_last_alloc_err =
                    cdd_json_nodelist_append_sticky(&next_arr, child, &rc);
            }
          }
        }
      }
    } else if (*p == '[') {
      ++p;
      if (*p == '?') {
        const char *expr_start;
        int dummy = 0;
        ++p;
        expr_start = p;
        rc = cdd_json_eval_filter_expr(root, NULL, &p, &dummy);
        if (rc == CDD_C_SUCCESS) {
          g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
          if (*p != ']') {
            rc = CDD_C_ERROR_NOT_SUPPORTED;
          } else {
            ++p;
            for (ni = 0; ni < cur_list->array_val->count; ++ni) {
              const JSON_Value *node = cur_list->array_val->items[ni];
              if (node->type == JSONArray) {
                size_t j;
                for (j = 0; j < node->array_val->count; ++j) {
                  const JSON_Value *elem = node->array_val->items[j];
                  const char *ep = expr_start;
                  int matched = 0;
                  g_cdd_last_alloc_err =
                      cdd_json_eval_filter_expr(root, elem, &ep, &matched);
                  if (matched)
                    g_cdd_last_alloc_err =
                        cdd_json_nodelist_append_sticky(&next_arr, elem, &rc);
                }
              }
            }
          }
        }
      } else if (strncmp(p, "*]", 2) == 0) {
        p += 2;
        for (ni = 0; ni < cur_list->array_val->count; ++ni) {
          g_cdd_last_alloc_err = cdd_json_nodelist_append_children(
              cur_list->array_val->items[ni], &next_arr, &rc);
        }
      } else {
        /* Union of quoted keys or integer indices/slices `[a, b, ...]` */
        while (rc == CDD_C_SUCCESS && *p != ']') {
          g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
          if (*p == 39 || *p == '"') {
            size_t klen = 0;
            char *key = NULL;
            rc = cdd_json_parse_jp_quoted(&p, &key, &klen);
            if (rc == CDD_C_SUCCESS) {
              for (ni = 0; ni < cur_list->array_val->count; ++ni) {
                const JSON_Value *child = NULL;
                g_cdd_last_alloc_err = cdd_json_filter_lookup_prop(
                    cur_list->array_val->items[ni], key, klen, &child);
                if (child)
                  g_cdd_last_alloc_err =
                      cdd_json_nodelist_append_sticky(&next_arr, child, &rc);
              }
              g_cdd_last_alloc_err = cdd_json_pointer_free_token(key);
            }
          } else {
            char *endp = NULL;
            long idx_val = 0;
            if (*p != ':') {
              idx_val = strtol(p, &endp, 10);
              if (endp == p)
                rc = CDD_C_ERROR_INVALID_ARGUMENT;
              else
                p = endp;
            }
            if (rc == CDD_C_SUCCESS && *p == ':') {
              int has_end = 0;
              long end_idx = 0;
              long step = 1;
              ++p;
              rc = cdd_json_parse_slice_rest(&p, &has_end, &end_idx, &step);
              if (rc == CDD_C_SUCCESS) {
                for (ni = 0; ni < cur_list->array_val->count; ++ni) {
                  const JSON_Value *node = cur_list->array_val->items[ni];
                  if (node->type == JSONArray) {
                    long cnt = (long)node->array_val->count;
                    long s = idx_val < 0 ? cnt + idx_val : idx_val;
                    long e =
                        has_end ? (end_idx < 0 ? cnt + end_idx : end_idx) : cnt;
                    long k;
                    if (s < 0)
                      s = 0;
                    if (e > cnt)
                      e = cnt;
                    for (k = s; k < e; k += step) {
                      g_cdd_last_alloc_err = cdd_json_nodelist_append_sticky(
                          &next_arr, node->array_val->items[(size_t)k], &rc);
                    }
                  }
                }
              }
            } else if (rc == CDD_C_SUCCESS) {
              for (ni = 0; ni < cur_list->array_val->count; ++ni) {
                const JSON_Value *node = cur_list->array_val->items[ni];
                if (node->type == JSONArray) {
                  long cnt = (long)node->array_val->count;
                  long eff = idx_val < 0 ? cnt + idx_val : idx_val;
                  if (eff >= 0 && eff < cnt) {
                    g_cdd_last_alloc_err = cdd_json_nodelist_append_sticky(
                        &next_arr, node->array_val->items[(size_t)eff], &rc);
                  }
                }
              }
            }
          }
          if (rc == CDD_C_SUCCESS) {
            g_cdd_last_alloc_err = cdd_json_filter_skip_ws(&p);
            if (*p == ',')
              ++p;
            else if (*p != ']')
              rc = CDD_C_ERROR_INVALID_ARGUMENT;
          }
        }
        if (rc == CDD_C_SUCCESS)
          ++p;
      }
    } else {
      rc = CDD_C_ERROR_INVALID_ARGUMENT;
    }

    g_cdd_last_alloc_err = cdd_json_array_clear(cur_list->array_val);
    g_cdd_last_alloc_err = cdd_json_dealloc((void *)cur_list->array_val->items);
    *cur_list->array_val = next_arr;
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(cur_list);
      return rc;
    }
  }

  *out_array = cur_list;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_merge_patch(const JSON_Value *target,
                                   const JSON_Value *patch, JSON_Value **out) {
  JSON_Value *result = NULL;
  size_t i;
  cdd_c_error_t rc;
  if (!out || !patch)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (patch->type != JSONObject) {
    return cdd_json_value_deep_copy(patch, out);
  }
  if (target && target->type == JSONObject) {
    rc = cdd_json_value_deep_copy(target, &result);
  } else {
    rc = cdd_json_value_init_object(&result);
  }
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < patch->object_val->count; ++i) {
    const char *key = patch->object_val->names[i];
    const JSON_Value *pval = patch->object_val->values[i];
    if (pval->type == JSONNull) {
      g_cdd_last_alloc_err = cdd_json_object_remove(result->object_val, key);
    } else {
      JSON_Value *orig_child = NULL;
      JSON_Value *merged_child = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_value(result->object_val, key, &orig_child);
      g_cdd_last_alloc_err =
          cdd_json_merge_patch(orig_child, pval, &merged_child);
      rc = cdd_json_object_set_owned(result->object_val, key, merged_child);
      if (rc != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_value_free(result);
        return rc;
      }
    }
  }
  *out = result;
  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves the parent container and final unescaped token of a JSON
 * Pointer for RFC 6902 mutation operations.
 *
 * @param[in] root Root JSON_Value.
 * @param[in] path JSON Pointer string (must start with `/`).
 * @param[out] out_parent Resolved parent container.
 * @param[out] out_token Newly allocated unescaped leaf token.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t cdd_json_patch_resolve_parent(JSON_Value *root,
                                                   const char *path,
                                                   JSON_Value **out_parent,
                                                   char **out_token) {
  const char *last_slash;
  cdd_c_error_t rc;
  *out_parent = NULL;
  *out_token = NULL;
  last_slash = strrchr(path, '/');
  if (last_slash == path) {
    *out_parent = root;
  } else {
    char *parent_ptr = NULL;
    const JSON_Value *cparent = NULL;
    rc = cdd_json_strndup(path, (size_t)(last_slash - path), &parent_ptr);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_pointer_get(root, parent_ptr, &cparent);
    g_cdd_last_alloc_err = cdd_json_dealloc(parent_ptr);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_parent = (JSON_Value *)(size_t)cparent;
  }
  return cdd_json_pointer_unescape_strict(last_slash + 1,
                                          strlen(last_slash + 1), out_token);
}

/**
 * @brief Inserts or replaces `val` at `pointer` in `root`.
 *
 * @param[in,out] root Root JSON_Value.
 * @param[in] pointer JSON Pointer string.
 * @param[in] val Value to insert.
 * @param[in] replace_existing Non-zero to replace existing array element, 0 to
 * insert before.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t cdd_json_pointer_set_internal(JSON_Value *root,
                                                   const char *pointer,
                                                   JSON_Value *val,
                                                   int replace_existing) {
  const char *p;
  JSON_Value *parent = NULL;
  char *token = NULL;
  cdd_c_error_t rc;
  if (!root || !pointer || !val || val->parent != NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  p = pointer;
  if (*p == '#')
    p++;
  if (*p != '/')
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_patch_resolve_parent(root, p, &parent, &token);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (parent->type == JSONObject) {
    rc = cdd_json_object_set_value(parent->object_val, token, val);
  } else if (parent->type == JSONArray) {
    JSON_Array *arr = parent->array_val;
    char *endp = NULL;
    unsigned long idx;
    if (strcmp(token, "-") == 0) {
      rc = cdd_json_array_append_value(arr, val);
    } else if (token[0] == '\0' || (token[0] == '0' && token[1] != '\0')) {
      rc = CDD_C_ERROR_INVALID_ARGUMENT;
    } else {
      idx = strtoul(token, &endp, 10);
      if (*endp != '\0' || idx > arr->count)
        rc = CDD_C_ERROR_INVALID_ARGUMENT;
      else if (replace_existing && idx < arr->count)
        rc = cdd_json_array_replace_value(arr, (size_t)idx, val);
      else
        rc = cdd_json_array_insert_value(arr, (size_t)idx, val);
    }
  } else {
    rc = CDD_C_ERROR_INVALID_ARGUMENT;
  }
  g_cdd_last_alloc_err = cdd_json_pointer_free_token(token);
  return rc;
}

cdd_c_error_t cdd_json_pointer_set(JSON_Value *root, const char *pointer,
                                   JSON_Value *val) {
  return cdd_json_pointer_set_internal(root, pointer, val, 1);
}

/**
 * @brief Deep-copies `v_src`, optionally removes `remove_from`, and applies an
 * RFC 6902 `add` operation at `path` (updating `*root_ptr` on empty path, or
 * freeing the copy on error).
 *
 * @param[in,out] root_ptr Pointer to root JSON_Value.
 * @param[in] path Target JSON Pointer.
 * @param[in] v_src Source value to clone and insert.
 * @param[in] remove_from Optional JSON Pointer to remove after cloning `v_src`
 * (or NULL).
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_patch_add_path(JSON_Value **root_ptr,
                                             const char *path,
                                             const JSON_Value *v_src,
                                             const char *remove_from) {
  JSON_Value *v_copy = NULL;
  cdd_c_error_t rc = cdd_json_value_deep_copy(v_src, &v_copy);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (remove_from)
    g_cdd_last_alloc_err = cdd_json_pointer_remove(*root_ptr, remove_from);
  if (path[0] == '\0') {
    g_cdd_last_alloc_err = cdd_json_value_free(*root_ptr);
    *root_ptr = v_copy;
    return CDD_C_SUCCESS;
  }
  rc = cdd_json_pointer_set_internal(*root_ptr, path, v_copy, 0);
  if (rc != CDD_C_SUCCESS)
    g_cdd_last_alloc_err = cdd_json_value_free(v_copy);
  return rc;
}

cdd_c_error_t cdd_json_pointer_remove(JSON_Value *root, const char *pointer) {
  const char *p;
  JSON_Value *parent = NULL;
  char *token = NULL;
  cdd_c_error_t rc;
  if (!root || !pointer)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  p = pointer;
  if (*p == '#')
    p++;
  if (*p != '/')
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_patch_resolve_parent(root, p, &parent, &token);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (parent->type == JSONObject) {
    rc = cdd_json_object_remove(parent->object_val, token);
  } else if (parent->type == JSONArray) {
    char *endp = NULL;
    unsigned long idx;
    if (token[0] == '\0' || (token[0] == '0' && token[1] != '\0')) {
      rc = CDD_C_ERROR_INVALID_ARGUMENT;
    } else {
      idx = strtoul(token, &endp, 10);
      if (*endp != '\0' || idx >= parent->array_val->count)
        rc = CDD_C_ERROR_NOT_FOUND;
      else
        rc = cdd_json_array_remove(parent->array_val, (size_t)idx);
    }
  } else {
    rc = CDD_C_ERROR_INVALID_ARGUMENT;
  }
  g_cdd_last_alloc_err = cdd_json_pointer_free_token(token);
  return rc;
}

cdd_c_error_t cdd_json_apply_patch(const JSON_Value *doc,
                                   const JSON_Value *patch_arr,
                                   JSON_Value **out) {
  JSON_Value *cur = NULL;
  size_t i;
  cdd_c_error_t rc;
  if (!doc || !patch_arr || !out || patch_arr->type != JSONArray)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  rc = cdd_json_value_deep_copy(doc, &cur);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; rc == CDD_C_SUCCESS && i < patch_arr->array_val->count; ++i) {
    const JSON_Value *op_val = patch_arr->array_val->items[i];
    const char *op = NULL;
    const char *path = NULL;
    if (op_val->type != JSONObject ||
        cdd_json_object_get_string(op_val->object_val, "op", &op) !=
            CDD_C_SUCCESS ||
        cdd_json_object_get_string(op_val->object_val, "path", &path) !=
            CDD_C_SUCCESS) {
      rc = CDD_C_ERROR_INVALID_ARGUMENT;
    } else if (strcmp(op, "test") == 0) {
      const JSON_Value *target_val = NULL;
      JSON_Value *expected_val = NULL;
      int eq = 0;
      if (cdd_json_object_get_value(op_val->object_val, "value",
                                    &expected_val) != CDD_C_SUCCESS ||
          cdd_json_pointer_get(cur, path, &target_val) != CDD_C_SUCCESS) {
        rc = CDD_C_ERROR_INVALID_ARGUMENT;
      } else {
        g_cdd_last_alloc_err =
            cdd_json_value_equals(target_val, expected_val, &eq);
        if (!eq)
          rc = CDD_C_ERROR_INVALID_ARGUMENT;
      }
    } else if (strcmp(op, "add") == 0 || strcmp(op, "replace") == 0) {
      JSON_Value *v_src = NULL;
      if (cdd_json_object_get_value(op_val->object_val, "value", &v_src) !=
          CDD_C_SUCCESS) {
        rc = CDD_C_ERROR_INVALID_ARGUMENT;
      } else {
        if (strcmp(op, "replace") == 0 && path[0] != '\0')
          rc = cdd_json_pointer_remove(cur, path);
        if (rc == CDD_C_SUCCESS)
          rc = cdd_json_patch_add_path(&cur, path, v_src, NULL);
      }
    } else if (strcmp(op, "remove") == 0) {
      rc = cdd_json_pointer_remove(cur, path);
    } else if (strcmp(op, "copy") == 0 || strcmp(op, "move") == 0) {
      const char *from_path = NULL;
      const JSON_Value *from_val = NULL;
      if (cdd_json_object_get_string(op_val->object_val, "from", &from_path) !=
              CDD_C_SUCCESS ||
          (strcmp(op, "move") == 0 && from_path[0] == '\0') ||
          cdd_json_pointer_get(cur, from_path, &from_val) != CDD_C_SUCCESS) {
        rc = CDD_C_ERROR_INVALID_ARGUMENT;
      } else {
        const char *rem =
            (strcmp(op, "move") == 0 && strcmp(from_path, path) != 0)
                ? from_path
                : NULL;
        rc = cdd_json_patch_add_path(&cur, path, from_val, rem);
      }
    } else {
      rc = CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }
  if (rc != CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = cdd_json_value_free(cur);
    return rc;
  }
  *out = cur;
  return CDD_C_SUCCESS;
}
