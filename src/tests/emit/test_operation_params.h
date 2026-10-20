/**
 * @file test_operation_params.h
 * @brief Parameter and media type tests for operation generator.
 */

#ifndef TEST_OPERATION_PARAMS_H
#define TEST_OPERATION_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_operation_common.h"
/* clang-format on */

/**
 * @brief Unit test test_operation_apply_format_to_schema_ref.
 * @return Greatest test result.
 */
TEST test_operation_apply_format_to_schema_ref(void) {
  struct OpenAPI_SchemaRef schema;
  struct OpenApiTypeMapping map;
  int applied = -1;
  memset(&schema, 0, sizeof(schema));
  memset(&map, 0, sizeof(map));

  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(NULL, &map, "fmt", &applied));
  ASSERT_EQ(0, applied);
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, NULL, "fmt", &applied));
  ASSERT_EQ(0, applied);

  map.oa_type = cdd_test_mut_str("integer");
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  ASSERT_EQ(0, applied);
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, "", &applied));
  ASSERT_EQ(0, applied);

  map.oa_type = cdd_test_mut_str("object");
  map.oa_format = cdd_test_mut_str("custom");
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  ASSERT_EQ(0, applied);

  map.kind = OA_TYPE_ARRAY;
  map.oa_type = cdd_test_mut_str("integer");
  map.oa_format = cdd_test_mut_str("int64");
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  ASSERT_EQ(1, applied);
  ASSERT_EQ(1, schema.is_array);
  ASSERT_STR_EQ("integer", schema.inline_type);
  ASSERT_STR_EQ("int64", schema.items_format);

  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, "int32", &applied));
  ASSERT_EQ(1, applied);
  ASSERT_STR_EQ("int32", schema.items_format);

  free(schema.inline_type);
  free(schema.items_format);
  schema.inline_type = NULL;
  schema.items_format = NULL;
  schema.is_array = 0;

  map.kind = OA_TYPE_PRIMITIVE;
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  ASSERT_EQ(1, applied);
  ASSERT_STR_EQ("integer", schema.inline_type);
  ASSERT_STR_EQ("int64", schema.format);

  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, "int32", &applied));
  ASSERT_EQ(1, applied);
  ASSERT_STR_EQ("int32", schema.format);

  free(schema.inline_type);
  free(schema.format);
  schema.inline_type = NULL;
  schema.format = NULL;

  map.kind = OA_TYPE_ARRAY;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  g_cdd_strdup_fail = 0;
  if (schema.inline_type) {
    free(schema.inline_type);
    schema.inline_type = NULL;
  }

  map.kind = OA_TYPE_PRIMITIVE;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  g_cdd_strdup_fail = 0;
  if (schema.inline_type) {
    free(schema.inline_type);
    schema.inline_type = NULL;
  }

  PASS();
}

/**
 * @brief Unit test test_operation_set_querystring_schema_from_type_map.
 * @return Greatest test result.
 */
TEST test_operation_set_querystring_schema_from_type_map(void) {
  struct OpenAPI_Parameter param;
  struct OpenApiTypeMapping tm;
  memset(&param, 0, sizeof(param));
  memset(&tm, 0, sizeof(tm));

  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(NULL, &tm));
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, NULL));

  tm.ref_name = cdd_test_mut_str("UserRef");
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, &tm));
  ASSERT_EQ(1, param.schema_set);
  ASSERT_EQ(0, param.schema.is_array);
  ASSERT_STR_EQ("UserRef", param.schema.ref_name);
  free(param.schema.ref_name);
  param.schema.ref_name = NULL;

  tm.kind = OA_TYPE_ARRAY;
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, &tm));
  ASSERT_EQ(1, param.schema.is_array);
  ASSERT_STR_EQ("UserRef", param.schema.ref_name);
  free(param.schema.ref_name);
  param.schema.ref_name = NULL;
  tm.ref_name = NULL;

  tm.oa_type = cdd_test_mut_str("integer");
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, &tm));
  ASSERT_STR_EQ("array", param.type);
  ASSERT_STR_EQ("integer", param.items_type);
  free(param.type);
  free(param.items_type);
  param.type = NULL;
  param.items_type = NULL;

  tm.oa_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, &tm));
  ASSERT_STR_EQ("array", param.type);
  ASSERT_EQ(NULL, param.items_type);
  free(param.type);
  param.type = NULL;

  tm.kind = OA_TYPE_PRIMITIVE;
  tm.oa_type = cdd_test_mut_str("boolean");
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, &tm));
  ASSERT_STR_EQ("boolean", param.type);
  free(param.type);
  param.type = NULL;

  tm.oa_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, &tm));
  ASSERT_STR_EQ("string", param.type);
  free(param.type);
  param.type = NULL;

  tm.ref_name = cdd_test_mut_str("Ref");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            set_querystring_schema_from_type_map(&param, &tm));
  g_cdd_strdup_fail = 0;
  tm.ref_name = NULL;

  tm.kind = OA_TYPE_ARRAY;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            set_querystring_schema_from_type_map(&param, &tm));
  g_cdd_strdup_fail = 0;

  tm.oa_type = cdd_test_mut_str("integer");
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            set_querystring_schema_from_type_map(&param, &tm));
  g_cdd_strdup_fail = 0;
  if (param.type) {
    free(param.type);
    param.type = NULL;
  }

  PASS();
}

/**
 * @brief Unit test test_operation_init_and_add_response_media_type.
 * @return Greatest test result.
 */
TEST test_operation_init_and_add_response_media_type(void) {
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Response resp;
  memset(&mt, 0, sizeof(mt));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_response(NULL, "app/json", &resp, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_response(&mt, NULL, &resp, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_response(&mt, "app/json", NULL, 0));

  ASSERT_EQ(CDD_C_SUCCESS,
            init_media_type_from_response(&mt, "app/json", &resp, 0));
  ASSERT_STR_EQ("app/json", mt.name);
  ASSERT_EQ(0, mt.schema_set);
  free(mt.name);

  resp.schema.ref_name = cdd_test_mut_str("User");
  ASSERT_EQ(CDD_C_SUCCESS,
            init_media_type_from_response(&mt, "app/json", &resp, 1));
  ASSERT_EQ(1, mt.item_schema_set);
  ASSERT_STR_EQ("User", mt.item_schema.ref_name);
  free(mt.name);
  free(mt.item_schema.ref_name);

  ASSERT_EQ(CDD_C_SUCCESS,
            init_media_type_from_response(&mt, "app/json", &resp, 0));
  ASSERT_EQ(1, mt.schema_set);
  ASSERT_STR_EQ("User", mt.schema.ref_name);
  free(mt.name);
  free(mt.schema.ref_name);

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            init_media_type_from_response(&mt, "app/json", &resp, 0));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            init_media_type_from_response(&mt, "app/json", &resp, 0));
  g_cdd_strdup_fail = 0;
  free(mt.name);
  mt.name = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, add_response_media_type(NULL, "app/json", 0));
  ASSERT_EQ(CDD_C_SUCCESS, add_response_media_type(&resp, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, add_response_media_type(&resp, "", 0));

  resp.content_type = cdd_test_mut_str("application/xml");
  ASSERT_EQ(CDD_C_SUCCESS,
            add_response_media_type(&resp, "application/json", 0));
  ASSERT_EQ(2, resp.n_content_media_types);

  ASSERT_EQ(CDD_C_SUCCESS,
            add_response_media_type(&resp, "application/json", 0));
  ASSERT_EQ(2, resp.n_content_media_types);

  ASSERT_EQ(CDD_C_SUCCESS, add_response_media_type(&resp, "text/plain", 0));
  ASSERT_EQ(3, resp.n_content_media_types);

  {
    size_t i;
    for (i = 0; i < resp.n_content_media_types; ++i) {
      if (resp.content_media_types[i].name)
        free(resp.content_media_types[i].name);
      if (resp.content_media_types[i].schema.ref_name)
        free(resp.content_media_types[i].schema.ref_name);
    }
    free(resp.content_media_types);
    resp.content_media_types = NULL;
    resp.n_content_media_types = 0;
    resp.content_type = NULL;
    resp.schema.ref_name = NULL;
  }

  ASSERT_EQ(CDD_C_SUCCESS,
            add_response_media_type(&resp, "application/json", 0));
  ASSERT_EQ(1, resp.n_content_media_types);
  free(resp.content_media_types[0].name);
  free(resp.content_media_types);
  resp.content_media_types = NULL;
  resp.n_content_media_types = 0;

  resp.content_type = cdd_test_mut_str("application/xml");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_response_media_type(&resp, "application/json", 0));
  g_cdd_strdup_fail = 0;
  if (resp.content_media_types) {
    free(resp.content_media_types);
    resp.content_media_types = NULL;
  }

  PASS();
}

/**
 * @brief Unit test test_operation_request_body_media_types.
 * @return Greatest test result.
 */
TEST test_operation_request_body_media_types(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_MediaType mt;
  int has = -1;

  memset(&op, 0, sizeof(op));
  memset(&mt, 0, sizeof(mt));

  ASSERT_EQ(CDD_C_SUCCESS, request_body_has_media_type(NULL, "app/json", &has));
  ASSERT_EQ(0, has);
  ASSERT_EQ(CDD_C_SUCCESS, request_body_has_media_type(&op, NULL, &has));
  ASSERT_EQ(0, has);
  ASSERT_EQ(CDD_C_SUCCESS, request_body_has_media_type(&op, "app/json", NULL));

  ASSERT_EQ(CDD_C_SUCCESS, request_body_has_media_type(&op, "app/json", &has));
  ASSERT_EQ(0, has);

  op.req_body.content_type = cdd_test_mut_str("application/json");
  ASSERT_EQ(CDD_C_SUCCESS,
            request_body_has_media_type(&op, "application/json", &has));
  ASSERT_EQ(1, has);

  op.req_body.content_type = NULL;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;
  mt.name = cdd_test_mut_str("application/xml");
  ASSERT_EQ(CDD_C_SUCCESS,
            request_body_has_media_type(&op, "application/xml", &has));
  ASSERT_EQ(1, has);
  ASSERT_EQ(CDD_C_SUCCESS,
            request_body_has_media_type(&op, "application/json", &has));
  ASSERT_EQ(0, has);
  op.req_body_media_types = NULL;
  op.n_req_body_media_types = 0;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_request_body(NULL, "app/json", &op, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_request_body(&mt, NULL, &op, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_request_body(&mt, "app/json", NULL, 0));

  ASSERT_EQ(CDD_C_SUCCESS,
            init_media_type_from_request_body(&mt, "app/json", &op, 0));
  free(mt.name);

  op.req_body.ref_name = cdd_test_mut_str("Payload");
  ASSERT_EQ(CDD_C_SUCCESS,
            init_media_type_from_request_body(&mt, "app/json", &op, 1));
  ASSERT_EQ(1, mt.item_schema_set);
  ASSERT_STR_EQ("Payload", mt.item_schema.ref_name);
  free(mt.name);
  free(mt.item_schema.ref_name);

  ASSERT_EQ(CDD_C_SUCCESS,
            init_media_type_from_request_body(&mt, "app/json", &op, 0));
  ASSERT_EQ(1, mt.schema_set);
  ASSERT_STR_EQ("Payload", mt.schema.ref_name);
  free(mt.name);
  free(mt.schema.ref_name);

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            init_media_type_from_request_body(&mt, "app/json", &op, 0));
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            init_media_type_from_request_body(&mt, "app/json", &op, 0));
  g_cdd_strdup_fail = 0;
  free(mt.name);
  mt.name = NULL;
  op.req_body.ref_name = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, add_request_body_media_type(NULL, "app/json", 0));
  ASSERT_EQ(CDD_C_SUCCESS, add_request_body_media_type(&op, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, add_request_body_media_type(&op, "", 0));

  op.req_body.content_type = cdd_test_mut_str("application/xml");
  ASSERT_EQ(CDD_C_SUCCESS,
            add_request_body_media_type(&op, "application/json", 0));
  ASSERT_EQ(2, op.n_req_body_media_types);

  ASSERT_EQ(CDD_C_SUCCESS,
            add_request_body_media_type(&op, "application/json", 0));
  ASSERT_EQ(2, op.n_req_body_media_types);

  ASSERT_EQ(CDD_C_SUCCESS, add_request_body_media_type(&op, "text/plain", 0));
  ASSERT_EQ(3, op.n_req_body_media_types);

  {
    size_t i;
    for (i = 0; i < op.n_req_body_media_types; ++i) {
      if (op.req_body_media_types[i].name)
        free(op.req_body_media_types[i].name);
    }
    free(op.req_body_media_types);
    op.req_body_media_types = NULL;
    op.n_req_body_media_types = 0;
    op.req_body.content_type = NULL;
  }

  ASSERT_EQ(CDD_C_SUCCESS,
            add_request_body_media_type(&op, "application/json", 0));
  ASSERT_EQ(1, op.n_req_body_media_types);
  free(op.req_body_media_types[0].name);
  free(op.req_body_media_types);
  op.req_body_media_types = NULL;
  op.n_req_body_media_types = 0;

  op.req_body.content_type = cdd_test_mut_str("application/xml");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_request_body_media_type(&op, "application/json", 0));
  g_cdd_strdup_fail = 0;
  if (op.req_body_media_types) {
    free(op.req_body_media_types);
    op.req_body_media_types = NULL;
  }

  PASS();
}

/**
 * @brief Unit test test_operation_media_types_item_schema_oom.
 * @return Greatest test result.
 */
TEST test_operation_media_types_item_schema_oom(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Operation op;
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&op, 0, sizeof(op));

  resp.schema.ref_name = cdd_test_mut_str("SchemaRef");
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            init_media_type_from_response(&mt, "app/json", &resp, 1));
  g_cdd_strdup_fail = 0;
  free(mt.name);
  mt.name = NULL;

  op.req_body.ref_name = cdd_test_mut_str("BodyRef");
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            init_media_type_from_request_body(&mt, "app/json", &op, 1));
  g_cdd_strdup_fail = 0;
  free(mt.name);
  mt.name = NULL;

  resp.schema.ref_name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, add_response_media_type(&resp, "app/json", 0));
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_response_media_type(&resp, "app/xml", 0));
  g_cdd_strdup_fail = 0;
  if (resp.content_media_types) {
    if (resp.content_media_types[0].name)
      free(resp.content_media_types[0].name);
    free(resp.content_media_types);
    resp.content_media_types = NULL;
    resp.n_content_media_types = 0;
  }

  op.req_body.ref_name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, add_request_body_media_type(&op, "app/json", 0));
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_request_body_media_type(&op, "app/xml", 0));
  g_cdd_strdup_fail = 0;
  if (op.req_body_media_types) {
    if (op.req_body_media_types[0].name)
      free(op.req_body_media_types[0].name);
    free(op.req_body_media_types);
    op.req_body_media_types = NULL;
    op.n_req_body_media_types = 0;
  }

  PASS();
}

/**
 * @brief Unit test test_operation_c2openapi_build_operation_nulls.
 * @return Greatest test result.
 */
TEST test_operation_c2openapi_build_operation_nulls(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct OpenAPI_Operation op;
  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(&op, 0, sizeof(op));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2openapi_build_operation(NULL, &op));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c2openapi_build_operation(&ctx, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2openapi_build_operation(&ctx, &op));

  PASS();
}

/**
 * @brief Unit test test_operation_c2openapi_build_operation_verbs.
 * @return Greatest test result.
 */
TEST test_operation_c2openapi_build_operation_verbs(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct OpenAPI_Operation op;

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(&doc, 0, sizeof(doc));
  memset(&op, 0, sizeof(op));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "test_func";

  doc.verb = cdd_test_mut_str("GET");
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_GET, op.verb);
  reset_operation_test(&op);

  doc.verb = cdd_test_mut_str("POST");
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_POST, op.verb);
  reset_operation_test(&op);

  doc.verb = cdd_test_mut_str("PUT");
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_PUT, op.verb);
  reset_operation_test(&op);

  doc.verb = cdd_test_mut_str("DELETE");
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_DELETE, op.verb);
  reset_operation_test(&op);

  doc.verb = cdd_test_mut_str("PATCH");
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_PATCH, op.verb);
  reset_operation_test(&op);

  doc.verb = cdd_test_mut_str("HEAD");
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_HEAD, op.verb);
  reset_operation_test(&op);

  doc.verb = cdd_test_mut_str("OPTIONS");
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_OPTIONS, op.verb);
  reset_operation_test(&op);

  doc.verb = cdd_test_mut_str("TRACE");
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_TRACE, op.verb);
  reset_operation_test(&op);

  doc.verb = cdd_test_mut_str("QUERY");
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_QUERY, op.verb);
  reset_operation_test(&op);

  doc.verb = cdd_test_mut_str("CUSTOM");
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_UNKNOWN, op.verb);
  ASSERT_EQ(1, op.is_additional);
  ASSERT_STR_EQ("CUSTOM", op.method);
  reset_operation_test(&op);

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_strdup_fail = 0;
  reset_operation_test(&op);

  doc.verb = NULL;
  ctx.func_name = "api_post_item";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_POST, op.verb);
  reset_operation_test(&op);

  ctx.func_name = "item_create";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_POST, op.verb);
  reset_operation_test(&op);

  ctx.func_name = "api_put_item";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_PUT, op.verb);
  reset_operation_test(&op);

  ctx.func_name = "item_update";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_PUT, op.verb);
  reset_operation_test(&op);

  ctx.func_name = "api_delete_item";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_DELETE, op.verb);
  reset_operation_test(&op);

  ctx.func_name = "item_delete";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_DELETE, op.verb);
  reset_operation_test(&op);

  ctx.func_name = "api_other_op";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_GET, op.verb);
  reset_operation_test(&op);

  PASS();
}

/**
 * @brief Unit test test_operation_c2openapi_build_operation_metadata.
 * @return Greatest test result.
 */
TEST test_operation_c2openapi_build_operation_metadata(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocSecurityRequirement sec;
  struct DocServer srv;
  struct OpenAPI_Operation op;
  char *tags[2];
  char *scopes[2];

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(&doc, 0, sizeof(doc));
  memset(&sec, 0, sizeof(sec));
  memset(&srv, 0, sizeof(srv));
  memset(&op, 0, sizeof(op));

  tags[0] = cdd_test_mut_str("tagA");
  tags[1] = cdd_test_mut_str("tagB");
  scopes[0] = cdd_test_mut_str("read");
  scopes[1] = cdd_test_mut_str("write");

  sec.scheme = cdd_test_mut_str("OAuth2");
  sec.scopes = scopes;
  sec.n_scopes = 2;

  srv.url = cdd_test_mut_str("https://example.com/v1");
  srv.name = cdd_test_mut_str("Primary");
  srv.description = cdd_test_mut_str("Primary server");

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_widget_get";

  doc.operation_id = cdd_test_mut_str("getWidget");
  doc.summary = cdd_test_mut_str("Widget Summary");
  doc.description = cdd_test_mut_str("Widget Description");
  doc.deprecated_set = 1;
  doc.deprecated = 1;
  doc.external_docs_url = cdd_test_mut_str("https://docs.example.com");
  doc.external_docs_description = cdd_test_mut_str("Full docs");
  doc.tags = tags;
  doc.n_tags = 2;
  doc.security = &sec;
  doc.n_security = 1;
  doc.servers = &srv;
  doc.n_servers = 1;

  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_STR_EQ("getWidget", op.operation_id);
  ASSERT_STR_EQ("Widget Summary", op.summary);
  ASSERT_STR_EQ("Widget Description", op.description);
  ASSERT_EQ(1, op.deprecated);
  ASSERT_STR_EQ("https://docs.example.com", op.external_docs.url);
  ASSERT_STR_EQ("Full docs", op.external_docs.description);
  ASSERT_EQ(2, op.n_tags);
  ASSERT_EQ(1, op.n_security);
  ASSERT_EQ(1, op.n_servers);
  reset_operation_test(&op);

  doc.n_tags = 0;
  doc.tags = NULL;
  ctx.func_name = "api_pet_get";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(1, op.n_tags);
  ASSERT_STR_EQ("Pet", op.tags[0]);
  reset_operation_test(&op);

  PASS();
}

/**
 * @brief Unit test test_operation_c2openapi_build_operation_arguments.
 * @return Greatest test result.
 */
TEST test_operation_c2openapi_build_operation_arguments(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[8];
  struct DocMetadata doc;
  struct DocParam params[8];
  struct OpenAPI_Operation op;

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(args, 0, sizeof(args));
  memset(&doc, 0, sizeof(doc));
  memset(params, 0, sizeof(params));
  memset(&op, 0, sizeof(op));

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_user_update";
  doc.verb = cdd_test_mut_str("PUT");
  doc.route = cdd_test_mut_str("/user/{uid}");

  sig.args = args;
  sig.n_args = 8;
  doc.params = params;
  doc.n_params = 8;

  args[0].name = cdd_test_mut_str("uid");
  args[0].type = cdd_test_mut_str("int");

  args[1].name = cdd_test_mut_str("session_id");
  args[1].type = cdd_test_mut_str("char *");
  params[1].name = cdd_test_mut_str("session_id");
  params[1].in_loc = cdd_test_mut_str("cookie");
  params[1].description = cdd_test_mut_str("Session cookie");
  params[1].required = 1;
  params[1].style_set = 1;
  params[1].style = DOC_PARAM_STYLE_FORM;
  params[1].explode_set = 1;
  params[1].explode = 1;
  params[1].allow_reserved_set = 1;
  params[1].allow_reserved = 1;
  params[1].allow_empty_value_set = 1;
  params[1].allow_empty_value = 1;
  params[1].deprecated_set = 1;
  params[1].deprecated = 1;
  params[1].example = cdd_test_mut_str("abc123xyz");

  args[2].name = cdd_test_mut_str("Accept");
  args[2].type = cdd_test_mut_str("char *");
  params[2].name = cdd_test_mut_str("Accept");
  params[2].in_loc = cdd_test_mut_str("header");
  params[2].description = cdd_test_mut_str("Reserved accept header");

  args[3].name = cdd_test_mut_str("X-Trace-Id");
  args[3].type = cdd_test_mut_str("char *");
  params[3].name = cdd_test_mut_str("X-Trace-Id");
  params[3].in_loc = cdd_test_mut_str("header");
  params[3].content_type = cdd_test_mut_str("text/plain");
  params[3].example = cdd_test_mut_str("tr-999");

  args[4].name = cdd_test_mut_str("filter");
  args[4].type = cdd_test_mut_str("char *");
  params[4].name = cdd_test_mut_str("filter");
  params[4].in_loc = cdd_test_mut_str("querystring");

  args[5].name = cdd_test_mut_str("tags");
  args[5].type = cdd_test_mut_str("int[]");
  params[5].name = cdd_test_mut_str("tags");
  params[5].format = cdd_test_mut_str("int32");
  params[5].item_schema = 1;

  args[6].name = cdd_test_mut_str("body");
  args[6].type = cdd_test_mut_str("struct User *");

  args[7].name = cdd_test_mut_str("out_result");
  args[7].type = cdd_test_mut_str("struct Result **");

  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(1, op.req_body_required);
  ASSERT_STR_EQ("application/json", op.req_body.content_type);
  ASSERT(op.n_responses >= 1);
  reset_operation_test(&op);

  args[6].type = cdd_test_mut_str("const struct User *");
  sig.n_args = 7;
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(1, op.req_body_required);
  reset_operation_test(&op);

  params[0].name = cdd_test_mut_str("uid");
  params[0].in_loc = cdd_test_mut_str("body");
  sig.n_args = 1;
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  reset_operation_test(&op);

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPERATION_PARAMS_H */
