#ifndef TEST_CODE2SCHEMA_COVERAGE_H
#define TEST_CODE2SCHEMA_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "c_cdd/memory.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include <greatest.h>
#include <parson.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT volatile int g_c2s_helper_fail;
#endif

static int g_parson_fail_at = -1;
static void *mock_parson_malloc(size_t sz) {
  if (g_parson_fail_at == 0) {
    return NULL;
  }
  if (g_parson_fail_at > 0)
    g_parson_fail_at--;
  return malloc(sz);
}
static void mock_parson_free(void *ptr) { free(ptr); }

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

TEST test_code2schema_write_struct_oom(void) {
  int i;
  for (i = 1; i < 40; ++i) {
    struct StructFields sf;
    JSON_Value *root = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(root);

    struct_fields_init(&sf);
    c_cdd_strdup("{\"x-extra\":1}", &sf.schema_extra_json);
    struct_fields_add(&sf, "arr_field", "array", "string", NULL, NULL);
    c_cdd_strdup("{\"x-items\":2}", &sf.fields[0].items_extra_json);
    sf.fields[0].has_min_items = 1;
    sf.fields[0].min_items = 1;

    struct_fields_add(&sf, "obj_field", "object", "MyObj", NULL, NULL);
    struct_fields_add(&sf, "int_field", "integer", NULL, "42", NULL);
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
  PASS();
}

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

  /* 7. g_c2s_helper_fail error percolation loops */
  for (i = 1; i <= 35; ++i) {
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

    g_c2s_helper_fail = i;
    json_object_to_struct_fields_ex_codegen(obj, &sf, root_obj,
                                            "UnionHelperFail");
    g_c2s_helper_fail = 0;

    struct_fields_free(&sf);
    json_value_free(val);
    json_value_free(root);
  }

  for (i = 1; i <= 25; ++i) {
    struct StructFields sf;
    JSON_Value *root = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(root);

    struct_fields_init(&sf);
    c_cdd_strdup("{\"x-custom\":1}", &sf.schema_extra_json);
    struct_fields_add(&sf, "arr_field", "array", "ItemModel", NULL, NULL);
    sf.fields[0].has_min_items = 1;
    sf.fields[0].min_items = 1;
    struct_fields_add(&sf, "str_field", "string", NULL, NULL, NULL);
    sf.fields[1].has_min_len = 1;
    sf.fields[1].min_len = 2;
    struct_fields_add(&sf, "num_field", "number", NULL, NULL, NULL);
    sf.fields[2].has_min = 1;
    sf.fields[2].min_val = 1.0;
    CDD_STRCPY(sf.fields[2].default_val, sizeof(sf.fields[2].default_val),
               "10.0");

    g_c2s_helper_fail = i;
    write_struct_to_json_schema(schemas_obj, "HelperStruct", &sf);
    g_c2s_helper_fail = 0;

    struct_fields_free(&sf);
    json_value_free(root);
  }

  for (i = 1; i <= 20; ++i) {
    struct StructFields sf;
    struct_fields_init(&sf);

    g_c2s_helper_fail = i;
    parse_struct_member_line(
        "uint32_t numbers[10]; // @shard_key @slow_query_warn(500)", &sf);
    g_c2s_helper_fail = 0;

    struct_fields_free(&sf);
  }

  {
    FILE *fp = fopen("test_main_oom2.h", "w");
    if (fp) {
      fprintf(fp, "struct MainOom {\n  struct { int nested_x; } inner;\n  int "
                  "val;\n};\nunion U { int x; };\n");
      fclose(fp);

      for (i = 1; i <= 20; ++i) {
        char *argv[3];
        argv[0] = C_CDD_STR_LIT("test_main_oom2.h");
        argv[1] = C_CDD_STR_LIT("test_main_oom2.json");
        argv[2] = NULL;

        g_c2s_helper_fail = i;
        code2schema_main(2, argv);
        g_c2s_helper_fail = 0;
        remove("test_main_oom2.json");
      }
      remove("test_main_oom2.h");
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

  {
    JSON_Value *v = json_value_init_object();
    JSON_Object *o = json_value_get_object(v);
    char *types[2];
    types[0] = C_CDD_STR_LIT("string");
    types[1] = C_CDD_STR_LIT("null");
    g_parson_fail_at = 1;
    c2s_write_type_union(o, "string", types, 2);
    g_parson_fail_at = -1;
    json_value_free(v);
  }
  json_set_allocation_functions(malloc, free);

  PASS();
}

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
