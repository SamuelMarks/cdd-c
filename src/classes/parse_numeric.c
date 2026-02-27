/**
 * @file numeric_parser.c
 * @brief Implementation of C numeric literal parsing.
 *
 * Implements a state-machine-like scanner to differentiate between integers
 * and floats, handles base prefixes, computes values for non-standard bases
 * (binary), and detects type suffixes including C23 Decimal Floats.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "classes/parse_numeric.h"

/**
 * @brief Parse integer type suffixes (u, l, ll).
 *
 * @param[in] str Pointer to the start of suffixes.
 * @param[in,out] info Structure to populate flags in.
 * @return 0 on success, -1 if invalid characters found.
 */
static int parse_int_suffixes(const char *str, struct IntegerInfo *info) {
  size_t i = 0;
  while (str[i] != '\0') {
    char c = str[i];
    if (c == 'u' || c == 'U') {
      if (info->is_unsigned) /* Duplicate 'u' not allowed */
        return -1;
      info->is_unsigned = 1;
      i++;
    } else if (c == 'l' || c == 'L') {
      if (info->is_long_long) /* Already LL */
        return -1;
      if (info->is_long) {
        /* Second 'l' becomes 'll', assumes contiguous ll */
        if (str[i + 1] == 'l' || str[i + 1] == 'L') { /* Parser logic glitch? */
          /* Actually, if we see 'L' and we are already long, it implies LL if
             we handle standard order. But standard allows 'Lu', 'uL', 'LLu',
             'uLL'. We need to handle `ll` as a unit or statefully. */
          info->is_long = 0;
          info->is_long_long = 1;
        } else {
          /* Just valid transition from single L to LL if current is 'l'?
             Actually logic:
             If seen 'l', seeing another 'l' transitions to 'll'.
             But if seeing 'l' later, e.g. "ulL", that's invalid standard C
             (suffix order mixing). C99 6.4.4.1: integer-suffix: unsigned-suffix
             long-suffix(opt) unsigned-suffix long-long-suffix long-suffix
             unsigned-suffix(opt) long-long-suffix unsigned-suffix(opt) i.e. 'u'
             and 'l'/'ll' blocks can swap, but 'l's must be together. This
             parser is permissive: counts 'l's. */
          info->is_long_long = 1;
          info->is_long = 0;
        }
      } else {
        /* Check if next char is also 'l' for atomic LL detection */
        if (str[i + 1] == 'l' || str[i + 1] == 'L') {
          info->is_long_long = 1;
          i++; /* Consume extra */
        } else {
          info->is_long = 1;
        }
      }
      i++;
    } else {
      return -1; /* Invalid char in suffix */
    }
  }
  return 0;
}

/**
 * @brief Manually parse binary string to integer.
 * stdlib lacks `strtoull` for base 2 in strict C89 (and `0b` is an extension).
 */
static uint64_t parse_binary_str(const char *str, char **endptr) {
  uint64_t val = 0;
  const char *p = str;
  while (*p == '0' || *p == '1') {
    if (val > (UINT64_MAX >> 1)) {
      val = UINT64_MAX; /* Overflow saturation */
      errno = ERANGE;
    } else {
      val = (val << 1) | (*p - '0');
    }
    p++;
  }
  if (endptr)
    *endptr = (char *)p;
  return val;
}

int parse_numeric_literal(const char *str, struct NumericValue *out) {
  int is_hex = 0;
  int is_bin = 0;
  int is_oct = 0;
  int is_float = 0; /* Has decimal point or exponent */
  const char *p = str;
  const char *start_digits;
  char *end_ptr = NULL;

  if (!str || !out)
    return EINVAL;

  /* Reset output */
  memset(out, 0, sizeof(*out));
  out->kind = NUMERIC_ERROR;

  /* Skip leading whitespace (unlikely in parsed token but safe) */
  while (isspace((unsigned char)*p))
    p++;
  if (*p == '\0')
    return EINVAL;

  /* Detect Base */
  if (*p == '0') {
    if (p[1] == 'x' || p[1] == 'X') {
      is_hex = 1;
      p += 2;
    } else if (p[1] == 'b' || p[1] == 'B') {
      is_bin = 1;
      p += 2; /* 0b extension */
    } else if (isdigit((unsigned char)p[1]) || p[1] == '.') {
      /* If 0.123 -> Float. If 0123 -> Octal. */
      /* We distinguish later by scanning for '.' or 'e' */
      is_oct = 1; /* Tentative, might be overridden by float check */
      /* Don't advance p yet, need '0' for octal strtoul or float parser */
    } else {
      /* Just "0" */
      is_oct = 0; /* Base 10 or Octal 0 is same value */
    }
  }

  start_digits = p;

  /* Scan content to distinguish Float vs Int */
  /* Hex float: 0x...p... */
  /* Int: 0x... */
  /* Decimal: 1.23, 1e5 */
  while (*p &&
         (isalnum((unsigned char)*p) || *p == '.' || *p == '+' || *p == '-')) {
    if (*p == '.') {
      is_float = 1;
    } else if (!is_hex && (*p == 'e' || *p == 'E')) {
      /* 10e5 */
      is_float = 1;
    } else if (is_hex && (*p == 'p' || *p == 'P')) {
      /* 0x1p5 */
      is_float = 1;
    }
    p++;
  }

  /* Dispatch Parsing */
  if (is_float) {
    if (is_bin)
      return EINVAL; /* binary floats not standard logic usually */

    out->kind = NUMERIC_FLOAT;
    errno = 0;
    out->data.floating.value = strtod(str, &end_ptr);
    if (errno == ERANGE)
      return ERANGE;
    if (str == end_ptr)
      return EINVAL;

    /* Parse Suffixes */
    if (*end_ptr != '\0') {
      /* C23 Decimal Float Suffixes: df/DF, dd/DD, dl/DL */
      if ((end_ptr[0] == 'd' || end_ptr[0] == 'D') &&
          (end_ptr[1] == 'f' || end_ptr[1] == 'F')) {
        out->data.floating.is_decimal = DFP_32;
        if (end_ptr[2] != '\0')
          return EINVAL;
      } else if ((end_ptr[0] == 'd' || end_ptr[0] == 'D') &&
                 (end_ptr[1] == 'd' || end_ptr[1] == 'D')) {
        out->data.floating.is_decimal = DFP_64;
        if (end_ptr[2] != '\0')
          return EINVAL;
      } else if ((end_ptr[0] == 'd' || end_ptr[0] == 'D') &&
                 (end_ptr[1] == 'l' || end_ptr[1] == 'L')) {
        out->data.floating.is_decimal = DFP_128;
        if (end_ptr[2] != '\0')
          return EINVAL;
      }
      /* Standard Binary Float Suffixes */
      else if (*end_ptr == 'f' || *end_ptr == 'F') {
        out->data.floating.is_float = 1;
        if (end_ptr[1] != '\0')
          return EINVAL; /* Junk after suffix */
      } else if (*end_ptr == 'l' || *end_ptr == 'L') {
        out->data.floating.is_long_double = 1;
        if (end_ptr[1] != '\0')
          return EINVAL;
      } else {
        return EINVAL; /* Invalid suffix */
      }
    }
  } else {
    /* Integer */
    uint64_t val;
    out->kind = NUMERIC_INTEGER;
    errno = 0;

    if (is_bin) {
      val = parse_binary_str(start_digits, &end_ptr);
      out->data.integer.base = 2;
    } else {
      /* Handle hex/oct/dec via strtoull */
      int base = is_hex ? 16 : (is_oct ? 8 : 10);
      /* Use base 0 to let stdlib handle prefixes if we preserved them?
         We advanced 'start_digits' past 0x. */
      val = strtoull(start_digits, &end_ptr, base);
      out->data.integer.base = base;
    }

    if (errno == ERANGE)
      return ERANGE;
    if (end_ptr == start_digits && *start_digits != '\0')
      return EINVAL;

    out->data.integer.value = val;

    /* Suffixes */
    if (*end_ptr != '\0') {
      if (parse_int_suffixes(end_ptr, &out->data.integer) != 0) {
        return EINVAL;
      }
    }
  }

  return 0;
}
