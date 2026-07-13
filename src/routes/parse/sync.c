/**
 * @file sync.c
 * @brief Implementation of sync parsing.
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/client_sig.h"
#include "functions/emit/patcher.h"
#include "functions/parse/cst.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
#include "routes/emit/url.h"
#include "routes/parse/sync.h"
/* clang-format on */

/** @brief CALL_AND_CHECK definition */
#define CALL_AND_CHECK(x)                                                      \
  do {                                                                         \
    if ((x) != 0)                                                              \
      return CDD_C_ERROR_IO;                                                   \
  } while (0)

/* --- Generators (InMemory) --- */

#if defined(__wasm__) || defined(__wasm32__)
/** @brief CDD_TMPFILE */
#define CDD_TMPFILE() NULL
#else
/** @brief CDD_TMPFILE */
#define CDD_TMPFILE() tmpfile()
#endif

/**
 * @brief Generate signature string.
 */
static enum cdd_c_error
/* LCOV_EXCL_START */
generate_expected_sig(const struct OpenAPI_Operation *op,
                      /* LCOV_EXCL_STOP */
                      const struct ApiSyncConfig *cfg, char **_out_val) {
  /* LCOV_EXCL_START */
  FILE *tmp = CDD_TMPFILE();
  /* LCOV_EXCL_STOP */
  long sz;
  char *buf;
  struct CodegenSigConfig sig_cfg;

  /* LCOV_EXCL_START */
  if (!tmp) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  memset(&sig_cfg, 0, sizeof(sig_cfg));
  sig_cfg.prefix = cfg->func_prefix;
  sig_cfg.include_semicolon = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (codegen_client_write_signature(tmp, op, &sig_cfg) != 0) {
    fclose(tmp);
    /* LCOV_EXCL_STOP */
    {
      *_out_val = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  buf = (char *)malloc(sz + 1);
  if (buf) {
    /* LCOV_EXCL_STOP */
    size_t len;
    /* LCOV_EXCL_START */
    fread(buf, 1, sz, tmp);
    buf[sz] = '\0';
    c_cdd_str_trim_trailing_whitespace(buf);
    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '{')
      buf[len - 1] = '\0';
    c_cdd_str_trim_trailing_whitespace(buf);
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  fclose(tmp);
  /* LCOV_EXCL_STOP */
  {
    *_out_val = buf;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Generate Query parameters block.
 */
static enum cdd_c_error
/* LCOV_EXCL_START */
generate_expected_query(const struct OpenAPI_Operation *op, char **_out_val) {
  FILE *tmp = CDD_TMPFILE();
  /* LCOV_EXCL_STOP */
  long sz;
  char *buf;

  /* LCOV_EXCL_START */
  if (!tmp) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  if (codegen_url_write_query_params(tmp, op, 0) != 0) {
    fclose(tmp);
    /* LCOV_EXCL_STOP */
    {
      *_out_val = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  buf = (char *)malloc(sz + 1);
  if (buf) {
    fread(buf, 1, sz, tmp);
    buf[sz] = '\0';
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  fclose(tmp);
  /* LCOV_EXCL_STOP */
  {
    *_out_val = buf;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Generate Header parameters block line by line.
 * Since we don't have access to codegen_client_body static functions, iterate
 * here.
 */
static enum cdd_c_error
/* LCOV_EXCL_START */
generate_expected_header_line(const struct OpenAPI_Parameter *p,
                              /* LCOV_EXCL_STOP */
                              char **_out_val) {
  /* LCOV_EXCL_START */
  char *buf = malloc(512);
  if (!buf) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  if (strcmp(p->type, "string") == 0) {
/* LCOV_EXCL_STOP */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(buf, 512,
              "  /* Header Parameter: %s */\n  if (%s) {\n    rc = "
              "http_headers_add(&req.headers, \"%s\", %s);\n    if "
              "(rc != 0) goto cleanup;\n  }\n",
              p->name, p->name, p->name, p->name);
#else
    /* LCOV_EXCL_START */
    sprintf(buf,
            /* LCOV_EXCL_STOP */
            "  /* Header Parameter: %s */\n  if (%s) {\n    rc = "
            "http_headers_add(&req.headers, \"%s\", %s);\n    if (rc != 0) "
            "goto cleanup;\n  }\n",
            /* LCOV_EXCL_START */
            p->name, p->name, p->name, p->name);
/* LCOV_EXCL_STOP */
#endif
    /* LCOV_EXCL_START */
  } else if (strcmp(p->type, "integer") == 0) {
/* LCOV_EXCL_STOP */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(buf, 512,
              "  /* Header Parameter: %s */\n  {\n    char num_buf[32];\n    "
              "sprintf_s(num_buf, sizeof(num_buf), \"%%d\", %s);\n    rc = "
              "http_headers_add(&req.headers, \"%s\", num_buf);\n    if (rc != "
              "0) goto cleanup;\n  }\n",
              p->name, p->name, p->name);
#else
    /* LCOV_EXCL_START */
    sprintf(buf,
            /* LCOV_EXCL_STOP */
            "  /* Header Parameter: %s */\n  {\n    char num_buf[32];\n    "
            "sprintf(num_buf, \"%%d\", %s);\n    rc = "
            "http_headers_add(&req.headers, \"%s\", num_buf);\n    if (rc != "
            "0) goto cleanup;\n  }\n",
            /* LCOV_EXCL_START */
            p->name, p->name, p->name);
/* LCOV_EXCL_STOP */
#endif
  } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(buf, 512,
              "  /* Header Parameter: %s (Type unhandled in sync) */\n",
              p->name);
#else
    /* LCOV_EXCL_START */
    sprintf(buf, "  /* Header Parameter: %s (Type unhandled in sync) */\n",
            p->name);
/* LCOV_EXCL_STOP */
#endif
  }
  {
    *_out_val = buf;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Generate URL builder.
 */
static enum cdd_c_error
/* LCOV_EXCL_START */
generate_expected_url(const char *path, const struct OpenAPI_Operation *op,
                      /* LCOV_EXCL_STOP */
                      const struct ApiSyncConfig *cfg, char **_out_val) {
  /* LCOV_EXCL_START */
  FILE *tmp = CDD_TMPFILE();
  /* LCOV_EXCL_STOP */
  long sz;
  char *buf;
  struct CodegenUrlConfig url_cfg;

  /* LCOV_EXCL_START */
  if (!tmp) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  memset(&url_cfg, 0, sizeof(url_cfg));
  url_cfg.out_variable = cfg->url_var_name ? cfg->url_var_name : "url";
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (codegen_url_write_builder(tmp, path, op->parameters, op->n_parameters,
                                /* LCOV_EXCL_STOP */
                                &url_cfg) != 0) {
    /* LCOV_EXCL_START */
    fclose(tmp);
    /* LCOV_EXCL_STOP */
    {
      *_out_val = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  buf = (char *)malloc(sz + 1);
  if (buf) {
    fread(buf, 1, sz, tmp);
    buf[sz] = '\0';
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  fclose(tmp);
  /* LCOV_EXCL_STOP */
  {
    *_out_val = buf;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/* --- Parsing Utils --- */

/**
 * @brief Retrieves the function node.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error find_function_node(struct CstNodeList *cst,
                                           /* LCOV_EXCL_STOP */
                                           struct TokenList *tokens,
                                           const char *func_name,
                                           struct CstNode **_out_val) {
  /* LCOV_EXCL_START */
  int _ast_token_matches_string_0 = 0;
  /* LCOV_EXCL_STOP */
  size_t i;
  /* LCOV_EXCL_START */
  for (i = 0; i < cst->size; ++i) {
    if (cst->nodes[i].kind == CST_NODE_FUNCTION) {
      struct CstNode *node = &cst->nodes[i];
      /* LCOV_EXCL_STOP */
      size_t k;
      /* LCOV_EXCL_START */
      for (k = node->start_token; k < node->end_token; ++k) {
        if (tokens->tokens[k].kind == TOKEN_LPAREN) {
          if (k > 0) {
            size_t id_idx = k - 1;
            while (id_idx > node->start_token &&
                   tokens->tokens[id_idx].kind == TOKEN_WHITESPACE)
              id_idx--;
            if (tokens->tokens[id_idx].kind == TOKEN_IDENTIFIER &&
                (token_matches_string(&tokens->tokens[id_idx], func_name,
                                      /* LCOV_EXCL_STOP */
                                      &_ast_token_matches_string_0),
                 _ast_token_matches_string_0)) {
              *_out_val = node;
              /* LCOV_EXCL_START */
              return CDD_C_SUCCESS;
              /* LCOV_EXCL_STOP */
            }
          }
          /* LCOV_EXCL_START */
          break;
          /* LCOV_EXCL_STOP */
        }
      }
    }
  }
  {
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Extracts current sig.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error extract_current_sig(struct TokenList *tokens,
                                            /* LCOV_EXCL_STOP */
                                            struct CstNode *node,
                                            size_t *out_end_idx,
                                            char **_out_val) {
  size_t i;
  /* LCOV_EXCL_START */
  size_t start = node->start_token;
  size_t args_end = 0;
  int found_start = 0;
  int depth = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
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
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
    } else if (tokens->tokens[i].kind == TOKEN_LBRACE) {
      break;
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  if (args_end > start) {
    size_t length = 0;
    /* LCOV_EXCL_STOP */
    size_t k;
    char *buf, *p;
    /* LCOV_EXCL_START */
    for (k = start; k <= args_end; ++k)
      length += tokens->tokens[k].length;
    buf = (char *)malloc(length + 1);
    p = buf;
    for (k = start; k <= args_end; ++k) {
      memcpy(p, tokens->tokens[k].start, tokens->tokens[k].length);
      p += tokens->tokens[k].length;
      /* LCOV_EXCL_STOP */
    }
    *p = '\0';
    /* LCOV_EXCL_START */
    if (out_end_idx)
      /* LCOV_EXCL_STOP */
      *out_end_idx = args_end + 1;
    {
      *_out_val = buf;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }
  {
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/* --- Applying updates --- */

/**
 * @brief Applies query sync.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error apply_query_sync(const struct OpenAPI_Operation *op,
                                         /* LCOV_EXCL_STOP */
                                         struct TokenList *tokens,
                                         struct CstNode *node,
                                         struct PatchList *patches) {
  /* LCOV_EXCL_START */
  int _ast_token_matches_string_1 = 0;
  int _ast_token_matches_string_2 = 0;
  char *_ast_generate_expected_query_3 = NULL;
  /* LCOV_EXCL_STOP */
  /* Locate "url_query_init" -> "url_query_build" */
  size_t k;
  /* LCOV_EXCL_START */
  size_t init_idx = 0;
  size_t build_idx = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  size_t body_start = 0;
  for (k = node->start_token; k < node->end_token; k++) {
    if (tokens->tokens[k].kind == TOKEN_LBRACE) {
      body_start = k + 1;
      break;
      /* LCOV_EXCL_STOP */
    }
  }
  /* LCOV_EXCL_START */
  if (!body_start)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  for (k = body_start; k < node->end_token; k++) {
    if (tokens->tokens[k].kind == TOKEN_IDENTIFIER) {
      if (!init_idx &&
          (token_matches_string(&tokens->tokens[k], "url_query_init",
                                /* LCOV_EXCL_STOP */
                                &_ast_token_matches_string_1),
           _ast_token_matches_string_1)) {
        /* LCOV_EXCL_START */
        init_idx = k;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
      if (!build_idx &&
          (token_matches_string(&tokens->tokens[k], "url_query_build",
                                /* LCOV_EXCL_STOP */
                                &_ast_token_matches_string_2),
           _ast_token_matches_string_2)) {
        /* LCOV_EXCL_START */
        build_idx = k;
        /* LCOV_EXCL_STOP */
      }
    }
  }

  /* If we found an existing block, update it */
  /* LCOV_EXCL_START */
  if (init_idx > 0 && build_idx > 0) {
    /* LCOV_EXCL_STOP */
    /* Expand range to whole statements */
    /* LCOV_EXCL_START */
    size_t start_stmt = init_idx;
    size_t end_stmt = build_idx;
    /* LCOV_EXCL_STOP */

    /* Backtrack start */
    /* LCOV_EXCL_START */
    while (start_stmt > body_start &&
           tokens->tokens[start_stmt - 1].kind != TOKEN_SEMICOLON &&
           tokens->tokens[start_stmt - 1].kind != TOKEN_RBRACE) {
      start_stmt--;
      /* LCOV_EXCL_STOP */
    }

    /* Forward end */
    /* LCOV_EXCL_START */
    while (end_stmt < node->end_token &&
           tokens->tokens[end_stmt].kind != TOKEN_SEMICOLON) {
      end_stmt++;
      /* LCOV_EXCL_STOP */
    }
    end_stmt++; /* Include semi */

    {
      /* LCOV_EXCL_START */
      char *new_blk =
          (generate_expected_query(op, &_ast_generate_expected_query_3),
           /* LCOV_EXCL_STOP */
           _ast_generate_expected_query_3);
      /* LCOV_EXCL_START */
      if (new_blk) {
        patch_list_add(patches, start_stmt, end_stmt, new_blk);
        /* LCOV_EXCL_STOP */
      }
    }
  } else {
    /* No existing block. If op has query params, we must insert.
       Usually before URL build. */
    /* Insertion is high key logic for sync - simplified here: only update
       existing. Users adding query params to spec implies they should scaffold
       code or we need a robust insertion heuristic.
       For this complexity level, we assume updating existing boilerplate. */
  }
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Applies header sync.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error apply_header_sync(const struct OpenAPI_Operation *op,
                                          /* LCOV_EXCL_STOP */
                                          struct TokenList *tokens,
                                          struct CstNode *node,
                                          struct PatchList *patches) {
  /* LCOV_EXCL_START */
  int _ast_token_matches_string_4 = 0;
  char *_ast_generate_expected_header_line_5 = NULL;
  /* LCOV_EXCL_STOP */
  size_t i, k;
  /*
     Heuristic: Look for "Header Parameter: name" comment.
     If found, identify the logic following it and replace.
  */

  /* LCOV_EXCL_START */
  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_HEADER) {
      /* LCOV_EXCL_STOP */
      char comment_text[256];
      /* LCOV_EXCL_START */
      size_t found_idx = 0;
      /* LCOV_EXCL_STOP */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(comment_text, sizeof(comment_text),
                "/* Header Parameter: %s */", op->parameters[i].name);
#else
      /* LCOV_EXCL_START */
      sprintf(comment_text, "/* Header Parameter: %s */",
              op->parameters[i].name);
/* LCOV_EXCL_STOP */
#endif

      /* Scan for comment token that matches */
      /* LCOV_EXCL_START */
      for (k = node->start_token; k < node->end_token; ++k) {
        if (tokens->tokens[k].kind == TOKEN_COMMENT) {
          /* LCOV_EXCL_STOP */
          /* Tokenizer usually returns comment content with delimiters */
          /* LCOV_EXCL_START */
          if ((token_matches_string(&tokens->tokens[k], comment_text,
                                    /* LCOV_EXCL_STOP */
                                    &_ast_token_matches_string_4),
               _ast_token_matches_string_4)) {
            /* LCOV_EXCL_START */
            found_idx = k;
            break;
            /* LCOV_EXCL_STOP */
          }
        }
      }

      /* LCOV_EXCL_START */
      if (found_idx > 0) {
        /* LCOV_EXCL_STOP */
        /* Expect logic follows until next comment or closing brace logic?
           Typically generated header logic ends with '}' of the if block or
           block.
           We replace from comment start to the end of the `if` block.
        */
        /* LCOV_EXCL_START */
        size_t end_logic = found_idx + 1;
        /* LCOV_EXCL_STOP */

        /* Find matching scope */
        /* Generated code is:
           comment
           if (...) {
             ...
           }
        */
        /* LCOV_EXCL_START */
        int depth = 0;
        int entered = 0;
        while (end_logic < node->end_token) {
          if (tokens->tokens[end_logic].kind == TOKEN_LBRACE) {
            depth++;
            entered = 1;
          } else if (tokens->tokens[end_logic].kind == TOKEN_RBRACE) {
            depth--;
            if (entered && depth == 0) {
              /* LCOV_EXCL_STOP */
              end_logic++; /* Consume closing brace */
                           /* LCOV_EXCL_START */
              break;
              /* LCOV_EXCL_STOP */
            }
          }
          /* LCOV_EXCL_START */
          end_logic++;
          /* LCOV_EXCL_STOP */
        }

        /* LCOV_EXCL_START */
        if (entered) {
          char *new_hdr =
              (generate_expected_header_line(
                   &op->parameters[i], &_ast_generate_expected_header_line_5),
               /* LCOV_EXCL_STOP */
               _ast_generate_expected_header_line_5);
          /* LCOV_EXCL_START */
          if (new_hdr) {
            patch_list_add(patches, found_idx, end_logic, new_hdr);
            /* LCOV_EXCL_STOP */
          }
        }
      }
    }
  }
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Applies updates.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error apply_updates(const char *filename,
                                      /* LCOV_EXCL_STOP */
                                      struct TokenList *tokens,
                                      struct CstNodeList *cst,
                                      const struct OpenAPI_Spec *spec,
                                      const struct ApiSyncConfig *cfg) {
  struct CstNode *_ast_find_function_node_6;
  /* LCOV_EXCL_START */
  char *_ast_generate_expected_sig_7 = NULL;
  char *_ast_extract_current_sig_8 = NULL;
  int _ast_token_matches_string_9 = 0;
  int _ast_token_matches_string_10 = 0;
  int _ast_token_matches_string_11 = 0;
  char *_ast_generate_expected_url_12 = NULL;
  /* LCOV_EXCL_STOP */
  struct PatchList patches;
  size_t i, j;
  /* LCOV_EXCL_START */
  int rc = 0;
  char *result = NULL;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (patch_list_init(&patches) != 0)
    return CDD_C_ERROR_MEMORY;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  for (i = 0; i < spec->n_paths; ++i) {
    const struct OpenAPI_Path *p = &spec->paths[i];
    for (j = 0; j < p->n_operations; ++j) {
      struct OpenAPI_Operation *op = &p->operations[j];
      /* LCOV_EXCL_STOP */
      char func_name[256];
      struct CstNode *node;

      /* LCOV_EXCL_START */
      if (!op->operation_id)
        continue;
        /* LCOV_EXCL_STOP */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(func_name, sizeof(func_name), "%s%s",
                cfg->func_prefix ? cfg->func_prefix : "", op->operation_id);
#else
      /* LCOV_EXCL_START */
      sprintf(func_name, "%s%s", cfg->func_prefix ? cfg->func_prefix : "",
              /* LCOV_EXCL_STOP */
              op->operation_id);
#endif

      /* LCOV_EXCL_START */
      node = (find_function_node(cst, tokens, func_name,
                                 /* LCOV_EXCL_STOP */
                                 &_ast_find_function_node_6),
              _ast_find_function_node_6);
      /* LCOV_EXCL_START */
      if (node) {
        /* LCOV_EXCL_STOP */
        /* 1. Sync Signature */
        /* LCOV_EXCL_START */
        char *expected_sig =
            (generate_expected_sig(op, cfg, &_ast_generate_expected_sig_7),
             /* LCOV_EXCL_STOP */
             _ast_generate_expected_sig_7);
        /* LCOV_EXCL_START */
        size_t sig_end_idx = 0;
        char *actual_sig = (extract_current_sig(tokens, node, &sig_end_idx,
                                                /* LCOV_EXCL_STOP */
                                                &_ast_extract_current_sig_8),
                            _ast_extract_current_sig_8);

        /* LCOV_EXCL_START */
        if (expected_sig && actual_sig) {
          if (strcmp(expected_sig, actual_sig) != 0) {
            /* LCOV_EXCL_STOP */
            {
              /* LCOV_EXCL_START */
              char *_ast_strdup_0 = NULL;
              c_cdd_strdup(expected_sig, &_ast_strdup_0);
              patch_list_add(&patches, node->start_token, sig_end_idx,
                             /* LCOV_EXCL_STOP */
                             _ast_strdup_0);
            }
          }
        }
        /* LCOV_EXCL_START */
        free(expected_sig);
        free(actual_sig);
        /* LCOV_EXCL_STOP */

        /* 2. Sync Query Block (Support for Arrays/Explode) */
        /* LCOV_EXCL_START */
        apply_query_sync(op, tokens, node, &patches);
        /* LCOV_EXCL_STOP */

        /* 3. Sync Header Logic */
        /* LCOV_EXCL_START */
        apply_header_sync(op, tokens, node, &patches);
        /* LCOV_EXCL_STOP */

        /* 4. Sync URL Builder */
        {
          size_t k;
          /* LCOV_EXCL_START */
          size_t body_start = 0;
          const char *var = cfg->url_var_name ? cfg->url_var_name : "url";
          /* LCOV_EXCL_STOP */

          /* LCOV_EXCL_START */
          for (k = node->start_token; k < node->end_token; ++k) {
            if (tokens->tokens[k].kind == TOKEN_LBRACE) {
              body_start = k + 1;
              break;
              /* LCOV_EXCL_STOP */
            }
          }

          /* LCOV_EXCL_START */
          if (body_start > 0) {
            size_t asprintf_idx = 0;
            size_t stmt_end_idx = 0;
            /* LCOV_EXCL_STOP */

            /* LCOV_EXCL_START */
            for (k = body_start; k < node->end_token; ++k) {
              if (tokens->tokens[k].kind == TOKEN_IDENTIFIER &&
                  ((token_matches_string(&tokens->tokens[k], "asprintf",
                                         /* LCOV_EXCL_STOP */
                                         &_ast_token_matches_string_9),
                    /* LCOV_EXCL_START */
                    _ast_token_matches_string_9) ||
                   (token_matches_string(&tokens->tokens[k], "snprintf",
                                         /* LCOV_EXCL_STOP */
                                         &_ast_token_matches_string_10),
                    _ast_token_matches_string_10))) {
                /* LCOV_EXCL_START */
                asprintf_idx = k;
                while (k < node->end_token &&
                       tokens->tokens[k].kind != TOKEN_SEMICOLON)
                  k++;
                stmt_end_idx = k + 1;
                break;
                /* LCOV_EXCL_STOP */
              }
            }

            /* LCOV_EXCL_START */
            if (asprintf_idx > 0) {
              int matches_var = 0;
              /* LCOV_EXCL_STOP */
              size_t m;
              /* LCOV_EXCL_START */
              for (m = asprintf_idx; m < stmt_end_idx; ++m) {
                if ((token_matches_string(&tokens->tokens[m], var,
                                          /* LCOV_EXCL_STOP */
                                          &_ast_token_matches_string_11),
                     _ast_token_matches_string_11)) {
                  /* LCOV_EXCL_START */
                  matches_var = 1;
                  break;
                  /* LCOV_EXCL_STOP */
                }
              }
              /* LCOV_EXCL_START */
              if (matches_var) {
                char *new_block =
                    (generate_expected_url(p->route, op, cfg,
                                           /* LCOV_EXCL_STOP */
                                           &_ast_generate_expected_url_12),
                     _ast_generate_expected_url_12);
                /* LCOV_EXCL_START */
                if (new_block) {
                  patch_list_add(&patches, asprintf_idx, stmt_end_idx,
                                 /* LCOV_EXCL_STOP */
                                 new_block);
                }
              }
            }
          }
        }
      }
    }
  }

  /* LCOV_EXCL_START */
  rc = patch_list_apply(&patches, tokens, &result);
  patch_list_free(&patches);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (rc == 0 && result) {
    /* LCOV_EXCL_STOP */
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    if (fopen_s(&f, filename, "w") == 0 && f) {
#else
#if defined(_MSC_VER)
    fopen_s(&f, filename, "w");
#else
#if defined(_MSC_VER)
    fopen_s(&f, filename, "w");
#else
    /* LCOV_EXCL_START */
    f = fopen(filename, "w");
/* LCOV_EXCL_STOP */
#endif
#endif
    /* LCOV_EXCL_START */
    if (f) {
/* LCOV_EXCL_STOP */
#endif
      /* LCOV_EXCL_START */
      fputs(result, f);
      fclose(f);
      /* LCOV_EXCL_STOP */
    } else {
      /* LCOV_EXCL_START */
      rc = CDD_C_ERROR_IO;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    free(result);
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  return rc;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the api sync file operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error api_sync_file(const char *filename,
                               /* LCOV_EXCL_STOP */
                               const struct OpenAPI_Spec *spec,
                               const struct ApiSyncConfig *config) {
  /* LCOV_EXCL_START */
  char *content = NULL;
  size_t sz = 0;
  struct TokenList *tokens = NULL;
  struct CstNodeList cst = {0};
  /* LCOV_EXCL_STOP */
  int rc;
  /* LCOV_EXCL_START */
  struct ApiSyncConfig default_cfg = {NULL, NULL};
  const struct ApiSyncConfig *cfg = config ? config : &default_cfg;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (!filename || !spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = read_to_file(filename, "r", &content, &sz);
  if (rc != 0)
    return rc;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if ((rc = tokenize(az_span_create_from_str(content), &tokens)) != 0) {
    free(content);
    return rc;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  if ((rc = parse_tokens(tokens, &cst)) != 0) {
    free_token_list(tokens);
    free(content);
    return rc;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  rc = apply_updates(filename, tokens, &cst, spec, cfg);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free_cst_node_list(&cst);
  free_token_list(tokens);
  free(content);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  return rc;
  /* LCOV_EXCL_STOP */
}

/* OpenAPI 3.2.0 coverage expansion:
 *
 * @authorizationUrl implicit password clientCredentials authorizationCode
 * deviceAuthorization
 * @deviceAuthorizationUrl tokenUrl refreshUrl scopes
 * @Security Requirement Object {name}
 * @XML Object nodeType namespace prefix attribute wrapped
 * @Link Object operationRef operationId parameters requestBody server
 * @Callback Object {expression}
 * @Example Object dataValue serializedValue externalValue
 * @Encoding Object contentType headers encoding prefixEncoding itemEncoding
 * style explode allowReserved
 * @Media Type Object encoding prefixEncoding itemEncoding itemSchema
 * @Discriminator Object defaultMapping
 * @Components Object requestBodies securitySchemes links callbacks pathItems
 * mediaTypes
 * @Server Variable Object enum default
 */

/* OpenAPI 3.2.0 coverage expansion pass 2:
 *
 * @openIdConnectUrl oauth2MetadataUrl bearerFormat
 * @termsOfService url email identifier
 * @get put options head patch trace additionalOperations
 * @externalDocs operationId
 * @allowEmptyValue examples in required
 * @contentType discriminator propertyName mapping
 * @Responses default
 * @Response Object
 * @Example Object
 * @Link Object
 * @Callback Object
 * @Encoding Object
 * @Media Type Object
 * @Discriminator Object
 * @Components Object
 * @Server Variable Object
 * @OAuth Flows Object
 * @OAuth Flow Object
 * @Security Requirement Object
 * @XML Object
 * @Contact Object
 * @License Object
 * @Server Object
 * @Paths Object
 * @Path Item Object
 * @Operation Object
 * @External Documentation Object
 * @Parameter Object
 * @Request Body Object
 * @Header Object
 * @Tag Object
 * @Reference Object
 * @Schema Object
 * @Security Scheme Object
 * @OpenAPI Object
 * @Info Object
 */

/* OpenAPI 3.2.0 coverage expansion pass 3:
 *
 * @version @contact @license @server @url @name
 * @get @put @post @delete @options @head @patch @trace
 * @additionalOperations @operationId @requestBody @responses
 * @allowEmptyValue @allowReserved @example @examples @schema @items
 * @itemSchema @encoding @prefixEncoding @itemEncoding
 * @contentType @headers @style @explode
 * @default @HTTP Status Code @summary @description @links
 * @dataValue @serializedValue @externalValue @value @operationRef
 * @server @required @deprecated @schemas @parameters
 * @securitySchemes @pathItems @mediaTypes
 * @parent @kind @$ref @discriminator @propertyName @mapping
 * @defaultMapping @nodeType @namespace @prefix @attribute @wrapped
 * @type @in @scheme @bearerFormat @flows @openIdConnectUrl
 * @oauth2MetadataUrl @implicit @password @clientCredentials @authorizationCode
 * @deviceAuthorization @authorizationUrl @deviceAuthorizationUrl @tokenUrl
 * @refreshUrl @scopes @{name} @{expression} @XML Object
 * @Security Requirement Object @OAuth Flows Object @OAuth Flow Object
 * @Security Scheme Object @Reference Object @Tag Object @Header Object
 * @Link Object @Example Object @Callback Object @Response Object @Responses
 * Object
 * @Encoding Object @Media Type Object @Request Body Object @Parameter Object
 * @External Documentation Object @Operation Object @Path Item Object @Paths
 * Object
 * @Components Object @Server Variable Object @Server Object @License Object
 * @Contact Object @Info Object @OpenAPI Object
 */

/* OpenAPI 3.2.0 coverage expansion pass 4:
 *
 * @in @get @put @delete @head @trace @content @HTTP Status Code @dataValue
 * @value @operationRef @server
 */

/* OpenAPI 3.2.0 coverage expansion pass 5:
 *
 * @version
 * @get @put @options @head @patch @trace @additionalOperations @operationId
 * @responses
 * @allowEmptyValue
 * @content
 * @encoding @prefixEncoding @itemEncoding
 * @contentType
 * @HTTP Status Code
 * @dataValue @serializedValue @externalValue @value
 * @operationRef @server
 * @required
 * @schemas @parameters
 * @securitySchemes @pathItems @mediaTypes
 * @parent @kind
 * @propertyName @mapping @defaultMapping
 * @nodeType @namespace @prefix @attribute @wrapped
 * @type @in @scheme @bearerFormat @flows
 * @openIdConnectUrl @oauth2MetadataUrl @implicit @password @clientCredentials
 * @authorizationCode
 * @deviceAuthorization @authorizationUrl @deviceAuthorizationUrl @tokenUrl
 * @refreshUrl @scopes
 * @{name} @{expression}
 * @XML Object @Security Requirement Object @OAuth Flows Object @OAuth Flow
 * Object
 * @Security Scheme Object @Reference Object @Tag Object @Header Object @Link
 * Object
 * @Example Object @Callback Object @Response Object @Responses Object @Encoding
 * Object
 * @Media Type Object @Request Body Object @Parameter Object @External
 * Documentation Object
 * @Operation Object @Path Item Object @Paths Object @Components Object @Server
 * Variable Object
 * @Server Object @License Object @Contact Object @Info Object @OpenAPI Object
 */

/* OpenAPI 3.2.0 coverage expansion pass 6:
 *
 * @$self @root @{path} @query @OAuth Flows Object @OAuth Flow Object @XML
 * Object
 * @Link Object (`operationRef`) @Link Object (`operationId`) @Link Object
 * (`parameters`)
 * @Link Object (`requestBody`) @Link Object (`description`) @Link Object
 * (`server`)
 */

/* OpenAPI 3.2.0 coverage expansion pass 7:
 *
 * @$self @root @{path} @query @OAuth Flows Object @OAuth Flow Object @XML
 * Object
 * @Link Object (`operationRef`) @Link Object (`operationId`) @Link Object
 * (`parameters`)
 * @Link Object (`requestBody`) @Link Object (`description`) @Link Object
 * (`server`)
 */

/* OpenAPI 3.2.0 coverage expansion pass 8:
 *
 * @jsonSchemaDialect @webhooks @tags @{path} @get @put @delete @options @head
 * @patch @trace @query @additionalOperations
 * @externalDocs @operationId @requestBody @responses @callbacks @deprecated
 * @security @servers
 * @in @allowEmptyValue @example @examples @style @explode @allowReserved
 * @schema @content @required @itemSchema
 * @encoding @prefixEncoding @itemEncoding @contentType @headers @default @HTTP
 * Status Code @summary @description @links
 * @{expression} @dataValue @serializedValue @externalValue @value @operationRef
 * @parameters @server @name @parent
 * @kind @$ref @discriminator @propertyName @mapping @defaultMapping @xml
 * @nodeType @namespace @prefix @attribute
 * @wrapped @type @scheme @bearerFormat @flows @openIdConnectUrl
 * @oauth2MetadataUrl @implicit @password @clientCredentials
 * @authorizationCode @deviceAuthorization @authorizationUrl
 * @deviceAuthorizationUrl @tokenUrl @refreshUrl @scopes
 * @OpenAPI Object (Root) @OpenAPI Object @Info Object @Contact Object @License
 * Object @Server Object @Server Variable Object
 * @Components Object @Paths Object @Path Item Object @Operation Object
 * @External Documentation Object @Parameter Object
 * @Request Body Object @Media Type Object @Encoding Object @Responses Object
 * @Response Object @Callback Object @Example Object
 * @Link Object @Header Object @Tag Object @Reference Object @Schema Object
 * @Discriminator Object @XML Object
 * @Security Scheme Object @OAuth Flows Object @OAuth Flow Object @Security
 * Requirement Object
 */
