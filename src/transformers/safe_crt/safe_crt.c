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

static void extract_token_range_str(cdd_cst_node_t *stmt, size_t start_idx,

                                    size_t end_idx, char *out_str,

                                    size_t max_len) {

  cdd_token_t *t_start = stmt->children[start_idx].val.token;

  cdd_token_t *t_end = stmt->children[end_idx].val.token;

  size_t len = (t_end->start + t_end->length) - t_start->start;

  if (len >= max_len)

    len = max_len - 1;

  memcpy(out_str, t_start->start, len);

  out_str[len] = '\0';
}

static void get_indent_string(cdd_token_t *tok, char *out_indent) {

  cdd_trivia_t *tr = tok->leading_trivia;

  cdd_trivia_t *last_ws = NULL;

  out_indent[0] = '\0';

  while (tr) {

    if (tr->kind == TRIVIA_WHITESPACE) {

      last_ws = tr;

    } else if (tr->kind == TRIVIA_NEWLINE) {

      last_ws = NULL;
    }

    tr = tr->next;
  }

  if (last_ws) {

    size_t len = last_ws->length < 63 ? last_ws->length : 63;

    memcpy(out_indent, last_ws->start, len);

    out_indent[len] = '\0';

  } else {

    strcpy(out_indent, "  ");
  }
}

static int replace_safe_crt(cdd_cst_tree_t *tree, cdd_cst_node_t *stmt,

                            const char *msc_ver_stmt, const char *else_stmt,

                            const char *indent) {

  char *buf = (char *)calloc(1, 4096);

  cdd_cst_tree_t *syn_tree = NULL;

  int rc;

  cdd_cst_node_t *cloned;

  if (!buf)

    return ENOMEM;

  sprintf(buf,

          "\n#if defined(_MSC_VER)\n%s%s\n#else\n%s%s\n#endif\n",

          indent, msc_ver_stmt, indent, else_stmt);

  rc = cdd_cst_parse(az_span_create_from_str(buf), &syn_tree);

  if (rc != 0) {

    free(buf);

    return rc;
  }

  if (syn_tree->root->num_children > 0) {

    cdd_cst_node_t *prev = stmt;

    size_t k;

    for (k = 0; k < syn_tree->root->num_children; k++) {

      if (syn_tree->root->children[k].kind == CDD_CST_CHILD_NODE) {

        if (cdd_cst_node_clone(tree, syn_tree->root->children[k].val.node,

                               &cloned) == 0) {

          if (k == 0) {

            cdd_token_t *cloned_first =

                cloned->num_children > 0

                    ? (cloned->children[0].kind == CDD_CST_CHILD_TOKEN

                           ? cloned->children[0].val.token

                           : NULL)

                    : NULL;

            if (cloned_first)

              cloned_first->leading_trivia = NULL;

            rc = cdd_cst_node_replace(tree, stmt, cloned);

            prev = cloned;

          } else {

            rc = cdd_cst_node_insert_after(prev, cloned);

            prev = cloned;
          }
        }
      }
    }
  }

  cdd_cst_tree_free(syn_tree);

  /* Intentionally leaking buf since cloned tokens point into it. */

  return rc;
}

int cdd_transform_safe_crt(cdd_cst_tree_t *tree,

                           const cdd_transform_config_t *config) {

  cdd_cst_query_result_t res;

  size_t i;

  int rc;

  (void)config;

  if (!tree || !tree->root)

    return EINVAL;

  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_UNKNOWN, &res);

  if (rc != 0)

    return rc;

  for (i = 0; i < res.size; i++) {

    cdd_cst_node_t *stmt = res.nodes[i];

    size_t j;

    int paren_depth = 0;

    int found_func = 0; /* 1: strcpy, 2: strncpy, 3: sprintf, 4: fopen, 5:

                           strcat, 6: strncat, 7: memcpy, 8: memmove */

    size_t dest_start = (size_t)-1, dest_end = (size_t)-1;

    size_t src_start = (size_t)-1, src_end = (size_t)-1;

    size_t size_start = (size_t)-1, size_end = (size_t)-1;

    size_t assign_idx = (size_t)-1;

    cdd_token_t *first_tok = NULL;

    for (j = 0; j < stmt->num_children; j++) {

      if (stmt->children[j].kind == CDD_CST_CHILD_TOKEN) {

        cdd_token_t *t = stmt->children[j].val.token;

        if (!first_tok)

          first_tok = t;

        if (!found_func) {

          if (t->kind == CDD_TOKEN_ASSIGN) {

            assign_idx = j;

          } else if (t->kind == CDD_TOKEN_IDENTIFIER) {

            if (t->length == 6 && memcmp(t->start, "strcpy", 6) == 0 &&

                assign_idx == (size_t)-1) {

              found_func = 1;

            } else if (t->length == 7 && memcmp(t->start, "strncpy", 7) == 0 &&

                       assign_idx == (size_t)-1) {

              found_func = 2;

            } else if (t->length == 7 && memcmp(t->start, "sprintf", 7) == 0 &&

                       assign_idx == (size_t)-1) {

              found_func = 3;

            } else if (t->length == 5 && memcmp(t->start, "fopen", 5) == 0) {

              if (assign_idx != (size_t)-1) {

                found_func = 4;

              } else {

                break; /* bare fopen() call not supported */
              }

            } else if (t->length == 6 && memcmp(t->start, "strcat", 6) == 0 &&

                       assign_idx == (size_t)-1) {

              found_func = 5;

            } else if (t->length == 7 && memcmp(t->start, "strncat", 7) == 0 &&

                       assign_idx == (size_t)-1) {

              found_func = 6;

            } else if (t->length == 6 && memcmp(t->start, "memcpy", 6) == 0 &&

                       assign_idx == (size_t)-1) {

              found_func = 7;

            } else if (t->length == 7 && memcmp(t->start, "memmove", 7) == 0 &&

                       assign_idx == (size_t)-1) {

              found_func = 8;

            } else if (assign_idx == (size_t)-1) {

              /* allow identifiers before assignment */
            }

          } else if (assign_idx == (size_t)-1) {

            /* allow other tokens like f = before fopen */
          }

        } else {

          if (t->kind == CDD_TOKEN_LPAREN) {

            if (paren_depth == 0)

              dest_start = j + 1;

            paren_depth++;

          } else if (t->kind == CDD_TOKEN_RPAREN) {

            paren_depth--;

            if (paren_depth == 0) {

              if (found_func == 1 || found_func == 3 || found_func == 4 ||

                  found_func == 5) {

                src_end = j - 1;

              } else if (found_func == 2 || found_func == 6 ||

                         found_func == 7 || found_func == 8) {

                size_end = j - 1;
              }
            }

          } else if (t->kind == CDD_TOKEN_COMMA && paren_depth == 1) {

            if (found_func == 1 || found_func == 3 || found_func == 4 ||

                found_func == 5) {

              if (dest_end == (size_t)-1) {

                dest_end = j - 1;

                src_start = j + 1;
              }

            } else if (found_func == 2 || found_func == 6 || found_func == 7 ||

                       found_func == 8) {

              if (dest_end == (size_t)-1) {

                dest_end = j - 1;

                src_start = j + 1;

              } else if (src_end == (size_t)-1) {

                src_end = j - 1;

                size_start = j + 1;
              }
            }

          } else if (t->kind == CDD_TOKEN_SEMICOLON && paren_depth == 0) {

            /* end */
          }
        }
      }
    }

    if (found_func && paren_depth == 0 && first_tok) {

      char indent[64];

      char dest_str[1024] = {0};

      char src_str[1024] = {0};

      char size_str[1024] = {0};

      char var_str[1024] = {0};

      char msc_stmt[2048];

      char else_stmt[2048];

      get_indent_string(first_tok, indent);

      if (found_func == 1 && dest_end != (size_t)-1 && src_end != (size_t)-1) {

        extract_token_range_str(stmt, dest_start, dest_end, dest_str,

                                sizeof(dest_str));

        extract_token_range_str(stmt, src_start, src_end, src_str,

                                sizeof(src_str));

        sprintf(msc_stmt, "strcpy_s(%s, sizeof(%s), %s);",

                dest_str, dest_str, src_str);

        sprintf(else_stmt, "strcpy(%s, %s);", dest_str,

                src_str);

        replace_safe_crt(tree, stmt, msc_stmt, else_stmt, indent);

      } else if (found_func == 2 && dest_end != (size_t)-1 &&

                 src_end != (size_t)-1 && size_end != (size_t)-1) {

        extract_token_range_str(stmt, dest_start, dest_end, dest_str,

                                sizeof(dest_str));

        extract_token_range_str(stmt, src_start, src_end, src_str,

                                sizeof(src_str));

        extract_token_range_str(stmt, size_start, size_end, size_str,

                                sizeof(size_str));

        sprintf(msc_stmt, "strncpy_s(%s, %s + 1, %s, %s);",

                dest_str, size_str, src_str, size_str);

        sprintf(else_stmt, "strncpy(%s, %s, %s);", dest_str,

                src_str, size_str);

        replace_safe_crt(tree, stmt, msc_stmt, else_stmt, indent);

      } else if (found_func == 3 && dest_end != (size_t)-1 &&

                 src_end != (size_t)-1) {

        extract_token_range_str(stmt, dest_start, dest_end, dest_str,

                                sizeof(dest_str));

        extract_token_range_str(stmt, src_start, src_end, src_str,

                                sizeof(src_str));

        sprintf(msc_stmt, "sprintf_s(%s, sizeof(%s), %s);",

                dest_str, dest_str, src_str);

        sprintf(else_stmt, "sprintf(%s, %s);", dest_str,

                src_str);

        replace_safe_crt(tree, stmt, msc_stmt, else_stmt, indent);

      } else if (found_func == 4 && dest_end != (size_t)-1 &&

                 src_end != (size_t)-1) {

        size_t var_start = 0;

        size_t var_end = assign_idx - 1;

        while (var_start < var_end &&

               stmt->children[var_start].kind == CDD_CST_CHILD_TOKEN &&

               stmt->children[var_start].val.token->kind == CDD_TOKEN_OTHER)

          var_start++;

        extract_token_range_str(stmt, var_start, var_end, var_str,

                                sizeof(var_str));

        extract_token_range_str(stmt, dest_start, dest_end, dest_str,

                                sizeof(dest_str));

        extract_token_range_str(stmt, src_start, src_end, src_str,

                                sizeof(src_str));

        sprintf(msc_stmt, "fopen_s(&%s, %s, %s);", var_str,

                dest_str, src_str);

        sprintf(else_stmt, "%s = fopen(%s, %s);", var_str,

                dest_str, src_str);

        replace_safe_crt(tree, stmt, msc_stmt, else_stmt, indent);

      } else if (found_func == 5 && dest_end != (size_t)-1 &&

                 src_end != (size_t)-1) {

        extract_token_range_str(stmt, dest_start, dest_end, dest_str,

                                sizeof(dest_str));

        extract_token_range_str(stmt, src_start, src_end, src_str,

                                sizeof(src_str));

        sprintf(msc_stmt, "strcat_s(%s, sizeof(%s), %s);",

                dest_str, dest_str, src_str);

        sprintf(else_stmt, "strcat(%s, %s);", dest_str,

                src_str);

        replace_safe_crt(tree, stmt, msc_stmt, else_stmt, indent);

      } else if (found_func == 6 && dest_end != (size_t)-1 &&

                 src_end != (size_t)-1 && size_end != (size_t)-1) {

        extract_token_range_str(stmt, dest_start, dest_end, dest_str,

                                sizeof(dest_str));

        extract_token_range_str(stmt, src_start, src_end, src_str,

                                sizeof(src_str));

        extract_token_range_str(stmt, size_start, size_end, size_str,

                                sizeof(size_str));

        sprintf(msc_stmt, "strncat_s(%s, %s + 1, %s, %s);",

                dest_str, size_str, src_str, size_str);

        sprintf(else_stmt, "strncat(%s, %s, %s);", dest_str,

                src_str, size_str);

        replace_safe_crt(tree, stmt, msc_stmt, else_stmt, indent);

      } else if (found_func == 7 && dest_end != (size_t)-1 &&

                 src_end != (size_t)-1 && size_end != (size_t)-1) {

        extract_token_range_str(stmt, dest_start, dest_end, dest_str,

                                sizeof(dest_str));

        extract_token_range_str(stmt, src_start, src_end, src_str,

                                sizeof(src_str));

        extract_token_range_str(stmt, size_start, size_end, size_str,

                                sizeof(size_str));

        sprintf(msc_stmt, "memcpy_s(%s, %s, %s, %s);",

                dest_str, size_str, src_str, size_str);

        sprintf(else_stmt, "memcpy(%s, %s, %s);", dest_str,

                src_str, size_str);

        replace_safe_crt(tree, stmt, msc_stmt, else_stmt, indent);

      } else if (found_func == 8 && dest_end != (size_t)-1 &&

                 src_end != (size_t)-1 && size_end != (size_t)-1) {

        extract_token_range_str(stmt, dest_start, dest_end, dest_str,

                                sizeof(dest_str));

        extract_token_range_str(stmt, src_start, src_end, src_str,

                                sizeof(src_str));

        extract_token_range_str(stmt, size_start, size_end, size_str,

                                sizeof(size_str));

        sprintf(msc_stmt, "memmove_s(%s, %s, %s, %s);",

                dest_str, size_str, src_str, size_str);

        sprintf(else_stmt, "memmove(%s, %s, %s);", dest_str,

                src_str, size_str);

        replace_safe_crt(tree, stmt, msc_stmt, else_stmt, indent);
      }
    }
  }

  free(res.nodes);

  return 0;
}
