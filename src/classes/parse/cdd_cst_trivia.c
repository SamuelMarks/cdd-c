/**
 * @file cdd_cst_trivia.c
 * @brief Concrete Syntax Tree (CST) trivia implementation and utilities.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_cst_trivia.h"
#include "cdd_cst_builder_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief Static newline string for trivia nodes */
static const uint8_t s_trivia_newline[2] = {10, 0};

/**
 * @brief Inspects a CST's base tokens to infer its indentation style (tabs vs
 * spaces and width).
 *
 * @param[in] tree CST tree to inspect.
 * @param[out] out_config Output structure receiving the detected format
 * configuration.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t
cdd_cst_detect_format_config(cdd_cst_tree_t *tree,
                             cdd_cst_format_config_t *out_config) {
  size_t i;
  int tab_count = 0;
  int space_count = 0;
  size_t total_space_indents = 0;
  size_t sum_space_indent = 0;

  if (!tree || !out_config)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Default fallback */
  out_config->use_tabs = 0;
  out_config->indent_width = 2;

  if (!tree->base_tokens)
    return CDD_C_SUCCESS;

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *tok = &tree->base_tokens->tokens[i];
    cdd_trivia_t *t = tok->leading_trivia;
    while (t) {
      if (t->kind == TRIVIA_NEWLINE) {
        size_t j;
        size_t last_nl = 0;
        int found_nl = 0;
        for (j = 0; j < t->length; j++) {
          if (t->start[j] == 10) {
            last_nl = j;
            found_nl = 1;
          }
        }
        if (found_nl && last_nl + 1 < t->length) {
          if (t->start[last_nl + 1] == 9) {
            tab_count++;
          } else if (t->start[last_nl + 1] == ' ') {
            size_t spaces = t->length - (last_nl + 1);
            space_count++;
            if (spaces <= 8) {
              total_space_indents++;
              sum_space_indent += spaces;
            }
          }
        }
      }
      t = t->next;
    }
  }

  if (tab_count > space_count) {
    out_config->use_tabs = 1;
    out_config->indent_width = 1;
  } else if (total_space_indents > 0) {
    size_t avg;
    out_config->use_tabs = 0;
    avg = sum_space_indent / total_space_indents;
    if (avg >= 3)
      out_config->indent_width = 4;
    else
      out_config->indent_width = 2;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Generates a newline + indentation trivia chain for a given indentation
 * level.
 *
 * @param[in,out] tree Optional CST tree to pool indentation whitespace in.
 * @param[in] config Formatting configuration specifying tabs vs spaces and
 * width.
 * @param[in] indent_level Nesting indentation level.
 * @param[out] out_trivia Output pointer receiving the newly allocated trivia
 * chain.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t
cdd_cst_generate_indent_trivia(cdd_cst_tree_t *tree,
                               const cdd_cst_format_config_t *config,
                               size_t indent_level, cdd_trivia_t **out_trivia) {
  cdd_trivia_t *nl;
  cdd_trivia_t *ws;
  size_t total_spaces;
  uint8_t *ws_buf;

  if (!config || !out_trivia)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  nl = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
  if (!nl) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
    return CDD_C_ERROR_MEMORY;
  }
  nl->kind = TRIVIA_NEWLINE;
  nl->start = s_trivia_newline;
  nl->length = 1;

  if (indent_level == 0) {
    *out_trivia = nl;
    return CDD_C_SUCCESS;
  }

  ws = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
  if (!ws) {
    C_CDD_FREE(nl);
    return CDD_C_ERROR_MEMORY;
  }

  total_spaces = config->indent_width * indent_level;
  ws_buf = (uint8_t *)C_CDD_MALLOC(total_spaces + 1);
  if (!ws_buf) {
    C_CDD_FREE(nl);
    C_CDD_FREE(ws);
    return CDD_C_ERROR_MEMORY;
  }

  if (config->use_tabs) {
    memset(ws_buf, 9, total_spaces);
  } else {
    memset(ws_buf, ' ', total_spaces);
  }
  ws_buf[total_spaces] = 0;

  if (tree) {
    const char *pooled = NULL;
    if (cdd_cst_pool_string(tree, (const char *)ws_buf, &pooled) !=
        CDD_C_SUCCESS) {
      C_CDD_FREE(ws_buf);
      C_CDD_FREE(nl);
      C_CDD_FREE(ws);
      return CDD_C_ERROR_MEMORY;
    }
    C_CDD_FREE(ws_buf);
    ws->start = (const uint8_t *)pooled;
  } else {
    ws->start = ws_buf;
  }
  ws->kind = TRIVIA_WHITESPACE;
  ws->length = total_spaces;

  nl->next = ws;
  *out_trivia = nl;
  return CDD_C_SUCCESS;
}
