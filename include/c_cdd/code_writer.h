/**
 * @file code_writer.h
 * @brief Structured code-emission buffer and stream writer abstraction.
 *
 * Provides indentation tracking, formatted emission, and centralized
 * single-point error/OOM tracking for C and FFI code generators.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODE_WRITER_H
/** @brief Header guard for code_writer.h */
#define C_CDD_CODE_WRITER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
/* clang-format on */

/**
 * @brief Structured code emission writer context.
 */
typedef struct cdd_code_writer_t {
  FILE *fp;            /**< Optional underlying output file stream */
  char *buf;           /**< Optional dynamically allocated memory buffer */
  size_t len;          /**< Current byte length of memory buffer */
  size_t cap;          /**< Allocated capacity of memory buffer */
  size_t indent_level; /**< Current indentation depth */
  size_t indent_width; /**< Number of spaces per indentation level */
  int at_line_start;   /**< Non-zero if cursor is at the start of a new line */
  cdd_c_error_t error; /**< Sticky error state (CDD_C_SUCCESS if healthy) */
} cdd_code_writer_t;

/**
 * @brief Initialize a code writer backed by an open FILE stream.
 *
 * @param[out] writer Writer instance to initialize.
 * @param[in] fp Open FILE pointer to write to.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_code_writer_init_stream(cdd_code_writer_t *writer, FILE *fp);

/**
 * @brief Initialize a code writer backed by an in-memory dynamic buffer.
 *
 * @param[out] writer Writer instance to initialize.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_code_writer_init_buffer(cdd_code_writer_t *writer);

/**
 * @brief Release any owned memory buffer inside a code writer.
 *
 * Does not close the underlying `fp` if initialized via
 * `cdd_code_writer_init_stream`.
 *
 * @param[in,out] writer Writer instance to clean up.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t cdd_code_writer_free(cdd_code_writer_t *writer);

/**
 * @brief Increase the writer's indentation level by one.
 *
 * @param[in,out] writer Writer instance.
 * @return CDD_C_SUCCESS on success, or error code if writer is NULL.
 */
C_CDD_EXPORT cdd_c_error_t cdd_code_writer_indent(cdd_code_writer_t *writer);

/**
 * @brief Decrease the writer's indentation level by one (saturating at 0).
 *
 * @param[in,out] writer Writer instance.
 * @return CDD_C_SUCCESS on success, or error code if writer is NULL.
 */
C_CDD_EXPORT cdd_c_error_t cdd_code_writer_dedent(cdd_code_writer_t *writer);

/**
 * @brief Write a raw null-terminated string to the code writer.
 *
 * Automatically emits leading indentation spaces when at the start of a line.
 * If the writer is already in an error state, this is a no-op returning that
 * sticky error.
 *
 * @param[in,out] writer Writer instance.
 * @param[in] text Null-terminated string to write.
 * @return CDD_C_SUCCESS on success, or sticky error code on I/O or OOM failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_code_writer_write(cdd_code_writer_t *writer,
                                                 const char *text);

/**
 * @brief Write a raw string followed by a newline character (`
`).
 *
 * @param[in,out] writer Writer instance.
 * @param[in] line Null-terminated line content to write.
 * @return CDD_C_SUCCESS on success, or sticky error code on I/O or OOM failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_code_writer_writeln(cdd_code_writer_t *writer,
                                                   const char *line);

/**
 * @brief Write formatted text to the code writer.
 *
 * @param[in,out] writer Writer instance.
 * @param[in] fmt Printf-style format string.
 * @return CDD_C_SUCCESS on success, or sticky error code on I/O or OOM failure.
 */
#if defined(__GNUC__) || defined(__clang__)
C_CDD_EXPORT cdd_c_error_t cdd_code_writer_printf(cdd_code_writer_t *writer,
                                                  const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
C_CDD_EXPORT cdd_c_error_t cdd_code_writer_printf(cdd_code_writer_t *writer,
                                                  const char *fmt, ...);
#endif

/**
 * @brief Retrieve the current sticky error state of the writer.
 *
 * @param[in] writer Writer instance.
 * @param[out] out_err Output error code.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
C_CDD_EXPORT cdd_c_error_t cdd_code_writer_check_error(
    const cdd_code_writer_t *writer, cdd_c_error_t *out_err);

/**
 * @brief Detach and take ownership of the accumulated in-memory string buffer.
 *
 * @param[in,out] writer Writer instance.
 * @param[out] out_str Pointer to receive the null-terminated allocated string.
 * @return CDD_C_SUCCESS on success, or sticky error code.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_code_writer_take_string(cdd_code_writer_t *writer, char **out_str);

/** @brief Fault injection counter for failing after N I/O operations */
extern C_CDD_EXPORT int g_fail_io_after;
/** @brief Running counter of I/O operations executed */
extern C_CDD_EXPORT int g_io_calls;
/** @brief Fault injection counter for failing after N schema I/O operations */
extern C_CDD_EXPORT int g_schema_fail_io_after;
/** @brief Running counter of schema I/O operations executed */
extern C_CDD_EXPORT int g_schema_io_calls;
/** @brief Global flag to force simulated failure in schema codegen */
extern C_CDD_EXPORT int g_schema_codegen_force_fail;
/** @brief Fault injection step selector for client generator tests */
extern C_CDD_EXPORT int g_client_gen_fail;

/**
 * @brief Centralized stream fprintf wrapper with test fault injection.
 *
 * @param[in,out] stream Target output stream.
 * @param[in] format Printf format string.
 * @return Number of characters written, or negative on injected/stream error.
 */
#if defined(__GNUC__) || defined(__clang__)
extern C_CDD_EXPORT cdd_c_error_t cdd_io_fprintf_hook(FILE *stream,
                                                      const char *format, ...)
    __attribute__((format(printf, 2, 3)));
#else
extern C_CDD_EXPORT cdd_c_error_t cdd_io_fprintf_hook(FILE *stream,
                                                      const char *format, ...);
#endif

#ifndef FPRINTF_HOOK
/** @brief Macro alias for formatted stream emission hook */
#define FPRINTF_HOOK cdd_io_fprintf_hook
#endif

#ifndef FPUTS_HOOK
/** @brief Macro alias for raw string stream emission hook */
#define FPUTS_HOOK fputs
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODE_WRITER_H */
