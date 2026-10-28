/**
 * @file cli.c
 * @brief C to OpenAPI CLI routes parser and generator main entry points.
 *
 * Provides command line interfaces and helper functions for:
 * - Parsing C source/header files into OpenAPI 3.x specifications
 * (`c2openapi`).
 * - Emitting documentation JSON structures (`to_docs_json`).
 * - Generating multi-language bindings (`cdd-c bind`).
 * - Converting between C code signatures, docstrings, and OpenAPI models.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/memory.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../win_compat_sym.h"

#include "../../cdd_api.h"
#include "classes/emit/schema.h"
#include "classes/parse/code2schema.h"
#include "classes/parse/inspector.h"
#include "docstrings/parse/doc.h"
#include "functions/parse/cst.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
#include "openapi/emit/openapi.h"
#include "openapi/parse/openapi.h"
#include "openapi/parse/openapi_internal.h"
#include "routes/emit/aggregator.h"
#include "routes/emit/operation.h"
#include "routes/parse/cli_internal.h"
#include "routes/parse/cli.h"
/* clang-format on */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

/**
 * @brief Simple signature parser to split "int foo(int x, char
 * *y)" Populates `out`. Caller must free internals.
 *
 * @param[in] sig_str C function signature string.
 * @param[out] out Output parsed signature structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_c_signature_string(const char *sig_str,
                                       struct C2OpenAPI_ParsedSig *out) {
  struct TokenList *tl = NULL;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  size_t lp = 0, rp = 0;

  if (!sig_str || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(out, 0, sizeof(*out));

  if (tokenize(az_span_create_from_str(sig_str), &tl) != 0) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  /* Naive extraction: Name is identifier before LPAREN */
  for (i = 0; i < tl->size; ++i) {
    if (tl->tokens[i].kind == TOKEN_LPAREN) {
      lp = i;
      break;
    }
  }

  /* Need at least name and parens */
  if (lp < 1) {
    free_token_list(tl);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  /* Extract Name (token before lparen, ignoring WS) */
  {
    size_t k = lp - 1;
    while (k > 0 && tl->tokens[k].kind == TOKEN_WHITESPACE)
      k--;
    if (tl->tokens[k].kind == TOKEN_IDENTIFIER) {
      size_t len = tl->tokens[k].length;
      char *n = C_CDD_MALLOC(len + 1);
      if (!n) {
        rc = CDD_C_ERROR_MEMORY;
        goto cleanup;
      }
      memcpy(n, tl->tokens[k].start, len);
      n[len] = '\0';
      out->name = n;
    }
  }

  if (!out->name) {
    rc = CDD_C_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  /* Extract Args between ( and ) */
  /* Split by COMMA. For each segment, last ID is name, rest is
   * type. */
  token_find_next(tl, lp, tl->size, TOKEN_RPAREN, &rp);
  if (rp >= tl->size) {
    rc = CDD_C_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  if (rp > lp + 1) {
    /* Non-empty args */
    size_t start = lp + 1;
    size_t end = start;

    while (end < rp) {
      /* Find comma or RP */
      size_t seg_end = end;
      while (seg_end < rp && tl->tokens[seg_end].kind != TOKEN_COMMA)
        seg_end++;

      /* Process segment [start, seg_end) */
      {
        /* Find name: last identifier in segment */
        size_t k = seg_end;
        size_t name_idx = 0;
        int found_name = 0;

        while (k > start) {
          k--;
          if (tl->tokens[k].kind == TOKEN_IDENTIFIER) {
            name_idx = k;
            found_name = 1;
            break;
          }
        }

        if (found_name) {
          /* Type is start..name_idx (exclusive) + modifiers
           * after? */
          /* Simple approach: Type is [start, name_idx), Name is
             name_idx. Postfix arrays `[]` might be after name.
           */
          /* Let's grab name string */
          const struct Token *nt = &tl->tokens[name_idx];
          size_t t_end = name_idx;

          /* Check if type is pointer/const/struct before name
           */
          /* Construct type string */
          size_t t_len = 0;
          char *t_str;
          size_t m;
          char *n_str = C_CDD_MALLOC(nt->length + 1);
          if (!n_str) {
            rc = CDD_C_ERROR_MEMORY;
            goto cleanup;
          }
          memcpy(n_str, nt->start, nt->length);
          n_str[nt->length] = '\0';

          /* Calc type len */
          for (m = start; m < t_end; m++)
            t_len += tl->tokens[m].length;
          /* Add postfix */
          for (m = name_idx + 1; m < seg_end; m++)
            t_len += tl->tokens[m].length;

          t_str = C_CDD_MALLOC(t_len + 1);
          if (!t_str) {
            C_CDD_FREE(n_str);
            rc = CDD_C_ERROR_MEMORY;
            goto cleanup;
          }
          {
            char *p = t_str;
            for (m = start; m < t_end; m++) {
              memcpy(p, tl->tokens[m].start, tl->tokens[m].length);
              p += tl->tokens[m].length;
            }
            for (m = name_idx + 1; m < seg_end; m++) {
              memcpy(p, tl->tokens[m].start, tl->tokens[m].length);
              p += tl->tokens[m].length;
            }
            *p = '\0';
          }
          c_cdd_str_trim_trailing_whitespace(t_str);
          {
            char *tp = t_str;
            while (*tp == ' ' || *tp == '\t')
              tp++;
            if (tp > t_str)
              memmove(t_str, tp, strlen(tp) + 1);
          }

          /* Add to list */
          {
            struct C2OpenAPI_ParsedArg *new_arr = C_CDD_REALLOC(
                out->args,
                (out->n_args + 1) * sizeof(struct C2OpenAPI_ParsedArg));
            if (!new_arr) {
              C_CDD_FREE(n_str);
              C_CDD_FREE(t_str);
              rc = CDD_C_ERROR_MEMORY;
              goto cleanup;
            }
            out->args = new_arr;
            out->args[out->n_args].name = n_str;
            out->args[out->n_args].type = t_str;
            out->n_args++;
          }

        } else {
          /* Void arg or unnamed? ignore */
        }
      }

      start = seg_end + 1; /* Skip comma */
      end = start;
    }
  }

cleanup:
  free_token_list(tl);
  if (rc != CDD_C_SUCCESS) {
    free_parsed_sig(out);
  }
  return rc;
}

/**
 * @brief Frees the memory associated with parsed sig.
 *
 * @param[in,out] sig Parsed signature structure to clean.
 */
void free_parsed_sig(struct C2OpenAPI_ParsedSig *sig) {
  size_t i;
  if (!sig)
    return;
  if (sig->name)
    C_CDD_FREE(sig->name);
  if (sig->return_type)
    C_CDD_FREE(sig->return_type);
  if (sig->args) {
    for (i = 0; i < sig->n_args; ++i) {
      C_CDD_FREE(sig->args[i].name);
      C_CDD_FREE(sig->args[i].type);
    }
    C_CDD_FREE(sig->args);
  }
  memset(sig, 0, sizeof(*sig));
}

/**
 * @brief Infers an HTTP verb and route path from a C client function CST node.
 *
 * @param[in] func_node CST function node to inspect.
 * @param[in] tokens Token list containing the function tokens.
 * @param[out] out_route Output allocated route string (e.g. "GET /pets"), or
 * NULL.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
c2openapi_infer_client_route(const struct CstNode *func_node,
                             const struct TokenList *tokens, char **out_route) {
  size_t k;
  const char *verb = NULL;
  char *path_str = NULL;

  if (!func_node || !tokens || !out_route) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  *out_route = NULL;

  for (k = func_node->start_token;
       (k < func_node->end_token) & (k < tokens->size); ++k) {
    const struct Token *tok = &tokens->tokens[k];
    if (tok->kind == TOKEN_IDENTIFIER) {
      int m = 0;
      if (!verb) {
        token_matches_string(tok, "HTTP_GET", &m);
        if (m) {
          verb = "GET";
        } else {
          token_matches_string(tok, "HTTP_POST", &m);
          if (m) {
            verb = "POST";
          } else {
            token_matches_string(tok, "HTTP_PUT", &m);
            if (m) {
              verb = "PUT";
            } else {
              token_matches_string(tok, "HTTP_DELETE", &m);
              if (m) {
                verb = "DELETE";
              } else {
                token_matches_string(tok, "HTTP_PATCH", &m);
                if (m) {
                  verb = "PATCH";
                }
              }
            }
          }
        }
      }
    } else if (tok->kind == TOKEN_STRING_LITERAL) {
      if (!path_str && tok->length >= 3 && tok->start[1] == '/') {
        size_t len = tok->length - 2;
        path_str = (char *)C_CDD_MALLOC(len + 1);
        if (!path_str) {
          return CDD_C_ERROR_MEMORY;
        }
        memcpy(path_str, tok->start + 1, len);
        path_str[len] = '\0';
      }
    }
  }

  if (verb && path_str) {
    size_t r_len;
    char *route;
    r_len = strlen(verb) + 1 + strlen(path_str) + 1;
    route = (char *)C_CDD_MALLOC(r_len);
    if (!route) {
      C_CDD_FREE(path_str);
      return CDD_C_ERROR_MEMORY;
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(route, r_len, "%s %s", verb, path_str);
#else
    CDD_SNPRINTF(route, r_len, "%s %s", verb, path_str);
#endif
    C_CDD_FREE(path_str);
    *out_route = route;
    return CDD_C_SUCCESS;
  }
  if (path_str) {
    C_CDD_FREE(path_str);
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Scans a token list for `c_rest_router_add` calls and registers server
 * routes.
 *
 * @param[in] tokens Token list to scan.
 * @param[in,out] spec Target OpenAPI specification.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
c2openapi_scan_server_routes(const struct TokenList *tokens,
                             struct OpenAPI_Spec *spec) {
  size_t i;
  cdd_c_error_t rc;

  if (!tokens || !spec) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
      int matches = 0;
      token_matches_string(&tokens->tokens[i], "c_rest_router_add", &matches);
      if (matches) {
        size_t j = i + 1;
        char verb_str[16];
        char path_str[256];
        char handler_str[128];
        char route_str[300];
        int arg_idx = 0;

        verb_str[0] = '\0';
        path_str[0] = '\0';
        handler_str[0] = '\0';

        while (j < tokens->size && tokens->tokens[j].kind != TOKEN_LPAREN) {
          j++;
        }
        if (j < tokens->size) {
          j++;
          while (j < tokens->size &&
                 !((tokens->tokens[j].kind == TOKEN_RPAREN) |
                   (tokens->tokens[j].kind == TOKEN_SEMICOLON))) {
            if (tokens->tokens[j].kind == TOKEN_COMMA) {
              arg_idx++;
            } else if (arg_idx == 1 &&
                       tokens->tokens[j].kind == TOKEN_STRING_LITERAL) {
              size_t len = tokens->tokens[j].length - 2;
              if (len > 0 && len < sizeof(verb_str)) {
                memcpy(verb_str, tokens->tokens[j].start + 1, len);
                verb_str[len] = '\0';
              }
            } else if (arg_idx == 2 &&
                       tokens->tokens[j].kind == TOKEN_STRING_LITERAL) {
              size_t len = tokens->tokens[j].length - 2;
              if ((len > 0) & (len < sizeof(path_str))) {
                memcpy(path_str, tokens->tokens[j].start + 1, len);
                path_str[len] = '\0';
              }
            } else if (arg_idx == 3 &&
                       tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
              size_t len = tokens->tokens[j].length;
              if (len < sizeof(handler_str)) {
                memcpy(handler_str, tokens->tokens[j].start, len);
                handler_str[len] = '\0';
              }
            }
            j++;
          }
        }

        if (verb_str[0] && path_str[0]) {
          struct OpenAPI_Operation op;
          const char *op_id = handler_str;
          memset(&op, 0, sizeof(op));
          if (strncmp(op_id, "handle_", 7) == 0) {
            op_id += 7;
          }
          if (op_id[0]) {
            rc = c_cdd_strdup(op_id, &op.operation_id);
            if (rc != CDD_C_SUCCESS) {
              return rc;
            }
          }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          sprintf_s(route_str, sizeof(route_str), "%s %s", verb_str, path_str);
#else
          CDD_SNPRINTF(route_str, sizeof(route_str), "%s %s", verb_str,
                       path_str);
#endif
          rc = openapi_aggregator_add_operation(spec, route_str, &op);
          if (op.operation_id) {
            C_CDD_FREE(op.operation_id);
          }
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
        }
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Scans a token list for `render_<op>_view` identifiers and registers
 * GUI routes.
 *
 * @param[in] tokens Token list to scan.
 * @param[in,out] spec Target OpenAPI specification.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t c2openapi_scan_gui_views(const struct TokenList *tokens,
                                              struct OpenAPI_Spec *spec) {
  size_t i;
  cdd_c_error_t rc;

  if (!tokens || !spec) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
      const char *ident = (const char *)tokens->tokens[i].start;
      size_t len = tokens->tokens[i].length;
      if (len > 12 && strncmp(ident, "render_", 7) == 0 &&
          strncmp(ident + len - 5, "_view", 5) == 0) {
        size_t op_len = len - 12;
        char op_id[128];
        char route_str[256];
        struct OpenAPI_Operation op;

        if (op_len < sizeof(op_id)) {
          memcpy(op_id, ident + 7, op_len);
          op_id[op_len] = '\0';

          memset(&op, 0, sizeof(op));
          rc = c_cdd_strdup(op_id, &op.operation_id);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          sprintf_s(route_str, sizeof(route_str), "GET /gui/%s", op_id);
#else
          CDD_SNPRINTF(route_str, sizeof(route_str), "GET /gui/%s", op_id);
#endif
          rc = openapi_aggregator_add_operation(spec, route_str, &op);
          C_CDD_FREE(op.operation_id);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
        }
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Scans a token list for MCP `tool_obj` registrations and adds them to
 * the OpenAPI specification.
 *
 * @param[in] tokens Token list to scan.
 * @param[in,out] spec Target OpenAPI specification.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t c2openapi_scan_mcp_tools(const struct TokenList *tokens,
                                              struct OpenAPI_Spec *spec) {
  size_t i;
  cdd_c_error_t rc;

  if (!tokens || !spec) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
      int is_tool_obj = 0;
      token_matches_string(&tokens->tokens[i], "tool_obj", &is_tool_obj);
      if (is_tool_obj && i + 4 < tokens->size &&
          tokens->tokens[i + 1].kind == TOKEN_COMMA &&
          tokens->tokens[i + 2].kind == TOKEN_STRING_LITERAL &&
          tokens->tokens[i + 3].kind == TOKEN_COMMA &&
          tokens->tokens[i + 4].kind == TOKEN_STRING_LITERAL) {
        const struct Token *key_tok = &tokens->tokens[i + 2];
        const struct Token *val_tok = &tokens->tokens[i + 4];
        if (key_tok->length == 6 &&
            strncmp((const char *)key_tok->start, "\"name\"", 6) == 0 &&
            val_tok->length > 2 && val_tok->length - 2 < 128) {
          char op_id[128];
          char route_str[160];
          struct OpenAPI_Operation op;
          size_t name_len = val_tok->length - 2;
          memcpy(op_id, val_tok->start + 1, name_len);
          op_id[name_len] = '\0';
          memset(&op, 0, sizeof(op));
          rc = c_cdd_strdup(op_id, &op.operation_id);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          sprintf_s(route_str, sizeof(route_str), "POST /mcp/tools/%s", op_id);
#else
          CDD_SNPRINTF(route_str, sizeof(route_str), "POST /mcp/tools/%s",
                       op_id);
#endif
          rc = openapi_aggregator_add_operation(spec, route_str, &op);
          C_CDD_FREE(op.operation_id);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
        }
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a single C source/header file and extracts schemas and route
 * operations.
 *
 * @param[in] path Path to the C source or header file.
 * @param[in,out] spec Target OpenAPI specification to populate.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t process_file(const char *path, struct OpenAPI_Spec *spec) {
  char *content = NULL;
  size_t sz = 0;
  struct TokenList *tokens = NULL;
  struct CstNodeList cst = {0};
  int *comment_used = NULL;
  cdd_c_error_t rc;
  size_t i;

  if (!path || !spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* 1. Register Types (Structs/Enums) */
  {
    struct TypeDefList types;
    type_def_list_init(&types);
    if (c_inspector_scan_file_types(path, &types) == 0) {
      c2openapi_register_types(spec, &types);
    }
    type_def_list_free(&types);
  }

  /* 2. Parse Code for Functions & Docs */
  rc = read_to_file(path, "r", &content, &sz);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (tokenize(az_span_create_from_str(content), &tokens) != 0) {
    C_CDD_FREE(content);
    return CDD_C_ERROR_IO;
  }
  parse_tokens(tokens, &cst); /* Best effort */

  if (cst.size > 0) {
    comment_used = (int *)C_CDD_CALLOC(cst.size, sizeof(int));
    if (!comment_used) {
      free_cst_node_list(&cst);
      free_token_list(tokens);
      C_CDD_FREE(content);
      return CDD_C_ERROR_MEMORY;
    }
  }

  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_FUNCTION) {
      struct CstNode *func_node = &cst.nodes[i];
      struct CstNode *doc_node = NULL;
      size_t doc_index = (size_t)-1;

      /* Look backwards for doc comment */
      if (i > 0 && cst.nodes[i - 1].kind == CST_NODE_COMMENT) {
        doc_node = &cst.nodes[i - 1];
        doc_index = i - 1;
      }

      if (doc_node) {
        /* Extract comment text */
        char *doc_text = C_CDD_MALLOC(doc_node->length + 1);
        if (doc_text) {
          struct DocMetadata meta;
          memcpy(doc_text, doc_node->start, doc_node->length);
          doc_text[doc_node->length] = '\0';

          doc_metadata_init(&meta);
          if (doc_parse_block(doc_text, &meta) == 0) {
            cdd_c_error_t rc_meta = apply_all_doc_meta(spec, &meta);
            if (rc_meta != CDD_C_SUCCESS) {
              doc_metadata_free(&meta);
              C_CDD_FREE(doc_text);
              free_cst_node_list(&cst);
              free_token_list(tokens);
              C_CDD_FREE(content);
              C_CDD_FREE(comment_used);
              return rc_meta;
            }
            comment_used[doc_index] = 1;
          }
          if (!meta.route) {
            c2openapi_infer_client_route(func_node, tokens, &meta.route);
          }
          if (meta.route) {
            /* Found Valid Documented Route! */

            /* Extract Signature Text */
            size_t sig_len = func_node->length; /* Approximation, includes
                                                   body? */
            /* We need signature string up to brace. CST Node
             * includes body. */
            char *sig_raw = (char *)C_CDD_MALLOC(sig_len + 1);
            if (sig_raw) {
              struct C2OpenAPI_ParsedSig psig;
              const uint8_t *brace = memchr(func_node->start, '{', sig_len);
              size_t effective_len = (size_t)(brace - func_node->start);

              memcpy(sig_raw, func_node->start, effective_len);
              sig_raw[effective_len] = '\0';

              if (parse_c_signature_string(sig_raw, &psig) == CDD_C_SUCCESS) {
                struct OpenAPI_Operation op = {0};
                struct OpBuilderContext ctx;

                ctx.sig = &psig;
                ctx.doc = &meta;
                ctx.func_name = psig.name;

                if (c2openapi_build_operation(&ctx, &op) == CDD_C_SUCCESS) {
                  cdd_c_error_t rc_add;
                  if (meta.is_webhook) {
                    rc_add = openapi_aggregator_add_webhook_operation(
                        spec, meta.route, &op);
                  } else {
                    rc_add =
                        openapi_aggregator_add_operation(spec, meta.route, &op);
                  }
                  if (rc_add != CDD_C_SUCCESS) {
                    free_operation(&op);
                  }
                } else {
                  free_operation(&op);
                }
                free_parsed_sig(&psig);
              }
              C_CDD_FREE(sig_raw);
            }
          }
          doc_metadata_free(&meta);
          C_CDD_FREE(doc_text);
        }
      } else {
        char *inferred_route = NULL;
        c2openapi_infer_client_route(func_node, tokens, &inferred_route);
        if (inferred_route) {
          struct DocMetadata meta;
          size_t sig_len = func_node->length;
          char *sig_raw = (char *)C_CDD_MALLOC(sig_len + 1);
          if (sig_raw) {
            struct C2OpenAPI_ParsedSig psig;
            const uint8_t *brace =
                (const uint8_t *)memchr(func_node->start, '{', sig_len);
            size_t effective_len = (size_t)(brace - func_node->start);

            memcpy(sig_raw, func_node->start, effective_len);
            sig_raw[effective_len] = '\0';

            doc_metadata_init(&meta);
            meta.route = inferred_route;

            if (parse_c_signature_string(sig_raw, &psig) == CDD_C_SUCCESS) {
              struct OpenAPI_Operation op = {0};
              struct OpBuilderContext ctx;

              ctx.sig = &psig;
              ctx.doc = &meta;
              ctx.func_name = psig.name;

              if (c2openapi_build_operation(&ctx, &op) == CDD_C_SUCCESS) {
                if (openapi_aggregator_add_operation(spec, meta.route, &op) !=
                    CDD_C_SUCCESS) {
                  free_operation(&op);
                }
              } else {
                free_operation(&op);
              }
              free_parsed_sig(&psig);
            }
            doc_metadata_free(&meta);
            C_CDD_FREE(sig_raw);
          } else {
            C_CDD_FREE(inferred_route);
          }
        }
      }
    }
  }

  /* Parse standalone comment blocks for global metadata. */
  if (comment_used) {
    for (i = 0; i < cst.size; ++i) {
      if (cst.nodes[i].kind == CST_NODE_COMMENT && !comment_used[i]) {
        struct DocMetadata meta;
        char *doc_text = C_CDD_MALLOC(cst.nodes[i].length + 1);
        if (!doc_text)
          continue;
        memcpy(doc_text, cst.nodes[i].start, cst.nodes[i].length);
        doc_text[cst.nodes[i].length] = '\0';

        doc_metadata_init(&meta);
        if (doc_parse_block(doc_text, &meta) == 0) {
          cdd_c_error_t rc_meta = apply_all_doc_meta(spec, &meta);
          if (rc_meta != CDD_C_SUCCESS) {
            doc_metadata_free(&meta);
            C_CDD_FREE(doc_text);
            free_cst_node_list(&cst);
            free_token_list(tokens);
            C_CDD_FREE(content);
            C_CDD_FREE(comment_used);
            return rc_meta;
          }
        }
        doc_metadata_free(&meta);
        C_CDD_FREE(doc_text);
      }
    }
  }

  /* Scan for server router registrations */
  rc = c2openapi_scan_server_routes(tokens, spec);
  if (rc != CDD_C_SUCCESS) {
    free_cst_node_list(&cst);
    free_token_list(tokens);
    C_CDD_FREE(content);
    C_CDD_FREE(comment_used);
    return rc;
  }

  /* Scan for client GUI views */
  rc = c2openapi_scan_gui_views(tokens, spec);
  if (rc != CDD_C_SUCCESS) {
    free_cst_node_list(&cst);
    free_token_list(tokens);
    C_CDD_FREE(content);
    C_CDD_FREE(comment_used);
    return rc;
  }

  /* Scan for MCP tool definitions if no routes were registered yet */
  if (spec->n_paths == 0) {
    rc = c2openapi_scan_mcp_tools(tokens, spec);
    if (rc != CDD_C_SUCCESS) {
      free_cst_node_list(&cst);
      free_token_list(tokens);
      C_CDD_FREE(content);
      C_CDD_FREE(comment_used);
      return rc;
    }
  }

  free_cst_node_list(&cst);
  free_token_list(tokens);
  C_CDD_FREE(content);
  C_CDD_FREE(comment_used);

  return CDD_C_SUCCESS;
}

/**
 * @brief Directory traversal callback that processes C source and header files
 * for c2openapi.
 *
 * @param[in] path File path encountered during traversal.
 * @param[in,out] user_data Pointer to target `OpenAPI_Spec` structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t walker_cb(const char *path, void *user_data) {
  struct OpenAPI_Spec *spec = (struct OpenAPI_Spec *)user_data;
  if (!path || !user_data)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  {
    int is_src = 0;
    is_source_file(path, &is_src);
    if (!is_src)
      return CDD_C_SUCCESS;
  }
  printf("Scanning: %s\n", path);
  {

    cdd_c_error_t rc = process_file(path, spec);

    if (rc == CDD_C_ERROR_MEMORY)
      return rc;
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Warning: Failed to process %s (error %d), skipping.\n",
              path, rc);
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Loads an existing base OpenAPI JSON file to merge extracted C routes
 * into.
 *
 * @param[in] path Path to the base OpenAPI JSON file.
 * @param[out] spec Target OpenAPI specification to populate.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t load_base_spec(const char *path, struct OpenAPI_Spec *spec) {
  JSON_Value *root = NULL;
  cdd_c_error_t rc;

  if (!path || !spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  root = json_parse_file(path);
  if (!root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = openapi_load_from_json(root, spec);
  json_value_free(root);
  return rc;
}

/**
 * @brief Generates an OpenAPI specification from C source code using a typed
 * configuration struct.
 *
 * @param[in] config Configuration struct.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t
c2openapi_generate(const cdd_to_openapi_config_t *config) {
  struct OpenAPI_Spec spec;
  const char *src_dir;
  const char *out_file;
  const char *base_file;
  const char *self_uri;
  const char *dialect_uri;
  char *json = NULL;
  cdd_c_error_t rc;

  if (!config || !config->input || !config->output) {
    fprintf(stderr, "Usage: c2openapi [--base <openapi.json>] "
                    "[--self <uri>] "
                    "[--dialect <uri>] "
                    "<src_dir> <out.json>\n");
    return CDD_C_ERROR_UNKNOWN;
  }

  src_dir = config->input;
  out_file = config->output;
  base_file = config->base_file;
  self_uri = config->self_uri;
  dialect_uri = config->dialect_uri;

  openapi_spec_init(&spec);

  if (base_file) {
    rc = load_base_spec(base_file, &spec);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Failed to load base OpenAPI spec %s: %d\n", base_file,
              rc);
      openapi_spec_free(&spec);
      return CDD_C_ERROR_UNKNOWN;
    }
  }

  if (self_uri && *self_uri) {
    C_CDD_FREE(spec.self_uri);
    spec.self_uri = NULL;
    rc = c_cdd_strdup(self_uri, &spec.self_uri);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Failed to set $self URI\n");
      openapi_spec_free(&spec);
      return CDD_C_ERROR_UNKNOWN;
    }
  }
  if (dialect_uri && *dialect_uri) {
    C_CDD_FREE(spec.json_schema_dialect);
    spec.json_schema_dialect = NULL;
    rc = c_cdd_strdup(dialect_uri, &spec.json_schema_dialect);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Failed to set jsonSchemaDialect\n");
      openapi_spec_free(&spec);
      return CDD_C_ERROR_UNKNOWN;
    }
  }

  /* 1. Walk & Process */
  rc = walk_directory(src_dir, walker_cb, &spec);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Error walking directory %s: %d\n", src_dir, rc);
    openapi_spec_free(&spec);
    return CDD_C_ERROR_UNKNOWN;
  }

  /* Derive top-level tags from operation tags */
  rc = collect_spec_tags(&spec);
  if (rc != CDD_C_SUCCESS) {
    openapi_spec_free(&spec);
    return rc;
  }

  /* 2. Write */
  rc = openapi_write_spec_to_json(&spec, &json);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Error serializing spec: %d\n", rc);
    openapi_spec_free(&spec);
    return CDD_C_ERROR_UNKNOWN;
  }

  rc = fs_write_to_file(out_file, json);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Failed to write %s\n", out_file);
    rc = CDD_C_ERROR_UNKNOWN;
  } else {
    printf("Written %s\n", out_file);
    rc = CDD_C_SUCCESS;
  }

  C_CDD_FREE(json);
  openapi_spec_free(&spec);

  return rc;
}

/**
 * @brief Main CLI entry point for c2openapi command.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t c2openapi_cli_main(int argc, char **argv) {
  cdd_to_openapi_config_t cfg;
  int argi = 1;

  memset(&cfg, 0, sizeof(cfg));

  while (argi < argc && argv[argi][0] == '-') {
    if (strcmp(argv[argi], "--base") == 0 || strcmp(argv[argi], "-b") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr, "Usage: c2openapi [--base "
                        "<openapi.json>] [--self <uri>] "
                        "[--dialect <uri>] "
                        "<src_dir> <out.json>\n");
        return CDD_C_ERROR_UNKNOWN;
      }
      cfg.base_file = argv[argi + 1];
      argi += 2;
      continue;
    }
    if (strcmp(argv[argi], "--self") == 0 || strcmp(argv[argi], "-s") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr, "Usage: c2openapi [--base "
                        "<openapi.json>] [--self <uri>] "
                        "[--dialect <uri>] "
                        "<src_dir> <out.json>\n");
        return CDD_C_ERROR_UNKNOWN;
      }
      cfg.self_uri = argv[argi + 1];
      argi += 2;
      continue;
    }
    if (strcmp(argv[argi], "--dialect") == 0 ||
        strcmp(argv[argi], "--jsonSchemaDialect") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr, "Usage: c2openapi [--base "
                        "<openapi.json>] [--self <uri>] "
                        "[--dialect <uri>] "
                        "<src_dir> <out.json>\n");
        return CDD_C_ERROR_UNKNOWN;
      }
      cfg.dialect_uri = argv[argi + 1];
      argi += 2;
      continue;
    }
    break;
  }

  if (argc - argi != 2) {
    fprintf(stderr, "Usage: c2openapi [--base <openapi.json>] "
                    "[--self <uri>] "
                    "[--dialect <uri>] "
                    "<src_dir> <out.json>\n");
    return CDD_C_ERROR_UNKNOWN;
  }

  cfg.input = argv[argi];
  cfg.output = argv[argi + 1];
  return c2openapi_generate(&cfg);
}

/**
 * @brief Main CLI entry point for to_docs_json command.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t to_docs_json_cli_main(int argc, char **argv) {
  cdd_docs_json_config_t cfg;
  int i;

  memset(&cfg, 0, sizeof(cfg));
  cfg.input = getenv("CDD_INPUT") ? getenv("CDD_INPUT") : getenv("INPUT_FILE");
  cfg.output = getenv("CDD_OUTPUT") ? getenv("CDD_OUTPUT") : getenv("OUT_FILE");
  cfg.no_imports = getenv("CDD_NO_IMPORTS") ? 1 : 0;
  cfg.no_wrapping = getenv("CDD_NO_WRAPPING") ? 1 : 0;

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      puts("Usage: cdd-c to_docs_json [args]");
      puts("");
      puts("Options:");
      puts("  -i, --input <spec.json>   Input OpenAPI spec file");
      puts("  -o, --output <docs.json>  Output docs JSON file (default: "
           "stdout)");
      puts("  --no-imports              Omit #include statements from "
           "generated snippets");
      puts("  --no-wrapping             Omit main() wrapper from generated "
           "snippets");
      return CDD_C_SUCCESS;
    } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
      if (i + 1 < argc)
        cfg.input = argv[++i];
    } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
      if (i + 1 < argc)
        cfg.output = argv[++i];
    } else if (strcmp(argv[i], "--no-imports") == 0) {
      cfg.no_imports = CDD_TRUE;
    } else if (strcmp(argv[i], "--no-wrapping") == 0) {
      cfg.no_wrapping = CDD_TRUE;
    }
  }

  return cdd_generate_docs_json(&cfg);
}

/**
 * @brief Main CLI entry point for generate_bindings command.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t generate_bindings_cli_main(int argc, char **argv) {
  cdd_generate_bindings_config_t config = {0};
  int i;
  cdd_c_error_t rc;

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      puts("Usage: cdd-c bind [OPTIONS]\n\n"
           "Options:\n"
           "  -i, --input <file|dir>    Input C header/source "
           "or directory\n"
           "  -o, --output-dir <dir>    Output directory for "
           "bindings\n"
           "  -l, --lang <langs>        Comma-separated list "
           "of languages or "
           "'*' "
           "(e.g., python,rust)");
      puts("  -n, --lib-name <name>     Name of the shared "
           "library (e.g., "
           "sqlite3)\n"
           "  -m, --module-name <name>  Name of the generated "
           "namespace/module\n"
           "  --skip-static             Skip static inline "
           "functions\n"
           "  --opaque-pointers         Treat unknown structs "
           "as void* "
           "(opaque)\n"
           "  --generate-tests          Generate basic "
           "sanity-check tests\n"
           "  -h, --help                Show this help "
           "message");
      return CDD_C_SUCCESS;
    } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
      if (i + 1 < argc)
        config.input = argv[++i];
    } else if (strcmp(argv[i], "-o") == 0 ||
               strcmp(argv[i], "--output-dir") == 0) {
      if (i + 1 < argc)
        config.output_dir = argv[++i];
    } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--lang") == 0) {
      if (i + 1 < argc)
        config.target_langs = argv[++i];
    } else if (strcmp(argv[i], "-n") == 0 ||
               strcmp(argv[i], "--lib-name") == 0) {
      if (i + 1 < argc)
        config.library_name = argv[++i];
    } else if (strcmp(argv[i], "-m") == 0 ||
               strcmp(argv[i], "--module-name") == 0) {
      if (i + 1 < argc)
        config.module_name = argv[++i];
    } else if (strcmp(argv[i], "--skip-static") == 0) {
      config.skip_static = CDD_TRUE;
    } else if (strcmp(argv[i], "--opaque-pointers") == 0) {
      config.opaque_pointers = CDD_TRUE;
    } else if (strcmp(argv[i], "--generate-tests") == 0) {
      config.generate_tests = CDD_TRUE;
    }
  }

  if (!config.input || !config.output_dir || !config.target_langs) {
    fprintf(stderr, "Error: --input, --output-dir, and --lang "
                    "are required.\n");
    return CDD_C_ERROR_UNKNOWN;
  }

  rc = cdd_generate_bindings(&config);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Error: Binding generation failed with code %d\n", rc);
    return CDD_C_ERROR_UNKNOWN;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Registers parsed C types into an OpenAPI specification schemas list.
 *
 * @param[in,out] spec OpenAPI specification to populate.
 * @param[in] types List of parsed C type definitions.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t c2openapi_register_types(
    struct OpenAPI_Spec *spec, const struct TypeDefList *types) {
  size_t i, j;
  if (!spec || !types)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < types->size; i++) {
    const struct TypeDefinition *def = &types->items[i];

    /* Skip duplicates */
    int is_duplicate = 0;
    for (j = 0; j < spec->n_defined_schemas; j++) {
      if (strcmp(spec->defined_schema_names[j], def->name) == 0) {
        is_duplicate = 1;
        break;
      }
    }
    if (is_duplicate)
      continue;

    if (def->kind == KIND_STRUCT && def->name && def->details.struct_fields) {
      size_t new_idx = spec->n_defined_schemas;
      spec->n_defined_schemas++;
      spec->defined_schema_names = C_CDD_REALLOC(
          spec->defined_schema_names, spec->n_defined_schemas * sizeof(char *));
      spec->defined_schemas =
          C_CDD_REALLOC(spec->defined_schemas,
                        spec->n_defined_schemas * sizeof(struct StructFields));
      if (spec->defined_schema_ids) {
        spec->defined_schema_ids = C_CDD_REALLOC(
            spec->defined_schema_ids, spec->n_defined_schemas * sizeof(char *));
        spec->defined_schema_ids[new_idx] = NULL;
      }
      if (spec->defined_schema_anchors) {
        spec->defined_schema_anchors =
            C_CDD_REALLOC(spec->defined_schema_anchors,
                          spec->n_defined_schemas * sizeof(char *));
        spec->defined_schema_anchors[new_idx] = NULL;
      }
      if (spec->defined_schema_dynamic_anchors) {
        spec->defined_schema_dynamic_anchors =
            C_CDD_REALLOC(spec->defined_schema_dynamic_anchors,
                          spec->n_defined_schemas * sizeof(char *));
        spec->defined_schema_dynamic_anchors[new_idx] = NULL;
      }
      spec->defined_schema_names[new_idx] = C_CDD_STRDUP(def->name);

      struct_fields_init(&spec->defined_schemas[new_idx]);
      for (j = 0; j < def->details.struct_fields->size; j++) {
        struct StructField *f = &def->details.struct_fields->fields[j];
        struct StructField *new_f;
        size_t k;
        struct_fields_add(&spec->defined_schemas[new_idx], f->name, f->type,
                          f->ref, f->default_val, f->bit_width);

        new_f = &spec->defined_schemas[new_idx]
                     .fields[spec->defined_schemas[new_idx].size - 1];
        if (f->n_type_union > 0) {
          new_f->n_type_union = f->n_type_union;
          new_f->type_union = C_CDD_CALLOC(f->n_type_union, sizeof(char *));
          for (k = 0; k < f->n_type_union; k++)
            new_f->type_union[k] = C_CDD_STRDUP(f->type_union[k]);
        }
        if (f->n_items_type_union > 0) {
          new_f->n_items_type_union = f->n_items_type_union;
          new_f->items_type_union =
              C_CDD_CALLOC(f->n_items_type_union, sizeof(char *));
          for (k = 0; k < f->n_items_type_union; k++)
            new_f->items_type_union[k] = C_CDD_STRDUP(f->items_type_union[k]);
        }
      }
    } else if (def->kind == KIND_ENUM && def->name &&
               def->details.enum_members) {
      size_t new_idx = spec->n_defined_schemas;
      spec->n_defined_schemas++;
      spec->defined_schema_names = C_CDD_REALLOC(
          spec->defined_schema_names, spec->n_defined_schemas * sizeof(char *));
      spec->defined_schemas =
          C_CDD_REALLOC(spec->defined_schemas,
                        spec->n_defined_schemas * sizeof(struct StructFields));
      if (spec->defined_schema_ids) {
        spec->defined_schema_ids = C_CDD_REALLOC(
            spec->defined_schema_ids, spec->n_defined_schemas * sizeof(char *));
        spec->defined_schema_ids[new_idx] = NULL;
      }
      if (spec->defined_schema_anchors) {
        spec->defined_schema_anchors =
            C_CDD_REALLOC(spec->defined_schema_anchors,
                          spec->n_defined_schemas * sizeof(char *));
        spec->defined_schema_anchors[new_idx] = NULL;
      }
      if (spec->defined_schema_dynamic_anchors) {
        spec->defined_schema_dynamic_anchors =
            C_CDD_REALLOC(spec->defined_schema_dynamic_anchors,
                          spec->n_defined_schemas * sizeof(char *));
        spec->defined_schema_dynamic_anchors[new_idx] = NULL;
      }
      spec->defined_schema_names[new_idx] = C_CDD_STRDUP(def->name);

      struct_fields_init(&spec->defined_schemas[new_idx]);
      spec->defined_schemas[new_idx].is_enum = 1;
      enum_members_init(&spec->defined_schemas[new_idx].enum_members);
      for (j = 0; j < def->details.enum_members->size; j++) {
        enum_members_add(&spec->defined_schemas[new_idx].enum_members,
                         def->details.enum_members->members[j]);
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Sets custom JSON memory allocation functions in the library.
 *
 * @param[in] malloc_fun Custom memory allocation function.
 * @param[in] free_fun Custom memory deallocation function.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t c2openapi_set_json_allocators(
    void *(*malloc_fun)(size_t), void (*free_fun)(void *)) {
  json_set_allocation_functions(malloc_fun, free_fun);
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT cdd_c_error_t cdd_test_c2openapi_infer_client_route(
    const struct CstNode *func_node, const struct TokenList *tokens,
    char **out_route) {
  return c2openapi_infer_client_route(func_node, tokens, out_route);
}

C_CDD_EXPORT cdd_c_error_t cdd_test_c2openapi_scan_server_routes(
    const struct TokenList *tokens, struct OpenAPI_Spec *spec) {
  return c2openapi_scan_server_routes(tokens, spec);
}

C_CDD_EXPORT cdd_c_error_t cdd_test_c2openapi_scan_gui_views(
    const struct TokenList *tokens, struct OpenAPI_Spec *spec) {
  return c2openapi_scan_gui_views(tokens, spec);
}

/**
 * @brief Helper function cdd_test_c2openapi_scan_mcp_tools.
 *
 * @param[in] tokens Token list to scan for MCP tool registrations.
 * @param[in,out] spec Target OpenAPI specification.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_c2openapi_scan_mcp_tools(
    const struct TokenList *tokens, struct OpenAPI_Spec *spec) {
  return c2openapi_scan_mcp_tools(tokens, spec);
}
#endif /* CDD_BUILD_TESTS */
