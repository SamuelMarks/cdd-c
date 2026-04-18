/* clang-format off */
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include "classes/parse/numeric.h"
#include "c_str_span.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
/* clang-format on */

static void parse_128_literal(const char *str, size_t len, uint64_t *out_high,
                              uint64_t *out_low) {
  uint64_t high = 0;
  uint64_t low = 0;
  size_t j;
  for (j = 0; j < len; j++) {
    uint64_t d;
    uint64_t low_part, high_part;
    if (str[j] >= '0' && str[j] <= '9') {
      d = (uint64_t)(str[j] - '0');
    } else {
      break; /* Reached suffix or end */
    }
    low_part = low * 10;
    high_part = high * 10 + (low_part < low ? 1 : 0) +
                (low / 1844674407370955161ULL); /* Roughly */
    /* Accurate 128-bit multiply by 10 */
    {
      uint64_t al = low & 0xFFFFFFFF;
      uint64_t ah = low >> 32;
      uint64_t bl = 10;
      uint64_t p00 = al * bl;
      uint64_t p01 = al * 0;
      uint64_t p10 = ah * bl;
      uint64_t mid = p01 + (p00 >> 32) + p10;
      low_part = (mid << 32) | (p00 & 0xFFFFFFFF);
      high_part = high * 10 + (mid >> 32);
    }
    low = low_part + d;
    high = high_part + (low < low_part ? 1 : 0);
  }
  *out_high = high;
  *out_low = low;
}

static void parse_hex_128_literal(const char *str, size_t len,
                                  uint64_t *out_high, uint64_t *out_low) {
  uint64_t high = 0;
  uint64_t low = 0;
  size_t j = 2; /* Skip 0x */
  for (; j < len; j++) {
    uint64_t d;
    if (str[j] >= '0' && str[j] <= '9') {
      d = (uint64_t)(str[j] - '0');
    } else if (str[j] >= 'a' && str[j] <= 'f') {
      d = (uint64_t)(str[j] - 'a' + 10);
    } else if (str[j] >= 'A' && str[j] <= 'F') {
      d = (uint64_t)(str[j] - 'A' + 10);
    } else {
      break; /* Suffix */
    }
    high = (high << 4) | (low >> 60);
    low = (low << 4) | d;
  }
  *out_high = high;
  *out_low = low;
}

/** @brief Struct definition */
struct magic_ctx {
  /** @brief field */
  /** @brief field */
  const uint8_t *func_name;
  /** @brief field */
  /** @brief field */
  size_t func_len;
};

static int magic_visitor(cdd_cst_node_t *node, void *user_data) {
  struct magic_ctx *ctx = (struct magic_ctx *)user_data;
  size_t i;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *tok = node->children[i].val.token;
      if (tok->kind == CDD_TOKEN_IDENTIFIER) {
        if ((tok->length == 12 &&
             memcmp(tok->start, "__FUNCTION__", 12) == 0) ||
            (tok->length == 19 &&
             memcmp(tok->start, "__PRETTY_FUNCTION__", 19) == 0) ||
            (tok->length == 8 && memcmp(tok->start, "__func__", 8) == 0)) {
          char *buf = (char *)malloc(ctx->func_len + 3);
          if (buf) {
            buf[0] = '"';
            memcpy(buf + 1, ctx->func_name, ctx->func_len);
            buf[1 + ctx->func_len] = '"';
            buf[2 + ctx->func_len] = '\0';
            tok->start = (const uint8_t *)buf;
            tok->length = ctx->func_len + 2;
          }
        }
      }
    }
  }
  return 0;
}

/** @brief Struct definition */
struct tramp_ctx {
  /** @brief field */
  /** @brief field */
  const uint8_t *name;
  /** @brief field */
  /** @brief field */
  size_t length;
  /** @brief field */
  /** @brief field */
  int is_tramp;
  /** @brief field */
  /** @brief field */
  cdd_cst_node_t *func_node;
};

static int tramp_visitor(cdd_cst_node_t *node, void *user_data) {
  struct tramp_ctx *ctx = (struct tramp_ctx *)user_data;
  size_t i;
  if (ctx->is_tramp)
    return 0;

  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *tok = node->children[i].val.token;
      if (tok->kind == CDD_TOKEN_IDENTIFIER && tok->length == ctx->length &&
          memcmp(tok->start, ctx->name, ctx->length) == 0) {
        /* Check if it's the declaration name itself.
           If the identifier's parent is the function node we are checking, it's
           the name. */
        if (node == ctx->func_node) {
          continue; /* Skip the definition name itself */
        }

        {
          cdd_token_t *next = tok + 1;
          /* If followed by LPAREN, it's a direct call, not a trampoline */
          while (next->length == 0) {
            next++;
          }
          if (next->kind != CDD_TOKEN_LPAREN) {
            ctx->is_tramp = 1;
            return 1; /* abort traversal */
          }
        }
      }
    }
  }
  return 0;
}

static int asm_visitor(cdd_cst_node_t *node, void *user_data) {
  size_t i;
  (void)user_data;
  if (node->kind == CDD_CST_ASM_STATEMENT) {
    /* Basic validation of constraint strings and symbol references */
    for (i = 0; i < node->num_children; i++) {
      if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = node->children[i].val.token;
        if (tok->kind == CDD_TOKEN_STRING) {
          const uint8_t *s = tok->start;
          size_t len = tok->length;
          /* Check constraints like "r", "m", "memory", "a", "b", "c", "d", "S",
           * "D" */
          if (len > 2 && s[0] == '"') {
            /* ARM and x86 specific constraint logic could be expanded here */
          }
        } else if (tok->kind == CDD_TOKEN_IDENTIFIER) {
          /* Symbol reference binding logic would map this token to scope */
        }
      }
    }
  }
  return 0;
}

int cdd_transform_gnu(cdd_cst_tree_t *tree,
                      const cdd_transform_config_t *config) {
  size_t i;
  cdd_cst_query_result_t res = {0};
  (void)config;

  if (!tree || !tree->root)
    return EINVAL;

  cdd_cst_traverse_preorder(tree->root, asm_visitor, NULL);

  if (cdd_cst_find_nodes_by_type(tree->root, CDD_CST_FUNCTION_DEFINITION,
                                 &res) == 0) {
    for (i = 0; i < res.size; i++) {
      cdd_cst_node_t *func = res.nodes[i];
      size_t j;
      cdd_token_t *name_tok = NULL;
      for (j = 0; j < func->num_children; j++) {
        if (func->children[j].kind == CDD_CST_CHILD_TOKEN) {
          cdd_token_t *tok = func->children[j].val.token;
          if (tok->kind == CDD_TOKEN_LPAREN) {
            if (j > 0 && func->children[j - 1].kind == CDD_CST_CHILD_TOKEN) {
              struct magic_ctx ctx;
              name_tok = func->children[j - 1].val.token;
              ctx.func_name = name_tok->start;
              ctx.func_len = name_tok->length;
              cdd_cst_traverse_preorder(func, magic_visitor, &ctx);
            }
            break;
          }
        }
      }

      /* Detect nested functions and trampolines */
      if (name_tok && func->parent && func->parent->kind == CDD_CST_BLOCK) {
        struct tramp_ctx t_ctx;
        cdd_cst_node_t *parent_func;
        t_ctx.name = name_tok->start;
        t_ctx.length = name_tok->length;
        t_ctx.is_tramp = 0;
        t_ctx.func_node = func;

        /* Traverse the parent function (or the whole translation unit) to find
         * references */
        parent_func = func->parent;
        while (parent_func &&
               parent_func->kind != CDD_CST_FUNCTION_DEFINITION) {
          parent_func = parent_func->parent;
        }
        if (parent_func) {
          cdd_cst_traverse_preorder(parent_func, tramp_visitor, &t_ctx);
        } else {
          cdd_cst_traverse_preorder(tree->root, tramp_visitor, &t_ctx);
        }

        if (t_ctx.is_tramp) {
          /*
           * Lambda Lifting / Closure Conversion logic:
           * This would involve creating a shared closure struct holding local
           * variables and dynamically allocating it, or allocating a trampoline
           * executable thunk. For now, standard C89 cannot safely execute stack
           * trampolines.
           */
          if (res.nodes)
            free(res.nodes);
          return 129; /* ENOTSUP isn't defined everywhere in old MSVC */
        }

      /* Non-trampoline nested function (called directly)
         We can safely lift it out of the function scope to global scope
         if it does not capture local variables.
         Closure conversion would require deep CFG analysis to verify escapes.
      */      }
    }
    if (res.nodes) {
      free(res.nodes);
    }
  }

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *tok = &tree->base_tokens->tokens[i];
    if (tok->kind == CDD_TOKEN_PREPROC_DEFINE) {
      const char *p = (const char *)tok->start;
      size_t len = tok->length;
      char *buf = (char *)malloc(len + 1);
      if (buf) {
        memcpy(buf, p, len);
        buf[len] = '\0';

        {
          char *id_start = buf;
          while (*id_start && *id_start != ' ' && *id_start != '\t')
            id_start++;
          while (*id_start == ' ' || *id_start == '\t')
            id_start++;

          if (*id_start) {
            char *id_end = id_start;
            while (isalnum((unsigned char)*id_end) || *id_end == '_')
              id_end++;

            if (*id_end == '(') {
              char *lparen = id_end;
              char *rparen = strchr(lparen, ')');
              if (rparen) {
                char *ellipsis = strstr(lparen, "...");
                if (ellipsis && ellipsis < rparen) {
                  char *var_name = NULL;
                  size_t var_len = 0;
                  char *start_id = ellipsis;
                  while (start_id > lparen &&
                         (isalnum((unsigned char)start_id[-1]) ||
                          start_id[-1] == '_')) {
                    start_id--;
                  }
                  if (start_id < ellipsis) {
                    var_len = ellipsis - start_id;
                    var_name = (char *)malloc(var_len + 1);
                    memcpy(var_name, start_id, var_len);
                    var_name[var_len] = '\0';
                  } else {
                    var_name = strdup("__VA_ARGS__");
                    var_len = 11;
                  }

                  {
                    size_t out_cap = len * 2 + 128;
                    char *out_buf = (char *)malloc(out_cap);
                    if (out_buf) {
                      char *out_p = out_buf;
                      char *in_p = buf;

                      memcpy(out_p, in_p, start_id - in_p);
                      out_p += (start_id - in_p);
                      in_p = start_id;

                      if (start_id < ellipsis) {
                        memcpy(out_p, "...", 3);
                        out_p += 3;
                        in_p = ellipsis + 3;
                      } else {
                        memcpy(out_p, "...", 3);
                        out_p += 3;
                        in_p = ellipsis + 3;
                      }

                      memcpy(out_p, in_p, (rparen + 1) - in_p);
                      out_p += (rparen + 1) - in_p;
                      in_p = rparen + 1;

                      while (*in_p) {
                        int matched = 0;
                        if (*in_p == ',') {
                          char *t = in_p + 1;
                          while (*t == ' ' || *t == '\t')
                            t++;
                          if (t[0] == '#' && t[1] == '#') {
                            t += 2;
                            while (*t == ' ' || *t == '\t')
                              t++;
                            if (strncmp(t, var_name, var_len) == 0 &&
                                !isalnum((unsigned char)t[var_len]) &&
                                t[var_len] != '_') {
                              strcpy(out_p, " __VA_OPT__(,) __VA_ARGS__");
                              out_p += strlen(" __VA_OPT__(,) __VA_ARGS__");
                              in_p = t + var_len;
                              matched = 1;
                            }
                          }
                        }
                        if (!matched && strncmp(in_p, var_name, var_len) == 0 &&
                            !isalnum((unsigned char)in_p[var_len]) &&
                            in_p[var_len] != '_' &&
                            (in_p == buf ||
                             (!isalnum((unsigned char)in_p[-1]) &&
                              in_p[-1] != '_'))) {
                          strcpy(out_p, "__VA_ARGS__");
                          out_p += strlen("__VA_ARGS__");
                          in_p += var_len;
                          matched = 1;
                        }

                        if (!matched) {
                          *out_p++ = *in_p++;
                        }
                      }
                      *out_p = '\0';
                      tok->start = (const uint8_t *)out_buf;
                      tok->length = out_p - out_buf;
                    }
                  }
                  free(var_name);
                }
              }
            }
          }
        }
        free(buf);
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___INT128) {
      if (i > 0 &&
          tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_IDENTIFIER &&
          tree->base_tokens->tokens[i - 1].length == 8 &&
          memcmp(tree->base_tokens->tokens[i - 1].start, "unsigned", 8) == 0) {
        tree->base_tokens->tokens[i - 1].length = 0;
        tok->start = (const uint8_t *)"cdd_uint128_t";
        tok->length = 13;
      } else {
        tok->start = (const uint8_t *)"cdd_int128_t";
        tok->length = 12;
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___COMPLEX__) {
      /* `__complex__ int` -> `struct { int real, imag; }` */
      if (i + 1 < tree->base_tokens->size) {
        cdd_token_t *next_tok = &tree->base_tokens->tokens[i + 1];
        char *repl = (char *)malloc(256);
        sprintf(repl, "struct { %.*s real, imag; }", (int)next_tok->length,
                next_tok->start);
        tok->start = (const uint8_t *)repl;
        tok->length = strlen(repl);
        next_tok->length = 0;
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___REAL__) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_IDENTIFIER) {
        cdd_token_t *next_tok = &tree->base_tokens->tokens[i + 1];
        char *repl = (char *)malloc(128);
        sprintf(repl, "%.*s.real", (int)next_tok->length, next_tok->start);
        tok->start = (const uint8_t *)repl;
        tok->length = strlen(repl);
        next_tok->length = 0;
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___IMAG__) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_IDENTIFIER) {
        cdd_token_t *next_tok = &tree->base_tokens->tokens[i + 1];
        char *repl = (char *)malloc(128);
        sprintf(repl, "%.*s.imag", (int)next_tok->length, next_tok->start);
        tok->start = (const uint8_t *)repl;
        tok->length = strlen(repl);
        next_tok->length = 0;
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD_TYPEOF) {
      if (i + 6 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
          tree->base_tokens->tokens[i + 3].kind == CDD_TOKEN_LBRACKET &&
          tree->base_tokens->tokens[i + 5].kind == CDD_TOKEN_RBRACKET &&
          tree->base_tokens->tokens[i + 6].kind == CDD_TOKEN_RPAREN) {
        /* typeof(type[size]) array decay behavior check */
        cdd_token_t *type_tok = &tree->base_tokens->tokens[i + 2];
        cdd_token_t *size_tok = &tree->base_tokens->tokens[i + 4];
        char *repl = (char *)malloc(256);
        sprintf(repl,
                "typedef %.*s __cdd_typeof_arr_%d[%.*s]; __cdd_typeof_arr_%d",
                (int)type_tok->length, type_tok->start, (int)i,
                (int)size_tok->length, size_tok->start, (int)i);
        tok->start = (const uint8_t *)repl;
        tok->length = strlen(repl);
        tree->base_tokens->tokens[i + 1].length = 0;
        tree->base_tokens->tokens[i + 2].length = 0;
        tree->base_tokens->tokens[i + 3].length = 0;
        tree->base_tokens->tokens[i + 4].length = 0;
        tree->base_tokens->tokens[i + 5].length = 0;
        tree->base_tokens->tokens[i + 6].length = 0;
      } else if (i + 3 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
                 tree->base_tokens->tokens[i + 3].kind == CDD_TOKEN_RPAREN) {
        cdd_token_t *inner = &tree->base_tokens->tokens[i + 2];
        if (inner->length == 3 && memcmp(inner->start, "int", 3) == 0) {
          tok->start = (const uint8_t *)"int";
          tok->length = 3;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
        } else if (inner->length == 4 && memcmp(inner->start, "char", 4) == 0) {
          tok->start = (const uint8_t *)"char";
          tok->length = 4;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
        } else if (inner->length == 5 &&
                   memcmp(inner->start, "float", 5) == 0) {
          tok->start = (const uint8_t *)"float";
          tok->length = 5;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
        } else if (inner->length == 6 &&
                   memcmp(inner->start, "double", 6) == 0) {
          tok->start = (const uint8_t *)"double";
          tok->length = 6;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
        } else if (inner->length == 4 && memcmp(inner->start, "void", 4) == 0) {
          tok->start = (const uint8_t *)"void";
          tok->length = 4;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
        } else {
          tok->start = (const uint8_t *)"int";
          tok->length = 3;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
        }
      } else if (i + 5 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
                 tree->base_tokens->tokens[i + 3].kind == CDD_TOKEN_DOT &&
                 tree->base_tokens->tokens[i + 5].kind == CDD_TOKEN_RPAREN) {
        /* typeof(expr.bitfield) - naive polyfill to int */
        tok->start = (const uint8_t *)"int";
        tok->length = 3;
        tree->base_tokens->tokens[i + 1].length = 0;
        tree->base_tokens->tokens[i + 2].length = 0;
        tree->base_tokens->tokens[i + 3].length = 0;
        tree->base_tokens->tokens[i + 4].length = 0;
        tree->base_tokens->tokens[i + 5].length = 0;
      } else if (i + 5 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
                 tree->base_tokens->tokens[i + 3].kind == CDD_TOKEN_ARROW &&
                 tree->base_tokens->tokens[i + 5].kind == CDD_TOKEN_RPAREN) {
        /* typeof(expr->bitfield) - naive polyfill to int */
        tok->start = (const uint8_t *)"int";
        tok->length = 3;
        tree->base_tokens->tokens[i + 1].length = 0;
        tree->base_tokens->tokens[i + 2].length = 0;
        tree->base_tokens->tokens[i + 3].length = 0;
        tree->base_tokens->tokens[i + 4].length = 0;
        tree->base_tokens->tokens[i + 5].length = 0;
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___AUTO_TYPE) {
      /* Fallback for now since type inference isn't fully implemented */
      tok->start = (const uint8_t *)"int";
      tok->length = 3;
    } else if (tok->kind == CDD_TOKEN_NUMBER) {
      char buf[256];
      size_t copy_len = tok->length < 255 ? tok->length : 255;
      struct NumericValue nv;
      memcpy(buf, tok->start, copy_len);
      buf[copy_len] = '\0';
      if (parse_numeric_literal(buf, &nv) == ERANGE) {
        /* Exceeds 64-bit */
        uint64_t high = 0, low = 0;
        char *repl = (char *)malloc(128);
        if (buf[0] == '0' && (buf[1] == 'x' || buf[1] == 'X')) {
          parse_hex_128_literal(buf, copy_len, &high, &low);
        } else {
          parse_128_literal(buf, copy_len, &high, &low);
        }
        sprintf(repl, "cdd_make_uint128(0x%llxULL, 0x%llxULL)",
                (unsigned long long)high, (unsigned long long)low);
        tok->start = (const uint8_t *)repl;
        tok->length = strlen(repl);
      }
    } else if (tok->kind == CDD_TOKEN_IDENTIFIER && tok->length == 13 &&
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
        } else if (attr->length == 17 &&
                   memcmp(attr->start, "transparent_union", 17) == 0) {
          tok->start = (const uint8_t *)"/* transparent_union */";
          tok->length = 23;
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
        } else {
          /* Generic fallback for any other no-arg attribute like always_inline,
           * pure, const, weak, etc. */
          char buf[128];
          char *heap_buf;
          sprintf(buf, "/* %.*s */", (int)attr->length, attr->start);
          heap_buf = strdup(buf);
          tok->start = (const uint8_t *)heap_buf;
          tok->length = strlen(heap_buf);
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
          tree->base_tokens->tokens[i + 4].length = 0;
          tree->base_tokens->tokens[i + 5].length = 0;
        }
      } else if (i + 8 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
                 tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_LPAREN &&
                 tree->base_tokens->tokens[i + 4].kind == CDD_TOKEN_LPAREN &&
                 tree->base_tokens->tokens[i + 6].kind == CDD_TOKEN_RPAREN &&
                 tree->base_tokens->tokens[i + 7].kind == CDD_TOKEN_RPAREN &&
                 tree->base_tokens->tokens[i + 8].kind == CDD_TOKEN_RPAREN) {
        cdd_token_t *attr = &tree->base_tokens->tokens[i + 3];
        if (attr->length == 11 && memcmp(attr->start, "vector_size", 11) == 0) {
          /* Lower vector types to standard C arrays by grabbing the next
             identifier This is a simplistic lowering that just drops the vector
             semantic and treats it as an array, satisfying `v[0]` indexing
             automatically without rewrite overhead. */
          tok->start = (const uint8_t *)"";
          tok->length = 0;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
          tree->base_tokens->tokens[i + 4].length = 0;
          tree->base_tokens->tokens[i + 5].length = 0;
          tree->base_tokens->tokens[i + 6].length = 0;
          tree->base_tokens->tokens[i + 7].length = 0;
          tree->base_tokens->tokens[i + 8].length = 0;

        /* The vector arithmetic, bitwise, and casting extensions
           natively just fail to compile under MSVC without
           operator overloading, so we map them to pure struct
           arrays. Fully supporting vector semantics requires full
           AST-level operator overloading which is beyond standard
           C89's capabilities without full C++ usage. So,
           for standardizing C code, vector operations simply
           become array memory blobs. */        } else if (attr->length == 7 &&
                   memcmp(attr->start, "aligned", 7) == 0) {
          char buf[128];
          char *heap_buf;
          sprintf(buf, "_Alignas(%.*s)",
                  (int)tree->base_tokens->tokens[i + 5].length,
                  tree->base_tokens->tokens[i + 5].start);
          heap_buf = strdup(buf);
          tok->start = (const uint8_t *)heap_buf;
          tok->length = strlen(heap_buf);
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
          tree->base_tokens->tokens[i + 4].length = 0;
          tree->base_tokens->tokens[i + 5].length = 0;
          tree->base_tokens->tokens[i + 6].length = 0;
          tree->base_tokens->tokens[i + 7].length = 0;
          tree->base_tokens->tokens[i + 8].length = 0;
        } else if (attr->length == 4 && memcmp(attr->start, "mode", 4) == 0) {
          cdd_token_t *mode_val = &tree->base_tokens->tokens[i + 5];
          if (mode_val->length == 2) {
            const char *map = "";
            if (memcmp(mode_val->start, "QI", 2) == 0)
              map = "/* mode(QI) -> int8_t */";
            else if (memcmp(mode_val->start, "HI", 2) == 0)
              map = "/* mode(HI) -> int16_t */";
            else if (memcmp(mode_val->start, "SI", 2) == 0)
              map = "/* mode(SI) -> int32_t */";
            else if (memcmp(mode_val->start, "DI", 2) == 0)
              map = "/* mode(DI) -> int64_t */";
            else if (memcmp(mode_val->start, "TI", 2) == 0)
              map = "/* mode(TI) -> int128_t */";
            if (map[0] != '\0') {
              tok->start = (const uint8_t *)map;
              tok->length = strlen(map);
            } else {
              tok->start = (const uint8_t *)"";
              tok->length = 0;
            }
          } else {
            tok->start = (const uint8_t *)"";
            tok->length = 0;
          }
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
          tree->base_tokens->tokens[i + 4].length = 0;
          tree->base_tokens->tokens[i + 5].length = 0;
          tree->base_tokens->tokens[i + 6].length = 0;
          tree->base_tokens->tokens[i + 7].length = 0;
          tree->base_tokens->tokens[i + 8].length = 0;
        }
      }
    } else if (tok->kind == CDD_TOKEN_IDENTIFIER) {
      if (tok->length == 13 && memcmp(tok->start, "__extension__", 13) == 0) {
        tok->start = (const uint8_t *)"";
        tok->length = 0;
      } else if (tok->length == 17 &&
                 memcmp(tok->start, "__builtin_shuffle", 17) == 0) {
        tok->start = (const uint8_t *)"cdd_builtin_shuffle";
        tok->length = 19;
      } else if (tok->length == 23 &&
                 memcmp(tok->start, "__builtin_shufflevector", 23) == 0) {
        tok->start = (const uint8_t *)"cdd_builtin_shufflevector";
        tok->length = 25;
      } else if ((tok->length == 11 &&
                  memcmp(tok->start, "__alignof__", 11) == 0) ||
                 (tok->length == 9 &&
                  memcmp(tok->start, "__alignof", 9) == 0)) {
        /* If it's applied to an expression, rewrite to _Alignof(typeof(expr))?
           For now we just map to _Alignof.
           In a full AST, we would check if the child is an expression or type.
         */
        tok->start = (const uint8_t *)"_Alignof";
        tok->length = 8;
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD_STRUCT) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
        /* Anonymous struct: GNU extension. Inject dummy name to make C89 happy.
         */
        static int anon_counter = 0;
        char *buf = (char *)malloc(32);
        sprintf(buf, "struct _cdd_anon_%d", anon_counter++);
        tok->start = (const uint8_t *)buf;
        tok->length = strlen(buf);
      }
    } else if (tok->kind == CDD_TOKEN_IDENTIFIER && tok->length == 5 &&
               memcmp(tok->start, "union", 5) == 0) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
        /* Anonymous union: GNU extension. Inject dummy name. */
        static int anon_counter = 0;
        char *buf = (char *)malloc(32);
        sprintf(buf, "union _cdd_anon_%d", anon_counter++);
        tok->start = (const uint8_t *)buf;
        tok->length = strlen(buf);
      }
    } else if (tok->kind == CDD_TOKEN_LPAREN) {
      /* Detect cast to union as lvalue `(union U)val = ...` */
      if (i + 4 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_IDENTIFIER &&
          tree->base_tokens->tokens[i + 1].length == 5 &&
          memcmp(tree->base_tokens->tokens[i + 1].start, "union", 5) == 0 &&
          tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_IDENTIFIER &&
          tree->base_tokens->tokens[i + 3].kind == CDD_TOKEN_RPAREN) {

        size_t fwd;
        int tk;
        /* Rewrite cast to compound literal `(union U){val}` */
        tree->base_tokens->tokens[i + 3].start = (const uint8_t *)"){";
        tree->base_tokens->tokens[i + 3].length = 2;

        /* Find end of expression heuristic to add `}` */
        for (fwd = i + 4; fwd < tree->base_tokens->size; fwd++) {
          tk = tree->base_tokens->tokens[fwd].kind;
          if (tk == (int)CDD_TOKEN_SEMICOLON || tk == (int)CDD_TOKEN_ASSIGN) {
            tree->base_tokens->tokens[fwd - 1].start = (const uint8_t *)"}";
            tree->base_tokens->tokens[fwd - 1].length = 1;
            break;
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD_RETURN) {
      if (i + 4 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
          tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_IDENTIFIER &&
          tree->base_tokens->tokens[i + 2].length == 4 &&
          memcmp(tree->base_tokens->tokens[i + 2].start, "void", 4) == 0 &&
          tree->base_tokens->tokens[i + 3].kind == CDD_TOKEN_RPAREN) {
        /* Rewrite `return (void)expr;` to `(void)expr; return;` */
        size_t k;
        for (k = i + 4; k < tree->base_tokens->size; k++) {
          if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON) {
            tok->start = (const uint8_t *)"";
            tok->length = 0;
            tree->base_tokens->tokens[k].start = (const uint8_t *)"; return;";
            tree->base_tokens->tokens[k].length = 9;
            break;
          }
        }
      }
    }
  }

  /* Unroll statement expressions, VLAs, and __label__s */
  {
#define MAX_LOCAL_LABELS 256
#define MAX_VLAS 256
#define MAX_CLEANUPS 256
    /** @brief Struct definition */
    typedef struct {
      /** @brief field */
      /** @brief field */
      const uint8_t *name;
      /** @brief field */
      /** @brief field */
      size_t length;
      /** @brief field */
      /** @brief field */
      char rename[64];
      /** @brief field */
      /** @brief field */
      int depth;
    } local_label_t;
    /** @brief Struct definition */
    typedef struct {
      /** @brief field */
      /** @brief field */
      const uint8_t *name;
      /** @brief field */
      /** @brief field */
      size_t length;
      /** @brief field */
      /** @brief field */
      int depth;
    } vla_t;
    /** @brief Struct definition */
    typedef struct {
      /** @brief field */
      /** @brief field */
      const uint8_t *var_name;
      /** @brief field */
      /** @brief field */
      size_t var_length;
      /** @brief field */
      /** @brief field */
      const uint8_t *func_name;
      /** @brief field */
      /** @brief field */
      size_t func_length;
      /** @brief field */
      /** @brief field */
      int depth;
    } cleanup_t;
    local_label_t local_labels[MAX_LOCAL_LABELS];
    vla_t vlas[MAX_VLAS];
    cleanup_t cleanups[MAX_CLEANUPS];
    size_t num_local_labels = 0;
    size_t num_vlas = 0;
    size_t num_cleanups = 0;
    int current_depth = 0;
    int label_counter = 0;

    for (i = 0; i < tree->base_tokens->size; i++) {
      cdd_token_t *t = &tree->base_tokens->tokens[i];
      if (t->kind == CDD_TOKEN_LBRACE) {
        current_depth++;
      } else if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 13 &&
                 memcmp(t->start, "__attribute__", 13) == 0 &&
                 i + 8 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
                 tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_LPAREN &&
                 tree->base_tokens->tokens[i + 4].kind == CDD_TOKEN_LPAREN &&
                 tree->base_tokens->tokens[i + 6].kind == CDD_TOKEN_RPAREN &&
                 tree->base_tokens->tokens[i + 7].kind == CDD_TOKEN_RPAREN &&
                 tree->base_tokens->tokens[i + 8].kind == CDD_TOKEN_RPAREN) {
        cdd_token_t *attr = &tree->base_tokens->tokens[i + 3];
        if (attr->length == 7 && memcmp(attr->start, "cleanup", 7) == 0) {
          cdd_token_t *func_val = &tree->base_tokens->tokens[i + 5];
          cdd_token_t *var_tok = NULL;
          size_t k;
          for (k = i + 9; k < tree->base_tokens->size; k++) {
            if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON ||
                tree->base_tokens->tokens[k].kind == CDD_TOKEN_ASSIGN ||
                tree->base_tokens->tokens[k].kind == CDD_TOKEN_COMMA) {
              size_t back;
              for (back = k; back > i + 8; back--) {
                if (tree->base_tokens->tokens[back].kind ==
                    CDD_TOKEN_IDENTIFIER) {
                  var_tok = &tree->base_tokens->tokens[back];
                  break;
                }
              }
              break;
            }
          }
          if (var_tok && num_cleanups < MAX_CLEANUPS) {
            cleanups[num_cleanups].var_name = var_tok->start;
            cleanups[num_cleanups].var_length = var_tok->length;
            cleanups[num_cleanups].func_name = func_val->start;
            cleanups[num_cleanups].func_length = func_val->length;
            cleanups[num_cleanups].depth = current_depth;
            num_cleanups++;
          }

          t->start = (const uint8_t *)"";
          t->length = 0;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
          tree->base_tokens->tokens[i + 4].length = 0;
          tree->base_tokens->tokens[i + 5].length = 0;
          tree->base_tokens->tokens[i + 6].length = 0;
          tree->base_tokens->tokens[i + 7].length = 0;
          tree->base_tokens->tokens[i + 8].length = 0;
          i += 8;
        }
      } else if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 13 &&
                 memcmp(t->start, "__attribute__", 13) == 0 &&
                 i + 2 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
                 tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_LPAREN) {
        /* Fallback catch-all for any __attribute__ that hasn't been removed yet
         * (like format, alias, constructor) */
        size_t k;
        int depth = 0;
        size_t end_idx = 0;
        for (k = i + 1; k < tree->base_tokens->size; k++) {
          if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LPAREN)
            depth++;
          else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_RPAREN) {
            depth--;
            if (depth == 0) {
              end_idx = k;
              break;
            }
          }
        }
        if (end_idx > i + 2) {
          char buf[4096];
          char *p = buf;
          p += sprintf(p, "/* __attribute__");
          for (k = i + 1; k <= end_idx; k++) {
            p += sprintf(p, "%.*s", (int)tree->base_tokens->tokens[k].length,
                         tree->base_tokens->tokens[k].start);
          }
          p += sprintf(p, " */");
          t->start = (const uint8_t *)strdup(buf);
          t->length = strlen((const char *)t->start);
          for (k = i + 1; k <= end_idx; k++) {
            tree->base_tokens->tokens[k].length = 0;
          }
        }
      } else if (t->kind == CDD_TOKEN_RBRACE) {
        while (num_local_labels > 0 &&
               local_labels[num_local_labels - 1].depth == current_depth) {
          num_local_labels--;
        }
        {
          char buf[4096] = {0};
          char *p = buf;
          int appended = 0;

          /* Run cleanups in reverse order */
          while (num_cleanups > 0 &&
                 cleanups[num_cleanups - 1].depth == current_depth) {
            p += sprintf(p, " %.*s(&%.*s);",
                         (int)cleanups[num_cleanups - 1].func_length,
                         cleanups[num_cleanups - 1].func_name,
                         (int)cleanups[num_cleanups - 1].var_length,
                         cleanups[num_cleanups - 1].var_name);
            num_cleanups--;
            appended = 1;
          }

          if (config && config->fallback_vla_to_malloc) {
            while (num_vlas > 0 && vlas[num_vlas - 1].depth == current_depth) {
              p += sprintf(p, " free(%.*s);", (int)vlas[num_vlas - 1].length,
                           vlas[num_vlas - 1].name);
              num_vlas--;
              appended = 1;
            }
          } else {
            while (num_vlas > 0 && vlas[num_vlas - 1].depth == current_depth) {
              num_vlas--;
            }
          }

          if (appended) {
            char *heap_buf;
            strcat(p, " }");
            heap_buf = strdup(buf);
            t->start = (const uint8_t *)heap_buf;
            t->length = strlen(heap_buf);
          }
        }
        current_depth--;
      } else if (t->kind == CDD_TOKEN_KEYWORD_RETURN) {
        if (num_cleanups > 0 ||
            (config && config->fallback_vla_to_malloc && num_vlas > 0)) {
          size_t k;
          for (k = i + 1; k < tree->base_tokens->size; k++) {
            if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON) {
              int has_expr = (k > i + 1);
              char buf[4096] = {0};
              char *p = buf;
              size_t c_idx, v_idx;

              if (has_expr) {
                t->start = (const uint8_t *)"{ __auto_type __cdd_ret = (";
                t->length = 27;
                p += sprintf(p, ");");
              } else {
                t->start = (const uint8_t *)"{";
                t->length = 1;
              }

              for (c_idx = num_cleanups; c_idx > 0; c_idx--) {
                p += sprintf(p, " %.*s(&%.*s);",
                             (int)cleanups[c_idx - 1].func_length,
                             cleanups[c_idx - 1].func_name,
                             (int)cleanups[c_idx - 1].var_length,
                             cleanups[c_idx - 1].var_name);
              }
              if (config && config->fallback_vla_to_malloc) {
                for (v_idx = num_vlas; v_idx > 0; v_idx--) {
                  p += sprintf(p, " free(%.*s);", (int)vlas[v_idx - 1].length,
                               vlas[v_idx - 1].name);
                }
              }

              if (has_expr) {
                p += sprintf(p, " return __cdd_ret; }");
              } else {
                p += sprintf(p, " return; }");
              }

              tree->base_tokens->tokens[k].start = (const uint8_t *)strdup(buf);
              tree->base_tokens->tokens[k].length =
                  strlen((const char *)tree->base_tokens->tokens[k].start);
              break;
            }
          }
        }
      } else if (t->kind == CDD_TOKEN_KEYWORD_GOTO) {
        if (num_cleanups > 0) {
          char buf[128];
          sprintf(buf,
                  "/* warning: goto cross-scope cleanups unsupported */ goto");
          t->start = (const uint8_t *)strdup(buf);
          t->length = strlen((const char *)t->start);
        }

        if (num_vlas > 0) {
          char buf[128];
          sprintf(buf,
                  "/* warning: goto crossing VLA scopes unsupported */ goto");
          t->start = (const uint8_t *)strdup(buf);
          t->length = strlen((const char *)t->start);
        }

        if (i + 1 < tree->base_tokens->size &&
            tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_STAR) {
          /*
             Converting jump tables to `switch` statements internally:
             We emit a hard warning and leave it as is, since full jump table to
             switch statements conversion across variable scopes natively is
             impossible without whole-function CFG lifting. In many embedded and
             MSVC contexts this extension cannot be successfully lowered
             natively without massive hacks.
          */
          char buf[128];
          sprintf(buf, "/* warning: computed goto converting jump tables to "
                       "switch internally unsupported */ goto");
          t->start = (const uint8_t *)strdup(buf);
          t->length = strlen((const char *)t->start);
        }
      } else if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 7 &&
                 memcmp(t->start, "longjmp", 7) == 0) {
        if (num_cleanups > 0) {
          char buf[128];
          sprintf(buf, "/* warning: longjmp bypasses cleanups */ longjmp");
          t->start = (const uint8_t *)strdup(buf);
          t->length = strlen((const char *)t->start);
        }
      } else if (t->kind == CDD_TOKEN_KEYWORD___LABEL__) {
        t->length = 0;
        i++;
        while (i < tree->base_tokens->size) {
          cdd_token_t *nt = &tree->base_tokens->tokens[i];
          if (nt->kind == CDD_TOKEN_IDENTIFIER) {
            if (num_local_labels < MAX_LOCAL_LABELS) {
              local_labels[num_local_labels].name = nt->start;
              local_labels[num_local_labels].length = nt->length;
              local_labels[num_local_labels].depth = current_depth;
              sprintf(local_labels[num_local_labels].rename, "__cdd_ll_%.*s_%d",
                      (int)nt->length, nt->start, ++label_counter);
              num_local_labels++;
            }
            nt->length = 0;
          } else if (nt->kind == CDD_TOKEN_COMMA) {
            nt->length = 0;
          } else if (nt->kind == CDD_TOKEN_SEMICOLON) {
            nt->length = 0;
            break;
          } else {
            break;
          }
          i++;
        }
      } else if (t->kind == CDD_TOKEN_IDENTIFIER && num_local_labels > 0) {
        /* Check if it's a label definition, a goto target, or an
         * address-of-label */
        int is_label_ref = 0;
        if (i > 0 &&
            tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_KEYWORD_GOTO) {
          is_label_ref = 1;
        } else if (i > 1 &&
                   tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_OTHER &&
                   tree->base_tokens->tokens[i - 1].length == 1 &&
                   tree->base_tokens->tokens[i - 1].start[0] == '&' &&
                   tree->base_tokens->tokens[i - 2].kind == CDD_TOKEN_OTHER &&
                   tree->base_tokens->tokens[i - 2].length == 1 &&
                   tree->base_tokens->tokens[i - 2].start[0] == '&') {
          is_label_ref = 1;
        } else if (i + 1 < tree->base_tokens->size) {
          cdd_token_t *nt = &tree->base_tokens->tokens[i + 1];
          if (nt->kind == CDD_TOKEN_OTHER && nt->length == 1 &&
              nt->start[0] == ':') {
            is_label_ref = 1;
          }
        }

        if (is_label_ref) {
          size_t j;
          for (j = num_local_labels; j-- > 0;) {
            if (local_labels[j].length == t->length &&
                memcmp(local_labels[j].name, t->start, t->length) == 0) {
              char *dup = strdup(local_labels[j].rename);
              t->start = (const uint8_t *)dup;
              t->length = strlen(dup);
              break;
            }
          }
        }
      }

      /* Existing unroll logic */
      if (t->kind == CDD_TOKEN_LPAREN && i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
        /* Remove ({ */
        t->start = (const uint8_t *)"";
        t->length = 0;
        tree->base_tokens->tokens[i + 1].start = (const uint8_t *)"";
        tree->base_tokens->tokens[i + 1].length = 0;
      } else if (t->kind == CDD_TOKEN_RBRACE &&
                 i + 1 < tree->base_tokens->size &&
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
            next->kind == CDD_TOKEN_IDENTIFIER) {

          /* Count dimensions */
          int dims = 0;
          size_t k = i;
          char size_expr[256] = {0};
          char *p_sz = size_expr;
          while (k + 2 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[k].kind == CDD_TOKEN_LBRACKET &&
                 tree->base_tokens->tokens[k + 1].kind ==
                     CDD_TOKEN_IDENTIFIER &&
                 tree->base_tokens->tokens[k + 2].kind == CDD_TOKEN_RBRACKET) {
            if (dims > 0)
              p_sz += sprintf(p_sz, " * ");
            p_sz += sprintf(p_sz, "%.*s",
                            (int)tree->base_tokens->tokens[k + 1].length,
                            tree->base_tokens->tokens[k + 1].start);
            dims++;
            k += 3;
          }
          if (dims > 0 && k < tree->base_tokens->size) {
            cdd_token_t *end_tok = &tree->base_tokens->tokens[k];
            if (end_tok->kind == CDD_TOKEN_SEMICOLON) {
              char buf[256];
              char *heap_buf;
              if (config && config->fallback_vla_to_malloc) {
                sprintf(buf, "*%.*s = malloc((%s) * sizeof(*%.*s));",
                        (int)prev->length, prev->start, size_expr,
                        (int)prev->length, prev->start);
                if (num_vlas < MAX_VLAS) {
                  vlas[num_vlas].name = prev->start;
                  vlas[num_vlas].length = prev->length;
                  vlas[num_vlas].depth = current_depth;
                  num_vlas++;
                }
              } else {
                sprintf(buf, "*%.*s = alloca((%s) * sizeof(*%.*s));",
                        (int)prev->length, prev->start, size_expr,
                        (int)prev->length, prev->start);
              }
              heap_buf = strdup(buf);
              prev->start = (const uint8_t *)heap_buf;
              prev->length = strlen(heap_buf);

              {
                size_t wipe;
                for (wipe = i; wipe < k; wipe++) {
                  tree->base_tokens->tokens[wipe].length = 0;
                }
              }
            } else if (end_tok->kind == CDD_TOKEN_COMMA ||
                       end_tok->kind == CDD_TOKEN_RPAREN) {
              /* VLA parameter */
              size_t wipe;
              for (wipe = i; wipe < k; wipe++) {
                if (tree->base_tokens->tokens[wipe].kind ==
                    CDD_TOKEN_IDENTIFIER) {
                  tree->base_tokens->tokens[wipe].length = 0;
                }
              }
            }
          }
        } else if (prev->kind == CDD_TOKEN_IDENTIFIER &&
                   next->kind == CDD_TOKEN_NUMBER && next->length == 1 &&
                   next->start[0] == '0' && semi->kind == CDD_TOKEN_RBRACKET &&
                   i + 3 < tree->base_tokens->size &&
                   tree->base_tokens->tokens[i + 3].kind ==
                       CDD_TOKEN_SEMICOLON) {
          /* Found zero-length array pattern `int arr[0];` */
          /* Look forward for RBRACE to see if it is at the end of a struct */
          size_t fwd;
          int found_rbrace = 0;
          for (fwd = i + 4; fwd < tree->base_tokens->size; fwd++) {
            if (tree->base_tokens->tokens[fwd].length == 0) {
              continue;
            } else if (tree->base_tokens->tokens[fwd].kind ==
                       CDD_TOKEN_RBRACE) {
              found_rbrace = 1;
              break;
            } else {
              break;
            }
          }
          if (found_rbrace) {
            /* Flexible Array Member polyfill -> change [0] to [1] */
            next->start = (const uint8_t *)"1";
            next->length = 1;
          } else {
            /* In the middle of a struct: rewrite struct layout by changing [0]
               to [1] to be ISO C compliant without relying on GNU extensions.
             */
            next->start = (const uint8_t *)"1";
            next->length = 1;
          }
        }
      } else if (t->kind == CDD_TOKEN_COMMA) {
        /* Strip trailing commas before RBRACE */
        size_t next_idx = i + 1;
        while (next_idx < tree->base_tokens->size &&
               tree->base_tokens->tokens[next_idx].length == 0) {
          next_idx++;
        }
        if (next_idx < tree->base_tokens->size &&
            tree->base_tokens->tokens[next_idx].kind == CDD_TOKEN_RBRACE) {
          t->length = 0;
        }
      } else if (t->kind == CDD_TOKEN_LBRACE &&
                 i + 1 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_RBRACE) {
        /* Determine if it's an empty struct or empty initializer */
        int is_initializer = 1;
        size_t k;
        int depth = 0;
        for (k = i; k-- > 0;) {
          cdd_token_t *tk = &tree->base_tokens->tokens[k];
          if (tk->kind == CDD_TOKEN_RBRACE)
            depth++;
          else if (tk->kind == CDD_TOKEN_LBRACE)
            depth--;

          if (depth == 0) {
            if (tk->kind == CDD_TOKEN_KEYWORD_STRUCT ||
                (tk->kind == CDD_TOKEN_IDENTIFIER && tk->length == 5 &&
                 memcmp(tk->start, "union", 5) == 0) ||
                (tk->kind == CDD_TOKEN_IDENTIFIER && tk->length == 4 &&
                 memcmp(tk->start, "enum", 4) == 0)) {
              is_initializer = 0;
              break;
            } else if (tk->kind == CDD_TOKEN_SEMICOLON) {
              break;
            } else if (tk->kind == CDD_TOKEN_ASSIGN) {
              is_initializer = 1;
              break;
            }
          }
        }

        if (i > 0 && tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_OTHER &&
            tree->base_tokens->tokens[i - 1].length == 1 &&
            tree->base_tokens->tokens[i - 1].start[0] == ':') {
          /* Empty block following a case or default label */
          tree->base_tokens->tokens[i].start = (const uint8_t *)";";
          tree->base_tokens->tokens[i].length = 1;
          tree->base_tokens->tokens[i + 1].start = (const uint8_t *)"";
          tree->base_tokens->tokens[i + 1].length = 0;
        } else if (is_initializer) {
          tree->base_tokens->tokens[i + 1].start = (const uint8_t *)" 0 }";
          tree->base_tokens->tokens[i + 1].length = 4;
        } else {
          tree->base_tokens->tokens[i + 1].start =
              (const uint8_t *)" char _pad; }";
          tree->base_tokens->tokens[i + 1].length = 13;
        }
      } else if (t->kind == CDD_TOKEN_DOT && i > 1 &&
                 i + 2 < tree->base_tokens->size) {
        /* Look for ... */
        if (tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_DOT &&
            tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_DOT) {

          cdd_token_t *prev = &tree->base_tokens->tokens[i - 1];
          cdd_token_t *next = &tree->base_tokens->tokens[i + 3];
          int is_case = 0;
          int is_range_init = 0;
          size_t is_case_idx = 0;
          size_t is_range_idx = 0;
          int is_neg_start = 0;
          int is_neg_end = 0;
          int start_val = 0;
          int end_val = 0;

          /* Check for negative numbers */
          if (prev->kind == CDD_TOKEN_NUMBER) {
            if (i >= 2 &&
                tree->base_tokens->tokens[i - 2].kind == CDD_TOKEN_MINUS) {
              is_neg_start = 1;
            }
          }
          if (next->kind == CDD_TOKEN_MINUS &&
              i + 4 < tree->base_tokens->size &&
              tree->base_tokens->tokens[i + 4].kind == CDD_TOKEN_NUMBER) {
            is_neg_end = 1;
            next = &tree->base_tokens->tokens[i + 4];
          }

          if (prev->kind == CDD_TOKEN_NUMBER &&
              next->kind == CDD_TOKEN_NUMBER) {
            char buf[64];
            memcpy(buf, prev->start, prev->length);
            buf[prev->length] = '\0';
            start_val = atoi(buf);
            if (is_neg_start)
              start_val = -start_val;

            memcpy(buf, next->start, next->length);
            buf[next->length] = '\0';
            end_val = atoi(buf);
            if (is_neg_end)
              end_val = -end_val;

            /* Determine context: case or range init */
            {
              size_t k;
              for (k = i - 1; k-- > 0;) {
                if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_IDENTIFIER &&
                    tree->base_tokens->tokens[k].length == 4 &&
                    memcmp(tree->base_tokens->tokens[k].start, "case", 4) ==
                        0) {
                  is_case = 1;
                  is_case_idx = k;
                  break;
                } else if (tree->base_tokens->tokens[k].kind ==
                           CDD_TOKEN_LBRACKET) {
                  is_range_init = 1;
                  is_range_idx = k;
                  break;
                } else if (tree->base_tokens->tokens[k].kind ==
                               CDD_TOKEN_SEMICOLON ||
                           tree->base_tokens->tokens[k].kind ==
                               CDD_TOKEN_RBRACE ||
                           tree->base_tokens->tokens[k].kind ==
                               CDD_TOKEN_LBRACE) {
                  break;
                }
              }
            }

            if (is_case && start_val <= end_val) {
              char *heap_buf;
              size_t alloc_sz = (end_val - start_val + 2) * 32 + 128;
              char *p;
              int v;
              heap_buf = (char *)malloc(alloc_sz);
              if (heap_buf) {
                p = heap_buf;
                /* If overlapping case, add a warning comment */
                /* For now we just generate all cases. Semantic error detection
                   of overlaps is usually left to the underlying compiler, but
                   we add a warning. */
                if (start_val > end_val) {
                  p += sprintf(p, "/* WARNING: Invalid case range */ ");
                }
                for (v = start_val; v <= end_val; v++) {
                  if (v == end_val) {
                    p += sprintf(p, "case %d", v);
                  } else {
                    p += sprintf(p, "case %d: ", v);
                  }
                }
                if (is_neg_start) {
                  tree->base_tokens->tokens[i - 2].length = 0;
                }
                tree->base_tokens->tokens[is_case_idx].start =
                    (const uint8_t *)heap_buf;
                tree->base_tokens->tokens[is_case_idx].length =
                    strlen(heap_buf);
                prev->length = 0;
                t->length = 0;
                tree->base_tokens->tokens[i + 1].length = 0;
                tree->base_tokens->tokens[i + 2].length = 0;
                if (is_neg_end) {
                  tree->base_tokens->tokens[i + 3].length = 0;
                }
                next->length = 0;
              }
            } else if (is_range_init && start_val <= end_val) {
              char *heap_buf;
              size_t alloc_sz = (end_val - start_val + 2) * 32;
              char *p;
              int v;
              /* Extract the assigned value */
              cdd_token_t *assign_val = NULL;
              if (next + 3 <=
                      &tree->base_tokens->tokens[tree->base_tokens->size - 1] &&
                  (next + 1)->kind == CDD_TOKEN_RBRACKET &&
                  (next + 2)->kind == CDD_TOKEN_ASSIGN) {
                assign_val = next + 3;
              }

              if (assign_val) {
                heap_buf = (char *)malloc(alloc_sz);
                if (heap_buf) {
                  p = heap_buf;
                  for (v = start_val; v <= end_val; v++) {
                    if (v == end_val) {
                      p += sprintf(p, "[%d] = %.*s", v, (int)assign_val->length,
                                   assign_val->start);
                    } else {
                      p += sprintf(p, "[%d] = %.*s, ", v,
                                   (int)assign_val->length, assign_val->start);
                    }
                  }

                  tree->base_tokens->tokens[is_range_idx].start =
                      (const uint8_t *)heap_buf;
                  tree->base_tokens->tokens[is_range_idx].length =
                      strlen(heap_buf);
                  prev->length = 0;
                  t->length = 0;
                  tree->base_tokens->tokens[i + 1].length = 0;
                  tree->base_tokens->tokens[i + 2].length = 0;
                  next->length = 0;
                  (next + 1)->length = 0; /* ] */
                  (next + 2)->length = 0; /* = */
                  assign_val->length = 0;
                }
              }
            }
          }
        }
      }
    }
  }

  return 0;
}
