/**
 * @file schema_codegen.c
 * @brief Implementation of C code generation from JSON Schema.
 *
 * Implements a multi-pass approach to support forward declarations and
 * delegates struct/enum generation to specialized modules.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parson.h>

#include "classes/emit_enum.h"
#include "classes/emit_json.h"
#include "classes/emit_schema_codegen.h"
#include "classes/emit_struct.h"
#include "classes/emit_types.h"
#include "classes/parse_code2schema.h"
#include "functions/emit_codegen.h" /* Facade header */
#include "functions/parse_fs.h"
#include "functions/parse_str.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#define PATH_MAX _MAX_PATH
#else
#include <limits.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)
#define CHECK_RC(x)                                                            \
  do {                                                                         \
    int err = (x);                                                             \
    if (err != 0)                                                              \
      return err;                                                              \
  } while (0)

/* Write Header Guard Start */
static int print_header_guard(FILE *const hfile, const char *const basename) {
  CHECK_IO(fprintf(hfile, "#ifndef %s_H\n", basename));
  CHECK_IO(fprintf(hfile, "#define %s_H\n\n", basename));
  return 0;
}

/* Write Header Guard End */
static int print_header_guard_end(FILE *const hfile,
                                  const char *const basename) {
  CHECK_IO(fprintf(hfile, "#endif /* !%s_H */\n", basename));
  return 0;
}

static int print_enum_declaration(FILE *const hfile,
                                  const char *const enum_name,
                                  const struct StructFields *const sf,
                                  const struct CodegenConfig *const config) {
  size_t i;
  if (!hfile || !enum_name || !sf)
    return EINVAL;

  CHECK_IO(fprintf(hfile, "enum LIB_EXPORT %s {\n", enum_name));
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

static int print_union_declaration(FILE *const hfile,
                                   const char *const union_name,
                                   const struct StructFields *const sf,
                                   const struct CodegenConfig *const config) {
  size_t i;
  if (!hfile || !union_name || !sf)
    return EINVAL;

  CHECK_IO(fprintf(hfile, "enum %s_tag {\n", union_name));
  CHECK_IO(fprintf(hfile, "  %s_UNKNOWN = 0,\n", union_name));
  for (i = 0; i < sf->size; ++i) {
    CHECK_IO(fprintf(hfile, "  %s_%s,\n", union_name, sf->fields[i].name));
  }
  CHECK_IO(fputs("};\n\n", hfile));

  CHECK_IO(fprintf(hfile, "struct LIB_EXPORT %s {\n", union_name));
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
      CHECK_IO(fprintf(hfile, "    enum %s %s;\n", get_type_from_ref(r), n));
    } else if (strcmp(t, "object") == 0) {
      CHECK_IO(fprintf(hfile, "    struct %s *%s;\n", get_type_from_ref(r), n));
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
        CHECK_IO(
            fprintf(hfile, "      struct %s **%s;\n", get_type_from_ref(r), n));
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

static int print_struct_declaration(FILE *const hfile,
                                    const char *const struct_name,
                                    const struct StructFields *const sf,
                                    const struct CodegenConfig *const config) {
  size_t i;

  CHECK_IO(fprintf(hfile, "struct LIB_EXPORT %s {\n", struct_name));
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
      CHECK_IO(fprintf(hfile, "  enum %s %s;\n", get_type_from_ref(r), n));
    } else if (strcmp(t, "object") == 0) {
      CHECK_IO(fprintf(hfile, "  struct %s *%s;\n", get_type_from_ref(r), n));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(fprintf(hfile, "  size_t n_%s;\n", n));
      if (strcmp(r, "string") == 0) {
        CHECK_IO(fprintf(hfile, "  char **%s;\n", n));
      } else if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
        CHECK_IO(fprintf(hfile, "  int *%s;\n", n));
      } else if (strcmp(r, "number") == 0) {
        CHECK_IO(fprintf(hfile, "  double *%s;\n", n));
      } else {
        CHECK_IO(
            fprintf(hfile, "  struct %s **%s;\n", get_type_from_ref(r), n));
      }
    } else {
      CHECK_IO(fprintf(hfile, "  void *%s;\n", n));
    }
  }
  CHECK_IO(fputs("};\n\n", hfile));

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
  if (config && config->json_guard)
    CHECK_IO(fprintf(hfile, "#endif\n"));

  if (config && config->utils_guard)
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->utils_guard));
  CHECK_IO(fprintf(hfile, "extern LIB_EXPORT void %s_cleanup(struct %s *);\n",
                   struct_name, struct_name));
  if (config && config->utils_guard)
    CHECK_IO(fprintf(hfile, "#endif\n"));

  return 0;
}

static int generate_header(const char *basename, JSON_Object *schemas_obj,
                           const struct CodegenConfig *config) {
  char fname[256];
  FILE *fp;
  size_t i;

  sprintf(fname, "%s.h", basename);
  fp = fopen(fname, "w");
  if (!fp)
    return errno;

  CHECK_RC(print_header_guard(fp, basename));
  CHECK_IO(fprintf(fp,
                   "#include <stdlib.h>\n#include \"lib_export.h\"\n\n#ifdef "
                   "__cplusplus\nextern \"C\" {\n#endif\n\n"));

  /* Pass 1: Forward Decls */
  for (i = 0; i < json_object_get_count(schemas_obj); i++) {
    const char *name = json_object_get_name(schemas_obj, i);
    const JSON_Object *s =
        json_value_get_object(json_object_get_value_at(schemas_obj, i));
    struct StructFields sf;
    int is_object_schema = 0;
    const char *type = json_object_get_string(s, "type");
    const JSON_Object *props = json_object_get_object(s, "properties");

    if (struct_fields_init(&sf) != 0) {
      fclose(fp);
      return ENOMEM;
    }
    if (json_object_to_struct_fields_ex_codegen(s, &sf, schemas_obj, name) !=
        0) {
      struct_fields_free(&sf);
      fclose(fp);
      return ENOMEM;
    }

    is_object_schema =
        (type && strcmp(type, "object") == 0) || props != NULL || sf.size > 0;

    if (sf.is_union || is_object_schema) {
      CHECK_RC(write_forward_decl(fp, name));
    }
    struct_fields_free(&sf);
  }
  fprintf(fp, "\n");

  /* Pass 2: Definitions */
  for (i = 0; i < json_object_get_count(schemas_obj); i++) {
    const char *name = json_object_get_name(schemas_obj, i);
    const JSON_Object *s =
        json_value_get_object(json_object_get_value_at(schemas_obj, i));
    struct StructFields sf;
    int is_object_schema = 0;
    const char *type = json_object_get_string(s, "type");
    const JSON_Object *props = json_object_get_object(s, "properties");

    if (struct_fields_init(&sf) != 0) {
      fclose(fp);
      return ENOMEM;
    }
    if (json_object_to_struct_fields_ex_codegen(s, &sf, schemas_obj, name) !=
        0) {
      struct_fields_free(&sf);
      fclose(fp);
      return ENOMEM;
    }

    is_object_schema =
        (type && strcmp(type, "object") == 0) || props != NULL || sf.size > 0;

    if (sf.is_enum) {
      CHECK_RC(print_enum_declaration(fp, name, &sf, config));
    } else if (sf.is_union) {
      CHECK_RC(print_union_declaration(fp, name, &sf, config));
    } else if (is_object_schema) {
      CHECK_RC(print_struct_declaration(fp, name, &sf, config));
    }

    struct_fields_free(&sf);
  }

  CHECK_IO(fprintf(fp, "#ifdef __cplusplus\n}\n#endif\n"));
  CHECK_RC(print_header_guard_end(fp, basename));
  fclose(fp);
  return 0;
}

static int generate_source(const char *basename, JSON_Object *schemas_obj,
                           const struct CodegenConfig *config) {
  char fname[256];
  FILE *fp;
  size_t i;
  struct CodegenJsonConfig json_cfg;
  struct CodegenStructConfig struct_cfg;
  struct CodegenTypesConfig types_cfg;
  struct CodegenEnumConfig enum_cfg;

  if (config) {
    json_cfg.guard_macro = config->json_guard;
    struct_cfg.guard_macro = config->utils_guard;
    types_cfg.json_guard = config->json_guard;
    types_cfg.utils_guard = config->utils_guard;
    enum_cfg.guard_macro = config->enum_guard;
  } else {
    json_cfg.guard_macro = NULL;
    struct_cfg.guard_macro = NULL;
    types_cfg.json_guard = NULL;
    types_cfg.utils_guard = NULL;
    enum_cfg.guard_macro = NULL;
  }

  sprintf(fname, "%s.c", basename);
  fp = fopen(fname, "w");
  if (!fp)
    return errno;

  CHECK_IO(fprintf(
      fp,
      "#include <stdlib.h>\n#include <string.h>\n#include <parson.h>\n#include "
      "<c89stringutils_string_extras.h>\n#include \"%s.h\"\n\n",
      basename));

  for (i = 0; i < json_object_get_count(schemas_obj); i++) {
    const char *name = json_object_get_name(schemas_obj, i);
    const JSON_Object *s =
        json_value_get_object(json_object_get_value_at(schemas_obj, i));
    const char *type = json_object_get_string(s, "type");
    const JSON_Object *props = json_object_get_object(s, "properties");
    struct StructFields sf;
    int is_object_schema = 0;

    if (struct_fields_init(&sf) != 0) {
      fclose(fp);
      return ENOMEM;
    }
    if (json_object_to_struct_fields_ex_codegen(s, &sf, schemas_obj, name) !=
        0) {
      struct_fields_free(&sf);
      fclose(fp);
      return ENOMEM; /* Parse error */
    }

    is_object_schema =
        (type && strcmp(type, "object") == 0) || props != NULL || sf.size > 0;

    if (sf.is_enum) {
      CHECK_RC(write_enum_to_str_func(fp, name, &sf.enum_members, &enum_cfg));
      CHECK_RC(write_enum_from_str_func(fp, name, &sf.enum_members, &enum_cfg));
    } else if (sf.is_union) {
      CHECK_RC(write_union_from_jsonObject_func(fp, name, &sf, &types_cfg));
      CHECK_RC(write_union_from_json_func(fp, name, &sf, &types_cfg));
      CHECK_RC(write_union_to_json_func(fp, name, &sf, &types_cfg));
      CHECK_RC(write_union_cleanup_func(fp, name, &sf, &types_cfg));
    } else if (is_object_schema) {
      CHECK_RC(write_struct_from_jsonObject_func(fp, name, &sf, &json_cfg));
      CHECK_RC(write_struct_from_json_func(fp, name, &json_cfg));
      CHECK_RC(write_struct_to_json_func(fp, name, &sf, &json_cfg));
      CHECK_RC(write_struct_cleanup_func(fp, name, &sf, &struct_cfg));
    }

    struct_fields_free(&sf);
  }
  fclose(fp);
  return 0;
}

int schema2code_main(int argc, char **argv) {
  const char *schema_file, *basename;
  struct CodegenConfig config = {0};
  JSON_Value *root = NULL;
  JSON_Object *schemas = NULL;
  int i;

  if (argc < 2)
    return EXIT_FAILURE;
  schema_file = argv[0];
  basename = argv[1];

  for (i = 2; i < argc; ++i) {
    if (str_starts_with(argv[i], "--guard-enum="))
      config.enum_guard = argv[i] + 13;
    else if (str_starts_with(argv[i], "--guard-json="))
      config.json_guard = argv[i] + 13;
    else if (str_starts_with(argv[i], "--guard-utils="))
      config.utils_guard = argv[i] + 14;
  }

  root = json_parse_file(schema_file);
  if (!root)
    return EXIT_FAILURE;

  schemas = json_object_get_object(json_value_get_object(root), "components");
  if (schemas)
    schemas = json_object_get_object(schemas, "schemas");
  if (!schemas)
    schemas = json_object_get_object(json_value_get_object(root), "$defs");

  if (!schemas) {
    json_value_free(root);
    return EXIT_FAILURE;
  }

  if (generate_header(basename, schemas, &config) != 0) {
    json_value_free(root);
    return EXIT_FAILURE;
  }
  if (generate_source(basename, schemas, &config) != 0) {
    json_value_free(root);
    return EXIT_FAILURE;
  }

  json_value_free(root);
  return EXIT_SUCCESS;
}
