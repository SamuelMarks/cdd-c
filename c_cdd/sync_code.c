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

  while (fgets(line, sizeof(line), fp)) {
    char *trimmed_line = line;
    while (isspace((unsigned char)*trimmed_line))
      trimmed_line++;
    if (strlen(trimmed_line) == 0)
      continue;

    if (state == NONE) {
      if (str_starts_with(trimmed_line, "enum ")) {
        char *brace = strchr(trimmed_line, '{');
        sscanf(trimmed_line + 5, "%63s", enum_name);
        {
          char *name_brace = strchr(enum_name, '{');
          if (name_brace)
            *name_brace = '\0';
        }
        enum_members_free(&em);
        enum_members_init(&em);
        state = IN_ENUM;

        if (brace)
          trimmed_line = brace + 1;
        else
          continue;
      } else if (str_starts_with(trimmed_line, "struct ")) {
        char *brace = strchr(trimmed_line, '{');
        sscanf(trimmed_line + 7, "%63s", struct_name);
        {
          char *name_brace = strchr(struct_name, '{');
          if (name_brace)
            *name_brace = '\0';
        }
        struct_fields_free(&sf);
        struct_fields_init(&sf);
        state = IN_STRUCT;

        if (brace)
          trimmed_line = brace + 1;
        else
          continue;
      }
    }

    if (state == IN_ENUM) {
      char *end_brace = strchr(trimmed_line, '}');
      if (end_brace)
        *end_brace = '\0';

      {
        char *context = NULL;
        char *token = strtok_r(trimmed_line, ",", &context);
        while (token) {
          char *p = token;
          while (isspace((unsigned char)*p))
            p++;
          trim_trailing(p);
          if (strlen(p) > 0) {
            char *eq = strchr(p, '=');
            if (eq)
              *eq = '\0';
            trim_trailing(p);
            if (strlen(p) > 0)
              enum_members_add(&em, p);
          }
          token = strtok_r(NULL, ",", &context);
        }
      }

      if (end_brace) {
        if (enum_count < sizeof(enums) / sizeof(enums[0])) {
          size_t j;
          enum_members_init(&enums[enum_count]);
          for (j = 0; j < em.size; j++)
            enum_members_add(&enums[enum_count], em.members[j]);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          strncpy_s(enum_names[enum_count], sizeof(enum_names[enum_count]),
                    enum_name, _TRUNCATE);
#else
          strncpy(enum_names[enum_count], enum_name,
                  sizeof(enum_names[enum_count]));
          enum_names[enum_count][sizeof(enum_names[enum_count]) - 1] = '\0';
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */
          enum_count++;
        }
        state = NONE;
      }
    } else if (state == IN_STRUCT) {
      char *end_brace = strchr(trimmed_line, '}');
      if (end_brace)
        *end_brace = '\0';
      {
        char *context = NULL;
        char *token = strtok_r(trimmed_line, ";", &context);
        while (token) {
          char *p = token;
          while (isspace((unsigned char)*p))
            p++;
          if (strlen(p) > 0)
            parse_struct_member_line(p, &sf);
          token = strtok_r(NULL, ";", &context);
        }
      }

      if (end_brace) {
        if (struct_count < sizeof(structs) / sizeof(structs[0])) {
          size_t j;
          struct_fields_init(&structs[struct_count]);
          for (j = 0; j < sf.size; j++) {
            struct_fields_add(&structs[struct_count], sf.fields[j].name,
                              sf.fields[j].type, sf.fields[j].ref);
          }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          strncpy_s(struct_names[struct_count],
                    sizeof(struct_names[struct_count]), struct_name, _TRUNCATE);
#else
          strncpy(struct_names[struct_count], struct_name,
                  sizeof(struct_names[struct_count]));
          struct_names[struct_count][sizeof(struct_names[struct_count]) - 1] =
              '\0';
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */
          struct_count++;
        }
        state = NONE;
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
