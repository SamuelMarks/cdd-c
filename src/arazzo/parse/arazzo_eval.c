/**
 * @file arazzo_eval.c
 * @brief Implementation of Arazzo v1.1.0 runtime expression, JSON Pointer,
 * JSONPath, XPath, criterion, selector, and payload replacement evaluation.
 *
 * Strictly adheres to ISO C90 (C89) and returns `cdd_c_error_t` on all
 * non-void functions.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "arazzo/parse/arazzo_eval.h"
#include "arazzo/parse/arazzo_expr.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Unescapes an RFC 6901 JSON Pointer reference token (`~1` -> `/`, `~0`
 * -> `~`).
 *
 * @param[in] start Token start pointer.
 * @param[in] len Token byte length.
 * @param[out] out_token Newly allocated unescaped token string.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` on bad `~`
 * escape, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t unescape_pointer_token(const char *start, size_t len,
                                            char **out_token) {
  char *buf;
  size_t i;
  size_t j = 0;

  buf = (char *)C_CDD_MALLOC(len + 1);
  if (!buf)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < len; ++i) {
    if (start[i] == '~') {
      if (i + 1 < len && start[i + 1] == '1') {
        buf[j++] = '/';
        ++i;
      } else if (i + 1 < len && start[i + 1] == '0') {
        buf[j++] = '~';
        ++i;
      } else {
        C_CDD_FREE(buf);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    } else {
      buf[j++] = start[i];
    }
  }
  buf[j] = '\0';
  *out_token = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Converts a `JSON_Value` into a newly allocated string representation.
 *
 * @param[in] val Source JSON value.
 * @param[out] out_str Newly allocated string.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_NOT_FOUND` if NULL,
 * or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t json_val_to_str(const JSON_Value *val, char **out_str) {
  char num_buf[64];
  char *ser;

  if (!val)
    return CDD_C_ERROR_NOT_FOUND;

  switch (json_value_get_type(val)) {
  case JSONString:
    *out_str = C_CDD_STRDUP(json_value_get_string(val));
    return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  case JSONNumber:
    CDD_SNPRINTF(num_buf, sizeof(num_buf), "%.15g", json_value_get_number(val));
    *out_str = C_CDD_STRDUP(num_buf);
    return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  case JSONBoolean:
    *out_str = C_CDD_STRDUP(json_value_get_boolean(val) ? "true" : "false");
    return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  case JSONNull:
    *out_str = C_CDD_STRDUP("null");
    return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  default:
    ser = json_serialize_to_string(val);
    *out_str = C_CDD_STRDUP(ser);
    json_free_serialized_string(ser);
    return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }
}

/**
 * @brief Evaluates an RFC 6901 JSON Pointer against a Parson `JSON_Value`.
 *
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_NOT_FOUND` if missing, or
 * `CDD_C_ERROR_INVALID_ARGUMENT` if malformed.
 */
cdd_c_error_t arazzo_eval_json_pointer(const JSON_Value *root,
                                       const char *pointer,
                                       const JSON_Value **out_val) {
  const char *p;
  const JSON_Value *cur;

  if (!root || !pointer || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  p = pointer;
  if (*p == '#')
    ++p;
  if (*p == '\0') {
    *out_val = root;
    return CDD_C_SUCCESS;
  }
  if (*p != '/')
    return CDD_C_ERROR_INVALID_ARGUMENT;

  cur = root;
  while (*p == '/') {
    const char *slash;
    size_t tok_len;
    char *tok = NULL;
    cdd_c_error_t rc;

    ++p;
    slash = strchr(p, '/');
    tok_len = slash ? (size_t)(slash - p) : strlen(p);
    rc = unescape_pointer_token(p, tok_len, &tok);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (json_value_get_type(cur) == JSONObject) {
      cur = json_object_get_value(json_value_get_object(cur), tok);
    } else if (json_value_get_type(cur) == JSONArray) {
      char *endp = NULL;
      unsigned long idx = strtoul(tok, &endp, 10);
      if (tok[0] == '\0' || *endp != '\0') {
        C_CDD_FREE(tok);
        return CDD_C_ERROR_NOT_FOUND;
      }
      cur = json_array_get_value(json_value_get_array(cur), (size_t)idx);
    } else {
      cur = NULL;
    }

    C_CDD_FREE(tok);
    if (!cur)
      return CDD_C_ERROR_NOT_FOUND;
    p += tok_len;
  }

  *out_val = cur;
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates an RFC 9535 JSONPath expression against a Parson
 * `JSON_Value`.
 *
 * Supports dot selectors (`$.a.b`), wildcard selectors (`$.*`, `$[*]`),
 * bracketed string keys (`$['a']`, `$["a"]`), non-negative and negative array
 * indices (`$[0]`, `$[-1]`), and returns `CDD_C_ERROR_NOT_SUPPORTED` for
 * slices (`[start:end]`) and filter selectors (`[?(...)]`).
 *
 * @return `CDD_C_SUCCESS` on match, `CDD_C_ERROR_NOT_FOUND` if not matched,
 * `CDD_C_ERROR_NOT_SUPPORTED` for unsupported RFC 9535 selectors, or
 * `CDD_C_ERROR_INVALID_ARGUMENT` if syntax is invalid.
 */
cdd_c_error_t arazzo_eval_jsonpath(const JSON_Value *root, const char *jsonpath,
                                   const JSON_Value **out_val) {
  const char *p;
  const JSON_Value *cur;

  if (!root || !jsonpath || !out_val || jsonpath[0] != '$')
    return CDD_C_ERROR_INVALID_ARGUMENT;

  p = jsonpath + 1;
  cur = root;

  while (*p != '\0') {
    if (*p == '.') {
      const char *start;
      size_t len = 0;
      char *key;

      ++p;
      if (*p == '*') {
        ++p;
        if (json_value_get_type(cur) == JSONObject) {
          cur = json_object_get_value_at(json_value_get_object(cur), 0);
        } else if (json_value_get_type(cur) == JSONArray) {
          cur = json_array_get_value(json_value_get_array(cur), 0);
        } else {
          cur = NULL;
        }
        if (!cur)
          return CDD_C_ERROR_NOT_FOUND;
        continue;
      }

      start = p;
      while (*p != '\0' && *p != '.' && *p != '[') {
        ++p;
        ++len;
      }
      if (len == 0)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      if (json_value_get_type(cur) != JSONObject)
        return CDD_C_ERROR_NOT_FOUND;

      key = (char *)C_CDD_MALLOC(len + 1);
      if (!key)
        return CDD_C_ERROR_MEMORY;
      memcpy(key, start, len);
      key[len] = '\0';
      cur = json_object_get_value(json_value_get_object(cur), key);
      C_CDD_FREE(key);
      if (!cur)
        return CDD_C_ERROR_NOT_FOUND;
    } else if (*p == '[') {
      ++p;
      if (*p == '?')
        return CDD_C_ERROR_NOT_SUPPORTED;
      if (strncmp(p, "*]", 2) == 0) {
        p += 2;
        if (json_value_get_type(cur) == JSONArray) {
          cur = json_array_get_value(json_value_get_array(cur), 0);
        } else if (json_value_get_type(cur) == JSONObject) {
          cur = json_object_get_value_at(json_value_get_object(cur), 0);
        } else {
          cur = NULL;
        }
        if (!cur)
          return CDD_C_ERROR_NOT_FOUND;
      } else if (*p == 39 || *p == '"') {
        char quote = *p++;
        const char *endq = strchr(p, quote);
        size_t klen;
        char *key;
        if (!endq || endq[1] != ']')
          return CDD_C_ERROR_INVALID_ARGUMENT;
        if (json_value_get_type(cur) != JSONObject)
          return CDD_C_ERROR_NOT_FOUND;
        klen = (size_t)(endq - p);
        key = (char *)C_CDD_MALLOC(klen + 1);
        if (!key)
          return CDD_C_ERROR_MEMORY;
        memcpy(key, p, klen);
        key[klen] = '\0';
        cur = json_object_get_value(json_value_get_object(cur), key);
        C_CDD_FREE(key);
        if (!cur)
          return CDD_C_ERROR_NOT_FOUND;
        p = endq + 2;
      } else {
        char *endp = NULL;
        long idx;
        size_t arr_cnt;
        size_t eff_idx;

        if (*p != '-' && !isdigit((unsigned char)*p))
          return CDD_C_ERROR_INVALID_ARGUMENT;
        idx = strtol(p, &endp, 10);
        if (*endp == ':')
          return CDD_C_ERROR_NOT_SUPPORTED;
        if (*endp != ']')
          return CDD_C_ERROR_INVALID_ARGUMENT;
        if (json_value_get_type(cur) != JSONArray)
          return CDD_C_ERROR_NOT_FOUND;
        arr_cnt = json_array_get_count(json_value_get_array(cur));
        if (idx < 0) {
          long pos_idx = (long)arr_cnt + idx;
          if (pos_idx < 0)
            return CDD_C_ERROR_NOT_FOUND;
          eff_idx = (size_t)pos_idx;
        } else {
          eff_idx = (size_t)idx;
        }
        cur = json_array_get_value(json_value_get_array(cur), eff_idx);
        if (!cur)
          return CDD_C_ERROR_NOT_FOUND;
        p = endp + 1;
      }
    } else {
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }

  *out_val = cur;
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates a hierarchical XPath query against an XML string.
 *
 * Walks each `/segment` in hierarchy order, supporting both double- and
 * single-quoted XML attributes (`/@attr`) and namespace-prefixed element tags.
 *
 * @return `CDD_C_SUCCESS` on match, `CDD_C_ERROR_NOT_FOUND` if not found, or
 * error code.
 */
cdd_c_error_t arazzo_eval_xpath(const char *xml_str, const char *xpath,
                                char **out_str) {
  const char *xp;
  const char *cur_xml;
  const char *last_open_tag = NULL;
  char last_elem[128];

  if (!xml_str || !xpath || !out_str || xpath[0] != '/')
    return CDD_C_ERROR_INVALID_ARGUMENT;

  xp = xpath;
  cur_xml = xml_str;
  last_elem[0] = '\0';

  while (*xp == '/') {
    const char *slash;
    size_t seg_len;
    char seg[128];

    ++xp;
    slash = strchr(xp, '/');
    seg_len = slash ? (size_t)(slash - xp) : strlen(xp);
    if (seg_len == 0)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    memcpy(seg, xp, seg_len);
    seg[seg_len] = '\0';
    xp += seg_len;

    if (seg[0] == '@') {
      const char *attr_name = seg + 1;
      const char *search_base = last_open_tag ? last_open_tag : cur_xml;
      char pat_dq[128];
      char pat_sq[128];
      const char *found;
      char quote_ch = '"';
      const char *val_start;
      const char *val_end;
      size_t len;
      char *res;

      if (*attr_name == '\0' || *xp != '\0')
        return CDD_C_ERROR_INVALID_ARGUMENT;

      CDD_SNPRINTF(pat_dq, sizeof(pat_dq), "%s=%c", attr_name, '"');
      CDD_SNPRINTF(pat_sq, sizeof(pat_sq), "%s=%c", attr_name, 39);
      found = strstr(search_base, pat_dq);
      if (!found) {
        found = strstr(search_base, pat_sq);
        quote_ch = 39;
      }
      if (!found || !strchr(found, '>'))
        return CDD_C_ERROR_NOT_FOUND;

      val_start = found + strlen(attr_name) + 2;
      val_end = strchr(val_start, quote_ch);
      if (!val_end)
        return CDD_C_ERROR_NOT_FOUND;
      len = (size_t)(val_end - val_start);
      res = (char *)C_CDD_MALLOC(len + 1);
      if (!res)
        return CDD_C_ERROR_MEMORY;
      memcpy(res, val_start, len);
      res[len] = '\0';
      *out_str = res;
      return CDD_C_SUCCESS;
    } else {
      char open_tag[132];
      const char *found = cur_xml;
      size_t open_len;

      CDD_SNPRINTF(open_tag, sizeof(open_tag), "<%s", seg);
      open_len = strlen(open_tag);
      for (;;) {
        char next_ch;
        found = strstr(found, open_tag);
        if (!found)
          return CDD_C_ERROR_NOT_FOUND;
        next_ch = found[open_len];
        if (next_ch == '>' || isspace((unsigned char)next_ch))
          break;
        found += open_len;
      }
      last_open_tag = found;
      CDD_SNPRINTF(last_elem, sizeof(last_elem), "%s", seg);
      cur_xml = strchr(found, '>') + 1;
    }
  }

  {
    char close_tag[132];
    const char *end_tag;
    size_t len;
    char *res;

    CDD_SNPRINTF(close_tag, sizeof(close_tag), "</%s>", last_elem);
    end_tag = strstr(cur_xml, close_tag);
    if (!end_tag)
      return CDD_C_ERROR_NOT_FOUND;
    len = (size_t)(end_tag - cur_xml);
    res = (char *)C_CDD_MALLOC(len + 1);
    if (!res)
      return CDD_C_ERROR_MEMORY;
    memcpy(res, cur_xml, len);
    res[len] = '\0';
    *out_str = res;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Resolves a single non-template `Arazzo_RuntimeExpr` against `ctx`.
 *
 * @param[in] expr Parsed runtime expression.
 * @param[in] ctx Execution context.
 * @param[out] out_str Newly allocated resolved string.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_NOT_FOUND` if missing,
 * or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t
eval_single_expr_to_str(const struct Arazzo_RuntimeExpr *expr,
                        const struct Arazzo_ExecutionContext *ctx,
                        char **out_str) {
  char num_buf[32];
  const JSON_Value *target_val = NULL;
  const JSON_Object *sub_obj = NULL;
  cdd_c_error_t rc;

  switch (expr->kind) {
  case ARAZZO_EXPR_LITERAL:
    *out_str = C_CDD_STRDUP(expr->raw);
    return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  case ARAZZO_EXPR_URL:
  case ARAZZO_EXPR_METHOD:
  case ARAZZO_EXPR_SELF: {
    const char *str_val = ctx->self_uri;
    if (expr->kind == ARAZZO_EXPR_URL)
      str_val = ctx->url;
    else if (expr->kind == ARAZZO_EXPR_METHOD)
      str_val = ctx->method;
    if (!str_val)
      return CDD_C_ERROR_NOT_FOUND;
    *out_str = C_CDD_STRDUP(str_val);
    return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }
  case ARAZZO_EXPR_STATUS_CODE:
    CDD_SNPRINTF(num_buf, sizeof(num_buf), "%d", ctx->status_code);
    *out_str = C_CDD_STRDUP(num_buf);
    return *out_str ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  case ARAZZO_EXPR_REQUEST_HEADER:
  case ARAZZO_EXPR_REQUEST_QUERY:
  case ARAZZO_EXPR_REQUEST_PATH:
  case ARAZZO_EXPR_RESPONSE_HEADER:
  case ARAZZO_EXPR_INPUTS:
  case ARAZZO_EXPR_OUTPUTS: {
    const JSON_Object *map = ctx->outputs;
    if (expr->kind == ARAZZO_EXPR_REQUEST_HEADER)
      map = ctx->req_headers;
    else if (expr->kind == ARAZZO_EXPR_REQUEST_QUERY)
      map = ctx->req_query;
    else if (expr->kind == ARAZZO_EXPR_REQUEST_PATH)
      map = ctx->req_path;
    else if (expr->kind == ARAZZO_EXPR_RESPONSE_HEADER)
      map = ctx->resp_headers;
    else if (expr->kind == ARAZZO_EXPR_INPUTS)
      map = ctx->inputs;
    target_val = map ? json_object_get_value(map, expr->property_name) : NULL;
    if (target_val && expr->json_pointer) {
      rc =
          arazzo_eval_json_pointer(target_val, expr->json_pointer, &target_val);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return json_val_to_str(target_val, out_str);
  }
  case ARAZZO_EXPR_REQUEST_BODY:
  case ARAZZO_EXPR_RESPONSE_BODY:
  case ARAZZO_EXPR_MESSAGE_PAYLOAD: {
    const JSON_Value *body = ctx->msg_payload;
    if (expr->kind == ARAZZO_EXPR_REQUEST_BODY)
      body = ctx->req_body;
    else if (expr->kind == ARAZZO_EXPR_RESPONSE_BODY)
      body = ctx->resp_body;
    if (!body)
      return CDD_C_ERROR_NOT_FOUND;
    if (expr->json_pointer) {
      rc = arazzo_eval_json_pointer(body, expr->json_pointer, &target_val);
      if (rc != CDD_C_SUCCESS)
        return rc;
      return json_val_to_str(target_val, out_str);
    }
    return json_val_to_str(body, out_str);
  }
  case ARAZZO_EXPR_MESSAGE_HEADER:
    if (!ctx->msg_header)
      return CDD_C_ERROR_NOT_FOUND;
    if (expr->property_name) {
      target_val = json_object_get_value(json_value_get_object(ctx->msg_header),
                                         expr->property_name);
    } else if (expr->json_pointer) {
      rc = arazzo_eval_json_pointer(ctx->msg_header, expr->json_pointer,
                                    &target_val);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      target_val = ctx->msg_header;
    }
    return json_val_to_str(target_val, out_str);
  default: {
    const JSON_Object *outer = ctx->components;
    const char *key = expr->source_name;
    if (expr->kind == ARAZZO_EXPR_STEPS) {
      outer = ctx->steps;
      key = expr->step_or_workflow_id;
    } else if (expr->kind == ARAZZO_EXPR_WORKFLOWS) {
      outer = ctx->workflows;
      key = expr->step_or_workflow_id;
    } else if (expr->kind == ARAZZO_EXPR_SOURCE_DESCRIPTIONS) {
      outer = ctx->sources;
    }
    sub_obj = outer ? json_object_get_object(outer, key) : NULL;
    target_val =
        sub_obj ? json_object_get_value(sub_obj, expr->property_name) : NULL;
    if (target_val && expr->json_pointer) {
      rc =
          arazzo_eval_json_pointer(target_val, expr->json_pointer, &target_val);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return json_val_to_str(target_val, out_str);
  }
  }
}

/**
 * @brief Evaluates an Arazzo runtime expression or embedded template string to
 * a newly allocated string.
 *
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_NOT_FOUND` if missing, or
 * error code.
 */
cdd_c_error_t
arazzo_eval_runtime_expr_to_str(const char *expr_str,
                                const struct Arazzo_ExecutionContext *ctx,
                                char **out_str) {
  struct Arazzo_RuntimeExpr parsed;
  cdd_c_error_t rc;

  if (!expr_str || !ctx || !out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = arazzo_runtime_expr_parse(expr_str, &parsed);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (parsed.is_embedded_template) {
    char *acc;
    const char *cursor = expr_str;
    arazzo_runtime_expr_free(&parsed);

    acc = C_CDD_STRDUP("");
    if (!acc)
      return CDD_C_ERROR_MEMORY;

    while (*cursor != '\0') {
      const char *open_b = strstr(cursor, "{$");
      if (!open_b) {
        size_t old_len = strlen(acc);
        size_t tail_len = strlen(cursor);
        char *next_acc = (char *)C_CDD_REALLOC(acc, old_len + tail_len + 1);
        if (!next_acc) {
          C_CDD_FREE(acc);
          return CDD_C_ERROR_MEMORY;
        }
        acc = next_acc;
        memcpy(acc + old_len, cursor, tail_len + 1);
        break;
      } else {
        const char *close_b = strchr(open_b + 2, '}');
        size_t prefix_len = (size_t)(open_b - cursor);
        size_t inner_len = (size_t)(close_b - (open_b + 1));
        char *inner_expr;
        char *resolved = NULL;
        size_t old_len;
        size_t res_len;
        char *next_acc;

        inner_expr = (char *)C_CDD_MALLOC(inner_len + 1);
        if (!inner_expr) {
          C_CDD_FREE(acc);
          return CDD_C_ERROR_MEMORY;
        }
        memcpy(inner_expr, open_b + 1, inner_len);
        inner_expr[inner_len] = '\0';

        rc = arazzo_eval_runtime_expr_to_str(inner_expr, ctx, &resolved);
        C_CDD_FREE(inner_expr);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(acc);
          return rc;
        }

        old_len = strlen(acc);
        res_len = strlen(resolved);
        next_acc =
            (char *)C_CDD_REALLOC(acc, old_len + prefix_len + res_len + 1);
        if (!next_acc) {
          C_CDD_FREE(resolved);
          C_CDD_FREE(acc);
          return CDD_C_ERROR_MEMORY;
        }
        acc = next_acc;
        memcpy(acc + old_len, cursor, prefix_len);
        memcpy(acc + old_len + prefix_len, resolved, res_len);
        acc[old_len + prefix_len + res_len] = '\0';
        C_CDD_FREE(resolved);
        cursor = close_b + 1;
      }
    }
    *out_str = acc;
    return CDD_C_SUCCESS;
  }

  rc = eval_single_expr_to_str(&parsed, ctx, out_str);
  arazzo_runtime_expr_free(&parsed);
  return rc;
}

/**
 * @brief Trims leading and trailing whitespace and single/double quotes in
 * place.
 *
 * @param[in,out] s Mutable string buffer.
 * @param[out] out_trimmed Pointer receiving trimmed start inside `s`.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t trim_quotes_and_ws(char *s, char **out_trimmed) {
  size_t len;
  while (isspace((unsigned char)*s))
    ++s;
  len = strlen(s);
  while (len > 0 && isspace((unsigned char)s[len - 1])) {
    s[--len] = '\0';
  }
  if (len >= 2 && (s[0] == 39 || s[0] == '"') && s[len - 1] == s[0]) {
    s[len - 1] = '\0';
    ++s;
  }
  *out_trimmed = s;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a string represents a valid numeric literal (including
 * optional leading `-` and decimal point).
 *
 * @param[in] s Input string.
 * @param[out] out_num Parsed double value.
 * @param[out] out_is_num Receives `1` if numeric, `0` otherwise.
 * @return `CDD_C_SUCCESS`.
 */
static cdd_c_error_t try_parse_number(const char *s, double *out_num,
                                      int *out_is_num) {
  char *endp = NULL;
  const char *p = s;
  if (*p == '-')
    ++p;
  if (!isdigit((unsigned char)*p)) {
    *out_is_num = 0;
    return CDD_C_SUCCESS;
  }
  *out_num = strtod(s, &endp);
  *out_is_num = (*endp == '\0') ? 1 : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates a single atomic `simple` comparison (`lhs op rhs`).
 *
 * Supports quote-aware operator scanning (so operators inside `'...'` or
 * `"..."` literals are not split) and signed floating-point comparisons.
 *
 * @param[in] atom Null-terminated comparison string.
 * @param[in] ctx Execution context.
 * @param[out] out_passed Receives `1` if comparison holds, `0` otherwise.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t eval_simple_atom(const char *atom,
                                      const struct Arazzo_ExecutionContext *ctx,
                                      int *out_passed) {
  static const char *const ops[] = {"==", "!=", ">=", "<=", ">", "<"};
  size_t op_idx;
  char *op_pos = NULL;
  const char *op_str = NULL;
  char *buf;
  char *lhs_raw = NULL;
  char *rhs_raw = NULL;
  char *lhs_val = NULL;
  char *rhs_val = NULL;
  double d1 = 0.0;
  double d2 = 0.0;
  int is_num1 = 0;
  int is_num2 = 0;
  int cmp;
  cdd_c_error_t rc;

  buf = C_CDD_STRDUP(atom);
  if (!buf)
    return CDD_C_ERROR_MEMORY;

  for (op_idx = 0; op_idx < 6 && !op_pos; ++op_idx) {
    const char *candidate = ops[op_idx];
    size_t clen = strlen(candidate);
    char quote = '\0';
    char *scan;
    for (scan = buf; *scan != '\0'; ++scan) {
      if (quote != '\0') {
        if (*scan == quote)
          quote = '\0';
      } else if (*scan == 39 || *scan == '"') {
        quote = *scan;
      } else if (strncmp(scan, candidate, clen) == 0) {
        op_pos = scan;
        op_str = candidate;
        break;
      }
    }
  }

  if (!op_pos) {
    char *trimmed = NULL;
    rc = trim_quotes_and_ws(buf, &trimmed);
    if (trimmed[0] == '$') {
      if (arazzo_eval_runtime_expr_to_str(trimmed, ctx, &lhs_val) ==
          CDD_C_SUCCESS) {
        *out_passed = (strcmp(lhs_val, "false") != 0 &&
                       strcmp(lhs_val, "null") != 0 && lhs_val[0] != '\0')
                          ? 1
                          : 0;
        C_CDD_FREE(lhs_val);
      } else {
        *out_passed = 0;
      }
    } else {
      *out_passed = (strcmp(trimmed, "true") == 0);
    }
    C_CDD_FREE(buf);
    return rc;
  }

  *op_pos = '\0';
  rc = trim_quotes_and_ws(buf, &lhs_raw);
  rc = trim_quotes_and_ws(op_pos + strlen(op_str), &rhs_raw);

  if (lhs_raw[0] == '$') {
    if (arazzo_eval_runtime_expr_to_str(lhs_raw, ctx, &lhs_val) !=
        CDD_C_SUCCESS) {
      C_CDD_FREE(buf);
      *out_passed = 0;
      return CDD_C_SUCCESS;
    }
  } else {
    lhs_val = C_CDD_STRDUP(lhs_raw);
    if (!lhs_val) {
      C_CDD_FREE(buf);
      return CDD_C_ERROR_MEMORY;
    }
  }

  if (rhs_raw[0] == '$') {
    if (arazzo_eval_runtime_expr_to_str(rhs_raw, ctx, &rhs_val) !=
        CDD_C_SUCCESS) {
      C_CDD_FREE(lhs_val);
      C_CDD_FREE(buf);
      *out_passed = 0;
      return CDD_C_SUCCESS;
    }
  } else {
    rhs_val = C_CDD_STRDUP(rhs_raw);
    if (!rhs_val) {
      C_CDD_FREE(lhs_val);
      C_CDD_FREE(buf);
      return CDD_C_ERROR_MEMORY;
    }
  }

  try_parse_number(lhs_val, &d1, &is_num1);
  try_parse_number(rhs_val, &d2, &is_num2);
  if (is_num1 && is_num2) {
    cmp = (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
  } else {
    cmp = strcmp(lhs_val, rhs_val);
  }

  if (strcmp(op_str, "==") == 0)
    *out_passed = (cmp == 0);
  else if (strcmp(op_str, "!=") == 0)
    *out_passed = (cmp != 0);
  else if (strcmp(op_str, ">=") == 0)
    *out_passed = (cmp >= 0);
  else if (strcmp(op_str, "<=") == 0)
    *out_passed = (cmp <= 0);
  else if (strcmp(op_str, ">") == 0)
    *out_passed = (cmp > 0);
  else
    *out_passed = (cmp < 0);

  C_CDD_FREE(lhs_val);
  C_CDD_FREE(rhs_val);
  C_CDD_FREE(buf);
  return rc;
}

/**
 * @brief Finds a top-level binary boolean operator (`||` or `&&`) outside
 * parentheses and quotes.
 *
 * @param[in] expr Expression string.
 * @param[in] op Two-character operator (`"||"` or `"&&"`).
 * @param[out] out_pos Pointer to operator match or `NULL`.
 * @return `CDD_C_SUCCESS`.
 */
static cdd_c_error_t find_top_level_bool_op(const char *expr, const char *op,
                                            const char **out_pos) {
  int depth = 0;
  char quote = '\0';
  const char *p;
  *out_pos = NULL;
  for (p = expr; *p != '\0'; ++p) {
    if (quote != '\0') {
      if (*p == quote)
        quote = '\0';
    } else if (*p == 39 || *p == '"') {
      quote = *p;
    } else if (*p == '(') {
      ++depth;
    } else if (*p == ')') {
      --depth;
    } else if (depth == 0 && strncmp(p, op, 2) == 0) {
      *out_pos = p;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Recursively evaluates a `simple` boolean expression supporting `||`,
 * `&&`, `!`, and `(...)`.
 *
 * @param[in] expr Boolean expression string.
 * @param[in] ctx Execution context.
 * @param[out] out_passed Receives `1` if expression evaluates to true, `0`
 * otherwise.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t
eval_simple_bool_expr(const char *expr,
                      const struct Arazzo_ExecutionContext *ctx,
                      int *out_passed) {
  const char *op_pos = NULL;
  const char *s = expr;
  size_t len;
  cdd_c_error_t rc;

  while (isspace((unsigned char)*s))
    ++s;
  len = strlen(s);
  while (len > 0 && isspace((unsigned char)s[len - 1]))
    --len;

  find_top_level_bool_op(s, "||", &op_pos);
  if (op_pos) {
    size_t left_len = (size_t)(op_pos - s);
    char *left_str = (char *)C_CDD_MALLOC(left_len + 1);
    int p1 = 0;
    if (!left_str)
      return CDD_C_ERROR_MEMORY;
    memcpy(left_str, s, left_len);
    left_str[left_len] = '\0';
    rc = eval_simple_bool_expr(left_str, ctx, &p1);
    C_CDD_FREE(left_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (p1) {
      *out_passed = 1;
      return CDD_C_SUCCESS;
    }
    return eval_simple_bool_expr(op_pos + 2, ctx, out_passed);
  }

  find_top_level_bool_op(s, "&&", &op_pos);
  if (op_pos) {
    size_t left_len = (size_t)(op_pos - s);
    char *left_str = (char *)C_CDD_MALLOC(left_len + 1);
    int p1 = 0;
    if (!left_str)
      return CDD_C_ERROR_MEMORY;
    memcpy(left_str, s, left_len);
    left_str[left_len] = '\0';
    rc = eval_simple_bool_expr(left_str, ctx, &p1);
    C_CDD_FREE(left_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!p1) {
      *out_passed = 0;
      return CDD_C_SUCCESS;
    }
    return eval_simple_bool_expr(op_pos + 2, ctx, out_passed);
  }

  if (s[0] == '!') {
    int inner_pass = 0;
    rc = eval_simple_bool_expr(s + 1, ctx, &inner_pass);
    *out_passed = !inner_pass;
    return rc;
  }

  if (s[0] == '(') {
    char *inner = (char *)C_CDD_MALLOC(len - 1);
    if (!inner)
      return CDD_C_ERROR_MEMORY;
    memcpy(inner, s + 1, len - 2);
    inner[len - 2] = '\0';
    rc = eval_simple_bool_expr(inner, ctx, out_passed);
    C_CDD_FREE(inner);
    return rc;
  }

  return eval_simple_atom(s, ctx, out_passed);
}

/**
 * @brief Checks if a single character `c` matches the regex atom at `pat` and
 * advances `*out_next_pat` past the atom.
 *
 * @param[in] c Subject character (`\0` if at end of string).
 * @param[in] pat Pattern pointer at atom start.
 * @param[out] out_next_pat Receives pointer past the atom in `pat`.
 * @param[out] out_matched Receives `1` if `c` matched the atom, `0` otherwise.
 * @return `CDD_C_SUCCESS`.
 */
static cdd_c_error_t regex_match_atom(char c, const char *pat,
                                      const char **out_next_pat,
                                      int *out_matched) {
  unsigned char uc = (unsigned char)c;
  if (pat[0] == 92) {
    char esc = pat[1];
    *out_next_pat = pat + 2;
    if (esc == 'd') {
      *out_matched = (isdigit(uc) != 0);
    } else if (esc == 'w') {
      *out_matched = (isalnum(uc) != 0) | (c == '_');
    } else if (esc == 's') {
      *out_matched = (isspace(uc) != 0);
    } else {
      *out_matched = (c == esc);
    }
    return CDD_C_SUCCESS;
  }

  if (pat[0] == '[') {
    const char *p = pat + 1;
    int negate = 0;
    int hit = 0;
    if (*p == '^') {
      negate = 1;
      ++p;
    }
    while (*p != ']') {
      if (p[1] == '-') {
        if (uc >= (unsigned char)p[0] && uc <= (unsigned char)p[2])
          hit = 1;
        p += 3;
      } else {
        if (c == *p)
          hit = 1;
        ++p;
      }
    }
    *out_next_pat = p + 1;
    *out_matched = (c != '\0') & (negate ? !hit : hit);
    return CDD_C_SUCCESS;
  }

  *out_next_pat = pat + 1;
  *out_matched = (c == pat[0]);
  return CDD_C_SUCCESS;
}

/**
 * @brief Matches an anchored regex pattern from the current `text` position.
 *
 * @param[in] text Subject position.
 * @param[in] pat Pattern position.
 * @param[out] out_matched Receives `1` if matched, `0` otherwise.
 * @return `CDD_C_SUCCESS`.
 */
static cdd_c_error_t regex_match_here(const char *text, const char *pat,
                                      int *out_matched) {
  const char *next_pat = NULL;
  const char *after_q = NULL;
  size_t min_rep = 1;
  size_t max_rep = 1;
  int atom_ok = 0;

  if (pat[0] == '\0') {
    *out_matched = 1;
    return CDD_C_SUCCESS;
  }
  if (pat[0] == '$') {
    *out_matched = (*text == '\0');
    return CDD_C_SUCCESS;
  }

  regex_match_atom(*text, pat, &next_pat, &atom_ok);

  if (*next_pat == '*' || *next_pat == '+' || *next_pat == '?') {
    char q = *next_pat;
    min_rep = (q == '+') ? 1 : 0;
    max_rep = (q == '?') ? 1 : strlen(text);
    after_q = next_pat + 1;
  } else if (*next_pat == '{') {
    char *endp = NULL;
    min_rep = (size_t)strtoul(next_pat + 1, &endp, 10);
    max_rep = (*endp == ',') ? (size_t)strtoul(endp + 1, &endp, 10) : min_rep;
    after_q = endp + 1;
  }

  if (after_q) {
    size_t count = 0;
    const char *t = text;

    while (count < max_rep) {
      const char *dummy = NULL;
      int m = 0;
      regex_match_atom(*t, pat, &dummy, &m);
      if (!m)
        break;
      ++t;
      ++count;
    }

    while (count >= min_rep) {
      int tail_ok = 0;
      regex_match_here(text + count, after_q, &tail_ok);
      if (tail_ok) {
        *out_matched = 1;
        return CDD_C_SUCCESS;
      }
      if (count == 0)
        break;
      --count;
    }
    *out_matched = 0;
    return CDD_C_SUCCESS;
  }

  if (atom_ok)
    return regex_match_here(text + 1, next_pat, out_matched);

  *out_matched = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates a portable regular expression against `text`, supporting
 * `^`, `$`, `.`, `[...]`, `\d`, `\w`, `\s`, `*`, `+`, `?`, `{m,n}`, and `|`.
 *
 * @param[in] text Subject string.
 * @param[in] pattern Pattern string.
 * @param[out] out_matched Receives 1 if matched, 0 otherwise.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t match_simple_regex(const char *text, const char *pattern,
                                        int *out_matched) {
  const char *pipe = strchr(pattern, '|');
  const char *t;

  if (pipe) {
    size_t left_len = (size_t)(pipe - pattern);
    char *left_pat = (char *)C_CDD_MALLOC(left_len + 1);
    int m = 0;
    if (!left_pat)
      return CDD_C_ERROR_MEMORY;
    memcpy(left_pat, pattern, left_len);
    left_pat[left_len] = '\0';
    match_simple_regex(text, left_pat, &m);
    C_CDD_FREE(left_pat);
    if (m) {
      *out_matched = 1;
      return CDD_C_SUCCESS;
    }
    return match_simple_regex(text, pipe + 1, out_matched);
  }

  if (pattern[0] == '^')
    return regex_match_here(text, pattern + 1, out_matched);

  t = text;
  for (;;) {
    int m = 0;
    regex_match_here(t, pattern, &m);
    if (m) {
      *out_matched = 1;
      return CDD_C_SUCCESS;
    }
    if (*t == '\0')
      break;
    ++t;
  }
  *out_matched = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates a single `Arazzo_Criterion` against an execution context.
 *
 * @return `CDD_C_SUCCESS` on success, or error code if malformed.
 */
cdd_c_error_t arazzo_eval_criterion(const struct Arazzo_Criterion *criterion,
                                    const struct Arazzo_ExecutionContext *ctx,
                                    int *out_passed) {
  enum Arazzo_CriterionType eff_type;
  cdd_c_error_t rc;

  if (!criterion || !criterion->condition || !ctx || !out_passed)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  eff_type = criterion->type;
  if (eff_type == ARAZZO_CRITERION_EXPRESSION_TYPE) {
    if (criterion->expression_type.type == ARAZZO_DIALECT_JSONPATH)
      eff_type = ARAZZO_CRITERION_JSONPATH;
    else if (criterion->expression_type.type == ARAZZO_DIALECT_XPATH)
      eff_type = ARAZZO_CRITERION_XPATH;
    else
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (eff_type == ARAZZO_CRITERION_SIMPLE) {
    return eval_simple_bool_expr(criterion->condition, ctx, out_passed);
  }

  if (eff_type == ARAZZO_CRITERION_REGEX) {
    char *ctx_str = NULL;
    if (!criterion->context)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (arazzo_eval_runtime_expr_to_str(criterion->context, ctx, &ctx_str) !=
        CDD_C_SUCCESS) {
      *out_passed = 0;
      return CDD_C_SUCCESS;
    }
    rc = match_simple_regex(ctx_str, criterion->condition, out_passed);
    C_CDD_FREE(ctx_str);
    return rc;
  }

  if (eff_type == ARAZZO_CRITERION_JSONPATH) {
    const JSON_Value *root = ctx->resp_body ? ctx->resp_body : ctx->req_body;
    const JSON_Value *matched = NULL;
    if (!root) {
      *out_passed = 0;
      return CDD_C_SUCCESS;
    }
    rc = arazzo_eval_jsonpath(root, criterion->condition, &matched);
    *out_passed = (rc == CDD_C_SUCCESS) ? 1 : 0;
    return CDD_C_SUCCESS;
  }

  if (eff_type == ARAZZO_CRITERION_XPATH) {
    char *ctx_xml = NULL;
    char *extracted = NULL;
    if (!criterion->context)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (arazzo_eval_runtime_expr_to_str(criterion->context, ctx, &ctx_xml) !=
        CDD_C_SUCCESS) {
      *out_passed = 0;
      return CDD_C_SUCCESS;
    }
    rc = arazzo_eval_xpath(ctx_xml, criterion->condition, &extracted);
    C_CDD_FREE(ctx_xml);
    C_CDD_FREE(extracted);
    *out_passed = (rc == CDD_C_SUCCESS) ? 1 : 0;
    return CDD_C_SUCCESS;
  }

  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Evaluates an `Arazzo_Selector` (`context` + `selector`) to extract a
 * value from the execution context.
 *
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_NOT_FOUND` if missing, or
 * error code.
 */
cdd_c_error_t arazzo_eval_selector(const struct Arazzo_Selector *selector,
                                   const struct Arazzo_ExecutionContext *ctx,
                                   char **out_str) {
  enum Arazzo_CriterionType eff_type;
  cdd_c_error_t rc;

  if (!selector || !selector->context || !selector->selector || !ctx ||
      !out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  eff_type = selector->type;
  if (eff_type == ARAZZO_CRITERION_EXPRESSION_TYPE) {
    if (selector->expression_type.type == ARAZZO_DIALECT_JSONPATH)
      eff_type = ARAZZO_CRITERION_JSONPATH;
    else if (selector->expression_type.type == ARAZZO_DIALECT_XPATH)
      eff_type = ARAZZO_CRITERION_XPATH;
    else
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (eff_type == ARAZZO_CRITERION_JSONPATH) {
    const JSON_Value *root = strcmp(selector->context, "$request.body") == 0
                                 ? ctx->req_body
                                 : ctx->resp_body;
    const JSON_Value *matched = NULL;
    if (!root)
      return CDD_C_ERROR_NOT_FOUND;
    rc = arazzo_eval_jsonpath(root, selector->selector, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return json_val_to_str(matched, out_str);
  }

  if (eff_type == ARAZZO_CRITERION_XPATH) {
    char *xml_str = NULL;
    rc = arazzo_eval_runtime_expr_to_str(selector->context, ctx, &xml_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = arazzo_eval_xpath(xml_str, selector->selector, out_str);
    C_CDD_FREE(xml_str);
    return rc;
  }

  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Applies an array of `Arazzo_PayloadReplacement` rules to a mutable
 * JSON request body object or array.
 *
 * Supports RFC 6901 unescaping (`~0`, `~1`) on the final property token as
 * well as replacing elements inside `JSONArray` targets via numeric indices.
 *
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t arazzo_apply_payload_replacements(
    JSON_Value *payload, const struct Arazzo_PayloadReplacement *replacements,
    size_t n_replacements, const struct Arazzo_ExecutionContext *ctx) {
  size_t i;

  if (!payload || !replacements || !ctx)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < n_replacements; ++i) {
    const struct Arazzo_PayloadReplacement *rep = &replacements[i];
    const char *target = rep->target;
    const char *last_sep;
    const JSON_Value *parent_const = NULL;
    char *final_tok = NULL;
    char *new_str_val = NULL;
    cdd_c_error_t rc;

    if (!target || target[0] != '/')
      return CDD_C_ERROR_INVALID_ARGUMENT;

    last_sep = strrchr(target, '/');
    if (last_sep == target) {
      parent_const = payload;
    } else {
      size_t parent_len = (size_t)(last_sep - target);
      char *parent_ptr = (char *)C_CDD_MALLOC(parent_len + 1);
      if (!parent_ptr)
        return CDD_C_ERROR_MEMORY;
      memcpy(parent_ptr, target, parent_len);
      parent_ptr[parent_len] = '\0';
      rc = arazzo_eval_json_pointer(payload, parent_ptr, &parent_const);
      C_CDD_FREE(parent_ptr);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }

    rc = unescape_pointer_token(last_sep + 1, strlen(last_sep + 1), &final_tok);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (json_value_get_type(parent_const) == JSONObject) {
      JSON_Object *parent_obj = json_value_get_object(parent_const);
      if (rep->value.type == OA_ANY_STRING) {
        rc = arazzo_eval_runtime_expr_to_str(
            rep->value.string ? rep->value.string : "", ctx, &new_str_val);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(final_tok);
          return rc;
        }
        json_object_set_string(parent_obj, final_tok, new_str_val);
        C_CDD_FREE(new_str_val);
      } else if (rep->value.type == OA_ANY_NUMBER) {
        json_object_set_number(parent_obj, final_tok, rep->value.number);
      } else if (rep->value.type == OA_ANY_BOOL) {
        json_object_set_boolean(parent_obj, final_tok, rep->value.boolean);
      } else {
        json_object_set_null(parent_obj, final_tok);
      }
    } else if (json_value_get_type(parent_const) == JSONArray) {
      JSON_Array *parent_arr = json_value_get_array(parent_const);
      char *endp = NULL;
      unsigned long idx = strtoul(final_tok, &endp, 10);
      if (*endp != '\0' || (size_t)idx >= json_array_get_count(parent_arr)) {
        C_CDD_FREE(final_tok);
        return CDD_C_ERROR_NOT_FOUND;
      }
      if (rep->value.type == OA_ANY_STRING) {
        rc = arazzo_eval_runtime_expr_to_str(
            rep->value.string ? rep->value.string : "", ctx, &new_str_val);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(final_tok);
          return rc;
        }
        json_array_replace_string(parent_arr, (size_t)idx, new_str_val);
        C_CDD_FREE(new_str_val);
      } else if (rep->value.type == OA_ANY_NUMBER) {
        json_array_replace_number(parent_arr, (size_t)idx, rep->value.number);
      } else if (rep->value.type == OA_ANY_BOOL) {
        json_array_replace_boolean(parent_arr, (size_t)idx, rep->value.boolean);
      } else {
        json_array_replace_null(parent_arr, (size_t)idx);
      }
    } else {
      C_CDD_FREE(final_tok);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }

    C_CDD_FREE(final_tok);
  }

  return CDD_C_SUCCESS;
}
