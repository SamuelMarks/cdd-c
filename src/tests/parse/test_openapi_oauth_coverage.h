/**
 * @file test_openapi_oauth_coverage.h
 * @brief Unit tests for OpenAPI OAuth2 scope and flow parsing
 * (openapi_examples.c).
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_OAUTH_COVERAGE_H
#define TEST_OPENAPI_OAUTH_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Helper function free_test_oauth_scopes.
 * @param[in,out] scopes Parameter scopes.
 * @param[in] count Parameter count.
 */
static void free_test_oauth_scopes(struct OpenAPI_OAuthScope *scopes,
                                   size_t count) {
  size_t i;
  if (!scopes)
    return;
  for (i = 0; i < count; ++i) {
    if (scopes[i].name)
      free(scopes[i].name);
    if (scopes[i].description)
      free(scopes[i].description);
  }
  free(scopes);
}

/**
 * @brief Helper function free_test_security_scheme_flows.
 * @param[in,out] sec Parameter sec.
 */
static void
free_test_security_scheme_flows(struct OpenAPI_SecurityScheme *sec) {
  size_t f, s;
  if (!sec || !sec->flows)
    return;
  for (f = 0; f < sec->n_flows; ++f) {
    struct OpenAPI_OAuthFlow *flow = &sec->flows[f];
    if (flow->authorization_url)
      free(flow->authorization_url);
    if (flow->token_url)
      free(flow->token_url);
    if (flow->refresh_url)
      free(flow->refresh_url);
    if (flow->device_authorization_url)
      free(flow->device_authorization_url);
    if (flow->extensions_json)
      free(flow->extensions_json);
    if (flow->scopes) {
      for (s = 0; s < flow->n_scopes; ++s) {
        if (flow->scopes[s].name)
          free(flow->scopes[s].name);
        if (flow->scopes[s].description)
          free(flow->scopes[s].description);
      }
      free(flow->scopes);
      flow->scopes = NULL;
    }
  }
  free(sec->flows);
  sec->flows = NULL;
  sec->n_flows = 0;
}

/**
 * @brief Unit test for parse_oauth_scopes and parse_oauth_flows across all
 * branches.
 * @return Greatest test result.
 */
TEST test_openapi_oauth_scopes_and_flows_branches(void) {
  struct OpenAPI_OAuthScope *scopes = NULL;
  size_t n_scopes = 0;
  struct OpenAPI_SecurityScheme sec;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;

  memset(&sec, 0, sizeof(sec));

  free_test_oauth_scopes(NULL, 0);
  free_test_security_scheme_flows(NULL);
  free_test_security_scheme_flows(&sec);

  /* 6. parse_oauth_scopes */
  ASSERT_EQ(CDD_C_SUCCESS, parse_oauth_scopes(NULL, NULL, &n_scopes));
  ASSERT_EQ(CDD_C_SUCCESS, parse_oauth_scopes(NULL, &scopes, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, parse_oauth_scopes(NULL, &scopes, &n_scopes));
  ASSERT_EQ(NULL, scopes);
  ASSERT_EQ(0, n_scopes);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_oauth_scopes(jo, &scopes, &n_scopes));
  ASSERT_EQ(NULL, scopes);
  ASSERT_EQ(0, n_scopes);
  json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"read:pets\": \"read pets\", \"write:pets\": "
                          "\"write pets\"}",
                          &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_oauth_scopes(jo, &scopes, &n_scopes));
  g_cdd_alloc_fail = 0;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_oauth_scopes(jo, &scopes, &n_scopes));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_oauth_scopes(jo, &scopes, &n_scopes));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, parse_oauth_scopes(jo, &scopes, &n_scopes));
  ASSERT_EQ(2, n_scopes);
  free_test_oauth_scopes(scopes, n_scopes);
  scopes = NULL;
  json_value_free(jv);

  /* scope with non-string description */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"read:pets\": 123}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_oauth_scopes(jo, &scopes, &n_scopes));
  ASSERT_EQ(1, n_scopes);
  ASSERT_EQ(NULL, scopes[0].description);
  free_test_oauth_scopes(scopes, n_scopes);
  scopes = NULL;
  json_value_free(jv);

  /* 7. parse_oauth_flows */
  ASSERT_EQ(CDD_C_SUCCESS, parse_oauth_flows(NULL, &sec));
  ASSERT_EQ(CDD_C_SUCCESS, parse_oauth_flows(NULL, NULL));

  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  json_value_free(jv);

  /* flows_obj != NULL, out == NULL */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"implicit\": {\"authorizationUrl\": "
                          "\"http://auth\", \"scopes\": {}}}",
                          &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_oauth_flows(jo, NULL));
  json_value_free(jv);

  /* flow_obj is not an object */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"implicit\": \"not_an_obj\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Invalid flow name without scopes */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"invalidFlow\": {}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Invalid flow name with scopes (reaches switch default /
   * OA_OAUTH_FLOW_UNKNOWN) */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"invalidFlow\": {\"scopes\": {}}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Implicit flow without scopes */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"implicit\": {}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Implicit flow with non-object scopes */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"implicit\": {\"scopes\": 123}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Implicit flow without authorizationUrl */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"implicit\": {\"scopes\": {}}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Password flow without tokenUrl */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"password\": {\"scopes\": {}}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Client credentials flow without tokenUrl */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"clientCredentials\": {\"scopes\": {}}}",
                          &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Client credentials flow without tokenUrl */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"clientCredentials\": {\"scopes\": {}}}",
                          &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Authorization code flow without tokenUrl */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"authorizationCode\": {\"scopes\": {}, "
                          "\"authorizationUrl\": \"http://auth\"}}",
                          &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Authorization code flow without authorizationUrl */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"authorizationCode\": {\"scopes\": {}, "
                          "\"tokenUrl\": \"http://tok\"}}",
                          &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Device authorization flow without tokenUrl */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"deviceAuthorization\": {\"scopes\": {}, "
                          "\"deviceAuthorizationUrl\": \"http://dev\"}}",
                          &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Device authorization flow without deviceAuthorizationUrl */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"deviceAuthorization\": {\"scopes\": {}, "
                          "\"tokenUrl\": \"http://tok\"}}",
                          &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Valid flows for all types */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string(
        "{"
        "\"implicit\": {\"authorizationUrl\": \"http://auth\", "
        "\"refreshUrl\": \"http://ref\", \"scopes\": {\"s\": "
        "\"desc\"}, \"x-f\": 1},"
        "\"password\": {\"tokenUrl\": \"http://tok\", "
        "\"scopes\": {}},"
        "\"clientCredentials\": {\"tokenUrl\": \"http://tok\", "
        "\"scopes\": {}},"
        "\"authorizationCode\": {\"authorizationUrl\": "
        "\"http://auth\", \"tokenUrl\": \"http://tok\", "
        "\"scopes\": {}},"
        "\"deviceAuthorization\": {\"deviceAuthorizationUrl\": "
        "\"http://dev\", \"tokenUrl\": \"http://tok\", "
        "\"scopes\": {}}"
        "}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_oauth_flows(jo, &sec));
  g_cdd_alloc_fail = 0;
  free_test_security_scheme_flows(&sec);

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_oauth_flows(jo, &sec));
  g_cdd_strdup_fail = 0;
  free_test_security_scheme_flows(&sec);

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_oauth_flows(jo, &sec));
  g_cdd_strdup_fail = 0;
  free_test_security_scheme_flows(&sec);

  ASSERT_EQ(CDD_C_SUCCESS, parse_oauth_flows(jo, &sec));
  ASSERT_EQ(5, sec.n_flows);
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Flow extension failure */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"implicit\": {\"authorizationUrl\": "
                          "\"http://auth\", \"scopes\": {}, \"x-f\": 1}}",
                          &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_oauth_flows(jo, &sec));
  g_cdd_strdup_fail = 0;
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_OAUTH_COVERAGE_H */
