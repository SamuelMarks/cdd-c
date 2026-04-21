cat << 'INNER_EOF' >> include/c_cdd/int128_math.h

/**
 * @brief Bitwise AND of two 128-bit integers.
 */
static int cdd_uint128_and(cdd_uint128_t a, cdd_uint128_t b, cdd_uint128_t *out) {
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
static int cdd_uint128_or(cdd_uint128_t a, cdd_uint128_t b, cdd_uint128_t *out) {
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
static int cdd_uint128_xor(cdd_uint128_t a, cdd_uint128_t b, cdd_uint128_t *out) {
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
INNER_EOF
