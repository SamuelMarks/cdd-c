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

#include "code2schema.h"
#include "codegen.h" /* Facade header */
#include "codegen_enum.h"
#include "codegen_json.h"
#include "codegen_struct.h"
#include "codegen_types.h"
#include "fs.h"
#include "schema_codegen.h"
#include "str_utils.h"

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

static int print_struct_declaration(FILE *const hfile,
                                    const char *const struct_name,
                                    const JSON_Object *const schema_obj,
                                    const struct CodegenConfig *const config) {
  const JSON_Object *const props =
      json_object_get_object(schema_obj, "properties");
  const size_t nprops = props ? json_object_get_count(props) : 0;
  size_t i;

  CHECK_IO(fprintf(hfile, "struct LIB_EXPORT %s {\n", struct_name));
  for (i = 0; i < nprops; i++) {
    const char *prop_name = json_object_get_name(props, i);
    const JSON_Object *p_obj = json_object_get_object(props, prop_name);
    const char *type = json_object_get_string(p_obj, "type");
    const char *ref = json_object_get_string(p_obj, "$ref");

    if (ref) {
      CHECK_IO(fprintf(hfile, "  struct %s *%s;\n",
                       c_cdd_str_after_last(ref, '/'), prop_name));
    } else if (type && strcmp(type, "string") == 0) {
      CHECK_IO(fprintf(hfile, "  const char *%s;\n", prop_name));
    } else if (type && strcmp(type, "integer") == 0) {
      CHECK_IO(fprintf(hfile, "  int %s;\n", prop_name));
    } else if (type && strcmp(type, "boolean") == 0) {
      CHECK_IO(fprintf(hfile, "  int %s;\n", prop_name));
    } else {
      CHECK_IO(fprintf(hfile, "  /* Unhandled type for %s */\n", prop_name));
    }
  }
  CHECK_IO(fputs("};\n\n", hfile));

  /* Prototypes */
  if (config->json_guard)
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->json_guard));
  CHECK_IO(fprintf(
      hfile,
      "extern LIB_EXPORT int %s_from_json(const char *, struct %s **);\n",
      struct_name, struct_name));
  CHECK_IO(fprintf(
      hfile, "extern LIB_EXPORT int %s_to_json(const struct %s *, char **);\n",
      struct_name, struct_name));
  if (config->json_guard)
    CHECK_IO(fprintf(hfile, "#endif\n"));

  if (config->utils_guard)
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->utils_guard));
  CHECK_IO(fprintf(hfile, "extern LIB_EXPORT void %s_cleanup(struct %s *);\n",
                   struct_name, struct_name));
  if (config->utils_guard)
    CHECK_IO(fprintf(hfile, "#endif\n"));

  return 0;
}

static int generate_header(const char *basename, JSON_Object *schemas_obj,
                           const struct CodegenConfig *config) {
  char fname[256];
  FILE *fp;
  size_t i, n = json_object_get_count(schemas_obj);

  sprintf(fname, "%s.h", basename);
  fp = fopen(fname, "w");
  if (!fp)
    return errno;

  CHECK_RC(print_header_guard(fp, basename));
  CHECK_IO(fprintf(fp,
                   "#include <stdlib.h>\n#include \"lib_export.h\"\n\n#ifdef "
                   "__cplusplus\nextern \"C\" {\n#endif\n\n"));

  /* Pass 1: Forward Decls */
  for (i = 0; i < n; i++) {
    const char *name = json_object_get_name(schemas_obj, i);
    const JSON_Object *s =
        json_value_get_object(json_object_get_value_at(schemas_obj, i));
    if (strcmp(json_object_get_string(s, "type"), "object") == 0) {
      CHECK_RC(write_forward_decl(fp, name));
    }
  }
  fprintf(fp, "\n");

  /* Pass 2: Definitions */
  for (i = 0; i < n; i++) {
    const char *name = json_object_get_name(schemas_obj, i);
    const JSON_Object *s =
        json_value_get_object(json_object_get_value_at(schemas_obj, i));
    const char *type = json_object_get_string(s, "type");

    if (strcmp(type, "object") == 0) {
      CHECK_RC(print_struct_declaration(fp, name, s, config));
    }
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
  size_t i, n = json_object_get_count(schemas_obj);
  struct CodegenJsonConfig json_cfg;
  struct CodegenStructConfig struct_cfg;

  if (config) {
    json_cfg.guard_macro = config->json_guard;
    struct_cfg.guard_macro = config->utils_guard;
  } else {
    json_cfg.guard_macro = NULL;
    struct_cfg.guard_macro = NULL;
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

  for (i = 0; i < n; i++) {
    const char *name = json_object_get_name(schemas_obj, i);
    const JSON_Object *s =
        json_value_get_object(json_object_get_value_at(schemas_obj, i));
    const char *type = json_object_get_string(s, "type");

    if (strcmp(type, "object") == 0) {
      struct StructFields sf;
      if (struct_fields_init(&sf) != 0) {
        fclose(fp);
        return ENOMEM;
      }
      if (json_object_to_struct_fields(s, &sf, schemas_obj) != 0) {
        struct_fields_free(&sf);
        fclose(fp);
        return ENOMEM; /* Parse error */
      }

      CHECK_RC(write_struct_from_jsonObject_func(fp, name, &sf, &json_cfg));
      CHECK_RC(write_struct_from_json_func(fp, name, &json_cfg));
      CHECK_RC(write_struct_to_json_func(fp, name, &sf, &json_cfg));
      CHECK_RC(write_struct_cleanup_func(fp, name, &sf, &struct_cfg));

      struct_fields_free(&sf);
    }
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
