/**
 * @file cdd_serde.c
 * @brief Implementation of Serde-style C89 case conversion, string rewriting,
 * wire-name resolution, and string escaping utilities.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "classes/parse/cdd_serde_internal.h"
/* clang-format on */

/*
 * @brief Parses a case transformation rule string into `enum CddSerdeCaseRule`.
 */
cdd_c_error_t cdd_serde_parse_case_rule(const char *str,
                                        enum CddSerdeCaseRule *out_rule) {
  int eq = 0;
  cdd_c_error_t rc;
  if (!str || !out_rule)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_serde_str_ieq(str, "none", &eq);
  if (eq || str[0] == '\0') {
    *out_rule = CDD_SERDE_CASE_NONE;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "lowercase", &eq);
  if (eq) {
    *out_rule = CDD_SERDE_CASE_LOWERCASE;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "uppercase", &eq);
  if (eq) {
    *out_rule = CDD_SERDE_CASE_UPPERCASE;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "pascalcase", &eq);
  if (eq) {
    *out_rule = CDD_SERDE_CASE_PASCALCASE;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "camelcase", &eq);
  if (eq) {
    *out_rule = CDD_SERDE_CASE_CAMELCASE;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "snake_case", &eq);
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "snakecase", &eq);
  }
  if (eq) {
    *out_rule = CDD_SERDE_CASE_SNAKECASE;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "screaming_snake_case", &eq);
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "screaming_snakecase", &eq);
  }
  if (eq) {
    *out_rule = CDD_SERDE_CASE_SCREAMING_SNAKECASE;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "kebab-case", &eq);
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "kebabcase", &eq);
  }
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "kebab_case", &eq);
  }
  if (eq) {
    *out_rule = CDD_SERDE_CASE_KEBABCASE;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "screaming-kebab-case", &eq);
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "screaming_kebabcase", &eq);
  }
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "screaming_kebab_case", &eq);
  }
  if (eq) {
    *out_rule = CDD_SERDE_CASE_SCREAMING_KEBABCASE;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "train-case", &eq);
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "traincase", &eq);
  }
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "train_case", &eq);
  }
  if (eq) {
    *out_rule = CDD_SERDE_CASE_TRAINCASE;
    return rc;
  }

  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/*
 * @brief Converts an `enum CddSerdeCaseRule` value to its canonical string
 * representation.
 */
cdd_c_error_t cdd_serde_case_rule_to_str(enum CddSerdeCaseRule rule,
                                         const char **out_str) {
  if (!out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (rule) {
  case CDD_SERDE_CASE_LOWERCASE:
    *out_str = "lowercase";
    break;
  case CDD_SERDE_CASE_UPPERCASE:
    *out_str = "UPPERCASE";
    break;
  case CDD_SERDE_CASE_PASCALCASE:
    *out_str = "PascalCase";
    break;
  case CDD_SERDE_CASE_CAMELCASE:
    *out_str = "camelCase";
    break;
  case CDD_SERDE_CASE_SNAKECASE:
    *out_str = "snake_case";
    break;
  case CDD_SERDE_CASE_SCREAMING_SNAKECASE:
    *out_str = "SCREAMING_SNAKE_CASE";
    break;
  case CDD_SERDE_CASE_KEBABCASE:
    *out_str = "kebab-case";
    break;
  case CDD_SERDE_CASE_SCREAMING_KEBABCASE:
    *out_str = "SCREAMING-KEBAB-CASE";
    break;
  case CDD_SERDE_CASE_TRAINCASE:
    *out_str = "Train-Case";
    break;
  case CDD_SERDE_CASE_NONE:
  default:
    *out_str = "none";
    break;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Parses a CLI action string into `enum CddCliAction`.
 */
cdd_c_error_t cdd_cli_parse_action(const char *str,
                                   enum CddCliAction *out_action) {
  int eq = 0;
  cdd_c_error_t rc;
  if (!str || !out_action)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_serde_str_ieq(str, "auto", &eq);
  if (eq || str[0] == '\0') {
    *out_action = CDD_CLI_ACTION_AUTO;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "set", &eq);
  if (eq) {
    *out_action = CDD_CLI_ACTION_SET;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "set_true", &eq);
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "settrue", &eq);
  }
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "true", &eq);
  }
  if (eq) {
    *out_action = CDD_CLI_ACTION_SET_TRUE;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "set_false", &eq);
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "setfalse", &eq);
  }
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "false", &eq);
  }
  if (eq) {
    *out_action = CDD_CLI_ACTION_SET_FALSE;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "count", &eq);
  if (eq) {
    *out_action = CDD_CLI_ACTION_COUNT;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "append", &eq);
  if (eq) {
    *out_action = CDD_CLI_ACTION_APPEND;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "help", &eq);
  if (eq) {
    *out_action = CDD_CLI_ACTION_HELP;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "version", &eq);
  if (eq) {
    *out_action = CDD_CLI_ACTION_VERSION;
    return rc;
  }

  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/*
 * @brief Converts an `enum CddCliAction` value to its canonical string
 * representation.
 */
cdd_c_error_t cdd_cli_action_to_str(enum CddCliAction action,
                                    const char **out_str) {
  if (!out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (action) {
  case CDD_CLI_ACTION_SET:
    *out_str = "set";
    break;
  case CDD_CLI_ACTION_SET_TRUE:
    *out_str = "set_true";
    break;
  case CDD_CLI_ACTION_SET_FALSE:
    *out_str = "set_false";
    break;
  case CDD_CLI_ACTION_COUNT:
    *out_str = "count";
    break;
  case CDD_CLI_ACTION_APPEND:
    *out_str = "append";
    break;
  case CDD_CLI_ACTION_HELP:
    *out_str = "help";
    break;
  case CDD_CLI_ACTION_VERSION:
    *out_str = "version";
    break;
  case CDD_CLI_ACTION_AUTO:
  default:
    *out_str = "auto";
    break;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Parses a CLI value hint string into `enum CddCliValueHint`.
 */
cdd_c_error_t cdd_cli_parse_value_hint(const char *str,
                                       enum CddCliValueHint *out_hint) {
  int eq = 0;
  cdd_c_error_t rc;
  if (!str || !out_hint)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_serde_str_ieq(str, "none", &eq);
  if (eq || str[0] == '\0') {
    *out_hint = CDD_CLI_HINT_NONE;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "file", &eq);
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "file_path", &eq);
  }
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "filepath", &eq);
  }
  if (eq) {
    *out_hint = CDD_CLI_HINT_FILE_PATH;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "dir", &eq);
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "dir_path", &eq);
  }
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "dirpath", &eq);
  }
  if (eq) {
    *out_hint = CDD_CLI_HINT_DIR_PATH;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "path", &eq);
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "any_path", &eq);
  }
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "anypath", &eq);
  }
  if (eq) {
    *out_hint = CDD_CLI_HINT_ANY_PATH;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "cmd", &eq);
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "executable", &eq);
  }
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "command", &eq);
  }
  if (eq) {
    *out_hint = CDD_CLI_HINT_EXECUTABLE;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "url", &eq);
  if (eq) {
    *out_hint = CDD_CLI_HINT_URL;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "hostname", &eq);
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "host", &eq);
  }
  if (eq) {
    *out_hint = CDD_CLI_HINT_HOSTNAME;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "username", &eq);
  if (!eq) {
    rc = cdd_serde_str_ieq(str, "user", &eq);
  }
  if (eq) {
    *out_hint = CDD_CLI_HINT_USERNAME;
    return rc;
  }

  rc = cdd_serde_str_ieq(str, "email", &eq);
  if (eq) {
    *out_hint = CDD_CLI_HINT_EMAIL;
    return rc;
  }

  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/*
 * @brief Converts an `enum CddCliValueHint` value to its canonical string
 * representation.
 */
cdd_c_error_t cdd_cli_value_hint_to_str(enum CddCliValueHint hint,
                                        const char **out_str) {
  if (!out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (hint) {
  case CDD_CLI_HINT_FILE_PATH:
    *out_str = "file";
    break;
  case CDD_CLI_HINT_DIR_PATH:
    *out_str = "dir";
    break;
  case CDD_CLI_HINT_ANY_PATH:
    *out_str = "path";
    break;
  case CDD_CLI_HINT_EXECUTABLE:
    *out_str = "executable";
    break;
  case CDD_CLI_HINT_URL:
    *out_str = "url";
    break;
  case CDD_CLI_HINT_HOSTNAME:
    *out_str = "hostname";
    break;
  case CDD_CLI_HINT_USERNAME:
    *out_str = "username";
    break;
  case CDD_CLI_HINT_EMAIL:
    *out_str = "email";
    break;
  case CDD_CLI_HINT_NONE:
  default:
    *out_str = "none";
    break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Splits an identifier string into constituent word tokens.
 *
 * @param[in] input Input identifier string.
 * @param[out] words 2D array receiving word tokens.
 * @param[out] out_count Number of words extracted.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t
serde_split_words(const char *input,
                  char words[CDD_SERDE_MAX_WORDS][CDD_SERDE_MAX_WORD_LEN],
                  size_t *out_count) {
  size_t w_idx = 0;
  size_t c_idx = 0;
  size_t i = 0;
  size_t len = strlen(input);

  for (i = 0; i < len; ++i) {
    unsigned char ch = (unsigned char)input[i];
    if (ch == '_' || ch == '-' || ch == '.' || isspace(ch)) {
      if (c_idx > 0) {
        words[w_idx][c_idx] = '\0';
        if (w_idx + 1 < CDD_SERDE_MAX_WORDS)
          w_idx++;
        c_idx = 0;
      }
      continue;
    }

    if (c_idx > 0 && isupper(ch)) {
      unsigned char prev = (unsigned char)input[i - 1];
      unsigned char next =
          (i + 1 < len) ? (unsigned char)input[i + 1] : (unsigned char)0;
      if (islower(prev) || isdigit(prev) || islower(next)) {
        words[w_idx][c_idx] = '\0';
        w_idx++;
        c_idx = 0;
      }
    }

    if (c_idx + 1 < CDD_SERDE_MAX_WORD_LEN) {
      words[w_idx][c_idx++] = (char)ch;
    }
  }

  if (c_idx > 0) {
    words[w_idx][c_idx] = '\0';
    w_idx++;
  }

  *out_count = w_idx;
  return CDD_C_SUCCESS;
}

/*
 * @brief Transforms an identifier string according to a `CddSerdeCaseRule`.
 */
cdd_c_error_t cdd_serde_transform_case(const char *input,
                                       enum CddSerdeCaseRule rule,
                                       char *out_buf, size_t out_sz) {
  char words[CDD_SERDE_MAX_WORDS][CDD_SERDE_MAX_WORD_LEN];
  size_t n_words = 0;
  size_t i, j, pos = 0;
  cdd_c_error_t rc;

  if (!input || !out_buf || out_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (rule == CDD_SERDE_CASE_NONE) {
    return cdd_serde_copy_str(out_buf, out_sz, input);
  }

  if (rule == CDD_SERDE_CASE_LOWERCASE) {
    for (i = 0; input[i] != '\0' && pos + 1 < out_sz; ++i) {
      out_buf[pos++] = (char)tolower((unsigned char)input[i]);
    }
    out_buf[pos] = '\0';
    return CDD_C_SUCCESS;
  }

  if (rule == CDD_SERDE_CASE_UPPERCASE) {
    for (i = 0; input[i] != '\0' && pos + 1 < out_sz; ++i) {
      out_buf[pos++] = (char)toupper((unsigned char)input[i]);
    }
    out_buf[pos] = '\0';
    return CDD_C_SUCCESS;
  }

  rc = serde_split_words(input, words, &n_words);

  if (n_words == 0) {
    return cdd_serde_copy_str(out_buf, out_sz, input);
  }

  for (i = 0; i < n_words; ++i) {
    if (i > 0) {
      if (rule == CDD_SERDE_CASE_SNAKECASE ||
          rule == CDD_SERDE_CASE_SCREAMING_SNAKECASE) {
        if (pos + 1 < out_sz)
          out_buf[pos++] = '_';
      } else if (rule == CDD_SERDE_CASE_KEBABCASE ||
                 rule == CDD_SERDE_CASE_SCREAMING_KEBABCASE ||
                 rule == CDD_SERDE_CASE_TRAINCASE) {
        if (pos + 1 < out_sz)
          out_buf[pos++] = '-';
      }
    }

    for (j = 0; words[i][j] != '\0' && pos + 1 < out_sz; ++j) {
      unsigned char ch = (unsigned char)words[i][j];
      if (rule == CDD_SERDE_CASE_PASCALCASE ||
          rule == CDD_SERDE_CASE_TRAINCASE) {
        out_buf[pos++] = (char)(j == 0 ? toupper(ch) : tolower(ch));
      } else if (rule == CDD_SERDE_CASE_CAMELCASE) {
        if (i == 0)
          out_buf[pos++] = (char)tolower(ch);
        else
          out_buf[pos++] = (char)(j == 0 ? toupper(ch) : tolower(ch));
      } else if (rule == CDD_SERDE_CASE_SNAKECASE ||
                 rule == CDD_SERDE_CASE_KEBABCASE) {
        out_buf[pos++] = (char)tolower(ch);
      } else {
        out_buf[pos++] = (char)toupper(ch);
      }
    }
  }

  out_buf[pos] = '\0';
  return rc;
}

/*
 * @brief Replaces all occurrences of `from_pat` with `to_pat` in `input`.
 */
cdd_c_error_t cdd_serde_replace_str(const char *input, const char *from_pat,
                                    const char *to_pat, char *out_buf,
                                    size_t out_sz) {
  size_t from_len, to_len, pos = 0;
  const char *cur;
  const char *match;

  if (!input || !out_buf || out_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!from_pat || from_pat[0] == '\0' || !to_pat) {
    return cdd_serde_copy_str(out_buf, out_sz, input);
  }

  from_len = strlen(from_pat);
  to_len = strlen(to_pat);
  cur = input;

  while ((match = strstr(cur, from_pat)) != NULL) {
    size_t prefix_len = (size_t)(match - cur);
    size_t k;
    for (k = 0; k < prefix_len && pos + 1 < out_sz; ++k) {
      out_buf[pos++] = cur[k];
    }
    for (k = 0; k < to_len && pos + 1 < out_sz; ++k) {
      out_buf[pos++] = to_pat[k];
    }
    cur = match + from_len;
  }

  while (*cur != '\0' && pos + 1 < out_sz) {
    out_buf[pos++] = *cur++;
  }
  out_buf[pos] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Computes a single direction (serialize or deserialize) wire name for
 * a struct field.
 *
 * @param[in] sf Enclosing `StructFields` container.
 * @param[in] f Field descriptor.
 * @param[in] is_serialize Non-zero for serialization name, 0 for
 * deserialization name.
 * @param[out] out_name Output buffer.
 * @param[in] out_sz Capacity of `out_name`.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t serde_resolve_single_name(const struct StructFields *sf,
                                               const struct StructField *f,
                                               int is_serialize, char *out_name,
                                               size_t out_sz) {
  char buf1[128];
  char buf2[128];
  enum CddSerdeCaseRule container_case;
  cdd_c_error_t rc;

  if (out_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* 1. Explicit directional rename has highest precedence */
  if (is_serialize && f->serde_rename_ser[0] != '\0') {
    return cdd_serde_copy_str(out_name, out_sz, f->serde_rename_ser);
  }
  if (!is_serialize && f->serde_rename_de[0] != '\0') {
    return cdd_serde_copy_str(out_name, out_sz, f->serde_rename_de);
  }

  /* 2. Explicit bilateral rename */
  if (f->serde_rename[0] != '\0') {
    return cdd_serde_copy_str(out_name, out_sz, f->serde_rename);
  }

  /* 3. Start with C identifier */
  if (f->name[0] == '\0') {
    out_name[0] = '\0';
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  rc = cdd_serde_copy_str(buf1, sizeof(buf1), f->name);

  /* 4. Apply container prefix / suffix if present */
  if (sf->serde_prefix[0] != '\0' || sf->serde_suffix[0] != '\0') {
    size_t pos = 0;
    size_t k;
    for (k = 0; sf->serde_prefix[k] != '\0'; ++k)
      buf2[pos++] = sf->serde_prefix[k];
    for (k = 0; buf1[k] != '\0'; ++k)
      buf2[pos++] = buf1[k];
    for (k = 0; sf->serde_suffix[k] != '\0' && pos + 1 < sizeof(buf2); ++k)
      buf2[pos++] = sf->serde_suffix[k];
    buf2[pos] = '\0';
    rc = cdd_serde_copy_str(buf1, sizeof(buf1), buf2);
  }

  /* 5. Container case rule vs Field-level case rule override */
  if (f->serde_case != CDD_SERDE_CASE_NONE) {
    rc = cdd_serde_transform_case(buf1, f->serde_case, buf2, sizeof(buf2));
    rc = cdd_serde_copy_str(buf1, sizeof(buf1), buf2);
  } else {
    container_case = sf->serde_rename_all;
    if (is_serialize && sf->serde_rename_all_ser != CDD_SERDE_CASE_NONE)
      container_case = sf->serde_rename_all_ser;
    else if (!is_serialize && sf->serde_rename_all_de != CDD_SERDE_CASE_NONE)
      container_case = sf->serde_rename_all_de;

    if (container_case != CDD_SERDE_CASE_NONE) {
      rc = cdd_serde_transform_case(buf1, container_case, buf2, sizeof(buf2));
      rc = cdd_serde_copy_str(buf1, sizeof(buf1), buf2);
    }
  }

  /* 6. Container replace_all */
  if (sf->serde_replace_from[0] != '\0') {
    rc = cdd_serde_replace_str(buf1, sf->serde_replace_from,
                               sf->serde_replace_to, buf2, sizeof(buf2));
    rc = cdd_serde_copy_str(buf1, sizeof(buf1), buf2);
  }

  /* 7. Field-level replace override */
  if (f->serde_replace_from[0] != '\0') {
    rc = cdd_serde_replace_str(buf1, f->serde_replace_from, f->serde_replace_to,
                               buf2, sizeof(buf2));
    rc = cdd_serde_copy_str(buf1, sizeof(buf1), buf2);
  }

  rc = cdd_serde_copy_str(out_name, out_sz, buf1);
  return rc;
}

/*
 * @brief Resolves the effective serialization (`out_ser_name`) and
 * deserialization (`out_de_name`) wire names for a struct field.
 */
cdd_c_error_t cdd_serde_resolve_field_names(const struct StructFields *sf,
                                            size_t field_idx,
                                            char *out_ser_name, size_t ser_sz,
                                            char *out_de_name, size_t de_sz) {
  const struct StructField *f;
  cdd_c_error_t rc;

  if (!sf || field_idx >= sf->size)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  f = &sf->fields[field_idx];

  if (out_ser_name) {
    rc = serde_resolve_single_name(sf, f, 1, out_ser_name, ser_sz);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (out_de_name) {
    rc = serde_resolve_single_name(sf, f, 0, out_de_name, de_sz);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Resolves the effective wire string for an enum variant.
 */
cdd_c_error_t cdd_serde_resolve_enum_member_name(const struct EnumMembers *em,
                                                 size_t member_idx,
                                                 char *out_name,
                                                 size_t out_sz) {
  char buf1[128];
  char buf2[128];
  cdd_c_error_t rc;

  if (!em || member_idx >= em->size || !out_name || out_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (em->serde_names && em->serde_names[member_idx]) {
    return cdd_serde_copy_str(out_name, out_sz, em->serde_names[member_idx]);
  }

  if (em->members[member_idx][0] == '\0') {
    out_name[0] = '\0';
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  rc = cdd_serde_copy_str(buf1, sizeof(buf1), em->members[member_idx]);

  if (em->serde_rename_all != CDD_SERDE_CASE_NONE) {
    rc = cdd_serde_transform_case(buf1, em->serde_rename_all, buf2,
                                  sizeof(buf2));
    rc = cdd_serde_copy_str(buf1, sizeof(buf1), buf2);
  }

  if (em->serde_replace_from[0] != '\0') {
    rc = cdd_serde_replace_str(buf1, em->serde_replace_from,
                               em->serde_replace_to, buf2, sizeof(buf2));
    rc = cdd_serde_copy_str(buf1, sizeof(buf1), buf2);
  }

  rc = cdd_serde_copy_str(out_name, out_sz, buf1);
  return rc;
}

/*
 * @brief Escapes a raw wire string for safe inclusion inside generated C string
 * literals (`"..."`).
 */
cdd_c_error_t cdd_escape_json_c_str(const char *raw, char *out, size_t out_sz) {
  size_t i, pos = 0;
  if (!raw || !out || out_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; raw[i] != '\0' && pos + 2 < out_sz; ++i) {
    unsigned char uc = (unsigned char)raw[i];
    if (uc == '"' || uc == '\\') {
      out[pos++] = '\\';
      out[pos++] = (char)uc;
    } else if (uc == '\b') {
      out[pos++] = '\\';
      out[pos++] = 'b';
    } else if (uc == '\f') {
      out[pos++] = '\\';
      out[pos++] = 'f';
    } else if (uc == '\n') {
      out[pos++] = '\\';
      out[pos++] = 'n';
    } else if (uc == '\r') {
      out[pos++] = '\\';
      out[pos++] = 'r';
    } else if (uc == '\t') {
      out[pos++] = '\\';
      out[pos++] = 't';
    } else if (uc < 0x20) {
      if (pos + 4 < out_sz) {
        static const char hex[] = "0123456789abcdef";
        out[pos++] = '\\';
        out[pos++] = 'x';
        out[pos++] = hex[(uc >> 4) & 0x0F];
        out[pos++] = hex[uc & 0x0F];
      }
    } else {
      out[pos++] = (char)uc;
    }
  }
  out[pos] = '\0';
  return CDD_C_SUCCESS;
}

/*
 * @brief Escapes a raw string for safe inclusion inside C89 block comment
 * directive arguments.
 */
cdd_c_error_t cdd_escape_comment_str(const char *raw, char *out,
                                     size_t out_sz) {
  size_t i, pos = 0;
  if (!raw || !out || out_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; raw[i] != '\0' && pos + 2 < out_sz; ++i) {
    unsigned char uc = (unsigned char)raw[i];
    if (uc == '"' || uc == '\\') {
      out[pos++] = '\\';
      out[pos++] = (char)uc;
    } else if (uc == '*' && raw[i + 1] == '/') {
      out[pos++] = '*';
      out[pos++] = '\\';
      out[pos++] = '/';
      i++;
    } else if (uc == '\n') {
      out[pos++] = '\\';
      out[pos++] = 'n';
    } else if (uc == '\r') {
      out[pos++] = '\\';
      out[pos++] = 'r';
    } else if (uc == '\t') {
      out[pos++] = '\\';
      out[pos++] = 't';
    } else {
      out[pos++] = (char)uc;
    }
  }
  out[pos] = '\0';
  return CDD_C_SUCCESS;
}
