extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_schema_codegen.h
 * @brief Unit tests for schema to code generation.
 */

#include "classes/emit/schema.h"
#ifndef TEST_SCHEMA_CODEGEN_H

#define TEST_SCHEMA_CODEGEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
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

  int rc;
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

  rc = schema2code_main(2, (char **)argv);

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

  FILE *tmp = tmpfile();

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

  config.guard_macro = "ENABLE_JSON";

  /* Generate */

  ASSERT_EQ(0, write_struct_to_json_func(tmp, "GuardStruct", &sf, &config));

  ASSERT_EQ(0, write_struct_from_json_func(tmp, "GuardStruct", &config));

  ASSERT_EQ(

      0, write_struct_from_jsonObject_func(tmp, "GuardStruct", &sf, &config));

  /* Check content */

  fseek(tmp, 0, SEEK_END);

  sz = ftell(tmp);

  rewind(tmp);

  content = (char *)calloc(1, sz + 1);

  fread(content, 1, sz, tmp);

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

  fclose(tmp);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief test union json guards
 * @return TEST
 */
TEST test_union_config_json_guards(void) {

  FILE *tmp = tmpfile();

  struct StructFields sf;

  /* Specific config struct */
  struct CodegenTypesConfig config;

  char *content = NULL;

  long sz;

  ASSERT(tmp);

  struct_fields_init(&sf);

  struct_fields_add(&sf, "x", "integer", NULL, NULL, NULL);

  memset(&config, 0, sizeof(config));

  config.json_guard = "UNION_GUARD";

  ASSERT_EQ(0, write_union_to_json_func(tmp, "U", &sf, &config));

  ASSERT_EQ(0, write_union_from_jsonObject_func(tmp, "U", &sf, &config));

  ASSERT_EQ(0, write_union_from_json_func(tmp, "U", &sf, &config));

  fseek(tmp, 0, SEEK_END);

  sz = ftell(tmp);

  rewind(tmp);

  content = (char *)calloc(1, sz + 1);

  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "#ifdef UNION_GUARD"));

  ASSERT(strstr(content, "#endif /* UNION_GUARD */"));

  free(content);

  struct_fields_free(&sf);

  fclose(tmp);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief test codegen union output
 * @return TEST
 */
TEST test_schema_codegen_union_output(void) {
  int rc;
  char *header_content = NULL;
  char *source_content = NULL;
  size_t sz;
  const char *const filename = "union_schema.json";
  const char *argv[2];
  const char *schema = "{"
                       "\"components\":{"
                       "\"schemas\":{"
                       "\"Cat\":{\"type\":\"object\",\"properties\":{\"meow\":{"
                       "\"type\":\"string\"}}},"
                       "\"Dog\":{\"type\":\"object\",\"properties\":{\"bark\":{"
                       "\"type\":\"string\"}}},"
                       "\"Pet\":{\"oneOf\":["
                       "{\"$ref\":\"#/components/schemas/Cat\"},"
                       "{\"$ref\":\"#/components/schemas/Dog\"}"
                       "],\"discriminator\":{\"propertyName\":\"petType\"}}"
                       "}}}";
  argv[0] = filename;
  argv[1] = "union_out";

  rc = write_to_file(filename, schema);
  ASSERT_EQ(0, rc);

  rc = schema2code_main(2, (char **)argv);
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
  int rc;
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

  rc = schema2code_main(2, (char **)argv);
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
  int rc;
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

  rc = schema2code_main(2, (char **)argv);
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

  FILE *tmp = tmpfile();

  struct StructFields sf;

  /* Specific config struct */
  struct CodegenStructConfig config;

  char *content = NULL;

  long sz;

  ASSERT(tmp);

  struct_fields_init(&sf);

  struct_fields_add(&sf, "name", "string", NULL, NULL, NULL);

  memset(&config, 0, sizeof(config));

  config.guard_macro = "DATA_UTILS";

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

  content = (char *)calloc(1, sz + 1);

  fread(content, 1, sz, tmp);

  /* Just sample checks */

  ASSERT(strstr(content, "#ifdef DATA_UTILS"));

  ASSERT(strstr(content, "#endif /* DATA_UTILS */"));

  ASSERT(strstr(content, "void S_cleanup("));

  free(content);

  struct_fields_free(&sf);

  fclose(tmp);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief test schema constraints validation
 * @return TEST
 */
#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_schema_strdup_fail;
#endif

TEST test_schema_constraints_bounds(void) {
  struct SchemaConstraints sc;
  if (getenv("RUNNING_UNDER_VALGRIND"))
    SKIPm("Valgrind crash");

  ASSERT_EQ(EINVAL, schema_constraints_init(NULL));
  ASSERT_EQ(0, schema_constraints_init(&sc));

  ASSERT_EQ(EINVAL, schema_constraints_add_required(NULL, "a"));
  ASSERT_EQ(EINVAL, schema_constraints_add_required(&sc, NULL));

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

    ASSERT_EQ(ENOMEM, schema_constraints_add_required(&sc, "oom"));

    sc.required_capacity = old_cap;
    sc.required_count = old_count;
    sc.required = old_req;

    g_schema_strdup_fail = 1;
    ASSERT_EQ(ENOMEM, schema_constraints_add_required(&sc, "oom2"));
    g_schema_strdup_fail = 0;
  }
#endif

  schema_constraints_cleanup(&sc);
  schema_constraints_cleanup(NULL); /* Should be safe */

  /* Test cleanup of additional properties */
  schema_constraints_init(&sc);
  sc.has_additional_properties = 1;
  sc.additional_properties =
      (struct SchemaType *)calloc(1, sizeof(struct SchemaType));
  sc.additional_properties->name = (char *)malloc(2);
  strcpy(sc.additional_properties->name, "n");
  sc.additional_properties->type = (char *)malloc(2);
  strcpy(sc.additional_properties->type, "t");
  sc.additional_properties->ref = (char *)malloc(2);
  strcpy(sc.additional_properties->ref, "r");
  schema_constraints_cleanup(&sc);
  g_fail_io_after = -1;

  PASS();
}
/**
 * @brief Suite for schema codegen
 */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_schema_fail_io_after;
extern C_CDD_EXPORT int g_schema_io_calls;
#endif

TEST test_schema_codegen_cli_exhaustive_io(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  int rc;
  const char *schema_json = "{"
                            "\"components\": {"
                            "  \"schemas\": {"
                            "    \"MyStruct\": {"
                            "      \"type\": \"object\","
                            "      \"properties\": {"
                            "        \"foo\": { \"type\": \"string\" }"
                            "      }"
                            "    }"
                            "  }"
                            "}"
                            "}";
  FILE *f = fopen("test_codegen_schema_io.json", "w");
  fputs(schema_json, f);
  fclose(f);

  for (i = 0; i < 1000; ++i) {
    void *root;
    void *schemas;
    g_schema_fail_io_after = i;
    g_schema_io_calls = 0;

    root = json_parse_file("test_codegen_schema_io.json");
    schemas = json_object_get_object(json_value_get_object(root), "components");
    schemas = json_object_get_object(schemas, "schemas");

    rc = generate_header("test_codegen_schema_io.json", "out_prefix", schemas,
                         NULL);
    if (rc == 0)
      rc = generate_source("test_codegen_schema_io.json", "out_prefix", schemas,
                           NULL);
    json_value_free(root);

    if (rc == 0)
      break;
    ASSERT(rc != 0);
  }

  g_schema_fail_io_after = -1;
  remove("test_codegen_schema_io.json");
#endif
  g_fail_io_after = -1;
  PASS();
}

TEST test_schema_codegen_union_arrays(void) {
  int rc;
  const char *const filename = "union_array_schema.json";
  const char *argv[2];
  const char *schema =
      "{"
      "\"components\":{"
      "\"schemas\":{"
      "\"Pet\":{\"type\":\"object\",\"properties\":{\"meow\":{\"type\":"
      "\"string\"}}},"
      "\"UnionNumber\": { \"oneOf\": [ { \"type\": \"array\", \"items\": { "
      "\"type\": \"number\" } } ] },\"UnionInteger\": { \"oneOf\": [ { "
      "\"type\": \"array\", \"items\": { \"type\": \"integer\" } } ] },"
      "\"UnionBool\": { \"oneOf\": [ { \"type\": \"array\", \"items\": { "
      "\"type\": \"boolean\" } } ] },"
      "\"UnionRef\": { \"oneOf\": [ { \"type\": \"array\", \"items\": { "
      "\"$ref\": \"#/components/schemas/Pet\" } } ] }"
      "}}}";
  argv[0] = filename;
  argv[1] = "union_array_out";

  rc = write_to_file(filename, schema);
  ASSERT_EQ(0, rc);

  {
    void *root = json_parse_file(filename);
    void *schemas =
        json_object_get_object(json_value_get_object(root), "components");
    schemas = json_object_get_object(schemas, "schemas");
    rc = generate_header(filename, argv[1], schemas, NULL);
    if (rc == 0)
      rc = generate_source(filename, argv[1], schemas, NULL);
    json_value_free(root);
  }
  ASSERT_EQ(0, rc);

  remove(filename);
  remove("union_array_out.h");
  remove("union_array_out.c");
  remove("union_array_out_types.h");
  g_fail_io_after = -1;
  PASS();
}

TEST test_schema_codegen_specific_structs(void) {
  int rc;
  const char *const filename = "specific_structs.json";
  const char *argv[2];
  const char *schema =
      "{"
      "\"components\":{"
      "\"schemas\":{"
      "\"OAuth2Error\":{\"type\":\"object\",\"properties\":{\"error\":{"
      "\"type\":\"string\"}}},"
      "\"JwtPayload\":{\"type\":\"object\",\"properties\":{\"sub\":{\"type\":"
      "\"string\"}}},"
      "\"OAuth2TokenResponse\":{\"type\":\"object\",\"properties\":{\"access_"
      "token\":{\"type\":\"string\"}}}"
      "}}}";
  argv[0] = filename;
  argv[1] = "specific_out";

  rc = write_to_file(filename, schema);
  ASSERT_EQ(0, rc);

  rc = schema2code_main(2, (char **)argv);
  ASSERT_EQ(0, rc);

  remove(filename);
  remove("specific_out.h");
  remove("specific_out.c");
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_fail_alloc;
#endif

TEST test_schema_codegen_main_paths(void) {
  int rc;
  const char *const filename = "main_paths.json";
  const char *argv[5];
  const char *schema_defs =
      "{"
      "\"$defs\":{"
      "\"X\":{\"type\":\"object\",\"properties\":{\"x\":{\"type\":\"string\"}}}"
      "}}";

  /* 1. argc < 2 */
  rc = schema2code_main(1, (char **)argv);
  ASSERT(rc != 0);

  /* 2. get_basename fails */
#ifdef CDD_BUILD_TESTS
  argv[0] = "a";
  argv[1] = "b";
  g_cdd_fail_alloc = 1;
  rc = schema2code_main(2, (char **)argv);
  ASSERT(rc != 0);
  g_cdd_fail_alloc = 0;
#endif

  /* 3. flags parsing */
  rc = write_to_file(filename, schema_defs);
  ASSERT_EQ(0, rc);
  argv[0] = filename;
  argv[1] = "main_out";
  argv[2] = "--guard-enum=EG";
  argv[3] = "--guard-json=JG";
  argv[4] = "--guard-utils=UG";
  rc = schema2code_main(5, (char **)argv);
  ASSERT_EQ(0, rc);
  remove("main_out.h");
  remove("main_out.c");

  /* 4. json_parse_file fails */
  argv[0] = "does_not_exist.json";
  rc = schema2code_main(2, (char **)argv);
  ASSERT(rc != 0);

  /* 5. missing schemas */
  write_to_file(filename, "{}");
  argv[0] = filename;
  rc = schema2code_main(2, (char **)argv);
  ASSERT(rc != 0);

  /* 6. generate_header / generate_source fails */
#ifdef CDD_BUILD_TESTS
  write_to_file(filename, schema_defs);
  g_schema_fail_io_after = 0;
  g_schema_io_calls = 0;
  rc = schema2code_main(2, (char **)argv);
  ASSERT(rc != 0);

  g_schema_fail_io_after = 3; /* Succeed header start, fail later */
  g_schema_io_calls = 0;
  rc = schema2code_main(2, (char **)argv);
  ASSERT(rc != 0);

  g_schema_fail_io_after = -1;
#endif

  remove(filename);
  remove("main_out.h");
  remove("main_out.c");
  g_fail_io_after = -1;
  PASS();
}

SUITE(schema_codegen_suite) {
  RUN_TEST(test_schema_codegen_cli_exhaustive_io);
  RUN_TEST(test_schema_constraints_bounds);
  RUN_TEST(test_schema_codegen_circular_refs);
  RUN_TEST(test_codegen_config_json_guards);
  RUN_TEST(test_union_config_json_guards);
  RUN_TEST(test_schema_codegen_union_output);
  RUN_TEST(test_schema_codegen_union_inline_variants);
  RUN_TEST(test_schema_codegen_union_arrays);
  RUN_TEST(test_schema_codegen_enum_output);
  RUN_TEST(test_codegen_config_utils_guards);
  RUN_TEST(test_schema_codegen_specific_structs);
  RUN_TEST(test_schema_codegen_main_paths);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_SCHEMA_CODEGEN_H */
