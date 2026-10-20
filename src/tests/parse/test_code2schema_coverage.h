/**
 * @file test_code2schema_coverage.h
 * @brief Unit tests and mocks in test_code2schema_coverage.h.
 */
#ifndef TEST_CODE2SCHEMA_COVERAGE_H
#define TEST_CODE2SCHEMA_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include "routes/parse/cli.h"
#include <greatest.h>
#include <parson.h>
/* clang-format on */

extern C_CDD_EXPORT cdd_c_error_t c2s_parse_union_tokens_and_write(
    const struct TokenList *tl, size_t *idx, JSON_Object *schemas_obj,
    const char *union_name);

/**
 * @brief Helper function set_both_parson_allocs_c2s.
 * @param[in,out] malloc_fun Parameter malloc_fun.
 * @param[in,out] free_fun Parameter free_fun.
 */
static void set_both_parson_allocs_c2s(void *(*malloc_fun)(size_t),
                                       void (*free_fun)(void *)) {
  (json_set_allocation_functions)(malloc_fun, free_fun);
  c2openapi_set_json_allocators(malloc_fun, free_fun);
}
#ifdef json_set_allocation_functions
#undef json_set_allocation_functions
#endif
/**
 * @brief Macro json_set_allocation_functions.
 */
#define json_set_allocation_functions set_both_parson_allocs_c2s

/** @brief Test definition g_parson_fail_at. */
static int g_parson_fail_at = -1;
/**
 * @brief Helper function mock_parson_malloc.
 * @param[in] sz Parameter sz.
 * @return Result status or value.
 */
static void *mock_parson_malloc(size_t sz) {
  if (g_parson_fail_at == 0) {
    g_parson_fail_at = -1;
    return NULL;
  }
  if (g_parson_fail_at > 0)
    g_parson_fail_at--;
  return malloc(sz);
}
/**
 * @brief Helper function mock_parson_free.
 * @param[in,out] ptr Parameter ptr.
 */
static void mock_parson_free(void *ptr) { free(ptr); }

/**
 * @brief Unit test test_code2schema_coverage_oom.
 * @return Greatest test result.
 */
TEST test_code2schema_coverage_oom(void) {
  int i;
  JSON_Value *val;
  JSON_Object *obj;

  for (i = 1; i < 50; ++i) {
    struct StructFields sf;

    val = json_parse_string("{\"type\":\"object\",\"properties\":{\"name\":{"
                            "\"type\":\"string\",\"x-extra\":true}}}");
    obj = json_value_get_object(val);

    struct_fields_init(&sf);

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;

    json_object_to_struct_fields(obj, &sf, NULL);

    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    struct_fields_free(&sf);
    json_value_free(val);
  }
  PASS();
}

/**
 * @brief Unit test test_code2schema_write_struct_oom.
 * @return Greatest test result.
 */
TEST test_code2schema_write_struct_oom(void) {
  int i;
  for (i = 1; i < 80; ++i) {
    struct StructFields sf;
    JSON_Value *root = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(root);

    struct_fields_init(&sf);
    c_cdd_strdup("{\"x-extra\":1}", &sf.schema_extra_json);
    struct_fields_add(&sf, "arr_field", "array", "string", NULL, NULL);
    c_cdd_strdup("{\"x-items\":1}", &sf.fields[0].items_extra_json);
    sf.fields[0].has_min_items = 1;
    sf.fields[0].min_items = 1;

    struct_fields_add(&sf, "obj_field", "object", "MyObj", NULL, NULL);
    struct_fields_add(&sf, "int_field", "integer", NULL, "42", NULL);
    c_cdd_strdup("{\"x-prop\":1}", &sf.fields[2].schema_extra_json);
    sf.fields[2].has_min = 1;
    sf.fields[2].min_val = 0;

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;

    write_struct_to_json_schema(schemas_obj, "TestStruct", &sf);

    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    json_value_free(root);
    struct_fields_free(&sf);
  }
  for (i = 1; i < 40; ++i) {
    struct StructFields sf;
    JSON_Value *root = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(root);

    struct_fields_init(&sf);
    struct_fields_add(&sf, "int_field", "integer", NULL, NULL, NULL);
    c_cdd_strdup("{\"x-prop\":1}", &sf.fields[0].schema_extra_json);

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;
    write_struct_to_json_schema(schemas_obj, "TestPropExtra", &sf);
    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    json_value_free(root);
    struct_fields_free(&sf);
  }
  for (i = 1; i < 40; ++i) {
    struct StructFields sf;
    JSON_Value *root = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(root);

    struct_fields_init(&sf);
    c_cdd_strdup("{\"x-struct\":1}", &sf.schema_extra_json);

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;
    write_struct_to_json_schema(schemas_obj, "TestStructExtra", &sf);
    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    json_value_free(root);
    struct_fields_free(&sf);
  }
  {
    FILE *fh = fopen("build/cdd_test_c2s_write_oom.h", "w");
    char *av[2];
    fputs("struct S { int x; };\n", fh);
    fclose(fh);
    av[0] = cdd_test_mut_str("build/cdd_test_c2s_write_oom.h");
    av[1] = cdd_test_mut_str("build/cdd_test_c2s_write_oom.json");
    for (i = 1; i < 30; ++i) {
      json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
      g_parson_fail_at = i;
      code2schema_main(2, av);
      json_set_allocation_functions(malloc, free);
      g_parson_fail_at = -1;
    }
    remove("build/cdd_test_c2s_write_oom.h");
    remove("build/cdd_test_c2s_write_oom.json");
  }
  PASS();
}

/**
 * @brief Unit test test_code2schema_union_oom.
 * @return Greatest test result.
 */
TEST test_code2schema_union_oom(void) {
  int i;
  for (i = 1; i < 50; ++i) {
    struct StructFields sf;
    JSON_Value *root = json_value_init_object();
    JSON_Object *root_obj = json_value_get_object(root);
    JSON_Value *val =
        json_parse_string("{\"discriminator\":{\"propertyName\":\"kind\"},"
                          "\"oneOf\":["
                          "  "
                          "{\"type\":\"object\",\"properties\":{\"f\":{"
                          "\"type\":\"string\"}},\"required\":[\"f\"]},"
                          "  "
                          "{\"type\":\"array\",\"items\":{\"properties\":{"
                          "\"x\":{\"type\":\"integer\"}}}}"
                          "]}");
    JSON_Object *obj = json_value_get_object(val);
    struct_fields_init(&sf);

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;

    json_object_to_struct_fields_ex_codegen(obj, &sf, root_obj, "UnionSchema");

    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    struct_fields_free(&sf);
    json_value_free(val);
    json_value_free(root);
  }
  PASS();
}

/**
 * @brief Unit test test_code2schema_write_oom.
 * @return Greatest test result.
 */
TEST test_code2schema_write_oom(void) {
  int i;
  for (i = 1; i < 50; ++i) {
    struct StructFields sf;
    JSON_Value *root = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(root);
    char *out_s = NULL;

    struct_fields_init(&sf);
    c_cdd_strdup("{\"x-cdd-extra\": true}", &out_s);
    sf.schema_extra_json = out_s;
    sf.is_enum = 1;
    sf.enum_members.members = (char **)C_CDD_MALLOC(sizeof(char *) * 2);
    sf.enum_members.size = 2;
    sf.enum_members.capacity = 2;
    c_cdd_strdup("A", &out_s);
    sf.enum_members.members[0] = out_s;
    sf.enum_members.members[1] = NULL;

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;

    write_struct_to_json_schema(schemas_obj, "TestStruct", &sf);

    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    json_value_free(root);
    struct_fields_free(&sf);
  }
  PASS();
}

/**
 * @brief Unit test test_code2schema_cdd_alloc_fail_comprehensive.
 * @return Greatest test result.
 */
TEST test_code2schema_cdd_alloc_fail_comprehensive(void) {
  int i;

  /* 1. c2s_json_object_to_struct_fields with OOM */
  for (i = 1; i <= 40; ++i) {
    struct StructFields sf;
    JSON_Value *val = json_parse_string(
        "{\"type\":\"object\",\"properties\":{"
        "  \"f_str\":{\"type\":\"string\",\"x-extra\":1},"
        "  \"f_arr\":{\"type\":\"array\",\"items\":{\"type\":\"integer\"}},"
        "  \"f_enum\":{\"type\":\"string\",\"enum\":[\"A\",\"B\"]},"
        "  \"f_union\":{\"type\":[\"string\",\"null\"]},"
        "  "
        "\"f_arr_un\":{\"type\":\"array\",\"items\":{\"type\":[\"number\","
        "\"null\"]}}"
        "},\"allOf\":[{\"type\":\"object\",\"properties\":{\"sub\":{\"type\":"
        "\"boolean\"}}}]"
        ",\"anyOf\":[{\"type\":\"object\",\"properties\":{\"a1\":{\"type\":"
        "\"string\"}}}]"
        ",\"oneOf\":[{\"type\":\"object\",\"properties\":{\"o1\":{\"type\":"
        "\"string\"}}}]}");
    JSON_Object *obj = json_value_get_object(val);
    struct_fields_init(&sf);

    g_cdd_alloc_fail = i;
    g_cdd_strdup_fail = i;
    c2s_json_object_to_struct_fields_internal(obj, &sf, NULL, "OomSchema", 1);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;

    struct_fields_free(&sf);
    json_value_free(val);
  }

  /* 2. Union codegen with OOM */
  for (i = 1; i <= 50; ++i) {
    struct StructFields sf;
    JSON_Value *root = json_value_init_object();
    JSON_Object *root_obj = json_value_get_object(root);
    JSON_Value *val = json_parse_string(
        "{\"discriminator\":{\"propertyName\":\"kind\"},"
        "\"oneOf\":["
        "  "
        "{\"type\":\"object\",\"title\":\"V1\",\"properties\":{\"p\":{\"type\":"
        "\"string\"}},\"required\":[\"p\"]},"
        "  {\"type\":\"array\",\"items\":{\"type\":\"string\"}},"
        "  "
        "{\"type\":\"array\",\"items\":{\"properties\":{\"p2\":{\"type\":"
        "\"integer\"}}}},"
        "  {\"type\":\"boolean\"}"
        "]}");
    JSON_Object *obj = json_value_get_object(val);
    struct_fields_init(&sf);

    g_cdd_alloc_fail = i;
    g_cdd_strdup_fail = i;
    json_object_to_struct_fields_ex_codegen(obj, &sf, root_obj, "UnionOom");
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;

    struct_fields_free(&sf);
    json_value_free(val);
    json_value_free(root);
  }

  /* 3. write_struct_to_json_schema with OOM */
  for (i = 1; i <= 40; ++i) {
    struct StructFields sf;
    JSON_Value *root = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(root);
    char *arr_types[2];
    arr_types[0] = C_CDD_STR_LIT("string");
    arr_types[1] = C_CDD_STR_LIT("null");

    struct_fields_init(&sf);
    c_cdd_strdup("{\"x-custom\":1}", &sf.schema_extra_json);
    struct_fields_add(&sf, "arr_field", "array", "ItemModel", NULL, NULL);
    copy_string_array_code2schema(&sf.fields[0].items_type_union,
                                  &sf.fields[0].n_items_type_union, arr_types,
                                  2);
    c_cdd_strdup("{\"x-item\":2}", &sf.fields[0].items_extra_json);
    struct_fields_add(&sf, "str_field", "string", NULL, NULL, NULL);
    sf.fields[1].required = 1;

    g_cdd_alloc_fail = i;
    g_cdd_strdup_fail = i;
    write_struct_to_json_schema(schemas_obj, "OomStruct", &sf);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;

    struct_fields_free(&sf);
    json_value_free(root);
  }

  /* 4. merge_struct_fields with OOM */
  for (i = 1; i <= 20; ++i) {
    struct StructFields src, dest;
    char *arr_types[2];
    arr_types[0] = C_CDD_STR_LIT("string");
    arr_types[1] = C_CDD_STR_LIT("null");

    struct_fields_init(&src);
    struct_fields_init(&dest);
    struct_fields_add(&src, "f1", "string", NULL, NULL, NULL);
    c_cdd_strdup("{\"x\":1}", &src.fields[0].schema_extra_json);
    copy_string_array_code2schema(&src.fields[0].type_union,
                                  &src.fields[0].n_type_union, arr_types, 2);

    g_cdd_alloc_fail = i;
    g_cdd_strdup_fail = i;
    merge_struct_fields(&dest, &src);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;

    struct_fields_free(&src);
    struct_fields_free(&dest);
  }

  /* 5. parse_struct_member_line with OOM */
  for (i = 1; i <= 15; ++i) {
    struct StructFields sf;
    struct_fields_init(&sf);

    g_cdd_alloc_fail = i;
    g_cdd_strdup_fail = i;
    parse_struct_member_line(
        "uint32_t numbers[10]; // @shard_key @slow_query_warn(500)", &sf);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;

    struct_fields_free(&sf);
  }

  /* 6. code2schema_main with OOM */
  {
    FILE *fp = fopen("test_main_oom.h", "w");
    if (fp) {
      fprintf(fp, "struct MainOom { int val; };\n");
      fclose(fp);

      for (i = 1; i <= 15; ++i) {
        char *argv[3];
        argv[0] = C_CDD_STR_LIT("test_main_oom.h");
        argv[1] = C_CDD_STR_LIT("test_main_oom.json");
        argv[2] = NULL;

        g_cdd_alloc_fail = i;
        g_cdd_strdup_fail = i;
        code2schema_main(2, argv);
        g_cdd_alloc_fail = 0;
        g_cdd_strdup_fail = 0;
        remove("test_main_oom.json");
      }
      remove("test_main_oom.h");
    }
  }

  /* 8. Parson failure loops for c2s_merge_schema_extras_strings and
   * register_inline_schema_c2s */
  json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
  for (i = 1; i <= 20; ++i) {
    char *dest = NULL;
    c_cdd_strdup("{\"a\":1}", &dest);
    g_parson_fail_at = i;
    c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_parson_fail_at = -1;
    C_CDD_FREE(dest);
  }

  for (i = 1; i <= 15; ++i) {
    JSON_Value *root_val = json_value_init_object();
    JSON_Object *root = json_value_get_object(root_val);
    JSON_Value *sch_val = json_parse_string("{\"type\":\"string\"}");
    char *out_name = NULL;
    g_parson_fail_at = i;
    register_inline_schema_c2s(root, "Schema", "Variant", "Suffix", sch_val,
                               &out_name);
    g_parson_fail_at = -1;
    json_value_free(sch_val);
    json_value_free(root_val);
  }

  for (i = 1; i <= 10; ++i) {
    JSON_Value *v = json_value_init_object();
    JSON_Object *o = json_value_get_object(v);
    char *types[2];
    types[0] = C_CDD_STR_LIT("string");
    types[1] = C_CDD_STR_LIT("null");
    g_parson_fail_at = i;
    c2s_write_type_union(o, "string", types, 2);
    g_parson_fail_at = -1;
    json_value_free(v);
  }
  {
    for (i = 0; i <= 10; ++i) {
      JSON_Value *v = json_value_init_object();
      JSON_Object *o = json_value_get_object(v);
      struct StructField sf_def;
      memset(&sf_def, 0, sizeof(sf_def));
      CDD_STRCPY(sf_def.default_val, sizeof(sf_def.default_val), "nullptr");
      CDD_STRCPY(sf_def.type, sizeof(sf_def.type), "object");
      g_parson_fail_at = i;
      c2s_write_default_value(o, &sf_def);
      g_parson_fail_at = -1;
      json_value_free(v);
    }
  }
  {
    struct TokenList *utl = NULL;
    json_set_allocation_functions(malloc, free);
    tokenize(az_span_create_from_str("{ int u_i; char *u_s; };"), &utl);
    if (utl) {
      for (i = 0; i <= 35; ++i) {
        size_t uidx = 1;
        JSON_Value *v = json_value_init_object();
        JSON_Object *o = json_value_get_object(v);
        json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
        g_parson_fail_at = i;
        c2s_parse_union_tokens_and_write(utl, &uidx, o, "MyUnion");
        g_parson_fail_at = -1;
        json_set_allocation_functions(malloc, free);
        json_value_free(v);
      }
      free_token_list(utl);
    }
  }
  json_set_allocation_functions(malloc, free);

  {
    struct StructFields sf_null_sch;
    struct_fields_init(&sf_null_sch);
    struct_fields_add(&sf_null_sch, "int_field", "integer", NULL, NULL, NULL);
    sf_null_sch.fields[0].required = 1;
    write_struct_to_json_schema(NULL, "TestNullSchemas", &sf_null_sch);
    struct_fields_free(&sf_null_sch);
  }

  /* Test c2s_merge_schema_extras_strings when g_cdd_fail_json_set_value is set
   */
  {
    extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
    char *dest = NULL;
    JSON_Value *tv = json_value_init_object();
    JSON_Object *to = json_value_get_object(tv);
    c_cdd_strdup("{\"k1\":1}", &dest);
    g_cdd_fail_json_set_value = 1;
    c2s_merge_schema_extras_strings(&dest, "{\"k2\":2}");
    g_cdd_fail_json_set_value = 1;
    c2s_merge_schema_extras_object(to, "{\"k2\":2}");
    g_cdd_fail_json_set_value = 0;
    json_value_free(tv);
    C_CDD_FREE(dest);
  }

  PASS();
}

/**
 * @brief Executes the code2schema_coverage_suite test suite.
 */
SUITE(code2schema_coverage_suite) {
  RUN_TEST(test_code2schema_cdd_alloc_fail_comprehensive);
  RUN_TEST(test_code2schema_coverage_oom);
  RUN_TEST(test_code2schema_write_oom);
  RUN_TEST(test_code2schema_write_struct_oom);
  RUN_TEST(test_code2schema_union_oom);
}

#ifdef __cplusplus
}
#endif
#endif
