/**
 * @file test_openapi_schemas_coverage.h
 * @brief Unit tests for OpenAPI schema types, constraints, enums, and
 * composition (openapi_schemas.c).
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_SCHEMAS_COVERAGE_H
#define TEST_OPENAPI_SCHEMAS_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Unit test for parse_schema_type and parse_schema_constraints.
 * @return Greatest test result.
 */
TEST test_openapi_schemas_type_and_constraints(void) {
  int nullable = 0;
  const char *type_val = NULL;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  struct SchemaConstraintTarget target;
  struct OpenAPI_Any example_val;
  int example_set = 0;
  int has_min = 0, exclusive_min = 0;
  double min_val = 0.0;
  int has_max = 0, exclusive_max = 0;
  double max_val = 0.0;
  int has_min_len = 0;
  size_t min_len = 0;
  int has_max_len = 0;
  size_t max_len = 0;
  char *pattern = NULL;
  int has_min_items = 0;
  size_t min_items = 0;
  int has_max_items = 0;
  size_t max_items = 0;
  int unique_items = 0;

  /* 1. parse_schema_type */
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_type(NULL, &nullable, &type_val));
  ASSERT_EQ(NULL, type_val);
  ASSERT_EQ(0, nullable);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_type(NULL, NULL, &type_val));
  ASSERT_EQ(NULL, type_val);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"type\": \"integer\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_type(jo, &nullable, &type_val));
  ASSERT_STR_EQ("integer", type_val);
  ASSERT_EQ(0, nullable);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("{\"type\": 123}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_type(jo, &nullable, &type_val));
  ASSERT_EQ(NULL, type_val);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"type\": [\"string\", \"null\"]}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_type(jo, &nullable, &type_val));
  ASSERT_STR_EQ("string", type_val);
  ASSERT_EQ(1, nullable);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"type\": [\"null\"]}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_type(jo, &nullable, &type_val));
  ASSERT_STR_EQ("null", type_val);
  ASSERT_EQ(1, nullable);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_type(jo, NULL, &type_val));
  ASSERT_EQ(NULL, type_val);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("{\"type\": []}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_type(jo, &nullable, &type_val));
  ASSERT_EQ(NULL, type_val);
  ASSERT_EQ(0, nullable);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"type\": [123, \"boolean\"]}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_type(jo, NULL, &type_val));
  ASSERT_STR_EQ("boolean", type_val);
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 2. parse_schema_constraints */
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_constraints(NULL, &target));
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_constraints(NULL, NULL));

  memset(&target, 0, sizeof(target));
  memset(&example_val, 0, sizeof(example_val));
  target.example = &example_val;
  target.example_set = &example_set;
  target.has_min = &has_min;
  target.min_val = &min_val;
  target.exclusive_min = &exclusive_min;
  target.has_max = &has_max;
  target.max_val = &max_val;
  target.exclusive_max = &exclusive_max;
  target.has_min_len = &has_min_len;
  target.min_len = &min_len;
  target.has_max_len = &has_max_len;
  target.max_len = &max_len;
  target.pattern = &pattern;
  target.has_min_items = &has_min_items;
  target.min_items = &min_items;
  target.has_max_items = &has_max_items;
  target.max_items = &max_items;
  target.unique_items = &unique_items;

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"example\": \"ex\", \"minimum\": 1.5, \"exclusiveMinimum\": true, "
        "\"maximum\": 10.0, \"exclusiveMaximum\": false, \"minLength\": 2, "
        "\"maxLength\": 20, \"pattern\": \"^[a-z]+$\", \"minItems\": 1, "
        "\"maxItems\": 5, \"uniqueItems\": true}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_constraints(jo, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_constraints(jo, &target));
  ASSERT_EQ(1, *target.has_min);
  ASSERT_EQ(1, *target.exclusive_min);
  ASSERT_EQ(1, *target.has_max);
  ASSERT_EQ(0, *target.exclusive_max);
  ASSERT_EQ(1, *target.has_min_len);
  ASSERT_EQ(1, *target.has_max_len);
  ASSERT(pattern != NULL);
  free(pattern);
  pattern = NULL;
  if (example_set) {
    g_cdd_last_alloc_err = free_any_value(&example_val);
    example_set = 0;
  }

  target.example = &example_val;
  target.example_set = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_constraints(jo, &target));
  free(pattern);
  pattern = NULL;
  target.example = NULL;
  target.example_set = &example_set;
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_constraints(jo, &target));
  free(pattern);
  pattern = NULL;
  target.example = &example_val;

  g_cdd_last_alloc_err = json_value_free(jv);

  /* Test exclusiveMinimum / exclusiveMaximum as numeric (OpenAPI 3.1) */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"exclusiveMinimum\": 0.5, \"exclusiveMaximum\": 99.5}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_constraints(jo, &target));
  ASSERT_EQ(1, *target.exclusive_min);
  ASSERT_EQ(1, *target.exclusive_max);
  g_cdd_last_alloc_err = json_value_free(jv);

  /* Test exclusiveMinimum / exclusiveMaximum false */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"minimum\": 1.0, \"exclusiveMinimum\": false, "
                              "\"maximum\": 10.0, \"exclusiveMaximum\": false}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  target.exclusive_min = &exclusive_min;
  target.exclusive_max = &exclusive_max;
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_constraints(jo, &target));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* Test exclusiveMinimum / exclusiveMaximum with target exclusive_min/max NULL
   */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"minimum\": 1.0, \"exclusiveMinimum\": 0.5, "
                              "\"maximum\": 10.0, \"exclusiveMaximum\": 9.5}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  target.exclusive_min = NULL;
  target.exclusive_max = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_constraints(jo, &target));
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"minimum\": 1.0, \"exclusiveMinimum\": true, "
                              "\"maximum\": 10.0, \"exclusiveMaximum\": true}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  target.exclusive_min = NULL;
  target.exclusive_max = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_constraints(jo, &target));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* Partial targets (one pointer non-null, one null) */
  memset(&target, 0, sizeof(target));
  target.has_min = &has_min;
  target.has_max = &has_max;
  target.has_min_len = &has_min_len;
  target.has_max_len = &has_max_len;
  target.has_min_items = &has_min_items;
  target.has_max_items = &has_max_items;
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"minimum\": 1, \"maximum\": 2, \"minLength\": 1, "
        "\"maxLength\": 5, \"minItems\": 1, \"maxItems\": 2}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_constraints(jo, &target));
  g_cdd_last_alloc_err = json_value_free(jv);

  memset(&target, 0, sizeof(target));
  target.min_val = &min_val;
  target.max_val = &max_val;
  target.min_len = &min_len;
  target.max_len = &max_len;
  target.min_items = &min_items;
  target.max_items = &max_items;
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"minimum\": 1, \"maximum\": 2, \"minLength\": 1, "
        "\"maxLength\": 5, \"minItems\": 1, \"maxItems\": 2}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_constraints(jo, &target));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* example parse failure */
  memset(&target, 0, sizeof(target));
  target.example = &example_val;
  target.example_set = &example_set;
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"example\": \"ex\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_schema_constraints(jo, &target));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* pattern OOM */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"pattern\": \"test\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  target.pattern = &pattern;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_schema_constraints(jo, &target));
  g_cdd_strdup_fail = 0;
  target.pattern = NULL;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* Target with null internal pointers */
  memset(&target, 0, sizeof(target));
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"minimum\": 1, \"maximum\": 2, \"minLength\": 1, "
        "\"maxLength\": 5, \"pattern\": \"abc\", \"minItems\": "
        "1, \"maxItems\": 2, \"uniqueItems\": true}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_constraints(jo, &target));
  g_cdd_last_alloc_err = json_value_free(jv);

  PASS();
}

/**
 * @brief Unit test for string enum arrays, schema compatibility, and schema
 * refs.
 * @return Greatest test result.
 */
TEST test_openapi_schemas_enums_and_refs(void) {
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  JSON_Array *ja = NULL;
  char **enum_out = NULL;
  size_t enum_count = 0;
  char **copied = NULL;
  size_t copied_count = 0;
  char *sample_src[3];
  struct OpenAPI_SchemaRef *schema_refs = NULL;
  size_t schema_refs_count = 0;
  struct OpenAPI_SchemaRef *single_ref = NULL;
  struct OpenAPI_Spec spec;

  memset(&spec, 0, sizeof(spec));

  /* 3. parse_string_enum_array */
  ASSERT_EQ(CDD_C_SUCCESS, parse_string_enum_array(NULL, NULL, &enum_count));
  ASSERT_EQ(CDD_C_SUCCESS, parse_string_enum_array(NULL, &enum_out, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_string_enum_array(NULL, &enum_out, &enum_count));
  ASSERT_EQ(NULL, enum_out);
  ASSERT_EQ(0, enum_count);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("[]", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  ja = json_value_get_array(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_string_enum_array(ja, &enum_out, &enum_count));
  ASSERT_EQ(NULL, enum_out);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("[\"a\", 123]", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  ja = json_value_get_array(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_string_enum_array(ja, &enum_out, &enum_count));
  ASSERT_EQ(NULL, enum_out);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("[\"a\", \"b\"]", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  ja = json_value_get_array(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_string_enum_array(ja, &enum_out, &enum_count));
  g_cdd_alloc_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_string_enum_array(ja, &enum_out, &enum_count));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, parse_string_enum_array(ja, &enum_out, &enum_count));
  ASSERT_EQ(2, enum_count);
  ASSERT_STR_EQ("a", enum_out[0]);
  ASSERT_STR_EQ("b", enum_out[1]);
  free(enum_out[0]);
  free(enum_out[1]);
  free(enum_out);
  enum_out = NULL;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 4. copy_string_array */
  ASSERT_EQ(CDD_C_SUCCESS, copy_string_array(NULL, &copied_count, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, copy_string_array(&copied, NULL, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, copy_string_array(&copied, &copied_count, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            copy_string_array(&copied, &copied_count, sample_src, 0));
  ASSERT_EQ(NULL, copied);
  ASSERT_EQ(0, copied_count);

  sample_src[0] = cdd_test_mut_str("str1");
  sample_src[1] = NULL;
  sample_src[2] = cdd_test_mut_str("str2");

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            copy_string_array(&copied, &copied_count, sample_src, 3));
  g_cdd_alloc_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            copy_string_array(&copied, &copied_count, sample_src, 3));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS,
            copy_string_array(&copied, &copied_count, sample_src, 3));
  ASSERT_EQ(3, copied_count);
  ASSERT_STR_EQ("str1", copied[0]);
  ASSERT_EQ(NULL, copied[1]);
  ASSERT_STR_EQ("str2", copied[2]);
  free(copied[0]);
  free(copied[2]);
  free(copied);
  copied = NULL;

  /* 5. schema_is_string_enum_only */
  {
    int is_enum = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              schema_is_string_enum_only(NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_string_enum_only(NULL, &is_enum));
    ASSERT_EQ(0, is_enum);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string("{}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_string_enum_only(jo, &is_enum));
    ASSERT_EQ(0, is_enum);
    g_cdd_last_alloc_err = json_value_free(jv);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("{\"enum\": []}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_string_enum_only(jo, &is_enum));
    ASSERT_EQ(0, is_enum);
    g_cdd_last_alloc_err = json_value_free(jv);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"enum\": [\"a\"], \"type\": \"integer\"}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_string_enum_only(jo, &is_enum));
    ASSERT_EQ(0, is_enum);
    g_cdd_last_alloc_err = json_value_free(jv);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"enum\": [123], \"type\": \"string\"}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_string_enum_only(jo, &is_enum));
    ASSERT_EQ(0, is_enum);
    g_cdd_last_alloc_err = json_value_free(jv);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"enum\": [\"x\", \"y\"], \"type\": \"string\"}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_string_enum_only(jo, &is_enum));
    ASSERT_EQ(1, is_enum);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* 6. schema_is_struct_compatible */
  {
    int compat = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              schema_is_struct_compatible(NULL, NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_struct_compatible(NULL, NULL, &compat));
    ASSERT_EQ(0, compat);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string("true", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_struct_compatible(
                                 jv, (JSON_Object *)(size_t)1, &compat));
    ASSERT_EQ(0, compat);
    g_cdd_last_alloc_err = json_value_free(jv);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"enum\": [\"x\"], \"type\": \"string\"}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_struct_compatible(jv, jo, &compat));
    ASSERT_EQ(1, compat);
    g_cdd_last_alloc_err = json_value_free(jv);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("{\"type\": \"object\"}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_struct_compatible(jv, jo, &compat));
    ASSERT_EQ(1, compat);
    g_cdd_last_alloc_err = json_value_free(jv);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("{\"type\": \"string\"}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_struct_compatible(jv, jo, &compat));
    ASSERT_EQ(0, compat);
    g_cdd_last_alloc_err = json_value_free(jv);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("{\"properties\": {}}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_struct_compatible(jv, jo, &compat));
    ASSERT_EQ(1, compat);
    g_cdd_last_alloc_err = json_value_free(jv);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("{\"allOf\": []}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_struct_compatible(jv, jo, &compat));
    ASSERT_EQ(1, compat);
    g_cdd_last_alloc_err = json_value_free(jv);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("{\"anyOf\": []}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_struct_compatible(jv, jo, &compat));
    ASSERT_EQ(1, compat);
    g_cdd_last_alloc_err = json_value_free(jv);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("{\"oneOf\": []}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_struct_compatible(jv, jo, &compat));
    ASSERT_EQ(1, compat);
    g_cdd_last_alloc_err = json_value_free(jv);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string("{}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_is_struct_compatible(jv, jo, &compat));
    ASSERT_EQ(0, compat);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* 7. schema_has_composition */
  {
    int has_comp = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, schema_has_composition(NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, schema_has_composition(NULL, &has_comp));
    ASSERT_EQ(0, has_comp);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string("{}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_has_composition(jo, &has_comp));
    ASSERT_EQ(0, has_comp);
    g_cdd_last_alloc_err = json_value_free(jv);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("{\"allOf\": []}", &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, schema_has_composition(jo, &has_comp));
    ASSERT_EQ(1, has_comp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* 8. parse_schema_array_ref */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_schema_array_ref(NULL, NULL, NULL, &spec));
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("[]", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  ja = json_value_get_array(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_array_ref(ja, &schema_refs,
                                                  &schema_refs_count, &spec));
  ASSERT_EQ(NULL, schema_refs);
  ASSERT_EQ(0, schema_refs_count);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "[{\"type\": \"string\"}, {\"type\": \"integer\"}]", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  ja = json_value_get_array(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(
      CDD_C_ERROR_MEMORY,
      parse_schema_array_ref(ja, &schema_refs, &schema_refs_count, &spec));
  g_cdd_alloc_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(
      CDD_C_ERROR_MEMORY,
      parse_schema_array_ref(ja, &schema_refs, &schema_refs_count, &spec));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_array_ref(ja, &schema_refs,
                                                  &schema_refs_count, &spec));
  ASSERT_EQ(2, schema_refs_count);
  ASSERT(schema_refs != NULL);
  g_cdd_last_alloc_err = free_schema_ref_content(&schema_refs[0]);
  g_cdd_last_alloc_err = free_schema_ref_content(&schema_refs[1]);
  free(schema_refs);
  schema_refs = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_schema_array_ref(ja, NULL, &schema_refs_count, &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_schema_array_ref(ja, &schema_refs, NULL, &spec));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 9. parse_schema_ref_ptr */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_schema_ref_ptr(NULL, NULL, &spec));
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"type\": \"string\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_schema_ref_ptr(jo, NULL, &spec));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_schema_ref_ptr(jo, &single_ref, &spec));
  g_cdd_alloc_fail = 0;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_schema_ref_ptr(jo, &single_ref, &spec));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_ref_ptr(jo, &single_ref, &spec));
  ASSERT(single_ref != NULL);
  g_cdd_last_alloc_err = free_schema_ref_content(single_ref);
  free(single_ref);
  single_ref = NULL;
  g_cdd_last_alloc_err = json_value_free(jv);

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_SCHEMAS_COVERAGE_H */
