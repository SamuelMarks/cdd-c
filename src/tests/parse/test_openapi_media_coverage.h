/**
 * @file test_openapi_media_coverage.h
 * @brief Media type helper functions and content object parsing branch coverage
 * tests.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_MEDIA_COVERAGE_H
#define TEST_OPENAPI_MEDIA_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Tests media_type_base_len, media_type_base_equal, media_type_is_json,
 * media_type_specificity, media_type_preference_rank,
 * select_primary_media_type_index, and find_media_object_by_name.
 * @return Greatest test result.
 */
TEST test_openapi_media_type_helpers_branches(void) {
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  size_t blen = 0;
  int spec_val = 0;
  int rank_val = 0;
  int idx_val = 0;

  /* 1. media_type_base_len */
  ASSERT_EQ(CDD_C_SUCCESS, media_type_base_len(NULL, &blen));
  ASSERT_EQ(0, blen);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_base_len("text/plain", &blen));
  ASSERT_EQ(10, blen);
  ASSERT_EQ(CDD_C_SUCCESS,
            media_type_base_len("text/plain; charset=utf-8", &blen));
  ASSERT_EQ(10, blen);

  /* 2. media_type_base_equal */
  ASSERT_EQ(CDD_C_SUCCESS, media_type_base_equal(NULL, "text/plain"));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_base_equal("text/plain", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_base_equal(NULL, NULL));
  ASSERT_EQ(0, media_type_base_equal("text/plain", "application/json"));
  ASSERT_EQ(0, media_type_base_equal("text/plain", "text/p"));
  ASSERT_EQ(1, media_type_base_equal("text/plain; a=1", "text/plain; b=2"));
  ASSERT_EQ(0, media_type_base_equal("text/plaiX", "text/plain"));

  /* 3. media_type_is_json */
  ASSERT_EQ(CDD_C_SUCCESS, media_type_is_json(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_is_json(""));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_is_json("text"));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_is_json("application/jpeg"));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_is_json("text/plain"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, media_type_is_json("application/json"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            media_type_is_json("application/json; charset=utf-8"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            media_type_is_json("application/problem+json"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            media_type_is_json("application/problem+json; charset=utf-8"));

  /* 4. media_type_specificity */
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity(NULL, &spec_val));
  ASSERT_EQ(0, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("", &spec_val));
  ASSERT_EQ(0, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("invalidnoslash", &spec_val));
  ASSERT_EQ(2, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("invalidnoslash", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("/leading", &spec_val));
  ASSERT_EQ(0, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("/leading", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("trailing/", &spec_val));
  ASSERT_EQ(2, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("*/*", &spec_val));
  ASSERT_EQ(0, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("*/*", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("text/*", &spec_val));
  ASSERT_EQ(1, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("text/*", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("x/*", &spec_val));
  ASSERT_EQ(1, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("*/a", &spec_val));
  ASSERT_EQ(2, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("*/json", &spec_val));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("text/plain", &spec_val));
  ASSERT_EQ(2, spec_val);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_specificity("text/plain", NULL));

  /* 5. media_type_preference_rank */
  ASSERT_EQ(CDD_C_SUCCESS, media_type_preference_rank(NULL, &rank_val));
  ASSERT_EQ(0, rank_val);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_preference_rank(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            media_type_preference_rank("application/json", &rank_val));
  ASSERT_EQ(3, rank_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            media_type_preference_rank("application/json", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_preference_rank(
                               "application/x-www-form-urlencoded", &rank_val));
  ASSERT_EQ(2, rank_val);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_preference_rank(
                               "application/x-www-form-urlencoded", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            media_type_preference_rank("multipart/form-data", &rank_val));
  ASSERT_EQ(1, rank_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            media_type_preference_rank("multipart/form-data", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_preference_rank("image/png", &rank_val));
  ASSERT_EQ(0, rank_val);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_preference_rank("image/png", NULL));

  /* 6. select_primary_media_type_index */
  ASSERT_EQ(CDD_C_SUCCESS, select_primary_media_type_index(NULL, 0, &idx_val));
  ASSERT_EQ(-1, idx_val);
  ASSERT_EQ(CDD_C_SUCCESS, select_primary_media_type_index(NULL, 0, NULL));
  {
    struct OpenAPI_MediaType test_mts[6];
    memset(test_mts, 0, sizeof(test_mts));
    test_mts[0].name = cdd_test_mut_str("*/*");
    test_mts[1].name = cdd_test_mut_str("text/plain");
    test_mts[2].name = cdd_test_mut_str("application/json");
    test_mts[3].name = cdd_test_mut_str("multipart/form-data");
    test_mts[4].name = cdd_test_mut_str("image/png");
    test_mts[5].name = cdd_test_mut_str("*/*");
    ASSERT_EQ(CDD_C_SUCCESS,
              select_primary_media_type_index(test_mts, 6, &idx_val));
    ASSERT_EQ(2, idx_val);
    ASSERT_EQ(CDD_C_SUCCESS,
              select_primary_media_type_index(test_mts, 6, NULL));
  }

  /* 7. find_media_object_by_name */
  {
    const JSON_Object *found = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              find_media_object_by_name(NULL, "application/json", &found));
    ASSERT_EQ(NULL, found);
    ASSERT_EQ(CDD_C_SUCCESS, find_media_object_by_name(NULL, NULL, &found));
    ASSERT_EQ(NULL, found);

    jv = json_parse_string(
        "{\"application/json\": {\"schema\": {\"type\": \"string\"}}, "
        "\"text/plain; charset=utf-8\": {\"schema\": {\"type\": \"string\"}}}");
    ASSERT(jv != NULL);
    jo = json_value_get_object(jv);

    ASSERT_EQ(CDD_C_SUCCESS, find_media_object_by_name(jo, NULL, &found));
    ASSERT_EQ(NULL, found);
    ASSERT_EQ(CDD_C_SUCCESS,
              find_media_object_by_name(jo, "application/json", &found));
    ASSERT(found != NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              find_media_object_by_name(jo, "text/plain", &found));
    ASSERT(found != NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              find_media_object_by_name(jo, "nonexistent/mime", &found));
    ASSERT_EQ(NULL, found);
    json_value_free(jv);
  }

  PASS();
}

/**
 * @brief Tests parse_media_type_object and parse_content_object branches.
 * @return Greatest test result.
 */
TEST test_openapi_media_type_and_content_parse_branches(void) {
  struct OpenAPI_MediaType mt;
  struct OpenAPI_MediaType *out_mts = NULL;
  size_t out_count = 0;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_MediaType comp_mt;
  char *comp_name = cdd_test_mut_str("comp_mt");

  memset(&spec, 0, sizeof(spec));
  memset(&comp_mt, 0, sizeof(comp_mt));
  comp_mt.name = cdd_test_mut_str("application/json");
  spec.n_component_media_types = 1;
  spec.component_media_type_names = &comp_name;
  spec.component_media_types = &comp_mt;

  /* 8. parse_media_type_object */
  ASSERT_EQ(CDD_C_SUCCESS, parse_media_type_object(NULL, &mt, &spec, 0));
  ASSERT_EQ(CDD_C_SUCCESS, parse_media_type_object(NULL, NULL, &spec, 0));

  jv = json_parse_string("{\"encoding\": {}, \"prefixEncoding\": []}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_media_type_object(jo, NULL, &spec, 0));
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_media_type_object(jo, &mt, &spec, 0));
  json_value_free(jv);

  jv = json_parse_string("{\"encoding\": {}, \"itemEncoding\": {}}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_media_type_object(jo, &mt, &spec, 0));
  json_value_free(jv);

  /* $ref with and without resolution */
  jv = json_parse_string("{\"$ref\": \"#/components/mediaTypes/comp_mt\"}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, parse_media_type_object(jo, &mt, &spec, 1));
  free_media_type(&mt);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, parse_media_type_object(jo, &mt, &spec, 0));
  free_media_type(&mt);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, parse_media_type_object(jo, &mt, NULL, 1));
  free_media_type(&mt);
  json_value_free(jv);

  /* $ref with missing component */
  jv = json_parse_string("{\"$ref\": \"#/components/mediaTypes/missing\"}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, parse_media_type_object(jo, &mt, &spec, 1));
  free_media_type(&mt);

  /* $ref OOM */
  g_cdd_strdup_fail = 1;
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_media_type_object(jo, &mt, &spec, 1));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* schema boolean vs object vs other */
  jv = json_parse_string("{\"schema\": true, \"itemSchema\": false}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, parse_media_type_object(jo, &mt, &spec, 0));
  ASSERT_EQ(1, mt.schema_set);
  ASSERT_EQ(1, mt.schema.schema_is_boolean);
  ASSERT_EQ(1, mt.item_schema_set);
  ASSERT_EQ(1, mt.item_schema.schema_is_boolean);
  free_media_type(&mt);
  json_value_free(jv);

  jv = json_parse_string("{\"schema\": \"non_obj\", \"itemSchema\": 123}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, parse_media_type_object(jo, &mt, &spec, 0));
  free_media_type(&mt);
  json_value_free(jv);

  /* itemEncoding with OOM and success */
  jv = json_parse_string(
      "{\"itemEncoding\": {\"contentType\": \"text/plain\"}}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_media_type_object(jo, &mt, &spec, 0));
  g_cdd_alloc_fail = 0;
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_SUCCESS, parse_media_type_object(jo, &mt, &spec, 0));
  free_media_type(&mt);
  json_value_free(jv);

  /* itemEncoding error branch */
  jv = json_parse_string(
      "{\"itemEncoding\": {\"encoding\": {}, \"prefixEncoding\": []}}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_media_type_object(jo, &mt, &spec, 0));
  free_media_type(&mt);
  json_value_free(jv);

  /* prefixEncoding error branch */
  jv = json_parse_string(
      "{\"prefixEncoding\": [{\"encoding\": {}, \"prefixEncoding\": []}]}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_media_type_object(jo, &mt, &spec, 0));
  free_media_type(&mt);
  json_value_free(jv);

  /* collect_extensions failure */
  jv = json_parse_string("{\"x-ext\": \"val\"}");
  jo = json_value_get_object(jv);
  memset(&mt, 0, sizeof(mt));
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_media_type_object(jo, &mt, &spec, 0));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 9. parse_content_object */
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_content_object(NULL, NULL, &out_count, &spec, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_content_object(NULL, &out_mts, NULL, &spec, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_content_object(NULL, &out_mts, &out_count, &spec, 0));
  ASSERT_EQ(NULL, out_mts);
  ASSERT_EQ(0, out_count);

  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_content_object(jo, &out_mts, &out_count, &spec, 0));
  ASSERT_EQ(NULL, out_mts);
  ASSERT_EQ(0, out_count);
  json_value_free(jv);

  /* Content with non-object values -> valid == 0 */
  jv = json_parse_string("{\"application/json\": \"not_an_obj\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_content_object(jo, &out_mts, &out_count, &spec, 0));
  ASSERT_EQ(NULL, out_mts);
  ASSERT_EQ(0, out_count);
  json_value_free(jv);

  /* Content OOM on calloc */
  jv = json_parse_string("{\"application/json\": {}}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_content_object(jo, &out_mts, &out_count, &spec, 0));
  g_cdd_alloc_fail = 0;

  /* Content OOM on strdup */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_content_object(jo, &out_mts, &out_count, &spec, 0));
  g_cdd_strdup_fail = 0;

  /* Content with child parse error */
  json_value_free(jv);
  jv = json_parse_string(
      "{\"application/json\": {\"encoding\": {}, \"prefixEncoding\": []}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_content_object(jo, &out_mts, &out_count, &spec, 0));
  json_value_free(jv);

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_MEDIA_COVERAGE_H */
