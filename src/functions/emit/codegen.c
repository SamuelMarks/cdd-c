/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/codegen.h"
/* clang-format on */

/** @brief CHECK_IO definition */
#ifndef CHECK_IO
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)
#endif

int write_forward_decl(FILE *fp, const char *struct_name) {
  if (!fp || !struct_name) {
    return EINVAL;
  }
  if (fprintf(fp, "struct %s;\n", struct_name) < 0) {
    return EIO;
  }
  return 0;
}

int write_enum_declaration_h(FILE *hfile, const char *enum_name,
                             const struct StructFields *sf,
                             const struct CodegenConfig *config) {
  size_t i;
  if (!hfile || !enum_name || !sf)
    return EINVAL;

  CHECK_IO(fprintf(hfile, "enum %s {\n", enum_name));
  CHECK_IO(fprintf(hfile, "  %s_UNKNOWN = 0,\n", enum_name));
  for (i = 0; i < sf->enum_members.size; ++i) {
    const char *member = sf->enum_members.members[i];
    if (!member || strcmp(member, "UNKNOWN") == 0)
      continue;
    CHECK_IO(fprintf(hfile, "  %s_%s,\n", enum_name, member));
  }
  CHECK_IO(fputs("};\n\n", hfile));

  if (config && config->enum_guard)
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->enum_guard));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_from_str(const char *, enum %s "
                   "*);\n",
                   enum_name, enum_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_to_str(enum %s, char **);\n",
                   enum_name, enum_name));
  if (config && config->enum_guard)
    CHECK_IO(fprintf(hfile, "#endif\n"));

  return 0;
}
int write_union_declaration_h(FILE *hfile, const char *union_name,
                              const struct StructFields *sf,
                              const struct CodegenConfig *config) {
  char *_ast_get_type_from_ref_0 = NULL;
  char *_ast_get_type_from_ref_1 = NULL;
  char *_ast_get_type_from_ref_2 = NULL;
  size_t i;
  if (!hfile || !union_name || !sf)
    return EINVAL;

  CHECK_IO(fprintf(hfile, "enum %s_tag {\n", union_name));
  CHECK_IO(fprintf(hfile, "  %s_UNKNOWN = 0,\n", union_name));
  for (i = 0; i < sf->size; ++i) {
    CHECK_IO(fprintf(hfile, "  %s_%s,\n", union_name, sf->fields[i].name));
  }
  CHECK_IO(fputs("};\n\n", hfile));

  CHECK_IO(fprintf(hfile, "struct %s {\n", union_name));
  CHECK_IO(fprintf(hfile, "  enum %s_tag tag;\n", union_name));
  CHECK_IO(fputs("  union {\n", hfile));
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *field = &sf->fields[i];
    const char *n = field->name;
    const char *t = field->type;
    const char *r = field->ref;

    if (strcmp(t, "string") == 0) {
      CHECK_IO(fprintf(hfile, "    const char *%s;\n", n));
    } else if (strcmp(t, "integer") == 0) {
      CHECK_IO(fprintf(hfile, "    int %s;\n", n));
    } else if (strcmp(t, "number") == 0) {
      CHECK_IO(fprintf(hfile, "    double %s;\n", n));
    } else if (strcmp(t, "boolean") == 0) {
      CHECK_IO(fprintf(hfile, "    int %s;\n", n));
    } else if (strcmp(t, "enum") == 0) {
      CHECK_IO(fprintf(hfile, "    enum %s %s;\n",
                       (get_type_from_ref(r, &_ast_get_type_from_ref_0),
                        _ast_get_type_from_ref_0),
                       n));
    } else if (strcmp(t, "object") == 0) {
      CHECK_IO(fprintf(hfile, "    struct %s *%s;\n",
                       (get_type_from_ref(r, &_ast_get_type_from_ref_1),
                        _ast_get_type_from_ref_1),
                       n));
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
        CHECK_IO(fprintf(hfile, "      struct %s **%s;\n",
                         (get_type_from_ref(r, &_ast_get_type_from_ref_2),
                          _ast_get_type_from_ref_2),
                         n));
      }
      CHECK_IO(fprintf(hfile, "    } %s;\n", n));
    } else {
      CHECK_IO(fprintf(hfile, "    int %s;\n", n));
    }
  }
  CHECK_IO(fputs("  } data;\n};\n\n", hfile));

  if (config && config->json_guard)
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->json_guard));
  CHECK_IO(fprintf(
      hfile,
      "extern LIB_EXPORT int %s_from_json(const char *, struct %s **);\n",
      union_name, union_name));
  CHECK_IO(fprintf(
      hfile, "extern LIB_EXPORT int %s_to_json(const struct %s *, char **);\n",
      union_name, union_name));
  if (config && config->json_guard)
    CHECK_IO(fprintf(hfile, "#endif\n"));

  if (config && config->utils_guard)
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->utils_guard));
  CHECK_IO(fprintf(hfile, "extern LIB_EXPORT void %s_cleanup(struct %s *);\n",
                   union_name, union_name));
  if (config && config->utils_guard)
    CHECK_IO(fprintf(hfile, "#endif\n"));

  return 0;
}
int write_struct_declaration_h(FILE *hfile, const char *struct_name,
                               const struct StructFields *sf,
                               const struct CodegenConfig *config) {
  char *_ast_get_type_from_ref_3 = NULL;
  char *_ast_get_type_from_ref_4 = NULL;
  char *_ast_get_type_from_ref_5 = NULL;
  size_t i;

  if (strcmp(struct_name, "OAuth2TokenResponse") == 0) {
    CHECK_IO(fprintf(hfile, "#ifndef CDD_C_OMIT_OAUTH2_STRUCT\n"));
    CHECK_IO(fprintf(hfile, "#include \"c_orm_oauth2.h\"\n"));
    CHECK_IO(
        fprintf(hfile, "#define OAuth2TokenResponse c_orm_oauth2_token\n"));
    CHECK_IO(fprintf(hfile, "#endif\n\n"));
  } else if (strcmp(struct_name, "User") == 0) {
    CHECK_IO(fprintf(hfile, "#ifndef CDD_C_OMIT_USER_STRUCT\n"));
    CHECK_IO(fprintf(hfile, "#include \"c_orm_user.h\"\n"));
    CHECK_IO(fprintf(hfile, "#define User c_orm_user\n"));
    CHECK_IO(fprintf(hfile, "#endif\n\n"));
  } else {
    CHECK_IO(fprintf(hfile, "struct %s {\n", struct_name));
    for (i = 0; i < sf->size; i++) {
      const struct StructField *field = &sf->fields[i];
      const char *n = field->name;
      const char *t = field->type;
      const char *r = field->ref;

      if (strcmp(t, "string") == 0) {
        CHECK_IO(fprintf(hfile, "  const char *%s;\n", n));
      } else if (strcmp(t, "integer") == 0) {
        CHECK_IO(fprintf(hfile, "  int %s;\n", n));
      } else if (strcmp(t, "number") == 0) {
        CHECK_IO(fprintf(hfile, "  double %s;\n", n));
      } else if (strcmp(t, "boolean") == 0) {
        CHECK_IO(fprintf(hfile, "  int %s;\n", n));
      } else if (strcmp(t, "enum") == 0) {
        CHECK_IO(fprintf(hfile, "  enum %s %s;\n",
                         (get_type_from_ref(r, &_ast_get_type_from_ref_3),
                          _ast_get_type_from_ref_3),
                         n));
      } else if (strcmp(t, "object") == 0) {
        CHECK_IO(fprintf(hfile, "  struct %s *%s;\n",
                         (get_type_from_ref(r, &_ast_get_type_from_ref_4),
                          _ast_get_type_from_ref_4),
                         n));
      } else if (strcmp(t, "array") == 0) {
        CHECK_IO(fprintf(hfile, "  size_t n_%s;\n", n));
        if (strcmp(r, "string") == 0) {
          CHECK_IO(fprintf(hfile, "  char **%s;\n", n));
        } else if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
          CHECK_IO(fprintf(hfile, "  int *%s;\n", n));
        } else if (strcmp(r, "number") == 0) {
          CHECK_IO(fprintf(hfile, "  double *%s;\n", n));
        } else {
          CHECK_IO(fprintf(hfile, "  struct %s **%s;\n",
                           (get_type_from_ref(r, &_ast_get_type_from_ref_5),
                            _ast_get_type_from_ref_5),
                           n));
        }
      } else {
        CHECK_IO(fprintf(hfile, "  void *%s;\n", n));
      }
    }
    CHECK_IO(fputs("};\n\n", hfile));
  }
  /* Prototypes */
  if (config && config->json_guard)
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->json_guard));
  CHECK_IO(fprintf(
      hfile,
      "extern LIB_EXPORT int %s_from_json(const char *, struct %s **);\n",
      struct_name, struct_name));
  CHECK_IO(fprintf(
      hfile, "extern LIB_EXPORT int %s_to_json(const struct %s *, char **);\n",
      struct_name, struct_name));
  if (strcmp(struct_name, "OAuth2TokenResponse") == 0) {
    CHECK_IO(fprintf(hfile,
                     "extern LIB_EXPORT int cdd_c_parse_oauth2_token(const "
                     "char *, struct %s **);\n",
                     struct_name));
  }
  if (config && config->json_guard)
    CHECK_IO(fprintf(hfile, "#endif\n"));

  if (config && config->utils_guard)
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->utils_guard));
  CHECK_IO(fprintf(hfile, "extern LIB_EXPORT void %s_cleanup(struct %s *);\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(hfile, "extern LIB_EXPORT int %s_default(struct %s **);\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(
      hfile,
      "extern LIB_EXPORT int %s_deepcopy(const struct %s *, struct %s **);\n",
      struct_name, struct_name, struct_name));
  CHECK_IO(fprintf(
      hfile,
      "extern LIB_EXPORT int %s_eq(const struct %s *, const struct %s *);\n",
      struct_name, struct_name, struct_name));
  if (config && config->utils_guard)
    CHECK_IO(fprintf(hfile, "#endif\n"));

  return 0;
}
