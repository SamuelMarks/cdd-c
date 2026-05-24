/**
 * @file schema_codegen.c
 * @brief Implementation of C code generation from JSON Schema.
 *
 * Implements a multi-pass approach to support forward declarations and
 * delegates struct/enum generation to specialized modules.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parson.h>

#include "classes/emit/enum.h"
#include "classes/emit/json.h"
#include "classes/emit/form.h"
#include "classes/emit/jwt.h"
#include "classes/emit/oauth2_error.h"
#include "classes/emit/schema_codegen.h"
#include "classes/emit/struct.h"
#include "classes/emit/types.h"
#include "classes/parse/code2schema.h"
#include "functions/emit/codegen.h" /* Facade header */
#include "functions/parse/fs.h"
#include "functions/parse/str.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#define PATH_MAX _MAX_PATH
#else
#include <limits.h>
#endif
/* clang-format on */

#ifdef CDD_BUILD_TESTS
#include <stdarg.h>
int g_schema_fail_io_after = -1;
int g_schema_io_calls = 0;
static int cdd_fprintf_hook(FILE *stream, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
static int cdd_fprintf_hook(FILE *stream, const char *format, ...) {
  va_list args;
  int rc;
  printf("hook: after=%d calls=%d\n", g_schema_fail_io_after,
         g_schema_io_calls);
  if (g_schema_fail_io_after >= 0 &&
      ++g_schema_io_calls > g_schema_fail_io_after)
    return -1;
  va_start(args, format);
  rc = vfprintf(stream, format, args);
  va_end(args);
  return rc;
}
#define FPRINTF_HOOK cdd_fprintf_hook
#else
#define FPRINTF_HOOK fprintf
#endif

#ifndef PATH_MAX
/** @brief PATH_MAX definition */
#define PATH_MAX 4096
#endif

/** @brief CHECK_IO definition */
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)
/** @brief CHECK_RC definition */
#define CHECK_RC(x)                                                            \
  do {                                                                         \
    int err = (x);                                                             \
    if (err != 0)                                                              \
      return err;                                                              \
  } while (0)

/* Write Header Guard Start */
static int print_header_guard(FILE *hfile, const char *basename) {
  CHECK_IO(FPRINTF_HOOK(hfile, "#ifndef %s_H\n", basename));
  CHECK_IO(FPRINTF_HOOK(hfile, "#define %s_H\n\n", basename));
  return 0;
}

/* Write Header Guard End */
/**
 * @brief Executes the print header guard end operation.
 */
static int print_header_guard_end(FILE *hfile, const char *basename) {
  CHECK_IO(FPRINTF_HOOK(hfile, "#endif /* !%s_H */\n", basename));
  return 0;
}

/**
 * @brief Generates header.
 */
int generate_header(const char *prefix, const char *basename,
                    JSON_Object *schemas_obj,
                    const struct CodegenConfig *config) {
  char fname[256];
  FILE *fp;
  size_t i;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(fname, sizeof(fname), "%s.h", prefix);
#else
  sprintf(fname, "%s.h", prefix);
#endif
#if defined(_MSC_VER)
  if (fopen_s(&fp, fname, "w") != 0)
    fp = NULL;
#else
#if defined(_MSC_VER)
  fopen_s(&fp, fname, "w");
#else
  fp = fopen(fname, "w");
#endif
#endif
  if (!fp)
    return errno;

  CHECK_RC(print_header_guard(fp, basename));
  CHECK_IO(FPRINTF_HOOK(fp, "#include <stdlib.h>\n"
                            "#include \"lib_export.h\"\n\n"
                            "#if defined(_MSC_VER) && _MSC_VER < 1600\n"
                            "typedef signed __int8 int8_t;\n"
                            "typedef unsigned __int8 uint8_t;\n"
                            "typedef signed __int16 int16_t;\n"
                            "typedef unsigned __int16 uint16_t;\n"
                            "typedef signed __int32 int32_t;\n"
                            "typedef unsigned __int32 uint32_t;\n"
                            "typedef signed __int64 int64_t;\n"
                            "typedef unsigned __int64 uint64_t;\n"
                            "#else\n"
                            "#include <stdint.h>\n"
                            "#endif\n\n"
                            "#if defined(_MSC_VER) && _MSC_VER < 1800\n"
                            "#if !defined(__cplusplus)\n"
                            "#ifndef bool\n"
                            "#define bool unsigned char\n"
                            "#endif\n"
                            "#ifndef true\n"
                            "#define true 1\n"
                            "#endif\n"
                            "#ifndef false\n"
                            "#define false 0\n"
                            "#endif\n"
                            "#endif\n"
                            "#else\n"
                            "#include <stdbool.h>\n"
                            "#endif\n\n"
                            "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"));

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
      CHECK_RC(write_enum_declaration_h(fp, name, &sf, config));
    } else if (sf.is_union) {
      CHECK_RC(write_union_declaration_h(fp, name, &sf, config));
    } else if (is_object_schema) {
      CHECK_RC(write_struct_declaration_h(fp, name, &sf, config));
    }

    struct_fields_free(&sf);
  }

  CHECK_IO(FPRINTF_HOOK(fp, "#ifdef __cplusplus\n}\n#endif\n"));
  CHECK_RC(print_header_guard_end(fp, basename));
  fclose(fp);
  return 0;
}

/**
 * @brief Generates source.
 */
int generate_source(const char *prefix, const char *basename,
                    JSON_Object *schemas_obj,
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

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(fname, sizeof(fname), "%s.c", prefix);
#else
  sprintf(fname, "%s.c", prefix);
#endif
#if defined(_MSC_VER)
  if (fopen_s(&fp, fname, "w") != 0)
    fp = NULL;
#else
#if defined(_MSC_VER)
  fopen_s(&fp, fname, "w");
#else
  fp = fopen(fname, "w");
#endif
#endif
  if (!fp)
    return errno;

  CHECK_IO(
      FPRINTF_HOOK(fp,
                   "#include <errno.h>\\n#include <stdio.h>\\n#include "
                   "<stdlib.h>\\n#include "
                   "<string.h>\\n#include <parson.h>\\n#include "
                   "<c89stringutils_string_extras.h>\\n#include \"%s.h\"\\n\\n",
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
      CHECK_RC(write_struct_array_from_json_func(fp, name, &json_cfg));
      CHECK_RC(write_struct_to_json_func(fp, name, &sf, &json_cfg));
      CHECK_RC(write_struct_to_form_urlencoded_func(fp, name, &sf));
      if (strcmp(name, "OAuth2Error") == 0) {
        CHECK_RC(write_oauth2_error_parser_func(fp, name, &sf));
      }
      if (strcmp(name, "JwtPayload") == 0) {
        CHECK_RC(write_struct_from_jwt_func(fp, name, &sf));
      }
      if (strcmp(name, "OAuth2TokenResponse") == 0) {
        CHECK_RC(write_struct_from_json_standalone_func(fp, name, &sf));
      }
      CHECK_RC(write_struct_cleanup_func(fp, name, &sf, &struct_cfg));
      CHECK_RC(write_struct_default_func(fp, name, &sf, &struct_cfg));
      CHECK_RC(write_struct_deepcopy_func(fp, name, &sf, &struct_cfg));
      CHECK_RC(write_struct_eq_func(fp, name, &sf, &struct_cfg));
    }

    struct_fields_free(&sf);
  }
  fclose(fp);
  return 0;
}

/**
 * @brief Executes the schema2code main operation.
 */
int schema2code_main(int argc, char **argv) {
  const char *schema_file, *prefix;
  char *basename = NULL;
  struct CodegenConfig config = {0};
  JSON_Value *root = NULL;
  JSON_Object *schemas = NULL;
  int i;

  if (argc < 2)
    return EXIT_FAILURE;
  schema_file = argv[0];
  prefix = argv[1];
  if (get_basename(prefix, &basename) != 0)
    return EXIT_FAILURE;

  for (i = 2; i < argc; ++i) {
    int starts = 0;
    if ((str_starts_with(argv[i], "--guard-enum=", &starts), starts))
      config.enum_guard = argv[i] + 13;
    else if ((str_starts_with(argv[i], "--guard-json=", &starts), starts))
      config.json_guard = argv[i] + 13;
    else if ((str_starts_with(argv[i], "--guard-utils=", &starts), starts))
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
    free(basename);
    return EXIT_FAILURE;
  }

  if (generate_header(prefix, basename, schemas, &config) != 0) {
    json_value_free(root);
    free(basename);
    return EXIT_FAILURE;
  }
  if (generate_source(prefix, basename, schemas, &config) != 0) {
    json_value_free(root);
    free(basename);
    return EXIT_FAILURE;
  }

  json_value_free(root);
  free(basename);
  return EXIT_SUCCESS;
}
