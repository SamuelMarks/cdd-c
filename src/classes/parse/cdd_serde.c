/**
 * @file cdd_serde.c
 * @brief Implementation of Serde-style C89 struct and enum reflection, case
 * conversion, string rewriting, and comment directive parser.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "classes/parse/cdd_serde.h"
#include "c_cdd/memory.h"
#include "functions/parse/str.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief Maximum number of identifier words split during case conversion */
#define CDD_SERDE_MAX_WORDS 32

/** @brief Maximum length of a single identifier word */
#define CDD_SERDE_MAX_WORD_LEN 64

/**
 * @brief Safely copies a string into a fixed-size destination buffer with
 * truncation.
 *
 * @param[out] dst Destination buffer.
 * @param[in] dst_sz Capacity of `dst` in bytes.
 * @param[in] src Source null-terminated string.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_copy_str(char *dst, size_t dst_sz, const char *src) {
  if (dst_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  strncpy_s(dst, dst_sz, src, _TRUNCATE);
#else
  strncpy(dst, src, dst_sz - 1);
  dst[dst_sz - 1] = '\0';
#endif
  return CDD_C_SUCCESS;
}

/**
 * @brief Case-insensitive ASCII string comparison helper.
 *
 * @param[in] a First string.
 * @param[in] b Second string.
 * @param[out] out_eq Output flag set to 1 if equal, 0 otherwise.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_str_ieq(const char *a, const char *b, int *out_eq) {
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
 * @brief Parses a case transformation rule string into `enum CddSerdeCaseRule`.
 */
cdd_c_error_t cdd_serde_parse_case_rule(const char *str,
                                        enum CddSerdeCaseRule *out_rule) {
  int eq = 0;
  if (!str || !out_rule)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  serde_str_ieq(str, "none", &eq);
  if (eq || str[0] == '\0') {
    *out_rule = CDD_SERDE_CASE_NONE;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "lowercase", &eq);
  if (eq) {
    *out_rule = CDD_SERDE_CASE_LOWERCASE;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "uppercase", &eq);
  if (eq) {
    *out_rule = CDD_SERDE_CASE_UPPERCASE;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "pascalcase", &eq);
  if (eq) {
    *out_rule = CDD_SERDE_CASE_PASCALCASE;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "camelcase", &eq);
  if (eq) {
    *out_rule = CDD_SERDE_CASE_CAMELCASE;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "snake_case", &eq);
  if (!eq) {
    serde_str_ieq(str, "snakecase", &eq);
  }
  if (eq) {
    *out_rule = CDD_SERDE_CASE_SNAKECASE;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "screaming_snake_case", &eq);
  if (!eq) {
    serde_str_ieq(str, "screaming_snakecase", &eq);
  }
  if (eq) {
    *out_rule = CDD_SERDE_CASE_SCREAMING_SNAKECASE;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "kebab-case", &eq);
  if (!eq) {
    serde_str_ieq(str, "kebabcase", &eq);
  }
  if (!eq) {
    serde_str_ieq(str, "kebab_case", &eq);
  }
  if (eq) {
    *out_rule = CDD_SERDE_CASE_KEBABCASE;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "screaming-kebab-case", &eq);
  if (!eq) {
    serde_str_ieq(str, "screaming_kebabcase", &eq);
  }
  if (!eq) {
    serde_str_ieq(str, "screaming_kebab_case", &eq);
  }
  if (eq) {
    *out_rule = CDD_SERDE_CASE_SCREAMING_KEBABCASE;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "train-case", &eq);
  if (!eq) {
    serde_str_ieq(str, "traincase", &eq);
  }
  if (!eq) {
    serde_str_ieq(str, "train_case", &eq);
  }
  if (eq) {
    *out_rule = CDD_SERDE_CASE_TRAINCASE;
    return CDD_C_SUCCESS;
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
  if (!str || !out_action)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  serde_str_ieq(str, "auto", &eq);
  if (eq || str[0] == '\0') {
    *out_action = CDD_CLI_ACTION_AUTO;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "set", &eq);
  if (eq) {
    *out_action = CDD_CLI_ACTION_SET;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "set_true", &eq);
  if (!eq) {
    serde_str_ieq(str, "settrue", &eq);
  }
  if (!eq) {
    serde_str_ieq(str, "true", &eq);
  }
  if (eq) {
    *out_action = CDD_CLI_ACTION_SET_TRUE;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "set_false", &eq);
  if (!eq) {
    serde_str_ieq(str, "setfalse", &eq);
  }
  if (!eq) {
    serde_str_ieq(str, "false", &eq);
  }
  if (eq) {
    *out_action = CDD_CLI_ACTION_SET_FALSE;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "count", &eq);
  if (eq) {
    *out_action = CDD_CLI_ACTION_COUNT;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "append", &eq);
  if (eq) {
    *out_action = CDD_CLI_ACTION_APPEND;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "help", &eq);
  if (eq) {
    *out_action = CDD_CLI_ACTION_HELP;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "version", &eq);
  if (eq) {
    *out_action = CDD_CLI_ACTION_VERSION;
    return CDD_C_SUCCESS;
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
  if (!str || !out_hint)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  serde_str_ieq(str, "none", &eq);
  if (eq || str[0] == '\0') {
    *out_hint = CDD_CLI_HINT_NONE;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "file", &eq);
  if (!eq) {
    serde_str_ieq(str, "file_path", &eq);
  }
  if (!eq) {
    serde_str_ieq(str, "filepath", &eq);
  }
  if (eq) {
    *out_hint = CDD_CLI_HINT_FILE_PATH;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "dir", &eq);
  if (!eq) {
    serde_str_ieq(str, "dir_path", &eq);
  }
  if (!eq) {
    serde_str_ieq(str, "dirpath", &eq);
  }
  if (eq) {
    *out_hint = CDD_CLI_HINT_DIR_PATH;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "path", &eq);
  if (!eq) {
    serde_str_ieq(str, "any_path", &eq);
  }
  if (!eq) {
    serde_str_ieq(str, "anypath", &eq);
  }
  if (eq) {
    *out_hint = CDD_CLI_HINT_ANY_PATH;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "cmd", &eq);
  if (!eq) {
    serde_str_ieq(str, "executable", &eq);
  }
  if (!eq) {
    serde_str_ieq(str, "command", &eq);
  }
  if (eq) {
    *out_hint = CDD_CLI_HINT_EXECUTABLE;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "url", &eq);
  if (eq) {
    *out_hint = CDD_CLI_HINT_URL;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "hostname", &eq);
  if (!eq) {
    serde_str_ieq(str, "host", &eq);
  }
  if (eq) {
    *out_hint = CDD_CLI_HINT_HOSTNAME;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "username", &eq);
  if (!eq) {
    serde_str_ieq(str, "user", &eq);
  }
  if (eq) {
    *out_hint = CDD_CLI_HINT_USERNAME;
    return CDD_C_SUCCESS;
  }

  serde_str_ieq(str, "email", &eq);
  if (eq) {
    *out_hint = CDD_CLI_HINT_EMAIL;
    return CDD_C_SUCCESS;
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

  if (!input || !out_buf || out_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (rule == CDD_SERDE_CASE_NONE) {
    return serde_copy_str(out_buf, out_sz, input);
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

  serde_split_words(input, words, &n_words);

  if (n_words == 0) {
    return serde_copy_str(out_buf, out_sz, input);
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
  return CDD_C_SUCCESS;
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
    return serde_copy_str(out_buf, out_sz, input);
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

  /* 1. Explicit directional rename has highest precedence */
  if (is_serialize && f->serde_rename_ser[0] != '\0') {
    return serde_copy_str(out_name, out_sz, f->serde_rename_ser);
  }
  if (!is_serialize && f->serde_rename_de[0] != '\0') {
    return serde_copy_str(out_name, out_sz, f->serde_rename_de);
  }

  /* 2. Explicit bilateral rename */
  if (f->serde_rename[0] != '\0') {
    return serde_copy_str(out_name, out_sz, f->serde_rename);
  }

  /* 3. Start with C identifier */
  if (f->name[0] == '\0') {
    out_name[0] = '\0';
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  serde_copy_str(buf1, sizeof(buf1), f->name);

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
    serde_copy_str(buf1, sizeof(buf1), buf2);
  }

  /* 5. Container case rule vs Field-level case rule override */
  if (f->serde_case != CDD_SERDE_CASE_NONE) {
    cdd_serde_transform_case(buf1, f->serde_case, buf2, sizeof(buf2));
    serde_copy_str(buf1, sizeof(buf1), buf2);
  } else {
    container_case = sf->serde_rename_all;
    if (is_serialize && sf->serde_rename_all_ser != CDD_SERDE_CASE_NONE)
      container_case = sf->serde_rename_all_ser;
    else if (!is_serialize && sf->serde_rename_all_de != CDD_SERDE_CASE_NONE)
      container_case = sf->serde_rename_all_de;

    if (container_case != CDD_SERDE_CASE_NONE) {
      cdd_serde_transform_case(buf1, container_case, buf2, sizeof(buf2));
      serde_copy_str(buf1, sizeof(buf1), buf2);
    }
  }

  /* 6. Container replace_all */
  if (sf->serde_replace_from[0] != '\0') {
    cdd_serde_replace_str(buf1, sf->serde_replace_from, sf->serde_replace_to,
                          buf2, sizeof(buf2));
    serde_copy_str(buf1, sizeof(buf1), buf2);
  }

  /* 7. Field-level replace override */
  if (f->serde_replace_from[0] != '\0') {
    cdd_serde_replace_str(buf1, f->serde_replace_from, f->serde_replace_to,
                          buf2, sizeof(buf2));
    serde_copy_str(buf1, sizeof(buf1), buf2);
  }

  return serde_copy_str(out_name, out_sz, buf1);
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

  if (!em || member_idx >= em->size || !out_name || out_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (em->serde_names && em->serde_names[member_idx]) {
    return serde_copy_str(out_name, out_sz, em->serde_names[member_idx]);
  }

  if (em->members[member_idx][0] == '\0') {
    out_name[0] = '\0';
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  serde_copy_str(buf1, sizeof(buf1), em->members[member_idx]);

  if (em->serde_rename_all != CDD_SERDE_CASE_NONE) {
    cdd_serde_transform_case(buf1, em->serde_rename_all, buf2, sizeof(buf2));
    serde_copy_str(buf1, sizeof(buf1), buf2);
  }

  if (em->serde_replace_from[0] != '\0') {
    cdd_serde_replace_str(buf1, em->serde_replace_from, em->serde_replace_to,
                          buf2, sizeof(buf2));
    serde_copy_str(buf1, sizeof(buf1), buf2);
  }

  return serde_copy_str(out_name, out_sz, buf1);
}

/**
 * @brief Advances pointer past whitespace, commas, and comment asterisks.
 *
 * @param[in,out] p Pointer to current character pointer.
 * @param[in] end End of slice.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_skip_delims(const char **p, const char *end) {
  const char *cur = *p;
  while (cur < end &&
         (isspace((unsigned char)*cur) || *cur == ',' || *cur == '*')) {
    cur++;
  }
  *p = cur;
  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts a quoted string or bare identifier value starting at `*p`.
 *
 * @param[in,out] p Pointer to current character pointer.
 * @param[in] end End of slice.
 * @param[out] out_val Buffer receiving the extracted string.
 * @param[in] val_sz Capacity of `out_val`.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_parse_value(const char **p, const char *end,
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
      if (ch == '\\' && cur < end) {
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
      out_val[pos++] = ch;
    }
    if (cur < end)
      cur++;
  } else {
    while (cur < end && !isspace((unsigned char)*cur) && *cur != ',' &&
           *cur != ')') {
      out_val[pos++] = *cur++;
    }
  }

  if (paren_wrapped) {
    while (cur < end && (isspace((unsigned char)*cur) || *cur == ')'))
      cur++;
  }

  out_val[pos] = '\0';
  *p = cur;
  (void)val_sz;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a replacement tuple `("<from>" -> "<to>")` or
 * `("<from>", "<to>")` starting at `*p`.
 *
 * @param[in,out] p Pointer to current character pointer.
 * @param[in] end End of slice.
 * @param[out] out_from Buffer for source pattern.
 * @param[in] from_sz Capacity of `out_from`.
 * @param[out] out_to Buffer for target pattern.
 * @param[in] to_sz Capacity of `out_to`.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_parse_replace_tuple(const char **p, const char *end,
                                               char *out_from, size_t from_sz,
                                               char *out_to, size_t to_sz) {
  const char *cur = *p;

  while (isspace((unsigned char)*cur) || *cur == '(')
    cur++;

  serde_parse_value(&cur, end, out_from, from_sz);

  while (cur < end && (isspace((unsigned char)*cur) || *cur == ',' ||
                       *cur == '-' || *cur == '>' || *cur == ':')) {
    cur++;
  }

  serde_parse_value(&cur, end, out_to, to_sz);

  while (cur < end && (isspace((unsigned char)*cur) || *cur == ')'))
    cur++;

  *p = cur;
  return CDD_C_SUCCESS;
}

/**
 * @brief Locates the next `@cdd`, `@serde`, `@cli`, or `#[...( ` directive
 * block in a comment string.
 *
 * @param[in] cur Current scan pointer.
 * @param[out] out_start Start of directive arguments.
 * @param[out] out_end End of directive block.
 * @param[out] out_next Pointer to resume scanning after this block.
 * @param[out] out_is_cli Set to 1 if directive is explicitly CLI-scoped
 * (`@cli`, `#[command(`, `#[arg(`, `#[clap(`, `#[structopt(`), 0 otherwise.
 * @return CDD_C_SUCCESS on success (sets `*out_start = NULL` if none found).
 */
static cdd_c_error_t serde_find_directive_ex(const char *cur,
                                             const char **out_start,
                                             const char **out_end,
                                             const char **out_next,
                                             int *out_is_cli) {
  const char *p = cur;
  if (out_is_cli)
    *out_is_cli = 0;
  while (*p != '\0') {
    int is_cdd = (strncmp(p, "@cdd", 4) == 0);
    int is_serde = (strncmp(p, "@serde", 6) == 0);
    int is_cli = (strncmp(p, "@cli", 4) == 0);
    if (is_cdd || is_serde || is_cli) {
      const char *body = p + (is_serde ? 6 : 4);
      const char *end;
      int paren_depth = 0;
      char in_quote = 0;
      if (out_is_cli)
        *out_is_cli = is_cli ? 1 : 0;
      if (*body == '(') {
        paren_depth = 1;
        body++;
      }
      end = body;
      while (*end != '\0') {
        if (in_quote) {
          if (*end == '\\' && end[1] != '\0') {
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
            if (paren_depth == 0 && *(p + (is_serde ? 6 : 4)) == '(') {
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
      size_t prefix_len = 0;
      int cli_attr = 0;
      if (strncmp(p, "#[serde(", 8) == 0 || strncmp(p, "[[serde(", 8) == 0) {
        prefix_len = 8;
        cli_attr = 0;
      } else if (strncmp(p, "[[cdd(", 6) == 0) {
        prefix_len = 6;
        cli_attr = 0;
      } else if (strncmp(p, "[[cli(", 6) == 0) {
        prefix_len = 6;
        cli_attr = 1;
      } else if (strncmp(p, "#[command(", 10) == 0) {
        prefix_len = 10;
        cli_attr = 1;
      } else if (strncmp(p, "#[clap(", 7) == 0) {
        prefix_len = 7;
        cli_attr = 1;
      } else if (strncmp(p, "#[structopt(", 12) == 0) {
        prefix_len = 12;
        cli_attr = 1;
      } else if (strncmp(p, "#[arg(", 6) == 0) {
        prefix_len = 6;
        cli_attr = 1;
      }
      if (prefix_len > 0) {
        const char *body = p + prefix_len;
        const char *end = body;
        int depth = 1;
        char in_quote = 0;
        if (out_is_cli)
          *out_is_cli = cli_attr;
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
        *out_next = (*end == ')') ? end + 1 : end;
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

/**
 * @brief Extracts the next directive key identifier within `[ *p, end )`.
 *
 * @param[in,out] p Pointer to current scan position.
 * @param[in] end End of directive slice.
 * @param[out] key_buf Buffer receiving the key identifier.
 * @param[in] key_sz Capacity of `key_buf`.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_next_key(const char **p, const char *end,
                                    char *key_buf, size_t key_sz) {
  const char *cur;
  size_t pos = 0;
  (void)key_sz;

  serde_skip_delims(p, end);
  cur = *p;
  while (cur < end && *cur == ')') {
    cur++;
    *p = cur;
    serde_skip_delims(p, end);
    cur = *p;
  }
  if (strncmp(cur, "cdd::", 5) == 0 || strncmp(cur, "cli::", 5) == 0) {
    cur += 5;
  } else if (strncmp(cur, "serde::", 7) == 0) {
    cur += 7;
  }

  while (cur < end &&
         (isalnum((unsigned char)*cur) || *cur == '_' || *cur == '-')) {
    key_buf[pos++] = *cur++;
  }
  key_buf[pos] = '\0';
  *p = cur;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a boolean string (`"true"` / `"false"` / `"1"` / `"0"`) into
 * `0` or `1`.
 *
 * @param[in] val String value.
 * @param[out] out_bool Output integer (`0` or `1`).
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_parse_bool_val(const char *val, int *out_bool) {
  int eq_false = 0, eq_no = 0, eq_off = 0;
  serde_str_ieq(val, "false", &eq_false);
  serde_str_ieq(val, "no", &eq_no);
  serde_str_ieq(val, "off", &eq_off);
  if (eq_false || eq_no || eq_off || strcmp(val, "0") == 0) {
    *out_bool = 0;
  } else {
    *out_bool = 1;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a `clap` `num_args` specification (e.g. `"2"`, `"1..=3"`,
 * `"1..4"`, `"1.."`, `"..=3"`, `".."`).
 *
 * @param[in] val Input `num_args` string.
 * @param[out] out_min Minimum number of arguments.
 * @param[out] out_max Maximum number of arguments (`-1` for unbounded).
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_parse_num_args(const char *val, int *out_min,
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
  (void)out_sz;
  if (*p == ':')
    p++;
  while (*p == ' ' || *p == '\t')
    p++;
  while (*p != '\0' && *p != '\r' && *p != '\n' && strncmp(p, "*/", 2) != 0) {
    out_buf[pos++] = *p++;
  }
  while (pos > 0 && isspace((unsigned char)out_buf[pos - 1]))
    pos--;
  out_buf[pos] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Scans `comment_text` for Doxygen metadata (`@brief`, `@summary`,
 * `@details`, `@description`, `@version`, `@infoVersion`, `@author`,
 * `@contact`) and plain Doxygen docstrings.
 *
 * @param[in] comment_text Raw comment block.
 * @param[out] out_brief Buffer for brief/summary text.
 * @param[in] brief_sz Capacity of `out_brief`.
 * @param[out] out_details Buffer for details/description text.
 * @param[in] details_sz Capacity of `out_details`.
 * @param[out] out_version Buffer for version text (optional).
 * @param[in] version_sz Capacity of `out_version`.
 * @param[out] out_author Buffer for author text (optional).
 * @param[in] author_sz Capacity of `out_author`.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_extract_doxygen_fallbacks(
    const char *comment_text, char *out_brief, size_t brief_sz,
    char *out_details, size_t details_sz, char *out_version, size_t version_sz,
    char *out_author, size_t author_sz) {
  const char *p = comment_text;
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
        serde_extract_tag_line(p + 6, out_brief, brief_sz);
      } else if (out_brief[0] == '\0' && strncmp(p, "@summary", 8) == 0) {
        serde_extract_tag_line(p + 8, out_brief, brief_sz);
      } else if (out_details[0] == '\0' && strncmp(p, "@details", 8) == 0) {
        serde_extract_tag_line(p + 8, out_details, details_sz);
      } else if (out_details[0] == '\0' &&
                 strncmp(p, "@description", 12) == 0) {
        serde_extract_tag_line(p + 12, out_details, details_sz);
      } else if (out_version && out_version[0] == '\0' &&
                 strncmp(p, "@version", 8) == 0) {
        serde_extract_tag_line(p + 8, out_version, version_sz);
      } else if (out_version && out_version[0] == '\0' &&
                 strncmp(p, "@infoVersion", 12) == 0) {
        serde_extract_tag_line(p + 12, out_version, version_sz);
      } else if (out_author && out_author[0] == '\0' &&
                 strncmp(p, "@author", 7) == 0) {
        serde_extract_tag_line(p + 7, out_author, author_sz);
      } else if (out_author && out_author[0] == '\0' &&
                 strncmp(p, "@contact", 8) == 0) {
        serde_extract_tag_line(p + 8, out_author, author_sz);
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
        serde_extract_tag_line(s, out_brief, brief_sz);
        break;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Parses container-level `@cdd` or `@serde`
 * directives into a `StructFields` container.
 */
cdd_c_error_t cdd_serde_parse_container_comment(const char *comment_text,
                                                struct StructFields *sf) {
  const char *scan;
  const char *start = NULL;
  const char *end = NULL;
  const char *next = NULL;
  char dox_brief[256];
  char dox_details[512];
  char dox_version[64];
  char dox_author[128];
  cdd_c_error_t rc;

  if (!sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!comment_text || comment_text[0] == '\0')
    return CDD_C_SUCCESS;

  serde_extract_doxygen_fallbacks(comment_text, dox_brief, sizeof(dox_brief),
                                  dox_details, sizeof(dox_details), dox_version,
                                  sizeof(dox_version), dox_author,
                                  sizeof(dox_author));
  if (dox_brief[0] != '\0' && sf->cli_about[0] == '\0') {
    serde_copy_str(sf->cli_about, sizeof(sf->cli_about), dox_brief);
  }
  if (dox_details[0] != '\0' && !sf->cli_long_about) {
    rc = c_cdd_strdup(dox_details, &sf->cli_long_about);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (dox_version[0] != '\0' && sf->cli_version[0] == '\0') {
    serde_copy_str(sf->cli_version, sizeof(sf->cli_version), dox_version);
  }
  if (dox_author[0] != '\0' && sf->cli_author[0] == '\0') {
    serde_copy_str(sf->cli_author, sizeof(sf->cli_author), dox_author);
  }

  scan = comment_text;
  while (1) {
    const char *p;
    int is_cli_dir = 0;
    int in_cli_ctx = 0;
    serde_find_directive_ex(scan, &start, &end, &next, &is_cli_dir);
    if (!start)
      break;

    if (is_cli_dir) {
      sf->is_cli_command = 1;
      in_cli_ctx = 1;
    }

    p = start;
    while (p < end) {
      char key[64];
      char val[1024];
      enum CddSerdeCaseRule rule = CDD_SERDE_CASE_NONE;

      serde_next_key(&p, end, key, sizeof(key));
      if (key[0] == '\0') {
        if (p < end)
          p++;
        continue;
      }

      while (p < end && isspace((unsigned char)*p))
        p++;

      if (strcmp(key, "cli") == 0 || strcmp(key, "to_cli") == 0 ||
          strcmp(key, "from_cli") == 0 || strcmp(key, "to_json") == 0 ||
          strcmp(key, "from_json") == 0 || strcmp(key, "command") == 0 ||
          strcmp(key, "clap") == 0 || strcmp(key, "structopt") == 0) {
        if (strcmp(key, "to_json") != 0 && strcmp(key, "from_json") != 0) {
          sf->is_cli_command = 1;
          in_cli_ctx = 1;
        } else {
          in_cli_ctx = 0;
        }
        if (p < end && *p == '(')
          p++;
        continue;
      }

      if (p < end && (*p == '=' || *p == ':' || *p == '(')) {
        int bval = 1;
        if (*p != '(')
          p++;
        if (strcmp(key, "replace_all") == 0 || strcmp(key, "replace") == 0 ||
            strcmp(key, "rewrite_all") == 0 || strcmp(key, "rewrite") == 0) {
          serde_parse_replace_tuple(
              &p, end, sf->serde_replace_from, sizeof(sf->serde_replace_from),
              sf->serde_replace_to, sizeof(sf->serde_replace_to));
          continue;
        }
        serde_parse_value(&p, end, val, sizeof(val));

        if (strcmp(key, "rename") == 0) {
          serde_copy_str(sf->serde_rename, sizeof(sf->serde_rename), val);
        } else if (strcmp(key, "name") == 0) {
          sf->is_cli_command = 1;
          serde_copy_str(sf->cli_name, sizeof(sf->cli_name), val);
        } else if (strcmp(key, "bin_name") == 0) {
          sf->is_cli_command = 1;
          serde_copy_str(sf->cli_bin_name, sizeof(sf->cli_bin_name), val);
        } else if (strcmp(key, "version") == 0) {
          sf->is_cli_command = 1;
          serde_copy_str(sf->cli_version, sizeof(sf->cli_version), val);
        } else if (strcmp(key, "author") == 0) {
          sf->is_cli_command = 1;
          serde_copy_str(sf->cli_author, sizeof(sf->cli_author), val);
        } else if (strcmp(key, "about") == 0) {
          sf->is_cli_command = 1;
          serde_copy_str(sf->cli_about, sizeof(sf->cli_about), val);
        } else if (strcmp(key, "long_about") == 0) {
          sf->is_cli_command = 1;
          if (sf->cli_long_about) {
            C_CDD_FREE(sf->cli_long_about);
            sf->cli_long_about = NULL;
          }
          rc = c_cdd_strdup(val, &sf->cli_long_about);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "before_help") == 0) {
          sf->is_cli_command = 1;
          if (sf->cli_before_help) {
            C_CDD_FREE(sf->cli_before_help);
            sf->cli_before_help = NULL;
          }
          rc = c_cdd_strdup(val, &sf->cli_before_help);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "after_help") == 0) {
          sf->is_cli_command = 1;
          if (sf->cli_after_help) {
            C_CDD_FREE(sf->cli_after_help);
            sf->cli_after_help = NULL;
          }
          rc = c_cdd_strdup(val, &sf->cli_after_help);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "override_usage") == 0 ||
                   strcmp(key, "usage") == 0) {
          sf->is_cli_command = 1;
          serde_copy_str(sf->cli_usage_override, sizeof(sf->cli_usage_override),
                         val);
        } else if (strcmp(key, "cli_rename_all") == 0) {
          if (cdd_serde_parse_case_rule(val, &rule) == CDD_C_SUCCESS) {
            sf->cli_rename_all = rule;
          }
        } else if (strcmp(key, "rename_all") == 0 || strcmp(key, "case") == 0) {
          if (cdd_serde_parse_case_rule(val, &rule) == CDD_C_SUCCESS) {
            if (in_cli_ctx) {
              sf->cli_rename_all = rule;
            } else {
              sf->serde_rename_all = rule;
            }
          }
        } else if (strcmp(key, "rename_all_env") == 0) {
          if (cdd_serde_parse_case_rule(val, &rule) == CDD_C_SUCCESS) {
            sf->cli_rename_all_env = rule;
          }
        } else if (strcmp(key, "env_prefix") == 0) {
          sf->is_cli_command = 1;
          serde_copy_str(sf->cli_env_prefix, sizeof(sf->cli_env_prefix), val);
        } else if (strcmp(key, "alias") == 0 ||
                   strcmp(key, "visible_alias") == 0) {
          rc = struct_fields_add_cli_alias(sf, val);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "arg_required_else_help") == 0) {
          serde_parse_bool_val(val, &bval);
          sf->cli_arg_required_else_help = bval;
        } else if (strcmp(key, "subcommand_required") == 0) {
          serde_parse_bool_val(val, &bval);
          sf->cli_subcommand_required = bval;
        } else if (strcmp(key, "allow_external_subcommands") == 0) {
          serde_parse_bool_val(val, &bval);
          sf->cli_allow_external_subcommands = bval;
        } else if (strcmp(key, "disable_help_flag") == 0) {
          serde_parse_bool_val(val, &bval);
          sf->cli_disable_help_flag = bval;
        } else if (strcmp(key, "disable_version_flag") == 0) {
          serde_parse_bool_val(val, &bval);
          sf->cli_disable_version_flag = bval;
        } else if (strcmp(key, "disable_help_subcommand") == 0) {
          serde_parse_bool_val(val, &bval);
          sf->cli_disable_help_subcommand = bval;
        } else if (strcmp(key, "propagate_version") == 0) {
          serde_parse_bool_val(val, &bval);
          sf->cli_propagate_version = bval;
        } else if (strcmp(key, "allow_windows_slash_flags") == 0) {
          serde_parse_bool_val(val, &bval);
          sf->cli_allow_windows_slash_flags = bval;
        } else if (strcmp(key, "allow_response_files") == 0) {
          serde_parse_bool_val(val, &bval);
          sf->cli_allow_response_files = bval;
        } else if (strcmp(key, "case_insensitive") == 0 ||
                   strcmp(key, "ignore_case") == 0) {
          serde_parse_bool_val(val, &bval);
          sf->cli_case_insensitive_flags = bval;
        } else if (strcmp(key, "rename_all_serialize") == 0 ||
                   strcmp(key, "serialize") == 0) {
          if (cdd_serde_parse_case_rule(val, &rule) == CDD_C_SUCCESS) {
            sf->serde_rename_all_ser = rule;
          }
        } else if (strcmp(key, "rename_all_deserialize") == 0 ||
                   strcmp(key, "deserialize") == 0) {
          if (cdd_serde_parse_case_rule(val, &rule) == CDD_C_SUCCESS) {
            sf->serde_rename_all_de = rule;
          }
        } else if (strcmp(key, "prefix") == 0) {
          serde_copy_str(sf->serde_prefix, sizeof(sf->serde_prefix), val);
        } else if (strcmp(key, "suffix") == 0) {
          serde_copy_str(sf->serde_suffix, sizeof(sf->serde_suffix), val);
        } else if (strcmp(key, "tag") == 0) {
          serde_copy_str(sf->serde_tag, sizeof(sf->serde_tag), val);
        } else if (strcmp(key, "content") == 0) {
          serde_copy_str(sf->serde_content, sizeof(sf->serde_content), val);
        }
      } else {
        if (strcmp(key, "deny_unknown_fields") == 0) {
          sf->serde_deny_unknown_fields = 1;
        } else if (strcmp(key, "default") == 0) {
          sf->serde_default = 1;
        } else if (strcmp(key, "untagged") == 0) {
          sf->serde_untagged = 1;
        } else if (strcmp(key, "version") == 0) {
          sf->is_cli_command = 1;
          if (sf->cli_version[0] == '\0') {
            serde_copy_str(sf->cli_version, sizeof(sf->cli_version), "0.1.0");
          }
        } else if (strcmp(key, "author") == 0 || strcmp(key, "about") == 0 ||
                   strcmp(key, "long_about") == 0) {
          sf->is_cli_command = 1;
        } else if (strcmp(key, "arg_required_else_help") == 0) {
          sf->is_cli_command = 1;
          sf->cli_arg_required_else_help = 1;
        } else if (strcmp(key, "subcommand_required") == 0) {
          sf->is_cli_command = 1;
          sf->cli_subcommand_required = 1;
        } else if (strcmp(key, "allow_external_subcommands") == 0) {
          sf->is_cli_command = 1;
          sf->cli_allow_external_subcommands = 1;
        } else if (strcmp(key, "disable_help_flag") == 0) {
          sf->is_cli_command = 1;
          sf->cli_disable_help_flag = 1;
        } else if (strcmp(key, "disable_version_flag") == 0) {
          sf->is_cli_command = 1;
          sf->cli_disable_version_flag = 1;
        } else if (strcmp(key, "disable_help_subcommand") == 0) {
          sf->is_cli_command = 1;
          sf->cli_disable_help_subcommand = 1;
        } else if (strcmp(key, "propagate_version") == 0) {
          sf->is_cli_command = 1;
          sf->cli_propagate_version = 1;
        } else if (strcmp(key, "allow_windows_slash_flags") == 0) {
          sf->is_cli_command = 1;
          sf->cli_allow_windows_slash_flags = 1;
        } else if (strcmp(key, "allow_response_files") == 0) {
          sf->is_cli_command = 1;
          sf->cli_allow_response_files = 1;
        } else if (strcmp(key, "case_insensitive") == 0 ||
                   strcmp(key, "ignore_case") == 0) {
          sf->cli_case_insensitive_flags = 1;
        } else if (cdd_serde_parse_case_rule(key, &rule) == CDD_C_SUCCESS &&
                   rule != CDD_SERDE_CASE_NONE) {
          if (in_cli_ctx)
            sf->cli_rename_all = rule;
          else
            sf->serde_rename_all = rule;
        }
      }
    }

    scan = next;
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Parses container-level `@cdd` or `@serde`
 * directives into an `EnumMembers` container.
 */
cdd_c_error_t cdd_serde_parse_enum_comment(const char *comment_text,
                                           struct EnumMembers *em) {
  struct StructFields tmp_sf;
  cdd_c_error_t rc;
  if (!em)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!comment_text || comment_text[0] == '\0')
    return CDD_C_SUCCESS;

  memset(&tmp_sf, 0, sizeof(tmp_sf));
  rc = cdd_serde_parse_container_comment(comment_text, &tmp_sf);
  if (rc != CDD_C_SUCCESS) {
    struct_fields_free(&tmp_sf);
    return rc;
  }

  if (tmp_sf.serde_rename_all != CDD_SERDE_CASE_NONE)
    em->serde_rename_all = tmp_sf.serde_rename_all;
  if (tmp_sf.cli_rename_all != CDD_SERDE_CASE_NONE)
    em->cli_rename_all = tmp_sf.cli_rename_all;
  if (tmp_sf.cli_case_insensitive_flags)
    em->cli_case_insensitive = 1;
  if (tmp_sf.serde_replace_from[0] != '\0') {
    serde_copy_str(em->serde_replace_from, sizeof(em->serde_replace_from),
                   tmp_sf.serde_replace_from);
    serde_copy_str(em->serde_replace_to, sizeof(em->serde_replace_to),
                   tmp_sf.serde_replace_to);
  }
  if (tmp_sf.serde_tag[0] != '\0') {
    serde_copy_str(em->serde_tag, sizeof(em->serde_tag), tmp_sf.serde_tag);
  }
  if (tmp_sf.serde_content[0] != '\0') {
    serde_copy_str(em->serde_content, sizeof(em->serde_content),
                   tmp_sf.serde_content);
  }
  if (tmp_sf.serde_untagged)
    em->serde_untagged = 1;
  struct_fields_free(&tmp_sf);
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends a short option alias character to `field->cli_short_aliases`.
 *
 * @param[in,out] field Target StructField.
 * @param[in] ch Short flag character to append.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_add_short_alias_char(struct StructField *field,
                                                char ch) {
  size_t len = strlen(field->cli_short_aliases);
  size_t i;
  for (i = 0; i < len; ++i) {
    if (field->cli_short_aliases[i] == ch)
      return CDD_C_SUCCESS;
  }
  if (len + 1 < sizeof(field->cli_short_aliases)) {
    field->cli_short_aliases[len] = ch;
    field->cli_short_aliases[len + 1] = '\0';
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Infers a default short option character from a field name.
 *
 * @param[in] field_name C field identifier.
 * @param[out] out_ch Output pointer receiving the inferred character.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_infer_short_char(const char *field_name,
                                            char *out_ch) {
  const char *p = field_name;
  while (*p == '_')
    p++;
  *out_ch = (*p != '\0') ? *p : 'a';
  return CDD_C_SUCCESS;
}

/*
 * @brief Parses field-level `@cdd` or `@serde` directives
 * into a `StructField` descriptor.
 */
cdd_c_error_t cdd_serde_parse_field_comment(const char *comment_text,
                                            struct StructField *field) {
  const char *scan;
  const char *start = NULL;
  const char *end = NULL;
  const char *next = NULL;
  char dox_brief[256];
  char dox_details[512];
  cdd_c_error_t rc;

  if (!field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!comment_text || comment_text[0] == '\0')
    return CDD_C_SUCCESS;

  serde_extract_doxygen_fallbacks(comment_text, dox_brief, sizeof(dox_brief),
                                  dox_details, sizeof(dox_details), NULL, 0,
                                  NULL, 0);
  if (dox_brief[0] != '\0') {
    if (field->description[0] == '\0') {
      serde_copy_str(field->description, sizeof(field->description), dox_brief);
    }
    if (field->cli_help[0] == '\0') {
      serde_copy_str(field->cli_help, sizeof(field->cli_help), dox_brief);
    }
  }
  if (dox_details[0] != '\0' && !field->cli_long_help) {
    rc = c_cdd_strdup(dox_details, &field->cli_long_help);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  scan = comment_text;
  while (1) {
    const char *p;
    int is_cli_dir = 0;
    serde_find_directive_ex(scan, &start, &end, &next, &is_cli_dir);
    if (!start)
      break;

    p = start;
    while (p < end) {
      char key[64];
      char val[1024];
      enum CddSerdeCaseRule rule = CDD_SERDE_CASE_NONE;

      serde_next_key(&p, end, key, sizeof(key));
      if (key[0] == '\0') {
        if (p < end)
          p++;
        continue;
      }

      while (p < end && isspace((unsigned char)*p))
        p++;

      if (strcmp(key, "cli") == 0 || strcmp(key, "to_cli") == 0 ||
          strcmp(key, "from_cli") == 0 || strcmp(key, "to_json") == 0 ||
          strcmp(key, "from_json") == 0 || strcmp(key, "arg") == 0 ||
          strcmp(key, "clap") == 0 || strcmp(key, "structopt") == 0 ||
          strcmp(key, "command") == 0) {
        if (*p == '(')
          p++;
        continue;
      }

      if (p < end && (*p == '=' || *p == ':' || *p == '(')) {
        int bval = 1;
        if (*p != '(')
          p++;
        if (strcmp(key, "replace") == 0 || strcmp(key, "rewrite") == 0 ||
            strcmp(key, "replace_all") == 0) {
          serde_parse_replace_tuple(&p, end, field->serde_replace_from,
                                    sizeof(field->serde_replace_from),
                                    field->serde_replace_to,
                                    sizeof(field->serde_replace_to));
          continue;
        }
        if (strcmp(key, "required_if_eq") == 0 ||
            strcmp(key, "required_if") == 0) {
          serde_parse_replace_tuple(&p, end, field->cli_required_if_field,
                                    sizeof(field->cli_required_if_field),
                                    field->cli_required_if_val,
                                    sizeof(field->cli_required_if_val));
          continue;
        }

        serde_parse_value(&p, end, val, sizeof(val));

        if (strcmp(key, "rename") == 0) {
          serde_copy_str(field->serde_rename, sizeof(field->serde_rename), val);
        } else if (strcmp(key, "rename_serialize") == 0 ||
                   strcmp(key, "serialize") == 0) {
          serde_copy_str(field->serde_rename_ser,
                         sizeof(field->serde_rename_ser), val);
        } else if (strcmp(key, "rename_deserialize") == 0 ||
                   strcmp(key, "deserialize") == 0) {
          serde_copy_str(field->serde_rename_de, sizeof(field->serde_rename_de),
                         val);
        } else if (strcmp(key, "case") == 0 || strcmp(key, "rename_all") == 0) {
          if (cdd_serde_parse_case_rule(val, &rule) == CDD_C_SUCCESS) {
            field->serde_case = rule;
          }
        } else if (strcmp(key, "alias") == 0 ||
                   strcmp(key, "visible_alias") == 0) {
          rc = struct_field_add_alias(field, val);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "short") == 0) {
          if (strcmp(val, "false") == 0 || strcmp(val, "0") == 0) {
            field->cli_short = '\0';
          } else if (strcmp(val, "true") == 0 || val[0] == '\0') {
            serde_infer_short_char(field->name, &field->cli_short);
          } else {
            field->cli_short = val[0];
          }
        } else if (strcmp(key, "short_alias") == 0 ||
                   strcmp(key, "visible_short_alias") == 0) {
          if (val[0] != '\0') {
            serde_add_short_alias_char(field, val[0]);
          }
        } else if (strcmp(key, "long") == 0) {
          if (strcmp(val, "false") == 0 || strcmp(val, "0") == 0) {
            field->cli_has_long = -1;
            field->cli_long[0] = '\0';
          } else if (strcmp(val, "true") == 0 || val[0] == '\0') {
            field->cli_has_long = 1;
            if (field->cli_long[0] == '\0' && field->name[0] != '\0') {
              cdd_serde_transform_case(field->name, CDD_SERDE_CASE_KEBABCASE,
                                       field->cli_long,
                                       sizeof(field->cli_long));
            }
          } else {
            field->cli_has_long = 1;
            serde_copy_str(field->cli_long, sizeof(field->cli_long), val);
          }
        } else if (strcmp(key, "negatable") == 0 ||
                   strcmp(key, "negate_with_no") == 0) {
          serde_parse_bool_val(val, &bval);
          field->cli_negatable = bval;
        } else if (strcmp(key, "positional") == 0) {
          serde_parse_bool_val(val, &bval);
          field->cli_positional = bval;
        } else if (strcmp(key, "index") == 0) {
          field->cli_positional = 1;
          field->cli_index = atoi(val);
        } else if (strcmp(key, "last") == 0) {
          serde_parse_bool_val(val, &bval);
          field->cli_last = bval;
          if (bval)
            field->cli_positional = 1;
        } else if (strcmp(key, "trailing_var_arg") == 0) {
          serde_parse_bool_val(val, &bval);
          field->cli_trailing_var_arg = bval;
          if (bval)
            field->cli_positional = 1;
        } else if (strcmp(key, "value_name") == 0) {
          serde_copy_str(field->cli_value_name, sizeof(field->cli_value_name),
                         val);
        } else if (strcmp(key, "help") == 0) {
          serde_copy_str(field->cli_help, sizeof(field->cli_help), val);
          if (field->description[0] == '\0') {
            serde_copy_str(field->description, sizeof(field->description), val);
          }
        } else if (strcmp(key, "long_help") == 0) {
          if (field->cli_long_help) {
            C_CDD_FREE(field->cli_long_help);
            field->cli_long_help = NULL;
          }
          rc = c_cdd_strdup(val, &field->cli_long_help);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "help_heading") == 0) {
          serde_copy_str(field->cli_help_heading,
                         sizeof(field->cli_help_heading), val);
        } else if (strcmp(key, "env") == 0) {
          if (strcmp(val, "true") == 0 || val[0] == '\0') {
            if (field->name[0] != '\0') {
              cdd_serde_transform_case(field->name,
                                       CDD_SERDE_CASE_SCREAMING_SNAKECASE,
                                       field->cli_env, sizeof(field->cli_env));
            } else {
              serde_copy_str(field->cli_env, sizeof(field->cli_env), "1");
            }
          } else if (strcmp(val, "false") == 0 || strcmp(val, "0") == 0) {
            field->cli_env[0] = '\0';
          } else {
            serde_copy_str(field->cli_env, sizeof(field->cli_env), val);
          }
        } else if (strcmp(key, "hide_env") == 0) {
          serde_parse_bool_val(val, &bval);
          field->cli_hide_env = bval;
        } else if (strcmp(key, "hide_env_values") == 0) {
          serde_parse_bool_val(val, &bval);
          field->cli_hide_env_values = bval;
        } else if (strcmp(key, "default_missing_value") == 0) {
          serde_copy_str(field->cli_default_missing_val,
                         sizeof(field->cli_default_missing_val), val);
        } else if (strcmp(key, "hide_default_value") == 0) {
          serde_parse_bool_val(val, &bval);
          field->cli_hide_default_value = bval;
        } else if (strcmp(key, "action") == 0) {
          enum CddCliAction act = CDD_CLI_ACTION_AUTO;
          if (cdd_cli_parse_action(val, &act) == CDD_C_SUCCESS) {
            field->cli_action = act;
          }
        } else if (strcmp(key, "value_hint") == 0) {
          enum CddCliValueHint hint = CDD_CLI_HINT_NONE;
          if (cdd_cli_parse_value_hint(val, &hint) == CDD_C_SUCCESS) {
            field->cli_value_hint = hint;
          }
        } else if (strcmp(key, "value_delimiter") == 0 ||
                   strcmp(key, "delimiter") == 0) {
          field->cli_value_delimiter = (val[0] != '\0') ? val[0] : ',';
        } else if (strcmp(key, "num_args") == 0) {
          serde_parse_num_args(val, &field->cli_num_args_min,
                               &field->cli_num_args_max);
        } else if (strcmp(key, "require_equals") == 0) {
          serde_parse_bool_val(val, &bval);
          field->cli_require_equals = bval;
        } else if (strcmp(key, "allow_hyphen_values") == 0 ||
                   strcmp(key, "allow_negative_numbers") == 0) {
          serde_parse_bool_val(val, &bval);
          field->cli_allow_hyphen_values = bval;
        } else if (strcmp(key, "global") == 0) {
          serde_parse_bool_val(val, &bval);
          field->cli_global = bval;
        } else if (strcmp(key, "hide") == 0 || strcmp(key, "hidden") == 0) {
          serde_parse_bool_val(val, &bval);
          field->cli_hidden = bval;
        } else if (strcmp(key, "exclusive") == 0) {
          serde_parse_bool_val(val, &bval);
          field->cli_exclusive = bval;
        } else if (strcmp(key, "subcommand") == 0) {
          serde_parse_bool_val(val, &bval);
          field->cli_subcommand = bval;
        } else if (strcmp(key, "flatten") == 0) {
          serde_parse_bool_val(val, &bval);
          field->serde_flatten = bval;
        } else if (strcmp(key, "required") == 0) {
          serde_parse_bool_val(val, &bval);
          field->required = bval;
        } else if (strcmp(key, "value_parser") == 0) {
          serde_copy_str(field->cli_value_parser,
                         sizeof(field->cli_value_parser), val);
        } else if (strcmp(key, "group") == 0) {
          serde_copy_str(field->cli_group, sizeof(field->cli_group), val);
        } else if (strcmp(key, "conflicts_with") == 0) {
          rc = struct_field_add_cli_conflict(field, val);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "requires") == 0) {
          rc = struct_field_add_cli_requires(field, val);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "required_unless_present") == 0 ||
                   strcmp(key, "required_unless") == 0) {
          rc = struct_field_add_cli_required_unless(field, val);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "skip_serializing_if") == 0) {
          int eq_null = 0, eq_nullptr = 0, eq_empty = 0, eq_zero = 0;
          serde_str_ieq(val, "null", &eq_null);
          serde_str_ieq(val, "nullptr", &eq_nullptr);
          serde_str_ieq(val, "empty", &eq_empty);
          if (!eq_empty) {
            serde_str_ieq(val, "is_empty", &eq_empty);
          }
          serde_str_ieq(val, "zero", &eq_zero);
          if (!eq_zero) {
            serde_str_ieq(val, "is_zero", &eq_zero);
          }

          if (eq_null || eq_nullptr) {
            field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_NULL;
          } else if (eq_empty) {
            field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_EMPTY;
          } else if (eq_zero) {
            field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_ZERO;
          } else if (val[0] != '\0') {
            field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_FUNC;
            serde_copy_str(field->serde_skip_if_fn,
                           sizeof(field->serde_skip_if_fn), val);
          }
        } else if (strcmp(key, "default") == 0 ||
                   strcmp(key, "default_value") == 0 ||
                   strcmp(key, "default_value_t") == 0) {
          field->serde_has_default = 1;
          serde_copy_str(field->default_val, sizeof(field->default_val), val);
        } else if (strcmp(key, "default_fn") == 0) {
          field->serde_has_default = 1;
          serde_copy_str(field->serde_default_fn,
                         sizeof(field->serde_default_fn), val);
        } else if (strcmp(key, "with") == 0) {
          serde_copy_str(field->serde_with, sizeof(field->serde_with), val);
        } else if (strcmp(key, "serialize_with") == 0) {
          serde_copy_str(field->serde_serialize_with,
                         sizeof(field->serde_serialize_with), val);
        } else if (strcmp(key, "deserialize_with") == 0) {
          serde_copy_str(field->serde_deserialize_with,
                         sizeof(field->serde_deserialize_with), val);
        } else if (strcmp(key, "c_type") == 0) {
          serde_copy_str(field->c_type, sizeof(field->c_type), val);
        } else if (strcmp(key, "nullable_ptr") == 0) {
          serde_parse_bool_val(val, &bval);
          field->is_nullable_ptr = bval;
        } else if (strcmp(key, "fixed_array_len") == 0) {
          field->fixed_array_len = (size_t)atoi(val);
        }
      } else {
        if (strcmp(key, "skip") == 0) {
          field->serde_skip = 1;
          field->serde_skip_serializing = 1;
          field->serde_skip_deserializing = 1;
        } else if (strcmp(key, "nullable_ptr") == 0) {
          field->is_nullable_ptr = 1;
        } else if (strcmp(key, "skip_serializing") == 0) {
          field->serde_skip_serializing = 1;
        } else if (strcmp(key, "skip_deserializing") == 0) {
          field->serde_skip_deserializing = 1;
        } else if (strcmp(key, "flatten") == 0) {
          field->serde_flatten = 1;
        } else if (strcmp(key, "default") == 0 ||
                   strcmp(key, "default_value") == 0 ||
                   strcmp(key, "default_value_t") == 0) {
          field->serde_has_default = 1;
        } else if (strcmp(key, "short") == 0) {
          serde_infer_short_char(field->name, &field->cli_short);
        } else if (strcmp(key, "long") == 0) {
          field->cli_has_long = 1;
          if (field->cli_long[0] == '\0' && field->name[0] != '\0') {
            cdd_serde_transform_case(field->name, CDD_SERDE_CASE_KEBABCASE,
                                     field->cli_long, sizeof(field->cli_long));
          }
        } else if (strcmp(key, "negatable") == 0 ||
                   strcmp(key, "negate_with_no") == 0) {
          field->cli_negatable = 1;
        } else if (strcmp(key, "positional") == 0) {
          field->cli_positional = 1;
        } else if (strcmp(key, "last") == 0) {
          field->cli_last = 1;
          field->cli_positional = 1;
        } else if (strcmp(key, "trailing_var_arg") == 0) {
          field->cli_trailing_var_arg = 1;
          field->cli_positional = 1;
        } else if (strcmp(key, "env") == 0) {
          if (field->name[0] != '\0') {
            cdd_serde_transform_case(field->name,
                                     CDD_SERDE_CASE_SCREAMING_SNAKECASE,
                                     field->cli_env, sizeof(field->cli_env));
          } else {
            serde_copy_str(field->cli_env, sizeof(field->cli_env), "1");
          }
        } else if (strcmp(key, "hide_env") == 0) {
          field->cli_hide_env = 1;
        } else if (strcmp(key, "hide_env_values") == 0) {
          field->cli_hide_env_values = 1;
        } else if (strcmp(key, "hide_default_value") == 0) {
          field->cli_hide_default_value = 1;
        } else if (strcmp(key, "value_delimiter") == 0 ||
                   strcmp(key, "delimiter") == 0) {
          field->cli_value_delimiter = ',';
        } else if (strcmp(key, "require_equals") == 0) {
          field->cli_require_equals = 1;
        } else if (strcmp(key, "allow_hyphen_values") == 0 ||
                   strcmp(key, "allow_negative_numbers") == 0) {
          field->cli_allow_hyphen_values = 1;
        } else if (strcmp(key, "global") == 0) {
          field->cli_global = 1;
        } else if (strcmp(key, "hide") == 0 || strcmp(key, "hidden") == 0) {
          field->cli_hidden = 1;
        } else if (strcmp(key, "exclusive") == 0) {
          field->cli_exclusive = 1;
        } else if (strcmp(key, "subcommand") == 0) {
          field->cli_subcommand = 1;
        } else if (strcmp(key, "required") == 0) {
          field->required = 1;
        } else if (strcmp(key, "count") == 0) {
          field->cli_action = CDD_CLI_ACTION_COUNT;
        } else if (cdd_serde_parse_case_rule(key, &rule) == CDD_C_SUCCESS &&
                   rule != CDD_SERDE_CASE_NONE) {
          field->serde_case = rule;
        }
      }
    }

    scan = next;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Frees temporary heap allocations on a stack `struct StructField`.
 *
 * @param[in,out] f Stack-allocated StructField to clean up.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_cleanup_tmp_field(struct StructField *f) {
  size_t i;
  if (f->serde_aliases) {
    for (i = 0; i < f->n_serde_aliases; ++i) {
      C_CDD_FREE(f->serde_aliases[i]);
    }
    C_CDD_FREE(f->serde_aliases);
    f->serde_aliases = NULL;
    f->n_serde_aliases = 0;
  }
  if (f->cli_long_help) {
    C_CDD_FREE(f->cli_long_help);
    f->cli_long_help = NULL;
  }
  if (f->cli_conflicts_with) {
    for (i = 0; i < f->n_cli_conflicts_with; ++i) {
      C_CDD_FREE(f->cli_conflicts_with[i]);
    }
    C_CDD_FREE(f->cli_conflicts_with);
    f->cli_conflicts_with = NULL;
    f->n_cli_conflicts_with = 0;
  }
  if (f->cli_requires) {
    for (i = 0; i < f->n_cli_requires; ++i) {
      C_CDD_FREE(f->cli_requires[i]);
    }
    C_CDD_FREE(f->cli_requires);
    f->cli_requires = NULL;
    f->n_cli_requires = 0;
  }
  if (f->cli_required_unless) {
    for (i = 0; i < f->n_cli_required_unless; ++i) {
      C_CDD_FREE(f->cli_required_unless[i]);
    }
    C_CDD_FREE(f->cli_required_unless);
    f->cli_required_unless = NULL;
    f->n_cli_required_unless = 0;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Extended parser for enum-variant-level `@cdd` or `@serde` directives
 * including `alias` and `other`.
 */
cdd_c_error_t cdd_serde_parse_enum_variant_comment_ex(
    const char *comment_text, char *out_rename, size_t rename_sz, int *out_skip,
    enum CddSerdeCaseRule *out_case, char *out_rep_from, size_t rep_from_sz,
    char *out_rep_to, size_t rep_to_sz, int *out_other,
    struct EnumMembers *em_for_aliases, size_t member_idx) {
  struct StructField tmp;
  size_t i;
  cdd_c_error_t rc;

  if (!out_rename || rename_sz == 0 || !out_skip || !out_case ||
      !out_rep_from || rep_from_sz == 0 || !out_rep_to || rep_to_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  out_rename[0] = '\0';
  *out_skip = 0;
  *out_case = CDD_SERDE_CASE_NONE;
  out_rep_from[0] = '\0';
  out_rep_to[0] = '\0';
  if (out_other)
    *out_other = 0;

  if (!comment_text || comment_text[0] == '\0')
    return CDD_C_SUCCESS;

  /* Scan for `other` flag in directive block */
  if (out_other) {
    const char *scan = comment_text;
    const char *start = NULL, *end = NULL, *next = NULL;
    while (1) {
      const char *p;
      serde_find_directive_ex(scan, &start, &end, &next, NULL);
      if (!start)
        break;
      p = start;
      while (p < end) {
        char key[64];
        serde_next_key(&p, end, key, sizeof(key));
        if (key[0] == '\0') {
          if (p < end)
            p++;
          continue;
        }
        if (strcmp(key, "other") == 0) {
          *out_other = 1;
        }
      }
      scan = next;
    }
  }

  memset(&tmp, 0, sizeof(tmp));
  tmp.cli_index = -1;
  rc = cdd_serde_parse_field_comment(comment_text, &tmp);
  if (rc != CDD_C_SUCCESS) {
    serde_cleanup_tmp_field(&tmp);
    return rc;
  }

  if (em_for_aliases) {
    if (tmp.serde_aliases) {
      for (i = 0; i < tmp.n_serde_aliases; ++i) {
        rc = enum_member_add_alias(em_for_aliases, member_idx,
                                   tmp.serde_aliases[i]);
        if (rc != CDD_C_SUCCESS) {
          serde_cleanup_tmp_field(&tmp);
          return rc;
        }
      }
    }
    if (tmp.cli_help[0] != '\0' || tmp.cli_hidden) {
      rc = enum_member_set_cli_meta(em_for_aliases, member_idx,
                                    tmp.cli_help[0] ? tmp.cli_help : NULL,
                                    tmp.cli_hidden);
      if (rc != CDD_C_SUCCESS) {
        serde_cleanup_tmp_field(&tmp);
        return rc;
      }
    }
  }

  serde_cleanup_tmp_field(&tmp);

  if (tmp.serde_rename[0] != '\0') {
    serde_copy_str(out_rename, rename_sz, tmp.serde_rename);
  }
  if (tmp.serde_skip || tmp.serde_skip_serializing ||
      tmp.serde_skip_deserializing) {
    *out_skip = 1;
  }
  *out_case = tmp.serde_case;
  if (tmp.serde_replace_from[0] != '\0') {
    serde_copy_str(out_rep_from, rep_from_sz, tmp.serde_replace_from);
    serde_copy_str(out_rep_to, rep_to_sz, tmp.serde_replace_to);
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Parses enum-variant-level `@cdd` or `@serde`
 * directives for a single enum constant.
 */
cdd_c_error_t cdd_serde_parse_enum_variant_comment(
    const char *comment_text, char *out_rename, size_t rename_sz, int *out_skip,
    enum CddSerdeCaseRule *out_case, char *out_rep_from, size_t rep_from_sz,
    char *out_rep_to, size_t rep_to_sz) {
  return cdd_serde_parse_enum_variant_comment_ex(
      comment_text, out_rename, rename_sz, out_skip, out_case, out_rep_from,
      rep_from_sz, out_rep_to, rep_to_sz, NULL, NULL, 0);
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
