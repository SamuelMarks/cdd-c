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
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../win_compat_sym.h"

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "classes/parse/cdd_lexer.h"
#include "classes/parse/cdd_serde.h"
#include "classes/parse/code2schema.h" /* For parse_struct_member_line */
#include "classes/parse/inspector.h"
#include "functions/parse/cst.h" /* For CST analysis */
#include "functions/parse/str.h" /* For string duplication and utilities */
#include "functions/parse/tokenizer.h"
/* clang-format on */

/* --- Type Definitions Logic --- */

/*
 * @brief Initializes a list of discovered C type definitions.
 *
 * @param[out] list Pointer to the TypeDefList structure to initialize.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t type_def_list_init(struct TypeDefList *list) {
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_type_def_list_init;
  if (g_cdd_fail_type_def_list_init && --g_cdd_fail_type_def_list_init == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  if (!list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  list->size = 0;
  list->capacity = 0;
  list->items = NULL;
  return CDD_C_SUCCESS;
}

/*
 * @brief Releases memory held by a C type definition list.
 *
 * @param[in,out] list Pointer to the TypeDefList structure to free.
 */
void type_def_list_free(struct TypeDefList *list) {
  size_t i;
  if (!list)
    return;
  if (list->items) {
    for (i = 0; i < list->size; ++i) {
      if (list->items[i].name)
        C_CDD_FREE(list->items[i].name);
      if (list->items[i].kind == KIND_ENUM) {
        if (list->items[i].details.enum_members) {
          enum_members_free(list->items[i].details.enum_members);
          C_CDD_FREE(list->items[i].details.enum_members);
        }
      } else {
        if (list->items[i].details.struct_fields) {
          struct_fields_free(list->items[i].details.struct_fields);
          C_CDD_FREE(list->items[i].details.struct_fields);
        }
      }
    }
    C_CDD_FREE(list->items);
    list->items = NULL;
  }
  list->size = 0;
  list->capacity = 0;
}

/**
 * @brief Adds or sets type def.
 *
 * @param[in,out] list Target `TypeDefList` to append to.
 * @param[in] kind Type definition kind (`KIND_ENUM` or `KIND_STRUCT`).
 * @param[in] name Type identifier name.
 * @param[in] details Pointer to `EnumMembers` or `StructFields` details.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static cdd_c_error_t add_type_def(struct TypeDefList *list,
                                  const enum TypeDefinitionKind kind,
                                  const char *name, void *details) {
  struct TypeDefinition *item;

  if (list->size >= list->capacity) {
    size_t new_cap = (list->capacity == 0) ? 8 : list->capacity * 2;
    struct TypeDefinition *new_items = (struct TypeDefinition *)C_CDD_REALLOC(
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
  {
    cdd_c_error_t rc_str = c_cdd_strdup(name, &item->name);
    if (rc_str != CDD_C_SUCCESS)
      return rc_str;
  }

  if (kind == KIND_ENUM)
    item->details.enum_members = (struct EnumMembers *)details;
  else
    item->details.struct_fields = (struct StructFields *)details;

  list->size++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if `s` starts with a C block or line comment delimiter.
 *
 * @param[in] s String to inspect.
 * @param[out] out_is_cmt Set to 1 if `s` starts with a comment delimiter, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t is_comment_start(const char *s, int *out_is_cmt) {
  *out_is_cmt = (s[0] == '/' && (s[1] == '*' || s[1] == '/')) ? 1 : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Finds the first occurrence of `target` outside closed inline comments
 * and string literals.
 *
 * @param[in] s Input string to scan.
 * @param[in] target Character to locate (e.g. '{' or '}').
 * @param[out] out Pointer to matching character in `s`, or NULL if not found.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t find_code_char(char *s, char target, char **out) {
  int in_str = 0;
  while (*s) {
    if (!in_str && s[0] == '/' && s[1] == '/') {
      *out = NULL;
      return CDD_C_SUCCESS;
    }
    if (!in_str && s[0] == '/' && s[1] == '*') {
      char *end = strstr(s + 2, "*/");
      if (end) {
        s = end + 2;
        continue;
      }
    }
    if (*s == '"') {
      in_str = !in_str;
    } else if (!in_str && *s == target) {
      *out = s;
      return CDD_C_SUCCESS;
    }
    s++;
  }
  *out = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Splits a mutable string on `delim` while ignoring delimiters inside
 * closed inline comments or string literals.
 *
 * @param[in,out] cursor Pointer to current scan position (updated to next
 * segment or NULL).
 * @param[in] delim Delimiter character (e.g. ';' or ',').
 * @param[out] out_tok Output pointer to token start, or NULL if exhausted.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t next_delim_tok(char **cursor, char delim, char **out_tok) {
  char *s = *cursor;
  char *start;
  int in_str = 0;
  if (!s) {
    *out_tok = NULL;
    return CDD_C_SUCCESS;
  }
  while (*s == delim)
    s++;
  if (!*s) {
    *cursor = NULL;
    *out_tok = NULL;
    return CDD_C_SUCCESS;
  }
  start = s;
  while (*s) {
    if (!in_str && s[0] == '/' && s[1] == '*') {
      char *end = strstr(s + 2, "*/");
      if (end) {
        s = end + 2;
        continue;
      }
    }
    if (*s == '"') {
      in_str = !in_str;
    } else if (!in_str && *s == delim) {
      *s = '\0';
      *cursor = s + 1;
      *out_tok = start;
      return CDD_C_SUCCESS;
    }
    s++;
  }
  *cursor = NULL;
  *out_tok = start;
  return CDD_C_SUCCESS;
}

/*
 * @brief Scans a C source or header file for struct, enum, and typedef
 * declarations.
 *
 * @param[in] filename Path to the C source or header file to scan.
 * @param[in,out] out Target TypeDefList to populate with discovered types.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c_inspector_scan_file_types(const char *filename,
                                          struct TypeDefList *out) {
  FILE *fp = NULL;
  char line[2048];
  char pending_comment[2048];
  size_t pending_len = 0;
  /* Simple State Machine */
  enum { ST_NONE, ST_ENUM, ST_STRUCT } state = ST_NONE;
  char current_name[64] = {0};
  int in_union_sub = 0;
  int in_block_comment = 0;
  int is_typedef = 0;
  int awaiting_brace = 0;
  int pending_is_enum = 0;

  struct EnumMembers *curr_em = NULL;
  struct StructFields *curr_sf = NULL;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!filename || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  pending_comment[0] = '\0';

#if defined(_MSC_VER)
  if (fopen_s(&fp, filename, "r") != 0)
    fp = NULL;
#else
  fp = fopen(filename, "r");
#endif
  if (!fp) {
    if (errno == ENOENT)
      return CDD_C_ERROR_NOT_FOUND;
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
        int is_cmt = 0;
        is_comment_start(p, &is_cmt);
        /* Collect standalone or leading comment lines before struct/enum */
        if (in_block_comment || is_cmt) {
          int is_blk = in_block_comment || (p[1] == '*');
          char *c_end =
              is_blk ? strstr(in_block_comment ? p : (p + 2), "*/") : NULL;
          size_t clen = c_end ? (size_t)((c_end + 2) - p) : strlen(p);
          size_t k;
          for (k = 0; k < clen && pending_len + 1 < sizeof(pending_comment);
               ++k) {
            pending_comment[pending_len++] = p[k];
          }
          if (pending_len + 1 < sizeof(pending_comment))
            pending_comment[pending_len++] = '\n';
          pending_comment[pending_len] = '\0';
          if (c_end) {
            in_block_comment = 0;
            p = c_end + 2;
            continue;
          } else {
            if (is_blk)
              in_block_comment = 1;
            break;
          }
        }
        if (awaiting_brace && *p == '{') {
          awaiting_brace = 0;
          starts1 = pending_is_enum;
          starts2 = !pending_is_enum;
        } else {
          awaiting_brace = 0;
          if (strncmp(p, "typedef ", 8) == 0) {
            is_typedef = 1;
            p += 8;
          }
          {
            cdd_c_error_t rc_str = c_cdd_str_starts_with(p, "enum ", &starts1);
            if (rc_str != CDD_C_SUCCESS) {
              rc = rc_str;
              break;
            }
          }
          {
            cdd_c_error_t rc_str =
                c_cdd_str_starts_with(p, "struct ", &starts2);
            if (rc_str != CDD_C_SUCCESS) {
              rc = rc_str;
              break;
            }
          }
          if (!starts1 && strncmp(p, "enum{", 5) == 0)
            starts1 = 1;
          if (!starts2 && strncmp(p, "struct{", 7) == 0)
            starts2 = 1;
        }
        if (starts1 || starts2) {
          char *brace = NULL;
          int is_enum = starts1;
          find_code_char(p, '{', &brace);

          if (*p != '{') {
            const char *name_start =
                p + (is_enum ? (p[4] == '{' ? 4 : 5) : (p[6] == '{' ? 6 : 7));
            const char *name_end_ptr =
                brace ? (const char *)brace : (p + strlen(p));

            /* Handle C23 enum fixed type or C++ inheritance: [enum|struct] Name
             * : ... { */
            {
              const char *colon = NULL;
              const char *scan = name_start;
              while (scan < name_end_ptr) {
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

              memcpy(current_name, name_start, n_len);
              current_name[n_len] = '\0';
            } else {
              current_name[0] = 0;
            }
          }

          if (brace) {
            if (is_enum) {
              curr_em = (struct EnumMembers *)C_CDD_CALLOC(1, sizeof(*curr_em));
              if (!curr_em || enum_members_init(curr_em) != 0) {
                rc = CDD_C_ERROR_MEMORY;
                break;
              }
              if (pending_comment[0] != '\0') {
                rc = cdd_serde_parse_enum_comment(pending_comment, curr_em);
                pending_comment[0] = '\0';
                pending_len = 0;
                if (rc != CDD_C_SUCCESS)
                  break;
              }
              state = ST_ENUM;
            } else {
              curr_sf =
                  (struct StructFields *)C_CDD_CALLOC(1, sizeof(*curr_sf));
              if (!curr_sf || struct_fields_init(curr_sf) != 0) {
                rc = CDD_C_ERROR_MEMORY;
                break;
              }
              if (pending_comment[0] != '\0') {
                rc =
                    cdd_serde_parse_container_comment(pending_comment, curr_sf);
                pending_comment[0] = '\0';
                pending_len = 0;
                if (rc != CDD_C_SUCCESS)
                  break;
              }
              state = ST_STRUCT;
            }

            /* Advance p to inside the brace */
            p = brace + 1;
            continue;
          } else {
            /* If no semicolon on line, opening brace may be on the next line */
            if (strchr(p, ';') == NULL) {
              awaiting_brace = 1;
              pending_is_enum = is_enum;
            } else {
              pending_comment[0] = '\0';
              pending_len = 0;
              is_typedef = 0;
            }
            break;
          }
        }
        /* Skip unknown content in ST_NONE */
        if (*p == ';') {
          pending_comment[0] = '\0';
          pending_len = 0;
          is_typedef = 0;
        }
        p++;
      } else {
        /* Inside a definition block */
        find_code_char(p, '}', &close_brace);

        if (close_brace && state == ST_STRUCT && in_union_sub) {
          /* Closing `}` of nested `union { ... } data;` inside a tagged union
           * struct */
          in_union_sub = 0;
          pending_comment[0] = '\0';
          pending_len = 0;
          p = close_brace + 1;
          while (*p && (*p != ';' && *p != '}'))
            p++;
          if (*p == ';')
            p++;
          continue;
        }

        if (close_brace) {
          *close_brace =
              '\0'; /* Temporarily terminate to parse members up to } */
        }

        /* Parse Member string in p (up to terminator) */
        /* For enums on one line: "A, B" */
        if (state == ST_ENUM && *p) {
          int is_cmt = 0;
          is_comment_start(p, &is_cmt);
          if (is_cmt) {
            size_t clen = strlen(p);
            size_t k;
            for (k = 0; k < clen && pending_len + 1 < sizeof(pending_comment);
                 ++k) {
              pending_comment[pending_len++] = p[k];
            }
            if (pending_len + 1 < sizeof(pending_comment))
              pending_comment[pending_len++] = '\n';
            pending_comment[pending_len] = '\0';
          } else {
            char *copy = NULL;
            {
              cdd_c_error_t rc_str = c_cdd_strdup(p, &copy);
              if (rc_str != CDD_C_SUCCESS) {
                rc = rc_str;
                break;
              }
            }
            {
              char *ctx = copy;
              char *tok = NULL;
              next_delim_tok(&ctx, ',', &tok);
              while (tok) {
                char var_cmt[512];
                size_t vc_pos = 0;
                char var_rename[64] = {0};
                int var_skip = 0;
                int var_other = 0;
                enum CddSerdeCaseRule var_case = CDD_SERDE_CASE_NONE;
                char var_rep_from[32] = {0};
                char var_rep_to[32] = {0};
                char *c_start;
                char *eq;

                var_cmt[0] = '\0';
                if (pending_comment[0] != '\0') {
                  size_t k;
                  for (k = 0; pending_comment[k] != '\0' &&
                              vc_pos + 1 < sizeof(var_cmt);
                       ++k) {
                    var_cmt[vc_pos++] = pending_comment[k];
                  }
                  var_cmt[vc_pos] = '\0';
                  pending_comment[0] = '\0';
                  pending_len = 0;
                }
                while ((c_start = strstr(tok, "/*")) != NULL) {
                  char *c_end = strstr(c_start + 2, "*/");
                  size_t clen =
                      c_end ? (size_t)((c_end + 2) - c_start) : strlen(c_start);
                  size_t k;
                  for (k = 0; k < clen && vc_pos + 1 < sizeof(var_cmt); ++k)
                    var_cmt[vc_pos++] = c_start[k];
                  var_cmt[vc_pos] = '\0';
                  if (c_end)
                    memmove(c_start, c_end + 2, strlen(c_end + 2) + 1);
                  else
                    *c_start = '\0';
                }
                eq = strchr(tok, '=');
                if (eq)
                  *eq = 0;
                c_cdd_str_trim_trailing_whitespace(tok);
                while (*tok && isspace((unsigned char)*tok))
                  tok++;
                if (*tok) {
                  char effective_rename[128] = {0};
                  const char *member_tok = tok;
                  size_t ename_len = strlen(current_name);
                  cdd_c_error_t rc_str;
                  if (strncmp(tok, current_name, ename_len) == 0 &&
                      tok[ename_len] == '_' && tok[ename_len + 1] != '\0') {
                    member_tok = tok + ename_len + 1;
                  }
                  if (var_cmt[0] != '\0') {
                    rc_str = cdd_serde_parse_enum_variant_comment_ex(
                        var_cmt, var_rename, sizeof(var_rename), &var_skip,
                        &var_case, var_rep_from, sizeof(var_rep_from),
                        var_rep_to, sizeof(var_rep_to), &var_other, NULL, 0);
                    if (rc_str != CDD_C_SUCCESS) {
                      rc = rc_str;
                      C_CDD_FREE(copy);
                      copy = NULL;
                      break;
                    }
                  }
                  if (var_rename[0] != '\0') {
                    CDD_STRCPY(effective_rename, sizeof(effective_rename),
                               var_rename);
                  } else if (var_case != CDD_SERDE_CASE_NONE ||
                             var_rep_from[0] != '\0') {
                    char tmp_v[128];
                    CDD_STRCPY(tmp_v, sizeof(tmp_v), member_tok);
                    if (var_case != CDD_SERDE_CASE_NONE) {
                      cdd_serde_transform_case(tmp_v, var_case,
                                               effective_rename,
                                               sizeof(effective_rename));
                      CDD_STRCPY(tmp_v, sizeof(tmp_v), effective_rename);
                    }
                    if (var_rep_from[0] != '\0') {
                      cdd_serde_replace_str(tmp_v, var_rep_from, var_rep_to,
                                            effective_rename,
                                            sizeof(effective_rename));
                    }
                  }
                  rc_str = enum_members_add_ex(
                      curr_em, member_tok,
                      effective_rename[0] ? effective_rename : NULL, var_skip);
                  if (rc_str != CDD_C_SUCCESS) {
                    rc = rc_str;
                    C_CDD_FREE(copy);
                    copy = NULL;
                    break;
                  }
                  if (var_other) {
                    curr_em->serde_other_idx = (int)(curr_em->size - 1);
                  }
                  if (var_cmt[0] != '\0') {
                    rc_str = cdd_serde_parse_enum_variant_comment_ex(
                        var_cmt, var_rename, sizeof(var_rename), &var_skip,
                        &var_case, var_rep_from, sizeof(var_rep_from),
                        var_rep_to, sizeof(var_rep_to), NULL, curr_em,
                        curr_em->size - 1);
                    if (rc_str != CDD_C_SUCCESS) {
                      rc = rc_str;
                      C_CDD_FREE(copy);
                      copy = NULL;
                      break;
                    }
                  }
                }
                next_delim_tok(&ctx, ',', &tok);
              }
              if (copy) {
                C_CDD_FREE(copy);
                copy = NULL;
              }
              if (rc != CDD_C_SUCCESS)
                break;
            }
          }
        } else if (state == ST_STRUCT) {
          if (*p) {
            int is_cmt = 0;
            /* Check for nested `union {` inside tagged union struct */
            if (strncmp(p, "union", 5) == 0 &&
                (isspace((unsigned char)p[5]) || p[5] == '{') &&
                strchr(p, '{') != NULL) {
              char *ubrace = strchr(p, '{');
              in_union_sub = 1;
              curr_sf->is_union = 1;
              /* Remove preceding `enum <Name>_tag tag;` field if present */
              if (curr_sf->size > 0 &&
                  strcmp(curr_sf->fields[curr_sf->size - 1].name, "tag") == 0) {
                curr_sf->size--;
              }
              p = ubrace + 1;
              if (close_brace) {
                *close_brace = '}';
                close_brace = NULL;
              }
              continue;
            }
            /* If the line is purely a comment, accumulate into
             * pending_comment for the next field */
            is_comment_start(p, &is_cmt);
            if (is_cmt) {
              size_t clen = strlen(p);
              size_t k;
              for (k = 0; k < clen && pending_len + 1 < sizeof(pending_comment);
                   ++k) {
                pending_comment[pending_len++] = p[k];
              }
              if (pending_len + 1 < sizeof(pending_comment))
                pending_comment[pending_len++] = '\n';
              pending_comment[pending_len] = '\0';
            } else {
              /* Support multiple fields on same line separated by semicolon,
               * while attaching trailing comments on the same line to the
               * preceding field */
              char *copy = NULL;
              {
                cdd_c_error_t rc_str = c_cdd_strdup(p, &copy);
                if (rc_str != CDD_C_SUCCESS) {
                  rc = rc_str;
                  break;
                }
              }
              {
                char *ctx = copy;
                char *tok = NULL;
                next_delim_tok(&ctx, ';', &tok);
                while (tok) {
                  /* Ensure not just whitespace */
                  char *chk = tok;
                  while (*chk && isspace((unsigned char)*chk))
                    chk++;
                  if (*chk) {
                    if (chk[0] == '/' && (chk[1] == '*' || chk[1] == '/')) {
                      /* Trailing comment after ';' on same line -> apply to
                       * most recently added field if any */
                      if (curr_sf->size > 0) {
                        cdd_c_error_t rc_sc = cdd_serde_parse_field_comment(
                            chk, &curr_sf->fields[curr_sf->size - 1]);
                        if (rc_sc != CDD_C_SUCCESS) {
                          rc = rc_sc;
                          C_CDD_FREE(copy);
                          copy = NULL;
                          break;
                        }
                      }
                    } else {
                      size_t prev_sz = curr_sf->size;
                      cdd_c_error_t rc_str =
                          parse_struct_member_line(tok, curr_sf);
                      if (rc_str != CDD_C_SUCCESS) {
                        rc = rc_str;
                        C_CDD_FREE(copy);
                        copy = NULL;
                        break;
                      }
                      if (curr_sf->size > prev_sz &&
                          pending_comment[0] != '\0') {
                        rc_str = cdd_serde_parse_field_comment(
                            pending_comment,
                            &curr_sf->fields[curr_sf->size - 1]);
                        pending_comment[0] = '\0';
                        pending_len = 0;
                        if (rc_str != CDD_C_SUCCESS) {
                          rc = rc_str;
                          C_CDD_FREE(copy);
                          copy = NULL;
                          break;
                        }
                      }
                    }
                  }
                  next_delim_tok(&ctx, ';', &tok);
                }
                if (copy) {
                  C_CDD_FREE(copy);
                  copy = NULL;
                }
                if (rc != CDD_C_SUCCESS)
                  break;
              }
            }
          }
        }

        if (close_brace) {
          pending_comment[0] = '\0';
          pending_len = 0;
          in_union_sub = 0;
          if (is_typedef) {
            const char *ap = close_brace + 1;
            size_t alen = 0;
            while (isspace((unsigned char)*ap))
              ap++;
            while (isalnum((unsigned char)ap[alen]) || ap[alen] == '_')
              alen++;
            memcpy(current_name, ap, alen);
            current_name[alen] = '\0';
            is_typedef = 0;
          }
          /* Definition Ended */
          if (current_name[0] != '\0') {
            if (state == ST_ENUM) {
              if (add_type_def(out, KIND_ENUM, current_name, curr_em) != 0) {
                enum_members_free(curr_em);
                C_CDD_FREE(curr_em);
                rc = CDD_C_ERROR_MEMORY;
              }
              curr_em = NULL;
            } else {
              c2s_collapse_arrays(curr_sf);
              if (add_type_def(out, KIND_STRUCT, current_name, curr_sf) != 0) {
                struct_fields_free(curr_sf);
                C_CDD_FREE(curr_sf);
                rc = CDD_C_ERROR_MEMORY;
              }
              curr_sf = NULL;
            }
          } else {
            if (curr_em) {
              enum_members_free(curr_em);
              C_CDD_FREE(curr_em);
              curr_em = NULL;
            }
            if (curr_sf) {
              struct_fields_free(curr_sf);
              C_CDD_FREE(curr_sf);
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
    if (rc != CDD_C_SUCCESS)
      break;
  }

  /* Cleanup leftovers */
  if (curr_em) {
    enum_members_free(curr_em);
    C_CDD_FREE(curr_em);
  }
  if (curr_sf) {
    struct_fields_free(curr_sf);
    C_CDD_FREE(curr_sf);
  }
  if (rc != CDD_C_SUCCESS) {
    type_def_list_free(out);
  }
  fclose(fp);
  return rc;
}

/* --- Function Signature Logic --- */

/*
 * @brief Initializes a list of extracted C function signatures.
 *
 * @param[out] list Pointer to the FuncSigList structure to initialize.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t func_sig_list_init(struct FuncSigList *list) {
  if (!list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  list->size = 0;
  list->capacity = 0;
  list->items = NULL;
  return CDD_C_SUCCESS;
}

/*
 * @brief Releases memory held by a C function signature list.
 *
 * @param[in,out] list Pointer to the FuncSigList structure to free.
 */
void func_sig_list_free(struct FuncSigList *list) {
  size_t i;
  if (!list)
    return;
  if (list->items) {
    for (i = 0; i < list->size; ++i) {
      C_CDD_FREE(list->items[i].name);
      C_CDD_FREE(list->items[i].sig);
      if (list->items[i].doc)
        C_CDD_FREE(list->items[i].doc);
    }
    C_CDD_FREE(list->items);
    list->items = NULL;
  }
  list->size = 0;
  list->capacity = 0;
}

/**
 * @brief Extracts span text.
 *
 * @param[in] tokens Token list containing the span.
 * @param[in] start Inclusive start token index.
 * @param[in] end Exclusive end token index.
 * @param[out] _out_val Pointer to receive the allocated concatenated text.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t extract_span_text(const struct TokenList *tokens,
                                       size_t start, size_t end,
                                       char **_out_val) {
  size_t total_len = 0;
  size_t i;
  char *buf, *p;

  for (i = start; i < end; i++)
    total_len += tokens->tokens[i].length;

  buf = (char *)C_CDD_MALLOC(total_len + 1);
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
 * @brief Copies a token span into a fixed-size buffer with leading/trailing
 * whitespace trimmed.
 *
 * @param[in] tokens Token list containing the span.
 * @param[in] start Inclusive start token index.
 * @param[in] end Exclusive end token index.
 * @param[out] dst Destination fixed-size buffer.
 * @param[in] dst_sz Capacity of `dst`.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t copy_token_span_trimmed(const struct TokenList *tokens,
                                             size_t start, size_t end,
                                             char *dst, size_t dst_sz) {
  size_t pos = 0;
  size_t i;
  (void)dst_sz;
  dst[0] = '\0';
  while (tokens->tokens[start].kind == TOKEN_WHITESPACE) {
    start++;
  }
  while (tokens->tokens[end - 1].kind == TOKEN_WHITESPACE) {
    end--;
  }
  for (i = start; i < end; ++i) {
    size_t k;
    for (k = 0; k < tokens->tokens[i].length; ++k) {
      dst[pos++] = (char)tokens->tokens[i].start[k];
    }
  }
  dst[pos] = '\0';
  c_cdd_str_trim_trailing_whitespace(dst);
  return CDD_C_SUCCESS;
}

/*
 * @brief Extracts C function signatures and docstrings from a source file.
 *
 * @param[in] source_code Null-terminated C source code string.
 * @param[in,out] out Target FuncSigList to populate with extracted function
 * signatures.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c_inspector_extract_signatures(const char *source_code,
                                             struct FuncSigList *out) {
  struct TokenList *tl = NULL;
  struct CstNodeList cst = {0};
  cdd_c_error_t rc;
  size_t i;

  if (!source_code || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = tokenize(az_span_create_from_str(source_code), &tl);
  if (rc == CDD_C_SUCCESS) {
    rc = parse_tokens(tl, &cst);
  }
  if (rc != CDD_C_SUCCESS) {
    if (tl)
      free_token_list(tl);
    return rc;
  }

  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_FUNCTION) {
      size_t start_idx = cst.nodes[i].start_token;
      size_t sig_end_idx = start_idx;
      size_t name_idx = 0;
      size_t lparen_idx = 0;
      int found_name = 0;
      int is_variadic = 0;

      {
        size_t k = start_idx;
        for (;;) {
          if (tl->tokens[k].kind == TOKEN_LBRACE) {
            sig_end_idx = k;
            break;
          }
          if (tl->tokens[k].kind == TOKEN_ELLIPSIS) {
            is_variadic = 1;
          }
          if (tl->tokens[k].kind == TOKEN_LPAREN && !found_name) {
            size_t n = k;
            lparen_idx = k;
            do {
              n--;
            } while (n > start_idx && tl->tokens[n].kind == TOKEN_WHITESPACE);

            if (tl->tokens[n].kind == TOKEN_IDENTIFIER) {
              name_idx = n;
              found_name = 1;
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
          struct FuncSignature *new_items =
              (struct FuncSignature *)C_CDD_REALLOC(
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
          if (out->items[out->size].name)
            C_CDD_FREE(out->items[out->size].name);
          if (out->items[out->size].sig)
            C_CDD_FREE(out->items[out->size].sig);
          if (out->items[out->size].doc)
            C_CDD_FREE(out->items[out->size].doc);
          rc = CDD_C_ERROR_MEMORY;
          goto cleanup;
        }
        memset(out->items[out->size].return_type, 0,
               sizeof(out->items[out->size].return_type));
        memset(out->items[out->size].params, 0,
               sizeof(out->items[out->size].params));
        out->items[out->size].n_params = 0;
        copy_token_span_trimmed(tl, start_idx, name_idx,
                                out->items[out->size].return_type,
                                sizeof(out->items[out->size].return_type));
        {
          size_t p_start = lparen_idx + 1;
          int depth = 1;
          size_t cur = lparen_idx + 1;
          while (depth > 0) {
            enum TokenKind tk = tl->tokens[cur].kind;
            if (tk == TOKEN_LPAREN) {
              depth++;
            } else if (tk == TOKEN_RPAREN) {
              depth--;
            }
            if ((depth == 1 && tk == TOKEN_COMMA) ||
                (depth == 0 && tk == TOKEN_RPAREN)) {
              size_t p_end = cur;
              size_t last_id = p_end;
              size_t scan_p;
              for (scan_p = p_start; scan_p < p_end; ++scan_p) {
                if (tl->tokens[scan_p].kind == TOKEN_IDENTIFIER)
                  last_id = scan_p;
              }
              if (last_id < p_end) {
                size_t pidx = out->items[out->size].n_params;
                copy_token_span_trimmed(
                    tl, p_start, last_id,
                    out->items[out->size].params[pidx].type,
                    sizeof(out->items[out->size].params[pidx].type));
                copy_token_span_trimmed(
                    tl, last_id, last_id + 1,
                    out->items[out->size].params[pidx].name,
                    sizeof(out->items[out->size].params[pidx].name));
                out->items[out->size].n_params++;
              }
              p_start = cur + 1;
            }
            cur++;
          }
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
