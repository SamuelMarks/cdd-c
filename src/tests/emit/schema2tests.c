/**
 * @file schema2tests.c
 * @brief Implementation of JSON Schema C test suite generation logic.
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/memory.h"
#include "c_cdd/json_dom.h"
#include "c_cdd/reflect.h"
#include "c_cdd/safe_crt.h"
#include "classes/emit/cli_struct.h"
#include "functions/parse/fs.h"
#include "tests/emit/schema2tests.h"
#include "c89stringutils_string_extras.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER) || defined(_WIN32)
#include <io.h>
#ifndef F_OK
#define F_OK 0
#endif
#else
#include <unistd.h>
#endif
/* clang-format on */

#ifdef CDD_BUILD_TESTS
/** @brief Test I/O call counter. */
extern C_CDD_EXPORT int g_io_calls;
/** @brief Test I/O fault injection threshold (`-1` disables failure). */
extern C_CDD_EXPORT int g_fail_io_after;
#endif

/**
 * @brief Writes formatted text to `fp` with test fault injection support.
 *
 * @param[in,out] fp Target file stream.
 * @param[in] fmt `printf`-style format string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 3)))
#endif
static NO_DISCARD cdd_c_error_t
s2t_fprintf(FILE *fp, const char *fmt, ...) {
  int ret = -1;
#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after < 0 || ++g_io_calls <= g_fail_io_after)
#endif
  {
    va_list args;
    va_start(args, fmt);
    ret = vfprintf(fp, fmt, args);
    va_end(args);
  }
  return (ret < 0) ? CDD_C_ERROR_IO : CDD_C_SUCCESS;
}

/**
 * @brief Formats a heap-allocated string with OOM fault injection support.
 *
 * @param[out] strp Destination string pointer.
 * @param[in] fmt `printf`-style format string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on allocation
 * failure.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 3)))
#endif
static NO_DISCARD cdd_c_error_t
s2t_asprintf(char **strp, const char *fmt, ...) {
  int ret = -1;
  *strp = NULL;
#ifdef CDD_BUILD_TESTS
  if (!g_cdd_alloc_fail || --g_cdd_alloc_fail != 0)
#endif
  {
    va_list args;
    va_start(args, fmt);
    ret = c89stringutils_vasprintf(strp, fmt, args);
    va_end(args);
  }
  return (ret < 0) ? CDD_C_ERROR_MEMORY : CDD_C_SUCCESS;
}

/**
 * @brief Opens a file for writing with Safe CRT and fault injection support.
 *
 * @param[in] path Destination file path.
 * @param[out] out_fp Opened file stream on success, or NULL on error.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_UNKNOWN on open failure.
 */
static NO_DISCARD cdd_c_error_t s2t_fopen_write(const char *path,
                                                FILE **out_fp) {
  FILE *fp = NULL;
  *out_fp = NULL;
#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after < 0 || ++g_io_calls <= g_fail_io_after)
#endif
  {
#if defined(_MSC_VER)
    if (fopen_s(&fp, path, "w") != 0)
      fp = NULL;
#else
    fp = fopen(path, "w");
#endif
  }
  *out_fp = fp;
  return fp ? CDD_C_SUCCESS : CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief Resolves parent directory of `path` with OOM fault injection support.
 *
 * @param[in] path File path.
 * @param[out] dir Allocated parent directory string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static NO_DISCARD cdd_c_error_t s2t_get_dirname(const char *path, char **dir) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_alloc_fail && --g_cdd_alloc_fail == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  return get_dirname(path, dir);
}

/**
 * @brief Creates directory hierarchy `path` with OOM fault injection support.
 *
 * @param[in] path Directory path to create.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static NO_DISCARD cdd_c_error_t s2t_makedirs(const char *path) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_alloc_fail && --g_cdd_alloc_fail == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  return makedirs(path);
}

/**
 * @brief Sanitizes an arbitrary schema name into a valid C identifier.
 *
 * @param[out] out Output character buffer (`outsz >= 2`).
 * @param[in] outsz Capacity of `out` in bytes.
 * @param[in] in Input string to sanitize.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t to_c_ident(char *out, const size_t outsz,
                                           const char *const in) {
  size_t i;
  for (i = 0; i + 1 < outsz && in[i] != '\0'; i++) {
    if (isalnum((unsigned char)in[i])) {
      out[i] = in[i];
    } else {
      out[i] = '_';
    }
  }
  out[i] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits a `greatest.h` roundtrip test for a string enum schema.
 *
 * @param[in,out] f Output stream.
 * @param[in] enum_name Schema enum name.
 * @param[in] enum_vals JSON array of enum string literals.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
static NO_DISCARD cdd_c_error_t write_test_enum(FILE *f,
                                                const char *const enum_name,
                                                const JSON_Array *enum_vals) {
  size_t i;
  size_t n = 0;
  char c_enum_name[128];
  cdd_c_error_t rc = CDD_C_SUCCESS;

  rc = cdd_c_keep_err(rc, cdd_json_array_get_count(enum_vals, &n));
  rc = cdd_c_keep_err(rc,
                      to_c_ident(c_enum_name, sizeof(c_enum_name), enum_name));

  rc = s2t_fprintf(f, "/* Test enum %s to_str/from_str */\n", enum_name);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = s2t_fprintf(f,
                   "TEST test_%s_to_str_from_str(void) {\n"
                   "  char *str = NULL;\n"
                   "  enum %s val;\n"
                   "  cdd_c_error_t rc;\n\n",
                   c_enum_name, enum_name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < n; i++) {
    const char *val = NULL;
    char c_val[128];
    cdd_c_error_t str_rc = cdd_json_array_get_string(enum_vals, i, &val);
    if (str_rc != CDD_C_SUCCESS)
      continue;
    rc = cdd_c_keep_err(rc, to_c_ident(c_val, sizeof(c_val), val));

    rc = s2t_fprintf(f,
                     "  rc = %s_to_str(%s_%s, &str);\n"
                     "  ASSERT_EQ(0, rc);\n"
                     "  ASSERT_STR_EQ(\"%s\", str);\n"
                     "  C_CDD_FREE(str);\n\n",
                     enum_name, enum_name, c_val, val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  for (i = 0; i < n; i++) {
    const char *val = NULL;
    char c_val[128];
    cdd_c_error_t str_rc = cdd_json_array_get_string(enum_vals, i, &val);
    if (str_rc != CDD_C_SUCCESS)
      continue;
    rc = cdd_c_keep_err(rc, to_c_ident(c_val, sizeof(c_val), val));

    rc = s2t_fprintf(f,
                     "  rc = %s_from_str(\"%s\", &val);\n"
                     "  ASSERT_EQ(0, rc);\n"
                     "  ASSERT_EQ(%s_%s, val);\n\n",
                     enum_name, val, enum_name, c_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = s2t_fprintf(f,
                   "  rc = %s_from_str(\"INVALID\", &val);\n"
                   "  ASSERT_EQ(0, rc);\n"
                   "  ASSERT_EQ(%s_UNKNOWN, val);\n\n",
                   enum_name, enum_name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return s2t_fprintf(f, "%s", "  PASS();\n}\n");
}

/**
 * @brief Emits `greatest.h` lifecycle and JSON roundtrip tests for an object
 * schema.
 *
 * @param[in,out] f Output stream.
 * @param[in] struct_name Schema object name.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
static NO_DISCARD cdd_c_error_t
write_test_struct(FILE *f, const char *const struct_name) {
  char c_struct_name[128];
  cdd_c_error_t rc = CDD_C_SUCCESS;

  rc = cdd_c_keep_err(
      rc, to_c_ident(c_struct_name, sizeof(c_struct_name), struct_name));

  rc = s2t_fprintf(f,
                   "/* Test %s default / deepcopy / eq / cleanup */\n"
                   "TEST test_%s_default_deepcopy_eq_cleanup(void) {\n"
                   "  struct %s *obj0 = NULL;\n"
                   "  struct %s *obj1 = NULL;\n"
                   "  cdd_c_error_t rc;\n\n"
                   "  rc = %s_default(&obj0);\n"
                   "  if (rc != 0 || obj0 == NULL) FAIL();\n\n",
                   struct_name, c_struct_name, struct_name, struct_name,
                   struct_name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = s2t_fprintf(
      f,
      "  rc = %s_deepcopy(obj0, &obj1);\n"
      "  if (rc != 0 || obj1 == NULL) { %s_cleanup(obj0); FAIL(); }\n\n"
      "  ASSERT(%s_eq(obj0, obj1));\n\n"
      "  /* Modify obj0 to break equality */\n"
      "  /* skipping modifying fields because unknown type here */\n\n"
      "  %s_cleanup(obj0);\n"
      "  %s_cleanup(obj1);\n\n"
      "  PASS();\n"
      "}\n\n",
      struct_name, struct_name, struct_name, struct_name, struct_name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return s2t_fprintf(f,
                     "TEST test_%s_json_roundtrip(void) {\n"
                     "  struct %s *obj_in = NULL;\n"
                     "  struct %s *obj_out = NULL;\n"
                     "  char *json_str = NULL;\n"
                     "  cdd_c_error_t rc;\n"
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
                     "  C_CDD_FREE(json_str);\n"
                     "  %s_cleanup(obj_in);\n"
                     "  %s_cleanup(obj_out);\n"
                     "\n"
                     "  PASS();\n"
                     "}\n\n",
                     c_struct_name, struct_name, struct_name, struct_name,
                     struct_name, struct_name, struct_name, struct_name,
                     struct_name);
}

/**
 * @brief CLI configuration for `jsonschema2tests_main`.
 */
struct cdd_schema2tests_config {
  /** @brief Input JSON Schema file path. */
  const char *schema_file;
  /** @brief Header file under test. */
  const char *header_to_test;
  /** @brief Output test header file path. */
  const char *output_file;
};

/** @brief Reflected fields for `cdd_schema2tests_config`. */
static const struct cdd_field_info g_cdd_schema2tests_config_fields[] = {
    {"schema_file",
     "schema_file",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct cdd_schema2tests_config, schema_file),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"header_to_test",
     "header_to_test",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct cdd_schema2tests_config, header_to_test),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"output_file",
     "output_file",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct cdd_schema2tests_config, output_file),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

/** @brief Reflected type descriptor for `cdd_schema2tests_config`. */
static const struct cdd_type_info g_cdd_schema2tests_config_type_info = {
    "cdd_schema2tests_config",
    sizeof(struct cdd_schema2tests_config),
    0,
    g_cdd_schema2tests_config_fields,
    sizeof(g_cdd_schema2tests_config_fields) /
        sizeof(g_cdd_schema2tests_config_fields[0]),
    0};

/**
 * @brief Emits header `#include` directives for per-schema `.h` files present
 * in `output_dir`.
 *
 * @param[in,out] f Output test header stream.
 * @param[in] schemas_obj Schemas JSON object.
 * @param[in] output_dir Target directory containing generated headers.
 * @return CDD_C_SUCCESS on success, or error code on OOM/IO failure.
 */
static NO_DISCARD cdd_c_error_t emit_schema_includes(
    FILE *f, const JSON_Object *schemas_obj, const char *output_dir) {
  size_t count = 0;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  rc = cdd_c_keep_err(rc, cdd_json_object_get_count(schemas_obj, &count));
  for (i = 0; i < count; i++) {
    const char *schema_name = NULL;
    char sanitized_name[128];
    char *path_to_check = NULL;
    char *include_name = NULL;
    const char *probe_path;

    rc = cdd_c_keep_err(rc,
                        cdd_json_object_get_name(schemas_obj, i, &schema_name));
    rc = cdd_c_keep_err(
        rc, to_c_ident(sanitized_name, sizeof(sanitized_name), schema_name));

    rc = s2t_asprintf(&include_name, "%s.h", sanitized_name);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (strcmp(output_dir, ".") != 0) {
      rc = s2t_asprintf(&path_to_check, "%s%s%s", output_dir, PATH_SEP,
                        include_name);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(include_name);
        return rc;
      }
      probe_path = path_to_check;
    } else {
      probe_path = include_name;
    }

#if defined(_MSC_VER) || defined(_WIN32)
    if (_access(probe_path, F_OK) == 0) {
#else
    if (access(probe_path, F_OK) == 0) {
#endif
      rc = s2t_fprintf(f, "#include \"%s\"\n", include_name);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(path_to_check);
        C_CDD_FREE(include_name);
        return rc;
      }
    }
    C_CDD_FREE(path_to_check);
    C_CDD_FREE(include_name);
  }
  return rc;
}

/**
 * @brief Emits all test bodies and `SUITE` registrations into the generated
 * test header.
 *
 * @param[in,out] f Output test header stream.
 * @param[in] schemas_obj Schemas JSON object.
 * @param[in] sanitized Sanitized header guard prefix.
 * @return CDD_C_SUCCESS on success, or error code on I/O failure.
 */
static NO_DISCARD cdd_c_error_t emit_schema_suites(
    FILE *f, const JSON_Object *schemas_obj, const char *sanitized) {
  size_t count = 0;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  rc = cdd_c_keep_err(rc, cdd_json_object_get_count(schemas_obj, &count));

  for (i = 0; i < count; i++) {
    const char *schema_name = NULL;
    JSON_Value *val = NULL;
    JSON_Object *schema_obj = NULL;
    const char *type_str = NULL;

    rc = cdd_c_keep_err(rc,
                        cdd_json_object_get_name(schemas_obj, i, &schema_name));
    rc = cdd_c_keep_err(rc, cdd_json_object_get_value_at(schemas_obj, i, &val));
    if (cdd_json_value_get_object(val, &schema_obj) != CDD_C_SUCCESS)
      continue;
    if (cdd_json_object_get_string(schema_obj, "type", &type_str) !=
        CDD_C_SUCCESS)
      continue;

    if (strcmp(type_str, "string") == 0) {
      JSON_Array *enum_arr = NULL;
      if (cdd_json_object_get_array(schema_obj, "enum", &enum_arr) ==
          CDD_C_SUCCESS) {
        rc = write_test_enum(f, schema_name, enum_arr);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (strcmp(type_str, "object") == 0) {
      rc = write_test_struct(f, schema_name);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  rc = s2t_fprintf(f, "/* Test suites */\nSUITE(enums_suite) {\n");
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < count; i++) {
    const char *schema_name = NULL;
    JSON_Value *val = NULL;
    JSON_Object *schema_obj = NULL;
    const char *type_str = NULL;
    JSON_Array *enum_arr = NULL;

    rc = cdd_c_keep_err(rc,
                        cdd_json_object_get_name(schemas_obj, i, &schema_name));
    rc = cdd_c_keep_err(rc, cdd_json_object_get_value_at(schemas_obj, i, &val));
    if (cdd_json_value_get_object(val, &schema_obj) != CDD_C_SUCCESS)
      continue;
    if (cdd_json_object_get_string(schema_obj, "type", &type_str) !=
        CDD_C_SUCCESS)
      continue;
    if (strcmp(type_str, "string") == 0 &&
        cdd_json_object_get_array(schema_obj, "enum", &enum_arr) ==
            CDD_C_SUCCESS) {
      char sanitized0[128];
      rc = cdd_c_keep_err(
          rc, to_c_ident(sanitized0, sizeof(sanitized0), schema_name));
      rc = s2t_fprintf(f, "  RUN_TEST(test_%s_to_str_from_str);\n", sanitized0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  rc = s2t_fprintf(f, "}\n\nSUITE(structs_suite) {\n");
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < count; i++) {
    const char *schema_name = NULL;
    JSON_Value *val = NULL;
    JSON_Object *schema_obj = NULL;
    const char *type_str = NULL;

    rc = cdd_c_keep_err(rc,
                        cdd_json_object_get_name(schemas_obj, i, &schema_name));
    rc = cdd_c_keep_err(rc, cdd_json_object_get_value_at(schemas_obj, i, &val));
    if (cdd_json_value_get_object(val, &schema_obj) != CDD_C_SUCCESS)
      continue;
    if (cdd_json_object_get_string(schema_obj, "type", &type_str) !=
        CDD_C_SUCCESS)
      continue;
    if (strcmp(type_str, "object") == 0) {
      char sanitized0[128];
      rc = cdd_c_keep_err(
          rc, to_c_ident(sanitized0, sizeof(sanitized0), schema_name));
      rc = s2t_fprintf(f,
                       "  RUN_TEST(test_%s_default_deepcopy_eq_cleanup);\n"
                       "  RUN_TEST(test_%s_json_roundtrip);\n",
                       sanitized0, sanitized0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return s2t_fprintf(f, "}\n\n#endif /* !%s_TESTS_H */\n", sanitized);
}

/* Main function: load JSON schema and generate tests source */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t jsonschema2tests_main(int argc,
                                                            char **argv) {
  struct cdd_schema2tests_config cfg;
  int first_unk = -1;
  const char *schema_file;
  const char *header_to_test;
  const char *output_file;
  JSON_Value *root_val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Object *schemas_obj = NULL;
  JSON_Object *components_obj = NULL;
  char *output_dir = NULL;
  char *base = NULL;
  char *runner_path = NULL;
  FILE *f = NULL;
  FILE *f0 = NULL;
  char sanitized[128];
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (argc != 3 || !argv || !argv[0] || !argv[1] || !argv[2]) {
    fprintf(stderr, "Usage: jsonschema2tests <schema.json> <header_to_test.h> "
                    "<output-test.h>\n");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  memset(&cfg, 0, sizeof(cfg));
  rc = cdd_cli_parse_reflected_argv(argc, argv, 0,
                                    &g_cdd_schema2tests_config_type_info, &cfg,
                                    NULL, &first_unk);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!cfg.schema_file || !cfg.header_to_test || !cfg.output_file) {
    fprintf(stderr, "Usage: jsonschema2tests <schema.json> <header_to_test.h> "
                    "<output-test.h>\n");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  schema_file = cfg.schema_file;
  header_to_test = cfg.header_to_test;
  output_file = cfg.output_file;

  if (cdd_json_parse_file(schema_file, &root_val) != CDD_C_SUCCESS) {
    fprintf(stderr, "Failed to parse JSON schema file: %s\n", schema_file);
    return CDD_C_ERROR_UNKNOWN;
  }

  if (cdd_json_value_get_object(root_val, &root_obj) != CDD_C_SUCCESS) {
    fprintf(stderr, "Invalid JSON schema document\n");
    rc = cdd_c_keep_err(CDD_C_ERROR_UNKNOWN, cdd_json_value_free(root_val));
    return rc;
  }

  if (cdd_json_object_get_object(root_obj, "components", &components_obj) ==
      CDD_C_SUCCESS) {
    if (cdd_json_object_get_object(components_obj, "schemas", &schemas_obj) !=
        CDD_C_SUCCESS) {
      schemas_obj = NULL;
    }
  }
  if (!schemas_obj) {
    if (cdd_json_object_get_object(root_obj, "$defs", &schemas_obj) !=
        CDD_C_SUCCESS) {
      fprintf(
          stderr,
          "Schema does not contain 'components/schemas' or '$defs' object\n");
      rc = cdd_c_keep_err(CDD_C_ERROR_UNKNOWN, cdd_json_value_free(root_val));
      return rc;
    }
  }

  rc = s2t_get_dirname(output_file, &output_dir);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Failed to get dirname of output file: %s (rc %d)\n",
            output_file, (int)rc);
    return cdd_c_keep_err(rc, cdd_json_value_free(root_val));
  }

  rc = s2t_makedirs(output_dir);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Failed to create output directory: %s (rc %d)\n",
            output_dir, (int)rc);
    C_CDD_FREE(output_dir);
    return cdd_c_keep_err(rc, cdd_json_value_free(root_val));
  }

  rc = s2t_fopen_write(output_file, &f);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Failed to open output file: %s\n", output_file);
    C_CDD_FREE(output_dir);
    return cdd_c_keep_err(rc, cdd_json_value_free(root_val));
  }

  rc = get_basename(schema_file, &base);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Failed to get basename of: %s\n", schema_file);
    fclose(f);
    C_CDD_FREE(output_dir);
    return cdd_c_keep_err(rc, cdd_json_value_free(root_val));
  }
  rc = cdd_c_keep_err(rc, to_c_ident(sanitized, sizeof(sanitized), base));
  C_CDD_FREE(base);
  base = NULL;

  rc = s2t_fprintf(f,
                   "#ifndef %s_TESTS_H\n"
                   "#define %s_TESTS_H\n"
                   "/* Auto-generated test source from JSON Schema %s */\n\n"
                   "#include <stdlib.h>\n"
                   "#include <string.h>\n\n"
                   "#include <greatest.h>\n\n"
                   "#include \"%s\"\n",
                   sanitized, sanitized, schema_file, header_to_test);
  if (rc == CDD_C_SUCCESS)
    rc = emit_schema_includes(f, schemas_obj, output_dir);
  if (rc == CDD_C_SUCCESS)
    rc = s2t_fprintf(f, "\n");
  if (rc == CDD_C_SUCCESS)
    rc = emit_schema_suites(f, schemas_obj, sanitized);

  fclose(f);
  rc = cdd_c_keep_err(rc, cdd_json_value_free(root_val));
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(output_dir);
    return rc;
  }

  rc =
      s2t_asprintf(&runner_path, "%s%s%s", output_dir, PATH_SEP, "test_main.c");
  C_CDD_FREE(output_dir);
  output_dir = NULL;
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = s2t_fopen_write(runner_path, &f0);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Failed to open output file: %s\n", runner_path);
    C_CDD_FREE(runner_path);
    return rc;
  }

  rc = get_basename(output_file, &base);
  if (rc == CDD_C_SUCCESS) {
    rc = s2t_fprintf(f0,
                     "#include <greatest.h>\n"
                     "#include \"%s\"\n\n"
                     "GREATEST_MAIN_DEFS();\n"
                     "\n"
                     "int main(int argc, char **argv) {\n"
                     "  GREATEST_MAIN_BEGIN();\n"
                     "  RUN_SUITE(enums_suite);\n"
                     "  RUN_SUITE(structs_suite);\n"
                     "  GREATEST_MAIN_END();\n"
                     "}\n",
                     base);
    C_CDD_FREE(base);
  } else {
    fprintf(stderr, "Failed to determine basename for %s\n", output_file);
  }

  fclose(f0);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(runner_path);
    return rc;
  }

  printf("Test runner generated and written to:\t%s\n", runner_path);
  C_CDD_FREE(runner_path);
  printf("Tests generated and written to:\t\t\t%s\n", output_file);
  return CDD_C_SUCCESS;
}
