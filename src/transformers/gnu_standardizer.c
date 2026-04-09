/* clang-format off */
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include "c_str_span.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

int cdd_transform_gnu(cdd_cst_tree_t *tree,
                      const cdd_transform_config_t *config) {
  size_t i;
  (void)config;

  if (!tree || !tree->root)
    return EINVAL;

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *tok = &tree->base_tokens->tokens[i];
    if (tok->kind == CDD_TOKEN_IDENTIFIER && tok->length == 13 &&
        memcmp(tok->start, "__attribute__", 13) == 0) {
      if (i + 5 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
          tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_LPAREN &&
          tree->base_tokens->tokens[i + 4].kind == CDD_TOKEN_RPAREN &&
          tree->base_tokens->tokens[i + 5].kind == CDD_TOKEN_RPAREN) {

        cdd_token_t *attr = &tree->base_tokens->tokens[i + 3];
        if (attr->length == 8 && memcmp(attr->start, "noreturn", 8) == 0) {
          tok->start = (const uint8_t *)"_Noreturn";
          tok->length = 9;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
          tree->base_tokens->tokens[i + 4].length = 0;
          tree->base_tokens->tokens[i + 5].length = 0;
        } else if (attr->length == 6 && memcmp(attr->start, "unused", 6) == 0) {
          tok->start = (const uint8_t *)"/* unused */";
          tok->length = 12;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
          tree->base_tokens->tokens[i + 4].length = 0;
          tree->base_tokens->tokens[i + 5].length = 0;
        } else if (attr->length == 6 && memcmp(attr->start, "packed", 6) == 0) {
          tok->start = (const uint8_t *)"\n#pragma pack(push, 1)\n";
          tok->length = 23;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
          tree->base_tokens->tokens[i + 4].length = 0;
          tree->base_tokens->tokens[i + 5].length = 0;

          /* scan forward for closing brace and semicolon */
          {
            size_t k;
            int depth = 0;
            for (k = i; k < tree->base_tokens->size; k++) {
              if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LBRACE)
                depth++;
              else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_RBRACE)
                depth--;
              else if (depth == 0 && tree->base_tokens->tokens[k].kind ==
                                         CDD_TOKEN_SEMICOLON) {
                tree->base_tokens->tokens[k].start =
                    (const uint8_t *)";\n#pragma pack(pop)\n";
                tree->base_tokens->tokens[k].length = 19;
                break;
              }
            }
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_IDENTIFIER) {
      if (tok->length == 13 && memcmp(tok->start, "__extension__", 13) == 0) {
        tok->start = (const uint8_t *)"";
        tok->length = 0;
      } else if (tok->length == 11 &&
                 memcmp(tok->start, "__alignof__", 11) == 0) {
        tok->start = (const uint8_t *)"_Alignof";
        tok->length = 8;
      }
    }
  }

  /* Unroll statement expressions and VLAs */
  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->kind == CDD_TOKEN_LPAREN && i + 1 < tree->base_tokens->size &&
        tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
      /* Remove ({ */
      t->start = (const uint8_t *)"";
      t->length = 0;
      tree->base_tokens->tokens[i + 1].start = (const uint8_t *)"";
      tree->base_tokens->tokens[i + 1].length = 0;
    } else if (t->kind == CDD_TOKEN_RBRACE && i + 1 < tree->base_tokens->size &&
               tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_RPAREN) {
      /* Remove }) */
      t->start = (const uint8_t *)"";
      t->length = 0;
      tree->base_tokens->tokens[i + 1].start = (const uint8_t *)"";
      tree->base_tokens->tokens[i + 1].length = 0;
    } else if (t->kind == CDD_TOKEN_LBRACKET && i > 0) {
      cdd_token_t *prev = &tree->base_tokens->tokens[i - 1];
      cdd_token_t *next = &tree->base_tokens->tokens[i + 1];
      cdd_token_t *semi = &tree->base_tokens->tokens[i + 2];
      if (prev->kind == CDD_TOKEN_IDENTIFIER &&
          next->kind == CDD_TOKEN_IDENTIFIER &&
          semi->kind == CDD_TOKEN_RBRACKET) {
        /* This is an array declaration, possibly VLA. Let's see if it ends with
         * semicolon */
        size_t k;
        cdd_token_t *semi_tok = NULL;
        for (k = i + 2; k < tree->base_tokens->size; k++) {
          if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON) {
            semi_tok = &tree->base_tokens->tokens[k];
            break;
          }
        }
        if (semi_tok) {
          /* Found VLA pattern `int arr[n];` */
          /* Find type specifier before identifier */
          if (i >= 2) {
            char buf[256];
            char *heap_buf;
            sprintf(buf, "*%.*s = alloca(%.*s * sizeof(*%.*s));",
                    (int)prev->length, prev->start, (int)next->length,
                    next->start, (int)prev->length, prev->start);
            heap_buf = strdup(buf);
            prev->start = (const uint8_t *)heap_buf;
            prev->length = strlen(heap_buf);
            t->length = 0;
            next->length = 0;
            semi->length = 0;
          }
        }
      }
    } else if (t->kind == CDD_TOKEN_LBRACE && i + 1 < tree->base_tokens->size &&
               tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_RBRACE) {
      tree->base_tokens->tokens[i + 1].start = (const uint8_t *)" char _pad; }";
      tree->base_tokens->tokens[i + 1].length = 13;
    }
  }

  return 0;
}
