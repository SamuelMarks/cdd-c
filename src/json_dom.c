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
  if (val < 0.0) {
    sign = 1;
    val = -val;
  }
  hi = (uint64_t)(val / p2_64);
  lo = (uint64_t)(val - ((double)hi * p2_64));
  if (sign) {
    uint64_t neg_lo = 0u - lo;
    uint64_t borrow = (lo | neg_lo) >> 63;
    out->low = neg_lo;
    out->high = (int64_t)(0u - hi - borrow);
  } else {
    out->low = lo;
    out->high = (int64_t)hi;
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

/**
 * @brief Frees `p` via custom deallocator or `free`.
 *
 * @param[in] p Pointer to free.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_dealloc_free(void *p) {
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
static cdd_c_error_t cdd_json_ctx_dealloc_free(void *p) {
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
  char *copy = NULL;
  cdd_c_error_t rc = cdd_json_alloc_fn(n + 1, (void **)&copy);
  if (rc != CDD_C_SUCCESS)
    return rc;
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
  return (JSON_Status)((int)JSONSuccess +
                       ((int)cdd_json_set_allocation_functions(malloc_fun,
                                                               free_fun) -
                        (int)CDD_C_SUCCESS));
}

cdd_c_error_t cdd_json_value_init_object(JSON_Value **out) {
  JSON_Value *val = NULL;
  JSON_Object *obj = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  rc = cdd_json_alloc_fn(sizeof(JSON_Value), (void **)&val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_alloc_fn(sizeof(JSON_Object), (void **)&obj);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc | (int)cdd_json_dealloc_free(val));
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
  JSON_Value *val = NULL;
  JSON_Array *arr = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  rc = cdd_json_alloc_fn(sizeof(JSON_Value), (void **)&val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_alloc_fn(sizeof(JSON_Array), (void **)&arr);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc | (int)cdd_json_dealloc_free(val));
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
  JSON_Value *val = NULL;
  cdd_c_error_t rc = cdd_json_alloc_fn(sizeof(JSON_Value), (void **)&val);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc | (int)cdd_json_dealloc_free(owned_str));
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
  JSON_Value *val = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (number != number || (number * 0.0 != 0.0))
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_alloc_fn(sizeof(JSON_Value), (void **)&val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  memset(val, 0, sizeof(JSON_Value));
  val->type = JSONNumber;
  val->number_val = number;
  val->num_kind = 0;
  *out = val;
  return CDD_C_SUCCESS;
}

/**
 * @brief Formats an unsigned 64-bit integer into a heap-allocated decimal
 * string.
 *
 * @param[in] uval Unsigned 64-bit integer.
 * @param[in] is_neg Non-zero to prefix with `-`.
 * @param[out] out_str Output heap-allocated decimal string.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_strdup_u64(uint64_t uval, int is_neg,
                                         char **out_str) {
  char buf[32];
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
  return cdd_json_strndup(buf, pos, out_str);
}

cdd_c_error_t cdd_json_value_init_int64(int64_t val, JSON_Value **out) {
  JSON_Value *jv = NULL;
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
  rc = cdd_json_value_init_number((double)val, &jv);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_strdup_u64(mag, is_neg, &jv->raw_num_str);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, json_value_free(jv));
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
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  rc = cdd_json_value_init_number((double)val, &jv);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_strdup_u64(val, 0, &jv->raw_num_str);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, json_value_free(jv));
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
 * @brief Formats an unsigned 128-bit integer into a heap-allocated decimal
 * string.
 *
 * @param[in] uval Unsigned 128-bit integer.
 * @param[in] is_neg Non-zero to prefix with `-`.
 * @param[out] out_str Output heap-allocated decimal string.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_strdup_u128(cdd_uint128_t uval, int is_neg,
                                          char **out_str) {
  char buf[48];
  char rev[48];
  size_t rlen = 0;
  size_t pos = 0;
  if (uval.high == 0u && uval.low == 0u) {
    rev[rlen++] = '0';
  } else {
    while (uval.high != 0u || uval.low != 0u) {
      cdd_uint128_t q;
      uint64_t r = 0u;
      int bit_i;
      q.low = 0u;
      q.high = 0u;
      for (bit_i = 127; bit_i >= 0; --bit_i) {
        uint64_t bit;
        r <<= 1u;
        if (bit_i >= 64) {
          bit = (uval.high >> (unsigned int)(bit_i - 64)) & 1u;
        } else {
          bit = (uval.low >> (unsigned int)bit_i) & 1u;
        }
        r |= bit;
        if (r >= 10u) {
          r -= 10u;
          if (bit_i >= 64) {
            q.high |= (((uint64_t)1u) << (unsigned int)(bit_i - 64));
          } else {
            q.low |= (((uint64_t)1u) << (unsigned int)bit_i);
          }
        }
      }
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
  return cdd_json_strndup(buf, pos, out_str);
}

cdd_c_error_t cdd_json_value_init_int128(cdd_int128_t val, JSON_Value **out) {
  JSON_Value *jv = NULL;
  cdd_uint128_t mag;
  double p2_32 = 4294967296.0;
  double p2_64 = p2_32 * p2_32;
  double dval;
  int is_neg = (val.high < 0);
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (is_neg) {
    uint64_t neg_lo = 0u - val.low;
    uint64_t borrow = (val.low | neg_lo) >> 63;
    mag.low = neg_lo;
    mag.high = 0u - (uint64_t)val.high - borrow;
  } else {
    mag.low = val.low;
    mag.high = (uint64_t)val.high;
  }
  dval = (double)mag.high * p2_64 + (double)mag.low;
  if (is_neg)
    dval = -dval;
  rc = cdd_json_value_init_number(dval, &jv);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_strdup_u128(mag, is_neg, &jv->raw_num_str);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, json_value_free(jv));
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
  double dval;
  double p2_32 = 4294967296.0;
  double p2_64 = p2_32 * p2_32;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  dval = (double)val.high * p2_64 + (double)val.low;
  rc = cdd_json_value_init_number(dval, &jv);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_strdup_u128(val, 0, &jv->raw_num_str);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, json_value_free(jv));
  }
  jv->uint128_val = val;
  jv->int128_val.low = val.low;
  jv->int128_val.high = (int64_t)val.high;
  if (val.high == 0) {
    uint64_t max_i64 =
        (((uint64_t)0x7fffffffUL) << 32) | (uint64_t)0xffffffffUL;
    jv->uint64_val = val.low;
    jv->int64_val = (int64_t)val.low;
    jv->num_kind = (val.low <= max_i64) ? 1 : 2;
  } else {
    jv->num_kind = 4;
  }
  *out = jv;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_get_int64(const JSON_Value *val, int64_t *out) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!val || !out || val->type != JSONNumber)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (val->num_kind == 1) {
    *out = val->int64_val;
    return CDD_C_SUCCESS;
  }
  if (val->num_kind == 2) {
    uint64_t max_i64 =
        (((uint64_t)0x7fffffffUL) << 32) | (uint64_t)0xffffffffUL;
    if (val->uint64_val > max_i64)
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
  double p2_32 = 4294967296.0;
  double p2_64 = p2_32 * p2_32;
  uint64_t hi;
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
  hi = (uint64_t)(val->number_val / p2_64);
  out->high = hi;
  out->low = (uint64_t)(val->number_val - ((double)hi * p2_64));
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_init_boolean(int boolean, JSON_Value **out) {
  JSON_Value *val = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  rc = cdd_json_alloc_fn(sizeof(JSON_Value), (void **)&val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  memset(val, 0, sizeof(JSON_Value));
  val->type = JSONBoolean;
  val->boolean_val = boolean ? 1 : 0;
  *out = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_init_null(JSON_Value **out) {
  JSON_Value *val = NULL;
  cdd_c_error_t rc;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  rc = cdd_json_alloc_fn(sizeof(JSON_Value), (void **)&val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  memset(val, 0, sizeof(JSON_Value));
  val->type = JSONNull;
  *out = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_free(JSON_Value *val) {
  JSON_Value *stack;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!val)
    return CDD_C_SUCCESS;
  val->parent = NULL;
  stack = val;
  while (stack) {
    JSON_Value *cur = stack;
    stack = cur->parent;
    if (cur->type == JSONString) {
      rc = cdd_json_dealloc_free(cur->string_val);
    } else if (cur->type == JSONNumber) {
      if (cur->raw_num_str)
        rc = cdd_json_dealloc_free(cur->raw_num_str);
    } else if (cur->type == JSONObject) {
      JSON_Object *obj = cur->object_val;
      for (i = 0; i < obj->count; ++i) {
        rc = cdd_json_dealloc_free(obj->names[i]);
        obj->values[i]->parent = stack;
        stack = obj->values[i];
      }
      if (obj->names) {
        rc = cdd_json_dealloc_free((void *)obj->names);
        rc = cdd_json_dealloc_free((void *)obj->name_lens);
        rc = cdd_json_dealloc_free((void *)obj->values);
      }
      if (obj->hash_buckets)
        rc = cdd_json_dealloc_free((void *)obj->hash_buckets);
      rc = cdd_json_dealloc_free(obj);
    } else if (cur->type == JSONArray) {
      JSON_Array *arr = cur->array_val;
      for (i = 0; i < arr->count; ++i) {
        arr->items[i]->parent = stack;
        stack = arr->items[i];
      }
      if (arr->items)
        rc = cdd_json_dealloc_free((void *)arr->items);
      rc = cdd_json_dealloc_free(arr);
    }
    rc = cdd_json_dealloc_free(cur);
  }
  return rc;
}

cdd_c_error_t json_value_free(JSON_Value *val) {
  return cdd_json_value_free(val);
}

cdd_c_error_t cdd_json_free_serialized_string(char *str) {
  if (str)
    return cdd_json_dealloc_free(str);
  return CDD_C_SUCCESS;
}

cdd_c_error_t json_free_serialized_string(char *str) {
  return cdd_json_free_serialized_string(str);
}

cdd_c_error_t cdd_json_value_get_type(const JSON_Value *val,
                                      JSON_Value_Type *out_type) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!val || !out_type)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_type = val->type;
  return CDD_C_SUCCESS;
}

JSON_Value_Type json_value_get_type(const JSON_Value *val) {
  return val ? val->type : JSONError;
}

cdd_c_error_t cdd_json_value_get_boolean(const JSON_Value *val, int *out_bool) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
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
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!val || !out_num || val->type != JSONNumber)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_num = val->number_val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_get_object(const JSON_Value *val,
                                        JSON_Object **out_obj) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
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
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
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
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_str = NULL;
  if (!val || val->type != JSONString)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_str = val->string_val;
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
  cdd_c_error_t rc = CDD_C_SUCCESS;
  cdd_c_error_t rc_dom = CDD_C_SUCCESS;
  if (obj->count < 8) {
    return CDD_C_SUCCESS;
  }
  while (target_cap < obj->count * 2) {
    target_cap *= 2;
  }
  if (target_cap > obj->hash_cap) {
    size_t *new_buckets = NULL;
    rc = cdd_json_alloc_fn(target_cap * sizeof(size_t), (void **)&new_buckets);
    if (rc == CDD_C_SUCCESS) {
      if (obj->hash_buckets) {
        rc_dom = cdd_json_dealloc_free((void *)obj->hash_buckets);
      }
      obj->hash_buckets = new_buckets;
      obj->hash_cap = target_cap;
      goto fill_buckets;
    }
    if (obj->hash_buckets) {
      rc_dom = cdd_json_dealloc_free((void *)obj->hash_buckets);
      obj->hash_buckets = NULL;
    }
    obj->hash_cap = 0;
    return rc_dom;
  }
fill_buckets:
  memset(obj->hash_buckets, 0, obj->hash_cap * sizeof(size_t));
  for (i = 0; i < obj->count; ++i) {
    size_t klen = obj->name_lens[i];
    size_t h = 2166136261u;
    size_t hi;
    size_t slot;
    for (hi = 0; hi < klen; ++hi) {
      h = (h ^ (size_t)(unsigned char)obj->names[i][hi]) * 16777619u;
    }
    slot = h & (obj->hash_cap - 1);
    while (obj->hash_buckets[slot] != 0) {
      slot = (slot + 1) & (obj->hash_cap - 1);
    }
    obj->hash_buckets[slot] = i + 1;
  }
  return cdd_c_keep_err(rc, rc_dom);
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
    size_t h = 2166136261u;
    size_t hi;
    size_t slot;
    for (hi = 0; hi < name_len; ++hi) {
      h = (h ^ (size_t)(unsigned char)name[hi]) * 16777619u;
    }
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
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
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
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!obj || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_count = obj->count;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_object_get_name(const JSON_Object *obj, size_t idx,
                                       const char **out_name) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!obj || !out_name || idx >= obj->count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_name = obj->names[idx];
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_object_get_value_at(const JSON_Object *obj, size_t idx,
                                           JSON_Value **out_val) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
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
  if (!dot) {
    return cdd_json_object_get_value(obj, name, out);
  }
  rc = cdd_json_object_getn_value(obj, name, (size_t)(dot - name), &next_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (next_val->type != JSONObject)
    return CDD_C_ERROR_NOT_FOUND;
  return cdd_json_object_dotget_value(next_val->object_val, dot + 1, out);
}

cdd_c_error_t cdd_json_object_dothas_value(const JSON_Object *obj,
                                           const char *name, int *out_has) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out_has)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has = 0;
  rc = cdd_json_object_dotget_value(obj, name, &v);
  if (rc != CDD_C_SUCCESS)
    return (CDD_C_ERROR_NOT_FOUND == rc) ? CDD_C_SUCCESS : rc;
  *out_has = 1;
  return CDD_C_SUCCESS;
}

cdd_json_bool_t json_object_dothas_value(const JSON_Object *obj,
                                         const char *name) {
  int has = 0;
  return (cdd_json_object_dothas_value(obj, name, &has) == CDD_C_SUCCESS && has)
             ? CDD_JSON_TRUE
             : CDD_JSON_FALSE;
}

cdd_json_bool_t json_object_get_boolean(const JSON_Object *obj,
                                        const char *name) {
  JSON_Value *v = NULL;
  return (cdd_json_object_get_value(obj, name, &v) == CDD_C_SUCCESS)
             ? json_value_get_boolean(v)
             : CDD_JSON_ERR_BOOL;
}

cdd_c_error_t cdd_json_object_has_value(const JSON_Object *obj,
                                        const char *name, int *out_has) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!out_has)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has = 0;
  rc = cdd_json_object_get_value(obj, name, &v);
  if (rc != CDD_C_SUCCESS)
    return (CDD_C_ERROR_NOT_FOUND == rc) ? CDD_C_SUCCESS : rc;
  *out_has = 1;
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
  cdd_c_error_t rc;
  if (!out_has)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has = 0;
  rc = cdd_json_object_get_value(obj, name, &v);
  if (rc != CDD_C_SUCCESS)
    return (CDD_C_ERROR_NOT_FOUND == rc) ? CDD_C_SUCCESS : rc;
  *out_has = (v->type == type) ? 1 : 0;
  return CDD_C_SUCCESS;
}

cdd_json_bool_t json_object_has_value_of_type(const JSON_Object *obj,
                                              const char *name,
                                              JSON_Value_Type type) {
  JSON_Value *v = NULL;
  return (cdd_json_object_get_value(obj, name, &v) == CDD_C_SUCCESS &&
          v->type == type)
             ? CDD_JSON_TRUE
             : CDD_JSON_FALSE;
}

cdd_c_error_t cdd_json_array_get_count(const JSON_Array *arr,
                                       size_t *out_count) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!arr || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_count = arr->count;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_array_get_value(const JSON_Array *arr, size_t idx,
                                       JSON_Value **out_val) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
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
  char **new_names = NULL;
  size_t *new_lens = NULL;
  JSON_Value **new_vals = NULL;
  cdd_c_error_t rc;
  cdd_c_error_t rc_dom = CDD_C_SUCCESS;
  rc = cdd_json_alloc_fn(new_cap * sizeof(char *), (void **)&new_names);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_alloc_fn(new_cap * sizeof(size_t), (void **)&new_lens);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc |
                           (int)cdd_json_dealloc_free((void *)new_names));
  }
  rc = cdd_json_alloc_fn(new_cap * sizeof(JSON_Value *), (void **)&new_vals);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc |
                           (int)cdd_json_dealloc_free((void *)new_names) |
                           (int)cdd_json_dealloc_free((void *)new_lens));
  }
  if (obj->count > 0) {
    memcpy((void *)new_names, (const void *)obj->names,
           obj->count * sizeof(char *));
    memcpy((void *)new_lens, (const void *)obj->name_lens,
           obj->count * sizeof(size_t));
    memcpy((void *)new_vals, (const void *)obj->values,
           obj->count * sizeof(JSON_Value *));
    rc_dom =
        (cdd_c_error_t)((int)rc_dom |
                        (int)cdd_json_dealloc_free((void *)obj->names) |
                        (int)cdd_json_dealloc_free((void *)obj->name_lens) |
                        (int)cdd_json_dealloc_free((void *)obj->values));
  }
  obj->names = new_names;
  obj->name_lens = new_lens;
  obj->values = new_vals;
  obj->capacity = new_cap;
  return rc_dom;
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
  cdd_c_error_t rc_dom = CDD_C_SUCCESS;
  if (val->parent != NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (obj->count >= 8 && obj->hash_buckets) {
    size_t h = 2166136261u;
    size_t hi;
    size_t slot;
    for (hi = 0; hi < name_len; ++hi) {
      h = (h ^ (size_t)(unsigned char)name[hi]) * 16777619u;
    }
    slot = h & (obj->hash_cap - 1);
    while (obj->hash_buckets[slot] != 0) {
      size_t idx = obj->hash_buckets[slot] - 1;
      size_t klen = obj->name_lens[idx];
      if (klen == name_len && memcmp(obj->names[idx], name, name_len) == 0) {
        rc_dom = cdd_json_value_free(obj->values[idx]);
        val->parent = obj->wrapping_value;
        obj->values[idx] = val;
        return rc_dom;
      }
      slot = (slot + 1) & (obj->hash_cap - 1);
    }
  } else {
    for (i = 0; i < obj->count; ++i) {
      size_t klen = obj->name_lens[i];
      if (klen == name_len && memcmp(obj->names[i], name, name_len) == 0) {
        rc_dom = cdd_json_value_free(obj->values[i]);
        val->parent = obj->wrapping_value;
        obj->values[i] = val;
        return rc_dom;
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
      size_t h = 2166136261u;
      size_t hi;
      size_t slot;
      for (hi = 0; hi < name_len; ++hi) {
        h = (h ^ (size_t)(unsigned char)name[hi]) * 16777619u;
      }
      slot = h & (obj->hash_cap - 1);
      while (obj->hash_buckets[slot] != 0) {
        slot = (slot + 1) & (obj->hash_cap - 1);
      }
      obj->hash_buckets[slot] = obj->count;
    } else {
      return cdd_json_object_rebuild_hash(obj);
    }
  }
  return rc_dom;
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
  cdd_c_error_t rc = cdd_json_object_setn_value(obj, name, strlen(name), val);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(val));
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_object_set_string(JSON_Object *obj, const char *name,
                                         const char *str) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name || !str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_string(str, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_set_owned(obj, name, v);
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
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_set_owned(obj, name, v);
}

cdd_c_error_t cdd_json_object_set_int64(JSON_Object *obj, const char *name,
                                        int64_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_int64(val, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_set_owned(obj, name, v);
}

cdd_c_error_t cdd_json_object_set_uint64(JSON_Object *obj, const char *name,
                                         uint64_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_uint64(val, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_set_owned(obj, name, v);
}

cdd_c_error_t cdd_json_object_set_int128(JSON_Object *obj, const char *name,
                                         cdd_int128_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_int128(val, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_set_owned(obj, name, v);
}

cdd_c_error_t cdd_json_object_set_uint128(JSON_Object *obj, const char *name,
                                          cdd_uint128_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_uint128(val, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_set_owned(obj, name, v);
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
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_set_owned(obj, name, v);
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
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_set_owned(obj, name, v);
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
  if (!dot) {
    return cdd_json_object_set_value(obj, name, val);
  }
  prefix_len = (size_t)(dot - name);
  rc = cdd_json_object_getn_value(obj, name, prefix_len, &sub_val);
  if (rc == CDD_C_SUCCESS) {
    if (sub_val->type == JSONObject) {
      return cdd_c_keep_err(
          rc, cdd_json_object_dotset_value(sub_val->object_val, dot + 1, val));
    }
  }
  {
    JSON_Value *new_obj_val = NULL;
    rc = cdd_json_value_init_object(&new_obj_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_object_setn_value(obj, name, prefix_len, new_obj_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(new_obj_val));
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
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(val));
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_object_dotset_string(JSON_Object *obj, const char *name,
                                            const char *str) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!obj || !name || !str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_string(str, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_dotset_owned(obj, name, v);
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
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_dotset_owned(obj, name, v);
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
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_dotset_owned(obj, name, v);
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
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_dotset_owned(obj, name, v);
}

JSON_Status json_object_dotset_null(JSON_Object *obj, const char *name) {
  return (cdd_json_object_dotset_null(obj, name) == CDD_C_SUCCESS)
             ? JSONSuccess
             : JSONFailure;
}

cdd_c_error_t cdd_json_object_remove(JSON_Object *obj, const char *name) {
  size_t i, j;
  size_t name_len;
  cdd_c_error_t rc_dom = CDD_C_SUCCESS;
  cdd_c_error_t rc_val = CDD_C_SUCCESS;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  name_len = strlen(name);
  for (i = 0; i < obj->count; ++i) {
    size_t klen = obj->name_lens[i];
    if (klen == name_len && memcmp(obj->names[i], name, name_len) == 0) {
      rc_dom = cdd_json_dealloc_free(obj->names[i]);
      rc_val = cdd_json_value_free(obj->values[i]);
      for (j = i; j + 1 < obj->count; ++j) {
        obj->names[j] = obj->names[j + 1];
        obj->name_lens[j] = obj->name_lens[j + 1];
        obj->values[j] = obj->values[j + 1];
      }
      obj->count--;
      return cdd_c_keep_err(cdd_c_keep_err(rc_dom, rc_val),
                            cdd_json_object_rebuild_hash(obj));
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
  cdd_c_error_t rc;
  if (!obj || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  dot = strchr(name, 46);
  if (!dot) {
    return cdd_json_object_remove(obj, name);
  }
  rc = cdd_json_object_getn_value(obj, name, (size_t)(dot - name), &sub_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (sub_val->type != JSONObject)
    return CDD_C_ERROR_NOT_FOUND;
  return cdd_json_object_dotremove(sub_val->object_val, dot + 1);
}

JSON_Status json_object_dotremove(JSON_Object *obj, const char *name) {
  return (cdd_json_object_dotremove(obj, name) == CDD_C_SUCCESS) ? JSONSuccess
                                                                 : JSONFailure;
}

cdd_c_error_t cdd_json_object_clear(JSON_Object *obj) {
  size_t i;
  cdd_c_error_t rc_dom = CDD_C_SUCCESS;
  if (!obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < obj->count; ++i) {
    rc_dom = cdd_json_dealloc_free(obj->names[i]);
    rc_dom = cdd_json_value_free(obj->values[i]);
  }
  obj->count = 0;
  if (obj->hash_buckets) {
    memset(obj->hash_buckets, 0, obj->hash_cap * sizeof(size_t));
  }
  return rc_dom;
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
  JSON_Value **new_items = NULL;
  cdd_c_error_t rc =
      cdd_json_alloc_fn(new_cap * sizeof(JSON_Value *), (void **)&new_items);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (arr->count > 0) {
    memcpy((void *)new_items, (const void *)arr->items,
           arr->count * sizeof(JSON_Value *));
    rc = cdd_json_dealloc_free((void *)arr->items);
  }
  arr->items = new_items;
  arr->capacity = new_cap;
  return rc;
}

cdd_c_error_t cdd_json_array_append_value(JSON_Array *arr, JSON_Value *val) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
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
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(val));
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_array_append_int64(JSON_Array *arr, int64_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_int64(val, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_array_append_owned(arr, v);
}

cdd_c_error_t cdd_json_array_append_uint64(JSON_Array *arr, uint64_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_uint64(val, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_array_append_owned(arr, v);
}

cdd_c_error_t cdd_json_array_append_int128(JSON_Array *arr, cdd_int128_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_int128(val, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_array_append_owned(arr, v);
}

cdd_c_error_t cdd_json_array_append_uint128(JSON_Array *arr,
                                            cdd_uint128_t val) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_uint128(val, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_array_append_owned(arr, v);
}

cdd_c_error_t cdd_json_array_append_string(JSON_Array *arr, const char *str) {
  JSON_Value *v = NULL;
  cdd_c_error_t rc;
  if (!arr || !str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_string(str, &v);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_array_append_owned(arr, v);
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
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_array_append_owned(arr, v);
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
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_array_append_owned(arr, v);
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
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_array_append_owned(arr, v);
}

JSON_Status json_array_append_null(JSON_Array *arr) {
  return (cdd_json_array_append_null(arr) == CDD_C_SUCCESS) ? JSONSuccess
                                                            : JSONFailure;
}

cdd_c_error_t cdd_json_array_replace_value(JSON_Array *arr, size_t i,
                                           JSON_Value *val) {
  cdd_c_error_t rc_dom;
  if (!arr || i >= arr->count || !val || val->parent != NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc_dom = cdd_json_value_free(arr->items[i]);
  val->parent = arr->wrapping_value;
  arr->items[i] = val;
  return rc_dom;
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
  cdd_c_error_t rc_dom;
  if (!arr || idx >= arr->count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc_dom = cdd_json_value_free(arr->items[idx]);
  for (j = idx; j + 1 < arr->count; ++j) {
    arr->items[j] = arr->items[j + 1];
  }
  arr->count--;
  return rc_dom;
}

JSON_Status json_array_remove(JSON_Array *arr, size_t idx) {
  return (cdd_json_array_remove(arr, idx) == CDD_C_SUCCESS) ? JSONSuccess
                                                            : JSONFailure;
}

cdd_c_error_t cdd_json_array_clear(JSON_Array *arr) {
  size_t i;
  cdd_c_error_t rc_dom = CDD_C_SUCCESS;
  if (!arr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < arr->count; ++i) {
    rc_dom =
        (cdd_c_error_t)((int)rc_dom | (int)cdd_json_value_free(arr->items[i]));
  }
  arr->count = 0;
  return rc_dom;
}

JSON_Status json_array_clear(JSON_Array *arr) {
  return (cdd_json_array_clear(arr) == CDD_C_SUCCESS) ? JSONSuccess
                                                      : JSONFailure;
}

/**
 * @brief Explicit worklist frame for iterative JSON structural equality.
 */
struct cdd_json_eq_frame {
  /** @brief Left container value at this nesting level. */
  const JSON_Value *a;
  /** @brief Right container value at this nesting level. */
  const JSON_Value *b;
  /** @brief Next child index to compare. */
  size_t idx;
};

/**
 * @brief Pushes a container pair onto the iterative equality worklist.
 *
 * @param[in,out] stack_ptr Pointer to active frame array (inline or heap).
 * @param[in] inline_base Base address of inline stack buffer.
 * @param[in,out] cap_ptr Pointer to frame array capacity.
 * @param[in,out] sp_ptr Pointer to active stack depth.
 * @param[in] a Left container node.
 * @param[in] b Right container node.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_eq_push(struct cdd_json_eq_frame **stack_ptr,
                                      struct cdd_json_eq_frame *inline_base,
                                      size_t *cap_ptr, size_t *sp_ptr,
                                      const JSON_Value *a,
                                      const JSON_Value *b) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (*sp_ptr >= *cap_ptr) {
    size_t ncap = *cap_ptr * 2;
    struct cdd_json_eq_frame *nstack = NULL;
    rc = cdd_json_alloc_fn(ncap * sizeof(struct cdd_json_eq_frame),
                           (void **)&nstack);
    if (rc != CDD_C_SUCCESS)
      return rc;
    memcpy(nstack, *stack_ptr, *sp_ptr * sizeof(struct cdd_json_eq_frame));
    if (*stack_ptr != inline_base) {
      rc = cdd_json_dealloc_free(*stack_ptr);
    }
    *stack_ptr = nstack;
    *cap_ptr = ncap;
  }
  (*stack_ptr)[*sp_ptr].a = a;
  (*stack_ptr)[*sp_ptr].b = b;
  (*stack_ptr)[*sp_ptr].idx = 0;
  (*sp_ptr)++;
  return rc;
}

/**
 * @brief Compares two JSON_Value nodes shallowly without recursing into
 * children.
 *
 * @param[in] a First value.
 * @param[in] b Second value.
 * @param[out] out_eq Output 1 if shallowly equal, 0 otherwise.
 * @param[out] out_descend Output 1 if container children must be compared.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_eq_node_shallow(const JSON_Value *a,
                                              const JSON_Value *b, int *out_eq,
                                              int *out_descend) {
  *out_eq = 0;
  *out_descend = 0;
  if (a == b) {
    *out_eq = (a != NULL) ? 1 : 0;
    return CDD_C_SUCCESS;
  }
  if (!b || a->type != b->type)
    return CDD_C_SUCCESS;
  switch (a->type) {
  case JSONNull:
    *out_eq = 1;
    break;
  case JSONBoolean:
    *out_eq = ((a->boolean_val ? 1 : 0) == (b->boolean_val ? 1 : 0)) ? 1 : 0;
    break;
  case JSONNumber:
    if (a->raw_num_str && b->raw_num_str &&
        strcmp(a->raw_num_str, b->raw_num_str) == 0) {
      *out_eq = 1;
    } else {
      *out_eq = (a->number_val == b->number_val) ? 1 : 0;
    }
    break;
  case JSONString:
    if (a->string_len == b->string_len &&
        memcmp(a->string_val, b->string_val, a->string_len) == 0) {
      *out_eq = 1;
    }
    break;
  case JSONArray:
    if (a->array_val->count == b->array_val->count) {
      *out_eq = 1;
      *out_descend = (a->array_val->count > 0) ? 1 : 0;
    }
    break;
  case JSONObject:
    if (a->object_val->count == b->object_val->count) {
      *out_eq = 1;
      *out_descend = (a->object_val->count > 0) ? 1 : 0;
    }
    break;
  default:
    break;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_equals(const JSON_Value *a, const JSON_Value *b,
                                    int *out_eq) {
  struct cdd_json_eq_frame inline_frames[32];
  struct cdd_json_eq_frame *stack = inline_frames;
  size_t cap = sizeof(inline_frames) / sizeof(inline_frames[0]);
  size_t sp = 0;
  int node_eq = 0;
  int descend = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  cdd_c_error_t rc_get = CDD_C_SUCCESS;
  cdd_c_error_t rc_ign = CDD_C_SUCCESS;

  if (!out_eq)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_eq = 0;
  rc = cdd_json_eq_node_shallow(a, b, &node_eq, &descend);
  if (!node_eq || !descend) {
    *out_eq = node_eq;
    return rc;
  }
  inline_frames[0].a = a;
  inline_frames[0].b = b;
  inline_frames[0].idx = 0;
  sp = 1;

  while (sp > 0) {
    struct cdd_json_eq_frame *top = &stack[sp - 1];
    int is_arr = (top->a->type == JSONArray);
    size_t cnt = is_arr ? top->a->array_val->count : top->a->object_val->count;
    const JSON_Value *child_a = NULL;
    const JSON_Value *child_b = NULL;

    if (top->idx >= cnt) {
      sp--;
      continue;
    }
    if (sp > CDD_JSON_MAX_DEPTH) {
      rc = CDD_C_ERROR_PARSE;
      goto cleanup;
    }
    if (is_arr) {
      child_a = top->a->array_val->items[top->idx];
      child_b = top->b->array_val->items[top->idx];
    } else {
      JSON_Value *b_found = NULL;
      size_t klen = top->a->object_val->name_lens[top->idx];
      rc_get = cdd_json_object_getn_value(top->b->object_val,
                                          top->a->object_val->names[top->idx],
                                          klen, &b_found);
      if (rc_get == CDD_C_SUCCESS) {
        child_a = top->a->object_val->values[top->idx];
        child_b = b_found;
        goto compare_children;
      }
      rc_get = CDD_C_SUCCESS;
      goto cleanup;
    }
  compare_children:
    top->idx++;
    rc = cdd_json_eq_node_shallow(child_a, child_b, &node_eq, &descend);
    if (!node_eq) {
      goto cleanup;
    }
    if (descend) {
      rc = cdd_json_eq_push(&stack, inline_frames, &cap, &sp, child_a, child_b);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }
  *out_eq = 1;

cleanup:
  if (stack != inline_frames) {
    rc_ign = cdd_json_dealloc_free(stack);
  }
  return cdd_c_keep_err(rc, cdd_c_keep_err(rc_get, rc_ign));
}

cdd_json_bool_t json_value_equals(const JSON_Value *a, const JSON_Value *b) {
  int eq = 0;
  return (cdd_json_value_equals(a, b, &eq) == CDD_C_SUCCESS && eq)
             ? CDD_JSON_TRUE
             : CDD_JSON_FALSE;
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
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      }
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
 * @brief Allocates a shallow copy of a single `JSON_Value` node (initializing
 * empty `JSONArray` / `JSONObject` shells for containers).
 *
 * @param[in] val Source JSON_Value node.
 * @param[out] out Output shallow-copied JSON_Value node.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_copy_node_shallow(const JSON_Value *val,
                                                JSON_Value **out) {
  JSON_Value *copy = NULL;
  cdd_c_error_t rc;
  switch (val->type) {
  case JSONNull: {
    rc = cdd_json_value_init_null(&copy);
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  }
  case JSONBoolean: {
    rc = cdd_json_value_init_boolean(val->boolean_val, &copy);
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  }
  case JSONNumber: {
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
        return cdd_c_keep_err(rc, cdd_json_value_free(copy));
      }
    }
    break;
  }
  case JSONString: {
    rc = cdd_json_value_init_string_with_len(val->string_val, val->string_len,
                                             &copy);
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  }
  case JSONArray: {
    rc = cdd_json_value_init_array(&copy);
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  }
  case JSONObject: {
    rc = cdd_json_value_init_object(&copy);
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  }
  default:
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  copy->src_line = val->src_line;
  copy->src_col = val->src_col;
  copy->src_offset = val->src_offset;
  *out = copy;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_deep_copy(const JSON_Value *val,
                                       JSON_Value **out) {
  JSON_Value *root_copy = NULL;
  const JSON_Value *cur_src;
  JSON_Value *cur_dst;
  size_t depth = 0;
  cdd_c_error_t rc;

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_copy_node_shallow(val, &root_copy);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (val->type != JSONArray && val->type != JSONObject) {
    *out = root_copy;
    return CDD_C_SUCCESS;
  }

  cur_src = val;
  cur_dst = root_copy;
  for (;;) {
    int is_arr = (cur_src->type == JSONArray);
    size_t cnt =
        is_arr ? cur_src->array_val->count : cur_src->object_val->count;
    size_t idx =
        is_arr ? cur_dst->array_val->count : cur_dst->object_val->count;
    const JSON_Value *child_src;
    JSON_Value *child_copy = NULL;

    if (idx >= cnt) {
      if (depth == 0)
        break;
      cur_src = cur_src->parent;
      cur_dst = cur_dst->parent;
      depth--;
      continue;
    }
    if (depth + 1 > CDD_JSON_MAX_DEPTH) {
      return cdd_c_keep_err(CDD_C_ERROR_PARSE, cdd_json_value_free(root_copy));
    }
    child_src = is_arr ? cur_src->array_val->items[idx]
                       : cur_src->object_val->values[idx];
    rc = cdd_json_copy_node_shallow(child_src, &child_copy);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(root_copy));
    }
    if (is_arr) {
      rc = cdd_json_array_append_value(cur_dst->array_val, child_copy);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc,
                              cdd_c_keep_err(cdd_json_value_free(child_copy),
                                             cdd_json_value_free(root_copy)));
      }
    } else {
      rc = cdd_json_object_setn_value(
          cur_dst->object_val, cur_src->object_val->names[idx],
          cur_src->object_val->name_lens[idx], child_copy);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc,
                              cdd_c_keep_err(cdd_json_value_free(child_copy),
                                             cdd_json_value_free(root_copy)));
      }
    }
    if (child_src->type == JSONArray || child_src->type == JSONObject) {
      cur_src = child_src;
      cur_dst = child_copy;
      depth++;
    }
  }

  *out = root_copy;
  return CDD_C_SUCCESS;
}

/* --- RFC 8259 Recursive-Descent Parser --- */

/**
 * @brief Cursor state tracking source position during JSON parsing.
 */
struct cdd_json_parse_ctx {
  /** @brief Start of the JSON source buffer. */
  const char *start;
  /** @brief Current cursor position. */
  const char *p;
  /** @brief Current 1-based line number. */
  size_t line;
  /** @brief Current 1-based column number. */
  size_t col;
};

/**
 * @brief Skips ASCII whitespace in a JSON parse context.
 *
 * @param[in,out] ctx Parse context.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_skip_ws_ctx(struct cdd_json_parse_ctx *ctx) {
  while (*ctx->p == 32 || *ctx->p == 9 || *ctx->p == 10 || *ctx->p == 13) {
    if (*ctx->p == 10) {
      ctx->line++;
      ctx->col = 1;
    } else {
      ctx->col++;
    }
    ctx->p++;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Skips ASCII whitespace and records the resulting token position.
 *
 * @param[in,out] ctx Parse context.
 * @param[out] out_line Token 1-based line number.
 * @param[out] out_col Token 1-based column number.
 * @param[out] out_offset Token 0-based byte offset.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_skip_ws_tok(struct cdd_json_parse_ctx *ctx,
                                          size_t *out_line, size_t *out_col,
                                          size_t *out_offset) {
  while (*ctx->p == 32 || *ctx->p == 9 || *ctx->p == 10 || *ctx->p == 13) {
    if (*ctx->p == 10) {
      ctx->line++;
      ctx->col = 1;
    } else {
      ctx->col++;
    }
    ctx->p++;
  }
  *out_line = ctx->line;
  *out_col = ctx->col;
  *out_offset = (size_t)(ctx->p - ctx->start);
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_value_get_location(const JSON_Value *val,
                                          size_t *out_line, size_t *out_col,
                                          size_t *out_offset) {
  if (!val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (out_line)
    *out_line = val->src_line;
  if (out_col)
    *out_col = val->src_col;
  if (out_offset)
    *out_offset = val->src_offset;
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
  char *res = NULL;
  cdd_c_error_t rc;
  (*p)++;
  scan = *p;
  while (*scan && *scan != 34) {
    unsigned char b0 = (unsigned char)*scan;
    if (b0 < 0x20)
      return CDD_C_ERROR_PARSE;
    if (b0 == 92) {
      scan++;
      if (!*scan)
        return CDD_C_ERROR_PARSE;
      scan++;
    } else if (b0 < 0x80) {
      scan++;
    } else if (b0 >= 0xC2 && b0 <= 0xDF) {
      unsigned char b1 = (unsigned char)scan[1];
      if ((b1 & 0xC0) != 0x80)
        return CDD_C_ERROR_PARSE;
      scan += 2;
    } else if (b0 >= 0xE0 && b0 <= 0xEF) {
      unsigned char b1 = (unsigned char)scan[1];
      unsigned char b2 = (unsigned char)scan[2];
      if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 ||
          (b0 == 0xE0 && b1 < 0xA0) || (b0 == 0xED && b1 >= 0xA0))
        return CDD_C_ERROR_PARSE;
      scan += 3;
    } else if (b0 >= 0xF0 && b0 <= 0xF4) {
      unsigned char b1 = (unsigned char)scan[1];
      unsigned char b2 = (unsigned char)scan[2];
      unsigned char b3 = (unsigned char)scan[3];
      if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80 ||
          (b0 == 0xF0 && b1 < 0x90) || (b0 == 0xF4 && b1 >= 0x90))
        return CDD_C_ERROR_PARSE;
      scan += 4;
    } else {
      return CDD_C_ERROR_PARSE;
    }
  }
  if (*scan != 34)
    return CDD_C_ERROR_PARSE;
  max_len = (size_t)(scan - *p);
  rc = cdd_json_alloc_fn(max_len + 1, (void **)&res);
  if (rc != CDD_C_SUCCESS)
    return rc;
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
        rc = cdd_json_parse_hex4(*p, &cp);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc, cdd_json_dealloc_free(res));
        }
        *p += 4;
        if (cp >= 0xD800 && cp <= 0xDBFF) {
          unsigned int low = 0;
          if ((*p)[0] != 92 || (*p)[1] != 117) {
            return cdd_c_keep_err(CDD_C_ERROR_PARSE,
                                  cdd_json_dealloc_free(res));
          }
          rc = cdd_json_parse_hex4(*p + 2, &low);
          if (rc != CDD_C_SUCCESS) {
            return cdd_c_keep_err(rc, cdd_json_dealloc_free(res));
          }
          if (low < 0xDC00 || low > 0xDFFF) {
            return cdd_c_keep_err(CDD_C_ERROR_PARSE,
                                  cdd_json_dealloc_free(res));
          }
          cp = 0x10000 + (((cp - 0xD800) << 10) | (low - 0xDC00));
          *p += 6;
        } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
          return cdd_c_keep_err(CDD_C_ERROR_PARSE, cdd_json_dealloc_free(res));
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
        return cdd_c_keep_err(CDD_C_ERROR_PARSE, cdd_json_dealloc_free(res));
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
cdd_json_parse_value(struct cdd_json_parse_ctx *ctx, size_t depth,
                     const struct cdd_json_parse_options *opts,
                     JSON_Value **out);

/**
 * @brief Parses a JSON object starting at "{".
 *
 * @param[in,out] ctx Parse context.
 * @param[in] depth Current nesting depth.
 * @param[in] opts Optional parse options.
 * @param[out] out Parsed JSON_Value.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t
cdd_json_parse_object(struct cdd_json_parse_ctx *ctx, size_t depth,
                      const struct cdd_json_parse_options *opts,
                      JSON_Value **out) {
  JSON_Value *val = NULL;
  cdd_c_error_t rc;
  cdd_c_error_t rc_dom;
  ctx->p++;
  ctx->col++;
  rc = cdd_json_value_init_object(&val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc_dom = cdd_json_skip_ws_ctx(ctx);
  if (*ctx->p == 125) {
    ctx->p++;
    ctx->col++;
    *out = val;
    return rc_dom;
  }
  for (;;) {
    char *key = NULL;
    size_t key_len = 0;
    JSON_Value *child = NULL;
    const char *p_cur;
    cdd_c_error_t rc_free;
    rc_dom = cdd_json_skip_ws_ctx(ctx);
    if (*ctx->p == 125) {
      if (opts && !opts->allow_trailing_commas) {
        return cdd_c_keep_err(CDD_C_ERROR_PARSE,
                              cdd_c_keep_err(rc_dom, cdd_json_value_free(val)));
      }
      ctx->p++;
      ctx->col++;
      *out = val;
      return rc_dom;
    }
    if (*ctx->p != 34) {
      return cdd_c_keep_err(CDD_C_ERROR_PARSE,
                            cdd_c_keep_err(rc_dom, cdd_json_value_free(val)));
    }
    p_cur = ctx->p;
    rc = cdd_json_parse_raw_string_len(&p_cur, &key, &key_len);
    if (rc != CDD_C_SUCCESS) {
      ctx->col += (size_t)(p_cur - ctx->p);
      ctx->p = p_cur;
      return cdd_c_keep_err(rc, cdd_json_value_free(val));
    }
    ctx->col += (size_t)(p_cur - ctx->p);
    ctx->p = p_cur;
    if (opts && opts->reject_duplicate_keys) {
      JSON_Value *existing = NULL;
      cdd_c_error_t rc_dup =
          cdd_json_object_getn_value(val->object_val, key, key_len, &existing);
      if (rc_dup == CDD_C_SUCCESS) {
        return cdd_c_keep_err(
            CDD_C_ERROR_PARSE,
            cdd_c_keep_err(rc_dup, cdd_c_keep_err(cdd_json_dealloc_free(key),
                                                  cdd_json_value_free(val))));
      }
    }
    rc_dom = cdd_json_skip_ws_ctx(ctx);
    if (*ctx->p != 58) {
      return cdd_c_keep_err(
          CDD_C_ERROR_PARSE,
          cdd_c_keep_err(rc_dom, cdd_c_keep_err(cdd_json_dealloc_free(key),
                                                cdd_json_value_free(val))));
    }
    ctx->p++;
    ctx->col++;
    rc = cdd_json_parse_value(ctx, depth + 1, opts, &child);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_dealloc_free(key),
                                               cdd_json_value_free(val)));
    }
    rc = cdd_json_object_setn_value(val->object_val, key, key_len, child);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(
          rc, cdd_c_keep_err(cdd_json_dealloc_free(key),
                             cdd_c_keep_err(cdd_json_value_free(child),
                                            cdd_json_value_free(val))));
    }
    rc_free = cdd_json_dealloc_free(key);
    rc_dom = cdd_json_skip_ws_ctx(ctx);
    if (*ctx->p == 125) {
      ctx->p++;
      ctx->col++;
      *out = val;
      return cdd_c_keep_err(rc_dom, rc_free);
    } else if (*ctx->p == 44) {
      ctx->p++;
      ctx->col++;
    } else {
      return cdd_c_keep_err(
          CDD_C_ERROR_PARSE,
          cdd_c_keep_err(rc_dom,
                         cdd_c_keep_err(rc_free, cdd_json_value_free(val))));
    }
  }
}

/**
 * @brief Parses a JSON array starting at "[".
 *
 * @param[in,out] ctx Parse context.
 * @param[in] depth Current nesting depth.
 * @param[in] opts Optional parse options.
 * @param[out] out Parsed JSON_Value.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t
cdd_json_parse_array(struct cdd_json_parse_ctx *ctx, size_t depth,
                     const struct cdd_json_parse_options *opts,
                     JSON_Value **out) {
  JSON_Value *val = NULL;
  cdd_c_error_t rc;
  cdd_c_error_t rc_dom;
  ctx->p++;
  ctx->col++;
  rc = cdd_json_value_init_array(&val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc_dom = cdd_json_skip_ws_ctx(ctx);
  if (*ctx->p == 93) {
    ctx->p++;
    ctx->col++;
    *out = val;
    return rc_dom;
  }
  for (;;) {
    JSON_Value *item = NULL;
    rc_dom = cdd_json_skip_ws_ctx(ctx);
    if (*ctx->p == 93) {
      if (opts && !opts->allow_trailing_commas) {
        return cdd_c_keep_err(CDD_C_ERROR_PARSE,
                              cdd_c_keep_err(rc_dom, cdd_json_value_free(val)));
      }
      ctx->p++;
      ctx->col++;
      *out = val;
      return rc_dom;
    }
    rc = cdd_json_parse_value(ctx, depth + 1, opts, &item);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(val));
    }
    rc = cdd_json_array_append_value(val->array_val, item);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(item),
                                               cdd_json_value_free(val)));
    }
    rc_dom = cdd_json_skip_ws_ctx(ctx);
    if (*ctx->p == 93) {
      ctx->p++;
      ctx->col++;
      *out = val;
      return rc_dom;
    } else if (*ctx->p == 44) {
      ctx->p++;
      ctx->col++;
    } else {
      return cdd_c_keep_err(CDD_C_ERROR_PARSE,
                            cdd_c_keep_err(rc_dom, cdd_json_value_free(val)));
    }
  }
}

/**
 * @brief Scales a base-10 mantissa by 10^exp10 and initializes a JSONNumber.
 *
 * @param[in] base Base mantissa value.
 * @param[in] exp10 Base-10 exponent.
 * @param[in] is_neg Non-zero to negate the resulting double.
 * @param[out] out Output initialized JSON_Value.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_init_scaled_number(double base, int exp10,
                                                 int is_neg, JSON_Value **out) {
  static const double k_pow10_pos[9] = {1e1,  1e2,  1e4,   1e8,  1e16,
                                        1e32, 1e64, 1e128, 1e256};
  double v = base;
  int e = exp10;
  int idx = 0;
  if (v != 0.0 && e != 0) {
    if (e > 308) {
      v *= 1e308;
    } else if (e < -324) {
      v = 0.0;
    } else if (e > 0) {
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
  }
  if (is_neg)
    v = -v;
  return cdd_json_value_init_number(v, out);
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
  cdd_uint128_t u128_max_div10;
  uint64_t mant = 0;
  int mant_digits = 0;
  int exp_adj = 0;
  int exp_val = 0;
  int exp_neg = 0;
  JSON_Value *jv = NULL;
  cdd_c_error_t rc;

  u128.low = 0u;
  u128.high = 0u;
  /* (2^128 - 1) / 10 = 0x19999999999999999999999999999999 */
  u128_max_div10.high =
      (((uint64_t)0x19999999UL) << 32) | (uint64_t)0x99999999UL;
  u128_max_div10.low =
      (((uint64_t)0x99999999UL) << 32) | (uint64_t)0x99999999UL;

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
        uint64_t a_lo = u128.low & 0xFFFFFFFFu;
        uint64_t a_hi = u128.low >> 32;
        uint64_t p00 = a_lo * 10u;
        uint64_t p10 = a_hi * 10u;
        uint64_t mid = p10 + (p00 >> 32);
        uint64_t prod_lo = (mid << 32) | (p00 & 0xFFFFFFFFu);
        uint64_t prod_hi = u128.high * 10u + (mid >> 32);
        u128.low = prod_lo + digit;
        u128.high = prod_hi + (u128.low < prod_lo ? 1u : 0u);
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
  *p = endp;
  rc =
      cdd_json_init_scaled_number((double)mant, exp_val + exp_adj, is_neg, &jv);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (is_int && !overflow128) {
    size_t tok_len = (size_t)(endp - start);
    rc = cdd_json_strndup(start, tok_len, &jv->raw_num_str);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(jv));
    }
    if (!overflow64) {
      uint64_t max_i64 =
          (((uint64_t)0x7fffffffUL) << 32) | (uint64_t)0xffffffffUL;
      if (is_neg) {
        uint64_t max_neg = ((uint64_t)1u) << 63;
        if (uacc <= max_neg) {
          jv->int64_val =
              (uacc == max_neg) ? (-(int64_t)max_i64 - 1) : -(int64_t)uacc;
          jv->int128_val.low = (uint64_t)jv->int64_val;
          jv->int128_val.high = -1;
          jv->num_kind = 1;
        } else {
          uint64_t neg_lo = 0u - u128.low;
          uint64_t borrow = (u128.low | neg_lo) >> 63;
          jv->int128_val.low = neg_lo;
          jv->int128_val.high = (int64_t)(0u - u128.high - borrow);
          jv->num_kind = 3;
        }
      } else {
        jv->uint64_val = uacc;
        jv->uint128_val = u128;
        jv->int128_val.low = u128.low;
        jv->int128_val.high = 0;
        if (uacc <= max_i64) {
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
          uint64_t neg_lo = 0u - u128.low;
          uint64_t borrow = (u128.low | neg_lo) >> 63;
          jv->int128_val.low = neg_lo;
          jv->int128_val.high = (int64_t)(0u - u128.high - borrow);
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
 * @brief Parses any JSON value at *ctx->p.
 *
 * @param[in,out] ctx Parse context.
 * @param[in] depth Current nesting depth.
 * @param[in] opts Optional parse options.
 * @param[out] out Parsed JSON_Value.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t
cdd_json_parse_value(struct cdd_json_parse_ctx *ctx, size_t depth,
                     const struct cdd_json_parse_options *opts,
                     JSON_Value **out) {
  cdd_c_error_t rc;
  cdd_c_error_t rc_dom;
  size_t tok_line = 0;
  size_t tok_col = 0;
  size_t tok_offset = 0;
  JSON_Value *val = NULL;
  size_t limit =
      (opts && opts->max_depth > 0) ? opts->max_depth : CDD_JSON_MAX_DEPTH;
  if (depth > limit)
    return CDD_C_ERROR_PARSE;
  rc_dom = cdd_json_skip_ws_tok(ctx, &tok_line, &tok_col, &tok_offset);
  switch (*ctx->p) {
  case 123: {
    rc = cdd_json_parse_object(ctx, depth, opts, &val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  }
  case 91: {
    rc = cdd_json_parse_array(ctx, depth, opts, &val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  }
  case 34: {
    char *s = NULL;
    size_t slen = 0;
    const char *p_cur = ctx->p;
    rc = cdd_json_parse_raw_string_len(&p_cur, &s, &slen);
    if (rc != CDD_C_SUCCESS) {
      ctx->col += (size_t)(p_cur - ctx->p);
      ctx->p = p_cur;
      return rc;
    }
    ctx->col += (size_t)(p_cur - ctx->p);
    ctx->p = p_cur;
    rc = cdd_json_value_init_string_no_copy_len(s, slen, &val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  }
  case 116: {
    if (strncmp(ctx->p, "true", 4) == 0) {
      ctx->p += 4;
      ctx->col += 4;
      rc = cdd_json_value_init_boolean(1, &val);
      if (rc != CDD_C_SUCCESS)
        return rc;
      break;
    }
    return CDD_C_ERROR_PARSE;
  }
  case 102: {
    if (strncmp(ctx->p, "false", 5) == 0) {
      ctx->p += 5;
      ctx->col += 5;
      rc = cdd_json_value_init_boolean(0, &val);
      if (rc != CDD_C_SUCCESS)
        return rc;
      break;
    }
    return CDD_C_ERROR_PARSE;
  }
  case 110: {
    if (strncmp(ctx->p, "null", 4) == 0) {
      ctx->p += 4;
      ctx->col += 4;
      rc = cdd_json_value_init_null(&val);
      if (rc != CDD_C_SUCCESS)
        return rc;
      break;
    }
    return CDD_C_ERROR_PARSE;
  }
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
  case 57: {
    const char *p_cur = ctx->p;
    rc = cdd_json_parse_number(&p_cur, &val);
    if (rc != CDD_C_SUCCESS) {
      ctx->col += (size_t)(p_cur - ctx->p);
      ctx->p = p_cur;
      return rc;
    }
    ctx->col += (size_t)(p_cur - ctx->p);
    ctx->p = p_cur;
    break;
  }
  default: {
    return cdd_c_keep_err(CDD_C_ERROR_PARSE, rc_dom);
  }
  }
  val->src_line = tok_line;
  val->src_col = tok_col;
  val->src_offset = tok_offset;
  *out = val;
  return rc_dom;
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
  struct cdd_json_parse_ctx ctx;
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
  ctx.start = str;
  ctx.p = str;
  ctx.line = 1;
  ctx.col = 1;
  if ((unsigned char)ctx.p[0] == 0xEF && (unsigned char)ctx.p[1] == 0xBB &&
      (unsigned char)ctx.p[2] == 0xBF) {
    ctx.p += 3;
    ctx.col += 3;
  }
  rc = cdd_json_parse_value(&ctx, 0, NULL, &val);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_fill_parse_error(str, ctx.p, err_out));
  }
  rc = cdd_json_skip_ws_ctx(&ctx);
  if (*ctx.p != 0) {
    return cdd_c_keep_err(
        CDD_C_ERROR_PARSE,
        cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(val),
                                          cdd_json_fill_parse_error(str, ctx.p,
                                                                    err_out))));
  }

  *out = val;
  return rc;
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
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_dealloc_free(copy));
  }
  return cdd_json_dealloc_free(copy);
}

/**
 * @brief Strips C/C++ comments from a JSON string, replacing comment bytes with
 * spaces (preserving newlines) so line, column, and byte offsets remain intact.
 *
 * @param[in] str Input JSON text.
 * @param[out] out_cleaned Newly allocated cleaned string.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_preprocess_source(const char *str,
                                                char **out_cleaned) {
  size_t len, i = 0, oi = 0;
  char *cleaned = NULL;
  int in_str = 0;
  cdd_c_error_t rc;
  *out_cleaned = NULL;
  len = strlen(str);
  rc = cdd_json_alloc_fn(len + 1, (void **)&cleaned);
  if (rc != CDD_C_SUCCESS)
    return rc;
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
        cleaned[oi++] = ' ';
        cleaned[oi++] = ' ';
        i += 2;
        while (i < len && str[i] != 10) {
          cleaned[oi++] = ' ';
          i++;
        }
      } else if (str[i] == 47 && i + 1 < len && str[i + 1] == 42) {
        cleaned[oi++] = ' ';
        cleaned[oi++] = ' ';
        i += 2;
        while (i + 1 < len && !(str[i] == 42 && str[i + 1] == 47)) {
          cleaned[oi++] = (str[i] == 10) ? (char)10 : ' ';
          i++;
        }
        if (i + 1 >= len) {
          return cdd_c_keep_err(CDD_C_ERROR_PARSE,
                                cdd_json_dealloc_free(cleaned));
        }
        cleaned[oi++] = ' ';
        cleaned[oi++] = ' ';
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
  struct cdd_json_parse_ctx ctx;
  JSON_Value *val = NULL;
  cdd_c_error_t rc;
  cdd_c_error_t rc_ws = CDD_C_SUCCESS;
  cdd_c_error_t rc_free = CDD_C_SUCCESS;
  cdd_c_error_t rc_clean = CDD_C_SUCCESS;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!options) {
    return cdd_json_parse_string(str, out);
  }
  if (options->allow_comments) {
    rc = cdd_json_preprocess_source(str, &cleaned);
    if (rc != CDD_C_SUCCESS)
      return rc;
    src = cleaned;
  } else {
    src = str;
  }
  ctx.start = src;
  ctx.p = src;
  ctx.line = 1;
  ctx.col = 1;
  if ((unsigned char)ctx.p[0] == 0xEF && (unsigned char)ctx.p[1] == 0xBB &&
      (unsigned char)ctx.p[2] == 0xBF) {
    ctx.p += 3;
    ctx.col += 3;
  }
  rc = cdd_json_parse_value(&ctx, 0, options, &val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc_ws = cdd_json_skip_ws_ctx(&ctx);
  if (*ctx.p != 0) {
    rc_free = cdd_json_value_free(val);
    rc = CDD_C_ERROR_PARSE;
    goto cleanup;
  }
  *out = val;
cleanup:
  if (cleaned) {
    rc_clean = cdd_json_dealloc_free(cleaned);
  }
  return cdd_c_keep_err(
      rc, cdd_c_keep_err(rc_ws, cdd_c_keep_err(rc_free, rc_clean)));
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
  rc = cdd_json_alloc_fn((size_t)fsize + 1, (void **)&buf);
  if (rc != CDD_C_SUCCESS) {
    fclose(fp);
    return rc;
  }
  read_bytes = fread(buf, 1, (size_t)fsize, fp);
  fclose(fp);
  buf[read_bytes] = 0;
  rc = cdd_json_parse_string(buf, out);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_dealloc_free(buf));
  }
  return cdd_json_dealloc_free(buf);
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
  cdd_c_error_t rc_free = CDD_C_SUCCESS;
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
    char *nbuf = NULL;
    cdd_c_error_t rc = cdd_json_alloc_fn(ncap, (void **)&nbuf);
    if (rc != CDD_C_SUCCESS) {
      b->err = rc;
      return rc;
    }
    if (b->len > 0) {
      memcpy(nbuf, b->data, b->len);
      rc_free = cdd_json_dealloc_free(b->data);
    }
    b->data = nbuf;
    b->cap = ncap;
  }
  memcpy(b->data + b->len, s, n);
  b->len += n;
  b->data[b->len] = 0;
  return cdd_c_keep_err(b->err, rc_free);
}

/**
 * @brief Decodes the next UTF-16 code unit from a UTF-8 byte sequence for
 * RFC 8785 §3.2.3 key ordering.
 *
 * @param[in] s UTF-8 string bytes.
 * @param[in] len Total byte length of `s`.
 * @param[in,out] pos Current byte offset in `s`.
 * @param[in,out] pending_low Pending low surrogate (0 if none).
 * @return Next 16-bit UTF-16 code unit.
 */
static int sort_next_utf16_unit(const char *s, size_t len, size_t *pos,
                                unsigned int *pending_low) {
  unsigned char b0;
  unsigned int cp;
  if (*pending_low != 0) {
    unsigned int low = *pending_low;
    *pending_low = 0;
    return (int)low;
  }
  b0 = (unsigned char)s[*pos];
  if (b0 < 0x80) {
    (*pos) += 1;
    return (int)b0;
  }
  if ((b0 & 0xE0) == 0xC0 && *pos + 1 < len) {
    cp = ((unsigned int)(b0 & 0x1F) << 6) |
         (unsigned int)((unsigned char)s[*pos + 1] & 0x3F);
    (*pos) += 2;
    return (int)cp;
  }
  if ((b0 & 0xF0) == 0xE0 && *pos + 2 < len) {
    cp = ((unsigned int)(b0 & 0x0F) << 12) |
         ((unsigned int)((unsigned char)s[*pos + 1] & 0x3F) << 6) |
         (unsigned int)((unsigned char)s[*pos + 2] & 0x3F);
    (*pos) += 3;
    return (int)cp;
  }
  if ((b0 & 0xF8) == 0xF0 && *pos + 3 < len) {
    cp = ((unsigned int)(b0 & 0x07) << 18) |
         ((unsigned int)((unsigned char)s[*pos + 1] & 0x3F) << 12) |
         ((unsigned int)((unsigned char)s[*pos + 2] & 0x3F) << 6) |
         (unsigned int)((unsigned char)s[*pos + 3] & 0x3F);
    (*pos) += 4;
    cp -= 0x10000;
    *pending_low = 0xDC00u + (cp & 0x3FFu);
    return (int)(0xD800u + (cp >> 10));
  }
  (*pos) += 1;
  return (int)b0;
}

/**
 * @brief Compares two UTF-8 strings by their UTF-16 code unit sequences per
 * RFC 8785 §3.2.3.
 *
 * @param[in] a First string.
 * @param[in] la Byte length of `a`.
 * @param[in] b Second string.
 * @param[in] lb Byte length of `b`.
 * @return Negative if `a < b`, positive if `a > b`.
 */
static int cdd_json_utf16_keys_cmp(const char *a, size_t la, const char *b,
                                   size_t lb) {
  size_t pa = 0, pb = 0;
  unsigned int low_a = 0, low_b = 0;
  for (;;) {
    int ua, ub;
    if (pa >= la && low_a == 0) {
      return -1;
    }
    if (pb >= lb && low_b == 0) {
      return 1;
    }
    ua = sort_next_utf16_unit(a, la, &pa, &low_a);
    ub = sort_next_utf16_unit(b, lb, &pb, &low_b);
    if (ua != ub) {
      return (ua < ub) ? -1 : 1;
    }
  }
}

/**
 * @brief Formats and appends a `JSONNumber` value according to RFC 8785
 * §3.2.2.3 (ECMAScript Number::toString canonicalization).
 *
 * @param[in] val JSONNumber value.
 * @param[in,out] b Output serialization buffer.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_ser_canonical_number(const JSON_Value *val,
                                                   struct cdd_json_ser_buf *b) {
  char nbuf[64];
  char *e_ptr;
  uint64_t max_safe = (((uint64_t)1u) << 53) - 1u;
  int64_t max_safe_s = (int64_t)max_safe;
  if (val->number_val == 0.0) {
    return cdd_json_ser_append(b, "0", 1);
  }
  if (val->num_kind == 1 && val->int64_val >= -max_safe_s &&
      val->int64_val <= max_safe_s) {
    char rev[32];
    size_t rlen = 0;
    size_t pos = 0;
    uint64_t uval = (val->int64_val < 0)
                        ? (uint64_t)(-(val->int64_val + 1)) + 1u
                        : (uint64_t)val->int64_val;
    while (uval > 0) {
      rev[rlen++] = (char)('0' + (int)(uval % 10u));
      uval /= 10u;
    }
    if (val->int64_val < 0)
      nbuf[pos++] = '-';
    while (rlen > 0) {
      nbuf[pos++] = rev[--rlen];
    }
    nbuf[pos] = '\0';
    return cdd_json_ser_append(b, nbuf, pos);
  }
#if defined(_MSC_VER)
  sprintf_s(nbuf, sizeof(nbuf), "%.15g", val->number_val);
#else
  sprintf(nbuf, "%.15g", val->number_val);
#endif
  if (strtod(nbuf, NULL) != val->number_val) {
#if defined(_MSC_VER)
    sprintf_s(nbuf, sizeof(nbuf), "%.17g", val->number_val);
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
  return cdd_json_ser_append(b, nbuf, strlen(nbuf));
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
  for (i = 0; i < level; ++i) {
    cdd_c_error_t rc = cdd_json_ser_append(b, "    ", 4);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
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
  cdd_c_error_t rc;
  static const char k_quote[1] = {34};
  static const char k_esc_quote[2] = {92, 34};
  static const char k_esc_bs[2] = {92, 92};
  static const char k_esc_slash[2] = {92, 47};
  static const char k_esc_b[2] = {92, 98};
  static const char k_esc_f[2] = {92, 102};
  static const char k_esc_n[2] = {92, 110};
  static const char k_esc_r[2] = {92, 114};
  static const char k_esc_t[2] = {92, 116};
  rc = cdd_json_ser_append(b, k_quote, 1);
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (i = 0; i < len; ++i) {
    unsigned char c = (unsigned char)s[i];
    if (c == 34) {
      rc = cdd_json_ser_append(b, k_esc_quote, 2);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (c == 92) {
      rc = cdd_json_ser_append(b, k_esc_bs, 2);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (c == 47) {
      rc = cdd_json_ser_append(b, k_esc_slash, 2);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (c == 8) {
      rc = cdd_json_ser_append(b, k_esc_b, 2);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (c == 12) {
      rc = cdd_json_ser_append(b, k_esc_f, 2);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (c == 10) {
      rc = cdd_json_ser_append(b, k_esc_n, 2);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (c == 13) {
      rc = cdd_json_ser_append(b, k_esc_r, 2);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (c == 9) {
      rc = cdd_json_ser_append(b, k_esc_t, 2);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (c < 0x20) {
      static const char k_hex_digits[] = "0123456789abcdef";
      char uhex[6];
      uhex[0] = 92;
      uhex[1] = 117;
      uhex[2] = '0';
      uhex[3] = '0';
      uhex[4] = k_hex_digits[(c >> 4) & 0x0F];
      uhex[5] = k_hex_digits[c & 0x0F];
      rc = cdd_json_ser_append(b, uhex, 6);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      char ch = (char)c;
      rc = cdd_json_ser_append(b, &ch, 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return cdd_json_ser_append(b, k_quote, 1);
}

/**
 * @brief Serializes a JSONNumber leaf value into `b` using a leaf stack frame
 * so recursive `cdd_json_ser_value` frames stay compact under ASan.
 *
 * @param[in] val JSONNumber value to serialize.
 * @param[in,out] b Output serialization buffer.
 * @param[in] canonical Non-zero for RFC 8785 canonical number formatting.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_ser_number(const JSON_Value *val,
                                         struct cdd_json_ser_buf *b,
                                         int canonical) {
  char nbuf[64];
  if (canonical) {
    return cdd_json_ser_canonical_number(val, b);
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

/**
 * @brief Explicit worklist frame for iterative JSON serialization.
 */
struct cdd_json_ser_frame {
  /** @brief Value being serialized at this nesting level. */
  const JSON_Value *val;
  /** @brief Next child index to emit for arrays/objects. */
  size_t idx;
  /** @brief Optional sorted index permutation for canonical objects. */
  size_t *order;
};

/**
 * @brief Builds a lexicographically sorted UTF-16 code-unit key index array
 * for RFC 8785 canonical JSON object serialization.
 *
 * @param[in] obj Non-empty JSON object with count > 1.
 * @param[in,out] b Serialization buffer tracking error state.
 * @param[out] out_order Output pointer receiving allocated index array.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_build_canonical_order(const JSON_Object *obj,
                                                    struct cdd_json_ser_buf *b,
                                                    size_t **out_order) {
  size_t *order = NULL;
  size_t a_idx, gap;
  cdd_c_error_t rc =
      cdd_json_alloc_fn(obj->count * sizeof(size_t), (void **)&order);
  if (rc != CDD_C_SUCCESS) {
    b->err = rc;
    return rc;
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
        int cmp =
            cdd_json_utf16_keys_cmp(obj->names[prev], pl, obj->names[temp], tl);
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
  *out_order = order;
  return CDD_C_SUCCESS;
}

/**
 * @brief Pushes a child `JSON_Value` onto the iterative serialization stack.
 *
 * @param[in,out] stack_ptr Pointer to active frame array (inline or heap).
 * @param[in] inline_base Base address of inline stack buffer.
 * @param[in,out] cap_ptr Pointer to frame array capacity.
 * @param[in,out] sp_ptr Pointer to active stack depth.
 * @param[in] child Child value to push.
 * @param[in,out] b Serialization buffer tracking error state.
 * @return CDD_C_SUCCESS or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_ser_push(struct cdd_json_ser_frame **stack_ptr,
                                       struct cdd_json_ser_frame *inline_base,
                                       size_t *cap_ptr, size_t *sp_ptr,
                                       const JSON_Value *child,
                                       struct cdd_json_ser_buf *b) {
  cdd_c_error_t rc_free = CDD_C_SUCCESS;
  if (*sp_ptr >= *cap_ptr) {
    size_t ncap = *cap_ptr * 2;
    struct cdd_json_ser_frame *nstack = NULL;
    cdd_c_error_t rc = cdd_json_alloc_fn(
        ncap * sizeof(struct cdd_json_ser_frame), (void **)&nstack);
    if (rc != CDD_C_SUCCESS) {
      b->err = rc;
      return rc;
    }
    memcpy(nstack, *stack_ptr, *sp_ptr * sizeof(struct cdd_json_ser_frame));
    if (*stack_ptr != inline_base) {
      rc_free = cdd_json_dealloc_free(*stack_ptr);
    }
    *stack_ptr = nstack;
    *cap_ptr = ncap;
  }
  (*stack_ptr)[*sp_ptr].val = child;
  (*stack_ptr)[*sp_ptr].idx = 0;
  (*stack_ptr)[*sp_ptr].order = NULL;
  (*sp_ptr)++;
  return rc_free;
}

/**
 * @brief Executes a single iterative serialization worklist step.
 *
 * @param[in,out] stack_ptr Active frame stack pointer.
 * @param[in] inline_frames Base inline frame buffer.
 * @param[in,out] cap_ptr Active frame stack capacity.
 * @param[in,out] sp_ptr Active frame stack depth.
 * @param[in,out] b Output serialization buffer.
 * @param[in] pretty Non-zero for pretty printing.
 * @param[in] canonical Non-zero for RFC 8785 canonical key sorting.
 * @param[in] level Current nesting depth.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_ser_step(struct cdd_json_ser_frame **stack_ptr,
                                       struct cdd_json_ser_frame *inline_frames,
                                       size_t *cap_ptr, size_t *sp_ptr,
                                       struct cdd_json_ser_buf *b, int pretty,
                                       int canonical, size_t level) {
  static const char k_arr_pretty_open[2] = {91, 10};
  static const char k_obj_pretty_open[2] = {123, 10};
  static const char k_comma_nl[2] = {44, 10};
  static const char k_nl[1] = {10};
  struct cdd_json_ser_frame *top = &(*stack_ptr)[*sp_ptr - 1];
  const JSON_Value *cur = top->val;
  cdd_c_error_t rc;

  switch (cur->type) {
  case JSONNull: {
    (*sp_ptr)--;
    return cdd_json_ser_append(b, "null", 4);
  }
  case JSONBoolean: {
    (*sp_ptr)--;
    return cdd_json_ser_append(b, cur->boolean_val ? "true" : "false",
                               cur->boolean_val ? 4 : 5);
  }
  case JSONNumber: {
    (*sp_ptr)--;
    return cdd_json_ser_number(cur, b, canonical);
  }
  case JSONString: {
    (*sp_ptr)--;
    return cdd_json_ser_string_len(b, cur->string_val, cur->string_len);
  }
  case JSONArray: {
    const JSON_Array *arr = cur->array_val;
    if (arr->count == 0) {
      (*sp_ptr)--;
      return cdd_json_ser_append(b, "[]", 2);
    }
    if (top->idx == 0) {
      rc = cdd_json_ser_append(b, pretty ? k_arr_pretty_open : "[",
                               pretty ? 2 : 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (pretty) {
        rc = cdd_json_ser_indent(b, level + 1);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      top->idx = 1;
      return cdd_json_ser_push(stack_ptr, inline_frames, cap_ptr, sp_ptr,
                               arr->items[0], b);
    }
    if (top->idx < arr->count) {
      const JSON_Value *item = arr->items[top->idx];
      rc = cdd_json_ser_append(b, pretty ? k_comma_nl : ",", pretty ? 2 : 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (pretty) {
        rc = cdd_json_ser_indent(b, level + 1);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      top->idx++;
      return cdd_json_ser_push(stack_ptr, inline_frames, cap_ptr, sp_ptr, item,
                               b);
    }
    if (pretty) {
      rc = cdd_json_ser_append(b, k_nl, 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = cdd_json_ser_indent(b, level);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    (*sp_ptr)--;
    return cdd_json_ser_append(b, "]", 1);
  }
  case JSONObject: {
    const JSON_Object *obj = cur->object_val;
    if (obj->count == 0) {
      (*sp_ptr)--;
      return cdd_json_ser_append(b, "{}", 2);
    }
    if (top->idx == 0) {
      size_t real_idx;
      if (canonical && obj->count > 1) {
        rc = cdd_json_build_canonical_order(obj, b, &top->order);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      rc = cdd_json_ser_append(b, pretty ? k_obj_pretty_open : "{",
                               pretty ? 2 : 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (pretty) {
        rc = cdd_json_ser_indent(b, level + 1);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      real_idx = top->order ? top->order[0] : 0;
      rc = cdd_json_ser_string_len(b, obj->names[real_idx],
                                   obj->name_lens[real_idx]);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = cdd_json_ser_append(b, pretty ? ": " : ":", pretty ? 2 : 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
      top->idx = 1;
      return cdd_json_ser_push(stack_ptr, inline_frames, cap_ptr, sp_ptr,
                               obj->values[real_idx], b);
    }
    if (top->idx < obj->count) {
      size_t real_idx = top->order ? top->order[top->idx] : top->idx;
      rc = cdd_json_ser_append(b, pretty ? k_comma_nl : ",", pretty ? 2 : 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (pretty) {
        rc = cdd_json_ser_indent(b, level + 1);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      rc = cdd_json_ser_string_len(b, obj->names[real_idx],
                                   obj->name_lens[real_idx]);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = cdd_json_ser_append(b, pretty ? ": " : ":", pretty ? 2 : 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
      top->idx++;
      return cdd_json_ser_push(stack_ptr, inline_frames, cap_ptr, sp_ptr,
                               obj->values[real_idx], b);
    }
    {
      cdd_c_error_t rc_free = CDD_C_SUCCESS;
      if (top->order) {
        rc_free = cdd_json_dealloc_free(top->order);
        top->order = NULL;
      }
      if (pretty) {
        rc = cdd_json_ser_append(b, k_nl, 1);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc, rc_free);
        }
        rc = cdd_json_ser_indent(b, level);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc, rc_free);
        }
      }
      (*sp_ptr)--;
      return cdd_c_keep_err(cdd_json_ser_append(b, "}", 1), rc_free);
    }
  }
  default:
    b->err = CDD_C_ERROR_INVALID_ARGUMENT;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
}

/**
 * @brief Iteratively serializes a JSON_Value into b using O(1) call-stack
 * space.
 *
 * @param[in] val Value to serialize.
 * @param[in,out] b Output buffer.
 * @param[in] pretty Non-zero for pretty printing.
 * @param[in] canonical Non-zero for RFC 8785 lexicographical key sorting.
 * @param[in] start_level Initial indentation/nesting level.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_ser_value(const JSON_Value *val,
                                        struct cdd_json_ser_buf *b, int pretty,
                                        int canonical, size_t start_level) {
  struct cdd_json_ser_frame inline_frames[32];
  struct cdd_json_ser_frame *stack = inline_frames;
  size_t cap = sizeof(inline_frames) / sizeof(inline_frames[0]);
  size_t sp = 1;
  size_t k;
  cdd_c_error_t step_rc = CDD_C_SUCCESS;
  cdd_c_error_t rc_ign = CDD_C_SUCCESS;

  inline_frames[0].val = val;
  inline_frames[0].idx = 0;
  inline_frames[0].order = NULL;

  while (sp > 0) {
    size_t level = start_level + (sp - 1);
    if (level > CDD_JSON_MAX_DEPTH) {
      b->err = CDD_C_ERROR_PARSE;
      step_rc = CDD_C_ERROR_PARSE;
      break;
    }
    step_rc = cdd_json_ser_step(&stack, inline_frames, &cap, &sp, b, pretty,
                                canonical, level);
    if (step_rc != CDD_C_SUCCESS)
      goto cleanup;
  }

cleanup:
  for (k = 0; k < sp; ++k) {
    if (stack[k].order) {
      rc_ign = cdd_json_dealloc_free(stack[k].order);
    }
  }
  if (stack != inline_frames) {
    rc_ign = cdd_json_dealloc_free(stack);
  }
  return cdd_c_keep_err(step_rc, cdd_c_keep_err(b->err, rc_ign));
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
    if (b.data) {
      return cdd_c_keep_err(rc, cdd_json_dealloc_free(b.data));
    }
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
  return cdd_json_dealloc_free(s);
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
    return (cdd_c_error_t)((int)CDD_C_ERROR_INVALID_ARGUMENT |
                           (int)cdd_json_dealloc_free(s));
  }
  memcpy(buf, s, len + 1);
  return cdd_json_dealloc_free(s);
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

/**
 * @brief Closes an open file stream and propagates `rc`.
 * @param[in,out] fp Open file stream to close.
 * @param[in] rc Error status to propagate.
 * @return `rc` unaltered.
 */
static cdd_c_error_t cdd_json_close_fp_and_return(FILE *fp, cdd_c_error_t rc) {
  fclose(fp);
  return rc;
}

cdd_c_error_t cdd_json_serialize_to_file_pretty(const JSON_Value *val,
                                                const char *filename) {
  FILE *fp = NULL;
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
  return cdd_json_close_fp_and_return(fp, cdd_json_serialize_to_fp(val, fp, 1));
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
        return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(arr_val));
      }
      rc = cdd_json_array_append_value(arr_val->array_val, item);
      if (rc != CDD_C_SUCCESS) {
        return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(item) |
                               (int)cdd_json_value_free(arr_val));
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
  cdd_c_error_t rc = CDD_C_SUCCESS;
  cdd_c_error_t rc_ign = CDD_C_SUCCESS;
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
    rc = cdd_json_ser_value(arr->items[i], &b, 0, 0, 0);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_json_ser_append(&b, "\n", 1);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  *out_ndjson = b.data;
  return CDD_C_SUCCESS;
cleanup:
  if (b.data) {
    rc_ign = cdd_json_dealloc_free(b.data);
  }
  return cdd_c_keep_err(rc, rc_ign);
}

/* --- RFC 6901 JSON Pointer, RFC 7396 Merge Patch & RFC 6902 JSON Patch --- */

cdd_c_error_t cdd_json_pointer_unescape(const char *in, size_t len,
                                        char **out_token) {
  size_t i, j = 0;
  char *out = NULL;
  cdd_c_error_t rc;
  if (!out_token)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_token = NULL;
  if (!in)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_ctx_alloc_fn(len + 1, (void **)&out);
  if (rc != CDD_C_SUCCESS)
    return rc;
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
static cdd_c_error_t cdd_json_pointer_token_free(char *tok) {
  return cdd_json_ctx_dealloc_free(tok);
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
  cdd_c_error_t rc_free = CDD_C_SUCCESS;
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
      return (cdd_c_error_t)((int)rc | (int)rc_free);
    if (cur->type == JSONObject) {
      JSON_Value *child = NULL;
      rc = cdd_json_object_get_value(cur->object_val, token, &child);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(
            rc, cdd_c_keep_err(rc_free, cdd_json_pointer_token_free(token)));
      }
      cur = child;
    } else if (cur->type == JSONArray) {
      char *endp = NULL;
      unsigned long idx;
      if (token[0] == '\0' || (token[0] == '0' && token[1] != '\0')) {
        return (cdd_c_error_t)((int)CDD_C_ERROR_NOT_FOUND | (int)rc_free |
                               (int)cdd_json_pointer_token_free(token));
      }
      idx = strtoul(token, &endp, 10);
      if (*endp != '\0' || idx >= cur->array_val->count) {
        return (cdd_c_error_t)((int)CDD_C_ERROR_NOT_FOUND | (int)rc_free |
                               (int)cdd_json_pointer_token_free(token));
      }
      cur = cur->array_val->items[idx];
    } else {
      return (cdd_c_error_t)((int)CDD_C_ERROR_NOT_FOUND | (int)rc_free |
                             (int)cdd_json_pointer_token_free(token));
    }
    rc_free =
        (cdd_c_error_t)((int)rc_free | (int)cdd_json_pointer_token_free(token));
    p += seg_len;
  }
  *out_val = cur;
  return rc_free;
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
  const JSON_Value *src = elem;
  *out_fval = NULL;
  if (src->type != JSONObject)
    return CDD_C_SUCCESS;
  for (ki = 0; ki < src->object_val->count; ++ki) {
    size_t nlen = src->object_val->name_lens[ki];
    if (nlen == klen && memcmp(src->object_val->names[ki], kstart, klen) == 0) {
      *out_fval = src->object_val->values[ki];
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
 * `cdd_json_ctx_alloc_fn`.
 * @param[out] out_len Output byte length.
 * @return CDD_C_SUCCESS, CDD_C_ERROR_INVALID_ARGUMENT, or CDD_C_ERROR_MEMORY.
 */
static cdd_c_error_t cdd_json_parse_jp_quoted(const char **cursor,
                                              char **out_str, size_t *out_len) {
  const char *p = *cursor;
  char quote;
  const char *scan;
  size_t max_len = 0;
  char *buf = NULL;
  size_t oi = 0;
  cdd_c_error_t rc;
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
  rc = cdd_json_ctx_alloc_fn(max_len + 1, (void **)&buf);
  if (rc != CDD_C_SUCCESS)
    return rc;
  while (*p != quote) {
    if (*p == '\\') {
      ++p;
      if (*p == 'n') {
        buf[oi++] = '\n';
        ++p;
      } else if (*p == 'r') {
        buf[oi++] = '\r';
        ++p;
      } else if (*p == 't') {
        buf[oi++] = '\t';
        ++p;
      } else if (*p == 'b') {
        buf[oi++] = '\b';
        ++p;
      } else if (*p == 'f') {
        buf[oi++] = '\f';
        ++p;
      } else if (*p == 'u') {
        unsigned int cp = 0;
        ++p;
        rc = cdd_json_parse_hex4(p, &cp);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_c_keep_err(rc, cdd_json_pointer_token_free(buf)));
        }
        p += 4;
        if (cp >= 0xD800 && cp <= 0xDBFF) {
          unsigned int low = 0;
          if (p[0] != '\\' || p[1] != 'u') {
            return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                                  cdd_json_pointer_token_free(buf));
          }
          rc = cdd_json_parse_hex4(p + 2, &low);
          if (rc != CDD_C_SUCCESS) {
            return cdd_c_keep_err(
                CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_c_keep_err(rc, cdd_json_pointer_token_free(buf)));
          }
          if (low < 0xDC00 || low > 0xDFFF) {
            return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                                  cdd_json_pointer_token_free(buf));
          }
          cp = 0x10000 + (((cp - 0xD800) << 10) | (low - 0xDC00));
          p += 6;
        } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
          return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                                cdd_json_pointer_token_free(buf));
        }
        if (cp <= 0x7F) {
          buf[oi++] = (char)cp;
        } else if (cp <= 0x7FF) {
          buf[oi++] = (char)(0xC0 | ((cp >> 6) & 0x1F));
          buf[oi++] = (char)(0x80 | (cp & 0x3F));
        } else if (cp <= 0xFFFF) {
          buf[oi++] = (char)(0xE0 | ((cp >> 12) & 0x0F));
          buf[oi++] = (char)(0x80 | ((cp >> 6) & 0x3F));
          buf[oi++] = (char)(0x80 | (cp & 0x3F));
        } else {
          buf[oi++] = (char)(0xF0 | ((cp >> 18) & 0x07));
          buf[oi++] = (char)(0x80 | ((cp >> 12) & 0x3F));
          buf[oi++] = (char)(0x80 | ((cp >> 6) & 0x3F));
          buf[oi++] = (char)(0x80 | (cp & 0x3F));
        }
      } else {
        buf[oi++] = *p++;
      }
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
  cdd_c_error_t rc = CDD_C_SUCCESS;
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
      rc = cdd_json_iregexp_branch(pat + next_pat, pat_len - next_pat, str,
                                   str_len, cur, anchor_end, &m);
      if (m) {
        *out_matched = 1;
        return rc;
      }
      if (cur >= str_len)
        return rc;
      rc = cdd_json_iregexp_atom(pat, atom_len, (unsigned char)str[cur], &m);
      if (!m)
        return rc;
      ++cur;
    }
  }
  if (quant == '+') {
    size_t cur = str_pos;
    int m_atom = 0;
    if (cur >= str_len)
      return CDD_C_SUCCESS;
    rc = cdd_json_iregexp_atom(pat, atom_len, (unsigned char)str[cur], &m_atom);
    if (!m_atom)
      return rc;
    ++cur;
    for (;;) {
      int m = 0;
      rc = cdd_json_iregexp_branch(pat + next_pat, pat_len - next_pat, str,
                                   str_len, cur, anchor_end, &m);
      if (m) {
        *out_matched = 1;
        return rc;
      }
      if (cur >= str_len)
        return rc;
      rc = cdd_json_iregexp_atom(pat, atom_len, (unsigned char)str[cur], &m);
      if (!m)
        return rc;
      ++cur;
    }
  }
  if (quant == '?') {
    int m = 0;
    if (str_pos < str_len) {
      int m_atom = 0;
      rc = cdd_json_iregexp_atom(pat, atom_len, (unsigned char)str[str_pos],
                                 &m_atom);
      if (m_atom) {
        cdd_c_error_t rc_sub =
            cdd_json_iregexp_branch(pat + next_pat, pat_len - next_pat, str,
                                    str_len, str_pos + 1, anchor_end, &m);
        if (m) {
          *out_matched = 1;
          return cdd_c_keep_err(rc, rc_sub);
        }
      }
    }
    return cdd_json_iregexp_branch(pat + next_pat, pat_len - next_pat, str,
                                   str_len, str_pos, anchor_end, out_matched);
  }
  if (str_pos < str_len) {
    int m_atom = 0;
    rc = cdd_json_iregexp_atom(pat, atom_len, (unsigned char)str[str_pos],
                               &m_atom);
    if (m_atom) {
      return cdd_c_keep_err(
          rc, cdd_json_iregexp_branch(pat + next_pat, pat_len - next_pat, str,
                                      str_len, str_pos + 1, anchor_end,
                                      out_matched));
    }
    return rc;
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
      rc = (cdd_c_error_t)((int)rc | (int)cdd_json_iregexp_branch(
                                         pat + seg_start, i - seg_start, str,
                                         str_len, str_pos, anchor_end, &m));
      if (m) {
        *out_matched = 1;
        return rc;
      }
      seg_start = i + 1;
    }
  }
  return rc;
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
 * @brief Looks up `key[0..klen-1]` on `elem` and frees `key`.
 *
 * @param[in] elem Candidate JSON_Value element.
 * @param[in] key Owned key string to free.
 * @param[in] klen Byte length of `key`.
 * @param[out] out_fval Resolved property value (or NULL if missing).
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t
cdd_json_filter_lookup_prop_free_key(const JSON_Value *elem, char *key,
                                     size_t klen, const JSON_Value **out_fval) {
  size_t ki;
  *out_fval = NULL;
  if (elem->type == JSONObject) {
    for (ki = 0; ki < elem->object_val->count; ++ki) {
      if (elem->object_val->name_lens[ki] == klen &&
          memcmp(elem->object_val->names[ki], key, klen) == 0) {
        *out_fval = elem->object_val->values[ki];
        break;
      }
    }
  }
  return cdd_json_pointer_token_free(key);
}

/**
 * @brief Executes a single `.` or `[` step inside
 * `cdd_json_filter_resolve_path`.
 *
 * @param[in,out] io_cur Current resolved node (or NULL).
 * @param[in,out] io_p Cursor positioned at `.` or `[`.
 * @return CDD_C_SUCCESS, CDD_C_ERROR_INVALID_ARGUMENT, or
 * CDD_C_ERROR_NOT_SUPPORTED.
 */
static cdd_c_error_t cdd_json_filter_resolve_step(const JSON_Value **io_cur,
                                                  const char **io_p) {
  const char *p = *io_p;
  cdd_c_error_t rc;
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
    *io_p = p;
    if (*io_cur) {
      return cdd_json_filter_lookup_prop(*io_cur, kstart, klen, io_cur);
    }
    return CDD_C_SUCCESS;
  }
  ++p;
  rc = cdd_json_filter_skip_ws(&p);
  if (*p == 39 || *p == '"') {
    char *key = NULL;
    size_t klen = 0;
    cdd_c_error_t qrc = cdd_json_parse_jp_quoted(&p, &key, &klen);
    if (qrc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(qrc, rc);
    }
    qrc = cdd_json_filter_skip_ws(&p);
    if (*p != ']') {
      return cdd_c_keep_err(
          CDD_C_ERROR_INVALID_ARGUMENT,
          cdd_c_keep_err(qrc, cdd_json_pointer_token_free(key)));
    }
    *io_p = p + 1;
    if (*io_cur) {
      return cdd_c_keep_err(
          rc, cdd_json_filter_lookup_prop_free_key(*io_cur, key, klen, io_cur));
    }
    return cdd_c_keep_err(rc, cdd_json_pointer_token_free(key));
  }
  if (*p == '-' || (*p >= '0' && *p <= '9')) {
    char *endp = NULL;
    long idx = strtol(p, &endp, 10);
    cdd_c_error_t qrc;
    if (endp == p)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    p = endp;
    qrc = cdd_json_filter_skip_ws(&p);
    if (*p != ']') {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, qrc);
    }
    *io_p = p + 1;
    if (*io_cur) {
      if ((*io_cur)->type == JSONArray) {
        long cnt = (long)(*io_cur)->array_val->count;
        long eff = idx < 0 ? cnt + idx : idx;
        *io_cur = (eff >= 0 && eff < cnt)
                      ? (*io_cur)->array_val->items[(size_t)eff]
                      : NULL;
      } else {
        *io_cur = NULL;
      }
    }
    return CDD_C_SUCCESS;
  }
  return CDD_C_ERROR_INVALID_ARGUMENT;
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
    cdd_c_error_t rc = cdd_json_filter_resolve_step(&cur, &p);
    if (rc != CDD_C_SUCCESS)
      return rc;
    ++steps;
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
 * @return 1 if true, 0 otherwise.
 */
static int cdd_json_double_op_cmp(int op, double lhs, double rhs) {
  if (op == 1)
    return lhs == rhs;
  if (op == 2)
    return lhs != rhs;
  if (op == 3)
    return lhs <= rhs;
  if (op == 4)
    return lhs < rhs;
  if (op == 5)
    return lhs >= rhs;
  return lhs > rhs;
}

/**
 * @brief Searches for any unanchored substring match of I-Regexp `pat` inside
 * `str`.
 *
 * @param[in] pat I-Regexp pattern string.
 * @param[in] pat_len Byte length of `pat`.
 * @param[in] str Input string to search.
 * @param[in] str_len Byte length of `str`.
 * @param[out] out_matched Receives 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS on valid evaluation, or error code on invalid pattern.
 */
static cdd_c_error_t cdd_json_iregexp_search(const char *pat, size_t pat_len,
                                             const char *str, size_t str_len,
                                             int *out_matched) {
  size_t si;
  *out_matched = 0;
  if (pat_len == 0) {
    *out_matched = 1;
    return CDD_C_SUCCESS;
  }
  for (si = 0; si <= str_len; ++si) {
    int m = 0;
    cdd_c_error_t rc =
        cdd_json_iregexp_exec(pat, pat_len, str, str_len, si, 0, &m);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (m) {
      *out_matched = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates a `match` or `search` I-Regexp filter call (or validates
 * syntax when `elem` is NULL) and frees `pat_owned`.
 *
 * @param[in] fn_kind 2 for `match`, 3 for `search`.
 * @param[in] pat_owned Owned pattern string to free.
 * @param[in] pat_len Pattern byte length.
 * @param[in] elem Candidate JSON_Value element (or NULL for syntax pass).
 * @param[in] fval Resolved target value (or NULL).
 * @param[out] out_res Output match flag (1 or 0).
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t
cdd_json_eval_filter_regex_owned(int fn_kind, char *pat_owned, size_t pat_len,
                                 const JSON_Value *elem, const JSON_Value *fval,
                                 int *out_res) {
  int dummy_m = 0;
  const char *s_val = "";
  size_t s_len = 0;
  int *target_res = &dummy_m;
  cdd_c_error_t rc;
  if (elem) {
    if (!fval || fval->type != JSONString) {
      return cdd_json_pointer_token_free(pat_owned);
    }
    s_val = fval->string_val;
    s_len = fval->string_len;
    target_res = out_res;
  }
  if (fn_kind == 2) {
    rc = cdd_json_iregexp_exec(pat_owned, pat_len, s_val, s_len, 0, 1,
                               target_res);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_pointer_token_free(pat_owned));
    }
    return cdd_json_pointer_token_free(pat_owned);
  }
  rc = cdd_json_iregexp_search(pat_owned, pat_len, s_val, s_len, target_res);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_pointer_token_free(pat_owned));
  }
  return cdd_json_pointer_token_free(pat_owned);
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
  cdd_c_error_t rc_ws;
  cdd_c_error_t rc_ws2;
  cdd_c_error_t rc_ws3;

  *out_res = 0;
  rc_ws = cdd_json_filter_skip_ws(&p);

  if (strncmp(p, "length(@", 8) == 0 || strncmp(p, "count(@", 7) == 0 ||
      strncmp(p, "value(@", 7) == 0) {
    cdd_c_error_t rc_p;
    fn_kind = (p[0] == 'v') ? 4 : 1;
    p += (p[0] == 'l') ? 8 : 7;
    rc = cdd_json_filter_resolve_path(elem, &p, &fval);
    if (rc != CDD_C_SUCCESS) {
      return (CDD_C_ERROR_NOT_SUPPORTED == rc)
                 ? cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc_ws)
                 : cdd_c_keep_err(rc, rc_ws);
    }
    rc_p = cdd_json_filter_skip_ws(&p);
    if (*p != ')') {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc_p);
    }
    ++p;
  } else if (strncmp(p, "match(@", 7) == 0 || strncmp(p, "search(@", 8) == 0) {
    char *pat_owned = NULL;
    size_t pat_len = 0;
    cdd_c_error_t rc_c1, rc_c2, rc_c3;
    fn_kind = (p[0] == 'm') ? 2 : 3;
    p += (fn_kind == 2) ? 7 : 8;
    rc = cdd_json_filter_resolve_path(elem, &p, &fval);
    if (rc != CDD_C_SUCCESS) {
      return (CDD_C_ERROR_NOT_SUPPORTED == rc) ? CDD_C_ERROR_INVALID_ARGUMENT
                                               : rc;
    }
    rc_c1 = cdd_json_filter_skip_ws(&p);
    if (*p != ',') {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc_c1);
    }
    ++p;
    rc_c2 = cdd_json_filter_skip_ws(&p);
    if (*p != 39 && *p != '"') {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc_c2);
    }
    rc = cdd_json_parse_jp_quoted(&p, &pat_owned, &pat_len);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc_c3 = cdd_json_filter_skip_ws(&p);
    if (*p != ')') {
      return cdd_c_keep_err(
          CDD_C_ERROR_INVALID_ARGUMENT,
          cdd_c_keep_err(rc_c3, cdd_json_pointer_token_free(pat_owned)));
    }
    ++p;
    *cursor = p;
    return cdd_json_eval_filter_regex_owned(fn_kind, pat_owned, pat_len, elem,
                                            fval, out_res);
  } else if (p[0] == '@' && (p[1] == '.' || p[1] == '[')) {
    ++p;
    rc = cdd_json_filter_resolve_path(elem, &p, &fval);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    return CDD_C_ERROR_NOT_SUPPORTED;
  }

  rc_ws2 = cdd_json_filter_skip_ws(&p);
  if ((fn_kind == 0 || fn_kind == 4) &&
      (*p == ')' || *p == ']' || strncmp(p, "&&", 2) == 0 ||
       strncmp(p, "||", 2) == 0)) {
    *cursor = p;
    if (elem) {
      *out_res = (fval != NULL) ? 1 : 0;
    }
    return rc_ws2;
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
  rc_ws3 = cdd_json_filter_skip_ws(&p);
  if (*p == 39 || *p == '"') {
    rc = cdd_json_parse_jp_quoted(&p, &rhs_str_owned, &rhs_len);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, rc_ws3);
    }
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
  } else if ((p[0] == '$' || p[0] == '@') && (p[1] == '.' || p[1] == '[')) {
    const JSON_Value *rval = NULL;
    const JSON_Value *rhs_base = (p[0] == '$') ? root : elem;
    ++p;
    rc = cdd_json_filter_resolve_path(rhs_base, &p, &rval);
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
    if (rhs_str_owned) {
      return cdd_json_pointer_token_free(rhs_str_owned);
    }
    return CDD_C_SUCCESS;
  }

  {
    int matched = 0;
    if (fval) {
      if (fn_kind == 1) {
        if (rhs_type == JSONNumber) {
          if (fval->type == JSONString)
            matched =
                cdd_json_double_op_cmp(op, (double)fval->string_len, rhs_num);
          else if (fval->type == JSONArray)
            matched = cdd_json_double_op_cmp(op, (double)fval->array_val->count,
                                             rhs_num);
          else if (fval->type == JSONObject)
            matched = cdd_json_double_op_cmp(
                op, (double)fval->object_val->count, rhs_num);
        }
      } else if (rhs_type == JSONString && fval->type == JSONString) {
        int eq = (fval->string_len == rhs_len &&
                  memcmp(fval->string_val, rhs_str, rhs_len) == 0);
        matched = (op == 1) ? eq : !eq;
      } else if (rhs_type == JSONNumber && fval->type == JSONNumber) {
        matched = cdd_json_double_op_cmp(op, fval->number_val, rhs_num);
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
  if (rhs_str_owned) {
    return cdd_json_pointer_token_free(rhs_str_owned);
  }
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
  while (*p == ' ' || *p == '\t' || *p == '!') {
    if (*p == '!')
      neg = !neg;
    ++p;
  }
  if (*p == '(') {
    cdd_c_error_t rc_ws;
    ++p;
    rc = cdd_json_eval_filter_expr(root, elem, &p, &sub);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc_ws = cdd_json_filter_skip_ws(&p);
    if (*p != ')') {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc_ws);
    }
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
    int is_and;
    int rhs = 0;
    cdd_c_error_t rc_ws = cdd_json_filter_skip_ws(&p);
    if (strncmp(p, "&&", 2) != 0 && strncmp(p, "||", 2) != 0) {
      *cursor = p;
      *out_res = lhs;
      return rc_ws;
    }
    is_and = (p[0] == '&');
    p += 2;
    rc = cdd_json_eval_filter_unary(root, elem, &p, &rhs);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_and)
      lhs = (lhs && rhs) ? 1 : 0;
    else
      lhs = (lhs || rhs) ? 1 : 0;
  }
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
  cdd_c_error_t rc_ws;

  rc = cdd_json_eval_filter_expr(root, NULL, &p, &dummy);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc_ws = cdd_json_filter_skip_ws(&p);
  if (*p != ']') {
    return cdd_c_keep_err(CDD_C_ERROR_NOT_SUPPORTED, rc_ws);
  }
  *cursor = p + 1;

  if (arr_val->type != JSONArray)
    return CDD_C_ERROR_NOT_FOUND;

  for (i = 0; i < arr_val->array_val->count; ++i) {
    const JSON_Value *elem = arr_val->array_val->items[i];
    const char *ep = expr_start;
    int matched = 0;
    rc = cdd_json_eval_filter_expr(root, elem, &ep, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (matched) {
      *out_matched = elem;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
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
  size_t i, cnt;
  JSON_Value *const *items;
  if (node->type == JSONObject) {
    cnt = node->object_val->count;
    items = node->object_val->values;
  } else if (node->type == JSONArray) {
    cnt = node->array_val->count;
    items = node->array_val->items;
  } else {
    return CDD_C_ERROR_NOT_FOUND;
  }
  if (strcmp(key, "*") == 0) {
    if (cnt > 0) {
      *out_found = items[0];
      return CDD_C_SUCCESS;
    }
    return CDD_C_ERROR_NOT_FOUND;
  }
  if (node->type == JSONObject) {
    JSON_Value *child = NULL;
    cdd_c_error_t rc = cdd_json_object_get_value(node->object_val, key, &child);
    if (rc == CDD_C_SUCCESS) {
      *out_found = child;
      return rc;
    }
  }
  for (i = 0; i < cnt; ++i) {
    cdd_c_error_t rc = cdd_json_find_recursive_first(items[i], key, out_found);
    if (rc == CDD_C_SUCCESS)
      return rc;
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
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_array_append_owned(arr, copy);
}

/**
 * @brief Appends a deep copy of `node[kstart..kstart+klen]` (if present on a
 * JSONObject) into `out_arr`.
 *
 * @param[in] node Candidate JSON_Value node.
 * @param[in] kstart Property name bytes.
 * @param[in] klen Property name byte length.
 * @param[in,out] out_arr Destination array.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_nodelist_append_prop(const JSON_Value *node,
                                                   const char *kstart,
                                                   size_t klen,
                                                   JSON_Array *out_arr) {
  if (node->type == JSONObject) {
    JSON_Value *child = NULL;
    cdd_c_error_t rc =
        cdd_json_object_getn_value(node->object_val, kstart, klen, &child);
    if (rc == CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_nodelist_append(out_arr, child));
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends all immediate children of `node` (if JSONObject or JSONArray)
 * to `out_arr`.
 *
 * @param[in] node Node whose children should be appended.
 * @param[in,out] out_arr Destination array.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_nodelist_append_children(const JSON_Value *node,
                                                       JSON_Array *out_arr) {
  size_t i, cnt;
  JSON_Value *const *items;
  if (node->type == JSONObject) {
    cnt = node->object_val->count;
    items = node->object_val->values;
  } else if (node->type == JSONArray) {
    cnt = node->array_val->count;
    items = node->array_val->items;
  } else {
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < cnt; ++i) {
    cdd_c_error_t rc = cdd_json_nodelist_append(out_arr, items[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Recursively collects all descendants matching `key[0..key_len-1]` (or
 * all children if `key` is NULL) into `out_arr`.
 *
 * @param[in] node Current node.
 * @param[in] key Key bytes or NULL for wildcard.
 * @param[in] key_len Byte length of `key`.
 * @param[in,out] out_arr Output JSON_Array receiving deep copies.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_collect_recursive(const JSON_Value *node,
                                                const char *key, size_t key_len,
                                                JSON_Array *out_arr) {
  size_t i, cnt;
  JSON_Value *const *items;
  cdd_c_error_t rc;
  if (!key) {
    rc = cdd_json_nodelist_append_children(node, out_arr);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    rc = cdd_json_nodelist_append_prop(node, key, key_len, out_arr);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (node->type == JSONObject) {
    cnt = node->object_val->count;
    items = node->object_val->values;
  } else if (node->type == JSONArray) {
    cnt = node->array_val->count;
    items = node->array_val->items;
  } else {
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < cnt; ++i) {
    rc = cdd_json_collect_recursive(items[i], key, key_len, out_arr);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
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
      if (endp == p || *out_step == 0)
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
  cdd_c_error_t rc_free = CDD_C_SUCCESS;

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
      cdd_c_error_t rc;

      ++p;
      if (*p == '.') {
        is_rec = 1;
        ++p;
      }
      if (*p == '*') {
        ++p;
        if (is_rec) {
          const JSON_Value *found = NULL;
          rc = cdd_json_find_recursive_first(cur, "*", &found);
          if (rc != CDD_C_SUCCESS) {
            return cdd_c_keep_err(rc, rc_free);
          }
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
        if (!cur) {
          return cdd_c_keep_err(CDD_C_ERROR_NOT_FOUND, rc_free);
        }
        continue;
      }

      start = p;
      while (*p != '\0' && *p != '.' && *p != '[') {
        ++p;
        ++len;
      }
      if (len == 0) {
        return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc_free);
      }

      rc = cdd_json_ctx_alloc_fn(len + 1, (void **)&key);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, rc_free);
      }
      memcpy(key, start, len);
      key[len] = '\0';
      if (is_rec) {
        const JSON_Value *found = NULL;
        rc = cdd_json_find_recursive_first(cur, key, &found);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, cdd_c_keep_err(rc_free, cdd_json_pointer_token_free(key)));
        }
        rc_free = cdd_json_pointer_token_free(key);
        cur = found;
        continue;
      }
      if (cur->type != JSONObject) {
        return cdd_c_keep_err(
            CDD_C_ERROR_NOT_FOUND,
            cdd_c_keep_err(rc_free, cdd_json_pointer_token_free(key)));
      }
      rc = cdd_json_object_get_value(cur->object_val, key, &child);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(
            rc, cdd_c_keep_err(rc_free, cdd_json_pointer_token_free(key)));
      }
      rc_free = cdd_json_pointer_token_free(key);
      cur = child;
    } else if (*p == '[') {
      ++p;
      if (*p == '?') {
        const JSON_Value *matched = NULL;
        cdd_c_error_t rc;
        ++p;
        rc = cdd_json_eval_filter_selector(root, cur, &p, &matched);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc, rc_free);
        }
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
        if (!cur) {
          return cdd_c_keep_err(CDD_C_ERROR_NOT_FOUND, rc_free);
        }
      } else if (*p == 39 || *p == '"') {
        size_t klen = 0;
        char *key = NULL;
        JSON_Value *child = NULL;
        cdd_c_error_t rc = cdd_json_parse_jp_quoted(&p, &key, &klen);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc, rc_free);
        }
        if (*p != ']') {
          return cdd_c_keep_err(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_c_keep_err(rc_free, cdd_json_pointer_token_free(key)));
        }
        ++p;
        if (cur->type != JSONObject) {
          return cdd_c_keep_err(
              CDD_C_ERROR_NOT_FOUND,
              cdd_c_keep_err(rc_free, cdd_json_pointer_token_free(key)));
        }
        rc = cdd_json_object_getn_value(cur->object_val, key, klen, &child);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, cdd_c_keep_err(rc_free, cdd_json_pointer_token_free(key)));
        }
        rc_free = cdd_json_pointer_token_free(key);
        cur = child;
      } else {
        char *endp = NULL;
        int has_start = 0;
        long start_idx = 0;
        if (*p != ':') {
          start_idx = strtol(p, &endp, 10);
          if (endp == p) {
            return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc_free);
          }
          has_start = 1;
          p = endp;
        }
        if (*p == ']') {
          size_t arr_cnt;
          size_t eff_idx;
          if (cur->type != JSONArray) {
            return cdd_c_keep_err(CDD_C_ERROR_NOT_FOUND, rc_free);
          }
          arr_cnt = cur->array_val->count;
          if (start_idx < 0) {
            long pos_idx = (long)arr_cnt + start_idx;
            if (pos_idx < 0) {
              return cdd_c_keep_err(CDD_C_ERROR_NOT_FOUND, rc_free);
            }
            eff_idx = (size_t)pos_idx;
          } else {
            eff_idx = (size_t)start_idx;
          }
          if (eff_idx >= arr_cnt) {
            return cdd_c_keep_err(CDD_C_ERROR_NOT_FOUND, rc_free);
          }
          cur = cur->array_val->items[eff_idx];
          ++p;
        } else if (*p == ':') {
          int has_end = 0;
          long end_idx = 0;
          long step = 1;
          long arr_cnt;
          long s, e;
          cdd_c_error_t rc;
          ++p;
          rc = cdd_json_parse_slice_rest(&p, &has_end, &end_idx, &step);
          if (rc != CDD_C_SUCCESS) {
            return cdd_c_keep_err(rc, rc_free);
          }
          if (*p != ']') {
            return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc_free);
          }
          ++p;
          if (cur->type != JSONArray) {
            return cdd_c_keep_err(CDD_C_ERROR_NOT_FOUND, rc_free);
          }
          arr_cnt = (long)cur->array_val->count;
          if (step > 0) {
            s = has_start ? (start_idx < 0 ? arr_cnt + start_idx : start_idx)
                          : 0;
            e = has_end ? (end_idx < 0 ? arr_cnt + end_idx : end_idx) : arr_cnt;
            if (s < 0)
              s = 0;
            if (e > arr_cnt)
              e = arr_cnt;
            if (s >= e) {
              return cdd_c_keep_err(CDD_C_ERROR_NOT_FOUND, rc_free);
            }
            cur = cur->array_val->items[(size_t)s];
          } else {
            s = has_start ? (start_idx < 0 ? arr_cnt + start_idx : start_idx)
                          : (arr_cnt - 1);
            e = has_end ? (end_idx < 0 ? arr_cnt + end_idx : end_idx) : -1;
            if (s >= arr_cnt)
              s = arr_cnt - 1;
            if (e < -1)
              e = -1;
            if (s <= e) {
              return cdd_c_keep_err(CDD_C_ERROR_NOT_FOUND, rc_free);
            }
            cur = cur->array_val->items[(size_t)s];
          }
        } else {
          return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc_free);
        }
      }
    } else {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc_free);
    }
  }

  *out_val = cur;
  return rc_free;
}

/**
 * @brief Executes a single segment of `cdd_json_jsonpath_query`, populating
 * `next_arr` from `src_arr`.
 *
 * @param[in] root Root document node.
 * @param[in] src_arr Current nodelist array.
 * @param[in,out] io_p JSONPath cursor.
 * @param[in,out] next_arr Destination nodelist array.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_jsonpath_query_step(const JSON_Value *root,
                                                  const JSON_Array *src_arr,
                                                  const char **io_p,
                                                  JSON_Array *next_arr) {
  const char *p = *io_p;
  size_t ni;
  cdd_c_error_t rc;

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
      *io_p = p;
      for (ni = 0; ni < src_arr->count; ++ni) {
        const JSON_Value *node = src_arr->items[ni];
        if (is_rec) {
          rc = cdd_json_collect_recursive(node, NULL, 0, next_arr);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else {
          rc = cdd_json_nodelist_append_children(node, next_arr);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
      return CDD_C_SUCCESS;
    }
    start = p;
    while (*p != '\0' && *p != '.' && *p != '[') {
      ++p;
      ++len;
    }
    if (len == 0)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    *io_p = p;
    for (ni = 0; ni < src_arr->count; ++ni) {
      const JSON_Value *node = src_arr->items[ni];
      if (is_rec) {
        rc = cdd_json_collect_recursive(node, start, len, next_arr);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else {
        rc = cdd_json_nodelist_append_prop(node, start, len, next_arr);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    return CDD_C_SUCCESS;
  }

  if (*p != '[')
    return CDD_C_ERROR_INVALID_ARGUMENT;
  ++p;

  if (*p == '?') {
    const char *expr_start;
    int dummy = 0;
    cdd_c_error_t rc_ws;
    ++p;
    expr_start = p;
    rc = cdd_json_eval_filter_expr(root, NULL, &p, &dummy);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc_ws = cdd_json_filter_skip_ws(&p);
    if (*p != ']') {
      return cdd_c_keep_err(CDD_C_ERROR_NOT_SUPPORTED, rc_ws);
    }
    *io_p = p + 1;
    for (ni = 0; ni < src_arr->count; ++ni) {
      const JSON_Value *node = src_arr->items[ni];
      if (node->type == JSONArray) {
        size_t j;
        for (j = 0; j < node->array_val->count; ++j) {
          const JSON_Value *elem = node->array_val->items[j];
          const char *ep = expr_start;
          int matched = 0;
          rc = cdd_json_eval_filter_expr(root, elem, &ep, &matched);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (matched) {
            rc = cdd_json_nodelist_append(next_arr, elem);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }
      }
    }
    return CDD_C_SUCCESS;
  }

  if (strncmp(p, "*]", 2) == 0) {
    *io_p = p + 2;
    for (ni = 0; ni < src_arr->count; ++ni) {
      rc = cdd_json_nodelist_append_children(src_arr->items[ni], next_arr);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return CDD_C_SUCCESS;
  }

  while (*p != ']') {
    cdd_c_error_t rc_ws1;
    cdd_c_error_t rc_ws2;
    rc_ws1 = cdd_json_filter_skip_ws(&p);
    if (*p == 39 || *p == '"') {
      size_t klen = 0;
      char *key = NULL;
      rc = cdd_json_parse_jp_quoted(&p, &key, &klen);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, rc_ws1);
      }
      for (ni = 0; ni < src_arr->count; ++ni) {
        rc = cdd_json_nodelist_append_prop(src_arr->items[ni], key, klen,
                                           next_arr);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc, cdd_json_pointer_token_free(key));
        }
      }
      rc_ws2 = cdd_json_pointer_token_free(key);
    } else {
      char *endp = NULL;
      int has_start = 0;
      long idx_val = 0;
      if (*p != ':') {
        idx_val = strtol(p, &endp, 10);
        if (endp == p)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        has_start = 1;
        p = endp;
      }
      if (*p == ':') {
        int has_end = 0;
        long end_idx = 0;
        long step = 1;
        ++p;
        rc = cdd_json_parse_slice_rest(&p, &has_end, &end_idx, &step);
        if (rc != CDD_C_SUCCESS)
          return rc;
        for (ni = 0; ni < src_arr->count; ++ni) {
          const JSON_Value *node = src_arr->items[ni];
          if (node->type == JSONArray) {
            long cnt = (long)node->array_val->count;
            long s, e, k;
            if (step > 0) {
              s = has_start ? (idx_val < 0 ? cnt + idx_val : idx_val) : 0;
              e = has_end ? (end_idx < 0 ? cnt + end_idx : end_idx) : cnt;
              if (s < 0)
                s = 0;
              if (e > cnt)
                e = cnt;
              for (k = s; k < e; k += step) {
                rc = cdd_json_nodelist_append(
                    next_arr, node->array_val->items[(size_t)k]);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            } else {
              s = has_start ? (idx_val < 0 ? cnt + idx_val : idx_val)
                            : (cnt - 1);
              e = has_end ? (end_idx < 0 ? cnt + end_idx : end_idx) : -1;
              if (s >= cnt)
                s = cnt - 1;
              if (e < -1)
                e = -1;
              for (k = s; k > e; k += step) {
                rc = cdd_json_nodelist_append(
                    next_arr, node->array_val->items[(size_t)k]);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            }
          }
        }
      } else {
        for (ni = 0; ni < src_arr->count; ++ni) {
          const JSON_Value *node = src_arr->items[ni];
          if (node->type == JSONArray) {
            long cnt = (long)node->array_val->count;
            long eff = idx_val < 0 ? cnt + idx_val : idx_val;
            if (eff >= 0 && eff < cnt) {
              rc = cdd_json_nodelist_append(
                  next_arr, node->array_val->items[(size_t)eff]);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          }
        }
      }
      rc_ws2 = CDD_C_SUCCESS;
    }
    rc = cdd_json_filter_skip_ws(&p);
    if (*p != ',' && *p != ']') {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                            cdd_c_keep_err(rc, cdd_c_keep_err(rc_ws1, rc_ws2)));
    }
    if (*p == ',')
      ++p;
  }
  *io_p = p + 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees all child `JSON_Value` items and the backing `items` buffer of a
 * `JSON_Array` without freeing the `JSON_Array` struct itself.
 *
 * @param[in,out] arr Target JSON_Array.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_array_items_free(JSON_Array *arr) {
  size_t di;
  cdd_c_error_t rc_ign = CDD_C_SUCCESS;
  for (di = 0; di < arr->count; ++di) {
    rc_ign = cdd_json_value_free(arr->items[di]);
  }
  if (arr->items) {
    rc_ign = cdd_json_dealloc_free((void *)arr->items);
  }
  return rc_ign;
}

cdd_c_error_t cdd_json_jsonpath_query(const JSON_Value *root,
                                      const char *jsonpath,
                                      JSON_Value **out_array) {
  const char *p;
  JSON_Value *cur_list = NULL;
  cdd_c_error_t rc;
  cdd_c_error_t rc_ign = CDD_C_SUCCESS;

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
    return cdd_c_keep_err(rc, cdd_json_value_free(cur_list));
  }

  p = jsonpath + 1;
  while (*p != '\0') {
    JSON_Array next_arr;
    memset(&next_arr, 0, sizeof(next_arr));
    next_arr.wrapping_value = cur_list;

    rc = cdd_json_jsonpath_query_step(root, cur_list->array_val, &p, &next_arr);
    if (rc != CDD_C_SUCCESS) {
      rc_ign = cdd_json_array_items_free(&next_arr);
      return cdd_c_keep_err(
          rc, cdd_c_keep_err(rc_ign, cdd_json_value_free(cur_list)));
    }
    rc_ign = cdd_json_array_items_free(cur_list->array_val);
    *cur_list->array_val = next_arr;
  }

  *out_array = cur_list;
  return rc_ign;
}

/**
 * @brief Removes `key` from `obj` if present, ignoring missing keys.
 *
 * @param[in,out] obj Target JSON_Object.
 * @param[in] key Key to remove.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_merge_patch_remove_opt(JSON_Object *obj,
                                                     const char *key) {
  cdd_c_error_t rc = cdd_json_object_remove(obj, key);
  if (rc == CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Applies a single RFC 7396 merge-patch property onto `res_obj`.
 *
 * @param[in,out] res_obj Destination JSON_Object.
 * @param[in] key Property name.
 * @param[in] pval Patch property value.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t cdd_json_merge_patch_entry(JSON_Object *res_obj,
                                                const char *key,
                                                const JSON_Value *pval) {
  JSON_Value *orig_child = NULL;
  JSON_Value *merged_child = NULL;
  size_t k, klen;
  cdd_c_error_t rc;
  if (pval->type == JSONNull) {
    return cdd_json_merge_patch_remove_opt(res_obj, key);
  }
  klen = strlen(key);
  for (k = 0; k < res_obj->count; ++k) {
    if (res_obj->name_lens[k] == klen &&
        memcmp(res_obj->names[k], key, klen) == 0) {
      orig_child = res_obj->values[k];
      break;
    }
  }
  rc = cdd_json_merge_patch(orig_child, pval, &merged_child);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_set_owned(res_obj, key, merged_child);
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
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    rc = cdd_json_value_init_object(&result);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  for (i = 0; i < patch->object_val->count; ++i) {
    rc = cdd_json_merge_patch_entry(result->object_val,
                                    patch->object_val->names[i],
                                    patch->object_val->values[i]);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(result));
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
  cdd_c_error_t rc_free = CDD_C_SUCCESS;
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
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_dealloc_free(parent_ptr));
    }
    rc_free = cdd_json_dealloc_free(parent_ptr);
    *out_parent = (JSON_Value *)(size_t)cparent;
  }
  return cdd_c_keep_err(cdd_json_pointer_unescape_strict(
                            last_slash + 1, strlen(last_slash + 1), out_token),
                        rc_free);
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
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_pointer_token_free(token));
    }
    return cdd_json_pointer_token_free(token);
  }
  if (parent->type == JSONArray) {
    JSON_Array *arr = parent->array_val;
    char *endp = NULL;
    unsigned long idx;
    cdd_c_error_t rc_tok;
    if (strcmp(token, "-") == 0) {
      rc_tok = cdd_json_pointer_token_free(token);
      return cdd_c_keep_err(cdd_json_array_append_value(arr, val), rc_tok);
    }
    if (token[0] == '\0' || (token[0] == '0' && token[1] != '\0')) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                            cdd_json_pointer_token_free(token));
    }
    idx = strtoul(token, &endp, 10);
    if (*endp != '\0' || idx > arr->count) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                            cdd_json_pointer_token_free(token));
    }
    rc_tok = cdd_json_pointer_token_free(token);
    if (replace_existing && idx < arr->count) {
      return cdd_c_keep_err(cdd_json_array_replace_value(arr, (size_t)idx, val),
                            rc_tok);
    }
    return cdd_c_keep_err(cdd_json_array_insert_value(arr, (size_t)idx, val),
                          rc_tok);
  }
  return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_json_pointer_token_free(token));
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
  cdd_c_error_t rc;
  cdd_c_error_t rc_ign;
  rc = cdd_json_value_deep_copy(v_src, &v_copy);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (remove_from) {
    rc = cdd_json_pointer_remove(*root_ptr, remove_from);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(v_copy));
    }
  }
  if (path[0] == '\0') {
    rc_ign = cdd_json_value_free(*root_ptr);
    *root_ptr = v_copy;
    return rc_ign;
  }
  rc = cdd_json_pointer_set_internal(*root_ptr, path, v_copy, 0);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(v_copy));
  }
  return CDD_C_SUCCESS;
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
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_pointer_token_free(token));
    }
    return cdd_json_pointer_token_free(token);
  }
  if (parent->type == JSONArray) {
    char *endp = NULL;
    unsigned long idx;
    cdd_c_error_t rc_tok;
    if (token[0] == '\0' || (token[0] == '0' && token[1] != '\0')) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                            cdd_json_pointer_token_free(token));
    }
    idx = strtoul(token, &endp, 10);
    if (*endp != '\0' || idx >= parent->array_val->count) {
      return cdd_c_keep_err(CDD_C_ERROR_NOT_FOUND,
                            cdd_json_pointer_token_free(token));
    }
    rc_tok = cdd_json_pointer_token_free(token);
    return cdd_c_keep_err(cdd_json_array_remove(parent->array_val, (size_t)idx),
                          rc_tok);
  }
  return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_json_pointer_token_free(token));
}

/**
 * @brief Executes a single RFC 6902 patch operation object against `*cur_ptr`.
 *
 * @param[in,out] cur_ptr Pointer to active document root.
 * @param[in] op_val Patch operation object.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t cdd_json_apply_patch_op(JSON_Value **cur_ptr,
                                             const JSON_Value *op_val) {
  const char *op = NULL;
  const char *path = NULL;
  cdd_c_error_t rc;
  if (op_val->type != JSONObject)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_object_get_string(op_val->object_val, "op", &op);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  rc = cdd_json_object_get_string(op_val->object_val, "path", &path);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  if (strcmp(op, "test") == 0) {
    const JSON_Value *target_val = NULL;
    JSON_Value *expected_val = NULL;
    int eq = 0;
    rc = cdd_json_object_get_value(op_val->object_val, "value", &expected_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    rc = cdd_json_pointer_get(*cur_ptr, path, &target_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    rc = cdd_json_value_equals(target_val, expected_val, &eq);
    if (!eq) {
      return cdd_c_keep_err(rc, CDD_C_ERROR_INVALID_ARGUMENT);
    }
    return rc;
  }
  if (strcmp(op, "add") == 0 || strcmp(op, "replace") == 0) {
    JSON_Value *v_src = NULL;
    rc = cdd_json_object_get_value(op_val->object_val, "value", &v_src);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    if (strcmp(op, "replace") == 0 && path[0] != '\0') {
      rc = cdd_json_pointer_remove(*cur_ptr, path);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return cdd_json_patch_add_path(cur_ptr, path, v_src, NULL);
  }
  if (strcmp(op, "remove") == 0) {
    return cdd_json_pointer_remove(*cur_ptr, path);
  }
  if (strcmp(op, "copy") == 0 || strcmp(op, "move") == 0) {
    const char *from_path = NULL;
    const JSON_Value *from_val = NULL;
    size_t from_len;
    const char *rem;
    rc = cdd_json_object_get_string(op_val->object_val, "from", &from_path);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    if (strcmp(op, "move") == 0 && from_path[0] == '\0')
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = cdd_json_pointer_get(*cur_ptr, from_path, &from_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    from_len = strlen(from_path);
    if (strcmp(op, "move") == 0 && strncmp(path, from_path, from_len) == 0 &&
        path[from_len] == '/') {
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    rem = (strcmp(op, "move") == 0 && strcmp(from_path, path) != 0) ? from_path
                                                                    : NULL;
    return cdd_json_patch_add_path(cur_ptr, path, from_val, rem);
  }
  return CDD_C_ERROR_INVALID_ARGUMENT;
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

  for (i = 0; i < patch_arr->array_val->count; ++i) {
    rc = cdd_json_apply_patch_op(&cur, patch_arr->array_val->items[i]);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(cur));
    }
  }
  *out = cur;
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends a single RFC 6902 patch operation object (`op`, `path`, and
 * optional deep-copied `value`) to `patch_arr`.
 *
 * @param[in,out] patch_arr Target JSON_Array of patch operations.
 * @param[in] op Operation name (`"add"`, `"remove"`, `"replace"`).
 * @param[in] path JSON Pointer path string.
 * @param[in] val Optional value for `"add"` or `"replace"` (NULL for
 * `"remove"`).
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t cdd_json_diff_emit_op(JSON_Array *patch_arr,
                                           const char *op, const char *path,
                                           const JSON_Value *val) {
  JSON_Value *op_val = NULL;
  cdd_c_error_t rc = cdd_json_value_init_object(&op_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_object_set_string(op_val->object_val, "op", op);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(op_val));
  }
  rc = cdd_json_object_set_string(op_val->object_val, "path", path);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(op_val));
  }
  if (val) {
    JSON_Value *vcopy = NULL;
    rc = cdd_json_value_deep_copy(val, &vcopy);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(op_val));
    }
    rc = cdd_json_object_set_value(op_val->object_val, "value", vcopy);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(vcopy),
                                               cdd_json_value_free(op_val)));
    }
  }
  rc = cdd_json_array_append_value(patch_arr, op_val);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(op_val));
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Builds a child JSON Pointer path `<base_path>/<escaped_token>`.
 *
 * @param[in] base_path Parent JSON Pointer string (empty string for root).
 * @param[in] token Unescaped child key or index string.
 * @param[out] out_child Newly allocated child JSON Pointer string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t cdd_json_diff_join_pointer(const char *base_path,
                                                const char *token,
                                                char **out_child) {
  size_t base_len = strlen(base_path);
  size_t tok_len = strlen(token);
  size_t esc_cap = tok_len * 2 + 2;
  char *buf = NULL;
  cdd_c_error_t rc = cdd_json_alloc_fn(base_len + 1 + esc_cap, (void **)&buf);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (base_len > 0)
    memcpy(buf, base_path, base_len);
  buf[base_len] = '/';
  *out_child = buf;
  return cdd_json_pointer_escape(token, buf + base_len + 1, esc_cap);
}

/**
 * @brief Recursively diffs `src` and `dst` at `base_path`, appending RFC 6902
 * operations to `patch_arr`.
 *
 * @param[in] src Source JSON_Value node.
 * @param[in] dst Target JSON_Value node.
 * @param[in] base_path Current JSON Pointer string.
 * @param[in,out] patch_arr Output JSON_Array of patch operations.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t cdd_json_diff_walk(const JSON_Value *src,
                                        const JSON_Value *dst,
                                        const char *base_path,
                                        JSON_Array *patch_arr) {
  int eq = 0;
  size_t i;
  cdd_c_error_t rc_free = CDD_C_SUCCESS;
  cdd_c_error_t rc = cdd_json_value_equals(src, dst, &eq);
  if (rc != CDD_C_SUCCESS || eq)
    return rc;

  if (src->type == JSONObject && dst->type == JSONObject) {
    for (i = 0; i < src->object_val->count; ++i) {
      const char *k = src->object_val->names[i];
      JSON_Value *dv = NULL;
      cdd_c_error_t get_rc = cdd_json_object_get_value(dst->object_val, k, &dv);
      if (get_rc != CDD_C_SUCCESS) {
        char *cpath = NULL;
        rc = cdd_json_diff_join_pointer(base_path, k, &cpath);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc, cdd_c_keep_err(get_rc, rc_free));
        }
        rc = cdd_json_diff_emit_op(patch_arr, "remove", cpath, NULL);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, cdd_c_keep_err(rc_free, cdd_json_dealloc_free(cpath)));
        }
        rc_free =
            (cdd_c_error_t)((int)rc_free | (int)cdd_json_dealloc_free(cpath));
      }
    }
    for (i = 0; i < dst->object_val->count; ++i) {
      const char *k = dst->object_val->names[i];
      const JSON_Value *dv = dst->object_val->values[i];
      JSON_Value *sv = NULL;
      char *cpath = NULL;
      cdd_c_error_t get_rc;
      rc = cdd_json_diff_join_pointer(base_path, k, &cpath);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, rc_free);
      }
      get_rc = cdd_json_object_get_value(src->object_val, k, &sv);
      if (get_rc != CDD_C_SUCCESS) {
        rc = cdd_json_diff_emit_op(patch_arr, "add", cpath, dv);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, cdd_c_keep_err(
                      get_rc,
                      cdd_c_keep_err(rc_free, cdd_json_dealloc_free(cpath))));
        }
        rc_free =
            (cdd_c_error_t)((int)rc_free | (int)cdd_json_dealloc_free(cpath));
      } else {
        rc = cdd_json_diff_walk(sv, dv, cpath, patch_arr);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, cdd_c_keep_err(rc_free, cdd_json_dealloc_free(cpath)));
        }
        rc_free =
            (cdd_c_error_t)((int)rc_free | (int)cdd_json_dealloc_free(cpath));
      }
    }
    return rc_free;
  }

  if (src->type == JSONArray && dst->type == JSONArray) {
    size_t scnt = src->array_val->count;
    size_t dcnt = dst->array_val->count;
    size_t common = scnt < dcnt ? scnt : dcnt;
    char ibuf[32];
    for (i = 0; i < common; ++i) {
      char *cpath = NULL;
#if defined(_MSC_VER)
      sprintf_s(ibuf, sizeof(ibuf), "%lu", (unsigned long)i);
#else
      sprintf(ibuf, "%lu", (unsigned long)i);
#endif
      rc = cdd_json_diff_join_pointer(base_path, ibuf, &cpath);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, rc_free);
      }
      rc = cdd_json_diff_walk(src->array_val->items[i],
                              dst->array_val->items[i], cpath, patch_arr);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(
            rc, cdd_c_keep_err(rc_free, cdd_json_dealloc_free(cpath)));
      }
      rc_free =
          (cdd_c_error_t)((int)rc_free | (int)cdd_json_dealloc_free(cpath));
    }
    if (dcnt > scnt) {
      char *cpath = NULL;
      rc = cdd_json_diff_join_pointer(base_path, "-", &cpath);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, rc_free);
      }
      for (i = scnt; i < dcnt; ++i) {
        rc = cdd_json_diff_emit_op(patch_arr, "add", cpath,
                                   dst->array_val->items[i]);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, cdd_c_keep_err(rc_free, cdd_json_dealloc_free(cpath)));
        }
      }
      rc_free =
          (cdd_c_error_t)((int)rc_free | (int)cdd_json_dealloc_free(cpath));
    } else if (scnt > dcnt) {
      for (i = scnt; i > dcnt; --i) {
        char *cpath = NULL;
#if defined(_MSC_VER)
        sprintf_s(ibuf, sizeof(ibuf), "%lu", (unsigned long)(i - 1));
#else
        sprintf(ibuf, "%lu", (unsigned long)(i - 1));
#endif
        rc = cdd_json_diff_join_pointer(base_path, ibuf, &cpath);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc, rc_free);
        }
        rc = cdd_json_diff_emit_op(patch_arr, "remove", cpath, NULL);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, cdd_c_keep_err(rc_free, cdd_json_dealloc_free(cpath)));
        }
        rc_free =
            (cdd_c_error_t)((int)rc_free | (int)cdd_json_dealloc_free(cpath));
      }
    }
    return rc_free;
  }

  return cdd_json_diff_emit_op(patch_arr, "replace", base_path, dst);
}

cdd_c_error_t cdd_json_diff_patch(const JSON_Value *src, const JSON_Value *dst,
                                  JSON_Value **out_patch) {
  JSON_Value *patch_val = NULL;
  cdd_c_error_t rc;
  if (!out_patch)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_patch = NULL;
  if (!src || !dst)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_value_init_array(&patch_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_diff_walk(src, dst, "", patch_val->array_val);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(patch_val));
  }
  *out_patch = patch_val;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if `instance` matches a single JSON Schema `"type"` keyword
 * string (`"null"`, `"boolean"`, `"object"`, `"array"`, `"number"`,
 * `"integer"`, `"string"`).
 *
 * @param[in] tname Type name string.
 * @param[in] instance Instance node.
 * @param[out] out_matches 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_schema_match_type(const char *tname,
                                                const JSON_Value *instance,
                                                int *out_matches) {
  *out_matches = 0;
  if (strcmp(tname, "null") == 0) {
    *out_matches = (instance->type == JSONNull);
  } else if (strcmp(tname, "boolean") == 0) {
    *out_matches = (instance->type == JSONBoolean);
  } else if (strcmp(tname, "object") == 0) {
    *out_matches = (instance->type == JSONObject);
  } else if (strcmp(tname, "array") == 0) {
    *out_matches = (instance->type == JSONArray);
  } else if (strcmp(tname, "string") == 0) {
    *out_matches = (instance->type == JSONString);
  } else if (strcmp(tname, "number") == 0) {
    *out_matches = (instance->type == JSONNumber);
  } else if (strcmp(tname, "integer") == 0) {
    if (instance->type == JSONNumber) {
      if (instance->num_kind != 0 ||
          (double)(int64_t)instance->number_val == instance->number_val) {
        *out_matches = 1;
      }
    }
  }
  return CDD_C_SUCCESS;
}

static cdd_c_error_t
cdd_json_schema_validate_with_root(const JSON_Value *root_schema,
                                   const JSON_Value *schema,
                                   const JSON_Value *instance);

/**
 * @brief Validates the `"type"` keyword value (`JSONString` or `JSONArray` of
 * strings) against `instance`.
 *
 * @param[in] kw `"type"` keyword value.
 * @param[in] instance Instance node.
 * @return CDD_C_SUCCESS if matched, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
static cdd_c_error_t cdd_json_schema_match_type_kw(const JSON_Value *kw,
                                                   const JSON_Value *instance) {
  int m = 0;
  size_t i;
  cdd_c_error_t rc;
  if (kw->type == JSONString) {
    rc = cdd_json_schema_match_type(kw->string_val, instance, &m);
    if (!m) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    return rc;
  }
  if (kw->type == JSONArray) {
    for (i = 0; i < kw->array_val->count; ++i) {
      const JSON_Value *titem = kw->array_val->items[i];
      if (titem->type == JSONString) {
        rc = cdd_json_schema_match_type(titem->string_val, instance, &m);
        if (m)
          return rc;
      }
    }
  }
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Validates the `"enum"` keyword array against `instance`.
 *
 * @param[in] kw `"enum"` keyword value.
 * @param[in] instance Instance node.
 * @return CDD_C_SUCCESS if matched, or error code.
 */
static cdd_c_error_t cdd_json_schema_match_enum_kw(const JSON_Value *kw,
                                                   const JSON_Value *instance) {
  size_t i;
  if (kw->type != JSONArray)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < kw->array_val->count; ++i) {
    int eq = 0;
    cdd_c_error_t rc =
        cdd_json_value_equals(instance, kw->array_val->items[i], &eq);
    if (eq)
      return rc;
  }
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Checks if any `"allOf"` subschema object defines `ikey` inside its
 * `"properties"` map.
 *
 * @param[in] sobj Parent schema object.
 * @param[in] ikey Instance property name.
 * @param[out] out_has Receives 1 if declared in an `"allOf"` subschema, 0
 * otherwise.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cdd_json_schema_allof_has_prop(const JSON_Object *sobj,
                                                    const char *ikey,
                                                    int *out_has) {
  JSON_Array *all_of_arr = NULL;
  size_t ai;
  cdd_c_error_t rc;
  *out_has = 0;
  rc = cdd_json_object_get_array(sobj, "allOf", &all_of_arr);
  if (rc == CDD_C_SUCCESS) {
    for (ai = 0; ai < all_of_arr->count; ++ai) {
      const JSON_Value *sub_s = all_of_arr->items[ai];
      if (sub_s->type == JSONObject) {
        JSON_Object *sub_props = NULL;
        cdd_c_error_t sp_rc = cdd_json_object_get_object(
            sub_s->object_val, "properties", &sub_props);
        if (sp_rc == CDD_C_SUCCESS) {
          JSON_Value *sub_p = NULL;
          cdd_c_error_t p_rc =
              cdd_json_object_get_value(sub_props, ikey, &sub_p);
          if (p_rc == CDD_C_SUCCESS) {
            *out_has = 1;
            return cdd_c_keep_err(rc, cdd_c_keep_err(sp_rc, p_rc));
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Validates `"anyOf"` subschemas against `instance`.
 *
 * @param[in] root_schema Root schema document.
 * @param[in] kw `"anyOf"` keyword value.
 * @param[in] instance Instance node.
 * @return CDD_C_SUCCESS if at least one subschema validates, or
 * CDD_C_ERROR_INVALID_ARGUMENT.
 */
static cdd_c_error_t cdd_json_schema_match_anyof(const JSON_Value *root_schema,
                                                 const JSON_Value *kw,
                                                 const JSON_Value *instance) {
  size_t i;
  if (kw->type != JSONArray)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < kw->array_val->count; ++i) {
    cdd_c_error_t rc = cdd_json_schema_validate_with_root(
        root_schema, kw->array_val->items[i], instance);
    if (rc == CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Validates `"oneOf"` subschemas against `instance`.
 *
 * @param[in] root_schema Root schema document.
 * @param[in] kw `"oneOf"` keyword value.
 * @param[in] instance Instance node.
 * @return CDD_C_SUCCESS if exactly one subschema validates, or
 * CDD_C_ERROR_INVALID_ARGUMENT.
 */
static cdd_c_error_t cdd_json_schema_match_oneof(const JSON_Value *root_schema,
                                                 const JSON_Value *kw,
                                                 const JSON_Value *instance) {
  size_t i;
  size_t ok_cnt = 0;
  if (kw->type != JSONArray)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < kw->array_val->count; ++i) {
    cdd_c_error_t rc = cdd_json_schema_validate_with_root(
        root_schema, kw->array_val->items[i], instance);
    if (rc == CDD_C_SUCCESS) {
      ++ok_cnt;
      if (ok_cnt > 1) {
        return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      }
    }
  }
  return (ok_cnt == 1) ? CDD_C_SUCCESS : CDD_C_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Internal recursive JSON Schema Draft 2020-12 validator carrying the
 * root schema document for `$ref` resolution.
 *
 * @param[in] root_schema Top-level root schema document.
 * @param[in] schema Current subschema node.
 * @param[in] instance Instance node to validate.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
static cdd_c_error_t
cdd_json_schema_validate_with_root(const JSON_Value *root_schema,
                                   const JSON_Value *schema,
                                   const JSON_Value *instance) {
  const JSON_Object *sobj;
  JSON_Value *kw = NULL;
  const char *ref_str = NULL;
  size_t i;
  cdd_c_error_t rc;
  cdd_c_error_t rc_ign = CDD_C_SUCCESS;

  if (!schema || !instance)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (schema->type == JSONBoolean) {
    return schema->boolean_val ? CDD_C_SUCCESS : CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (schema->type != JSONObject)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  sobj = schema->object_val;

  /* 0. "$ref" (local JSON Pointer against root_schema) */
  rc = cdd_json_object_get_string(sobj, "$ref", &ref_str);
  if (rc == CDD_C_SUCCESS) {
    const JSON_Value *target_schema = NULL;
    cdd_c_error_t ref_rc;
    if (strcmp(ref_str, "#") == 0) {
      target_schema = root_schema;
    } else if (ref_str[0] == '#' && ref_str[1] == '/') {
      ref_rc = cdd_json_pointer_get(root_schema, ref_str + 1, &target_schema);
      if (ref_rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                              cdd_c_keep_err(rc, ref_rc));
      }
    } else {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    ref_rc = cdd_json_schema_validate_with_root(root_schema, target_schema,
                                                instance);
    if (ref_rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(ref_rc, rc);
    }
  }

  /* 1. "type" (string or array of strings) */
  rc = cdd_json_object_get_value(sobj, "type", &kw);
  if (rc == CDD_C_SUCCESS) {
    cdd_c_error_t mt_rc = cdd_json_schema_match_type_kw(kw, instance);
    if (mt_rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(mt_rc, rc);
    }
  }

  /* 2. "const" */
  rc = cdd_json_object_get_value(sobj, "const", &kw);
  if (rc == CDD_C_SUCCESS) {
    int eq = 0;
    cdd_c_error_t eq_rc = cdd_json_value_equals(instance, kw, &eq);
    if (!eq) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                            cdd_c_keep_err(rc, eq_rc));
    }
  }

  /* 3. "enum" */
  rc = cdd_json_object_get_value(sobj, "enum", &kw);
  if (rc == CDD_C_SUCCESS) {
    cdd_c_error_t en_rc = cdd_json_schema_match_enum_kw(kw, instance);
    if (en_rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(en_rc, rc);
    }
  }

  /* 4. Numeric constraints */
  if (instance->type == JSONNumber) {
    double v = instance->number_val;
    double lim = 0.0;
    rc = cdd_json_object_get_number(sobj, "minimum", &lim);
    if (rc == CDD_C_SUCCESS && v < lim) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    rc = cdd_json_object_get_number(sobj, "maximum", &lim);
    if (rc == CDD_C_SUCCESS && v > lim) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    rc = cdd_json_object_get_number(sobj, "exclusiveMinimum", &lim);
    if (rc == CDD_C_SUCCESS && v <= lim) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    rc = cdd_json_object_get_number(sobj, "exclusiveMaximum", &lim);
    if (rc == CDD_C_SUCCESS && v >= lim) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    rc = cdd_json_object_get_number(sobj, "multipleOf", &lim);
    if (rc == CDD_C_SUCCESS) {
      double q, r_rem;
      if (lim <= 0.0) {
        return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      }
      q = v / lim;
      r_rem = q - (double)(int64_t)(q >= 0.0 ? (q + 0.5) : (q - 0.5));
      if (r_rem < -1e-9 || r_rem > 1e-9) {
        return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      }
    }
  }

  /* 5. String constraints */
  if (instance->type == JSONString) {
    double lim = 0.0;
    const char *pat = NULL;
    const char *c_enc = NULL;
    const char *c_mt = NULL;
    rc = cdd_json_object_get_number(sobj, "minLength", &lim);
    if (rc == CDD_C_SUCCESS && (double)instance->string_len < lim) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    rc = cdd_json_object_get_number(sobj, "maxLength", &lim);
    if (rc == CDD_C_SUCCESS && (double)instance->string_len > lim) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    rc = cdd_json_object_get_string(sobj, "pattern", &pat);
    if (rc == CDD_C_SUCCESS) {
      int matched = 0;
      cdd_c_error_t pat_rc =
          cdd_json_iregexp_search(pat, strlen(pat), instance->string_val,
                                  instance->string_len, &matched);
      if (pat_rc != CDD_C_SUCCESS || !matched) {
        return cdd_c_keep_err(pat_rc,
                              cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc));
      }
    }
    rc = cdd_json_object_get_string(sobj, "contentEncoding", &c_enc);
    if (rc == CDD_C_SUCCESS) {
      size_t si;
      int is_url = (strcmp(c_enc, "base64url") == 0);
      if (strcmp(c_enc, "base64") == 0 || is_url) {
        if (!is_url && (instance->string_len % 4) != 0) {
          return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
        }
        for (si = 0; si < instance->string_len; ++si) {
          char ch = instance->string_val[si];
          int valid_ch = ((ch >= 'A' && ch <= 'Z') ||
                          (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'));
          if (is_url) {
            valid_ch = valid_ch || (ch == '-' || ch == '_' || ch == '=');
          } else {
            valid_ch = valid_ch || (ch == '+' || ch == '/' || ch == '=');
          }
          if (!valid_ch) {
            return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
          }
        }
      } else {
        return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      }
    }
    rc = cdd_json_object_get_string(sobj, "contentMediaType", &c_mt);
    if (rc == CDD_C_SUCCESS) {
      if (strcmp(c_mt, "application/json") == 0) {
        JSON_Value *parsed_content = NULL;
        cdd_c_error_t p_rc = cdd_json_parse_buffer(
            instance->string_val, instance->string_len, &parsed_content);
        if (p_rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                                cdd_c_keep_err(rc, p_rc));
        }
        rc_ign = cdd_json_value_free(parsed_content);
      }
    }
  }

  /* 6. Array constraints */
  if (instance->type == JSONArray) {
    double lim = 0.0;
    size_t prefix_cnt = 0;
    int items_evaluated = 0;
    int uniq = 0;
    JSON_Array *pitems = NULL;
    JSON_Value *uneval_items = NULL;
    rc = cdd_json_object_get_number(sobj, "minItems", &lim);
    if (rc == CDD_C_SUCCESS && (double)instance->array_val->count < lim) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    rc = cdd_json_object_get_number(sobj, "maxItems", &lim);
    if (rc == CDD_C_SUCCESS && (double)instance->array_val->count > lim) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    rc = cdd_json_object_get_boolean(sobj, "uniqueItems", &uniq);
    if (rc == CDD_C_SUCCESS && uniq) {
      size_t j;
      for (i = 0; i < instance->array_val->count; ++i) {
        for (j = i + 1; j < instance->array_val->count; ++j) {
          int eq = 0;
          cdd_c_error_t eq_rc =
              cdd_json_value_equals(instance->array_val->items[i],
                                    instance->array_val->items[j], &eq);
          if (eq_rc != CDD_C_SUCCESS || eq) {
            return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                                  cdd_c_keep_err(rc, eq_rc));
          }
        }
      }
    }
    rc = cdd_json_object_get_array(sobj, "prefixItems", &pitems);
    if (rc == CDD_C_SUCCESS) {
      prefix_cnt = pitems->count;
      for (i = 0; i < prefix_cnt && i < instance->array_val->count; ++i) {
        cdd_c_error_t sub_rc = cdd_json_schema_validate_with_root(
            root_schema, pitems->items[i], instance->array_val->items[i]);
        if (sub_rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(sub_rc, rc);
        }
      }
    }
    rc = cdd_json_object_get_value(sobj, "items", &kw);
    if (rc == CDD_C_SUCCESS) {
      items_evaluated = 1;
      for (i = prefix_cnt; i < instance->array_val->count; ++i) {
        cdd_c_error_t sub_rc = cdd_json_schema_validate_with_root(
            root_schema, kw, instance->array_val->items[i]);
        if (sub_rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(sub_rc, rc);
        }
      }
    }
    if (!items_evaluated) {
      rc = cdd_json_object_get_value(sobj, "unevaluatedItems", &uneval_items);
      if (rc == CDD_C_SUCCESS) {
        for (i = prefix_cnt; i < instance->array_val->count; ++i) {
          cdd_c_error_t sub_rc = cdd_json_schema_validate_with_root(
              root_schema, uneval_items, instance->array_val->items[i]);
          if (sub_rc != CDD_C_SUCCESS) {
            return cdd_c_keep_err(sub_rc, rc);
          }
        }
      }
    }
  }

  /* 7. Object constraints */
  if (instance->type == JSONObject) {
    JSON_Array *req_arr = NULL;
    JSON_Object *props_obj = NULL;
    JSON_Object *pat_props_obj = NULL;
    JSON_Value *addl_val = NULL;
    JSON_Value *uneval_props = NULL;
    JSON_Value *pnames_val = NULL;
    JSON_Object *dep_req_obj = NULL;
    JSON_Object *dep_sch_obj = NULL;
    rc = cdd_json_object_get_array(sobj, "required", &req_arr);
    if (rc == CDD_C_SUCCESS) {
      for (i = 0; i < req_arr->count; ++i) {
        const char *rkey = NULL;
        JSON_Value *child = NULL;
        cdd_c_error_t r_rc = cdd_json_array_get_string(req_arr, i, &rkey);
        if (r_rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                                cdd_c_keep_err(rc, r_rc));
        }
        r_rc = cdd_json_object_get_value(instance->object_val, rkey, &child);
        if (r_rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                                cdd_c_keep_err(rc, r_rc));
        }
      }
    }
    rc = cdd_json_object_get_value(sobj, "propertyNames", &pnames_val);
    if (rc == CDD_C_SUCCESS) {
      for (i = 0; i < instance->object_val->count; ++i) {
        JSON_Value k_val;
        cdd_c_error_t sub_rc;
        memset(&k_val, 0, sizeof(k_val));
        k_val.type = JSONString;
        k_val.string_val = instance->object_val->names[i];
        k_val.string_len = instance->object_val->name_lens[i];
        sub_rc =
            cdd_json_schema_validate_with_root(root_schema, pnames_val, &k_val);
        if (sub_rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(sub_rc, rc);
        }
      }
    }
    rc = cdd_json_object_get_object(sobj, "dependentRequired", &dep_req_obj);
    if (rc == CDD_C_SUCCESS) {
      for (i = 0; i < dep_req_obj->count; ++i) {
        const char *dkey = dep_req_obj->names[i];
        JSON_Value *present_v = NULL;
        cdd_c_error_t p_rc =
            cdd_json_object_get_value(instance->object_val, dkey, &present_v);
        if (p_rc == CDD_C_SUCCESS) {
          const JSON_Value *darr_v = dep_req_obj->values[i];
          size_t di;
          if (darr_v->type != JSONArray) {
            return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                                  cdd_c_keep_err(rc, p_rc));
          }
          for (di = 0; di < darr_v->array_val->count; ++di) {
            const char *req_k = NULL;
            JSON_Value *dep_child = NULL;
            cdd_c_error_t dr_rc =
                cdd_json_array_get_string(darr_v->array_val, di, &req_k);
            if (dr_rc != CDD_C_SUCCESS) {
              return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                                    cdd_c_keep_err(rc, dr_rc));
            }
            dr_rc = cdd_json_object_get_value(instance->object_val, req_k,
                                              &dep_child);
            if (dr_rc != CDD_C_SUCCESS) {
              return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                                    cdd_c_keep_err(rc, dr_rc));
            }
          }
        }
      }
    }
    rc = cdd_json_object_get_object(sobj, "dependentSchemas", &dep_sch_obj);
    if (rc == CDD_C_SUCCESS) {
      for (i = 0; i < dep_sch_obj->count; ++i) {
        const char *dkey = dep_sch_obj->names[i];
        JSON_Value *present_v = NULL;
        cdd_c_error_t p_rc =
            cdd_json_object_get_value(instance->object_val, dkey, &present_v);
        if (p_rc == CDD_C_SUCCESS) {
          cdd_c_error_t sub_rc = cdd_json_schema_validate_with_root(
              root_schema, dep_sch_obj->values[i], instance);
          if (sub_rc != CDD_C_SUCCESS) {
            return cdd_c_keep_err(sub_rc, cdd_c_keep_err(rc, p_rc));
          }
        }
      }
    }
    for (i = 0; i < instance->object_val->count; ++i) {
      const char *ikey = instance->object_val->names[i];
      size_t ikey_len = instance->object_val->name_lens[i];
      const JSON_Value *ival = instance->object_val->values[i];
      JSON_Value *pschema = NULL;
      int prop_eval = 0;
      rc = cdd_json_object_get_object(sobj, "properties", &props_obj);
      if (rc == CDD_C_SUCCESS) {
        cdd_c_error_t p_rc =
            cdd_json_object_get_value(props_obj, ikey, &pschema);
        if (p_rc == CDD_C_SUCCESS) {
          cdd_c_error_t sub_rc;
          prop_eval = 1;
          sub_rc =
              cdd_json_schema_validate_with_root(root_schema, pschema, ival);
          if (sub_rc != CDD_C_SUCCESS) {
            return cdd_c_keep_err(sub_rc, cdd_c_keep_err(rc, p_rc));
          }
        }
      }
      rc =
          cdd_json_object_get_object(sobj, "patternProperties", &pat_props_obj);
      if (rc == CDD_C_SUCCESS) {
        size_t pi;
        for (pi = 0; pi < pat_props_obj->count; ++pi) {
          const char *ppat = pat_props_obj->names[pi];
          size_t pplen = pat_props_obj->name_lens[pi];
          int pmatched = 0;
          cdd_c_error_t pm_rc =
              cdd_json_iregexp_search(ppat, pplen, ikey, ikey_len, &pmatched);
          if (pm_rc != CDD_C_SUCCESS) {
            return cdd_c_keep_err(pm_rc, rc);
          }
          if (pmatched) {
            cdd_c_error_t sub_rc;
            prop_eval = 1;
            sub_rc = cdd_json_schema_validate_with_root(
                root_schema, pat_props_obj->values[pi], ival);
            if (sub_rc != CDD_C_SUCCESS) {
              return cdd_c_keep_err(sub_rc, rc);
            }
          }
        }
      }
      if (!prop_eval) {
        rc = cdd_json_object_get_value(sobj, "additionalProperties", &addl_val);
        if (rc == CDD_C_SUCCESS) {
          cdd_c_error_t sub_rc =
              cdd_json_schema_validate_with_root(root_schema, addl_val, ival);
          if (sub_rc != CDD_C_SUCCESS) {
            return cdd_c_keep_err(sub_rc, rc);
          }
        } else {
          rc = cdd_json_object_get_value(sobj, "unevaluatedProperties",
                                         &uneval_props);
          if (rc == CDD_C_SUCCESS) {
            int sub_eval = 0;
            cdd_c_error_t ao_rc =
                cdd_json_schema_allof_has_prop(sobj, ikey, &sub_eval);
            if (!sub_eval) {
              cdd_c_error_t sub_rc = cdd_json_schema_validate_with_root(
                  root_schema, uneval_props, ival);
              if (sub_rc != CDD_C_SUCCESS) {
                return cdd_c_keep_err(sub_rc, cdd_c_keep_err(rc, ao_rc));
              }
            }
          }
        }
      }
    }
  }

  /* 8. Conditional keywords ("if", "then", "else") */
  rc = cdd_json_object_get_value(sobj, "if", &kw);
  if (rc == CDD_C_SUCCESS) {
    JSON_Value *branch_kw = NULL;
    cdd_c_error_t if_rc =
        cdd_json_schema_validate_with_root(root_schema, kw, instance);
    if (if_rc == CDD_C_SUCCESS) {
      cdd_c_error_t th_rc = cdd_json_object_get_value(sobj, "then", &branch_kw);
      if (th_rc == CDD_C_SUCCESS) {
        cdd_c_error_t sub_rc = cdd_json_schema_validate_with_root(
            root_schema, branch_kw, instance);
        if (sub_rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              sub_rc, cdd_c_keep_err(rc, cdd_c_keep_err(if_rc, th_rc)));
        }
      }
    } else {
      cdd_c_error_t el_rc = cdd_json_object_get_value(sobj, "else", &branch_kw);
      if (el_rc == CDD_C_SUCCESS) {
        cdd_c_error_t sub_rc = cdd_json_schema_validate_with_root(
            root_schema, branch_kw, instance);
        if (sub_rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(sub_rc, cdd_c_keep_err(rc, el_rc));
        }
      }
    }
  }

  /* 9. Composition keywords ("allOf", "anyOf", "oneOf", "not") */
  rc = cdd_json_object_get_value(sobj, "allOf", &kw);
  if (rc == CDD_C_SUCCESS) {
    if (kw->type != JSONArray) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    }
    for (i = 0; i < kw->array_val->count; ++i) {
      cdd_c_error_t sub_rc = cdd_json_schema_validate_with_root(
          root_schema, kw->array_val->items[i], instance);
      if (sub_rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(sub_rc, rc);
      }
    }
  }
  rc = cdd_json_object_get_value(sobj, "anyOf", &kw);
  if (rc == CDD_C_SUCCESS) {
    cdd_c_error_t ao_rc =
        cdd_json_schema_match_anyof(root_schema, kw, instance);
    if (ao_rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(ao_rc, rc);
    }
  }
  rc = cdd_json_object_get_value(sobj, "oneOf", &kw);
  if (rc == CDD_C_SUCCESS) {
    cdd_c_error_t oo_rc =
        cdd_json_schema_match_oneof(root_schema, kw, instance);
    if (oo_rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(oo_rc, rc);
    }
  }
  rc = cdd_json_object_get_value(sobj, "not", &kw);
  if (rc == CDD_C_SUCCESS) {
    cdd_c_error_t sub_rc =
        cdd_json_schema_validate_with_root(root_schema, kw, instance);
    if (sub_rc == CDD_C_SUCCESS) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                            cdd_c_keep_err(rc, sub_rc));
    }
  }

  return rc_ign;
}

cdd_c_error_t cdd_json_schema_validate(const JSON_Value *schema,
                                       const JSON_Value *instance) {
  return cdd_json_schema_validate_with_root(schema, schema, instance);
}

cdd_c_error_t cdd_json_push_parser_init(struct cdd_json_push_parser *parser) {
  if (!parser)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  memset(parser, 0, sizeof(*parser));
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_push_parser_feed(struct cdd_json_push_parser *parser,
                                        const char *chunk, size_t chunk_len) {
  size_t i;
  if (!parser || (!chunk && chunk_len > 0))
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (chunk_len == 0)
    return CDD_C_SUCCESS;
  for (i = 0; i < chunk_len; ++i) {
    char c = chunk[i];
    if (parser->in_string) {
      if (parser->in_escape) {
        parser->in_escape = 0;
      } else if (c == '\\') {
        parser->in_escape = 1;
      } else if (c == '"') {
        parser->in_string = 0;
      } else if ((unsigned char)c < 0x20U) {
        return CDD_C_ERROR_PARSE;
      }
    } else if (c == '"') {
      parser->in_string = 1;
    } else if (c == '{' || c == '[') {
      if ((size_t)parser->depth < sizeof(parser->stack))
        parser->stack[parser->depth] = c;
      parser->depth++;
    } else if (c == '}' || c == ']') {
      char expected = (c == '}') ? '{' : '[';
      if (parser->depth <= 0 ||
          ((size_t)(parser->depth - 1) < sizeof(parser->stack) &&
           parser->stack[parser->depth - 1] != expected)) {
        return CDD_C_ERROR_PARSE;
      }
      parser->depth--;
    }
  }
  if (parser->len + chunk_len + 1 > parser->cap) {
    size_t ncap = parser->cap * 2 + chunk_len + 64;
    char *nbuf = NULL;
    cdd_c_error_t rc = cdd_json_alloc_fn(ncap, (void **)&nbuf);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (parser->buf) {
      memcpy(nbuf, parser->buf, parser->len);
      rc = cdd_json_dealloc_free(parser->buf);
    }
    parser->buf = nbuf;
    parser->cap = ncap;
  }
  memcpy(parser->buf + parser->len, chunk, chunk_len);
  parser->len += chunk_len;
  parser->buf[parser->len] = '\0';
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_json_push_parser_finish(struct cdd_json_push_parser *parser,
                                          JSON_Value **out) {
  cdd_c_error_t rc;
  cdd_c_error_t rc_free;
  if (!parser || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  if (!parser->buf || parser->len == 0)
    return CDD_C_ERROR_PARSE;
  rc = cdd_json_parse_buffer(parser->buf, parser->len, out);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_push_parser_free(parser));
  }
  rc_free = cdd_json_push_parser_free(parser);
  return rc_free;
}

cdd_c_error_t cdd_json_push_parser_free(struct cdd_json_push_parser *parser) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!parser)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (parser->buf) {
    rc = cdd_json_dealloc_free(parser->buf);
    parser->buf = NULL;
  }
  memset(parser, 0, sizeof(*parser));
  return rc;
}
