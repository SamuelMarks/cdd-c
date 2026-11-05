/**
 * @file test_openapi_free_and_registry.h
 * @brief Unit tests for OpenAPI free routines, spec initialization, and
 * document registry.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_FREE_AND_REGISTRY_H
#define TEST_OPENAPI_FREE_AND_REGISTRY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Unit test for freeing OpenAPI structures (RequestBody, Any, Link,
 * SecurityRequirement, PathItem, Callback, string array).
 * @return Greatest test result.
 */
TEST test_openapi_free_helpers(void) {
  struct OpenAPI_RequestBody rb;
  struct OpenAPI_Any any_val;
  struct OpenAPI_Link lnk;
  struct OpenAPI_SecurityRequirement sec_req;
  struct OpenAPI_Path path_item;
  struct OpenAPI_Callback cb;
  char **str_arr;

  /* 1. RequestBody free */
  memset(&rb, 0, sizeof(rb));
  rb.description = strdup("rb desc");
  rb.ref = strdup("#/components/requestBodies/B");
  rb.extensions_json = strdup("{\"x-r\":1}");
  free_request_body(&rb);
  free_request_body(NULL);

  /* 2. Any free */
  memset(&any_val, 0, sizeof(any_val));
  any_val.type = OA_ANY_STRING;
  any_val.string = strdup("str");
  free_any_value(&any_val);

  memset(&any_val, 0, sizeof(any_val));
  any_val.type = OA_ANY_JSON;
  any_val.json = strdup("{}");
  free_any_value(&any_val);

  memset(&any_val, 0, sizeof(any_val));
  any_val.type = OA_ANY_NUMBER;
  any_val.number = 42.0;
  free_any_value(&any_val);
  free_any_value(NULL);

  /* 3. Link free */
  memset(&lnk, 0, sizeof(lnk));
  lnk.name = strdup("lnk1");
  lnk.ref = strdup("#/components/links/L");
  lnk.operation_id = strdup("op1");
  lnk.operation_ref = strdup("#/paths/~1foo/get");
  lnk.description = strdup("lnk desc");
  lnk.summary = strdup("lnk sum");
  lnk.extensions_json = strdup("{\"x-lnk\":1}");
  free_link(&lnk);
  free_link(NULL);

  /* 4. Security Requirement free */
  memset(&sec_req, 0, sizeof(sec_req));
  sec_req.scheme = strdup("sec1");
  sec_req.scopes = (char **)malloc(sizeof(char *));
  if (sec_req.scopes) {
    sec_req.scopes[0] = strdup("read:all");
    sec_req.n_scopes = 1;
  }
  free_security_requirement(&sec_req);
  free_security_requirement(NULL);

  /* 5. PathItem free */
  memset(&path_item, 0, sizeof(path_item));
  path_item.route = strdup("/test");
  path_item.ref = strdup("#/components/pathItems/Item");
  path_item.summary = strdup("pi summary");
  path_item.description = strdup("pi description");
  path_item.extensions_json = strdup("{\"x-pi\":1}");
  free_path_item(&path_item);
  free_path_item(NULL);

  /* 6. Callback free */
  memset(&cb, 0, sizeof(cb));
  cb.name = strdup("cb1");
  cb.ref = strdup("#/components/callbacks/Cb");
  cb.extensions_json = strdup("{\"x-cb\":1}");
  free_callback(&cb);
  free_callback(NULL);

  /* 7. String array free */
  str_arr = (char **)malloc(2 * sizeof(char *));
  if (str_arr) {
    str_arr[0] = strdup("s1");
    str_arr[1] = strdup("s2");
    free_string_array(str_arr, 2);
  }
  free_string_array(NULL, 0);

  PASS();
}

/**
 * @brief Unit test for openapi_doc_registry initialization, registration, OOM,
 * and cleanup.
 * @return Greatest test result.
 */
TEST test_openapi_doc_registry(void) {
  extern C_CDD_EXPORT int g_cdd_alloc_fail;
  struct OpenAPI_DocRegistry reg;
  struct OpenAPI_Spec s1;
  struct OpenAPI_Spec s2;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, openapi_doc_registry_init(NULL));
  openapi_doc_registry_free(NULL);

  ASSERT_EQ(CDD_C_SUCCESS, openapi_doc_registry_init(&reg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, openapi_doc_registry_add(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, openapi_doc_registry_add(&reg, NULL));

  memset(&s1, 0, sizeof(s1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, openapi_doc_registry_add(&reg, &s1));

  s1.document_uri = strdup("");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, openapi_doc_registry_add(&reg, &s1));
  free(s1.document_uri);

  s1.document_uri = strdup("#only_fragment");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, openapi_doc_registry_add(&reg, &s1));
  free(s1.document_uri);

  s1.document_uri = strdup("http://example.com/spec.json");
  ASSERT_EQ(CDD_C_SUCCESS, openapi_doc_registry_add(&reg, &s1));

  /* Duplicate base_uri returns invalid argument */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, openapi_doc_registry_add(&reg, &s1));

  /* dup_substr failure */
  memset(&s2, 0, sizeof(s2));
  s2.document_uri = strdup("http://fail.com/spec.json");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_doc_registry_add(&reg, &s2));
  g_cdd_alloc_fail = 0;
  free(s2.document_uri);

  /* Realloc failure */
  memset(&s2, 0, sizeof(s2));
  s2.document_uri = strdup("http://other.com/spec.json");
  reg.count = reg.capacity;
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_doc_registry_add(&reg, &s2));
  g_cdd_alloc_fail = 0;
  free(s2.document_uri);
  reg.count = 1;

  openapi_doc_registry_free(&reg);
  openapi_spec_free(&s1);

  PASS();
}

/**
 * @brief Unit test for extra free paths and specific branch coverage in
 * openapi_free_components.c and openapi_free.c.
 * @return Greatest test result.
 */
TEST test_openapi_free_extra_and_branches(void) {
  struct OpenAPI_Spec s_free;
  struct OpenAPI_SchemaRef ref_free;
  struct OpenAPI_Encoding enc_free;
  struct OpenAPI_MediaType mt_free;
  struct OpenAPI_Header hdr_free;
  struct OpenAPI_SecurityRequirementSet sec_set_free;
  struct OpenAPI_Any val_null_json;
  struct OpenAPI_Link lnk_null_srv;
  struct OpenAPI_Spec s_free2;
  char **null_arr;

  /* 1. Extra free paths */
  openapi_spec_free(NULL);
  free_security_requirement_set(NULL);

  memset(&s_free, 0, sizeof(s_free));
  s_free.swagger_version = strdup("2.0");
  s_free.n_servers = 1;
  s_free.servers =
      (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
  s_free.servers[0].n_variables = 1;
  s_free.servers[0].variables = (struct OpenAPI_ServerVariable *)calloc(
      1, sizeof(struct OpenAPI_ServerVariable));
  s_free.servers[0].variables[0].extensions_json = strdup("{\"x-ext\":1}");
  s_free.n_tags = 1;
  s_free.tags = (struct OpenAPI_Tag *)calloc(1, sizeof(struct OpenAPI_Tag));
  s_free.tags[0].external_docs.extensions_json = strdup("{\"x-ext\":1}");
  s_free.n_security_schemes = 1;
  s_free.security_schemes = (struct OpenAPI_SecurityScheme *)calloc(
      1, sizeof(struct OpenAPI_SecurityScheme));
  s_free.security_schemes[0].n_flows = 1;
  s_free.security_schemes[0].flows =
      (struct OpenAPI_OAuthFlow *)calloc(1, sizeof(struct OpenAPI_OAuthFlow));
  s_free.security_schemes[0].flows[0].device_authorization_url =
      strdup("https://example.com/device");
  s_free.security_schemes[0].flows[0].extensions_json = strdup("{\"x-ext\":1}");
  openapi_spec_free(&s_free);

  memset(&ref_free, 0, sizeof(ref_free));
  ref_free.n_multipart_fields = 1;
  ref_free.multipart_fields = (struct OpenAPI_MultipartField *)calloc(
      1, sizeof(struct OpenAPI_MultipartField));
  ref_free.multipart_fields[0].name = strdup("f");
  ref_free.multipart_fields[0].type = strdup("t");
  free_schema_ref_content(&ref_free);

  memset(&enc_free, 0, sizeof(enc_free));
  enc_free.extensions_json = strdup("{\"x-ext\":1}");
  free_encoding(&enc_free);

  memset(&mt_free, 0, sizeof(mt_free));
  mt_free.extensions_json = strdup("{\"x-ext\":1}");
  free_media_type(&mt_free);

  memset(&hdr_free, 0, sizeof(hdr_free));
  hdr_free.extensions_json = strdup("{\"x-ext\":1}");
  free_header(&hdr_free);

  memset(&sec_set_free, 0, sizeof(sec_set_free));
  sec_set_free.n_requirements = 1;
  sec_set_free.requirements = (struct OpenAPI_SecurityRequirement *)calloc(
      1, sizeof(struct OpenAPI_SecurityRequirement));
  sec_set_free.requirements[0].scheme = strdup("Basic");
  sec_set_free.extensions_json = strdup("{\"x-ext\":1}");
  free_security_requirement_set(&sec_set_free);

  /* 2. Specific branch coverage for openapi_free_components.c and
   * openapi_free.c */
  memset(&val_null_json, 0, sizeof(val_null_json));
  val_null_json.type = OA_ANY_JSON;
  val_null_json.json = NULL;
  free_any_value(&val_null_json);

  memset(&lnk_null_srv, 0, sizeof(lnk_null_srv));
  lnk_null_srv.server_set = 1;
  lnk_null_srv.server = NULL;
  free_link(&lnk_null_srv);

  memset(&s_free2, 0, sizeof(s_free2));
  s_free2.n_component_media_types = 1;
  s_free2.component_media_types =
      (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
  s_free2.component_media_type_names = NULL;
  openapi_spec_free(&s_free2);

  memset(&s_free2, 0, sizeof(s_free2));
  s_free2.n_component_examples = 1;
  s_free2.component_examples =
      (struct OpenAPI_Example *)calloc(1, sizeof(struct OpenAPI_Example));
  s_free2.component_example_names = NULL;
  openapi_spec_free(&s_free2);

  memset(&s_free2, 0, sizeof(s_free2));
  s_free2.n_raw_schemas = 1;
  s_free2.raw_schema_names = (char **)calloc(1, sizeof(char *));
  s_free2.raw_schema_names[0] = strdup("raw1");
  s_free2.raw_schema_json = NULL;
  openapi_spec_free(&s_free2);

  memset(&s_free2, 0, sizeof(s_free2));
  s_free2.n_raw_schemas = 1;
  s_free2.raw_schema_names = NULL;
  s_free2.raw_schema_json = (char **)calloc(1, sizeof(char *));
  s_free2.raw_schema_json[0] = strdup("{}");
  openapi_spec_free(&s_free2);

  null_arr = (char **)calloc(1, sizeof(char *));
  null_arr[0] = NULL;
  free_string_array(null_arr, 1);

  {
    struct OpenAPI_Spec s_dummy;
    struct OpenAPI_DocRegistry empty_reg;

    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&s_dummy));

    memset(&empty_reg, 0, sizeof(empty_reg));
    openapi_doc_registry_free(&empty_reg);
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_FREE_AND_REGISTRY_H */
