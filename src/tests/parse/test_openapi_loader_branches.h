/**
 * @file test_openapi_loader_branches.h
 * @brief URI, schema ref, media type, and swagger2 branch tests.
 */

#ifndef TEST_OPENAPI_LOADER_BRANCHES_H
#define TEST_OPENAPI_LOADER_BRANCHES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

TEST test_openapi_loader_uri_and_schema_ref_branches(void) {
  char *res = NULL;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SchemaRef sref;
  struct StructFields *sf_out = NULL;
  cdd_c_error_t rc;

  /* 1. resolve_uri_reference branches */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_resolve_uri_reference(NULL, NULL, &res));
  ASSERT(res == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_resolve_uri_reference("http://base", "", &res));
  ASSERT_STR_EQ("", res);
  free(res);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_resolve_uri_reference(
                               "http://base", "http://abs/uri", &res));
  ASSERT_STR_EQ("http://abs/uri", res);
  free(res);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_resolve_uri_reference(NULL, "rel/path", &res));
  ASSERT_STR_EQ("rel/path", res);
  free(res);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_resolve_uri_reference("", "rel/path", &res));
  ASSERT_STR_EQ("rel/path", res);
  free(res);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_resolve_uri_reference("https://example.com/api",
                                           "//cdn.example.com/lib.js", &res));
  ASSERT_STR_EQ("https://cdn.example.com/lib.js", res);
  free(res);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_resolve_uri_reference(
                               "nodomain", "//cdn.example.com/lib.js", &res));
  ASSERT_STR_EQ("//cdn.example.com/lib.js", res);
  free(res);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_resolve_uri_reference("https://example.com", "sub", &res));
  ASSERT_STR_EQ("https://example.com/sub", res);
  free(res);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_resolve_uri_reference(
                               "https://example.com/", "sub", &res));
  ASSERT_STR_EQ("https://example.com/sub", res);
  free(res);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_resolve_uri_reference("https://example.com/api/v1/",
                                           "../v2/users", &res));
  ASSERT_STR_EQ("https://example.com/api/v2/users", res);
  free(res);

  /* 2. openapi_spec_find_schema_for_ref branches */
  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_SUCCESS,
            openapi_spec_find_schema_for_ref(NULL, NULL, &sf_out));
  ASSERT(sf_out == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            openapi_spec_find_schema_for_ref(&spec, NULL, &sf_out));
  ASSERT(sf_out == NULL);

  memset(&sref, 0, sizeof(sref));
  sref.all_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  sref.n_all_of = 1;
  sref.any_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  sref.n_any_of = 1;
  sref.one_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  sref.n_one_of = 1;
  sref.not_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  sref.if_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  sref.then_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  sref.else_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  sref.ref = strdup("#/components/schemas/NonExistent");
  sref.ref_is_dynamic = 1;

  ASSERT_EQ(CDD_C_SUCCESS,
            openapi_spec_find_schema_for_ref(&spec, &sref, &sf_out));
  ASSERT(sf_out == NULL);
  free(sref.ref);

  sref.all_of = NULL;
  sref.n_all_of = 0;
  sref.any_of = NULL;
  sref.n_any_of = 0;
  sref.one_of = NULL;
  sref.n_one_of = 0;
  sref.not_schema = NULL;
  sref.if_schema = NULL;
  sref.then_schema = NULL;
  sref.else_schema = NULL;

  sref.ref = strdup("#/components/schemas/NonExistent");
  sref.ref_is_dynamic = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            openapi_spec_find_schema_for_ref(&spec, &sref, &sf_out));
  ASSERT(sf_out == NULL);
  free(sref.ref);

  /* 3. component_callback_is_referenced branches */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_callback_is_referenced(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_callback_is_referenced(&spec, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_callback_is_referenced(NULL, "cb"));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_callback_is_referenced(&spec, "cb"));

  /* 4. parse_request_body_object array of inline objects */
  {
    const char *array_rb_json =
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"paths\":{\"/users\":{\"post\":{\"operationId\":\"createBatchUsers\","
        "\"requestBody\":{\"content\":{\"application/json\":{\"schema\":{"
        "\"type\":\"array\",\"items\":{\"type\":\"object\",\"properties\":{"
        "\"username\":{\"type\":\"string\"}}}}}}},"
        "\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";
    JSON_Value *jv = json_parse_string(array_rb_json);
    if (jv) {
      memset(&spec, 0, sizeof(spec));
      rc = openapi_load_from_json(jv, &spec);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      openapi_spec_free(&spec);
      json_value_free(jv);
    }
  }

  PASS();
}

TEST test_openapi_loader_media_type_and_response_examples(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_MediaType *mt_out = NULL;
  JSON_Object *jo_out = NULL;
  JSON_Value *jv_content = NULL;
  JSON_Object *jo_content = NULL;
  JSON_Value *jv = NULL;
  cdd_c_error_t rc;

  /* 1. find_component_media_type */
  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_media_type(NULL, NULL, &mt_out));
  ASSERT(mt_out == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_media_type(&spec, NULL, &mt_out));
  ASSERT(mt_out == NULL);

  spec.component_media_types =
      (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
  spec.component_media_type_names = (char **)calloc(1, sizeof(char *));
  if (spec.component_media_types && spec.component_media_type_names) {
    spec.component_media_types[0].name = strdup("application/json");
    spec.component_media_type_names[0] = strdup("Json");
    spec.n_component_media_types = 1;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_media_type(
                  &spec, "#/components/mediaTypes/Json", &mt_out));
    ASSERT(mt_out != NULL);

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_media_type(
                  &spec, "#/components/mediaTypes/Xml", &mt_out));
    ASSERT(mt_out == NULL);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_component_media_type(
                                 &spec, "#/bad/prefix/Json", &mt_out));
    ASSERT(mt_out == NULL);
  }
  openapi_spec_free(&spec);

  /* 2. find_media_object_by_name */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_media_object_by_name(NULL, NULL, &jo_out));
  ASSERT(jo_out == NULL);

  jv_content = json_parse_string(
      "{\"application/json\":{\"schema\":{\"type\":\"string\"}},\"text/html; "
      "charset=utf-8\":{\"schema\":{\"type\":\"string\"}}}");
  if (jv_content) {
    jo_content = json_value_get_object(jv_content);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_media_object_by_name(
                                 jo_content, "application/json", &jo_out));
    ASSERT(jo_out != NULL);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_media_object_by_name(
                                 jo_content, "text/html", &jo_out));
    ASSERT(jo_out != NULL);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_media_object_by_name(
                                 jo_content, "image/png", &jo_out));
    ASSERT(jo_out == NULL);

    json_value_free(jv_content);
  }

  /* 3. parse_response_object copying examples from primary media type */
  {
    const char *resp_ex_json =
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"paths\":{\"/"
        "test\":{\"get\":{\"responses\":{\"200\":{\"description\":"
        "\"OK\",\"content\":{\"application/json\":{\"schema\":{\"type\":"
        "\"string\"},\"examples\":{\"ex1\":{\"value\":\"v1\"},\"ex2\":{"
        "\"value\":"
        "\"v2\"}}}}}}}}}}";
    jv = json_parse_string(resp_ex_json);
    if (jv) {
      memset(&spec, 0, sizeof(spec));
      rc = openapi_load_from_json(jv, &spec);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, spec.n_paths);
      ASSERT_EQ(1, spec.paths[0].n_operations);
      ASSERT_EQ(1, spec.paths[0].operations[0].n_responses);
      ASSERT_EQ(2, spec.paths[0].operations[0].responses[0].n_examples);
      openapi_spec_free(&spec);
      json_value_free(jv);
    }
  }

  /* 4. parse_header_object and parse_parameter_object with itemSchema and media
     " "example */
  {
    const char *item_schema_json =
        "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"paths\":{\"/"
        "test\":{\"get\":{\"parameters\":[{\"name\":\"p1\",\"in\":"
        "\"query\",\"content\":{\"application/json\":{\"itemSchema\":{\"type\":"
        "\"integer\"},\"example\":42}}}],\"responses\":{\"200\":{"
        "\"description\":"
        "\"OK\",\"headers\":{\"X-Hdr\":{\"content\":{\"application/json\":{"
        "\"itemSchema\":{\"type\":\"string\"},\"example\":\"abc\"}}}}}}}}}}";
    jv = json_parse_string(item_schema_json);
    if (jv) {
      memset(&spec, 0, sizeof(spec));
      rc = openapi_load_from_json(jv, &spec);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, spec.n_paths);
      ASSERT_EQ(1, spec.paths[0].operations[0].n_parameters);
      ASSERT_EQ(1, spec.paths[0].operations[0].parameters[0].example_set);
      openapi_spec_free(&spec);
      json_value_free(jv);
    }
  }

  PASS();
}

TEST test_openapi_loader_swagger2_and_schema_ref_branches(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_SchemaRef sref;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  cdd_c_error_t rc;

  /* 1. cdd_test_parse_schema_ref edge cases */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_schema_ref(NULL, NULL, NULL));

  memset(&sref, 0, sizeof(sref));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_ref(NULL, &sref, NULL));

  jv = json_parse_string(
      "{\"$ref\":\"#/components/schemas/Bar\",\"summary\":\"Bar "
      "Sum\",\"description\":\"Bar Desc\",\"format\":\"custom\","
      "\"contentMediaType\":\"text/plain\",\"contentEncoding\":\"utf-8\","
      "\"externalDocs\":{\"url\":\"http://docs\"}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&sref, 0, sizeof(sref));
    rc = cdd_test_parse_schema_ref(jo, &sref, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("#/components/schemas/Bar", sref.ref);
    ASSERT_STR_EQ("Bar Sum", sref.summary);
    ASSERT_STR_EQ("Bar Desc", sref.description);
    ASSERT_STR_EQ("custom", sref.format);
    ASSERT_STR_EQ("text/plain", sref.content_media_type);
    ASSERT_STR_EQ("utf-8", sref.content_encoding);
    ASSERT_EQ(1, sref.external_docs_set);
    cdd_test_free_schema_ref_content(&sref);
    json_value_free(jv);
  }

  jv = json_parse_string(
      "{\"type\":\"array\",\"items\":{\"type\":\"string\",\"format\":\"byte\","
      "\"contentMediaType\":\"image/png\",\"contentEncoding\":\"base64\","
      "\"enum\":[\"a\"],\"examples\":[\"ex\"],\"const\":\"a\",\"default\":"
      "\"a\"}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&sref, 0, sizeof(sref));
    rc = cdd_test_parse_schema_ref(jo, &sref, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("byte", sref.items_format);
    ASSERT_STR_EQ("image/png", sref.items_content_media_type);
    ASSERT_STR_EQ("base64", sref.items_content_encoding);
    ASSERT_EQ(1, sref.n_items_enum_values);
    ASSERT_EQ(1, sref.n_items_examples);
    ASSERT_EQ(1, sref.items_const_value_set);
    ASSERT_EQ(1, sref.items_default_value_set);
    cdd_test_free_schema_ref_content(&sref);
    json_value_free(jv);
  }

  /* 2. Swagger 2.0 full spec */
  {
    const char *swagger2_json =
        "{\"swagger\":\"2.0\",\"info\":{\"title\":\"Swagger "
        "API\",\"version\":\"1.0\"},\"host\":\"api.example.com\",\"basePath\":"
        "\"/v1\",\"schemes\":[\"https\",\"http\"],\"consumes\":[\"application/"
        "json\"],\"produces\":[\"application/json\"],\"paths\":{\"/items\":{"
        "\"get\":{\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";
    jv = json_parse_string(swagger2_json);
    if (jv) {
      memset(&spec, 0, sizeof(spec));
      rc = openapi_load_from_json(jv, &spec);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_STR_EQ("2.0", spec.swagger_version);
      ASSERT_STR_EQ("api.example.com", spec.host);
      ASSERT_STR_EQ("/v1", spec.basePath);
      ASSERT_EQ(2, spec.n_schemes);
      ASSERT_EQ(1, spec.n_consumes);
      ASSERT_EQ(1, spec.n_produces);
      openapi_spec_free(&spec);
      json_value_free(jv);
    }
  }

  /* 3. Path item $ref with summary and description overrides */
  {
    const char *path_override_json =
        "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"components\":{\"pathItems\":{\"Item\":{\"summary\":\"Original "
        "Summary\",\"description\":\"Original "
        "Description\",\"get\":{\"responses\":{\"200\":{\"description\":"
        "\"OK\"}}}}}},\"paths\":{\"/items\":{\"$ref\":\"#/components/"
        "pathItems/Item\",\"summary\":\"Overridden "
        "Summary\",\"description\":\"Overridden Description\"}}}";
    jv = json_parse_string(path_override_json);
    if (jv) {
      memset(&spec, 0, sizeof(spec));
      rc = openapi_load_from_json(jv, &spec);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, spec.n_paths);
      ASSERT_STR_EQ("Overridden Summary", spec.paths[0].summary);
      ASSERT_STR_EQ("Overridden Description", spec.paths[0].description);
      openapi_spec_free(&spec);
      json_value_free(jv);
    }
  }

  /* 4. Root object invalid type for openapi_load_from_json */
  {
    JSON_Value *jv_int = json_parse_string("123");
    if (jv_int) {
      memset(&spec, 0, sizeof(spec));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                openapi_load_from_json(jv_int, &spec));
      json_value_free(jv_int);
    }
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_BRANCHES_H */
