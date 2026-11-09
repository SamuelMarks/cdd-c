/**
 * @file test_openapi_schema_ref_coverage.h
 * @brief Comprehensive 100% coverage tests for openapi_schema_ref.c.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_SCHEMA_REF_COVERAGE_H
#define TEST_OPENAPI_SCHEMA_REF_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Tests all branches and error handling in parse_schema_ref.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_schema_ref_all_branches(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_SchemaRef out;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  JSON_Value *jv2 = NULL;
  JSON_Object *jo2 = NULL;
  size_t k;

  memset(&spec, 0, sizeof(spec));
  memset(&out, 0, sizeof(out));

  /* 1. NULL checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_schema_ref(NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_ref(NULL, &out, NULL));

  /* 2. Type array errors */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"type\": [\"string\"]}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 3. allOf / anyOf / oneOf errors */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"allOf\": 123}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"anyOf\": 123}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"oneOf\": 123}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 4. not / if / then / else non-object error */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("{\"not\": 123}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("{\"if\": 123}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("{\"then\": 123}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("{\"else\": 123}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 5. contentSchema error */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"contentSchema\": {\"allOf\": 123}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 6. default field strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"default\": \"sample\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 7. constraints strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"pattern\": \"^[a-z]+$\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 8. enum alloc fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"enum\": [\"a\"]}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 9. format strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"format\": \"int32\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 10. contentMediaType strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"contentMediaType\": \"application/json\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 11. contentEncoding strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"contentEncoding\": \"base64\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 12. summary strdup fail (with ref) */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"$ref\": \"#/components/schemas/Foo\", \"summary\": \"Sum\"}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 13. description strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"description\": \"Desc\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 14. externalDocs error */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"externalDocs\": {\"url\": \"http://example.com\"}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 15. discriminator error */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"discriminator\": {\"propertyName\": \"type\"}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 16. xml error */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"xml\": {\"name\": \"Pet\"}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 17. const strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"const\": \"val\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 18. examples alloc fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"examples\": [\"ex\"]}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 19. External/relative ref with resolved.resolved_ref free */
  spec.document_uri = cdd_test_mut_str("http://example.com/base/spec.json");
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"$ref\": \"../other.json#/components/schemas/Foo\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = free_schema_ref_content(&out);
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 20. Dynamic ref with resolved.resolved_ref */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"$dynamicRef\": \"../other.json#/components/schemas/Foo\"}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = free_schema_ref_content(&out);
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 21. ref strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"$ref\": \"../other.json#/components/schemas/Foo\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 22. ref_name unescape fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"$ref\": \"#/components/schemas/Foo\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_schema_ref(jo, &out, &spec));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 23. items constraints fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"pattern\": \"abc\"}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 24. items type array fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"type\": [\"str\"]}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 25. items format strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"format\": \"int\"}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 26. items contentMediaType strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"contentMediaType\": "
        "\"text/plain\"}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 27. items contentEncoding strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"contentEncoding\": \"base64\"}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 28. items enum alloc fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"enum\": [\"a\"]}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 29. items examples alloc fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"examples\": [\"ex\"]}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 30. items const strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"const\": \"val\"}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 31. items default strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"default\": \"val\"}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 32. items contentSchema fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"contentSchema\": "
        "{\"allOf\": 123}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 33. items $ref with resolved_ref */
  spec.document_uri = cdd_test_mut_str("http://example.com/base/spec.json");
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"type\": \"array\", \"items\": {\"$ref\": "
                              "\"../other.json#/components/schemas/Bar\"}}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = free_schema_ref_content(&out);

  /* 34. items_ref strdup fail */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 35. items ref_name unescape fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"type\": \"array\", \"items\": {\"$ref\": "
                              "\"#/components/schemas/Bar\"}}",
                              &_cdd_jv_1);
    jv2 = _cdd_jv_1;
  }
  jo2 = json_value_get_object(jv2);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_schema_ref(jo2, &out, &spec));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv2);

  /* 36. items $dynamicRef */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"$dynamicRef\": "
        "\"#/components/schemas/Bar\"}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = free_schema_ref_content(&out);
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 37. items inline_type strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"type\": \"integer\"}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 38. root inline_type strdup fail */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"type\": \"integer\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 39. empty object */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("{}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = free_schema_ref_content(&out);
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 40. external ref with name_dec unescape fail (line 503) */
  spec.document_uri = cdd_test_mut_str("http://example.com/other.json");
  spec.self_uri = NULL;
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"$ref\": \"./other.json#/components/schemas/Foo\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  for (k = 1; k <= 15; ++k) {
    g_cdd_alloc_fail = (int)k;
    memset(&out, 0, sizeof(out));
    g_cdd_last_alloc_err = parse_schema_ref(jo, &out, &spec);
    g_cdd_last_alloc_err = free_schema_ref_content(&out);
    g_cdd_alloc_fail = 0;
  }
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 41. external items ref with name_dec unescape fail (line 676) */
  spec.document_uri = cdd_test_mut_str("http://example.com/other.json");
  spec.self_uri = NULL;
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"type\": \"array\", \"items\": {\"$ref\": "
                              "\"./other.json#/components/schemas/Bar\"}}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  for (k = 1; k <= 15; ++k) {
    g_cdd_alloc_fail = (int)k;
    memset(&out, 0, sizeof(out));
    g_cdd_last_alloc_err = parse_schema_ref(jo, &out, &spec);
    g_cdd_last_alloc_err = free_schema_ref_content(&out);
    g_cdd_alloc_fail = 0;
  }
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 42. external ref with out->ref strdup fail (line 493) */
  spec.document_uri = cdd_test_mut_str("http://example.com/base/spec.json");
  spec.self_uri = cdd_test_mut_str("http://example.com/other.json");
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"$ref\": \"../other.json#/components/schemas/Foo\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 43. external items ref with out->items_ref strdup fail (line 666) */
  spec.document_uri = cdd_test_mut_str("http://example.com/base/spec.json");
  spec.self_uri = cdd_test_mut_str("http://example.com/other.json");
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"type\": \"array\", \"items\": {\"$ref\": "
                              "\"../other.json#/components/schemas/Bar\"}}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 44. allOf success branch (line 269) */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"allOf\": [{}]}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = free_schema_ref_content(&out);
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 45. anyOf success branch (line 276) */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"anyOf\": [{}]}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = free_schema_ref_content(&out);
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 46. contentSchema non-object branch (line 312) */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"contentSchema\": 123}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = free_schema_ref_content(&out);
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 47. items contentSchema non-object branch (line 639) */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"contentSchema\": 123}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = free_schema_ref_content(&out);
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 48. items local ref out->items_ref fail (line 665) */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"type\": \"array\", \"items\": {\"$ref\": "
                              "\"#/components/schemas/Bar\"}}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 49. items without type or ref (line 685) */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"type\": \"array\", \"items\": {\"description\": \"no_type\"}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, parse_schema_ref(jo, &out, &spec));
  g_cdd_last_alloc_err = free_schema_ref_content(&out);
  g_cdd_last_alloc_err = json_value_free(jv);

  spec.document_uri = NULL;
  spec.self_uri = NULL;
  g_cdd_strdup_fail = 0;
  g_cdd_alloc_fail = 0;
  g_fail_io_after = -1;

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_SCHEMA_REF_COVERAGE_H */
