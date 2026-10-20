/**
 * @file cdd_ffi_variadic.h
 * @brief Variadic format parsing and C-level trampoline invocation.
 */
#ifndef CDD_FFI_VARIADIC_H
#define CDD_FFI_VARIADIC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "../../src/cdd_api.h"
#include "cdd_ffi_ir.h"
#include <stddef.h>

#if defined(_MSC_VER) && _MSC_VER < 1600
typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "msvc/stdint.h"
#else
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "c_cdd/msvc/stdint.h"
#else
#include <stdint.h>
#endif
#endif
/* clang-format on */
#endif

/**
 * @brief Parses a printf-style format string and returns an array of types.
 *
 * This heuristic parser is used to strongly type variadic arguments based on
 * format specifiers (e.g., %d -> int, %f -> double, %s -> char*).
 *
 * @param[in] fmt The format string to parse.
 * @param[out] out_types Pointer to an array of cdd_ffi_type_t to populate.
 * @param[in] max_types The maximum number of types out_types can hold.
 * @param[out] out_count Pointer to receive the number of arguments found.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_ffi_parse_printf_format(const char *fmt, cdd_ffi_type_t *out_types,
                            size_t max_types, size_t *out_count);

/**
 * @brief A generic union for passing arguments to the variadic trampoline.
 */
typedef union cdd_ffi_var_arg_t {
  int i;          /**< Signed integer argument value */
  unsigned int u; /**< Unsigned integer argument value */
  long l;         /**< Long integer argument value */
  int64_t ll;     /**< 64-bit integer argument value */
  double d;       /**< Double-precision floating-point argument value */
  void *p;        /**< Generic pointer argument value */
} cdd_ffi_var_arg_t;

/**
 * @brief Formats a `printf`-style string one specifier at a time into
 * `out_buf` using the exact C type for each `cdd_ffi_var_arg_t` argument.
 *
 * Avoids ABI register mismatch for `double`, `int`, `unsigned int`, `long`,
 * `int64_t`, `const char *`, and `void *` variadic arguments and supports any
 * number of arguments without an 8-argument limit.
 *
 * @param[out] out_buf Output character buffer.
 * @param[in] out_cap Capacity of `out_buf` in bytes (must be > 0).
 * @param[in] fmt Format string.
 * @param[in] args Array of typed variadic argument unions (may be NULL if
 * `argc == 0`).
 * @param[in] argc Number of arguments in `args`.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` on
 * invalid arguments.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_ffi_format_variadic(char *out_buf, size_t out_cap, const char *fmt,
                        const cdd_ffi_var_arg_t *args, size_t argc);

/**
 * @brief Invokes a variadic function using a C-level trampoline.
 *
 * Uses a switch statement to unpack the array of runtime arguments into
 * physical registers/stack, acting as a fallback when libffi is unavailable.
 * Supports up to 8 variadic arguments.
 *
 * @param[in,out] fn Pointer to the variadic function.
 * @param[in] fmt The format string or first fixed argument.
 * @param[in] args Array of generic arguments.
 * @param[in] argc Number of arguments in the args array.
 * @return The integer result of the variadic function.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_ffi_invoke_variadic(cdd_c_error_t (*fn)(const char *, ...), const char *fmt,
                        cdd_ffi_var_arg_t *args, size_t argc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_VARIADIC_H */
