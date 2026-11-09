/**
 * @file test_openapi_find_components.h
 * @brief Unit tests for OpenAPI component lookup routines (openapi_find.c).
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_FIND_COMPONENTS_H
#define TEST_OPENAPI_FIND_COMPONENTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Unit test for local OpenAPI component lookup routines.
 * @return Greatest test result.
 */
TEST test_openapi_find_local_components(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp;
  struct OpenAPI_Header hdr;
  struct OpenAPI_RequestBody rb;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Link lnk;
  struct OpenAPI_Callback cb;
  struct OpenAPI_Path path;
  struct OpenAPI_Example ex;

  struct OpenAPI_Parameter *out_param = NULL;
  struct OpenAPI_Response *out_resp = NULL;
  struct OpenAPI_Header *out_hdr = NULL;
  struct OpenAPI_RequestBody *out_rb = NULL;
  struct OpenAPI_MediaType *out_mt = NULL;
  struct OpenAPI_Link *out_lnk = NULL;
  struct OpenAPI_Callback *out_cb = NULL;
  struct OpenAPI_Path *out_path = NULL;
  struct OpenAPI_Example *out_ex = NULL;

  char *param_names[1];
  char *resp_names[1];
  char *hdr_names[1];
  char *rb_names[1];
  char *mt_names[1];
  char *path_names[1];
  char *ex_names[1];

  memset(&spec, 0, sizeof(spec));
  memset(&param, 0, sizeof(param));
  memset(&resp, 0, sizeof(resp));
  memset(&hdr, 0, sizeof(hdr));
  memset(&rb, 0, sizeof(rb));
  memset(&mt, 0, sizeof(mt));
  memset(&lnk, 0, sizeof(lnk));
  memset(&cb, 0, sizeof(cb));
  memset(&path, 0, sizeof(path));
  memset(&ex, 0, sizeof(ex));

  param_names[0] = cdd_test_mut_str("MyParam");
  spec.n_component_parameters = 1;
  spec.component_parameter_names = param_names;
  spec.component_parameters = &param;

  resp_names[0] = cdd_test_mut_str("MyResp");
  spec.n_component_responses = 1;
  spec.component_response_names = resp_names;
  spec.component_responses = &resp;

  hdr_names[0] = cdd_test_mut_str("MyHdr");
  spec.n_component_headers = 1;
  spec.component_header_names = hdr_names;
  spec.component_headers = &hdr;

  rb_names[0] = cdd_test_mut_str("MyBody");
  spec.n_component_request_bodies = 1;
  spec.component_request_body_names = rb_names;
  spec.component_request_bodies = &rb;

  mt_names[0] = cdd_test_mut_str("app/json");
  spec.n_component_media_types = 1;
  spec.component_media_type_names = mt_names;
  spec.component_media_types = &mt;

  lnk.name = cdd_test_mut_str("MyLink");
  spec.n_component_links = 1;
  spec.component_links = &lnk;

  cb.name = cdd_test_mut_str("MyCb");
  spec.n_component_callbacks = 1;
  spec.component_callbacks = &cb;

  path_names[0] = cdd_test_mut_str("/myPath");
  spec.n_component_path_items = 1;
  spec.component_path_item_names = path_names;
  spec.component_path_items = &path;

  ex_names[0] = cdd_test_mut_str("MyParam");
  spec.n_component_examples = 1;
  spec.component_example_names = ex_names;
  spec.component_examples = &ex;

  /* 1. NULL checks */
  ASSERT_EQ(CDD_C_SUCCESS, find_component_parameter(NULL, NULL, &out_param));
  ASSERT(out_param == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, find_component_response(NULL, NULL, &out_resp));
  ASSERT(out_resp == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, find_component_header(NULL, NULL, &out_hdr));
  ASSERT(out_hdr == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, find_component_request_body(NULL, NULL, &out_rb));
  ASSERT(out_rb == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, find_component_media_type(NULL, NULL, &out_mt));
  ASSERT(out_mt == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, find_component_link(NULL, NULL, &out_lnk));
  ASSERT(out_lnk == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, find_component_callback(NULL, NULL, &out_cb));
  ASSERT(out_cb == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, find_component_path_item(NULL, NULL, &out_path));
  ASSERT(out_path == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, find_component_example(NULL, NULL, &out_ex));
  ASSERT(out_ex == NULL);

  /* 2. Bad prefix / not found */
  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_parameter(&spec, "#/wrong/prefix", &out_param));
  ASSERT(out_param == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_parameter(&spec, "#/components/parameters/NotFound",
                                     &out_param));
  ASSERT(out_param == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_response(&spec, "#/wrong/prefix", &out_resp));
  ASSERT(out_resp == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_response(&spec, "#/components/responses/NotFound",
                                    &out_resp));
  ASSERT(out_resp == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_header(&spec, "#/wrong/prefix", &out_hdr));
  ASSERT(out_hdr == NULL);
  ASSERT_EQ(
      CDD_C_SUCCESS,
      find_component_header(&spec, "#/components/headers/NotFound", &out_hdr));
  ASSERT(out_hdr == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_request_body(&spec, "#/wrong/prefix", &out_rb));
  ASSERT(out_rb == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_request_body(
                &spec, "#/components/requestBodies/NotFound", &out_rb));
  ASSERT(out_rb == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_media_type(&spec, "#/wrong/prefix", &out_mt));
  ASSERT(out_mt == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_media_type(&spec, "#/components/mediaTypes/NotFound",
                                      &out_mt));
  ASSERT(out_mt == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_link(&spec, "#/wrong/prefix", &out_lnk));
  ASSERT(out_lnk == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, find_component_link(
                               &spec, "#/components/links/NotFound", &out_lnk));
  ASSERT(out_lnk == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_callback(&spec, "#/wrong/prefix", &out_cb));
  ASSERT(out_cb == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_callback(&spec, "#/components/callbacks/NotFound",
                                    &out_cb));
  ASSERT(out_cb == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_path_item(&spec, "#/wrong/prefix", &out_path));
  ASSERT(out_path == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_path_item(&spec, "#/components/pathItems/NotFound",
                                     &out_path));
  ASSERT(out_path == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_example(&spec, "#/wrong/prefix", &out_ex));
  ASSERT(out_ex == NULL);
  ASSERT_EQ(
      CDD_C_SUCCESS,
      find_component_example(&spec, "#/components/examples/NotFound", &out_ex));
  ASSERT(out_ex == NULL);

  /* 3. Found matches */
  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_parameter(&spec, "#/components/parameters/MyParam",
                                     &out_param));
  ASSERT(out_param == &param);

  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_response(&spec, "#/components/responses/MyResp",
                                    &out_resp));
  ASSERT(out_resp == &resp);

  ASSERT_EQ(CDD_C_SUCCESS, find_component_header(
                               &spec, "#/components/headers/MyHdr", &out_hdr));
  ASSERT(out_hdr == &hdr);

  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_request_body(
                &spec, "#/components/requestBodies/MyBody", &out_rb));
  ASSERT(out_rb == &rb);

  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_media_type(
                &spec, "#/components/mediaTypes/app~1json", &out_mt));
  ASSERT(out_mt == &mt);

  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_link(&spec, "#/components/links/MyLink", &out_lnk));
  ASSERT(out_lnk == &lnk);

  ASSERT_EQ(CDD_C_SUCCESS, find_component_callback(
                               &spec, "#/components/callbacks/MyCb", &out_cb));
  ASSERT(out_cb == &cb);

  ASSERT_EQ(CDD_C_SUCCESS,
            find_component_path_item(&spec, "#/components/pathItems/~1myPath",
                                     &out_path));
  ASSERT(out_path == &path);

  ASSERT_EQ(
      CDD_C_SUCCESS,
      find_component_example(&spec, "#/components/examples/MyParam", &out_ex));
  ASSERT(out_ex == &ex);

  PASS();
}

/**
 * @brief Unit test for external OpenAPI component lookup routines via document
 * registry.
 * @return Greatest test result.
 */
TEST test_openapi_find_external_components(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp;
  struct OpenAPI_Header hdr;
  struct OpenAPI_RequestBody rb;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Link lnk;
  struct OpenAPI_Callback cb;
  struct OpenAPI_Path path;
  struct OpenAPI_Example ex;

  struct OpenAPI_Parameter *out_param = NULL;
  struct OpenAPI_Response *out_resp = NULL;
  struct OpenAPI_Header *out_hdr = NULL;
  struct OpenAPI_RequestBody *out_rb = NULL;
  struct OpenAPI_MediaType *out_mt = NULL;
  struct OpenAPI_Link *out_lnk = NULL;
  struct OpenAPI_Callback *out_cb = NULL;
  struct OpenAPI_Path *out_path = NULL;
  struct OpenAPI_Example *out_ex = NULL;

  char *param_names[1];
  char *resp_names[1];
  char *hdr_names[1];
  char *rb_names[1];
  char *mt_names[1];
  char *path_names[1];
  char *ex_names[1];

  memset(&spec, 0, sizeof(spec));
  memset(&param, 0, sizeof(param));
  memset(&resp, 0, sizeof(resp));
  memset(&hdr, 0, sizeof(hdr));
  memset(&rb, 0, sizeof(rb));
  memset(&mt, 0, sizeof(mt));
  memset(&lnk, 0, sizeof(lnk));
  memset(&cb, 0, sizeof(cb));
  memset(&path, 0, sizeof(path));
  memset(&ex, 0, sizeof(ex));

  param_names[0] = cdd_test_mut_str("MyParam");
  spec.n_component_parameters = 1;
  spec.component_parameter_names = param_names;
  spec.component_parameters = &param;

  resp_names[0] = cdd_test_mut_str("MyResp");
  spec.n_component_responses = 1;
  spec.component_response_names = resp_names;
  spec.component_responses = &resp;

  hdr_names[0] = cdd_test_mut_str("MyHdr");
  spec.n_component_headers = 1;
  spec.component_header_names = hdr_names;
  spec.component_headers = &hdr;

  rb_names[0] = cdd_test_mut_str("MyBody");
  spec.n_component_request_bodies = 1;
  spec.component_request_body_names = rb_names;
  spec.component_request_bodies = &rb;

  mt_names[0] = cdd_test_mut_str("app/json");
  spec.n_component_media_types = 1;
  spec.component_media_type_names = mt_names;
  spec.component_media_types = &mt;

  lnk.name = cdd_test_mut_str("MyLink");
  spec.n_component_links = 1;
  spec.component_links = &lnk;

  cb.name = cdd_test_mut_str("MyCb");
  spec.n_component_callbacks = 1;
  spec.component_callbacks = &cb;

  path_names[0] = cdd_test_mut_str("/myPath");
  spec.n_component_path_items = 1;
  spec.component_path_item_names = path_names;
  spec.component_path_items = &path;

  ex_names[0] = cdd_test_mut_str("MyParam");
  spec.n_component_examples = 1;
  spec.component_example_names = ex_names;
  spec.component_examples = &ex;

  /* 4. External ref with registry to exercise resolved.resolved_ref free on
   * match and non-match */
  {
    struct OpenAPI_DocRegistry reg;
    struct OpenAPI_Spec other_spec;

    memset(&reg, 0, sizeof(reg));
    memset(&other_spec, 0, sizeof(other_spec));
    ASSERT_EQ(CDD_C_SUCCESS, openapi_doc_registry_init(&reg));

    other_spec.document_uri = cdd_test_mut_str("http://example.com/other.json");
    other_spec.doc_registry = &reg;
    other_spec.n_component_parameters = 1;
    other_spec.component_parameter_names = param_names;
    other_spec.component_parameters = &param;

    other_spec.n_component_responses = 1;
    other_spec.component_response_names = resp_names;
    other_spec.component_responses = &resp;

    other_spec.n_component_headers = 1;
    other_spec.component_header_names = hdr_names;
    other_spec.component_headers = &hdr;

    other_spec.n_component_request_bodies = 1;
    other_spec.component_request_body_names = rb_names;
    other_spec.component_request_bodies = &rb;

    other_spec.n_component_media_types = 1;
    other_spec.component_media_type_names = mt_names;
    other_spec.component_media_types = &mt;

    other_spec.n_component_links = 1;
    other_spec.component_links = &lnk;

    other_spec.n_component_callbacks = 1;
    other_spec.component_callbacks = &cb;

    other_spec.n_component_path_items = 1;
    other_spec.component_path_item_names = path_names;
    other_spec.component_path_items = &path;

    other_spec.n_component_examples = 1;
    other_spec.component_example_names = ex_names;
    other_spec.component_examples = &ex;

    ASSERT_EQ(CDD_C_SUCCESS, openapi_doc_registry_add(&reg, &other_spec));

    spec.document_uri = cdd_test_mut_str("http://example.com/base/spec.json");
    spec.doc_registry = &reg;

    /* A. Match found in external target */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_parameter(
            &spec, "../other.json#/components/parameters/MyParam", &out_param));
    ASSERT(out_param == &param);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_response(
            &spec, "../other.json#/components/responses/MyResp", &out_resp));
    ASSERT(out_resp == &resp);
    ASSERT_EQ(CDD_C_SUCCESS,
              find_component_header(
                  &spec, "../other.json#/components/headers/MyHdr", &out_hdr));
    ASSERT(out_hdr == &hdr);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_request_body(
            &spec, "../other.json#/components/requestBodies/MyBody", &out_rb));
    ASSERT(out_rb == &rb);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_media_type(
            &spec, "../other.json#/components/mediaTypes/app~1json", &out_mt));
    ASSERT(out_mt == &mt);
    ASSERT_EQ(CDD_C_SUCCESS,
              find_component_link(
                  &spec, "../other.json#/components/links/MyLink", &out_lnk));
    ASSERT(out_lnk == &lnk);
    ASSERT_EQ(CDD_C_SUCCESS,
              find_component_callback(
                  &spec, "../other.json#/components/callbacks/MyCb", &out_cb));
    ASSERT(out_cb == &cb);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_path_item(
            &spec, "../other.json#/components/pathItems/~1myPath", &out_path));
    ASSERT(out_path == &path);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_example(
            &spec, "../other.json#/components/examples/MyParam", &out_ex));
    ASSERT(out_ex == &ex);

    /* B. Not found in external target */
    ASSERT_EQ(CDD_C_SUCCESS,
              find_component_parameter(
                  &spec, "../other.json#/components/parameters/NotFound",
                  &out_param));
    ASSERT(out_param == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_response(
            &spec, "../other.json#/components/responses/NotFound", &out_resp));
    ASSERT(out_resp == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_header(
            &spec, "../other.json#/components/headers/NotFound", &out_hdr));
    ASSERT(out_hdr == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              find_component_request_body(
                  &spec, "../other.json#/components/requestBodies/NotFound",
                  &out_rb));
    ASSERT(out_rb == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_media_type(
            &spec, "../other.json#/components/mediaTypes/NotFound", &out_mt));
    ASSERT(out_mt == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              find_component_link(
                  &spec, "../other.json#/components/links/NotFound", &out_lnk));
    ASSERT(out_lnk == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_callback(
            &spec, "../other.json#/components/callbacks/NotFound", &out_cb));
    ASSERT(out_cb == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_path_item(
            &spec, "../other.json#/components/pathItems/NotFound", &out_path));
    ASSERT(out_path == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_example(
            &spec, "../other.json#/components/examples/NotFound", &out_ex));
    ASSERT(out_ex == NULL);

    /* C. Unescape failure */
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              find_component_media_type(
                  &spec, "#/components/mediaTypes/app~1json", &out_mt));
    ASSERT(out_mt == NULL);
    g_cdd_alloc_fail = 0;

    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              find_component_path_item(&spec, "#/components/pathItems/~1myPath",
                                       &out_path));
    ASSERT(out_path == NULL);
    g_cdd_alloc_fail = 0;

    /* D. External ref with wrong prefix */
    ASSERT_EQ(CDD_C_SUCCESS,
              find_component_parameter(&spec, "../other.json#/wrong/prefix",
                                       &out_param));
    ASSERT(out_param == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              find_component_response(&spec, "../other.json#/wrong/prefix",
                                      &out_resp));
    ASSERT(out_resp == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_header(&spec, "../other.json#/wrong/prefix", &out_hdr));
    ASSERT(out_hdr == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              find_component_request_body(&spec, "../other.json#/wrong/prefix",
                                          &out_rb));
    ASSERT(out_rb == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              find_component_media_type(&spec, "../other.json#/wrong/prefix",
                                        &out_mt));
    ASSERT(out_mt == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_link(&spec, "../other.json#/wrong/prefix", &out_lnk));
    ASSERT(out_lnk == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_callback(&spec, "../other.json#/wrong/prefix", &out_cb));
    ASSERT(out_cb == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              find_component_path_item(&spec, "../other.json#/wrong/prefix",
                                       &out_path));
    ASSERT(out_path == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        find_component_example(&spec, "../other.json#/wrong/prefix", &out_ex));
    ASSERT(out_ex == NULL);

    /* E. External unescape failure */
    {
      int k;
      for (k = 1; k <= 15; ++k) {
        g_cdd_alloc_fail = k;
        ASSERT_EQ(CDD_C_SUCCESS,
                  find_component_media_type(
                      &spec, "../other.json#/components/mediaTypes/app~1json",
                      &out_mt));
        g_cdd_alloc_fail = k;
        ASSERT_EQ(CDD_C_SUCCESS,
                  find_component_path_item(
                      &spec, "../other.json#/components/pathItems/~1myPath",
                      &out_path));
      }
      g_cdd_alloc_fail = 0;
    }

    /* F. Null names in component arrays */
    {
      param_names[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                find_component_parameter(
                    &spec, "#/components/parameters/MyParam", &out_param));
      ASSERT(out_param == NULL);
      param_names[0] = cdd_test_mut_str("MyParam");

      resp_names[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                find_component_response(&spec, "#/components/responses/MyResp",
                                        &out_resp));
      ASSERT(out_resp == NULL);
      resp_names[0] = cdd_test_mut_str("MyResp");

      hdr_names[0] = NULL;
      ASSERT_EQ(
          CDD_C_SUCCESS,
          find_component_header(&spec, "#/components/headers/MyHdr", &out_hdr));
      ASSERT(out_hdr == NULL);
      hdr_names[0] = cdd_test_mut_str("MyHdr");

      rb_names[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                find_component_request_body(
                    &spec, "#/components/requestBodies/MyBody", &out_rb));
      ASSERT(out_rb == NULL);
      rb_names[0] = cdd_test_mut_str("MyBody");

      mt_names[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                find_component_media_type(
                    &spec, "#/components/mediaTypes/app~1json", &out_mt));
      ASSERT(out_mt == NULL);
      mt_names[0] = cdd_test_mut_str("app/json");

      lnk.name = NULL;
      ASSERT_EQ(
          CDD_C_SUCCESS,
          find_component_link(&spec, "#/components/links/MyLink", &out_lnk));
      ASSERT(out_lnk == NULL);
      lnk.name = cdd_test_mut_str("MyLink");

      cb.name = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                find_component_callback(&spec, "#/components/callbacks/MyCb",
                                        &out_cb));
      ASSERT(out_cb == NULL);
      cb.name = cdd_test_mut_str("MyCb");

      ex_names[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                find_component_example(&spec, "#/components/examples/MyParam",
                                       &out_ex));
      ASSERT(out_ex == NULL);
      ex_names[0] = cdd_test_mut_str("MyParam");

      path_names[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                find_component_path_item(
                    &spec, "#/components/pathItems/~1myPath", &out_path));
      ASSERT(out_path == NULL);
      path_names[0] = cdd_test_mut_str("/myPath");

      spec.component_path_item_names = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                find_component_path_item(
                    &spec, "#/components/pathItems/~1myPath", &out_path));
      ASSERT(out_path == NULL);
      spec.component_path_item_names = path_names;
    }

    g_cdd_last_alloc_err = openapi_doc_registry_free(&reg);
    spec.document_uri = NULL;
    spec.doc_registry = NULL;
  }
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_FIND_COMPONENTS_H */
