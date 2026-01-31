/**
 * @file schema_codegen.c
 * @brief Implementation of C code generation from JSON Schema.
 * Implements a multi-pass approach to support forward declarations,
 * recursive types, and CLI-configurable build guards.
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parson.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif /* !strdup */
#define PATH_MAX _MAX_PATH
#else
#include <limits.h>
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */

#include "schema_codegen.h"

#include "code2schema.h"
#include "codegen.h"
#include "fs.h"

#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)
#define CHECK_RC(x)                                                            \
  do {                                                                         \
    const int err_code = (x);                                                  \
    if (err_code != 0)                                                         \
      return err_code;                                                         \
  } while (0)

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/*
 * Write the #ifndef guard header string to the .h file.
 */
static int print_header_guard(FILE *const hfile, const char *const basename) {
  CHECK_IO(fprintf(hfile, "#ifndef %s_H\n", basename));
  CHECK_IO(fprintf(hfile, "#define %s_H\n\n", basename));
  return 0;
}

/*
 * Close the #ifndef guard
 */
static int print_header_guard_end(FILE *const hfile,
                                  const char *const basename) {
  CHECK_IO(fprintf(hfile, "#endif /* !%s_H */\n", basename));
  return 0;
}

/* Map JSON types to C types for header struct fields */
static int print_c_type_for_schema_prop(FILE *const hfile,
                                        const char *const prop_name,
                                        const JSON_Object *const prop_obj,
                                        const JSON_Object *const schemas_obj) {
  const char *const type_str = json_object_get_string(prop_obj, "type");
  const char *const ref = json_object_get_string(prop_obj, "$ref");

  if (ref != NULL && schemas_obj != NULL) {
    const char *const ref_name = get_type_from_ref(ref);
    CHECK_IO(fprintf(hfile, "  struct %s *%s;\n", ref_name, prop_name));
    return 0;
  } else if (type_str == NULL) {
    CHECK_IO(fprintf(hfile, "  /* unknown type for %s */\n", prop_name));
  } else if (strcmp(type_str, "string") == 0) {
    CHECK_IO(fprintf(hfile, "  const char *%s;\n", prop_name));
  } else if (strcmp(type_str, "integer") == 0) {
    CHECK_IO(fprintf(hfile, "  int %s;\n", prop_name));
  } else if (strcmp(type_str, "object") == 0) {
    CHECK_IO(fprintf(hfile, "  /* object property %s */\n", prop_name));
  } else {
    CHECK_IO(fprintf(hfile, "  /* unhandled type %s for %s */\n", type_str,
                     prop_name));
  }
  return 0;
}

/*
 * Print struct declaration and function prototypes for struct.
 */
static int print_struct_declaration(FILE *const hfile,
                                    const char *const struct_name,
                                    const JSON_Object *const schema_obj,
                                    const JSON_Object *const schemas_obj,
                                    const struct CodegenConfig *const config) {
  const JSON_Object *const props =
      json_object_get_object(schema_obj, "properties");
  const size_t nprops = props ? json_object_get_count(props) : 0;
  size_t i;

  CHECK_IO(fprintf(hfile, "struct LIB_EXPORT %s {\n", struct_name));
  if (props) {
    for (i = 0; i < nprops; i++) {
      const char *const prop_name = json_object_get_name(props, i);
      const JSON_Object *const prop_obj =
          json_object_get_object(props, prop_name);
      if (!prop_obj)
        continue;
      CHECK_RC(print_c_type_for_schema_prop(hfile, prop_name, prop_obj,
                                            schemas_obj));
    }
  }
  CHECK_IO(fputs("};\n\n", hfile));

  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->json_guard));
  }
  CHECK_IO(fprintf(
      hfile,
      "extern LIB_EXPORT int %s_from_json(const char *, struct %s **);\n",
      struct_name, struct_name));
  CHECK_IO(fprintf(
      hfile, "extern LIB_EXPORT int %s_to_json(const struct %s *, char **);\n",
      struct_name, struct_name));
  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#endif /* %s */\n", config->json_guard));
  }

  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->utils_guard));
  }
  CHECK_IO(fprintf(hfile, "extern LIB_EXPORT void %s_cleanup(struct %s *);\n",
                   struct_name, struct_name));
  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#endif /* %s */\n", config->utils_guard));
  }

  return 0;
}

static int
print_root_array_declaration(FILE *const hfile, const char *const name,
                             const JSON_Object *const schema_obj,
                             const struct CodegenConfig *const config) {
  const JSON_Object *items = json_object_get_object(schema_obj, "items");
  const char *item_type = NULL, *item_ref = NULL;
  if (!items)
    return 0;

  item_type = json_object_get_string(items, "type");
  item_ref = json_object_get_string(items, "$ref");

  /* Prototypes */
  if (config && config->json_guard)
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->json_guard));

  if (item_type && strcmp(item_type, "integer") == 0) {
    CHECK_IO(fprintf(
        hfile,
        "extern LIB_EXPORT int %s_from_json(const char *, int **, size_t *);\n",
        name));
    CHECK_IO(fprintf(
        hfile,
        "extern LIB_EXPORT int %s_to_json(const int *, size_t, char **);\n",
        name));
  } else if (item_type && strcmp(item_type, "string") == 0) {
    CHECK_IO(fprintf(hfile,
                     "extern LIB_EXPORT int %s_from_json(const char *, char "
                     "***, size_t *);\n",
                     name));
    CHECK_IO(fprintf(
        hfile,
        "extern LIB_EXPORT int %s_to_json(char **const, size_t, char **);\n",
        name));
  } else if (item_ref) {
    const char *mn = get_type_from_ref(item_ref);
    CHECK_IO(fprintf(hfile,
                     "extern LIB_EXPORT int %s_from_json(const char *, struct "
                     "%s ***, size_t *);\n",
                     name, mn));
    CHECK_IO(fprintf(hfile,
                     "extern LIB_EXPORT int %s_to_json(struct %s **const, "
                     "size_t, char **);\n",
                     name, mn));
  }

  if (config && config->json_guard)
    CHECK_IO(fprintf(hfile, "#endif /* %s */\n", config->json_guard));

  if (config && config->utils_guard)
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->utils_guard));

  if (item_type && strcmp(item_type, "integer") == 0)
    CHECK_IO(fprintf(
        hfile, "extern LIB_EXPORT void %s_cleanup(int *, size_t);\n", name));
  else if (item_type && strcmp(item_type, "string") == 0)
    CHECK_IO(fprintf(
        hfile, "extern LIB_EXPORT void %s_cleanup(char **, size_t);\n", name));
  else if (item_ref)
    CHECK_IO(fprintf(
        hfile, "extern LIB_EXPORT void %s_cleanup(struct %s **, size_t);\n",
        name, get_type_from_ref(item_ref)));

  if (config && config->utils_guard)
    CHECK_IO(fprintf(hfile, "#endif /* %s */\n\n", config->utils_guard));

  return 0;
}

static int generate_header(const char *const basename,
                           JSON_Object *const schemas_obj,
                           const struct CodegenConfig *const config) {
  const size_t n = json_object_get_count(schemas_obj);
  size_t i;
  char header_filename[256];
  FILE *hfile = NULL;
  char guard_macro[128];
  size_t len = strlen(basename);

  if (len > 127)
    len = 127;

  /* Construct Guard */
  strncpy(guard_macro, basename, len);
  guard_macro[len] = 0;

  sprintf(header_filename, "%s.h", basename);
  hfile = fopen(header_filename, "w");
  if (!hfile)
    return errno ? errno : EIO;

  CHECK_RC(print_header_guard(hfile, guard_macro));
  CHECK_IO(fprintf(hfile, "#include <stdlib.h>\n#include <stdio.h>\n#include "
                          "\"lib_export.h\"\n\n"));
  CHECK_IO(fprintf(hfile, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"));

  /* PASS 1: Forward Declarations */
  for (i = 0; i < n; i++) {
    const char *const schema_name = json_object_get_name(schemas_obj, i);
    const JSON_Value *const val = json_object_get_value_at(schemas_obj, i);
    const JSON_Object *const schema_obj = json_value_get_object(val);
    const char *type_str;

    if (!schema_obj)
      continue;
    type_str = json_object_get_string(schema_obj, "type");

    if (type_str && strcmp(type_str, "object") == 0) {
      CHECK_RC(write_forward_decl(hfile, schema_name));
    }
  }
  CHECK_IO(fprintf(hfile, "\n"));

  /* PASS 2: Full Definitions */
  for (i = 0; i < n; i++) {
    const char *const schema_name = json_object_get_name(schemas_obj, i);
    const JSON_Value *const val = json_object_get_value_at(schemas_obj, i);
    const JSON_Object *const schema_obj = json_value_get_object(val);
    const char *type_str;
    if (!schema_obj)
      continue;

    type_str = json_object_get_string(schema_obj, "type");
    if (!type_str)
      continue;

    if (strcmp(type_str, "object") == 0) {
      CHECK_RC(print_struct_declaration(hfile, schema_name, schema_obj,
                                        schemas_obj, config));
    } else if (strcmp(type_str, "array") == 0) {
      /* Root Array Logic */
      CHECK_RC(
          print_root_array_declaration(hfile, schema_name, schema_obj, config));
    }
  }

  CHECK_IO(fprintf(hfile, "#ifdef __cplusplus\n}\n#endif\n\n"));
  CHECK_RC(print_header_guard_end(hfile, guard_macro));

  fclose(hfile);
  printf("Generated header: %s\n", header_filename);
  return 0;
}

static int generate_source(const char *const basename,
                           const JSON_Object *const schemas_obj,
                           const struct CodegenConfig *const config) {
  const size_t n = json_object_get_count(schemas_obj);
  size_t i;
  char source_filename[PATH_MAX];
  FILE *cfile = NULL;

  sprintf(source_filename, "%s.c", basename);
  cfile = fopen(source_filename, "w");
  if (!cfile)
    return errno ? errno : EIO;

  CHECK_IO(fprintf(
      cfile,
      "#include <stdlib.h>\n#include <string.h>\n#include <parson.h>\n#include "
      "<c89stringutils_string_extras.h>\n#include \"%s.h\"\n\n",
      basename));

  /* Iterate schemas */
  for (i = 0; i < n; i++) {
    const char *schema_name = json_object_get_name(schemas_obj, i);
    const JSON_Value *const val = json_object_get_value_at(schemas_obj, i);
    const JSON_Object *const schema_obj = json_value_get_object(val);
    const char *type_str;
    if (!schema_obj)
      continue;

    type_str = json_object_get_string(schema_obj, "type");
    if (!type_str)
      continue;

    if (strcmp(type_str, "object") == 0) {
      struct StructFields fields;
      int rc;
      if (struct_fields_init(&fields) != 0) {
        fclose(cfile);
        return ENOMEM;
      }
      rc = json_object_to_struct_fields(schema_obj, &fields, schemas_obj);
      if (rc != 0) {
        fprintf(stderr, "Failed to parse struct fields for %s\n", schema_name);
        struct_fields_free(&fields);
        fclose(cfile);
        return rc;
      }

      /* Generate struct functions */
      CHECK_RC(write_struct_from_jsonObject_func(cfile, schema_name, &fields,
                                                 config));
      CHECK_RC(write_struct_from_json_func(cfile, schema_name, config));
      CHECK_RC(write_struct_to_json_func(cfile, schema_name, &fields, config));
      CHECK_RC(write_struct_cleanup_func(cfile, schema_name, &fields, config));

      struct_fields_free(&fields);
    } else if (strcmp(type_str, "array") == 0) {
      /* Root Array Gen */
      const JSON_Object *items = json_object_get_object(schema_obj, "items");
      const char *item_type = json_object_get_string(items, "type");
      const char *item_ref = json_object_get_string(items, "$ref");
      /* Note: items might have $ref but no type field if it's an object
       * reference. fallback. */
      if (!item_type && item_ref)
        item_type = "object";

      CHECK_RC(write_root_array_from_json_func(cfile, schema_name, item_type,
                                               item_ref, config));
      CHECK_RC(write_root_array_to_json_func(cfile, schema_name, item_type,
                                             item_ref, config));
      CHECK_RC(write_root_array_cleanup_func(cfile, schema_name, item_type,
                                             item_ref, config));
    }
  }

  fclose(cfile);
  printf("Generated source: %s\n", source_filename);
  return 0;
}

int schema2code_main(int argc, char **argv) {
  const char *schema_file;
  const char *basename;
  JSON_Value *root_val = NULL;
  const JSON_Object *root_obj = NULL;
  JSON_Object *schemas_obj = NULL;
  struct CodegenConfig config;
  int ret;
  int i;

  /* Default config */
  memset(&config, 0, sizeof(config));

  if (argc < 2) {
    fprintf(stderr, "Usage schema2code: <schema.json> <basename> [opts...]\n");
    return EXIT_FAILURE;
  }
  schema_file = argv[0];
  basename = argv[1];

  /* Parse options */
  for (i = 2; i < argc; ++i) {
    if (str_starts_with(argv[i], "--guard-enum=")) {
      config.enum_guard = argv[i] + 13; /* len("--guard-enum=") */
    } else if (str_starts_with(argv[i], "--guard-json=")) {
      config.json_guard = argv[i] + 13; /* len("--guard-json=") */
    } else if (str_starts_with(argv[i], "--guard-utils=")) {
      config.utils_guard = argv[i] + 14; /* len("--guard-utils=") */
    } else {
      fprintf(stderr, "Unknown option: %s\n", argv[i]);
      return EXIT_FAILURE;
    }
  }

  root_val = json_parse_file(schema_file);
  if (!root_val) {
    fprintf(stderr, "Failed to parse JSON schema file: %s\n", schema_file);
    return EXIT_FAILURE;
  }
  root_obj = json_value_get_object(root_val);
  if (!root_obj) {
    json_value_free(root_val);
    return EXIT_FAILURE;
  }

  schemas_obj = json_object_get_object(root_obj, "components");
  if (schemas_obj)
    schemas_obj = json_object_get_object(schemas_obj, "schemas");
  if (!schemas_obj)
    schemas_obj = json_object_get_object(root_obj, "$defs");

  if (!schemas_obj) {
    fprintf(stderr,
            "Schema does not contain 'components/schemas' or '$defs'\n");
    json_value_free(root_val);
    return EXIT_FAILURE;
  }

  ret = generate_header(basename, schemas_obj, &config);
  if (ret != 0) {
    json_value_free(root_val);
    return ret;
  }
  ret = generate_source(basename, schemas_obj, &config);

  json_value_free(root_val);
  return ret;
}
