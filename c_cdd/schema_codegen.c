/**
 * @file schema_codegen.c
 * @brief Implementation of C code generation from JSON Schema.
 * Refactored to ensure strict error propagation and memory safety.
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
/* $ref is a JSON pointer: #/components/schemas/TypeName */
static int print_c_type_for_schema_prop(FILE *const hfile,
                                        const char *const prop_name,
                                        const JSON_Object *const prop_obj,
                                        const JSON_Object *const schemas_obj) {
  const char *const type_str = json_object_get_string(prop_obj, "type");
  const char *const ref = json_object_get_string(prop_obj, "$ref");

  if (ref != NULL && schemas_obj != NULL) {
    const char *const ref_name = get_type_from_ref(ref);
    const JSON_Object *ref_schema =
        json_object_get_object(schemas_obj, ref_name);
    if (ref_schema) {
      const char *const ref_type_str =
          json_object_get_string(ref_schema, "type");
      if (ref_type_str && strcmp(ref_type_str, "string") == 0 &&
          json_object_has_value(ref_schema, "enum")) {
        CHECK_IO(fprintf(hfile, "  enum %s %s;\n", ref_name, prop_name));
        return 0;
      }
    }
    /* Default for ref is a pointer to a struct */
    CHECK_IO(fprintf(hfile, "  struct %s *%s;\n", ref_name, prop_name));
    return 0;
  } else if (type_str == NULL) {
    CHECK_IO(fprintf(hfile, "  /* unknown type for %s */\n", prop_name));
  } else if (strcmp(type_str, "string") == 0) {
    CHECK_IO(fprintf(hfile, "  const char *%s;\n", prop_name));
  } else if (strcmp(type_str, "integer") == 0) {
    CHECK_IO(fprintf(hfile, "  int %s;\n", prop_name));
  } else if (strcmp(type_str, "number") == 0) {
    CHECK_IO(fprintf(hfile, "  double %s;\n", prop_name));
  } else if (strcmp(type_str, "boolean") == 0) {
    CHECK_IO(fprintf(hfile, "  int %s;\n", prop_name));
  } else if (strcmp(type_str, "object") == 0) {
    CHECK_IO(
        fprintf(hfile, "  /* object property (unresolved) %s */\n", prop_name));
  } else if (strcmp(type_str, "array") == 0) {
    /* New: handle array type fields with $ref items */
    const JSON_Object *const items_obj =
        json_object_get_object(prop_obj, "items");
    const char *items_ref = NULL;
    if (items_obj != NULL)
      items_ref = json_object_get_string(items_obj, "$ref");

    if (items_ref != NULL) {
      const char *const last_slash = strrchr(items_ref, '/');
      const char *const ref_name = last_slash ? last_slash + 1 : items_ref;
      CHECK_IO(fprintf(hfile, "  struct %s *%s;\n", ref_name, prop_name));
    } else {
      CHECK_IO(
          fprintf(hfile, "  /* array of unknown items for %s */\n", prop_name));
    }
  } else {
    CHECK_IO(fprintf(hfile, "  /* unhandled type %s for %s */\n", type_str,
                     prop_name));
  }
  return 0;
}

/*
 * Write enum declaration and function prototypes to header.
 * The JSON `enum` array must be strings.
 */
static int print_enum_declaration(FILE *const hfile,
                                  const char *const enum_name,
                                  const JSON_Array *const enum_values) {
  size_t i;
  const size_t n = json_array_get_count(enum_values);
  int has_unknown = 0;

  CHECK_IO(fprintf(hfile, "enum LIB_EXPORT %s {\n", enum_name));

  for (i = 0; i < n; i++) {
    const char *val = json_array_get_string(enum_values, i);
    if (!val)
      continue;

    if (strcmp(val, "UNKNOWN") == 0)
      has_unknown = 1;

    CHECK_IO(fprintf(hfile, "  %s_%s", enum_name, val));
    if (i + 1 < n)
      CHECK_IO(fputs(",", hfile));
  }

  /* Add UNKNOWN = -1 if it's not present */
  if (!has_unknown)
    CHECK_IO(fprintf(hfile, ",\n  %s_UNKNOWN = -1\n", enum_name));
  else
    CHECK_IO(fputc('\n', hfile));

  CHECK_IO(fputs("};\n", hfile));

  /* Declare enum related functions */
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_to_str(enum %s e, char "
                   "**str_out);\n",
                   enum_name, enum_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_from_str(const char *str, enum %s "
                   "*e);\n\n",
                   enum_name, enum_name));
  return 0;
}

/*
 * Print struct declaration and function prototypes for struct.
 * Prints required fields in required array.
 */
static int print_struct_declaration(FILE *const hfile,
                                    const char *const struct_name,
                                    const JSON_Object *const schema_obj,
                                    const JSON_Object *const schemas_obj) {
  const JSON_Object *const props =
      json_object_get_object(schema_obj, "properties");
  const size_t nprops = props ? json_object_get_count(props) : 0;
  size_t i;

  CHECK_IO(fprintf(hfile, "struct LIB_EXPORT %s {\n", struct_name));
  /* or if no properties: empty struct */
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

  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_debug(const struct %s *, FILE "
                   "*);\n\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_deepcopy(const struct %s *, "
                   "struct %s **);\n",
                   struct_name, struct_name, struct_name));
  CHECK_IO(fprintf(hfile, "extern LIB_EXPORT int %s_default(struct %s **);\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_display(const struct %s *, FILE "
                   "*);\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_eq(const struct %s *, const "
                   "struct %s *);\n\n",
                   struct_name, struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_from_json(const char *, struct %s "
                   "**);\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_from_jsonObject(const JSON_Object "
                   "*, "
                   "struct %s **);\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_to_json(const struct %s *, char "
                   "**);\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(hfile, "extern LIB_EXPORT void %s_cleanup(struct %s *);\n",
                   struct_name, struct_name));
  return 0;
}

/*
 * Write header file <basename>.h for all schemas in JSON 'schemas_obj'.
 * 'schemas_obj' refers to "components"/"schemas" or "$defs" object.
 */
static int generate_header(const char *const basename,
                           JSON_Object *const schemas_obj) {
  const size_t n = json_object_get_count(schemas_obj);
  size_t i;

  char header_filename[256];
  FILE *hfile = NULL;

  char guard_macro[128];
  size_t j;
  size_t len = strlen(basename);
  if (len > 127)
    len = 127; /* Cap length */

  for (j = 0; j < len; j++) {
    const char c = basename[j];
    if (c >= 'a' && c <= 'z')
      guard_macro[j] = (char)(c - ('a' - 'A'));
    else if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
      guard_macro[j] = c;
    else
      guard_macro[j] = '_';
  }
  guard_macro[j] = 0;

  /* Check for truncation or snprintf failure, C89 doesn't have snprintf in std,
   * but codebase has string_extras or platform */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(header_filename, sizeof(header_filename), "%s.h", basename);
  {
    errno_t err = fopen_s(&hfile, header_filename, "w, ccs=UTF-8");
    if (err != 0 || hfile == NULL) {
      if (err == EINVAL)
        return EINVAL;
      return errno ? errno : EIO;
    }
  }
#else
  snprintf(header_filename, sizeof(header_filename), "%s.h", basename);
  hfile = fopen(header_filename, "w");
  if (!hfile) {
    return errno ? errno : EIO;
  }
#endif

  CHECK_RC(print_header_guard(hfile, guard_macro));
  CHECK_IO(fprintf(hfile, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"));

  CHECK_IO(fprintf(hfile, "#include <stdlib.h>\n"
                          "#include <stdbool.h>\n"
                          "#include <stdio.h>\n\n"
                          "#include <parson.h>\n\n"
                          "#include \"lib_export.h\"\n\n"));

  /* Iterate schemas */
  for (i = 0; i < n; i++) {
    const char *const schema_name = json_object_get_name(schemas_obj, i);
    const JSON_Value *const val = json_object_get_value_at(schemas_obj, i);
    const JSON_Object *const schema_obj = json_value_get_object(val);
    const char *type_str;
    if (!schema_obj)
      continue;

    type_str = json_object_get_string(schema_obj, "type");

    if (!type_str) {
      continue;
    } else if (strcmp(type_str, "array") == 0) {
      fprintf(stderr, "Skipping top-level array schema: %s\n", schema_name);
    } else if (strcmp(type_str, "string") == 0) {
      const JSON_Array *const enum_arr =
          json_object_get_array(schema_obj, "enum");
      if (enum_arr != NULL) {
        CHECK_RC(print_enum_declaration(hfile, schema_name, enum_arr));
        continue;
      }
    } else if (strcmp(type_str, "object") == 0) {
      CHECK_RC(print_struct_declaration(hfile, schema_name, schema_obj,
                                        schemas_obj));
    }
  }

  CHECK_IO(fprintf(hfile, "#ifdef __cplusplus\n}\n#endif\n\n"));
  CHECK_RC(print_header_guard_end(hfile, guard_macro));

  fclose(hfile);
  printf("Generated header: %s\n", header_filename);
  return 0;
}

/*
 * Generate <basename>.c with all implementations.
 */
static int generate_source(const char *const basename,
                           const JSON_Object *const schemas_obj) {
  const size_t n = json_object_get_count(schemas_obj);
  size_t i;
  char source_filename[PATH_MAX];
  FILE *cfile = NULL;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(source_filename, sizeof(source_filename), "%s.c", basename);
  {
    errno_t err = fopen_s(&cfile, source_filename, "w, ccs=UTF-8");
    if (err != 0 || cfile == NULL) {
      return (int)err;
    }
  }
#else
  snprintf(source_filename, sizeof(source_filename), "%s.c", basename);
  cfile = fopen(source_filename, "w");
  if (!cfile) {
    return errno ? errno : EIO;
  }
#endif

  CHECK_IO(fputs(
      "#include <stdlib.h>\n"
      "#include <string.h>\n"
      "#include <stdio.h>\n\n"
      "#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)\n"
      "#else\n"
      "#include <sys/errno.h>\n"
      "#endif\n"
      "#include <parson.h>\n\n"
      "#include <c89stringutils_string_extras.h>\n\n",
      cfile));

  {
    char *base_name_str = NULL;
    CHECK_RC(get_basename(basename, &base_name_str));
    CHECK_IO(fprintf(cfile, "#include \"%s.h\"\n\n", base_name_str));
    free(base_name_str);
  }

  CHECK_IO(fprintf(
      cfile,
      "/* Helper for debug: quote string or replace null with '(null)' */\n"
      "static int quote_or_null(const char *s, char **out) {\n"
      "  size_t n;\n"
      "  size_t i;\n"
      "  char *buf;\n"
      "  if (s == NULL) {\n"
      "    *out = strdup(\"(null)\");\n"
      "    return *out == NULL ? ENOMEM : 0;\n"
      "  }\n"
      "  n = strlen(s);\n"
      "  buf = (char *)malloc(n + 3);\n"
      "  if (!buf) return ENOMEM;\n"
      "  buf[0] = '\"';\n"
      "  for (i = 0; i < n; i++) buf[i + 1] = s[i];\n"
      "  buf[n + 1] = '\"';\n"
      "  buf[n + 2] = '\\0';\n"
      "  *out = buf;\n"
      "  return 0;\n"
      "}\n\n"));

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

    if (strcmp(type_str, "string") == 0) {
      /* Possibly enum */
      const JSON_Array *const enum_arr =
          json_object_get_array(schema_obj, "enum");
      if (enum_arr != NULL) {
        struct EnumMembers em;
        int rc = json_array_to_enum_members(enum_arr, &em);
        if (rc == 0) {
          CHECK_RC(write_enum_to_str_func(cfile, schema_name, &em));
          CHECK_RC(write_enum_from_str_func(cfile, schema_name, &em));
          enum_members_free(&em);
        } else {
          fprintf(stderr,
                  "Failed to convert enum JSON array to EnumMembers for %s "
                  "(rc=%d)\n",
                  schema_name, rc);
          return rc;
        }
      }
      continue;
    }

    if (strcmp(type_str, "object") == 0) {
      struct StructFields fields;
      int rc = json_object_to_struct_fields(schema_obj, &fields, schemas_obj);
      if (rc != 0) {
        fprintf(stderr, "Failed to parse struct fields for %s (rc=%d)\n",
                schema_name, rc);
        return rc;
      }

      CHECK_RC(write_struct_debug_func(cfile, schema_name, &fields));
      CHECK_RC(write_struct_deepcopy_func(cfile, schema_name, &fields));
      CHECK_RC(write_struct_default_func(cfile, schema_name, &fields));
      CHECK_RC(write_struct_display_func(cfile, schema_name, &fields));
      CHECK_RC(write_struct_eq_func(cfile, schema_name, &fields));
      CHECK_RC(write_struct_from_jsonObject_func(cfile, schema_name, &fields));
      CHECK_RC(write_struct_from_json_func(cfile, schema_name));
      CHECK_RC(write_struct_to_json_func(cfile, schema_name, &fields));
      CHECK_RC(write_struct_cleanup_func(cfile, schema_name, &fields));

      /* Free fields data */
      if (fields.fields) {
        free(fields.fields);
      }
    }
  }

  fclose(cfile);
  printf("Generated source: %s\n", source_filename);
  return 0;
}

/*
 * argv[0] - schema filename
 * argv[1] - output basename
 */
int schema2code_main(int argc, char **argv) {
  const char *schema_file;
  const char *basename;
  JSON_Value *root_val = NULL;
  const JSON_Object *root_obj = NULL;
  JSON_Object *schemas_obj = NULL;
  int ret;

  if (argc != 2) {
    fprintf(stderr, "Usage schema2code: <schema.json> <basename>\n");
    return EXIT_FAILURE;
  }
  schema_file = argv[0];
  basename = argv[1];

  /* Parse schema file */
  root_val = json_parse_file(schema_file);
  if (!root_val) {
    fprintf(stderr, "Failed to parse JSON schema file: %s\n", schema_file);
    return EXIT_FAILURE;
  }
  root_obj = json_value_get_object(root_val);
  if (!root_obj) {
    fprintf(stderr, "Invalid JSON schema document\n");
    json_value_free(root_val);
    return EXIT_FAILURE;
  }

  /* Support OpenAPI style with components/schemas or JSON Schema $defs */
  schemas_obj = json_object_get_object(root_obj, "components");
  if (schemas_obj)
    schemas_obj = json_object_get_object(schemas_obj, "schemas");
  if (!schemas_obj)
    schemas_obj = json_object_get_object(root_obj, "$defs");

  if (!schemas_obj) {
    fprintf(stderr,
            "Schema does not contain 'components/schemas' or '$defs' object\n");
    json_value_free(root_val);
    return EXIT_FAILURE;
  }

  ret = generate_header(basename, schemas_obj);
  if (ret != 0) {
    fprintf(stderr, "generate_header failed with code %d\n", ret);
    json_value_free(root_val);
    return ret; /* Propagate specific error code */
  }
  ret = generate_source(basename, schemas_obj);
  if (ret != 0) {
    fprintf(stderr, "generate_source failed with code %d\n", ret);
    json_value_free(root_val);
    return ret;
  }

  json_value_free(root_val);
  return 0;
}
