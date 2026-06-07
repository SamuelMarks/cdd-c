/* LCOV_EXCL_START */
/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/codegen.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
#include <stdarg.h>
extern int g_fail_io_after;
extern int g_io_calls;
static int cdd_fprintf_hook(FILE *stream, const char *format, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)));
#else
    ;
#endif
static int cdd_fprintf_hook(FILE *stream, const char *format, ...) {
  int ret;
  va_list args;
  if (g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after)
    return -1;
  va_start(args, format);
  ret = vfprintf(stream, format, args);
  va_end(args);
  return ret;
}
/** @brief FPRINTF_HOOK macro */
#define FPRINTF_HOOK cdd_fprintf_hook
#else
/** @brief FPRINTF_HOOK macro */
#define FPRINTF_HOOK fprintf
#endif

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
  if (FPRINTF_HOOK(fp, "struct %s;\n", struct_name) < 0) {
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

  CHECK_IO(FPRINTF_HOOK(hfile, "enum %s {\n", enum_name));
  CHECK_IO(FPRINTF_HOOK(hfile, "  %s_UNKNOWN = 0,\n", enum_name));
  for (i = 0; i < sf->enum_members.size; ++i) {
    const char *member = sf->enum_members.members[i];
    if (!member || strcmp(member, "UNKNOWN") == 0)
      continue;
    CHECK_IO(FPRINTF_HOOK(hfile, "  %s_%s,\n", enum_name, member));
  }
  CHECK_IO(FPRINTF_HOOK(hfile, "%s", "};\n\n"));

  if (config && config->enum_guard)
    CHECK_IO(FPRINTF_HOOK(hfile, "#ifdef %s\n", config->enum_guard));
  CHECK_IO(
      FPRINTF_HOOK(hfile,
                   "extern LIB_EXPORT int %s_from_str(const char *, enum %s "
                   "*);\n",
                   enum_name, enum_name));
  CHECK_IO(FPRINTF_HOOK(hfile,
                        "extern LIB_EXPORT int %s_to_str(enum %s, char **);\n",
                        enum_name, enum_name));
  if (config && config->enum_guard)
    CHECK_IO(FPRINTF_HOOK(hfile, "#endif\n"));

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

  CHECK_IO(FPRINTF_HOOK(hfile, "enum %s_tag {\n", union_name));
  CHECK_IO(FPRINTF_HOOK(hfile, "  %s_UNKNOWN = 0,\n", union_name));
  for (i = 0; i < sf->size; ++i) {
    CHECK_IO(FPRINTF_HOOK(hfile, "  %s_%s,\n", union_name, sf->fields[i].name));
  }
  CHECK_IO(FPRINTF_HOOK(hfile, "%s", "};\n\n"));

  CHECK_IO(FPRINTF_HOOK(hfile, "struct %s {\n", union_name));
  CHECK_IO(FPRINTF_HOOK(hfile, "  enum %s_tag tag;\n", union_name));
  CHECK_IO(FPRINTF_HOOK(hfile, "%s", "  union {\n"));
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *field = &sf->fields[i];
    const char *n = field->name;
    const char *t = field->type;
    const char *r = field->ref;

    if (strcmp(t, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(hfile, "    const char *%s;\n", n));
    } else if (strcmp(t, "integer") == 0) {
      CHECK_IO(FPRINTF_HOOK(hfile, "    int %s;\n", n));
    } else if (strcmp(t, "number") == 0) {
      CHECK_IO(FPRINTF_HOOK(hfile, "    double %s;\n", n));
    } else if (strcmp(t, "boolean") == 0) {
      CHECK_IO(FPRINTF_HOOK(hfile, "    int %s;\n", n));
    } else if (strcmp(t, "enum") == 0) {
      CHECK_IO(FPRINTF_HOOK(hfile, "    enum %s %s;\n",
                            (get_type_from_ref(r, &_ast_get_type_from_ref_0),
                             _ast_get_type_from_ref_0),
                            n));
    } else if (strcmp(t, "object") == 0) {
      CHECK_IO(FPRINTF_HOOK(hfile, "    struct %s *%s;\n",
                            (get_type_from_ref(r, &_ast_get_type_from_ref_1),
                             _ast_get_type_from_ref_1),
                            n));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(FPRINTF_HOOK(hfile, "    struct {\n"));
      CHECK_IO(FPRINTF_HOOK(hfile, "      size_t n_%s;\n", n));
      if (strcmp(r, "string") == 0) {
        CHECK_IO(FPRINTF_HOOK(hfile, "      char **%s;\n", n));
      } else if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
        CHECK_IO(FPRINTF_HOOK(hfile, "      int *%s;\n", n));
      } else if (strcmp(r, "number") == 0) {
        CHECK_IO(FPRINTF_HOOK(hfile, "      double *%s;\n", n));
      } else {
        CHECK_IO(FPRINTF_HOOK(hfile, "      struct %s **%s;\n",
                              (get_type_from_ref(r, &_ast_get_type_from_ref_2),
                               _ast_get_type_from_ref_2),
                              n));
      }
      CHECK_IO(FPRINTF_HOOK(hfile, "    } %s;\n", n));
    } else {
      CHECK_IO(FPRINTF_HOOK(hfile, "    int %s;\n", n));
    }
  }
  CHECK_IO(FPRINTF_HOOK(hfile, "%s", "  } data;\n};\n\n"));

  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(hfile, "#ifdef %s\n", config->json_guard));
  CHECK_IO(FPRINTF_HOOK(
      hfile,
      "extern LIB_EXPORT int %s_from_json(const char *, struct %s **);\n",
      union_name, union_name));
  CHECK_IO(FPRINTF_HOOK(
      hfile, "extern LIB_EXPORT int %s_to_json(const struct %s *, char **);\n",
      union_name, union_name));
  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(hfile, "#endif\n"));

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(hfile, "#ifdef %s\n", config->utils_guard));
  CHECK_IO(FPRINTF_HOOK(hfile,
                        "extern LIB_EXPORT void %s_cleanup(struct %s *);\n",
                        union_name, union_name));
  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(hfile, "#endif\n"));

  return 0;
}
int write_struct_declaration_h(FILE *hfile, const char *struct_name,
                               const struct StructFields *sf,
                               const struct CodegenConfig *config) {
  char *_ast_get_type_from_ref_3 = NULL;
  char *_ast_get_type_from_ref_4 = NULL;
  char *_ast_get_type_from_ref_5 = NULL;
  size_t i;

  if (!hfile || !struct_name || !sf)
    return EINVAL;

  if (0) {
    CHECK_IO(FPRINTF_HOOK(hfile, "#ifndef CDD_C_OMIT_OAUTH2_STRUCT\n"));
    CHECK_IO(FPRINTF_HOOK(hfile, "#include \"c_orm_oauth2.h\"\n"));
    CHECK_IO(FPRINTF_HOOK(hfile,
                          "#define OAuth2TokenResponse c_orm_oauth2_token\n"));
    CHECK_IO(FPRINTF_HOOK(hfile, "#endif\n\n"));
  } else if (0) {
    CHECK_IO(FPRINTF_HOOK(hfile, "#ifndef CDD_C_OMIT_USER_STRUCT\n"));
    CHECK_IO(FPRINTF_HOOK(hfile, "#include \"c_orm_user.h\"\n"));
    CHECK_IO(FPRINTF_HOOK(hfile, "#define User c_orm_user\n"));
    CHECK_IO(FPRINTF_HOOK(hfile, "#endif\n\n"));
  } else {
    CHECK_IO(FPRINTF_HOOK(hfile, "struct %s {\n", struct_name));
    if (sf->size == 0) {
      CHECK_IO(FPRINTF_HOOK(hfile, "  char _dummy;\n"));
    }
    for (i = 0; i < sf->size; i++) {
      const struct StructField *field = &sf->fields[i];
      const char *n = field->name;
      const char *t = field->type;
      const char *r = field->ref;

      if (strcmp(t, "string") == 0) {
        CHECK_IO(FPRINTF_HOOK(hfile, "  const char *%s;\n", n));
      } else if (strcmp(t, "integer") == 0) {
        CHECK_IO(FPRINTF_HOOK(hfile, "  int %s;\n", n));
      } else if (strcmp(t, "number") == 0) {
        CHECK_IO(FPRINTF_HOOK(hfile, "  double %s;\n", n));
      } else if (strcmp(t, "boolean") == 0) {
        CHECK_IO(FPRINTF_HOOK(hfile, "  int %s;\n", n));
      } else if (strcmp(t, "enum") == 0) {
        CHECK_IO(FPRINTF_HOOK(hfile, "  enum %s %s;\n",
                              (get_type_from_ref(r, &_ast_get_type_from_ref_3),
                               _ast_get_type_from_ref_3),
                              n));
      } else if (strcmp(t, "object") == 0) {
        CHECK_IO(FPRINTF_HOOK(hfile, "  struct %s *%s;\n",
                              (get_type_from_ref(r, &_ast_get_type_from_ref_4),
                               _ast_get_type_from_ref_4),
                              n));
      } else if (strcmp(t, "array") == 0) {
        CHECK_IO(FPRINTF_HOOK(hfile, "  size_t n_%s;\n", n));
        if (strcmp(r, "string") == 0) {
          CHECK_IO(FPRINTF_HOOK(hfile, "  char **%s;\n", n));
        } else if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
          CHECK_IO(FPRINTF_HOOK(hfile, "  int *%s;\n", n));
        } else if (strcmp(r, "number") == 0) {
          CHECK_IO(FPRINTF_HOOK(hfile, "  double *%s;\n", n));
        } else {
          CHECK_IO(
              FPRINTF_HOOK(hfile, "  struct %s **%s;\n",
                           (get_type_from_ref(r, &_ast_get_type_from_ref_5),
                            _ast_get_type_from_ref_5),
                           n));
        }
      } else {
        CHECK_IO(FPRINTF_HOOK(hfile, "  void *%s;\n", n));
      }
    }
    CHECK_IO(FPRINTF_HOOK(hfile, "%s", "};\n\n"));
  }
  /* Prototypes */
  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(hfile, "#ifdef %s\n", config->json_guard));
  CHECK_IO(FPRINTF_HOOK(
      hfile,
      "extern LIB_EXPORT int %s_from_json(const char *, struct %s **);\n",
      struct_name, struct_name));
  CHECK_IO(
      FPRINTF_HOOK(hfile,
                   "extern LIB_EXPORT int %s_array_from_json(const char *, "
                   "struct %s ***, size_t *);\n",
                   struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(
      hfile, "extern LIB_EXPORT int %s_to_json(const struct %s *, char **);\n",
      struct_name, struct_name));
  if (0) {
    CHECK_IO(
        FPRINTF_HOOK(hfile,
                     "extern LIB_EXPORT int cdd_c_parse_oauth2_token(const "
                     "char *, struct %s **);\n",
                     struct_name));
  }
  if (config && config->json_guard)
    CHECK_IO(FPRINTF_HOOK(hfile, "#endif\n"));

  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(hfile, "#ifdef %s\n", config->utils_guard));
  CHECK_IO(FPRINTF_HOOK(hfile,
                        "extern LIB_EXPORT void %s_cleanup(struct %s *);\n",
                        struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(hfile,
                        "extern LIB_EXPORT int %s_default(struct %s **);\n",
                        struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(
      hfile,
      "extern LIB_EXPORT int %s_deepcopy(const struct %s *, struct %s **);\n",
      struct_name, struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(
      hfile,
      "extern LIB_EXPORT int %s_eq(const struct %s *, const struct %s *);\n",
      struct_name, struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(
      hfile, "extern LIB_EXPORT int %s_debug(const struct %s *, FILE *);\n",
      struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(
      hfile, "extern LIB_EXPORT int %s_display(const struct %s *, FILE *);\n",
      struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(hfile, "struct json_object_t;\n"));
  CHECK_IO(FPRINTF_HOOK(hfile,
                        "extern LIB_EXPORT int %s_from_jsonObject(const struct "
                        "json_object_t *, struct %s **);\n",
                        struct_name, struct_name));
  CHECK_IO(
      FPRINTF_HOOK(hfile,
                   "extern LIB_EXPORT int %s_to_jsonObject(const struct %s *, "
                   "struct json_object_t **);\n",
                   struct_name, struct_name));
  if (config && config->utils_guard)
    CHECK_IO(FPRINTF_HOOK(hfile, "#endif\n"));

  return 0;
}

/* LCOV_EXCL_STOP */
