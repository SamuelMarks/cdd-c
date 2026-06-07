/**
 * @file int128_math.h
 * @brief Polyfill arithmetic operations for 128-bit integers.
 */

#ifndef C_CDD_INT128_MATH_H
#define C_CDD_INT128_MATH_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "int128.h"
#include <stddef.h>
/* clang-format on */
/* LCOV_EXCL_START */

/**
 * @brief Adds two 128-bit unsigned integers.
 *
 * @param a First operand.
 * @param b Second operand.
 * @param out Pointer to result.
 * @return 0 on success.
 */
static int cdd_uint128_add(cdd_uint128_t a, cdd_uint128_t b,
                           cdd_uint128_t *out) {
  out->low = a.low + b.low;
  out->high = a.high + b.high + (out->low < a.low ? 1 : 0);
  return 0;
}

/**
 * @brief Adds two 128-bit signed integers.
 *
 * @param a First operand.
 * @param b Second operand.
 * @param out Pointer to result.
 * @return 0 on success.
 */
static int cdd_int128_add(cdd_int128_t a, cdd_int128_t b, cdd_int128_t *out) {
  out->low = a.low + b.low;
  out->high = a.high + b.high + (out->low < a.low ? 1 : 0);
  return 0;
}

/**
 * @brief Subtracts two 128-bit unsigned integers.
 *
 * @param a First operand.
 * @param b Second operand.
 * @param out Pointer to result.
 * @return 0 on success.
 */
static int cdd_uint128_sub(cdd_uint128_t a, cdd_uint128_t b,
                           cdd_uint128_t *out) {
  out->low = a.low - b.low;
  out->high = a.high - b.high - (a.low < b.low ? 1 : 0);
  return 0;
}

/**
 * @brief Subtracts two 128-bit signed integers.
 *
 * @param a First operand.
 * @param b Second operand.
 * @param out Pointer to result.
 * @return 0 on success.
 */
static int cdd_int128_sub(cdd_int128_t a, cdd_int128_t b, cdd_int128_t *out) {
  out->low = a.low - b.low;
  out->high = a.high - b.high - (a.low < b.low ? 1 : 0);
  return 0;
}

/**
 * @brief Multiplies two 128-bit unsigned integers.
 *
 * @param a First operand.
 * @param b Second operand.
 * @param out Pointer to result.
 * @return 0 on success.
 */
static int cdd_uint128_mul(cdd_uint128_t a, cdd_uint128_t b,
                           cdd_uint128_t *out) {
  uint64_t a_lo = a.low & 0xFFFFFFFF;
  uint64_t a_hi = a.low >> 32;
  uint64_t b_lo = b.low & 0xFFFFFFFF;
  uint64_t b_hi = b.low >> 32;

  uint64_t p00 = a_lo * b_lo;
  uint64_t p01 = a_lo * b_hi;
  uint64_t p10 = a_hi * b_lo;
  uint64_t p11 = a_hi * b_hi;

  uint64_t mid1 = p01 + (p00 >> 32);
  uint64_t carry = (mid1 < p01) ? 1ULL : 0ULL;
  uint64_t mid2 = mid1 + p10;
  carry += (mid2 < mid1) ? 1ULL : 0ULL;

  out->low = (mid2 << 32) | (p00 & 0xFFFFFFFF);
  out->high =
      a.high * b.low + a.low * b.high + p11 + (mid2 >> 32) + (carry << 32);
  return 0;
}

/**
 * @brief Multiplies two 128-bit signed integers.
 *
 * @param a First operand.
 * @param b Second operand.
 * @param out Pointer to result.
 * @return 0 on success.
 */
static int cdd_int128_mul(cdd_int128_t a, cdd_int128_t b, cdd_int128_t *out) {
  cdd_uint128_t ua;
  cdd_uint128_t ub;
  cdd_uint128_t uout;
  ua.low = a.low;
  ua.high = (uint64_t)a.high;
  ub.low = b.low;
  ub.high = (uint64_t)b.high;
  cdd_uint128_mul(ua, ub, &uout);
  out->low = uout.low;
  out->high = (int64_t)uout.high;
  return 0;
}

/**
 * @brief Shifts a 128-bit unsigned integer left.
 *
 * @param a The operand to shift.
 * @param shift The number of bits to shift.
 * @param out Pointer to result.
 * @return 0 on success.
 */
static int cdd_uint128_shl(cdd_uint128_t a, unsigned int shift,
                           cdd_uint128_t *out) {
  shift &= 127;
  if (shift == 0) {
    *out = a;
  } else if (shift < 64) {
    out->high = (a.high << shift) | (a.low >> (64 - shift));
    out->low = a.low << shift;
  } else {
    out->high = a.low << (shift - 64);
    out->low = 0;
  }
  return 0;
}

/**
 * @brief Shifts a 128-bit unsigned integer right.
 *
 * @param a The operand to shift.
 * @param shift The number of bits to shift.
 * @param out Pointer to result.
 * @return 0 on success.
 */
static int cdd_uint128_shr(cdd_uint128_t a, unsigned int shift,
                           cdd_uint128_t *out) {
  shift &= 127;
  if (shift == 0) {
    *out = a;
  } else if (shift < 64) {
    out->low = (a.low >> shift) | (a.high << (64 - shift));
    out->high = a.high >> shift;
  } else {
    out->low = a.high >> (shift - 64);
    out->high = 0;
  }
  return 0;
}

/**
 * @brief Shifts a 128-bit signed integer left.
 *
 * @param a The operand to shift.
 * @param shift The number of bits to shift.
 * @param out Pointer to result.
 * @return 0 on success.
 */
static int cdd_int128_shl(cdd_int128_t a, unsigned int shift,
                          cdd_int128_t *out) {
  cdd_uint128_t ua;
  cdd_uint128_t uout;
  ua.low = a.low;
  ua.high = (uint64_t)a.high;
  cdd_uint128_shl(ua, shift, &uout);
  out->low = uout.low;
  out->high = (int64_t)uout.high;
  return 0;
}

/**
 * @brief Shifts a 128-bit signed integer right (arithmetic shift).
 *
 * @param a The operand to shift.
 * @param shift The number of bits to shift.
 * @param out Pointer to result.
 * @return 0 on success.
 */
static int cdd_int128_shr(cdd_int128_t a, unsigned int shift,
                          cdd_int128_t *out) {
  shift &= 127;
  if (shift == 0) {
    *out = a;
  } else if (shift < 64) {
    out->low = (a.low >> shift) | ((uint64_t)a.high << (64 - shift));
    out->high = a.high >> shift;
  } else {
    out->low = (uint64_t)(a.high >> (shift - 64));
    out->high = a.high < 0 ? -1LL : 0LL;
  }
  return 0;
}

/**
 * @brief Internal helper to divide and modulo 128-bit unsigned integers.
 *
 * @param num The numerator.
 * @param den The denominator.
 * @param quot Pointer to quotient (can be NULL).
 * @param rem Pointer to remainder (can be NULL).
 * @return 0 on success, non-zero on division by zero.
 */
static int cdd_uint128_divmod(cdd_uint128_t num, cdd_uint128_t den,
                              cdd_uint128_t *quot, cdd_uint128_t *rem) {
  cdd_uint128_t q;
  cdd_uint128_t r;
  int i;
  if (den.high == 0 && den.low == 0) {
    return 1; /* Division by zero error */
  }
  q = cdd_make_uint128(0, 0);
  r = cdd_make_uint128(0, 0);

  for (i = 127; i >= 0; i--) {
    cdd_uint128_shl(r, 1, &r);

    /* Get i-th bit of num */
    {
      int bit = 0;
      if (i < 64) {
        bit = (int)((num.low >> i) & 1);
      } else {
        bit = (int)((num.high >> (i - 64)) & 1);
      }
      r.low |= (uint64_t)bit;
    }

    /* if r >= den */
    if (r.high > den.high || (r.high == den.high && r.low >= den.low)) {
      cdd_uint128_sub(r, den, &r);
      if (i < 64) {
        q.low |= (1ULL << i);
      } else {
        q.high |= (1ULL << (i - 64));
      }
    }
  }

  if (quot)
    *quot = q;
  if (rem)
    *rem = r;
  return 0;
}

/**
 * @brief Divides two 128-bit unsigned integers.
 *
 * @param a First operand.
 * @param b Second operand.
 * @param out Pointer to result.
 * @return 0 on success, non-zero on division by zero.
 */
static int cdd_uint128_div(cdd_uint128_t a, cdd_uint128_t b,
                           cdd_uint128_t *out) {
  return cdd_uint128_divmod(a, b, out, NULL);
}

/**
 * @brief Modulo of two 128-bit unsigned integers.
 *
 * @param a First operand.
 * @param b Second operand.
 * @param out Pointer to result.
 * @return 0 on success, non-zero on division by zero.
 */
static int cdd_uint128_mod(cdd_uint128_t a, cdd_uint128_t b,
                           cdd_uint128_t *out) {
  return cdd_uint128_divmod(a, b, NULL, out);
}

/**
 * @brief Divides two 128-bit signed integers.
 *
 * @param a First operand.
 * @param b Second operand.
 * @param out Pointer to result.
 * @return 0 on success, non-zero on division by zero.
 */
static int cdd_int128_div(cdd_int128_t a, cdd_int128_t b, cdd_int128_t *out) {
  cdd_uint128_t ua;
  cdd_uint128_t ub;
  cdd_uint128_t uq;
  int sign_a = (a.high < 0);
  int sign_b = (b.high < 0);
  int sign_res = sign_a ^ sign_b;
  int rc;

  if (sign_a) {
    cdd_uint128_t zero = cdd_make_uint128(0, 0);
    cdd_uint128_t tmp;
    tmp.high = (uint64_t)a.high;
    tmp.low = a.low;
    cdd_uint128_sub(zero, tmp, &ua);
  } else {
    ua.high = (uint64_t)a.high;
    ua.low = a.low;
  }

  if (sign_b) {
    cdd_uint128_t zero = cdd_make_uint128(0, 0);
    cdd_uint128_t tmp;
    tmp.high = (uint64_t)b.high;
    tmp.low = b.low;
    cdd_uint128_sub(zero, tmp, &ub);
  } else {
    ub.high = (uint64_t)b.high;
    ub.low = b.low;
  }

  rc = cdd_uint128_divmod(ua, ub, &uq, NULL);
  if (rc != 0)
    return rc;

  if (sign_res) {
    cdd_uint128_t zero = cdd_make_uint128(0, 0);
    cdd_uint128_sub(zero, uq, &uq);
  }

  out->high = (int64_t)uq.high;
  out->low = uq.low;
  return 0;
}

/**
 * @brief Modulo of two 128-bit signed integers.
 *
 * @param a First operand.
 * @param b Second operand.
 * @param out Pointer to result.
 * @return 0 on success, non-zero on division by zero.
 */
static int cdd_int128_mod(cdd_int128_t a, cdd_int128_t b, cdd_int128_t *out) {
  cdd_uint128_t ua;
  cdd_uint128_t ub;
  cdd_uint128_t ur;
  int sign_a = (a.high < 0);
  int sign_b = (b.high < 0);
  int rc;

  if (sign_a) {
    cdd_uint128_t zero = cdd_make_uint128(0, 0);
    cdd_uint128_t tmp;
    tmp.high = (uint64_t)a.high;
    tmp.low = a.low;
    cdd_uint128_sub(zero, tmp, &ua);
  } else {
    ua.high = (uint64_t)a.high;
    ua.low = a.low;
  }

  if (sign_b) {
    cdd_uint128_t zero = cdd_make_uint128(0, 0);
    cdd_uint128_t tmp;
    tmp.high = (uint64_t)b.high;
    tmp.low = b.low;
    cdd_uint128_sub(zero, tmp, &ub);
  } else {
    ub.high = (uint64_t)b.high;
    ub.low = b.low;
  }

  rc = cdd_uint128_divmod(ua, ub, NULL, &ur);
  if (rc != 0)
    return rc;

  if (sign_a) {
    cdd_uint128_t zero = cdd_make_uint128(0, 0);
    cdd_uint128_sub(zero, ur, &ur);
  }

  out->high = (int64_t)ur.high;
  out->low = ur.low;
  return 0;
}

/**
 * @brief Bitwise AND of two 128-bit integers.
 */
static int cdd_uint128_and(cdd_uint128_t a, cdd_uint128_t b,
                           cdd_uint128_t *out) {
  out->low = a.low & b.low;
  out->high = a.high & b.high;
  return 0;
}

static int cdd_int128_and(cdd_int128_t a, cdd_int128_t b, cdd_int128_t *out) {
  out->low = a.low & b.low;
  out->high = a.high & b.high;
  return 0;
}

/**
 * @brief Bitwise OR of two 128-bit integers.
 */
static int cdd_uint128_or(cdd_uint128_t a, cdd_uint128_t b,
                          cdd_uint128_t *out) {
  out->low = a.low | b.low;
  out->high = a.high | b.high;
  return 0;
}

static int cdd_int128_or(cdd_int128_t a, cdd_int128_t b, cdd_int128_t *out) {
  out->low = a.low | b.low;
  out->high = a.high | b.high;
  return 0;
}

/**
 * @brief Bitwise XOR of two 128-bit integers.
 */
static int cdd_uint128_xor(cdd_uint128_t a, cdd_uint128_t b,
                           cdd_uint128_t *out) {
  out->low = a.low ^ b.low;
  out->high = a.high ^ b.high;
  return 0;
}

static int cdd_int128_xor(cdd_int128_t a, cdd_int128_t b, cdd_int128_t *out) {
  out->low = a.low ^ b.low;
  out->high = a.high ^ b.high;
  return 0;
}

/**
 * @brief Bitwise NOT of a 128-bit integer.
 */
static int cdd_uint128_not(cdd_uint128_t a, cdd_uint128_t *out) {
  out->low = ~a.low;
  out->high = ~a.high;
  return 0;
}

static int cdd_int128_not(cdd_int128_t a, cdd_int128_t *out) {
  out->low = ~a.low;
  out->high = ~a.high;
  return 0;
}

/**
 * @brief Cast 64-bit unsigned to 128-bit unsigned.
 */
static int cdd_uint64_to_uint128(uint64_t val, cdd_uint128_t *out) {
  out->low = val;
  out->high = 0;
  return 0;
}

/**
 * @brief Cast 64-bit signed to 128-bit signed.
 */
static int cdd_int64_to_int128(int64_t val, cdd_int128_t *out) {
  out->low = (uint64_t)(int64_t)val;
  out->high = (val < 0) ? -1LL : 0LL;
  return 0;
}

/**
 * @brief Cast 128-bit unsigned to 64-bit unsigned.
 */
static int cdd_uint128_to_uint64(cdd_uint128_t val, uint64_t *out) {
  *out = val.low;
  return 0;
}

/**
 * @brief Cast 128-bit signed to 64-bit signed.
 */
static int cdd_int128_to_int64(cdd_int128_t val, int64_t *out) {
  *out = (int64_t)val.low;
  return 0;
}

/**
 * @brief Cast float to 128-bit signed integer.
 */
static int cdd_float_to_int128(float val, cdd_int128_t *out) {
  /* Stub implementation */
  out->low = (uint64_t)(int64_t)val;
  out->high = (val < 0) ? -1LL : 0LL;
  return 0;
}

/**
 * @brief Cast double to 128-bit signed integer.
 */
static int cdd_double_to_int128(double val, cdd_int128_t *out) {
  /* Stub implementation */
  out->low = (uint64_t)(int64_t)val;
  out->high = (val < 0) ? -1LL : 0LL;
  return 0;
}

/**
 * @brief Cast 128-bit signed integer to float.
 */
static int cdd_int128_to_float(cdd_int128_t val, float *out) {
  /* Stub implementation */
  *out = (float)(int64_t)val.low;
  return 0;
}

/**
 * @brief Cast 128-bit signed integer to double.
 */
static int cdd_int128_to_double(cdd_int128_t val, double *out) {
  /* Stub implementation */
  *out = (double)(int64_t)val.low;
  return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_INT128_MATH_H */

/* LCOV_EXCL_STOP */
