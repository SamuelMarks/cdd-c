/**
 * @file cdd_ffi_emit_common.c
 * @brief Implementation of shared FFI type-mapping, checked I/O, comment
 * translation, and file-scaffolding utilities.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_common.h"
#include "c_cdd/safe_crt.h"
#include <string.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
/** @brief Counter to simulate I/O failures after a specific number of calls in
 * tests. */
extern C_CDD_EXPORT int g_fail_io_after;
#endif

/*
 * @brief Writes a null-terminated string to `fp` with fault injection and error
 * checking.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] str Null-terminated string to write.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if NULL, or
 * CDD_C_ERROR_IO on stream write failure.
 */
NO_DISCARD cdd_c_error_t cdd_ffi_write_str(FILE *fp, const char *str) {
  if (!fp || !str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after > 0 && --g_fail_io_after == 0)
    return CDD_C_ERROR_IO;
#endif
  if (fputs(str, fp) < 0)
    return CDD_C_ERROR_IO;
  return CDD_C_SUCCESS;
}

/*
 * @brief Writes exactly `len` bytes from `buf` to `fp` with fault injection and
 * error checking.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] buf Source byte buffer (may be NULL only when `len == 0`).
 * @param[in] len Number of bytes to write.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL args,
 * or CDD_C_ERROR_IO on stream write failure.
 */
NO_DISCARD cdd_c_error_t cdd_ffi_write_bytes(FILE *fp, const char *buf,
                                             size_t len) {
  if (!fp || (!buf && len > 0))
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after > 0 && --g_fail_io_after == 0)
    return CDD_C_ERROR_IO;
#endif
  if (len > 0 && fwrite(buf, 1, len, fp) != len)
    return CDD_C_ERROR_IO;
  return CDD_C_SUCCESS;
}

/*
 * @brief Closes an open output file stream with fault injection and error
 * checking, nulling `*io_fp` afterward to prevent double-close.
 *
 * @param[in,out] io_fp Address of the `FILE *` pointer to close and clear.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if NULL, or
 * CDD_C_ERROR_IO on `fclose` or injected I/O failure.
 */
NO_DISCARD cdd_c_error_t cdd_ffi_close_output_file(FILE **io_fp) {
  FILE *fp;
  int close_rc;

  if (!io_fp || !*io_fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  fp = *io_fp;
  *io_fp = NULL;
  close_rc = fclose(fp);

#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after > 0 && --g_fail_io_after == 0)
    return CDD_C_ERROR_IO;
#endif

  if (close_rc != 0)
    return CDD_C_ERROR_IO;
  return CDD_C_SUCCESS;
}

/*
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
NO_DISCARD cdd_c_error_t cdd_ffi_vsnprintf_checked(char *out_buf,
                                                   size_t out_cap,
                                                   const char *fmt,
                                                   va_list ap) {
  int written;

  if (!out_buf || out_cap == 0 || !fmt)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef CDD_BUILD_TESTS
  if (g_ffi_emit_fail_step > 0 && --g_ffi_emit_fail_step == 0)
    return CDD_C_ERROR_MEMORY;
#endif

#if defined(_MSC_VER)
  written = _vsnprintf_s(out_buf, out_cap, _TRUNCATE, fmt, ap);
  if (written < 0)
    return CDD_C_ERROR_MEMORY;
#else
  written = vsnprintf(out_buf, out_cap, fmt, ap);
  if ((size_t)written >= out_cap) {
    out_buf[out_cap - 1] = '\0';
    return CDD_C_ERROR_MEMORY;
  }
#endif

  return CDD_C_SUCCESS;
}

/*
 * @brief Variadic wrapper around `cdd_ffi_vsnprintf_checked`.
 *
 * @param[out] out_buf Destination character buffer.
 * @param[in] out_cap Capacity of `out_buf` in bytes (including NUL).
 * @param[in] fmt Printf-style format string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL/zero
 * args, or CDD_C_ERROR_MEMORY if output was truncated.
 */
NO_DISCARD cdd_c_error_t cdd_ffi_snprintf_checked(char *out_buf, size_t out_cap,
                                                  const char *fmt, ...) {
  va_list ap;
  cdd_c_error_t rc;

  if (!out_buf || out_cap == 0 || !fmt)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  va_start(ap, fmt);
  rc = cdd_ffi_vsnprintf_checked(out_buf, out_cap, fmt, ap);
  if (rc != CDD_C_SUCCESS) {
    va_end(ap);
    return rc;
  }
  va_end(ap);
  return CDD_C_SUCCESS;
}

/*
 * @brief Formats and writes a `printf`-style string to `fp` with full error
 * percolation.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] fmt Printf-style format string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL args,
 * CDD_C_ERROR_MEMORY on OOM/truncation, or CDD_C_ERROR_IO on write failure.
 */
NO_DISCARD cdd_c_error_t cdd_ffi_write_fmt(FILE *fp, const char *fmt, ...) {
  char buf[4096];
  va_list ap;
  cdd_c_error_t rc;

  if (!fp || !fmt)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  va_start(ap, fmt);
  rc = cdd_ffi_vsnprintf_checked(buf, sizeof(buf), fmt, ap);
  if (rc != CDD_C_SUCCESS) {
    va_end(ap);
    return rc;
  }
  va_end(ap);

  return cdd_ffi_write_str(fp, buf);
}

/*
 * @brief Emits a linked list of FFI trivia tokens verbatim to an open file
 * stream.
 *
 * @param[in,out] f Output file stream.
 * @param[in] trivia Head of the FFI trivia chain to emit.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if `f`
 * is NULL.
 */
NO_DISCARD cdd_c_error_t cdd_ffi_emit_trivia(FILE *f,
                                             const cdd_ffi_trivia_t *trivia) {
  if (!f)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_ffi_emit_fail_step > 0 && --g_ffi_emit_fail_step == 0)
    return CDD_C_ERROR_IO;
#endif
  while (trivia) {
    if (trivia->text) {
      if (fputs(trivia->text, f) < 0)
        return CDD_C_ERROR_IO;
    }
    trivia = trivia->next;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes a comment segment while escaping nested block delimiters.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] src Source character buffer.
 * @param[in] len Number of bytes in `src`.
 * @param[in] style Comment style with optional nested delimiter escape rules.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
static NO_DISCARD cdd_c_error_t
emit_escaped_segment(FILE *fp, const char *src, size_t len,
                     const cdd_ffi_comment_style_t *style) {
  size_t open_len;
  size_t close_len;
  size_t i;
  size_t chunk_start;
  cdd_c_error_t rc;

  open_len = (style->nested_open_raw && style->nested_open_esc)
                 ? strlen(style->nested_open_raw)
                 : 0;
  close_len = (style->nested_close_raw && style->nested_close_esc)
                  ? strlen(style->nested_close_raw)
                  : 0;

  i = 0;
  chunk_start = 0;
  while (i < len) {
    if (open_len > 0 && i + open_len <= len &&
        memcmp(src + i, style->nested_open_raw, open_len) == 0) {
      if (i > chunk_start) {
        rc = cdd_ffi_write_bytes(fp, src + chunk_start, i - chunk_start);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      rc = cdd_ffi_write_str(fp, style->nested_open_esc);
      if (rc != CDD_C_SUCCESS)
        return rc;
      i += open_len;
      chunk_start = i;
    } else if (close_len > 0 && i + close_len <= len &&
               memcmp(src + i, style->nested_close_raw, close_len) == 0) {
      if (i > chunk_start) {
        rc = cdd_ffi_write_bytes(fp, src + chunk_start, i - chunk_start);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      rc = cdd_ffi_write_str(fp, style->nested_close_esc);
      if (rc != CDD_C_SUCCESS)
        return rc;
      i += close_len;
      chunk_start = i;
    } else {
      i++;
    }
  }

  if (len > chunk_start) {
    rc = cdd_ffi_write_bytes(fp, src + chunk_start, len - chunk_start);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Strips leading spaces/tabs, decorative C `*` markers, and trailing
 * whitespace from a single comment line.
 *
 * @param[in] raw_line Start of raw line slice.
 * @param[in] raw_len Length of raw line slice.
 * @param[in] strip_star Non-zero to strip a leading decorative `*` character.
 * @param[out] out_clean Receives start of cleaned text slice.
 * @param[out] out_len Receives byte length of cleaned text slice.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t clean_comment_line(const char *raw_line,
                                                   size_t raw_len,
                                                   int strip_star,
                                                   const char **out_clean,
                                                   size_t *out_len) {
  size_t start;
  size_t end;

#ifdef CDD_BUILD_TESTS
  if (g_ffi_emit_fail_step > 0 && --g_ffi_emit_fail_step == 0)
    return CDD_C_ERROR_IO;
#endif
  start = 0;
  end = raw_len;

  while (end > start &&
         memchr(" \t\r", (unsigned char)raw_line[end - 1], 3) != NULL) {
    end--;
  }
  while (start < end &&
         memchr(" \t", (unsigned char)raw_line[start], 2) != NULL) {
    start++;
  }
  if (strip_star && start < end && raw_line[start] == '*') {
    start++;
    if (start < end && raw_line[start] == ' ')
      start++;
  }

  *out_clean = raw_line + start;
  *out_len = end - start;
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits a single cleaned line using a line-comment prefix, wrapping at
 * word boundaries when `max_columns > 0`.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] indent Leading indentation string (non-NULL).
 * @param[in] prefix Line-comment prefix (non-NULL).
 * @param[in] line Cleaned comment line characters.
 * @param[in] len Byte length of `line`.
 * @param[in] max_columns Optional wrap limit (0 = unlimited).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
static NO_DISCARD cdd_c_error_t
emit_wrapped_line_comment(FILE *fp, const char *indent, const char *prefix,
                          const char *line, size_t len, size_t max_columns) {
  size_t indent_len;
  size_t prefix_len;
  size_t trim_prefix_len;
  size_t avail;
  size_t pos;
  cdd_c_error_t rc;

  indent_len = strlen(indent);
  prefix_len = strlen(prefix);

  if (len == 0) {
    trim_prefix_len = prefix_len;
    while (trim_prefix_len > 0 && prefix[trim_prefix_len - 1] == ' ') {
      trim_prefix_len--;
    }
    if (indent_len > 0) {
      rc = cdd_ffi_write_bytes(fp, indent, indent_len);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    rc = cdd_ffi_write_bytes(fp, prefix, trim_prefix_len);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_ffi_write_str(fp, "\n");
  }

  if (max_columns == 0 || indent_len + prefix_len + len <= max_columns) {
    if (indent_len > 0) {
      rc = cdd_ffi_write_bytes(fp, indent, indent_len);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    rc = cdd_ffi_write_bytes(fp, prefix, prefix_len);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_ffi_write_bytes(fp, line, len);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_ffi_write_str(fp, "\n");
  }

  avail = (max_columns > indent_len + prefix_len + 8)
              ? (max_columns - indent_len - prefix_len)
              : 16;
  pos = 0;
  while (pos < len) {
    size_t rem;
    size_t chunk;

    rem = len - pos;
    if (rem <= avail) {
      chunk = rem;
    } else {
      size_t k;
      chunk = 0;
      for (k = avail; k > 0; k--) {
        if (line[pos + k] == ' ') {
          chunk = k;
          break;
        }
      }
      if (chunk == 0)
        chunk = avail;
    }

    if (indent_len > 0) {
      rc = cdd_ffi_write_bytes(fp, indent, indent_len);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    rc = cdd_ffi_write_bytes(fp, prefix, prefix_len);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_ffi_write_bytes(fp, line + pos, chunk);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_ffi_write_str(fp, "\n");
    if (rc != CDD_C_SUCCESS)
      return rc;

    pos += chunk;
    while (pos < len && line[pos] == ' ')
      pos++;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Strips outer C block or single-line comment delimiters from a string.
 *
 * @param[in] raw Raw comment or docstring text.
 * @param[out] out_body Receives pointer to inner body slice.
 * @param[out] out_len Receives byte length of inner body slice.
 * @param[out] out_is_block Receives non-zero if block delimiters were
 * stripped.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t
strip_c_comment_delimiters(const char *raw, const char **out_body,
                           size_t *out_len, int *out_is_block) {
  size_t len;
  size_t start;
  size_t end;
  int is_block;

#ifdef CDD_BUILD_TESTS
  if (g_ffi_emit_fail_step > 0 && --g_ffi_emit_fail_step == 0)
    return CDD_C_ERROR_IO;
#endif
  len = strlen(raw);
  start = 0;
  end = len;
  is_block = 0;

  while (start < end && memchr(" \t", (unsigned char)raw[start], 2) != NULL) {
    start++;
  }
  while (end > start &&
         memchr(" \t\r\n", (unsigned char)raw[end - 1], 4) != NULL) {
    end--;
  }

  if (end - start >= 4 && memcmp(raw + start, "/*", 2) == 0 &&
      memcmp(raw + end - 2, "*/", 2) == 0) {
    is_block = 1;
    start += 2;
    end -= 2;
    if (start < end && memchr("*!", (unsigned char)raw[start], 2) != NULL &&
        (start + 1 == end ||
         memchr(" \t\r\n", (unsigned char)raw[start + 1], 4) != NULL)) {
      start++;
    }
  } else if (end - start >= 2 && memcmp(raw + start, "//", 2) == 0) {
    start += 2;
    if (start < end && memchr("/!", (unsigned char)raw[start], 2) != NULL) {
      start++;
    }
  }

  *out_body = raw + start;
  *out_len = end - start;
  *out_is_block = is_block;
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits a raw C comment or docstring body in the target language's
 * comment syntax.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] raw Raw comment or docstring string.
 * @param[in] force_block_star Non-zero if raw body came from a C block trivia
 * token without outer block-comment markers.
 * @param[in] indent Indentation string (non-NULL).
 * @param[in] style Target-language comment style.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static NO_DISCARD cdd_c_error_t
emit_comment_body(FILE *fp, const char *raw, int force_block_star,
                  const char *indent, const cdd_ffi_comment_style_t *style) {
  const char *body;
  size_t body_len;
  int is_block;
  int strip_star;
  const char *nl;
  cdd_c_error_t rc;

  rc = strip_c_comment_delimiters(raw, &body, &body_len, &is_block);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  strip_star = (is_block || force_block_star) ? 1 : 0;

  while (body_len > 0 && memchr("\r\n", (unsigned char)body[0], 2) != NULL) {
    body++;
    body_len--;
  }
  while (body_len > 0 &&
         memchr("\r\n", (unsigned char)body[body_len - 1], 2) != NULL) {
    body_len--;
  }

  nl = (const char *)memchr(body, '\n', body_len);

  if (style->line_prefix) {
    size_t pos;
    if (body_len == 0) {
      return emit_wrapped_line_comment(fp, indent, style->line_prefix, "", 0,
                                       style->max_columns);
    }
    pos = 0;
    for (;;) {
      const char *line_end;
      size_t slice_len;
      const char *clean_ptr;
      size_t clean_len;

      line_end = (const char *)memchr(body + pos, '\n', body_len - pos);
      slice_len = line_end ? (size_t)(line_end - (body + pos)) : body_len - pos;
      rc = clean_comment_line(body + pos, slice_len, strip_star, &clean_ptr,
                              &clean_len);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      rc = emit_wrapped_line_comment(fp, indent, style->line_prefix, clean_ptr,
                                     clean_len, style->max_columns);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }

      if (!line_end)
        break;
      pos += slice_len + 1;
    }
    return CDD_C_SUCCESS;
  }

  if (!nl) {
    const char *clean_ptr;
    size_t clean_len;

    rc = clean_comment_line(body, body_len, strip_star, &clean_ptr, &clean_len);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    if (indent[0] != '\0') {
      rc = cdd_ffi_write_str(fp, indent);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
    }
    rc = cdd_ffi_write_str(fp, style->block_open);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    rc = emit_escaped_segment(fp, clean_ptr, clean_len, style);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    rc = cdd_ffi_write_str(fp, style->block_close);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    return cdd_ffi_write_str(fp, "\n");
  } else {
    size_t pos;

    if (indent[0] != '\0') {
      rc = cdd_ffi_write_str(fp, indent);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
    }
    rc = cdd_ffi_write_str(fp, style->block_open);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    rc = cdd_ffi_write_str(fp, "\n");
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }

    pos = 0;
    for (;;) {
      const char *line_end;
      size_t slice_len;
      const char *clean_ptr;
      size_t clean_len;

      line_end = (const char *)memchr(body + pos, '\n', body_len - pos);
      slice_len = line_end ? (size_t)(line_end - (body + pos)) : body_len - pos;
      rc = clean_comment_line(body + pos, slice_len, strip_star, &clean_ptr,
                              &clean_len);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      if (indent[0] != '\0') {
        rc = cdd_ffi_write_str(fp, indent);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
      }
      if (clean_len > 0) {
        rc = cdd_ffi_write_str(fp, " * ");
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        rc = emit_escaped_segment(fp, clean_ptr, clean_len, style);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
      } else {
        rc = cdd_ffi_write_str(fp, " *");
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
      }
      rc = cdd_ffi_write_str(fp, "\n");
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }

      if (!line_end)
        break;
      pos += slice_len + 1;
    }

    if (indent[0] != '\0') {
      rc = cdd_ffi_write_str(fp, indent);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    rc = cdd_ffi_write_str(fp, style->block_close);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_ffi_write_str(fp, "\n");
  }
}

/*
 * @brief Translates C/C++ line and block trivia into target-language comments.
 *
 * @param[in,out] fp Open output file stream.
 * @param[in] trivia Linked list of FFI trivia nodes (may be NULL).
 * @param[in] style Target-language comment style configuration.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on invalid
 * arguments, or CDD_C_ERROR_IO on stream write failure.
 */
NO_DISCARD cdd_c_error_t
cdd_ffi_emit_translated_trivia(FILE *fp, const cdd_ffi_trivia_t *trivia,
                               const cdd_ffi_comment_style_t *style) {
  cdd_c_error_t rc;

  if (!fp || !style)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!style->line_prefix && (!style->block_open || !style->block_close))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  while (trivia) {
    if (trivia->text) {
      if (trivia->kind == CDD_FFI_TRIVIA_WHITESPACE) {
        size_t nl_count;
        const char *p;
        nl_count = 0;
        for (p = trivia->text; *p != '\0'; ++p) {
          if (*p == '\n')
            nl_count++;
        }
        if (nl_count >= 2) {
          rc = cdd_ffi_write_str(fp, "\n");
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      } else if (trivia->kind == CDD_FFI_TRIVIA_COMMENT_LINE ||
                 trivia->kind == CDD_FFI_TRIVIA_COMMENT_BLOCK) {
        int force_star;
        force_star = (trivia->kind == CDD_FFI_TRIVIA_COMMENT_BLOCK) ? 1 : 0;
        rc = emit_comment_body(fp, trivia->text, force_star, "", style);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    trivia = trivia->next;
  }
  return CDD_C_SUCCESS;
}

/*
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
NO_DISCARD cdd_c_error_t
cdd_ffi_emit_translated_doc(FILE *fp, const char *doc, const char *indent,
                            const cdd_ffi_comment_style_t *style) {
  const char *eff_indent;

  if (!fp || !style)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!style->line_prefix && (!style->block_open || !style->block_close))
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!doc || doc[0] == '\0')
    return CDD_C_SUCCESS;

  eff_indent = indent ? indent : "";
  return emit_comment_body(fp, doc, 1, eff_indent, style);
}

/*
 * @brief Maps a `cdd_ffi_type_t` to a target-language type string using a
 * lookup table.
 *
 * @param[in] table Language-specific type mapping lookup table.
 * @param[in] type FFI type descriptor to map.
 * @param[out] out_type Output pointer receiving the mapped target-language type
 * string.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * NULL.
 */
NO_DISCARD cdd_c_error_t
cdd_ffi_map_type_from_table(const cdd_ffi_type_map_table_t *table,
                            cdd_ffi_type_t type, const char **out_type) {
  if (!table || !out_type)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_ffi_emit_fail_step > 0 && --g_ffi_emit_fail_step == 0)
    return CDD_C_ERROR_IO;
#endif

  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      *out_type =
          table->char_ptr_type ? table->char_ptr_type : table->void_ptr_type;
      return CDD_C_SUCCESS;
    }
    *out_type = table->void_ptr_type;
    return CDD_C_SUCCESS;
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    *out_type = table->void_type;
    break;
  case CDD_FFI_KIND_BOOL:
    *out_type = table->bool_type;
    break;
  case CDD_FFI_KIND_INT8:
    *out_type = table->int8_type;
    break;
  case CDD_FFI_KIND_UINT8:
    *out_type = table->uint8_type;
    break;
  case CDD_FFI_KIND_INT16:
    *out_type = table->int16_type;
    break;
  case CDD_FFI_KIND_UINT16:
    *out_type = table->uint16_type;
    break;
  case CDD_FFI_KIND_INT32:
    *out_type = table->int32_type;
    break;
  case CDD_FFI_KIND_UINT32:
    *out_type = table->uint32_type;
    break;
  case CDD_FFI_KIND_INT64:
    *out_type = table->int64_type;
    break;
  case CDD_FFI_KIND_UINT64:
    *out_type = table->uint64_type;
    break;
  case CDD_FFI_KIND_FLOAT32:
    *out_type = table->float32_type;
    break;
  case CDD_FFI_KIND_FLOAT64:
    *out_type = table->float64_type;
    break;
  case CDD_FFI_KIND_STRUCT_REF:
    if (table->use_ref_name_for_struct && type.ref_name) {
      *out_type = type.ref_name;
    } else {
      *out_type = table->void_ptr_type;
    }
    break;
  case CDD_FFI_KIND_ENUM_REF:
    if (table->use_ref_name_for_enum && type.ref_name) {
      *out_type = type.ref_name;
    } else {
      *out_type = table->enum_type ? table->enum_type : table->int32_type;
    }
    break;
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (table->use_ref_name_for_typedef && type.ref_name) {
      *out_type = type.ref_name;
    } else {
      *out_type = table->void_ptr_type;
    }
    break;
  case CDD_FFI_KIND_FUNCTION_PTR:
    *out_type = table->function_ptr_type ? table->function_ptr_type
                                         : table->void_ptr_type;
    break;
  case CDD_FFI_KIND_OPAQUE_PTR:
  default:
    *out_type = table->void_ptr_type;
    break;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Opens an output file inside `output_dir` for FFI code emission.
 *
 * @param[in] output_dir Target output directory path.
 * @param[in] filename Output file basename.
 * @param[out] out_fp Output pointer receiving the opened `FILE *` stream.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_IO` /
 * `CDD_C_ERROR_INVALID_ARGUMENT` on failure.
 */
NO_DISCARD cdd_c_error_t cdd_ffi_open_output_file(const char *output_dir,
                                                  const char *filename,
                                                  FILE **out_fp) {
  char filepath[1024];
  FILE *fp;
  cdd_c_error_t rc;

  if (!output_dir || !filename || !out_fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_fp = NULL;
  fp = NULL;

#if defined(_WIN32)
  rc = cdd_ffi_snprintf_checked(filepath, sizeof(filepath), "%s\\%s",
                                output_dir, filename);
#else
  rc = cdd_ffi_snprintf_checked(filepath, sizeof(filepath), "%s/%s", output_dir,
                                filename);
#endif
  if (rc != CDD_C_SUCCESS)
    return rc;

#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after > 0 && --g_fail_io_after == 0) {
    return CDD_C_ERROR_IO;
  }
#endif

#if defined(_MSC_VER)
  if (fopen_s(&fp, filepath, "w") != 0)
    fp = NULL;
#else
  fp = fopen(filepath, "w");
#endif

  if (!fp)
    return CDD_C_ERROR_IO;

  *out_fp = fp;
  return CDD_C_SUCCESS;
}
