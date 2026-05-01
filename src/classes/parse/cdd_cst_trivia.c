/* clang-format off */
#include "cdd_cst_trivia.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/log.h"
/* clang-format on */

int cdd_cst_detect_format_config(cdd_cst_tree_t *tree,
                                 cdd_cst_format_config_t *out_config) {
  size_t i;
  int tab_count = 0;
  int space_count = 0;
  size_t total_space_indents = 0;
  size_t sum_space_indent = 0;

  if (!tree || !out_config)
    return EINVAL;

  /* Default fallback */
  out_config->use_tabs = 0;
  out_config->indent_width = 2;

  if (!tree->base_tokens)
    return 0;

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *tok = &tree->base_tokens->tokens[i];
    cdd_trivia_t *t = tok->leading_trivia;
    while (t) {
      if (t->kind == TRIVIA_NEWLINE) {
        /* Find the last newline character in this trivia */
        size_t j;
        size_t last_nl = 0;
        int found_nl = 0;
        for (j = 0; j < t->length; j++) {
          if (t->start[j] == '\n') {
            last_nl = j;
            found_nl = 1;
          }
        }
        if (found_nl && last_nl + 1 < t->length) {
          /* There is whitespace after the newline */
          if (t->start[last_nl + 1] == '\t') {
            tab_count++;
          } else if (t->start[last_nl + 1] == ' ') {
            size_t spaces = t->length - (last_nl + 1);
            space_count++;
            if (spaces > 0 && spaces <= 8) {
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
    out_config->use_tabs = 0; /* Try to guess standard width (2, 4, 8) */
    avg = sum_space_indent / total_space_indents;
    if (avg >= 3)
      out_config->indent_width = 4;
    else
      out_config->indent_width = 2;
  }

  return 0;
}

int cdd_cst_generate_indent_trivia(cdd_cst_tree_t *tree,
                                   const cdd_cst_format_config_t *config,
                                   size_t indent_level,
                                   cdd_trivia_t **out_trivia) {
  cdd_trivia_t *nl;
  cdd_trivia_t *ws;
  size_t total_spaces;
  uint8_t *ws_buf;

  (void)tree;
  if (!config || !out_trivia)
    return EINVAL;

  nl = (cdd_trivia_t *)calloc(1, sizeof(cdd_trivia_t));
  if (!nl) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }
  nl->kind = TRIVIA_NEWLINE;
  nl->start = (const uint8_t *)"\n";
  nl->length = 1;

  if (indent_level == 0) {
    *out_trivia = nl;
    return 0;
  }

  ws = (cdd_trivia_t *)calloc(1, sizeof(cdd_trivia_t));
  if (!ws) {
    free(nl);
    return ENOMEM;
  }

  total_spaces = config->indent_width * indent_level;
  ws_buf = (uint8_t *)malloc(total_spaces);
  if (!ws_buf) {
    free(nl);
    free(ws);
    return ENOMEM;
  }

  if (config->use_tabs) {
    memset(ws_buf, '\t', total_spaces);
  } else {
    memset(ws_buf, ' ', total_spaces);
  }

  ws->kind = TRIVIA_WHITESPACE;
  ws->start = ws_buf;
  ws->length = total_spaces;

  nl->next = ws;
  *out_trivia = nl;
  return 0;
}
