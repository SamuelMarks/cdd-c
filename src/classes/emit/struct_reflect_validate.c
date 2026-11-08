/**
 * @file struct_reflect_validate.c
 * @brief Runtime reflection regex matching, OpenAPI format validation, and
 * schema constraint validation.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "classes/emit/struct_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "win_compat_sym.h"
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Checks if a character matches a bracketed character class `[...]` or
 * `[^...]`.
 *
 * @param[in] pat Pointer just after the opening `[`.
 * @param[out] out_after Pointer just after the closing `]`.
 * @param[in] c Character to test (`'\0'` never matches).
 * @param[out] out_matched Non-zero if `c` matched the class.
 * @return CDD_C_SUCCESS on valid class, or CDD_C_ERROR_INVALID_ARGUMENT if
 * unclosed.
 */
static cdd_c_error_t regex_match_class(const char *pat, const char **out_after,
                                       char c, int *out_matched) {
  int invert = 0;
  int hit = 0;
  const char *p = pat;
  if (*p == '^') {
    invert = 1;
    p++;
  }
  if (*p == ']') {
    if (c == ']')
      hit = 1;
    p++;
  }
  while (*p != '\0' && *p != ']') {
    if (p[0] == '\\') {
      char esc = p[1];
      if (esc == 'd') {
        if (isdigit((unsigned char)c))
          hit = 1;
      } else if (esc == 'w') {
        if (isalnum((unsigned char)c) || c == '_')
          hit = 1;
      } else if (esc == 's') {
        if (isspace((unsigned char)c))
          hit = 1;
      } else if (c == esc) {
        hit = 1;
      }
      p += 2;
    } else if (p[1] == '-') {
      if ((unsigned char)c >= (unsigned char)p[0] &&
          (unsigned char)c <= (unsigned char)p[2]) {
        hit = 1;
      }
      p += 3;
    } else {
      if (c == *p)
        hit = 1;
      p++;
    }
  }
  if (*p != ']')
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_after = p + 1;
  *out_matched = (c != '\0') ? (invert ? !hit : hit) : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Tests whether a single regex atom at `pat` matches character `c` and
 * advances `*out_next_pat` past the atom (before any quantifier).
 *
 * @param[in] pat Current regex pattern cursor.
 * @param[out] out_next_pat Cursor positioned immediately after the atom.
 * @param[in] c Input character to test.
 * @param[out] out_matched Non-zero if `c` matches the atom.
 * @return CDD_C_SUCCESS on success, or error code on malformed pattern.
 */
static cdd_c_error_t regex_match_atom(const char *pat,
                                      const char **out_next_pat, char c,
                                      int *out_matched) {
  *out_matched = 0;
  if (*pat == '[') {
    return regex_match_class(pat + 1, out_next_pat, c, out_matched);
  }
  if (*pat == '\\' && pat[1] != '\0') {
    char esc = pat[1];
    int is_w = (isalnum((unsigned char)c) || c == '_') ? 1 : 0;
    *out_next_pat = pat + 2;
    if (c == '\0')
      return CDD_C_SUCCESS;
    if (esc == 'd' || esc == 'D')
      *out_matched = (isdigit((unsigned char)c) ? 1 : 0) ^ (esc == 'D');
    else if (esc == 's' || esc == 'S')
      *out_matched = (isspace((unsigned char)c) ? 1 : 0) ^ (esc == 'S');
    else if (esc == 'w' || esc == 'W')
      *out_matched = is_w ^ (esc == 'W');
    else
      *out_matched = (c == esc) ? 1 : 0;
    return CDD_C_SUCCESS;
  }
  *out_next_pat = pat + 1;
  if (c != '\0' && (*pat == '.' || c == *pat))
    *out_matched = 1;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t regex_match_here(const char *pat, const char *str,
                                      int *out_matched, unsigned int depth);

static cdd_c_error_t cdd_reflect_match_pattern_depth(const char *pattern,
                                                     const char *str,
                                                     int *out_matched,
                                                     unsigned int depth);

/**
 * @brief Finds the closing `)` matching an opening `(` at `pat[0]`, respecting
 * nested parentheses, character classes `[...]`, and `\` escapes.
 *
 * @param[in] pat Pattern starting at `'('`.
 * @param[out] out_close Pointer to the matching `')'`.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * unclosed.
 */
static cdd_c_error_t regex_find_group_close(const char *pat,
                                            const char **out_close) {
  const char *p = pat + 1;
  int paren_depth = 1;
  int in_brackets = 0;
  while (*p != '\0') {
    if (*p == '[') {
      in_brackets = 1;
    } else if (*p == ']') {
      in_brackets = 0;
    } else if (!in_brackets) {
      if (*p == '(') {
        paren_depth++;
      } else if (*p == ')') {
        paren_depth--;
        if (paren_depth == 0) {
          *out_close = p;
          return CDD_C_SUCCESS;
        }
      }
    }
    p++;
  }
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Matches a quantified parenthesized group `(sub){min_rep, max_rep}`
 * followed by `rest_pat` against `str`.
 *
 * @param[in] group_inner Inner group pattern (NUL-terminated).
 * @param[in] rest_pat Pattern following the quantifier.
 * @param[in] str Current input string cursor.
 * @param[in] min_rep Minimum required repetitions.
 * @param[in] max_rep Maximum allowed repetitions (`(size_t)-1` for unbounded).
 * @param[out] out_matched Non-zero if matched.
 * @param[in] depth Current recursion depth.
 * @return CDD_C_SUCCESS on success, or error code on malformed pattern.
 */
static cdd_c_error_t
regex_match_quantified_group(const char *group_inner, const char *rest_pat,
                             const char *str, size_t min_rep, size_t max_rep,
                             int *out_matched, unsigned int depth) {
  size_t s_len;
  size_t i;
  cdd_c_error_t rc;
  *out_matched = 0;

  if (min_rep == 0) {
    int tail_m = 0;
    rc = regex_match_here(rest_pat, str, &tail_m, depth + 1U);
    if (rc != CDD_C_SUCCESS || tail_m) {
      *out_matched = tail_m;
      return rc;
    }
  }
  if (max_rep == 0)
    return CDD_C_SUCCESS;

  s_len = strlen(str);
  for (i = 1; i <= s_len; ++i) {
    char prefix_buf[256];
    char anchored_group[260];
    int grp_m = 0;
    memcpy(prefix_buf, str, i);
    prefix_buf[i] = '\0';
    CDD_SNPRINTF(anchored_group, sizeof(anchored_group), "^%s$", group_inner);
    rc = cdd_reflect_match_pattern_depth(anchored_group, prefix_buf, &grp_m,
                                         depth + 1U);
    if (grp_m) {
      int rem_m = 0;
      size_t next_min = (min_rep > 0) ? (min_rep - 1) : 0;
      size_t next_max = (max_rep == (size_t)-1) ? (size_t)-1 : (max_rep - 1);
      rc = regex_match_quantified_group(group_inner, rest_pat, str + i,
                                        next_min, next_max, &rem_m, depth + 1U);
      if (rc != CDD_C_SUCCESS || rem_m) {
        *out_matched = rem_m;
        return rc;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Matches a quantified atom `atom{min_rep, max_rep}` followed by
 * `rest_pat` against `str`.
 *
 * @param[in] atom_pat Start of the atom in the pattern.
 * @param[in] rest_pat Pattern following the quantifier.
 * @param[in] str Current input string cursor.
 * @param[in] min_rep Minimum required repetitions.
 * @param[in] max_rep Maximum allowed repetitions (`(size_t)-1` for unbounded).
 * @param[out] out_matched Non-zero if matched.
 * @param[in] depth Current recursion depth.
 * @return CDD_C_SUCCESS on success, or error code on malformed pattern.
 */
static cdd_c_error_t regex_match_quantified(const char *atom_pat,
                                            const char *rest_pat,
                                            const char *str, size_t min_rep,
                                            size_t max_rep, int *out_matched,
                                            unsigned int depth) {
  size_t count = 0;
  const char *s = str;
  const char *dummy_next = NULL;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  *out_matched = 0;

  while (count < max_rep && *s != '\0') {
    int m = 0;
    rc = regex_match_atom(atom_pat, &dummy_next, *s, &m);
    if (!m)
      break;
    s++;
    count++;
  }

  while (count >= min_rep && !*out_matched) {
    rc = regex_match_here(rest_pat, str + count, out_matched, depth + 1U);
    if (count == 0)
      break;
    count--;
  }
  return rc;
}

/**
 * @brief Parses an optional quantifier (`*`, `+`, `?`, `{m,n}`) at `after_atom`
 * and sets `*out_has_quant`, `*out_min`, `*out_max`, and `*out_rest`.
 *
 * @param[in] after_atom Cursor immediately following an atom or group.
 * @param[out] out_has_quant Set to 1 if a quantifier was parsed, 0 otherwise.
 * @param[out] out_min Minimum repetitions.
 * @param[out] out_max Maximum repetitions.
 * @param[out] out_rest Cursor following the quantifier.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT on invalid
 * `{m,n}` syntax.
 */
static cdd_c_error_t regex_parse_quantifier(const char *after_atom,
                                            int *out_has_quant, size_t *out_min,
                                            size_t *out_max,
                                            const char **out_rest) {
  *out_has_quant = 0;
  *out_min = 1;
  *out_max = 1;
  *out_rest = after_atom;
  if (*after_atom == '*') {
    *out_has_quant = 1;
    *out_min = 0;
    *out_max = (size_t)-1;
    *out_rest = after_atom + 1;
    return CDD_C_SUCCESS;
  }
  if (*after_atom == '+') {
    *out_has_quant = 1;
    *out_min = 1;
    *out_max = (size_t)-1;
    *out_rest = after_atom + 1;
    return CDD_C_SUCCESS;
  }
  if (*after_atom == '?') {
    *out_has_quant = 1;
    *out_min = 0;
    *out_max = 1;
    *out_rest = after_atom + 1;
    return CDD_C_SUCCESS;
  }
  if (*after_atom == '{') {
    const char *q = after_atom + 1;
    size_t min_r = 0;
    size_t max_r = 0;
    if (!isdigit((unsigned char)*q))
      return CDD_C_ERROR_INVALID_ARGUMENT;
    while (isdigit((unsigned char)*q)) {
      min_r = min_r * 10 + (size_t)(*q - '0');
      q++;
    }
    if (*q == ',') {
      q++;
      if (*q == '}') {
        max_r = (size_t)-1;
      } else {
        if (!isdigit((unsigned char)*q))
          return CDD_C_ERROR_INVALID_ARGUMENT;
        while (isdigit((unsigned char)*q)) {
          max_r = max_r * 10 + (size_t)(*q - '0');
          q++;
        }
      }
    } else {
      max_r = min_r;
    }
    if (*q != '}')
      return CDD_C_ERROR_INVALID_ARGUMENT;
    *out_has_quant = 1;
    *out_min = min_r;
    *out_max = max_r;
    *out_rest = q + 1;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Recursive anchored regex matcher from `pat` at `str`.
 *
 * @param[in] pat Pattern cursor.
 * @param[in] str Input string cursor.
 * @param[out] out_matched Non-zero if matched.
 * @param[in] depth Current recursion depth.
 * @return CDD_C_SUCCESS on success, or error code on malformed pattern.
 */
static cdd_c_error_t regex_match_here(const char *pat, const char *str,
                                      int *out_matched, unsigned int depth) {
  const char *after_atom = NULL;
  const char *rest_pat = NULL;
  int atom_m = 0;
  int has_quant = 0;
  size_t min_r = 1;
  size_t max_r = 1;
  cdd_c_error_t rc;
  *out_matched = 0;

  if (*pat == '\0') {
    *out_matched = 1;
    return CDD_C_SUCCESS;
  }
  if (pat[0] == '$') {
    *out_matched = (*str == '\0') ? 1 : 0;
    return CDD_C_SUCCESS;
  }

  if (*pat == '(') {
    const char *close_p = NULL;
    char group_buf[256];
    size_t g_len;
    rc = regex_find_group_close(pat, &close_p);
    if (rc != CDD_C_SUCCESS)
      return rc;
    g_len = (size_t)(close_p - (pat + 1));
    memcpy(group_buf, pat + 1, g_len);
    group_buf[g_len] = '\0';
    rc = regex_parse_quantifier(close_p + 1, &has_quant, &min_r, &max_r,
                                &rest_pat);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return regex_match_quantified_group(group_buf, rest_pat, str, min_r, max_r,
                                        out_matched, depth + 1U);
  }

  rc = regex_match_atom(pat, &after_atom, *str, &atom_m);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc =
      regex_parse_quantifier(after_atom, &has_quant, &min_r, &max_r, &rest_pat);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_quant) {
    return regex_match_quantified(pat, rest_pat, str, min_r, max_r, out_matched,
                                  depth + 1U);
  }

  if (atom_m) {
    return regex_match_here(after_atom, str + 1, out_matched, depth + 1U);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates a single regex branch (without top-level `|`) against `str`.
 *
 * @param[in] pat Pattern branch string.
 * @param[in] str Input string to test.
 * @param[out] out_matched Non-zero if `str` matches `pat`.
 * @param[in] depth Current recursion depth.
 * @return CDD_C_SUCCESS on success, or error code on malformed pattern.
 */
static cdd_c_error_t regex_match_branch(const char *pat, const char *str,
                                        int *out_matched, unsigned int depth) {
  const char *s = str;
  cdd_c_error_t rc;
  *out_matched = 0;
  if (*pat == '^') {
    return regex_match_here(pat + 1, str, out_matched, depth);
  }
  do {
    rc = regex_match_here(pat, s, out_matched, depth);
    if (rc != CDD_C_SUCCESS || *out_matched)
      return rc;
  } while (*s++ != '\0');
  return CDD_C_SUCCESS;
}

/**
 * @brief Depth-tracked helper for `cdd_reflect_match_pattern` supporting
 * top-level `|` alternation outside `[...]` and `(...)`.
 *
 * @param[in] pattern Regex pattern string.
 * @param[in] str Input string to validate.
 * @param[out] out_matched Non-zero if `str` matches `pattern`.
 * @param[in] depth Current recursion depth.
 * @return CDD_C_SUCCESS on success, or error code on malformed pattern.
 */
static cdd_c_error_t cdd_reflect_match_pattern_depth(const char *pattern,
                                                     const char *str,
                                                     int *out_matched,
                                                     unsigned int depth) {
  char branch_buf[256];
  const char *p = pattern;
  int in_brackets = 0;
  int paren_depth = 0;
  size_t b_len = 0;
  cdd_c_error_t rc;
  *out_matched = 0;

  while (*p != '\0') {
    if (*p == '\\' && p[1] != '\0') {
      if (b_len + 2 >= sizeof(branch_buf))
        return CDD_C_ERROR_INVALID_ARGUMENT;
      branch_buf[b_len++] = *p++;
      branch_buf[b_len++] = *p++;
      continue;
    }
    if (*p == '[')
      in_brackets = 1;
    else if (*p == ']')
      in_brackets = 0;
    else if (!in_brackets && *p == '(')
      paren_depth++;
    else if (!in_brackets && *p == ')') {
      if (paren_depth == 0)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      paren_depth--;
    } else if (*p == '|' && !in_brackets && paren_depth == 0) {
      branch_buf[b_len] = '\0';
      rc = regex_match_branch(branch_buf, str, out_matched, depth);
      if (rc != CDD_C_SUCCESS || *out_matched)
        return rc;
      b_len = 0;
      p++;
      continue;
    }
    if (b_len + 1 >= sizeof(branch_buf))
      return CDD_C_ERROR_INVALID_ARGUMENT;
    branch_buf[b_len++] = *p++;
  }
  branch_buf[b_len] = '\0';
  return regex_match_branch(branch_buf, str, out_matched, depth);
}

/**
 * @brief Portable C89 regex pattern evaluator supporting `^`, `$`, `.`,
 * `[...]`, `(...)`, `\d`, `\w`, `\s`, `*`, `+`, `?`, `{m,n}`, and `|`
 * alternation.
 *
 * @param[in] pattern Regex pattern string.
 * @param[in] str Input string to validate.
 * @param[out] out_matched Non-zero if `str` matches `pattern`.
 * @return CDD_C_SUCCESS on success, or error code on malformed pattern.
 */
static cdd_c_error_t cdd_reflect_match_pattern(const char *pattern,
                                               const char *str,
                                               int *out_matched) {
  return cdd_reflect_match_pattern_depth(pattern, str, out_matched, 0U);
}

/**
 * @brief Validates a string value against standard OpenAPI `format` constraints
 * (`"uuid"`, `"email"`, `"date"`, `"date-time"`, `"ipv4"`, `"ipv6"`, `"uri"`,
 * `"hostname"`).
 *
 * @param[in] format OpenAPI format identifier.
 * @param[in] s NUL-terminated string to validate.
 * @return CDD_C_SUCCESS if valid or unrecognized format, or
 * CDD_C_ERROR_INVALID_ARGUMENT on format violation.
 */
static cdd_c_error_t validate_string_format(const char *format, const char *s) {
  size_t len = strlen(s);
  size_t i;

  if (strcmp(format, "uuid") == 0) {
    if (len != 36)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    for (i = 0; i < 36; ++i) {
      if (i == 8 || i == 13 || i == 18 || i == 23) {
        if (s[i] != '-')
          return CDD_C_ERROR_INVALID_ARGUMENT;
      } else if (!isxdigit((unsigned char)s[i])) {
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
    return CDD_C_SUCCESS;
  }
  if (strcmp(format, "email") == 0) {
    const char *at = strchr(s, '@');
    const char *dot;
    if (!at || at == s || strchr(at + 1, '@') != NULL)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    dot = strchr(at + 1, '.');
    if (!dot || dot == at + 1 || dot[1] == '\0')
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return CDD_C_SUCCESS;
  }
  if (strcmp(format, "date") == 0 || strcmp(format, "date-time") == 0) {
    int month, day;
    if (len < 10 || s[4] != '-' || s[7] != '-' ||
        (strcmp(format, "date") == 0 && len != 10))
      return CDD_C_ERROR_INVALID_ARGUMENT;
    for (i = 0; i < 10; ++i) {
      if (i != 4 && i != 7 && !isdigit((unsigned char)s[i]))
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    month = (s[5] - '0') * 10 + (s[6] - '0');
    day = (s[8] - '0') * 10 + (s[9] - '0');
    if (month < 1 || month > 12 || day < 1 || day > 31)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (strcmp(format, "date-time") == 0) {
      if (len < 20 || s[10] != 'T' || s[13] != ':' || s[16] != ':')
        return CDD_C_ERROR_INVALID_ARGUMENT;
      for (i = 11; i <= 18; ++i) {
        if (i != 13 && i != 16 && !isdigit((unsigned char)s[i]))
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      if ((s[11] - '0') * 10 + (s[12] - '0') > 23 ||
          (s[14] - '0') * 10 + (s[15] - '0') > 59 ||
          (s[17] - '0') * 10 + (s[18] - '0') > 60)
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    return CDD_C_SUCCESS;
  }
  if (strcmp(format, "ipv4") == 0) {
    int octets = 0;
    const char *p = s;
    while (*p != '\0') {
      int val = 0;
      int digits = 0;
      while (isdigit((unsigned char)*p)) {
        val = val * 10 + (*p - '0');
        digits++;
        if (digits > 3 || val > 255)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        p++;
      }
      if (digits == 0)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      octets++;
      if (*p == '.') {
        p++;
        if (*p == '\0')
          return CDD_C_ERROR_INVALID_ARGUMENT;
      } else if (*p != '\0') {
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
    return (octets == 4) ? CDD_C_SUCCESS : CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (strcmp(format, "ipv6") == 0) {
    if (len < 2 || strchr(s, ':') == NULL)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    for (i = 0; i < len; ++i) {
      if (!isxdigit((unsigned char)s[i]) && s[i] != ':')
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    return CDD_C_SUCCESS;
  }
  if (strcmp(format, "uri") == 0) {
    const char *colon = strchr(s, ':');
    if (!colon || colon == s || !isalpha((unsigned char)s[0]))
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return CDD_C_SUCCESS;
  }
  if (strcmp(format, "hostname") == 0) {
    if (len == 0 || s[0] == '-' || s[len - 1] == '-') {
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    for (i = 0; i < len; ++i) {
      if (!isalnum((unsigned char)s[i]) && s[i] != '-' && s[i] != '.')
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Validates that all elements in an array field are pairwise distinct
 * when `field->unique_items != 0`.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected array field descriptor.
 * @param[in] alen Number of elements in the array.
 * @return CDD_C_SUCCESS if all items are unique, or
 * CDD_C_ERROR_INVALID_ARGUMENT if duplicates exist.
 */
static cdd_c_error_t
validate_array_unique_items(const void *obj, const struct cdd_field_info *field,
                            size_t alen) {
  size_t i, j;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  for (i = 0; i < alen; ++i) {
    const void *pi = NULL;
    rc = cdd_reflect_array_at(obj, field, i, &pi);
    for (j = i + 1; j < alen; ++j) {
      const void *pj = NULL;
      rc = cdd_reflect_array_at(obj, field, j, &pj);
      if (field->elem_kind == CDD_FIELD_KIND_STRING) {
        const char *si = *(const char *const *)pi;
        const char *sj = *(const char *const *)pj;
        if (si == sj || (si && sj && strcmp(si, sj) == 0))
          return CDD_C_ERROR_INVALID_ARGUMENT;
      } else if (field->elem_kind == CDD_FIELD_KIND_OBJECT) {
        const struct cdd_type_info *elem_ti = NULL;
        if (cdd_reflect_type_by_name(field->ref_name, &elem_ti) ==
            CDD_C_SUCCESS) {
          int is_p = (field->elem_size == sizeof(void *));
          const void *oi = is_p ? *(const void *const *)pi : pi;
          const void *oj = is_p ? *(const void *const *)pj : pj;
          int is_eq = 0;
          rc = cdd_reflect_struct_eq(oi, oj, elem_ti, &is_eq);
          if (is_eq)
            return CDD_C_ERROR_INVALID_ARGUMENT;
        }
      } else if (memcmp(pi, pj, field->elem_size) == 0) {
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
  }
  return rc;
}

/*
 * @brief Validates a single reflected field on `obj` against its schema
 * constraints (`required`, `has_min`/`min_val`, `exclusive_min`,
 * `has_max`/`max_val`, `exclusive_max`, `has_min_len`/`min_len`,
 * `has_max_len`/`max_len`, `unique_items`, `pattern`, `format`, and enum
 * membership).
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT if a
 * constraint is violated or arguments are NULL.
 */
cdd_c_error_t cdd_reflect_validate_field(const void *obj,
                                         const struct cdd_field_info *field) {
  if (!obj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (field->is_pointer && field->bit_width == 0) {
    const void *const *pptr = (const void *const *)((
        const void *)((const unsigned char *)obj + field->offset));
    if (!*pptr) {
      return (field->required && !field->is_nullable)
                 ? CDD_C_ERROR_INVALID_ARGUMENT
                 : CDD_C_SUCCESS;
    }
  }

  if (field->kind == CDD_FIELD_KIND_INTEGER ||
      field->kind == CDD_FIELD_KIND_ENUM) {
    int64_t v = 0;
    uint64_t uv = 0;
    double dv = 0.0;
    int is_u64 = 0;
    size_t width = 0;
    int is_unsigned = 0;
    cdd_c_error_t rc =
        cdd_reflect_resolve_field_scalar_width(field, &width, &is_unsigned);
    if (width == 8 && is_unsigned) {
      rc = cdd_reflect_get_uint64(obj, field, &uv);
      dv = (double)uv;
      is_u64 = 1;
    } else {
      rc = cdd_reflect_get_int64(obj, field, &v);
      dv = (double)v;
    }
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (field->has_min) {
      if (field->exclusive_min ? (dv <= field->min_val) : (dv < field->min_val))
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    if (field->has_max) {
      if (field->exclusive_max ? (dv >= field->max_val) : (dv > field->max_val))
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    if (field->format) {
      if (strcmp(field->format, "int32") == 0 &&
          (dv < (double)INT_MIN || dv > (double)INT_MAX))
        return CDD_C_ERROR_INVALID_ARGUMENT;
      if (strcmp(field->format, "uint32") == 0 &&
          (dv < 0.0 || dv > (double)0xFFFFFFFFUL))
        return CDD_C_ERROR_INVALID_ARGUMENT;
      if (strcmp(field->format, "uint64") == 0 && !is_u64 && v < 0)
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    if (field->kind == CDD_FIELD_KIND_ENUM) {
      const struct cdd_enum_info *ei = NULL;
      if (cdd_reflect_enum_by_name(field->ref_name, &ei) == CDD_C_SUCCESS &&
          ei->other_idx < 0) {
        const struct cdd_enum_variant_info *ev = NULL;
        if (cdd_reflect_find_enum_by_value(ei, (int)v, &ev) != CDD_C_SUCCESS)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
  } else if (field->kind == CDD_FIELD_KIND_NUMBER) {
    double d = 0.0;
    cdd_c_error_t rc = cdd_reflect_get_double(obj, field, &d);
    if (field->has_min) {
      if (field->exclusive_min ? (d <= field->min_val) : (d < field->min_val))
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    if (field->has_max) {
      if (field->exclusive_max ? (d >= field->max_val) : (d > field->max_val))
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    return rc;
  } else if (field->kind == CDD_FIELD_KIND_STRING) {
    const char *s = NULL;
    size_t slen;
    cdd_c_error_t rc = cdd_reflect_get_string(obj, field, &s);
    if (!s)
      return rc;
    slen = strlen(s);
    if (field->has_min_len && slen < field->min_len)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (field->has_max_len && slen > field->max_len)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (field->format && field->format[0] != '\0') {
      rc = validate_string_format(field->format, s);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (field->pattern) {
      int matched = 0;
      rc = cdd_reflect_match_pattern(field->pattern, s, &matched);
      if (rc != CDD_C_SUCCESS || !matched)
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  } else if (field->kind == CDD_FIELD_KIND_ARRAY) {
    size_t alen = 0;
    cdd_c_error_t rc = cdd_reflect_array_len(obj, field, &alen);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (field->has_min_len && alen < field->min_len)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (field->has_max_len && alen > field->max_len)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (field->unique_items && alen > 1) {
      return validate_array_unique_items(obj, field, alen);
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Context wrapper passed to `validate_field_visitor`.
 */
struct ValidateVisitorCtx {
  const void *obj;         /**< Const struct instance being validated */
  const void *visited[16]; /**< Ancestor object pointers for cycle breaking */
  unsigned int depth;      /**< Current recursion depth */
};

static cdd_c_error_t validate_struct_recursive(const void *obj,
                                               const struct cdd_type_info *ti,
                                               const void *const *visited,
                                               unsigned int depth);

/**
 * @brief Internal visitor callback used by `cdd_reflect_validate`.
 *
 * @param[in] field Current field descriptor.
 * @param[in] field_ptr Pointer to field data.
 * @param[in] user_data Pointer to `struct ValidateVisitorCtx`.
 * @return CDD_C_SUCCESS if field and any nested objects validate, or error
 * code.
 */
static cdd_c_error_t validate_field_visitor(const struct cdd_field_info *field,
                                            const void *field_ptr,
                                            void *user_data) {
  const struct ValidateVisitorCtx *ctx =
      (const struct ValidateVisitorCtx *)user_data;
  cdd_c_error_t rc = cdd_reflect_validate_field(ctx->obj, field);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (field->kind == CDD_FIELD_KIND_OBJECT) {
    const struct cdd_type_info *child_ti = NULL;
    if (cdd_reflect_type_by_name(field->ref_name, &child_ti) == CDD_C_SUCCESS) {
      const void *child_obj =
          field->is_pointer ? *(const void *const *)field_ptr : field_ptr;
      if (child_obj) {
        return validate_struct_recursive(child_obj, child_ti, ctx->visited,
                                         ctx->depth + 1U);
      }
    }
  } else if (field->kind == CDD_FIELD_KIND_ARRAY &&
             field->elem_kind == CDD_FIELD_KIND_OBJECT) {
    const struct cdd_type_info *elem_ti = NULL;
    if (cdd_reflect_type_by_name(field->ref_name, &elem_ti) == CDD_C_SUCCESS) {
      size_t alen = 0;
      size_t idx;
      rc = cdd_reflect_array_len(ctx->obj, field, &alen);
      for (idx = 0; idx < alen; ++idx) {
        const void *elem_ptr = NULL;
        rc = cdd_reflect_array_at(ctx->obj, field, idx, &elem_ptr);
        {
          const void *elem_obj = (field->elem_size == sizeof(void *))
                                     ? *(const void *const *)elem_ptr
                                     : elem_ptr;
          if (elem_obj) {
            rc = validate_struct_recursive(elem_obj, elem_ti, ctx->visited,
                                           ctx->depth + 1U);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }
      }
    }
  }
  return rc;
}

/**
 * @brief Cycle-safe recursive struct validator helper.
 *
 * @param[in] obj Struct instance to validate.
 * @param[in] ti Runtime type descriptor.
 * @param[in] visited Array of `depth` ancestor object pointers.
 * @param[in] depth Current recursion depth.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t validate_struct_recursive(const void *obj,
                                               const struct cdd_type_info *ti,
                                               const void *const *visited,
                                               unsigned int depth) {
  struct ValidateVisitorCtx ctx;
  unsigned int k;
  if (depth >= 16U)
    return CDD_C_SUCCESS;
  for (k = 0; k < depth; ++k) {
    if (visited[k] == obj)
      return CDD_C_SUCCESS;
    ctx.visited[k] = visited[k];
  }
  ctx.visited[depth] = obj;
  ctx.obj = obj;
  ctx.depth = depth;
  return cdd_reflect_visit_fields(obj, ti, validate_field_visitor, &ctx);
}

/*
 * @brief Validates all non-skipped fields of a struct or active union branch on
 * `obj` (and any registered nested objects/arrays) against their schema
 * constraints in `ti`.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @return CDD_C_SUCCESS if all fields pass validation, or
 * CDD_C_ERROR_INVALID_ARGUMENT on constraint violation or NULL arguments.
 */
cdd_c_error_t cdd_reflect_validate(const void *obj,
                                   const struct cdd_type_info *ti) {
  if (!obj || !ti)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return validate_struct_recursive(obj, ti, NULL, 0U);
}
