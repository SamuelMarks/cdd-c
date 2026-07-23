/**
 * @file inspector.c
 * @brief Implementation of C code inspection logic.
 *
 * Implements scanning of types (using text heuristics) and functions (using
 * Token/CST analysis).
 *
 * Updated to support C23 Enum fixed types and single-line definitions.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../win_compat_sym.h"

#include "classes/parse/code2schema.h" /* For parse_struct_member_line */
#include "classes/parse/inspector.h"
#include "functions/parse/cst.h" /* For CST analysis */
#include "functions/parse/str.h" /* For string duplication and utilities */
#include "functions/parse/tokenizer.h"
#include "c_cdd/log.h"
/* clang-format on */

/* --- Type Definitions Logic --- */

/**
 * @brief Executes the type def list init operation.
 */
enum cdd_c_error type_def_list_init(struct TypeDefList *list) {
  if (!list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  list->size = 0;
  list->capacity = 0;
  list->items = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the type def list free operation.
 */
void type_def_list_free(struct TypeDefList *list) {
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

/**
 * @brief Adds or sets type def.
 */
static enum cdd_c_error add_type_def(struct TypeDefList *list,
                                     const enum TypeDefinitionKind kind,
                                     const char *name, void *details) {
  struct TypeDefinition *item;

  if (list->size >= list->capacity) {
    size_t new_cap = (list->capacity == 0) ? 8 : list->capacity * 2;
    struct TypeDefinition *new_items = (struct TypeDefinition *)realloc(
        list->items, new_cap * sizeof(struct TypeDefinition));
    if (!new_items) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    list->items = new_items;
    list->capacity = new_cap;
  }

  item = &list->items[list->size];
  item->kind = kind;
  c_cdd_strdup(name, &item->name);
  if (!item->name) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  if (kind == KIND_ENUM)
    item->details.enum_members = (struct EnumMembers *)details;
  else
    item->details.struct_fields = (struct StructFields *)details;

  list->size++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the c inspector scan file types operation.
 */
enum cdd_c_error c_inspector_scan_file_types(const char *filename,
                                             struct TypeDefList *out) {
  FILE *fp = NULL;
  char line[2048];
  /* Simple State Machine */
  enum { ST_NONE, ST_ENUM, ST_STRUCT } state = ST_NONE;
  char current_name[64] = {0};

  struct EnumMembers *curr_em = NULL;
  struct StructFields *curr_sf = NULL;
  int rc = 0;

  if (!filename || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#if defined(_MSC_VER)
  if (fopen_s(&fp, filename, "r") != 0)
    fp = NULL;
#else
#if defined(_MSC_VER)
  fopen_s(&fp, filename, "r");
#else
  fp = fopen(filename, "r");
#endif
#endif
  if (!fp) {
    if (errno == ENOENT)
      return CDD_C_ERROR_NOT_FOUND;
    if (errno == ENOMEM)
      return CDD_C_ERROR_MEMORY;
    if (errno == EINVAL)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    return CDD_C_ERROR_IO;
  }

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
        int starts1 = false, starts2 = false;
        c_cdd_str_starts_with(p, "enum ", &starts1);
        c_cdd_str_starts_with(p, "struct ", &starts2);
        if (starts1 || starts2) {
          char *brace = strchr(p, '{');

          if (brace) {
            int is_enum = starts1;
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
                rc = CDD_C_ERROR_MEMORY;
                break;
              }
              state = ST_ENUM;
            } else {
              curr_sf = (struct StructFields *)calloc(1, sizeof(*curr_sf));
              if (!curr_sf || struct_fields_init(curr_sf) != 0) {
                rc = CDD_C_ERROR_MEMORY;
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
          char *copy = NULL;
          c_cdd_strdup(p, &copy);
          if (copy) {
            char *ctx = NULL;
#ifdef _WIN32
            char *tok = strtok_s(copy, ",", &ctx);
#else
            char *tok = strtok_r(copy, ",", &ctx);
#endif
            while (tok) {
              char *eq = strchr(tok, '=');
              if (eq)
                *eq = 0;
              c_cdd_str_trim_trailing_whitespace(tok);
              while (*tok && isspace((unsigned char)*tok))
                tok++;
              if (*tok)
                enum_members_add(curr_em, tok);
#ifdef _WIN32
              tok = strtok_s(NULL, ",", &ctx);
#else
              tok = strtok_r(NULL, ",", &ctx);
#endif
            }
            free(copy);
          }
        } else if (state == ST_STRUCT && curr_sf) {
          if (*p) {
            /* Support multiple fields on same line separated by semicolon */
            char *copy = NULL;
            c_cdd_strdup(p, &copy);
            if (copy) {
              char *ctx = NULL;
#ifdef _WIN32
              char *tok = strtok_s(copy, ";", &ctx);
#else
              char *tok = strtok_r(copy, ";", &ctx);
#endif
              while (tok) {
                /* Ensure not just whitespace */
                char *chk = tok;
                while (*chk && isspace((unsigned char)*chk))
                  chk++;
                if (*chk)
                  parse_struct_member_line(tok, curr_sf);
#ifdef _WIN32
                tok = strtok_s(NULL, ";", &ctx);
#else
                tok = strtok_r(NULL, ";", &ctx);
#endif
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
                rc = CDD_C_ERROR_MEMORY;
              curr_em = NULL;
            } else {
              if (add_type_def(out, KIND_STRUCT, current_name, curr_sf) != 0)
                rc = CDD_C_ERROR_MEMORY;
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

/**
 * @brief Executes the func sig list init operation.
 */
enum cdd_c_error func_sig_list_init(struct FuncSigList *list) {
  if (!list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  list->size = 0;
  list->capacity = 0;
  list->items = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the func sig list free operation.
 */
void func_sig_list_free(struct FuncSigList *list) {
  size_t i;
  if (!list)
    return;
  if (list->items) {
    for (i = 0; i < list->size; ++i) {
      if (list->items[i].name)
        free(list->items[i].name);
      if (list->items[i].sig)
        free(list->items[i].sig);
      if (list->items[i].doc)
        free(list->items[i].doc);
    }
    free(list->items);
    list->items = NULL;
  }
  list->size = 0;
  list->capacity = 0;
}

/**
 * @brief Extracts span text.
 */
static enum cdd_c_error extract_span_text(const struct TokenList *tokens,
                                          size_t start, size_t end,
                                          char **_out_val) {
  size_t total_len = 0;
  size_t i;
  char *buf, *p;

  if (start >= end || !tokens) {
    c_cdd_strdup("", _out_val);
    return CDD_C_SUCCESS;
  }

  for (i = start; i < end; i++)
    total_len += tokens->tokens[i].length;

  buf = (char *)malloc(total_len + 1);
  if (!buf) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  p = buf;
  for (i = start; i < end; i++) {
    memcpy(p, tokens->tokens[i].start, tokens->tokens[i].length);
    p += tokens->tokens[i].length;
  }
  *p = '\0';
  {
    *_out_val = buf;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the c inspector extract signatures operation.
 */
enum cdd_c_error c_inspector_extract_signatures(const char *source_code,
                                                struct FuncSigList *out) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  int rc;
  size_t i;

  if (!source_code || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

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
      int is_variadic = 0;

      {
        size_t k = start_idx;
        while (k < end_idx) {
          if (tl->tokens[k].kind == TOKEN_LBRACE) {
            sig_end_idx = k;
            break;
          }
          if (tl->tokens[k].kind == TOKEN_ELLIPSIS) {
            is_variadic = 1;
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
        size_t comment_idx = start_idx;
        int found_comment = 0;
        while (comment_idx > 0) {
          comment_idx--;
          if (tl->tokens[comment_idx].kind == TOKEN_COMMENT) {
            found_comment = 1;
            break;
          } else if (tl->tokens[comment_idx].kind != TOKEN_WHITESPACE) {
            break;
          }
        }

        if (out->size >= out->capacity) {
          size_t c = (out->capacity == 0) ? 8 : out->capacity * 2;
          struct FuncSignature *new_items = (struct FuncSignature *)realloc(
              out->items, c * sizeof(struct FuncSignature));
          if (!new_items) {
            rc = CDD_C_ERROR_MEMORY;
            goto cleanup;
          }
          out->items = new_items;
          out->capacity = c;
        }

        out->items[out->size].name = NULL;
        extract_span_text(tl, name_idx, name_idx + 1,
                          &out->items[out->size].name);
        out->items[out->size].sig = NULL;
        extract_span_text(tl, start_idx, sig_end_idx,
                          &out->items[out->size].sig);
        out->items[out->size].doc = NULL;
        if (found_comment) {
          extract_span_text(tl, comment_idx, comment_idx + 1,
                            &out->items[out->size].doc);
        }
        out->items[out->size].is_variadic = is_variadic;

        if (!out->items[out->size].name || !out->items[out->size].sig) {
          rc = CDD_C_ERROR_MEMORY;
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
