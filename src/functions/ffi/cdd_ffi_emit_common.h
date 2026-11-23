/**
 * @file cdd_ffi_emit_common.h
 * @brief Shared FFI type-mapping, checked I/O, comment translation, and
 * file-scaffolding utilities across language emitters.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_FFI_EMIT_COMMON_H
/** @brief Header guard for cdd_ffi_emit_common.h */
#define C_CDD_FFI_EMIT_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "c_cdd/no_discard.h"
#include "cdd_api.h"
#include "cdd_c_error.h"
#include "ffi/cdd_ffi_ir.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
/** @brief Countdown fault-injection counter for FFI emitter steps. */
extern C_CDD_EXPORT int g_ffi_emit_fail_step;
#endif

/**
 * @brief Table of target-language type names for C FFI primitives and pointers.
 */
typedef struct cdd_ffi_type_map_table_t {
  const char *void_type;         /**< Void type representation */
  const char *bool_type;         /**< Boolean type representation */
  const char *int8_type;         /**< 8-bit signed integer representation */
  const char *uint8_type;        /**< 8-bit unsigned integer representation */
  const char *int16_type;        /**< 16-bit signed integer representation */
  const char *uint16_type;       /**< 16-bit unsigned integer representation */
  const char *int32_type;        /**< 32-bit signed integer representation */
  const char *uint32_type;       /**< 32-bit unsigned integer representation */
  const char *int64_type;        /**< 64-bit signed integer representation */
  const char *uint64_type;       /**< 64-bit unsigned integer representation */
  const char *float32_type;      /**< 32-bit float representation */
  const char *float64_type;      /**< 64-bit double representation */
  const char *char_ptr_type;     /**< String / char pointer representation */
  const char *void_ptr_type;     /**< Generic / opaque pointer representation */
  const char *function_ptr_type; /**< Optional function pointer representation
                                  */
  const char *enum_type;         /**< Default enum representation */
  int use_ref_name_for_struct; /**< Non-zero to use type.ref_name for structs */
  int use_ref_name_for_enum;   /**< Non-zero to use type.ref_name for enums */
  int use_ref_name_for_typedef; /**< Non-zero to use type.ref_name for typedefs
                                 */
} cdd_ffi_type_map_table_t;

/**
 * @brief Target-language comment formatting and escaping specification.
 */
typedef struct cdd_ffi_comment_style_t {
  /** @brief Line-comment prefix (e.g., `"! "` or `"*> "`), or NULL for block */
  const char *line_prefix;
  /** @brief Block-comment opening delimiter (e.g., `"(* "`), or NULL */
  const char *block_open;
  /** @brief Block-comment closing delimiter (e.g., `" *)"`), or NULL */
  const char *block_close;
  /** @brief Raw nested opening token to escape inside block comments */
  const char *nested_open_raw;
  /** @brief Escaped replacement for `nested_open_raw` */
  const char *nested_open_esc;
  /** @brief Raw nested closing token to escape inside block comments */
  const char *nested_close_raw;
  /** @brief Escaped replacement for `nested_close_raw` */
  const char *nested_close_esc;
  /** @brief Optional maximum column width for line comments (0 = unlimited) */
  size_t max_columns;
} cdd_ffi_comment_style_t;

/**
 * @brief Emit a linked list of FFI trivia tokens to an open file stream.
 *
 * @param[in,out] f Output file stream.
 * @param[in] trivia Head of trivia linked list (may be NULL).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if f is
 * NULL.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t
cdd_ffi_emit_trivia(FILE *f, const cdd_ffi_trivia_t *trivia);

/**
 * @brief Translates C/C++ line and block trivia into target-language comments.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] trivia Linked list of FFI trivia nodes (may be NULL).
 * @param[in] style Target-language comment style configuration.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on invalid
 * arguments, or CDD_C_ERROR_IO on stream write failure.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t
cdd_ffi_emit_translated_trivia(FILE *fp, const cdd_ffi_trivia_t *trivia,
                               const cdd_ffi_comment_style_t *style);

/**
 * @brief Translates and emits a C/Doxygen docstring using a target-language
 * comment style.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] doc Source docstring text (may be NULL or empty).
 * @param[in] indent Optional indentation prefix prepended to each line.
 * @param[in] style Target-language comment style configuration.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on invalid
 * arguments, or CDD_C_ERROR_IO on stream write failure.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t
cdd_ffi_emit_translated_doc(FILE *fp, const char *doc, const char *indent,
                            const cdd_ffi_comment_style_t *style);

/**
 * @brief Map a `cdd_ffi_type_t` to a target-language type string using a lookup
 * table.
 *
 * @param[in] table Language type mapping table.
 * @param[in] type FFI IR type descriptor.
 * @param[out] out_type Output pointer receiving the resolved type string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t
cdd_ffi_map_type_from_table(const cdd_ffi_type_map_table_t *table,
                            cdd_ffi_type_t type, const char **out_type);

/**
 * @brief Writes a null-terminated string to `fp` with fault injection and error
 * checking.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] str Null-terminated string to write.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if NULL, or
 * CDD_C_ERROR_IO on stream write failure.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_write_str(FILE *fp,
                                                        const char *str);

/**
 * @brief Writes exactly `len` bytes from `buf` to `fp` with fault injection and
 * error checking.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] buf Source byte buffer (may be NULL only when `len == 0`).
 * @param[in] len Number of bytes to write.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL args,
 * or CDD_C_ERROR_IO on stream write failure.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_write_bytes(FILE *fp,
                                                          const char *buf,
                                                          size_t len);

/**
 * @brief Formats into a bounded buffer using MSVC Safe CRT or standard C
 * formatting, returning an error on truncation.
 *
 * @param[out] out_buf Destination character buffer.
 * @param[in] out_cap Capacity of `out_buf` in bytes (including NUL).
 * @param[in] fmt Printf-style format string.
 * @param[in] ap Initialized `va_list` argument list.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL/zero
 * args, or CDD_C_ERROR_MEMORY if output was truncated.
 */
#if defined(__GNUC__) || defined(__clang__)
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_vsnprintf_checked(char *out_buf,
                                                                size_t out_cap,
                                                                const char *fmt,
                                                                va_list ap)
    __attribute__((format(printf, 3, 0)));
#else
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_vsnprintf_checked(char *out_buf,
                                                                size_t out_cap,
                                                                const char *fmt,
                                                                va_list ap);
#endif

/**
 * @brief Variadic wrapper around `cdd_ffi_vsnprintf_checked`.
 *
 * @param[out] out_buf Destination character buffer.
 * @param[in] out_cap Capacity of `out_buf` in bytes (including NUL).
 * @param[in] fmt Printf-style format string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL/zero
 * args, or CDD_C_ERROR_MEMORY if output was truncated.
 */
#if defined(__GNUC__) || defined(__clang__)
C_CDD_EXPORT NO_DISCARD cdd_c_error_t
cdd_ffi_snprintf_checked(char *out_buf, size_t out_cap, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));
#else
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_snprintf_checked(char *out_buf,
                                                               size_t out_cap,
                                                               const char *fmt,
                                                               ...);
#endif

/**
 * @brief Formats and writes a `printf`-style string to `fp` with full error
 * percolation.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] fmt Printf-style format string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL args,
 * CDD_C_ERROR_MEMORY on OOM/truncation, or CDD_C_ERROR_IO on write failure.
 */
#if defined(__GNUC__) || defined(__clang__)
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_write_fmt(FILE *fp,
                                                        const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_write_fmt(FILE *fp,
                                                        const char *fmt, ...);
#endif

/**
 * @brief Open an output file inside `output_dir` for FFI code emission.
 *
 * @param[in] output_dir Target directory path.
 * @param[in] filename Relative filename inside `output_dir`.
 * @param[out] out_fp Pointer to receive the opened FILE stream.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO /
 * CDD_C_ERROR_INVALID_ARGUMENT on failure.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_open_output_file(
    const char *output_dir, const char *filename, FILE **out_fp);

/**
 * @brief Closes an open output file stream with fault injection and error
 * checking, nulling `*io_fp` afterward to prevent double-close.
 *
 * @param[in,out] io_fp Address of the `FILE *` pointer to close and clear.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if NULL, or
 * CDD_C_ERROR_IO on `fclose` or injected I/O failure.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_close_output_file(FILE **io_fp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_FFI_EMIT_COMMON_H */
