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
    if (p[0] == '\\' && p[1] != '\0') {
      char esc = p[1];
      if (esc == 'd' && isdigit((unsigned char)c))
        hit = 1;
      else if (esc == 'w' && (isalnum((unsigned char)c) || c == '_'))
        hit = 1;
      else if (esc == 's' && isspace((unsigned char)c))
        hit = 1;
      else if (c == esc)
        hit = 1;
      p += 2;
    } else if (p[1] == '-' && p[2] != '\0' && p[2] != ']') {
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
    *out_next_pat = pat + 2;
    if (c == '\0')
      return CDD_C_SUCCESS;
    if (esc == 'd')
      *out_matched = isdigit((unsigned char)c) ? 1 : 0;
    else if (esc == 'D')
      *out_matched = !isdigit((unsigned char)c) ? 1 : 0;
    else if (esc == 'w')
      *out_matched = (isalnum((unsigned char)c) || c == '_') ? 1 : 0;
    else if (esc == 'W')
      *out_matched = !(isalnum((unsigned char)c) || c == '_') ? 1 : 0;
    else if (esc == 's')
      *out_matched = isspace((unsigned char)c) ? 1 : 0;
    else if (esc == 'S')
      *out_matched = !isspace((unsigned char)c) ? 1 : 0;
    else
      *out_matched = (c == esc) ? 1 : 0;
    return CDD_C_SUCCESS;
  }
  *out_next_pat = pat + 1;
  if (c == '\0')
    return CDD_C_SUCCESS;
  if (*pat == '.') {
    *out_matched = (c != '\n') ? 1 : 0;
  } else {
    *out_matched = (c == *pat) ? 1 : 0;
  }
  return CDD_C_SUCCESS;
}

static cdd_c_error_t regex_match_here(const char *pat, const char *str,
                                      int *out_matched);

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
 * @return CDD_C_SUCCESS on success, or error code on malformed pattern.
 */
static cdd_c_error_t regex_match_quantified(const char *atom_pat,
                                            const char *rest_pat,
                                            const char *str, size_t min_rep,
                                            size_t max_rep, int *out_matched) {
  size_t count = 0;
  const char *s = str;
  const char *dummy_next = NULL;
  cdd_c_error_t rc;
  *out_matched = 0;

  while (count < max_rep && *s != '\0') {
    int m = 0;
    rc = regex_match_atom(atom_pat, &dummy_next, *s, &m);
    if (rc != CDD_C_SUCCESS || !m)
      break;
    s++;
    count++;
  }

  while (count >= min_rep) {
    int tail_m = 0;
    rc = regex_match_here(rest_pat, str + count, &tail_m);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (tail_m) {
      *out_matched = 1;
      return CDD_C_SUCCESS;
    }
    if (count == 0)
      break;
    count--;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Recursive anchored regex matcher from `pat` at `str`.
 *
 * @param[in] pat Pattern cursor.
 * @param[in] str Input string cursor.
 * @param[out] out_matched Non-zero if matched.
 * @return CDD_C_SUCCESS on success, or error code on malformed pattern.
 */
static cdd_c_error_t regex_match_here(const char *pat, const char *str,
                                      int *out_matched) {
  const char *after_atom = NULL;
  int atom_m = 0;
  cdd_c_error_t rc;
  *out_matched = 0;

  if (*pat == '\0') {
    *out_matched = 1;
    return CDD_C_SUCCESS;
  }
  if (pat[0] == '$' && pat[1] == '\0') {
    *out_matched = (*str == '\0') ? 1 : 0;
    return CDD_C_SUCCESS;
  }

  rc = regex_match_atom(pat, &after_atom, *str, &atom_m);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (*after_atom == '*') {
    return regex_match_quantified(pat, after_atom + 1, str, 0, (size_t)-1,
                                  out_matched);
  }
  if (*after_atom == '+') {
    return regex_match_quantified(pat, after_atom + 1, str, 1, (size_t)-1,
                                  out_matched);
  }
  if (*after_atom == '?') {
    return regex_match_quantified(pat, after_atom + 1, str, 0, 1, out_matched);
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
    return regex_match_quantified(pat, q + 1, str, min_r, max_r, out_matched);
  }

  if (atom_m && *str != '\0') {
    return regex_match_here(after_atom, str + 1, out_matched);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates a single regex branch (without top-level `|`) against `str`.
 *
 * @param[in] pat Pattern branch string.
 * @param[in] str Input string to test.
 * @param[out] out_matched Non-zero if `str` matches `pat`.
 * @return CDD_C_SUCCESS on success, or error code on malformed pattern.
 */
static cdd_c_error_t regex_match_branch(const char *pat, const char *str,
                                        int *out_matched) {
  const char *s = str;
  cdd_c_error_t rc;
  *out_matched = 0;
  if (*pat == '^') {
    return regex_match_here(pat + 1, str, out_matched);
  }
  do {
    rc = regex_match_here(pat, s, out_matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (*out_matched)
      return CDD_C_SUCCESS;
  } while (*s++ != '\0');
  return CDD_C_SUCCESS;
}

/**
 * @brief Portable C89 regex pattern evaluator supporting `^`, `$`, `.`,
 * `[...]`,
 * `\d`, `\w`, `\s`, `*`, `+`, `?`, `{m,n}`, and top-level `|` alternation.
 *
 * @param[in] pattern Regex pattern string.
 * @param[in] str Input string to validate.
 * @param[out] out_matched Non-zero if `str` matches `pattern`.
 * @return CDD_C_SUCCESS on success, or error code on malformed pattern.
 */
static cdd_c_error_t cdd_reflect_match_pattern(const char *pattern,
                                               const char *str,
                                               int *out_matched) {
  char branch_buf[256];
  const char *p = pattern;
  int in_brackets = 0;
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
    else if (*p == '|' && !in_brackets) {
      branch_buf[b_len] = '\0';
      rc = regex_match_branch(branch_buf, str, out_matched);
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
  return regex_match_branch(branch_buf, str, out_matched);
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
    int year = 0, month = 0, day = 0;
    if (len < 10 || s[4] != '-' || s[7] != '-')
      return CDD_C_ERROR_INVALID_ARGUMENT;
    for (i = 0; i < 10; ++i) {
      if (i == 4 || i == 7)
        continue;
      if (!isdigit((unsigned char)s[i]))
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    year = (s[0] - '0') * 1000 + (s[1] - '0') * 100 + (s[2] - '0') * 10 +
           (s[3] - '0');
    month = (s[5] - '0') * 10 + (s[6] - '0');
    day = (s[8] - '0') * 10 + (s[9] - '0');
    if (year < 1 || month < 1 || month > 12 || day < 1 || day > 31)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (strcmp(format, "date") == 0 && len != 10)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (strcmp(format, "date-time") == 0) {
      int hr, mn, sc;
      if (len < 20 || (s[10] != 'T' && s[10] != 't' && s[10] != ' ') ||
          s[13] != ':' || s[16] != ':')
        return CDD_C_ERROR_INVALID_ARGUMENT;
      if (!isdigit((unsigned char)s[11]) || !isdigit((unsigned char)s[12]) ||
          !isdigit((unsigned char)s[14]) || !isdigit((unsigned char)s[15]) ||
          !isdigit((unsigned char)s[17]) || !isdigit((unsigned char)s[18]))
        return CDD_C_ERROR_INVALID_ARGUMENT;
      hr = (s[11] - '0') * 10 + (s[12] - '0');
      mn = (s[14] - '0') * 10 + (s[15] - '0');
      sc = (s[17] - '0') * 10 + (s[18] - '0');
      if (hr > 23 || mn > 59 || sc > 60)
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
      if (!isxdigit((unsigned char)s[i]) && s[i] != ':' && s[i] != '.')
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
    if (len == 0 || len > 253 || s[0] == '-' || s[0] == '.' ||
        s[len - 1] == '-')
      return CDD_C_ERROR_INVALID_ARGUMENT;
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
  for (i = 0; i < alen; ++i) {
    const void *pi = NULL;
    cdd_reflect_array_at(obj, field, i, &pi);
    for (j = i + 1; j < alen; ++j) {
      const void *pj = NULL;
      cdd_reflect_array_at(obj, field, j, &pj);
      if (field->elem_kind == CDD_FIELD_KIND_STRING &&
          field->elem_size == sizeof(char *)) {
        const char *si = *(const char *const *)pi;
        const char *sj = *(const char *const *)pj;
        if (si == sj || (si && sj && strcmp(si, sj) == 0))
          return CDD_C_ERROR_INVALID_ARGUMENT;
      } else if (field->elem_kind == CDD_FIELD_KIND_OBJECT && field->ref_name &&
                 field->ref_name[0] != '\0') {
        const struct cdd_type_info *elem_ti = NULL;
        if (cdd_reflect_type_by_name(field->ref_name, &elem_ti) ==
                CDD_C_SUCCESS &&
            elem_ti) {
          const void *oi = (field->elem_size == sizeof(void *) &&
                            elem_ti->size != sizeof(void *))
                               ? *(const void *const *)pi
                               : pi;
          const void *oj = (field->elem_size == sizeof(void *) &&
                            elem_ti->size != sizeof(void *))
                               ? *(const void *const *)pj
                               : pj;
          int is_eq = 0;
          cdd_reflect_struct_eq(oi, oj, elem_ti, &is_eq);
          if (is_eq)
            return CDD_C_ERROR_INVALID_ARGUMENT;
        }
      } else if (field->elem_size > 0 &&
                 memcmp(pi, pj, field->elem_size) == 0) {
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
  }
  return CDD_C_SUCCESS;
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
    int have_v = 0;
    if (field->has_min || field->has_max ||
        field->kind == CDD_FIELD_KIND_ENUM ||
        (field->format && field->format[0] != '\0')) {
      cdd_c_error_t rc = cdd_reflect_get_int64(obj, field, &v);
      if (rc != CDD_C_SUCCESS)
        return rc;
      have_v = 1;
    }
    if (have_v && field->has_min) {
      if (field->exclusive_min ? ((double)v <= field->min_val)
                               : ((double)v < field->min_val))
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    if (have_v && field->has_max) {
      if (field->exclusive_max ? ((double)v >= field->max_val)
                               : ((double)v > field->max_val))
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    if (have_v && field->format && field->format[0] != '\0') {
      if (strcmp(field->format, "int32") == 0 &&
          (v < (int64_t)INT_MIN || v > (int64_t)INT_MAX))
        return CDD_C_ERROR_INVALID_ARGUMENT;
      if (strcmp(field->format, "uint32") == 0 &&
          (v < 0 || v > (int64_t)0xFFFFFFFFUL))
        return CDD_C_ERROR_INVALID_ARGUMENT;
      if (strcmp(field->format, "uint64") == 0 && v < 0)
        return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    if (have_v && field->kind == CDD_FIELD_KIND_ENUM && field->ref_name &&
        field->ref_name[0] != '\0') {
      const struct cdd_enum_info *ei = NULL;
      if (cdd_reflect_enum_by_name(field->ref_name, &ei) == CDD_C_SUCCESS &&
          ei && ei->other_idx < 0) {
        const struct cdd_enum_variant_info *ev = NULL;
        if (cdd_reflect_find_enum_by_value(ei, (int)v, &ev) != CDD_C_SUCCESS)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
  } else if (field->kind == CDD_FIELD_KIND_NUMBER) {
    if (field->has_min || field->has_max) {
      double d = 0.0;
      cdd_c_error_t rc = cdd_reflect_get_double(obj, field, &d);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (field->has_min) {
        if (field->exclusive_min ? (d <= field->min_val) : (d < field->min_val))
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      if (field->has_max) {
        if (field->exclusive_max ? (d >= field->max_val) : (d > field->max_val))
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
  } else if (field->kind == CDD_FIELD_KIND_STRING) {
    const char *s = NULL;
    size_t slen;
    cdd_c_error_t rc;
    cdd_reflect_get_string(obj, field, &s);
    if (!s)
      return CDD_C_SUCCESS;
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
    if (field->pattern && field->pattern[0] != '\0') {
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
      rc = validate_array_unique_items(obj, field, alen);
      if (rc != CDD_C_SUCCESS)
        return rc;
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

  if (field->kind == CDD_FIELD_KIND_OBJECT && field->ref_name &&
      field->ref_name[0] != '\0' && field_ptr) {
    const struct cdd_type_info *child_ti = NULL;
    if (cdd_reflect_type_by_name(field->ref_name, &child_ti) == CDD_C_SUCCESS &&
        child_ti) {
      const void *child_obj =
          field->is_pointer ? *(const void *const *)field_ptr : field_ptr;
      if (child_obj) {
        rc = validate_struct_recursive(child_obj, child_ti, ctx->visited,
                                       ctx->depth + 1U);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  } else if (field->kind == CDD_FIELD_KIND_ARRAY &&
             field->elem_kind == CDD_FIELD_KIND_OBJECT && field->ref_name &&
             field->ref_name[0] != '\0') {
    const struct cdd_type_info *elem_ti = NULL;
    if (cdd_reflect_type_by_name(field->ref_name, &elem_ti) == CDD_C_SUCCESS &&
        elem_ti) {
      size_t alen = 0;
      size_t idx;
      if (cdd_reflect_array_len(ctx->obj, field, &alen) == CDD_C_SUCCESS) {
        for (idx = 0; idx < alen; ++idx) {
          const void *elem_ptr = NULL;
          if (cdd_reflect_array_at(ctx->obj, field, idx, &elem_ptr) ==
                  CDD_C_SUCCESS &&
              elem_ptr) {
            const void *elem_obj = (field->elem_size == sizeof(void *) &&
                                    elem_ti->size != sizeof(void *))
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
  }
  return CDD_C_SUCCESS;
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
