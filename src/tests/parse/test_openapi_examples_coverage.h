/**
 * @file test_openapi_examples_coverage.h
 * @brief Unit tests for OpenAPI Example object copying, lookup, and parsing
 * (openapi_examples.c).
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_EXAMPLES_COVERAGE_H
#define TEST_OPENAPI_EXAMPLES_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Unit test for copy_example_fields and find_component_example.
 * @return Greatest test result.
 */
TEST test_openapi_examples_copy_and_find(void) {
  struct OpenAPI_Example ex_src;
  struct OpenAPI_Example ex_dst;
  struct OpenAPI_Example *out_ex = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Example comp_ex;
  char *comp_name = cdd_test_mut_str("comp_ex");

  memset(&spec, 0, sizeof(spec));
  memset(&ex_src, 0, sizeof(ex_src));
  memset(&ex_dst, 0, sizeof(ex_dst));
  memset(&comp_ex, 0, sizeof(comp_ex));

  comp_ex.summary = cdd_test_mut_str("comp_summary");
  spec.component_examples = &comp_ex;
  spec.component_example_names = &comp_name;
  spec.n_component_examples = 1;

  /* 1. copy_example_fields */
  ASSERT_EQ(CDD_C_SUCCESS, copy_example_fields(NULL, &ex_src));
  ASSERT_EQ(CDD_C_SUCCESS, copy_example_fields(&ex_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_example_fields(NULL, NULL));

  ex_src.name = cdd_test_mut_str("ex1");
  ex_src.ref = cdd_test_mut_str("#/ref");
  ex_src.extensions_json = cdd_test_mut_str("{\"x-e\": 1}");
  ex_src.summary = cdd_test_mut_str("sum");
  ex_src.description = cdd_test_mut_str("desc");
  ex_src.data_value_set = 1;
  ex_src.value_set = 1;
  ex_src.serialized_value = cdd_test_mut_str("serialized");
  ex_src.external_value = cdd_test_mut_str("http://ext");

  /* copy all fields successfully */
  ASSERT_EQ(CDD_C_SUCCESS, copy_example_fields(&ex_dst, &ex_src));
  /* copy when dst fields are already populated */
  ASSERT_EQ(CDD_C_SUCCESS, copy_example_fields(&ex_dst, &ex_src));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  /* Test OOM on each strdup in copy_example_fields */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_example_fields(&ex_dst, &ex_src));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ex_dst.name = strdup("name");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_example_fields(&ex_dst, &ex_src));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ex_dst.name = strdup("name");
  ex_dst.ref = strdup("ref");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_example_fields(&ex_dst, &ex_src));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ex_dst.name = strdup("name");
  ex_dst.ref = strdup("ref");
  ex_dst.extensions_json = strdup("{}");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_example_fields(&ex_dst, &ex_src));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ex_dst.name = strdup("name");
  ex_dst.ref = strdup("ref");
  ex_dst.extensions_json = strdup("{}");
  ex_dst.summary = strdup("sum");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_example_fields(&ex_dst, &ex_src));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ex_dst.name = strdup("name");
  ex_dst.ref = strdup("ref");
  ex_dst.extensions_json = strdup("{}");
  ex_dst.summary = strdup("sum");
  ex_dst.description = strdup("desc");
  ex_dst.data_value_set = 1;
  ex_dst.value_set = 1;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_example_fields(&ex_dst, &ex_src));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ex_dst.name = strdup("name");
  ex_dst.ref = strdup("ref");
  ex_dst.extensions_json = strdup("{}");
  ex_dst.summary = strdup("sum");
  ex_dst.description = strdup("desc");
  ex_dst.data_value_set = 1;
  ex_dst.value_set = 1;
  ex_dst.serialized_value = strdup("ser");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_example_fields(&ex_dst, &ex_src));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_strdup_fail = 0;

  memset(&ex_src, 0, sizeof(ex_src));
  memset(&ex_dst, 0, sizeof(ex_dst));
  ex_src.data_value_set = 1;
  ex_src.data_value.type = OA_ANY_STRING;
  ex_src.data_value.string = cdd_test_mut_str("foo");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_example_fields(&ex_dst, &ex_src));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_strdup_fail = 0;

  memset(&ex_src, 0, sizeof(ex_src));
  memset(&ex_dst, 0, sizeof(ex_dst));
  ex_src.value_set = 1;
  ex_src.value.type = OA_ANY_STRING;
  ex_src.value.string = cdd_test_mut_str("bar");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_example_fields(&ex_dst, &ex_src));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_strdup_fail = 0;

  /* 2. find_component_example */
  ASSERT_EQ(CDD_C_SUCCESS, find_component_example(
                               NULL, "#/components/examples/comp_ex", &out_ex));
  ASSERT_EQ(NULL, out_ex);
  ASSERT_EQ(CDD_C_SUCCESS, find_component_example(&spec, NULL, &out_ex));
  ASSERT_EQ(NULL, out_ex);
  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_example(&spec, "invalid_prefix", &out_ex));
  ASSERT_EQ(NULL, out_ex);
  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_example(&spec, "#/components/examples/nonexistent",
                                   &out_ex));
  ASSERT_EQ(NULL, out_ex);
  ASSERT_EQ(
      CDD_C_SUCCESS,
      find_component_example(&spec, "#/components/examples/comp_ex", &out_ex));
  ASSERT_EQ(&comp_ex, out_ex);

  /* json_pointer_unescape failure in find_component_example */
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(
      CDD_C_SUCCESS,
      find_component_example(&spec, "#/components/examples/comp_ex", &out_ex));
  ASSERT_EQ(NULL, out_ex);
  g_cdd_alloc_fail = 0;

  /* spec without component_example_names */
  out_ex = NULL;
  spec.component_example_names = NULL;
  ASSERT_EQ(
      CDD_C_SUCCESS,
      find_component_example(&spec, "#/components/examples/comp_ex", &out_ex));
  ASSERT_EQ(NULL, out_ex);
  spec.component_example_names = &comp_name;

  PASS();
}

/**
 * @brief Unit test for parse_example_object, parse_examples_object, and
 * parse_media_examples.
 * @return Greatest test result.
 */
TEST test_openapi_examples_parse_all_branches(void) {
  struct OpenAPI_Example ex_dst;
  struct OpenAPI_Example *examples_arr = NULL;
  size_t n_examples = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Example comp_ex;
  char *comp_name = cdd_test_mut_str("comp_ex");
  struct OpenAPI_Any example_any;
  int example_set = 0;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;

  memset(&spec, 0, sizeof(spec));
  memset(&ex_dst, 0, sizeof(ex_dst));
  memset(&comp_ex, 0, sizeof(comp_ex));
  memset(&example_any, 0, sizeof(example_any));

  comp_ex.summary = cdd_test_mut_str("comp_summary");
  spec.component_examples = &comp_ex;
  spec.component_example_names = &comp_name;
  spec.n_component_examples = 1;

  /* 3. parse_example_object */
  ASSERT_EQ(CDD_C_SUCCESS, parse_example_object(NULL, "ex", &ex_dst, &spec, 0));
  ASSERT_EQ(CDD_C_SUCCESS, parse_example_object(NULL, NULL, NULL, &spec, 0));

  /* ex_obj != NULL, out == NULL */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("{}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_example_object(jo, "ex", NULL, &spec, 0));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* name OOM */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"value\": \"v\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_example_object(jo, "name", &ex_dst, &spec, 0));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* $ref with and without resolve_refs */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"$ref\": \"#/components/examples/comp_ex\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_example_object(jo, "name", &ex_dst, &spec, 1));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ASSERT_EQ(CDD_C_SUCCESS, parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ASSERT_EQ(CDD_C_SUCCESS, parse_example_object(jo, NULL, &ex_dst, NULL, 1));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  /* $ref OOM */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_example_object(jo, NULL, &ex_dst, &spec, 1));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  /* $ref copy failure on OOM */
  g_cdd_strdup_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_example_object(jo, "name", &ex_dst, &spec, 1));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* serializedValue OOM */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"serializedValue\": \"ser\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* extensions failure */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"value\": 1, \"x-ext\": 1}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* dataValue failure */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"dataValue\": \"str\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* summary, description, externalValue */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"summary\": \"s\", \"description\": \"d\", "
                              "\"externalValue\": \"http://ext\"}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  g_cdd_strdup_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_strdup_fail = 0;

  /* Valid externalValue example */
  ASSERT_EQ(CDD_C_SUCCESS, parse_example_object(jo, "ex1", &ex_dst, &spec, 0));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* dataValue and value parsing */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"value\": \"v\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_example_object(jo, "ex2", &ex_dst, &spec, 0));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"dataValue\": \"dv\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_example_object(jo, "ex3", &ex_dst, &spec, 0));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* Validation failure: both value and externalValue */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"value\": \"v\", \"externalValue\": \"http://ext\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  g_cdd_last_alloc_err = free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 4. parse_examples_object */
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_examples_object(NULL, NULL, &n_examples, &spec, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_examples_object(NULL, &examples_arr, NULL, &spec, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_examples_object(NULL, &examples_arr, &n_examples, &spec, 0));
  ASSERT_EQ(NULL, examples_arr);
  ASSERT_EQ(0, n_examples);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("{}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_examples_object(jo, &examples_arr, &n_examples, &spec, 0));
  ASSERT_EQ(NULL, examples_arr);
  ASSERT_EQ(0, n_examples);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"ex1\": {\"value\": 1}, \"invalid_child\": "
                              "\"not_obj\", \"ex2\": {\"value\": 2}}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_examples_object(jo, &examples_arr, &n_examples, &spec, 0));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS,
            parse_examples_object(jo, &examples_arr, &n_examples, &spec, 0));
  ASSERT_EQ(3, n_examples);
  ASSERT(examples_arr != NULL);
  g_cdd_last_alloc_err = free_example(&examples_arr[0]);
  g_cdd_last_alloc_err = free_example(&examples_arr[2]);
  free(examples_arr);
  examples_arr = NULL;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 5. parse_media_examples */
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_media_examples(NULL, &example_any, &example_set,
                                 &examples_arr, &n_examples, &spec, 0));

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"example\": 1, \"examples\": {}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_media_examples(jo, &example_any, &example_set, &examples_arr,
                                 &n_examples, &spec, 0));
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"examples\": {\"ex\": {\"value\": \"test\"}}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_media_examples(jo, &example_any, &example_set, &examples_arr,
                                 &n_examples, &spec, 0));
  g_cdd_last_alloc_err = free_example(&examples_arr[0]);
  free(examples_arr);
  examples_arr = NULL;
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"example\": \"val\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_media_examples(jo, &example_any, &example_set, &examples_arr,
                                 &n_examples, &spec, 0));
  ASSERT_EQ(1, example_set);
  g_cdd_last_alloc_err = free_any_value(&example_any);
  g_cdd_last_alloc_err = json_value_free(jv);

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_EXAMPLES_COVERAGE_H */
