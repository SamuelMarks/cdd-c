/**
 * @file cdd_cst_transform.c
 * @brief Concrete Syntax Tree (CST) transform implementation and utilities.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_cst_transform.h"
#include "c_cdd/memory.h"
#include "c_str_span.h"
#include "classes/parse/cdd_cst_factory.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef CDD_CST_TRANSFORM_LEGACY_STANDALONE
/** @brief Standalone test symbol name for extern "C" CST transform. */
#define CDD_TRANSFORM_EXTERN_C_FN cdd_legacy_transform_extern_c
/** @brief Standalone test symbol name for MSVC CST transform. */
#define CDD_TRANSFORM_MSVC_FN cdd_legacy_transform_msvc
/** @brief Standalone test symbol name for GNU CST transform. */
#define CDD_TRANSFORM_GNU_FN cdd_legacy_transform_gnu
/**
 * @brief Standalone test entrypoint `cdd_legacy_transform_extern_c`.
 * @param tree Parameter `tree`.
 * @param config Parameter `config`.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
NO_DISCARD cdd_c_error_t cdd_legacy_transform_extern_c(
    cdd_cst_tree_t *tree, const cdd_transform_config_t *config);
/**
 * @brief Standalone test entrypoint `cdd_legacy_transform_msvc`.
 * @param tree Parameter `tree`.
 * @param config Parameter `config`.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
NO_DISCARD cdd_c_error_t cdd_legacy_transform_msvc(
    cdd_cst_tree_t *tree, const cdd_transform_config_t *config);
/**
 * @brief Standalone test entrypoint `cdd_legacy_transform_gnu`.
 * @param tree Parameter `tree`.
 * @param config Parameter `config`.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
NO_DISCARD cdd_c_error_t cdd_legacy_transform_gnu(
    cdd_cst_tree_t *tree, const cdd_transform_config_t *config);
#else
/** @brief Production symbol name for extern "C" CST transform. */
#define CDD_TRANSFORM_EXTERN_C_FN cdd_transform_extern_c
/** @brief Production symbol name for MSVC CST transform. */
#define CDD_TRANSFORM_MSVC_FN cdd_transform_msvc
/** @brief Production symbol name for GNU CST transform. */
#define CDD_TRANSFORM_GNU_FN cdd_transform_gnu
#endif

/**
 * @brief Checks whether a CST token slice contains a given ASCII substring.
 *
 * @param[in] tok Token whose byte slice is inspected.
 * @param[in] needle Null-terminated substring to locate.
 * @param[out] out_found Set to 1 if `needle` occurs in `tok`, otherwise 0.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT`.
 */
static NO_DISCARD cdd_c_error_t cdd_legacy_tok_contains(const cdd_token_t *tok,
                                                        const char *needle,
                                                        int *out_found) {
  size_t nlen;
  size_t off;
  if (!tok)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_found = 0;
  nlen = strlen(needle);
  if (tok->length < nlen)
    return CDD_C_SUCCESS;
  for (off = 0; off + nlen <= tok->length; off++) {
    if (memcmp(tok->start + off, needle, nlen) == 0) {
      *out_found = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Stores a heap-duplicated string in `tree->string_pool` so
 * `cdd_cst_tree_free` releases it automatically.
 *
 * @param[in,out] tree Owning CST tree.
 * @param[in] text Null-terminated string to duplicate and pool.
 * @param[out] out_pooled Output pointer receiving the pooled string buffer.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static NO_DISCARD cdd_c_error_t cdd_legacy_pool_str(cdd_cst_tree_t *tree,
                                                    const char *text,
                                                    char **out_pooled) {
  char *dup;
  if (tree->num_strings >= tree->string_capacity) {
    size_t new_cap = tree->string_capacity == 0 ? 8 : tree->string_capacity * 2;
    char **new_pool =
        (char **)C_CDD_REALLOC(tree->string_pool, new_cap * sizeof(char *));
    if (!new_pool)
      return CDD_C_ERROR_MEMORY;
    tree->string_pool = new_pool;
    tree->string_capacity = new_cap;
  }
  dup = C_CDD_STRDUP(text);
  if (!dup)
    return CDD_C_ERROR_MEMORY;
  tree->string_pool[tree->num_strings++] = dup;
  *out_pooled = dup;
  return CDD_C_SUCCESS;
}

/**
 * @brief Synthesizes a `CDD_CST_PREPROC_CONDITIONAL` guard node containing a
 * single `CDD_TOKEN_PREPROC_IFDEF` literal token.
 *
 * @param[in,out] tree Owning CST tree.
 * @param[in] guard_text Static string literal for the guard block.
 * @param[out] out_node Output pointer receiving the allocated CST node.
 * @return `CDD_C_SUCCESS` on success, or an error code on OOM.
 */
static NO_DISCARD cdd_c_error_t cdd_legacy_make_guard_node(
    cdd_cst_tree_t *tree, const char *guard_text, cdd_cst_node_t **out_node) {
  cdd_cst_node_t *node = NULL;
  cdd_token_t *tok = NULL;
  cdd_c_error_t rc;
  rc = cdd_cst_alloc_node(CDD_CST_PREPROC_CONDITIONAL, &node);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_cst_create_token(tree, CDD_TOKEN_PREPROC_IFDEF, guard_text, &tok);
  if (rc != CDD_C_SUCCESS)
    return cdd_c_keep_err(rc, cdd_cst_free_node(node));
  rc = cdd_cst_append_child_token(node, tok);
  if (rc != CDD_C_SUCCESS)
    return cdd_c_keep_err(rc, cdd_cst_free_node(node));
  *out_node = node;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a C preprocessor snippet and clones its first top-level CST
 * node into `dest_tree`.
 *
 * @param[in,out] dest_tree Destination CST tree owning cloned tokens.
 * @param[in] snippet Source snippet to parse and clone.
 * @param[out] out_cloned Output pointer receiving the cloned CST node.
 * @return `CDD_C_SUCCESS` on success, or an error code on OOM/parse failure.
 */
static NO_DISCARD cdd_c_error_t
cdd_legacy_parse_and_clone_first(cdd_cst_tree_t *dest_tree, const char *snippet,
                                 cdd_cst_node_t **out_cloned) {
  cdd_cst_tree_t *frag_tree = NULL;
  cdd_c_error_t rc;
  rc = cdd_cst_parse(az_span_create_from_str(snippet), &frag_tree);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_cst_clone_tree(dest_tree, frag_tree->root->children[0].val.node,
                          out_cloned);
  return cdd_c_keep_err(rc, cdd_cst_tree_free(frag_tree));
}

/**
 * @brief Replaces a POSIX `#include` directive node with an `_MSC_VER`-guarded
 * conditional block.
 *
 * @param[in,out] tree Owning CST tree.
 * @param[in,out] dir Existing `#include` directive CST node to replace.
 * @param[in] guarded_snippet Replacement `#ifndef _MSC_VER ... #endif` snippet.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static NO_DISCARD cdd_c_error_t cdd_legacy_replace_posix_include(
    cdd_cst_tree_t *tree, cdd_cst_node_t *dir, const char *guarded_snippet) {
  cdd_cst_node_t *cloned = NULL;
  cdd_c_error_t rc;
  rc = cdd_legacy_parse_and_clone_first(tree, guarded_snippet, &cloned);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_cst_replace_node(tree, dir, cloned);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_cst_free_node(cloned));
  }
  return cdd_cst_free_node(dir);
}

/**
 * @brief Wraps a CST header tree in `#ifdef __cplusplus extern "C" { #endif`
 * guards if not already present.
 *
 * @param[in,out] tree CST tree to transform.
 * @param[in] config Optional transformation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
cdd_c_error_t CDD_TRANSFORM_EXTERN_C_FN(cdd_cst_tree_t *tree,
                                        const cdd_transform_config_t *config) {
  cdd_c_error_t rc;
  cdd_cst_query_result_t res;
  size_t i;
  int found_cpp = (int)((size_t)config & 0);
  int stop_scan = 0;
  cdd_cst_node_t *child;
  cdd_cst_node_t *first_node = NULL;
  cdd_cst_node_t *last_node = NULL;
  cdd_cst_node_t *insert_after_node = NULL;
  cdd_cst_node_t *top_cloned = NULL;
  cdd_cst_node_t *bot_cloned = NULL;

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE) {
      if (!first_node)
        first_node = tree->root->children[i].val.node;
      last_node = tree->root->children[i].val.node;
    }
  }
  if (!first_node)
    return CDD_C_SUCCESS;

  /* 1. Check if __cplusplus is already checked in a conditional block */
  rc =
      cdd_cst_find_nodes_by_type(tree->root, CDD_CST_PREPROC_CONDITIONAL, &res);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < res.size; i++) {
    cdd_cst_node_t *cond = res.nodes[i];
    if (cond->num_children > 0 &&
        cond->children[0].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *tok = cond->children[0].val.token;
      int has_cpp = 0;
      rc = cdd_legacy_tok_contains(tok, "__cplusplus", &has_cpp);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(res.nodes);
        return rc;
      }
      if (tok->kind == CDD_TOKEN_PREPROC_IFDEF && has_cpp) {
        found_cpp = 1;
        break;
      }
    }
  }
  C_CDD_FREE(res.nodes);

  if (found_cpp)
    return CDD_C_SUCCESS;

  /* 2. Find insertion boundary: after #includes, before C declarations */
  for (i = 0; i < tree->root->num_children && !stop_scan; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE) {
      child = tree->root->children[i].val.node;
      if (child->kind == CDD_CST_PREPROC_DIRECTIVE && child->num_children > 0 &&
          child->children[0].kind == CDD_CST_CHILD_TOKEN &&
          child->children[0].val.token->kind == CDD_TOKEN_PREPROC_INCLUDE) {
        insert_after_node = child;
      } else if (child->kind == CDD_CST_DECLARATION ||
                 child->kind == CDD_CST_FUNCTION_DEFINITION) {
        stop_scan = 1;
      }
    }
  }

  /* 3. Synthesize and insert opening guard */
  rc = cdd_legacy_make_guard_node(
      tree, "\n#ifdef __cplusplus\nextern \"C\" {\n#endif /* __cplusplus */\n",
      &top_cloned);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (insert_after_node) {
    rc = cdd_cst_insert_node_after(insert_after_node, top_cloned);
  } else {
    rc = cdd_cst_insert_node_before(first_node, top_cloned);
  }
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_cst_free_node(top_cloned));
  }

  /* 4. Synthesize and insert closing guard */
  rc = cdd_legacy_make_guard_node(
      tree, "\n#ifdef __cplusplus\n}\n#endif /* __cplusplus */\n", &bot_cloned);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_cst_insert_node_after(last_node, bot_cloned);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_cst_free_node(bot_cloned));
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Transforms POSIX headers and identifiers in a CST for MSVC
 * compatibility.
 *
 * @param[in,out] tree CST tree to transform.
 * @param[in] config Optional transformation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
cdd_c_error_t CDD_TRANSFORM_MSVC_FN(cdd_cst_tree_t *tree,
                                    const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i = (size_t)config & 0;
  cdd_c_error_t rc;

  if (!tree || !tree->root || !tree->base_tokens)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* 1. Wrap unistd.h and sys/time.h */
  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_PREPROC_DIRECTIVE, &res);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < res.size; i++) {
    cdd_cst_node_t *dir = res.nodes[i];
    if (dir->num_children > 0 && dir->children[0].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *tok = dir->children[0].val.token;
      int is_unistd = 0;
      int is_systime = 0;
      rc = cdd_legacy_tok_contains(tok, "<unistd.h>", &is_unistd);
      if (rc == CDD_C_SUCCESS && !is_unistd) {
        rc = cdd_legacy_tok_contains(tok, "<sys/time.h>", &is_systime);
      }
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(res.nodes);
        return rc;
      }
      if (tok->kind == CDD_TOKEN_PREPROC_INCLUDE) {
        if (is_unistd) {
          rc = cdd_legacy_replace_posix_include(
              tree, dir,
              "#ifndef _MSC_VER\n#include <unistd.h>\n#else\n#include "
              "\"win_compat_sym.h\"\n#endif\n");
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(res.nodes);
            return rc;
          }
        } else if (is_systime) {
          rc = cdd_legacy_replace_posix_include(
              tree, dir,
              "#ifndef _MSC_VER\n#include <sys/time.h>\n#else\n#include "
              "\"win_compat_sym.h\"\n#endif\n");
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(res.nodes);
            return rc;
          }
        }
      }
    }
  }
  C_CDD_FREE(res.nodes);

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

  return CDD_C_SUCCESS;
}

/**
 * @brief Transforms GNU C extensions in a CST into portable standard C
 * constructs.
 *
 * @param[in,out] tree CST tree to transform.
 * @param[in] config Optional transformation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code.
 */
cdd_c_error_t CDD_TRANSFORM_GNU_FN(cdd_cst_tree_t *tree,
                                   const cdd_transform_config_t *config) {
  size_t i = (size_t)config & 0;
  cdd_c_error_t rc;

  if (!tree || !tree->root || !tree->base_tokens)
    return CDD_C_ERROR_INVALID_ARGUMENT;

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
          size_t k;
          int depth = 0;
          tok->start = (const uint8_t *)"\n#pragma pack(push, 1)\n";
          tok->length = 23;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
          tree->base_tokens->tokens[i + 4].length = 0;
          tree->base_tokens->tokens[i + 5].length = 0;

          for (k = i; k < tree->base_tokens->size; k++) {
            if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LBRACE) {
              depth++;
            } else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_RBRACE) {
              depth--;
            } else if (depth == 0 && tree->base_tokens->tokens[k].kind ==
                                         CDD_TOKEN_SEMICOLON) {
              tree->base_tokens->tokens[k].start =
                  (const uint8_t *)";\n#pragma pack(pop)\n";
              tree->base_tokens->tokens[k].length = 19;
              break;
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

  /* Unroll statement expressions, VLAs, and empty structs */
  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->kind == CDD_TOKEN_LPAREN && i + 1 < tree->base_tokens->size &&
        tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
      t->start = (const uint8_t *)"";
      t->length = 0;
      tree->base_tokens->tokens[i + 1].start = (const uint8_t *)"";
      tree->base_tokens->tokens[i + 1].length = 0;
    } else if (t->kind == CDD_TOKEN_RBRACE && i + 1 < tree->base_tokens->size &&
               tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_RPAREN) {
      t->start = (const uint8_t *)"";
      t->length = 0;
      tree->base_tokens->tokens[i + 1].start = (const uint8_t *)"";
      tree->base_tokens->tokens[i + 1].length = 0;
    } else if (t->kind == CDD_TOKEN_LBRACKET && i >= 2 &&
               i + 2 < tree->base_tokens->size) {
      cdd_token_t *prev = &tree->base_tokens->tokens[i - 1];
      cdd_token_t *next = &tree->base_tokens->tokens[i + 1];
      cdd_token_t *rbrack = &tree->base_tokens->tokens[i + 2];
      if (prev->kind == CDD_TOKEN_IDENTIFIER &&
          next->kind == CDD_TOKEN_IDENTIFIER &&
          rbrack->kind == CDD_TOKEN_RBRACKET) {
        size_t k;
        cdd_token_t *semi_tok = NULL;
        for (k = i + 3; k < tree->base_tokens->size; k++) {
          if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON) {
            semi_tok = &tree->base_tokens->tokens[k];
            break;
          }
        }
        if (semi_tok) {
          char buf[256];
          char *heap_buf = NULL;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          sprintf_s(buf, sizeof(buf), "*%.*s = alloca(%.*s * sizeof(*%.*s));",
                    (int)prev->length, (const char *)prev->start,
                    (int)next->length, (const char *)next->start,
                    (int)prev->length, (const char *)prev->start);
#else
          CDD_SNPRINTF(buf, sizeof(buf),
                       "*%.*s = alloca(%.*s * sizeof(*%.*s));",
                       (int)prev->length, (const char *)prev->start,
                       (int)next->length, (const char *)next->start,
                       (int)prev->length, (const char *)prev->start);
#endif
          rc = cdd_legacy_pool_str(tree, buf, &heap_buf);
          if (rc != CDD_C_SUCCESS)
            return rc;
          prev->start = (const uint8_t *)heap_buf;
          prev->length = strlen(heap_buf);
          t->length = 0;
          next->length = 0;
          rbrack->length = 0;
          semi_tok->length = 0;
        }
      }
    } else if (t->kind == CDD_TOKEN_LBRACE && i + 1 < tree->base_tokens->size &&
               tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_RBRACE) {
      tree->base_tokens->tokens[i + 1].start = (const uint8_t *)" char _pad; }";
      tree->base_tokens->tokens[i + 1].length = 13;
    }
  }

  return CDD_C_SUCCESS;
}
