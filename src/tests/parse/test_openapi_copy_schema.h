/**
 * @file test_openapi_copy_schema.h
 * @brief Unit tests for OpenAPI schema reference and any-value deep-copy
 * routines.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_COPY_SCHEMA_H
#define TEST_OPENAPI_COPY_SCHEMA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Unit test for basic schema ref, item schema as array, and any-value
 * copy and OOM helpers.
 * @return Greatest test result.
 */
TEST test_openapi_copy_schema_helpers(void) {
  int k;
  struct OpenAPI_SchemaRef sr_src, sr_dst;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  /* 1. SchemaRef basic copy & free */
  memset(&sr_src, 0, sizeof(sr_src));
  memset(&sr_dst, 0, sizeof(sr_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_schema_ref(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_schema_ref(&sr_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_schema_ref(NULL, &sr_src));

  sr_src.inline_type = strdup("string");
  sr_src.format = strdup("date-time");
  sr_src.description = strdup("sr desc");
  sr_src.ref = strdup("#/components/schemas/MyString");

  rc = copy_schema_ref(&sr_dst, &sr_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("string", sr_dst.inline_type);
  ASSERT_STR_EQ("date-time", sr_dst.format);
  ASSERT_STR_EQ("sr desc", sr_dst.description);
  ASSERT_STR_EQ("#/components/schemas/MyString", sr_dst.ref);

  g_cdd_last_alloc_err = free_schema_ref_content(&sr_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&sr_dst);
  g_cdd_last_alloc_err = free_schema_ref_content(NULL);

  /* 2. Copy item schema as array */
  {
    struct OpenAPI_SchemaRef arr_dst, arr_src;
    memset(&arr_dst, 0, sizeof(arr_dst));
    memset(&arr_src, 0, sizeof(arr_src));
    arr_src.schema_is_boolean = 1;
    arr_src.schema_boolean_value = 1;
    arr_src.inline_type = strdup("string");
    rc = copy_item_schema_as_array(&arr_dst, &arr_src);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = free_schema_ref_content(&arr_dst);

    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              copy_item_schema_as_array(&arr_dst, &arr_src));
    g_cdd_strdup_fail = 0;

    g_cdd_last_alloc_err = free_schema_ref_content(&arr_src);
    ASSERT_EQ(CDD_C_SUCCESS, copy_item_schema_as_array(NULL, NULL));
  }

  /* 3. Copy any value JSON */
  {
    struct OpenAPI_Any any_src_json, any_dst_json;
    memset(&any_src_json, 0, sizeof(any_src_json));
    memset(&any_dst_json, 0, sizeof(any_dst_json));
    any_src_json.type = OA_ANY_JSON;
    any_src_json.json = strdup("{\"key\":\"value\"}");
    rc = copy_any_value(&any_dst_json, &any_src_json);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = free_any_value(&any_src_json);
    g_cdd_last_alloc_err = free_any_value(&any_dst_json);
    ASSERT_EQ(CDD_C_SUCCESS, copy_any_value(NULL, NULL));
  }

  /* 4. SchemaRef OOM loop */
  for (k = 1; k <= 6; ++k) {
    memset(&sr_src, 0, sizeof(sr_src));
    memset(&sr_dst, 0, sizeof(sr_dst));
    sr_src.inline_type = cdd_test_mut_str("string");
    sr_src.format = cdd_test_mut_str("date");
    sr_src.description = cdd_test_mut_str("desc");
    sr_src.ref = cdd_test_mut_str("ref");
    g_cdd_alloc_fail = k;
    rc = copy_schema_ref(&sr_dst, &sr_src);
    g_cdd_alloc_fail = 0;
    if (rc != CDD_C_SUCCESS) {
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    g_cdd_last_alloc_err = free_schema_ref_content(&sr_dst);
  }

  PASS();
}

/**
 * @brief Unit test for comprehensive SchemaRef deep copy, OOM loops, and
 * boolean constraint parsing.
 * @return Greatest test result.
 */
TEST test_openapi_copy_schema_comprehensive(void) {
  struct OpenAPI_SchemaRef sr_src, sr_dst;
  struct OpenAPI_Spec spec;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  memset(&sr_src, 0, sizeof(sr_src));
  memset(&sr_dst, 0, sizeof(sr_dst));

  sr_src.inline_type = strdup("string");
  sr_src.format = strdup("date-time");
  sr_src.description = strdup("sr desc");
  sr_src.summary = strdup("sr sum");
  sr_src.ref = strdup("#/components/schemas/MyString");
  sr_src.ref_name = strdup("MyString");
  sr_src.content_type = strdup("application/json");
  sr_src.content_media_type = strdup("application/json");
  sr_src.content_encoding = strdup("base64");
  sr_src.items_format = strdup("int32");
  sr_src.items_ref = strdup("#/components/schemas/Item");
  sr_src.items_content_media_type = strdup("text/plain");
  sr_src.items_content_encoding = strdup("7bit");
  sr_src.pattern = strdup("^[a-z]+$");
  sr_src.items_pattern = strdup("^[0-9]+$");
  sr_src.schema_extra_json = strdup("{\"x-extra\":1}");
  sr_src.items_extra_json = strdup("{\"x-item-extra\":2}");
  sr_src.deprecated_set = 1;
  sr_src.read_only_set = 1;
  sr_src.write_only_set = 1;
  sr_src.const_value_set = 1;
  sr_src.const_value.type = OA_ANY_STRING;
  sr_src.const_value.string = strdup("const_val");
  sr_src.items_const_value_set = 1;
  sr_src.items_const_value.type = OA_ANY_STRING;
  sr_src.items_const_value.string = strdup("item_const");
  sr_src.example_set = 1;
  sr_src.example.type = OA_ANY_STRING;
  sr_src.example.string = strdup("ex_val");
  sr_src.items_example_set = 1;
  sr_src.items_example.type = OA_ANY_STRING;
  sr_src.items_example.string = strdup("item_ex");
  sr_src.default_value_set = 1;
  sr_src.default_value.type = OA_ANY_STRING;
  sr_src.default_value.string = strdup("def_val");
  sr_src.items_default_value_set = 1;
  sr_src.items_default_value.type = OA_ANY_STRING;
  sr_src.items_default_value.string = strdup("item_def");

  sr_src.type_union = (char **)calloc(1, sizeof(char *));
  if (sr_src.type_union) {
    sr_src.type_union[0] = strdup("string");
    sr_src.n_type_union = 1;
  }
  sr_src.items_type_union = (char **)calloc(1, sizeof(char *));
  if (sr_src.items_type_union) {
    sr_src.items_type_union[0] = strdup("integer");
    sr_src.n_items_type_union = 1;
  }

  sr_src.enum_values =
      (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
  if (sr_src.enum_values) {
    sr_src.enum_values[0].type = OA_ANY_STRING;
    sr_src.enum_values[0].string = strdup("e1");
    sr_src.n_enum_values = 1;
  }
  sr_src.items_enum_values =
      (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
  if (sr_src.items_enum_values) {
    sr_src.items_enum_values[0].type = OA_ANY_STRING;
    sr_src.items_enum_values[0].string = strdup("ie1");
    sr_src.n_items_enum_values = 1;
  }

  sr_src.examples = (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
  if (sr_src.examples) {
    sr_src.examples[0].type = OA_ANY_STRING;
    sr_src.examples[0].string = strdup("ex1");
    sr_src.n_examples = 1;
  }
  sr_src.items_examples =
      (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
  if (sr_src.items_examples) {
    sr_src.items_examples[0].type = OA_ANY_STRING;
    sr_src.items_examples[0].string = strdup("iex1");
    sr_src.n_items_examples = 1;
  }

  sr_src.external_docs.description = strdup("ed_desc");
  sr_src.external_docs.url = strdup("http://ed_url");
  sr_src.external_docs.extensions_json = strdup("{\"x-ed\":1}");

  sr_src.discriminator.property_name = strdup("kind");
  sr_src.discriminator.default_mapping = strdup("def_map");
  sr_src.discriminator.extensions_json = strdup("{\"x-disc\":1}");
  sr_src.discriminator.mapping = (struct OpenAPI_DiscriminatorMap *)calloc(
      1, sizeof(struct OpenAPI_DiscriminatorMap));
  if (sr_src.discriminator.mapping) {
    sr_src.discriminator.mapping[0].value = strdup("k1");
    sr_src.discriminator.mapping[0].schema = strdup("Sub1");
    sr_src.discriminator.n_mapping = 1;
  }

  sr_src.xml.name = strdup("my_xml");
  sr_src.xml.namespace_uri = strdup("http://ns");
  sr_src.xml.prefix = strdup("pfx");
  sr_src.xml.extensions_json = strdup("{\"x-xml\":1}");

  sr_src.not_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.not_schema)
    sr_src.not_schema->inline_type = strdup("null");

  sr_src.if_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.if_schema)
    sr_src.if_schema->inline_type = strdup("object");

  sr_src.then_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.then_schema)
    sr_src.then_schema->inline_type = strdup("object");

  sr_src.else_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.else_schema)
    sr_src.else_schema->inline_type = strdup("object");

  sr_src.content_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.content_schema)
    sr_src.content_schema->inline_type = strdup("string");

  sr_src.items_content_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.items_content_schema)
    sr_src.items_content_schema->inline_type = strdup("string");

  sr_src.all_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.all_of) {
    sr_src.all_of[0].inline_type = strdup("string");
    sr_src.n_all_of = 1;
  }
  sr_src.any_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.any_of) {
    sr_src.any_of[0].inline_type = strdup("integer");
    sr_src.n_any_of = 1;
  }
  sr_src.one_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.one_of) {
    sr_src.one_of[0].inline_type = strdup("boolean");
    sr_src.n_one_of = 1;
  }

  sr_src.n_multipart_fields = 1;
  sr_src.multipart_fields = (struct OpenAPI_MultipartField *)calloc(
      1, sizeof(struct OpenAPI_MultipartField));
  if (sr_src.multipart_fields) {
    sr_src.multipart_fields[0].name = strdup("f1");
    sr_src.multipart_fields[0].type = strdup("string");
  }

  rc = copy_schema_ref(&sr_dst, &sr_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("string", sr_dst.inline_type);
  ASSERT_STR_EQ("date-time", sr_dst.format);
  ASSERT_STR_EQ("sr desc", sr_dst.description);
  ASSERT_STR_EQ("sr sum", sr_dst.summary);
  ASSERT_STR_EQ("#/components/schemas/MyString", sr_dst.ref);

  g_cdd_last_alloc_err = free_schema_ref_content(&sr_dst);

  {
    int k;
    for (k = 1; k <= 200; ++k) {
      memset(&sr_dst, 0, sizeof(sr_dst));
      g_cdd_alloc_fail = k;
      rc = copy_schema_ref(&sr_dst, &sr_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      g_cdd_last_alloc_err = free_schema_ref_content(&sr_dst);
    }
    g_cdd_alloc_fail = 0;
    for (k = 1; k <= 200; ++k) {
      memset(&sr_dst, 0, sizeof(sr_dst));
      g_cdd_strdup_fail = k;
      rc = copy_schema_ref(&sr_dst, &sr_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      g_cdd_last_alloc_err = free_schema_ref_content(&sr_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  g_cdd_last_alloc_err = free_schema_ref_content(&sr_src);

  /* Test schema boolean constraints parsing */
  {
    const char *excl_json =
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"paths\":{\"/"
        "items\":{\"get\":{\"responses\":{\"200\":{\"description\":"
        "\"OK\",\"content\":{\"application/json\":{\"schema\":{\"type\":"
        "\"integer\",\"minimum\":5,\"exclusiveMinimum\":true,\"maximum\":100,"
        "\"exclusiveMaximum\":true}}}}}}}}}";
    JSON_Value *jv = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(excl_json, &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    if (jv) {
      memset(&spec, 0, sizeof(spec));
      rc = openapi_load_from_json(jv, &spec);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      g_cdd_last_alloc_err = openapi_spec_free(&spec);
      g_cdd_last_alloc_err = json_value_free(jv);
    }
  }

  PASS();
}

/**
 * @brief Unit test for focused sub-object failure branches in
 * openapi_copy_schema.c.
 * @return Greatest test result.
 */
TEST test_openapi_copy_schema_subobject_failures(void) {
  struct OpenAPI_SchemaRef t_src, t_dst;

  /* 1. examples copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.n_examples = 1;
  t_src.examples = (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
  t_src.examples[0].type = OA_ANY_STRING;
  t_src.examples[0].string = strdup("ex");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 2. enum_values copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.n_enum_values = 1;
  t_src.enum_values =
      (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
  t_src.enum_values[0].type = OA_ANY_STRING;
  t_src.enum_values[0].string = strdup("ev");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 3. items_examples copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.n_items_examples = 1;
  t_src.items_examples =
      (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
  t_src.items_examples[0].type = OA_ANY_STRING;
  t_src.items_examples[0].string = strdup("iex");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 4. items_enum_values copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.n_items_enum_values = 1;
  t_src.items_enum_values =
      (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
  t_src.items_enum_values[0].type = OA_ANY_STRING;
  t_src.items_enum_values[0].string = strdup("iev");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 5. discriminator copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.discriminator.property_name = strdup("prop");
  t_src.discriminator.n_mapping = 1;
  t_src.discriminator.mapping = (struct OpenAPI_DiscriminatorMap *)calloc(
      1, sizeof(struct OpenAPI_DiscriminatorMap));
  t_src.discriminator.mapping[0].value = strdup("v");
  t_src.discriminator.mapping[0].schema = strdup("s");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 6. not_schema copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.not_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  t_src.not_schema->inline_type = strdup("s");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 7. if_schema copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.if_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  t_src.if_schema->inline_type = strdup("s");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 8. then_schema copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.then_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  t_src.then_schema->inline_type = strdup("s");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 9. else_schema copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.else_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  t_src.else_schema->inline_type = strdup("s");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 10. content_schema copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.content_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  t_src.content_schema->inline_type = strdup("s");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 11. items_content_schema copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.items_content_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  t_src.items_content_schema->inline_type = strdup("s");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 12. all_of copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.n_all_of = 1;
  t_src.all_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  t_src.all_of[0].inline_type = strdup("s");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 13. any_of copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.n_any_of = 1;
  t_src.any_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  t_src.any_of[0].inline_type = strdup("s");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 14. one_of copy failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.n_one_of = 1;
  t_src.one_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  t_src.one_of[0].inline_type = strdup("s");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  /* 15. multipart_fields calloc failure */
  memset(&t_src, 0, sizeof(t_src));
  memset(&t_dst, 0, sizeof(t_dst));
  t_src.n_multipart_fields = 1;
  t_src.multipart_fields = (struct OpenAPI_MultipartField *)calloc(
      1, sizeof(struct OpenAPI_MultipartField));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref(&t_dst, &t_src));
  g_cdd_alloc_fail = 0;
  g_cdd_last_alloc_err = free_schema_ref_content(&t_src);
  g_cdd_last_alloc_err = free_schema_ref_content(&t_dst);

  PASS();
}

/**
 * @brief Unit test for edge-case branch coverage in
 * openapi_copy_schema.c.
 * @return Greatest test result.
 */
TEST test_openapi_copy_schema_branches(void) {
  struct OpenAPI_SchemaRef s_src, s_dst;
  struct OpenAPI_Any any_src, any_dst;
  char dummy_str_buf[8];
  char *dummy_str;
  struct OpenAPI_Any dummy_any;
  struct OpenAPI_DiscriminatorMap dummy_map;
  struct OpenAPI_SchemaRef dummy_ref;
  struct OpenAPI_MultipartField dummy_field;

  dummy_str_buf[0] = 'd';
  dummy_str_buf[1] = '\0';
  dummy_str = dummy_str_buf;
  memset(&dummy_any, 0, sizeof(dummy_any));
  memset(&dummy_map, 0, sizeof(dummy_map));
  memset(&dummy_ref, 0, sizeof(dummy_ref));
  memset(&dummy_field, 0, sizeof(dummy_field));

  /* A. Non-null array pointers with n == 0 */
  memset(&s_src, 0, sizeof(s_src));
  memset(&s_dst, 0, sizeof(s_dst));
  s_src.type_union = &dummy_str;
  s_src.n_type_union = 0;
  s_src.items_type_union = &dummy_str;
  s_src.n_items_type_union = 0;
  s_src.examples = &dummy_any;
  s_src.n_examples = 0;
  s_src.enum_values = &dummy_any;
  s_src.n_enum_values = 0;
  s_src.discriminator.mapping = &dummy_map;
  s_src.discriminator.n_mapping = 0;
  s_src.items_enum_values = &dummy_any;
  s_src.n_items_enum_values = 0;
  s_src.items_examples = &dummy_any;
  s_src.n_items_examples = 0;
  s_src.all_of = &dummy_ref;
  s_src.n_all_of = 0;
  s_src.any_of = &dummy_ref;
  s_src.n_any_of = 0;
  s_src.one_of = &dummy_ref;
  s_src.n_one_of = 0;
  s_src.multipart_fields = NULL;
  s_src.n_multipart_fields = 1;
  ASSERT_EQ(CDD_C_SUCCESS, copy_schema_ref(&s_dst, &s_src));

  /* B. Discriminator mapping and multipart_fields with NULL subfields
   */
  memset(&s_src, 0, sizeof(s_src));
  memset(&s_dst, 0, sizeof(s_dst));
  s_src.discriminator.mapping = &dummy_map;
  s_src.discriminator.n_mapping = 1;
  dummy_map.value = NULL;
  dummy_map.schema = NULL;
  s_src.multipart_fields = &dummy_field;
  s_src.n_multipart_fields = 1;
  dummy_field.name = NULL;
  dummy_field.type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, copy_schema_ref(&s_dst, &s_src));
  g_cdd_last_alloc_err = free_schema_ref_content(&s_dst);

  /* C. copy_item_schema_as_array with non-boolean schema and NULL
   * checks */
  memset(&s_src, 0, sizeof(s_src));
  memset(&s_dst, 0, sizeof(s_dst));
  s_src.schema_is_boolean = 0;
  ASSERT_EQ(CDD_C_SUCCESS, copy_item_schema_as_array(NULL, &s_src));
  ASSERT_EQ(CDD_C_SUCCESS, copy_item_schema_as_array(&s_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_item_schema_as_array(&s_dst, &s_src));

  /* D. copy_any_value NULL checks and NULL string/json */
  memset(&any_src, 0, sizeof(any_src));
  memset(&any_dst, 0, sizeof(any_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_any_value(NULL, &any_src));
  ASSERT_EQ(CDD_C_SUCCESS, copy_any_value(&any_dst, NULL));

  any_src.type = OA_ANY_STRING;
  any_src.string = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, copy_any_value(&any_dst, &any_src));

  any_src.type = OA_ANY_JSON;
  any_src.json = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, copy_any_value(&any_dst, &any_src));

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_COPY_SCHEMA_H */
