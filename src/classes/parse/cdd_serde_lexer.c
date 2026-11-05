/**
 * @file cdd_serde_lexer.c
 * @brief Low-level directive block scanner, token/tuple parsers, and Doxygen
 * metadata fallback extractors for C89 reflection comments.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "classes/parse/cdd_serde_internal.h"
/* clang-format on */

/*
 * @brief Safely copies a string into a fixed-size destination buffer with
 * truncation.
 */
cdd_c_error_t cdd_serde_copy_str(char *dst, size_t dst_sz, const char *src) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  strncpy_s(dst, dst_sz, src, _TRUNCATE);
#else
  strncpy(dst, src, dst_sz - 1);
  dst[dst_sz - 1] = '\0';
#endif
  return CDD_C_SUCCESS;
}

/*
 * @brief Case-insensitive ASCII string comparison helper.
 */
cdd_c_error_t cdd_serde_str_ieq(const char *a, const char *b, int *out_eq) {
  while (*a && *b) {
    if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
      *out_eq = 0;
      return CDD_C_SUCCESS;
    }
    a++;
    b++;
  }
  *out_eq = (*a == '\0' && *b == '\0') ? 1 : 0;
  return CDD_C_SUCCESS;
}

/*
 * @brief Advances pointer past whitespace, commas, and comment asterisks.
 */
cdd_c_error_t cdd_serde_skip_delims(const char **p, const char *end) {
  const char *cur = *p;
  while (cur < end &&
         (isspace((unsigned char)*cur) || *cur == ',' || *cur == '*')) {
    cur++;
  }
  *p = cur;
  return CDD_C_SUCCESS;
}

/*
 * @brief Extracts a quoted string or bare identifier value starting at `*p`.
 */
cdd_c_error_t cdd_serde_parse_value(const char **p, const char *end,
                                    char *out_val, size_t val_sz) {
  const char *cur = *p;
  size_t pos = 0;
  int paren_wrapped = 0;

  while (cur < end && isspace((unsigned char)*cur))
    cur++;

  if (cur < end && *cur == '(') {
    paren_wrapped = 1;
    cur++;
    while (isspace((unsigned char)*cur))
      cur++;
  }

  if (cur < end && (*cur == '"' || *cur == '\'')) {
    char quote = *cur++;
    while (cur < end && *cur != quote) {
      char ch = *cur++;
      if (ch == '\\') {
        char esc = *cur++;
        if (esc == 'n')
          ch = '\n';
        else if (esc == 'r')
          ch = '\r';
        else if (esc == 't')
          ch = '\t';
        else
          ch = esc;
      }
      if (pos + 1 < val_sz)
        out_val[pos++] = ch;
    }
    if (cur < end)
      cur++;
  } else {
    while (cur < end && !isspace((unsigned char)*cur) && *cur != ',' &&
           *cur != ')') {
      if (pos + 1 < val_sz)
        out_val[pos++] = *cur;
      cur++;
    }
  }

  if (paren_wrapped) {
    while (cur < end && (isspace((unsigned char)*cur) || *cur == ')'))
      cur++;
  }

  out_val[pos] = '\0';
  *p = cur;
  return CDD_C_SUCCESS;
}

/*
 * @brief Parses a replacement tuple `("<from>" -> "<to>")` or
 * `("<from>", "<to>")` starting at `*p`.
 */
cdd_c_error_t cdd_serde_parse_replace_tuple(const char **p, const char *end,
                                            char *out_from, size_t from_sz,
                                            char *out_to, size_t to_sz) {
  const char *cur = *p;
  cdd_c_error_t rc;

  while (isspace((unsigned char)*cur) || *cur == '(')
    cur++;

  rc = cdd_serde_parse_value(&cur, end, out_from, from_sz);

  while (cur < end && (isspace((unsigned char)*cur) || *cur == ',' ||
                       *cur == '-' || *cur == '>' || *cur == ':')) {
    cur++;
  }

  rc = cdd_serde_parse_value(&cur, end, out_to, to_sz);

  while (cur < end && (isspace((unsigned char)*cur) || *cur == ')'))
    cur++;

  *p = cur;
  return rc;
}

/*
 * @brief Locates the next `@cdd`, `@serde`, `@cli`, `@to_json`, `@from_json`,
 * `@to_cli`, `@from_cli`, `#[...(`, or `[[...]]` directive block in a comment
 * string.
 */
cdd_c_error_t cdd_serde_find_directive_ex(const char *cur,
                                          const char **out_start,
                                          const char **out_end,
                                          const char **out_next,
                                          enum CddDirectiveScope *out_scope) {
  const char *p = cur;
  if (out_scope)
    *out_scope = CDD_SCOPE_GENERAL;
  while (*p != '\0') {
    size_t tag_len = 0;
    enum CddDirectiveScope tag_scope = CDD_SCOPE_GENERAL;
    if (strncmp(p, "@from_json", 10) == 0 ||
        strncmp(p, "@from_form", 10) == 0) {
      tag_len = 10;
      tag_scope = CDD_SCOPE_FROM_JSON;
    } else if (strncmp(p, "@to_json", 8) == 0 ||
               strncmp(p, "@to_form", 8) == 0) {
      tag_len = 8;
      tag_scope = CDD_SCOPE_TO_JSON;
    } else if (strncmp(p, "@from_cli", 9) == 0) {
      tag_len = 9;
      tag_scope = CDD_SCOPE_FROM_CLI;
    } else if (strncmp(p, "@to_cli", 7) == 0) {
      tag_len = 7;
      tag_scope = CDD_SCOPE_TO_CLI;
    } else if (strncmp(p, "@serde", 6) == 0) {
      tag_len = 6;
      tag_scope = CDD_SCOPE_GENERAL;
    } else if (strncmp(p, "@cli", 4) == 0) {
      tag_len = 4;
      tag_scope = CDD_SCOPE_CLI;
    } else if (strncmp(p, "@cdd", 4) == 0) {
      tag_len = 4;
      tag_scope = CDD_SCOPE_GENERAL;
    }
    if (tag_len > 0 && !isalnum((unsigned char)p[tag_len]) &&
        p[tag_len] != '_') {
      const char *body = p + tag_len;
      const char *end;
      int paren_depth = 0;
      char in_quote = 0;
      if (out_scope)
        *out_scope = tag_scope;
      if (*body == '(') {
        paren_depth = 1;
        body++;
      }
      end = body;
      while (*end != '\0') {
        if (in_quote) {
          if (*end == '\\') {
            end += 2;
            continue;
          }
          if (*end == in_quote) {
            in_quote = 0;
          }
          end++;
          continue;
        }
        if (end[0] == '*' && end[1] == '/')
          break;
        if (*end == '"' || *end == '\'') {
          in_quote = *end++;
          continue;
        }
        if (*end == '(') {
          paren_depth++;
          end++;
          continue;
        }
        if (*end == ')') {
          if (paren_depth > 0) {
            paren_depth--;
            if (paren_depth == 0 && *(p + tag_len) == '(') {
              break;
            }
          }
          end++;
          continue;
        }
        if (paren_depth == 0 && (*end == '\n' || *end == '@'))
          break;
        end++;
      }
      *out_start = body;
      *out_end = end;
      *out_next = (*end == ')') ? end + 1 : end;
      return CDD_C_SUCCESS;
    }
    if (strncmp(p, "#[", 2) == 0 || strncmp(p, "[[", 2) == 0) {
      const char *attr = p + 2;
      size_t ns_len = (strncmp(attr, "cdd::", 5) == 0) ? 5 : 0;
      const char *sub = attr + ns_len;
      size_t sub_len = 0;
      enum CddDirectiveScope attr_scope = CDD_SCOPE_GENERAL;
      if (strncmp(sub, "serde(", 6) == 0) {
        sub_len = 6;
      } else if (strncmp(sub, "cdd(", 4) == 0) {
        sub_len = 4;
      } else if (strncmp(sub, "cli(", 4) == 0) {
        sub_len = 4;
        attr_scope = CDD_SCOPE_CLI;
      } else if (strncmp(sub, "to_json(", 8) == 0) {
        sub_len = 8;
        attr_scope = CDD_SCOPE_TO_JSON;
      } else if (strncmp(sub, "from_json(", 10) == 0) {
        sub_len = 10;
        attr_scope = CDD_SCOPE_FROM_JSON;
      } else if (strncmp(sub, "to_cli(", 7) == 0) {
        sub_len = 7;
        attr_scope = CDD_SCOPE_TO_CLI;
      } else if (strncmp(sub, "from_cli(", 9) == 0) {
        sub_len = 9;
        attr_scope = CDD_SCOPE_FROM_CLI;
      } else if (strncmp(sub, "command(", 8) == 0) {
        sub_len = 8;
        attr_scope = CDD_SCOPE_CLI;
      } else if (strncmp(sub, "clap(", 5) == 0) {
        sub_len = 5;
        attr_scope = CDD_SCOPE_CLI;
      } else if (strncmp(sub, "structopt(", 10) == 0) {
        sub_len = 10;
        attr_scope = CDD_SCOPE_CLI;
      } else if (strncmp(sub, "arg(", 4) == 0) {
        sub_len = 4;
        attr_scope = CDD_SCOPE_CLI;
      }
      if (sub_len == 0 && (ns_len > 0 || strncmp(attr, "serde::", 7) == 0 ||
                           strncmp(attr, "cli::", 5) == 0)) {
        sub = attr;
        sub_len = 1;
        attr_scope = (strncmp(attr, "cli::", 5) == 0) ? CDD_SCOPE_CLI
                                                      : CDD_SCOPE_GENERAL;
      }
      if (sub_len > 0) {
        const char *body =
            (sub == attr && sub_len == 1) ? attr : (sub + sub_len);
        const char *end = body;
        int depth = 1;
        char in_quote = 0;
        if (out_scope)
          *out_scope = attr_scope;
        while (*end != '\0' && !(end[0] == '*' && end[1] == '/')) {
          if (in_quote) {
            if (*end == '\\') {
              end += 2;
              continue;
            }
            if (*end == in_quote)
              in_quote = 0;
            end++;
            continue;
          }
          if (*end == '"' || *end == '\'') {
            in_quote = *end++;
            continue;
          }
          if (end[0] == ']' && end[1] == ']')
            break;
          if (*end == '(')
            depth++;
          else if (*end == ')') {
            depth--;
            if (depth == 0)
              break;
          }
          end++;
        }
        *out_start = body;
        *out_end = end;
        *out_next = (*end == ')') ? (end + 1) : end;
        return CDD_C_SUCCESS;
      }
    }
    p++;
  }
  *out_start = NULL;
  *out_end = NULL;
  *out_next = p;
  return CDD_C_SUCCESS;
}

/*
 * @brief Extracts the next directive key identifier within `[ *p, end )`.
 */
cdd_c_error_t cdd_serde_next_key(const char **p, const char *end, char *key_buf,
                                 size_t key_sz) {
  const char *cur;
  size_t pos = 0;
  cdd_c_error_t rc;

  rc = cdd_serde_skip_delims(p, end);
  cur = *p;
  while (cur < end && *cur == ')') {
    cur++;
    *p = cur;
    rc = cdd_serde_skip_delims(p, end);
    cur = *p;
  }
  if (strncmp(cur, "cdd::", 5) == 0 || strncmp(cur, "cli::", 5) == 0) {
    cur += 5;
  } else if (strncmp(cur, "serde::", 7) == 0) {
    cur += 7;
  }

  while (cur < end &&
         (isalnum((unsigned char)*cur) || *cur == '_' || *cur == '-')) {
    if (pos + 1 < key_sz)
      key_buf[pos++] = *cur;
    cur++;
  }
  key_buf[pos] = '\0';
  *p = cur;
  return rc;
}

/*
 * @brief Parses a boolean string (`"true"` / `"false"` / `"1"` / `"0"`) into
 * `0` or `1`.
 */
cdd_c_error_t cdd_serde_parse_bool_val(const char *val, int *out_bool) {
  int eq_false = 0, eq_no = 0, eq_off = 0;
  cdd_c_error_t rc;

  rc = cdd_serde_str_ieq(val, "false", &eq_false);
  rc = cdd_serde_str_ieq(val, "no", &eq_no);
  rc = cdd_serde_str_ieq(val, "off", &eq_off);
  if (eq_false || eq_no || eq_off || strcmp(val, "0") == 0) {
    *out_bool = 0;
  } else {
    *out_bool = 1;
  }
  return rc;
}

/*
 * @brief Parses a `clap` `num_args` specification (e.g. `"2"`, `"1..=3"`,
 * `"1..4"`, `"1.."`, `"..=3"`, `".."`).
 */
cdd_c_error_t cdd_serde_parse_num_args(const char *val, int *out_min,
                                       int *out_max) {
  const char *dots = strstr(val, "..");
  if (!dots) {
    int n = atoi(val);
    *out_min = n;
    *out_max = n;
    return CDD_C_SUCCESS;
  }
  if (dots == val) {
    *out_min = 0;
  } else {
    *out_min = atoi(val);
  }
  dots += 2;
  if (*dots == '=') {
    dots++;
    *out_max = (*dots != '\0') ? atoi(dots) : -1;
  } else if (*dots != '\0') {
    int upper = atoi(dots);
    *out_max = (upper > 0) ? (upper - 1) : 0;
  } else {
    *out_max = -1;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts a single line of text following a Doxygen tag (such as
 * `@brief `, `@details `, `@version `, `@author `).
 *
 * @param[in] start Pointer to first character after the tag name.
 * @param[out] out_buf Destination buffer.
 * @param[in] out_sz Capacity of `out_buf`.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_extract_tag_line(const char *start, char *out_buf,
                                            size_t out_sz) {
  const char *p = start;
  size_t pos = 0;
  if (*p == ':')
    p++;
  while (*p == ' ' || *p == '\t')
    p++;
  while (*p != '\0' && *p != '\r' && *p != '\n' && strncmp(p, "*/", 2) != 0 &&
         pos + 1 < out_sz) {
    out_buf[pos++] = *p++;
  }
  while (pos > 0 && isspace((unsigned char)out_buf[pos - 1]))
    pos--;
  out_buf[pos] = '\0';
  return CDD_C_SUCCESS;
}

/*
 * @brief Scans `comment_text` for Doxygen metadata (`@brief`, `@summary`,
 * `@details`, `@description`, `@version`, `@infoVersion`, `@author`,
 * `@contact`) and plain Doxygen docstrings.
 */
cdd_c_error_t cdd_serde_extract_doxygen_fallbacks(
    const char *comment_text, char *out_brief, size_t brief_sz,
    char *out_details, size_t details_sz, char *out_version, size_t version_sz,
    char *out_author, size_t author_sz) {
  const char *p = comment_text;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  out_brief[0] = '\0';
  out_details[0] = '\0';
  if (out_version)
    out_version[0] = '\0';
  if (out_author)
    out_author[0] = '\0';

  while (*p != '\0') {
    if (*p == '@') {
      if (out_brief[0] == '\0' && strncmp(p, "@brief", 6) == 0 &&
          !isalnum((unsigned char)p[6])) {
        rc = serde_extract_tag_line(p + 6, out_brief, brief_sz);
      } else if (out_brief[0] == '\0' && strncmp(p, "@summary", 8) == 0) {
        rc = serde_extract_tag_line(p + 8, out_brief, brief_sz);
      } else if (out_details[0] == '\0' && strncmp(p, "@details", 8) == 0) {
        rc = serde_extract_tag_line(p + 8, out_details, details_sz);
      } else if (out_details[0] == '\0' &&
                 strncmp(p, "@description", 12) == 0) {
        rc = serde_extract_tag_line(p + 12, out_details, details_sz);
      } else if (out_version && out_version[0] == '\0' &&
                 strncmp(p, "@version", 8) == 0) {
        rc = serde_extract_tag_line(p + 8, out_version, version_sz);
      } else if (out_version && out_version[0] == '\0' &&
                 strncmp(p, "@infoVersion", 12) == 0) {
        rc = serde_extract_tag_line(p + 12, out_version, version_sz);
      } else if (out_author && out_author[0] == '\0' &&
                 strncmp(p, "@author", 7) == 0) {
        rc = serde_extract_tag_line(p + 7, out_author, author_sz);
      } else if (out_author && out_author[0] == '\0' &&
                 strncmp(p, "@contact", 8) == 0) {
        rc = serde_extract_tag_line(p + 8, out_author, author_sz);
      }
    }
    p++;
  }

  /* If no explicit @brief was found, check for a plain Doxygen block or
   * /// line that does not start with @ or #[ */
  if (out_brief[0] == '\0') {
    const char *doc = strstr(comment_text, "/**");
    if (!doc)
      doc = strstr(comment_text, "/*!");
    if (doc) {
      const char *s = doc + 3;
      if (*s == '<')
        s++;
      while (*s != '\0' && strncmp(s, "*/", 2) != 0) {
        while (isspace((unsigned char)*s) ||
               (*s == '*' && strncmp(s, "*/", 2) != 0)) {
          s++;
        }
        if (*s == '\0' || strncmp(s, "*/", 2) == 0)
          break;
        if (*s == '@' || strncmp(s, "#[", 2) == 0) {
          /* Skip directive line */
          while (*s != '\0' && *s != '\n' && strncmp(s, "*/", 2) != 0)
            s++;
          continue;
        }
        rc = serde_extract_tag_line(s, out_brief, brief_sz);
        break;
      }
    }
  }
  return rc;
}

/*
 * @brief Matches a directive scope keyword (`to_json`, `from_json`, `cli`,
 * etc.) to its `enum CddDirectiveScope`.
 */
cdd_c_error_t cdd_serde_match_scope_key(const char *key,
                                        enum CddDirectiveScope *out_scope) {
  if (strcmp(key, "to_json") == 0 || strcmp(key, "to_form") == 0) {
    *out_scope = CDD_SCOPE_TO_JSON;
  } else if (strcmp(key, "from_json") == 0 || strcmp(key, "from_form") == 0) {
    *out_scope = CDD_SCOPE_FROM_JSON;
  } else if (strcmp(key, "to_cli") == 0) {
    *out_scope = CDD_SCOPE_TO_CLI;
  } else if (strcmp(key, "from_cli") == 0) {
    *out_scope = CDD_SCOPE_FROM_CLI;
  } else if (strcmp(key, "cli") == 0 || strcmp(key, "arg") == 0 ||
             strcmp(key, "clap") == 0 || strcmp(key, "structopt") == 0 ||
             strcmp(key, "command") == 0) {
    *out_scope = CDD_SCOPE_CLI;
  } else {
    *out_scope = CDD_SCOPE_GENERAL;
  }
  return CDD_C_SUCCESS;
}
