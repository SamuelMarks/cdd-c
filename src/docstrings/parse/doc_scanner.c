/* clang-format off */
#include "c_cdd/memory.h"
/**
 * @file doc_scanner.c
 * @brief Scanner and text parsing routines for documentation comments.
 *
 * @author Samuel Marks
 */

#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd_export.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "docstrings/parse/doc.h"
#include "docstrings/parse/doc_internal.h"
#include "functions/parse/str.h"
/* clang-format on */

cdd_c_error_t doc_skip_ws(const char *p, const char **out_pos) {
  if (!out_pos) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (!p) {
    *out_pos = NULL;
    return CDD_C_SUCCESS;
  }
  while (*p && isspace((unsigned char)*p) && !DOC_IS_EOL(*p)) {
    p++;
  }
  *out_pos = p;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_extract_word(const char *str, const char *end,
                               const char **next_out, char **out_val) {
  const char *p = NULL;
  const char *word_start;
  size_t len;
  char *res;
  cdd_c_error_t rc;

  if (!next_out || !out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = doc_skip_ws(str, &p);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  word_start = p;

  while (p < end && !isspace((unsigned char)*p)) {
    p++;
  }

  len = (size_t)(p - word_start);
  if (len == 0) {
    *next_out = p;
    *out_val = NULL;
    return CDD_C_SUCCESS;
  }

  res = (char *)(size_t)C_CDD_MALLOC(len + 1);
  if (!res) {
    *out_val = NULL;
    return CDD_C_SUCCESS;
  }

  memcpy(res, word_start, len);
  res[len] = '\0';

  *next_out = p;
  *out_val = res;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_extract_rest(const char *str, const char *end,
                               char **out_val) {
  const char *p = NULL;
  const char *e = end;
  size_t len;
  char *res;
  cdd_c_error_t rc;

  if (!out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = doc_skip_ws(str, &p);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  /* Trim trailing */
  while (e > p && isspace((unsigned char)*(e - 1))) {
    e--;
  }

  len = (size_t)(e - p);
  if (len == 0) {
    *out_val = NULL;
    return CDD_C_SUCCESS;
  }

  res = (char *)(size_t)C_CDD_MALLOC(len + 1);
  if (!res) {
    *out_val = NULL;
    return CDD_C_SUCCESS;
  }

  memcpy(res, p, len);
  res[len] = '\0';
  *out_val = res;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_trim_segment(char *s, char **out_val) {
  char *start;
  char *end;
  if (!out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (!s) {
    *out_val = NULL;
    return CDD_C_SUCCESS;
  }
  start = s;
  while (*start && isspace((unsigned char)*start))
    start++;
  end = start + strlen(start);
  while (end > start && isspace((unsigned char)*(end - 1)))
    end--;
  *end = '\0';
  *out_val = start;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_bool_text(const char *s, int *out) {
  int diff;
  cdd_c_error_t rc;

  if (!s || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (strcmp(s, "1") == 0) {
    *out = 1;
    return CDD_C_SUCCESS;
  }
  if (strcmp(s, "0") == 0) {
    *out = 0;
    return CDD_C_SUCCESS;
  }

  rc = c_cdd_stricmp(s, "true", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out = 1;
    return CDD_C_SUCCESS;
  }

  rc = c_cdd_stricmp(s, "yes", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out = 1;
    return CDD_C_SUCCESS;
  }

  rc = c_cdd_stricmp(s, "false", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out = 0;
    return CDD_C_SUCCESS;
  }

  rc = c_cdd_stricmp(s, "no", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out = 0;
    return CDD_C_SUCCESS;
  }

  return CDD_C_ERROR_UNKNOWN;
}

cdd_c_error_t doc_parse_style_text(const char *s, enum DocParamStyle *out) {
  int diff;
  cdd_c_error_t rc;

  if (!s || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = c_cdd_stricmp(s, "form", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out = DOC_PARAM_STYLE_FORM;
    return CDD_C_SUCCESS;
  }

  rc = c_cdd_stricmp(s, "simple", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out = DOC_PARAM_STYLE_SIMPLE;
    return CDD_C_SUCCESS;
  }

  rc = c_cdd_stricmp(s, "matrix", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out = DOC_PARAM_STYLE_MATRIX;
    return CDD_C_SUCCESS;
  }

  rc = c_cdd_stricmp(s, "label", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out = DOC_PARAM_STYLE_LABEL;
    return CDD_C_SUCCESS;
  }

  rc = c_cdd_stricmp(s, "spaceDelimited", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out = DOC_PARAM_STYLE_SPACE_DELIMITED;
    return CDD_C_SUCCESS;
  }

  rc = c_cdd_stricmp(s, "pipeDelimited", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out = DOC_PARAM_STYLE_PIPE_DELIMITED;
    return CDD_C_SUCCESS;
  }

  rc = c_cdd_stricmp(s, "deepObject", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out = DOC_PARAM_STYLE_DEEP_OBJECT;
    return CDD_C_SUCCESS;
  }

  rc = c_cdd_stricmp(s, "cookie", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out = DOC_PARAM_STYLE_COOKIE;
    return CDD_C_SUCCESS;
  }

  return CDD_C_ERROR_UNKNOWN;
}

cdd_c_error_t doc_parse_optional_bool_attr(const char *attr, const char *key,
                                           int *out_set, int *out_val) {
  size_t key_len;
  cdd_c_error_t parsed;

#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_doc_fail_parse_optional_bool_attr;
  if (g_doc_fail_parse_optional_bool_attr > 0) {
    g_doc_fail_parse_optional_bool_attr--;
    if (g_doc_fail_parse_optional_bool_attr == 0)
      return CDD_C_ERROR_MEMORY;
  }
#endif

  if (!attr || !key || !out_set || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  key_len = strlen(key);
  if (strcmp(attr, key) == 0) {
    *out_set = 1;
    *out_val = 1;
    return CDD_C_SUCCESS;
  }
  if (strncmp(attr, key, key_len) == 0 &&
      (attr[key_len] == ':' || attr[key_len] == '=')) {
    int value = 0;
    char *val_trimmed = NULL;
    cdd_c_error_t rc_trim =
        doc_trim_segment((char *)(size_t)(attr + key_len + 1), &val_trimmed);
    if (rc_trim != CDD_C_SUCCESS)
      return rc_trim;
    parsed = doc_parse_bool_text(val_trimmed, &value);
    if (parsed == CDD_C_SUCCESS) {
      *out_set = 1;
      *out_val = value;
    }
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_optional_example_attr(const char *attr,
                                              char **out_example) {
  char *val = NULL;
  char *dup_val = NULL;
  cdd_c_error_t rc;

  if (!attr || !out_example)
    return CDD_C_SUCCESS;
  if (strncmp(attr, "example:", 8) != 0 && strncmp(attr, "example=", 8) != 0)
    return CDD_C_SUCCESS;

  rc = doc_trim_segment((char *)(size_t)(attr + 8), &val);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (!*val)
    return CDD_C_ERROR_UNKNOWN;
  if (*out_example)
    C_CDD_FREE(*out_example);

  rc = c_cdd_strdup(val, &dup_val);
  if (rc != CDD_C_SUCCESS || !dup_val)
    return CDD_C_ERROR_MEMORY;

  *out_example = dup_val;
  return 1;
}

cdd_c_error_t doc_find_key_token(char *s, const char *key, size_t *key_len,
                                 char **out_val) {
  char *p;
  size_t klen;
  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!s || !key) {
    *out_val = NULL;
    return CDD_C_SUCCESS;
  }
  klen = strlen(key);
  p = strstr(s, key);
  while (p) {
    if ((p == s || isspace((unsigned char)p[-1])) &&
        (p[klen] == '=' || p[klen] == ':')) {
      if (key_len)
        *key_len = klen + 1;
      *out_val = p;
      return CDD_C_SUCCESS;
    }
    p = strstr(p + klen, key);
  }
  *out_val = NULL;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_split_scopes(const char *input, char ***out_scopes,
                               size_t *out_count) {
  char *buf = NULL;
  char *token;
  char *saveptr = NULL;
  char **scopes = NULL;
  size_t n = 0;
  cdd_c_error_t rc;

  if (!out_scopes || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_scopes = NULL;
  *out_count = 0;
  if (!input || !*input)
    return CDD_C_SUCCESS;

  rc = c_cdd_strdup(input, &buf);
  if (rc != CDD_C_SUCCESS || !buf) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

#ifdef _WIN32
  token = strtok_s(buf, ",", &saveptr);
#else
  token = strtok_r(buf, ",", &saveptr);
#endif
  while (token) {
    char *trimmed = NULL;
    char **new_scopes;
    rc = doc_trim_segment(token, &trimmed);
    if (rc != CDD_C_SUCCESS) {
      size_t i;
      for (i = 0; i < n; ++i)
        C_CDD_FREE(scopes[i]);
      C_CDD_FREE(scopes);
      C_CDD_FREE(buf);
      return rc;
    }
    if (!*trimmed) {
#ifdef _WIN32
      token = strtok_s(NULL, ",", &saveptr);
#else
      token = strtok_r(NULL, ",", &saveptr);
#endif
      continue;
    }
    new_scopes = (char **)C_CDD_REALLOC(scopes, (n + 1) * sizeof(char *));
    if (!new_scopes) {
      size_t i;
      for (i = 0; i < n; ++i)
        C_CDD_FREE(scopes[i]);
      C_CDD_FREE(scopes);
      C_CDD_FREE(buf);
      return CDD_C_ERROR_MEMORY;
    }
    scopes = new_scopes;
    rc = c_cdd_strdup(trimmed, &scopes[n]);
    if (rc != CDD_C_SUCCESS || !scopes[n]) {
      size_t i;
      for (i = 0; i < n; ++i)
        C_CDD_FREE(scopes[i]);
      C_CDD_FREE(scopes);
      C_CDD_FREE(buf);
      return CDD_C_ERROR_MEMORY;
    }
    n++;
#ifdef _WIN32
    token = strtok_s(NULL, ",", &saveptr);
#else
    token = strtok_r(NULL, ",", &saveptr);
#endif
  }

  C_CDD_FREE(buf);
  *out_scopes = scopes;
  *out_count = n;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_split_enum_values(const char *input, char ***out_vals,
                                    size_t *out_count) {
  char *buf = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!out_vals || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_vals = NULL;
  *out_count = 0;
  if (!input || !*input)
    return CDD_C_SUCCESS;

  rc = c_cdd_strdup(input, &buf);
  if (rc != CDD_C_SUCCESS || !buf) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  for (i = 0; buf[i]; ++i) {
    if (buf[i] == '|')
      buf[i] = ',';
  }
  rc = doc_split_scopes(buf, out_vals, out_count);
  C_CDD_FREE(buf);
  return rc;
}
