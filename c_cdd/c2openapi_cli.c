/**
 * @file c2openapi_cli.c
 * @brief Implementation of the C-to-OpenAPI CLI orchestrator.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c2openapi_cli.h"
#include "c2openapi_operation.h" /* For OpBuilder and C2OpenAPI_ParsedSig */
#include "c2openapi_schema.h"    /* For Type Registry */
#include "c_inspector.h"
#include "cst_parser.h"
#include "doc_parser.h"
#include "fs.h"
#include "openapi_aggregator.h"
#include "openapi_loader.h"
#include "openapi_writer.h"
#include "str_utils.h"
#include "tokenizer.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif

/* --- Helpers --- */

static int is_source_file(const char *path) {
  const char *ext = strrchr(path, '.');
  if (!ext)
    return 0;
  return (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0);
}

/**
 * @brief Simple signature parser to split "int foo(int x, char *y)"
 * Populates `out`. Caller must free internals.
 */
static int parse_c_signature_string(const char *sig_str,
                                    struct C2OpenAPI_ParsedSig *out) {
  struct TokenList *tl = NULL;
  size_t i;
  int rc = 0;
  size_t lp = 0, rp = 0;

  if (!sig_str || !out)
    return EINVAL;

  memset(out, 0, sizeof(*out));

  if (tokenize(az_span_create_from_str((char *)sig_str), &tl) != 0) {
    return EINVAL;
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
    return EINVAL;
  }

  /* Extract Name (token before lparen, ignoring WS) */
  {
    size_t k = lp - 1;
    while (k > 0 && tl->tokens[k].kind == TOKEN_WHITESPACE)
      k--;
    if (tl->tokens[k].kind == TOKEN_IDENTIFIER) {
      size_t len = tl->tokens[k].length;
      char *n = malloc(len + 1);
      if (!n) {
        rc = ENOMEM;
        goto cleanup;
      }
      memcpy(n, tl->tokens[k].start, len);
      n[len] = '\0';
      out->name = n;
    }
  }

  if (!out->name) {
    rc = EINVAL;
    goto cleanup;
  }

  /* Extract Args between ( and ) */
  /* Split by COMMA. For each segment, last ID is name, rest is type. */
  rp = token_find_next(tl, lp, tl->size, TOKEN_RPAREN);
  if (rp >= tl->size) {
    rc = EINVAL;
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
          /* Type is start..name_idx (exclusive) + modifiers after? */
          /* Simple approach: Type is [start, name_idx), Name is name_idx.
             Postfix arrays `[]` might be after name. */
          /* Let's grab name string */
          const struct Token *nt = &tl->tokens[name_idx];
          size_t t_end = name_idx;

          /* Check if type is pointer/const/struct before name */
          /* Construct type string */
          size_t t_len = 0;
          char *t_str;
          size_t m;
          char *n_str = malloc(nt->length + 1);
          if (!n_str) {
            rc = ENOMEM;
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

          t_str = malloc(t_len + 1);
          if (!t_str) {
            free(n_str);
            rc = ENOMEM;
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

          /* Add to list */
          {
            struct C2OpenAPI_ParsedArg *new_arr =
                realloc(out->args,
                        (out->n_args + 1) * sizeof(struct C2OpenAPI_ParsedArg));
            if (!new_arr) {
              free(n_str);
              free(t_str);
              rc = ENOMEM;
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
  if (rc != 0) {
    if (out->name)
      free(out->name);
    if (out->args) {
      size_t k;
      for (k = 0; k < out->n_args; k++) {
        free(out->args[k].name);
        free(out->args[k].type);
      }
      free(out->args);
    }
    memset(out, 0, sizeof(*out));
  }
  return rc;
}

static void free_parsed_sig(struct C2OpenAPI_ParsedSig *sig) {
  size_t i;
  if (sig->name)
    free(sig->name);
  if (sig->return_type)
    free(sig->return_type);
  if (sig->args) {
    for (i = 0; i < sig->n_args; ++i) {
      free(sig->args[i].name);
      free(sig->args[i].type);
    }
    free(sig->args);
  }
  memset(sig, 0, sizeof(*sig));
}

static int process_file(const char *path, struct OpenAPI_Spec *spec) {
  char *content = NULL;
  size_t sz = 0;
  struct TokenList *tokens = NULL;
  struct CstNodeList cst = {0};
  int rc;
  size_t i;

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
  if (rc != 0)
    return rc;

  if (tokenize(az_span_create_from_str(content), &tokens) != 0) {
    free(content);
    return EIO;
  }
  parse_tokens(tokens, &cst); /* Best effort */

  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_FUNCTION) {
      struct CstNode *func_node = &cst.nodes[i];
      struct CstNode *doc_node = NULL;

      /* Look backwards for doc comment */
      if (i > 0 && cst.nodes[i - 1].kind == CST_NODE_COMMENT) {
        doc_node = &cst.nodes[i - 1];
      } else if (i > 1 && cst.nodes[i - 1].kind == CST_NODE_WHITESPACE &&
                 cst.nodes[i - 2].kind == CST_NODE_COMMENT) {
        doc_node = &cst.nodes[i - 2];
      }

      if (doc_node) {
        /* Extract comment text */
        char *doc_text = malloc(doc_node->length + 1);
        if (doc_text) {
          struct DocMetadata meta;
          memcpy(doc_text, doc_node->start, doc_node->length);
          doc_text[doc_node->length] = '\0';

          doc_metadata_init(&meta);
          if (doc_parse_block(doc_text, &meta) == 0 && meta.route) {
            /* Found Valid Documented Route! */

            /* Extract Signature Text */
            size_t sig_len =
                func_node->length; /* Approximation, includes body? */
            /* We need signature string up to brace. CST Node includes body. */
            char *sig_raw = malloc(sig_len + 1);
            if (sig_raw) {
              struct C2OpenAPI_ParsedSig psig;
              const uint8_t *brace = memchr(func_node->start, '{', sig_len);
              size_t effective_len =
                  brace ? (size_t)(brace - func_node->start) : sig_len;

              memcpy(sig_raw, func_node->start, effective_len);
              sig_raw[effective_len] = '\0';

              /* Parse Signature */
              if (parse_c_signature_string(sig_raw, &psig) == 0) {
                struct OpenAPI_Operation op = {0};
                struct OpBuilderContext ctx;

                ctx.sig = &psig;
                ctx.doc = &meta;
                ctx.func_name = psig.name;

                /* Build Operation */
                if (c2openapi_build_operation(&ctx, &op) == 0) {
                  /* Aggregate */
                  openapi_aggregator_add_operation(spec, meta.route, &op);
                }
                free_parsed_sig(&psig);
              }
              free(sig_raw);
            }
          }
          doc_metadata_free(&meta);
          free(doc_text);
        }
      }
    }
  }

  free_cst_node_list(&cst);
  free_token_list(tokens);
  free(content);
  return 0;
}

static int walker_cb(const char *path, void *user_data) {
  struct OpenAPI_Spec *spec = (struct OpenAPI_Spec *)user_data;
  if (!is_source_file(path))
    return 0;
  printf("Scanning: %s\n", path);
  process_file(path, spec);
  return 0;
}

int c2openapi_cli_main(int argc, char **argv) {
  struct OpenAPI_Spec spec;
  const char *src_dir;
  const char *out_file;
  char *json = NULL;
  int rc;

  if (argc != 3) {
    fprintf(stderr, "Usage: c2openapi <src_dir> <out.json>\n");
    return EXIT_FAILURE;
  }

  src_dir = argv[1];
  out_file = argv[2];

  openapi_spec_init(&spec);

  /* 1. Walk & Process */
  rc = walk_directory(src_dir, walker_cb, &spec);
  if (rc != 0) {
    fprintf(stderr, "Error walking directory %s: %d\n", src_dir, rc);
    openapi_spec_free(&spec);
    return EXIT_FAILURE;
  }

  /* 2. Write */
  rc = openapi_write_spec_to_json(&spec, &json);
  if (rc != 0 || !json) {
    fprintf(stderr, "Error serializing spec: %d\n", rc);
    openapi_spec_free(&spec);
    return EXIT_FAILURE;
  }

  rc = fs_write_to_file(out_file, json);
  if (rc != 0) {
    fprintf(stderr, "Failed to write %s\n", out_file);
    rc = EXIT_FAILURE;
  } else {
    printf("Written %s\n", out_file);
    rc = 0;
  }

  free(json);
  openapi_spec_free(&spec);

  return (rc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
