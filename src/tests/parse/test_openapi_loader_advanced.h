/**
 * @file test_openapi_loader_advanced.h
 * @brief Advanced path item, security scheme, and swagger fallback tests.
 */

#ifndef TEST_OPENAPI_LOADER_ADVANCED_H
#define TEST_OPENAPI_LOADER_ADVANCED_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

TEST test_openapi_loader_helper_fallbacks(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme *sec_out = NULL;
  struct OpenAPI_MediaType *mt_out = NULL;
  struct OpenAPI_MediaType mt;
  char *buf_out = NULL;
  const char *chunks[1];

  memset(&spec, 0, sizeof(spec));
  memset(&mt, 0, sizeof(mt));
  chunks[0] = "a";

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, load_spec_str(NULL, &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            load_spec_str_with_context(NULL, NULL, NULL, &spec));

  ASSERT_EQ(-1, find_raw_schema_index(NULL, NULL));
  ASSERT_EQ(-1, find_raw_schema_index(&spec, "missing"));

  ASSERT_EQ(0, find_scheme(NULL, NULL, &sec_out));
  ASSERT_EQ(NULL, sec_out);
  ASSERT_EQ(0, find_scheme(&spec, "missing", &sec_out));
  ASSERT_EQ(NULL, sec_out);

  ASSERT_EQ(0, find_media_type(NULL, 0, NULL, &mt_out));
  ASSERT_EQ(NULL, mt_out);
  ASSERT_EQ(0, find_media_type(&mt, 1, "missing", &mt_out));
  ASSERT_EQ(NULL, mt_out);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, concat_chunks(NULL, 0, NULL));

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, concat_chunks(chunks, 1, &buf_out));
  g_cdd_alloc_fail = 0;

  PASS();
}

TEST test_load_comprehensive_path_item_and_schema_copying(void) {
  const char *chunks[] = {
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},",
      "\"components\":{",
      "\"securitySchemes\":{\"oauth2_sec\":{\"type\":\"oauth2\",\"flows\":{"
      "\"authorizationCode\":{"
      "\"authorizationUrl\":\"https://example.com/auth\",\"tokenUrl\":\"https:/"
      "/"
      "example.com/token\","
      "\"scopes\":{\"read:users\":\"Read\",\"write:users\":\"Write\"}}}}},",
      "\"schemas\":{",
      "\"Sub1\":{\"type\":\"string\"},",
      "\"RichSchema\":{",
      "\"type\":[\"string\",\"null\"],\"format\":\"date-time\",",
      "\"contentMediaType\":\"application/json\",\"contentEncoding\":\"base64\""
      ",",
      "\"contentSchema\":{\"type\":\"string\"},\"summary\":\"s\",\"description"
      "\":\"d\",",
      "\"deprecated\":true,\"readOnly\":true,\"writeOnly\":false,",
      "\"const\":\"fixed_val\",\"default\":\"def_val\",\"example\":\"ex_val\",",
      "\"examples\":[\"ex_val1\",\"ex_val2\"],\"enum\":[\"val1\",\"val2\"],",
      "\"externalDocs\":{\"url\":\"https://example.com/schema-docs\","
      "\"description\":\"sd\",\"x-ext\":\"val\"},",
      "\"discriminator\":{\"propertyName\":\"kind\",\"mapping\":{\"k1\":\"#/"
      "components/schemas/Sub1\"},\"defaultMapping\":\"k1\"},",
      "\"xml\":{\"name\":\"xmlNode\",\"namespace\":\"http://example.com/xml\","
      "\"prefix\":\"ex\",\"attribute\":true,\"wrapped\":true},",
      "\"multipleOf\":2,\"maximum\":100,\"exclusiveMaximum\":99,\"minimum\":10,"
      "\"exclusiveMinimum\":11,",
      "\"maxLength\":50,\"minLength\":2,\"pattern\":\"^[a-z]+$\",",
      "\"maxItems\":10,\"minItems\":1,\"uniqueItems\":true,\"maxContains\":5,"
      "\"minContains\":1,",
      "\"maxProperties\":10,\"minProperties\":1,\"dependentRequired\":{\"foo\":"
      "["
      "\"bar\"]},",
      "\"allOf\":[{\"type\":\"string\"}],\"anyOf\":[{\"type\":\"string\"}],"
      "\"oneOf\":[{\"type\":\"string\"}],",
      "\"not\":{\"type\":\"integer\"},\"if\":{\"type\":\"string\"},\"then\":{"
      "\"maxLength\":20},\"else\":{\"maxLength\":5},",
      "\"items\":{\"type\":\"string\",\"format\":\"email\",\"pattern\":\"^[a-z]"
      "+$"
      "\",\"example\":\"ex\",\"const\":\"c1\"}",
      "}},",
      "\"pathItems\":{\"P1\":{",
      "\"summary\":\"PSum\",\"description\":\"PDesc\",",
      "\"servers\":[{\"url\":\"https://api.example.com/v1\",\"description\":"
      "\"sd\",\"variables\":{\"env\":{\"default\":\"prod\",\"enum\":[\"prod\"]}"
      "}}],",
      "\"parameters\":[{\"name\":\"p_path\",\"in\":\"path\",\"required\":true,"
      "\"description\":\"pd\",\"deprecated\":true,\"style\":\"simple\","
      "\"explode\":false,\"schema\":{\"type\":\"string\"}}],",
      "\"get\":{",
      "\"operationId\":\"opGet\",\"summary\":\"s\",\"description\":\"d\","
      "\"deprecated\":true,",
      "\"tags\":[\"tag1\",\"tag2\"],\"externalDocs\":{\"url\":\"https://"
      "example.com/docs\",\"description\":\"dd\"},",
      "\"servers\":[{\"url\":\"https://op.example.com\"}],\"security\":[{"
      "\"oauth2_sec\":[\"read:users\"]}],",
      "\"parameters\":[{\"name\":\"q\",\"in\":\"query\",\"schema\":{\"$ref\":"
      "\"#/components/schemas/RichSchema\"}}],",
      "\"requestBody\":{\"description\":\"rbd\",\"required\":true,\"content\":{"
      "\"application/json\":{\"schema\":{\"$ref\":\"#/components/schemas/"
      "RichSchema\"},\"example\":{\"name\":\"test\"},\"encoding\":{\"prop1\":{"
      "\"contentType\":\"text/"
      "plain\",\"style\":\"form\",\"explode\":true}}}}},",
      "\"responses\":{",
      "\"200\":{\"description\":\"Success\",\"headers\":{\"X-RateLimit\":{"
      "\"deprecated\":true,\"schema\":{\"$ref\":\"#/components/schemas/"
      "RichSchema\"}}},\"content\":{\"application/json\":{\"schema\":{\"$ref\":"
      "\"#/components/schemas/RichSchema\"}}},\"links\":{\"LinkOp\":{"
      "\"operationId\":\"opOther\",\"description\":\"ld\"}}},",
      "\"default\":{\"description\":\"Default\",\"content\":{\"application/"
      "json\":{\"schema\":{\"type\":\"string\"}}}}",
      "},",
      "\"callbacks\":{\"onEvent\":{\"$request.query.queryUrl\":{\"post\":{"
      "\"operationId\":\"callbackPost\",\"responses\":{\"200\":{"
      "\"description\":"
      "\"ok\"}}}}}},",
      "\"x-custom-attr\":\"custom_val\"",
      "},",
      "\"additionalOperations\":[{\"method\":\"CHECK\",\"operationId\":"
      "\"opCheck"
      "\",\"responses\":{\"200\":{\"description\":\"ok\"}}}]",
      "}}",
      "},",
      "\"paths\":{\"/test/{p_path}\":{\"$ref\":\"#/components/pathItems/P1\"}}",
      "}"};
  char *json = NULL;
  struct OpenAPI_Spec spec = {0};
  int rc;
  cdd_c_error_t err;

  err = concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]), &json);
  ASSERT_EQ(CDD_C_SUCCESS, err);
  ASSERT(json != NULL);

  rc = load_spec_str(json, &spec);
  free(json);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(1, spec.paths[0].n_operations);
  openapi_spec_free(&spec);

  {
    int k;
    for (k = 1; k <= 150; ++k) {
      char *oom_json = NULL;
      struct OpenAPI_Spec oom_spec = {0};
      if (concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]),
                        &oom_json) == CDD_C_SUCCESS) {
        g_cdd_strdup_fail = k;
        load_spec_str(oom_json, &oom_spec);
        g_cdd_strdup_fail = 0;
        openapi_spec_free(&oom_spec);
        free(oom_json);
      }
    }
    for (k = 1; k <= 60; ++k) {
      char *oom_json = NULL;
      struct OpenAPI_Spec oom_spec = {0};
      if (concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]),
                        &oom_json) == CDD_C_SUCCESS) {
        g_cdd_alloc_fail = k;
        load_spec_str(oom_json, &oom_spec);
        g_cdd_alloc_fail = 0;
        openapi_spec_free(&oom_spec);
        free(oom_json);
      }
    }
  }

  g_fail_io_after = -1;
  PASS();
}

TEST test_load_components_ref_overrides_and_deep_copying(void) {
  const char *chunks[] = {
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},",
      "\"components\":{",
      "\"headers\":{\"H1\":{",
      "\"description\":\"HDesc\",\"deprecated\":true,\"required\":true,",
      "\"style\":\"simple\",\"explode\":false,\"schema\":{\"type\":\"string\"}"
      ",",
      "\"examples\":{\"e1\":{\"value\":\"hex2\"}}",
      "}},",
      "\"parameters\":{\"P1\":{",
      "\"name\":\"param1\",\"in\":\"query\",\"description\":\"PDesc\",",
      "\"required\":false,\"deprecated\":true,\"allowEmptyValue\":true,",
      "\"style\":\"form\",\"explode\":true,\"schema\":{\"type\":\"integer\"},",
      "\"examples\":{\"e1\":{\"value\":456}}",
      "}},",
      "\"requestBodies\":{\"RB1\":{",
      "\"description\":\"RBDesc\",\"required\":true,",
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"object\"},",
      "\"examples\":{\"ex1\":{\"value\":{\"a\":2}}},",
      "\"encoding\":{\"a\":{\"contentType\":\"text/plain\",\"style\":\"form\","
      "\"explode\":true}}}}",
      "}},",
      "\"responses\":{\"R1\":{",
      "\"description\":\"RDesc\",\"headers\":{\"X-RespHdr\":{\"$ref\":\"#/"
      "components/headers/H1\",\"description\":\"HOverride\"}},",
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"string\"}}},",
      "\"links\":{\"L1\":{\"operationId\":\"otherOp\",\"description\":"
      "\"LDesc\","
      "\"parameters\":{\"param1\":\"$response.body\"},\"requestBody\":\"$"
      "request.body\",",
      "\"server\":{\"url\":\"https://link.com\"}}}",
      "}},",
      "\"callbacks\":{\"CB1\":{",
      "\"{$request.body#/url}\":{",
      "\"post\":{\"operationId\":\"cbOp\",\"description\":\"cb "
      "desc\",\"responses\":{\"200\":{\"description\":\"ok\"}}}",
      "}",
      "}}",
      "},",
      "\"paths\":{\"/endpoint\":{",
      "\"get\":{",
      "\"operationId\":\"mainOp\",\"summary\":\"mSummary\",",
      "\"parameters\":[{\"$ref\":\"#/components/parameters/"
      "P1\",\"description\":"
      "\"POverride\"}],",
      "\"requestBody\":{\"$ref\":\"#/components/requestBodies/RB1\","
      "\"description\":\"RBOverride\"},",
      "\"responses\":{",
      "\"200\":{\"$ref\":\"#/components/responses/R1\",\"description\":"
      "\"ROverride\"},",
      "\"default\":{\"description\":\"defResp\",\"headers\":{\"X-DirectHdr\":{"
      "\"description\":\"direct\",",
      "\"deprecated\":false,\"required\":false,\"schema\":{\"type\":\"string\"}"
      "}}",
      "}",
      "},",
      "\"callbacks\":{\"cbRef\":{\"$ref\":\"#/components/callbacks/CB1\"}}",
      "}",
      "}}",
      "}"};
  char *json = NULL;
  struct OpenAPI_Spec spec = {0};
  int rc;
  cdd_c_error_t err;

  err = concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]), &json);
  ASSERT_EQ(CDD_C_SUCCESS, err);
  ASSERT(json != NULL);

  rc = load_spec_str(json, &spec);
  free(json);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(1, spec.paths[0].n_operations);
  openapi_spec_free(&spec);

  {
    int k;
    for (k = 1; k <= 100; ++k) {
      char *oom_json = NULL;
      struct OpenAPI_Spec oom_spec = {0};
      if (concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]),
                        &oom_json) == CDD_C_SUCCESS) {
        g_cdd_strdup_fail = k;
        load_spec_str(oom_json, &oom_spec);
        g_cdd_strdup_fail = 0;
        openapi_spec_free(&oom_spec);
        free(oom_json);
      }
    }
    for (k = 1; k <= 40; ++k) {
      char *oom_json = NULL;
      struct OpenAPI_Spec oom_spec = {0};
      if (concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]),
                        &oom_json) == CDD_C_SUCCESS) {
        g_cdd_alloc_fail = k;
        load_spec_str(oom_json, &oom_spec);
        g_cdd_alloc_fail = 0;
        openapi_spec_free(&oom_spec);
        free(oom_json);
      }
    }
  }

  g_fail_io_after = -1;
  PASS();
}

TEST test_load_all_security_scheme_types_and_flows(void) {
  const char *chunks[] = {
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},",
      "\"components\":{\"securitySchemes\":{",
      "\"sec_basic\":{\"type\":\"http\",\"scheme\":\"basic\",\"description\":"
      "\"Basic auth\"},",
      "\"sec_api_hdr\":{\"type\":\"apiKey\",\"in\":\"header\",\"name\":\"X-API-"
      "Key\",\"description\":\"Key in header\"},",
      "\"sec_api_query\":{\"type\":\"apiKey\",\"in\":\"query\",\"name\":\"api_"
      "key\"},",
      "\"sec_api_cookie\":{\"type\":\"apiKey\",\"in\":\"cookie\",\"name\":"
      "\"session_id\",\"deprecated\":true},",
      "\"sec_http_bearer\":{\"type\":\"http\",\"scheme\":\"bearer\","
      "\"bearerFormat\":\"JWT\"},",
      "\"sec_http_custom\":{\"type\":\"http\",\"scheme\":\"custom_scheme\"},",
      "\"sec_openid\":{\"type\":\"openIdConnect\",\"openIdConnectUrl\":"
      "\"https://auth.example.com/openid\"},",
      "\"sec_mutual\":{\"type\":\"mutualTLS\"},",
      "\"sec_oauth\":{",
      "\"type\":\"oauth2\",\"oauth2MetadataUrl\":\"https://example.com/oauth-"
      "meta\",\"description\":\"Full OAuth2\",",
      "\"flows\":{",
      "\"implicit\":{\"authorizationUrl\":\"https://example.com/auth\","
      "\"refreshUrl\":\"https://example.com/refresh\",\"scopes\":{\"read\":"
      "\"Read scope\"}},",
      "\"password\":{\"tokenUrl\":\"https://example.com/token\",\"scopes\":{"
      "\"admin\":\"Admin scope\"}},",
      "\"clientCredentials\":{\"tokenUrl\":\"https://example.com/token2\","
      "\"refreshUrl\":\"https://example.com/refresh2\",\"scopes\":{\"service\":"
      "\"Service scope\"}},",
      "\"authorizationCode\":{\"authorizationUrl\":\"https://example.com/"
      "auth2\",\"tokenUrl\":\"https://example.com/token3\",\"scopes\":{"
      "\"user\":\"User scope\"}},",
      "\"deviceAuthorization\":{\"deviceAuthorizationUrl\":\"https://"
      "example.com/device\",\"tokenUrl\":\"https://example.com/token4\","
      "\"scopes\":{\"device\":\"Device scope\"}}",
      "}}",
      "}},",
      "\"paths\":{\"/endpoint\":{\"get\":{\"operationId\":\"opSec\","
      "\"responses\":{\"200\":{\"description\":\"ok\"}}}}}",
      "}"};
  char *json = NULL;
  struct OpenAPI_Spec spec = {0};
  int rc;
  cdd_c_error_t err;

  err = concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]), &json);
  ASSERT_EQ(CDD_C_SUCCESS, err);
  ASSERT(json != NULL);

  rc = load_spec_str(json, &spec);
  free(json);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(9, spec.n_security_schemes);
  openapi_spec_free(&spec);

  {
    int k;
    for (k = 1; k <= 80; ++k) {
      char *oom_json = NULL;
      struct OpenAPI_Spec oom_spec = {0};
      if (concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]),
                        &oom_json) == CDD_C_SUCCESS) {
        g_cdd_strdup_fail = k;
        load_spec_str(oom_json, &oom_spec);
        g_cdd_strdup_fail = 0;
        openapi_spec_free(&oom_spec);
        free(oom_json);
      }
    }
    for (k = 1; k <= 35; ++k) {
      char *oom_json = NULL;
      struct OpenAPI_Spec oom_spec = {0};
      if (concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]),
                        &oom_json) == CDD_C_SUCCESS) {
        g_cdd_alloc_fail = k;
        load_spec_str(oom_json, &oom_spec);
        g_cdd_alloc_fail = 0;
        openapi_spec_free(&oom_spec);
        free(oom_json);
      }
    }
  }

  g_fail_io_after = -1;
  PASS();
}

TEST test_security_scheme_validation_errors(void) {
  const char *bad_specs[] = {
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "components\":{\"securitySchemes\":{\"s1\":{\"type\":\"apiKey\",\"name\":"
      "\"k\"}}},\"paths\":{}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "components\":{\"securitySchemes\":{\"s1\":{\"type\":\"apiKey\",\"in\":"
      "\"header\"}}},\"paths\":{}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "components\":{\"securitySchemes\":{\"s1\":{\"type\":\"http\"}}},\"paths"
      "\":{}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "components\":{\"securitySchemes\":{\"s1\":{\"type\":\"openIdConnect\"}}}"
      ",\"paths\":{}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "components\":{\"securitySchemes\":{\"s1\":{\"type\":\"oauth2\","
      "\"flows\":{}}}},\"paths\":{}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "components\":{\"securitySchemes\":{\"s1\":{\"type\":\"oauth2\","
      "\"flows\":{\"implicit\":{\"authorizationUrl\":\"https://a.com\"}}}}},\""
      "paths\":{}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "components\":{\"securitySchemes\":{\"s1\":{\"type\":\"oauth2\","
      "\"flows\":{\"implicit\":{\"scopes\":{}}}}}},\"paths\":{}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "components\":{\"securitySchemes\":{\"s1\":{\"type\":\"oauth2\","
      "\"flows\":{\"password\":{\"scopes\":{}}}}}},\"paths\":{}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "components\":{\"securitySchemes\":{\"s1\":{\"type\":\"oauth2\","
      "\"flows\":{\"clientCredentials\":{\"scopes\":{}}}}}},\"paths\":{}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "components\":{\"securitySchemes\":{\"s1\":{\"type\":\"oauth2\","
      "\"flows\":{\"authorizationCode\":{\"authorizationUrl\":\"https://a.com\""
      ",\"scopes\":{}}}}}},\"paths\":{}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "components\":{\"securitySchemes\":{\"s1\":{\"type\":\"oauth2\","
      "\"flows\":{\"deviceAuthorization\":{\"scopes\":{}}}}}},\"paths\":{}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "components\":{\"securitySchemes\":{\"s1\":{\"type\":\"unknown_scheme\"}"
      "}},\"paths\":{}}"};
  size_t i;
  for (i = 0; i < sizeof(bad_specs) / sizeof(bad_specs[0]); ++i) {
    struct OpenAPI_Spec spec = {0};
    int rc = load_spec_str(bad_specs[i], &spec);
    ASSERT(rc != 0);
    openapi_spec_free(&spec);
  }
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_header_schema_array_and_ref_propagation(void) {
  const char *chunks[] = {
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},",
      "\"components\":{\"schemas\":{\"Item\":{\"type\":\"string\"}}},",
      "\"paths\":{\"/endpoint\":{\"get\":{\"operationId\":\"opHdr\",",
      "\"responses\":{\"200\":{\"description\":\"ok\",",
      "\"headers\":{",
      "\"X-Arr-Inline\":{\"description\":\"d1\",\"schema\":{\"type\":\"array\","
      "\"items\":{\"type\":\"integer\"}}},",
      "\"X-Arr-Ref\":{\"description\":\"d2\",\"schema\":{\"type\":\"array\","
      "\"items\":{\"$ref\":\"#/components/schemas/Item\"}}},",
      "\"X-Obj-Ref\":{\"description\":\"d3\",\"schema\":{\"$ref\":\"#/"
      "components/schemas/Item\"}},",
      "\"X-Int\":{\"description\":\"d4\",\"schema\":{\"type\":\"integer\"}}",
      "}}}}}}",
      "}"};
  char *json = NULL;
  struct OpenAPI_Spec spec = {0};
  int rc;
  cdd_c_error_t err;

  err = concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]), &json);
  ASSERT_EQ(CDD_C_SUCCESS, err);
  ASSERT(json != NULL);

  rc = load_spec_str(json, &spec);
  free(json);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(1, spec.paths[0].n_operations);
  ASSERT_EQ(4, spec.paths[0].operations[0].responses[0].n_headers);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].headers[0].is_array);
  ASSERT_STR_EQ("array",
                spec.paths[0].operations[0].responses[0].headers[0].type);
  ASSERT_STR_EQ("integer",
                spec.paths[0].operations[0].responses[0].headers[0].items_type);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].headers[1].is_array);
  ASSERT_STR_EQ("Item",
                spec.paths[0].operations[0].responses[0].headers[1].items_type);
  ASSERT_EQ(0, spec.paths[0].operations[0].responses[0].headers[2].is_array);
  ASSERT_STR_EQ("Item",
                spec.paths[0].operations[0].responses[0].headers[2].type);
  ASSERT_EQ(0, spec.paths[0].operations[0].responses[0].headers[3].is_array);
  ASSERT_STR_EQ("integer",
                spec.paths[0].operations[0].responses[0].headers[3].type);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_uri_resolution_and_self_matching(void) {
  const char *chunks[] = {
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},",
      "\"$id\":\"../v2/other.json\",",
      "\"components\":{\"schemas\":{\"User\":{\"type\":\"object\"}}},",
      "\"paths\":{\"/endpoint\":{\"get\":{\"operationId\":\"opUri\",",
      "\"parameters\":[",
      "{\"name\":\"p1\",\"in\":\"query\",\"schema\":{\"$ref\":\"https://"
      "example.com/api/v2/other.json#/components/schemas/User\"}},",
      "{\"name\":\"p2\",\"in\":\"query\",\"schema\":{\"$ref\":\"./other.json#/"
      "components/schemas/User\"}},",
      "{\"name\":\"p3\",\"in\":\"query\",\"schema\":{\"$ref\":\"../v2/other."
      "json#/components/schemas/User\"}}",
      "],",
      "\"responses\":{\"200\":{\"description\":\"ok\"}}",
      "}}}}"};
  char *json = NULL;
  struct OpenAPI_Spec spec = {0};
  int rc;
  cdd_c_error_t err;

  err = concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]), &json);
  ASSERT_EQ(CDD_C_SUCCESS, err);
  ASSERT(json != NULL);

  rc = load_spec_str_with_context(
      json, "https://example.com/api/v1/openapi.json", NULL, &spec);
  free(json);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(3, spec.paths[0].operations[0].n_parameters);
  openapi_spec_free(&spec);

  {
    int k;
    for (k = 1; k <= 30; ++k) {
      char *oom_json = NULL;
      struct OpenAPI_Spec oom_spec = {0};
      if (concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]),
                        &oom_json) == CDD_C_SUCCESS) {
        g_cdd_strdup_fail = k;
        load_spec_str_with_context(oom_json,
                                   "https://example.com/api/v1/openapi.json",
                                   NULL, &oom_spec);
        g_cdd_strdup_fail = 0;
        openapi_spec_free(&oom_spec);
        free(oom_json);
      }
    }
    for (k = 1; k <= 30; ++k) {
      char *oom_json = NULL;
      struct OpenAPI_Spec oom_spec = {0};
      if (concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]),
                        &oom_json) == CDD_C_SUCCESS) {
        g_cdd_alloc_fail = k;
        load_spec_str_with_context(oom_json,
                                   "https://example.com/api/v1/openapi.json",
                                   NULL, &oom_spec);
        g_cdd_alloc_fail = 0;
        openapi_spec_free(&oom_spec);
        free(oom_json);
      }
    }
  }

  /* Test self_uri with fragment */
  {
    const char *self_chunks[] = {
        "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},",
        "\"self\":\"https://example.com/api/spec.json#frag\",",
        "\"components\":{\"schemas\":{\"User\":{\"type\":\"object\"}}},",
        "\"paths\":{\"/endpoint\":{\"get\":{\"operationId\":\"opSelf\",",
        "\"parameters\":[",
        ("{\"name\":\"p1\",\"in\":\"query\",\"schema\":{\"$ref\":\"https://"
         "example.com/api/spec.json#/components/schemas/User\"}}"),
        "],",
        "\"responses\":{\"200\":{\"description\":\"ok\"}}",
        "}}}}"};
    char *self_json = NULL;
    struct OpenAPI_Spec self_spec = {0};
    err = concat_chunks(
        self_chunks, sizeof(self_chunks) / sizeof(self_chunks[0]), &self_json);
    ASSERT_EQ(CDD_C_SUCCESS, err);
    rc = load_spec_str(self_json, &self_spec);
    free(self_json);
    ASSERT_EQ(0, rc);
    ASSERT_EQ(1, self_spec.n_paths);
    openapi_spec_free(&self_spec);
  }

  g_fail_io_after = -1;
  PASS();
}

TEST test_header_and_parameter_style_and_content_variations(void) {
  const char *chunks[] = {
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},",
      "\"components\":{",
      "\"mediaTypes\":{\"JsonMT\":{\"schema\":{\"type\":\"string\"}}},",
      "\"headers\":{\"H1\":{\"description\":\"H1Desc\",\"schema\":{\"type\":"
      "\"string\"}}},",
      "\"parameters\":{\"P1\":{\"name\":\"p1\",\"in\":\"query\","
      "\"description\":\"P1Desc\",\"schema\":{\"type\":\"string\"}}},",
      "\"requestBodies\":{\"RB1\":{\"description\":\"RB1Desc\",\"content\":{"
      "\"application/json\":{\"schema\":{\"type\":\"string\"}}}}},",
      "\"responses\":{\"R1\":{\"summary\":\"R1Sum\",\"description\":\"R1Desc\""
      ",\"content\":{\"application/json\":{\"schema\":{\"type\":\"string\"}}}}}"
      ",",
      "},",
      "\"paths\":{\"/endpoint/{matrix_param}/{label_param}\":{",
      "\"get\":{",
      "\"operationId\":\"opVariations\",",
      "\"parameters\":[",
      "{\"name\":\"matrix_param\",\"in\":\"path\",\"required\":true,\"style\":"
      "\"matrix\",\"explode\":true,\"schema\":{\"type\":\"string\"}},",
      "{\"name\":\"label_param\",\"in\":\"path\",\"required\":true,\"style\":"
      "\"label\",\"explode\":false,\"schema\":{\"type\":\"string\"}},",
      "{\"name\":\"q_form\",\"in\":\"query\",\"style\":\"form\",\"explode\":"
      "false,\"schema\":{\"type\":\"string\"}},",
      "{\"name\":\"q_space\",\"in\":\"query\",\"style\":\"spaceDelimited\","
      "\"explode\":false,\"schema\":{\"type\":\"array\",\"items\":{\"type\":"
      "\"string\"}}},",
      "{\"name\":\"q_pipe\",\"in\":\"query\",\"style\":\"pipeDelimited\","
      "\"explode\":false,\"schema\":{\"type\":\"array\",\"items\":{\"type\":"
      "\"string\"}}},",
      "{\"name\":\"q_deep\",\"in\":\"query\",\"style\":\"deepObject\","
      "\"explode\":true,\"schema\":{\"type\":\"object\"}},",
      "{\"name\":\"q_reserved\",\"in\":\"query\",\"allowReserved\":true,"
      "\"schema\":{\"type\":\"string\"}},",
      "{\"name\":\"q_content\",\"in\":\"query\",\"content\":{\"application/"
      "json\":{\"schema\":{\"type\":\"integer\"}}}},",
      "{\"name\":\"q_mt_ref\",\"in\":\"query\",\"content\":{\"application/"
      "json\":{\"$ref\":\"#/components/mediaTypes/JsonMT\"}}},",
      "{\"name\":\"q_bool_true\",\"in\":\"query\",\"schema\":true},",
      "{\"name\":\"q_bool_false\",\"in\":\"query\",\"schema\":false},",
      "{\"$ref\":\"#/components/parameters/P1\",\"description\":\"POverride\"}",
      "],",
      "\"requestBody\":{\"$ref\":\"#/components/requestBodies/RB1\","
      "\"description\":\"RBOverride\"},",
      "\"responses\":{",
      "\"200\":{",
      "\"summary\":\"SuccessResp\",",
      "\"description\":\"ok\",",
      "\"headers\":{",
      "\"X-Content-Hdr\":{\"content\":{\"application/json\":{\"schema\":{"
      "\"type\":\"string\"}}}},",
      "\"X-MT-Ref\":{\"content\":{\"application/json\":{\"$ref\":\"#/"
      "components/mediaTypes/JsonMT\"}}},",
      "\"X-Bool-True\":{\"schema\":true},",
      "\"X-Bool-False\":{\"schema\":false},",
      "\"X-Ref\":{\"$ref\":\"#/components/headers/H1\",\"description\":"
      "\"HOverride\"}",
      "},",
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"string\"}}}",
      "},",
      "\"201\":{\"$ref\":\"#/components/responses/R1\",\"summary\":"
      "\"ROverrideSum\",\"description\":\"ROverrideDesc\"}",
      "}}}}}",
  };
  char *json = NULL;
  struct OpenAPI_Spec spec = {0};
  int rc;
  cdd_c_error_t err;

  err = concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]), &json);
  ASSERT_EQ(CDD_C_SUCCESS, err);
  ASSERT(json != NULL);

  rc = load_spec_str(json, &spec);
  free(json);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(1, spec.paths[0].n_operations);
  openapi_spec_free(&spec);

  {
    int k;
    for (k = 1; k <= 30; ++k) {
      char *oom_json = NULL;
      struct OpenAPI_Spec oom_spec = {0};
      if (concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]),
                        &oom_json) == CDD_C_SUCCESS) {
        g_cdd_strdup_fail = k;
        load_spec_str(oom_json, &oom_spec);
        g_cdd_strdup_fail = 0;
        openapi_spec_free(&oom_spec);
        free(oom_json);
      }
    }
  }

  g_fail_io_after = -1;
  PASS();
}

TEST test_header_and_parameter_validation_errors(void) {
  const char *bad_specs[] = {
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/endpoint\":{\"get\":{\"responses\":{\"200\":{\"description"
      "\":\"ok\",\"headers\":{\"X-H\":{\"style\":\"form\"}}}}}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/endpoint\":{\"get\":{\"responses\":{\"200\":{\"description"
      "\":\"ok\",\"headers\":{\"X-H\":{\"schema\":{\"type\":\"string\"},\""
      "content\":{\"application/json\":{\"schema\":{\"type\":\"string\"}}}}}}}"
      "}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/endpoint\":{\"get\":{\"responses\":{\"200\":{\"description"
      "\":\"ok\",\"headers\":{\"X-H\":{\"content\":{\"application/json\":{\""
      "schema\":{\"type\":\"string\"}},\"text/plain\":{\"schema\":{\"type\":\""
      "string\"}}}}}}}}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/endpoint\":{\"get\":{\"parameters\":[{\"name\":\"p\",\"in\":"
      "\"query\",\"schema\":{\"type\":\"string\"},\"content\":{\"application/"
      "json\":{\"schema\":{\"type\":\"string\"}}}}],\"responses\":{\"200\":{\""
      "description\":\"ok\"}}}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/endpoint\":{\"get\":{\"parameters\":[{\"name\":\"p\",\"in\":"
      "\"query\",\"content\":{\"application/json\":{\"schema\":{\"type\":\""
      "string\"}},\"text/plain\":{\"schema\":{\"type\":\"string\"}}}}],\""
      "responses\":{\"200\":{\"description\":\"ok\"}}}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/endpoint\":{\"get\":{\"parameters\":[{\"name\":\"p\",\"in\":"
      "\"cookie\",\"style\":\"simple\"}],\"responses\":{\"200\":{\"description"
      "\":\"ok\"}}}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/endpoint\":{\"get\":{\"requestBody\":{\"content\":{}},\""
      "responses\":{\"200\":{\"description\":\"ok\"}}}}}}"};
  size_t i;
  for (i = 0; i < sizeof(bad_specs) / sizeof(bad_specs[0]); ++i) {
    struct OpenAPI_Spec spec = {0};
    int rc = load_spec_str(bad_specs[i], &spec);
    ASSERT(rc != 0);
    openapi_spec_free(&spec);
  }
  g_fail_io_after = -1;
  PASS();
}

TEST test_swagger2_parameter_and_response_type_fallbacks(void) {
  const char *chunks[] = {
      "{\"swagger\":\"2.0\",\"info\":{\"title\":\"s2\",\"version\":\"1\"},",
      "\"paths\":{\"/endpoint\":{",
      "\"get\":{",
      "\"operationId\":\"opSwagger2\",",
      "\"parameters\":[",
      ("{\"name\":\"p_type\",\"in\":\"query\",\"type\":\"string\",\"format\":"
       "\"byte\"}"),
      "],",
      "\"responses\":{",
      "\"200\":{",
      "\"description\":\"ok\",",
      "\"schema\":{\"type\":\"string\"},",
      "\"headers\":{",
      "\"X-Hdr-Type\":{\"type\":\"string\",\"format\":\"uuid\"}",
      "}}",
      "}}}}}"};
  char *json = NULL;
  struct OpenAPI_Spec spec = {0};
  int rc;
  cdd_c_error_t err;

  err = concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]), &json);
  ASSERT_EQ(CDD_C_SUCCESS, err);
  ASSERT(json != NULL);

  rc = load_spec_str(json, &spec);
  free(json);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(1, spec.paths[0].n_operations);
  ASSERT_EQ(1, spec.paths[0].operations[0].n_parameters);
  ASSERT_EQ(1, spec.paths[0].operations[0].n_responses);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_webhooks_and_component_callbacks(void) {
  const char *chunks[] = {
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},",
      "\"paths\":{\"/"
      "endpoint\":{\"get\":{\"operationId\":\"opP\",\"responses\":"
      "{\"200\":{\"description\":\"ok\"}}}}},",
      "\"webhooks\":{",
      "\"newPet\":{",
      "\"post\":{",
      "\"operationId\":\"opWebhookPet\",",
      "\"requestBody\":{\"description\":\"b\",\"required\":true,\"content\":{\""
      "application/json\":{\"schema\":{\"type\":\"string\"}}}},",
      "\"responses\":{\"200\":{\"description\":\"ok\"}}",
      "},",
      "\"servers\":[{\"url\":\"https://wh.example.com\"}],",
      "\"parameters\":[{\"name\":\"X-Wh-Hdr\",\"in\":\"header\",\"schema\":{\""
      "type\":\"string\"}}]",
      "},",
      "\"x-webhooks-custom\":\"wh_val\"",
      "}}"};
  char *json = NULL;
  struct OpenAPI_Spec spec = {0};
  int rc;
  cdd_c_error_t err;

  err = concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]), &json);
  ASSERT_EQ(CDD_C_SUCCESS, err);
  ASSERT(json != NULL);

  rc = load_spec_str(json, &spec);
  free(json);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(1, spec.n_webhooks);
  ASSERT_STR_EQ("newPet", spec.webhooks[0].route);
  ASSERT_EQ(1, spec.webhooks[0].n_operations);
  ASSERT_STR_EQ("opWebhookPet", spec.webhooks[0].operations[0].operation_id);
  openapi_spec_free(&spec);

  {
    int k;
    for (k = 1; k <= 20; ++k) {
      char *oom_json = NULL;
      struct OpenAPI_Spec oom_spec = {0};
      if (concat_chunks(chunks, sizeof(chunks) / sizeof(chunks[0]),
                        &oom_json) == CDD_C_SUCCESS) {
        g_cdd_strdup_fail = k;
        load_spec_str(oom_json, &oom_spec);
        g_cdd_strdup_fail = 0;
        openapi_spec_free(&oom_spec);
        free(oom_json);
      }
    }
  }

  g_fail_io_after = -1;
  PASS();
}

TEST test_path_template_validation_and_collision_errors(void) {
  const char *bad_paths[] = {
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/users\":{\"parameters\":[{\"name\":\"id\",\"in\":\"path\","
      "\"required\":true,\"schema\":{\"type\":\"string\"}}],\"get\":{\""
      "responses\":{\"200\":{\"description\":\"ok\"}}}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/users/{id}\":{\"get\":{\"parameters\":[{\"name\":\"id\","
      "\"in\":\"path\",\"required\":false,\"schema\":{\"type\":\"string\"}}],"
      "\"responses\":{\"200\":{\"description\":\"ok\"}}}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/users/{id}\":{\"get\":{\"responses\":{\"200\":{\""
      "description\":\"ok\"}}}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/users/{id}/{id}\":{\"get\":{\"parameters\":[{\"name\":\"id"
      "\",\"in\":\"path\",\"required\":true,\"schema\":{\"type\":\"string\"}}"
      "],\"responses\":{\"200\":{\"description\":\"ok\"}}}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/users/{id\":{\"get\":{\"responses\":{\"200\":{\""
      "description\":\"ok\"}}}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/users/{}\":{\"get\":{\"responses\":{\"200\":{\""
      "description\":\"ok\"}}}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/users/{id}\":{\"get\":{\"parameters\":[{\"name\":\"id\","
      "\"in\":\"path\",\"required\":true,\"schema\":{\"type\":\"string\"}}],"
      "\"responses\":{\"200\":{\"description\":\"ok\"}}}},\"/users/{userId}\":"
      "{\"get\":{\"parameters\":[{\"name\":\"userId\",\"in\":\"path\","
      "\"required\":true,\"schema\":{\"type\":\"string\"}}],\"responses\":{"
      "\"200\":{\"description\":\"ok\"}}}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/p1\":{\"get\":{\"operationId\":\"dupOp\",\"responses\":{"
      "\"200\":{\"description\":\"ok\"}}}},\"/p2\":{\"get\":{\"operationId\":"
      "\"dupOp\",\"responses\":{\"200\":{\"description\":\"ok\"}}}}}}",
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\""
      "paths\":{\"/p1\":{\"get\":{\"operationId\":\"dupWhOp\",\"responses\":{"
      "\"200\":{\"description\":\"ok\"}}}}},\"webhooks\":{\"wh\":{\"post\":{\""
      "operationId\":\"dupWhOp\",\"responses\":{\"200\":{\"description\":\"ok\""
      "}}}}}}"};
  size_t i;
  for (i = 0; i < sizeof(bad_paths) / sizeof(bad_paths[0]); ++i) {
    struct OpenAPI_Spec spec = {0};
    int rc = load_spec_str(bad_paths[i], &spec);
    ASSERT(rc != 0);
    openapi_spec_free(&spec);
  }
  g_fail_io_after = -1;
  PASS();
}

/* Static and unit test coverage for openapi.c remaining branches */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_ADVANCED_H */
