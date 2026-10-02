#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
/**
 * @file test_schema_codegen.h
 * @brief Unit tests for schema to code generation.
 */

#ifndef TEST_SCHEMA_CODEGEN_H

#define TEST_SCHEMA_CODEGEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "classes/emit/schema.h"
#include "c_cdd_export.h"
#include <greatest.h>
#include "classes/emit/schema_codegen.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "classes/parse/code2schema.h" /* Included to avoid implicit declaration warnings */
#include "functions/emit/codegen.h"
#include "functions/parse/fs.h"

/* clang-format on */

/* Forward declare static functions from schema_codegen.c if we want unit-test

 * integration */

/* However, since schema_codegen.c is compiled separately, we verify via full

 * CLI integration or file inspection */

/**
 * @brief test circular
 * @return TEST
 */
TEST test_schema_codegen_circular_refs(void) {

  /*

   * Verify that circular dependencies generate valid forward declarations using

   * a multi-pass header generation. A references B, B references A.

   */

  int rc = 0;
  char *header_content = NULL;
  size_t sz;
  const char *const filename = "circular.json";
  const char *argv[2];
  const char *schema = "{\"components\": {\"schemas\": {"
                       "\"A\": {\"type\": \"object\", \"properties\": {\"b\": "
                       "{\"$ref\": \"#/components/schemas/B\"}}},"
                       "\"B\": {\"type\": \"object\", \"properties\": {\"a\": "
                       "{\"$ref\": \"#/components/schemas/A\"}}}"
                       "}}}";

  argv[0] = filename;
  argv[1] = "circular_out";

  rc = write_to_file(filename, schema);

  ASSERT_EQ(0, rc);

  rc = schema2code_main(2, (char **)(size_t)argv);

  ASSERT_EQ(0, rc);

  /* Read Generated Header */

  rc = read_to_file("circular_out.h", "r", &header_content, &sz);

  ASSERT_EQ(0, rc);

  /* ASSERTIONS: */

  /* 1. struct A; and struct B; must be present BEFORE their full definitions */

  {

    char *fwd_a;
    char *fwd_b;
    char *def_a;
    char *def_b;

    fwd_a = strstr(header_content, "struct A;");
    fwd_b = strstr(header_content, "struct B;");
    def_a = strstr(header_content, "struct A {");
    def_b = strstr(header_content, "struct B {");

    ASSERT(fwd_a != NULL);

    ASSERT(fwd_b != NULL);

    ASSERT(def_a != NULL);

    ASSERT(def_b != NULL);

    /* Verify Ordering */

    ASSERT(fwd_a < def_a);

    ASSERT(fwd_b < def_b);

    /* The referenced types inside the structs should rely on these forward

     * declarations being valid C */
  }

  free(header_content);

  remove(filename);

  remove("circular_out.h");

  remove("circular_out.c");
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief test json guards
 * @return TEST
 */
TEST test_codegen_config_json_guards(void) {

  /*

   * Verify that generated functions are wrapped in #ifdef TO_JSON ... #endif

   * when configured.

   */

  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif

  {
    struct StructFields sf;

    /* We use the specific config struct now */
    struct CodegenJsonConfig config;

    char *content = NULL;

    long sz;

    /* Setup */

    ASSERT(tmp);

    struct_fields_init(&sf);

    struct_fields_add(&sf, "x", "integer", NULL, NULL, NULL);

    memset(&config, 0, sizeof(config));

    config.guard_macro = (char *)(size_t)(size_t) "ENABLE_JSON";

    /* Generate */

    ASSERT_EQ(0, write_struct_to_json_func(tmp, "GuardStruct", &sf, &config));

    ASSERT_EQ(0, write_struct_from_json_func(tmp, "GuardStruct", &config));

    ASSERT_EQ(

        0, write_struct_from_jsonObject_func(tmp, "GuardStruct", &sf, &config));

    /* Check content */

    fseek(tmp, 0, SEEK_END);

    sz = ftell(tmp);

    rewind(tmp);

    content = (char *)(size_t)calloc(1, (size_t)sz + 1);

    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    /* Check Guards exist */

    ASSERT(strstr(content, "#ifdef ENABLE_JSON"));

    ASSERT(strstr(content, "#endif /* ENABLE_JSON */"));

    /* Verify the guards appear multiple times (once per function block) */

    {

      char *p;
      int count;
      p = content;

      count = 0;

      while ((p = strstr(p, "#ifdef ENABLE_JSON")) != NULL) {

        count++;

        p++;
      }

      /* We called 3 write_ functions, expecting 3 blocks */

      ASSERT_EQ(3, count);
    }

    free(content);

    struct_fields_free(&sf);

    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;

    PASS();
  }
}

/**
 * @brief test union json guards
 * @return TEST
 */
TEST test_union_config_json_guards(void) {

  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif

  {
    struct StructFields sf;

    /* Specific config struct */
    struct CodegenTypesConfig config;

    char *content = NULL;

    long sz;

    ASSERT(tmp);

    struct_fields_init(&sf);

    struct_fields_add(&sf, "x", "integer", NULL, NULL, NULL);

    memset(&config, 0, sizeof(config));

    config.json_guard = (char *)(size_t)(size_t) "UNION_GUARD";

    ASSERT_EQ(0, write_union_to_json_func(tmp, "U", &sf, &config));

    ASSERT_EQ(0, write_union_from_jsonObject_func(tmp, "U", &sf, &config));

    ASSERT_EQ(0, write_union_from_json_func(tmp, "U", &sf, &config));

    fseek(tmp, 0, SEEK_END);

    sz = ftell(tmp);

    rewind(tmp);

    content = (char *)(size_t)calloc(1, (size_t)sz + 1);

    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "#ifdef UNION_GUARD"));

    ASSERT(strstr(content, "#endif /* UNION_GUARD */"));

    free(content);

    struct_fields_free(&sf);

    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;

    PASS();
  }
}

/**
 * @brief test codegen union output
 * @return TEST
 */
TEST test_schema_codegen_union_output(void) {
  int rc = 0;
  char *header_content = NULL;
  char *source_content = NULL;
  size_t sz;
  const char *const filename = "union_schema.json";
  const char *argv[2];
  const char schema[] = {
      123, 34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 34,  58,  123,
      34,  115, 99,  104, 101, 109, 97,  115, 34,  58,  123, 34,  67,  97,  116,
      34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101,
      99,  116, 34,  44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115,
      34,  58,  123, 34,  109, 101, 111, 119, 34,  58,  123, 34,  116, 121, 112,
      101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 125, 44,
      34,  68,  111, 103, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,
      111, 98,  106, 101, 99,  116, 34,  44,  34,  112, 114, 111, 112, 101, 114,
      116, 105, 101, 115, 34,  58,  123, 34,  98,  97,  114, 107, 34,  58,  123,
      34,  116, 121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,
      125, 125, 125, 44,  34,  80,  101, 116, 34,  58,  123, 34,  111, 110, 101,
      79,  102, 34,  58,  91,  123, 34,  36,  114, 101, 102, 34,  58,  34,  35,
      47,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 47,  115, 99,  104,
      101, 109, 97,  115, 47,  67,  97,  116, 34,  125, 44,  123, 34,  36,  114,
      101, 102, 34,  58,  34,  35,  47,  99,  111, 109, 112, 111, 110, 101, 110,
      116, 115, 47,  115, 99,  104, 101, 109, 97,  115, 47,  68,  111, 103, 34,
      125, 93,  44,  34,  100, 105, 115, 99,  114, 105, 109, 105, 110, 97,  116,
      111, 114, 34,  58,  123, 34,  112, 114, 111, 112, 101, 114, 116, 121, 78,
      97,  109, 101, 34,  58,  34,  112, 101, 116, 84,  121, 112, 101, 34,  125,
      125, 125, 125, 125, 0};
  argv[0] = filename;
  argv[1] = "union_out";

  rc = write_to_file(filename, schema);
  ASSERT_EQ(0, rc);

  rc = schema2code_main(2, (char **)(size_t)argv);
  ASSERT_EQ(0, rc);

  rc = read_to_file("union_out.h", "r", &header_content, &sz);
  ASSERT_EQ(0, rc);
  rc = read_to_file("union_out.c", "r", &source_content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(header_content, "enum Pet_tag"));
  ASSERT(strstr(header_content, "struct Pet"));
  ASSERT(strstr(header_content, "Pet_from_json"));
  ASSERT(strstr(header_content, "Pet_to_json"));

  ASSERT(strstr(source_content, "Pet_from_jsonObject"));
  ASSERT(strstr(source_content, "Pet_from_json"));
  ASSERT(strstr(source_content, "Pet_to_json"));

  free(header_content);
  free(source_content);
  remove(filename);
  remove("union_out.h");
  remove("union_out.c");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test inline variants
 * @return TEST
 */
TEST test_schema_codegen_union_inline_variants(void) {
  int rc = 0;
  char *header_content = NULL;
  char *source_content = NULL;
  size_t sz;
  const char *const filename = "union_inline_schema.json";
  const char *argv[2];
  const char *schema =
      "{"
      "\"components\":{"
      "\"schemas\":{"
      "\"Pet\":{\"oneOf\":["
      "{\"title\":\"InlineCat\",\"type\":\"object\",\"properties\":"
      "{\"meow\":{\"type\":\"string\"}}},"
      "{\"title\":\"TagList\",\"type\":\"array\",\"items\":{\"type\":"
      "\"string\"}}"
      "]}"
      "}}}";
  argv[0] = filename;
  argv[1] = "union_inline_out";

  rc = write_to_file(filename, schema);
  ASSERT_EQ(0, rc);

  rc = schema2code_main(2, (char **)(size_t)argv);
  ASSERT_EQ(0, rc);

  rc = read_to_file("union_inline_out.h", "r", &header_content, &sz);
  ASSERT_EQ(0, rc);
  rc = read_to_file("union_inline_out.c", "r", &source_content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(header_content, "struct Pet_InlineCat"));
  ASSERT(strstr(header_content, "meow"));
  ASSERT(strstr(header_content, "struct Pet"));
  ASSERT(strstr(header_content, "n_TagList"));
  ASSERT(strstr(header_content, "TagList"));

  ASSERT(strstr(source_content, "case JSONArray"));
  ASSERT(strstr(source_content, "Pet_InlineCat_from_jsonObject"));

  free(header_content);
  free(source_content);
  remove(filename);
  remove("union_inline_out.h");
  remove("union_inline_out.c");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test enum output
 * @return TEST
 */
TEST test_schema_codegen_enum_output(void) {
  int rc = 0;
  char *header_content = NULL;
  char *source_content = NULL;
  size_t sz;
  const char *const filename = "enum_schema.json";
  const char *argv[2];
  const char *schema =
      "{"
      "\"components\":{"
      "\"schemas\":{"
      "\"Color\":{\"type\":\"string\",\"enum\":[\"RED\",\"GREEN\"]}"
      "}}}";
  argv[0] = filename;
  argv[1] = "enum_out";

  rc = write_to_file(filename, schema);
  ASSERT_EQ(0, rc);

  rc = schema2code_main(2, (char **)(size_t)argv);
  ASSERT_EQ(0, rc);

  rc = read_to_file("enum_out.h", "r", &header_content, &sz);
  ASSERT_EQ(0, rc);
  rc = read_to_file("enum_out.c", "r", &source_content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(header_content, "enum Color"));
  ASSERT(strstr(header_content, "Color_RED"));
  ASSERT(strstr(header_content, "Color_GREEN"));
  ASSERT(strstr(header_content, "Color_from_str"));
  ASSERT(strstr(header_content, "Color_to_str"));

  ASSERT(strstr(source_content, "Color_from_str"));
  ASSERT(strstr(source_content, "Color_to_str"));

  free(header_content);
  free(source_content);
  remove(filename);
  remove("enum_out.h");
  remove("enum_out.c");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test config guards
 * @return TEST
 */
TEST test_codegen_config_utils_guards(void) {

  /*

   * Verify that struct helpers are wrapped in #ifdef DATA_UTILS ... #endif

   * when configured.

   */

  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif

  {
    struct StructFields sf;

    /* Specific config struct */
    struct CodegenStructConfig config;

    char *content = NULL;

    long sz;

    ASSERT(tmp);

    struct_fields_init(&sf);

    struct_fields_add(&sf, "name", "string", NULL, NULL, NULL);

    memset(&config, 0, sizeof(config));

    config.guard_macro = (char *)(size_t)(size_t) "DATA_UTILS";

    /* Generate helpers */

    ASSERT_EQ(0, write_struct_cleanup_func(tmp, "S", &sf, &config));

    ASSERT_EQ(0, write_struct_debug_func(tmp, "S", &sf, &config));

    ASSERT_EQ(0, write_struct_deepcopy_func(tmp, "S", &sf, &config));

    ASSERT_EQ(0, write_struct_default_func(tmp, "S", &sf, &config));

    ASSERT_EQ(0, write_struct_display_func(tmp, "S", &sf, &config));

    ASSERT_EQ(0, write_struct_eq_func(tmp, "S", &sf, &config));

    fseek(tmp, 0, SEEK_END);

    sz = ftell(tmp);

    rewind(tmp);

    content = (char *)(size_t)calloc(1, (size_t)sz + 1);

    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    /* Just sample checks */

    ASSERT(strstr(content, "#ifdef DATA_UTILS"));

    ASSERT(strstr(content, "#endif /* DATA_UTILS */"));

    ASSERT(strstr(content, "cdd_c_error_t S_cleanup("));

    free(content);

    struct_fields_free(&sf);

    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;

    PASS();
  }
}

/**
 * @brief test schema constraints validation
 * @return TEST
 */
#ifdef CDD_BUILD_TESTS
/* extern C_CDD_EXPORT int g_schema_strdup_fail; (moved to global) */
/* extern C_CDD_EXPORT int g_schema_realloc_fail; (moved to global) */
#endif

TEST test_schema_constraints_bounds(void) {
  struct SchemaConstraints sc;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, schema_constraints_init(NULL));
  ASSERT_EQ(0, schema_constraints_init(&sc));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            schema_constraints_add_required(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            schema_constraints_add_required(&sc, NULL));

  ASSERT_EQ(0, schema_constraints_add_required(&sc, "field_a"));
  ASSERT_EQ(0, schema_constraints_add_required(&sc, "field_b"));

#ifdef CDD_BUILD_TESTS
  /* Test OOM via overflow */
  {
    size_t old_cap = sc.required_capacity;
    size_t old_count = sc.required_count;
    char **old_req = sc.required;
    sc.required_capacity = ((size_t)-1) / 16 - 100;
    sc.required_count = sc.required_capacity;

    ASSERT_EQ(CDD_C_ERROR_MEMORY, schema_constraints_add_required(&sc, "oom"));

    /* Force integer overflow on required_capacity * 2 */
    sc.required_capacity = ((size_t)-1) / 2 + 10;
    sc.required_count = sc.required_capacity;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              schema_constraints_add_required(&sc, "oom_overflow"));

    sc.required_capacity = old_cap;
    sc.required_count = old_count;
    sc.required = old_req;

    g_schema_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, schema_constraints_add_required(&sc, "oom2"));
    g_schema_strdup_fail = 0;

    old_cap = sc.required_capacity;
    sc.required_capacity = sc.required_count;
    g_schema_realloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, schema_constraints_add_required(&sc, "oom3"));
    g_schema_realloc_fail = 0;
    sc.required_capacity = old_cap;
  }
#endif

  schema_constraints_free(&sc);
  schema_constraints_free(NULL); /* Should be safe */

  /* Test cleanup of additional properties */
  schema_constraints_init(&sc);
  sc.has_additional_properties = 1;
  sc.additional_properties =
      (struct SchemaType *)calloc(1, sizeof(struct SchemaType));
  sc.additional_properties->name = (char *)(size_t)malloc(2);
#if defined(_MSC_VER)
  strcpy_s(sc.additional_properties->name, 2, "n");
#else
  strcpy(sc.additional_properties->name, "n");
#endif
  sc.additional_properties->type = (char *)(size_t)malloc(2);
#if defined(_MSC_VER)
  strcpy_s(sc.additional_properties->type, 2, "t");
#else
  strcpy(sc.additional_properties->type, "t");
#endif
  sc.additional_properties->ref = (char *)(size_t)malloc(2);
#if defined(_MSC_VER)
  strcpy_s(sc.additional_properties->ref, 2, "r");
#else
  strcpy(sc.additional_properties->ref, "r");
#endif
  schema_constraints_free(&sc);
  g_fail_io_after = -1;

  PASS();
}
/**
 * @brief Suite for schema codegen
 */

#ifdef CDD_BUILD_TESTS
/* extern C_CDD_EXPORT int g_schema_fail_io_after; (moved to global) */
/* extern C_CDD_EXPORT int g_schema_io_calls; (moved to global) */
#endif

SUITE(schema_codegen_suite) {
  RUN_TEST(test_schema_constraints_bounds);
  RUN_TEST(test_schema_codegen_circular_refs);
  RUN_TEST(test_codegen_config_json_guards);
  RUN_TEST(test_union_config_json_guards);
  RUN_TEST(test_schema_codegen_union_output);
  RUN_TEST(test_schema_codegen_union_inline_variants);
  RUN_TEST(test_schema_codegen_enum_output);
  RUN_TEST(test_codegen_config_utils_guards);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_SCHEMA_CODEGEN_H */
