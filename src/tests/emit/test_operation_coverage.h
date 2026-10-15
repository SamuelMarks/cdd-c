/**
 * @file test_operation_coverage.h
 * @brief Comprehensive coverage tests for operation generator.
 */

#ifndef TEST_OPERATION_COVERAGE_H
#define TEST_OPERATION_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_operation_common.h"
/* clang-format on */

TEST test_operation_100_percent_coverage(void) {
  struct OpenAPI_Link link;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp;
  struct OpenAPI_Operation op;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Any any;
  struct OpenAPI_Server srv;
  struct DocResponseHeader dh;
  struct DocLink dl;
  struct DocServer ds;
  struct DocServerVar dsv;
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[2];
  struct DocMetadata doc;
  struct DocResponse ret;
  char *enums[2];
  JSON_Value *val_null;
  JSON_Value *val_obj;
  struct OpenAPI_LinkParam *lp = NULL;
  size_t lp_count = 0;

  memset(&link, 0, sizeof(link));
  memset(&param, 0, sizeof(param));
  memset(&resp, 0, sizeof(resp));
  memset(&op, 0, sizeof(op));
  memset(&mt, 0, sizeof(mt));
  memset(&any, 0, sizeof(any));
  memset(&srv, 0, sizeof(srv));
  memset(&dh, 0, sizeof(dh));
  memset(&dl, 0, sizeof(dl));
  memset(&ds, 0, sizeof(ds));
  memset(&dsv, 0, sizeof(dsv));
  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(args, 0, sizeof(args));
  memset(&doc, 0, sizeof(doc));
  memset(&ret, 0, sizeof(ret));

  /* 1. cleanup_link_fields */

  ASSERT_EQ(CDD_C_SUCCESS, cleanup_link_fields(NULL));
  link.server =
      (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
  ASSERT(link.server != NULL);
  link.server->name = (char *)malloc(12);
  ASSERT(link.server->name != NULL);
  CDD_STRCPY(link.server->name, 12, "ServerName");
  link.server->url = (char *)malloc(20);
  ASSERT(link.server->url != NULL);
  CDD_STRCPY(link.server->url, 20, "http://example.com");
  link.server->description = (char *)malloc(12);
  ASSERT(link.server->description != NULL);
  CDD_STRCPY(link.server->description, 12, "ServerDesc");
  link.server_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cleanup_link_fields(&link));

  /* 2. free_param_fields */
  ASSERT_EQ(CDD_C_SUCCESS, free_param_fields(NULL));
  param.name = (char *)malloc(5);
  ASSERT(param.name != NULL);
  CDD_STRCPY(param.name, 5, "name");
  param.type = (char *)malloc(5);
  ASSERT(param.type != NULL);
  CDD_STRCPY(param.type, 5, "type");
  param.description = (char *)malloc(5);
  ASSERT(param.description != NULL);
  CDD_STRCPY(param.description, 5, "desc");
  param.items_type = (char *)malloc(5);
  ASSERT(param.items_type != NULL);
  CDD_STRCPY(param.items_type, 5, "item");
  param.content_type = (char *)malloc(5);
  ASSERT(param.content_type != NULL);
  CDD_STRCPY(param.content_type, 5, "text");
  param.schema.ref_name = (char *)malloc(5);
  ASSERT(param.schema.ref_name != NULL);
  CDD_STRCPY(param.schema.ref_name, 5, "rnam");
  param.schema.ref = (char *)malloc(5);
  ASSERT(param.schema.ref != NULL);
  CDD_STRCPY(param.schema.ref, 5, "href");
  param.schema.inline_type = (char *)malloc(5);
  ASSERT(param.schema.inline_type != NULL);
  CDD_STRCPY(param.schema.inline_type, 5, "int");
  param.schema.items_ref = (char *)malloc(5);
  ASSERT(param.schema.items_ref != NULL);
  CDD_STRCPY(param.schema.items_ref, 5, "iref");
  param.schema.format = (char *)malloc(5);
  ASSERT(param.schema.format != NULL);
  CDD_STRCPY(param.schema.format, 5, "fmt");
  param.schema.items_format = (char *)malloc(5);
  ASSERT(param.schema.items_format != NULL);
  CDD_STRCPY(param.schema.items_format, 5, "ifmt");
  param.schema.content_media_type = (char *)malloc(5);
  ASSERT(param.schema.content_media_type != NULL);
  CDD_STRCPY(param.schema.content_media_type, 5, "cmed");
  param.schema.content_encoding = (char *)malloc(5);
  ASSERT(param.schema.content_encoding != NULL);
  CDD_STRCPY(param.schema.content_encoding, 5, "cenc");
  param.schema.items_content_media_type = (char *)malloc(5);
  ASSERT(param.schema.items_content_media_type != NULL);
  CDD_STRCPY(param.schema.items_content_media_type, 5, "imed");
  param.schema.items_content_encoding = (char *)malloc(5);
  ASSERT(param.schema.items_content_encoding != NULL);
  CDD_STRCPY(param.schema.items_content_encoding, 5, "ienc");
  param.example_set = 1;
  param.example.type = OA_ANY_STRING;
  param.example.string = (char *)malloc(5);
  ASSERT(param.example.string != NULL);
  CDD_STRCPY(param.example.string, 5, "str");
  ASSERT_EQ(CDD_C_SUCCESS, free_param_fields(&param));

  param.example_set = 1;
  param.example.type = OA_ANY_JSON;
  param.example.json = (char *)malloc(5);
  ASSERT(param.example.json != NULL);
  CDD_STRCPY(param.example.json, 5, "{}");
  ASSERT_EQ(CDD_C_SUCCESS, free_param_fields(&param));

  /* 3. any_from_json_value */
  val_null = json_value_init_null();
  ASSERT_EQ(CDD_C_SUCCESS, any_from_json_value(val_null, &any));
  ASSERT_EQ(OA_ANY_NULL, any.type);
  json_value_free(val_null);

  val_obj = json_parse_string("{\"key\": \"value\"}");
  ASSERT(val_obj != NULL);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, any_from_json_value(val_obj, &any));
  g_cdd_strdup_fail = 0;
  json_value_free(val_obj);

  /* 4. parse_link_params_json calloc OOM */
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_link_params_json("{\"param1\": \"value1\"}", &lp, &lp_count));
  g_cdd_alloc_fail = 0;

  /* 5. copy_doc_server_variables_op calloc OOM */
  dsv.name = cdd_test_mut_str("env");
  dsv.default_value = cdd_test_mut_str("dev");
  enums[0] = cdd_test_mut_str("dev");
  enums[1] = cdd_test_mut_str("prod");
  dsv.enum_values = enums;
  dsv.n_enum_values = 2;
  ds.url = cdd_test_mut_str("https://api.example.com");
  ds.variables = &dsv;
  ds.n_variables = 1;

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_doc_server_variables_op(&srv, &ds));
  g_cdd_alloc_fail = 0;

  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_doc_server_variables_op(&srv, &ds));
  g_cdd_alloc_fail = 0;

  /* 6. add_header_to_response realloc OOM */
  dh.name = cdd_test_mut_str("X-Custom");
  dh.type = cdd_test_mut_str("string");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_alloc_fail = 0;

  /* 7. add_link_to_response realloc & calloc OOM */
  dl.name = cdd_test_mut_str("LinkName");
  dl.operation_id = cdd_test_mut_str("opId");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_link_to_response(&resp, &dl));
  g_cdd_alloc_fail = 0;

  dl.server_url = cdd_test_mut_str("http://server.com");
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_link_to_response(&resp, &dl));
  g_cdd_alloc_fail = 0;
  free(resp.links);
  resp.links = NULL;

  /* 8. add_param_to_op realloc OOM */
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_param_to_op(&op, &param));
  g_cdd_alloc_fail = 0;

  /* 9. add_response_media_type & add_request_body_media_type calloc & realloc
   * OOM */
  memset(&resp, 0, sizeof(resp));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_response_media_type(&resp, "application/json", 0));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS,
            add_response_media_type(&resp, "application/json", 0));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_response_media_type(&resp, "text/plain", 0));
  g_cdd_alloc_fail = 0;
  free(resp.content_media_types[0].name);
  free(resp.content_media_types);
  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_request_body_media_type(&op, "application/json", 0));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS,
            add_request_body_media_type(&op, "application/json", 0));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_request_body_media_type(&op, "text/plain", 0));
  g_cdd_alloc_fail = 0;
  reset_operation_test(&op);

  /* 11. Primitive output argument (hits lines 2100-2102) */
  memset(&op, 0, sizeof(op));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_test_primitive_out";
  sig.args = args;
  sig.n_args = 1;
  args[0].name = cdd_test_mut_str("out_val");
  args[0].type = cdd_test_mut_str("int **");
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("integer", op.responses[0].schema.inline_type);
  reset_operation_test(&op);

  /* 12. ensure_response_for_code realloc OOM */
  memset(&op, 0, sizeof(op));
  {
    struct OpenAPI_Response *out_resp = NULL;
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_SUCCESS, ensure_response_for_code(&op, "200", &out_resp));
    ASSERT_EQ(NULL, out_resp);
    g_cdd_alloc_fail = 0;
  }

  /* 13. New response in doc.returns realloc OOM (line 2495) */
  memset(&op, 0, sizeof(op));
  ret.code = cdd_test_mut_str("404");
  doc.returns = &ret;
  doc.n_returns = 1;
  sig.n_args = 0;
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_alloc_fail = 0;
  reset_operation_test(&op);

  /* 14. Fallback 200 response realloc OOM (line 2608) */
  memset(&op, 0, sizeof(op));
  doc.returns = NULL;
  doc.n_returns = 0;
  sig.n_args = 0;
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_alloc_fail = 0;
  reset_operation_test(&op);

  /* 15. Doc response header with non-200 code (hits line 2565 branch) */
  {
    struct DocResponseHeader drh;
    memset(&drh, 0, sizeof(drh));
    drh.code = cdd_test_mut_str("400");
    drh.name = cdd_test_mut_str("X-Err");
    drh.type = cdd_test_mut_str("string");
    doc.response_headers = &drh;
    doc.n_response_headers = 1;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
    doc.response_headers = NULL;
    doc.n_response_headers = 0;
  }

  /* 16. Doc link with non-200 code (hits line 2594 branch) */
  {
    struct DocLink dlk;
    memset(&dlk, 0, sizeof(dlk));
    dlk.code = cdd_test_mut_str("400");
    dlk.name = cdd_test_mut_str("ErrLink");
    dlk.operation_id = cdd_test_mut_str("handleErr");
    doc.links = &dlk;
    doc.n_links = 1;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
    doc.links = NULL;
    doc.n_links = 0;
  }

  /* 17. any_from_json_value json_serialize failure (line 106) */
  val_obj = json_parse_string("{\"key\": \"value\"}");
  ASSERT(val_obj != NULL);
  g_cdd_fail_json_serialize = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, any_from_json_value(val_obj, &any));
  g_cdd_fail_json_serialize = 0;
  json_value_free(val_obj);

  /* 19. add_header_to_response existing header content_type OOM (line 667) &
   * loop exit (line 676) */
  memset(&resp, 0, sizeof(resp));
  dh.name = cdd_test_mut_str("X-Custom");
  dh.type = cdd_test_mut_str("string");
  dh.content_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp, &dh));
  /* Loop exit when header name differs (hits line 676) */
  dh.name = cdd_test_mut_str("X-Different");
  ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp, &dh));
  /* Matching header OOM on content_type (hits line 667) */
  dh.name = cdd_test_mut_str("X-Custom");
  dh.content_type = cdd_test_mut_str("application/json");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;
  free(resp.headers[0].name);
  free(resp.headers[0].type);
  free(resp.headers[1].name);
  free(resp.headers[1].type);
  free(resp.headers);

  /* 20. add_link_to_response duplicate link name (line 854) & loop exit (line
   * 863) */
  memset(&resp, 0, sizeof(resp));
  dl.server_url = NULL;
  dl.name = cdd_test_mut_str("LinkName");
  dl.operation_id = cdd_test_mut_str("opId");
  ASSERT_EQ(CDD_C_SUCCESS, add_link_to_response(&resp, &dl));
  /* Loop exit when link name differs (hits line 863) */
  dl.name = cdd_test_mut_str("AnotherLink");
  ASSERT_EQ(CDD_C_SUCCESS, add_link_to_response(&resp, &dl));
  /* Duplicate link name (hits line 854) */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));
  cleanup_link_fields(&resp.links[0]);
  cleanup_link_fields(&resp.links[1]);
  free(resp.links);
  resp.links = NULL;
  resp.n_links = 0;

  /* 21. Output arg with apply_format_to_schema_ref OOM */
  memset(&op, 0, sizeof(op));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_test_out_fmt_fail";
  sig.args = args;
  sig.n_args = 1;
  args[0].name = cdd_test_mut_str("out_val");
  args[0].type = cdd_test_mut_str("int **");
  g_cdd_strdup_fail = 6;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_strdup_fail = 0;
  reset_operation_test(&op);

  /* 22. Body arg with inline_type format failure */
  memset(&op, 0, sizeof(op));
  ctx.func_name = "api_post_user";
  args[0].name = cdd_test_mut_str("in_body");
  args[0].type = cdd_test_mut_str("int");
  {
    struct DocParam dp_body;
    memset(&dp_body, 0, sizeof(dp_body));
    dp_body.name = cdd_test_mut_str("in_body");
    dp_body.in_loc = cdd_test_mut_str("body");
    doc.params = &dp_body;
    doc.n_params = 1;
    g_cdd_strdup_fail = 5;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_strdup_fail = 0;
    doc.params = NULL;
    doc.n_params = 0;
    reset_operation_test(&op);
  }

  /* 23. Param description OOM (lines 2169-2171) */
  memset(&op, 0, sizeof(op));
  ctx.func_name = "api_user_get";
  args[0].name = cdd_test_mut_str("user_id");
  args[0].type = cdd_test_mut_str("int");
  {
    struct DocParam dp_desc;
    int k_step;
    int hit_oom = 0;
    memset(&dp_desc, 0, sizeof(dp_desc));
    dp_desc.name = cdd_test_mut_str("user_id");
    dp_desc.description = cdd_test_mut_str("User identifier");
    doc.params = &dp_desc;
    doc.n_params = 1;
    for (k_step = 1; k_step <= 6; ++k_step) {
      cdd_c_error_t rc_step;
      g_cdd_strdup_fail = k_step;
      rc_step = c2openapi_build_operation(&ctx, &op);
      if (rc_step != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_step);
        hit_oom = 1;
      }
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }
    ASSERT_EQ(1, hit_oom);
    doc.params = NULL;
    doc.n_params = 0;
  }

  /* 24. Param example error percolation (lines 2303-2305) */
  memset(&op, 0, sizeof(op));
  ctx.func_name = "api_user_get";
  args[0].name = cdd_test_mut_str("page");
  args[0].type = cdd_test_mut_str("int");
  {
    struct DocParam dp_ex;
    memset(&dp_ex, 0, sizeof(dp_ex));
    dp_ex.name = cdd_test_mut_str("page");
    dp_ex.example = cdd_test_mut_str("{invalid_json");
    doc.params = &dp_ex;
    doc.n_params = 1;
    /* In parse_example_any, invalid JSON falls back to OA_ANY_STRING unless
     * strdup fails */
    /* If 4th strdup fails (1: op_id, 2: name, 3: example in parse_example_any):
     */
    g_cdd_strdup_fail = 3;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_strdup_fail = 0;
    doc.params = NULL;
    doc.n_params = 0;
    reset_operation_test(&op);
  }

  /* 25. Existing response description update (lines 2445-2451) */
  memset(&op, 0, sizeof(op));
  ctx.func_name = "api_user_get";
  sig.n_args = 1;
  args[0].name = cdd_test_mut_str("out_user");
  args[0].type = cdd_test_mut_str("struct User **");
  {
    struct DocResponse ret_desc;
    memset(&ret_desc, 0, sizeof(ret_desc));
    ret_desc.code = cdd_test_mut_str("200");
    ret_desc.description = cdd_test_mut_str("Successful retrieval");
    doc.returns = &ret_desc;
    doc.n_returns = 1;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    ASSERT_EQ(1, op.n_responses);
    ASSERT_STR_EQ("Successful retrieval", op.responses[0].description);
    reset_operation_test(&op);

    /* OOM when duplicating description: 1st is op_id, 2nd is code 200, 3rd is
     * "Success", 4th is return description */
    g_cdd_strdup_fail = 4;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);

    doc.returns = NULL;
    doc.n_returns = 0;
  }

  /* 26. Request body description and content_type (lines 2417, 2431) */
  memset(&op, 0, sizeof(op));
  ctx.func_name = "api_post_user";
  sig.n_args = 0;
  {
    int k_step;
    int hit_oom = 0;
    doc.request_body_description = cdd_test_mut_str("Request payload");
    doc.request_body_content_type = cdd_test_mut_str("application/json");
    doc.n_request_bodies = 0;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);

    for (k_step = 1; k_step <= 4; ++k_step) {
      cdd_c_error_t rc_step;
      g_cdd_strdup_fail = k_step;
      rc_step = c2openapi_build_operation(&ctx, &op);
      if (rc_step != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_step);
        hit_oom = 1;
      }
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }
    ASSERT_EQ(1, hit_oom);

    doc.request_body_description = NULL;
    doc.request_body_content_type = NULL;
  }

  /* 27. Response header description OOM (line 2566) */
  {
    struct DocResponseHeader drh;
    int k_step;
    int hit_oom = 0;
    memset(&drh, 0, sizeof(drh));
    drh.code = cdd_test_mut_str("200");
    drh.name = cdd_test_mut_str("X-Res");
    drh.type = cdd_test_mut_str("string");
    doc.response_headers = &drh;
    doc.n_response_headers = 1;
    for (k_step = 1; k_step <= 5; ++k_step) {
      cdd_c_error_t rc_step;
      g_cdd_strdup_fail = k_step;
      rc_step = c2openapi_build_operation(&ctx, &op);
      if (rc_step != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_step);
        hit_oom = 1;
      }
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }
    ASSERT_EQ(1, hit_oom);
    doc.response_headers = NULL;
    doc.n_response_headers = 0;
  }

  /* 28. Link description OOM (line 2595) */
  {
    struct DocLink dlk;
    int k_step;
    int hit_oom = 0;
    memset(&dlk, 0, sizeof(dlk));
    dlk.code = cdd_test_mut_str("200");
    dlk.name = cdd_test_mut_str("MyLink");
    dlk.operation_id = cdd_test_mut_str("myOp");
    doc.links = &dlk;
    doc.n_links = 1;
    for (k_step = 1; k_step <= 5; ++k_step) {
      cdd_c_error_t rc_step;
      g_cdd_strdup_fail = k_step;
      rc_step = c2openapi_build_operation(&ctx, &op);
      if (rc_step != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_step);
        hit_oom = 1;
      }
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }
    ASSERT_EQ(1, hit_oom);
    doc.links = NULL;
    doc.n_links = 0;
  }

  /* 29. Fallback 200 description OOM (line 2623) */
  memset(&op, 0, sizeof(op));
  sig.n_args = 0;
  doc.returns = NULL;
  doc.n_returns = 0;
  /* 1st is op_id, 2nd is code "200", 3rd is description "Success" */
  g_cdd_strdup_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_strdup_fail = 0;
  reset_operation_test(&op);

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPERATION_COVERAGE_H */
