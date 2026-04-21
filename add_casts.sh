cat << 'INNER_EOF' >> include/c_cdd/int128_math.h

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
  out->low = (uint64_t)val;
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
INNER_EOF
