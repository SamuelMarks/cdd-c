/**
 * @file sync_code.c
 * @brief Implementation of code synchronization and header patching.
 * Refactored to ensure strict error checking and resource management.
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parson.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif /* !strdup */
/* strtok_s is provided by MSVC runtime */
#else
/* Standard C doesn't have strtok_s, use strtok_r if available or fallback */
#ifndef strtok_r
#define strtok_r strtok_r
#endif
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

#include "sync_code.h"

#include "c_str_span.h"
#include "code2schema.h" /* For struct/enum list parsing */
#include "codegen.h"     /* For code generation functions */
#include "cst_parser.h"  /* For parsing refactored source */
#include "fs.h"          /* For utilities */
#include "tokenizer.h"

#define CHECK_RC(x)                                                            \
  do {                                                                         \
    const int err_code = (x);                                                  \
    if (err_code != 0)                                                         \
      return err_code;                                                         \
  } while (0)

#if defined(_win32)
#define strtok_r strtok_s
#endif

/* --- Sync Header -> Impl Logic --- */

static int read_line_sync(FILE *fp, char *buf, size_t bufsz) {
  if (!fgets(buf, (int)bufsz, fp))
    return 0;
  {
    const size_t len = strlen(buf);
    if (len && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
      buf[len - 1] = '\0';
  }
  return 1;
}

static void extract_name_sync(char *dest, size_t dest_sz, const char *start,
                              const char *end) {
  const char *name_start = start;
  const char *name_end;
  while (isspace((unsigned char)*name_start))
    name_start++;
  name_end = name_start;
  while (name_end < end && !isspace((unsigned char)*name_end))
    name_end++;
  if (name_end > name_start && (size_t)(name_end - name_start) < dest_sz) {
    memcpy(dest, name_start, name_end - name_start);
    dest[name_end - name_start] = '\0';
  } else {
    dest[0] = '\0';
  }
}

int sync_code_main(int argc, char **argv) {
  const char *header_filename, *impl_filename;
  FILE *fp = NULL, *out = NULL;

  enum { NONE, IN_ENUM, IN_STRUCT } state = NONE;

  char line[512];
  char enum_name[64] = {0};
  struct EnumMembers em;
  char struct_name[64] = {0};
  struct StructFields sf;

  /* Assuming static limits for now as per original design, or can make dynamic
   * list */
  struct EnumMembers enums[64];
  size_t enum_count = 0;
  char enum_names[64][64];

  struct StructFields structs[64];
  char struct_names[64][64];
  size_t struct_count = 0, i;
  int rc = 0;

  if (argc != 2) {
    fputs("Usage: sync_code <header.h> <impl.c>\n", stderr);
    return EXIT_FAILURE;
  }

  header_filename = argv[0];
  impl_filename = argv[1];

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, header_filename, "r, ccs=UTF-8");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to header file %s\n", header_filename);
      return (int)err;
    }
  }
#else
  fp = fopen(header_filename, "r");
  if (!fp) {
    /* fprintf(stderr, "Failed to open header file: %s\n", header_filename); */
    /* Return ENOENT specifically to allow tests to verify error codes without
     * stderr noise if desired */
    return errno ? errno : ENOENT;
  }
#endif

  if ((rc = enum_members_init(&em)) != 0) {
    fclose(fp);
    return rc;
  }
  if ((rc = struct_fields_init(&sf)) != 0) {
    enum_members_free(&em);
    fclose(fp);
    return rc;
  }

  while (read_line_sync(fp, line, sizeof(line))) {
    char *p = line;
  process_line_sync:
    while (isspace((unsigned char)*p))
      p++;
    if (!*p)
      continue;

    if (state == NONE) {
      char *brace = strchr(p, '{');
      char *semi = strchr(p, ';');

      if ((str_starts_with(p, "enum ") || str_starts_with(p, "struct ")) &&
          semi && (!brace || semi < brace)) {
        p = semi + 1;
        goto process_line_sync;
      }

      if ((str_starts_with(p, "enum ") || str_starts_with(p, "struct ")) &&
          brace) {
        if (str_starts_with(p, "enum ")) {
          extract_name_sync(enum_name, sizeof(enum_name), p + 5, brace);
          enum_members_free(&em);
          CHECK_RC(enum_members_init(&em));
          state = IN_ENUM;
        } else { /* struct */
          extract_name_sync(struct_name, sizeof(struct_name), p + 7, brace);
          struct_fields_free(&sf);
          CHECK_RC(struct_fields_init(&sf));
          state = IN_STRUCT;
        }
        p = brace + 1;
        goto process_line_sync;
      }
    }

    if (state == IN_ENUM) {
      char *end_brace = strchr(p, '}');
      char *body_to_parse;
      size_t len;

      if (end_brace) {
        len = end_brace - p;
        body_to_parse = (char *)malloc(len + 1);
        if (!body_to_parse) {
          state = NONE;
          rc = ENOMEM;
          goto cleanup;
        }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
        strncpy_s(body_to_parse, len + 1, p, len);
#else
        strncpy(body_to_parse, p, len);
        body_to_parse[len] = '\0';
#endif
      } else {
        body_to_parse = strdup(p);
        if (!body_to_parse) {
          state = NONE;
          rc = ENOMEM;
          goto cleanup;
        }
      }

      {
        char *context = NULL;
        char *token = strtok_r(body_to_parse, ",", &context);
        while (token) {
          char *member_p = token;
          while (isspace((unsigned char)*member_p))
            member_p++;
          trim_trailing(member_p);
          if (strlen(member_p) > 0) {
            char *eq = strchr(member_p, '=');
            if (eq)
              *eq = '\0';
            trim_trailing(member_p);
            if (strlen(member_p) > 0) {
              rc = enum_members_add(&em, member_p);
              if (rc != 0) {
                free(body_to_parse);
                goto cleanup;
              }
            }
          }
          token = strtok_r(NULL, ",", &context);
        }
      }
      free(body_to_parse);

      if (end_brace) {
        if (enum_count < sizeof(enums) / sizeof(enums[0])) {
          size_t j;
          rc = enum_members_init(&enums[enum_count]);
          if (rc != 0)
            goto cleanup;

          for (j = 0; j < em.size; j++) {
            rc = enum_members_add(&enums[enum_count], em.members[j]);
            if (rc != 0)
              goto cleanup;
          }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
          strncpy_s((char *)enum_names[enum_count],
                    sizeof(enum_names[enum_count]), enum_name, _TRUNCATE);
#else
          strncpy(enum_names[enum_count], enum_name,
                  sizeof(enum_names[enum_count]));
          enum_names[enum_count][sizeof(enum_names[enum_count]) - 1] = '\0';
#endif
          enum_count++;
        }
        state = NONE;
        p = end_brace + 1;
        while (*p && (isspace((unsigned char)*p) || *p == ';'))
          p++;
        if (*p)
          goto process_line_sync;
      }
    } else if (state == IN_STRUCT) {
      char *end_brace = strchr(p, '}');
      char *body_to_parse;
      size_t len;

      if (end_brace) {
        len = end_brace - p;
        body_to_parse = (char *)malloc(len + 1);
        if (!body_to_parse) {
          state = NONE;
          rc = ENOMEM;
          goto cleanup;
        }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
        strncpy_s(body_to_parse, len + 1, p, len);
#else
        strncpy(body_to_parse, p, len);
        body_to_parse[len] = '\0';
#endif
      } else {
        body_to_parse = strdup(p);
        if (!body_to_parse) {
          state = NONE;
          rc = ENOMEM;
          goto cleanup;
        }
      }

      {
        char *context = NULL;
        char *token = strtok_r(body_to_parse, ";", &context);
        while (token) {
          char *field_p = token;
          while (isspace((unsigned char)*field_p))
            field_p++;
          if (strlen(field_p) > 0) {
            rc = parse_struct_member_line(field_p, &sf);
            if (rc < 0) { /* Negative implies memory error from our new
                             semantics? No, parse returns 0 on success/ignored,
                             ENOMEM on fail */
              if (rc != 0) {
                free(body_to_parse);
                goto cleanup;
              }
            } else if (rc > 0) {
              /* Was successfully added */
            }
          }
          token = strtok_r(NULL, ";", &context);
        }
      }
      free(body_to_parse);

      if (end_brace) {
        if (struct_count < sizeof(structs) / sizeof(structs[0])) {
          size_t j;
          rc = struct_fields_init(&structs[struct_count]);
          if (rc != 0)
            goto cleanup;

          for (j = 0; j < sf.size; j++) {
            rc = struct_fields_add(&structs[struct_count], sf.fields[j].name,
                                   sf.fields[j].type, sf.fields[j].ref);
            if (rc != 0)
              goto cleanup;
          }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
          strncpy_s(struct_names[struct_count],
                    sizeof(struct_names[struct_count]), struct_name, _TRUNCATE);
#else
          strncpy(struct_names[struct_count], struct_name,
                  sizeof(struct_names[struct_count]));
          struct_names[struct_count][sizeof(struct_names[struct_count]) - 1] =
              '\0';
#endif
          struct_count++;
        }
        state = NONE;
        p = end_brace + 1;
        while (*p && (isspace((unsigned char)*p) || *p == ';'))
          p++;
        if (*p)
          goto process_line_sync;
      }
    }
  }

  fclose(fp);
  fp = NULL;

  /* Output Phase */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&out, impl_filename, "w, ccs=UTF-8");
    if (err != 0 || out == NULL) {
      fprintf(stderr, "Failed to open impl file %s\n", impl_filename);
      rc = (int)err;
      goto cleanup_memory;
    }
  }
#else
  out = fopen(impl_filename, "w");
  if (!out) {
    fprintf(stderr, "Failed to open impl file: %s\n", impl_filename);
    rc = errno ? errno : EIO;
    goto cleanup_memory;
  }
#endif

  if (fputs(
          "#include <stdlib.h>\n"
          "#include <string.h>\n"
          "#include <stdio.h>\n\n"
          "#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)\n"
          "#else\n"
          "#include <sys/errno.h>\n"
          "#endif\n"
          "#include <parson.h>\n\n",
          out) < 0) {
    rc = EIO;
    goto cleanup;
  }
  {
    char *basename_str = NULL;
    if ((rc = get_basename(header_filename, &basename_str)) != 0)
      goto cleanup;
    if (fprintf(out, "#include \"%s\"\n\n", basename_str) < 0) {
      free(basename_str);
      rc = EIO;
      goto cleanup;
    }
    free(basename_str);
  }

  for (i = 0; i < enum_count; i++) {
    CHECK_RC(write_enum_to_str_func(out, enum_names[i], &enums[i]));
    CHECK_RC(write_enum_from_str_func(out, enum_names[i], &enums[i]));
    enum_members_free(&enums[i]);
  }

  for (i = 0; i < struct_count; i++) {
    CHECK_RC(write_struct_debug_func(out, struct_names[i], &structs[i]));
    CHECK_RC(write_struct_deepcopy_func(out, struct_names[i], &structs[i]));
    CHECK_RC(write_struct_default_func(out, struct_names[i], &structs[i]));
    CHECK_RC(write_struct_display_func(out, struct_names[i], &structs[i]));
    CHECK_RC(write_struct_eq_func(out, struct_names[i], &structs[i]));
    CHECK_RC(
        write_struct_from_jsonObject_func(out, struct_names[i], &structs[i]));
    CHECK_RC(write_struct_from_json_func(out, struct_names[i]));
    CHECK_RC(write_struct_to_json_func(out, struct_names[i], &structs[i]));
    CHECK_RC(write_struct_cleanup_func(out, struct_names[i], &structs[i]));
    struct_fields_free(&structs[i]);
  }

  fclose(out);
  printf("Synchronized implementation file %s from header %s\n", impl_filename,
         header_filename);

  rc = 0;

cleanup:
  if (fp)
    fclose(fp);
  if (out && rc != 0) { /* If error occurred during writing */
    fclose(out);
  }
cleanup_memory:
  enum_members_free(&em);
  struct_fields_free(&sf);
  /* Free any remaining stored structs/enums if error happened mid-loop */
  for (i = 0; i < enum_count; i++)
    enum_members_free(&enums[i]);
  for (i = 0; i < struct_count; i++)
    struct_fields_free(&structs[i]);

  return rc;
}

/* --- Header Patcher Logic --- */

/* Helper for finding match in Tokens */
static int find_token_idx(const struct TokenList *tl, size_t start,
                          enum TokenKind kind) {
  size_t i;
  for (i = start; i < tl->size; i++) {
    if (tl->tokens[i].kind == kind)
      return (int)i;
  }
  return -1;
}

/* Helper to compare token text */
static int token_eq_str(const struct Token *tok, const char *s) {
  size_t len = strlen(s);
  return (tok->length == len && strncmp((const char *)tok->start, s, len) == 0);
}

/* Extract function signature from a function definition node */
/* Returns allocated string: "int func(int a, char *b)" */
static char *extract_signature_string(const struct TokenList *tokens,
                                      size_t start, size_t end) {
  /* find left brace, signature is everything before it */
  size_t i;
  size_t sig_end = end;
  char *sig = NULL;
  int lbrace_idx = find_token_idx(tokens, start, TOKEN_LBRACE);

  if (lbrace_idx >= 0 && (size_t)lbrace_idx < end) {
    sig_end = (size_t)lbrace_idx;
  }

  /* Construct string */
  {
    size_t len = 0;
    for (i = start; i < sig_end; i++)
      len += tokens->tokens[i].length + 1;
    sig = malloc(len + 2);
    if (!sig)
      return NULL;
    sig[0] = 0;

    for (i = start; i < sig_end; i++) {
      size_t tok_len = tokens->tokens[i].length;
      strncat(sig, (char *)tokens->tokens[i].start, tok_len);
    }
    strcat(sig, ";");
  }
  return sig;
}

int patch_header_from_source(const char *header_path,
                             const char *refactored_source) {
  struct TokenList *src_tokens = NULL;
  struct CstNodeList src_cst = {0};
  char *header_content = NULL;
  size_t header_len = 0;
  char *new_header = NULL;
  int rc = 0;

  /* 1. Parse refactored source */
  if ((rc = tokenize(az_span_create_from_str((char *)refactored_source),
                     &src_tokens)) != 0)
    return rc;
  if ((rc = parse_tokens(src_tokens, &src_cst)) != 0) {
    free_token_list(src_tokens);
    return rc;
  }

  /* 2. Read Header */
  if ((rc = read_to_file(header_path, "r", &header_content, &header_len)) !=
      0) {
    free_token_list(src_tokens);
    free_cst_node_list(&src_cst);
    return rc;
  }

  {
    struct TokenList *hdr_tokens = NULL;
    size_t estimated_cap;
    size_t current_len = 0;

    if ((rc = tokenize(az_span_create_from_str(header_content), &hdr_tokens)) !=
        0)
      goto cleanup_hdr_tokens;

    estimated_cap = header_len * 2;
    new_header = malloc(estimated_cap);
    if (!new_header) {
      rc = ENOMEM;
      goto cleanup_hdr_tokens;
    }
    new_header[0] = 0;

    {
      size_t h_idx = 0;
      size_t start_stmt = 0;

      while (h_idx < hdr_tokens->size) {
        size_t semi = h_idx;
        while (semi < hdr_tokens->size &&
               hdr_tokens->tokens[semi].kind != TOKEN_SEMICOLON) {
          semi++;
        }

        if (semi >= hdr_tokens->size) {
          size_t k;
          for (k = h_idx; k < hdr_tokens->size; k++) {
            struct Token *t = &hdr_tokens->tokens[k];
            size_t next_len = current_len + t->length + 1;
            if (next_len >= estimated_cap) {
              estimated_cap *= 2;
              new_header = realloc(new_header, estimated_cap);
            }
            memcpy(new_header + current_len, t->start, t->length);
            current_len += t->length;
            new_header[current_len] = 0;
          }
          break;
        }

        {
          int matched_src = -1;
          size_t k;

          for (k = start_stmt; k < semi; k++) {
            if (hdr_tokens->tokens[k].kind == TOKEN_IDENTIFIER) {
              size_t next = k + 1;
              while (next < semi &&
                     hdr_tokens->tokens[next].kind == TOKEN_WHITESPACE)
                next++;
              if (next < semi &&
                  hdr_tokens->tokens[next].kind == TOKEN_LPAREN) {
                size_t s;
                for (s = 0; s < src_cst.size; s++) {
                  if (src_cst.nodes[s].kind == CST_NODE_FUNCTION) {
                    char *hdr_fname = malloc(hdr_tokens->tokens[k].length + 1);
                    memcpy(hdr_fname, hdr_tokens->tokens[k].start,
                           hdr_tokens->tokens[k].length);
                    hdr_fname[hdr_tokens->tokens[k].length] = 0;

                    /* Check source using simple token matching logic */
                    {
                      size_t st;
                      int balance = 0;
                      for (st = 0; st < src_tokens->size; st++) {
                        if (src_tokens->tokens[st].kind == TOKEN_LPAREN)
                          balance++;
                        else if (src_tokens->tokens[st].kind == TOKEN_RPAREN)
                          balance--;

                        if (balance == 0 &&
                            src_tokens->tokens[st].kind == TOKEN_IDENTIFIER) {
                          if (token_eq_str(&src_tokens->tokens[st],
                                           hdr_fname)) {
                            size_t nx = st + 1;
                            while (nx < src_tokens->size &&
                                   src_tokens->tokens[nx].kind ==
                                       TOKEN_WHITESPACE)
                              nx++;
                            if (nx < src_tokens->size &&
                                src_tokens->tokens[nx].kind == TOKEN_LPAREN) {
                              size_t scan_body = nx;
                              while (scan_body < src_tokens->size &&
                                     src_tokens->tokens[scan_body].kind !=
                                         TOKEN_LBRACE &&
                                     src_tokens->tokens[scan_body].kind !=
                                         TOKEN_SEMICOLON)
                                scan_body++;
                              if (scan_body < src_tokens->size &&
                                  src_tokens->tokens[scan_body].kind ==
                                      TOKEN_LBRACE) {
                                matched_src = (int)st;
                                break;
                              }
                            }
                          }
                        }
                      }
                    }

                    free(hdr_fname);

                    if (matched_src != -1) {
                      size_t sig_start = matched_src;
                      size_t sig_end;
                      char *new_proto;

                      while (sig_start > 0 &&
                             src_tokens->tokens[sig_start - 1].kind !=
                                 TOKEN_RBRACE &&
                             src_tokens->tokens[sig_start - 1].kind !=
                                 TOKEN_SEMICOLON) {
                        sig_start--;
                      }
                      while (sig_start < matched_src &&
                             src_tokens->tokens[sig_start].kind ==
                                 TOKEN_WHITESPACE)
                        sig_start++;

                      sig_end = matched_src;
                      while (sig_end < src_tokens->size &&
                             src_tokens->tokens[sig_end].kind != TOKEN_LBRACE)
                        sig_end++;

                      new_proto = extract_signature_string(src_tokens,
                                                           sig_start, sig_end);
                      if (new_proto) {
                        size_t np_len = strlen(new_proto);
                        while (current_len + np_len + 2 > estimated_cap) {
                          estimated_cap *= 2;
                          new_header = realloc(new_header, estimated_cap);
                        }
                        strcat(new_header, new_proto);
                        strcat(new_header, "\n");
                        current_len += np_len + 1;
                        free(new_proto);

                        start_stmt = semi + 1;
                        h_idx = start_stmt;
                        goto next_stmt;
                      }
                    }
                  }
                  if (matched_src != -1)
                    break;
                }
              }
            }
            if (matched_src != -1)
              break;
          }

          {
            size_t tok_range_idx;
            for (tok_range_idx = start_stmt; tok_range_idx <= semi;
                 tok_range_idx++) {
              struct Token *t = &hdr_tokens->tokens[tok_range_idx];
              while (current_len + t->length + 1 > estimated_cap) {
                estimated_cap *= 2;
                new_header = realloc(new_header, estimated_cap);
              }
              memcpy(new_header + current_len, t->start, t->length);
              current_len += t->length;
              new_header[current_len] = 0;
            }
          }
          start_stmt = semi + 1;
          h_idx = start_stmt;
        }

      next_stmt:;
      }
    }

  cleanup_hdr_tokens:
    free_token_list(hdr_tokens);
  }

  if (rc == 0 && new_header) {
    FILE *fp_out;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__)
    errno_t err = fopen_s(&fp_out, header_path, "w, ccs=UTF-8");
    if (err != 0 || !fp_out)
      rc = EIO;
#else
    fp_out = fopen(header_path, "w");
    if (!fp_out)
      rc = EIO;
#endif
    if (fp_out) {
      fputs(new_header, fp_out);
      fclose(fp_out);
    }
  }

  free(header_content);
  free(new_header);
  free_token_list(src_tokens);
  free_cst_node_list(&src_cst);
  return rc;
}
