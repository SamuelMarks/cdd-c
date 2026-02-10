/**
 * @file refactor_api_sync.c
 * @brief Implementation of API Synchronization.
 *
 * Updates implementation bodies to match OpenApi specs.
 * Handles:
 * - Signature rewrites.
 * - Query Parameter block generation (supporting Explode/Arrays).
 * - Header Parameter injection (primitive implementation locally).
 * - URL construction updates.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen_client_sig.h"
#include "codegen_url.h"
#include "cst_parser.h"
#include "fs.h"
#include "refactor_api_sync.h"
#include "str_utils.h"
#include "text_patcher.h"
#include "tokenizer.h"

#define CALL_AND_CHECK(x)                                                      \
  do {                                                                         \
    if ((x) != 0)                                                              \
      return EIO;                                                              \
  } while (0)

/* --- Generators (InMemory) --- */

/**
 * @brief Generate signature string.
 */
static char *generate_expected_sig(const struct OpenAPI_Operation *op,
                                   const struct ApiSyncConfig *cfg) {
  FILE *tmp = tmpfile();
  long sz;
  char *buf;
  struct CodegenSigConfig sig_cfg;

  if (!tmp)
    return NULL;

  memset(&sig_cfg, 0, sizeof(sig_cfg));
  sig_cfg.prefix = cfg->func_prefix;
  sig_cfg.include_semicolon = 0;

  if (codegen_client_write_signature(tmp, op, &sig_cfg) != 0) {
    fclose(tmp);
    return NULL;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  buf = (char *)malloc(sz + 1);
  if (buf) {
    fread(buf, 1, sz, tmp);
    buf[sz] = '\0';
    c_cdd_str_trim_trailing_whitespace(buf);
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '{')
      buf[len - 1] = '\0';
    c_cdd_str_trim_trailing_whitespace(buf);
  }
  fclose(tmp);
  return buf;
}

/**
 * @brief Generate Query parameters block.
 */
static char *generate_expected_query(const struct OpenAPI_Operation *op) {
  FILE *tmp = tmpfile();
  long sz;
  char *buf;

  if (!tmp)
    return NULL;

  if (codegen_url_write_query_params(tmp, op) != 0) {
    fclose(tmp);
    return NULL;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  buf = (char *)malloc(sz + 1);
  if (buf) {
    fread(buf, 1, sz, tmp);
    buf[sz] = '\0';
  }
  fclose(tmp);
  return buf;
}

/**
 * @brief Generate Header parameters block line by line.
 * Since we don't have access to codegen_client_body static functions, iterate
 * here.
 */
static char *generate_expected_header_line(const struct OpenAPI_Parameter *p) {
  char *buf = malloc(512);
  if (!buf)
    return NULL;

  if (strcmp(p->type, "string") == 0) {
    sprintf(buf,
            "  /* Header Parameter: %s */\n  if (%s) {\n    rc = "
            "http_headers_add(&req.headers, \"%s\", %s);\n    if (rc != 0) "
            "goto cleanup;\n  }\n",
            p->name, p->name, p->name, p->name);
  } else if (strcmp(p->type, "integer") == 0) {
    sprintf(buf,
            "  /* Header Parameter: %s */\n  {\n    char num_buf[32];\n    "
            "sprintf(num_buf, \"%%d\", %s);\n    rc = "
            "http_headers_add(&req.headers, \"%s\", num_buf);\n    if (rc != "
            "0) goto cleanup;\n  }\n",
            p->name, p->name, p->name);
  } else {
    sprintf(buf, "  /* Header Parameter: %s (Type unhandled in sync) */\n",
            p->name);
  }
  return buf;
}

/**
 * @brief Generate URL builder.
 */
static char *generate_expected_url(const char *path,
                                   const struct OpenAPI_Operation *op,
                                   const struct ApiSyncConfig *cfg) {
  FILE *tmp = tmpfile();
  long sz;
  char *buf;
  struct CodegenUrlConfig url_cfg;

  if (!tmp)
    return NULL;

  memset(&url_cfg, 0, sizeof(url_cfg));
  url_cfg.out_variable = cfg->url_var_name ? cfg->url_var_name : "url";

  if (codegen_url_write_builder(tmp, path, op->parameters, op->n_parameters,
                                &url_cfg) != 0) {
    fclose(tmp);
    return NULL;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  buf = (char *)malloc(sz + 1);
  if (buf) {
    fread(buf, 1, sz, tmp);
    buf[sz] = '\0';
  }
  fclose(tmp);
  return buf;
}

/* --- Parsing Utils --- */

static struct CstNode *find_function_node(struct CstNodeList *cst,
                                          struct TokenList *tokens,
                                          const char *func_name) {
  size_t i;
  for (i = 0; i < cst->size; ++i) {
    if (cst->nodes[i].kind == CST_NODE_FUNCTION) {
      struct CstNode *node = &cst->nodes[i];
      size_t k;
      for (k = node->start_token; k < node->end_token; ++k) {
        if (tokens->tokens[k].kind == TOKEN_LPAREN) {
          if (k > 0) {
            size_t id_idx = k - 1;
            while (id_idx > node->start_token &&
                   tokens->tokens[id_idx].kind == TOKEN_WHITESPACE)
              id_idx--;
            if (tokens->tokens[id_idx].kind == TOKEN_IDENTIFIER &&
                token_matches_string(&tokens->tokens[id_idx], func_name))
              return node;
          }
          break;
        }
      }
    }
  }
  return NULL;
}

static char *extract_current_sig(struct TokenList *tokens, struct CstNode *node,
                                 size_t *out_end_idx) {
  size_t i;
  size_t start = node->start_token;
  size_t args_end = 0;
  int found_start = 0;
  int depth = 0;

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
    buf = (char *)malloc(length + 1);
    p = buf;
    for (k = start; k <= args_end; ++k) {
      memcpy(p, tokens->tokens[k].start, tokens->tokens[k].length);
      p += tokens->tokens[k].length;
    }
    *p = '\0';
    if (out_end_idx)
      *out_end_idx = args_end + 1;
    return buf;
  }
  return NULL;
}

/* --- Applying updates --- */

static void apply_query_sync(const struct OpenAPI_Operation *op,
                             struct TokenList *tokens, struct CstNode *node,
                             struct PatchList *patches) {
  /* Locate "url_query_init" -> "url_query_build" */
  size_t k;
  size_t init_idx = 0;
  size_t build_idx = 0;

  size_t body_start = 0;
  for (k = node->start_token; k < node->end_token; k++) {
    if (tokens->tokens[k].kind == TOKEN_LBRACE) {
      body_start = k + 1;
      break;
    }
  }
  if (!body_start)
    return;

  for (k = body_start; k < node->end_token; k++) {
    if (tokens->tokens[k].kind == TOKEN_IDENTIFIER) {
      if (!init_idx &&
          token_matches_string(&tokens->tokens[k], "url_query_init")) {
        init_idx = k;
      }
      if (!build_idx &&
          token_matches_string(&tokens->tokens[k], "url_query_build")) {
        build_idx = k;
      }
    }
  }

  /* If we found an existing block, update it */
  if (init_idx > 0 && build_idx > 0) {
    /* Expand range to whole statements */
    size_t start_stmt = init_idx;
    size_t end_stmt = build_idx;

    /* Backtrack start */
    while (start_stmt > body_start &&
           tokens->tokens[start_stmt - 1].kind != TOKEN_SEMICOLON &&
           tokens->tokens[start_stmt - 1].kind != TOKEN_RBRACE) {
      start_stmt--;
    }

    /* Forward end */
    while (end_stmt < node->end_token &&
           tokens->tokens[end_stmt].kind != TOKEN_SEMICOLON) {
      end_stmt++;
    }
    end_stmt++; /* Include semi */

    {
      char *new_blk = generate_expected_query(op);
      if (new_blk) {
        patch_list_add(patches, start_stmt, end_stmt, new_blk);
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
}

static void apply_header_sync(const struct OpenAPI_Operation *op,
                              struct TokenList *tokens, struct CstNode *node,
                              struct PatchList *patches) {
  size_t i, k;
  /*
     Heuristic: Look for "Header Parameter: name" comment.
     If found, identify the logic following it and replace.
  */

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_HEADER) {
      char comment_text[256];
      size_t found_idx = 0;

      sprintf(comment_text, "/* Header Parameter: %s */",
              op->parameters[i].name);

      /* Scan for comment token that matches */
      for (k = node->start_token; k < node->end_token; ++k) {
        if (tokens->tokens[k].kind == TOKEN_COMMENT) {
          /* Tokenizer usually returns comment content with delimiters */
          if (token_matches_string(&tokens->tokens[k], comment_text)) {
            found_idx = k;
            break;
          }
        }
      }

      if (found_idx > 0) {
        /* Expect logic follows until next comment or closing brace logic?
           Typically generated header logic ends with '}' of the if block or
           block.
           We replace from comment start to the end of the `if` block.
        */
        size_t end_logic = found_idx + 1;

        /* Find matching scope */
        /* Generated code is:
           comment
           if (...) {
             ...
           }
        */
        int depth = 0;
        int entered = 0;
        while (end_logic < node->end_token) {
          if (tokens->tokens[end_logic].kind == TOKEN_LBRACE) {
            depth++;
            entered = 1;
          } else if (tokens->tokens[end_logic].kind == TOKEN_RBRACE) {
            depth--;
            if (entered && depth == 0) {
              end_logic++; /* Consume closing brace */
              break;
            }
          }
          end_logic++;
        }

        if (entered) {
          char *new_hdr = generate_expected_header_line(&op->parameters[i]);
          if (new_hdr) {
            patch_list_add(patches, found_idx, end_logic, new_hdr);
          }
        }
      }
    }
  }
}

static int apply_updates(const char *filename, const char *content,
                         struct TokenList *tokens, struct CstNodeList *cst,
                         const struct OpenAPI_Spec *spec,
                         const struct ApiSyncConfig *cfg) {
  struct PatchList patches;
  size_t i, j;
  int rc = 0;
  char *result = NULL;

  if (patch_list_init(&patches) != 0)
    return ENOMEM;

  for (i = 0; i < spec->n_paths; ++i) {
    const struct OpenAPI_Path *p = &spec->paths[i];
    for (j = 0; j < p->n_operations; ++j) {
      struct OpenAPI_Operation *op = &p->operations[j];
      char func_name[256];
      struct CstNode *node;

      if (!op->operation_id)
        continue;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(func_name, sizeof(func_name), "%s%s",
                cfg->func_prefix ? cfg->func_prefix : "", op->operation_id);
#else
      sprintf(func_name, "%s%s", cfg->func_prefix ? cfg->func_prefix : "",
              op->operation_id);
#endif

      node = find_function_node(cst, tokens, func_name);
      if (node) {
        /* 1. Sync Signature */
        char *expected_sig = generate_expected_sig(op, cfg);
        size_t sig_end_idx = 0;
        char *actual_sig = extract_current_sig(tokens, node, &sig_end_idx);

        if (expected_sig && actual_sig) {
          if (strcmp(expected_sig, actual_sig) != 0) {
            patch_list_add(&patches, node->start_token, sig_end_idx,
                           c_cdd_strdup(expected_sig));
          }
        }
        free(expected_sig);
        free(actual_sig);

        /* 2. Sync Query Block (Support for Arrays/Explode) */
        apply_query_sync(op, tokens, node, &patches);

        /* 3. Sync Header Logic */
        apply_header_sync(op, tokens, node, &patches);

        /* 4. Sync URL Builder */
        {
          size_t k;
          size_t body_start = 0;
          const char *var = cfg->url_var_name ? cfg->url_var_name : "url";

          for (k = node->start_token; k < node->end_token; ++k) {
            if (tokens->tokens[k].kind == TOKEN_LBRACE) {
              body_start = k + 1;
              break;
            }
          }

          if (body_start > 0) {
            size_t asprintf_idx = 0;
            size_t stmt_end_idx = 0;

            for (k = body_start; k < node->end_token; ++k) {
              if (tokens->tokens[k].kind == TOKEN_IDENTIFIER &&
                  (token_matches_string(&tokens->tokens[k], "asprintf") ||
                   token_matches_string(&tokens->tokens[k], "snprintf"))) {
                asprintf_idx = k;
                while (k < node->end_token &&
                       tokens->tokens[k].kind != TOKEN_SEMICOLON)
                  k++;
                stmt_end_idx = k + 1;
                break;
              }
            }

            if (asprintf_idx > 0) {
              int matches_var = 0;
              size_t m;
              for (m = asprintf_idx; m < stmt_end_idx; ++m) {
                if (token_matches_string(&tokens->tokens[m], var)) {
                  matches_var = 1;
                  break;
                }
              }
              if (matches_var) {
                char *new_block = generate_expected_url(p->route, op, cfg);
                if (new_block) {
                  patch_list_add(&patches, asprintf_idx, stmt_end_idx,
                                 new_block);
                }
              }
            }
          }
        }
      }
    }
  }

  rc = patch_list_apply(&patches, tokens, &result);
  patch_list_free(&patches);

  if (rc == 0 && result) {
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    if (fopen_s(&f, filename, "w") == 0 && f) {
#else
    f = fopen(filename, "w");
    if (f) {
#endif
      fputs(result, f);
      fclose(f);
    } else {
      rc = EIO;
    }
    free(result);
  }

  return rc;
}

int api_sync_file(const char *const filename,
                  const struct OpenAPI_Spec *const spec,
                  const struct ApiSyncConfig *const config) {
  char *content = NULL;
  size_t sz = 0;
  struct TokenList *tokens = NULL;
  struct CstNodeList cst = {0};
  int rc;
  struct ApiSyncConfig default_cfg = {NULL, NULL};
  const struct ApiSyncConfig *cfg = config ? config : &default_cfg;

  if (!filename || !spec)
    return EINVAL;

  rc = read_to_file(filename, "r", &content, &sz);
  if (rc != 0)
    return rc;

  if ((rc = tokenize(az_span_create_from_str(content), &tokens)) != 0) {
    free(content);
    return rc;
  }

  if ((rc = parse_tokens(tokens, &cst)) != 0) {
    free_token_list(tokens);
    free(content);
    return rc;
  }

  rc = apply_updates(filename, content, tokens, &cst, spec, cfg);

  free_cst_node_list(&cst);
  free_token_list(tokens);
  free(content);

  return rc;
}
