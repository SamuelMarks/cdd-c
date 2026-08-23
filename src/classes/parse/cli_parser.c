/* clang-format off */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/parse/cli_parser.h"
/* clang-format on */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

/**
 * @brief Executes the cdd strndup2 operation.
 */
static cdd_c_error_t cdd_strndup2(const char *s, size_t n, char **_out_val) {
  char *result;
  size_t len = 0;
  while (len < n && s[len] != '\0')
    len++;
  result = malloc(len + 1);
  if (!result) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  memcpy(result, s, len);
  result[len] = '\0';
  {
    *_out_val = result;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the cli command init operation.
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
 * @brief Adds or sets option.
 */
static cdd_c_error_t add_option(struct CliCommand *cmd,
                                struct CliOption **_out_val) {
  struct CliOption *opt;
  cmd->options =
      realloc(cmd->options, (cmd->n_options + 1) * sizeof(struct CliOption));
  opt = &cmd->options[cmd->n_options++];
  memset(opt, 0, sizeof(*opt));
  {
    *_out_val = opt;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the cst extract cli command operation.
 */
cdd_c_error_t cst_extract_cli_command(const struct CstNodeList *nodes,
                                      const struct TokenList *tokens,
                                      struct CliCommand *cmd) {
  size_t i, j;
  int in_getopt = 0;
  if (!nodes || !tokens || !cmd)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (cli_command_init(cmd) != CDD_C_SUCCESS)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  cmd->name = strdup("cli_app");

  /* Very naive parsing: Look for `while (getopt(...) != -1)`
     or switch cases with char literals. */
  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
      const char *str = (const char *)tokens->tokens[i].start;
      size_t len = tokens->tokens[i].length;
      if (len == 6 && strncmp(str, "getopt", 6) == 0) {
        in_getopt = 1;
        /* Next string literal usually has short flags */
        for (j = i + 1; j < i + 10 && j < tokens->size; ++j) {
          if (tokens->tokens[j].kind == TOKEN_STRING_LITERAL) {
            const char *flags =
                (const char *)tokens->tokens[j].start + 1; /* skip quote */
            size_t flen = tokens->tokens[j].length - 2;
            size_t k;
            for (k = 0; k < flen; ++k) {
              if (isalpha(flags[k])) {
                struct CliOption *opt = NULL;
                if (add_option(cmd, &opt) != CDD_C_SUCCESS)
                  return CDD_C_ERROR_MEMORY;
                opt->short_flag = flags[k];
                if (k + 1 < flen && flags[k + 1] == ':') {
                  opt->has_arg = 1;
                  k++;
                } else {
                  opt->has_arg = 0;
                }
                opt->description = strdup("Auto-extracted option");
              }
            }
            break;
          }
        }
      }
    }
  }

  /* Scan for assignments inside switch cases for getopt?
     This is complex and AST dependent, but we can look for `case 'h':` and then
     an assignment. */
  if (in_getopt) {
    for (i = 0; i < tokens->size; ++i) {
      if (tokens->tokens[i].kind == TOKEN_KEYWORD_CASE) {
        size_t next_idx = i + 1;
        while (next_idx < tokens->size &&
               tokens->tokens[next_idx].kind == TOKEN_WHITESPACE)
          next_idx++;
        if (next_idx < tokens->size &&
            tokens->tokens[next_idx].kind == TOKEN_CHAR_LITERAL) {
          char c = tokens->tokens[next_idx].start[1];
          size_t opt_idx;
          for (opt_idx = 0; opt_idx < cmd->n_options; ++opt_idx) {
            if (cmd->options[opt_idx].short_flag == c) {
              /* Try to find assignment `config->val = ...` before `break` */
              for (j = i + 2; j < tokens->size &&
                              tokens->tokens[j].kind != TOKEN_KEYWORD_BREAK;
                   ++j) {
                if (tokens->tokens[j].kind == TOKEN_ASSIGN) {
                  size_t prev_idx = j > 0 ? j - 1 : 0;
                  while (prev_idx > 0 &&
                         tokens->tokens[prev_idx].kind == TOKEN_WHITESPACE) {
                    prev_idx--;
                  }
                  printf("DEBUG: j=%zu, prev_idx=%zu, kind=%d, len=%zu\n", j,
                         prev_idx, tokens->tokens[prev_idx].kind,
                         tokens->tokens[prev_idx].length);
                  if (tokens->tokens[prev_idx].kind == TOKEN_IDENTIFIER) {
                    cdd_strndup2((const char *)tokens->tokens[prev_idx].start,
                                 tokens->tokens[prev_idx].length,
                                 &cmd->options[opt_idx].mapped_struct_field);
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

  return CDD_C_SUCCESS;
}
