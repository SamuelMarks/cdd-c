/**
 * @file cli_parser.c
 * @brief Implementation of CLI parser extraction from C source code.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/memory.h"
#include "classes/parse/cdd_serde.h"
#include "classes/parse/cli_parser.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
/* clang-format on */

/**
 * @brief Allocates a null-terminated copy of up to n bytes of a string.
 *
 * @param[in] s Source string
 * @param[in] n Maximum length to duplicate
 * @param[out] _out_val Pointer to receive allocated string
 * @return CDD_C_SUCCESS on success, error enum on failure
 */
static cdd_c_error_t cdd_strndup2(const char *s, size_t n, char **_out_val) {
  char *result;
  size_t len = 0;
  if (!s || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  while (len < n && s[len] != '\0')
    len++;
  result = (char *)C_CDD_MALLOC(len + 1);
  if (!result) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
  memcpy(result, s, len);
  result[len] = '\0';
  *_out_val = result;
  return CDD_C_SUCCESS;
}

/*
 * @brief Initializes a parsed CLI command descriptor.
 *
 * @param[out] cmd Pointer to the CliCommand structure to initialize.
 * @return CDD_C_SUCCESS on success, error enum on failure
 */
cdd_c_error_t cli_command_init(struct CliCommand *cmd) {
  if (!cmd)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  memset(cmd, 0, sizeof(*cmd));
  return CDD_C_SUCCESS;
}

/*
 * @brief Releases memory associated with a parsed CLI command descriptor.
 *
 * @param[in,out] cmd Pointer to the CliCommand structure to free.
 * @return CDD_C_SUCCESS.
 */
cdd_c_error_t cli_command_free(struct CliCommand *cmd) {
  size_t i;
  if (!cmd)
    return CDD_C_SUCCESS;
  if (cmd->name)
    C_CDD_FREE(cmd->name);
  if (cmd->version)
    C_CDD_FREE(cmd->version);
  if (cmd->about)
    C_CDD_FREE(cmd->about);
  if (cmd->author)
    C_CDD_FREE(cmd->author);
  if (cmd->options) {
    for (i = 0; i < cmd->n_options; ++i) {
      if (cmd->options[i].long_flag)
        C_CDD_FREE(cmd->options[i].long_flag);
      if (cmd->options[i].description)
        C_CDD_FREE(cmd->options[i].description);
      if (cmd->options[i].mapped_struct_field)
        C_CDD_FREE(cmd->options[i].mapped_struct_field);
      if (cmd->options[i].default_value)
        C_CDD_FREE(cmd->options[i].default_value);
      if (cmd->options[i].env_var)
        C_CDD_FREE(cmd->options[i].env_var);
      if (cmd->options[i].value_name)
        C_CDD_FREE(cmd->options[i].value_name);
      if (cmd->options[i].type_str)
        C_CDD_FREE(cmd->options[i].type_str);
    }
    C_CDD_FREE(cmd->options);
  }
  if (cmd->subcommands) {
    for (i = 0; i < cmd->n_subcommands; ++i) {
      cli_command_free(&cmd->subcommands[i]);
    }
    C_CDD_FREE(cmd->subcommands);
  }
  memset(cmd, 0, sizeof(*cmd));
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets option in CLI command.
 *
 * @param[in,out] cmd CLI command structure
 * @param[out] _out_val Pointer to receive new option
 * @return CDD_C_SUCCESS on success, error enum on failure
 */
static cdd_c_error_t add_option(struct CliCommand *cmd,
                                struct CliOption **_out_val) {
  struct CliOption *opt;
  void *new_arr;
  if (!cmd || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  new_arr = C_CDD_REALLOC(cmd->options,
                          (cmd->n_options + 1) * sizeof(struct CliOption));
  if (!new_arr)
    return CDD_C_ERROR_MEMORY;
  cmd->options = (struct CliOption *)new_arr;
  opt = &cmd->options[cmd->n_options++];
  memset(opt, 0, sizeof(*opt));
  opt->index = -1;
  *_out_val = opt;
  return CDD_C_SUCCESS;
}

/**
 * @brief Advances token index `idx` past any whitespace or comment tokens.
 *
 * @param[in] tokens Token list.
 * @param[in] idx Starting index.
 * @param[out] out_idx Next non-trivia token index.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t skip_trivia_tok(const struct TokenList *tokens, size_t idx,
                                     size_t *out_idx) {
  while (idx < tokens->size && (tokens->tokens[idx].kind == TOKEN_WHITESPACE ||
                                tokens->tokens[idx].kind == TOKEN_COMMENT)) {
    idx++;
  }
  *out_idx = idx;
  return CDD_C_SUCCESS;
}

/**
 * @brief Finds an existing option in `cmd` by `short_flag`, `long_flag`, or
 * `mapped_struct_field`.
 *
 * @param[in] cmd CLI command container.
 * @param[in] short_flag Short option character (or 0).
 * @param[in] long_flag Long option string (or NULL).
 * @param[in] field_name Mapped struct field name (or NULL).
 * @param[out] out_opt Pointer to matching `CliOption`, or NULL if not found.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t find_cli_option(struct CliCommand *cmd, char short_flag,
                                     const char *long_flag,
                                     const char *field_name,
                                     struct CliOption **out_opt) {
  size_t i;
  *out_opt = NULL;
  for (i = 0; i < cmd->n_options; ++i) {
    if (short_flag != '\0' && cmd->options[i].short_flag == short_flag) {
      *out_opt = &cmd->options[i];
      return CDD_C_SUCCESS;
    }
    if (long_flag && cmd->options[i].long_flag &&
        strcmp(cmd->options[i].long_flag, long_flag) == 0) {
      *out_opt = &cmd->options[i];
      return CDD_C_SUCCESS;
    }
    if (field_name && cmd->options[i].mapped_struct_field &&
        strcmp(cmd->options[i].mapped_struct_field, field_name) == 0) {
      *out_opt = &cmd->options[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Extracts CLI option and subcommand metadata from a C CST.
 *
 * @param[in] nodes Parsed CST node list.
 * @param[in] tokens Lexical token list.
 * @param[out] cmd Output CliCommand descriptor to populate.
 * @return CDD_C_SUCCESS on success, or an error code on failure.
 */
cdd_c_error_t cst_extract_cli_command(const struct CstNodeList *nodes,
                                      const struct TokenList *tokens,
                                      struct CliCommand *cmd) {
  size_t i, j;
  int in_getopt = 0;
  cdd_c_error_t rc;

  if (!nodes || !tokens)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cli_command_init(cmd);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c_cdd_strdup("cli_app", &cmd->name);
  if (rc != CDD_C_SUCCESS) {
    cli_command_free(cmd);
    return rc;
  }

  /* 1. Look for `getopt(...)` or `getopt_long(...)` short optstring */
  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
      const char *str = (const char *)tokens->tokens[i].start;
      size_t len = tokens->tokens[i].length;
      if ((len == 6 && strncmp(str, "getopt", 6) == 0) ||
          (len == 11 && strncmp(str, "getopt_long", 11) == 0)) {
        size_t limit;
        in_getopt = 1;
        limit = i + 10 < tokens->size ? i + 10 : tokens->size;
        /* Next string literal usually has short flags */
        for (j = i + 1; j < limit; ++j) {
          if (tokens->tokens[j].kind == TOKEN_STRING_LITERAL) {
            const char *flags =
                (const char *)tokens->tokens[j].start + 1; /* skip quote */
            size_t flen = tokens->tokens[j].length - 2;
            size_t k;
            for (k = 0; k < flen; ++k) {
              if (isalpha((unsigned char)flags[k])) {
                struct CliOption *opt = NULL;
                rc = add_option(cmd, &opt);
                if (rc != CDD_C_SUCCESS) {
                  cli_command_free(cmd);
                  return rc;
                }
                opt->short_flag = flags[k];
                if (k + 1 < flen && flags[k + 1] == ':') {
                  if (k + 2 < flen && flags[k + 2] == ':') {
                    opt->has_arg = 2;
                    k += 2;
                  } else {
                    opt->has_arg = 1;
                    k++;
                  }
                } else {
                  opt->has_arg = 0;
                }
                rc = c_cdd_strdup("Auto-extracted option", &opt->description);
                if (rc != CDD_C_SUCCESS) {
                  cli_command_free(cmd);
                  return rc;
                }
              }
            }
            break;
          }
        }
      }
    }
  }

  /* 2. Scan for `struct option` tables: `{ "input", required_argument, 0, 'i'
   * }` */
  for (i = 0; i + 8 < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_KEYWORD_STRUCT) {
      size_t id_idx = 0;
      rc = skip_trivia_tok(tokens, i + 1, &id_idx);
      if (tokens->tokens[id_idx].kind == TOKEN_IDENTIFIER &&
          tokens->tokens[id_idx].length == 6 &&
          strncmp((const char *)tokens->tokens[id_idx].start, "option", 6) ==
              0) {
        size_t b_idx = id_idx + 1;
        while (b_idx < tokens->size &&
               tokens->tokens[b_idx].kind != TOKEN_LBRACE &&
               tokens->tokens[b_idx].kind != TOKEN_SEMICOLON) {
          b_idx++;
        }
        if (b_idx < tokens->size &&
            tokens->tokens[b_idx].kind == TOKEN_LBRACE) {
          size_t cur = b_idx + 1;
          while (cur < tokens->size &&
                 tokens->tokens[cur].kind != TOKEN_SEMICOLON) {
            if (tokens->tokens[cur].kind == TOKEN_LBRACE) {
              size_t t1 = 0;
              rc = skip_trivia_tok(tokens, cur + 1, &t1);
              if (t1 < tokens->size &&
                  tokens->tokens[t1].kind == TOKEN_STRING_LITERAL) {
                char lflag[64];
                size_t l_len = tokens->tokens[t1].length - 2;
                int has_arg_val = 0;
                char sflag = '\0';
                size_t scan_e = t1 + 1;
                if (l_len >= sizeof(lflag))
                  l_len = sizeof(lflag) - 1;
                memcpy(lflag, tokens->tokens[t1].start + 1, l_len);
                lflag[l_len] = '\0';

                while (scan_e < tokens->size &&
                       tokens->tokens[scan_e].kind != TOKEN_RBRACE) {
                  if (tokens->tokens[scan_e].kind == TOKEN_IDENTIFIER) {
                    const char *id_s =
                        (const char *)tokens->tokens[scan_e].start;
                    size_t id_l = tokens->tokens[scan_e].length;
                    if (id_l == 17 &&
                        strncmp(id_s, "required_argument", 17) == 0)
                      has_arg_val = 1;
                    else if (id_l == 17 &&
                             strncmp(id_s, "optional_argument", 17) == 0)
                      has_arg_val = 2;
                  } else if (tokens->tokens[scan_e].kind ==
                             TOKEN_CHAR_LITERAL) {
                    sflag = (char)tokens->tokens[scan_e].start[1];
                  }
                  scan_e++;
                }

                if (lflag[0] != '\0') {
                  struct CliOption *opt = NULL;
                  rc = find_cli_option(cmd, sflag, lflag, NULL, &opt);
                  if (!opt) {
                    rc = add_option(cmd, &opt);
                    if (rc != CDD_C_SUCCESS) {
                      cli_command_free(cmd);
                      return rc;
                    }
                    rc = c_cdd_strdup("Auto-extracted option",
                                      &opt->description);
                    if (rc != CDD_C_SUCCESS) {
                      cli_command_free(cmd);
                      return rc;
                    }
                  }
                  if (sflag != '\0')
                    opt->short_flag = sflag;
                  opt->has_arg = has_arg_val;
                  if (!opt->long_flag) {
                    rc = c_cdd_strdup(lflag, &opt->long_flag);
                    if (rc != CDD_C_SUCCESS) {
                      cli_command_free(cmd);
                      return rc;
                    }
                  }
                }
                cur = scan_e;
              }
            }
            cur++;
          }
        }
      }
    }
  }

  /* 3. Scan for assignments inside switch cases for getopt */
  if (in_getopt) {
    for (i = 0; i < tokens->size; ++i) {
      if (tokens->tokens[i].kind == TOKEN_KEYWORD_CASE) {
        size_t next_idx = i + 1;
        while (next_idx < tokens->size) {
          if (tokens->tokens[next_idx].kind != TOKEN_WHITESPACE)
            break;
          next_idx++;
        }
        if (next_idx < tokens->size) {
          if (tokens->tokens[next_idx].kind == TOKEN_CHAR_LITERAL) {
            char c = (char)tokens->tokens[next_idx].start[1];
            size_t opt_idx;
            for (opt_idx = 0; opt_idx < cmd->n_options; ++opt_idx) {
              if (cmd->options[opt_idx].short_flag == c) {
                /* Find assignment before break */
                for (j = i + 2; j < tokens->size; ++j) {
                  if (tokens->tokens[j].kind == TOKEN_KEYWORD_BREAK)
                    break;
                  if (tokens->tokens[j].kind == TOKEN_ASSIGN) {
                    size_t prev_idx = j - 1;
                    while (tokens->tokens[prev_idx].kind == TOKEN_WHITESPACE)
                      prev_idx--;
                    if (tokens->tokens[prev_idx].kind == TOKEN_IDENTIFIER) {
                      rc = cdd_strndup2(
                          (const char *)tokens->tokens[prev_idx].start,
                          tokens->tokens[prev_idx].length,
                          &cmd->options[opt_idx].mapped_struct_field);
                      if (rc != CDD_C_SUCCESS) {
                        cli_command_free(cmd);
                        return rc;
                      }
                      break;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  /* 4. Scan for `strcmp(argv[i], "--flag")` / `strcmp(argv[i], "-f")` loops */
  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER &&
        tokens->tokens[i].length == 6 &&
        strncmp((const char *)tokens->tokens[i].start, "strcmp", 6) == 0) {
      size_t limit = (i + 16 < tokens->size) ? (i + 16) : tokens->size;
      int has_argv = 0;
      const uint8_t *str_start = NULL;
      size_t str_len = 0;
      for (j = i + 1; j < limit; ++j) {
        if (tokens->tokens[j].kind == TOKEN_RPAREN)
          break;
        if (tokens->tokens[j].kind == TOKEN_IDENTIFIER &&
            tokens->tokens[j].length == 4 &&
            strncmp((const char *)tokens->tokens[j].start, "argv", 4) == 0) {
          has_argv = 1;
        } else if (tokens->tokens[j].kind == TOKEN_STRING_LITERAL &&
                   tokens->tokens[j].length >= 3) {
          str_start = tokens->tokens[j].start + 1;
          str_len = tokens->tokens[j].length - 2;
        }
      }
      if (has_argv && str_start && str_start[0] == '-') {
        char lflag[64];
        char sflag = '\0';
        lflag[0] = '\0';
        if (str_len >= 3 && str_start[1] == '-') {
          size_t clen = str_len - 2;
          if (clen >= sizeof(lflag))
            clen = sizeof(lflag) - 1;
          memcpy(lflag, str_start + 2, clen);
          lflag[clen] = '\0';
        } else if (str_len == 2 && str_start[1] != '-') {
          sflag = (char)str_start[1];
        }
        if ((lflag[0] != '\0' && strcmp(lflag, "help") != 0 &&
             strcmp(lflag, "version") != 0) ||
            (sflag != '\0' && sflag != 'h' && sflag != 'V')) {
          /* Look ahead in the branch body for assignment and argv[++i] */
          size_t b_limit = (i + 50 < tokens->size) ? (i + 50) : tokens->size;
          char field_nm[64];
          int consumes_arg = 0;
          int is_int_type = 0;
          struct CliOption *opt;
          field_nm[0] = '\0';
          for (j = i + 1; j < b_limit; ++j) {
            if (tokens->tokens[j].kind == TOKEN_RBRACE)
              break;
            if (tokens->tokens[j].kind == TOKEN_INC)
              consumes_arg = 1;
            if (tokens->tokens[j].kind == TOKEN_IDENTIFIER &&
                ((tokens->tokens[j].length == 4 &&
                  strncmp((const char *)tokens->tokens[j].start, "atoi", 4) ==
                      0) ||
                 (tokens->tokens[j].length == 6 &&
                  strncmp((const char *)tokens->tokens[j].start, "strtol", 6) ==
                      0))) {
              is_int_type = 1;
              consumes_arg = 1;
            }
            if (tokens->tokens[j].kind == TOKEN_ASSIGN) {
              size_t p_idx = j - 1;
              while (tokens->tokens[p_idx].kind == TOKEN_WHITESPACE)
                p_idx--;
              if (tokens->tokens[p_idx].kind == TOKEN_IDENTIFIER &&
                  !(tokens->tokens[p_idx].length == 1 &&
                    tokens->tokens[p_idx].start[0] == 'i')) {
                size_t fl = tokens->tokens[p_idx].length;
                if (fl >= sizeof(field_nm))
                  fl = sizeof(field_nm) - 1;
                memcpy(field_nm, tokens->tokens[p_idx].start, fl);
                field_nm[fl] = '\0';
              }
            }
          }
          rc = find_cli_option(cmd, sflag, lflag[0] ? lflag : NULL,
                               field_nm[0] ? field_nm : NULL, &opt);
          if (!opt) {
            rc = add_option(cmd, &opt);
            if (rc != CDD_C_SUCCESS) {
              cli_command_free(cmd);
              return rc;
            }
            rc = c_cdd_strdup("Auto-extracted option", &opt->description);
            if (rc != CDD_C_SUCCESS) {
              cli_command_free(cmd);
              return rc;
            }
          }
          if (sflag != '\0' && opt->short_flag == '\0')
            opt->short_flag = sflag;
          if (lflag[0] != '\0' && !opt->long_flag) {
            rc = c_cdd_strdup(lflag, &opt->long_flag);
            if (rc != CDD_C_SUCCESS) {
              cli_command_free(cmd);
              return rc;
            }
          }
          if (consumes_arg)
            opt->has_arg = 1;
          if (field_nm[0] != '\0' && !opt->mapped_struct_field) {
            rc = c_cdd_strdup(field_nm, &opt->mapped_struct_field);
            if (rc != CDD_C_SUCCESS) {
              cli_command_free(cmd);
              return rc;
            }
          }
          if (!opt->type_str) {
            rc = c_cdd_strdup(
                is_int_type ? "integer" : (opt->has_arg ? "string" : "boolean"),
                &opt->type_str);
            if (rc != CDD_C_SUCCESS) {
              cli_command_free(cmd);
              return rc;
            }
          }
        }
      }
    }
  }

  /* 5. Scan for `getenv("VAR")` fallbacks associated with mapped fields */
  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER &&
        tokens->tokens[i].length == 6 &&
        strncmp((const char *)tokens->tokens[i].start, "getenv", 6) == 0) {
      size_t lp = 0;
      size_t s_tok = 0;
      rc = skip_trivia_tok(tokens, i + 1, &lp);
      rc = skip_trivia_tok(tokens, lp + 1, &s_tok);
      if (lp < tokens->size && tokens->tokens[lp].kind == TOKEN_LPAREN &&
          s_tok < tokens->size &&
          tokens->tokens[s_tok].kind == TOKEN_STRING_LITERAL &&
          tokens->tokens[s_tok].length >= 3) {
        char env_nm[64];
        size_t el = tokens->tokens[s_tok].length - 2;
        size_t back;
        if (el >= sizeof(env_nm))
          el = sizeof(env_nm) - 1;
        memcpy(env_nm, tokens->tokens[s_tok].start + 1, el);
        env_nm[el] = '\0';
        /* Look backwards on the same statement for `<field> = ... getenv(...)`
         */
        for (back = i; back > 0 && i - back < 25; --back) {
          if (tokens->tokens[back].kind == TOKEN_SEMICOLON ||
              tokens->tokens[back].kind == TOKEN_LBRACE)
            break;
          if (tokens->tokens[back].kind == TOKEN_ASSIGN) {
            size_t lhs = back - 1;
            while (lhs > 0 && tokens->tokens[lhs].kind == TOKEN_WHITESPACE)
              lhs--;
            if (tokens->tokens[lhs].kind == TOKEN_IDENTIFIER) {
              char fnm[64];
              size_t fl = tokens->tokens[lhs].length;
              struct CliOption *opt = NULL;
              if (fl >= sizeof(fnm))
                fl = sizeof(fnm) - 1;
              memcpy(fnm, tokens->tokens[lhs].start, fl);
              fnm[fl] = '\0';
              rc = find_cli_option(cmd, '\0', NULL, fnm, &opt);
              if (opt && !opt->env_var) {
                rc = c_cdd_strdup(env_nm, &opt->env_var);
                if (rc != CDD_C_SUCCESS) {
                  cli_command_free(cmd);
                  return rc;
                }
              }
              break;
            }
          }
        }
      }
    }
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Converts an extracted `CliCommand` IR into a `StructFields` container
 * populated with CLI reflection metadata.
 */
cdd_c_error_t cli_command_to_struct_fields(const struct CliCommand *cmd,
                                           struct StructFields *sf) {
  size_t i;
  cdd_c_error_t rc;
  if (!cmd || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  sf->is_cli_command = 1;
  if (cmd->name && cmd->name[0] != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(sf->cli_name, sizeof(sf->cli_name), cmd->name, _TRUNCATE);
#else
    strncpy(sf->cli_name, cmd->name, sizeof(sf->cli_name) - 1);
    sf->cli_name[sizeof(sf->cli_name) - 1] = '\0';
#endif
  }
  if (cmd->version && cmd->version[0] != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(sf->cli_version, sizeof(sf->cli_version), cmd->version,
              _TRUNCATE);
#else
    strncpy(sf->cli_version, cmd->version, sizeof(sf->cli_version) - 1);
    sf->cli_version[sizeof(sf->cli_version) - 1] = '\0';
#endif
  }
  if (cmd->about && cmd->about[0] != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(sf->cli_about, sizeof(sf->cli_about), cmd->about, _TRUNCATE);
#else
    strncpy(sf->cli_about, cmd->about, sizeof(sf->cli_about) - 1);
    sf->cli_about[sizeof(sf->cli_about) - 1] = '\0';
#endif
  }

  for (i = 0; i < cmd->n_options; ++i) {
    const struct CliOption *opt = &cmd->options[i];
    char raw_name[64];
    char *san_name = NULL;
    const char *ftype;
    struct StructField *f;

    if (opt->mapped_struct_field && opt->mapped_struct_field[0] != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      strncpy_s(raw_name, sizeof(raw_name), opt->mapped_struct_field,
                _TRUNCATE);
#else
      strncpy(raw_name, opt->mapped_struct_field, sizeof(raw_name) - 1);
      raw_name[sizeof(raw_name) - 1] = '\0';
#endif
    } else if (opt->long_flag && opt->long_flag[0] != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      strncpy_s(raw_name, sizeof(raw_name), opt->long_flag, _TRUNCATE);
#else
      strncpy(raw_name, opt->long_flag, sizeof(raw_name) - 1);
      raw_name[sizeof(raw_name) - 1] = '\0';
#endif
    } else if (opt->short_flag != '\0') {
      raw_name[0] = opt->short_flag;
      raw_name[1] = '\0';
    } else {
      continue;
    }

    rc = sanitize_identifier(raw_name, &san_name);
    if (rc != CDD_C_SUCCESS)
      return rc;

    ftype =
        opt->type_str ? opt->type_str : (opt->has_arg ? "string" : "boolean");
    rc = struct_fields_add(sf, san_name, ftype, NULL, opt->default_value, NULL);
    C_CDD_FREE(san_name);
    if (rc != CDD_C_SUCCESS)
      return rc;

    f = &sf->fields[sf->size - 1];
    f->cli_short = opt->short_flag;
    if (opt->long_flag && opt->long_flag[0] != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      strncpy_s(f->cli_long, sizeof(f->cli_long), opt->long_flag, _TRUNCATE);
#else
      strncpy(f->cli_long, opt->long_flag, sizeof(f->cli_long) - 1);
      f->cli_long[sizeof(f->cli_long) - 1] = '\0';
#endif
      f->cli_has_long = 1;
    }
    if (opt->description && opt->description[0] != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      strncpy_s(f->cli_help, sizeof(f->cli_help), opt->description, _TRUNCATE);
      strncpy_s(f->description, sizeof(f->description), opt->description,
                _TRUNCATE);
#else
      strncpy(f->cli_help, opt->description, sizeof(f->cli_help) - 1);
      f->cli_help[sizeof(f->cli_help) - 1] = '\0';
      strncpy(f->description, opt->description, sizeof(f->description) - 1);
      f->description[sizeof(f->description) - 1] = '\0';
#endif
    }
    if (opt->env_var && opt->env_var[0] != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      strncpy_s(f->cli_env, sizeof(f->cli_env), opt->env_var, _TRUNCATE);
#else
      strncpy(f->cli_env, opt->env_var, sizeof(f->cli_env) - 1);
      f->cli_env[sizeof(f->cli_env) - 1] = '\0';
#endif
    }
    if (opt->value_name && opt->value_name[0] != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      strncpy_s(f->cli_value_name, sizeof(f->cli_value_name), opt->value_name,
                _TRUNCATE);
#else
      strncpy(f->cli_value_name, opt->value_name,
              sizeof(f->cli_value_name) - 1);
      f->cli_value_name[sizeof(f->cli_value_name) - 1] = '\0';
#endif
    }
    f->cli_positional = opt->is_positional;
    f->cli_index = opt->index;
    f->required = opt->is_required;
    f->cli_action = opt->action;
  }

  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
/**
 * @brief Helper for testing internal error conditions.
 *
 * @return CDD_C_SUCCESS on success, error enum on failure
 */
C_CDD_EXPORT cdd_c_error_t test_cli_parser_internal_errors(void);
C_CDD_EXPORT cdd_c_error_t test_cli_parser_internal_errors(void) {
  char *out = NULL;
  struct CliOption *opt = NULL;
  cdd_c_error_t err1 = cdd_strndup2(NULL, 0, &out);
  cdd_c_error_t err2 = cdd_strndup2("abc", 3, NULL);
  cdd_c_error_t err3 = add_option(NULL, &opt);
  cdd_c_error_t err4 = add_option((struct CliCommand *)1, NULL);
  cdd_c_error_t err5, err6, err_extra;

  g_cdd_alloc_fail = 1;
  err5 = cdd_strndup2("abc", 3, &out);
  g_cdd_alloc_fail = 0;

  g_cdd_alloc_fail = 2;
  err_extra = cdd_strndup2("abc", 10, &out);
  C_CDD_FREE(out);
  out = NULL;
  err6 = cdd_strndup2("abc", 3, &out);
  g_cdd_alloc_fail = 0;

  return (cdd_c_error_t)((err1 ^ CDD_C_ERROR_INVALID_ARGUMENT) |
                         (err2 ^ CDD_C_ERROR_INVALID_ARGUMENT) |
                         (err3 ^ CDD_C_ERROR_INVALID_ARGUMENT) |
                         (err4 ^ CDD_C_ERROR_INVALID_ARGUMENT) |
                         (err5 ^ CDD_C_ERROR_MEMORY) |
                         (err6 ^ CDD_C_ERROR_MEMORY) |
                         (err_extra ^ CDD_C_SUCCESS));
}
#endif
