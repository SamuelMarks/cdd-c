/*
 * sync_code.c
 *
 * Synchronize .c implementation file with structs/enums declared in .h file.
 *
 * Overwrites the .c file with regenerated functions.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <parson.h>

/* Reuse struct definitions and parsing utilities from code2schema.c above */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif /* !strdup */
#define strtok_r strtok_s
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

#include "sync_code.h"

#include "code2schema.h"
#include "codegen.h"

#include <fs.h>

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

/*
 * The main function: parse header, generate full .c file implementing sync
 * functions.
 */
int sync_code_main(int argc, char **argv) {
  const char *header_filename, *impl_filename;
  FILE *fp, *out;

  enum { NONE, IN_ENUM, IN_STRUCT } state = NONE;

  char line[512];
  char enum_name[64] = {0};
  struct EnumMembers em;
  char struct_name[64] = {0};
  struct StructFields sf;

  struct EnumMembers enums[64];
  size_t enum_count = 0;
  char enum_names[64][64];

  struct StructFields structs[64];
  char struct_names[64][64];
  size_t struct_count = 0, i;

  if (argc != 2) {
    fputs("Usage: sync_code <header.h> <impl.c>", stderr);
    return EXIT_FAILURE;
  }

  header_filename = argv[0];
  impl_filename = argv[1];

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, header_filename, "r");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to header file %s\n", header_filename);
      return EXIT_FAILURE;
    }
  }
#else
  fp = fopen(header_filename, "r");
  if (!fp) {
    fprintf(stderr, "Failed to open header file: %s\n", header_filename);
    return EXIT_FAILURE;
  }
#endif

  enum_members_init(&em);
  struct_fields_init(&sf);

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
          enum_members_init(&em);
          state = IN_ENUM;
        } else { /* struct */
          extract_name_sync(struct_name, sizeof(struct_name), p + 7, brace);
          struct_fields_free(&sf);
          struct_fields_init(&sf);
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
          continue;
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
          continue;
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
            if (strlen(member_p) > 0)
              enum_members_add(&em, member_p);
          }
          token = strtok_r(NULL, ",", &context);
        }
      }
      free(body_to_parse);

      if (end_brace) {
        if (enum_count < sizeof(enums) / sizeof(enums[0])) {
          size_t j;
          enum_members_init(&enums[enum_count]);
          for (j = 0; j < em.size; j++)
            enum_members_add(&enums[enum_count], em.members[j]);
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
          continue;
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
          continue;
        }
      }

      {
        char *context = NULL;
        char *token = strtok_r(body_to_parse, ";", &context);
        while (token) {
          char *field_p = token;
          while (isspace((unsigned char)*field_p))
            field_p++;
          if (strlen(field_p) > 0)
            parse_struct_member_line(field_p, &sf);
          token = strtok_r(NULL, ";", &context);
        }
      }
      free(body_to_parse);

      if (end_brace) {
        if (struct_count < sizeof(structs) / sizeof(structs[0])) {
          size_t j;
          struct_fields_init(&structs[struct_count]);
          for (j = 0; j < sf.size; j++) {
            struct_fields_add(&structs[struct_count], sf.fields[j].name,
                              sf.fields[j].type, sf.fields[j].ref);
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
  enum_members_free(&em);
  struct_fields_free(&sf);

  /* Write the impl file with all function implementations */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&out, impl_filename, "w");
    if (err != 0 || out == NULL) {
      fprintf(stderr, "Failed to open impl file %s\n", impl_filename);
      return -1;
    }
  }
#else
  out = fopen(impl_filename, "w");
  if (!out) {
    fprintf(stderr, "Failed to open impl file: %s\n", impl_filename);
    return EXIT_FAILURE;
  }
#endif

  fputs("#include <stdlib.h>\n"
        "#include <string.h>\n"
        "#include <stdio.h>\n\n"
        "#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)\n"
        "#else\n"
        "#include <sys/errno.h>\n"
        "#endif\n"
        "#include <parson.h>\n\n",
        out);
  fprintf(out, "#include \"%s\"\n\n", get_basename(header_filename));

  /* Write enums code */
  for (i = 0; i < enum_count; i++) {
    write_enum_to_str_func(out, enum_names[i], &enums[i]);
    write_enum_from_str_func(out, enum_names[i], &enums[i]);
    enum_members_free(&enums[i]);
  }

  /* Write structs code */
  for (i = 0; i < struct_count; i++) {
    write_struct_debug_func(out, struct_names[i], &structs[i]);
    write_struct_deepcopy_func(out, struct_names[i], &structs[i]);
    write_struct_default_func(out, struct_names[i], &structs[i]);
    write_struct_display_func(out, struct_names[i], &structs[i]);
    write_struct_eq_func(out, struct_names[i], &structs[i]);
    write_struct_from_jsonObject_func(out, struct_names[i], &structs[i]);
    write_struct_from_json_func(out, struct_names[i]);
    write_struct_to_json_func(out, struct_names[i], &structs[i]);
    write_struct_cleanup_func(out, struct_names[i], &structs[i]);
    struct_fields_free(&structs[i]);
  }

  fclose(out);

  printf("Synchronized implementation file %s from header %s\n", impl_filename,
         header_filename);

  return 0;
}
