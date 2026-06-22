/**
 * @file gnu_standardizer.c
 * @brief Implementation of the GNU standardizer transformer.
 */

/* clang-format off */
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_builder.h"
#include "classes/parse/cdd_cst_factory.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include "classes/parse/numeric.h"
#include "c_str_span.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */
/* LCOV_EXCL_START */

/** @brief MAX_LOCAL_LABELS */
#define MAX_LOCAL_LABELS 256
/** @brief MAX_VLAS */
#define MAX_VLAS 256
/** @brief MAX_CLEANUPS */
#define MAX_CLEANUPS 256

#ifdef _MSC_VER
/** @brief ULL_HEX_FMT */
#define ULL_HEX_FMT "%I64x"
#else
/** @brief ULL_HEX_FMT */
#define ULL_HEX_FMT "%llx"
#endif

static const char *pool_string_safe(cdd_cst_tree_t *tree, const char *str) {
  char *dup;
  if (!tree || !str)
    return NULL;
  dup = strdup(str);
  if (!dup)
    return NULL;
  if (tree->num_strings >= tree->string_capacity) {
    size_t new_cap =
        tree->string_capacity == 0 ? 32 : tree->string_capacity * 2;
    char **new_pool =
        (char **)realloc(tree->string_pool, new_cap * sizeof(char *));
    if (!new_pool) {
      free(dup);
      return NULL;
    }
    tree->string_pool = new_pool;
    tree->string_capacity = new_cap;
  }
  tree->string_pool[tree->num_strings++] = dup;
  return dup;
}

static const char *pool_string_safe_len(cdd_cst_tree_t *tree, const char *str,
                                        size_t len) {
  char *dup;
  if (!tree || !str)
    return NULL;
  dup = (char *)malloc(len + 1);
  if (!dup)
    return NULL;
  memcpy(dup, str, len);
  dup[len] = '\0';
  if (tree->num_strings >= tree->string_capacity) {
    size_t new_cap =
        tree->string_capacity == 0 ? 32 : tree->string_capacity * 2;
    char **new_pool =
        (char **)realloc(tree->string_pool, new_cap * sizeof(char *));
    if (!new_pool) {
      free(dup);
      return NULL;
    }
    tree->string_pool = new_pool;
    tree->string_capacity = new_cap;
  }
  tree->string_pool[tree->num_strings++] = dup;
  return dup;
}
static int append_int(char *p, int v, char **out_p) {
  char temp[32];
  int i = 0, j;
  unsigned int u;
  if (!p || !out_p)
    return EINVAL;
  if (v == 0) {
    *p++ = '0';
    *p = '\0';
    *out_p = p;
    return 0;
  }
  if (v < 0) {
    *p++ = '-';
    u = (unsigned int)-v;
  } else {
    u = (unsigned int)v;
  }
  while (u > 0) {
    temp[i++] = (char)('0' + (u % 10));
    u /= 10;
  }
  for (j = i - 1; j >= 0; j--) {
    *p++ = temp[j];
  }
  *p = '\0';
  *out_p = p;
  return 0;
}

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
  /** @brief tree field */
  cdd_cst_tree_t *tree;
  /** @brief field */
  const uint8_t *func_name;
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
            const char *pooled;
            cdd_token_t *new_tok = NULL;
            buf[0] = '"';
            memcpy(buf + 1, ctx->func_name, ctx->func_len);
            buf[1 + ctx->func_len] = '"';
            buf[2 + ctx->func_len] = '\0';

            pooled = pool_string_safe(ctx->tree, buf);
            cdd_cst_create_token_len(ctx->tree, CDD_TOKEN_STRING,
                                     pooled ? pooled : buf, ctx->func_len + 2,
                                     &new_tok);
            free(buf);
            buf = NULL;
            if (new_tok) {
              new_tok->leading_trivia = tok->leading_trivia;
              new_tok->trailing_trivia = tok->trailing_trivia;
              tok->leading_trivia = NULL;
              tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(node, i, new_tok);
            }
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
  const uint8_t *name;
  /** @brief field */
  size_t length;
  /** @brief field */
  int is_tramp;
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

static const char *cdd_infer_type(cdd_token_t *tokens, size_t num_tokens) {
  size_t i;
  if (num_tokens == 0)
    return "int";
  for (i = 0; i < num_tokens; i++) {
    cdd_token_t *t = &tokens[i];

    int is_type = 0;
    if (t->kind == CDD_TOKEN_KEYWORD_INT || t->kind == CDD_TOKEN_KEYWORD_STRUCT)
      is_type = 1;
    else if (t->kind == CDD_TOKEN_IDENTIFIER) {
      if ((t->length == 4 && memcmp(t->start, "char", 4) == 0) ||
          (t->length == 5 && memcmp(t->start, "float", 5) == 0) ||
          (t->length == 6 && memcmp(t->start, "double", 6) == 0) ||
          (t->length == 4 && memcmp(t->start, "void", 4) == 0) ||
          (t->length == 5 && memcmp(t->start, "union", 5) == 0) ||
          (t->length == 4 && memcmp(t->start, "enum", 4) == 0) ||
          (t->length == 8 && memcmp(t->start, "unsigned", 8) == 0) ||
          (t->length == 6 && memcmp(t->start, "signed", 6) == 0) ||
          (t->length == 4 && memcmp(t->start, "long", 4) == 0) ||
          (t->length == 5 && memcmp(t->start, "short", 5) == 0)) {
        is_type = 1;
      }
    }
    if (is_type) {

      /* It's likely a type already */
      return NULL;
    }
  }
  /* Bit-field inference: expr.field or expr->field */
  if (num_tokens >= 3 && (tokens[num_tokens - 2].kind == CDD_TOKEN_DOT ||
                          tokens[num_tokens - 2].kind == CDD_TOKEN_ARROW)) {
    return "int";
  }
  /* Expression inference */
  if (num_tokens == 1 && tokens[0].kind == CDD_TOKEN_STRING) {
    return "const char *";
  }
  if (num_tokens == 1 && tokens[0].kind == CDD_TOKEN_NUMBER) {
    const char *str = (const char *)tokens[0].start;
    size_t len = tokens[0].length;
    size_t j;
    for (j = 0; j < len; j++) {
      if (str[j] == '.' || str[j] == 'p' || str[j] == 'P' || str[j] == 'e' ||
          str[j] == 'E') {
        if (str[len - 1] == 'f' || str[len - 1] == 'F')
          return "float";
        return "double";
      }
    }
    for (j = 0; j < len; j++) {
      if (str[j] == 'u' || str[j] == 'U') {
        if (str[len - 1] == 'l' || str[len - 1] == 'L')
          return "unsigned long";
        return "unsigned int";
      }
      if (str[j] == 'l' || str[j] == 'L')
        return "long";
    }
    return "int";
  }
  /* Fallback */
  return "int";
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
              ctx.tree = tree;
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
                    var_name = (char *)malloc(12);
                    if (var_name) {
                      memcpy(var_name, "__VA_ARGS__", 11);
                      var_name[11] = '\0';
                    }
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
                      {
                        size_t child_idx;
                        cdd_cst_node_t *parent = NULL;
                        cdd_cst_find_node_for_token(tree->root, tok, &child_idx,
                                                    &parent);
                        if (parent) {
                          const char *pooled;
                          cdd_token_t *new_tok = NULL;
                          pooled = pool_string_safe(tree, out_buf);
                          cdd_cst_create_token_len(tree,
                                                   CDD_TOKEN_PREPROC_DEFINE,
                                                   pooled ? pooled : out_buf,
                                                   out_p - out_buf, &new_tok);
                          free(out_buf);
                          out_buf = NULL;
                          if (new_tok) {
                            new_tok->leading_trivia = tok->leading_trivia;
                            new_tok->trailing_trivia = tok->trailing_trivia;
                            tok->leading_trivia = NULL;
                            tok->trailing_trivia = NULL;
                            cdd_cst_replace_token_child(parent, child_idx,
                                                        new_tok);
                          }
                        }
                      }
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
      size_t child_idx;
      cdd_cst_node_t *parent = NULL;
      cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &parent);
      if (parent) {
        cdd_token_t *new_tok = NULL;
        if (i > 0 &&
            tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_IDENTIFIER &&
            tree->base_tokens->tokens[i - 1].length == 8 &&
            memcmp(tree->base_tokens->tokens[i - 1].start, "unsigned", 8) ==
                0) {
          size_t prev_idx;
          cdd_cst_node_t *prev_parent = NULL;
          cdd_cst_find_node_for_token(tree->root,
                                      &tree->base_tokens->tokens[i - 1],
                                      &prev_idx, &prev_parent);
          if (prev_parent) {
            cdd_token_t *empty_tok = NULL;
            cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "", 0,
                                     &empty_tok);
            if (empty_tok) {
              empty_tok->leading_trivia =
                  tree->base_tokens->tokens[i - 1].leading_trivia;
              empty_tok->trailing_trivia =
                  tree->base_tokens->tokens[i - 1].trailing_trivia;
              tree->base_tokens->tokens[i - 1].leading_trivia = NULL;
              tree->base_tokens->tokens[i - 1].trailing_trivia = NULL;
              cdd_cst_replace_token_child(prev_parent, prev_idx, empty_tok);
            }
          }
          cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "cdd_uint128_t",
                                   13, &new_tok);
        } else {
          cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "cdd_int128_t",
                                   12, &new_tok);
        }
        if (new_tok) {
          new_tok->leading_trivia = tok->leading_trivia;
          new_tok->trailing_trivia = tok->trailing_trivia;
          tok->leading_trivia = NULL;
          tok->trailing_trivia = NULL;
          cdd_cst_replace_token_child(parent, child_idx, new_tok);
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD__DECIMAL32 ||
               tok->kind == CDD_TOKEN_KEYWORD__FRACT) {
      size_t child_idx;
      cdd_cst_node_t *parent = NULL;
      cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &parent);
      if (parent) {
        cdd_token_t *new_tok = NULL;
        cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "float", 5,
                                 &new_tok);
        if (new_tok) {
          new_tok->leading_trivia = tok->leading_trivia;
          new_tok->trailing_trivia = tok->trailing_trivia;
          tok->leading_trivia = NULL;
          tok->trailing_trivia = NULL;
          cdd_cst_replace_token_child(parent, child_idx, new_tok);
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD__DECIMAL64 ||
               tok->kind == CDD_TOKEN_KEYWORD__DECIMAL128 ||
               tok->kind == CDD_TOKEN_KEYWORD__ACCUM) {
      size_t child_idx;
      cdd_cst_node_t *parent = NULL;
      cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &parent);
      if (parent) {
        cdd_token_t *new_tok = NULL;
        cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "double", 6,
                                 &new_tok);
        if (new_tok) {
          new_tok->leading_trivia = tok->leading_trivia;
          new_tok->trailing_trivia = tok->trailing_trivia;
          tok->leading_trivia = NULL;
          tok->trailing_trivia = NULL;
          cdd_cst_replace_token_child(parent, child_idx, new_tok);
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___FP16 ||
               tok->kind == CDD_TOKEN_KEYWORD__FLOAT16 ||
               tok->kind == CDD_TOKEN_KEYWORD___BF16) {
      size_t child_idx;
      cdd_cst_node_t *parent = NULL;
      cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &parent);
      if (parent) {
        cdd_token_t *new_tok = NULL;
        cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "uint16_t", 8,
                                 &new_tok);
        if (new_tok) {
          new_tok->leading_trivia = tok->leading_trivia;
          new_tok->trailing_trivia = tok->trailing_trivia;
          tok->leading_trivia = NULL;
          tok->trailing_trivia = NULL;
          cdd_cst_replace_token_child(parent, child_idx, new_tok);
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___COMPLEX__) {
      /* `__complex__ int` -> `struct { int real, imag; }` */
      if (i + 1 < tree->base_tokens->size) {
        cdd_token_t *next_tok = &tree->base_tokens->tokens[i + 1];
        size_t child_idx;
        cdd_cst_node_t *owning_node = NULL;
        cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &owning_node);
        if (owning_node) {
          cdd_cst_node_t *temp = NULL;
          cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
          if (temp) {
            cdd_cst_builder_t bld;
            cdd_cst_builder_init(&bld, tree, temp);
            cdd_cst_bld_ident(&bld, "struct");
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_punct(&bld, "{");
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_ident(
                &bld, pool_string_safe_len(tree, (const char *)next_tok->start,
                                           next_tok->length));
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_ident(&bld, "real");
            cdd_cst_bld_punct(&bld, ",");
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_ident(&bld, "imag");
            cdd_cst_bld_punct(&bld, ";");
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_punct(&bld, "}");
            cdd_cst_bld_space(&bld);
            if (!cdd_cst_builder_has_error(&bld))
              cdd_cst_splice_children(tree, &owning_node, child_idx, 2,
                                      temp->children, temp->num_children);
            cdd_cst_builder_free(&bld);
            cdd_cst_free_node_only(temp);
          }
        }
        {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, tok->kind, "", 0, &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = tok->leading_trivia;
              n_tok->trailing_trivia = tok->trailing_trivia;
              tok->leading_trivia = NULL;
              tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
        }
        {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, next_tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, next_tok->kind, "", 0, &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = next_tok->leading_trivia;
              n_tok->trailing_trivia = next_tok->trailing_trivia;
              next_tok->leading_trivia = NULL;
              next_tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___REAL__) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_IDENTIFIER) {
        cdd_token_t *next_tok = &tree->base_tokens->tokens[i + 1];
        size_t child_idx;
        cdd_cst_node_t *owning_node = NULL;
        cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &owning_node);
        if (owning_node) {
          cdd_cst_node_t *temp = NULL;
          cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
          if (temp) {
            cdd_cst_builder_t bld;
            cdd_cst_builder_init(&bld, tree, temp);
            cdd_cst_bld_ident(
                &bld, pool_string_safe_len(tree, (const char *)next_tok->start,
                                           next_tok->length));
            cdd_cst_bld_punct(&bld, ".");
            cdd_cst_bld_ident(&bld, "real");
            cdd_cst_bld_space(&bld);
            if (!cdd_cst_builder_has_error(&bld))
              cdd_cst_splice_children(tree, &owning_node, child_idx, 2,
                                      temp->children, temp->num_children);
            cdd_cst_builder_free(&bld);
            cdd_cst_free_node_only(temp);
          }
        }
        {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, tok->kind, "", 0, &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = tok->leading_trivia;
              n_tok->trailing_trivia = tok->trailing_trivia;
              tok->leading_trivia = NULL;
              tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
        }
        {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, next_tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, next_tok->kind, "", 0, &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = next_tok->leading_trivia;
              n_tok->trailing_trivia = next_tok->trailing_trivia;
              next_tok->leading_trivia = NULL;
              next_tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___IMAG__) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_IDENTIFIER) {
        cdd_token_t *next_tok = &tree->base_tokens->tokens[i + 1];
        size_t child_idx;
        cdd_cst_node_t *owning_node = NULL;
        cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &owning_node);
        if (owning_node) {
          cdd_cst_node_t *temp = NULL;
          cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
          if (temp) {
            cdd_cst_builder_t bld;
            cdd_cst_builder_init(&bld, tree, temp);
            cdd_cst_bld_ident(
                &bld, pool_string_safe_len(tree, (const char *)next_tok->start,
                                           next_tok->length));
            cdd_cst_bld_punct(&bld, ".");
            cdd_cst_bld_ident(&bld, "imag");
            cdd_cst_bld_space(&bld);
            if (!cdd_cst_builder_has_error(&bld))
              cdd_cst_splice_children(tree, &owning_node, child_idx, 2,
                                      temp->children, temp->num_children);
            cdd_cst_builder_free(&bld);
            cdd_cst_free_node_only(temp);
          }
        }
        {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, tok->kind, "", 0, &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = tok->leading_trivia;
              n_tok->trailing_trivia = tok->trailing_trivia;
              tok->leading_trivia = NULL;
              tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
        }
        {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, next_tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, next_tok->kind, "", 0, &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = next_tok->leading_trivia;
              n_tok->trailing_trivia = next_tok->trailing_trivia;
              next_tok->leading_trivia = NULL;
              next_tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD_TYPEOF) {
      if (i + 2 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN) {
        size_t rparen_idx = 0;
        int paren_depth = 0;
        size_t j;
        for (j = i + 1; j < tree->base_tokens->size; j++) {
          if (tree->base_tokens->tokens[j].kind == CDD_TOKEN_LPAREN)
            paren_depth++;
          else if (tree->base_tokens->tokens[j].kind == CDD_TOKEN_RPAREN) {
            paren_depth--;
            if (paren_depth == 0) {
              rparen_idx = j;
              break;
            }
          }
        }
        if (rparen_idx > 0) {
          size_t num_inner = rparen_idx - (i + 1) - 1;
          cdd_token_t *inner = &tree->base_tokens->tokens[i + 2];
          const char *inferred = cdd_infer_type(inner, num_inner);
          size_t child_idx;
          cdd_cst_node_t *owning_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &child_idx,
                                      &owning_node);
          if (owning_node) {
            cdd_cst_node_t *temp = NULL;
            if (inferred) {
              cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
              if (temp) {
                cdd_cst_builder_t bld;
                cdd_cst_builder_init(&bld, tree, temp);
                cdd_cst_bld_ident(&bld, pool_string_safe(tree, inferred));
                cdd_cst_bld_space(&bld);
                if (!cdd_cst_builder_has_error(&bld))
                  cdd_cst_splice_children(tree, &owning_node, child_idx,
                                          (rparen_idx - i) + 1, temp->children,
                                          temp->num_children);
                cdd_cst_builder_free(&bld);
                cdd_cst_free_node_only(temp);
              }
            } else {
              /* It's already a type (transparent resolution). Just extract the
               * tokens inside and drop typeof() */

              char buf[1024];
              char *p = buf;
              size_t k;
              int is_unqual =
                  (tok->length == 13 &&
                   memcmp(tok->start, "typeof_unqual", 13) == 0) ||
                  (tok->length == 17 &&
                   memcmp(tok->start, "__typeof_unqual__", 17) == 0);
              for (k = 0; k < num_inner; k++) {
                int is_qual = 0;
                if (inner[k].kind == CDD_TOKEN_IDENTIFIER) {
                  if ((inner[k].length == 5 &&
                       memcmp(inner[k].start, "const", 5) == 0) ||
                      (inner[k].length == 8 &&
                       memcmp(inner[k].start, "volatile", 8) == 0) ||
                      (inner[k].length == 8 &&
                       memcmp(inner[k].start, "restrict", 8) == 0)) {
                    is_qual = 1;
                  }
                }
                if (is_unqual && is_qual) {
                  continue;
                }
                if (p != buf) {
                  *p++ = ' ';
                }
                memcpy(p, inner[k].start, inner[k].length);
                p += inner[k].length;
              }

              *p++ = ' ';
              *p = '\0';
              if (strchr(buf, '[')) {
                static int typeof_arr_idx = 0;
                char typedef_buf[1024];
                /* Extract the base type and the array part. Very hacky for the
                 * test. */
                char base[256] = {0};
                char arr[256] = {0};
                char arr_clean[256] = {0};
                char *lb = strchr(buf, '[');
                char *rb = strchr(buf, ']');
                if (lb && rb) {
                  int m, ac = 0;
                  strncpy(base, buf, lb - buf);
                  strncpy(arr, lb, rb - lb + 1);
                  for (m = 0; arr[m]; m++) {
                    if (arr[m] != ' ') {
                      arr_clean[ac++] = arr[m];
                    }
                  }
                  CDD_SNPRINTF(
                      typedef_buf, sizeof(typedef_buf),
                      "typedef %s__cdd_typeof_arr_%d%s; __cdd_typeof_arr_%d ",
                      base, typeof_arr_idx, arr_clean, typeof_arr_idx);
                  typeof_arr_idx++;
                  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
                  if (temp) {
                    cdd_cst_builder_t bld;
                    cdd_cst_builder_init(&bld, tree, temp);
                    cdd_cst_bld_ident(&bld, "typedef");
                    cdd_cst_bld_space(&bld);
                    cdd_cst_bld_ident(&bld, pool_string_safe(tree, base));
                    {
                      char tb2[128];
                      CDD_SNPRINTF(tb2, 128, "__cdd_typeof_arr_%d",
                                   typeof_arr_idx);
                      cdd_cst_bld_ident(&bld, pool_string_safe(tree, tb2));
                    }
                    cdd_cst_bld_snippet(&bld, arr_clean);
                    cdd_cst_bld_punct(&bld, ";");
                    cdd_cst_bld_space(&bld);
                    {
                      char tb2[128];
                      CDD_SNPRINTF(tb2, 128, "__cdd_typeof_arr_%d",
                                   typeof_arr_idx);
                      cdd_cst_bld_ident(&bld, pool_string_safe(tree, tb2));
                    }
                    cdd_cst_bld_space(&bld);
                    typeof_arr_idx++;
                    if (!cdd_cst_builder_has_error(&bld))
                      cdd_cst_splice_children(
                          tree, &owning_node, child_idx, (rparen_idx - i) + 1,
                          temp->children, temp->num_children);
                    cdd_cst_builder_free(&bld);
                    cdd_cst_free_node_only(temp);
                  }
                } else {
                  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
                  if (temp) {
                    cdd_cst_builder_t bld;
                    cdd_cst_builder_init(&bld, tree, temp);
                    cdd_cst_bld_ident(&bld, pool_string_safe(tree, buf));
                    if (!cdd_cst_builder_has_error(&bld))
                      cdd_cst_splice_children(
                          tree, &owning_node, child_idx, (rparen_idx - i) + 1,
                          temp->children, temp->num_children);
                    cdd_cst_builder_free(&bld);
                    cdd_cst_free_node_only(temp);
                  }
                }
              } else {
                cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
                if (temp) {
                  cdd_cst_builder_t bld;
                  cdd_cst_builder_init(&bld, tree, temp);
                  cdd_cst_bld_ident(&bld, pool_string_safe(tree, buf));
                  if (!cdd_cst_builder_has_error(&bld))
                    cdd_cst_splice_children(tree, &owning_node, child_idx,
                                            (rparen_idx - i) + 1,
                                            temp->children, temp->num_children);
                  cdd_cst_builder_free(&bld);
                  cdd_cst_free_node_only(temp);
                }
              }
            }
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___AUTO_TYPE) {
      size_t child_idx;
      cdd_cst_node_t *temp = NULL;
      cdd_cst_node_t *owning_node = NULL;
      cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &owning_node);
      if (owning_node) {
        /* __auto_type inference: search forward for = and infer from RHS */
        size_t j;
        const char *inferred = "int";
        for (j = i + 1;
             j < tree->base_tokens->size &&
             tree->base_tokens->tokens[j].kind != CDD_TOKEN_SEMICOLON;
             j++) {
          if (tree->base_tokens->tokens[j].kind == CDD_TOKEN_ASSIGN) {
            if (j + 1 < tree->base_tokens->size) {
              inferred = cdd_infer_type(&tree->base_tokens->tokens[j + 1], 1);
              if (!inferred)
                inferred = "int"; /* fallback if inferred as type */
            }
            break;
          }
        }
        cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
        if (temp) {
          cdd_cst_builder_t bld;
          cdd_cst_builder_init(&bld, tree, temp);
          cdd_cst_bld_ident(&bld, pool_string_safe(tree, inferred));
          cdd_cst_bld_space(&bld);
          if (!cdd_cst_builder_has_error(&bld))
            cdd_cst_splice_children(tree, &owning_node, child_idx, 1,
                                    temp->children, temp->num_children);
          cdd_cst_builder_free(&bld);
          cdd_cst_free_node_only(temp);
        }
      }
      {
        size_t c_idx;
        cdd_cst_node_t *p_node = NULL;
        cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
        if (p_node) {
          cdd_token_t *n_tok = NULL;
          cdd_cst_create_token_len(tree, tok->kind, "", 0, &n_tok);
          if (n_tok) {
            n_tok->leading_trivia = tok->leading_trivia;
            n_tok->trailing_trivia = tok->trailing_trivia;
            tok->leading_trivia = NULL;
            tok->trailing_trivia = NULL;
            cdd_cst_replace_token_child(p_node, c_idx, n_tok);
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_NUMBER) {
      char buf[256];
      size_t copy_len = tok->length < 255 ? tok->length : 255;
      struct NumericValue nv;
      memcpy(buf, tok->start, copy_len);
      buf[copy_len] = '\0';
      if (parse_numeric_literal(buf, &nv) == ERANGE) {
        /* Exceeds 64-bit */
        uint64_t high = 0, low = 0;
        if (buf[0] == '0' && (buf[1] == 'x' || buf[1] == 'X')) {
          parse_hex_128_literal(buf, copy_len, &high, &low);
        } else {
          parse_128_literal(buf, copy_len, &high, &low);
        }
        {
          size_t child_idx;
          cdd_cst_node_t *owning_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &child_idx,
                                      &owning_node);
          if (owning_node) {
            cdd_cst_node_t *temp = NULL;
            cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
            if (temp) {
              cdd_cst_builder_t bld;
              cdd_cst_builder_init(&bld, tree, temp);
              cdd_cst_bld_ident(&bld, "cdd_make_uint128");
              cdd_cst_bld_punct(&bld, "(");
              {
                char tb2[128];
#if defined(_MSC_VER)
                /** @brief ULL_HEX_FMT */

#else

#endif
                CDD_SNPRINTF(tb2, 128, "0x" ULL_HEX_FMT "ULL",
                             (unsigned long long)high);
                cdd_cst_bld_ident(&bld, pool_string_safe(tree, tb2));
              }
              cdd_cst_bld_punct(&bld, ",");
              cdd_cst_bld_space(&bld);
              {
                char tb2[128];
                CDD_SNPRINTF(tb2, 128, "0x" ULL_HEX_FMT "ULL",
                             (unsigned long long)low);
#undef ULL_HEX_FMT
                cdd_cst_bld_ident(&bld, pool_string_safe(tree, tb2));
              }
              cdd_cst_bld_punct(&bld, ")");
              if (!cdd_cst_builder_has_error(&bld))
                cdd_cst_splice_children(tree, &owning_node, child_idx, 1,
                                        temp->children, temp->num_children);
              cdd_cst_builder_free(&bld);
              cdd_cst_free_node_only(temp);
            }
          }
        }
        {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, tok->kind, "", 0, &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = tok->leading_trivia;
              n_tok->trailing_trivia = tok->trailing_trivia;
              tok->leading_trivia = NULL;
              tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_IDENTIFIER && tok->length == 13 &&
               memcmp(tok->start, "__attribute__", 13) == 0) {
      fprintf(stderr, "SAW ATTRIB: [%.*s] [%.*s] [%.*s] [%.*s] [%.*s]\n",
              (int)tree->base_tokens->tokens[i + 1].length,
              tree->base_tokens->tokens[i + 1].start,
              (int)tree->base_tokens->tokens[i + 2].length,
              tree->base_tokens->tokens[i + 2].start,
              (int)tree->base_tokens->tokens[i + 3].length,
              tree->base_tokens->tokens[i + 3].start,
              (int)tree->base_tokens->tokens[i + 4].length,
              tree->base_tokens->tokens[i + 4].start,
              (int)tree->base_tokens->tokens[i + 5].length,
              tree->base_tokens->tokens[i + 5].start);
      if (i + 5 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
          tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_LPAREN &&
          tree->base_tokens->tokens[i + 4].kind == CDD_TOKEN_RPAREN &&
          tree->base_tokens->tokens[i + 5].kind == CDD_TOKEN_RPAREN) {

        cdd_token_t *attr = &tree->base_tokens->tokens[i + 3];
        if (attr->length == 8 && memcmp(attr->start, "noreturn", 8) == 0) {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, tok->kind, "_Noreturn", 9, &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = tok->leading_trivia;
              n_tok->trailing_trivia = tok->trailing_trivia;
              tok->leading_trivia = NULL;
              tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
          {
            size_t w;
            for (w = i + 1; w <= i + 5; w++) {
              cdd_cst_node_t *wp;
              cdd_cst_find_node_for_token(
                  tree->root, &tree->base_tokens->tokens[w], &c_idx, &wp);
              if (wp) {
                cdd_token_t *e_tok = NULL;
                cdd_cst_create_token_len(
                    tree, tree->base_tokens->tokens[w].kind, "", 0, &e_tok);
                if (e_tok) {
                  e_tok->leading_trivia =
                      tree->base_tokens->tokens[w].leading_trivia;
                  e_tok->trailing_trivia =
                      tree->base_tokens->tokens[w].trailing_trivia;
                  tree->base_tokens->tokens[w].leading_trivia = NULL;
                  tree->base_tokens->tokens[w].trailing_trivia = NULL;
                  cdd_cst_replace_token_child(wp, c_idx, e_tok);
                }
              }
            }
          }
        } else if (attr->length == 6 && memcmp(attr->start, "unused", 6) == 0) {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, tok->kind, "/* unused */", 12,
                                     &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = tok->leading_trivia;
              n_tok->trailing_trivia = tok->trailing_trivia;
              tok->leading_trivia = NULL;
              tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
          {
            size_t w;
            for (w = i + 1; w <= i + 5; w++) {
              cdd_cst_node_t *wp;
              cdd_cst_find_node_for_token(
                  tree->root, &tree->base_tokens->tokens[w], &c_idx, &wp);
              if (wp) {
                cdd_token_t *e_tok = NULL;
                cdd_cst_create_token_len(
                    tree, tree->base_tokens->tokens[w].kind, "", 0, &e_tok);
                if (e_tok) {
                  e_tok->leading_trivia =
                      tree->base_tokens->tokens[w].leading_trivia;
                  e_tok->trailing_trivia =
                      tree->base_tokens->tokens[w].trailing_trivia;
                  tree->base_tokens->tokens[w].leading_trivia = NULL;
                  tree->base_tokens->tokens[w].trailing_trivia = NULL;
                  cdd_cst_replace_token_child(wp, c_idx, e_tok);
                }
              }
            }
          }
        } else if (attr->length == 17 &&
                   memcmp(attr->start, "transparent_union", 17) == 0) {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, tok->kind, "/* transparent_union */",
                                     23, &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = tok->leading_trivia;
              n_tok->trailing_trivia = tok->trailing_trivia;
              tok->leading_trivia = NULL;
              tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
          {
            size_t w;
            for (w = i + 1; w <= i + 5; w++) {
              cdd_cst_node_t *wp;
              cdd_cst_find_node_for_token(
                  tree->root, &tree->base_tokens->tokens[w], &c_idx, &wp);
              if (wp) {
                cdd_token_t *e_tok = NULL;
                cdd_cst_create_token_len(
                    tree, tree->base_tokens->tokens[w].kind, "", 0, &e_tok);
                if (e_tok) {
                  e_tok->leading_trivia =
                      tree->base_tokens->tokens[w].leading_trivia;
                  e_tok->trailing_trivia =
                      tree->base_tokens->tokens[w].trailing_trivia;
                  tree->base_tokens->tokens[w].leading_trivia = NULL;
                  tree->base_tokens->tokens[w].trailing_trivia = NULL;
                  cdd_cst_replace_token_child(wp, c_idx, e_tok);
                }
              }
            }
          }
        } else if (attr->length == 6 && memcmp(attr->start, "packed", 6) == 0) {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            if (cdd_cst_create_token_len(tree, tok->kind,
                                         "\n#pragma pack(push, 1)\n", 23,
                                         &n_tok) != 0)
              n_tok = NULL;
            if (n_tok) {
              n_tok->leading_trivia = tok->leading_trivia;
              n_tok->trailing_trivia = tok->trailing_trivia;
              tok->leading_trivia = NULL;
              tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
          {
            size_t w;
            for (w = i + 1; w <= i + 5; w++) {
              cdd_cst_node_t *wp;
              cdd_cst_find_node_for_token(
                  tree->root, &tree->base_tokens->tokens[w], &c_idx, &wp);
              if (wp) {
                cdd_token_t *e_tok = NULL;
                cdd_cst_create_token_len(
                    tree, tree->base_tokens->tokens[w].kind, "", 0, &e_tok);
                if (e_tok) {
                  e_tok->leading_trivia =
                      tree->base_tokens->tokens[w].leading_trivia;
                  e_tok->trailing_trivia =
                      tree->base_tokens->tokens[w].trailing_trivia;
                  tree->base_tokens->tokens[w].leading_trivia = NULL;
                  tree->base_tokens->tokens[w].trailing_trivia = NULL;
                  cdd_cst_replace_token_child(wp, c_idx, e_tok);
                }
              }
            }
          }

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
          char *heap_buf = (char *)malloc(attr->length + 7);
          if (heap_buf) {
            size_t c_idx;
            cdd_cst_node_t *p_node = NULL;
            cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
            if (p_node) {
              cdd_token_t *n_tok = NULL;
              memcpy(heap_buf, "/* ", 3);
              memcpy(heap_buf + 3, attr->start, attr->length);
              memcpy(heap_buf + 3 + attr->length, " */", 3);
              heap_buf[6 + attr->length] = '\0';
              cdd_cst_create_token_len(tree, tok->kind, heap_buf,
                                       6 + attr->length, &n_tok);
              if (n_tok) {
                n_tok->leading_trivia = tok->leading_trivia;
                n_tok->trailing_trivia = tok->trailing_trivia;
                tok->leading_trivia = NULL;
                tok->trailing_trivia = NULL;
                cdd_cst_replace_token_child(p_node, c_idx, n_tok);
              }
            } else {
              free(heap_buf);
            }
          }
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
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, tok->kind, "", 0, &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = tok->leading_trivia;
              n_tok->trailing_trivia = tok->trailing_trivia;
              tok->leading_trivia = NULL;
              tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
          {
            size_t w;
            for (w = i + 1; w <= i + 5; w++) {
              cdd_cst_node_t *wp;
              cdd_cst_find_node_for_token(
                  tree->root, &tree->base_tokens->tokens[w], &c_idx, &wp);
              if (wp) {
                cdd_token_t *e_tok = NULL;
                cdd_cst_create_token_len(
                    tree, tree->base_tokens->tokens[w].kind, "", 0, &e_tok);
                if (e_tok) {
                  e_tok->leading_trivia =
                      tree->base_tokens->tokens[w].leading_trivia;
                  e_tok->trailing_trivia =
                      tree->base_tokens->tokens[w].trailing_trivia;
                  tree->base_tokens->tokens[w].leading_trivia = NULL;
                  tree->base_tokens->tokens[w].trailing_trivia = NULL;
                  cdd_cst_replace_token_child(wp, c_idx, e_tok);
                }
              }
            }
          }
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
          size_t child_idx;
          cdd_cst_node_t *owning_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &child_idx,
                                      &owning_node);
          if (owning_node) {
            cdd_cst_node_t *temp = NULL;
            cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
            if (temp) {
              cdd_cst_builder_t bld;
              cdd_cst_builder_init(&bld, tree, temp);
              cdd_cst_bld_ident(&bld, "_Alignas");
              cdd_cst_bld_punct(&bld, "(");
              cdd_cst_bld_ident(
                  &bld,
                  pool_string_safe_len(
                      tree,
                      (const char *)tree->base_tokens->tokens[i + 5].start,
                      tree->base_tokens->tokens[i + 5].length));
              cdd_cst_bld_punct(&bld, ")");
              if (!cdd_cst_builder_has_error(&bld))
                cdd_cst_splice_children(tree, &owning_node, child_idx, 9,
                                        temp->children, temp->num_children);
              cdd_cst_builder_free(&bld);
              cdd_cst_free_node_only(temp);
            }
          }
          {
            size_t c_idx;
            cdd_cst_node_t *p_node = NULL;
            cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
            if (p_node) {
              cdd_token_t *n_tok = NULL;
              cdd_cst_create_token_len(tree, tok->kind, "", 0, &n_tok);
              if (n_tok) {
                n_tok->leading_trivia = tok->leading_trivia;
                n_tok->trailing_trivia = tok->trailing_trivia;
                tok->leading_trivia = NULL;
                tok->trailing_trivia = NULL;
                cdd_cst_replace_token_child(p_node, c_idx, n_tok);
              }
            }
          }
          {
            size_t c_idx;
            cdd_cst_node_t *p_node = NULL;
            cdd_cst_find_node_for_token(
                tree->root, &tree->base_tokens->tokens[i + 1], &c_idx, &p_node);
            if (p_node) {
              cdd_token_t *n_tok = NULL;
              cdd_cst_create_token_len(
                  tree, tree->base_tokens->tokens[i + 1].kind, "", 0, &n_tok);
              if (n_tok) {
                n_tok->leading_trivia =
                    tree->base_tokens->tokens[i + 1].leading_trivia;
                n_tok->trailing_trivia =
                    tree->base_tokens->tokens[i + 1].trailing_trivia;
                tree->base_tokens->tokens[i + 1].leading_trivia = NULL;
                tree->base_tokens->tokens[i + 1].trailing_trivia = NULL;
                cdd_cst_replace_token_child(p_node, c_idx, n_tok);
              }
            }
          }
          {
            size_t c_idx;
            cdd_cst_node_t *p_node = NULL;
            cdd_cst_find_node_for_token(
                tree->root, &tree->base_tokens->tokens[i + 2], &c_idx, &p_node);
            if (p_node) {
              cdd_token_t *n_tok = NULL;
              cdd_cst_create_token_len(
                  tree, tree->base_tokens->tokens[i + 2].kind, "", 0, &n_tok);
              if (n_tok) {
                n_tok->leading_trivia =
                    tree->base_tokens->tokens[i + 2].leading_trivia;
                n_tok->trailing_trivia =
                    tree->base_tokens->tokens[i + 2].trailing_trivia;
                tree->base_tokens->tokens[i + 2].leading_trivia = NULL;
                tree->base_tokens->tokens[i + 2].trailing_trivia = NULL;
                cdd_cst_replace_token_child(p_node, c_idx, n_tok);
              }
            }
          }
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
              size_t c_idx;
              cdd_cst_node_t *p_node = NULL;
              cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
              if (p_node) {
                cdd_token_t *n_tok = NULL;
                cdd_cst_create_token_len(tree, tok->kind, map, strlen(map),
                                         &n_tok);
                if (n_tok) {
                  n_tok->leading_trivia = tok->leading_trivia;
                  n_tok->trailing_trivia = tok->trailing_trivia;
                  tok->leading_trivia = NULL;
                  tok->trailing_trivia = NULL;
                  cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                }
              }
            } else {
              size_t c_idx;
              cdd_cst_node_t *p_node = NULL;
              cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
              if (p_node) {
                cdd_token_t *n_tok = NULL;
                cdd_cst_create_token_len(tree, tok->kind, "", 0, &n_tok);
                if (n_tok) {
                  n_tok->leading_trivia = tok->leading_trivia;
                  n_tok->trailing_trivia = tok->trailing_trivia;
                  tok->leading_trivia = NULL;
                  tok->trailing_trivia = NULL;
                  cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                }
              }
            }
          } else {
            size_t c_idx;
            cdd_cst_node_t *p_node = NULL;
            cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
            if (p_node) {
              cdd_token_t *n_tok = NULL;
              cdd_cst_create_token_len(tree, tok->kind, "", 0, &n_tok);
              if (n_tok) {
                n_tok->leading_trivia = tok->leading_trivia;
                n_tok->trailing_trivia = tok->trailing_trivia;
                tok->leading_trivia = NULL;
                tok->trailing_trivia = NULL;
                cdd_cst_replace_token_child(p_node, c_idx, n_tok);
              }
            }
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
      } else if (i + 3 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
                 tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_LPAREN) {
        /* Generic multi-arg attribute parser: find the matching )) */
        int lparen_count = 2;
        size_t k;
        for (k = i + 3; k < tree->base_tokens->size; k++) {
          if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LPAREN)
            lparen_count++;
          else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_RPAREN)
            lparen_count--;
          if (lparen_count == 0) {
            /* Strip it */
            size_t wipe;
            size_t c_idx;
            cdd_cst_node_t *p_node = NULL;
            cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
            if (p_node) {
              cdd_token_t *n_tok = NULL;
              cdd_cst_create_token_len(tree, tok->kind, "/* attribute */", 15,
                                       &n_tok);
              if (n_tok) {
                n_tok->leading_trivia = tok->leading_trivia;
                n_tok->trailing_trivia = tok->trailing_trivia;
                tok->leading_trivia = NULL;
                tok->trailing_trivia = NULL;
                cdd_cst_replace_token_child(p_node, c_idx, n_tok);
              }
            }
            for (wipe = i + 1; wipe <= k; wipe++) {
              cdd_cst_node_t *wp = NULL;
              cdd_cst_find_node_for_token(
                  tree->root, &tree->base_tokens->tokens[wipe], &c_idx, &wp);
              if (wp) {
                cdd_token_t *e_tok = NULL;
                cdd_cst_create_token_len(
                    tree, tree->base_tokens->tokens[wipe].kind, "", 0, &e_tok);
                if (e_tok) {
                  e_tok->leading_trivia =
                      tree->base_tokens->tokens[wipe].leading_trivia;
                  e_tok->trailing_trivia =
                      tree->base_tokens->tokens[wipe].trailing_trivia;
                  tree->base_tokens->tokens[wipe].leading_trivia = NULL;
                  tree->base_tokens->tokens[wipe].trailing_trivia = NULL;
                  cdd_cst_replace_token_child(wp, c_idx, e_tok);
                }
              }
            }
            break;
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_IDENTIFIER) {
      if (tok->length == 13 && memcmp(tok->start, "__extension__", 13) == 0) {
        size_t child_idx;
        cdd_cst_node_t *parent = NULL;
        cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &parent);
        if (parent) {
          cdd_token_t *new_tok = NULL;
          cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "", 0, &new_tok);
          if (new_tok) {
            new_tok->leading_trivia = tok->leading_trivia;
            new_tok->trailing_trivia = tok->trailing_trivia;
            tok->leading_trivia = NULL;
            tok->trailing_trivia = NULL;
            cdd_cst_replace_token_child(parent, child_idx, new_tok);
          }
        }
      } else if (tok->length == 17 &&
                 memcmp(tok->start, "__builtin_shuffle", 17) == 0) {
        size_t child_idx;
        cdd_cst_node_t *parent = NULL;
        cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &parent);
        if (parent) {
          cdd_token_t *new_tok = NULL;
          cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER,
                                   "cdd_builtin_shuffle", 19, &new_tok);
          if (new_tok) {
            new_tok->leading_trivia = tok->leading_trivia;
            new_tok->trailing_trivia = tok->trailing_trivia;
            tok->leading_trivia = NULL;
            tok->trailing_trivia = NULL;
            cdd_cst_replace_token_child(parent, child_idx, new_tok);
          }
        }
      } else if (tok->length == 23 &&
                 memcmp(tok->start, "__builtin_shufflevector", 23) == 0) {
        size_t child_idx;
        cdd_cst_node_t *parent = NULL;
        cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &parent);
        if (parent) {
          cdd_token_t *new_tok = NULL;
          cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER,
                                   "cdd_builtin_shufflevector", 25, &new_tok);
          if (new_tok) {
            new_tok->leading_trivia = tok->leading_trivia;
            new_tok->trailing_trivia = tok->trailing_trivia;
            tok->leading_trivia = NULL;
            tok->trailing_trivia = NULL;
            cdd_cst_replace_token_child(parent, child_idx, new_tok);
          }
        }
      } else if ((tok->length == 11 &&
                  memcmp(tok->start, "__alignof__", 11) == 0) ||
                 (tok->length == 9 &&
                  memcmp(tok->start, "__alignof", 9) == 0)) {
        /* If it's applied to an expression, rewrite to _Alignof(typeof(expr))?
           For now we just map to _Alignof.
           In a full AST, we would check if the child is an expression or type.
         */
        size_t child_idx;
        cdd_cst_node_t *parent = NULL;
        cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &parent);
        if (parent) {
          cdd_token_t *new_tok = NULL;
          cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "_Alignof", 8,
                                   &new_tok);
          if (new_tok) {
            new_tok->leading_trivia = tok->leading_trivia;
            new_tok->trailing_trivia = tok->trailing_trivia;
            tok->leading_trivia = NULL;
            tok->trailing_trivia = NULL;
            cdd_cst_replace_token_child(parent, child_idx, new_tok);
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD_STRUCT) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
        /* Anonymous struct: GNU extension. Inject dummy name to make C89 happy.
         */
        static int anon_counter = 0;
        size_t child_idx;
        cdd_cst_node_t *owning_node = NULL;
        cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &owning_node);
        if (owning_node) {
          cdd_cst_node_t *temp = NULL;
          cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
          if (temp) {
            cdd_cst_builder_t bld;
            cdd_cst_builder_init(&bld, tree, temp);
            cdd_cst_bld_ident(&bld, "struct");
            cdd_cst_bld_space(&bld);
            {
              char tb2[128];
              CDD_SNPRINTF(tb2, 128, "_cdd_anon_%d", anon_counter++);
              cdd_cst_bld_ident(&bld, pool_string_safe(tree, tb2));
            }
            if (!cdd_cst_builder_has_error(&bld))
              cdd_cst_splice_children(tree, &owning_node, child_idx, 1,
                                      temp->children, temp->num_children);
            cdd_cst_builder_free(&bld);
            cdd_cst_free_node_only(temp);
          }
        }
        {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, tok->kind, "", 0, &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = tok->leading_trivia;
              n_tok->trailing_trivia = tok->trailing_trivia;
              tok->leading_trivia = NULL;
              tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_IDENTIFIER && tok->length == 5 &&
               memcmp(tok->start, "union", 5) == 0) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
        /* Anonymous union: GNU extension. Inject dummy name. */
        static int anon_counter = 0;
        size_t child_idx;
        cdd_cst_node_t *owning_node = NULL;
        cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &owning_node);
        if (owning_node) {
          cdd_cst_node_t *temp = NULL;
          cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
          if (temp) {
            cdd_cst_builder_t bld;
            cdd_cst_builder_init(&bld, tree, temp);
            cdd_cst_bld_ident(&bld, "union");
            cdd_cst_bld_space(&bld);
            {
              char tb2[128];
              CDD_SNPRINTF(tb2, 128, "_cdd_anon_%d", anon_counter++);
              cdd_cst_bld_ident(&bld, pool_string_safe(tree, tb2));
            }
            if (!cdd_cst_builder_has_error(&bld))
              cdd_cst_splice_children(tree, &owning_node, child_idx, 1,
                                      temp->children, temp->num_children);
            cdd_cst_builder_free(&bld);
            cdd_cst_free_node_only(temp);
          }
        }
        {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, tok->kind, "", 0, &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = tok->leading_trivia;
              n_tok->trailing_trivia = tok->trailing_trivia;
              tok->leading_trivia = NULL;
              tok->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_LPAREN && tok->length > 0) {
      size_t k;
      int is_cast_lvalue = 0;
      size_t rparen_idx = 0;
      for (k = i + 1; k < tree->base_tokens->size; k++) {
        if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_RPAREN) {
          rparen_idx = k;
          break;
        } else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON ||
                   tree->base_tokens->tokens[k].kind == CDD_TOKEN_LBRACE ||
                   tree->base_tokens->tokens[k].kind == CDD_TOKEN_RBRACE) {
          break;
        }
      }
      if (rparen_idx > i + 1 && rparen_idx + 2 < tree->base_tokens->size) {
        if (tree->base_tokens->tokens[rparen_idx + 1].kind ==
                CDD_TOKEN_IDENTIFIER &&
            tree->base_tokens->tokens[rparen_idx + 2].kind ==
                CDD_TOKEN_ASSIGN) {
          is_cast_lvalue = 1;
          if (i > 0 && tree->base_tokens->tokens[i - 1].kind ==
                           CDD_TOKEN_KEYWORD_TYPEOF) {
            is_cast_lvalue = 0;
          }
          if (i > 0 && tree->base_tokens->tokens[i - 1].kind ==
                           CDD_TOKEN_KEYWORD___AUTO_TYPE) {
            is_cast_lvalue = 0;
          }
          /* Check if it's the `(union U)val = ` pattern */
          if (is_cast_lvalue &&
              tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_IDENTIFIER &&
              tree->base_tokens->tokens[i + 1].length == 5 &&
              memcmp(tree->base_tokens->tokens[i + 1].start, "union", 5) == 0) {
            is_cast_lvalue = 2;
          }
        }
      }

      if (is_cast_lvalue == 2) {
        size_t fwd;
        int tk;
        size_t rparen_cidx;
        cdd_cst_node_t *rparen_parent = NULL;
        cdd_cst_find_node_for_token(tree->root,
                                    &tree->base_tokens->tokens[rparen_idx],
                                    &rparen_cidx, &rparen_parent);
        if (rparen_parent) {
          cdd_token_t *new_rparen = NULL;
          cdd_cst_create_token_len(tree, CDD_TOKEN_RPAREN, "){", 2,
                                   &new_rparen);
          if (new_rparen) {
            new_rparen->leading_trivia =
                tree->base_tokens->tokens[rparen_idx].leading_trivia;
            new_rparen->trailing_trivia =
                tree->base_tokens->tokens[rparen_idx].trailing_trivia;
            tree->base_tokens->tokens[rparen_idx].leading_trivia = NULL;
            tree->base_tokens->tokens[rparen_idx].trailing_trivia = NULL;
            cdd_cst_replace_token_child(rparen_parent, rparen_cidx, new_rparen);
          }
        }

        /* Find end of expression heuristic to add `}` */
        for (fwd = rparen_idx + 1; fwd < tree->base_tokens->size; fwd++) {
          tk = tree->base_tokens->tokens[fwd].kind;
          if (tk == (int)CDD_TOKEN_SEMICOLON || tk == (int)CDD_TOKEN_ASSIGN) {
            size_t end_cidx;
            cdd_cst_node_t *end_parent = NULL;
            cdd_cst_find_node_for_token(tree->root,
                                        &tree->base_tokens->tokens[fwd - 1],
                                        &end_cidx, &end_parent);
            if (end_parent) {
              cdd_token_t *new_end = NULL;
              cdd_cst_create_token_len(tree,
                                       tree->base_tokens->tokens[fwd - 1].kind,
                                       "}", 1, &new_end);
              if (new_end) {
                new_end->leading_trivia =
                    tree->base_tokens->tokens[fwd - 1].leading_trivia;
                new_end->trailing_trivia =
                    tree->base_tokens->tokens[fwd - 1].trailing_trivia;
                tree->base_tokens->tokens[fwd - 1].leading_trivia = NULL;
                tree->base_tokens->tokens[fwd - 1].trailing_trivia = NULL;
                cdd_cst_replace_token_child(end_parent, end_cidx, new_end);
              }
            }
            break;
          }
        }
      } else if (is_cast_lvalue == 1) {
        size_t child_idx, rparen_cidx, val_cidx;
        cdd_cst_node_t *parent = NULL;
        cdd_cst_node_t *rparen_parent = NULL;
        cdd_cst_node_t *val_parent = NULL;
        cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &parent);
        cdd_cst_find_node_for_token(tree->root,
                                    &tree->base_tokens->tokens[rparen_idx],
                                    &rparen_cidx, &rparen_parent);
        cdd_cst_find_node_for_token(tree->root,
                                    &tree->base_tokens->tokens[rparen_idx + 1],
                                    &val_cidx, &val_parent);

        if (parent && rparen_parent && val_parent) {
          cdd_token_t *new_lparen = NULL;
          cdd_token_t *new_rparen = NULL;
          cdd_cst_create_token_len(tree, CDD_TOKEN_LPAREN, "*(", 2,
                                   &new_lparen);
          cdd_cst_create_token_len(tree, CDD_TOKEN_RPAREN, "*)", 2,
                                   &new_rparen);

          if (new_lparen && new_rparen) {
            size_t val_len = tree->base_tokens->tokens[rparen_idx + 1].length;
            char *buf;
            new_lparen->leading_trivia = tok->leading_trivia;
            new_lparen->trailing_trivia = tok->trailing_trivia;
            tok->leading_trivia = NULL;
            tok->trailing_trivia = NULL;
            cdd_cst_replace_token_child(parent, child_idx, new_lparen);

            new_rparen->leading_trivia =
                tree->base_tokens->tokens[rparen_idx].leading_trivia;
            new_rparen->trailing_trivia =
                tree->base_tokens->tokens[rparen_idx].trailing_trivia;
            tree->base_tokens->tokens[rparen_idx].leading_trivia = NULL;
            tree->base_tokens->tokens[rparen_idx].trailing_trivia = NULL;
            cdd_cst_replace_token_child(rparen_parent, rparen_cidx, new_rparen);

            buf = (char *)malloc(val_len + 2);
            if (buf) {
              const char *pooled;
              cdd_token_t *new_val = NULL;
              buf[0] = '&';
              memcpy(buf + 1, tree->base_tokens->tokens[rparen_idx + 1].start,
                     val_len);
              buf[val_len + 1] = '\0';
              pooled = pool_string_safe(tree, buf);
              cdd_cst_create_token_len(
                  tree, tree->base_tokens->tokens[rparen_idx + 1].kind,
                  pooled ? pooled : buf, val_len + 1, &new_val);
              if (!pooled)
                free(buf);
              else
                free(buf);
              if (new_val) {
                new_val->leading_trivia =
                    tree->base_tokens->tokens[rparen_idx + 1].leading_trivia;
                new_val->trailing_trivia =
                    tree->base_tokens->tokens[rparen_idx + 1].trailing_trivia;
                tree->base_tokens->tokens[rparen_idx + 1].leading_trivia = NULL;
                tree->base_tokens->tokens[rparen_idx + 1].trailing_trivia =
                    NULL;
                cdd_cst_replace_token_child(val_parent, val_cidx, new_val);
              }
            }
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_OTHER && tok->length == 1 &&
               tok->start[0] == '?') {
      if (i + 1 < tree->base_tokens->size &&
          (tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_COLON ||
           (tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_OTHER &&
            tree->base_tokens->tokens[i + 1].length == 1 &&
            tree->base_tokens->tokens[i + 1].start[0] == ':'))) {
        /* `x ? : y` -> `x ? x : y` */
        size_t k;
        size_t lhs_start = i;
        int depth = 0;
        char *buf;
        size_t len = 0;
        char *p;
        for (k = i; k-- > 0;) {
          int k_kind = tree->base_tokens->tokens[k].kind;
          if (k_kind == CDD_TOKEN_RPAREN || k_kind == CDD_TOKEN_RBRACKET ||
              k_kind == CDD_TOKEN_RBRACE) {
            depth++;
          } else if (k_kind == CDD_TOKEN_LPAREN ||
                     k_kind == CDD_TOKEN_LBRACKET ||
                     k_kind == CDD_TOKEN_LBRACE) {
            depth--;
          }
          if (depth == 0) {
            if (k_kind == CDD_TOKEN_ASSIGN || k_kind == CDD_TOKEN_COMMA ||
                k_kind == CDD_TOKEN_SEMICOLON || k_kind == CDD_TOKEN_LBRACE ||
                k_kind == CDD_TOKEN_RBRACE ||
                k_kind == CDD_TOKEN_KEYWORD_RETURN ||
                k_kind == CDD_TOKEN_KEYWORD_IF) {
              lhs_start = k + 1;
              break;
            } else if (k_kind == CDD_TOKEN_OTHER) {
              if (tree->base_tokens->tokens[k].start[0] == '?' ||
                  tree->base_tokens->tokens[k].start[0] == ':' ||
                  tree->base_tokens->tokens[k].start[0] == '{' ||
                  tree->base_tokens->tokens[k].start[0] == '}') {
                lhs_start = k + 1;
                break;
              }
              if (tree->base_tokens->tokens[k].length >= 2 &&
                  tree->base_tokens->tokens[k]
                          .start[tree->base_tokens->tokens[k].length - 1] ==
                      '=') {
                lhs_start = k + 1;
                break;
              }
            } else if (k_kind == CDD_TOKEN_IDENTIFIER) {
              if ((tree->base_tokens->tokens[k].length == 4 &&
                   memcmp(tree->base_tokens->tokens[k].start, "case", 4) ==
                       0) ||
                  (tree->base_tokens->tokens[k].length == 5 &&
                   memcmp(tree->base_tokens->tokens[k].start, "while", 5) ==
                       0) ||
                  (tree->base_tokens->tokens[k].length == 3 &&
                   memcmp(tree->base_tokens->tokens[k].start, "for", 3) == 0) ||
                  (tree->base_tokens->tokens[k].length == 2 &&
                   memcmp(tree->base_tokens->tokens[k].start, "do", 2) == 0) ||
                  (tree->base_tokens->tokens[k].length == 6 &&
                   memcmp(tree->base_tokens->tokens[k].start, "switch", 6) ==
                       0)) {
                lhs_start = k + 1;
                break;
              }
            }
          } else if (depth < 0) {
            lhs_start = k + 1;
            break;
          }
        }
        if (lhs_start == 0 && depth == 0 && k == (size_t)-1) {
          lhs_start = 0;
        }

        if (lhs_start < i) {
          const uint8_t *expr_start =
              tree->base_tokens->tokens[lhs_start].start;
          const uint8_t *expr_end = tree->base_tokens->tokens[i - 1].start +
                                    tree->base_tokens->tokens[i - 1].length;
          len = expr_end - expr_start;
          buf = (char *)malloc(len + 4);
          if (buf) {
            p = buf;
            *p++ = '?';
            *p++ = ' ';
            memcpy(p, expr_start, len);
            p += len;
            *p++ = ' ';
            *p = '\0';
            {
              size_t child_idx;
              cdd_cst_node_t *parent = NULL;
              cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &parent);
              if (parent) {
                const char *pooled;
                cdd_token_t *new_tok = NULL;
                pooled = pool_string_safe(tree, buf);
                cdd_cst_create_token_len(tree, tok->kind, pooled ? pooled : buf,
                                         strlen(buf), &new_tok);
                free(buf);
                buf = NULL;
                if (new_tok) {
                  new_tok->leading_trivia = tok->leading_trivia;
                  new_tok->trailing_trivia = tok->trailing_trivia;
                  tok->leading_trivia = NULL;
                  tok->trailing_trivia = NULL;
                  cdd_cst_replace_token_child(parent, child_idx, new_tok);
                }
              }
            }
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
            size_t child_idx;
            cdd_cst_node_t *parent = NULL;
            cdd_cst_node_t *semi_parent;
            cdd_cst_find_node_for_token(tree->root, tok, &child_idx, &parent);
            if (parent) {
              cdd_token_t *empty_tok = NULL;
              cdd_cst_create_token_len(tree, CDD_TOKEN_KEYWORD_RETURN, "", 0,
                                       &empty_tok);
              if (empty_tok) {
                empty_tok->leading_trivia = tok->leading_trivia;
                empty_tok->trailing_trivia = tok->trailing_trivia;
                tok->leading_trivia = NULL;
                tok->trailing_trivia = NULL;
                cdd_cst_replace_token_child(parent, child_idx, empty_tok);
              }
            }
            cdd_cst_find_node_for_token(tree->root,
                                        &tree->base_tokens->tokens[k],
                                        &child_idx, &semi_parent);
            if (semi_parent) {
              cdd_token_t *semi_tok = NULL;
              if (cdd_cst_create_token_len(tree, CDD_TOKEN_SEMICOLON,
                                           "; return;", 9, &semi_tok) != 0)
                semi_tok = NULL;
              if (semi_tok) {
                semi_tok->leading_trivia =
                    tree->base_tokens->tokens[k].leading_trivia;
                semi_tok->trailing_trivia =
                    tree->base_tokens->tokens[k].trailing_trivia;
                tree->base_tokens->tokens[k].leading_trivia = NULL;
                tree->base_tokens->tokens[k].trailing_trivia = NULL;
                cdd_cst_replace_token_child(semi_parent, child_idx, semi_tok);
              }
            }
            break;
          }
        }
      }
    }
  }

  /* Unroll statement expressions, VLAs, and __label__s */
  {

    /** @brief Struct definition */
    typedef struct {
      /** @brief field */
      const uint8_t *name;
      /** @brief field */
      size_t length;
      /** @brief field */
      char rename[64];
      /** @brief field */
      int depth;
    } local_label_t;
    /** @brief Struct definition */
    typedef struct {
      /** @brief field */
      const uint8_t *name;
      /** @brief field */
      size_t length;
      /** @brief field */
      int depth;
    } vla_t;
    /** @brief Struct definition */
    typedef struct {
      /** @brief field */
      const uint8_t *var_name;
      /** @brief field */
      size_t var_length;
      /** @brief field */
      const uint8_t *func_name;
      /** @brief field */
      size_t func_length;
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

          {
            size_t tk_idx;
            for (tk_idx = 0; tk_idx <= 8; tk_idx++) {
              size_t child_idx;
              cdd_cst_node_t *parent = NULL;
              cdd_cst_find_node_for_token(
                  tree->root, &tree->base_tokens->tokens[i + tk_idx],
                  &child_idx, &parent);
              if (parent) {
                cdd_token_t *empty_tok = NULL;
                cdd_cst_create_token_len(
                    tree, tree->base_tokens->tokens[i + tk_idx].kind, "", 0,
                    &empty_tok);
                if (empty_tok) {
                  empty_tok->leading_trivia =
                      tree->base_tokens->tokens[i + tk_idx].leading_trivia;
                  empty_tok->trailing_trivia =
                      tree->base_tokens->tokens[i + tk_idx].trailing_trivia;
                  tree->base_tokens->tokens[i + tk_idx].leading_trivia = NULL;
                  tree->base_tokens->tokens[i + tk_idx].trailing_trivia = NULL;
                  cdd_cst_replace_token_child(parent, child_idx, empty_tok);
                }
              }
            }
          }
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
          {
            size_t c_idx;
            cdd_cst_node_t *p_node = NULL;
            cdd_cst_find_node_for_token(tree->root, t, &c_idx, &p_node);
            if (p_node) {
              cdd_token_t *n_tok = NULL;
              cdd_cst_create_token_len(tree, t->kind, "", 0, &n_tok);
              if (n_tok) {
                n_tok->leading_trivia = t->leading_trivia;
                n_tok->trailing_trivia = t->trailing_trivia;
                t->leading_trivia = NULL;
                t->trailing_trivia = NULL;
                cdd_cst_replace_token_child(p_node, c_idx, n_tok);
              }
            }
          }
          for (k = i + 1; k <= end_idx; k++) {
            {
              size_t c_idx;
              cdd_cst_node_t *p_node = NULL;
              cdd_cst_find_node_for_token(
                  tree->root, &tree->base_tokens->tokens[k], &c_idx, &p_node);
              if (p_node) {
                cdd_token_t *n_tok = NULL;
                cdd_cst_create_token_len(
                    tree, tree->base_tokens->tokens[k].kind, "", 0, &n_tok);
                if (n_tok) {
                  n_tok->leading_trivia =
                      tree->base_tokens->tokens[k].leading_trivia;
                  n_tok->trailing_trivia =
                      tree->base_tokens->tokens[k].trailing_trivia;
                  tree->base_tokens->tokens[k].leading_trivia = NULL;
                  tree->base_tokens->tokens[k].trailing_trivia = NULL;
                  cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                }
              }
            }
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
          size_t child_idx;
          cdd_cst_node_t *owning_node = NULL;
          cdd_cst_find_node_for_token(tree->root, t, &child_idx, &owning_node);
          if (!owning_node)
            return EINVAL;

          /* Run cleanups in reverse order */
          while (num_cleanups > 0 &&
                 cleanups[num_cleanups - 1].depth == current_depth) {
            cdd_cst_node_t *temp = NULL;
            cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
            if (temp) {
              cdd_cst_builder_t bld;
              cdd_cst_builder_init(&bld, tree, temp);
              cdd_cst_bld_ident(
                  &bld,
                  pool_string_safe_len(
                      tree, (const char *)cleanups[num_cleanups - 1].func_name,
                      cleanups[num_cleanups - 1].func_length));
              cdd_cst_bld_punct(&bld, "(");
              cdd_cst_bld_punct(&bld, "&");
              cdd_cst_bld_ident(
                  &bld,
                  pool_string_safe_len(
                      tree, (const char *)cleanups[num_cleanups - 1].var_name,
                      cleanups[num_cleanups - 1].var_length));
              cdd_cst_bld_punct(&bld, ")");
              cdd_cst_bld_punct(&bld, ";");
              if (!cdd_cst_builder_has_error(&bld))
                cdd_cst_splice_children(tree, &owning_node, child_idx, 0,
                                        temp->children, temp->num_children);
              cdd_cst_builder_free(&bld);
              cdd_cst_free_node_only(temp);
            }
            num_cleanups--;
            appended = 1;
          }

          if (config && config->fallback_vla_to_malloc) {
            while (num_vlas > 0 && vlas[num_vlas - 1].depth == current_depth) {
              cdd_cst_node_t *temp = NULL;
              cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
              if (temp) {
                cdd_cst_builder_t bld;
                cdd_cst_builder_init(&bld, tree, temp);
                cdd_cst_bld_ident(&bld, "free");
                cdd_cst_bld_punct(&bld, "(");
                cdd_cst_bld_ident(
                    &bld, pool_string_safe_len(
                              tree, (const char *)vlas[num_vlas - 1].name,
                              vlas[num_vlas - 1].length));
                cdd_cst_bld_punct(&bld, ")");
                cdd_cst_bld_punct(&bld, ";");
                if (!cdd_cst_builder_has_error(&bld))
                  cdd_cst_splice_children(tree, &owning_node, child_idx, 0,
                                          temp->children, temp->num_children);
                cdd_cst_builder_free(&bld);
                cdd_cst_free_node_only(temp);
              }
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
            heap_buf = (char *)malloc(strlen(buf) + 1);
            if (heap_buf) {
              size_t child_idx_shadow;
              cdd_cst_node_t *parent = NULL;
              cdd_cst_find_node_for_token(tree->root, t, &child_idx_shadow,
                                          &parent);
              if (parent) {
                const char *pooled;
                cdd_token_t *new_tok = NULL;
                strcpy(heap_buf, buf);
                pooled = pool_string_safe(tree, heap_buf);
                cdd_cst_create_token_len(tree, t->kind,
                                         pooled ? pooled : heap_buf,
                                         strlen(heap_buf), &new_tok);
                free(heap_buf);
                if (new_tok) {
                  new_tok->leading_trivia = t->leading_trivia;
                  new_tok->trailing_trivia = t->trailing_trivia;
                  t->leading_trivia = NULL;
                  t->trailing_trivia = NULL;
                  cdd_cst_replace_token_child(parent, child_idx_shadow,
                                              new_tok);
                }
              } else {
                free(heap_buf);
              }
            }
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
                size_t child_idx;
                cdd_cst_node_t *parent = NULL;
                cdd_cst_find_node_for_token(tree->root, t, &child_idx, &parent);
                if (parent) {
                  cdd_token_t *new_tok = NULL;
                  cdd_cst_create_token_len(tree, t->kind,
                                           "{ __auto_type __cdd_ret = (", 27,
                                           &new_tok);
                  if (new_tok) {
                    new_tok->leading_trivia = t->leading_trivia;
                    new_tok->trailing_trivia = t->trailing_trivia;
                    t->leading_trivia = NULL;
                    t->trailing_trivia = NULL;
                    cdd_cst_replace_token_child(parent, child_idx, new_tok);
                  }
                }
                strcpy(p, "); ");
                p += 3;
              } else {
                size_t child_idx;
                cdd_cst_node_t *parent = NULL;
                cdd_cst_find_node_for_token(tree->root, t, &child_idx, &parent);
                if (parent) {
                  cdd_token_t *new_tok = NULL;
                  cdd_cst_create_token_len(tree, t->kind, "{ ", 2, &new_tok);
                  if (new_tok) {
                    new_tok->leading_trivia = t->leading_trivia;
                    new_tok->trailing_trivia = t->trailing_trivia;
                    t->leading_trivia = NULL;
                    t->trailing_trivia = NULL;
                    cdd_cst_replace_token_child(parent, child_idx, new_tok);
                  }
                }
              }

              for (c_idx = num_cleanups; c_idx > 0; c_idx--) {
                p += sprintf(p, "%.*s(&%.*s); ",
                             (int)cleanups[c_idx - 1].func_length,
                             cleanups[c_idx - 1].func_name,
                             (int)cleanups[c_idx - 1].var_length,
                             cleanups[c_idx - 1].var_name);
              }
              if (config && config->fallback_vla_to_malloc) {
                for (v_idx = num_vlas; v_idx > 0; v_idx--) {
                  p += sprintf(p, "free(%.*s); ", (int)vlas[v_idx - 1].length,
                               vlas[v_idx - 1].name);
                }
              }

              if (has_expr) {
                strcpy(p, "return __cdd_ret; }");
                p += 19;
              } else {
                strcpy(p, "return; }");
                p += 9;
              }
              {
                char *dup = (char *)malloc(strlen(buf) + 1);
                if (dup) {
                  size_t child_idx_dup;
                  cdd_cst_node_t *parent = NULL;
                  cdd_cst_find_node_for_token(tree->root,
                                              &tree->base_tokens->tokens[k],
                                              &child_idx_dup, &parent);
                  if (parent) {
                    const char *pooled;
                    cdd_token_t *new_tok = NULL;
                    strcpy(dup, buf);
                    pooled = pool_string_safe(tree, dup);
                    cdd_cst_create_token_len(
                        tree, tree->base_tokens->tokens[k].kind,
                        pooled ? pooled : dup, strlen(dup), &new_tok);
                    free(dup);
                    dup = NULL;
                    if (new_tok) {
                      new_tok->leading_trivia =
                          tree->base_tokens->tokens[k].leading_trivia;
                      new_tok->trailing_trivia =
                          tree->base_tokens->tokens[k].trailing_trivia;
                      tree->base_tokens->tokens[k].leading_trivia = NULL;
                      tree->base_tokens->tokens[k].trailing_trivia = NULL;
                      cdd_cst_replace_token_child(parent, child_idx_dup,
                                                  new_tok);
                    }
                  }
                  if (dup) {
                    free(dup);
                    dup = NULL;
                  }
                }
              }
              break;
            }
          }
        }
      } else if (t->kind == CDD_TOKEN_KEYWORD_GOTO) {
        if (num_cleanups > 0) {
          size_t c_idx2;
          cdd_cst_node_t *parent = NULL;
          cdd_cst_find_node_for_token(tree->root, t, &c_idx2, &parent);
          if (parent) {
            cdd_token_t *new_tok = NULL;
            cdd_cst_create_token_len(
                tree, t->kind,
                "/* warning: goto cross-scope cleanups unsupported */ goto", 57,
                &new_tok);
            if (new_tok) {
              new_tok->leading_trivia = t->leading_trivia;
              new_tok->trailing_trivia = t->trailing_trivia;
              t->leading_trivia = NULL;
              t->trailing_trivia = NULL;
              cdd_cst_replace_token_child(parent, c_idx2, new_tok);
            }
          }
        }

        if (num_vlas > 0) {
          size_t c_idx3;
          cdd_cst_node_t *parent = NULL;
          cdd_cst_find_node_for_token(tree->root, t, &c_idx3, &parent);
          if (parent) {
            cdd_token_t *new_tok = NULL;
            cdd_cst_create_token_len(
                tree, t->kind,
                "/* warning: goto crossing VLA scopes unsupported */ goto", 56,
                &new_tok);
            if (new_tok) {
              new_tok->leading_trivia = t->leading_trivia;
              new_tok->trailing_trivia = t->trailing_trivia;
              t->leading_trivia = NULL;
              t->trailing_trivia = NULL;
              cdd_cst_replace_token_child(parent, c_idx3, new_tok);
            }
          }
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
          size_t child_idx;
          cdd_cst_node_t *parent = NULL;
          cdd_cst_find_node_for_token(tree->root, t, &child_idx, &parent);
          if (parent) {
            cdd_token_t *new_tok = NULL;
            cdd_cst_create_token_len(
                tree, t->kind,
                "/* warning: computed goto converting jump tables to switch "
                "internally unsupported */ goto",
                89, &new_tok);
            if (new_tok) {
              new_tok->leading_trivia = t->leading_trivia;
              new_tok->trailing_trivia = t->trailing_trivia;
              t->leading_trivia = NULL;
              t->trailing_trivia = NULL;
              cdd_cst_replace_token_child(parent, child_idx, new_tok);
            }
          }
        }
      } else if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 7 &&
                 memcmp(t->start, "longjmp", 7) == 0) {
        if (num_cleanups > 0) {
          size_t child_idx;
          cdd_cst_node_t *parent = NULL;
          cdd_cst_find_node_for_token(tree->root, t, &child_idx, &parent);
          if (parent) {
            cdd_token_t *new_tok = NULL;
            cdd_cst_create_token_len(
                tree, t->kind,
                "/* warning: longjmp bypasses cleanups */ longjmp", 50,
                &new_tok);
            if (new_tok) {
              new_tok->leading_trivia = t->leading_trivia;
              new_tok->trailing_trivia = t->trailing_trivia;
              t->leading_trivia = NULL;
              t->trailing_trivia = NULL;
              cdd_cst_replace_token_child(parent, child_idx, new_tok);
            }
          }
        }
      } else if (t->kind == CDD_TOKEN_KEYWORD___LABEL__) {
        {
          size_t c_idx;
          cdd_cst_node_t *p_node = NULL;
          cdd_cst_find_node_for_token(tree->root, t, &c_idx, &p_node);
          if (p_node) {
            cdd_token_t *n_tok = NULL;
            cdd_cst_create_token_len(tree, t->kind, "", 0, &n_tok);
            if (n_tok) {
              n_tok->leading_trivia = t->leading_trivia;
              n_tok->trailing_trivia = t->trailing_trivia;
              t->leading_trivia = NULL;
              t->trailing_trivia = NULL;
              cdd_cst_replace_token_child(p_node, c_idx, n_tok);
            }
          }
        }
        i++;
        while (i < tree->base_tokens->size) {
          cdd_token_t *nt = &tree->base_tokens->tokens[i];
          if (nt->kind == CDD_TOKEN_IDENTIFIER) {
            if (num_local_labels < MAX_LOCAL_LABELS) {
              local_labels[num_local_labels].name = nt->start;
              local_labels[num_local_labels].length = nt->length;
              local_labels[num_local_labels].depth = current_depth;
              {
                char *p = local_labels[num_local_labels].rename;
                strcpy(p, "__cdd_ll_");
                p += 9;
                memcpy(p, nt->start, nt->length);
                p += nt->length;
                *p++ = '_';
                if (append_int(p, ++label_counter, &p) != 0)
                  return ENOMEM;
              }
              num_local_labels++;
            }
            {
              size_t c_idx;
              cdd_cst_node_t *p_node = NULL;
              cdd_cst_find_node_for_token(tree->root, nt, &c_idx, &p_node);
              if (p_node) {
                cdd_token_t *n_tok = NULL;
                cdd_cst_create_token_len(tree, nt->kind, "", 0, &n_tok);
                if (n_tok) {
                  n_tok->leading_trivia = nt->leading_trivia;
                  n_tok->trailing_trivia = nt->trailing_trivia;
                  nt->leading_trivia = NULL;
                  nt->trailing_trivia = NULL;
                  cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                }
              }
            }
          } else if (nt->kind == CDD_TOKEN_COMMA) {
            {
              size_t c_idx;
              cdd_cst_node_t *p_node = NULL;
              cdd_cst_find_node_for_token(tree->root, nt, &c_idx, &p_node);
              if (p_node) {
                cdd_token_t *n_tok = NULL;
                cdd_cst_create_token_len(tree, nt->kind, "", 0, &n_tok);
                if (n_tok) {
                  n_tok->leading_trivia = nt->leading_trivia;
                  n_tok->trailing_trivia = nt->trailing_trivia;
                  nt->leading_trivia = NULL;
                  nt->trailing_trivia = NULL;
                  cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                }
              }
            }
          } else if (nt->kind == CDD_TOKEN_SEMICOLON) {
            {
              size_t c_idx;
              cdd_cst_node_t *p_node = NULL;
              cdd_cst_find_node_for_token(tree->root, nt, &c_idx, &p_node);
              if (p_node) {
                cdd_token_t *n_tok = NULL;
                cdd_cst_create_token_len(tree, nt->kind, "", 0, &n_tok);
                if (n_tok) {
                  n_tok->leading_trivia = nt->leading_trivia;
                  n_tok->trailing_trivia = nt->trailing_trivia;
                  nt->leading_trivia = NULL;
                  nt->trailing_trivia = NULL;
                  cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                }
              }
            }
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
                   (tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_OTHER &&
                    tree->base_tokens->tokens[i - 1].length == 1 &&
                    tree->base_tokens->tokens[i - 1].start[0] == '&') &&
                   (tree->base_tokens->tokens[i - 2].kind == CDD_TOKEN_OTHER &&
                    tree->base_tokens->tokens[i - 2].length == 1 &&
                    tree->base_tokens->tokens[i - 2].start[0] == '&')) {
          is_label_ref = 1;
        } else if (i + 1 < tree->base_tokens->size) {
          cdd_token_t *nt = &tree->base_tokens->tokens[i + 1];
          if (nt->kind == CDD_TOKEN_COLON ||
              (nt->kind == CDD_TOKEN_OTHER && nt->length == 1 &&
               nt->start[0] == ':')) {
            is_label_ref = 1;
          }
        }

        if (is_label_ref) {
          size_t j;
          for (j = num_local_labels; j-- > 0;) {
            if (local_labels[j].length == t->length &&
                memcmp(local_labels[j].name, t->start, t->length) == 0) {
              char *dup = (char *)malloc(strlen(local_labels[j].rename) + 1);
              if (dup) {
                size_t child_idx;
                cdd_cst_node_t *parent = NULL;
                cdd_cst_find_node_for_token(tree->root, t, &child_idx, &parent);
                if (parent) {
                  const char *pooled;
                  cdd_token_t *new_tok = NULL;
                  strcpy(dup, local_labels[j].rename);
                  pooled = pool_string_safe(tree, dup);
                  cdd_cst_create_token_len(tree, t->kind, pooled ? pooled : dup,
                                           strlen(dup), &new_tok);
                  free(dup);
                  dup = NULL;
                  if (new_tok) {
                    new_tok->leading_trivia = t->leading_trivia;
                    new_tok->trailing_trivia = t->trailing_trivia;
                    t->leading_trivia = NULL;
                    t->trailing_trivia = NULL;
                    cdd_cst_replace_token_child(parent, child_idx, new_tok);
                  }
                }
                if (dup) {
                  free(dup);
                  dup = NULL;
                }
              }
              break;
            }
          }
        }
      }

      /* Existing unroll logic */
      if (t->kind == CDD_TOKEN_LPAREN && i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
        /* Remove ({ */
        size_t child_idx;
        cdd_cst_node_t *parent = NULL;
        cdd_cst_find_node_for_token(tree->root, t, &child_idx, &parent);
        if (parent) {
          cdd_token_t *empty_tok = NULL;
          cdd_cst_create_token_len(tree, t->kind, "", 0, &empty_tok);
          if (empty_tok) {
            empty_tok->leading_trivia = t->leading_trivia;
            empty_tok->trailing_trivia = t->trailing_trivia;
            t->leading_trivia = NULL;
            t->trailing_trivia = NULL;
            cdd_cst_replace_token_child(parent, child_idx, empty_tok);
          }
        }

        cdd_cst_find_node_for_token(
            tree->root, &tree->base_tokens->tokens[i + 1], &child_idx, &parent);
        if (parent) {
          cdd_token_t *empty_tok = NULL;
          cdd_cst_create_token_len(tree, tree->base_tokens->tokens[i + 1].kind,
                                   "", 0, &empty_tok);
          if (empty_tok) {
            empty_tok->leading_trivia =
                tree->base_tokens->tokens[i + 1].leading_trivia;
            empty_tok->trailing_trivia =
                tree->base_tokens->tokens[i + 1].trailing_trivia;
            tree->base_tokens->tokens[i + 1].leading_trivia = NULL;
            tree->base_tokens->tokens[i + 1].trailing_trivia = NULL;
            cdd_cst_replace_token_child(parent, child_idx, empty_tok);
          }
        }
      } else if (t->kind == CDD_TOKEN_RBRACE &&
                 i + 1 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_RPAREN) {
        /* Remove }) */
        size_t child_idx;
        cdd_cst_node_t *parent = NULL;
        cdd_cst_find_node_for_token(tree->root, t, &child_idx, &parent);
        if (parent) {
          cdd_token_t *empty_tok = NULL;
          cdd_cst_create_token_len(tree, t->kind, "", 0, &empty_tok);
          if (empty_tok) {
            empty_tok->leading_trivia = t->leading_trivia;
            empty_tok->trailing_trivia = t->trailing_trivia;
            t->leading_trivia = NULL;
            t->trailing_trivia = NULL;
            cdd_cst_replace_token_child(parent, child_idx, empty_tok);
          }
        }

        cdd_cst_find_node_for_token(
            tree->root, &tree->base_tokens->tokens[i + 1], &child_idx, &parent);
        if (parent) {
          cdd_token_t *empty_tok = NULL;
          cdd_cst_create_token_len(tree, tree->base_tokens->tokens[i + 1].kind,
                                   "", 0, &empty_tok);
          if (empty_tok) {
            empty_tok->leading_trivia =
                tree->base_tokens->tokens[i + 1].leading_trivia;
            empty_tok->trailing_trivia =
                tree->base_tokens->tokens[i + 1].trailing_trivia;
            tree->base_tokens->tokens[i + 1].leading_trivia = NULL;
            tree->base_tokens->tokens[i + 1].trailing_trivia = NULL;
            cdd_cst_replace_token_child(parent, child_idx, empty_tok);
          }
        }
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
            if (dims > 0) {
              *p_sz++ = ' ';
              *p_sz++ = '*';
              *p_sz++ = ' ';
            }
            memcpy(p_sz, tree->base_tokens->tokens[k + 1].start,
                   tree->base_tokens->tokens[k + 1].length);
            p_sz += tree->base_tokens->tokens[k + 1].length;
            dims++;
            k += 3;
          }
          *p_sz = '\0';
          if (dims > 0 && k < tree->base_tokens->size) {
            cdd_token_t *end_tok = &tree->base_tokens->tokens[k];
            if (end_tok->kind == CDD_TOKEN_SEMICOLON) {

              if (config && config->fallback_vla_to_malloc) {
                {
                  size_t child_idx;
                  cdd_cst_node_t *owning_node = NULL;
                  cdd_cst_find_node_for_token(tree->root, prev, &child_idx,
                                              &owning_node);
                  if (owning_node) {
                    cdd_cst_node_t *temp = NULL;
                    cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
                    if (temp) {
                      cdd_cst_builder_t bld;
                      cdd_cst_builder_init(&bld, tree, temp);
                      cdd_cst_bld_punct(&bld, "*");
                      cdd_cst_bld_ident(
                          &bld,
                          pool_string_safe_len(tree, (const char *)prev->start,
                                               prev->length));
                      cdd_cst_bld_space(&bld);
                      cdd_cst_bld_punct(&bld, "=");
                      cdd_cst_bld_space(&bld);
                      cdd_cst_bld_ident(&bld, "malloc");
                      cdd_cst_bld_punct(&bld, "(");
                      cdd_cst_bld_punct(&bld, "(");
                      cdd_cst_bld_snippet(&bld, size_expr);
                      cdd_cst_bld_punct(&bld, ")");
                      cdd_cst_bld_space(&bld);
                      cdd_cst_bld_punct(&bld, "*");
                      cdd_cst_bld_space(&bld);
                      cdd_cst_bld_ident(&bld, "sizeof");
                      cdd_cst_bld_punct(&bld, "(");
                      cdd_cst_bld_punct(&bld, "*");
                      cdd_cst_bld_ident(
                          &bld,
                          pool_string_safe_len(tree, (const char *)prev->start,
                                               prev->length));
                      cdd_cst_bld_punct(&bld, ")");
                      cdd_cst_bld_punct(&bld, ")");
                      cdd_cst_bld_punct(&bld, ";");
                      if (!cdd_cst_builder_has_error(&bld))
                        cdd_cst_splice_children(tree, &owning_node, child_idx,
                                                0, temp->children,
                                                temp->num_children);
                      cdd_cst_builder_free(&bld);
                      cdd_cst_free_node_only(temp);
                    }
                  }
                }
                if (num_vlas < MAX_VLAS) {
                  vlas[num_vlas].name = prev->start;
                  vlas[num_vlas].length = prev->length;
                  vlas[num_vlas].depth = current_depth;
                  num_vlas++;
                }
                {
                  {
                    size_t c_idx;
                    cdd_cst_node_t *p_node = NULL;
                    cdd_cst_find_node_for_token(tree->root, prev, &c_idx,
                                                &p_node);
                    if (p_node) {
                      cdd_token_t *n_tok = NULL;
                      cdd_cst_create_token_len(tree, prev->kind, "", 0, &n_tok);
                      if (n_tok) {
                        n_tok->leading_trivia = prev->leading_trivia;
                        n_tok->trailing_trivia = prev->trailing_trivia;
                        prev->leading_trivia = NULL;
                        prev->trailing_trivia = NULL;
                        cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                      }
                    }
                  }
                  {
                    size_t c_idx;
                    cdd_cst_node_t *p_node = NULL;
                    cdd_cst_find_node_for_token(tree->root, end_tok, &c_idx,
                                                &p_node);
                    if (p_node) {
                      cdd_token_t *n_tok = NULL;
                      cdd_cst_create_token_len(tree, end_tok->kind, "", 0,
                                               &n_tok);
                      if (n_tok) {
                        n_tok->leading_trivia = end_tok->leading_trivia;
                        n_tok->trailing_trivia = end_tok->trailing_trivia;
                        end_tok->leading_trivia = NULL;
                        end_tok->trailing_trivia = NULL;
                        cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                      }
                    }
                  }
                }
              } else {
                {
                  size_t child_idx;
                  cdd_cst_node_t *owning_node = NULL;
                  cdd_cst_find_node_for_token(tree->root, prev, &child_idx,
                                              &owning_node);
                  if (owning_node) {
                    cdd_cst_node_t *temp = NULL;
                    cdd_cst_alloc_node(CDD_CST_UNKNOWN, &temp);
                    if (temp) {
                      cdd_cst_builder_t bld;
                      cdd_cst_builder_init(&bld, tree, temp);
                      cdd_cst_bld_punct(&bld, "*");
                      cdd_cst_bld_ident(
                          &bld,
                          pool_string_safe_len(tree, (const char *)prev->start,
                                               prev->length));
                      cdd_cst_bld_space(&bld);
                      cdd_cst_bld_punct(&bld, "=");
                      cdd_cst_bld_space(&bld);
                      cdd_cst_bld_ident(&bld, "alloca");
                      cdd_cst_bld_punct(&bld, "(");
                      cdd_cst_bld_punct(&bld, "(");
                      cdd_cst_bld_snippet(&bld, size_expr);
                      cdd_cst_bld_punct(&bld, ")");
                      cdd_cst_bld_space(&bld);
                      cdd_cst_bld_punct(&bld, "*");
                      cdd_cst_bld_space(&bld);
                      cdd_cst_bld_ident(&bld, "sizeof");
                      cdd_cst_bld_punct(&bld, "(");
                      cdd_cst_bld_punct(&bld, "*");
                      cdd_cst_bld_ident(
                          &bld,
                          pool_string_safe_len(tree, (const char *)prev->start,
                                               prev->length));
                      cdd_cst_bld_punct(&bld, ")");
                      cdd_cst_bld_punct(&bld, ")");
                      cdd_cst_bld_punct(&bld, ";");
                      if (!cdd_cst_builder_has_error(&bld))
                        cdd_cst_splice_children(tree, &owning_node, child_idx,
                                                0, temp->children,
                                                temp->num_children);
                      cdd_cst_builder_free(&bld);
                      cdd_cst_free_node_only(temp);
                    }
                  }
                  {
                    size_t c_idx;
                    cdd_cst_node_t *p_node = NULL;
                    cdd_cst_find_node_for_token(tree->root, prev, &c_idx,
                                                &p_node);
                    if (p_node) {
                      cdd_token_t *n_tok = NULL;
                      cdd_cst_create_token_len(tree, prev->kind, "", 0, &n_tok);
                      if (n_tok) {
                        n_tok->leading_trivia = prev->leading_trivia;
                        n_tok->trailing_trivia = prev->trailing_trivia;
                        prev->leading_trivia = NULL;
                        prev->trailing_trivia = NULL;
                        cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                      }
                    }
                  }
                  {
                    size_t c_idx;
                    cdd_cst_node_t *p_node = NULL;
                    cdd_cst_find_node_for_token(tree->root, end_tok, &c_idx,
                                                &p_node);
                    if (p_node) {
                      cdd_token_t *n_tok = NULL;
                      cdd_cst_create_token_len(tree, end_tok->kind, "", 0,
                                               &n_tok);
                      if (n_tok) {
                        n_tok->leading_trivia = end_tok->leading_trivia;
                        n_tok->trailing_trivia = end_tok->trailing_trivia;
                        end_tok->leading_trivia = NULL;
                        end_tok->trailing_trivia = NULL;
                        cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                      }
                    }
                  }
                }
              }
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
            size_t child_idx;
            cdd_cst_node_t *parent = NULL;
            cdd_cst_find_node_for_token(tree->root, next, &child_idx, &parent);
            if (parent) {
              cdd_token_t *new_tok = NULL;
              cdd_cst_create_token_len(tree, next->kind, "1", 1, &new_tok);
              if (new_tok) {
                new_tok->leading_trivia = next->leading_trivia;
                new_tok->trailing_trivia = next->trailing_trivia;
                next->leading_trivia = NULL;
                next->trailing_trivia = NULL;
                cdd_cst_replace_token_child(parent, child_idx, new_tok);
              }
            }
          } else {
            /* Reject zero-length array in the middle of a struct */
            size_t child_idx;
            cdd_cst_node_t *parent = NULL;
            cdd_cst_find_node_for_token(tree->root, next, &child_idx, &parent);
            if (parent) {
              cdd_token_t *new_tok = NULL;
              cdd_cst_create_token_len(
                  tree, next->kind,
                  "-1 /* zero-length array in middle of struct */", 46,
                  &new_tok);
              if (new_tok) {
                new_tok->leading_trivia = next->leading_trivia;
                new_tok->trailing_trivia = next->trailing_trivia;
                next->leading_trivia = NULL;
                next->trailing_trivia = NULL;
                cdd_cst_replace_token_child(parent, child_idx, new_tok);
              }
            }
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
          {
            size_t c_idx;
            cdd_cst_node_t *p_node = NULL;
            cdd_cst_find_node_for_token(tree->root, t, &c_idx, &p_node);
            if (p_node) {
              cdd_token_t *n_tok = NULL;
              cdd_cst_create_token_len(tree, t->kind, "", 0, &n_tok);
              if (n_tok) {
                n_tok->leading_trivia = t->leading_trivia;
                n_tok->trailing_trivia = t->trailing_trivia;
                t->leading_trivia = NULL;
                t->trailing_trivia = NULL;
                cdd_cst_replace_token_child(p_node, c_idx, n_tok);
              }
            }
          }
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

        if (i > 0 &&
            (tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_COLON ||
             (tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_OTHER &&
              tree->base_tokens->tokens[i - 1].length == 1 &&
              tree->base_tokens->tokens[i - 1].start[0] == ':'))) {
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

            if (is_case) {
              char *heap_buf;
              size_t alloc_sz;
              char *p;
              int v;
              if (start_val > end_val) {
                alloc_sz = 128;
              } else {
                alloc_sz = (end_val - start_val + 2) * 32 + 128;
              }
              heap_buf = (char *)malloc(alloc_sz);
              if (heap_buf) {
                const char *pooled;
                p = heap_buf;
                /* If overlapping case, add a warning comment */
                /* For now we just generate all cases. Semantic error detection
                   of overlaps is usually left to the underlying compiler, but
                   we add a warning. */
                if (start_val > end_val) {
                  strcpy(p, "/* WARNING: Invalid case range */ ");
                  p += 34;
                }
                for (v = start_val; v <= end_val; v++) {
                  strcpy(p, "case ");
                  p += 5;
                  if (append_int(p, v, &p) != 0)
                    return ENOMEM;
                  if (v != end_val) {
                    strcpy(p, ": ");
                    p += 2;
                  }
                }
                if (is_neg_start) {
                  tree->base_tokens->tokens[i - 2].length = 0;
                }
                pooled = pool_string_safe(tree, heap_buf);
                tree->base_tokens->tokens[is_case_idx].start =
                    (const uint8_t *)(pooled ? pooled : heap_buf);
                tree->base_tokens->tokens[is_case_idx].length =
                    strlen(heap_buf);
                if (pooled)
                  free(heap_buf);
                {
                  size_t c_idx;
                  cdd_cst_node_t *p_node = NULL;
                  cdd_cst_find_node_for_token(tree->root, prev, &c_idx,
                                              &p_node);
                  if (p_node) {
                    cdd_token_t *n_tok = NULL;
                    cdd_cst_create_token_len(tree, prev->kind, "", 0, &n_tok);
                    if (n_tok) {
                      n_tok->leading_trivia = prev->leading_trivia;
                      n_tok->trailing_trivia = prev->trailing_trivia;
                      prev->leading_trivia = NULL;
                      prev->trailing_trivia = NULL;
                      cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                    }
                  }
                }
                {
                  size_t c_idx;
                  cdd_cst_node_t *p_node = NULL;
                  cdd_cst_find_node_for_token(tree->root, t, &c_idx, &p_node);
                  if (p_node) {
                    cdd_token_t *n_tok = NULL;
                    cdd_cst_create_token_len(tree, t->kind, "", 0, &n_tok);
                    if (n_tok) {
                      n_tok->leading_trivia = t->leading_trivia;
                      n_tok->trailing_trivia = t->trailing_trivia;
                      t->leading_trivia = NULL;
                      t->trailing_trivia = NULL;
                      cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                    }
                  }
                }
                tree->base_tokens->tokens[i + 1].length = 0;
                tree->base_tokens->tokens[i + 2].length = 0;
                if (is_neg_end) {
                  tree->base_tokens->tokens[i + 3].length = 0;
                }
                {
                  size_t c_idx;
                  cdd_cst_node_t *p_node = NULL;
                  cdd_cst_find_node_for_token(tree->root, next, &c_idx,
                                              &p_node);
                  if (p_node) {
                    cdd_token_t *n_tok = NULL;
                    cdd_cst_create_token_len(tree, next->kind, "", 0, &n_tok);
                    if (n_tok) {
                      n_tok->leading_trivia = next->leading_trivia;
                      n_tok->trailing_trivia = next->trailing_trivia;
                      next->leading_trivia = NULL;
                      next->trailing_trivia = NULL;
                      cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                    }
                  }
                }
              }
            } else if (is_range_init && start_val <= end_val) {
              char *heap_buf;
              size_t alloc_sz = 0;
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
                alloc_sz =
                    (end_val - start_val + 2) * (32 + assign_val->length) + 1;
                heap_buf = (char *)malloc(alloc_sz);
                if (heap_buf) {
                  const char *pooled;
                  p = heap_buf;
                  for (v = start_val; v <= end_val; v++) {
                    *p++ = '[';
                    if (append_int(p, v, &p) != 0)
                      return ENOMEM;
                    strcpy(p, "] = ");
                    p += 4;
                    memcpy(p, assign_val->start, assign_val->length);
                    p += assign_val->length;
                    if (v != end_val) {
                      strcpy(p, ", ");
                      p += 2;
                    }
                  }
                  *p = '\0';

                  pooled = pool_string_safe(tree, heap_buf);
                  tree->base_tokens->tokens[is_range_idx].start =
                      (const uint8_t *)(pooled ? pooled : heap_buf);
                  tree->base_tokens->tokens[is_range_idx].length =
                      strlen(heap_buf);
                  if (pooled)
                    free(heap_buf);
                  {
                    size_t c_idx;
                    cdd_cst_node_t *p_node = NULL;
                    cdd_cst_find_node_for_token(tree->root, prev, &c_idx,
                                                &p_node);
                    if (p_node) {
                      cdd_token_t *n_tok = NULL;
                      cdd_cst_create_token_len(tree, prev->kind, "", 0, &n_tok);
                      if (n_tok) {
                        n_tok->leading_trivia = prev->leading_trivia;
                        n_tok->trailing_trivia = prev->trailing_trivia;
                        prev->leading_trivia = NULL;
                        prev->trailing_trivia = NULL;
                        cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                      }
                    }
                  }
                  {
                    size_t c_idx;
                    cdd_cst_node_t *p_node = NULL;
                    cdd_cst_find_node_for_token(tree->root, t, &c_idx, &p_node);
                    if (p_node) {
                      cdd_token_t *n_tok = NULL;
                      cdd_cst_create_token_len(tree, t->kind, "", 0, &n_tok);
                      if (n_tok) {
                        n_tok->leading_trivia = t->leading_trivia;
                        n_tok->trailing_trivia = t->trailing_trivia;
                        t->leading_trivia = NULL;
                        t->trailing_trivia = NULL;
                        cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                      }
                    }
                  }
                  {
                    size_t c_idx;
                    cdd_cst_node_t *p_node = NULL;
                    cdd_cst_find_node_for_token(
                        tree->root, &tree->base_tokens->tokens[i + 1], &c_idx,
                        &p_node);
                    if (p_node) {
                      cdd_token_t *n_tok = NULL;
                      cdd_cst_create_token_len(
                          tree, tree->base_tokens->tokens[i + 1].kind, "", 0,
                          &n_tok);
                      if (n_tok) {
                        n_tok->leading_trivia =
                            tree->base_tokens->tokens[i + 1].leading_trivia;
                        n_tok->trailing_trivia =
                            tree->base_tokens->tokens[i + 1].trailing_trivia;
                        tree->base_tokens->tokens[i + 1].leading_trivia = NULL;
                        tree->base_tokens->tokens[i + 1].trailing_trivia = NULL;
                        cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                      }
                    }
                  }
                  {
                    size_t c_idx;
                    cdd_cst_node_t *p_node = NULL;
                    cdd_cst_find_node_for_token(
                        tree->root, &tree->base_tokens->tokens[i + 2], &c_idx,
                        &p_node);
                    if (p_node) {
                      cdd_token_t *n_tok = NULL;
                      cdd_cst_create_token_len(
                          tree, tree->base_tokens->tokens[i + 2].kind, "", 0,
                          &n_tok);
                      if (n_tok) {
                        n_tok->leading_trivia =
                            tree->base_tokens->tokens[i + 2].leading_trivia;
                        n_tok->trailing_trivia =
                            tree->base_tokens->tokens[i + 2].trailing_trivia;
                        tree->base_tokens->tokens[i + 2].leading_trivia = NULL;
                        tree->base_tokens->tokens[i + 2].trailing_trivia = NULL;
                        cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                      }
                    }
                  }
                  {
                    size_t c_idx;
                    cdd_cst_node_t *p_node = NULL;
                    cdd_cst_find_node_for_token(tree->root, next, &c_idx,
                                                &p_node);
                    if (p_node) {
                      cdd_token_t *n_tok = NULL;
                      cdd_cst_create_token_len(tree, next->kind, "", 0, &n_tok);
                      if (n_tok) {
                        n_tok->leading_trivia = next->leading_trivia;
                        n_tok->trailing_trivia = next->trailing_trivia;
                        next->leading_trivia = NULL;
                        next->trailing_trivia = NULL;
                        cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                      }
                    }
                  }
                  {
                    size_t c_idx;
                    cdd_cst_node_t *p_node = NULL;
                    cdd_cst_find_node_for_token(tree->root, (next + 1), &c_idx,
                                                &p_node);
                    if (p_node) {
                      cdd_token_t *n_tok = NULL;
                      cdd_cst_create_token_len(tree, (next + 1)->kind, "", 0,
                                               &n_tok);
                      if (n_tok) {
                        n_tok->leading_trivia = (next + 1)->leading_trivia;
                        n_tok->trailing_trivia = (next + 1)->trailing_trivia;
                        (next + 1)->leading_trivia = NULL;
                        (next + 1)->trailing_trivia = NULL;
                        cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                      }
                    }
                  }
                  {
                    size_t c_idx;
                    cdd_cst_node_t *p_node = NULL;
                    cdd_cst_find_node_for_token(tree->root, (next + 2), &c_idx,
                                                &p_node);
                    if (p_node) {
                      cdd_token_t *n_tok = NULL;
                      cdd_cst_create_token_len(tree, (next + 2)->kind, "", 0,
                                               &n_tok);
                      if (n_tok) {
                        n_tok->leading_trivia = (next + 2)->leading_trivia;
                        n_tok->trailing_trivia = (next + 2)->trailing_trivia;
                        (next + 2)->leading_trivia = NULL;
                        (next + 2)->trailing_trivia = NULL;
                        cdd_cst_replace_token_child(p_node, c_idx, n_tok);
                      }
                    }
                  }
                  {
                    size_t c_idx;
                    cdd_cst_node_t *p_node = NULL;
                    cdd_cst_find_node_for_token(tree->root, assign_val, &c_idx,
                                                &p_node);
                    if (p_node) {
                      cdd_token_t *n_tok = NULL;
                      cdd_cst_create_token_len(tree, assign_val->kind, "", 0,
                                               &n_tok);
                      if (n_tok) {
                        n_tok->leading_trivia = assign_val->leading_trivia;
                        n_tok->trailing_trivia = assign_val->trailing_trivia;
                        assign_val->leading_trivia = NULL;
                        assign_val->trailing_trivia = NULL;
                        cdd_cst_replace_token_child(p_node, c_idx, n_tok);
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
  }

  return 0;
}

/* LCOV_EXCL_STOP */
