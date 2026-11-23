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
#include "classes/emit/json.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include "openapi/parse/openapi.h"
#include "routes/parse/cli.h"
#include <greatest.h>
#include <parson.h>
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_c2s_fail_step;
extern C_CDD_EXPORT cdd_c_error_t c2s_parse_union_tokens_and_write(
    const struct TokenList *tl, size_t *idx, JSON_Object *schemas_obj,
    const char *union_name);

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

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"type\":\"object\",\"properties\":{\"name\":{"
          "\"type\":\"string\",\"x-extra\":true}}}",
          &_cdd_jv_1);
      val = _cdd_jv_1;
    }
    obj = json_value_get_object(val);

    g_cdd_last_alloc_err = struct_fields_init(&sf);

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;

    g_cdd_last_alloc_err = json_object_to_struct_fields(obj, &sf, NULL);

    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    g_cdd_last_alloc_err = struct_fields_free(&sf);
    g_cdd_last_alloc_err = json_value_free(val);
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
    JSON_Value *root = NULL;
    JSON_Object *schemas_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      root = _cdd_jv_1;
    }
    schemas_obj = json_value_get_object(root);

    g_cdd_last_alloc_err = struct_fields_init(&sf);
    g_cdd_last_alloc_err =
        c_cdd_strdup("{\"x-extra\":1}", &sf.schema_extra_json);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "arr_field", "array", "string", NULL, NULL);
    g_cdd_last_alloc_err =
        c_cdd_strdup("{\"x-items\":1}", &sf.fields[0].items_extra_json);
    sf.fields[0].has_min_items = 1;
    sf.fields[0].min_items = 1;

    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "obj_field", "object", "MyObj", NULL, NULL);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "int_field", "integer", NULL, "42", NULL);
    g_cdd_last_alloc_err =
        c_cdd_strdup("{\"x-prop\":1}", &sf.fields[2].schema_extra_json);
    sf.fields[2].has_min = 1;
    sf.fields[2].min_val = 0;

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;

    g_cdd_last_alloc_err =
        write_struct_to_json_schema(schemas_obj, "TestStruct", &sf);

    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    g_cdd_last_alloc_err = json_value_free(root);
    g_cdd_last_alloc_err = struct_fields_free(&sf);
  }
  for (i = 1; i < 40; ++i) {
    struct StructFields sf;
    JSON_Value *root = NULL;
    JSON_Object *schemas_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      root = _cdd_jv_1;
    }
    schemas_obj = json_value_get_object(root);

    g_cdd_last_alloc_err = struct_fields_init(&sf);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "int_field", "integer", NULL, NULL, NULL);
    g_cdd_last_alloc_err =
        c_cdd_strdup("{\"x-prop\":1}", &sf.fields[0].schema_extra_json);

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;
    g_cdd_last_alloc_err =
        write_struct_to_json_schema(schemas_obj, "TestPropExtra", &sf);
    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    g_cdd_last_alloc_err = json_value_free(root);
    g_cdd_last_alloc_err = struct_fields_free(&sf);
  }
  for (i = 1; i < 40; ++i) {
    struct StructFields sf;
    JSON_Value *root = NULL;
    JSON_Object *schemas_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      root = _cdd_jv_1;
    }
    schemas_obj = json_value_get_object(root);

    g_cdd_last_alloc_err = struct_fields_init(&sf);
    g_cdd_last_alloc_err =
        c_cdd_strdup("{\"x-struct\":1}", &sf.schema_extra_json);

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;
    g_cdd_last_alloc_err =
        write_struct_to_json_schema(schemas_obj, "TestStructExtra", &sf);
    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    g_cdd_last_alloc_err = json_value_free(root);
    g_cdd_last_alloc_err = struct_fields_free(&sf);
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
      g_cdd_last_alloc_err = code2schema_main(2, av);
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
    JSON_Value *root = NULL;
    JSON_Object *root_obj = NULL;
    JSON_Value *val = NULL;
    JSON_Object *obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      root = _cdd_jv_1;
    }
    root_obj = json_value_get_object(root);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"discriminator\":{\"propertyName\":\"kind\"},"
          "\"oneOf\":["
          "  "
          "{\"type\":\"object\",\"properties\":{\"f\":{"
          "\"type\":\"string\"}},\"required\":[\"f\"]},"
          "  "
          "{\"type\":\"array\",\"items\":{\"properties\":{"
          "\"x\":{\"type\":\"integer\"}}}}"
          "]}",
          &_cdd_jv_1);
      val = _cdd_jv_1;
    }
    obj = json_value_get_object(val);
    g_cdd_last_alloc_err = struct_fields_init(&sf);

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;

    g_cdd_last_alloc_err = json_object_to_struct_fields_ex_codegen(
        obj, &sf, root_obj, "UnionSchema");

    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    g_cdd_last_alloc_err = struct_fields_free(&sf);
    g_cdd_last_alloc_err = json_value_free(val);
    g_cdd_last_alloc_err = json_value_free(root);
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
    JSON_Value *root = NULL;
    JSON_Object *schemas_obj = NULL;
    char *out_s = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      root = _cdd_jv_1;
    }
    schemas_obj = json_value_get_object(root);

    g_cdd_last_alloc_err = struct_fields_init(&sf);
    g_cdd_last_alloc_err = c_cdd_strdup("{\"x-cdd-extra\": true}", &out_s);
    sf.schema_extra_json = out_s;
    sf.is_enum = 1;
    sf.enum_members.members = (char **)C_CDD_MALLOC(sizeof(char *) * 2);
    sf.enum_members.size = 2;
    sf.enum_members.capacity = 2;
    g_cdd_last_alloc_err = c_cdd_strdup("A", &out_s);
    sf.enum_members.members[0] = out_s;
    sf.enum_members.members[1] = NULL;

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;

    g_cdd_last_alloc_err =
        write_struct_to_json_schema(schemas_obj, "TestStruct", &sf);

    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    g_cdd_last_alloc_err = json_value_free(root);
    g_cdd_last_alloc_err = struct_fields_free(&sf);
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
  json_set_allocation_functions(malloc, free);
  for (i = 1; i <= 40; ++i) {
    struct StructFields sf;
    JSON_Value *val = NULL;
    JSON_Object *obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
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
          "\"string\"}}}]}",
          &_cdd_jv_1);
      val = _cdd_jv_1;
    }
    obj = json_value_get_object(val);
    g_cdd_last_alloc_err = struct_fields_init(&sf);

    g_cdd_alloc_fail = i;
    g_cdd_strdup_fail = i;
    g_cdd_last_alloc_err = c2s_json_object_to_struct_fields_internal(
        obj, &sf, NULL, "OomSchema", 1);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;

    g_cdd_last_alloc_err = struct_fields_free(&sf);
    g_cdd_last_alloc_err = json_value_free(val);
  }

  /* 2. Union codegen with OOM */
  for (i = 1; i <= 50; ++i) {
    struct StructFields sf;
    JSON_Value *root = NULL;
    JSON_Object *root_obj = NULL;
    JSON_Value *val = NULL;
    JSON_Object *obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      root = _cdd_jv_1;
    }
    root_obj = json_value_get_object(root);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"discriminator\":{\"propertyName\":\"kind\"},"
          "\"oneOf\":["
          "  "
          "{\"type\":\"object\",\"title\":\"V1\",\"properties\":{\"p\":{"
          "\"type\":"
          "\"string\"}},\"required\":[\"p\"]},"
          "  {\"type\":\"array\",\"items\":{\"type\":\"string\"}},"
          "  "
          "{\"type\":\"array\",\"items\":{\"properties\":{\"p2\":{\"type\":"
          "\"integer\"}}}},"
          "  {\"type\":\"boolean\"}"
          "]}",
          &_cdd_jv_1);
      val = _cdd_jv_1;
    }
    obj = json_value_get_object(val);
    g_cdd_last_alloc_err = struct_fields_init(&sf);

    g_cdd_alloc_fail = i;
    g_cdd_strdup_fail = i;
    g_cdd_last_alloc_err =
        json_object_to_struct_fields_ex_codegen(obj, &sf, root_obj, "UnionOom");
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;

    g_cdd_last_alloc_err = struct_fields_free(&sf);
    g_cdd_last_alloc_err = json_value_free(val);
    g_cdd_last_alloc_err = json_value_free(root);
  }

  /* 3. write_struct_to_json_schema with OOM */
  for (i = 1; i <= 40; ++i) {
    struct StructFields sf;
    JSON_Value *root = NULL;
    JSON_Object *schemas_obj = NULL;
    char *arr_types[2];
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      root = _cdd_jv_1;
    }
    schemas_obj = json_value_get_object(root);
    arr_types[0] = C_CDD_STR_LIT("string");
    arr_types[1] = C_CDD_STR_LIT("null");

    g_cdd_last_alloc_err = struct_fields_init(&sf);
    g_cdd_last_alloc_err =
        c_cdd_strdup("{\"x-custom\":1}", &sf.schema_extra_json);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "arr_field", "array", "ItemModel", NULL, NULL);
    g_cdd_last_alloc_err = copy_string_array_code2schema(
        &sf.fields[0].items_type_union, &sf.fields[0].n_items_type_union,
        arr_types, 2);
    g_cdd_last_alloc_err =
        c_cdd_strdup("{\"x-item\":2}", &sf.fields[0].items_extra_json);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "str_field", "string", NULL, NULL, NULL);
    sf.fields[1].required = 1;

    g_cdd_alloc_fail = i;
    g_cdd_strdup_fail = i;
    g_cdd_last_alloc_err =
        write_struct_to_json_schema(schemas_obj, "OomStruct", &sf);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;

    g_cdd_last_alloc_err = struct_fields_free(&sf);
    g_cdd_last_alloc_err = json_value_free(root);
  }

  /* 4. merge_struct_fields with OOM */
  for (i = 1; i <= 20; ++i) {
    struct StructFields src, dest;
    char *arr_types[2];
    arr_types[0] = C_CDD_STR_LIT("string");
    arr_types[1] = C_CDD_STR_LIT("null");

    g_cdd_last_alloc_err = struct_fields_init(&src);
    g_cdd_last_alloc_err = struct_fields_init(&dest);
    g_cdd_last_alloc_err =
        struct_fields_add(&src, "f1", "string", NULL, NULL, NULL);
    g_cdd_last_alloc_err =
        c_cdd_strdup("{\"x\":1}", &src.fields[0].schema_extra_json);
    g_cdd_last_alloc_err = copy_string_array_code2schema(
        &src.fields[0].type_union, &src.fields[0].n_type_union, arr_types, 2);

    g_cdd_alloc_fail = i;
    g_cdd_strdup_fail = i;
    g_cdd_last_alloc_err = merge_struct_fields(&dest, &src);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;

    g_cdd_last_alloc_err = struct_fields_free(&src);
    g_cdd_last_alloc_err = struct_fields_free(&dest);
  }

  /* 5. parse_struct_member_line with OOM */
  for (i = 1; i <= 15; ++i) {
    struct StructFields sf;
    g_cdd_last_alloc_err = struct_fields_init(&sf);

    g_cdd_alloc_fail = i;
    g_cdd_strdup_fail = i;
    g_cdd_last_alloc_err = parse_struct_member_line(
        "uint32_t numbers[10]; // @shard_key @slow_query_warn(500)", &sf);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;

    g_cdd_last_alloc_err = struct_fields_free(&sf);
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
        g_cdd_last_alloc_err = code2schema_main(2, argv);
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
  for (i = 1; i <= 40; ++i) {
    char *dest = NULL;
    g_cdd_last_alloc_err = c_cdd_strdup("{\"a\":1}", &dest);
    g_parson_fail_at = i;
    g_cdd_last_alloc_err = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_parson_fail_at = -1;
    C_CDD_FREE(dest);
  }

  for (i = 1; i <= 15; ++i) {
    JSON_Value *root_val = NULL;
    JSON_Object *root = NULL;
    JSON_Value *sch_val = NULL;
    char *out_name = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      root_val = _cdd_jv_1;
    }
    root = json_value_get_object(root_val);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("{\"type\":\"string\"}", &_cdd_jv_1);
      sch_val = _cdd_jv_1;
    }
    out_name = NULL;
    g_parson_fail_at = i;
    g_cdd_last_alloc_err = register_inline_schema_c2s(
        root, "Schema", "Variant", "Suffix", sch_val, &out_name);
    g_parson_fail_at = -1;
    C_CDD_FREE(out_name);
    out_name = NULL;
    g_cdd_last_alloc_err = json_value_free(sch_val);
    g_cdd_last_alloc_err = json_value_free(root_val);
  }

  for (i = 1; i <= 10; ++i) {
    JSON_Value *v = NULL;
    JSON_Object *o = NULL;
    char *types[2];
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      v = _cdd_jv_1;
    }
    o = json_value_get_object(v);
    types[0] = C_CDD_STR_LIT("string");
    types[1] = C_CDD_STR_LIT("null");
    g_parson_fail_at = i;
    g_cdd_last_alloc_err = c2s_write_type_union(o, "string", types, 2);
    g_parson_fail_at = -1;
    g_cdd_last_alloc_err = json_value_free(v);
  }
  {
    for (i = 0; i <= 10; ++i) {
      JSON_Value *v = NULL;
      JSON_Object *o = NULL;
      struct StructField sf_def;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
        v = _cdd_jv_1;
      }
      o = json_value_get_object(v);
      memset(&sf_def, 0, sizeof(sf_def));
      CDD_STRCPY(sf_def.default_val, sizeof(sf_def.default_val), "nullptr");
      CDD_STRCPY(sf_def.type, sizeof(sf_def.type), "object");
      g_parson_fail_at = i;
      g_cdd_last_alloc_err = c2s_write_default_value(o, &sf_def);
      g_parson_fail_at = -1;
      g_cdd_last_alloc_err = json_value_free(v);
    }
  }
  {
    struct TokenList *utl = NULL;
    json_set_allocation_functions(malloc, free);
    g_cdd_last_alloc_err =
        tokenize(az_span_create_from_str("{ int u_i; char *u_s; };"), &utl);
    if (utl) {
      for (i = 0; i <= 35; ++i) {
        size_t uidx = 1;
        JSON_Value *v = NULL;
        JSON_Object *o = NULL;
        {
          JSON_Value *_cdd_jv_1 = NULL;
          g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
          v = _cdd_jv_1;
        }
        o = json_value_get_object(v);
        json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
        g_parson_fail_at = i;
        g_cdd_last_alloc_err =
            c2s_parse_union_tokens_and_write(utl, &uidx, o, "MyUnion");
        g_parson_fail_at = -1;
        json_set_allocation_functions(malloc, free);
        g_cdd_last_alloc_err = cdd_json_value_free(v);
      }
      {
        size_t uidx = 1;
        g_cdd_last_alloc_err =
            c2s_parse_union_tokens_and_write(utl, &uidx, NULL, "MyUnion");
      }
      g_cdd_last_alloc_err = free_token_list(utl);
    }
  }
  json_set_allocation_functions(malloc, free);

  {
    struct StructFields sf_null_sch;
    g_cdd_last_alloc_err = struct_fields_init(&sf_null_sch);
    g_cdd_last_alloc_err = struct_fields_add(&sf_null_sch, "int_field",
                                             "integer", NULL, NULL, NULL);
    sf_null_sch.fields[0].required = 1;
    g_cdd_last_alloc_err =
        write_struct_to_json_schema(NULL, "TestNullSchemas", &sf_null_sch);
    g_cdd_last_alloc_err = struct_fields_free(&sf_null_sch);
  }

  /* Test c2s_merge_schema_extras_strings when g_cdd_fail_json_set_value is set
   */
  {
    extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
    char *dest = NULL;
    JSON_Value *tv = NULL;
    JSON_Object *to = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&tv);
    g_cdd_last_alloc_err = cdd_json_value_get_object(tv, &to);
    g_cdd_last_alloc_err = c_cdd_strdup("{\"k1\":1}", &dest);
    g_cdd_fail_json_set_value = 1;
    g_cdd_last_alloc_err = c2s_merge_schema_extras_strings(&dest, "{\"k2\":2}");
    g_cdd_fail_json_set_value = 1;
    g_cdd_last_alloc_err = c2s_merge_schema_extras_object(to, "{\"k2\":2}");
    g_cdd_fail_json_set_value = 0;
    g_cdd_last_alloc_err = cdd_json_value_free(tv);
    C_CDD_FREE(dest);
  }

  /* Step-fail and Parson OOM sweeps for code2schema_parser and unions */
  {
    struct TokenList *utl_empty = NULL;
    FILE *ufp;
    json_set_allocation_functions(malloc, free);
    g_cdd_last_alloc_err =
        tokenize(az_span_create_from_str("{ };"), &utl_empty);
    if (utl_empty) {
      for (i = 0; i <= 15; ++i) {
        size_t uidx = 1;
        JSON_Value *v = NULL;
        g_cdd_last_alloc_err = cdd_json_value_init_object(&v);
        json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
        g_parson_fail_at = i;
        g_cdd_last_alloc_err = c2s_parse_union_tokens_and_write(
            utl_empty, &uidx, json_value_get_object(v), "EmptyU");
        g_parson_fail_at = -1;
        json_set_allocation_functions(malloc, free);
        g_cdd_last_alloc_err = cdd_json_value_free(v);
      }
      g_cdd_last_alloc_err = free_token_list(utl_empty);
    }

    ufp = fopen("cdd_test_union_oom.tmp", "w");
    fputs("int a;\n};\n", ufp);
    fclose(ufp);
    for (i = 0; i <= 25; ++i) {
      JSON_Value *v = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&v);
      ufp = fopen("cdd_test_union_oom.tmp", "r");
      json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
      g_parson_fail_at = i;
      g_cdd_last_alloc_err =
          c2s_parse_union_and_write(ufp, json_value_get_object(v), "FileU");
      g_parson_fail_at = -1;
      json_set_allocation_functions(malloc, free);
      fclose(ufp);
      g_cdd_last_alloc_err = cdd_json_value_free(v);
    }
    remove("cdd_test_union_oom.tmp");

    for (i = 1; i <= 10; ++i) {
      struct StructFields sf;
      g_cdd_last_alloc_err = struct_fields_init(&sf);
      g_cdd_step_fail = i;
      g_cdd_last_alloc_err =
          parse_struct_member_line("  int a : 4; // comment", &sf);
      g_cdd_step_fail = 0;
      g_cdd_last_alloc_err = struct_fields_free(&sf);
    }
    {
      struct StructFields sf;
      g_cdd_last_alloc_err = struct_fields_init(&sf);
      g_cdd_step_fail = 1;
      g_cdd_last_alloc_err = c2s_collapse_arrays(&sf);
      g_cdd_step_fail = 2;
      g_cdd_last_alloc_err = c2s_collapse_arrays(&sf);
      g_cdd_step_fail = 0;
      g_cdd_last_alloc_err = struct_fields_free(&sf);
    }

    {
      const char *u_json =
          "{\"RefObj\":{\"type\":\"object\",\"required\":[\"x\"],"
          "\"properties\":{\"x\":{\"type\":\"integer\"}}},"
          "\"RefEnum\":{\"type\":\"string\",\"enum\":[\"A\"]},"
          "\"U\":{\"discriminator\":{\"propertyName\":\"kind\"},"
          "\"oneOf\":["
          "{\"$ref\":\"#/components/schemas/RefObj\"},"
          "{\"title\":\"InlineS\",\"type\":\"string\"},"
          "{\"type\":\"integer\"},"
          "{\"type\":\"array\",\"items\":{\"$ref\":\"#/components/schemas/"
          "RefEnum\"}},"
          "{\"type\":\"array\",\"items\":{\"type\":[\"string\",\"null\"]}},"
          "{\"type\":\"array\",\"items\":{\"properties\":{\"p\":{\"type\":"
          "\"string\"}}}}"
          "]}}";
      JSON_Value *rv = NULL;
      JSON_Object *ro = NULL;
      JSON_Object *uo = NULL;
      JSON_Array *uarr = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(u_json, &rv);
      ro = json_value_get_object(rv);
      g_cdd_last_alloc_err = cdd_json_object_get_object(ro, "U", &uo);
      g_cdd_last_alloc_err = cdd_json_object_get_array(uo, "oneOf", &uarr);

      for (i = 1; i <= 220; ++i) {
        JSON_Value *rv_m = NULL;
        JSON_Object *ro_m = NULL;
        JSON_Object *uo_m = NULL;
        struct StructFields sf;
        g_cdd_last_alloc_err = cdd_json_parse_string(u_json, &rv_m);
        ro_m = json_value_get_object(rv_m);
        g_cdd_last_alloc_err = cdd_json_object_get_object(ro_m, "U", &uo_m);
        g_cdd_last_alloc_err = struct_fields_init(&sf);
        g_cdd_step_fail = i;
        g_cdd_last_alloc_err =
            json_object_to_struct_fields_ex_codegen(uo_m, &sf, ro_m, "U");
        if (g_cdd_step_fail > 0) {
          g_cdd_step_fail = 0;
          g_cdd_last_alloc_err = struct_fields_free(&sf);
          g_cdd_last_alloc_err = cdd_json_value_free(rv_m);
          break;
        }
        g_cdd_step_fail = 0;
        g_cdd_last_alloc_err = struct_fields_free(&sf);
        g_cdd_last_alloc_err = cdd_json_value_free(rv_m);
      }
      for (i = 1; i <= 40; ++i) {
        JSON_Value *rv_m = NULL;
        JSON_Object *ro_m = NULL;
        JSON_Object *uo_m = NULL;
        struct StructFields sf;
        g_cdd_last_alloc_err = cdd_json_parse_string(u_json, &rv_m);
        ro_m = json_value_get_object(rv_m);
        g_cdd_last_alloc_err = cdd_json_object_get_object(ro_m, "U", &uo_m);
        g_cdd_last_alloc_err = struct_fields_init(&sf);
        g_cdd_c2s_fail_step = i;
        g_cdd_last_alloc_err =
            json_object_to_struct_fields_ex_codegen(uo_m, &sf, ro_m, "U");
        g_cdd_c2s_fail_step = 0;
        g_cdd_last_alloc_err = struct_fields_free(&sf);
        g_cdd_last_alloc_err = cdd_json_value_free(rv_m);
      }
      for (i = 1; i <= 15; ++i) {
        struct StructFields sf;
        g_cdd_last_alloc_err = struct_fields_init(&sf);
        g_cdd_step_fail = i;
        g_cdd_last_alloc_err =
            apply_union_to_struct_fields_fallback(uarr, &sf, ro);
        g_cdd_step_fail = 0;
        g_cdd_last_alloc_err = struct_fields_free(&sf);
      }
      for (i = 1; i <= 15; ++i) {
        struct StructFields sf;
        memset(&sf, 0, sizeof(sf));
        g_cdd_alloc_fail = i;
        g_cdd_last_alloc_err =
            apply_union_to_struct_fields_fallback(uarr, &sf, ro);
        g_cdd_alloc_fail = 0;
        g_cdd_last_alloc_err = struct_fields_free(&sf);
      }
      {
        const char *arr_cases[] = {
            "{\"type\":\"array\",\"items\":{\"$ref\":\"#/components/schemas/"
            "RefEnum\"}}",
            "{\"type\":\"array\",\"items\":{\"type\":[\"string\",\"null\"]}}",
            "{\"type\":\"array\",\"items\":{\"properties\":{\"a\":{\"type\":"
            "\"integer\"}}}}"};
        size_t ci;
        for (ci = 0; ci < 3; ++ci) {
          JSON_Value *av = NULL;
          int sup = 0;
          g_cdd_last_alloc_err = cdd_json_parse_string(arr_cases[ci], &av);
          for (i = 1; i <= 15; ++i) {
            g_cdd_step_fail = i;
            g_cdd_last_alloc_err = c2s_union_array_items_supported(
                json_value_get_object(av), ro, 1, &sup);
            g_cdd_step_fail = 0;
            g_cdd_c2s_fail_step = i;
            g_cdd_last_alloc_err = c2s_union_array_items_supported(
                json_value_get_object(av), ro, 1, &sup);
            g_cdd_c2s_fail_step = 0;
          }
          g_cdd_last_alloc_err = cdd_json_value_free(av);
        }
      }
      for (i = 1; i <= 30; ++i) {
        struct StructFields sf;
        g_cdd_last_alloc_err = struct_fields_init(&sf);
        g_cdd_c2s_fail_step = i;
        g_cdd_last_alloc_err =
            apply_union_to_struct_fields_ex(uarr, &sf, ro, "U", 0, uo, 1);
        g_cdd_c2s_fail_step = 0;
        g_cdd_last_alloc_err = struct_fields_free(&sf);

        g_cdd_last_alloc_err = struct_fields_init(&sf);
        g_cdd_c2s_fail_step = i;
        g_cdd_last_alloc_err =
            apply_union_to_struct_fields_fallback(uarr, &sf, ro);
        g_cdd_c2s_fail_step = 0;
        g_cdd_last_alloc_err = struct_fields_free(&sf);
      }
      g_cdd_last_alloc_err = cdd_json_value_free(rv);
    }
  }

  PASS();
}

/**
 * @brief Exercises all remaining error-percolation and edge-case branches
 * across `code2schema*.c`, `json_dom.c`, `json.c`, and `openapi_*.c`.
 * @return Greatest test result.
 */
TEST test_percolation_remaining_100_coverage(void) {
  int step;

  /* 1. json_dom.c NULL and has_value_of_type branches */
  {
    JSON_Value *jv = NULL;
    JSON_Object *jo = NULL;
    JSON_Array *ja = NULL;
    int hv = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_get_object(NULL, &jo));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_get_array(NULL, &ja));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_has_value_of_type(NULL, "k", JSONNumber, &hv));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&jv));
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_value(jo, "n", NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_number(jo, "n", 1.0));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_has_value_of_type(
                                 jo, "missing", JSONNumber, &hv));
    ASSERT_EQ(0, hv);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_has_value_of_type(jo, "n", JSONNumber, &hv));
    ASSERT_EQ(1, hv);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_has_value_of_type(jo, "n", JSONString, &hv));
    ASSERT_EQ(0, hv);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(jv));
  }

  /* 2. classes/emit/json.c step-fail on write_struct_from_jsonObject_func */
  {
    struct StructFields jsf;
    FILE *jfp = tmpfile();
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&jsf));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&jsf, "val", "string", NULL, NULL, NULL));
    for (step = 1; step <= 25; ++step) {
      g_cdd_step_fail = step;
      g_cdd_last_alloc_err =
          write_struct_from_jsonObject_func(jfp, "StepJsonS", &jsf, NULL);
      if (g_cdd_step_fail > 0) {
        g_cdd_step_fail = 0;
        break;
      }
      g_cdd_step_fail = 0;
    }
    if (jfp)
      fclose(jfp);
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_free(&jsf));
  }

  /* 3. openapi_*.c step-fail sweep on OpenAPI 3.2 spec */
  {
    JSON_Value *oa_val = NULL;
    JSON_Object *oa = NULL;
    JSON_Value *info_v = NULL;
    JSON_Value *comp_v = NULL;
    JSON_Object *comp = NULL;
    JSON_Value *cparams_v = NULL;
    JSON_Value *bp_v = NULL;
    JSON_Value *bps_v = NULL;
    JSON_Value *chdrs_v = NULL;
    JSON_Value *h1_v = NULL;
    JSON_Value *h1s_v = NULL;
    JSON_Value *h1t_v = NULL;
    JSON_Value *paths_v = NULL;
    JSON_Object *paths = NULL;
    JSON_Value *p1_v = NULL;
    JSON_Object *p1 = NULL;
    JSON_Value *p1_pars_v = NULL;
    JSON_Value *pp_sub_v = NULL;
    JSON_Value *pp_subs_v = NULL;
    JSON_Value *get1_v = NULL;
    JSON_Object *get1 = NULL;
    JSON_Value *op_pars_v = NULL;
    JSON_Value *op_id_v = NULL;
    JSON_Value *op_ids_v = NULL;
    JSON_Value *op_ref_v = NULL;
    JSON_Value *op_c1_v = NULL;
    JSON_Value *c1_cnt_v = NULL;
    JSON_Value *c1_mt_v = NULL;
    JSON_Value *op_c2_v = NULL;
    JSON_Value *c2_cnt_v = NULL;
    JSON_Value *c2_mt_v = NULL;
    JSON_Value *op_c3_v = NULL;
    JSON_Value *c3_cnt_v = NULL;
    JSON_Value *c3_mt_v = NULL;
    JSON_Value *resp1_v = NULL;
    JSON_Value *r200_1_v = NULL;
    JSON_Value *p2_v = NULL;
    JSON_Object *p2 = NULL;
    JSON_Value *p2_pars_v = NULL;
    JSON_Value *pp_oid_v = NULL;
    JSON_Value *pp_oids_v = NULL;
    JSON_Value *get2_v = NULL;
    JSON_Value *resp2_v = NULL;
    JSON_Value *r200_2_v = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&oa_val));
    oa = json_value_get_object(oa_val);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(oa, "openapi", "3.2.0"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&info_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(
                                 json_value_get_object(info_v), "title", "T"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(info_v),
                                         "version", "1.0"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_value(oa, "info", info_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&comp_v));
    comp = json_value_get_object(comp_v);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&cparams_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&bp_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(
                                 json_value_get_object(bp_v), "name", "bp"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(
                                 json_value_get_object(bp_v), "in", "query"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&bps_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(bps_v), "type",
                                         "string"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_value(
                                 json_value_get_object(bp_v), "schema", bps_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(cparams_v),
                                        "BaseP", bp_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(comp, "parameters", cparams_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&chdrs_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&h1_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(h1_v), "style",
                                         "simple"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(h1_v),
                                         "deprecated", "no"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&h1s_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&h1t_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_string(
                                 json_value_get_array(h1t_v), "string"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_string(
                                 json_value_get_array(h1t_v), "null"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_value(
                                 json_value_get_object(h1s_v), "type", h1t_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_value(
                                 json_value_get_object(h1_v), "schema", h1s_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_value(
                                 json_value_get_object(chdrs_v), "H1", h1_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(comp, "headers", chdrs_v));
    {
      JSON_Value *mts_v = NULL;
      JSON_Value *xml_mt_v = NULL;
      JSON_Value *xml_s_v = NULL;
      JSON_Value *csch_v = NULL;
      JSON_Value *co_v = NULL;
      JSON_Value *co_arr_v = NULL;
      JSON_Value *co_item_v = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&mts_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&xml_mt_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&xml_s_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_string(json_value_get_object(xml_s_v),
                                           "type", "string"));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(json_value_get_object(xml_mt_v),
                                          "schema", xml_s_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(json_value_get_object(mts_v), "Xml",
                                          xml_mt_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(comp, "mediaTypes", mts_v));

      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&csch_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&co_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&co_arr_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&co_item_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_string(json_value_get_object(co_item_v),
                                           "type", "string"));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_value(
                                   json_value_get_array(co_arr_v), co_item_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(json_value_get_object(co_v), "oneOf",
                                          co_arr_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(json_value_get_object(csch_v),
                                          "CompOneOf", co_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(comp, "schemas", csch_v));
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(oa, "components", comp_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&paths_v));
    paths = json_value_get_object(paths_v);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&p1_v));
    p1 = json_value_get_object(p1_v);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&p1_pars_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&pp_sub_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(pp_sub_v),
                                         "name", "sub"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(pp_sub_v), "in",
                                         "path"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_boolean(json_value_get_object(pp_sub_v),
                                          "required", 1));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(pp_sub_v),
                                         "style", "simple"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&pp_subs_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(pp_subs_v),
                                         "type", "string"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(pp_sub_v),
                                        "schema", pp_subs_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_value(
                                 json_value_get_array(p1_pars_v), pp_sub_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(p1, "parameters", p1_pars_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&get1_v));
    get1 = json_value_get_object(get1_v);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&op_pars_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&op_id_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(
                                 json_value_get_object(op_id_v), "name", "id"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(
                                 json_value_get_object(op_id_v), "in", "path"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_boolean(json_value_get_object(op_id_v),
                                          "required", 1));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&op_ids_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(op_ids_v),
                                         "type", "string"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(op_id_v),
                                        "schema", op_ids_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_value(
                                 json_value_get_array(op_pars_v), op_id_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&op_ref_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(
                                 json_value_get_object(op_ref_v), "$ref",
                                 "#/components/parameters/BaseP"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(op_ref_v),
                                         "description", "ovr"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_value(
                                 json_value_get_array(op_pars_v), op_ref_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&op_c1_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(
                                 json_value_get_object(op_c1_v), "name", "c1"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(op_c1_v), "in",
                                         "query"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&c1_cnt_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&c1_mt_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_boolean(
                                 json_value_get_object(c1_mt_v), "schema", 1));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(c1_cnt_v),
                                        "text/plain", c1_mt_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(op_c1_v),
                                        "content", c1_cnt_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_value(
                                 json_value_get_array(op_pars_v), op_c1_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&op_c2_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(
                                 json_value_get_object(op_c2_v), "name", "c2"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(op_c2_v), "in",
                                         "query"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&c2_cnt_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&c2_mt_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(c2_mt_v), "$ref",
                                         "#/components/mediaTypes/Xml"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(c2_cnt_v),
                                        "application/xml", c2_mt_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(op_c2_v),
                                        "content", c2_cnt_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_value(
                                 json_value_get_array(op_pars_v), op_c2_v));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(get1, "parameters", op_pars_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&resp1_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&r200_1_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(r200_1_v),
                                         "description", "ok"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(resp1_v), "200",
                                        r200_1_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(get1, "responses", resp1_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_value(p1, "get", get1_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(paths, "/items/{id}/{sub}", p1_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&p2_v));
    p2 = json_value_get_object(p2_v);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&p2_pars_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&pp_oid_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(pp_oid_v),
                                         "name", "oid"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(pp_oid_v), "in",
                                         "path"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_boolean(json_value_get_object(pp_oid_v),
                                          "required", 1));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&pp_oids_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(pp_oids_v),
                                         "type", "string"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(pp_oid_v),
                                        "schema", pp_oids_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_value(
                                 json_value_get_array(p2_pars_v), pp_oid_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(p2, "parameters", p2_pars_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&get2_v));
    {
      JSON_Value *op2_pars_v = NULL;
      JSON_Value *enc_map_v = NULL;
      JSON_Value *enc_f_v = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&op2_pars_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&op_c3_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_string(json_value_get_object(op_c3_v),
                                           "name", "c3"));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_string(json_value_get_object(op_c3_v), "in",
                                           "querystring"));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&c3_cnt_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&c3_mt_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&enc_map_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&enc_f_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_string(json_value_get_object(enc_f_v),
                                           "style", "form"));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(json_value_get_object(enc_map_v), "f",
                                          enc_f_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(json_value_get_object(c3_mt_v),
                                          "encoding", enc_map_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(json_value_get_object(c3_cnt_v),
                                          "application/x-www-form-urlencoded",
                                          c3_mt_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(json_value_get_object(op_c3_v),
                                          "content", c3_cnt_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_value(
                                   json_value_get_array(op2_pars_v), op_c3_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(json_value_get_object(get2_v),
                                          "parameters", op2_pars_v));
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&resp2_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&r200_2_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(r200_2_v),
                                         "description", "ok"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(resp2_v), "200",
                                        r200_2_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(get2_v),
                                        "responses", resp2_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_value(p2, "get", get2_v));
    {
      JSON_Value *add_ops_v = NULL;
      JSON_Value *purge_v = NULL;
      JSON_Value *presp_v = NULL;
      JSON_Value *pr200_v = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&add_ops_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&purge_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&presp_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&pr200_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_string(json_value_get_object(pr200_v),
                                           "description", "ok"));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(json_value_get_object(presp_v), "200",
                                          pr200_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(json_value_get_object(purge_v),
                                          "responses", presp_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(json_value_get_object(add_ops_v),
                                          "PURGE", purge_v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_value(
                                   p2, "additionalOperations", add_ops_v));
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(paths, "/other/{oid}", p2_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_value(oa, "paths", paths_v));

    {
      struct OpenAPI_Spec base_spec;
      memset(&base_spec, 0, sizeof(base_spec));
      ASSERT_EQ(CDD_C_SUCCESS, openapi_load_from_json(oa_val, &base_spec));
      ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_free(&base_spec));
    }
    for (step = 1; step <= 5000; ++step) {
      struct OpenAPI_Spec oa_spec;
      memset(&oa_spec, 0, sizeof(oa_spec));
      g_cdd_step_fail = step;
      g_cdd_last_alloc_err = openapi_load_from_json(oa_val, &oa_spec);
      if (g_cdd_step_fail > 0) {
        g_cdd_step_fail = 0;
        g_cdd_last_alloc_err = openapi_spec_free(&oa_spec);
        break;
      }
      g_cdd_step_fail = 0;
      g_cdd_last_alloc_err = openapi_spec_free(&oa_spec);
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(oa_val));
  }

  /* 4. code2schema.c step-fail & parson OOM & CLI error sweep */
  {
    FILE *fh = fopen("cdd_test_step_c2s.h", "w");
    char *av_ok[2];
    char *av_bad_cli[2];
    ASSERT(fh != NULL);
    fprintf(fh, "/** @serde(rename_all = %ccamelCase%c) */%c", 34, 34, 10);
    fprintf(fh, "struct S1 {%c", 10);
    fprintf(fh, "  /** @brief nested */%c", 10);
    fprintf(fh, "  struct { ; /** @brief nf */ int nx; } sub;%c", 10);
    fprintf(fh, "  ; /** @brief f1 */ int f1; /* inline */%c", 10);
    fprintf(fh, "};%c", 10);
    fprintf(fh, "struct { int anon_s; } gs;%c", 10);
    fprintf(fh, "union U1 { int ui; char *us; };%c", 10);
    fprintf(fh, "union { int anon_u; } gu;%c", 10);
    fprintf(fh, "/** @serde(rename_all = %csnake_case%c) */%c", 34, 34, 10);
    fprintf(fh, "enum E1 {%c", 10);
    fprintf(fh,
            "  /** @serde(case = %cUPPERCASE%c, replace(%cA%c, %cB%c)) */%c",
            34, 34, 34, 34, 34, 34, 10);
    fprintf(fh, "  E1_A = 1, /* inline */%c", 10);
    fprintf(fh, "  E1_B%c", 10);
    fprintf(fh, "};%c", 10);
    fprintf(fh, "enum { ANON_E } ge;%c", 10);
    fclose(fh);

    av_bad_cli[0] = C_CDD_STR_LIT("--unknown-opt");
    av_bad_cli[1] = C_CDD_STR_LIT("val");
    ASSERT_NEQ(CDD_C_SUCCESS, code2schema_main(2, av_bad_cli));

    av_ok[0] = C_CDD_STR_LIT("cdd_test_step_c2s.h");
    av_ok[1] = C_CDD_STR_LIT("cdd_test_step_c2s.json");
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, av_ok));
    for (step = 1; step <= 5000; ++step) {
      g_cdd_step_fail = step;
      g_cdd_last_alloc_err = code2schema_main(2, av_ok);
      if (g_cdd_step_fail > 0) {
        g_cdd_step_fail = 0;
        break;
      }
      g_cdd_step_fail = 0;
    }

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    for (step = 0; step <= 120; ++step) {
      g_parson_fail_at = step;
      g_cdd_last_alloc_err = code2schema_main(2, av_ok);
      if (g_parson_fail_at >= 0) {
        g_parson_fail_at = -1;
        break;
      }
      g_parson_fail_at = -1;
    }
    json_set_allocation_functions(malloc, free);

    remove("cdd_test_step_c2s.h");
    remove("cdd_test_step_c2s.json");
  }

  /* 5. code2schema_mapper.c & code2schema_enum.c & code2schema_defaults.c */
  {
    JSON_Value *root_v = NULL;
    JSON_Object *root_o = NULL;
    JSON_Value *empty_flat_v = NULL;
    JSON_Value *ref_enum_v = NULL;
    JSON_Value *re_arr_v = NULL;
    JSON_Value *sch_v = NULL;
    JSON_Object *sch = NULL;
    JSON_Value *props_v = NULL;
    JSON_Object *props = NULL;
    JSON_Value *p_flat_v = NULL;
    JSON_Value *p_arr_v = NULL;
    JSON_Value *p_arr_items_v = NULL;
    JSON_Value *p_arr_it_u_v = NULL;
    JSON_Value *p_earr_v = NULL;
    JSON_Value *p_earr_items_v = NULL;
    JSON_Value *p_misc_v = NULL;
    JSON_Object *p_misc = NULL;
    JSON_Value *x_obj_v = NULL;
    JSON_Value *x_arr_v = NULL;
    JSON_Value *enum_sch_v = NULL;
    JSON_Object *enum_sch = NULL;
    JSON_Value *ev_arr_v = NULL;
    JSON_Value *ec_arr_v = NULL;
    JSON_Value *en_arr_v = NULL;
    JSON_Value *ea_obj_v = NULL;
    JSON_Value *ea_list_v = NULL;
    JSON_Value *ed_obj_v = NULL;
    JSON_Value *eh_arr_v = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&root_v));
    root_o = json_value_get_object(root_v);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&empty_flat_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(empty_flat_v),
                                         "type", "object"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(root_o, "EmptyFlat", empty_flat_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&ref_enum_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(ref_enum_v),
                                         "type", "string"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&re_arr_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_string(
                                 json_value_get_array(re_arr_v), "V1"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(ref_enum_v),
                                        "enum", re_arr_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(root_o, "RefE", ref_enum_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&sch_v));
    sch = json_value_get_object(sch_v);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(sch, "type", "object"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(sch, "x-cdd-rename-all", "camelCase"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(sch, "x-cdd-rename-all-serialize",
                                         "camelCase"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(sch, "x-cdd-rename-all-deserialize",
                                         "camelCase"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(
                                 sch, "x-cdd-cli-rename-all", "kebab-case"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(
                                 sch, "x-cdd-cli-rename-all-env", "UPPERCASE"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_boolean(sch, "x-cdd-unknown-bool", 1));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&props_v));
    props = json_value_get_object(props_v);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&p_flat_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(
                                 json_value_get_object(p_flat_v), "$ref",
                                 "#/components/schemas/EmptyFlat"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_boolean(json_value_get_object(p_flat_v),
                                          "x-cdd-flatten", 1));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(props, "flat_empty", p_flat_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&p_arr_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(p_arr_v), "type",
                                         "array"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&p_arr_items_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&p_arr_it_u_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_string(
                                 json_value_get_array(p_arr_it_u_v), "string"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_string(
                                 json_value_get_array(p_arr_it_u_v), "null"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(p_arr_items_v),
                                        "type", p_arr_it_u_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(p_arr_v), "items",
                                        p_arr_items_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_number(json_value_get_object(p_arr_v),
                                         "minItems", 1.0));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_number(json_value_get_object(p_arr_v),
                                         "maxItems", 5.0));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_boolean(json_value_get_object(p_arr_v),
                                          "uniqueItems", 1));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(props, "arr_u", p_arr_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&p_earr_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(p_earr_v),
                                         "type", "array"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&p_earr_items_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(p_earr_items_v),
                                         "$ref", "#/components/schemas/RefE"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(p_earr_v),
                                        "items", p_earr_items_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(props, "earr", p_earr_v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&p_misc_v));
    p_misc = json_value_get_object(p_misc_v);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(p_misc, "type", "string"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_null(p_misc, "default"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_number(p_misc, "minLength", 1.0));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_number(p_misc, "maxLength", 9.0));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(p_misc, "pattern", "^a"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(p_misc, "x-cdd-case", "lowercase"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(p_misc, "x-cdd-cli-action", "set"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(
                                 p_misc, "x-cdd-cli-value-hint", "bad_hint"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&x_obj_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(p_misc, "x-extra-obj", x_obj_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&x_arr_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(p_misc, "x-extra-arr", x_arr_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(props, "foo-bar", p_misc_v));

    /* Also numeric field with exclusiveMinimum/Maximum bools */
    {
      JSON_Value *p_num_v = NULL;
      JSON_Object *p_num = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&p_num_v));
      p_num = json_value_get_object(p_num_v);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_string(p_num, "type", "number"));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_number(p_num, "minimum", 0.0));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_boolean(p_num, "exclusiveMinimum", 1));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_number(p_num, "maximum", 10.0));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_boolean(p_num, "exclusiveMaximum", 1));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_string(
                                   p_num, "x-cdd-cli-value-hint", "file"));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_value(props, "num_f", p_num_v));
    }

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(sch, "properties", props_v));

    for (step = 1; step <= 350; ++step) {
      struct StructFields sf_t;
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_t));
      g_cdd_step_fail = step;
      g_cdd_last_alloc_err =
          json_object_to_struct_fields_ex(sch, &sf_t, root_o, "bad-schema");
      if (g_cdd_step_fail > 0) {
        g_cdd_step_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_free(&sf_t));
        break;
      }
      g_cdd_step_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_free(&sf_t));
    }
    for (step = 1; step <= 40; ++step) {
      struct StructFields sf_t;
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_t));
      g_cdd_c2s_fail_step = step;
      g_cdd_last_alloc_err =
          json_object_to_struct_fields_ex(sch, &sf_t, root_o, "bad-schema");
      if (g_cdd_c2s_fail_step > 0) {
        g_cdd_c2s_fail_step = 0;
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_free(&sf_t));
        break;
      }
      g_cdd_c2s_fail_step = 0;
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_free(&sf_t));
    }

    /* Enum schema step-fail sweep */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&enum_sch_v));
    enum_sch = json_value_get_object(enum_sch_v);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(enum_sch, "type", "string"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&ev_arr_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_string(
                                 json_value_get_array(ev_arr_v), "a"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(enum_sch, "enum", ev_arr_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&ec_arr_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_string(
                                 json_value_get_array(ec_arr_v), "E_A"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_value(
                                 enum_sch, "x-cdd-enum-c-names", ec_arr_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&en_arr_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_string(
                                 json_value_get_array(en_arr_v), "a"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_value(
                                 enum_sch, "x-cdd-enum-names", en_arr_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&ea_obj_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&ea_list_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_string(
                                 json_value_get_array(ea_list_v), "al"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(json_value_get_object(ea_obj_v), "E_A",
                                        ea_list_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_value(
                                 enum_sch, "x-cdd-enum-aliases", ea_obj_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&ed_obj_v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(json_value_get_object(ed_obj_v), "E_A",
                                         "desc"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_value(enum_sch, "x-cdd-enum-cli-descriptions",
                                        ed_obj_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&eh_arr_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_string(
                                 json_value_get_array(eh_arr_v), "E_A"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_value(
                                 enum_sch, "x-cdd-enum-cli-hidden", eh_arr_v));

    for (step = 1; step <= 60; ++step) {
      struct StructFields esf;
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&esf));
      g_cdd_step_fail = step;
      g_cdd_last_alloc_err =
          json_object_to_struct_fields_ex(enum_sch, &esf, root_o, "EnumS");
      if (g_cdd_step_fail > 0) {
        g_cdd_step_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_free(&esf));
        break;
      }
      g_cdd_step_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_free(&esf));
    }

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(enum_sch_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(sch_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(root_v));
  }

  /* 6. code2schema_writer.c & code2schema_defaults.c OOM + step-fail sweeps */
  {
    struct StructFields w_sf;
    struct StructFields e_sf;
    struct StructField *f0;
    struct StructField *f1;
    struct StructField *f2;
    char *tu[2];
    tu[0] = C_CDD_STR_LIT("string");
    tu[1] = C_CDD_STR_LIT("null");

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&w_sf));
    CDD_STRCPY(w_sf.serde_rename, sizeof(w_sf.serde_rename), "WireStruct");
    w_sf.serde_rename_all = CDD_SERDE_CASE_CAMELCASE;
    CDD_STRCPY(w_sf.serde_replace_from, sizeof(w_sf.serde_replace_from), "x");
    CDD_STRCPY(w_sf.serde_replace_to, sizeof(w_sf.serde_replace_to), "y");
    CDD_STRCPY(w_sf.serde_prefix, sizeof(w_sf.serde_prefix), "p_");
    w_sf.is_cli_command = 1;
    ASSERT_EQ(CDD_C_SUCCESS, c_cdd_strdup("Long about", &w_sf.cli_long_about));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add_cli_alias(&w_sf, "ws"));

    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&w_sf, "f_str", "string", NULL, "abc", "8"));
    f0 = &w_sf.fields[0];
    f0->required = 1;
    f0->fixed_array_len = 16;
    f0->is_nullable_ptr = 1;
    f0->serde_case = CDD_SERDE_CASE_SNAKECASE;
    CDD_STRCPY(f0->serde_replace_from, sizeof(f0->serde_replace_from), "a");
    CDD_STRCPY(f0->serde_replace_to, sizeof(f0->serde_replace_to), "b");
    ASSERT_EQ(CDD_C_SUCCESS, struct_field_add_alias(f0, "a1"));
    f0->serde_skip_serializing = 1;
    f0->serde_skip_deserializing = 1;
    f0->serde_skip_if_kind = CDD_SERDE_SKIP_IF_NULL;
    f0->cli_short = 's';
    f0->cli_index = 0;
    f0->cli_action = CDD_CLI_ACTION_SET;
    f0->cli_value_delimiter = ',';
    ASSERT_EQ(CDD_C_SUCCESS, c_cdd_strdup("Long help", &f0->cli_long_help));
    f0->cli_value_hint = CDD_CLI_HINT_FILE_PATH;
    f0->cli_num_args_min = 1;
    f0->cli_num_args_max = 2;
    CDD_STRCPY(f0->cli_required_if_field, sizeof(f0->cli_required_if_field),
               "other");
    CDD_STRCPY(f0->cli_required_if_val, sizeof(f0->cli_required_if_val), "v");
    ASSERT_EQ(CDD_C_SUCCESS, struct_field_add_cli_conflict(f0, "c1"));
    ASSERT_EQ(CDD_C_SUCCESS, struct_field_add_cli_requires(f0, "r1"));
    ASSERT_EQ(CDD_C_SUCCESS, struct_field_add_cli_required_unless(f0, "u1"));
    f0->has_min_len = 1;
    f0->min_len = 1;
    f0->has_max_len = 1;
    f0->max_len = 10;
    CDD_STRCPY(f0->pattern, sizeof(f0->pattern), "^a");
    CDD_STRCPY(f0->description, sizeof(f0->description), "desc");
    CDD_STRCPY(f0->format, sizeof(f0->format), "uuid");
    f0->deprecated_set = 1;
    f0->deprecated = 1;
    f0->read_only_set = 1;
    f0->read_only = 1;
    f0->write_only_set = 1;
    f0->write_only = 1;

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&w_sf, "f_arr", "array",
                                               "integer", NULL, NULL));
    f1 = &w_sf.fields[1];
    f1->required = 1;
    f1->has_min_items = 1;
    f1->min_items = 1;
    f1->has_max_items = 1;
    f1->max_items = 5;
    f1->unique_items = 1;
    f1->serde_skip = 1;

    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&w_sf, "f_num", "integer", NULL, "42", NULL));
    f2 = &w_sf.fields[2];
    f2->required = 1;
    f2->has_min = 1;
    f2->min_val = 0.0;
    f2->has_max = 1;
    f2->max_val = 99.0;

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&w_sf, "f_bool", "boolean", NULL,
                                               "true", NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&w_sf, "f_obj", "object",
                                               "SubRef", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&w_sf, "f_obj_inl", "object",
                                               NULL, NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&w_sf, "f_earr", "array",
                                               "string", NULL, NULL));
    w_sf.fields[6].is_enum_array = 1;
    CDD_STRCPY(w_sf.fields[6].enum_array_ref,
               sizeof(w_sf.fields[6].enum_array_ref), "MyEnum");
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&w_sf, "f_tu_arr", "array",
                                               "CustomItem", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&w_sf, "f_itu_arr", "array",
                                               "string", NULL, NULL));
    w_sf.fields[8].items_type_union = tu;
    w_sf.fields[8].n_items_type_union = 2;

    for (step = 1; step <= 80; ++step) {
      JSON_Value *sv = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&sv));
      g_cdd_step_fail = step;
      g_cdd_last_alloc_err = write_struct_to_json_schema(
          json_value_get_object(sv), "OrigS", &w_sf);
      if (g_cdd_step_fail > 0) {
        g_cdd_step_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(sv));
        break;
      }
      g_cdd_step_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(sv));
    }
    for (step = 1; step <= 30; ++step) {
      JSON_Value *sv = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&sv));
      g_cdd_c2s_fail_step = step;
      g_cdd_last_alloc_err = write_struct_to_json_schema(
          json_value_get_object(sv), "OrigS", &w_sf);
      if (g_cdd_c2s_fail_step > 0) {
        g_cdd_c2s_fail_step = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(sv));
        break;
      }
      g_cdd_c2s_fail_step = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(sv));
    }

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    for (step = 0; step <= 420; ++step) {
      JSON_Value *sv = NULL;
      g_parson_fail_at = -1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&sv));
      g_parson_fail_at = step;
      g_cdd_last_alloc_err = write_struct_to_json_schema(
          json_value_get_object(sv), "OrigS", &w_sf);
      if (g_parson_fail_at >= 0) {
        g_parson_fail_at = -1;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(sv));
        break;
      }
      g_parson_fail_at = -1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(sv));
    }
    json_set_allocation_functions(malloc, free);

    w_sf.fields[8].items_type_union = NULL;
    w_sf.fields[8].n_items_type_union = 0;
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_free(&w_sf));

    /* Rich enum write_struct_to_json_schema OOM + step-fail sweeps */
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&e_sf));
    e_sf.is_enum = 1;
    ASSERT_EQ(CDD_C_SUCCESS, enum_members_init(&e_sf.enum_members));
    e_sf.enum_members.serde_rename_all = CDD_SERDE_CASE_LOWERCASE;
    CDD_STRCPY(e_sf.enum_members.serde_replace_from,
               sizeof(e_sf.enum_members.serde_replace_from), "a");
    CDD_STRCPY(e_sf.enum_members.serde_replace_to,
               sizeof(e_sf.enum_members.serde_replace_to), "b");
    ASSERT_EQ(CDD_C_SUCCESS,
              enum_members_add_ex(&e_sf.enum_members, "VAL_ONE", "v_one", 0));
    e_sf.enum_members.serde_other_idx = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              enum_member_add_alias(&e_sf.enum_members, 0, "alias1"));
    ASSERT_EQ(CDD_C_SUCCESS,
              enum_member_set_cli_meta(&e_sf.enum_members, 0, "help1", 1));

    for (step = 1; step <= 30; ++step) {
      JSON_Value *sv = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&sv));
      g_cdd_step_fail = step;
      g_cdd_last_alloc_err = write_struct_to_json_schema(
          json_value_get_object(sv), "EnumS", &e_sf);
      if (g_cdd_step_fail > 0) {
        g_cdd_step_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(sv));
        break;
      }
      g_cdd_step_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(sv));
    }

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    for (step = 0; step <= 80; ++step) {
      JSON_Value *sv = NULL;
      g_parson_fail_at = -1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&sv));
      g_parson_fail_at = step;
      g_cdd_last_alloc_err = write_struct_to_json_schema(
          json_value_get_object(sv), "EnumS", &e_sf);
      if (g_parson_fail_at >= 0) {
        g_parson_fail_at = -1;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(sv));
        break;
      }
      g_parson_fail_at = -1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(sv));
    }
    json_set_allocation_functions(malloc, free);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_free(&e_sf));
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
  RUN_TEST(test_percolation_remaining_100_coverage);
}

#ifdef __cplusplus
}
#endif
#endif
