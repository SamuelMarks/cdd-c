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
#include "classes/parse/cli_parser.h"
#include "functions/parse/str.h"
/* clang-format on */

/**
 * @brief Executes the cdd strndup2 operation.
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
  result = (char *)(size_t)C_CDD_MALLOC(len + 1);
  if (!result) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
  memcpy(result, s, len);
  result[len] = '\0';
  *_out_val = result;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the cli command init operation.
 *
 * @param[out] cmd CLI command structure to initialize
 * @return CDD_C_SUCCESS on success, error enum on failure
 */
cdd_c_error_t cli_command_init(struct CliCommand *cmd) {
  if (!cmd)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  cmd->name = NULL;
  cmd->options = NULL;
  cmd->n_options = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the cli command free operation.
 *
 * @param[in,out] cmd CLI command structure to release
 */
void cli_command_free(struct CliCommand *cmd) {
  size_t i;
  if (!cmd)
    return;
  if (cmd->name)
    free(cmd->name);
  if (cmd->options) {
    for (i = 0; i < cmd->n_options; ++i) {
      if (cmd->options[i].long_flag)
        free(cmd->options[i].long_flag);
      if (cmd->options[i].description)
        free(cmd->options[i].description);
      if (cmd->options[i].mapped_struct_field)
        free(cmd->options[i].mapped_struct_field);
    }
    free(cmd->options);
  }
  cmd->name = NULL;
  cmd->options = NULL;
  cmd->n_options = 0;
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
  *_out_val = opt;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the cst extract cli command operation.
 *
 * @param[in] nodes CST node list
 * @param[in] tokens Token list
 * @param[out] cmd CLI command structure to populate
 * @return CDD_C_SUCCESS on success, error enum on failure
 */
cdd_c_error_t cst_extract_cli_command(const struct CstNodeList *nodes,
                                      const struct TokenList *tokens,
                                      struct CliCommand *cmd) {
  size_t i, j;
  int in_getopt = 0;
  cdd_c_error_t rc;

  if (!nodes || !tokens || !cmd)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  (void)cli_command_init(cmd);
  rc = c_cdd_strdup("cli_app", &cmd->name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  /* Look for `while (getopt(...) != -1)` */
  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
      const char *str = (const char *)tokens->tokens[i].start;
      size_t len = tokens->tokens[i].length;
      if (len == 6 && strncmp(str, "getopt", 6) == 0) {
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
                if (rc != CDD_C_SUCCESS)
                  return rc;
                opt->short_flag = flags[k];
                if (k + 1 < flen && flags[k + 1] == ':') {
                  opt->has_arg = 1;
                  k++;
                } else {
                  opt->has_arg = 0;
                }
                rc = c_cdd_strdup("Auto-extracted option", &opt->description);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            }
            break;
          }
        }
      }
    }
  }

  /* Scan for assignments inside switch cases for getopt */
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
                      if (rc != CDD_C_SUCCESS)
                        return rc;
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
  cdd_c_error_t err5, err6;

  g_cdd_alloc_fail = 1;
  err5 = cdd_strndup2("abc", 3, &out);
  g_cdd_alloc_fail = 0;

  g_cdd_alloc_fail = 2;
  (void)cdd_strndup2("abc", 3, &out);
  free(out);
  out = NULL;
  err6 = cdd_strndup2("abc", 3, &out);
  g_cdd_alloc_fail = 0;

  return (cdd_c_error_t)((err1 ^ CDD_C_ERROR_INVALID_ARGUMENT) |
                         (err2 ^ CDD_C_ERROR_INVALID_ARGUMENT) |
                         (err3 ^ CDD_C_ERROR_INVALID_ARGUMENT) |
                         (err4 ^ CDD_C_ERROR_INVALID_ARGUMENT) |
                         (err5 ^ CDD_C_ERROR_MEMORY) |
                         (err6 ^ CDD_C_ERROR_MEMORY));
}
#endif
