/**
 * @file c_inspector.c
 * @brief Implementation of C code inspection logic.
 *
 * Implements scanning of types (using text heuristics) and functions (using
 * Token/CST analysis).
 *
 * Updated to support C23 Enum fixed types and single-line definitions.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_inspector.h"
#include "code2schema.h" /* For parse_struct_member_line */
#include "cst_parser.h"  /* For CST analysis */
#include "str_utils.h"   /* For string duplication and utilities */
#include "tokenizer.h"

/* --- Type Definitions Logic --- */

int type_def_list_init(struct TypeDefList *const list) {
  if (!list)
    return EINVAL;
  list->size = 0;
  list->capacity = 0;
  list->items = NULL;
  return 0;
}

void type_def_list_free(struct TypeDefList *const list) {
  size_t i;
  if (!list)
    return;
  if (list->items) {
    for (i = 0; i < list->size; ++i) {
      if (list->items[i].name)
        free(list->items[i].name);
      if (list->items[i].kind == KIND_ENUM) {
        if (list->items[i].details.enum_members) {
          enum_members_free(list->items[i].details.enum_members);
          free(list->items[i].details.enum_members);
        }
      } else {
        if (list->items[i].details.struct_fields) {
          struct_fields_free(list->items[i].details.struct_fields);
          free(list->items[i].details.struct_fields);
        }
      }
    }
    free(list->items);
    list->items = NULL;
  }
  list->size = 0;
  list->capacity = 0;
}

static int add_type_def(struct TypeDefList *const list,
                        const enum TypeDefinitionKind kind,
                        const char *const name, void *details) {
  struct TypeDefinition *item;

  if (list->size >= list->capacity) {
    size_t new_cap = (list->capacity == 0) ? 8 : list->capacity * 2;
    struct TypeDefinition *new_items = (struct TypeDefinition *)realloc(
        list->items, new_cap * sizeof(struct TypeDefinition));
    if (!new_items)
      return ENOMEM;
    list->items = new_items;
    list->capacity = new_cap;
  }

  item = &list->items[list->size];
  item->kind = kind;
  item->name = c_cdd_strdup(name);
  if (!item->name)
    return ENOMEM;

  if (kind == KIND_ENUM)
    item->details.enum_members = (struct EnumMembers *)details;
  else
    item->details.struct_fields = (struct StructFields *)details;

  list->size++;
  return 0;
}

int c_inspector_scan_file_types(const char *const filename,
                                struct TypeDefList *const out) {
  FILE *fp = NULL;
  char line[2048];
  /* Simple State Machine */
  enum { ST_NONE, ST_ENUM, ST_STRUCT } state = ST_NONE;
  char current_name[64] = {0};

  struct EnumMembers *curr_em = NULL;
  struct StructFields *curr_sf = NULL;
  int rc = 0;

  if (!filename || !out)
    return EINVAL;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (fopen_s(&fp, filename, "r") != 0)
    fp = NULL;
#else
  fp = fopen(filename, "r");
#endif
  if (!fp)
    return errno ? errno : ENOENT;

  while (fgets(line, sizeof(line), fp)) {
    char *p = line;
    char *close_brace = NULL;

    /* Inner loop to handle multiple tokens/state changes on one line */
    while (*p) {
      /* Trim Leading WS */
      while (*p && isspace((unsigned char)*p))
        p++;
      if (!*p)
        break;

      if (state == ST_NONE) {
        if (c_cdd_str_starts_with(p, "enum ") ||
            c_cdd_str_starts_with(p, "struct ")) {
          char *brace = strchr(p, '{');

          if (brace) {
            int is_enum = c_cdd_str_starts_with(p, "enum ");
            /* Extract "Name" from "enum Name {" or "struct Name {" */
            const char *name_start = p + (is_enum ? 5 : 7);
            const char *name_end_ptr = brace;

            /* Handle C23 enum fixed type: enum Name : type { */
            if (is_enum) {
              char *colon = NULL;
              char *scan = (char *)name_start;
              while (scan < brace) {
                if (*scan == ':') {
                  colon = scan;
                  break;
                }
                scan++;
              }
              if (colon) {
                name_end_ptr = colon;
              }
            }

            if (name_end_ptr > name_start) {
              size_t n_len = (size_t)(name_end_ptr - name_start);

              /* Trim leading whitespace from name */
              while (n_len > 0 && isspace((unsigned char)*name_start)) {
                name_start++;
                n_len--;
              }

              /* Trim trailing spaces from name buffer range */
              while (n_len > 0 && isspace((unsigned char)name_start[n_len - 1]))
                n_len--;

              if (n_len > 0 && n_len < sizeof(current_name)) {
                memcpy(current_name, name_start, n_len);
                current_name[n_len] = '\0';
              } else
                current_name[0] = 0;
            } else {
              current_name[0] = 0;
            }

            if (is_enum) {
              curr_em = (struct EnumMembers *)calloc(1, sizeof(*curr_em));
              if (!curr_em || enum_members_init(curr_em) != 0) {
                rc = ENOMEM;
                break;
              }
              state = ST_ENUM;
            } else {
              curr_sf = (struct StructFields *)calloc(1, sizeof(*curr_sf));
              if (!curr_sf || struct_fields_init(curr_sf) != 0) {
                rc = ENOMEM;
                break;
              }
              state = ST_STRUCT;
            }

            /* Advance p to inside the brace */
            p = brace + 1;
            continue;
          } else {
            /* No brace found, possibly forward decl or split line. Skip line.
             */
            break;
          }
        }
        /* Skip unknown content in ST_NONE */
        p++;
      } else {
        /* Inside a definition block */
        close_brace = strchr(p, '}');

        if (close_brace) {
          *close_brace =
              '\0'; /* Temporarily terminate to parse members up to } */
        }

        /* Parse Member string in p (up to terminator) */
        /* For enums on one line: "A, B" */
        if (state == ST_ENUM && curr_em && *p) {
          char *copy = c_cdd_strdup(p);
          if (copy) {
            char *ctx = NULL;
            char *tok = strtok_r(copy, ",", &ctx);
            while (tok) {
              char *eq = strchr(tok, '=');
              if (eq)
                *eq = 0;
              c_cdd_str_trim_trailing_whitespace(tok);
              while (*tok && isspace((unsigned char)*tok))
                tok++;
              if (*tok)
                enum_members_add(curr_em, tok);
              tok = strtok_r(NULL, ",", &ctx);
            }
            free(copy);
          }
        } else if (state == ST_STRUCT && curr_sf) {
          if (*p) {
            /* Support multiple fields on same line separated by semicolon */
            char *copy = c_cdd_strdup(p);
            if (copy) {
              char *ctx = NULL;
              char *tok = strtok_r(copy, ";", &ctx);
              while (tok) {
                /* Ensure not just whitespace */
                char *chk = tok;
                while (*chk && isspace((unsigned char)*chk))
                  chk++;
                if (*chk)
                  parse_struct_member_line(tok, curr_sf);
                tok = strtok_r(NULL, ";", &ctx);
              }
              free(copy);
            }
          }
        }

        if (close_brace) {
          /* Definition Ended */
          if (current_name[0] != '\0') {
            if (state == ST_ENUM) {
              if (add_type_def(out, KIND_ENUM, current_name, curr_em) != 0)
                rc = ENOMEM;
              curr_em = NULL;
            } else {
              if (add_type_def(out, KIND_STRUCT, current_name, curr_sf) != 0)
                rc = ENOMEM;
              curr_sf = NULL;
            }
          } else {
            if (curr_em) {
              enum_members_free(curr_em);
              free(curr_em);
              curr_em = NULL;
            }
            if (curr_sf) {
              struct_fields_free(curr_sf);
              free(curr_sf);
              curr_sf = NULL;
            }
          }
          state = ST_NONE;
          p = close_brace + 1; /* Continue processing after } */
        } else {
          /* No closing brace, consumed rest of line as member */
          break;
        }
      }
    }
    if (rc != 0)
      break;
  }

  /* Cleanup leftovers */
  if (curr_em) {
    enum_members_free(curr_em);
    free(curr_em);
  }
  if (curr_sf) {
    struct_fields_free(curr_sf);
    free(curr_sf);
  }
  if (fp)
    fclose(fp);
  return rc;
}

/* --- Function Signature Logic --- */

int func_sig_list_init(struct FuncSigList *const list) {
  if (!list)
    return EINVAL;
  list->size = 0;
  list->capacity = 0;
  list->items = NULL;
  return 0;
}

void func_sig_list_free(struct FuncSigList *const list) {
  size_t i;
  if (!list)
    return;
  if (list->items) {
    for (i = 0; i < list->size; ++i) {
      free(list->items[i].name);
      free(list->items[i].sig);
    }
    free(list->items);
    list->items = NULL;
  }
  list->size = 0;
  list->capacity = 0;
}

static char *extract_span_text(const struct TokenList *tokens, size_t start,
                               size_t end) {
  size_t total_len = 0;
  size_t i;
  char *buf, *p;

  if (start >= end || !tokens)
    return c_cdd_strdup("");

  for (i = start; i < end; i++)
    total_len += tokens->tokens[i].length;

  buf = (char *)malloc(total_len + 1);
  if (!buf)
    return NULL;

  p = buf;
  for (i = start; i < end; i++) {
    memcpy(p, tokens->tokens[i].start, tokens->tokens[i].length);
    p += tokens->tokens[i].length;
  }
  *p = '\0';
  return buf;
}

int c_inspector_extract_signatures(const char *const source_code,
                                   struct FuncSigList *const out) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  int rc;
  size_t i;

  if (!source_code || !out)
    return EINVAL;

  if ((rc = tokenize(az_span_create_from_str((char *)source_code), &tl)) != 0)
    return rc;
  if ((rc = parse_tokens(tl, &cst)) != 0) {
    free_token_list(tl);
    return rc;
  }

  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_FUNCTION) {
      size_t start_idx = cst.nodes[i].start_token;
      size_t end_idx = cst.nodes[i].end_token;
      size_t sig_end_idx = start_idx;
      size_t name_idx = 0;
      int found_name = 0;

      {
        size_t k = start_idx;
        while (k < end_idx) {
          if (tl->tokens[k].kind == TOKEN_LBRACE) {
            sig_end_idx = k;
            break;
          }
          if (tl->tokens[k].kind == TOKEN_LPAREN && !found_name) {
            size_t n = k;
            while (n > start_idx) {
              n--;
              if (tl->tokens[n].kind == TOKEN_WHITESPACE)
                continue;
              if (tl->tokens[n].kind == TOKEN_IDENTIFIER) {
                name_idx = n;
                found_name = 1;
              }
              break;
            }
          }
          k++;
        }
      }

      if (found_name) {
        if (out->size >= out->capacity) {
          size_t c = (out->capacity == 0) ? 8 : out->capacity * 2;
          struct FuncSignature *new_items = (struct FuncSignature *)realloc(
              out->items, c * sizeof(struct FuncSignature));
          if (!new_items) {
            rc = ENOMEM;
            goto cleanup;
          }
          out->items = new_items;
          out->capacity = c;
        }

        out->items[out->size].name =
            extract_span_text(tl, name_idx, name_idx + 1);
        out->items[out->size].sig =
            extract_span_text(tl, start_idx, sig_end_idx);

        if (!out->items[out->size].name || !out->items[out->size].sig) {
          rc = ENOMEM;
          goto cleanup;
        }
        out->size++;
      }
    }
  }

cleanup:
  free_cst_node_list(&cst);
  free_token_list(tl);
  return rc;
}
