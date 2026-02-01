/**
 * @file numeric_parser.h
 * @brief Logic for parsing C numeric literal strings into typed values.
 *
 * Provides a unified interface to convert token strings (e.g., "0xFF", "1.5f",
 * "0b101") into structured numeric definitions representing their semantic
 * value and type properties (unsigned, long, float vs double).
 *
 * Supports:
 * - Hexadecimal (0x prefix)
 * - Octal (leading 0)
 * - Binary (0b prefix - GCC/Clang/C23 extension)
 * - Decimal
 * - Floating point (decimal and hexfloat)
 * - Integers with suffixes (u, l, ll, z)
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_NUMERIC_PARSER_H
#define C_CDD_NUMERIC_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "c_cdd_stdbool.h"
#include <stddef.h>
#include <stdint.h> /* C89 environment with stdint available or via compat */

/* Fallback for Pre-C99 environments missing stdint.h */
#if !defined(UINT64_MAX)
typedef unsigned long long uint64_t;
typedef long long int64_t;
#endif

/**
 * @brief Discriminator for the kind of numeric value parsed.
 */
enum NumericKind {
  NUMERIC_INTEGER, /**< Integer value (e.g. 123, 0xFF) */
  NUMERIC_FLOAT,   /**< Floating point value (e.g. 1.0, 1e-5) */
  NUMERIC_ERROR    /**< Parse failed or invalid format */
};

/**
 * @brief Parsed integer metadata and value.
 */
struct IntegerInfo {
  uint64_t value;   /**< The raw integer value */
  int is_unsigned;  /**< 1 if 'u' or 'U' suffix present */
  int is_long;      /**< 1 if 'l' or 'L' suffix present */
  int is_long_long; /**< 1 if 'll' or 'LL' suffix present */
  int base;         /**< The radix used (2, 8, 10, or 16) */
};

/**
 * @brief Parsed floating-point metadata and value.
 */
struct FloatInfo {
  double value;       /**< The floating point value (stored as double) */
  int is_float;       /**< 1 if 'f' or 'F' suffix present (indicating single
                         precision) */
  int is_long_double; /**< 1 if 'l' or 'L' suffix present */
};

/**
 * @brief Container for a parsed numeric literal.
 */
struct NumericValue {
  enum NumericKind kind; /**< Type of the number */
  union {
    struct IntegerInfo integer; /**< Valid if kind == NUMERIC_INTEGER */
    struct FloatInfo floating;  /**< Valid if kind == NUMERIC_FLOAT */
  } data;
};

/**
 * @brief Parse a text string representing a C numeric literal.
 *
 * Analyzes the string for prefixes (0x, 0b), decimal points, exponents,
 * and suffixes to determine the type and value.
 *
 * @param[in] str The null-terminated literal string (e.g., "0x1A", "3.14f").
 * @param[out] out Pointer to the result structure.
 * @return 0 on success, EINVAL if input is NULL or invalid format, ERANGE if
 * overflow.
 */
extern C_CDD_EXPORT int parse_numeric_literal(const char *str,
                                              struct NumericValue *out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_NUMERIC_PARSER_H */
