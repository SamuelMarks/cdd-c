/* clang-format off */
#ifndef CDD_FFI_VARIADIC_H
#define CDD_FFI_VARIADIC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../../src/cdd_api.h"
#include "cdd_ffi_ir.h"
#include <stddef.h>

/**
 * @brief Parses a printf-style format string and returns an array of types.
 *
 * This heuristic parser is used to strongly type variadic arguments based on
 * format specifiers (e.g., %d -> int, %f -> double, %s -> char*).
 *
 * @param fmt The format string to parse.
 * @param out_types Pointer to an array of cdd_ffi_type_t to populate.
 * @param max_types The maximum number of types out_types can hold.
 * @return The number of arguments found in the format string.
 */
C_CDD_EXPORT enum cdd_c_error cdd_ffi_parse_printf_format(const char *fmt,
                                                cdd_ffi_type_t *out_types,
                                                size_t max_types,
                                                size_t *out_count);

/**
 * @brief A generic union for passing arguments to the variadic trampoline.
 */
typedef union cdd_ffi_var_arg_t {
  int i;
  long l;
  long long ll;
  double d;
  void *p;
} cdd_ffi_var_arg_t;

/**
 * @brief Invokes a variadic function using a C-level trampoline.
 *
 * Uses a switch statement to unpack the array of runtime arguments into
 * physical registers/stack, acting as a fallback when libffi is unavailable.
 * Supports up to 8 variadic arguments.
 *
 * @param fn Pointer to the variadic function.
 * @param fmt The format string or first fixed argument.
 * @param args Array of generic arguments.
 * @param argc Number of arguments in the args array.
 * @return The integer result of the variadic function.
 */
C_CDD_EXPORT enum cdd_c_error cdd_ffi_invoke_variadic(enum cdd_c_error (*fn)(const char *, ...),
                                         const char *fmt,
                                         cdd_ffi_var_arg_t *args, size_t argc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_VARIADIC_H */
/* clang-format on */
