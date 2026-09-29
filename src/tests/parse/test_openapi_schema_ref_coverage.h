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
#include "openapi/parse/openapi_test_helpers.h"
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
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_schema_ref(NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(NULL, &out, NULL));

  /* 2. Type array errors */
  jv = json_parse_string("{\"type\": [\"string\"]}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_alloc_fail = 0;
  json_value_free(jv);

  /* 3. allOf / anyOf / oneOf errors */
  jv = json_parse_string("{\"allOf\": 123}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  json_value_free(jv);

  jv = json_parse_string("{\"anyOf\": 123}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  json_value_free(jv);

  jv = json_parse_string("{\"oneOf\": 123}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  json_value_free(jv);

  /* 4. not / if / then / else non-object error */
  jv = json_parse_string("{\"not\": 123}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  json_value_free(jv);

  jv = json_parse_string("{\"if\": 123}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  json_value_free(jv);

  jv = json_parse_string("{\"then\": 123}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  json_value_free(jv);

  jv = json_parse_string("{\"else\": 123}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  json_value_free(jv);

  /* 5. contentSchema error */
  jv = json_parse_string("{\"contentSchema\": {\"allOf\": 123}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  json_value_free(jv);

  /* 6. default field strdup fail */
  jv = json_parse_string("{\"default\": \"sample\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 7. constraints strdup fail */
  jv = json_parse_string("{\"pattern\": \"^[a-z]+$\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 8. enum alloc fail */
  jv = json_parse_string("{\"enum\": [\"a\"]}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 9. format strdup fail */
  jv = json_parse_string("{\"format\": \"int32\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 10. contentMediaType strdup fail */
  jv = json_parse_string("{\"contentMediaType\": \"application/json\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 11. contentEncoding strdup fail */
  jv = json_parse_string("{\"contentEncoding\": \"base64\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 12. summary strdup fail (with ref) */
  jv = json_parse_string(
      "{\"$ref\": \"#/components/schemas/Foo\", \"summary\": \"Sum\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 13. description strdup fail */
  jv = json_parse_string("{\"description\": \"Desc\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 14. externalDocs error */
  jv = json_parse_string(
      "{\"externalDocs\": {\"url\": \"http://example.com\"}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 15. discriminator error */
  jv = json_parse_string("{\"discriminator\": {\"propertyName\": \"type\"}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 16. xml error */
  jv = json_parse_string("{\"xml\": {\"name\": \"Pet\"}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 17. const strdup fail */
  jv = json_parse_string("{\"const\": \"val\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 18. examples alloc fail */
  jv = json_parse_string("{\"examples\": [\"ex\"]}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 19. External/relative ref with resolved.resolved_ref free */
  spec.document_uri = (char *)(size_t) "http://example.com/base/spec.json";
  jv = json_parse_string(
      "{\"$ref\": \"../other.json#/components/schemas/Foo\"}");
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  cdd_test_free_schema_ref_content(&out);
  json_value_free(jv);

  /* 20. Dynamic ref with resolved.resolved_ref */
  jv = json_parse_string(
      "{\"$dynamicRef\": \"../other.json#/components/schemas/Foo\"}");
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  cdd_test_free_schema_ref_content(&out);
  json_value_free(jv);

  /* 21. ref strdup fail */
  jv = json_parse_string(
      "{\"$ref\": \"../other.json#/components/schemas/Foo\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 22. ref_name unescape fail */
  jv = json_parse_string("{\"$ref\": \"#/components/schemas/Foo\"}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_alloc_fail = 0;
  json_value_free(jv);

  /* 23. items constraints fail */
  jv = json_parse_string(
      "{\"type\": \"array\", \"items\": {\"pattern\": \"abc\"}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 24. items type array fail */
  jv = json_parse_string(
      "{\"type\": \"array\", \"items\": {\"type\": [\"str\"]}}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_alloc_fail = 0;
  json_value_free(jv);

  /* 25. items format strdup fail */
  jv = json_parse_string(
      "{\"type\": \"array\", \"items\": {\"format\": \"int\"}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 26. items contentMediaType strdup fail */
  jv = json_parse_string(
      "{\"type\": \"array\", \"items\": {\"contentMediaType\": "
      "\"text/plain\"}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 27. items contentEncoding strdup fail */
  jv = json_parse_string(
      "{\"type\": \"array\", \"items\": {\"contentEncoding\": \"base64\"}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 28. items enum alloc fail */
  jv = json_parse_string(
      "{\"type\": \"array\", \"items\": {\"enum\": [\"a\"]}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 29. items examples alloc fail */
  jv = json_parse_string(
      "{\"type\": \"array\", \"items\": {\"examples\": [\"ex\"]}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 30. items const strdup fail */
  jv = json_parse_string(
      "{\"type\": \"array\", \"items\": {\"const\": \"val\"}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 31. items default strdup fail */
  jv = json_parse_string(
      "{\"type\": \"array\", \"items\": {\"default\": \"val\"}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 32. items contentSchema fail */
  jv = json_parse_string("{\"type\": \"array\", \"items\": {\"contentSchema\": "
                         "{\"allOf\": 123}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  json_value_free(jv);

  /* 33. items $ref with resolved_ref */
  spec.document_uri = (char *)(size_t) "http://example.com/base/spec.json";
  jv = json_parse_string("{\"type\": \"array\", \"items\": {\"$ref\": "
                         "\"../other.json#/components/schemas/Bar\"}}");
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  cdd_test_free_schema_ref_content(&out);

  /* 34. items_ref strdup fail */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 35. items ref_name unescape fail */
  jv2 = json_parse_string("{\"type\": \"array\", \"items\": {\"$ref\": "
                          "\"#/components/schemas/Bar\"}}");
  jo2 = json_value_get_object(jv2);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_schema_ref(jo2, &out, &spec));
  g_cdd_alloc_fail = 0;
  json_value_free(jv2);

  /* 36. items $dynamicRef */
  jv = json_parse_string("{\"type\": \"array\", \"items\": {\"$dynamicRef\": "
                         "\"#/components/schemas/Bar\"}}");
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  cdd_test_free_schema_ref_content(&out);
  json_value_free(jv);

  /* 37. items inline_type strdup fail */
  jv = json_parse_string(
      "{\"type\": \"array\", \"items\": {\"type\": \"integer\"}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 38. root inline_type strdup fail */
  jv = json_parse_string("{\"type\": \"integer\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 39. empty object */
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  cdd_test_free_schema_ref_content(&out);
  json_value_free(jv);

  /* 40. external ref with name_dec unescape fail (line 503) */
  spec.document_uri = (char *)(size_t) "http://example.com/other.json";
  spec.self_uri = NULL;
  jv =
      json_parse_string("{\"$ref\": \"./other.json#/components/schemas/Foo\"}");
  jo = json_value_get_object(jv);
  for (k = 1; k <= 15; ++k) {
    g_cdd_alloc_fail = (int)k;
    memset(&out, 0, sizeof(out));
    cdd_test_parse_schema_ref(jo, &out, &spec);
    cdd_test_free_schema_ref_content(&out);
    g_cdd_alloc_fail = 0;
  }
  json_value_free(jv);

  /* 41. external items ref with name_dec unescape fail (line 676) */
  spec.document_uri = (char *)(size_t) "http://example.com/other.json";
  spec.self_uri = NULL;
  jv = json_parse_string("{\"type\": \"array\", \"items\": {\"$ref\": "
                         "\"./other.json#/components/schemas/Bar\"}}");
  jo = json_value_get_object(jv);
  for (k = 1; k <= 15; ++k) {
    g_cdd_alloc_fail = (int)k;
    memset(&out, 0, sizeof(out));
    cdd_test_parse_schema_ref(jo, &out, &spec);
    cdd_test_free_schema_ref_content(&out);
    g_cdd_alloc_fail = 0;
  }
  json_value_free(jv);

  /* 42. external ref with out->ref strdup fail (line 493) */
  spec.document_uri = (char *)(size_t) "http://example.com/base/spec.json";
  spec.self_uri = (char *)(size_t) "http://example.com/other.json";
  jv = json_parse_string(
      "{\"$ref\": \"../other.json#/components/schemas/Foo\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 43. external items ref with out->items_ref strdup fail (line 666) */
  spec.document_uri = (char *)(size_t) "http://example.com/base/spec.json";
  spec.self_uri = (char *)(size_t) "http://example.com/other.json";
  jv = json_parse_string("{\"type\": \"array\", \"items\": {\"$ref\": "
                         "\"../other.json#/components/schemas/Bar\"}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 44. allOf success branch (line 269) */
  jv = json_parse_string("{\"allOf\": [{}]}");
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  cdd_test_free_schema_ref_content(&out);
  json_value_free(jv);

  /* 45. anyOf success branch (line 276) */
  jv = json_parse_string("{\"anyOf\": [{}]}");
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  cdd_test_free_schema_ref_content(&out);
  json_value_free(jv);

  /* 46. contentSchema non-object branch (line 312) */
  jv = json_parse_string("{\"contentSchema\": 123}");
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  cdd_test_free_schema_ref_content(&out);
  json_value_free(jv);

  /* 47. items contentSchema non-object branch (line 639) */
  jv = json_parse_string(
      "{\"type\": \"array\", \"items\": {\"contentSchema\": 123}}");
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  cdd_test_free_schema_ref_content(&out);
  json_value_free(jv);

  /* 48. items local ref out->items_ref fail (line 665) */
  jv = json_parse_string("{\"type\": \"array\", \"items\": {\"$ref\": "
                         "\"#/components/schemas/Bar\"}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_schema_ref(jo, &out, &spec));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 49. items without type or ref (line 685) */
  jv = json_parse_string(
      "{\"type\": \"array\", \"items\": {\"description\": \"no_type\"}}");
  jo = json_value_get_object(jv);
  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(jo, &out, &spec));
  cdd_test_free_schema_ref_content(&out);
  json_value_free(jv);

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
