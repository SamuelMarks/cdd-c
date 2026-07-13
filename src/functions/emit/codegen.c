/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions/emit/codegen.h"
#include <stdarg.h>

/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern int g_fail_io_after;
extern int g_io_calls;
static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)));
#else
    ;
#endif
/* LCOV_EXCL_START */
/* LCOV_EXCL_START */
static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...) {
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  int ret;
  va_list args;
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after)
    return -1;
  va_start(args, format);
  ret = vfprintf(stream, format, args);
  va_end(args);
  return ret;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
}
/** @brief fprintf macro */
#define fprintf test_cdd_fprintf_hook
#else
/** @brief fprintf macro */
#define fprintf fprintf
#endif

/** @brief CHECK_IO definition */
#ifndef CHECK_IO
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return CDD_C_ERROR_IO;                                                   \
  } while (0)
#endif

/* LCOV_EXCL_START */
/* LCOV_EXCL_START */
enum cdd_c_error write_forward_decl(FILE *fp, const char *struct_name) {
  if (!fp || !struct_name) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (fprintf(fp, "struct %s;\n", struct_name) < 0) {
    return CDD_C_ERROR_IO;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
/* LCOV_EXCL_START */
enum cdd_c_error write_enum_declaration_h(FILE *hfile, const char *enum_name,
                                          /* LCOV_EXCL_STOP */
                                          /* LCOV_EXCL_STOP */
                                          const struct StructFields *sf,
                                          const struct CodegenConfig *config) {
  size_t i;
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (!hfile || !enum_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile, "enum %s {\n", enum_name));
  CHECK_IO(fprintf(hfile, "  %s_UNKNOWN = 0,\n", enum_name));
  for (i = 0; i < sf->enum_members.size; ++i) {
    const char *member = sf->enum_members.members[i];
    if (!member || strcmp(member, "UNKNOWN") == 0)
      continue;
    CHECK_IO(fprintf(hfile, "  %s_%s,\n", enum_name, member));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile, "%s", "};\n\n"));
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (config && config->enum_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->enum_guard));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile,
                   /* LCOV_EXCL_STOP */
                   /* LCOV_EXCL_STOP */
                   "extern LIB_EXPORT int %s_from_str(const char *, enum %s "
                   "*);\n",
                   enum_name, enum_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile,
                   /* LCOV_EXCL_STOP */
                   /* LCOV_EXCL_STOP */
                   "extern LIB_EXPORT int %s_to_str(enum %s, char **);\n",
                   enum_name, enum_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (config && config->enum_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
}
/* LCOV_EXCL_START */
/* LCOV_EXCL_START */
enum cdd_c_error write_union_declaration_h(FILE *hfile, const char *union_name,
                                           /* LCOV_EXCL_STOP */
                                           /* LCOV_EXCL_STOP */
                                           const struct StructFields *sf,
                                           const struct CodegenConfig *config) {
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  char *_ast_get_type_from_ref_0 = NULL;
  char *_ast_get_type_from_ref_1 = NULL;
  char *_ast_get_type_from_ref_2 = NULL;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  size_t i;
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (!hfile || !union_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile, "enum %s_tag {\n", union_name));
  CHECK_IO(fprintf(hfile, "  %s_UNKNOWN = 0,\n", union_name));
  for (i = 0; i < sf->size; ++i) {
    CHECK_IO(fprintf(hfile, "  %s_%s,\n", union_name, sf->fields[i].name));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile, "%s", "};\n\n"));
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile, "struct %s {\n", union_name));
  CHECK_IO(fprintf(hfile, "  enum %s_tag tag;\n", union_name));
  CHECK_IO(fprintf(hfile, "%s", "  union {\n"));
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *field = &sf->fields[i];
    const char *n = field->name;
    const char *t = field->type;
    const char *r = field->ref;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
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
                       /* LCOV_EXCL_STOP */
                       /* LCOV_EXCL_STOP */
                       (get_type_from_ref(r, &_ast_get_type_from_ref_0),
                        _ast_get_type_from_ref_0),
                       n));
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
    } else if (strcmp(t, "object") == 0) {
      CHECK_IO(fprintf(hfile, "    struct %s *%s;\n",
                       /* LCOV_EXCL_STOP */
                       /* LCOV_EXCL_STOP */
                       (get_type_from_ref(r, &_ast_get_type_from_ref_1),
                        _ast_get_type_from_ref_1),
                       n));
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(fprintf(hfile, "    struct {\n"));
      CHECK_IO(fprintf(hfile, "      size_t n_%s;\n", n));
      if (strcmp(r, "string") == 0) {
        CHECK_IO(fprintf(hfile, "      char **%s;\n", n));
      } else if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
        CHECK_IO(fprintf(hfile, "      int *%s;\n", n));
      } else if (strcmp(r, "number") == 0) {
        CHECK_IO(fprintf(hfile, "      double *%s;\n", n));
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_STOP */
      } else {
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
        CHECK_IO(fprintf(hfile, "      struct %s **%s;\n",
                         /* LCOV_EXCL_STOP */
                         /* LCOV_EXCL_STOP */
                         (get_type_from_ref(r, &_ast_get_type_from_ref_2),
                          _ast_get_type_from_ref_2),
                         n));
      }
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      CHECK_IO(fprintf(hfile, "    } %s;\n", n));
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
    } else {
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      CHECK_IO(fprintf(hfile, "    int %s;\n", n));
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
    }
  }
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile, "%s", "  } data;\n};\n\n"));
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->json_guard));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile,
                   /* LCOV_EXCL_STOP */
                   /* LCOV_EXCL_STOP */
                   "extern LIB_EXPORT enum cdd_c_error %s_from_json(const "
                   "char *, struct %s **);\n",
                   union_name, union_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile,
                   /* LCOV_EXCL_STOP */
                   /* LCOV_EXCL_STOP */
                   "extern LIB_EXPORT enum cdd_c_error %s_to_json(const "
                   "struct %s *, char **);\n",
                   union_name, union_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->utils_guard));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      hfile, "extern LIB_EXPORT enum cdd_c_error %s_cleanup(struct %s *);\n",
      union_name, union_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
}
enum cdd_c_error
/* LCOV_EXCL_START */
/* LCOV_EXCL_START */
write_struct_declaration_h(FILE *hfile, const char *struct_name,
                           /* LCOV_EXCL_STOP */
                           /* LCOV_EXCL_STOP */
                           const struct StructFields *sf,
                           const struct CodegenConfig *config) {
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  char *_ast_get_type_from_ref_3 = NULL;
  char *_ast_get_type_from_ref_4 = NULL;
  char *_ast_get_type_from_ref_5 = NULL;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  size_t i;

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (!hfile || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  if (0) {
    CHECK_IO(fprintf(hfile, "#ifndef CDD_C_OMIT_OAUTH2_STRUCT\n"));
    CHECK_IO(fprintf(hfile, "#include \"c_orm_oauth2.h\"\n"));
    CHECK_IO(
        fprintf(hfile, "#define OAuth2TokenResponse c_orm_oauth2_token\n"));
    CHECK_IO(fprintf(hfile, "#endif\n\n"));
  } else if (0) {
    CHECK_IO(fprintf(hfile, "#ifndef CDD_C_OMIT_USER_STRUCT\n"));
    CHECK_IO(fprintf(hfile, "#include \"c_orm_user.h\"\n"));
    CHECK_IO(fprintf(hfile, "#define User c_orm_user\n"));
    CHECK_IO(fprintf(hfile, "#endif\n\n"));
  } else {
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    CHECK_IO(fprintf(hfile, "struct %s {\n", struct_name));
    if (sf->size == 0) {
      CHECK_IO(fprintf(hfile, "  char _dummy;\n"));
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    for (i = 0; i < sf->size; i++) {
      const struct StructField *field = &sf->fields[i];
      const char *n = field->name;
      const char *t = field->type;
      const char *r = field->ref;
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
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
                         /* LCOV_EXCL_STOP */
                         /* LCOV_EXCL_STOP */
                         (get_type_from_ref(r, &_ast_get_type_from_ref_3),
                          _ast_get_type_from_ref_3),
                         n));
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
      } else if (strcmp(t, "object") == 0) {
        CHECK_IO(fprintf(hfile, "  struct %s *%s;\n",
                         /* LCOV_EXCL_STOP */
                         /* LCOV_EXCL_STOP */
                         (get_type_from_ref(r, &_ast_get_type_from_ref_4),
                          _ast_get_type_from_ref_4),
                         n));
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
      } else if (strcmp(t, "array") == 0) {
        CHECK_IO(fprintf(hfile, "  size_t n_%s;\n", n));
        if (strcmp(r, "string") == 0) {
          CHECK_IO(fprintf(hfile, "  char **%s;\n", n));
        } else if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
          CHECK_IO(fprintf(hfile, "  int *%s;\n", n));
        } else if (strcmp(r, "number") == 0) {
          CHECK_IO(fprintf(hfile, "  double *%s;\n", n));
          /* LCOV_EXCL_STOP */
          /* LCOV_EXCL_STOP */
        } else {
          /* LCOV_EXCL_START */
          /* LCOV_EXCL_START */
          CHECK_IO(fprintf(hfile, "  struct %s **%s;\n",
                           /* LCOV_EXCL_STOP */
                           /* LCOV_EXCL_STOP */
                           (get_type_from_ref(r, &_ast_get_type_from_ref_5),
                            _ast_get_type_from_ref_5),
                           n));
        }
      } else {
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
        CHECK_IO(fprintf(hfile, "  void *%s;\n", n));
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_STOP */
      }
    }
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    CHECK_IO(fprintf(hfile, "%s", "};\n\n"));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* Prototypes */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->json_guard));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile,
                   /* LCOV_EXCL_STOP */
                   /* LCOV_EXCL_STOP */
                   "extern LIB_EXPORT enum cdd_c_error %s_from_json(const "
                   "char *, struct %s **);\n",
                   struct_name, struct_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      hfile,
      "extern LIB_EXPORT enum cdd_c_error %s_array_from_json(const char *, "
      "struct %s ***, size_t *);\n",
      struct_name, struct_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile,
                   /* LCOV_EXCL_STOP */
                   /* LCOV_EXCL_STOP */
                   "extern LIB_EXPORT enum cdd_c_error %s_to_json(const "
                   "struct %s *, char **);\n",
                   struct_name, struct_name));
  if (0) {
    CHECK_IO(fprintf(
        hfile,
        "extern LIB_EXPORT enum cdd_c_error cdd_c_parse_oauth2_token(const "
        "char *, struct %s **);\n",
        struct_name));
  }
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->utils_guard));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      hfile, "extern LIB_EXPORT enum cdd_c_error %s_cleanup(struct %s *);\n",
      struct_name, struct_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      hfile, "extern LIB_EXPORT enum cdd_c_error %s_default(struct %s **);\n",
      struct_name, struct_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile,
                   /* LCOV_EXCL_STOP */
                   /* LCOV_EXCL_STOP */
                   "extern LIB_EXPORT enum cdd_c_error %s_deepcopy(const "
                   "struct %s *, struct %s **);\n",
                   struct_name, struct_name, struct_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile,
                   /* LCOV_EXCL_STOP */
                   /* LCOV_EXCL_STOP */
                   "extern LIB_EXPORT enum cdd_c_error %s_eq(const struct "
                   "%s *, const struct %s *, int *out_eq);\n",
                   struct_name, struct_name, struct_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      hfile, "extern LIB_EXPORT int %s_debug(const struct %s *, FILE *);\n",
      struct_name, struct_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      hfile, "extern LIB_EXPORT int %s_display(const struct %s *, FILE *);\n",
      struct_name, struct_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(hfile, "struct json_object_t;\n"));
  CHECK_IO(fprintf(
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      hfile,
      "extern LIB_EXPORT enum cdd_c_error %s_from_jsonObject(const struct "
      "json_object_t *, struct %s **);\n",
      struct_name, struct_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  CHECK_IO(fprintf(
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      hfile,
      "extern LIB_EXPORT enum cdd_c_error %s_to_jsonObject(const struct %s *, "
      "struct json_object_t **);\n",
      struct_name, struct_name));
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
}
