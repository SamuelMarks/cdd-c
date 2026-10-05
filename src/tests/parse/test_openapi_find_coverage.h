/**
 * @file test_openapi_find_coverage.h
 * @brief Comprehensive 100% coverage tests for openapi_find.c and
 * find_component_example.
 */

#ifndef TEST_OPENAPI_FIND_COVERAGE_H
#define TEST_OPENAPI_FIND_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
#include "openapi/parse/openapi_test_helpers.h"
/* clang-format on */

TEST test_openapi_find_all_components(void) {
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

  param_names[0] = (char *)(size_t) "MyParam";
  spec.n_component_parameters = 1;
  spec.component_parameter_names = param_names;
  spec.component_parameters = &param;

  resp_names[0] = (char *)(size_t) "MyResp";
  spec.n_component_responses = 1;
  spec.component_response_names = resp_names;
  spec.component_responses = &resp;

  hdr_names[0] = (char *)(size_t) "MyHdr";
  spec.n_component_headers = 1;
  spec.component_header_names = hdr_names;
  spec.component_headers = &hdr;

  rb_names[0] = (char *)(size_t) "MyBody";
  spec.n_component_request_bodies = 1;
  spec.component_request_body_names = rb_names;
  spec.component_request_bodies = &rb;

  mt_names[0] = (char *)(size_t) "app/json";
  spec.n_component_media_types = 1;
  spec.component_media_type_names = mt_names;
  spec.component_media_types = &mt;

  lnk.name = (char *)(size_t) "MyLink";
  spec.n_component_links = 1;
  spec.component_links = &lnk;

  cb.name = (char *)(size_t) "MyCb";
  spec.n_component_callbacks = 1;
  spec.component_callbacks = &cb;

  path_names[0] = (char *)(size_t) "/myPath";
  spec.n_component_path_items = 1;
  spec.component_path_item_names = path_names;
  spec.component_path_items = &path;

  ex_names[0] = (char *)(size_t) "MyParam";
  spec.n_component_examples = 1;
  spec.component_example_names = ex_names;
  spec.component_examples = &ex;

  /* 1. NULL checks */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_parameter(NULL, NULL, &out_param));
  ASSERT(out_param == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_response(NULL, NULL, &out_resp));
  ASSERT(out_resp == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_header(NULL, NULL, &out_hdr));
  ASSERT(out_hdr == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_request_body(NULL, NULL, &out_rb));
  ASSERT(out_rb == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_media_type(NULL, NULL, &out_mt));
  ASSERT(out_mt == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_component_link(NULL, NULL, &out_lnk));
  ASSERT(out_lnk == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_callback(NULL, NULL, &out_cb));
  ASSERT(out_cb == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_path_item(NULL, NULL, &out_path));
  ASSERT(out_path == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_example(NULL, NULL, &out_ex));
  ASSERT(out_ex == NULL);

  /* 2. Bad prefix / not found */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_component_parameter(
                               &spec, "#/wrong/prefix", &out_param));
  ASSERT(out_param == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_parameter(
                &spec, "#/components/parameters/NotFound", &out_param));
  ASSERT(out_param == NULL);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_component_response(
                               &spec, "#/wrong/prefix", &out_resp));
  ASSERT(out_resp == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_response(
                &spec, "#/components/responses/NotFound", &out_resp));
  ASSERT(out_resp == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_header(&spec, "#/wrong/prefix", &out_hdr));
  ASSERT(out_hdr == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_header(
                &spec, "#/components/headers/NotFound", &out_hdr));
  ASSERT(out_hdr == NULL);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_component_request_body(
                               &spec, "#/wrong/prefix", &out_rb));
  ASSERT(out_rb == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_request_body(
                &spec, "#/components/requestBodies/NotFound", &out_rb));
  ASSERT(out_rb == NULL);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_component_media_type(
                               &spec, "#/wrong/prefix", &out_mt));
  ASSERT(out_mt == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_media_type(
                &spec, "#/components/mediaTypes/NotFound", &out_mt));
  ASSERT(out_mt == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_link(&spec, "#/wrong/prefix", &out_lnk));
  ASSERT(out_lnk == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_component_link(
                               &spec, "#/components/links/NotFound", &out_lnk));
  ASSERT(out_lnk == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_callback(&spec, "#/wrong/prefix", &out_cb));
  ASSERT(out_cb == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_callback(
                &spec, "#/components/callbacks/NotFound", &out_cb));
  ASSERT(out_cb == NULL);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_component_path_item(
                               &spec, "#/wrong/prefix", &out_path));
  ASSERT(out_path == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_path_item(
                &spec, "#/components/pathItems/NotFound", &out_path));
  ASSERT(out_path == NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_example(&spec, "#/wrong/prefix", &out_ex));
  ASSERT(out_ex == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_example(
                &spec, "#/components/examples/NotFound", &out_ex));
  ASSERT(out_ex == NULL);

  /* 3. Found matches */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_parameter(
                &spec, "#/components/parameters/MyParam", &out_param));
  ASSERT(out_param == &param);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_response(
                &spec, "#/components/responses/MyResp", &out_resp));
  ASSERT(out_resp == &resp);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_component_header(
                               &spec, "#/components/headers/MyHdr", &out_hdr));
  ASSERT(out_hdr == &hdr);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_request_body(
                &spec, "#/components/requestBodies/MyBody", &out_rb));
  ASSERT(out_rb == &rb);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_media_type(
                &spec, "#/components/mediaTypes/app~1json", &out_mt));
  ASSERT(out_mt == &mt);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_component_link(
                               &spec, "#/components/links/MyLink", &out_lnk));
  ASSERT(out_lnk == &lnk);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_component_callback(
                               &spec, "#/components/callbacks/MyCb", &out_cb));
  ASSERT(out_cb == &cb);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_path_item(
                &spec, "#/components/pathItems/~1myPath", &out_path));
  ASSERT(out_path == &path);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_example(
                &spec, "#/components/examples/MyParam", &out_ex));
  ASSERT(out_ex == &ex);

  /* 4. External ref with registry to exercise resolved.resolved_ref free on
   * match and non-match */
  {
    struct OpenAPI_DocRegistry reg;
    struct OpenAPI_Spec other_spec;

    memset(&reg, 0, sizeof(reg));
    memset(&other_spec, 0, sizeof(other_spec));
    openapi_doc_registry_init(&reg);

    other_spec.document_uri = (char *)(size_t) "http://example.com/other.json";
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

    openapi_doc_registry_add(&reg, &other_spec);

    spec.document_uri = (char *)(size_t) "http://example.com/base/spec.json";
    spec.doc_registry = &reg;

    /* A. Match found in external target */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_test_find_component_parameter(
            &spec, "../other.json#/components/parameters/MyParam", &out_param));
    ASSERT(out_param == &param);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_test_find_component_response(
            &spec, "../other.json#/components/responses/MyResp", &out_resp));
    ASSERT(out_resp == &resp);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_header(
                  &spec, "../other.json#/components/headers/MyHdr", &out_hdr));
    ASSERT(out_hdr == &hdr);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_test_find_component_request_body(
            &spec, "../other.json#/components/requestBodies/MyBody", &out_rb));
    ASSERT(out_rb == &rb);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_test_find_component_media_type(
            &spec, "../other.json#/components/mediaTypes/app~1json", &out_mt));
    ASSERT(out_mt == &mt);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_link(
                  &spec, "../other.json#/components/links/MyLink", &out_lnk));
    ASSERT(out_lnk == &lnk);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_callback(
                  &spec, "../other.json#/components/callbacks/MyCb", &out_cb));
    ASSERT(out_cb == &cb);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_test_find_component_path_item(
            &spec, "../other.json#/components/pathItems/~1myPath", &out_path));
    ASSERT(out_path == &path);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_test_find_component_example(
            &spec, "../other.json#/components/examples/MyParam", &out_ex));
    ASSERT(out_ex == &ex);

    /* B. Not found in external target */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_parameter(
                  &spec, "../other.json#/components/parameters/NotFound",
                  &out_param));
    ASSERT(out_param == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_test_find_component_response(
            &spec, "../other.json#/components/responses/NotFound", &out_resp));
    ASSERT(out_resp == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_test_find_component_header(
            &spec, "../other.json#/components/headers/NotFound", &out_hdr));
    ASSERT(out_hdr == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_request_body(
                  &spec, "../other.json#/components/requestBodies/NotFound",
                  &out_rb));
    ASSERT(out_rb == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_test_find_component_media_type(
            &spec, "../other.json#/components/mediaTypes/NotFound", &out_mt));
    ASSERT(out_mt == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_link(
                  &spec, "../other.json#/components/links/NotFound", &out_lnk));
    ASSERT(out_lnk == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_test_find_component_callback(
            &spec, "../other.json#/components/callbacks/NotFound", &out_cb));
    ASSERT(out_cb == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_test_find_component_path_item(
            &spec, "../other.json#/components/pathItems/NotFound", &out_path));
    ASSERT(out_path == NULL);
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_test_find_component_example(
            &spec, "../other.json#/components/examples/NotFound", &out_ex));
    ASSERT(out_ex == NULL);

    /* C. Unescape failure */
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_media_type(
                  &spec, "#/components/mediaTypes/app~1json", &out_mt));
    ASSERT(out_mt == NULL);
    g_cdd_alloc_fail = 0;

    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_path_item(
                  &spec, "#/components/pathItems/~1myPath", &out_path));
    ASSERT(out_path == NULL);
    g_cdd_alloc_fail = 0;

    /* D. External ref with wrong prefix */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_parameter(
                  &spec, "../other.json#/wrong/prefix", &out_param));
    ASSERT(out_param == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_response(
                  &spec, "../other.json#/wrong/prefix", &out_resp));
    ASSERT(out_resp == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_header(
                  &spec, "../other.json#/wrong/prefix", &out_hdr));
    ASSERT(out_hdr == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_request_body(
                  &spec, "../other.json#/wrong/prefix", &out_rb));
    ASSERT(out_rb == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_media_type(
                  &spec, "../other.json#/wrong/prefix", &out_mt));
    ASSERT(out_mt == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_link(&spec, "../other.json#/wrong/prefix",
                                           &out_lnk));
    ASSERT(out_lnk == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_callback(
                  &spec, "../other.json#/wrong/prefix", &out_cb));
    ASSERT(out_cb == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_path_item(
                  &spec, "../other.json#/wrong/prefix", &out_path));
    ASSERT(out_path == NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_find_component_example(
                  &spec, "../other.json#/wrong/prefix", &out_ex));
    ASSERT(out_ex == NULL);

    /* E. External unescape failure */
    {
      int k;
      for (k = 1; k <= 15; ++k) {
        g_cdd_alloc_fail = k;
        cdd_test_find_component_media_type(
            &spec, "../other.json#/components/mediaTypes/app~1json", &out_mt);
        g_cdd_alloc_fail = k;
        cdd_test_find_component_path_item(
            &spec, "../other.json#/components/pathItems/~1myPath", &out_path);
      }
      g_cdd_alloc_fail = 0;
    }

    /* F. Null names in component arrays */
    {
      param_names[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_find_component_parameter(
                    &spec, "#/components/parameters/MyParam", &out_param));
      ASSERT(out_param == NULL);
      param_names[0] = (char *)(size_t) "MyParam";

      resp_names[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_find_component_response(
                    &spec, "#/components/responses/MyResp", &out_resp));
      ASSERT(out_resp == NULL);
      resp_names[0] = (char *)(size_t) "MyResp";

      hdr_names[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_find_component_header(
                    &spec, "#/components/headers/MyHdr", &out_hdr));
      ASSERT(out_hdr == NULL);
      hdr_names[0] = (char *)(size_t) "MyHdr";

      rb_names[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_find_component_request_body(
                    &spec, "#/components/requestBodies/MyBody", &out_rb));
      ASSERT(out_rb == NULL);
      rb_names[0] = (char *)(size_t) "MyBody";

      mt_names[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_find_component_media_type(
                    &spec, "#/components/mediaTypes/app~1json", &out_mt));
      ASSERT(out_mt == NULL);
      mt_names[0] = (char *)(size_t) "app/json";

      lnk.name = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_find_component_link(&spec, "#/components/links/MyLink",
                                             &out_lnk));
      ASSERT(out_lnk == NULL);
      lnk.name = (char *)(size_t) "MyLink";

      cb.name = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_find_component_callback(
                    &spec, "#/components/callbacks/MyCb", &out_cb));
      ASSERT(out_cb == NULL);
      cb.name = (char *)(size_t) "MyCb";

      ex_names[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_find_component_example(
                    &spec, "#/components/examples/MyParam", &out_ex));
      ASSERT(out_ex == NULL);
      ex_names[0] = (char *)(size_t) "MyParam";

      path_names[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_find_component_path_item(
                    &spec, "#/components/pathItems/~1myPath", &out_path));
      ASSERT(out_path == NULL);
      path_names[0] = (char *)(size_t) "/myPath";

      spec.component_path_item_names = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_find_component_path_item(
                    &spec, "#/components/pathItems/~1myPath", &out_path));
      ASSERT(out_path == NULL);
      spec.component_path_item_names = path_names;
    }

    openapi_doc_registry_free(&reg);
    spec.document_uri = NULL;
    spec.doc_registry = NULL;
  }

  PASS();
}

TEST test_openapi_schemas_all_branches(void) {
  int nullable = 0;
  char *type_val = NULL;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  JSON_Array *ja = NULL;
  struct SchemaConstraintTarget target;
  struct OpenAPI_Any example_val;
  int example_set = 0;
  int has_min = 0, exclusive_min = 0;
  double min_val = 0.0;
  int has_max = 0, exclusive_max = 0;
  double max_val = 0.0;
  int has_min_len = 0;
  size_t min_len = 0;
  int has_max_len = 0;
  size_t max_len = 0;
  char *pattern = NULL;
  int has_min_items = 0;
  size_t min_items = 0;
  int has_max_items = 0;
  size_t max_items = 0;
  int unique_items = 0;
  char **enum_out = NULL;
  size_t enum_count = 0;
  char **copied = NULL;
  size_t copied_count = 0;
  char *sample_src[3];
  struct OpenAPI_SchemaRef *schema_refs = NULL;
  size_t schema_refs_count = 0;
  struct OpenAPI_SchemaRef *single_ref = NULL;
  struct OpenAPI_Spec spec;

  memset(&spec, 0, sizeof(spec));

  /* 1. parse_schema_type */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_schema_type(NULL, &nullable, &type_val));
  ASSERT_EQ(NULL, type_val);
  ASSERT_EQ(0, nullable);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_type(NULL, NULL, &type_val));
  ASSERT_EQ(NULL, type_val);

  jv = json_parse_string("{\"type\": \"integer\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_schema_type(jo, &nullable, &type_val));
  ASSERT_STR_EQ("integer", type_val);
  ASSERT_EQ(0, nullable);
  json_value_free(jv);

  jv = json_parse_string("{\"type\": 123}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_schema_type(jo, &nullable, &type_val));
  ASSERT_EQ(NULL, type_val);
  json_value_free(jv);

  jv = json_parse_string("{\"type\": [\"string\", \"null\"]}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_schema_type(jo, &nullable, &type_val));
  ASSERT_STR_EQ("string", type_val);
  ASSERT_EQ(1, nullable);
  json_value_free(jv);

  jv = json_parse_string("{\"type\": [\"null\"]}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_schema_type(jo, &nullable, &type_val));
  ASSERT_STR_EQ("null", type_val);
  ASSERT_EQ(1, nullable);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_type(jo, NULL, &type_val));
  ASSERT_EQ(NULL, type_val);
  json_value_free(jv);

  jv = json_parse_string("{\"type\": []}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_schema_type(jo, &nullable, &type_val));
  ASSERT_EQ(NULL, type_val);
  ASSERT_EQ(0, nullable);
  json_value_free(jv);

  jv = json_parse_string("{\"type\": [123, \"boolean\"]}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_type(jo, NULL, &type_val));
  ASSERT_STR_EQ("boolean", type_val);
  json_value_free(jv);

  /* 2. parse_schema_constraints */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_constraints(NULL, &target));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_constraints(NULL, NULL));

  memset(&target, 0, sizeof(target));
  memset(&example_val, 0, sizeof(example_val));
  target.example = &example_val;
  target.example_set = &example_set;
  target.has_min = &has_min;
  target.min_val = &min_val;
  target.exclusive_min = &exclusive_min;
  target.has_max = &has_max;
  target.max_val = &max_val;
  target.exclusive_max = &exclusive_max;
  target.has_min_len = &has_min_len;
  target.min_len = &min_len;
  target.has_max_len = &has_max_len;
  target.max_len = &max_len;
  target.pattern = &pattern;
  target.has_min_items = &has_min_items;
  target.min_items = &min_items;
  target.has_max_items = &has_max_items;
  target.max_items = &max_items;
  target.unique_items = &unique_items;

  jv = json_parse_string(
      "{\"example\": \"ex\", \"minimum\": 1.5, \"exclusiveMinimum\": true, "
      "\"maximum\": 10.0, \"exclusiveMaximum\": false, \"minLength\": 2, "
      "\"maxLength\": 20, \"pattern\": \"^[a-z]+$\", \"minItems\": 1, "
      "\"maxItems\": 5, \"uniqueItems\": true}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_constraints(jo, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_constraints(jo, &target));
  ASSERT_EQ(1, *target.has_min);
  ASSERT_EQ(1, *target.exclusive_min);
  ASSERT_EQ(1, *target.has_max);
  ASSERT_EQ(0, *target.exclusive_max);
  ASSERT_EQ(1, *target.has_min_len);
  ASSERT_EQ(1, *target.has_max_len);
  ASSERT(pattern != NULL);
  free(pattern);
  pattern = NULL;

  target.example = &example_val;
  target.example_set = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_constraints(jo, &target));
  target.example = NULL;
  target.example_set = &example_set;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_constraints(jo, &target));
  target.example = &example_val;

  json_value_free(jv);

  /* Test exclusiveMinimum / exclusiveMaximum as numeric (OpenAPI 3.1) */
  jv = json_parse_string(
      "{\"exclusiveMinimum\": 0.5, \"exclusiveMaximum\": 99.5}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_constraints(jo, &target));
  ASSERT_EQ(1, *target.exclusive_min);
  ASSERT_EQ(1, *target.exclusive_max);
  json_value_free(jv);

  /* Test exclusiveMinimum / exclusiveMaximum false */
  jv = json_parse_string("{\"minimum\": 1.0, \"exclusiveMinimum\": false, "
                         "\"maximum\": 10.0, \"exclusiveMaximum\": false}");
  jo = json_value_get_object(jv);
  target.exclusive_min = &exclusive_min;
  target.exclusive_max = &exclusive_max;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_constraints(jo, &target));
  json_value_free(jv);

  /* Test exclusiveMinimum / exclusiveMaximum with target exclusive_min/max NULL
   */
  jv = json_parse_string("{\"minimum\": 1.0, \"exclusiveMinimum\": 0.5, "
                         "\"maximum\": 10.0, \"exclusiveMaximum\": 9.5}");
  jo = json_value_get_object(jv);
  target.exclusive_min = NULL;
  target.exclusive_max = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_constraints(jo, &target));
  json_value_free(jv);

  jv = json_parse_string("{\"minimum\": 1.0, \"exclusiveMinimum\": true, "
                         "\"maximum\": 10.0, \"exclusiveMaximum\": true}");
  jo = json_value_get_object(jv);
  target.exclusive_min = NULL;
  target.exclusive_max = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_constraints(jo, &target));
  json_value_free(jv);

  /* Partial targets (one pointer non-null, one null) */
  memset(&target, 0, sizeof(target));
  target.has_min = &has_min;
  target.has_max = &has_max;
  target.has_min_len = &has_min_len;
  target.has_max_len = &has_max_len;
  target.has_min_items = &has_min_items;
  target.has_max_items = &has_max_items;
  jv = json_parse_string("{\"minimum\": 1, \"maximum\": 2, \"minLength\": 1, "
                         "\"maxLength\": 5, \"minItems\": 1, \"maxItems\": 2}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_constraints(jo, &target));
  json_value_free(jv);

  memset(&target, 0, sizeof(target));
  target.min_val = &min_val;
  target.max_val = &max_val;
  target.min_len = &min_len;
  target.max_len = &max_len;
  target.min_items = &min_items;
  target.max_items = &max_items;
  jv = json_parse_string("{\"minimum\": 1, \"maximum\": 2, \"minLength\": 1, "
                         "\"maxLength\": 5, \"minItems\": 1, \"maxItems\": 2}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_constraints(jo, &target));
  json_value_free(jv);

  /* example parse failure */
  memset(&target, 0, sizeof(target));
  target.example = &example_val;
  target.example_set = &example_set;
  jv = json_parse_string("{\"example\": \"ex\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_schema_constraints(jo, &target));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* pattern OOM */
  jv = json_parse_string("{\"pattern\": \"test\"}");
  jo = json_value_get_object(jv);
  target.pattern = &pattern;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_schema_constraints(jo, &target));
  g_cdd_strdup_fail = 0;
  target.pattern = NULL;
  json_value_free(jv);

  /* Target with null internal pointers */
  memset(&target, 0, sizeof(target));
  jv =
      json_parse_string("{\"minimum\": 1, \"maximum\": 2, \"minLength\": 1, "
                        "\"maxLength\": 5, \"pattern\": \"abc\", \"minItems\": "
                        "1, \"maxItems\": 2, \"uniqueItems\": true}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_constraints(jo, &target));
  json_value_free(jv);

  /* 3. parse_string_enum_array */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_string_enum_array(NULL, NULL, &enum_count));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_string_enum_array(NULL, &enum_out, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_string_enum_array(NULL, &enum_out, &enum_count));
  ASSERT_EQ(NULL, enum_out);
  ASSERT_EQ(0, enum_count);

  jv = json_parse_string("[]");
  ja = json_value_get_array(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_string_enum_array(ja, &enum_out, &enum_count));
  ASSERT_EQ(NULL, enum_out);
  json_value_free(jv);

  jv = json_parse_string("[\"a\", 123]");
  ja = json_value_get_array(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_string_enum_array(ja, &enum_out, &enum_count));
  ASSERT_EQ(NULL, enum_out);
  json_value_free(jv);

  jv = json_parse_string("[\"a\", \"b\"]");
  ja = json_value_get_array(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_string_enum_array(ja, &enum_out, &enum_count));
  g_cdd_alloc_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_string_enum_array(ja, &enum_out, &enum_count));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_string_enum_array(ja, &enum_out, &enum_count));
  ASSERT_EQ(2, enum_count);
  ASSERT_STR_EQ("a", enum_out[0]);
  ASSERT_STR_EQ("b", enum_out[1]);
  free(enum_out[0]);
  free(enum_out[1]);
  free(enum_out);
  enum_out = NULL;
  json_value_free(jv);

  /* 4. copy_string_array */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_copy_string_array(NULL, &copied_count, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_string_array(&copied, NULL, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_copy_string_array(&copied, &copied_count, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_copy_string_array(&copied, &copied_count, sample_src, 0));
  ASSERT_EQ(NULL, copied);
  ASSERT_EQ(0, copied_count);

  sample_src[0] = (char *)(size_t) "str1";
  sample_src[1] = NULL;
  sample_src[2] = (char *)(size_t) "str2";

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_copy_string_array(&copied, &copied_count, sample_src, 3));
  g_cdd_alloc_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_copy_string_array(&copied, &copied_count, sample_src, 3));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_copy_string_array(&copied, &copied_count, sample_src, 3));
  ASSERT_EQ(3, copied_count);
  ASSERT_STR_EQ("str1", copied[0]);
  ASSERT_EQ(NULL, copied[1]);
  ASSERT_STR_EQ("str2", copied[2]);
  free(copied[0]);
  free(copied[2]);
  free(copied);
  copied = NULL;

  /* 5. schema_is_string_enum_only */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_schema_is_string_enum_only(NULL));
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_schema_is_string_enum_only(jo));
  json_value_free(jv);

  jv = json_parse_string("{\"enum\": []}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_schema_is_string_enum_only(jo));
  json_value_free(jv);

  jv = json_parse_string("{\"enum\": [\"a\"], \"type\": \"integer\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_schema_is_string_enum_only(jo));
  json_value_free(jv);

  jv = json_parse_string("{\"enum\": [123], \"type\": \"string\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_schema_is_string_enum_only(jo));
  json_value_free(jv);

  jv = json_parse_string("{\"enum\": [\"x\", \"y\"], \"type\": \"string\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_schema_is_string_enum_only(jo));
  json_value_free(jv);

  /* 6. schema_is_struct_compatible */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_schema_is_struct_compatible(NULL, NULL));
  jv = json_parse_string("true");
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_schema_is_struct_compatible(jv, (JSON_Object *)(size_t)1));
  json_value_free(jv);

  jv = json_parse_string("{\"enum\": [\"x\"], \"type\": \"string\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_schema_is_struct_compatible(jv, jo));
  json_value_free(jv);

  jv = json_parse_string("{\"type\": \"object\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(1, cdd_test_schema_is_struct_compatible(jv, jo));
  json_value_free(jv);

  jv = json_parse_string("{\"type\": \"string\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(0, cdd_test_schema_is_struct_compatible(jv, jo));
  json_value_free(jv);

  jv = json_parse_string("{\"properties\": {}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_schema_is_struct_compatible(jv, jo));
  json_value_free(jv);

  jv = json_parse_string("{\"allOf\": []}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_schema_is_struct_compatible(jv, jo));
  json_value_free(jv);

  jv = json_parse_string("{\"anyOf\": []}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_schema_is_struct_compatible(jv, jo));
  json_value_free(jv);

  jv = json_parse_string("{\"oneOf\": []}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_schema_is_struct_compatible(jv, jo));
  json_value_free(jv);

  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_schema_is_struct_compatible(jv, jo));
  json_value_free(jv);

  /* 7. schema_has_composition */
  ASSERT_EQ(0, cdd_test_schema_has_composition(NULL));
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(0, cdd_test_schema_has_composition(jo));
  json_value_free(jv);

  jv = json_parse_string("{\"allOf\": []}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(0, cdd_test_schema_has_composition(jo));
  json_value_free(jv);

  /* 8. parse_schema_array_ref */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_schema_array_ref(NULL, NULL, NULL, &spec));
  jv = json_parse_string("[]");
  ja = json_value_get_array(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_array_ref(
                               ja, &schema_refs, &schema_refs_count, &spec));
  ASSERT_EQ(NULL, schema_refs);
  ASSERT_EQ(0, schema_refs_count);
  json_value_free(jv);

  jv = json_parse_string("[{\"type\": \"string\"}, {\"type\": \"integer\"}]");
  ja = json_value_get_array(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_schema_array_ref(ja, &schema_refs,
                                            &schema_refs_count, &spec));
  g_cdd_alloc_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_schema_array_ref(ja, &schema_refs,
                                            &schema_refs_count, &spec));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_schema_array_ref(
                               ja, &schema_refs, &schema_refs_count, &spec));
  ASSERT_EQ(2, schema_refs_count);
  ASSERT(schema_refs != NULL);
  cdd_test_free_schema_ref_content(&schema_refs[0]);
  cdd_test_free_schema_ref_content(&schema_refs[1]);
  free(schema_refs);
  schema_refs = NULL;

  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      cdd_test_parse_schema_array_ref(ja, NULL, &schema_refs_count, &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_schema_array_ref(ja, &schema_refs, NULL, &spec));
  json_value_free(jv);

  /* 9. parse_schema_ref_ptr */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_schema_ref_ptr(NULL, NULL, &spec));
  jv = json_parse_string("{\"type\": \"string\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_schema_ref_ptr(jo, NULL, &spec));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_schema_ref_ptr(jo, &single_ref, &spec));
  g_cdd_alloc_fail = 0;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_schema_ref_ptr(jo, &single_ref, &spec));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_schema_ref_ptr(jo, &single_ref, &spec));
  ASSERT(single_ref != NULL);
  cdd_test_free_schema_ref_content(single_ref);
  free(single_ref);
  single_ref = NULL;
  json_value_free(jv);

  PASS();
}

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

TEST test_openapi_examples_all_branches(void) {
  struct OpenAPI_Example ex_src;
  struct OpenAPI_Example ex_dst;
  struct OpenAPI_Example *out_ex = NULL;
  struct OpenAPI_Example *examples_arr = NULL;
  size_t n_examples = 0;
  struct OpenAPI_OAuthScope *scopes = NULL;
  size_t n_scopes = 0;
  struct OpenAPI_SecurityScheme sec;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Example comp_ex;
  char *comp_name = (char *)(size_t) "comp_ex";
  struct OpenAPI_Any example_any;
  int example_set = 0;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;

  memset(&spec, 0, sizeof(spec));
  memset(&sec, 0, sizeof(sec));
  memset(&ex_src, 0, sizeof(ex_src));
  memset(&ex_dst, 0, sizeof(ex_dst));
  memset(&comp_ex, 0, sizeof(comp_ex));
  memset(&example_any, 0, sizeof(example_any));

  comp_ex.summary = (char *)(size_t) "comp_summary";
  spec.component_examples = &comp_ex;
  spec.component_example_names = &comp_name;
  spec.n_component_examples = 1;

  free_test_oauth_scopes(NULL, 0);
  free_test_security_scheme_flows(NULL);
  free_test_security_scheme_flows(&sec);

  /* 1. copy_example_fields */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_example_fields(NULL, &ex_src));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_example_fields(&ex_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_example_fields(NULL, NULL));

  ex_src.name = (char *)(size_t) "ex1";
  ex_src.ref = (char *)(size_t) "#/ref";
  ex_src.extensions_json = (char *)(size_t) "{\"x-e\": 1}";
  ex_src.summary = (char *)(size_t) "sum";
  ex_src.description = (char *)(size_t) "desc";
  ex_src.data_value_set = 1;
  ex_src.value_set = 1;
  ex_src.serialized_value = (char *)(size_t) "serialized";
  ex_src.external_value = (char *)(size_t) "http://ext";

  /* copy all fields successfully */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_example_fields(&ex_dst, &ex_src));
  /* copy when dst fields are already populated */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_example_fields(&ex_dst, &ex_src));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  /* Test OOM on each strdup in copy_example_fields */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_example_fields(&ex_dst, &ex_src));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ex_dst.name = strdup("name");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_example_fields(&ex_dst, &ex_src));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ex_dst.name = strdup("name");
  ex_dst.ref = strdup("ref");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_example_fields(&ex_dst, &ex_src));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ex_dst.name = strdup("name");
  ex_dst.ref = strdup("ref");
  ex_dst.extensions_json = strdup("{}");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_example_fields(&ex_dst, &ex_src));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ex_dst.name = strdup("name");
  ex_dst.ref = strdup("ref");
  ex_dst.extensions_json = strdup("{}");
  ex_dst.summary = strdup("sum");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_example_fields(&ex_dst, &ex_src));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ex_dst.name = strdup("name");
  ex_dst.ref = strdup("ref");
  ex_dst.extensions_json = strdup("{}");
  ex_dst.summary = strdup("sum");
  ex_dst.description = strdup("desc");
  ex_dst.data_value_set = 1;
  ex_dst.value_set = 1;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_example_fields(&ex_dst, &ex_src));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ex_dst.name = strdup("name");
  ex_dst.ref = strdup("ref");
  ex_dst.extensions_json = strdup("{}");
  ex_dst.summary = strdup("sum");
  ex_dst.description = strdup("desc");
  ex_dst.data_value_set = 1;
  ex_dst.value_set = 1;
  ex_dst.serialized_value = strdup("ser");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_example_fields(&ex_dst, &ex_src));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_strdup_fail = 0;

  memset(&ex_src, 0, sizeof(ex_src));
  memset(&ex_dst, 0, sizeof(ex_dst));
  ex_src.data_value_set = 1;
  ex_src.data_value.type = OA_ANY_STRING;
  ex_src.data_value.string = (char *)(size_t) "foo";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_example_fields(&ex_dst, &ex_src));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_strdup_fail = 0;

  memset(&ex_src, 0, sizeof(ex_src));
  memset(&ex_dst, 0, sizeof(ex_dst));
  ex_src.value_set = 1;
  ex_src.value.type = OA_ANY_STRING;
  ex_src.value.string = (char *)(size_t) "bar";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_example_fields(&ex_dst, &ex_src));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_strdup_fail = 0;

  /* 2. find_component_example */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_component_example(
                               NULL, "#/components/examples/comp_ex", &out_ex));
  ASSERT_EQ(NULL, out_ex);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_example(&spec, NULL, &out_ex));
  ASSERT_EQ(NULL, out_ex);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_example(&spec, "invalid_prefix", &out_ex));
  ASSERT_EQ(NULL, out_ex);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_example(
                &spec, "#/components/examples/nonexistent", &out_ex));
  ASSERT_EQ(NULL, out_ex);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_example(
                &spec, "#/components/examples/comp_ex", &out_ex));
  ASSERT_EQ(&comp_ex, out_ex);

  /* json_pointer_unescape failure in find_component_example */
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_example(
                &spec, "#/components/examples/comp_ex", &out_ex));
  ASSERT_EQ(NULL, out_ex);
  g_cdd_alloc_fail = 0;

  /* spec without component_example_names */
  out_ex = NULL;
  spec.component_example_names = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_component_example(
                &spec, "#/components/examples/comp_ex", &out_ex));
  ASSERT_EQ(NULL, out_ex);
  spec.component_example_names = &comp_name;

  /* 3. parse_example_object */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_example_object(NULL, "ex", &ex_dst, &spec, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_example_object(NULL, NULL, NULL, &spec, 0));

  /* ex_obj != NULL, out == NULL */
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_example_object(jo, "ex", NULL, &spec, 0));
  json_value_free(jv);

  /* name OOM */
  jv = json_parse_string("{\"value\": \"v\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_example_object(jo, "name", &ex_dst, &spec, 0));
  g_cdd_strdup_fail = 0;
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  json_value_free(jv);

  /* $ref with and without resolve_refs */
  jv = json_parse_string("{\"$ref\": \"#/components/examples/comp_ex\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_example_object(jo, "name", &ex_dst, &spec, 1));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_example_object(jo, NULL, &ex_dst, NULL, 1));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  /* $ref OOM */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_example_object(jo, NULL, &ex_dst, &spec, 1));
  g_cdd_strdup_fail = 0;
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  /* $ref copy failure on OOM */
  g_cdd_strdup_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_example_object(jo, "name", &ex_dst, &spec, 1));
  g_cdd_strdup_fail = 0;
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  json_value_free(jv);

  /* serializedValue OOM */
  jv = json_parse_string("{\"serializedValue\": \"ser\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  g_cdd_strdup_fail = 0;
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  json_value_free(jv);

  /* extensions failure */
  jv = json_parse_string("{\"value\": 1, \"x-ext\": 1}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  g_cdd_strdup_fail = 0;
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  json_value_free(jv);

  /* dataValue failure */
  jv = json_parse_string("{\"dataValue\": \"str\"}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  g_cdd_strdup_fail = 0;
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  json_value_free(jv);

  /* summary, description, externalValue */
  jv = json_parse_string("{\"summary\": \"s\", \"description\": \"d\", "
                         "\"externalValue\": \"http://ext\"}");
  jo = json_value_get_object(jv);

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));

  g_cdd_strdup_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  g_cdd_strdup_fail = 0;

  /* Valid externalValue example */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_example_object(jo, "ex1", &ex_dst, &spec, 0));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  json_value_free(jv);

  /* dataValue and value parsing */
  jv = json_parse_string("{\"value\": \"v\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_example_object(jo, "ex2", &ex_dst, &spec, 0));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  json_value_free(jv);

  jv = json_parse_string("{\"dataValue\": \"dv\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_example_object(jo, "ex3", &ex_dst, &spec, 0));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  json_value_free(jv);

  /* Validation failure: both value and externalValue */
  jv = json_parse_string(
      "{\"value\": \"v\", \"externalValue\": \"http://ext\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_example_object(jo, NULL, &ex_dst, &spec, 0));
  cdd_test_free_example(&ex_dst);
  memset(&ex_dst, 0, sizeof(ex_dst));
  json_value_free(jv);

  /* 4. parse_examples_object */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_examples_object(NULL, NULL, &n_examples, &spec, 0));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_examples_object(NULL, &examples_arr,
                                                          NULL, &spec, 0));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_examples_object(
                               NULL, &examples_arr, &n_examples, &spec, 0));
  ASSERT_EQ(NULL, examples_arr);
  ASSERT_EQ(0, n_examples);

  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_examples_object(
                               jo, &examples_arr, &n_examples, &spec, 0));
  ASSERT_EQ(NULL, examples_arr);
  ASSERT_EQ(0, n_examples);
  json_value_free(jv);

  jv = json_parse_string("{\"ex1\": {\"value\": 1}, \"invalid_child\": "
                         "\"not_obj\", \"ex2\": {\"value\": 2}}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_examples_object(
                                    jo, &examples_arr, &n_examples, &spec, 0));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_examples_object(
                               jo, &examples_arr, &n_examples, &spec, 0));
  ASSERT_EQ(3, n_examples);
  ASSERT(examples_arr != NULL);
  cdd_test_free_example(&examples_arr[0]);
  cdd_test_free_example(&examples_arr[2]);
  free(examples_arr);
  examples_arr = NULL;
  json_value_free(jv);

  /* 5. parse_media_examples */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_media_examples(
                               NULL, &example_any, &example_set, &examples_arr,
                               &n_examples, &spec, 0));

  jv = json_parse_string("{\"example\": 1, \"examples\": {}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_media_examples(jo, &example_any, &example_set,
                                          &examples_arr, &n_examples, &spec,
                                          0));
  json_value_free(jv);

  jv = json_parse_string("{\"examples\": {\"ex\": {\"value\": \"test\"}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_media_examples(
                               jo, &example_any, &example_set, &examples_arr,
                               &n_examples, &spec, 0));
  cdd_test_free_example(&examples_arr[0]);
  free(examples_arr);
  examples_arr = NULL;
  json_value_free(jv);

  jv = json_parse_string("{\"example\": \"val\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_media_examples(
                               jo, &example_any, &example_set, &examples_arr,
                               &n_examples, &spec, 0));
  ASSERT_EQ(1, example_set);
  json_value_free(jv);

  /* 6. parse_oauth_scopes */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_oauth_scopes(NULL, NULL, &n_scopes));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_oauth_scopes(NULL, &scopes, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_oauth_scopes(NULL, &scopes, &n_scopes));
  ASSERT_EQ(NULL, scopes);
  ASSERT_EQ(0, n_scopes);

  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_oauth_scopes(jo, &scopes, &n_scopes));
  ASSERT_EQ(NULL, scopes);
  ASSERT_EQ(0, n_scopes);
  json_value_free(jv);

  jv = json_parse_string("{\"read:pets\": \"read pets\", \"write:pets\": "
                         "\"write pets\"}");
  jo = json_value_get_object(jv);

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_oauth_scopes(jo, &scopes, &n_scopes));
  g_cdd_alloc_fail = 0;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_oauth_scopes(jo, &scopes, &n_scopes));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_oauth_scopes(jo, &scopes, &n_scopes));
  g_cdd_strdup_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_oauth_scopes(jo, &scopes, &n_scopes));
  ASSERT_EQ(2, n_scopes);
  free_test_oauth_scopes(scopes, n_scopes);
  scopes = NULL;
  json_value_free(jv);

  /* scope with non-string description */
  jv = json_parse_string("{\"read:pets\": 123}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_oauth_scopes(jo, &scopes, &n_scopes));
  ASSERT_EQ(1, n_scopes);
  ASSERT_EQ(NULL, scopes[0].description);
  free_test_oauth_scopes(scopes, n_scopes);
  scopes = NULL;
  json_value_free(jv);

  /* 7. parse_oauth_flows */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_oauth_flows(NULL, &sec));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_oauth_flows(NULL, NULL));

  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  json_value_free(jv);

  /* flows_obj != NULL, out == NULL */
  jv = json_parse_string("{\"implicit\": {\"authorizationUrl\": "
                         "\"http://auth\", \"scopes\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_oauth_flows(jo, NULL));
  json_value_free(jv);

  /* flow_obj is not an object */
  jv = json_parse_string("{\"implicit\": \"not_an_obj\"}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Invalid flow name without scopes */
  jv = json_parse_string("{\"invalidFlow\": {}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Invalid flow name with scopes (reaches switch default /
   * OA_OAUTH_FLOW_UNKNOWN) */
  jv = json_parse_string("{\"invalidFlow\": {\"scopes\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Implicit flow without scopes */
  jv = json_parse_string("{\"implicit\": {}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Implicit flow with non-object scopes */
  jv = json_parse_string("{\"implicit\": {\"scopes\": 123}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Implicit flow without authorizationUrl */
  jv = json_parse_string("{\"implicit\": {\"scopes\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Password flow without tokenUrl */
  jv = json_parse_string("{\"password\": {\"scopes\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Client credentials flow without tokenUrl */
  jv = json_parse_string("{\"clientCredentials\": {\"scopes\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Client credentials flow without tokenUrl */
  jv = json_parse_string("{\"clientCredentials\": {\"scopes\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Authorization code flow without tokenUrl */
  jv = json_parse_string("{\"authorizationCode\": {\"scopes\": {}, "
                         "\"authorizationUrl\": \"http://auth\"}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Authorization code flow without authorizationUrl */
  jv = json_parse_string("{\"authorizationCode\": {\"scopes\": {}, "
                         "\"tokenUrl\": \"http://tok\"}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Device authorization flow without tokenUrl */
  jv = json_parse_string("{\"deviceAuthorization\": {\"scopes\": {}, "
                         "\"deviceAuthorizationUrl\": \"http://dev\"}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Device authorization flow without deviceAuthorizationUrl */
  jv = json_parse_string("{\"deviceAuthorization\": {\"scopes\": {}, "
                         "\"tokenUrl\": \"http://tok\"}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_parse_oauth_flows(jo, &sec));
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Valid flows for all types */
  jv =
      json_parse_string("{"
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
                        "}");
  jo = json_value_get_object(jv);

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_oauth_flows(jo, &sec));
  g_cdd_alloc_fail = 0;
  free_test_security_scheme_flows(&sec);

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_oauth_flows(jo, &sec));
  g_cdd_strdup_fail = 0;
  free_test_security_scheme_flows(&sec);

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_oauth_flows(jo, &sec));
  g_cdd_strdup_fail = 0;
  free_test_security_scheme_flows(&sec);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_oauth_flows(jo, &sec));
  ASSERT_EQ(5, sec.n_flows);
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  /* Flow extension failure */
  jv = json_parse_string("{\"implicit\": {\"authorizationUrl\": "
                         "\"http://auth\", \"scopes\": {}, \"x-f\": 1}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_parse_oauth_flows(jo, &sec));
  g_cdd_strdup_fail = 0;
  free_test_security_scheme_flows(&sec);
  json_value_free(jv);

  PASS();
}

static void free_test_str_array(char **arr, size_t count) {
  size_t i;
  if (!arr)
    return;
  for (i = 0; i < count; ++i) {
    if (arr[i])
      free(arr[i]);
  }
  free(arr);
}

TEST test_openapi_validation_all_branches(void) {
  size_t qs_count = 0;
  int has_query = 0;
  struct OpenAPI_Parameter params[3];
  struct OpenAPI_Path paths[2];
  struct OpenAPI_Operation ops[2];
  struct OpenAPI_Callback cbs[2];
  struct OpenAPI_Spec spec;
  char **ids = NULL;
  size_t id_count = 0;
  size_t id_cap = 0;
  char *cpi_name = (char *)(size_t) "my_item";

  memset(params, 0, sizeof(params));
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));
  memset(&spec, 0, sizeof(spec));

  free_test_str_array(NULL, 0);

  /* 1. scan_querystring_usage */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_scan_querystring_usage(params, 1, NULL, &has_query));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_scan_querystring_usage(params, 1, &qs_count, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_scan_querystring_usage(params, 1, NULL, NULL));

  params[0].in = OA_PARAM_IN_QUERYSTRING;
  params[1].in = OA_PARAM_IN_QUERY;
  params[2].in = OA_PARAM_IN_HEADER;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_scan_querystring_usage(params, 3, &qs_count, &has_query));
  ASSERT_EQ(1, qs_count);
  ASSERT_EQ(1, has_query);

  /* 2. validate_querystring_usage */
  /* Path without route is skipped */
  paths[0].route = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_querystring_usage(paths, 1));

  paths[0].route = (char *)(size_t) "/test";
  paths[0].parameters = params;
  paths[0].n_parameters = 3; /* has qs_count=1 and has_query=1 -> invalid! */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage(paths, 1));

  /* path_qs > 1 */
  params[1].in = OA_PARAM_IN_QUERYSTRING;
  paths[0].n_parameters = 2; /* 2 querystring params -> invalid! */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage(paths, 1));

  /* path has 1 querystring param, op has query param -> invalid */
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  paths[0].n_parameters = 1;
  ops[0].parameters = &params[1];
  params[1].in = OA_PARAM_IN_QUERY;
  ops[0].n_parameters = 1;
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage(paths, 1));

  /* path has 1 querystring, op has 1 querystring -> total_qs > 1 */
  params[1].in = OA_PARAM_IN_QUERYSTRING;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage(paths, 1));

  /* path has 0 querystring, additional_operations has total_qs > 1 */
  paths[0].n_parameters = 0;
  paths[0].n_operations = 0;
  paths[0].additional_operations = ops;
  paths[0].n_additional_operations = 1;
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  params[1].in = OA_PARAM_IN_QUERYSTRING;
  ops[0].parameters = params;
  ops[0].n_parameters = 2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage(paths, 1));

  /* additional_operations total_qs > 0 && has_query */
  params[1].in = OA_PARAM_IN_QUERY;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage(paths, 1));

  /* scan_querystring_usage error branches */
  paths[0].parameters = NULL;
  paths[0].n_parameters = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage(paths, 1));
  paths[0].parameters = params;
  paths[0].n_parameters = 0;

  paths[0].operations = ops;
  paths[0].n_operations = 1;
  ops[0].parameters = NULL;
  ops[0].n_parameters = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage(paths, 1));
  ops[0].parameters = params;
  ops[0].n_parameters = 0;
  paths[0].n_operations = 0;

  paths[0].additional_operations = ops;
  paths[0].n_additional_operations = 1;
  ops[0].parameters = NULL;
  ops[0].n_parameters = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage(paths, 1));
  ops[0].parameters = params;
  ops[0].n_parameters = 0;
  paths[0].n_additional_operations = 0;

  /* operations with total_qs == 1 && has_query == 0 */
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  ops[0].parameters = params;
  ops[0].n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_querystring_usage(paths, 1));
  paths[0].n_operations = 0;

  /* additional_operations with total_qs == 1 && has_query == 0 */
  paths[0].additional_operations = ops;
  paths[0].n_additional_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_querystring_usage(paths, 1));
  paths[0].n_additional_operations = 0;

  /* path with query param (path_has_query == 1) */
  params[0].in = OA_PARAM_IN_QUERY;
  paths[0].parameters = params;
  paths[0].n_parameters = 1;
  paths[0].operations = ops;
  ops[0].n_parameters = 0;
  paths[0].n_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_querystring_usage(paths, 1));
  paths[0].n_operations = 0;
  paths[0].additional_operations = ops;
  paths[0].n_additional_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_querystring_usage(paths, 1));
  paths[0].n_additional_operations = 0;
  paths[0].n_parameters = 0;

  /* Valid querystring usage */
  ops[0].n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_querystring_usage(paths, 1));

  /* 3. validate_querystring_usage_in_callbacks */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_callbacks(NULL, 0));
  cbs[0].paths = NULL;
  cbs[0].n_paths = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_callbacks(cbs, 1));
  cbs[0].paths = paths;
  cbs[0].n_paths = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_callbacks(cbs, 1));
  cbs[0].paths = paths;
  cbs[0].n_paths = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_callbacks(cbs, 1));
  /* callback error */
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  params[1].in = OA_PARAM_IN_QUERYSTRING;
  ops[0].parameters = params;
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  ops[0].n_parameters = 2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage_in_callbacks(cbs, 1));
  ops[0].n_parameters = 1;

  /* 4. validate_querystring_usage_in_operations */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_operations(NULL, 0));
  ops[0].callbacks = cbs;
  ops[0].n_callbacks = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_operations(ops, 1));
  ops[0].n_parameters = 2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage_in_operations(ops, 1));
  ops[0].n_parameters = 1;

  /* 5. validate_querystring_usage_in_paths_callbacks */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_paths_callbacks(NULL, 0));
  paths[0].n_additional_operations = 0;
  paths[0].n_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_paths_callbacks(paths, 1));
  paths[0].n_operations = 0;
  paths[0].n_additional_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_paths_callbacks(paths, 1));
  ops[0].n_parameters = 2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage_in_paths_callbacks(paths, 1));
  paths[0].n_operations = 1;
  paths[0].n_additional_operations = 0;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage_in_paths_callbacks(paths, 1));
  ops[0].n_parameters = 1;

  /* 6. validate_querystring_usage_in_component_callbacks */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_component_callbacks(NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_component_callbacks(&spec));
  spec.component_callbacks = cbs;
  spec.n_component_callbacks = 1;
  cbs[0].paths = paths;
  cbs[0].n_paths = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_component_callbacks(&spec));
  cbs[0].paths = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_component_callbacks(&spec));
  cbs[0].paths = paths;
  cbs[0].n_paths = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_querystring_usage_in_component_callbacks(&spec));
  ops[0].n_parameters = 2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage_in_component_callbacks(&spec));
  ops[0].n_parameters = 1;
  memset(&spec, 0, sizeof(spec));

  /* 7. add_unique_operation_id */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_add_unique_operation_id(&ids, &id_count, &id_cap, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_add_unique_operation_id(&ids, &id_count, &id_cap, ""));

  /* Add first id */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_add_unique_operation_id(&ids, &id_count, &id_cap, "op1"));
  ASSERT_EQ(1, id_count);
  ASSERT(id_cap >= 1);

  /* Add duplicate */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_add_unique_operation_id(&ids, &id_count, &id_cap, "op1"));

  /* Add more to test realloc when count == cap */
  id_count = id_cap;
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_add_unique_operation_id(&ids, &id_count, &id_cap, "op2"));
  g_cdd_alloc_fail = 0;

  /* strdup failure */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_add_unique_operation_id(&ids, &id_count, &id_cap, "op2"));
  g_cdd_strdup_fail = 0;
  id_count = 1;

  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;

  /* 8. collect_operation_ids */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_collect_operation_ids(NULL, 0, &ids, &id_count, &id_cap));
  ops[0].operation_id = (char *)(size_t) "op1";
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  paths[0].additional_operations = &ops[1];
  paths[0].n_additional_operations = 1;
  ops[1].operation_id = (char *)(size_t) "op2";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_collect_operation_ids(paths, 1, &ids, &id_count, &id_cap));
  ASSERT_EQ(2, id_count);

  /* Duplicate in additional_operations */
  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;
  ops[0].operation_id = (char *)(size_t) "op1";
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  paths[0].additional_operations = &ops[1];
  paths[0].n_additional_operations = 1;
  ops[1].operation_id = (char *)(size_t) "op1";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_collect_operation_ids(paths, 1, &ids, &id_count, &id_cap));

  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;

  /* 9. path_item_ref_matches_component */
  ASSERT_EQ(0, cdd_test_path_item_ref_matches_component(NULL, NULL, "item"));
  ASSERT_EQ(0, cdd_test_path_item_ref_matches_component(&spec, "ref", NULL));
  ASSERT_EQ(
      0, cdd_test_path_item_ref_matches_component(&spec, "bad_prefix", "item"));
  ASSERT_EQ(1, cdd_test_path_item_ref_matches_component(
                   &spec, "#/components/pathItems/my_item", "my_item"));
  ASSERT_EQ(0, cdd_test_path_item_ref_matches_component(
                   &spec, "#/components/pathItems/other_item", "my_item"));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(0, cdd_test_path_item_ref_matches_component(
                   &spec, "#/components/pathItems/my_item", "my_item"));
  g_cdd_alloc_fail = 0;

  /* 10. component_path_item_is_referenced */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_path_item_is_referenced(NULL, "item"));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_path_item_is_referenced(&spec, NULL));
  spec.paths = paths;
  spec.n_paths = 1;
  paths[0].ref = (char *)(size_t) "#/components/pathItems/my_item";
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_component_path_item_is_referenced(&spec, "my_item"));
  paths[0].ref = (char *)(size_t) "#/components/pathItems/other_item";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_path_item_is_referenced(&spec, "my_item"));
  paths[0].ref = NULL;
  spec.webhooks = &paths[1];
  spec.n_webhooks = 1;
  paths[1].ref = (char *)(size_t) "#/components/pathItems/my_item";
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_component_path_item_is_referenced(&spec, "my_item"));
  paths[1].ref = (char *)(size_t) "#/components/pathItems/other_item";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_path_item_is_referenced(&spec, "my_item"));
  paths[1].ref = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_path_item_is_referenced(&spec, "my_item"));
  memset(&spec, 0, sizeof(spec));

  /* 11. callback_ref_matches_component */
  ASSERT_EQ(0, cdd_test_callback_ref_matches_component(NULL, NULL, "cb"));
  ASSERT_EQ(0, cdd_test_callback_ref_matches_component(&spec, "ref", NULL));
  ASSERT_EQ(0,
            cdd_test_callback_ref_matches_component(&spec, "bad_prefix", "cb"));
  ASSERT_EQ(1, cdd_test_callback_ref_matches_component(
                   &spec, "#/components/callbacks/my_cb", "my_cb"));
  ASSERT_EQ(0, cdd_test_callback_ref_matches_component(
                   &spec, "#/components/callbacks/other_cb", "my_cb"));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(0, cdd_test_callback_ref_matches_component(
                   &spec, "#/components/callbacks/my_cb", "my_cb"));
  g_cdd_alloc_fail = 0;

  /* 12. component_callback_is_referenced_in_ops */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_component_callback_is_referenced_in_ops(
                               NULL, 0, &spec, "cb"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_component_callback_is_referenced_in_ops(
                               ops, 1, NULL, "cb"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_component_callback_is_referenced_in_ops(
                               ops, 1, &spec, NULL));
  ops[0].callbacks = cbs;
  ops[0].n_callbacks = 1;
  cbs[0].ref = (char *)(size_t) "#/components/callbacks/my_cb";
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_component_callback_is_referenced_in_ops(ops, 1, &spec, "my_cb"));
  cbs[0].ref = (char *)(size_t) "#/components/callbacks/other_cb";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_component_callback_is_referenced_in_ops(
                               ops, 1, &spec, "my_cb"));
  cbs[0].ref = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_component_callback_is_referenced_in_ops(
                               ops, 1, &spec, "my_cb"));

  /* 13. component_callback_is_referenced */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_callback_is_referenced(NULL, "cb"));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_callback_is_referenced(&spec, NULL));
  /* referenced in paths operations */
  cbs[0].ref = (char *)(size_t) "#/components/callbacks/my_cb";
  spec.paths = paths;
  spec.n_paths = 1;
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_component_callback_is_referenced(&spec, "my_cb"));
  paths[0].n_operations = 0;
  paths[0].additional_operations = ops;
  paths[0].n_additional_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_component_callback_is_referenced(&spec, "my_cb"));
  paths[0].n_additional_operations = 0;

  /* referenced in webhooks operations */
  spec.webhooks = &paths[1];
  spec.n_webhooks = 1;
  paths[1].operations = ops;
  paths[1].n_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_component_callback_is_referenced(&spec, "my_cb"));
  paths[1].n_operations = 0;
  paths[1].additional_operations = ops;
  paths[1].n_additional_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_component_callback_is_referenced(&spec, "my_cb"));
  paths[1].n_additional_operations = 0;

  /* referenced in component_path_items */
  spec.paths = NULL;
  spec.n_paths = 0;
  spec.webhooks = &paths[1];
  spec.n_webhooks = 1;
  paths[1].operations = NULL;
  paths[1].n_operations = 0;
  spec.component_path_items = &paths[0];
  spec.n_component_path_items = 1;
  spec.component_path_item_names = &cpi_name;
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_component_callback_is_referenced(&spec, "my_cb"));
  paths[0].n_operations = 0;
  paths[0].additional_operations = ops;
  paths[0].n_additional_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_component_callback_is_referenced(&spec, "my_cb"));

  /* If component_path_item is itself referenced, it is skipped */
  paths[1].ref = (char *)(size_t) "#/components/pathItems/my_item";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_callback_is_referenced(&spec, "my_cb"));
  paths[1].ref = NULL;
  cbs[0].ref = NULL;

  /* component_path_items == NULL */
  spec.component_path_items = NULL;
  spec.n_component_path_items = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_callback_is_referenced(&spec, "my_cb"));

  /* component_path_items != NULL && n_component_path_items == 0 */
  spec.component_path_items = paths;
  spec.n_component_path_items = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_component_callback_is_referenced(&spec, "my_cb"));

  /* component_path_item_names == NULL */
  cbs[0].ref = (char *)(size_t) "#/components/callbacks/my_cb";
  spec.component_path_items = &paths[0];
  spec.n_component_path_items = 1;
  spec.component_path_item_names = NULL;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_component_callback_is_referenced(&spec, "my_cb"));

  /* component_path_item_names[0] == NULL */
  {
    char *null_name = NULL;
    char **null_names = &null_name;
    spec.component_path_item_names = null_names;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              cdd_test_component_callback_is_referenced(&spec, "my_cb"));
  }
  cbs[0].ref = NULL;

  memset(&spec, 0, sizeof(spec));
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));

  /* 14. collect_callback_operation_ids_from_callbacks */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_collect_callback_operation_ids_from_callbacks(
                NULL, 0, &ids, &id_count, &id_cap));
  cbs[0].paths = NULL;
  cbs[0].n_paths = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_collect_callback_operation_ids_from_callbacks(
                cbs, 1, &ids, &id_count, &id_cap));
  cbs[0].paths = paths;
  cbs[0].n_paths = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_collect_callback_operation_ids_from_callbacks(
                cbs, 1, &ids, &id_count, &id_cap));
  cbs[0].paths = paths;
  cbs[0].n_paths = 1;
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  ops[0].operation_id = (char *)(size_t) "cb_op";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_collect_callback_operation_ids_from_callbacks(
                cbs, 1, &ids, &id_count, &id_cap));
  ASSERT_EQ(1, id_count);
  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;

  /* 15. collect_callback_operation_ids_from_operations */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_collect_callback_operation_ids_from_operations(
                NULL, 0, &ids, &id_count, &id_cap));
  ops[0].callbacks = cbs;
  ops[0].n_callbacks = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_collect_callback_operation_ids_from_operations(
                ops, 1, &ids, &id_count, &id_cap));
  ASSERT_EQ(1, id_count);
  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;

  /* 16. collect_callback_operation_ids_from_paths */
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_collect_callback_operation_ids_from_paths(
                               NULL, 0, &ids, &id_count, &id_cap));
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  paths[0].additional_operations = &ops[1];
  paths[0].n_additional_operations = 1;
  ops[1].callbacks = &cbs[1];
  ops[1].n_callbacks = 1;
  cbs[1].paths = &paths[1];
  cbs[1].n_paths = 1;
  paths[1].operations = &ops[0];
  paths[1].n_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_collect_callback_operation_ids_from_paths(
                               paths, 1, &ids, &id_count, &id_cap));
  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;

  /* additional_operations callback duplicate in
   * collect_callback_operation_ids_from_paths */
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));
  paths[0].additional_operations = ops;
  paths[0].n_additional_operations = 1;
  ops[0].callbacks = cbs;
  ops[0].n_callbacks = 1;
  cbs[0].paths = &paths[1];
  cbs[0].n_paths = 1;
  paths[1].operations = &ops[1];
  paths[1].n_operations = 1;
  ops[1].operation_id = (char *)(size_t) "dup";
  cdd_test_add_unique_operation_id(&ids, &id_count, &id_cap, "dup");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_collect_callback_operation_ids_from_paths(
                paths, 1, &ids, &id_count, &id_cap));
  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;

  /* 17. validate_unique_operation_ids */
  memset(&spec, 0, sizeof(spec));
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_unique_operation_ids(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;

  /* Spec with component path item unreferenced vs referenced */
  spec.component_path_items = paths;
  spec.n_component_path_items = 1;
  spec.component_path_item_names = &cpi_name;
  ops[0].operation_id = (char *)(size_t) "unique_cpi";
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  paths[0].additional_operations = NULL;
  paths[0].n_additional_operations = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;

  /* Spec with unreferenced component callback */
  cbs[0].name = (char *)(size_t) "my_cb";
  cbs[0].paths = &paths[1];
  cbs[0].n_paths = 1;
  paths[1].operations = &ops[1];
  paths[1].n_operations = 1;
  ops[1].operation_id = (char *)(size_t) "unique_cb";
  spec.component_callbacks = cbs;
  spec.n_component_callbacks = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;

  /* Duplicate between component path item and component callback */
  ops[1].operation_id = (char *)(size_t) "unique_cpi";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_unique_operation_ids(&spec));

  /* Spec paths with duplicate */
  memset(&spec, 0, sizeof(spec));
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));
  spec.paths = paths;
  spec.n_paths = 1;
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  paths[0].additional_operations = &ops[1];
  paths[0].n_additional_operations = 1;
  ops[0].operation_id = (char *)(size_t) "dup";
  ops[1].operation_id = (char *)(size_t) "dup";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_unique_operation_ids(&spec));
  paths[0].n_additional_operations = 0;

  /* Spec webhooks with duplicate */
  spec.paths = NULL;
  spec.n_paths = 0;
  spec.webhooks = paths;
  spec.n_webhooks = 1;
  paths[0].additional_operations = &ops[1];
  paths[0].n_additional_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_unique_operation_ids(&spec));
  paths[0].n_additional_operations = 0;
  spec.webhooks = NULL;
  spec.n_webhooks = 0;

  /* Spec paths callback duplicate */
  spec.paths = paths;
  spec.n_paths = 1;
  ops[0].callbacks = cbs;
  ops[0].n_callbacks = 1;
  cbs[0].paths = &paths[1];
  cbs[0].n_paths = 1;
  paths[1].operations = ops;
  paths[1].n_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_unique_operation_ids(&spec));

  /* Spec webhooks callback duplicate */
  spec.paths = NULL;
  spec.n_paths = 0;
  spec.webhooks = paths;
  spec.n_webhooks = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_unique_operation_ids(&spec));
  memset(&spec, 0, sizeof(spec));
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));

  /* Spec with component_path_item_names == NULL and with null name */
  spec.component_path_items = paths;
  spec.n_component_path_items = 1;
  spec.component_path_item_names = NULL;
  ops[0].operation_id = (char *)(size_t) "unique_cpi2";
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;
  {
    char *null_name = NULL;
    char **null_names = &null_name;
    spec.component_path_item_names = null_names;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_unique_operation_ids(&spec));
    spec.component_path_item_names = NULL;
  }

  /* Duplicate in component_path_items callback */
  ops[0].callbacks = cbs;
  ops[0].n_callbacks = 1;
  cbs[0].paths = &paths[1];
  cbs[0].n_paths = 1;
  paths[1].operations = ops;
  paths[1].n_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_unique_operation_ids(&spec));
  ops[0].n_callbacks = 0;
  spec.component_path_items = NULL;
  spec.n_component_path_items = 0;

  /* Spec component_callbacks with name == NULL, and paths with n_paths == 0 */
  cbs[0].name = NULL;
  cbs[0].paths = paths;
  cbs[0].n_paths = 0;
  spec.component_callbacks = cbs;
  spec.n_component_callbacks = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;

  /* Spec with component_path_items != NULL && n_component_path_items == 0 */
  spec.component_path_items = paths;
  spec.n_component_path_items = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;
  spec.component_path_items = NULL;

  /* Spec with component_callbacks != NULL && n_component_callbacks == 0 */
  spec.component_callbacks = cbs;
  spec.n_component_callbacks = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;

  /* Spec with unreferenced component_callback having paths == NULL */
  cbs[0].name = (char *)(size_t) "cb_no_paths";
  cbs[0].paths = NULL;
  cbs[0].n_paths = 0;
  spec.component_callbacks = cbs;
  spec.n_component_callbacks = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;

  /* Spec with component_path_items having duplicate operation IDs */
  {
    struct OpenAPI_Operation dup_ops[2];
    memset(dup_ops, 0, sizeof(dup_ops));
    dup_ops[0].operation_id = (char *)(size_t) "dup_id";
    dup_ops[1].operation_id = (char *)(size_t) "dup_id";
    paths[0].operations = dup_ops;
    paths[0].n_operations = 2;
    paths[0].additional_operations = NULL;
    paths[0].n_additional_operations = 0;
    spec.component_path_items = paths;
    spec.n_component_path_items = 1;
    spec.component_path_item_names = NULL;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_test_validate_unique_operation_ids(&spec));
    spec.component_path_items = NULL;
    spec.n_component_path_items = 0;
    paths[0].operations = NULL;
    paths[0].n_operations = 0;
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_FIND_COVERAGE_H */
