/**
 * @file sync_code.c
 * @brief Implementation of code synchronization.
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
#define strtok_r strtok_s
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

#include "sync_code.h"

#include "code2schema.h" /* For struct/enum list parsing */
#include "codegen.h"     /* For code generation functions */
#include "fs.h"          /* For utilities */

#define CHECK_RC(x)                                                            \
  do {                                                                         \
    const int err_code = (x);                                                  \
    if (err_code != 0)                                                         \
      return err_code;                                                         \
  } while (0)

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
    fprintf(stderr, "Failed to open header file: %s\n", header_filename);
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
