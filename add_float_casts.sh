cat << 'INNER_EOF' >> include/c_cdd/int128_math.h

/**
 * @brief Cast float to 128-bit signed integer.
 */
static int cdd_float_to_int128(float val, cdd_int128_t *out) {
  /* Stub implementation */
  out->low = (uint64_t)val;
  out->high = (val < 0) ? -1LL : 0LL;
  return 0;
}

/**
 * @brief Cast double to 128-bit signed integer.
 */
static int cdd_double_to_int128(double val, cdd_int128_t *out) {
  /* Stub implementation */
  out->low = (uint64_t)val;
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
INNER_EOF
