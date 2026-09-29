/**
 * @file test_openapi_loader_security.h
 * @brief Security requirements, security schemes, OAuth2, and example tests.
 */

#ifndef TEST_OPENAPI_LOADER_SECURITY_H
#define TEST_OPENAPI_LOADER_SECURITY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

TEST test_load_server_url_query_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"servers\":[{\"url\":\"https://example.com/api?"
      "q=1\"}],\"paths\":{}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_security_requirements(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  115, 101, 99,  117, 114, 105, 116, 121, 34,  58,
      91,  123, 34,  65,  112, 105, 75,  101, 121, 65,  117, 116, 104, 34,  58,
      91,  93,  125, 44,  123, 34,  98,  101, 97,  114, 101, 114, 65,  117, 116,
      104, 34,  58,  91,  34,  114, 101, 97,  100, 58,  112, 101, 116, 115, 34,
      93,  125, 93,  44,  34,  112, 97,  116, 104, 115, 34,  58,  123, 34,  47,
      112, 101, 116, 115, 34,  58,  123, 34,  103, 101, 116, 34,  58,  123, 34,
      111, 112, 101, 114, 97,  116, 105, 111, 110, 73,  100, 34,  58,  34,  108,
      105, 115, 116, 80,  101, 116, 115, 34,  44,  34,  115, 101, 99,  117, 114,
      105, 116, 121, 34,  58,  91,  123, 125, 93,  44,  34,  114, 101, 115, 112,
      111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123,
      34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,
      79,  75,  34,  125, 125, 125, 125, 125, 44,  34,  99,  111, 109, 112, 111,
      110, 101, 110, 116, 115, 34,  58,  123, 34,  115, 101, 99,  117, 114, 105,
      116, 121, 83,  99,  104, 101, 109, 101, 115, 34,  58,  123, 34,  65,  112,
      105, 75,  101, 121, 65,  117, 116, 104, 34,  58,  123, 34,  116, 121, 112,
      101, 34,  58,  34,  97,  112, 105, 75,  101, 121, 34,  44,  34,  105, 110,
      34,  58,  34,  104, 101, 97,  100, 101, 114, 34,  44,  34,  110, 97,  109,
      101, 34,  58,  34,  88,  45,  65,  112, 105, 34,  125, 44,  34,  98,  101,
      97,  114, 101, 114, 65,  117, 116, 104, 34,  58,  123, 34,  116, 121, 112,
      101, 34,  58,  34,  104, 116, 116, 112, 34,  44,  34,  115, 99,  104, 101,
      109, 101, 34,  58,  34,  98,  101, 97,  114, 101, 114, 34,  125, 125, 125,
      125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.security_set);
  ASSERT_EQ(2, spec.n_security);
  ASSERT_EQ(1, spec.security[0].n_requirements);
  ASSERT_STR_EQ("ApiKeyAuth", spec.security[0].requirements[0].scheme);
  ASSERT_EQ(0, spec.security[0].requirements[0].n_scopes);
  ASSERT_EQ(1, spec.security[1].requirements[0].n_scopes);
  ASSERT_STR_EQ("read:pets", spec.security[1].requirements[0].scopes[0]);

  {
    struct OpenAPI_Operation *op = &spec.paths[0].operations[0];
    ASSERT_EQ(1, op->security_set);
    ASSERT_EQ(1, op->n_security);
    ASSERT_EQ(0, op->security[0].n_requirements); /* empty object */
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_security_schemes(void) {
  struct OpenAPI_SecurityScheme *_ast_find_scheme_0;
  struct OpenAPI_SecurityScheme *_ast_find_scheme_1;
  struct OpenAPI_SecurityScheme *_ast_find_scheme_2;

  const char *json =
      "{\"components\":{\"securitySchemes\":{\"bearerAuth\":{\"type\":\"http\","
      "\"scheme\":\"bearer\",\"bearerFormat\":\"JWT\"},\"apiKeyAuth\":{"
      "\"type\":\"apiKey\",\"in\":\"header\",\"name\":\"X-Api-Key\"},"
      "\"mtlsAuth\":{\"type\":\"mutualTLS\",\"description\":\"mTLS "
      "only\"}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  const struct OpenAPI_SecurityScheme *bearer;
  const struct OpenAPI_SecurityScheme *apikey;
  const struct OpenAPI_SecurityScheme *mtls;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(3, spec.n_security_schemes);

  bearer = (find_scheme(&spec, "bearerAuth", &_ast_find_scheme_0),
            _ast_find_scheme_0);
  ASSERT(bearer != NULL);
  ASSERT_EQ(OA_SEC_HTTP, bearer->type);
  ASSERT_STR_EQ("bearer", bearer->scheme);
  ASSERT_STR_EQ("JWT", bearer->bearer_format);

  apikey = (find_scheme(&spec, "apiKeyAuth", &_ast_find_scheme_1),
            _ast_find_scheme_1);
  ASSERT(apikey != NULL);
  ASSERT_EQ(OA_SEC_APIKEY, apikey->type);
  ASSERT_EQ(OA_SEC_IN_HEADER, apikey->in);
  ASSERT_STR_EQ("X-Api-Key", apikey->key_name);

  mtls =
      (find_scheme(&spec, "mtlsAuth", &_ast_find_scheme_2), _ast_find_scheme_2);
  ASSERT(mtls != NULL);
  ASSERT_EQ(OA_SEC_MUTUALTLS, mtls->type);
  ASSERT_STR_EQ("mTLS only", mtls->description);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_security_scheme_deprecated(void) {
  struct OpenAPI_SecurityScheme *_ast_find_scheme_3;

  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"oldKey\":{\"type\":\"apiKey\",\"in\":\"header\","
                     "\"name\":\"X-Old\",\"deprecated\":true}"
                     "}}}";

  struct OpenAPI_Spec spec = {0};
  const struct OpenAPI_SecurityScheme *old_key;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  old_key =
      (find_scheme(&spec, "oldKey", &_ast_find_scheme_3), _ast_find_scheme_3);
  ASSERT(old_key != NULL);
  ASSERT_EQ(1, old_key->deprecated_set);
  ASSERT_EQ(1, old_key->deprecated);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_oauth2_flows(void) {
  struct OpenAPI_SecurityScheme *_ast_find_scheme_4;

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  99,  111, 109, 112, 111,
      110, 101, 110, 116, 115, 34,  58,  123, 34,  115, 101, 99,  117, 114, 105,
      116, 121, 83,  99,  104, 101, 109, 101, 115, 34,  58,  123, 34,  111, 97,
      117, 116, 104, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111,
      97,  117, 116, 104, 50,  34,  44,  34,  102, 108, 111, 119, 115, 34,  58,
      123, 34,  97,  117, 116, 104, 111, 114, 105, 122, 97,  116, 105, 111, 110,
      67,  111, 100, 101, 34,  58,  123, 34,  97,  117, 116, 104, 111, 114, 105,
      122, 97,  116, 105, 111, 110, 85,  114, 108, 34,  58,  34,  104, 116, 116,
      112, 115, 58,  47,  47,  97,  117, 116, 104, 46,  101, 120, 97,  109, 112,
      108, 101, 46,  99,  111, 109, 34,  44,  34,  116, 111, 107, 101, 110, 85,
      114, 108, 34,  58,  34,  104, 116, 116, 112, 115, 58,  47,  47,  116, 111,
      107, 101, 110, 46,  101, 120, 97,  109, 112, 108, 101, 46,  99,  111, 109,
      34,  44,  34,  114, 101, 102, 114, 101, 115, 104, 85,  114, 108, 34,  58,
      34,  104, 116, 116, 112, 115, 58,  47,  47,  114, 101, 102, 114, 101, 115,
      104, 46,  101, 120, 97,  109, 112, 108, 101, 46,  99,  111, 109, 34,  44,
      34,  115, 99,  111, 112, 101, 115, 34,  58,  123, 34,  114, 101, 97,  100,
      34,  58,  34,  82,  101, 97,  100, 32,  97,  99,  99,  101, 115, 115, 34,
      125, 125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  const struct OpenAPI_SecurityScheme *oauth;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  oauth =
      (find_scheme(&spec, "oauth", &_ast_find_scheme_4), _ast_find_scheme_4);
  ASSERT(oauth != NULL);
  ASSERT_EQ(OA_SEC_OAUTH2, oauth->type);
  ASSERT_EQ(1, oauth->n_flows);
  ASSERT_EQ(OA_OAUTH_FLOW_AUTHORIZATION_CODE, oauth->flows[0].type);
  ASSERT_STR_EQ("https://auth.example.com", oauth->flows[0].authorization_url);
  ASSERT_STR_EQ("https://token.example.com", oauth->flows[0].token_url);
  ASSERT_STR_EQ("https://refresh.example.com", oauth->flows[0].refresh_url);
  ASSERT_EQ(1, oauth->flows[0].n_scopes);
  ASSERT_STR_EQ("read", oauth->flows[0].scopes[0].name);
  ASSERT_STR_EQ("Read access", oauth->flows[0].scopes[0].description);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_security_scheme_http_missing_scheme_rejected(void) {

  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"bad\":{\"type\":\"http\"}"
                     "}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_security_scheme_apikey_missing_name_rejected(void) {

  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"bad\":{\"type\":\"apiKey\",\"in\":\"header\"}"
                     "}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_security_scheme_apikey_missing_in_rejected(void) {

  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"bad\":{\"type\":\"apiKey\",\"name\":\"X-Api\"}"
                     "}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_security_scheme_openid_missing_url_rejected(void) {

  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"bad\":{\"type\":\"openIdConnect\"}"
                     "}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_oauth2_missing_flows_rejected(void) {

  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"oauth\":{\"type\":\"oauth2\"}"
                     "}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_oauth2_flow_missing_scopes_rejected(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  99,  111, 109, 112, 111,
      110, 101, 110, 116, 115, 34,  58,  123, 34,  115, 101, 99,  117, 114, 105,
      116, 121, 83,  99,  104, 101, 109, 101, 115, 34,  58,  123, 34,  111, 97,
      117, 116, 104, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111,
      97,  117, 116, 104, 50,  34,  44,  34,  102, 108, 111, 119, 115, 34,  58,
      123, 34,  97,  117, 116, 104, 111, 114, 105, 122, 97,  116, 105, 111, 110,
      67,  111, 100, 101, 34,  58,  123, 34,  97,  117, 116, 104, 111, 114, 105,
      122, 97,  116, 105, 111, 110, 85,  114, 108, 34,  58,  34,  104, 116, 116,
      112, 115, 58,  47,  47,  97,  117, 116, 104, 46,  101, 120, 97,  109, 112,
      108, 101, 46,  99,  111, 109, 34,  44,  34,  116, 111, 107, 101, 110, 85,
      114, 108, 34,  58,  34,  104, 116, 116, 112, 115, 58,  47,  47,  116, 111,
      107, 101, 110, 46,  101, 120, 97,  109, 112, 108, 101, 46,  99,  111, 109,
      34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_oauth2_flow_missing_required_urls_rejected(void) {

  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"oauth\":{\"type\":\"oauth2\",\"flows\":{"
                     "\"deviceAuthorization\":{"
                     "\"tokenUrl\":\"https://token.example.com\","
                     "\"scopes\":{}}"
                     "}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_oauth2_flow_unknown_rejected(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  99,  111, 109, 112, 111,
      110, 101, 110, 116, 115, 34,  58,  123, 34,  115, 101, 99,  117, 114, 105,
      116, 121, 83,  99,  104, 101, 109, 101, 115, 34,  58,  123, 34,  111, 97,
      117, 116, 104, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111,
      97,  117, 116, 104, 50,  34,  44,  34,  102, 108, 111, 119, 115, 34,  58,
      123, 34,  99,  117, 115, 116, 111, 109, 70,  108, 111, 119, 34,  58,  123,
      34,  97,  117, 116, 104, 111, 114, 105, 122, 97,  116, 105, 111, 110, 85,
      114, 108, 34,  58,  34,  104, 116, 116, 112, 115, 58,  47,  47,  97,  117,
      116, 104, 46,  101, 120, 97,  109, 112, 108, 101, 46,  99,  111, 109, 34,
      44,  34,  116, 111, 107, 101, 110, 85,  114, 108, 34,  58,  34,  104, 116,
      116, 112, 115, 58,  47,  47,  116, 111, 107, 101, 110, 46,  101, 120, 97,
      109, 112, 108, 101, 46,  99,  111, 109, 34,  44,  34,  115, 99,  111, 112,
      101, 115, 34,  58,  123, 125, 125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_parameter_examples_object(void) {

  const char *json =
      "{\"paths\":{\"/"
      "q\":{\"get\":{\"parameters\":[{\"name\":\"q\",\"in\":\"query\","
      "\"schema\":{\"type\":\"string\"},\"examples\":{\"basic\":{\"summary\":"
      "\"Basic\",\"dataValue\":\"hello\"}}}],\"responses\":{\"200\":{"
      "\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->n_examples);
    ASSERT_EQ(OA_EXAMPLE_LOC_OBJECT, p->example_location);
    ASSERT_STR_EQ("basic", p->examples[0].name);
    ASSERT_STR_EQ("Basic", p->examples[0].summary);
    ASSERT_EQ(1, p->examples[0].data_value_set);
    ASSERT_EQ(OA_ANY_STRING, p->examples[0].data_value.type);
    ASSERT_STR_EQ("hello", p->examples[0].data_value.string);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_parameter_examples_media(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  113, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,
      123, 34,  110, 97,  109, 101, 34,  58,  34,  113, 34,  44,  34,  105, 110,
      34,  58,  34,  113, 117, 101, 114, 121, 34,  44,  34,  99,  111, 110, 116,
      101, 110, 116, 34,  58,  123, 34,  97,  112, 112, 108, 105, 99,  97,  116,
      105, 111, 110, 47,  106, 115, 111, 110, 34,  58,  123, 34,  115, 99,  104,
      101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115,
      116, 114, 105, 110, 103, 34,  125, 44,  34,  101, 120, 97,  109, 112, 108,
      101, 115, 34,  58,  123, 34,  109, 34,  58,  123, 34,  115, 101, 114, 105,
      97,  108, 105, 122, 101, 100, 86,  97,  108, 117, 101, 34,  58,  34,  92,
      34,  104, 105, 92,  34,  34,  125, 125, 125, 125, 125, 93,  44,  34,  114,
      101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,
      34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110,
      34,  58,  34,  79,  75,  34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(OA_EXAMPLE_LOC_MEDIA, p->example_location);
    ASSERT_EQ(1, p->n_examples);
    ASSERT_STR_EQ("m", p->examples[0].name);
    ASSERT_STR_EQ("\"hi\"", p->examples[0].serialized_value);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_parameter_example_and_examples_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"q\",\"in\":\"query\","
      "\"schema\":{\"type\":\"string\"},"
      "\"example\":\"a\","
      "\"examples\":{\"ex\":{\"value\":\"b\"}}"
      "}],\"responses\":{\"200\":{\"description\":\"ok\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_header_example_and_examples_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"ok\","
      "\"headers\":{\"X-Test\":{"
      "\"schema\":{\"type\":\"string\"},"
      "\"example\":\"a\","
      "\"examples\":{\"ex\":{\"value\":\"b\"}}"
      "}}}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_media_example_and_examples_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"ok\","
      "\"content\":{\"application/json\":{"
      "\"schema\":{\"type\":\"string\"},"
      "\"example\":\"a\","
      "\"examples\":{\"ex\":{\"value\":\"b\"}}"
      "}}}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_example_data_value_and_value_rejected(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  84,  34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  113, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,
      123, 34,  110, 97,  109, 101, 34,  58,  34,  113, 34,  44,  34,  105, 110,
      34,  58,  34,  113, 117, 101, 114, 121, 34,  44,  34,  115, 99,  104, 101,
      109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116,
      114, 105, 110, 103, 34,  125, 44,  34,  101, 120, 97,  109, 112, 108, 101,
      115, 34,  58,  123, 34,  98,  97,  100, 34,  58,  123, 34,  100, 97,  116,
      97,  86,  97,  108, 117, 101, 34,  58,  34,  97,  34,  44,  34,  118, 97,
      108, 117, 101, 34,  58,  34,  98,  34,  125, 125, 125, 93,  44,  34,  114,
      101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,
      34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110,
      34,  58,  34,  79,  75,  34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_example_serialized_and_external_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "q\":{\"get\":{\"parameters\":[{\"name\":\"q\",\"in\":\"query\","
      "\"schema\":{\"type\":\"string\"},\"examples\":{\"bad\":{"
      "\"serializedValue\":\"x\",\"externalValue\":\"http://example.com/"
      "ex\"}}}],\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_response_examples_media(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"ok\","
      "\"content\":{\"application/json\":{"
      "\"example\":{\"id\":1}"
      "}}"
      "}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Response *resp = &spec.paths[0].operations[0].responses[0];
    ASSERT_EQ(1, resp->example_set);
    ASSERT_EQ(OA_ANY_JSON, resp->example.type);
    ASSERT(resp->example.json != NULL);
    ASSERT(strstr(resp->example.json, "\"id\"") != NULL);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_component_examples(void) {

  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"examples\":{"
                     "\"ex1\":{\"summary\":\"One\",\"value\":\"v\"}"
                     "}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_examples);
  ASSERT_STR_EQ("ex1", spec.component_example_names[0]);
  ASSERT_STR_EQ("One", spec.component_examples[0].summary);
  ASSERT_EQ(1, spec.component_examples[0].value_set);
  ASSERT_EQ(OA_ANY_STRING, spec.component_examples[0].value.type);
  ASSERT_STR_EQ("v", spec.component_examples[0].value.string);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_example_component_ref_strict(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  36,  115, 101, 108, 102, 34,  58,  34,  104, 116,
      116, 112, 115, 58,  47,  47,  101, 120, 97,  109, 112, 108, 101, 46,  99,
      111, 109, 47,  115, 112, 101, 99,  46,  106, 115, 111, 110, 34,  44,  34,
      105, 110, 102, 111, 34,  58,  123, 34,  116, 105, 116, 108, 101, 34,  58,
      34,  84,  34,  44,  34,  118, 101, 114, 115, 105, 111, 110, 34,  58,  34,
      49,  34,  125, 44,  34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115,
      34,  58,  123, 34,  101, 120, 97,  109, 112, 108, 101, 115, 34,  58,  123,
      34,  69,  120, 34,  58,  123, 34,  115, 117, 109, 109, 97,  114, 121, 34,
      58,  34,  82,  105, 103, 104, 116, 34,  44,  34,  118, 97,  108, 117, 101,
      34,  58,  34,  111, 107, 34,  125, 44,  34,  102, 111, 111, 34,  58,  123,
      34,  115, 117, 109, 109, 97,  114, 121, 34,  58,  34,  87,  114, 111, 110,
      103, 34,  44,  34,  118, 97,  108, 117, 101, 34,  58,  34,  98,  97,  100,
      34,  125, 125, 125, 44,  34,  112, 97,  116, 104, 115, 34,  58,  123, 34,
      47,  113, 34,  58,  123, 34,  103, 101, 116, 34,  58,  123, 34,  112, 97,
      114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,  123, 34,  110, 97,
      109, 101, 34,  58,  34,  113, 34,  44,  34,  105, 110, 34,  58,  34,  113,
      117, 101, 114, 121, 34,  44,  34,  115, 99,  104, 101, 109, 97,  34,  58,
      123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103,
      34,  125, 44,  34,  101, 120, 97,  109, 112, 108, 101, 115, 34,  58,  123,
      34,  103, 111, 111, 100, 34,  58,  123, 34,  36,  114, 101, 102, 34,  58,
      34,  104, 116, 116, 112, 115, 58,  47,  47,  101, 120, 97,  109, 112, 108,
      101, 46,  99,  111, 109, 47,  115, 112, 101, 99,  46,  106, 115, 111, 110,
      35,  47,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 47,  101, 120,
      97,  109, 112, 108, 101, 115, 47,  69,  120, 34,  125, 44,  34,  98,  97,
      100, 34,  58,  123, 34,  36,  114, 101, 102, 34,  58,  34,  35,  47,  99,
      111, 109, 112, 111, 110, 101, 110, 116, 115, 47,  101, 120, 97,  109, 112,
      108, 101, 115, 47,  69,  120, 47,  102, 111, 111, 34,  125, 125, 125, 93,
      44,  34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,
      50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116,
      105, 111, 110, 34,  58,  34,  79,  75,  34,  125, 125, 125, 125, 125, 125,
      0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(2, p->n_examples);
    ASSERT_STR_EQ("good", p->examples[0].name);
    ASSERT_STR_EQ("Right", p->examples[0].summary);
    ASSERT_STR_EQ("bad", p->examples[1].name);
    ASSERT_EQ(NULL, p->examples[1].summary);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_request_body_metadata_and_response_description(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"Test\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "p\":{\"post\":{\"requestBody\":{\"description\":\"Payload\","
      "\"required\":false,\"content\":{\"application/"
      "json\":{\"schema\":{\"type\":"
      "\"string\"}}}},\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":"
      "{\"application/json\":{\"schema\":{\"type\":\"string\"}}}}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Operation *op = &spec.paths[0].operations[0];
    ASSERT_STR_EQ("Payload", op->req_body_description);
    ASSERT_EQ(1, op->req_body_required_set);
    ASSERT_EQ(0, op->req_body_required);
    ASSERT_STR_EQ("OK", op->responses[0].description);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_request_body_component_ref(void) {

  const char json[] = {
      123, 34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 34,  58,  123,
      34,  115, 99,  104, 101, 109, 97,  115, 34,  58,  123, 34,  80,  101, 116,
      34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101,
      99,  116, 34,  44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115,
      34,  58,  123, 34,  105, 100, 34,  58,  123, 34,  116, 121, 112, 101, 34,
      58,  34,  105, 110, 116, 101, 103, 101, 114, 34,  125, 125, 125, 125, 44,
      34,  114, 101, 113, 117, 101, 115, 116, 66,  111, 100, 105, 101, 115, 34,
      58,  123, 34,  67,  114, 101, 97,  116, 101, 80,  101, 116, 34,  58,  123,
      34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,
      67,  114, 101, 97,  116, 101, 34,  44,  34,  114, 101, 113, 117, 105, 114,
      101, 100, 34,  58,  116, 114, 117, 101, 44,  34,  99,  111, 110, 116, 101,
      110, 116, 34,  58,  123, 34,  97,  112, 112, 108, 105, 99,  97,  116, 105,
      111, 110, 47,  106, 115, 111, 110, 34,  58,  123, 34,  115, 99,  104, 101,
      109, 97,  34,  58,  123, 34,  36,  114, 101, 102, 34,  58,  34,  35,  47,
      99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 47,  115, 99,  104, 101,
      109, 97,  115, 47,  80,  101, 116, 34,  125, 125, 125, 125, 125, 125, 44,
      34,  112, 97,  116, 104, 115, 34,  58,  123, 34,  47,  112, 101, 116, 115,
      34,  58,  123, 34,  112, 111, 115, 116, 34,  58,  123, 34,  114, 101, 113,
      117, 101, 115, 116, 66,  111, 100, 121, 34,  58,  123, 34,  36,  114, 101,
      102, 34,  58,  34,  35,  47,  99,  111, 109, 112, 111, 110, 101, 110, 116,
      115, 47,  114, 101, 113, 117, 101, 115, 116, 66,  111, 100, 105, 101, 115,
      47,  67,  114, 101, 97,  116, 101, 80,  101, 116, 34,  125, 44,  34,  114,
      101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,
      34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110,
      34,  58,  34,  79,  75,  34,  125, 125, 125, 125, 125, 44,  34,  111, 112,
      101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,  46,  48,  34,  125,
      0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_request_bodies);
  ASSERT_STR_EQ("CreatePet", spec.component_request_body_names[0]);
  ASSERT_STR_EQ("Create", spec.component_request_bodies[0].description);
  ASSERT_EQ(1, spec.component_request_bodies[0].required_set);
  ASSERT_EQ(1, spec.component_request_bodies[0].required);
  ASSERT_STR_EQ("application/json",
                spec.component_request_bodies[0].schema.content_type);
  ASSERT_STR_EQ("Pet", spec.component_request_bodies[0].schema.ref_name);

  {
    struct OpenAPI_Operation *op = &spec.paths[0].operations[0];
    ASSERT_STR_EQ("#/components/requestBodies/CreatePet", op->req_body_ref);
    ASSERT_STR_EQ("Create", op->req_body_description);
    ASSERT_EQ(1, op->req_body_required_set);
    ASSERT_EQ(1, op->req_body_required);
    ASSERT_STR_EQ("application/json", op->req_body.content_type);
    ASSERT_STR_EQ("Pet", op->req_body.ref_name);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_openapi_loader_security_branches(void) {
  struct OpenAPI_Spec spec;
  int k;

  /* 1. Swagger 2.0 basic auth */
  {
    const char *sw2_basic =
        "{\"swagger\":\"2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"securityDefinitions\":{\"basicAuth\":{\"type\":\"basic\"}},"
        "\"paths\":{}}";
    int rc;
    memset(&spec, 0, sizeof(spec));
    rc = load_spec_str(sw2_basic, &spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, spec.n_security_schemes);
    ASSERT_EQ(OA_SEC_HTTP, spec.security_schemes[0].type);
    ASSERT_STR_EQ("basic", spec.security_schemes[0].scheme);
    openapi_spec_free(&spec);
  }

  /* 2. Invalid component key in securitySchemes */
  {
    const char *bad_key =
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"components\":{\"securitySchemes\":{\"invalid "
        "key!\":{\"type\":\"http\",\"scheme\":\"bearer\"}}},\"paths\":{}}";
    memset(&spec, 0, sizeof(spec));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, load_spec_str(bad_key, &spec));
    openapi_spec_free(&spec);
  }

  /* 3. Unknown security type */
  {
    const char *bad_type =
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"components\":{\"securitySchemes\":{\"badType\":{\"type\":\"unknown_"
        "type\"}}},\"paths\":{}}";
    memset(&spec, 0, sizeof(spec));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, load_spec_str(bad_type, &spec));
    openapi_spec_free(&spec);
  }

  /* 4. apiKey with invalid in or missing name */
  {
    const char *bad_apikey =
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"components\":{\"securitySchemes\":{\"badApiKey\":{\"type\":"
        "\"apiKey\",\"in\":\"invalid_in\",\"name\":\"k\"}}},\"paths\":{}}";
    memset(&spec, 0, sizeof(spec));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, load_spec_str(bad_apikey, &spec));
    openapi_spec_free(&spec);
  }

  /* 5. http without scheme */
  {
    const char *bad_http =
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"components\":{\"securitySchemes\":{\"badHttp\":{\"type\":\"http\"}}}"
        ",\"paths\":{}}";
    memset(&spec, 0, sizeof(spec));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, load_spec_str(bad_http, &spec));
    openapi_spec_free(&spec);
  }

  /* 6. openIdConnect without openIdConnectUrl */
  {
    const char *bad_oidc =
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"components\":{\"securitySchemes\":{\"badOidc\":{\"type\":"
        "\"openIdConnect\"}}},\"paths\":{}}";
    memset(&spec, 0, sizeof(spec));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, load_spec_str(bad_oidc, &spec));
    openapi_spec_free(&spec);
  }

  /* 7. oauth2 without flows */
  {
    const char *bad_oauth =
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"components\":{\"securitySchemes\":{\"badOAuth\":{\"type\":"
        "\"oauth2\"}}},\"paths\":{}}";
    memset(&spec, 0, sizeof(spec));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, load_spec_str(bad_oauth, &spec));
    openapi_spec_free(&spec);
  }

  /* 8. OOM loops on parse_security_requirements */
  for (k = 1; k <= 15; ++k) {
    const char *sec_req_json =
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"security\":[{\"sec1\":[\"read\",\"write\"],\"x-sec\":\"val\"}],"
        "\"paths\":{}}";
    memset(&spec, 0, sizeof(spec));
    g_cdd_alloc_fail = k;
    load_spec_str(sec_req_json, &spec);
    g_cdd_alloc_fail = 0;
    openapi_spec_free(&spec);

    memset(&spec, 0, sizeof(spec));
    g_cdd_strdup_fail = k;
    load_spec_str(sec_req_json, &spec);
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&spec);
  }

  /* 9. OOM loops on securitySchemes */
  for (k = 1; k <= 20; ++k) {
    const char *sec_sch_json =
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"components\":{\"securitySchemes\":{\"sec1\":{\"type\":\"apiKey\","
        "\"name\":\"k\",\"in\":\"header\"},"
        "\"sec2\":{\"type\":\"http\",\"scheme\":\"bearer\",\"bearerFormat\":"
        "\"JWT\"},"
        "\"sec3\":{\"type\":\"openIdConnect\",\"openIdConnectUrl\":\"http://"
        "oidc\"},"
        "\"sec4\":{\"type\":\"oauth2\",\"oauth2MetadataUrl\":\"http://meta\","
        "\"flows\":{\"implicit\":{\"authorizationUrl\":\"http://"
        "auth\",\"scopes\":{\"read\":\"read\"}}}}}},\"paths\":{}}";
    memset(&spec, 0, sizeof(spec));
    g_cdd_alloc_fail = k;
    load_spec_str(sec_sch_json, &spec);
    g_cdd_alloc_fail = 0;
    openapi_spec_free(&spec);

    memset(&spec, 0, sizeof(spec));
    g_cdd_strdup_fail = k;
    load_spec_str(sec_sch_json, &spec);
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&spec);
  }

  /* 10. parse_security_field with non-array */
  {
    const char *bad_op_sec =
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"paths\":{\"/"
        "p\":{\"get\":{\"security\":\"not_array\",\"responses\":{\"200\":{"
        "\"description\":\"ok\"}}}}}}";
    memset(&spec, 0, sizeof(spec));
    load_spec_str(bad_op_sec, &spec);
    openapi_spec_free(&spec);
  }

  /* 11. NULL checks on test helpers */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_schemes(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_security_field(NULL, NULL, NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_security_requirements(NULL, NULL, NULL));

  /* 11b. Empty securitySchemes, null scheme, and empty security array */
  {
    memset(&spec, 0, sizeof(spec));
    load_spec_str(
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"components\":{\"securitySchemes\":{}},\"paths\":{}}",
        &spec);
    openapi_spec_free(&spec);

    memset(&spec, 0, sizeof(spec));
    load_spec_str(
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"components\":{\"securitySchemes\":{\"nullSec\":null}},\"paths\":{}}",
        &spec);
    openapi_spec_free(&spec);

    memset(&spec, 0, sizeof(spec));
    load_spec_str("{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\","
                  "\"version\":\"1\"},\"security\":[],\"paths\":{}}",
                  &spec);
    openapi_spec_free(&spec);
  }

  /* 12. Direct parse_security_schemes failure tests */
  {
    JSON_Value *jv_api =
        json_parse_string("{\"securitySchemes\":{\"apiAuth\":{\"type\":"
                          "\"apiKey\",\"name\":\"k\",\"in\":\"header\"}}}");
    JSON_Value *jv_sw2 = json_parse_string(
        "{\"securityDefinitions\":{\"basicAuth\":{\"type\":\"basic\"}}}");

    if (jv_api) {
      JSON_Object *jo = json_value_get_object(jv_api);

      /* calloc failure (line 51) */
      memset(&spec, 0, sizeof(spec));
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_security_schemes(jo, &spec));
      g_cdd_alloc_fail = 0;

      /* name strdup failure (line 64) */
      memset(&spec, 0, sizeof(spec));
      g_cdd_strdup_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_security_schemes(jo, &spec));
      g_cdd_strdup_fail = 0;

      /* key_name strdup failure (line 107) */
      memset(&spec, 0, sizeof(spec));
      g_cdd_strdup_fail = 2;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_security_schemes(jo, &spec));
      g_cdd_strdup_fail = 0;

      json_value_free(jv_api);
    }

    {
      JSON_Value *jv_ext =
          json_parse_string("{\"securitySchemes\":{\"s1\":{\"type\":\"apiKey\","
                            "\"name\":\"k\",\"in\":\"header\",\"x-s\":1}}}");
      if (jv_ext) {
        cdd_c_error_t rc_ext;
        memset(&spec, 0, sizeof(spec));
        g_cdd_strdup_fail = 2;
        rc_ext = cdd_test_parse_security_schemes(json_value_get_object(jv_ext),
                                                 &spec);
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_ext);
        g_cdd_strdup_fail = 0;
        json_value_free(jv_ext);
      }
    }

    if (jv_sw2) {
      JSON_Object *jo_sw2 = json_value_get_object(jv_sw2);

      /* basic auth scheme strdup failure (line 72) */
      memset(&spec, 0, sizeof(spec));
      spec.swagger_version = strdup("2.0");
      g_cdd_strdup_fail = 2;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cdd_test_parse_security_schemes(jo_sw2, &spec));
      g_cdd_strdup_fail = 0;
      free(spec.swagger_version);

      json_value_free(jv_sw2);
    }
  }

  /* 13. Direct parse_security_field failure tests */
  {
    JSON_Value *jv_sec = json_parse_string(
        "{\"security\":[{\"sec1\":[\"read\"],\"x-sec\":\"val\"}]}");
    if (jv_sec) {
      struct OpenAPI_SecurityRequirementSet *out_reqs = NULL;
      size_t out_cnt = 0;
      int set_flg = 0;
      JSON_Object *jo = json_value_get_object(jv_sec);

      /* calloc failure (line 192) */
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cdd_test_parse_security_field(jo, "security", &out_reqs,
                                              &out_cnt, &set_flg));
      g_cdd_alloc_fail = 0;

      /* collect_extensions failure (lines 196-198) */
      g_cdd_alloc_fail = 2;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cdd_test_parse_security_field(jo, "security", &out_reqs,
                                              &out_cnt, &set_flg));
      g_cdd_alloc_fail = 0;

      json_value_free(jv_sec);
    }
  }

  /* 14. Additional branch coverage for openapi_security.c */
  {
    JSON_Value *jv_sw2_noflow = json_parse_string(
        "{\"securityDefinitions\":{\"oauth\":{\"type\":\"oauth2\"}}}");
    JSON_Value *jv_notype =
        json_parse_string("{\"securitySchemes\":{\"noType\":{}}}");
    JSON_Value *jv_reqs = json_parse_string(
        "[{\"x-only\":\"val\"},{\"sec\":null},{\"sec\":[123]}]");
    JSON_Value *jv_obj = json_parse_string("{\"sec\":[]}");

    if (jv_sw2_noflow) {
      struct OpenAPI_Spec spec_sw2;
      memset(&spec_sw2, 0, sizeof(spec_sw2));
      spec_sw2.swagger_version = strdup("2.0");
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_security_schemes(
                    json_value_get_object(jv_sw2_noflow), &spec_sw2));
      openapi_spec_free(&spec_sw2);
      json_value_free(jv_sw2_noflow);
    }

    if (jv_notype) {
      struct OpenAPI_Spec spec_nt;
      memset(&spec_nt, 0, sizeof(spec_nt));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_schemes(
                                   json_value_get_object(jv_notype), NULL));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_test_parse_security_schemes(
                    json_value_get_object(jv_notype), &spec_nt));
      openapi_spec_free(&spec_nt);
      json_value_free(jv_notype);
    }

    if (jv_reqs) {
      JSON_Array *ja = json_value_get_array(jv_reqs);
      struct OpenAPI_SecurityRequirementSet *out_s = NULL;
      size_t out_c = 0;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_security_requirements(ja, NULL, &out_c));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_security_requirements(ja, &out_s, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_security_requirements(ja, &out_s, &out_c));
      if (out_s) {
        size_t idx;
        for (idx = 0; idx < out_c; ++idx)
          cdd_test_free_security_requirement_set(&out_s[idx]);
        free(out_s);
      }
      json_value_free(jv_reqs);
    }

    {
      JSON_Value *jv_empty_key =
          json_parse_string("{\"securitySchemes\":{\"api\":{\"type\":"
                            "\"apiKey\",\"in\":\"header\","
                            "\"name\":\"\"}}}");
      JSON_Value *jv_empty_sch =
          json_parse_string("{\"securitySchemes\":{\"http\":{\"type\":\"http\","
                            "\"scheme\":\"\"}}}");
      JSON_Value *jv_empty_oid = json_parse_string(
          "{\"securitySchemes\":{\"oid\":{\"type\":\"openIdConnect\","
          "\"openIdConnectUrl\":\"\"}}}");
      JSON_Value *jv_non_obj = json_parse_string("[123]");
      struct OpenAPI_Spec s_dummy;

      if (jv_empty_key) {
        memset(&s_dummy, 0, sizeof(s_dummy));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_test_parse_security_schemes(
                      json_value_get_object(jv_empty_key), &s_dummy));
        openapi_spec_free(&s_dummy);
        json_value_free(jv_empty_key);
      }

      if (jv_empty_sch) {
        memset(&s_dummy, 0, sizeof(s_dummy));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_test_parse_security_schemes(
                      json_value_get_object(jv_empty_sch), &s_dummy));
        openapi_spec_free(&s_dummy);
        json_value_free(jv_empty_sch);
      }

      if (jv_empty_oid) {
        memset(&s_dummy, 0, sizeof(s_dummy));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_test_parse_security_schemes(
                      json_value_get_object(jv_empty_oid), &s_dummy));
        openapi_spec_free(&s_dummy);
        json_value_free(jv_empty_oid);
      }

      if (jv_non_obj) {
        struct OpenAPI_SecurityRequirementSet *out_s = NULL;
        size_t out_c = 0;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_test_parse_security_requirements(
                      json_value_get_array(jv_non_obj), &out_s, &out_c));
        if (out_s) {
          size_t idx;
          for (idx = 0; idx < out_c; ++idx)
            cdd_test_free_security_requirement_set(&out_s[idx]);
          free(out_s);
        }
        json_value_free(jv_non_obj);
      }
    }

    if (jv_obj) {
      JSON_Object *jo = json_value_get_object(jv_obj);
      struct OpenAPI_SecurityRequirementSet *out_s = NULL;
      size_t out_c = 0;
      int set_f = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_field(
                                   NULL, "sec", &out_s, &out_c, &set_f));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_field(jo, NULL, &out_s,
                                                             &out_c, &set_f));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_security_field(jo, "sec", NULL, &out_c, &set_f));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_security_field(jo, "sec", &out_s, NULL, &set_f));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_security_field(jo, "sec", &out_s, &out_c, NULL));
      json_value_free(jv_obj);
    }
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_SECURITY_H */
