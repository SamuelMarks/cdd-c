/**
 * @file code_writer.c
 * @brief Implementation of the structured code-emission buffer and stream
 * writer abstraction.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/code_writer.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_cdd_fprintf_fail;
#endif

/**
 * @brief Appends raw bytes to the code writer stream or dynamic buffer.
 *
 * @param[in,out] writer Active code writer instance.
 * @param[in] data Pointer to raw character bytes.
 * @param[in] data_len Number of bytes to write.
 * @return CDD_C_SUCCESS on success, or error code on I/O or allocation failure.
 */
static cdd_c_error_t writer_raw_append(cdd_code_writer_t *writer,
                                       const char *data, size_t data_len) {
#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after >= 0) {
    if (g_fail_io_after == 0) {
      writer->error = CDD_C_ERROR_IO;
      return CDD_C_ERROR_IO;
    }
    g_fail_io_after--;
  }
  if (g_cdd_fprintf_fail > 0) {
    if (--g_cdd_fprintf_fail == 0) {
      writer->error = CDD_C_ERROR_IO;
      return CDD_C_ERROR_IO;
    }
  }
#endif

  if (writer->fp) {
    size_t written = fwrite(data, 1, data_len, writer->fp);
    if (written != data_len) {
      writer->error = CDD_C_ERROR_IO;
      return CDD_C_ERROR_IO;
    }
    return CDD_C_SUCCESS;
  } else {
    size_t need = writer->len + data_len + 1;
    if (need > writer->cap) {
      size_t new_cap = writer->cap == 0 ? 128 : writer->cap * 2;
      char *new_buf;
      while (new_cap < need)
        new_cap *= 2;
      new_buf = (char *)C_CDD_REALLOC(writer->buf, new_cap);
      if (!new_buf) {
        writer->error = CDD_C_ERROR_MEMORY;
        return CDD_C_ERROR_MEMORY;
      }
      writer->buf = new_buf;
      writer->cap = new_cap;
    }
    memcpy(writer->buf + writer->len, data, data_len);
    writer->len += data_len;
    writer->buf[writer->len] = '\0';
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Emits leading indentation spaces when positioned at the start of a
 * line.
 *
 * @param[in,out] writer Active code writer instance.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t writer_emit_indent_if_needed(cdd_code_writer_t *writer) {
  size_t spaces;
  size_t i;
  if (!writer->at_line_start)
    return CDD_C_SUCCESS;
  writer->at_line_start = 0;
  spaces = writer->indent_level * writer->indent_width;
  for (i = 0; i < spaces; ++i) {
    cdd_c_error_t rc = writer_raw_append(writer, " ", 1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Initializes a code writer backed by an open `FILE *` stream.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t cdd_code_writer_init_stream(cdd_code_writer_t *writer, FILE *fp) {
  if (!writer || !fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  memset(writer, 0, sizeof(*writer));
  writer->fp = fp;
  writer->indent_width = 2;
  writer->at_line_start = 1;
  writer->error = CDD_C_SUCCESS;
  return CDD_C_SUCCESS;
}

/**
 * @brief Initializes a code writer backed by a dynamically growing heap buffer.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t cdd_code_writer_init_buffer(cdd_code_writer_t *writer) {
  if (!writer)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  memset(writer, 0, sizeof(*writer));
  writer->indent_width = 2;
  writer->at_line_start = 1;
  writer->error = CDD_C_SUCCESS;
  return CDD_C_SUCCESS;
}

/**
 * @brief Releases any heap buffer owned by the code writer.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t cdd_code_writer_free(cdd_code_writer_t *writer) {
  if (!writer)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (writer->buf) {
    C_CDD_FREE(writer->buf);
    writer->buf = NULL;
  }
  writer->len = 0;
  writer->cap = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Increases the current indentation level by one step.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t cdd_code_writer_indent(cdd_code_writer_t *writer) {
  if (!writer)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  writer->indent_level++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Decreases the current indentation level by one step (clamped to 0).
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t cdd_code_writer_dedent(cdd_code_writer_t *writer) {
  if (!writer)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (writer->indent_level > 0)
    writer->indent_level--;
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes a raw string, automatically applying indentation at line
 * starts.
 *
 * @return CDD_C_SUCCESS on success, or error code on I/O or OOM failure.
 */
cdd_c_error_t cdd_code_writer_write(cdd_code_writer_t *writer,
                                    const char *text) {
  const char *p;
  if (!writer || !text)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (writer->error != CDD_C_SUCCESS)
    return writer->error;

  p = text;
  while (*p) {
    const char *nl = strchr(p, '\n');
    if (nl) {
      size_t seg_len = (size_t)(nl - p);
      if (seg_len > 0) {
        cdd_c_error_t rc = writer_emit_indent_if_needed(writer);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = writer_raw_append(writer, p, seg_len);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (writer_raw_append(writer, "\n", 1) != CDD_C_SUCCESS)
        return writer->error;
      writer->at_line_start = 1;
      p = nl + 1;
    } else {
      size_t rem_len = strlen(p);
      cdd_c_error_t rc = writer_emit_indent_if_needed(writer);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = writer_raw_append(writer, p, rem_len);
      if (rc != CDD_C_SUCCESS)
        return rc;
      break;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes a string followed by a newline character (`\n`).
 *
 * @return CDD_C_SUCCESS on success, or error code on I/O or OOM failure.
 */
cdd_c_error_t cdd_code_writer_writeln(cdd_code_writer_t *writer,
                                      const char *line) {
  cdd_c_error_t rc;
  if (!writer || !line)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_code_writer_write(writer, line);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_code_writer_write(writer, "\n");
}

/**
 * @brief Formats and writes a `printf`-style string to the code writer.
 *
 * @return CDD_C_SUCCESS on success, or error code on I/O or OOM failure.
 */
cdd_c_error_t cdd_code_writer_printf(cdd_code_writer_t *writer, const char *fmt,
                                     ...) {
  char stack_buf[2048];
  va_list args;
  int n;

  if (!writer || !fmt)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (writer->error != CDD_C_SUCCESS)
    return writer->error;

  va_start(args, fmt);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  n = vsnprintf_s(stack_buf, sizeof(stack_buf), _TRUNCATE, fmt, args);
  va_end(args);

  if (n >= 0 && (size_t)n < sizeof(stack_buf)) {
    return cdd_code_writer_write(writer, stack_buf);
  } else {
    size_t dyn_cap = 8192;
#else
  n = vsnprintf(stack_buf, sizeof(stack_buf), fmt, args);
  va_end(args);

  if ((size_t)n < sizeof(stack_buf)) {
    return cdd_code_writer_write(writer, stack_buf);
  } else {
    size_t dyn_cap = (size_t)n + 1;
#endif
    char *dyn_buf = (char *)C_CDD_MALLOC(dyn_cap);
    cdd_c_error_t rc;
    if (!dyn_buf) {
      writer->error = CDD_C_ERROR_MEMORY;
      return CDD_C_ERROR_MEMORY;
    }
    va_start(args, fmt);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    vsnprintf_s(dyn_buf, dyn_cap, _TRUNCATE, fmt, args);
#else
    vsnprintf(dyn_buf, dyn_cap, fmt, args);
#endif
    va_end(args);
    rc = cdd_code_writer_write(writer, dyn_buf);
    C_CDD_FREE(dyn_buf);
    return rc;
  }
}

/**
 * @brief Checks if any sticky error occurred on the code writer.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t cdd_code_writer_check_error(const cdd_code_writer_t *writer,
                                          cdd_c_error_t *out_err) {
  if (!writer || !out_err)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_err = writer->error;
  return CDD_C_SUCCESS;
}

/**
 * @brief Transfers ownership of the dynamically allocated string buffer to the
 * caller.
 *
 * @return CDD_C_SUCCESS on success, or error code if the writer encountered an
 * error.
 */
cdd_c_error_t cdd_code_writer_take_string(cdd_code_writer_t *writer,
                                          char **out_str) {
  if (!writer || !out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (writer->error != CDD_C_SUCCESS) {
    *out_str = NULL;
    return writer->error;
  }
  if (!writer->buf) {
    char *empty = (char *)C_CDD_CALLOC(1, 1);
    if (!empty) {
      writer->error = CDD_C_ERROR_MEMORY;
      *out_str = NULL;
      return CDD_C_ERROR_MEMORY;
    }
    *out_str = empty;
    return CDD_C_SUCCESS;
  }
  *out_str = writer->buf;
  writer->buf = NULL;
  writer->len = 0;
  writer->cap = 0;
  return CDD_C_SUCCESS;
}
