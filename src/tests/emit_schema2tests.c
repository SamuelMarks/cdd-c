/**
 * @file schema2tests.c
 * @brief Implementation of test generation logic.
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parson.h>

#include "functions/parse_fs.h"
#include "tests/emit_schema2tests.h"

#if defined(_BSD_SOURCE) || defined(_GNU_SOURCE) || defined(HAVE_ASPRINTF)
#include <stdio.h>
#else
#include <c89stringutils_string_extras.h>
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
#else
#include <sys/errno.h>
#endif

/* Helper macros for error checking */
#define CHECK_RC(x)                                                            \
  do {                                                                         \
    int _rc = (x);                                                             \
    if (_rc != 0)                                                              \
      return _rc;                                                              \
  } while (0)

#define CHECK_NULL(x, err)                                                     \
  do {                                                                         \
    if ((x) == NULL)                                                           \
      return (err);                                                            \
  } while (0)

/* Sanitize string for C identifier (underscores for invalid chars) */
static void to_c_ident(char *out, const size_t outsz, const char *const in) {
  size_t i;
  for (i = 0; i + 1 < outsz && in[i] != 0; i++) {
    if (isalnum((unsigned char)in[i])) {
      out[i] = in[i];
    } else {
      out[i] = '_';
    }
  }
  out[i] = '\0';
}

/* Generate test function for enum */
static int write_test_enum(FILE *f, const char *const enum_name,
                           const JSON_Array *enum_vals) {
  size_t i;
  const size_t n = json_array_get_count(enum_vals);

  char c_enum_name[128];
  to_c_ident(c_enum_name, sizeof(c_enum_name), enum_name);

  if (fprintf(f, "/* Test enum %s to_str/from_str */\n", enum_name) < 0)
    return EIO;
  if (fprintf(f,
              "TEST test_%s_to_str_from_str(void) {\n"
              "  char *str = NULL;\n"
              "  enum %s val;\n"
              "  int rc;\n\n",
              c_enum_name, enum_name) < 0)
    return EIO;

  /* Test to_str for each enum value */
  for (i = 0; i < n; i++) {
    const char *const val = json_array_get_string(enum_vals, i);
    char c_val[128];
    if (!val)
      continue;
    to_c_ident(c_val, sizeof(c_val), val);

    if (fprintf(f,
                "  rc = %s_to_str(%s_%s, &str);\n"
                "  ASSERT_EQ(0, rc);\n"
                "  ASSERT_STR_EQ(\"%s\", str);\n"
                "  free(str);\n\n",
                enum_name, enum_name, c_val, val) < 0)
      return EIO;
  }

  /* Test from_str for each enum value */
  for (i = 0; i < n; i++) {
    const char *const val = json_array_get_string(enum_vals, i);
    char c_val[128];
    if (!val)
      continue;
    to_c_ident(c_val, sizeof(c_val), val);

    if (fprintf(f,
                "  rc = %s_from_str(\"%s\", &val);\n"
                "  ASSERT_EQ(0, rc);\n"
                "  ASSERT_EQ(%s_%s, val);\n\n",
                enum_name, val, enum_name, c_val) < 0)
      return EIO;
  }

  /* Test from_str unknown string */
  if (fprintf(f,
              "  rc = %s_from_str(\"INVALID\", &val);\n"
              "  ASSERT_EQ(0, rc);\n"
              "  ASSERT_EQ(%s_UNKNOWN, val);\n\n",
              enum_name, enum_name) < 0)
    return EIO;

  if (fputs("  PASS();\n}\n", f) < 0)
    return EIO;

  return 0;
}

/* Generate test function for struct */
static int write_test_struct(FILE *f, const char *const struct_name,
                             const JSON_Object *schema_obj) {
  char c_struct_name[128];
  (void)schema_obj;
  to_c_ident(c_struct_name, sizeof(c_struct_name), struct_name);

  if (fprintf(f,
              "/* Test %s default / deepcopy / eq / cleanup */\n"
              "TEST test_%s_default_deepcopy_eq_cleanup(void) {\n"
              "  struct %s *obj0 = NULL;\n"
              "  struct %s *obj1 = NULL;\n"
              "  int rc;\n\n"
              "  rc = %s_default(&obj0);\n"
              "  if (rc != 0 || obj0 == NULL) FAIL();\n\n"
              "  rc = %s_deepcopy(obj0, &obj1);\n"
              "  if (rc != 0 || obj1 == NULL) { %s_cleanup(obj0); FAIL(); }\n\n"
              "  ASSERT(%s_eq(obj0, obj1));\n\n"
              "  /* Modify obj0 to break equality */\n"
              "  /* skipping modifying fields because unknown type here */\n\n"
              "  %s_cleanup(obj0);\n"
              "  %s_cleanup(obj1);\n\n"
              "  PASS();\n"
              "}\n\n",
              struct_name, c_struct_name, struct_name, struct_name, struct_name,
              struct_name, struct_name, struct_name, struct_name,
              struct_name) < 0)
    return EIO;

  /* Add JSON roundtrip test */
  if (fprintf(f,
              "TEST test_%s_json_roundtrip(void) {\n"
              "  struct %s *obj_in = NULL;\n"
              "  struct %s *obj_out = NULL;\n"
              "  char *json_str = NULL;\n"
              "  int rc;\n"
              "\n"
              "  rc = %s_default(&obj_in);\n"
              "  ASSERT_EQ(0, rc);\n"
              "  ASSERT(obj_in != NULL);\n"
              "\n"
              "  rc = %s_to_json(obj_in, &json_str);\n"
              "  ASSERT_EQ(0, rc);\n"
              "  ASSERT(json_str != NULL);\n"
              "\n"
              "  rc = %s_from_json(json_str, &obj_out);\n"
              "  ASSERT_EQ(0, rc);\n"
              "  ASSERT(obj_out != NULL);\n"
              "\n"
              "  ASSERT(%s_eq(obj_in, obj_out));\n"
              "\n"
              "  free(json_str);\n"
              "  %s_cleanup(obj_in);\n"
              "  %s_cleanup(obj_out);\n"
              "\n"
              "  PASS();\n"
              "}\n\n",
              c_struct_name, struct_name, struct_name, struct_name, struct_name,
              struct_name, struct_name, struct_name, struct_name) < 0)
    return EIO;

  return 0;
}

/* Main function: load JSON schema and generate tests source */
int jsonschema2tests_main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: jsonschema2tests <schema.json> <header_to_test.h> "
                    "<output-test.h>\n");
    return EXIT_FAILURE;
  }

  {
    const char *const schema_file = argv[0];
    const char *const header_to_test = argv[1];
    const char *const output_file = argv[2];

    JSON_Value *root_val = NULL;
    const JSON_Object *root_obj = NULL;
    JSON_Object *schemas_obj = NULL;
    FILE *f;
    char sanitized[128];
    int rc = 0;

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

    /* Look for schemas in common locations */
    schemas_obj = json_object_get_object(root_obj, "components");
    if (schemas_obj)
      schemas_obj = json_object_get_object(schemas_obj, "schemas");
    if (!schemas_obj)
      schemas_obj = json_object_get_object(root_obj, "$defs");

    if (!schemas_obj) {
      fprintf(
          stderr,
          "Schema does not contain 'components/schemas' or '$defs' object\n");
      json_value_free(root_val);
      return EXIT_FAILURE;
    }

    /* Ensure output directory exists */
    {
      /* Refactored: get_dirname returns allocated string or error code */
      char *output_dir = NULL;
      rc = get_dirname(output_file, &output_dir);
      if (rc != 0) {
        fprintf(stderr, "Failed to get dirname of output file: %s (rc %d)\n",
                output_file, rc);
        json_value_free(root_val);
        return rc;
      }
      rc = makedirs(output_dir);
      if (rc != 0) {
        fprintf(stderr, "Failed to create output directory: %s (rc %d)\n",
                output_dir, rc);
        free(output_dir);
        json_value_free(root_val);
        return rc;
      }
      free(output_dir);
    }

    /* Output file */
    {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      errno_t err = fopen_s(&f, output_file, "w, ccs=UTF-8");
      if (err != 0 || f == NULL) {
        fprintf(stderr, "Failed to open output file %s\n", output_file);
        json_value_free(root_val);
        return EXIT_FAILURE;
      }
#else
      f = fopen(output_file, "w");
      if (!f) {
        fprintf(stderr, "Failed to open output file: %s\n", output_file);
        json_value_free(root_val);
        return EXIT_FAILURE;
      }
#endif

      /* Refactored: get_basename returns allocated string via pointer */
      {
        char *base = NULL;
        rc = get_basename(schema_file, &base);
        if (rc != 0) {
          fprintf(stderr, "Failed to get basename of: %s\n", schema_file);
          fclose(f);
          json_value_free(root_val);
          return rc;
        }
        to_c_ident(sanitized, sizeof(sanitized), base);
        free(base);
      }

      fprintf(f,
              "#ifndef %s_TESTS_H\n"
              "#define %s_TESTS_H\n"
              "/* Auto-generated test source from JSON Schema %s */\n\n"
              "#include <stdlib.h>\n"
              "#include <string.h>\n\n"
              "#include <greatest.h>\n\n",
              sanitized, sanitized, schema_file);

      fprintf(f, "#include \"%s\"\n", header_to_test);

      /* include headers referenced by schema names */
      {
        char *output_dir = NULL;
        rc = get_dirname(output_file, &output_dir);
        if (rc == 0) {
          const size_t count = json_object_get_count(schemas_obj);
          size_t i;
          for (i = 0; i < count; i++) {
            const char *const schema_name =
                json_object_get_name(schemas_obj, i);
            char sanitized_name[128];
            to_c_ident(sanitized_name, sizeof(sanitized_name), schema_name);
            {
              char *path_to_check = NULL;
              char *include_name = NULL;

              if (asprintf(&include_name, "%s.h", sanitized_name) == -1)
                continue;

              if (strcmp(output_dir, ".") != 0) {
                if (asprintf(&path_to_check, "%s%s%s", output_dir, PATH_SEP,
                             include_name) == -1) {
                  free(include_name);
                  continue;
                }
              } else {
                path_to_check = strdup(include_name);
              }

              if (path_to_check && access(path_to_check, F_OK) == 0) {
                fprintf(f, "#include \"%s\"\n", include_name);
              }
              free(path_to_check);
              free(include_name);
            }
          }
          free(output_dir);
        }
      }
      fprintf(f, "\n");

      /* Generate test functions for enums and structs */
      {
        const size_t count = json_object_get_count(schemas_obj);
        size_t i;
        for (i = 0; i < count; i++) {
          const char *const schema_name = json_object_get_name(schemas_obj, i);
          const JSON_Value *const val =
              json_object_get_value_at(schemas_obj, i);
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
              write_test_enum(f, schema_name, enum_arr);
            }
          } else if (strcmp(type_str, "object") == 0) {
            write_test_struct(f, schema_name, schema_obj);
          }
        }
      }

      /* Write suites and main runner */

      fprintf(f, "/* Test suites */\n");

      fprintf(f, "SUITE(enums_suite) {\n");

      /* Add enum test calls */
      {
        const size_t count = json_object_get_count(schemas_obj);
        size_t i;
        for (i = 0; i < count; i++) {
          const char *const schema_name = json_object_get_name(schemas_obj, i);
          const JSON_Value *const val =
              json_object_get_value_at(schemas_obj, i);
          const JSON_Object *const schema_obj = json_value_get_object(val);
          const char *type_str;
          if (!schema_obj)
            continue;

          type_str = json_object_get_string(schema_obj, "type");
          if (!type_str)
            continue;

          if (strcmp(type_str, "string") == 0) {
            const JSON_Array *const enum_arr =
                json_object_get_array(schema_obj, "enum");
            if (enum_arr != NULL) {
              char sanitized0[128];
              to_c_ident(sanitized0, sizeof(sanitized0), schema_name);
              fprintf(f, "  RUN_TEST(test_%s_to_str_from_str);\n", sanitized0);
            }
          }
        }
      }

      fprintf(f, "}\n\n");

      fprintf(f, "SUITE(structs_suite) {\n");

      /* Add struct test calls */
      {
        const size_t count = json_object_get_count(schemas_obj);
        size_t i;
        for (i = 0; i < count; i++) {
          const char *const schema_name = json_object_get_name(schemas_obj, i);
          const JSON_Value *const val =
              json_object_get_value_at(schemas_obj, i);
          const JSON_Object *const schema_obj = json_value_get_object(val);
          const char *type_str;
          if (!schema_obj)
            continue;

          type_str = json_object_get_string(schema_obj, "type");
          if (!type_str)
            continue;

          if (strcmp(type_str, "object") == 0) {
            char sanitized0[128];
            to_c_ident(sanitized0, sizeof(sanitized0), schema_name);
            fprintf(f, "  RUN_TEST(test_%s_default_deepcopy_eq_cleanup);\n",
                    sanitized0);
            fprintf(f, "  RUN_TEST(test_%s_json_roundtrip);\n", sanitized0);
          }
        }
      }

      fprintf(f, "}\n\n#endif /* !%s_TESTS_H */\n", sanitized);

      fclose(f);
    }

    json_value_free(root_val);

    {
      /* Refactor test main generation */
      char *output_dir = NULL;
      char *p = NULL;
      rc = get_dirname(output_file, &output_dir);
      if (rc != 0)
        return rc;

      if (asprintf(&p, "%s%s%s", output_dir, PATH_SEP, "test_main.c") == -1) {
        free(output_dir);
        return ENOMEM;
      }
      free(output_dir);

      {
        FILE *f0;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
        errno_t err = fopen_s(&f0, p, "w, ccs=UTF-8");
        if (err != 0 || f0 == NULL) {
          fprintf(stderr, "Failed to open output file %s\n", p);
          free(p);
          return EXIT_FAILURE;
        }
#else
        f0 = fopen(p, "w");
        if (!f0) {
          fprintf(stderr, "Failed to open output file: %s\n", p);
          free(p);
          return EXIT_FAILURE;
        }
#endif
        {
          char *base = NULL;
          rc = get_basename(output_file, &base);
          if (rc == 0) {
            fprintf(f0,
                    "#include <greatest.h>\n"
                    "#include \"%s\"\n\n"
                    "GREATEST_MAIN_DEFS();\n\n"
                    "int main(int argc, char **argv) {\n"
                    "  GREATEST_MAIN_BEGIN();\n"
                    "  RUN_SUITE(enums_suite);\n"
                    "  RUN_SUITE(structs_suite);\n"
                    "  GREATEST_MAIN_END();\n"
                    "}\n",
                    base);
            free(base);
          } else {
            fprintf(stderr, "Failed to determine basename for %s\n",
                    output_file);
          }
        }
        fclose(f0);
        printf("Test runner generated and written to:\t%s\n", p);
      }
      free(p);
    }

    printf("Tests generated and written to:\t\t\t%s\n", output_file);

    return 0;
  }
}
