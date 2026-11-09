/**
 * @file test_openapi_loader_paths.h
 * @brief Path extensions, templates, styles, and links tests.
 */

#ifndef TEST_OPENAPI_LOADER_PATHS_H
#define TEST_OPENAPI_LOADER_PATHS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Unit test test_load_extensions_non_schema.
 * @return Greatest test result.
 */
TEST test_load_extensions_non_schema(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  120, 45,  114, 111, 111, 116, 34,  58,  49,  44,
      34,  105, 110, 102, 111, 34,  58,  123, 32,  32,  34,  116, 105, 116, 108,
      101, 34,  58,  34,  84,  101, 115, 116, 34,  44,  32,  32,  34,  118, 101,
      114, 115, 105, 111, 110, 34,  58,  34,  49,  34,  44,  32,  32,  34,  120,
      45,  105, 110, 102, 111, 34,  58,  34,  105, 110, 102, 111, 34,  44,  32,
      32,  34,  99,  111, 110, 116, 97,  99,  116, 34,  58,  123, 34,  110, 97,
      109, 101, 34,  58,  34,  83,  117, 112, 112, 111, 114, 116, 34,  44,  34,
      120, 45,  99,  111, 110, 116, 97,  99,  116, 34,  58,  116, 114, 117, 101,
      125, 44,  32,  32,  34,  108, 105, 99,  101, 110, 115, 101, 34,  58,  123,
      34,  110, 97,  109, 101, 34,  58,  34,  77,  73,  84,  34,  44,  34,  120,
      45,  108, 105, 99,  101, 110, 115, 101, 34,  58,  34,  108, 105, 99,  34,
      125, 125, 44,  34,  101, 120, 116, 101, 114, 110, 97,  108, 68,  111, 99,
      115, 34,  58,  123, 34,  117, 114, 108, 34,  58,  34,  104, 116, 116, 112,
      115, 58,  47,  47,  101, 120, 97,  109, 112, 108, 101, 46,  99,  111, 109,
      34,  44,  34,  120, 45,  101, 120, 116, 34,  58,  34,  101, 120, 116, 34,
      125, 44,  34,  116, 97,  103, 115, 34,  58,  91,  123, 34,  110, 97,  109,
      101, 34,  58,  34,  112, 101, 116, 34,  44,  34,  120, 45,  116, 97,  103,
      34,  58,  34,  116, 97,  103, 34,  125, 93,  44,  34,  115, 101, 99,  117,
      114, 105, 116, 121, 34,  58,  91,  123, 34,  97,  112, 105, 95,  107, 101,
      121, 34,  58,  91,  93,  44,  34,  120, 45,  115, 101, 99,  45,  114, 101,
      113, 34,  58,  34,  114, 101, 113, 34,  125, 93,  44,  34,  99,  111, 109,
      112, 111, 110, 101, 110, 116, 115, 34,  58,  123, 32,  32,  34,  115, 101,
      99,  117, 114, 105, 116, 121, 83,  99,  104, 101, 109, 101, 115, 34,  58,
      123, 32,  32,  32,  32,  34,  97,  112, 105, 95,  107, 101, 121, 34,  58,
      123, 32,  32,  32,  32,  32,  32,  34,  116, 121, 112, 101, 34,  58,  34,
      97,  112, 105, 75,  101, 121, 34,  44,  32,  32,  32,  32,  32,  32,  34,
      110, 97,  109, 101, 34,  58,  34,  88,  45,  65,  80,  73,  34,  44,  32,
      32,  32,  32,  32,  32,  34,  105, 110, 34,  58,  34,  104, 101, 97,  100,
      101, 114, 34,  44,  32,  32,  32,  32,  32,  32,  34,  120, 45,  115, 101,
      99,  34,  58,  34,  115, 101, 99,  34,  32,  32,  32,  32,  125, 32,  32,
      125, 125, 44,  34,  112, 97,  116, 104, 115, 34,  58,  123, 32,  32,  34,
      47,  112, 101, 116, 115, 34,  58,  123, 32,  32,  32,  32,  34,  120, 45,
      112, 97,  116, 104, 34,  58,  34,  112, 97,  116, 104, 34,  44,  32,  32,
      32,  32,  34,  103, 101, 116, 34,  58,  123, 32,  32,  32,  32,  32,  32,
      34,  120, 45,  111, 112, 34,  58,  50,  44,  32,  32,  32,  32,  32,  32,
      34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,  32,
      32,  32,  32,  32,  32,  32,  32,  123, 34,  110, 97,  109, 101, 34,  58,
      34,  105, 100, 34,  44,  34,  105, 110, 34,  58,  34,  113, 117, 101, 114,
      121, 34,  44,  34,  115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  116,
      121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 44,
      32,  32,  32,  32,  32,  32,  32,  32,  32,  34,  120, 45,  112, 97,  114,
      97,  109, 34,  58,  34,  112, 97,  114, 97,  109, 34,  125, 32,  32,  32,
      32,  32,  32,  93,  44,  32,  32,  32,  32,  32,  32,  34,  114, 101, 113,
      117, 101, 115, 116, 66,  111, 100, 121, 34,  58,  123, 32,  32,  32,  32,
      32,  32,  32,  32,  34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111,
      110, 34,  58,  34,  98,  111, 100, 121, 34,  44,  32,  32,  32,  32,  32,
      32,  32,  32,  34,  99,  111, 110, 116, 101, 110, 116, 34,  58,  123, 32,
      32,  32,  32,  32,  32,  32,  32,  32,  32,  34,  97,  112, 112, 108, 105,
      99,  97,  116, 105, 111, 110, 47,  106, 115, 111, 110, 34,  58,  123, 32,
      32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  34,  115, 99,  104,
      101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115,
      116, 114, 105, 110, 103, 34,  125, 32,  32,  32,  32,  32,  32,  32,  32,
      32,  32,  125, 32,  32,  32,  32,  32,  32,  32,  32,  125, 44,  32,  32,
      32,  32,  32,  32,  32,  32,  34,  120, 45,  114, 98,  34,  58,  123, 34,
      110, 111, 116, 101, 34,  58,  116, 114, 117, 101, 125, 32,  32,  32,  32,
      32,  32,  125, 44,  32,  32,  32,  32,  32,  32,  34,  114, 101, 115, 112,
      111, 110, 115, 101, 115, 34,  58,  123, 32,  32,  32,  32,  32,  32,  32,
      32,  34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105,
      112, 116, 105, 111, 110, 34,  58,  34,  111, 107, 34,  44,  34,  120, 45,
      114, 101, 115, 112, 34,  58,  123, 34,  111, 107, 34,  58,  116, 114, 117,
      101, 125, 125, 44,  32,  32,  32,  32,  32,  32,  32,  32,  34,  120, 45,
      114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  116, 114,
      97,  99,  101, 34,  58,  116, 114, 117, 101, 125, 32,  32,  32,  32,  32,
      32,  125, 44,  32,  32,  32,  32,  32,  32,  34,  99,  97,  108, 108, 98,
      97,  99,  107, 115, 34,  58,  123, 32,  32,  32,  32,  32,  32,  32,  32,
      34,  111, 110, 69,  118, 101, 110, 116, 34,  58,  123, 32,  32,  32,  32,
      32,  32,  32,  32,  32,  32,  34,  120, 45,  99,  98,  34,  58,  34,  99,
      98,  34,  44,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  34,  123,
      36,  114, 101, 113, 117, 101, 115, 116, 46,  98,  111, 100, 121, 35,  47,
      117, 114, 108, 125, 34,  58,  123, 32,  32,  32,  32,  32,  32,  32,  32,
      32,  32,  32,  32,  34,  112, 111, 115, 116, 34,  58,  123, 32,  32,  32,
      32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  34,  114, 101, 115,
      112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,
      123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,
      34,  111, 107, 34,  125, 125, 32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  32,  32,  125, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  125,
      32,  32,  32,  32,  32,  32,  32,  32,  125, 32,  32,  32,  32,  32,  32,
      125, 32,  32,  32,  32,  125, 32,  32,  125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root_ext = NULL;
    JSON_Object *root_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(spec.extensions_json, &_cdd_jv_1);
      root_ext = _cdd_jv_1;
    }
    root_obj = json_value_get_object(root_ext);
    {
      double _cdd_jn_1 = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_number(root_obj, "x-root", &_cdd_jn_1);
      ASSERT_EQ(1, (int)_cdd_jn_1);
    }
    g_cdd_last_alloc_err = json_value_free(root_ext);
  }

  {
    JSON_Value *info_ext = NULL;
    JSON_Object *info_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(spec.info.extensions_json, &_cdd_jv_1);
      info_ext = _cdd_jv_1;
    }
    info_obj = json_value_get_object(info_ext);
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(info_obj, "x-info", &_cdd_js_1);
      ASSERT_STR_EQ("info", _cdd_js_1);
    }
    g_cdd_last_alloc_err = json_value_free(info_ext);
  }

  {
    JSON_Value *contact_ext = NULL;
    JSON_Object *contact_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(spec.info.contact.extensions_json, &_cdd_jv_1);
      contact_ext = _cdd_jv_1;
    }
    contact_obj = json_value_get_object(contact_ext);
    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(contact_obj, "x-contact", &_cdd_jb_1) !=
          CDD_C_SUCCESS)
        _cdd_jb_1 = -1;
      ASSERT_EQ(1, _cdd_jb_1);
    }
    g_cdd_last_alloc_err = json_value_free(contact_ext);
  }

  {
    JSON_Value *license_ext = NULL;
    JSON_Object *license_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(spec.info.license.extensions_json, &_cdd_jv_1);
      license_ext = _cdd_jv_1;
    }
    license_obj = json_value_get_object(license_ext);
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(license_obj, "x-license", &_cdd_js_1);
      ASSERT_STR_EQ("lic", _cdd_js_1);
    }
    g_cdd_last_alloc_err = json_value_free(license_ext);
  }

  {
    JSON_Value *ext_docs_ext = NULL;
    JSON_Object *ext_docs_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(spec.external_docs.extensions_json, &_cdd_jv_1);
      ext_docs_ext = _cdd_jv_1;
    }
    ext_docs_obj = json_value_get_object(ext_docs_ext);
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(ext_docs_obj, "x-ext", &_cdd_js_1);
      ASSERT_STR_EQ("ext", _cdd_js_1);
    }
    g_cdd_last_alloc_err = json_value_free(ext_docs_ext);
  }

  {
    JSON_Value *tag_ext = NULL;
    JSON_Object *tag_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(spec.tags[0].extensions_json, &_cdd_jv_1);
      tag_ext = _cdd_jv_1;
    }
    tag_obj = json_value_get_object(tag_ext);
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(tag_obj, "x-tag", &_cdd_js_1);
      ASSERT_STR_EQ("tag", _cdd_js_1);
    }
    g_cdd_last_alloc_err = json_value_free(tag_ext);
  }

  {
    JSON_Value *sec_ext = NULL;
    JSON_Object *sec_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          spec.security_schemes[0].extensions_json, &_cdd_jv_1);
      sec_ext = _cdd_jv_1;
    }
    sec_obj = json_value_get_object(sec_ext);
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(sec_obj, "x-sec", &_cdd_js_1);
      ASSERT_STR_EQ("sec", _cdd_js_1);
    }
    g_cdd_last_alloc_err = json_value_free(sec_ext);
  }

  {
    JSON_Value *sec_req_ext = NULL;
    JSON_Object *sec_req_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(spec.security[0].extensions_json, &_cdd_jv_1);
      sec_req_ext = _cdd_jv_1;
    }
    sec_req_obj = json_value_get_object(sec_req_ext);
    ASSERT_EQ(1, spec.security_set);
    ASSERT_EQ(1, spec.n_security);
    ASSERT_EQ(1, spec.security[0].n_requirements);
    ASSERT_STR_EQ("api_key", spec.security[0].requirements[0].scheme);
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(sec_req_obj, "x-sec-req", &_cdd_js_1);
      ASSERT_STR_EQ("req", _cdd_js_1);
    }
    g_cdd_last_alloc_err = json_value_free(sec_req_ext);
  }

  {
    struct OpenAPI_Path *path = &spec.paths[0];
    struct OpenAPI_Operation *op = &path->operations[0];
    struct OpenAPI_Parameter *param = &op->parameters[0];
    struct OpenAPI_Response *resp = &op->responses[0];
    struct OpenAPI_Callback *cb = &op->callbacks[0];

    JSON_Value *path_ext, *op_ext, *rb_ext, *param_ext, *resp_ext, *resps_ext,
        *cb_ext;
    JSON_Object *path_obj, *op_obj, *rb_obj, *param_obj, *resp_obj, *resps_obj,
        *cb_obj;

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(path->extensions_json, &_cdd_jv_1);
      path_ext = _cdd_jv_1;
    }
    path_obj = json_value_get_object(path_ext);
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(path_obj, "x-path", &_cdd_js_1);
      ASSERT_STR_EQ("path", _cdd_js_1);
    }
    g_cdd_last_alloc_err = json_value_free(path_ext);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(op->extensions_json, &_cdd_jv_1);
      op_ext = _cdd_jv_1;
    }
    op_obj = json_value_get_object(op_ext);
    {
      double _cdd_jn_1 = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_number(op_obj, "x-op", &_cdd_jn_1);
      ASSERT_EQ(2, (int)_cdd_jn_1);
    }
    g_cdd_last_alloc_err = json_value_free(op_ext);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(op->req_body_extensions_json, &_cdd_jv_1);
      rb_ext = _cdd_jv_1;
    }
    rb_obj = json_value_get_object(rb_ext);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      int _cdd_jb_2 = -1;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(rb_obj, "x-rb", &_cdd_jo_1);
      if (cdd_json_object_get_boolean(_cdd_jo_1, "note", &_cdd_jb_2) !=
          CDD_C_SUCCESS)
        _cdd_jb_2 = -1;
      ASSERT_EQ(1, _cdd_jb_2);
    }
    g_cdd_last_alloc_err = json_value_free(rb_ext);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(param->extensions_json, &_cdd_jv_1);
      param_ext = _cdd_jv_1;
    }
    param_obj = json_value_get_object(param_ext);
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(param_obj, "x-param", &_cdd_js_1);
      ASSERT_STR_EQ("param", _cdd_js_1);
    }
    g_cdd_last_alloc_err = json_value_free(param_ext);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(resp->extensions_json, &_cdd_jv_1);
      resp_ext = _cdd_jv_1;
    }
    resp_obj = json_value_get_object(resp_ext);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      int _cdd_jb_2 = -1;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(resp_obj, "x-resp", &_cdd_jo_1);
      if (cdd_json_object_get_boolean(_cdd_jo_1, "ok", &_cdd_jb_2) !=
          CDD_C_SUCCESS)
        _cdd_jb_2 = -1;
      ASSERT_EQ(1, _cdd_jb_2);
    }
    g_cdd_last_alloc_err = json_value_free(resp_ext);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(op->responses_extensions_json, &_cdd_jv_1);
      resps_ext = _cdd_jv_1;
    }
    resps_obj = json_value_get_object(resps_ext);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      int _cdd_jb_2 = -1;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(resps_obj, "x-responses", &_cdd_jo_1);
      if (cdd_json_object_get_boolean(_cdd_jo_1, "trace", &_cdd_jb_2) !=
          CDD_C_SUCCESS)
        _cdd_jb_2 = -1;
      ASSERT_EQ(1, _cdd_jb_2);
    }
    g_cdd_last_alloc_err = json_value_free(resps_ext);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(cb->extensions_json, &_cdd_jv_1);
      cb_ext = _cdd_jv_1;
    }
    cb_obj = json_value_get_object(cb_ext);
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(cb_obj, "x-cb", &_cdd_js_1);
      ASSERT_STR_EQ("cb", _cdd_js_1);
    }
    g_cdd_last_alloc_err = json_value_free(cb_ext);
  }

  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_paths_webhooks_components_extensions.
 * @return Greatest test result.
 */
TEST test_load_paths_webhooks_components_extensions(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  84,  101, 115, 116, 34,  44,  34,  118, 101,
      114, 115, 105, 111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,
      116, 104, 115, 34,  58,  123, 32,  32,  34,  120, 45,  112, 97,  116, 104,
      115, 34,  58,  123, 34,  110, 111, 116, 101, 34,  58,  116, 114, 117, 101,
      125, 44,  32,  32,  34,  47,  112, 101, 116, 115, 34,  58,  123, 34,  103,
      101, 116, 34,  58,  123, 34,  114, 101, 115, 112, 111, 110, 115, 101, 115,
      34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,
      114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  111, 107, 34,  125, 125,
      125, 125, 125, 44,  34,  119, 101, 98,  104, 111, 111, 107, 115, 34,  58,
      123, 32,  32,  34,  120, 45,  104, 111, 111, 107, 115, 34,  58,  123, 34,
      104, 111, 111, 107, 34,  58,  49,  125, 44,  32,  32,  34,  101, 118, 101,
      110, 116, 123, 116, 121, 112, 101, 125, 34,  58,  123, 34,  112, 111, 115,
      116, 34,  58,  123, 34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,
      58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114,
      105, 112, 116, 105, 111, 110, 34,  58,  34,  111, 107, 34,  125, 125, 125,
      125, 125, 44,  34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 34,
      58,  123, 32,  32,  34,  120, 45,  99,  111, 109, 112, 115, 34,  58,  123,
      34,  109, 101, 116, 97,  34,  58,  34,  121, 101, 115, 34,  125, 44,  32,
      32,  34,  115, 99,  104, 101, 109, 97,  115, 34,  58,  123, 34,  80,  101,
      116, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106,
      101, 99,  116, 34,  125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_paths);
  ASSERT_STR_EQ("/pets", spec.paths[0].route);

  ASSERT_EQ(1, spec.n_webhooks);
  ASSERT_STR_EQ("event{type}", spec.webhooks[0].route);

  {
    JSON_Value *paths_ext = NULL;
    JSON_Object *paths_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(spec.paths_extensions_json, &_cdd_jv_1);
      paths_ext = _cdd_jv_1;
    }
    paths_obj = json_value_get_object(paths_ext);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      int _cdd_jb_2 = -1;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(paths_obj, "x-paths", &_cdd_jo_1);
      if (cdd_json_object_get_boolean(_cdd_jo_1, "note", &_cdd_jb_2) !=
          CDD_C_SUCCESS)
        _cdd_jb_2 = -1;
      ASSERT_EQ(1, _cdd_jb_2);
    }
    g_cdd_last_alloc_err = json_value_free(paths_ext);
  }

  {
    JSON_Value *hooks_ext = NULL;
    JSON_Object *hooks_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(spec.webhooks_extensions_json, &_cdd_jv_1);
      hooks_ext = _cdd_jv_1;
    }
    hooks_obj = json_value_get_object(hooks_ext);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      double _cdd_jn_2 = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(hooks_obj, "x-hooks", &_cdd_jo_1);
      g_cdd_last_alloc_err =
          cdd_json_object_get_number(_cdd_jo_1, "hook", &_cdd_jn_2);
      ASSERT_EQ(1, (int)_cdd_jn_2);
    }
    g_cdd_last_alloc_err = json_value_free(hooks_ext);
  }

  {
    JSON_Value *comps_ext = NULL;
    JSON_Object *comps_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(spec.components_extensions_json, &_cdd_jv_1);
      comps_ext = _cdd_jv_1;
    }
    comps_obj = json_value_get_object(comps_ext);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      const char *_cdd_js_2 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(comps_obj, "x-comps", &_cdd_jo_1);
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(_cdd_jo_1, "meta", &_cdd_js_2);
      ASSERT_STR_EQ("yes", _cdd_js_2);
    }
    g_cdd_last_alloc_err = json_value_free(comps_ext);
  }

  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_webhook_path_template_not_validated.
 * @return Greatest test result.
 */
TEST test_webhook_path_template_not_validated(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  84,  101, 115, 116, 34,  44,  34,  118, 101,
      114, 115, 105, 111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  119, 101,
      98,  104, 111, 111, 107, 115, 34,  58,  123, 32,  32,  34,  47,  101, 118,
      101, 110, 116, 115, 47,  123, 101, 118, 101, 110, 116, 73,  100, 125, 34,
      58,  123, 32,  32,  32,  32,  34,  112, 111, 115, 116, 34,  58,  123, 32,
      32,  32,  32,  32,  32,  34,  114, 101, 115, 112, 111, 110, 115, 101, 115,
      34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,
      114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  111, 107, 34,  125, 125,
      32,  32,  32,  32,  125, 32,  32,  125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_webhooks);
  ASSERT_STR_EQ("/events/{eventId}", spec.webhooks[0].route);

  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_component_schema_raw.
 * @return Greatest test result.
 */
TEST test_load_component_schema_raw(void) {

  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"components\":{\"schemas\":{"
      "\"Token\":{\"type\":\"string\"},"
      "\"Flag\":true,"
      "\"Nums\":{\"type\":\"array\",\"items\":{\"type\":\"integer\"}}"
      "}},"
      "\"paths\":{}"
      "}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, spec.n_defined_schemas);
  ASSERT_EQ(3, spec.n_raw_schemas);

  {
    int idx = find_raw_schema_index(&spec, "Token");
    JSON_Value *val;
    JSON_Object *obj;
    ASSERT(idx >= 0);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(spec.raw_schema_json[idx], &_cdd_jv_1);
      val = _cdd_jv_1;
    }
    obj = json_value_get_object(val);
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(obj, "type", &_cdd_js_1);
      ASSERT_STR_EQ("string", _cdd_js_1);
    }
    g_cdd_last_alloc_err = json_value_free(val);
  }

  {
    int idx = find_raw_schema_index(&spec, "Flag");
    JSON_Value *val;
    ASSERT(idx >= 0);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(spec.raw_schema_json[idx], &_cdd_jv_1);
      val = _cdd_jv_1;
    }
    ASSERT_EQ(JSONBoolean, json_value_get_type(val));
    ASSERT_EQ(1, json_value_get_boolean(val));
    g_cdd_last_alloc_err = json_value_free(val);
  }

  {
    int idx = find_raw_schema_index(&spec, "Nums");
    JSON_Value *val;
    JSON_Object *obj;
    JSON_Object *items;
    ASSERT(idx >= 0);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string(spec.raw_schema_json[idx], &_cdd_jv_1);
      val = _cdd_jv_1;
    }
    obj = json_value_get_object(val);
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(obj, "type", &_cdd_js_1);
      ASSERT_STR_EQ("array", _cdd_js_1);
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(obj, "items", &_cdd_jo_1);
      items = _cdd_jo_1;
    }
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(items, "type", &_cdd_js_1);
      ASSERT_STR_EQ("integer", _cdd_js_1);
    }
    g_cdd_last_alloc_err = json_value_free(val);
  }

  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_schema_external_ref.
 * @return Greatest test result.
 */
TEST test_load_schema_external_ref(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  84,  34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 101, 116, 115, 34,  58,  123, 34,  103, 101,
      116, 34,  58,  123, 34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,
      58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114,
      105, 112, 116, 105, 111, 110, 34,  58,  34,  111, 107, 34,  44,  34,  99,
      111, 110, 116, 101, 110, 116, 34,  58,  123, 34,  97,  112, 112, 108, 105,
      99,  97,  116, 105, 111, 110, 47,  106, 115, 111, 110, 34,  58,  123, 34,
      115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  36,  114, 101, 102, 34,
      58,  34,  104, 116, 116, 112, 115, 58,  47,  47,  101, 120, 97,  109, 112,
      108, 101, 46,  99,  111, 109, 47,  115, 99,  104, 101, 109, 97,  115, 47,
      80,  101, 116, 34,  125, 125, 125, 125, 125, 125, 125, 125, 125, 0};
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("https://example.com/schemas/Pet",
                spec.paths[0].operations[0].responses[0].schema.ref);
  ASSERT(spec.paths[0].operations[0].responses[0].schema.ref_name == NULL);
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_schema_ref_with_pointer_is_not_component.
 * @return Greatest test result.
 */
TEST test_load_schema_ref_with_pointer_is_not_component(void) {
  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  84,  34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 101, 116, 115, 34,  58,  123, 34,  103, 101,
      116, 34,  58,  123, 34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,
      58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114,
      105, 112, 116, 105, 111, 110, 34,  58,  34,  111, 107, 34,  44,  34,  99,
      111, 110, 116, 101, 110, 116, 34,  58,  123, 34,  97,  112, 112, 108, 105,
      99,  97,  116, 105, 111, 110, 47,  106, 115, 111, 110, 34,  58,  123, 34,
      115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  36,  114, 101, 102, 34,
      58,  34,  35,  47,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 47,
      115, 99,  104, 101, 109, 97,  115, 47,  80,  101, 116, 47,  112, 114, 111,
      112, 101, 114, 116, 105, 101, 115, 47,  105, 100, 34,  125, 125, 125, 125,
      125, 125, 125, 125, 125, 0};
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("#/components/schemas/Pet/properties/id",
                spec.paths[0].operations[0].responses[0].schema.ref);
  ASSERT(spec.paths[0].operations[0].responses[0].schema.ref_name == NULL);
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_schema_external_items_ref.
 * @return Greatest test result.
 */
TEST test_load_schema_external_items_ref(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  84,  34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 101, 116, 115, 34,  58,  123, 34,  103, 101,
      116, 34,  58,  123, 34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,
      58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114,
      105, 112, 116, 105, 111, 110, 34,  58,  34,  111, 107, 34,  44,  34,  99,
      111, 110, 116, 101, 110, 116, 34,  58,  123, 34,  97,  112, 112, 108, 105,
      99,  97,  116, 105, 111, 110, 47,  106, 115, 111, 110, 34,  58,  123, 34,
      115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,
      58,  34,  97,  114, 114, 97,  121, 34,  44,  34,  105, 116, 101, 109, 115,
      34,  58,  123, 34,  36,  114, 101, 102, 34,  58,  34,  104, 116, 116, 112,
      115, 58,  47,  47,  101, 120, 97,  109, 112, 108, 101, 46,  99,  111, 109,
      47,  115, 99,  104, 101, 109, 97,  115, 47,  80,  101, 116, 34,  125, 125,
      125, 125, 125, 125, 125, 125, 125, 125, 0};
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].schema.is_array);
  ASSERT_STR_EQ("https://example.com/schemas/Pet",
                spec.paths[0].operations[0].responses[0].schema.items_ref);
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_path_template_missing_param.
 * @return Greatest test result.
 */
TEST test_load_path_template_missing_param(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/pets/{petId}\":{\"get\":{"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}"
      "}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Unit test test_load_path_template_param_not_in_route.
 * @return Greatest test result.
 */
TEST test_load_path_template_param_not_in_route(void) {

  const char *json = "{\"paths\":{\"/"
                     "pets\":{\"parameters\":[{\"name\":\"petId\",\"in\":"
                     "\"path\",\"required\":true,\"schema\":{\"type\":"
                     "\"string\"}}],\"get\":{\"responses\":{\"200\":{"
                     "\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Unit test test_load_path_template_param_not_required.
 * @return Greatest test result.
 */
TEST test_load_path_template_param_not_required(void) {

  const char *json = "{\"paths\":{\"/pets/"
                     "{petId}\":{\"parameters\":[{\"name\":\"petId\",\"in\":"
                     "\"path\",\"required\":false,\"schema\":{\"type\":"
                     "\"string\"}}],\"get\":{\"responses\":{\"200\":{"
                     "\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Unit test test_load_root_missing_paths_components_webhooks_rejected.
 * @return Greatest test result.
 */
TEST test_load_root_missing_paths_components_webhooks_rejected(void) {

  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{"
                     "\"title\":\"Example API\",\"version\":\"1\"}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Unit test test_param_style_invalid_for_in_rejected.
 * @return Greatest test result.
 */
TEST test_param_style_invalid_for_in_rejected(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  84,  34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 101, 116, 115, 34,  58,  123, 34,  103, 101,
      116, 34,  58,  123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115,
      34,  58,  91,  123, 34,  110, 97,  109, 101, 34,  58,  34,  105, 100, 34,
      44,  34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 34,  44,  34,
      115, 116, 121, 108, 101, 34,  58,  34,  109, 97,  116, 114, 105, 120, 34,
      44,  34,  115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112,
      101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 93,  44,
      34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,
      48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105,
      111, 110, 34,  58,  34,  111, 107, 34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Unit test test_param_style_deep_object_scalar_rejected.
 * @return Greatest test result.
 */
TEST test_param_style_deep_object_scalar_rejected(void) {

  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/pets\":{\"get\":{"
      "\"parameters\":[{\"name\":\"filter\",\"in\":\"query\","
      "\"style\":\"deepObject\",\"schema\":{\"type\":\"string\"}}],"
      "\"responses\":{\"200\":{\"description\":\"ok\"}}"
      "}}}"
      "}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Unit test test_server_url_variable_missing_definition_rejected.
 * @return Greatest test result.
 */
TEST test_server_url_variable_missing_definition_rejected(void) {

  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"servers\":[{\"url\":\"https://{env}.example.com\"}],"
      "\"paths\":{\"/pets\":{\"get\":{\"responses\":{\"200\":{\"description\":"
      "\"ok\"}}}}}"
      "}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Unit test test_server_url_variable_duplicate_rejected.
 * @return Greatest test result.
 */
TEST test_server_url_variable_duplicate_rejected(void) {

  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"servers\":[{\"url\":\"https://{env}.example.com/{env}\","
      "\"variables\":{\"env\":{\"default\":\"prod\"}}}],"
      "\"paths\":{\"/pets\":{\"get\":{\"responses\":{\"200\":{\"description\":"
      "\"ok\"}}}}}"
      "}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Unit test test_load_server_missing_url_rejected.
 * @return Greatest test result.
 */
TEST test_load_server_missing_url_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"servers\":[{\"description\":\"No URL\"}],"
      "\"paths\":{}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Unit test test_load_additional_operations_standard_method_rejected.
 * @return Greatest test result.
 */
TEST test_load_additional_operations_standard_method_rejected(void) {

  const char *json =
      "{\"paths\":{\"/"
      "x\":{\"additionalOperations\":{\"POST\":{\"responses\":{\"200\":{"
      "\"description\":\"ok\"}}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Unit test test_load_link_missing_operation_ref_or_id_rejected.
 * @return Greatest test result.
 */
TEST test_load_link_missing_operation_ref_or_id_rejected(void) {

  const char *json = "{\"components\":{\"links\":{\"BadLink\":{\"parameters\":{"
                     "\"id\":1}}}},\"paths\":{\"/"
                     "x\":{\"get\":{\"responses\":{\"200\":{\"description\":"
                     "\"ok\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Unit test test_load_link_operation_ref_and_id_both_rejected.
 * @return Greatest test result.
 */
TEST test_load_link_operation_ref_and_id_both_rejected(void) {

  const char *json =
      "{\"components\":{\"links\":{\"BadLink\":{\"operationId\":\"op\","
      "\"operationRef\":\"#/paths/~1x/get\"}}},\"paths\":{\"/"
      "x\":{\"get\":{\"responses\":{\"200\":{\"description\":\"ok\"}}}}},"
      "\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Unit test test_load_link_full_fields.
 * @return Greatest test result.
 */
TEST test_load_link_full_fields(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"components\":{\"links\":{\"FullLink\":{"
      "\"operationId\":\"getFoo\","
      "\"summary\":\"Link summary\","
      "\"description\":\"Link desc\","
      "\"parameters\":{\"p1\":\"$response.body#/id\",\"p2\":\"const\"},"
      "\"requestBody\":\"$request.body\","
      "\"server\":{\"url\":\"http://link-server.com\"}"
      "}}},"
      "\"paths\":{\"/"
      "x\":{\"get\":{\"operationId\":\"getFoo\",\"responses\":{\"200\":{"
      "\"description\":\"ok\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.n_component_links);
  ASSERT_STR_EQ("FullLink", spec.component_links[0].name);
  ASSERT_STR_EQ("Link summary", spec.component_links[0].summary);
  ASSERT_STR_EQ("Link desc", spec.component_links[0].description);
  ASSERT_EQ(2, spec.component_links[0].n_parameters);
  ASSERT_EQ(1, spec.component_links[0].request_body_set);
  ASSERT_EQ(1, spec.component_links[0].server_set);
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_schema_content_schema.
 * @return Greatest test result.
 */
TEST test_load_schema_content_schema(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/x\":{\"get\":{\"parameters\":[{"
      "\"name\":\"p1\",\"in\":\"query\","
      "\"schema\":{\"type\":\"string\",\"contentMediaType\":\"application/"
      "json\","
      "\"contentEncoding\":\"base64\","
      "\"contentSchema\":{\"type\":\"object\"},"
      "\"examples\":[\"ex1\",\"ex2\"],\"const\":\"myconst\"}},{"
      "\"name\":\"p2\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\","
      "\"items\":{\"type\":\"string\",\"contentSchema\":{\"type\":\"string\"}}}"
      "}"
      "],"
      "\"responses\":{\"200\":{\"summary\":\"s\",\"description\":\"ok\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(2, spec.paths[0].operations[0].n_parameters);
  ASSERT(spec.paths[0].operations[0].parameters[0].schema.content_schema !=
         NULL);
  ASSERT(
      spec.paths[0].operations[0].parameters[1].schema.items_content_schema !=
      NULL);
  ASSERT_EQ(2, spec.paths[0].operations[0].parameters[0].schema.n_examples);
  ASSERT_EQ(1,
            spec.paths[0].operations[0].parameters[0].schema.const_value_set);
  ASSERT_STR_EQ("s", spec.paths[0].operations[0].responses[0].summary);
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_nested_encoding_fields.
 * @return Greatest test result.
 */
TEST test_load_nested_encoding_fields(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/x\":{\"post\":{\"requestBody\":{\"content\":{\"multipart/"
      "form-data\":{"
      "\"encoding\":{\"f1\":{\"contentType\":\"image/png\","
      "\"headers\":{\"X-Hdr\":{\"description\":\"h\",\"schema\":{\"type\":"
      "\"string\"}}},"
      "\"prefixEncoding\":[{\"style\":\"form\"}],\"itemEncoding\":{\"style\":"
      "\"form\"}}}"
      "}}},"
      "\"responses\":{\"200\":{\"description\":\"ok\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.n_paths);
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_parameter_and_header_content_media_types.
 * @return Greatest test result.
 */
TEST test_load_parameter_and_header_content_media_types(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/x\":{\"get\":{\"parameters\":[{"
      "\"name\":\"p\",\"in\":\"query\","
      "\"content\":{\"a/j\":{\"schema\":{\"type\":\"string\"}}},"
      "\"examples\":{\"e1\":{\"value\":\"v1\"}}"
      "}],"
      "\"responses\":{\"200\":{\"description\":\"ok\","
      "\"headers\":{\"X-H\":{"
      "\"content\":{\"t/p\":{\"schema\":{\"type\":\"string\"}}},"
      "\"examples\":{\"e1\":{\"value\":\"hello\"}}"
      "}},"
      "\"content\":{\"a/j\":{\"schema\":{\"type\":\"string\"}}},"
      "\"links\":{\"L1\":{\"operationId\":\"op\",\"description\":\"ld\"}}"
      "}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(1, spec.paths[0].operations[0].parameters[0].n_content_media_types);
  ASSERT_EQ(1, spec.paths[0].operations[0].parameters[0].n_examples);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].n_headers);
  ASSERT_EQ(1, spec.paths[0]
                   .operations[0]
                   .responses[0]
                   .headers[0]
                   .n_content_media_types);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].headers[0].n_examples);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].n_links);
  g_cdd_last_alloc_err = openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_PATHS_H */
