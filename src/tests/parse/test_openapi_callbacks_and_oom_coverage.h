/**
 * @file test_openapi_callbacks_and_oom_coverage.h
 * @brief Callback object parsing and OOM simulation tests for responses,
 * request bodies, and callbacks.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_CALLBACKS_AND_OOM_COVERAGE_H
#define TEST_OPENAPI_CALLBACKS_AND_OOM_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Tests parse_callback_object and parse_callbacks_object branches.
 * @return Greatest test result.
 */
TEST test_openapi_callbacks_coverage(void) {
  struct OpenAPI_Callback cb;
  struct OpenAPI_Callback *out_cbs = NULL;
  size_t out_cb_count = 0;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  struct OpenAPI_Spec spec;

  /* 1. NULL checks */
  memset(&cb, 0, sizeof(cb));
  ASSERT_EQ(CDD_C_SUCCESS, parse_callback_object(NULL, &cb, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, parse_callback_object((const JSON_Object *)(size_t)1,
                                                 NULL, NULL, 0));

  ASSERT_EQ(CDD_C_SUCCESS, parse_callbacks_object(NULL, NULL, NULL, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_callbacks_object((const JSON_Object *)(size_t)1, NULL,
                                   &out_cb_count, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_callbacks_object((const JSON_Object *)(size_t)1, &out_cbs,
                                   NULL, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_callbacks_object(NULL, &out_cbs, &out_cb_count, NULL, 0));

  /* 10. Callback object ref and extensions */
  memset(&spec, 0, sizeof(spec));
  spec.n_component_callbacks = 1;
  spec.component_callbacks =
      (struct OpenAPI_Callback *)calloc(1, sizeof(struct OpenAPI_Callback));
  spec.component_callbacks[0].name = strdup("MyCb");
  spec.component_callbacks[0].summary = strdup("comp_sum");

  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string(
        "{\"$ref\":\"#/components/callbacks/"
        "MyCb\",\"summary\":\"sum\",\"description\":\"desc\"}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&cb, 0, sizeof(cb));
    ASSERT_EQ(CDD_C_SUCCESS, parse_callback_object(jo, &cb, &spec, 1));
    ASSERT_STR_EQ("sum", cb.summary);
    ASSERT_STR_EQ("desc", cb.description);
    free_callback(&cb);

    /* ref with resolve_refs = 0 and spec = NULL */
    memset(&cb, 0, sizeof(cb));
    ASSERT_EQ(CDD_C_SUCCESS, parse_callback_object(jo, &cb, NULL, 0));
    free_callback(&cb);

    g_cdd_strdup_fail = 2;
    memset(&cb, 0, sizeof(cb));
    ASSERT_NEQ(CDD_C_SUCCESS, parse_callback_object(jo, &cb, &spec, 1));
    g_cdd_strdup_fail = 0;
    free_callback(&cb);

    memset(&cb, 0, sizeof(cb));
    ASSERT_EQ(CDD_C_SUCCESS, parse_callback_object(jo, &cb, NULL, 1));
    free_callback(&cb);
    json_value_free(jv);

    /* ref without summary and without description */
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string("{\"$ref\":\"#/components/callbacks/MyCb\"}",
                            &_cdd_jv_1);
      jv = _cdd_jv_1;
    }
    if (jv) {
      memset(&cb, 0, sizeof(cb));
      ASSERT_EQ(CDD_C_SUCCESS, parse_callback_object(json_value_get_object(jv),
                                                     &cb, &spec, 1));
      free_callback(&cb);
      json_value_free(jv);
    }
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"$ref\":\"#/components/callbacks/NotFound\"}",
                          &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    memset(&cb, 0, sizeof(cb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_callback_object(json_value_get_object(jv), &cb, &spec, 1));
    free_callback(&cb);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* Callback with paths and extensions */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string(
        "{\"x-ext\":\"val\",\"{$request.query.url}\":{\"post\":"
        "{\"responses\":{\"200\":{\"description\":\"OK\"}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&cb, 0, sizeof(cb));
    ASSERT_EQ(CDD_C_SUCCESS, parse_callback_object(jo, &cb, NULL, 0));
    free_callback(&cb);
    json_value_free(jv);
  }

  /* Pre-set summary and description on callback */
  memset(&cb, 0, sizeof(cb));
  cb.summary = strdup("existing_sum");
  cb.description = strdup("existing_desc");
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"$ref\":\"#/components/callbacks/"
                          "MyCb\",\"summary\":\"s\",\"description\":\"d\"}",
                          &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, parse_callback_object(jo, &cb, NULL, 0));
    free_callback(&cb);
    json_value_free(jv);
  }

  /* Callback with invalid method in paths */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"/path\":{\"invalid_method\":{}}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    cdd_c_error_t rc_cb;
    memset(&cb, 0, sizeof(cb));
    rc_cb = parse_callback_object(json_value_get_object(jv), &cb, NULL, 0);
    if (rc_cb != CDD_C_SUCCESS) {
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc_cb);
    }
    free_callback(&cb);
    json_value_free(jv);
  }

  /* 11. parse_callbacks_object */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    out_cbs = NULL;
    out_cb_count = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_callbacks_object(json_value_get_object(jv), &out_cbs,
                                     &out_cb_count, NULL, 0));
    ASSERT_EQ(0, out_cb_count);
    json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_string("{\"cb1\":{\"x-ext\":\"v\"},\"cb2\":123}",
                          &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    out_cbs = NULL;
    out_cb_count = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_callbacks_object(json_value_get_object(jv), &out_cbs,
                                     &out_cb_count, NULL, 0));
    ASSERT_EQ(2, out_cb_count);
    free_callback(&out_cbs[0]);
    free_callback(&out_cbs[1]);
    free(out_cbs);
    json_value_free(jv);
  }

  PASS();
}

/**
 * @brief Tests OOM resilience across responses, request bodies, and callbacks.
 * @return Greatest test result.
 */
TEST test_openapi_responses_request_bodies_callbacks_oom(void) {
  struct OpenAPI_RequestBody rb;
  struct OpenAPI_Response resp;
  struct OpenAPI_Operation op;
  struct OpenAPI_Callback *out_cbs = NULL;
  size_t out_cb_count = 0;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  struct OpenAPI_Spec spec;
  int k;
  size_t i;

  /* 12. OOM loop on responses, request bodies, and callbacks */
  {
    const char *oom_resps[13];
    size_t n_oom = 13;
    oom_resps[0] =
        "{\"description\":\"desc\",\"summary\":\"sum\","
        "\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"object\",\"properties\":{\"x\":{"
        "\"type\":\"string\"}}},"
        "\"examples\":{\"e1\":{\"value\":\"v1\"}}}},"
        "\"headers\":{\"X-H\":{\"description\":\"h\"}},"
        "\"links\":{\"L\":{\"operationId\":\"op\"}},\"x-ext\":\"val\"}";
    oom_resps[1] = "{\"content\":{\"application/"
                   "json\":{\"schema\":{\"type\":\"object\",\"properties\":{"
                   "\"x\":{\"type\":\"string\"}}},"
                   "\"examples\":{\"e1\":{\"value\":\"v1\"}}}},\"description\":"
                   "\"desc\",\"x-ext\":\"val\"}";
    oom_resps[2] =
        "{\"200\":{\"description\":\"OK\",\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"string\"}}}},\"x-resp-ext\":\"val\"}";
    oom_resps[3] = "{\"myCb\":{\"x-ext\":\"val\"}}";
    oom_resps[4] =
        "{\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"object\","
        "\"properties\":{\"a\":{\"type\":\"string\"}}}}}}}";
    oom_resps[5] = "{\"content\":{\"application/"
                   "json\":{\"itemSchema\":{\"type\":\"object\",\"properties\":"
                   "{\"a\":{\"type\":\"string\"}}}}}}";
    oom_resps[6] =
        "{\"description\":\"d\",\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"object\","
        "\"properties\":{\"a\":{\"type\":\"string\"}}}}}}}";
    oom_resps[7] = "{\"description\":\"d\",\"content\":{\"application/"
                   "json\":{\"itemSchema\":{\"type\":\"object\",\"properties\":"
                   "{\"a\":{\"type\":\"string\"}}}}}}";
    oom_resps[8] = "{\"content\":{\"application/json\":{\"$ref\":\"#/"
                   "components/mediaTypes/Mt\"}}}";
    oom_resps[9] = "{\"description\":\"d\",\"content\":{\"application/"
                   "json\":{\"$ref\":\"#/components/mediaTypes/Mt\"}}}";
    oom_resps[10] = "{\"content\":{\"application/"
                    "json\":{\"schema\":{\"type\":\"string\"}}}}";
    oom_resps[11] = "{\"myCb\":{\"$ref\":\"#/components/callbacks/"
                    "MyCb\",\"summary\":\"s\",\"description\":\"d\"}}";
    oom_resps[12] = "{\"description\":\"d\",\"content\":{\"application/"
                    "json\":{\"example\":\"single_val\"}}}";

    memset(&spec, 0, sizeof(spec));
    spec.n_component_callbacks = 1;
    spec.component_callbacks =
        (struct OpenAPI_Callback *)calloc(1, sizeof(struct OpenAPI_Callback));
    spec.component_callbacks[0].name = strdup("MyCb");

    for (i = 0; i < n_oom; ++i) {
      {
        JSON_Value *_cdd_jv_1 = NULL;
        cdd_json_parse_string(oom_resps[i], &_cdd_jv_1);
        jv = _cdd_jv_1;
      }
      if (jv) {
        jo = json_value_get_object(jv);
        for (k = 1; k <= 30; ++k) {
          cdd_c_error_t rc_oom;
          if (i == 0 || i == 6 || i == 7 || i == 9 || i == 12) {
            g_cdd_alloc_fail = k;
            memset(&resp, 0, sizeof(resp));
            rc_oom = parse_response_object(jo, &resp, &spec, 1, "op", "200");
            if (rc_oom != CDD_C_SUCCESS)
              ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_oom);
            free_response(&resp);
            g_cdd_alloc_fail = 0;

            g_cdd_strdup_fail = k;
            memset(&resp, 0, sizeof(resp));
            rc_oom = parse_response_object(jo, &resp, &spec, 1, "op", "200");
            if (rc_oom != CDD_C_SUCCESS)
              ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_oom);
            free_response(&resp);
            g_cdd_strdup_fail = 0;
          } else if (i == 1 || i == 4 || i == 5 || i == 8 || i == 10) {
            g_cdd_alloc_fail = k;
            memset(&rb, 0, sizeof(rb));
            rc_oom = parse_request_body_object(jo, &rb, &spec, 1, "op");
            if (rc_oom != CDD_C_SUCCESS)
              ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_oom);
            free_request_body(&rb);
            g_cdd_alloc_fail = 0;

            g_cdd_strdup_fail = k;
            memset(&rb, 0, sizeof(rb));
            rc_oom = parse_request_body_object(jo, &rb, &spec, 1, "op");
            if (rc_oom != CDD_C_SUCCESS)
              ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_oom);
            free_request_body(&rb);
            g_cdd_strdup_fail = 0;
          } else if (i == 2) {
            g_cdd_alloc_fail = k;
            memset(&op, 0, sizeof(op));
            rc_oom = parse_responses(jo, &op, &spec, "op");
            if (rc_oom != CDD_C_SUCCESS)
              ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_oom);
            free_operation(&op);
            g_cdd_alloc_fail = 0;

            g_cdd_strdup_fail = k;
            memset(&op, 0, sizeof(op));
            rc_oom = parse_responses(jo, &op, &spec, "op");
            if (rc_oom != CDD_C_SUCCESS)
              ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_oom);
            free_operation(&op);
            g_cdd_strdup_fail = 0;
          } else if (i == 3 || i == 11) {
            g_cdd_alloc_fail = k;
            out_cbs = NULL;
            out_cb_count = 0;
            rc_oom =
                parse_callbacks_object(jo, &out_cbs, &out_cb_count, &spec, 1);
            if (rc_oom != CDD_C_SUCCESS)
              ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_oom);
            if (out_cbs) {
              size_t m;
              for (m = 0; m < out_cb_count; ++m)
                free_callback(&out_cbs[m]);
              free(out_cbs);
            }
            g_cdd_alloc_fail = 0;

            g_cdd_strdup_fail = k;
            out_cbs = NULL;
            out_cb_count = 0;
            rc_oom =
                parse_callbacks_object(jo, &out_cbs, &out_cb_count, &spec, 1);
            if (rc_oom != CDD_C_SUCCESS)
              ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_oom);
            if (out_cbs) {
              size_t m;
              for (m = 0; m < out_cb_count; ++m)
                free_callback(&out_cbs[m]);
              free(out_cbs);
            }
            g_cdd_strdup_fail = 0;
          }
        }
        json_value_free(jv);
      }
    }
    openapi_spec_free(&spec);
  }

  g_cdd_alloc_fail = 0;
  g_cdd_strdup_fail = 0;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_CALLBACKS_AND_OOM_COVERAGE_H */
