/**
 * @file arazzo_expr.c
 * @brief Implementation of Arazzo v1.1.0 Runtime Expression parser and
 * formatter.
 *
 * Parses and formats all 17 Arazzo 1.1.0 runtime expression types and embedded
 * `{...}` string templates in strict C89 with full error enum percolation.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "arazzo/parse/arazzo_expr.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Duplicates a substring of length `len` into `*out`.
 *
 * @param[in] start Pointer to start of substring.
 * @param[in] len Number of characters to copy.
 * @param[out] out Output pointer receiving newly allocated null-terminated
 * string.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t dup_slice(const char *start, size_t len, char **out) {
  char *buf;
  buf = (char *)C_CDD_MALLOC(len + 1);
  if (!buf)
    return CDD_C_ERROR_MEMORY;
  memcpy(buf, start, len);
  buf[len] = '\0';
  *out = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Splits an optional `#<json-pointer>` suffix from `rest` into
 * `out_expr->json_pointer`.
 *
 * @param[in] rest Input string after prefix.
 * @param[in,out] out_expr Target runtime expression struct.
 * @param[out] out_head_len Length of `rest` prior to `#`.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if `#` is
 * empty, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t extract_json_pointer(const char *rest,
                                          struct Arazzo_RuntimeExpr *out_expr,
                                          size_t *out_head_len) {
  const char *hash_pos;
  hash_pos = strchr(rest, '#');
  if (!hash_pos) {
    *out_head_len = strlen(rest);
    return CDD_C_SUCCESS;
  }
  if (hash_pos[1] == '\0')
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_head_len = (size_t)(hash_pos - rest);
  out_expr->json_pointer = C_CDD_STRDUP(hash_pos + 1);
  if (!out_expr->json_pointer)
    return CDD_C_ERROR_MEMORY;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a two-part `<first>.<second>` or `<first>.outputs.<second>`
 * segment.
 *
 * @param[in] rest String starting after `$steps.`, `$workflows.`,
 * `$sourceDescriptions.`, or `$components.`.
 * @param[in] head_len Length of `rest` excluding any `#...` suffix.
 * @param[out] first_out Output receiving first segment.
 * @param[out] second_out Output receiving second segment (after optional
 * `.outputs.` or `.inputs.`).
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * malformed, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t parse_two_segments(const char *rest, size_t head_len,
                                        char **first_out, char **second_out) {
  const char *dot;
  size_t first_len;
  const char *rem;
  size_t rem_len;
  cdd_c_error_t rc;

  dot = (const char *)memchr(rest, '.', head_len);
  if (!dot || dot == rest)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  first_len = (size_t)(dot - rest);
  rem = dot + 1;
  rem_len = head_len - first_len - 1;

  if (rem_len >= 8 && strncmp(rem, "outputs.", 8) == 0) {
    rem += 8;
    rem_len -= 8;
  } else if (rem_len >= 7 && strncmp(rem, "inputs.", 7) == 0) {
    rem += 7;
    rem_len -= 7;
  }
  if (rem_len == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = dup_slice(rest, first_len, first_out);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = dup_slice(rem, rem_len, second_out);
  return rc;
}

/**
 * @brief Parses an Arazzo v1.1.0 runtime expression string into an
 * `Arazzo_RuntimeExpr`.
 *
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * malformed, or `CDD_C_ERROR_MEMORY` on OOM.
 */
cdd_c_error_t arazzo_runtime_expr_parse(const char *expr_str,
                                        struct Arazzo_RuntimeExpr *out_expr) {
  size_t head_len = 0;
  cdd_c_error_t rc;
  const char *open_brace;

  if (!expr_str || !out_expr)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(out_expr, 0, sizeof(*out_expr));
  out_expr->raw = C_CDD_STRDUP(expr_str);
  if (!out_expr->raw)
    return CDD_C_ERROR_MEMORY;

  if (expr_str[0] != '$') {
    open_brace = strstr(expr_str, "{$");
    if (open_brace) {
      const char *close_brace = strchr(open_brace + 2, '}');
      if (!close_brace || close_brace == open_brace + 2) {
        arazzo_runtime_expr_free(out_expr);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      out_expr->is_embedded_template = 1;
    }
    out_expr->kind = ARAZZO_EXPR_LITERAL;
    return CDD_C_SUCCESS;
  }

  if (strcmp(expr_str, "$url") == 0) {
    out_expr->kind = ARAZZO_EXPR_URL;
    return CDD_C_SUCCESS;
  }
  if (strcmp(expr_str, "$method") == 0) {
    out_expr->kind = ARAZZO_EXPR_METHOD;
    return CDD_C_SUCCESS;
  }
  if (strcmp(expr_str, "$statusCode") == 0) {
    out_expr->kind = ARAZZO_EXPR_STATUS_CODE;
    return CDD_C_SUCCESS;
  }
  if (strcmp(expr_str, "$self") == 0) {
    out_expr->kind = ARAZZO_EXPR_SELF;
    return CDD_C_SUCCESS;
  }

  /* Single-property prefixes ($request.header., $request.query.,
   * $request.path., $response.header., $message.header., $inputs., $outputs.)
   */
  if (strncmp(expr_str, "$request.header.", 16) == 0 ||
      strncmp(expr_str, "$request.query.", 15) == 0 ||
      strncmp(expr_str, "$request.path.", 14) == 0 ||
      strncmp(expr_str, "$response.header.", 17) == 0 ||
      strncmp(expr_str, "$message.header.", 16) == 0 ||
      strncmp(expr_str, "$inputs.", 8) == 0 ||
      strncmp(expr_str, "$outputs.", 9) == 0) {
    size_t pfx_len;
    if (strncmp(expr_str, "$request.header.", 16) == 0) {
      out_expr->kind = ARAZZO_EXPR_REQUEST_HEADER;
      pfx_len = 16;
    } else if (strncmp(expr_str, "$request.query.", 15) == 0) {
      out_expr->kind = ARAZZO_EXPR_REQUEST_QUERY;
      pfx_len = 15;
    } else if (strncmp(expr_str, "$request.path.", 14) == 0) {
      out_expr->kind = ARAZZO_EXPR_REQUEST_PATH;
      pfx_len = 14;
    } else if (strncmp(expr_str, "$response.header.", 17) == 0) {
      out_expr->kind = ARAZZO_EXPR_RESPONSE_HEADER;
      pfx_len = 17;
    } else if (strncmp(expr_str, "$message.header.", 16) == 0) {
      out_expr->kind = ARAZZO_EXPR_MESSAGE_HEADER;
      pfx_len = 16;
    } else if (strncmp(expr_str, "$inputs.", 8) == 0) {
      out_expr->kind = ARAZZO_EXPR_INPUTS;
      pfx_len = 8;
    } else {
      out_expr->kind = ARAZZO_EXPR_OUTPUTS;
      pfx_len = 9;
    }
    rc = extract_json_pointer(expr_str + pfx_len, out_expr, &head_len);
    if (rc != CDD_C_SUCCESS) {
      arazzo_runtime_expr_free(out_expr);
      return rc;
    }
    if (head_len == 0) {
      arazzo_runtime_expr_free(out_expr);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    rc = dup_slice(expr_str + pfx_len, head_len, &out_expr->property_name);
    if (rc != CDD_C_SUCCESS) {
      arazzo_runtime_expr_free(out_expr);
      return rc;
    }
    return CDD_C_SUCCESS;
  }

  /* Body / message payload & header roots */
  if (strncmp(expr_str, "$request.body", 13) == 0 ||
      strncmp(expr_str, "$response.body", 14) == 0 ||
      strncmp(expr_str, "$message.header", 15) == 0 ||
      strncmp(expr_str, "$message.payload", 16) == 0) {
    size_t pfx_len;
    if (strncmp(expr_str, "$request.body", 13) == 0) {
      out_expr->kind = ARAZZO_EXPR_REQUEST_BODY;
      pfx_len = 13;
    } else if (strncmp(expr_str, "$response.body", 14) == 0) {
      out_expr->kind = ARAZZO_EXPR_RESPONSE_BODY;
      pfx_len = 14;
    } else if (strncmp(expr_str, "$message.header", 15) == 0) {
      out_expr->kind = ARAZZO_EXPR_MESSAGE_HEADER;
      pfx_len = 15;
    } else {
      out_expr->kind = ARAZZO_EXPR_MESSAGE_PAYLOAD;
      pfx_len = 16;
    }
    if (expr_str[pfx_len] != '\0' && expr_str[pfx_len] != '#') {
      arazzo_runtime_expr_free(out_expr);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    rc = extract_json_pointer(expr_str + pfx_len, out_expr, &head_len);
    if (rc != CDD_C_SUCCESS) {
      arazzo_runtime_expr_free(out_expr);
      return rc;
    }
    return CDD_C_SUCCESS;
  }

  /* Two-segment prefixes ($steps., $workflows., $sourceDescriptions.,
   * $components.) */
  if (strncmp(expr_str, "$steps.", 7) == 0 ||
      strncmp(expr_str, "$workflows.", 11) == 0 ||
      strncmp(expr_str, "$sourceDescriptions.", 20) == 0 ||
      strncmp(expr_str, "$components.", 12) == 0) {
    size_t pfx_len;
    char **first_target;
    if (strncmp(expr_str, "$steps.", 7) == 0) {
      out_expr->kind = ARAZZO_EXPR_STEPS;
      pfx_len = 7;
      first_target = &out_expr->step_or_workflow_id;
    } else if (strncmp(expr_str, "$workflows.", 11) == 0) {
      out_expr->kind = ARAZZO_EXPR_WORKFLOWS;
      pfx_len = 11;
      first_target = &out_expr->step_or_workflow_id;
    } else if (strncmp(expr_str, "$sourceDescriptions.", 20) == 0) {
      out_expr->kind = ARAZZO_EXPR_SOURCE_DESCRIPTIONS;
      pfx_len = 20;
      first_target = &out_expr->source_name;
    } else {
      out_expr->kind = ARAZZO_EXPR_COMPONENTS;
      pfx_len = 12;
      first_target = &out_expr->source_name;
    }
    rc = extract_json_pointer(expr_str + pfx_len, out_expr, &head_len);
    if (rc != CDD_C_SUCCESS) {
      arazzo_runtime_expr_free(out_expr);
      return rc;
    }
    rc = parse_two_segments(expr_str + pfx_len, head_len, first_target,
                            &out_expr->property_name);
    if (rc != CDD_C_SUCCESS) {
      arazzo_runtime_expr_free(out_expr);
      return rc;
    }
    return CDD_C_SUCCESS;
  }

  arazzo_runtime_expr_free(out_expr);
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Formats a parsed `Arazzo_RuntimeExpr` back into its canonical string
 * representation.
 *
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_runtime_expr_format(const struct Arazzo_RuntimeExpr *expr,
                                         char **out_str) {
  char buf[512];
  const char *first_seg = NULL;

  if (!expr || !out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (expr->raw) {
    *out_str = C_CDD_STRDUP(expr->raw);
    return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }

  switch (expr->kind) {
  case ARAZZO_EXPR_URL:
    CDD_SNPRINTF(buf, sizeof(buf), "%s", "$url");
    break;
  case ARAZZO_EXPR_METHOD:
    CDD_SNPRINTF(buf, sizeof(buf), "%s", "$method");
    break;
  case ARAZZO_EXPR_STATUS_CODE:
    CDD_SNPRINTF(buf, sizeof(buf), "%s", "$statusCode");
    break;
  case ARAZZO_EXPR_SELF:
    CDD_SNPRINTF(buf, sizeof(buf), "%s", "$self");
    break;
  case ARAZZO_EXPR_REQUEST_HEADER:
  case ARAZZO_EXPR_REQUEST_QUERY:
  case ARAZZO_EXPR_REQUEST_PATH:
  case ARAZZO_EXPR_RESPONSE_HEADER:
  case ARAZZO_EXPR_INPUTS:
  case ARAZZO_EXPR_OUTPUTS: {
    const char *pfx = "$outputs.";
    if (!expr->property_name)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (expr->kind == ARAZZO_EXPR_REQUEST_HEADER)
      pfx = "$request.header.";
    else if (expr->kind == ARAZZO_EXPR_REQUEST_QUERY)
      pfx = "$request.query.";
    else if (expr->kind == ARAZZO_EXPR_REQUEST_PATH)
      pfx = "$request.path.";
    else if (expr->kind == ARAZZO_EXPR_RESPONSE_HEADER)
      pfx = "$response.header.";
    else if (expr->kind == ARAZZO_EXPR_INPUTS)
      pfx = "$inputs.";
    CDD_SNPRINTF(buf, sizeof(buf), "%s%s", pfx, expr->property_name);
    break;
  }
  case ARAZZO_EXPR_REQUEST_BODY:
  case ARAZZO_EXPR_RESPONSE_BODY:
  case ARAZZO_EXPR_MESSAGE_PAYLOAD: {
    const char *base = "$message.payload";
    if (expr->kind == ARAZZO_EXPR_REQUEST_BODY)
      base = "$request.body";
    else if (expr->kind == ARAZZO_EXPR_RESPONSE_BODY)
      base = "$response.body";
    if (expr->json_pointer)
      CDD_SNPRINTF(buf, sizeof(buf), "%s#%s", base, expr->json_pointer);
    else
      CDD_SNPRINTF(buf, sizeof(buf), "%s", base);
    break;
  }
  case ARAZZO_EXPR_MESSAGE_HEADER:
    if (expr->property_name)
      CDD_SNPRINTF(buf, sizeof(buf), "$message.header.%s", expr->property_name);
    else if (expr->json_pointer)
      CDD_SNPRINTF(buf, sizeof(buf), "$message.header#%s", expr->json_pointer);
    else
      CDD_SNPRINTF(buf, sizeof(buf), "%s", "$message.header");
    break;
  case ARAZZO_EXPR_STEPS:
  case ARAZZO_EXPR_WORKFLOWS:
  case ARAZZO_EXPR_SOURCE_DESCRIPTIONS:
  case ARAZZO_EXPR_COMPONENTS: {
    const char *pfx = "$components.";
    const char *mid = ".";
    first_seg =
        (expr->kind == ARAZZO_EXPR_STEPS || expr->kind == ARAZZO_EXPR_WORKFLOWS)
            ? expr->step_or_workflow_id
            : expr->source_name;
    if (!first_seg || !expr->property_name)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (expr->kind == ARAZZO_EXPR_STEPS) {
      pfx = "$steps.";
      mid = ".outputs.";
    } else if (expr->kind == ARAZZO_EXPR_WORKFLOWS) {
      pfx = "$workflows.";
      mid = ".outputs.";
    } else if (expr->kind == ARAZZO_EXPR_SOURCE_DESCRIPTIONS) {
      pfx = "$sourceDescriptions.";
    }
    CDD_SNPRINTF(buf, sizeof(buf), "%s%s%s%s", pfx, first_seg, mid,
                 expr->property_name);
    break;
  }
  default:
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  *out_str = C_CDD_STRDUP(buf);
  return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}
