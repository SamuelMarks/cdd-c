/* TODO: Generate prototypes in header file
 * TODO: Upsert `struct`/`enum`/function
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parson.h>

#ifdef _MSC_VER
#define strdup _strdup
#define PATH_MAX _MAX_PATH
#else
#include <limits.h>
#endif

#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */

#include "schema_codegen.h"

#include "code2schema.h"
#include "codegen.h"
#include "fs.h"

/*
 * Write the #ifndef guard header string to the .h file.
 */
static void print_header_guard(FILE *const hfile, const char *const basename) {
  fprintf(hfile, "#ifndef %s_H\n", basename);
  fprintf(hfile, "#define %s_H\n\n", basename);
}

/*
 * Close the #ifndef guard
 */
static void print_header_guard_end(FILE *const hfile,
                                   const char *const basename) {
  fprintf(hfile, "#endif /* !%s_H */\n", basename);
}

/* Map JSON types to C types for header struct fields */
/* enum ref is pointer to struct of that type */
static void print_c_type_for_schema_prop(FILE *const hfile,
                                         const char *const prop_name,
                                         const JSON_Object *const prop_obj) {
  const char *const type_str = json_object_get_string(prop_obj, "type");
  const char *const ref = json_object_get_string(prop_obj, "$ref");

  if (ref != NULL) {
    /* $ref to another schema - print pointer to struct */
    const char *const last_slash = strrchr(ref, '/');
    if (last_slash)
      fprintf(hfile, "  struct %s *%s;\n", last_slash + 1, prop_name);
    else
      fprintf(hfile, "  /* unknown $ref for %s: %s */\n", prop_name, ref);
  } else if (type_str == NULL) {
    fprintf(hfile, "  /* unknown type for %s */\n", prop_name);
  } else if (strcmp(type_str, "string") == 0) {
    fprintf(hfile, "  const char *%s;\n", prop_name);
  } else if (strcmp(type_str, "integer") == 0) {
    fprintf(hfile, "  int %s;\n", prop_name);
  } else if (strcmp(type_str, "number") == 0) {
    fprintf(hfile, "  double %s;\n", prop_name);
  } else if (strcmp(type_str, "boolean") == 0) {
    /* Use stdbool.h or c_cdd_stdbool.h */
    fprintf(hfile, "  int %s;\n", prop_name);
  } else if (strcmp(type_str, "object") == 0) {
    /* No direct object, must be $ref */
    fprintf(hfile, "  /* object property (unresolved) %s */\n", prop_name);
  } else {
    fprintf(hfile, "  /* unhandled type %s for %s */\n", type_str, prop_name);
  }
}

/*
 * Write enum declaration and function prototypes to header.
 * The JSON `enum` array must be strings.
 */
static void print_enum_declaration(FILE *const hfile,
                                   const char *const enum_name,
                                   const JSON_Array *const enum_values) {
  size_t i;
  const size_t n = json_array_get_count(enum_values);

  fprintf(hfile, "enum %s {\n", enum_name);
  for (i = 0; i < n; i++) {
    const char *val = json_array_get_string(enum_values, i);
    if (!val)
      continue;
    fprintf(hfile, "  %s", val);
    if (i + 1 < n)
      fprintf(hfile, ",\n");
    else {
      /* Add UNKNOWN = -1 if not already present */
      if (strcmp(val, "UNKNOWN") != 0)
        fprintf(hfile, ",\n  UNKNOWN = -1\n");
      else
        fprintf(hfile, "\n");
    }
  }
  fprintf(hfile, "};\n\n");

  /* Declare to_str/from_str functions */
  fprintf(hfile, "int %s_to_str(enum %s e, char **str_out);\n", enum_name,
          enum_name);
  fprintf(hfile, "int %s_from_str(const char *str, enum %s *e);\n\n", enum_name,
          enum_name);
}

/*
 * Print struct declaration and function prototypes for struct.
 * Prints required fields in required array.
 */
static void print_struct_declaration(FILE *const hfile,
                                     const char *const struct_name,
                                     const JSON_Object *const schema_obj) {
  const size_t nprops =
      json_object_get_count(json_object_get_object(schema_obj, "properties"));
  const JSON_Object *const props =
      json_object_get_object(schema_obj, "properties");
  /* const JSON_Array *required_arr = json_object_get_array(schema_obj,
   * "required"); */
  size_t i;

  fprintf(hfile, "struct %s {\n", struct_name);
  /* or if no properties: empty struct */
  if (props) {
    for (i = 0; i < nprops; i++) {
      const char *const prop_name = json_object_get_name(props, i);
      const JSON_Object *const prop_obj =
          json_object_get_object(props, prop_name);
      if (!prop_obj)
        continue;

      print_c_type_for_schema_prop(hfile, prop_name, prop_obj);
    }
  }
  fprintf(hfile, "};\n\n");

  fprintf(hfile, "int %s_debug(const struct %s *, FILE *);\n\n", struct_name,
          struct_name);
  fprintf(hfile, "int %s_deepcopy(const struct %s *, struct %s **);\n",
          struct_name, struct_name, struct_name);
  fprintf(hfile, "int %s_default(struct %s **);\n", struct_name, struct_name);
  fprintf(hfile, "int %s_display(const struct %s *, FILE *);\n", struct_name,
          struct_name);
  fprintf(hfile,
          "int %s_eq(const struct %s *const, const struct %s *const);\n\n",
          struct_name, struct_name, struct_name);
  fprintf(hfile, "int %s_from_json(const char *, struct %s **);\n", struct_name,
          struct_name);
  fprintf(hfile, "int %s_from_jsonObject(const JSON_Object *, struct %s **);\n",
          struct_name, struct_name);
  fprintf(hfile, "int %s_to_json(const struct %s *const, char **);\n",
          struct_name, struct_name);
  fprintf(hfile, "void %s_cleanup(struct %s *const);\n", struct_name,
          struct_name);
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
  const size_t len = strlen(basename);
  for (j = 0; j < len && j + 1 < sizeof(guard_macro); j++) {
    const char c = basename[j];
    if (c >= 'a' && c <= 'z')
      guard_macro[j] = (char)(c - ('a' - 'A'));
    else if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
      guard_macro[j] = c;
    else
      guard_macro[j] = '_';
  }
  guard_macro[j] = 0;

  snprintf(header_filename, sizeof(header_filename), "%s.h", basename);
  hfile = fopen(header_filename, "w");
  if (!hfile) {
    fprintf(stderr, "Failed to open header file: %s\n", header_filename);
    return -1;
  }

  fprintf(hfile,
          "/* Generated by c_cdd schema2code - do not edit directly */\n\n");

  print_header_guard(hfile, guard_macro);
  fprintf(hfile, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n");

  fprintf(hfile, "#include <parson.h>\n"
                 "#include <stdlib.h>\n"
                 "#include <stdbool.h>\n"
                 "#include <stdio.h>\n\n");

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
      /* Try $ref case? */
      continue;
    }

    if (strcmp(type_str, "string") == 0) {
      /* Possibly enum */
      const JSON_Array *const enum_arr =
          (JSON_Array *)json_object_get_array(schema_obj, "enum");
      if (enum_arr != NULL) {
        print_enum_declaration(hfile, schema_name, enum_arr);
        continue;
      }
    }

    if (strcmp(type_str, "object") == 0) {
      /* struct */
      print_struct_declaration(hfile, schema_name, schema_obj);
    }
  }

  fprintf(hfile, "#ifdef __cplusplus\n}\n#endif\n\n");
  print_header_guard_end(hfile, guard_macro);

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

  snprintf(source_filename, sizeof(source_filename), "%s.c", basename);
  cfile = fopen(source_filename, "w");
  if (!cfile) {
    fprintf(stderr, "Failed to open source file: %s\n", source_filename);
    return -1;
  }

  fputs("#include <stdlib.h>\n"
        "#include <string.h>\n"
        "#include <stdio.h>\n\n"
        "#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)\n"
        "#else\n"
        "#include <sys/errno.h>\n"
        "#endif\n"
        "#include <parson.h>\n\n",
        cfile);

  {
    const char *const base_name = get_basename(basename);
    fprintf(cfile, "#include \"%s.h\"\n\n", base_name);
  }

  fprintf(cfile,
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
          "}\n\n");

  /* Iterate schemas */
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

    if (strcmp(type_str, "string") == 0) {
      /* Possibly enum */
      const JSON_Array *const enum_arr =
          json_object_get_array(schema_obj, "enum");
      if (enum_arr != NULL) {
        struct EnumMembers em;
        const int rc = json_array_to_enum_members(enum_arr, &em);
        if (rc == 0) {
          write_enum_to_str_func(cfile, schema_name, &em);
          write_enum_from_str_func(cfile, schema_name, &em);
          enum_members_free(&em);
        } else {
          fprintf(stderr,
                  "Failed to convert enum JSON array to EnumMembers for %s\n",
                  schema_name);
        }
      }
      continue;
    }

    if (strcmp(type_str, "object") == 0) {
      struct StructFields fields;
      const int rc = json_object_to_struct_fields(schema_obj, &fields);
      if (rc != 0) {
        fprintf(stderr, "Failed to parse struct fields for %s\n", schema_name);
        continue;
      }

      write_struct_debug_func(cfile, schema_name, &fields);
      write_struct_deepcopy_func(cfile, schema_name, &fields);
      write_struct_default_func(cfile, schema_name, &fields);
      write_struct_display_func(cfile, schema_name, &fields);
      write_struct_eq_func(cfile, schema_name, &fields);
      write_struct_from_jsonObject_func(cfile, schema_name, &fields);
      write_struct_from_json_func(cfile, schema_name);
      write_struct_to_json_func(cfile, schema_name, &fields);
      write_struct_cleanup_func(cfile, schema_name, &fields);

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
    json_value_free(root_val);
    return EXIT_FAILURE;
  }
  ret = generate_source(basename, schemas_obj);
  if (ret != 0) {
    json_value_free(root_val);
    return EXIT_FAILURE;
  }

  json_value_free(root_val);
  return EXIT_SUCCESS;
}
