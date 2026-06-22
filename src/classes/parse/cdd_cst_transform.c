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

int cdd_transform_extern_c(cdd_cst_tree_t *tree,
                           const cdd_transform_config_t *config) {
  int rc;
  cdd_cst_query_result_t res;
  size_t i;
  int rc;
  int found_cpp = 0;
  cdd_cst_node_t *insert_after_node = NULL;
  (void)config;

  if (!tree || !tree->root) {
    rc = EINVAL;
    return rc;
  }

  /* 1. Check if __cplusplus is already checked */
  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_PREPROC_DIRECTIVE, &res);
  if (rc == 0) {
    for (i = 0; i < res.size; i++) {
      cdd_cst_node_t *dir = res.nodes[i];
      if (dir->num_children > 0 &&
          dir->children[0].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = dir->children[0].val.token;
        if (tok->kind == CDD_TOKEN_PREPROC_IFDEF) {
          if (dir->num_children > 1 &&
              dir->children[1].kind == CDD_CST_CHILD_TOKEN) {
            cdd_token_t *ident = dir->children[1].val.token;
            if (ident->length == 11 &&
                memcmp(ident->start, "__cplusplus", 11) == 0) {
              found_cpp = 1;
              break;
            }
          }
        }
      }
    }
    free(res.nodes);
  }

  if (found_cpp)
    return 0;

  /* 2. Find insertion boundary: After #includes and comments, before C
   * declarations */
  for (i = 0; i < tree->root->num_children; i++) {
    cdd_cst_node_t *child = tree->root->children[i].val.node;
    if (child->kind == CDD_CST_PREPROC_DIRECTIVE) {
      if (child->num_children > 0 &&
          child->children[0].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = child->children[0].val.token;
        if (tok->kind == CDD_TOKEN_PREPROC_INCLUDE) {
          insert_after_node = child;
        }
      }
    } else if (child->kind == CDD_CST_DECLARATION ||
               child->kind == CDD_CST_FUNCTION_DEFINITION) {
      break;
    }
  }

  /* 3. Synthesize Top Nodes */
  {
    cdd_cst_tree_t *top_tree = NULL;
    const char *top_str =
        "\n#ifdef __cplusplus\nextern \"C\" {\n#endif /* __cplusplus */\n";
    if (cdd_cst_parse(az_span_create_from_str((char *)top_str), &top_tree) ==
        0) {
      if (top_tree->root->num_children > 0) {
        cdd_cst_node_t *cloned = NULL;
        /* Insert all children of the parsed fragment */
        if (cdd_cst_clone_tree(tree, top_tree->root->children[0].val.node,
                               &cloned) == 0) {
          if (insert_after_node) {
            cdd_cst_insert_node_after(insert_after_node, cloned);
          } else if (tree->root->num_children > 0) {
            cdd_cst_insert_node_before(tree->root->children[0].val.node,
                                       cloned);
          }
        }
      }
      cdd_cst_tree_free(top_tree);
    }
  }

  /* 4. Synthesize Bottom Nodes */
  {
    cdd_cst_tree_t *bot_tree = NULL;
    const char *bot_str = "\n#ifdef __cplusplus\n}\n#endif /* __cplusplus */\n";
    if (cdd_cst_parse(az_span_create_from_str((char *)bot_str), &bot_tree) ==
        0) {
      if (bot_tree->root->num_children > 0) {
        cdd_cst_node_t *cloned = NULL;
        if (cdd_cst_clone_tree(tree, bot_tree->root->children[0].val.node,
                               &cloned) == 0) {
          if (tree->root->num_children > 0) {
            cdd_cst_node_t *last_node =
                tree->root->children[tree->root->num_children - 1].val.node;
            cdd_cst_insert_node_after(last_node, cloned);
          }
        }
      }
      cdd_cst_tree_free(bot_tree);
    }
  }

  return 0;
}

int cdd_transform_msvc(cdd_cst_tree_t *tree,
                       const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i;
  int rc;
  (void)config;

  if (!tree || !tree->root) {
    rc = EINVAL;
    return rc;
  }

  /* 1. Wrap unistd.h and sys/time.h */
  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_PREPROC_DIRECTIVE, &res);
  if (rc == 0) {
    for (i = 0; i < res.size; i++) {
      cdd_cst_node_t *dir = res.nodes[i];
      if (dir->num_children > 0 &&
          dir->children[0].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = dir->children[0].val.token;
        if (tok->kind == CDD_TOKEN_PREPROC_INCLUDE && dir->num_children > 1 &&
            dir->children[1].kind == CDD_CST_CHILD_TOKEN) {
          cdd_token_t *inc_tok = dir->children[1].val.token;
          if (inc_tok->length == 10 &&
              memcmp(inc_tok->start, "<unistd.h>", 10) == 0) {
            cdd_cst_tree_t *wrap_tree = NULL;
            if (cdd_cst_parse(
                    az_span_create_from_str("#ifndef _MSC_VER\n#include "
                                            "<unistd.h>\n#else\n#include "
                                            "\"win_compat_sym.h\"\n#endif\n"),
                    &wrap_tree) == 0) {
              if (wrap_tree->root->num_children > 0) {
                cdd_cst_node_t *cloned = NULL;
                rc = cdd_cst_clone_tree(
                    tree, wrap_tree->root->children[0].val.node, &cloned);
                if (rc == 0) {
                  rc = cdd_cst_replace_node(tree, dir, cloned);
                  if (rc == 0)
                    cdd_cst_free_node(dir);
                  if (rc != 0) {
                    /* Handle error */
                    fprintf(stderr, "Error replacing node: %d\n", rc);
                  }
                } else {
                  fprintf(stderr, "Error cloning node: %d\n", rc);
                }
              }
              cdd_cst_tree_free(wrap_tree);
            }
          } else if (inc_tok->length == 12 &&
                     memcmp(inc_tok->start, "<sys/time.h>", 12) == 0) {
            cdd_cst_tree_t *wrap_tree = NULL;
            if (cdd_cst_parse(
                    az_span_create_from_str("#ifndef _MSC_VER\n#include "
                                            "<sys/time.h>\n#else\n#include "
                                            "\"win_compat_sym.h\"\n#endif\n"),
                    &wrap_tree) == 0) {
              if (wrap_tree->root->num_children > 0) {
                cdd_cst_node_t *cloned = NULL;
                rc = cdd_cst_clone_tree(
                    tree, wrap_tree->root->children[0].val.node, &cloned);
                if (rc == 0) {
                  rc = cdd_cst_replace_node(tree, dir, cloned);
                  if (rc == 0)
                    cdd_cst_free_node(dir);
                  if (rc != 0) {
                    /* Handle error */
                    fprintf(stderr, "Error replacing node: %d\n", rc);
                  }
                } else {
                  fprintf(stderr, "Error cloning node: %d\n", rc);
                }
              }
              cdd_cst_tree_free(wrap_tree);
            }
          }
        }
      }
    }
    free(res.nodes);
  }

  /* 2. Traverse tokens and replace POSIX identifiers directly */
  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->kind == CDD_TOKEN_IDENTIFIER) {
      if (t->length == 10 && memcmp(t->start, "strcasecmp", 10) == 0) {
        t->start = (const uint8_t *)"_stricmp";
        t->length = 8;
      } else if (t->length == 11 && memcmp(t->start, "strncasecmp", 11) == 0) {
        t->start = (const uint8_t *)"_strnicmp";
        t->length = 9;
      } else if (t->length == 6 && memcmp(t->start, "strdup", 6) == 0) {
        t->start = (const uint8_t *)"_strdup";
        t->length = 7;
      } else if (t->length == 7 && memcmp(t->start, "ssize_t", 7) == 0) {
        t->start = (const uint8_t *)"SSIZE_T";
        t->length = 7;
      } else if (t->length == 16 &&
                 memcmp(t->start, "__builtin_expect", 16) == 0) {
        t->start = (const uint8_t *)"cdd_builtin_expect";
        t->length = 18;
      }
    }
  }

  return 0;
}

int cdd_transform_gnu(cdd_cst_tree_t *tree,
                      const cdd_transform_config_t *config) {
  size_t i;
  (void)config;

  if (!tree || !tree->root) {
    rc = EINVAL;
    return rc;
  }

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
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
            sprintf_s(buf, sizeof(buf), "*%.*s = alloca(%.*s * sizeof(*%.*s));",
                      (int)prev->length, prev->start, (int)next->length,
                      next->start, (int)prev->length, prev->start);
#else
            sprintf(buf, "*%.*s = alloca(%.*s * sizeof(*%.*s));",
                    (int)prev->length, prev->start, (int)next->length,
                    next->start, (int)prev->length, prev->start);
#endif
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
