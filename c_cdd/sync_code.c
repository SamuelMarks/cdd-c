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

#if defined(_BSD_SOURCE) || defined(_GNU_SOURCE) || defined(HAVE_ASPRINTF)
#include <stdio.h>
#else
#include <c89stringutils_string_extras.h>
#endif

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

  /* Codegen config default */
  struct CodegenConfig config;
  memset(&config, 0, sizeof(config));

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
            if (rc < 0) {
              if (rc != 0) {
                free(body_to_parse);
                goto cleanup;
              }
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
                                   sf.fields[j].type, sf.fields[j].ref, NULL);
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

  /*
   * Update calls to include config parameter (NULL for default behavior).
   */
  for (i = 0; i < enum_count; i++) {
    CHECK_RC(write_enum_to_str_func(out, enum_names[i], &enums[i], &config));
    CHECK_RC(write_enum_from_str_func(out, enum_names[i], &enums[i], &config));
    enum_members_free(&enums[i]);
  }

  for (i = 0; i < struct_count; i++) {
    CHECK_RC(
        write_struct_debug_func(out, struct_names[i], &structs[i], &config));
    CHECK_RC(
        write_struct_deepcopy_func(out, struct_names[i], &structs[i], &config));
    CHECK_RC(
        write_struct_default_func(out, struct_names[i], &structs[i], &config));
    CHECK_RC(
        write_struct_display_func(out, struct_names[i], &structs[i], &config));
    CHECK_RC(write_struct_eq_func(out, struct_names[i], &structs[i], &config));
    /* Pass NULL config to new function signatures */
    CHECK_RC(write_struct_from_jsonObject_func(out, struct_names[i],
                                               &structs[i], &config));
    CHECK_RC(write_struct_from_json_func(out, struct_names[i], &config));
    CHECK_RC(
        write_struct_to_json_func(out, struct_names[i], &structs[i], &config));
    CHECK_RC(
        write_struct_cleanup_func(out, struct_names[i], &structs[i], &config));
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

/* --- Patch Header Implementation --- */

struct FuncSig {
  char *name;
  char *sig;
};

static size_t find_next_token_sync(const struct TokenList *tokens, size_t start,
                                   enum TokenKind kind) {
  size_t i;
  for (i = start; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == kind)
      return i;
  }
  return tokens->size;
}

static char *get_token_string(const struct TokenList *tokens, size_t start,
                              size_t end) {
  size_t len = 0;
  size_t i;
  char *buf, *p;

  if (start >= end)
    return strdup("");

  for (i = start; i < end; ++i)
    len += tokens->tokens[i].length;

  buf = (char *)malloc(len + 1);
  if (!buf)
    return NULL;

  p = buf;
  for (i = start; i < end; ++i) {
    memcpy(p, tokens->tokens[i].start, tokens->tokens[i].length);
    p += tokens->tokens[i].length;
  }
  *p = '\0';
  return buf;
}

static int extract_signatures(const struct TokenList *tokens,
                              struct FuncSig **out_sigs, size_t *out_count) {
  size_t i = 0;
  struct FuncSig *sigs = NULL;
  size_t count = 0;
  size_t capacity = 0;

  while (i < tokens->size) {
    /* Look for function def syntax: [Type] Name ( ... ) { */
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER &&
        /* Heuristic: Avoid keywords */
        strncmp((const char *)tokens->tokens[i].start, "return", 6) != 0 &&
        strncmp((const char *)tokens->tokens[i].start, "if", 2) != 0 &&
        strncmp((const char *)tokens->tokens[i].start, "while", 5) != 0) {

      size_t name_idx = i;
      size_t next = i + 1;
      while (next < tokens->size &&
             tokens->tokens[next].kind == TOKEN_WHITESPACE)
        next++;

      if (next < tokens->size && tokens->tokens[next].kind == TOKEN_LPAREN) {
        /* Found Name ( */
        size_t lparen = next;
        size_t rparen = lparen + 1;
        int depth = 1;

        /* Scan matching ) */
        while (rparen < tokens->size && depth > 0) {
          if (tokens->tokens[rparen].kind == TOKEN_LPAREN)
            depth++;
          else if (tokens->tokens[rparen].kind == TOKEN_RPAREN)
            depth--;
          rparen++;
        }

        if (depth == 0) {
          /* Found matching ) */
          size_t after_paren = rparen;
          while (after_paren < tokens->size &&
                 tokens->tokens[after_paren].kind == TOKEN_WHITESPACE)
            after_paren++;

          if (after_paren < tokens->size &&
              tokens->tokens[after_paren].kind == TOKEN_LBRACE) {
            /* Found definition body Start */
            /* Identify Start of signature */
            /* Backtrack from name_idx to find return type / storage */
            size_t sig_start = name_idx;
            size_t prev;

            while (sig_start > 0) {
              prev = sig_start - 1;
              if (tokens->tokens[prev].kind == TOKEN_SEMICOLON ||
                  tokens->tokens[prev].kind == TOKEN_RBRACE ||
                  tokens->tokens[prev].kind == TOKEN_LBRACE) {
                /* Boundary found */
                break;
              }
              sig_start--;
            }
            /* Skip leading WS */
            while (sig_start < name_idx &&
                   tokens->tokens[sig_start].kind == TOKEN_WHITESPACE)
              sig_start++;

            /* Extract */
            if (count >= capacity) {
              capacity = capacity == 0 ? 8 : capacity * 2;
              {
                struct FuncSig *new_sigs =
                    realloc(sigs, capacity * sizeof(struct FuncSig));
                if (!new_sigs) {
                  size_t k;
                  for (k = 0; k < count; k++) {
                    free(sigs[k].name);
                    free(sigs[k].sig);
                  }
                  free(sigs);
                  return ENOMEM;
                }
                sigs = new_sigs;
              }
            }

            sigs[count].name = get_token_string(tokens, name_idx, name_idx + 1);
            sigs[count].sig = get_token_string(tokens, sig_start, rparen);
            count++;

            /* Skip body to avoid false positives inside */
            {
              int b_depth = 1;
              size_t k = after_paren + 1;
              while (k < tokens->size && b_depth > 0) {
                if (tokens->tokens[k].kind == TOKEN_LBRACE)
                  b_depth++;
                else if (tokens->tokens[k].kind == TOKEN_RBRACE)
                  b_depth--;
                k++;
              }
              i = k;
              continue;
            }
          }
        }
      }
    }
    i++;
  }

  *out_sigs = sigs;
  *out_count = count;
  return 0;
}

int patch_header_from_source(const char *header_path,
                             const char *refactored_source) {
  struct TokenList *src_tl = NULL;
  struct TokenList *hdr_tl = NULL;
  char *hdr_content = NULL;
  size_t hdr_sz = 0;
  struct FuncSig *sigs = NULL;
  size_t sig_count = 0;
  int rc = 0;
  char *new_header = NULL;

  /* 1. Parse Reference Source */
  if (tokenize(az_span_create_from_str((char *)refactored_source), &src_tl) !=
      0)
    return ENOMEM;

  if (extract_signatures(src_tl, &sigs, &sig_count) != 0) {
    free_token_list(src_tl);
    return ENOMEM;
  }

  /* 2. Read and Parse Header */
  rc = read_to_file(header_path, "r", &hdr_content, &hdr_sz);
  if (rc != 0) {
    free_token_list(src_tl);
    /* clean sigs */
    {
      size_t k;
      for (k = 0; k < sig_count; k++) {
        free(sigs[k].name);
        free(sigs[k].sig);
      }
      free(sigs);
    }
    return rc;
  }

  if (tokenize(az_span_create_from_str(hdr_content), &hdr_tl) != 0) {
    free(hdr_content);
    free_token_list(src_tl);
    {
      size_t k;
      for (k = 0; k < sig_count; k++) {
        free(sigs[k].name);
        free(sigs[k].sig);
      }
      free(sigs);
    }
    return ENOMEM;
  }

  /* 3. Patching Strategy: Collect Patches and Rebuild */
  {
    /* Define simple patch struct locally */
    struct LocalPatch {
      size_t start_token;
      size_t end_token; /* Exclusive */
      char *text;
    } *patches = NULL;
    size_t patch_count = 0;

    size_t i = 0;
    while (i < hdr_tl->size) {
      if (hdr_tl->tokens[i].kind == TOKEN_IDENTIFIER) {
        /* Check if potential function declaration Name */
        size_t next = i + 1;
        while (next < hdr_tl->size &&
               hdr_tl->tokens[next].kind == TOKEN_WHITESPACE)
          next++;
        if (next < hdr_tl->size && hdr_tl->tokens[next].kind == TOKEN_LPAREN) {
          char *nm = get_token_string(hdr_tl, i, i + 1);
          size_t k;
          for (k = 0; k < sig_count; k++) {
            if (strcmp(nm, sigs[k].name) == 0) {
              /* Match! Verify it is a declaration. */
              size_t semi = find_next_token_sync(hdr_tl, next, TOKEN_SEMICOLON);
              if (semi < hdr_tl->size) {
                /* Check against definition { */
                size_t brace = find_next_token_sync(hdr_tl, next, TOKEN_LBRACE);
                if (brace < semi) {
                  break; /* Is definition, ignore */
                }

                /* Found Declaration. Determine Start token index. */
                {
                  size_t decl_start = i;
                  struct LocalPatch *new_patches;

                  while (decl_start > 0) {
                    size_t prev = decl_start - 1;
                    if (hdr_tl->tokens[prev].kind == TOKEN_SEMICOLON ||
                        hdr_tl->tokens[prev].kind == TOKEN_RBRACE ||
                        hdr_tl->tokens[prev].kind == TOKEN_LBRACE ||
                        hdr_tl->tokens[prev].kind == TOKEN_COMMENT ||
                        hdr_tl->tokens[prev].kind == TOKEN_MACRO) {
                      break;
                    }
                    decl_start--;
                  }
                  /* Consume preceding whitespace for clean replacement */
                  while (decl_start < i &&
                         hdr_tl->tokens[decl_start].kind == TOKEN_WHITESPACE)
                    decl_start++;

                  /* Add Patch */
                  new_patches = realloc(patches, (patch_count + 1) *
                                                     sizeof(struct LocalPatch));
                  if (new_patches) {
                    patches = new_patches;
                    patches[patch_count].start_token = decl_start;
                    patches[patch_count].end_token =
                        semi; /* Replace up to semicolon (keep semi) */
                    patches[patch_count].text = sigs[k].sig;
                    patch_count++;
                  }
                }
              }
            }
          }
          free(nm);
        }
      }
      i++;
    }

    /* Apply Patches */
    {
      size_t cur_tok = 0;
      size_t p_idx = 0;
      size_t out_cap = hdr_sz + 8192;
      size_t out_len = 0;
      new_header = malloc(out_cap);
      if (new_header) {
        new_header[0] = '\0';

        while (cur_tok < hdr_tl->size) {
          if (p_idx < patch_count && patches[p_idx].start_token == cur_tok) {
            /* Insert Patch Text */
            const char *txt = patches[p_idx].text;
            size_t t_len = strlen(txt);
            if (out_len + t_len + 1 > out_cap) {
              out_cap = out_cap * 2 + t_len;
              {
                char *tmp = realloc(new_header, out_cap);
                if (!tmp) {
                  free(new_header);
                  new_header = NULL;
                  break;
                }
                new_header = tmp;
              }
            }
            memcpy(new_header + out_len, txt, t_len);
            out_len += t_len;
            new_header[out_len] = '\0';

            cur_tok = patches[p_idx].end_token;
            p_idx++;
          } else {
            /* Copy Original Token */
            const size_t t_len = hdr_tl->tokens[cur_tok].length;
            if (out_len + t_len + 1 > out_cap) {
              out_cap = out_cap * 2 + t_len;
              {
                char *tmp = realloc(new_header, out_cap);
                if (!tmp) {
                  free(new_header);
                  new_header = NULL;
                  break;
                }
                new_header = tmp;
              }
            }
            memcpy(new_header + out_len, hdr_tl->tokens[cur_tok].start, t_len);
            out_len += t_len;
            new_header[out_len] = '\0';
            cur_tok++;
          }
        }
      } else {
        rc = ENOMEM;
      }
    }
    if (patches)
      free(patches);
  }

  /* Write Back */
  if (new_header) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    FILE *f;
    errno_t err = fopen_s(&f, header_path, "w, ccs=UTF-8");
    if (err == 0 && f) {
      fputs(new_header, f);
      fclose(f);
    } else {
      rc = (int)err;
    }
#else
    FILE *f = fopen(header_path, "w");
    if (f) {
      fputs(new_header, f);
      fclose(f);
    } else {
      rc = errno ? errno : EIO;
    }
#endif
    free(new_header);
  } else {
    if (rc == 0)
      rc = ENOMEM;
  }

  free(hdr_content);
  free_token_list(src_tl);
  free_token_list(hdr_tl);
  {
    size_t k;
    for (k = 0; k < sig_count; k++) {
      free(sigs[k].name);
      free(sigs[k].sig);
    }
    free(sigs);
  }

  return rc;
}
