/**
 * @file codegen.c
 * @brief Core codegen definitions and implementation.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/code_writer.h"
#include "functions/emit/codegen.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
/** @brief fprintf macro */
#define fprintf cdd_io_fprintf_hook
#else
/** @brief fprintf macro */
#define fprintf fprintf
#endif

/** @brief CHECK_IO definition */
#ifndef CHECK_IO
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO
#endif

cdd_c_error_t write_forward_decl(FILE *fp, const char *struct_name) {
  if (!fp || !struct_name) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (fprintf(fp, "struct %s;\n", struct_name) < 0) {
    return CDD_C_ERROR_IO;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t write_enum_declaration_h(FILE *hfile, const char *enum_name,
                                       const struct StructFields *sf,
                                       const struct CodegenConfig *config) {
  size_t i;
  if (!hfile || !enum_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  CHECK_IO(fprintf(hfile, "enum %s {\n", enum_name));
  CHECK_IO(fprintf(hfile, "  %s_UNKNOWN = 0,\n", enum_name));
  for (i = 0; i < sf->enum_members.size; ++i) {
    const char *member = sf->enum_members.members[i];
    if (member == NULL) {
      continue;
    }
    if (strcmp(member, "UNKNOWN") == 0) {
      continue;
    }
    CHECK_IO(fprintf(hfile, "  %s_%s,\n", enum_name, member));
  }
  CHECK_IO(fprintf(hfile, "%s", "};\n\n"));

  if (config && config->enum_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->enum_guard));
  }
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_from_str(const char *, enum %s "
                   "*);\n",
                   enum_name, enum_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_to_str(enum %s, char **);\n",
                   enum_name, enum_name));
  if (config && config->enum_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
  }

  return CDD_C_SUCCESS;
}
cdd_c_error_t write_union_declaration_h(FILE *hfile, const char *union_name,
                                        const struct StructFields *sf,
                                        const struct CodegenConfig *config) {
  size_t i;
  if (!hfile || !union_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  CHECK_IO(fprintf(hfile, "enum %s_tag {\n", union_name));
  CHECK_IO(fprintf(hfile, "  %s_UNKNOWN = 0,\n", union_name));
  for (i = 0; i < sf->size; ++i) {
    CHECK_IO(fprintf(hfile, "  %s_%s,\n", union_name, sf->fields[i].name));
  }
  CHECK_IO(fprintf(hfile, "%s", "};\n\n"));

  CHECK_IO(fprintf(hfile, "struct %s {\n", union_name));
  CHECK_IO(fprintf(hfile, "  enum %s_tag tag;\n", union_name));
  CHECK_IO(fprintf(hfile, "%s", "  union {\n"));
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *field = &sf->fields[i];
    const char *n = field->name;
    const char *t = field->type;
    const char *r = field->ref;
    const char *ref_type = NULL;

    if (strcmp(t, "string") == 0) {
      CHECK_IO(fprintf(hfile, "    const char *%s;\n", n));
    } else if (strcmp(t, "integer") == 0) {
      CHECK_IO(fprintf(hfile, "    int %s;\n", n));
    } else if (strcmp(t, "number") == 0) {
      CHECK_IO(fprintf(hfile, "    double %s;\n", n));
    } else if (strcmp(t, "boolean") == 0) {
      CHECK_IO(fprintf(hfile, "    int %s;\n", n));
    } else if (strcmp(t, "enum") == 0) {
      get_type_from_ref(r, &ref_type);
      CHECK_IO(fprintf(hfile, "    enum %s %s;\n", ref_type, n));
    } else if (strcmp(t, "object") == 0) {
      get_type_from_ref(r, &ref_type);
      CHECK_IO(fprintf(hfile, "    struct %s *%s;\n", ref_type, n));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(fprintf(hfile, "    struct {\n"));
      CHECK_IO(fprintf(hfile, "      size_t n_%s;\n", n));
      if (strcmp(r, "string") == 0) {
        CHECK_IO(fprintf(hfile, "      char **%s;\n", n));
      } else if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
        CHECK_IO(fprintf(hfile, "      int *%s;\n", n));
      } else if (strcmp(r, "number") == 0) {
        CHECK_IO(fprintf(hfile, "      double *%s;\n", n));
      } else {
        get_type_from_ref(r, &ref_type);
        CHECK_IO(fprintf(hfile, "      struct %s **%s;\n", ref_type, n));
      }
      CHECK_IO(fprintf(hfile, "    } %s;\n", n));
    } else {
      CHECK_IO(fprintf(hfile, "    int %s;\n", n));
    }
  }
  CHECK_IO(fprintf(hfile, "%s", "  } data;\n};\n\n"));

  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->json_guard));
  }
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_from_json(const "
                   "char *, struct %s **);\n",
                   union_name, union_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_to_json(const "
                   "struct %s *, char **);\n",
                   union_name, union_name));
  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
  }

  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->utils_guard));
  }
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_cleanup(struct %s *);\n",
                   union_name, union_name));
  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
  }

  return CDD_C_SUCCESS;
}
cdd_c_error_t write_struct_declaration_h(FILE *hfile, const char *struct_name,
                                         const struct StructFields *sf,
                                         const struct CodegenConfig *config) {
  size_t i;

  if (!hfile || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  CHECK_IO(fprintf(hfile, "struct %s {\n", struct_name));
  if (sf->size == 0) {
    CHECK_IO(fprintf(hfile, "  char _dummy;\n"));
  }
  for (i = 0; i < sf->size; i++) {
    const struct StructField *field = &sf->fields[i];
    const char *n = field->name;
    const char *t = field->type;
    const char *r = field->ref;
    const char *ref_type = NULL;

    if (strcmp(t, "string") == 0) {
      CHECK_IO(fprintf(hfile, "  const char *%s;\n", n));
    } else if (strcmp(t, "integer") == 0) {
      CHECK_IO(fprintf(hfile, "  int %s;\n", n));
    } else if (strcmp(t, "number") == 0) {
      CHECK_IO(fprintf(hfile, "  double %s;\n", n));
    } else if (strcmp(t, "boolean") == 0) {
      CHECK_IO(fprintf(hfile, "  int %s;\n", n));
    } else if (strcmp(t, "enum") == 0) {
      get_type_from_ref(r, &ref_type);
      CHECK_IO(fprintf(hfile, "  enum %s %s;\n", ref_type, n));
    } else if (strcmp(t, "object") == 0) {
      get_type_from_ref(r, &ref_type);
      CHECK_IO(fprintf(hfile, "  struct %s *%s;\n", ref_type, n));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(fprintf(hfile, "  size_t n_%s;\n", n));
      if (strcmp(r, "string") == 0) {
        CHECK_IO(fprintf(hfile, "  char **%s;\n", n));
      } else if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
        CHECK_IO(fprintf(hfile, "  int *%s;\n", n));
      } else if (strcmp(r, "number") == 0) {
        CHECK_IO(fprintf(hfile, "  double *%s;\n", n));
      } else {
        get_type_from_ref(r, &ref_type);
        CHECK_IO(fprintf(hfile, "  struct %s **%s;\n", ref_type, n));
      }
    } else {
      CHECK_IO(fprintf(hfile, "  void *%s;\n", n));
    }
  }
  CHECK_IO(fprintf(hfile, "%s", "};\n\n"));
  /* Prototypes */
  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->json_guard));
  }
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_from_json(const "
                   "char *, struct %s **);\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(
      hfile,
      "extern LIB_EXPORT cdd_c_error_t %s_array_from_json(const char *, "
      "struct %s ***, size_t *);\n",
      struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_to_json(const "
                   "struct %s *, char **);\n",
                   struct_name, struct_name));
  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
  }

  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->utils_guard));
  }
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_cleanup(struct %s *);\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(
      hfile, "extern LIB_EXPORT cdd_c_error_t %s_default(struct %s **);\n",
      struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_deepcopy(const "
                   "struct %s *, struct %s **);\n",
                   struct_name, struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_eq(const struct "
                   "%s *, const struct %s *, int *out_eq);\n",
                   struct_name, struct_name, struct_name));
  CHECK_IO(fprintf(
      hfile,
      "extern LIB_EXPORT cdd_c_error_t %s_debug(const struct %s *, FILE *);\n",
      struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_display(const struct %s "
                   "*, FILE *);\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(hfile, "struct json_object_t;\n"));
  CHECK_IO(
      fprintf(hfile,
              "extern LIB_EXPORT cdd_c_error_t %s_from_jsonObject(const struct "
              "json_object_t *, struct %s **);\n",
              struct_name, struct_name));
  CHECK_IO(fprintf(
      hfile,
      "extern LIB_EXPORT cdd_c_error_t %s_to_jsonObject(const struct %s *, "
      "struct json_object_t **);\n",
      struct_name, struct_name));
  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
  }

  return CDD_C_SUCCESS;
}
