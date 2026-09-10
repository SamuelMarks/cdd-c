/**
 * @file sync.c
 * @brief Implementation of sync parsing.
 *
 * Scans C source files to find functions corresponding to OpenAPI operations,
 * and updates signatures, query parameters, header parameters, and URL
 * construction.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "functions/emit/client_sig.h"
#include "functions/emit/patcher.h"
#include "functions/parse/cst.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
#include "routes/emit/url.h"
#include "routes/parse/sync.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_fail_make_tmpfile;
extern C_CDD_EXPORT int g_cdd_fail_find_function_node;
extern C_CDD_EXPORT int g_cdd_fail_generate_expected_sig;
extern C_CDD_EXPORT int g_cdd_fail_extract_current_sig;
extern C_CDD_EXPORT int g_cdd_fail_apply_query_sync;
extern C_CDD_EXPORT int g_cdd_fail_apply_header_sync;
extern C_CDD_EXPORT int g_cdd_fail_generate_expected_url;
extern C_CDD_EXPORT int g_cdd_fail_codegen_write;
#endif

/**
 * @brief Creates a temporary file in memory or system temp.
 *
 * @param[out] out_file Pointer to receive the opened temporary file.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t make_cdd_tmpfile(FILE **out_file) {
  FILE *f = NULL;
  if (!out_file)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#if defined(__wasm__) || defined(__wasm32__)
  *out_file = NULL;
  return CDD_C_ERROR_IO;
#elif defined(_MSC_VER)
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_make_tmpfile) {
    g_cdd_fail_make_tmpfile = 0;
    *out_file = NULL;
    return CDD_C_ERROR_IO;
  }
#endif
  if (tmpfile_s(&f) != 0 || !f) {
    *out_file = NULL;
    return CDD_C_ERROR_IO;
  }
  *out_file = f;
  return CDD_C_SUCCESS;
#else
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_make_tmpfile) {
    g_cdd_fail_make_tmpfile = 0;
    f = NULL;
  } else
#endif
    f = tmpfile();
  if (!f) {
    *out_file = NULL;
    return CDD_C_ERROR_IO;
  }
  *out_file = f;
  return CDD_C_SUCCESS;
#endif
}

/**
 * @brief Generate expected signature string for an OpenAPI operation.
 *
 * @param[in] op Operation specification.
 * @param[in] cfg Synchronization configuration.
 * @param[out] _out_val Pointer to receive allocated signature string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t generate_expected_sig(const struct OpenAPI_Operation *op,
                                           const struct ApiSyncConfig *cfg,
                                           char **_out_val) {
  FILE *tmp = NULL;
  long sz;
  char *buf;
  struct CodegenSigConfig sig_cfg;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;

  if (!op || !cfg)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = make_cdd_tmpfile(&tmp);
  if (rc != CDD_C_SUCCESS)
    return rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_generate_expected_sig) {
    g_cdd_fail_generate_expected_sig = 0;
    fclose(tmp);
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  memset(&sig_cfg, 0, sizeof(sig_cfg));
  sig_cfg.prefix = cfg->func_prefix;
  sig_cfg.include_semicolon = 0;

  rc = codegen_client_write_signature(tmp, op, &sig_cfg);
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_codegen_write) {
    g_cdd_fail_codegen_write = 0;
    rc = CDD_C_ERROR_IO;
  }
#endif
  if (rc != CDD_C_SUCCESS) {
    fclose(tmp);
    return CDD_C_ERROR_IO;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  buf = (char *)C_CDD_MALLOC((size_t)sz + 1);
  if (!buf) {
    fclose(tmp);
    return CDD_C_ERROR_MEMORY;
  }

  {
    size_t len;
    size_t bytes_read = fread(buf, 1, (size_t)sz, tmp);
    buf[bytes_read] = '\0';
    c_cdd_str_trim_trailing_whitespace(buf);
    len = strlen(buf);
#ifdef CDD_BUILD_TESTS
    {
      extern C_CDD_EXPORT int g_cdd_sync_sig_no_brace;
      if (g_cdd_sync_sig_no_brace) {
        g_cdd_sync_sig_no_brace = 0;
        buf[len - 1] = ')';
      }
    }
#endif
    if (buf[len - 1] == '{')
      buf[len - 1] = '\0';
    c_cdd_str_trim_trailing_whitespace(buf);
  }
  fclose(tmp);
  *_out_val = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Generate Query parameters block for an OpenAPI operation.
 *
 * @param[in] op Operation specification.
 * @param[out] _out_val Pointer to receive allocated query block string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t generate_expected_query(const struct OpenAPI_Operation *op,
                                             char **_out_val) {
  FILE *tmp = NULL;
  long sz;
  char *buf;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;

  if (!op)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = make_cdd_tmpfile(&tmp);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = codegen_url_write_query_params(tmp, op, 0);
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_codegen_write) {
    g_cdd_fail_codegen_write = 0;
    rc = CDD_C_ERROR_IO;
  }
#endif
  if (rc != CDD_C_SUCCESS) {
    fclose(tmp);
    return CDD_C_ERROR_IO;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  buf = (char *)C_CDD_MALLOC((size_t)sz + 1);
  if (!buf) {
    fclose(tmp);
    return CDD_C_ERROR_MEMORY;
  }

  {
    size_t bytes_read = fread(buf, 1, (size_t)sz, tmp);
    buf[bytes_read] = '\0';
  }
  fclose(tmp);
  *_out_val = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Generate Header parameters block line by line.
 *
 * @param[in] p Parameter specification.
 * @param[out] _out_val Pointer to receive allocated header line string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
generate_expected_header_line(const struct OpenAPI_Parameter *p,
                              char **_out_val) {
  char *buf;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;

  if (!p || !p->name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  buf = (char *)C_CDD_MALLOC(512);
  if (!buf)
    return CDD_C_ERROR_MEMORY;

  if (p->type && strcmp(p->type, "string") == 0) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(buf, 512,
              "  /* Header Parameter: %s */\n  if (%s) {\n    rc = "
              "http_headers_add(&req.headers, \"%s\", %s);\n    if "
              "(rc != CDD_C_SUCCESS) goto cleanup;\n  }\n",
              p->name, p->name, p->name, p->name);
#else
    sprintf(buf,
            "  /* Header Parameter: %s */\n  if (%s) {\n    rc = "
            "http_headers_add(&req.headers, \"%s\", %s);\n    if (rc != "
            "CDD_C_SUCCESS) "
            "goto cleanup;\n  }\n",
            p->name, p->name, p->name, p->name);
#endif
  } else if (p->type && strcmp(p->type, "integer") == 0) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(buf, 512,
              "  /* Header Parameter: %s */\n  {\n    char num_buf[32];\n    "
              "sprintf_s(num_buf, sizeof(num_buf), \"%%d\", %s);\n    rc = "
              "http_headers_add(&req.headers, \"%s\", num_buf);\n    if (rc != "
              "0) goto cleanup;\n  }\n",
              p->name, p->name, p->name);
#else
    sprintf(buf,
            "  /* Header Parameter: %s */\n  {\n    char num_buf[32];\n    "
            "sprintf(num_buf, \"%%d\", %s);\n    rc = "
            "http_headers_add(&req.headers, \"%s\", num_buf);\n    if (rc != "
            "0) goto cleanup;\n  }\n",
            p->name, p->name, p->name);
#endif
  } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(buf, 512,
              "  /* Header Parameter: %s (Type unhandled in sync) */\n",
              p->name);
#else
    sprintf(buf, "  /* Header Parameter: %s (Type unhandled in sync) */\n",
            p->name);
#endif
  }
  *_out_val = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Generate URL builder logic for an OpenAPI operation.
 *
 * @param[in] path Path template string.
 * @param[in] op Operation specification.
 * @param[in] cfg Synchronization configuration.
 * @param[out] _out_val Pointer to receive allocated URL builder string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t generate_expected_url(const char *path,
                                           const struct OpenAPI_Operation *op,
                                           const struct ApiSyncConfig *cfg,
                                           char **_out_val) {
  FILE *tmp = NULL;
  long sz;
  char *buf;
  struct CodegenUrlConfig url_cfg;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;

  if (!path || !op || !cfg)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = make_cdd_tmpfile(&tmp);
  if (rc != CDD_C_SUCCESS)
    return rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_generate_expected_url) {
    g_cdd_fail_generate_expected_url = 0;
    fclose(tmp);
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  memset(&url_cfg, 0, sizeof(url_cfg));
  url_cfg.out_variable = cfg->url_var_name ? cfg->url_var_name : "url";

  rc = codegen_url_write_builder(tmp, path, op->parameters, op->n_parameters,
                                 &url_cfg);
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_codegen_write) {
    g_cdd_fail_codegen_write = 0;
    rc = CDD_C_ERROR_IO;
  }
#endif
  if (rc != CDD_C_SUCCESS) {
    fclose(tmp);
    return CDD_C_ERROR_IO;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  buf = (char *)C_CDD_MALLOC((size_t)sz + 1);
  if (!buf) {
    fclose(tmp);
    return CDD_C_ERROR_MEMORY;
  }

  {
    size_t bytes_read = fread(buf, 1, (size_t)sz, tmp);
    buf[bytes_read] = '\0';
  }
  fclose(tmp);
  *_out_val = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Finds a function node in the CST matching the given function name.
 *
 * @param[in] cst CST list.
 * @param[in] tokens Token list.
 * @param[in] func_name Target function name.
 * @param[out] _out_val Pointer to receive found CstNode or NULL.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t find_function_node(struct CstNodeList *cst,
                                        struct TokenList *tokens,
                                        const char *func_name,
                                        struct CstNode **_out_val) {
  size_t i;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;

  if (!cst || !tokens || !func_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_find_function_node) {
    g_cdd_fail_find_function_node = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  for (i = 0; i < cst->size; ++i) {
    if (cst->nodes[i].kind == CST_NODE_FUNCTION) {
      struct CstNode *node = &cst->nodes[i];
      size_t k;
      for (k = node->start_token; k < node->end_token; ++k) {
        if (tokens->tokens[k].kind == TOKEN_LPAREN) {
          if (k > node->start_token) {
            int _ast_token_matches_string_0 = 0;
            size_t id_idx = k - 1;
            cdd_c_error_t rc;
            while (id_idx > node->start_token &&
                   tokens->tokens[id_idx].kind == TOKEN_WHITESPACE)
              id_idx--;
            if (tokens->tokens[id_idx].kind == TOKEN_IDENTIFIER) {
              rc = token_matches_string(&tokens->tokens[id_idx], func_name,
                                        &_ast_token_matches_string_0);
              if (rc != CDD_C_SUCCESS)
                return rc;
              if (_ast_token_matches_string_0) {
                *_out_val = node;
                return CDD_C_SUCCESS;
              }
            }
          }
          break;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts the current function signature tokens from a CST node.
 *
 * @param[in] tokens Token list.
 * @param[in] node Function CST node.
 * @param[out] out_end_idx Pointer to receive token index after signature.
 * @param[out] _out_val Pointer to receive allocated signature string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t extract_current_sig(struct TokenList *tokens,
                                         struct CstNode *node,
                                         size_t *out_end_idx, char **_out_val) {
  size_t i;
  size_t start;
  size_t args_end = 0;
  int found_start = 0;
  int depth = 0;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;

  if (!tokens || !node)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_extract_current_sig) {
    g_cdd_fail_extract_current_sig = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  start = node->start_token;
  for (i = start; i < node->end_token; ++i) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      if (!found_start)
        depth = 0;
      found_start = 1;
      depth++;
    } else if (tokens->tokens[i].kind == TOKEN_RPAREN) {
      depth--;
      if (found_start && depth == 0) {
        args_end = i;
        break;
      }
    } else if (tokens->tokens[i].kind == TOKEN_LBRACE) {
      break;
    }
  }

  if (args_end > start) {
    size_t length = 0;
    size_t k;
    char *buf, *p;
    for (k = start; k <= args_end; ++k)
      length += tokens->tokens[k].length;
    buf = (char *)C_CDD_MALLOC(length + 1);
    if (!buf)
      return CDD_C_ERROR_MEMORY;
    p = buf;
    for (k = start; k <= args_end; ++k) {
      memcpy(p, tokens->tokens[k].start, tokens->tokens[k].length);
      p += tokens->tokens[k].length;
    }
    *p = '\0';
    if (out_end_idx)
      *out_end_idx = args_end + 1;
    *_out_val = buf;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Applies query synchronization patches to function body.
 *
 * @param[in] op Operation specification.
 * @param[in] tokens Token list.
 * @param[in] node Function CST node.
 * @param[in,out] patches Patch list to populate.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t apply_query_sync(const struct OpenAPI_Operation *op,
                                      struct TokenList *tokens,
                                      struct CstNode *node,
                                      struct PatchList *patches) {
  size_t k;
  size_t init_idx = 0;
  size_t build_idx = 0;
  size_t body_start = 0;
  cdd_c_error_t rc;

  if (!op || !tokens || !node || !patches)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_apply_query_sync) {
    g_cdd_fail_apply_query_sync = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  for (k = node->start_token; k < node->end_token; k++) {
    if (tokens->tokens[k].kind == TOKEN_LBRACE) {
      body_start = k + 1;
      break;
    }
  }
  if (!body_start)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (k = body_start; k < node->end_token; k++) {
    if (tokens->tokens[k].kind == TOKEN_IDENTIFIER) {
      if (!init_idx) {
        int _ast_token_matches_string_1 = 0;
        rc = token_matches_string(&tokens->tokens[k], "url_query_init",
                                  &_ast_token_matches_string_1);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (_ast_token_matches_string_1)
          init_idx = k;
      }
      if (!build_idx) {
        int _ast_token_matches_string_2 = 0;
        rc = token_matches_string(&tokens->tokens[k], "url_query_build",
                                  &_ast_token_matches_string_2);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (_ast_token_matches_string_2)
          build_idx = k;
      }
    }
  }

  if (init_idx > 0 && build_idx > 0) {
    size_t start_stmt = init_idx;
    size_t end_stmt = build_idx;
    char *new_blk = NULL;

    while (start_stmt > body_start &&
           tokens->tokens[start_stmt - 1].kind != TOKEN_SEMICOLON &&
           tokens->tokens[start_stmt - 1].kind != TOKEN_RBRACE) {
      start_stmt--;
    }

    while (end_stmt < node->end_token &&
           tokens->tokens[end_stmt].kind != TOKEN_SEMICOLON) {
      end_stmt++;
    }
    end_stmt++;

    rc = generate_expected_query(op, &new_blk);
    if (rc != CDD_C_SUCCESS)
      return rc;

    rc = patch_list_add(patches, start_stmt, end_stmt, new_blk);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Applies header synchronization patches to function body.
 *
 * @param[in] op Operation specification.
 * @param[in] tokens Token list.
 * @param[in] node Function CST node.
 * @param[in,out] patches Patch list to populate.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t apply_header_sync(const struct OpenAPI_Operation *op,
                                       struct TokenList *tokens,
                                       struct CstNode *node,
                                       struct PatchList *patches) {
  size_t i, k;
  cdd_c_error_t rc;

  if (!op || !tokens || !node || !patches)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_apply_header_sync) {
    g_cdd_fail_apply_header_sync = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_HEADER) {
      char comment_text[256];
      size_t found_idx = 0;

      if (!op->parameters[i].name)
        continue;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(comment_text, sizeof(comment_text),
                "/* Header Parameter: %s */", op->parameters[i].name);
#else
      sprintf(comment_text, "/* Header Parameter: %s */",
              op->parameters[i].name);
#endif

      for (k = node->start_token; k < node->end_token; ++k) {
        if (tokens->tokens[k].kind == TOKEN_COMMENT) {
          int _ast_token_matches_string_4 = 0;
          rc = token_matches_string(&tokens->tokens[k], comment_text,
                                    &_ast_token_matches_string_4);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (_ast_token_matches_string_4) {
            found_idx = k;
            break;
          }
        }
      }

      if (found_idx > 0) {
        size_t end_logic = found_idx + 1;
        int depth = 0;
        int entered = 0;
        while (end_logic < node->end_token) {
          if (tokens->tokens[end_logic].kind == TOKEN_LBRACE) {
            depth++;
            entered = 1;
          } else if (tokens->tokens[end_logic].kind == TOKEN_RBRACE) {
            depth--;
            if (entered && depth == 0) {
              end_logic++;
              break;
            }
          }
          end_logic++;
        }

        if (entered) {
          char *new_hdr = NULL;
          rc = generate_expected_header_line(&op->parameters[i], &new_hdr);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = patch_list_add(patches, found_idx, end_logic, new_hdr);
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
 * @brief Applies all updates to functions in source file matching the OpenAPI
 * spec.
 *
 * @param[in] filename Target source filename.
 * @param[in] tokens Token list.
 * @param[in] cst CST list.
 * @param[in] spec OpenAPI spec.
 * @param[in] cfg Synchronization configuration.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t apply_updates(const char *filename,
                                   struct TokenList *tokens,
                                   struct CstNodeList *cst,
                                   const struct OpenAPI_Spec *spec,
                                   const struct ApiSyncConfig *cfg) {
  struct PatchList patches;
  size_t i, j;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  char *result = NULL;

  if (!filename || !tokens || !cst || !spec || !cfg)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (patch_list_init(&patches) != 0)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < spec->n_paths; ++i) {
    const struct OpenAPI_Path *p = &spec->paths[i];
    for (j = 0; j < p->n_operations; ++j) {
      struct OpenAPI_Operation *op = &p->operations[j];
      char func_name[256];
      struct CstNode *node = NULL;

      if (!op->operation_id)
        continue;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(func_name, sizeof(func_name), "%s%s",
                cfg->func_prefix ? cfg->func_prefix : "", op->operation_id);
#else
      sprintf(func_name, "%s%s", cfg->func_prefix ? cfg->func_prefix : "",
              op->operation_id);
#endif

      rc = find_function_node(cst, tokens, func_name, &node);
      if (rc != CDD_C_SUCCESS) {
        patch_list_free(&patches);
        return rc;
      }

      if (node) {
        char *expected_sig = NULL;
        char *actual_sig = NULL;
        size_t sig_end_idx = 0;
        size_t body_start = 0;
        size_t k;

        for (k = node->start_token; k < node->end_token; ++k) {
          if (tokens->tokens[k].kind == TOKEN_LBRACE) {
            body_start = k + 1;
            break;
          }
        }
        if (!body_start)
          continue;

        rc = generate_expected_sig(op, cfg, &expected_sig);
        if (rc != CDD_C_SUCCESS) {
          patch_list_free(&patches);
          return rc;
        }

        rc = extract_current_sig(tokens, node, &sig_end_idx, &actual_sig);
        if (rc != CDD_C_SUCCESS) {
          free(expected_sig);
          patch_list_free(&patches);
          return rc;
        }

        if (actual_sig) {
          if (strcmp(expected_sig, actual_sig) != 0) {
            char *_ast_strdup_0 = NULL;
            rc = c_cdd_strdup(expected_sig, &_ast_strdup_0);
            if (rc != CDD_C_SUCCESS) {
              free(expected_sig);
              free(actual_sig);
              patch_list_free(&patches);
              return rc;
            }
            rc = patch_list_add(&patches, node->start_token, sig_end_idx,
                                _ast_strdup_0);
            if (rc != CDD_C_SUCCESS) {
              free(expected_sig);
              free(actual_sig);
              patch_list_free(&patches);
              return rc;
            }
          }
          free(actual_sig);
        }
        free(expected_sig);

        rc = apply_query_sync(op, tokens, node, &patches);
        if (rc != CDD_C_SUCCESS) {
          patch_list_free(&patches);
          return rc;
        }

        rc = apply_header_sync(op, tokens, node, &patches);
        if (rc != CDD_C_SUCCESS) {
          patch_list_free(&patches);
          return rc;
        }

        /* 4. Sync URL Builder */
        {
          size_t asprintf_idx = 0;
          size_t stmt_end_idx = 0;
          const char *var = cfg->url_var_name ? cfg->url_var_name : "url";

          for (k = body_start; k < node->end_token; ++k) {
            if (tokens->tokens[k].kind == TOKEN_IDENTIFIER) {
              int _ast_token_matches_string_9 = 0;
              int _ast_token_matches_string_10 = 0;
              rc = token_matches_string(&tokens->tokens[k], "asprintf",
                                        &_ast_token_matches_string_9);
              if (rc != CDD_C_SUCCESS) {
                patch_list_free(&patches);
                return rc;
              }
              rc = token_matches_string(&tokens->tokens[k], "snprintf",
                                        &_ast_token_matches_string_10);
              if (rc != CDD_C_SUCCESS) {
                patch_list_free(&patches);
                return rc;
              }
              if (_ast_token_matches_string_9 || _ast_token_matches_string_10) {
                asprintf_idx = k;
                while (k < node->end_token &&
                       tokens->tokens[k].kind != TOKEN_SEMICOLON)
                  k++;
                stmt_end_idx = k + 1;
                break;
              }
            }
          }

          if (asprintf_idx > 0) {
            int matches_var = 0;
            size_t m;
            for (m = asprintf_idx; m < stmt_end_idx; ++m) {
              int _ast_token_matches_string_11 = 0;
              rc = token_matches_string(&tokens->tokens[m], var,
                                        &_ast_token_matches_string_11);
              if (rc != CDD_C_SUCCESS) {
                patch_list_free(&patches);
                return rc;
              }
              if (_ast_token_matches_string_11) {
                matches_var = 1;
                break;
              }
            }
            if (matches_var) {
              char *new_block = NULL;
              rc = generate_expected_url(p->route, op, cfg, &new_block);
              if (rc != CDD_C_SUCCESS) {
                patch_list_free(&patches);
                return rc;
              }
              rc = patch_list_add(&patches, asprintf_idx, stmt_end_idx,
                                  new_block);
              if (rc != CDD_C_SUCCESS) {
                patch_list_free(&patches);
                return rc;
              }
            }
          }
        }
      }
    }
  }

  rc = patch_list_apply(&patches, tokens, &result);
  patch_list_free(&patches);

  if (rc == CDD_C_SUCCESS) {
    FILE *f = NULL;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    if (fopen_s(&f, filename, "w") == 0 && f) {
#else
    f = fopen(filename, "w");
    if (f) {
#endif
      fputs(result, f);
      fclose(f);
    } else {
      rc = CDD_C_ERROR_IO;
    }
    free(result);
  }

  return rc;
}

/**
 * @brief Synchronize a C source file with an OpenAPI specification.
 *
 * @param[in] filename Path to the C source file to update.
 * @param[in] spec The parsed OpenAPI specification.
 * @param[in] config Configuration options.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t api_sync_file(const char *filename,
                            const struct OpenAPI_Spec *spec,
                            const struct ApiSyncConfig *config) {
  char *content = NULL;
  size_t sz = 0;
  struct TokenList *tokens = NULL;
  struct CstNodeList cst;
  cdd_c_error_t rc;
  struct ApiSyncConfig default_cfg;
  const struct ApiSyncConfig *cfg;

  if (!filename || !spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  default_cfg.func_prefix = NULL;
  default_cfg.url_var_name = NULL;
  cfg = config ? config : &default_cfg;

  memset(&cst, 0, sizeof(cst));

  rc = read_to_file(filename, "r", &content, &sz);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = tokenize(az_span_create_from_str(content), &tokens);
  if (rc != CDD_C_SUCCESS) {
    free(content);
    return rc;
  }

  rc = parse_tokens(tokens, &cst);
  if (rc != CDD_C_SUCCESS) {
    free_token_list(tokens);
    free(content);
    return rc;
  }

  rc = apply_updates(filename, tokens, &cst, spec, cfg);

  free_cst_node_list(&cst);
  free_token_list(tokens);
  free(content);

  return rc;
}

#ifdef CDD_BUILD_TESTS
cdd_c_error_t cdd_test_sync_make_cdd_tmpfile(FILE **out_file) {
  return make_cdd_tmpfile(out_file);
}

cdd_c_error_t
cdd_test_sync_generate_expected_sig(const struct OpenAPI_Operation *op,
                                    const struct ApiSyncConfig *cfg,
                                    char **out_val) {
  return generate_expected_sig(op, cfg, out_val);
}

cdd_c_error_t
cdd_test_sync_generate_expected_query(const struct OpenAPI_Operation *op,
                                      char **out_val) {
  return generate_expected_query(op, out_val);
}

cdd_c_error_t
cdd_test_sync_generate_expected_header_line(const struct OpenAPI_Parameter *p,
                                            char **out_val) {
  return generate_expected_header_line(p, out_val);
}

cdd_c_error_t cdd_test_sync_generate_expected_url(
    const char *path, const struct OpenAPI_Operation *op,
    const struct ApiSyncConfig *cfg, char **out_val) {
  return generate_expected_url(path, op, cfg, out_val);
}

cdd_c_error_t cdd_test_sync_find_function_node(struct CstNodeList *cst,
                                               struct TokenList *tokens,
                                               const char *func_name,
                                               struct CstNode **out_val) {
  return find_function_node(cst, tokens, func_name, out_val);
}

cdd_c_error_t cdd_test_sync_extract_current_sig(struct TokenList *tokens,
                                                struct CstNode *node,
                                                size_t *out_end_idx,
                                                char **out_val) {
  return extract_current_sig(tokens, node, out_end_idx, out_val);
}

cdd_c_error_t cdd_test_sync_apply_query_sync(const struct OpenAPI_Operation *op,
                                             struct TokenList *tokens,
                                             struct CstNode *node,
                                             struct PatchList *patches) {
  return apply_query_sync(op, tokens, node, patches);
}

cdd_c_error_t
cdd_test_sync_apply_header_sync(const struct OpenAPI_Operation *op,
                                struct TokenList *tokens, struct CstNode *node,
                                struct PatchList *patches) {
  return apply_header_sync(op, tokens, node, patches);
}

cdd_c_error_t cdd_test_sync_apply_updates(const char *filename,
                                          struct TokenList *tokens,
                                          struct CstNodeList *cst,
                                          const struct OpenAPI_Spec *spec,
                                          const struct ApiSyncConfig *cfg) {
  return apply_updates(filename, tokens, cst, spec, cfg);
}
#endif /* CDD_BUILD_TESTS */
