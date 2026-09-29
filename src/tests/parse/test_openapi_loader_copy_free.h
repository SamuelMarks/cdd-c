/**
 * @file test_openapi_loader_copy_free.h
 * @brief Copy, free, and OOM branch tests for OpenAPI loader.
 */

#ifndef TEST_OPENAPI_LOADER_COPY_FREE_H
#define TEST_OPENAPI_LOADER_COPY_FREE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

TEST test_openapi_loader_copy_and_free_helpers(void) {
  struct OpenAPI_Parameter p_src, p_dst;
  struct OpenAPI_Header h_src, h_dst;
  struct OpenAPI_Encoding enc_src, enc_dst;
  struct OpenAPI_MediaType mt_src, mt_dst;
  struct OpenAPI_Response r_src, r_dst;
  struct OpenAPI_Operation op_src, op_dst;
  struct OpenAPI_SchemaRef sr_src, sr_dst;
  struct OpenAPI_Example ex_src, ex_dst;
  struct OpenAPI_RequestBody rb;
  struct OpenAPI_Any any_val;
  struct OpenAPI_Link lnk;
  struct OpenAPI_SecurityRequirement sec_req;
  struct OpenAPI_Path path_item;
  struct OpenAPI_Callback cb;
  char **str_arr;
  cdd_c_error_t rc;

  /* 1. Parameter copy & free */
  memset(&p_src, 0, sizeof(p_src));
  memset(&p_dst, 0, sizeof(p_dst));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_parameter_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_parameter_fields(&p_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_parameter_fields(NULL, &p_src));

  p_src.in = OA_PARAM_IN_QUERY;
  p_src.required = 1;
  p_src.deprecated = 1;
  p_src.deprecated_set = 1;
  p_src.is_array = 1;
  p_src.style = OA_STYLE_FORM;
  p_src.explode = 1;
  p_src.explode_set = 1;
  p_src.allow_reserved = 1;
  p_src.allow_reserved_set = 1;
  p_src.allow_empty_value = 1;
  p_src.allow_empty_value_set = 1;
  p_src.name = strdup("param1");
  p_src.description = strdup("param desc");
  p_src.content_type = strdup("application/json");
  p_src.content_ref = strdup("#/components/mediaTypes/Json");
  p_src.items_type = strdup("string");
  p_src.example_set = 1;
  p_src.example.type = OA_ANY_STRING;
  p_src.example.string = strdup("ex_val");
  p_src.extensions_json = strdup("{\"x-p\":1}");
  p_src.n_examples = 1;
  p_src.examples =
      (struct OpenAPI_Example *)calloc(1, sizeof(struct OpenAPI_Example));
  if (p_src.examples) {
    p_src.examples[0].name = strdup("ex1");
  }
  p_src.n_content_media_types = 1;
  p_src.content_media_types =
      (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
  if (p_src.content_media_types) {
    p_src.content_media_types[0].name = strdup("app/json");
  }

  rc = cdd_test_copy_parameter_fields(&p_dst, &p_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("param1", p_dst.name);
  ASSERT_STR_EQ("param desc", p_dst.description);
  ASSERT_STR_EQ("application/json", p_dst.content_type);
  ASSERT_STR_EQ("#/components/mediaTypes/Json", p_dst.content_ref);
  ASSERT_STR_EQ("string", p_dst.items_type);
  ASSERT_EQ(1, p_dst.example_set);
  ASSERT_STR_EQ("ex_val", p_dst.example.string);

  cdd_test_free_parameter(&p_dst);

  {
    int k;
    for (k = 1; k <= 15; ++k) {
      memset(&p_dst, 0, sizeof(p_dst));
      g_cdd_strdup_fail = k;
      cdd_test_copy_parameter_fields(&p_dst, &p_src);
      cdd_test_free_parameter(&p_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  cdd_test_free_parameter(&p_src);
  cdd_test_free_parameter(NULL);

  /* 2. Header copy & free */
  memset(&h_src, 0, sizeof(h_src));
  memset(&h_dst, 0, sizeof(h_dst));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_header_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_header_fields(&h_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_header_fields(NULL, &h_src));

  h_src.required = 1;
  h_src.deprecated = 1;
  h_src.deprecated_set = 1;
  h_src.style = OA_STYLE_SIMPLE;
  h_src.style_set = 1;
  h_src.explode = 1;
  h_src.explode_set = 1;
  h_src.is_array = 1;
  h_src.description = strdup("hdr desc");
  h_src.content_type = strdup("text/plain");
  h_src.content_ref = strdup("#/components/mediaTypes/Text");
  h_src.items_type = strdup("integer");
  h_src.example_set = 1;
  h_src.example.type = OA_ANY_STRING;
  h_src.example.string = strdup("hdr_val");
  h_src.extensions_json = strdup("{\"x-h\":1}");
  h_src.n_examples = 1;
  h_src.examples =
      (struct OpenAPI_Example *)calloc(1, sizeof(struct OpenAPI_Example));
  if (h_src.examples) {
    h_src.examples[0].name = strdup("ex1");
  }
  h_src.n_content_media_types = 1;
  h_src.content_media_types =
      (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
  if (h_src.content_media_types) {
    h_src.content_media_types[0].name = strdup("text/plain");
  }

  rc = cdd_test_copy_header_fields(&h_dst, &h_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("hdr desc", h_dst.description);
  ASSERT_STR_EQ("text/plain", h_dst.content_type);
  ASSERT_STR_EQ("#/components/mediaTypes/Text", h_dst.content_ref);
  ASSERT_STR_EQ("integer", h_dst.items_type);
  ASSERT_EQ(1, h_dst.example_set);

  cdd_test_free_header(&h_dst);

  {
    int k;
    for (k = 1; k <= 15; ++k) {
      memset(&h_dst, 0, sizeof(h_dst));
      g_cdd_strdup_fail = k;
      cdd_test_copy_header_fields(&h_dst, &h_src);
      cdd_test_free_header(&h_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  cdd_test_free_header(&h_src);
  cdd_test_free_header(NULL);

  /* 3. Encoding copy & free */
  memset(&enc_src, 0, sizeof(enc_src));
  memset(&enc_dst, 0, sizeof(enc_dst));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_encoding_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_encoding_fields(&enc_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_encoding_fields(NULL, &enc_src));

  enc_src.name = strdup("enc1");
  enc_src.content_type = strdup("image/png");
  enc_src.style = OA_STYLE_FORM;
  enc_src.explode = 1;
  enc_src.allow_reserved = 1;
  enc_src.n_headers = 1;
  enc_src.headers =
      (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
  if (enc_src.headers) {
    enc_src.headers[0].description = strdup("h_desc");
  }
  enc_src.item_encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  if (enc_src.item_encoding) {
    enc_src.item_encoding->name = strdup("item_enc");
  }

  rc = cdd_test_copy_encoding_fields(&enc_dst, &enc_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("enc1", enc_dst.name);
  ASSERT_STR_EQ("image/png", enc_dst.content_type);

  cdd_test_free_encoding(&enc_dst);

  {
    int k;
    for (k = 1; k <= 20; ++k) {
      memset(&enc_dst, 0, sizeof(enc_dst));
      g_cdd_strdup_fail = k;
      cdd_test_copy_encoding_fields(&enc_dst, &enc_src);
      cdd_test_free_encoding(&enc_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  cdd_test_free_encoding(&enc_src);
  cdd_test_free_encoding(NULL);

  /* 4. Media type copy & free */
  memset(&mt_src, 0, sizeof(mt_src));
  memset(&mt_dst, 0, sizeof(mt_dst));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_media_type_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_media_type_fields(&mt_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_media_type_fields(NULL, &mt_src));

  mt_src.name = strdup("application/json");
  mt_src.ref = strdup("#/components/mediaTypes/Json");
  mt_src.example_set = 1;
  mt_src.example.type = OA_ANY_STRING;
  mt_src.example.string = strdup("mt_ex");
  mt_src.item_encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  if (mt_src.item_encoding) {
    mt_src.item_encoding->name = strdup("mt_item_enc");
  }

  rc = cdd_test_copy_media_type_fields(&mt_dst, &mt_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("application/json", mt_dst.name);
  ASSERT_STR_EQ("#/components/mediaTypes/Json", mt_dst.ref);
  ASSERT_EQ(1, mt_dst.example_set);

  cdd_test_free_media_type(&mt_dst);

  {
    int k;
    for (k = 1; k <= 20; ++k) {
      memset(&mt_dst, 0, sizeof(mt_dst));
      g_cdd_strdup_fail = k;
      cdd_test_copy_media_type_fields(&mt_dst, &mt_src);
      cdd_test_free_media_type(&mt_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  cdd_test_free_media_type(&mt_src);
  cdd_test_free_media_type(NULL);

  /* Copy media type array */
  {
    struct OpenAPI_MediaType *mt_arr_dst = NULL;
    size_t mt_count = 0;
    struct OpenAPI_MediaType mt_arr_src[1];
    memset(mt_arr_src, 0, sizeof(mt_arr_src));
    mt_arr_src[0].name = strdup("app/json");

    cdd_test_copy_media_type_array(NULL, NULL, NULL, 0);
    cdd_test_copy_media_type_array(&mt_arr_dst, &mt_count, NULL, 0);

    cdd_test_copy_media_type_array(&mt_arr_dst, &mt_count, mt_arr_src, 1);
    ASSERT_EQ(1, mt_count);
    ASSERT(mt_arr_dst != NULL);

    {
      size_t m;
      for (m = 0; m < mt_count; ++m) {
        cdd_test_free_media_type(&mt_arr_dst[m]);
      }
      free(mt_arr_dst);
    }
    cdd_test_free_media_type(&mt_arr_src[0]);
  }

  /* 5. Response copy & free */
  memset(&r_src, 0, sizeof(r_src));
  memset(&r_dst, 0, sizeof(r_dst));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_response_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_response_fields(&r_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_response_fields(NULL, &r_src));

  r_src.summary = strdup("OK summary");
  r_src.description = strdup("OK response");
  r_src.example_set = 1;
  r_src.example.type = OA_ANY_STRING;
  r_src.example.string = strdup("resp_ex");
  r_src.n_headers = 1;
  r_src.headers =
      (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
  if (r_src.headers) {
    r_src.headers[0].description = strdup("rh_desc");
  }
  r_src.n_links = 1;
  r_src.links = (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
  if (r_src.links) {
    r_src.links[0].operation_id = strdup("op1");
  }

  rc = cdd_test_copy_response_fields(&r_dst, &r_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("OK summary", r_dst.summary);
  ASSERT_STR_EQ("OK response", r_dst.description);
  ASSERT_EQ(1, r_dst.example_set);

  cdd_test_free_response(&r_dst);

  {
    int k;
    for (k = 1; k <= 25; ++k) {
      memset(&r_dst, 0, sizeof(r_dst));
      g_cdd_strdup_fail = k;
      cdd_test_copy_response_fields(&r_dst, &r_src);
      cdd_test_free_response(&r_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  cdd_test_free_response(&r_src);
  cdd_test_free_response(NULL);

  /* 6. Operation copy & free */
  memset(&op_src, 0, sizeof(op_src));
  memset(&op_dst, 0, sizeof(op_dst));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_operation_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_operation_fields(&op_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_operation_fields(NULL, &op_src));

  op_src.operation_id = strdup("op1");
  op_src.summary = strdup("op summary");
  op_src.description = strdup("op description");
  op_src.req_body_ref = strdup("#/components/requestBodies/Body1");
  op_src.req_body_extensions_json = strdup("{\"x-rb\":1}");
  op_src.external_docs.description = strdup("docs desc");
  op_src.external_docs.url = strdup("http://docs");
  op_src.external_docs.extensions_json = strdup("{\"x-doc\":1}");
  op_src.req_body.inline_type = strdup("string");
  op_src.req_body.content_type = strdup("application/json");
  op_src.req_body.ref_name = strdup("MyRef");
  op_src.n_security = 1;
  op_src.security = (struct OpenAPI_SecurityRequirementSet *)calloc(
      1, sizeof(struct OpenAPI_SecurityRequirementSet));
  if (op_src.security) {
    op_src.security[0].n_requirements = 1;
    op_src.security[0].requirements =
        (struct OpenAPI_SecurityRequirement *)calloc(
            1, sizeof(struct OpenAPI_SecurityRequirement));
    if (op_src.security[0].requirements) {
      op_src.security[0].requirements[0].scheme = strdup("Basic");
      op_src.security[0].requirements[0].n_scopes = 1;
      op_src.security[0].requirements[0].scopes =
          (char **)calloc(1, sizeof(char *));
      if (op_src.security[0].requirements[0].scopes) {
        op_src.security[0].requirements[0].scopes[0] = strdup("read");
      }
    }
    op_src.security[0].extensions_json = strdup("{\"x-s\":1}");
  }

  rc = cdd_test_copy_operation_fields(&op_dst, &op_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("op1", op_dst.operation_id);
  ASSERT_STR_EQ("op summary", op_dst.summary);
  ASSERT_STR_EQ("op description", op_dst.description);
  ASSERT_STR_EQ("#/components/requestBodies/Body1", op_dst.req_body_ref);
  ASSERT_STR_EQ("{\"x-rb\":1}", op_dst.req_body_extensions_json);

  cdd_test_free_operation(&op_dst);

  {
    int k;
    for (k = 1; k <= 20; ++k) {
      memset(&op_dst, 0, sizeof(op_dst));
      g_cdd_strdup_fail = k;
      cdd_test_copy_operation_fields(&op_dst, &op_src);
      cdd_test_free_operation(&op_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  cdd_test_free_operation(&op_src);
  cdd_test_free_operation(NULL);

  /* 7. SchemaRef copy & free */
  memset(&sr_src, 0, sizeof(sr_src));
  memset(&sr_dst, 0, sizeof(sr_dst));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_schema_ref(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_schema_ref(&sr_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_schema_ref(NULL, &sr_src));

  sr_src.inline_type = strdup("string");
  sr_src.format = strdup("date-time");
  sr_src.description = strdup("sr desc");
  sr_src.ref = strdup("#/components/schemas/MyString");

  rc = cdd_test_copy_schema_ref(&sr_dst, &sr_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("string", sr_dst.inline_type);
  ASSERT_STR_EQ("date-time", sr_dst.format);
  ASSERT_STR_EQ("sr desc", sr_dst.description);
  ASSERT_STR_EQ("#/components/schemas/MyString", sr_dst.ref);

  cdd_test_free_schema_ref_content(&sr_src);
  cdd_test_free_schema_ref_content(&sr_dst);
  cdd_test_free_schema_ref_content(NULL);

  /* 8. Example copy & free */
  memset(&ex_src, 0, sizeof(ex_src));
  memset(&ex_dst, 0, sizeof(ex_dst));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_example_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_example_fields(&ex_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_example_fields(NULL, &ex_src));

  ex_src.name = strdup("ex_name");
  ex_src.summary = strdup("ex summary");
  ex_src.description = strdup("ex description");
  ex_src.external_value = strdup("http://ex.json");
  ex_src.serialized_value = strdup("raw_ex");
  ex_src.extensions_json = strdup("{\"x-e\":1}");

  rc = cdd_test_copy_example_fields(&ex_dst, &ex_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("ex_name", ex_dst.name);
  ASSERT_STR_EQ("ex summary", ex_dst.summary);
  ASSERT_STR_EQ("ex description", ex_dst.description);
  ASSERT_STR_EQ("http://ex.json", ex_dst.external_value);

  cdd_test_free_example(&ex_src);
  cdd_test_free_example(&ex_dst);
  cdd_test_free_example(NULL);

  /* Copy item schema as array */
  {
    struct OpenAPI_SchemaRef arr_dst, arr_src;
    memset(&arr_dst, 0, sizeof(arr_dst));
    memset(&arr_src, 0, sizeof(arr_src));
    arr_src.schema_is_boolean = 1;
    arr_src.schema_boolean_value = 1;
    arr_src.inline_type = strdup("string");
    cdd_test_copy_item_schema_as_array(&arr_dst, &arr_src);
    cdd_test_free_schema_ref_content(&arr_dst);

    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_item_schema_as_array(&arr_dst, &arr_src));
    g_cdd_strdup_fail = 0;

    cdd_test_free_schema_ref_content(&arr_src);
    cdd_test_copy_item_schema_as_array(NULL, NULL);
  }

  /* Copy any value JSON */
  {
    struct OpenAPI_Any any_src_json, any_dst_json;
    memset(&any_src_json, 0, sizeof(any_src_json));
    memset(&any_dst_json, 0, sizeof(any_dst_json));
    any_src_json.type = OA_ANY_JSON;
    any_src_json.json = strdup("{\"key\":\"value\"}");
    cdd_test_copy_any_value(&any_dst_json, &any_src_json);
    cdd_test_free_any_value(&any_src_json);
    cdd_test_free_any_value(&any_dst_json);
    cdd_test_copy_any_value(NULL, NULL);
  }

  /* 9. RequestBody free */
  memset(&rb, 0, sizeof(rb));
  rb.description = strdup("rb desc");
  rb.ref = strdup("#/components/requestBodies/B");
  rb.extensions_json = strdup("{\"x-r\":1}");
  cdd_test_free_request_body(&rb);
  cdd_test_free_request_body(NULL);

  /* 10. Any free */
  memset(&any_val, 0, sizeof(any_val));
  any_val.type = OA_ANY_STRING;
  any_val.string = strdup("str");
  cdd_test_free_any_value(&any_val);

  memset(&any_val, 0, sizeof(any_val));
  any_val.type = OA_ANY_JSON;
  any_val.json = strdup("{}");
  cdd_test_free_any_value(&any_val);

  memset(&any_val, 0, sizeof(any_val));
  any_val.type = OA_ANY_NUMBER;
  any_val.number = 42.0;
  cdd_test_free_any_value(&any_val);
  cdd_test_free_any_value(NULL);

  /* 11. Link free */
  memset(&lnk, 0, sizeof(lnk));
  lnk.name = strdup("lnk1");
  lnk.ref = strdup("#/components/links/L");
  lnk.operation_id = strdup("op1");
  lnk.operation_ref = strdup("#/paths/~1foo/get");
  lnk.description = strdup("lnk desc");
  lnk.summary = strdup("lnk sum");
  lnk.extensions_json = strdup("{\"x-lnk\":1}");
  cdd_test_free_link(&lnk);
  cdd_test_free_link(NULL);

  /* 12. Security Requirement free */
  memset(&sec_req, 0, sizeof(sec_req));
  sec_req.scheme = strdup("sec1");
  sec_req.scopes = (char **)malloc(sizeof(char *));
  if (sec_req.scopes) {
    sec_req.scopes[0] = strdup("read:all");
    sec_req.n_scopes = 1;
  }
  cdd_test_free_security_requirement(&sec_req);
  cdd_test_free_security_requirement(NULL);

  /* 13. PathItem free */
  memset(&path_item, 0, sizeof(path_item));
  path_item.route = strdup("/test");
  path_item.ref = strdup("#/components/pathItems/Item");
  path_item.summary = strdup("pi summary");
  path_item.description = strdup("pi description");
  path_item.extensions_json = strdup("{\"x-pi\":1}");
  cdd_test_free_path_item(&path_item);
  cdd_test_free_path_item(NULL);

  /* 14. Callback free */
  memset(&cb, 0, sizeof(cb));
  cb.name = strdup("cb1");
  cb.ref = strdup("#/components/callbacks/Cb");
  cb.extensions_json = strdup("{\"x-cb\":1}");
  cdd_test_free_callback(&cb);
  cdd_test_free_callback(NULL);

  /* 15. String array free */
  str_arr = (char **)malloc(2 * sizeof(char *));
  if (str_arr) {
    str_arr[0] = strdup("s1");
    str_arr[1] = strdup("s2");
    cdd_test_free_string_array(str_arr, 2);
  }
  cdd_test_free_string_array(NULL, 0);

  PASS();
}

TEST test_openapi_loader_copy_oom_branches(void) {
  int k;
  struct OpenAPI_Parameter p_src, p_dst;
  struct OpenAPI_Header h_src, h_dst;
  struct OpenAPI_Encoding enc_src, enc_dst;
  struct OpenAPI_Response r_src, r_dst;
  struct OpenAPI_Operation op_src, op_dst;
  struct OpenAPI_SchemaRef sr_src, sr_dst;
  struct OpenAPI_Example ex_src, ex_dst;

  for (k = 1; k <= 8; ++k) {
    memset(&p_src, 0, sizeof(p_src));
    memset(&p_dst, 0, sizeof(p_dst));
    p_src.name = (char *)(size_t) "p";
    p_src.description = (char *)(size_t) "d";
    p_src.content_type = (char *)(size_t) "t";
    p_src.content_ref = (char *)(size_t) "r";
    p_src.items_type = (char *)(size_t) "i";
    g_cdd_alloc_fail = k;
    cdd_test_copy_parameter_fields(&p_dst, &p_src);
    g_cdd_alloc_fail = 0;
    cdd_test_free_parameter(&p_dst);
  }

  for (k = 1; k <= 8; ++k) {
    memset(&h_src, 0, sizeof(h_src));
    memset(&h_dst, 0, sizeof(h_dst));
    h_src.description = (char *)(size_t) "d";
    h_src.content_type = (char *)(size_t) "t";
    h_src.content_ref = (char *)(size_t) "r";
    h_src.items_type = (char *)(size_t) "i";
    g_cdd_alloc_fail = k;
    cdd_test_copy_header_fields(&h_dst, &h_src);
    g_cdd_alloc_fail = 0;
    cdd_test_free_header(&h_dst);
  }

  for (k = 1; k <= 6; ++k) {
    memset(&enc_src, 0, sizeof(enc_src));
    memset(&enc_dst, 0, sizeof(enc_dst));
    enc_src.name = (char *)(size_t) "n";
    enc_src.content_type = (char *)(size_t) "t";
    g_cdd_alloc_fail = k;
    cdd_test_copy_encoding_fields(&enc_dst, &enc_src);
    g_cdd_alloc_fail = 0;
    cdd_test_free_encoding(&enc_dst);
  }

  for (k = 1; k <= 6; ++k) {
    memset(&r_src, 0, sizeof(r_src));
    memset(&r_dst, 0, sizeof(r_dst));
    r_src.code = (char *)(size_t) "200";
    r_src.description = (char *)(size_t) "d";
    g_cdd_alloc_fail = k;
    cdd_test_copy_response_fields(&r_dst, &r_src);
    g_cdd_alloc_fail = 0;
    cdd_test_free_response(&r_dst);
  }

  for (k = 1; k <= 8; ++k) {
    memset(&op_src, 0, sizeof(op_src));
    memset(&op_dst, 0, sizeof(op_dst));
    op_src.operation_id = (char *)(size_t) "o";
    op_src.summary = (char *)(size_t) "s";
    op_src.description = (char *)(size_t) "d";
    op_src.req_body_ref = (char *)(size_t) "r";
    op_src.req_body_extensions_json = (char *)(size_t) "e";
    op_src.external_docs.extensions_json = (char *)(size_t) "x";
    g_cdd_alloc_fail = k;
    cdd_test_copy_operation_fields(&op_dst, &op_src);
    g_cdd_alloc_fail = 0;
    cdd_test_free_operation(&op_dst);
  }

  for (k = 1; k <= 6; ++k) {
    memset(&sr_src, 0, sizeof(sr_src));
    memset(&sr_dst, 0, sizeof(sr_dst));
    sr_src.inline_type = (char *)(size_t) "string";
    sr_src.format = (char *)(size_t) "date";
    sr_src.description = (char *)(size_t) "desc";
    sr_src.ref = (char *)(size_t) "ref";
    g_cdd_alloc_fail = k;
    cdd_test_copy_schema_ref(&sr_dst, &sr_src);
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&sr_dst);
  }

  for (k = 1; k <= 6; ++k) {
    memset(&ex_src, 0, sizeof(ex_src));
    memset(&ex_dst, 0, sizeof(ex_dst));
    ex_src.name = (char *)(size_t) "n";
    ex_src.summary = (char *)(size_t) "s";
    ex_src.description = (char *)(size_t) "d";
    ex_src.external_value = (char *)(size_t) "ev";
    ex_src.serialized_value = (char *)(size_t) "sv";
    g_cdd_alloc_fail = k;
    cdd_test_copy_example_fields(&ex_dst, &ex_src);
    g_cdd_alloc_fail = 0;
    cdd_test_free_example(&ex_dst);
  }

  g_cdd_alloc_fail = 0;
  PASS();
}

TEST test_openapi_loader_copy_and_free_advanced(void) {
  struct OpenAPI_SchemaRef sr_src, sr_dst;
  struct OpenAPI_Path path_src, path_dst;
  struct OpenAPI_Callback cb_src, cb_dst;
  struct OpenAPI_RequestBody rb_src, rb_dst;
  struct OpenAPI_Server *srv_src, *srv_dst;
  struct OpenAPI_Link lnk_src, lnk_dst;
  struct OpenAPI_Spec spec;
  cdd_c_error_t rc;

  /* 1. SchemaRef comprehensive copy & free */
  memset(&sr_src, 0, sizeof(sr_src));
  memset(&sr_dst, 0, sizeof(sr_dst));

  sr_src.inline_type = strdup("string");
  sr_src.format = strdup("date-time");
  sr_src.description = strdup("sr desc");
  sr_src.summary = strdup("sr sum");
  sr_src.ref = strdup("#/components/schemas/MyString");
  sr_src.ref_name = strdup("MyString");
  sr_src.content_type = strdup("application/json");
  sr_src.content_media_type = strdup("application/json");
  sr_src.content_encoding = strdup("base64");
  sr_src.items_format = strdup("int32");
  sr_src.items_ref = strdup("#/components/schemas/Item");
  sr_src.items_content_media_type = strdup("text/plain");
  sr_src.items_content_encoding = strdup("7bit");
  sr_src.pattern = strdup("^[a-z]+$");
  sr_src.items_pattern = strdup("^[0-9]+$");
  sr_src.schema_extra_json = strdup("{\"x-extra\":1}");
  sr_src.items_extra_json = strdup("{\"x-item-extra\":2}");
  sr_src.deprecated_set = 1;
  sr_src.read_only_set = 1;
  sr_src.write_only_set = 1;
  sr_src.const_value_set = 1;
  sr_src.const_value.type = OA_ANY_STRING;
  sr_src.const_value.string = strdup("const_val");
  sr_src.items_const_value_set = 1;
  sr_src.items_const_value.type = OA_ANY_STRING;
  sr_src.items_const_value.string = strdup("item_const");
  sr_src.example_set = 1;
  sr_src.example.type = OA_ANY_STRING;
  sr_src.example.string = strdup("ex_val");
  sr_src.items_example_set = 1;
  sr_src.items_example.type = OA_ANY_STRING;
  sr_src.items_example.string = strdup("item_ex");
  sr_src.default_value_set = 1;
  sr_src.default_value.type = OA_ANY_STRING;
  sr_src.default_value.string = strdup("def_val");
  sr_src.items_default_value_set = 1;
  sr_src.items_default_value.type = OA_ANY_STRING;
  sr_src.items_default_value.string = strdup("item_def");

  sr_src.type_union = (char **)calloc(1, sizeof(char *));
  if (sr_src.type_union) {
    sr_src.type_union[0] = strdup("string");
    sr_src.n_type_union = 1;
  }
  sr_src.items_type_union = (char **)calloc(1, sizeof(char *));
  if (sr_src.items_type_union) {
    sr_src.items_type_union[0] = strdup("integer");
    sr_src.n_items_type_union = 1;
  }

  sr_src.enum_values =
      (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
  if (sr_src.enum_values) {
    sr_src.enum_values[0].type = OA_ANY_STRING;
    sr_src.enum_values[0].string = strdup("e1");
    sr_src.n_enum_values = 1;
  }
  sr_src.items_enum_values =
      (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
  if (sr_src.items_enum_values) {
    sr_src.items_enum_values[0].type = OA_ANY_STRING;
    sr_src.items_enum_values[0].string = strdup("ie1");
    sr_src.n_items_enum_values = 1;
  }

  sr_src.examples = (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
  if (sr_src.examples) {
    sr_src.examples[0].type = OA_ANY_STRING;
    sr_src.examples[0].string = strdup("ex1");
    sr_src.n_examples = 1;
  }
  sr_src.items_examples =
      (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
  if (sr_src.items_examples) {
    sr_src.items_examples[0].type = OA_ANY_STRING;
    sr_src.items_examples[0].string = strdup("iex1");
    sr_src.n_items_examples = 1;
  }

  sr_src.external_docs.description = strdup("ed_desc");
  sr_src.external_docs.url = strdup("http://ed_url");
  sr_src.external_docs.extensions_json = strdup("{\"x-ed\":1}");

  sr_src.discriminator.property_name = strdup("kind");
  sr_src.discriminator.default_mapping = strdup("def_map");
  sr_src.discriminator.extensions_json = strdup("{\"x-disc\":1}");
  sr_src.discriminator.mapping = (struct OpenAPI_DiscriminatorMap *)calloc(
      1, sizeof(struct OpenAPI_DiscriminatorMap));
  if (sr_src.discriminator.mapping) {
    sr_src.discriminator.mapping[0].value = strdup("k1");
    sr_src.discriminator.mapping[0].schema = strdup("Sub1");
    sr_src.discriminator.n_mapping = 1;
  }

  sr_src.xml.name = strdup("my_xml");
  sr_src.xml.namespace_uri = strdup("http://ns");
  sr_src.xml.prefix = strdup("pfx");
  sr_src.xml.extensions_json = strdup("{\"x-xml\":1}");

  sr_src.not_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.not_schema)
    sr_src.not_schema->inline_type = strdup("null");

  sr_src.if_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.if_schema)
    sr_src.if_schema->inline_type = strdup("object");

  sr_src.then_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.then_schema)
    sr_src.then_schema->inline_type = strdup("object");

  sr_src.else_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.else_schema)
    sr_src.else_schema->inline_type = strdup("object");

  sr_src.content_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.content_schema)
    sr_src.content_schema->inline_type = strdup("string");

  sr_src.items_content_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.items_content_schema)
    sr_src.items_content_schema->inline_type = strdup("string");

  sr_src.all_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.all_of) {
    sr_src.all_of[0].inline_type = strdup("string");
    sr_src.n_all_of = 1;
  }
  sr_src.any_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.any_of) {
    sr_src.any_of[0].inline_type = strdup("integer");
    sr_src.n_any_of = 1;
  }
  sr_src.one_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  if (sr_src.one_of) {
    sr_src.one_of[0].inline_type = strdup("boolean");
    sr_src.n_one_of = 1;
  }

  sr_src.n_multipart_fields = 1;
  sr_src.multipart_fields = (struct OpenAPI_MultipartField *)calloc(
      1, sizeof(struct OpenAPI_MultipartField));
  if (sr_src.multipart_fields) {
    sr_src.multipart_fields[0].name = strdup("f1");
    sr_src.multipart_fields[0].type = strdup("string");
  }

  rc = cdd_test_copy_schema_ref(&sr_dst, &sr_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("string", sr_dst.inline_type);
  ASSERT_STR_EQ("date-time", sr_dst.format);
  ASSERT_STR_EQ("sr desc", sr_dst.description);
  ASSERT_STR_EQ("sr sum", sr_dst.summary);
  ASSERT_STR_EQ("#/components/schemas/MyString", sr_dst.ref);

  cdd_test_free_schema_ref_content(&sr_dst);

  {
    int k;
    for (k = 1; k <= 200; ++k) {
      memset(&sr_dst, 0, sizeof(sr_dst));
      g_cdd_alloc_fail = k;
      cdd_test_copy_schema_ref(&sr_dst, &sr_src);
      cdd_test_free_schema_ref_content(&sr_dst);
    }
    g_cdd_alloc_fail = 0;
    for (k = 1; k <= 200; ++k) {
      memset(&sr_dst, 0, sizeof(sr_dst));
      g_cdd_strdup_fail = k;
      cdd_test_copy_schema_ref(&sr_dst, &sr_src);
      cdd_test_free_schema_ref_content(&sr_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  cdd_test_free_schema_ref_content(&sr_src);

  /* Focused sub-object failure tests for openapi_copy_schema.c */
  {
    struct OpenAPI_SchemaRef t_src, t_dst;

    /* 1. examples copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.n_examples = 1;
    t_src.examples =
        (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
    t_src.examples[0].type = OA_ANY_STRING;
    t_src.examples[0].string = strdup("ex");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 2. enum_values copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.n_enum_values = 1;
    t_src.enum_values =
        (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
    t_src.enum_values[0].type = OA_ANY_STRING;
    t_src.enum_values[0].string = strdup("ev");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 3. items_examples copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.n_items_examples = 1;
    t_src.items_examples =
        (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
    t_src.items_examples[0].type = OA_ANY_STRING;
    t_src.items_examples[0].string = strdup("iex");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 4. items_enum_values copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.n_items_enum_values = 1;
    t_src.items_enum_values =
        (struct OpenAPI_Any *)calloc(1, sizeof(struct OpenAPI_Any));
    t_src.items_enum_values[0].type = OA_ANY_STRING;
    t_src.items_enum_values[0].string = strdup("iev");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 5. discriminator copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.discriminator.property_name = strdup("prop");
    t_src.discriminator.n_mapping = 1;
    t_src.discriminator.mapping = (struct OpenAPI_DiscriminatorMap *)calloc(
        1, sizeof(struct OpenAPI_DiscriminatorMap));
    t_src.discriminator.mapping[0].value = strdup("v");
    t_src.discriminator.mapping[0].schema = strdup("s");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 6. not_schema copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.not_schema =
        (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
    t_src.not_schema->inline_type = strdup("s");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 7. if_schema copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.if_schema =
        (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
    t_src.if_schema->inline_type = strdup("s");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 8. then_schema copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.then_schema =
        (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
    t_src.then_schema->inline_type = strdup("s");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 9. else_schema copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.else_schema =
        (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
    t_src.else_schema->inline_type = strdup("s");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 10. content_schema copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.content_schema =
        (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
    t_src.content_schema->inline_type = strdup("s");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 11. items_content_schema copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.items_content_schema =
        (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
    t_src.items_content_schema->inline_type = strdup("s");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 12. all_of copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.n_all_of = 1;
    t_src.all_of =
        (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
    t_src.all_of[0].inline_type = strdup("s");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 13. any_of copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.n_any_of = 1;
    t_src.any_of =
        (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
    t_src.any_of[0].inline_type = strdup("s");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 14. one_of copy failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.n_one_of = 1;
    t_src.one_of =
        (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
    t_src.one_of[0].inline_type = strdup("s");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);

    /* 15. multipart_fields calloc failure */
    memset(&t_src, 0, sizeof(t_src));
    memset(&t_dst, 0, sizeof(t_dst));
    t_src.n_multipart_fields = 1;
    t_src.multipart_fields = (struct OpenAPI_MultipartField *)calloc(
        1, sizeof(struct OpenAPI_MultipartField));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_copy_schema_ref(&t_dst, &t_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_schema_ref_content(&t_src);
    cdd_test_free_schema_ref_content(&t_dst);
  }

  /* Focused sub-object failure tests for openapi_copy_components.c */
  {
    struct OpenAPI_Header th_src, th_dst;
    struct OpenAPI_Encoding te_src, te_dst;
    struct OpenAPI_MediaType tm_src, tm_dst;
    struct OpenAPI_Response tr_src, tr_dst;

    /* 1. Header examples copy failure */
    memset(&th_src, 0, sizeof(th_src));
    memset(&th_dst, 0, sizeof(th_dst));
    th_src.n_examples = 1;
    th_src.examples =
        (struct OpenAPI_Example *)calloc(1, sizeof(struct OpenAPI_Example));
    th_src.examples[0].summary = strdup("s");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_header_fields(&th_dst, &th_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_header(&th_src);
    cdd_test_free_header(&th_dst);

    /* 2. Encoding headers name and fields copy failure */
    memset(&te_src, 0, sizeof(te_src));
    memset(&te_dst, 0, sizeof(te_dst));
    te_src.n_headers = 1;
    te_src.headers =
        (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
    te_src.headers[0].name = strdup("n");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_encoding_fields(&te_dst, &te_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_encoding(&te_src);
    cdd_test_free_encoding(&te_dst);

    memset(&te_src, 0, sizeof(te_src));
    memset(&te_dst, 0, sizeof(te_dst));
    te_src.n_headers = 1;
    te_src.headers =
        (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
    te_src.headers[0].name = strdup("n");
    te_src.headers[0].description = strdup("d");
    g_cdd_strdup_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_encoding_fields(&te_dst, &te_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_encoding(&te_src);
    cdd_test_free_encoding(&te_dst);

    /* 3. Encoding nested encoding copy failure */
    memset(&te_src, 0, sizeof(te_src));
    memset(&te_dst, 0, sizeof(te_dst));
    te_src.n_encoding = 1;
    te_src.encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    te_src.encoding[0].content_type = strdup("c");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_encoding_fields(&te_dst, &te_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_encoding(&te_src);
    cdd_test_free_encoding(&te_dst);

    /* 4. Encoding prefix_encoding copy failure */
    memset(&te_src, 0, sizeof(te_src));
    memset(&te_dst, 0, sizeof(te_dst));
    te_src.n_prefix_encoding = 1;
    te_src.prefix_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    te_src.prefix_encoding[0].content_type = strdup("c");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_encoding_fields(&te_dst, &te_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_encoding(&te_src);
    cdd_test_free_encoding(&te_dst);

    /* 5. Encoding item_encoding copy failure */
    memset(&te_src, 0, sizeof(te_src));
    memset(&te_dst, 0, sizeof(te_dst));
    te_src.item_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    te_src.item_encoding->content_type = strdup("c");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_encoding_fields(&te_dst, &te_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_encoding(&te_src);
    cdd_test_free_encoding(&te_dst);

    /* 6. Media type nested encoding copy failure */
    memset(&tm_src, 0, sizeof(tm_src));
    memset(&tm_dst, 0, sizeof(tm_dst));
    tm_src.n_encoding = 1;
    tm_src.encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    tm_src.encoding[0].content_type = strdup("c");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_media_type_fields(&tm_dst, &tm_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_media_type(&tm_src);
    cdd_test_free_media_type(&tm_dst);

    /* 7. Media type prefix_encoding copy failure */
    memset(&tm_src, 0, sizeof(tm_src));
    memset(&tm_dst, 0, sizeof(tm_dst));
    tm_src.n_prefix_encoding = 1;
    tm_src.prefix_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    tm_src.prefix_encoding[0].content_type = strdup("c");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_media_type_fields(&tm_dst, &tm_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_media_type(&tm_src);
    cdd_test_free_media_type(&tm_dst);

    /* 8. Media type item_encoding copy failure */
    memset(&tm_src, 0, sizeof(tm_src));
    memset(&tm_dst, 0, sizeof(tm_dst));
    tm_src.item_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    tm_src.item_encoding->content_type = strdup("c");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_media_type_fields(&tm_dst, &tm_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_media_type(&tm_src);
    cdd_test_free_media_type(&tm_dst);

    /* 9. Response headers copy failure */
    memset(&tr_src, 0, sizeof(tr_src));
    memset(&tr_dst, 0, sizeof(tr_dst));
    tr_src.n_headers = 1;
    tr_src.headers =
        (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
    tr_src.headers[0].description = strdup("d");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_response_fields(&tr_dst, &tr_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_response(&tr_src);
    cdd_test_free_response(&tr_dst);

    /* 10. Response links copy failure */
    memset(&tr_src, 0, sizeof(tr_src));
    memset(&tr_dst, 0, sizeof(tr_dst));
    tr_src.n_links = 1;
    tr_src.links =
        (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
    tr_src.links[0].summary = strdup("s");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_response_fields(&tr_dst, &tr_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_response(&tr_src);
    cdd_test_free_response(&tr_dst);

    /* 11. Calloc failures across components */
    memset(&th_src, 0, sizeof(th_src));
    memset(&th_dst, 0, sizeof(th_dst));
    th_src.n_examples = 1;
    th_src.examples =
        (struct OpenAPI_Example *)calloc(1, sizeof(struct OpenAPI_Example));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_header_fields(&th_dst, &th_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_header(&th_src);
    cdd_test_free_header(&th_dst);

    memset(&te_src, 0, sizeof(te_src));
    memset(&te_dst, 0, sizeof(te_dst));
    te_src.n_headers = 1;
    te_src.headers =
        (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_encoding_fields(&te_dst, &te_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_encoding(&te_src);
    cdd_test_free_encoding(&te_dst);

    memset(&te_src, 0, sizeof(te_src));
    memset(&te_dst, 0, sizeof(te_dst));
    te_src.n_encoding = 1;
    te_src.encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_encoding_fields(&te_dst, &te_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_encoding(&te_src);
    cdd_test_free_encoding(&te_dst);

    memset(&te_src, 0, sizeof(te_src));
    memset(&te_dst, 0, sizeof(te_dst));
    te_src.n_prefix_encoding = 1;
    te_src.prefix_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_encoding_fields(&te_dst, &te_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_encoding(&te_src);
    cdd_test_free_encoding(&te_dst);

    memset(&te_src, 0, sizeof(te_src));
    memset(&te_dst, 0, sizeof(te_dst));
    te_src.item_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_encoding_fields(&te_dst, &te_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_encoding(&te_src);
    cdd_test_free_encoding(&te_dst);

    memset(&tm_src, 0, sizeof(tm_src));
    memset(&tm_dst, 0, sizeof(tm_dst));
    tm_src.n_examples = 1;
    tm_src.examples =
        (struct OpenAPI_Example *)calloc(1, sizeof(struct OpenAPI_Example));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_media_type_fields(&tm_dst, &tm_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_media_type(&tm_src);
    cdd_test_free_media_type(&tm_dst);

    memset(&tm_src, 0, sizeof(tm_src));
    memset(&tm_dst, 0, sizeof(tm_dst));
    tm_src.item_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_media_type_fields(&tm_dst, &tm_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_media_type(&tm_src);
    cdd_test_free_media_type(&tm_dst);

    memset(&tm_src, 0, sizeof(tm_src));
    memset(&tm_dst, 0, sizeof(tm_dst));
    tm_src.item_schema_set = 1;
    tm_src.item_schema.inline_type = strdup("s");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_media_type_fields(&tm_dst, &tm_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_media_type(&tm_src);
    cdd_test_free_media_type(&tm_dst);

    memset(&tm_src, 0, sizeof(tm_src));
    memset(&tm_dst, 0, sizeof(tm_dst));
    tm_src.n_encoding = 1;
    tm_src.encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_media_type_fields(&tm_dst, &tm_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_media_type(&tm_src);
    cdd_test_free_media_type(&tm_dst);

    memset(&tm_src, 0, sizeof(tm_src));
    memset(&tm_dst, 0, sizeof(tm_dst));
    tm_src.n_prefix_encoding = 1;
    tm_src.prefix_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_media_type_fields(&tm_dst, &tm_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_media_type(&tm_src);
    cdd_test_free_media_type(&tm_dst);

    memset(&tr_src, 0, sizeof(tr_src));
    memset(&tr_dst, 0, sizeof(tr_dst));
    tr_src.n_content_media_types = 1;
    tr_src.content_media_types =
        (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
    tr_src.content_media_types[0].name = strdup("s");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_response_fields(&tr_dst, &tr_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_response(&tr_src);
    cdd_test_free_response(&tr_dst);

    memset(&tr_src, 0, sizeof(tr_src));
    memset(&tr_dst, 0, sizeof(tr_dst));
    tr_src.n_links = 1;
    tr_src.links =
        (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
    tr_src.links[0].summary = strdup("s");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_response_fields(&tr_dst, &tr_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_response(&tr_src);
    cdd_test_free_response(&tr_dst);

    memset(&tr_src, 0, sizeof(tr_src));
    memset(&tr_dst, 0, sizeof(tr_dst));
    tr_src.n_examples = 1;
    tr_src.examples =
        (struct OpenAPI_Example *)calloc(1, sizeof(struct OpenAPI_Example));
    tr_src.examples[0].name = strdup("n");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_response_fields(&tr_dst, &tr_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_response(&tr_src);
    cdd_test_free_response(&tr_dst);

    memset(&tr_src, 0, sizeof(tr_src));
    memset(&tr_dst, 0, sizeof(tr_dst));
    tr_src.n_headers = 1;
    tr_src.headers =
        (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
    tr_src.headers[0].name = strdup("n");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_response_fields(&tr_dst, &tr_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_response(&tr_src);
    cdd_test_free_response(&tr_dst);

    memset(&tr_src, 0, sizeof(tr_src));
    memset(&tr_dst, 0, sizeof(tr_dst));
    tr_src.n_links = 1;
    tr_src.links =
        (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
    tr_src.links[0].name = strdup("n");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_response_fields(&tr_dst, &tr_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_response(&tr_src);
    cdd_test_free_response(&tr_dst);

    /* Test content_ref copy failure in response (lines 473-476) */
    memset(&tr_src, 0, sizeof(tr_src));
    memset(&tr_dst, 0, sizeof(tr_dst));
    tr_src.content_ref = strdup("#/components/mediaTypes/Mt");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_response_fields(&tr_dst, &tr_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_response(&tr_src);
    cdd_test_free_response(&tr_dst);

    /* Test link ref copy failure in response (lines 548-551) */
    memset(&tr_src, 0, sizeof(tr_src));
    memset(&tr_dst, 0, sizeof(tr_dst));
    tr_src.n_links = 1;
    tr_src.links =
        (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
    tr_src.links[0].ref = strdup("#/components/links/L1");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_response_fields(&tr_dst, &tr_src));
    g_cdd_strdup_fail = 0;
    cdd_test_free_response(&tr_src);
    cdd_test_free_response(&tr_dst);

    memset(&tr_src, 0, sizeof(tr_src));
    memset(&tr_dst, 0, sizeof(tr_dst));
    tr_src.n_examples = 1;
    tr_src.examples =
        (struct OpenAPI_Example *)calloc(1, sizeof(struct OpenAPI_Example));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_response_fields(&tr_dst, &tr_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_response(&tr_src);
    cdd_test_free_response(&tr_dst);

    memset(&tr_src, 0, sizeof(tr_src));
    memset(&tr_dst, 0, sizeof(tr_dst));
    tr_src.n_headers = 1;
    tr_src.headers =
        (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_response_fields(&tr_dst, &tr_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_response(&tr_src);
    cdd_test_free_response(&tr_dst);

    memset(&tr_src, 0, sizeof(tr_src));
    memset(&tr_dst, 0, sizeof(tr_dst));
    tr_src.n_links = 1;
    tr_src.links =
        (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_response_fields(&tr_dst, &tr_src));
    g_cdd_alloc_fail = 0;
    cdd_test_free_response(&tr_src);
    cdd_test_free_response(&tr_dst);
  }

  /* 2. Path copy & free */
  memset(&path_src, 0, sizeof(path_src));
  memset(&path_dst, 0, sizeof(path_dst));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_path_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_path_fields(&path_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_path_fields(NULL, &path_src));

  path_src.route = strdup("/test");
  path_src.ref = strdup("#/components/pathItems/MyPath");
  path_src.summary = strdup("path summary");
  path_src.description = strdup("path description");
  path_src.extensions_json = strdup("{\"x-p\":1}");
  path_src.parameters =
      (struct OpenAPI_Parameter *)calloc(1, sizeof(struct OpenAPI_Parameter));
  if (path_src.parameters) {
    path_src.parameters[0].name = strdup("p1");
    path_src.n_parameters = 1;
  }
  path_src.servers =
      (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
  if (path_src.servers) {
    path_src.servers[0].url = strdup("http://srv");
    path_src.n_servers = 1;
  }
  path_src.operations =
      (struct OpenAPI_Operation *)calloc(1, sizeof(struct OpenAPI_Operation));
  if (path_src.operations) {
    path_src.operations[0].operation_id = strdup("op1");
    path_src.n_operations = 1;
  }
  path_src.additional_operations =
      (struct OpenAPI_Operation *)calloc(1, sizeof(struct OpenAPI_Operation));
  if (path_src.additional_operations) {
    path_src.additional_operations[0].operation_id = strdup("add_op");
    path_src.n_additional_operations = 1;
  }

  rc = cdd_test_copy_path_fields(&path_dst, &path_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("/test", path_dst.route);
  ASSERT_STR_EQ("#/components/pathItems/MyPath", path_dst.ref);
  ASSERT_STR_EQ("path summary", path_dst.summary);
  ASSERT_STR_EQ("path description", path_dst.description);

  cdd_test_free_path_item(&path_dst);

  {
    int k;
    for (k = 1; k <= 25; ++k) {
      memset(&path_dst, 0, sizeof(path_dst));
      g_cdd_alloc_fail = k;
      cdd_test_copy_path_fields(&path_dst, &path_src);
      cdd_test_free_path_item(&path_dst);
    }
    g_cdd_alloc_fail = 0;
    for (k = 1; k <= 25; ++k) {
      memset(&path_dst, 0, sizeof(path_dst));
      g_cdd_strdup_fail = k;
      cdd_test_copy_path_fields(&path_dst, &path_src);
      cdd_test_free_path_item(&path_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  cdd_test_free_path_item(&path_src);

  /* 3. Callback copy & free */
  memset(&cb_src, 0, sizeof(cb_src));
  memset(&cb_dst, 0, sizeof(cb_dst));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_callback_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_callback_fields(&cb_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_callback_fields(NULL, &cb_src));

  cb_src.name = strdup("cb1");
  cb_src.ref = strdup("#/components/callbacks/MyCb");
  cb_src.summary = strdup("cb summary");
  cb_src.description = strdup("cb description");
  cb_src.extensions_json = strdup("{\"x-cb\":1}");
  cb_src.paths = (struct OpenAPI_Path *)calloc(1, sizeof(struct OpenAPI_Path));
  if (cb_src.paths) {
    cb_src.paths[0].route = strdup("/hook");
    cb_src.n_paths = 1;
  }

  rc = cdd_test_copy_callback_fields(&cb_dst, &cb_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("cb1", cb_dst.name);
  ASSERT_STR_EQ("#/components/callbacks/MyCb", cb_dst.ref);
  ASSERT_STR_EQ("cb summary", cb_dst.summary);

  cdd_test_free_callback(&cb_dst);

  {
    int k;
    for (k = 1; k <= 25; ++k) {
      memset(&cb_dst, 0, sizeof(cb_dst));
      g_cdd_alloc_fail = k;
      cdd_test_copy_callback_fields(&cb_dst, &cb_src);
      cdd_test_free_callback(&cb_dst);
    }
    g_cdd_alloc_fail = 0;
    for (k = 1; k <= 25; ++k) {
      memset(&cb_dst, 0, sizeof(cb_dst));
      g_cdd_strdup_fail = k;
      cdd_test_copy_callback_fields(&cb_dst, &cb_src);
      cdd_test_free_callback(&cb_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  cdd_test_free_callback(&cb_src);

  /* 4. RequestBody copy & free */
  memset(&rb_src, 0, sizeof(rb_src));
  memset(&rb_dst, 0, sizeof(rb_dst));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_request_body_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_request_body_fields(&rb_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_request_body_fields(NULL, &rb_src));

  rb_src.ref = strdup("#/components/requestBodies/MyRb");
  rb_src.description = strdup("rb description");
  rb_src.content_ref = strdup("#/components/mediaTypes/Mt");
  rb_src.extensions_json = strdup("{\"x-rb\":1}");
  rb_src.example_set = 1;
  rb_src.example.type = OA_ANY_STRING;
  rb_src.example.string = strdup("ex");

  /* Test copy_security_requirement_sets NULL paths */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_copy_security_requirement_sets(NULL, NULL, NULL, 0));
  {
    struct OpenAPI_SecurityRequirementSet *dst_dummy = NULL;
    size_t count_dummy = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_security_requirement_sets(
                                 &dst_dummy, &count_dummy, NULL, 0));
  }

  /* Set existing description on rb_dst to cover free on overwrite (lines
   * 642-643) */
  rb_dst.description = strdup("existing");
  rc = cdd_test_copy_request_body_fields(&rb_dst, &rb_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("#/components/requestBodies/MyRb", rb_dst.ref);
  ASSERT_STR_EQ("rb description", rb_dst.description);

  cdd_test_free_request_body(&rb_dst);

  {
    int k;
    for (k = 1; k <= 25; ++k) {
      memset(&rb_dst, 0, sizeof(rb_dst));
      g_cdd_alloc_fail = k;
      cdd_test_copy_request_body_fields(&rb_dst, &rb_src);
      cdd_test_free_request_body(&rb_dst);
    }
    g_cdd_alloc_fail = 0;
    for (k = 1; k <= 25; ++k) {
      memset(&rb_dst, 0, sizeof(rb_dst));
      g_cdd_strdup_fail = k;
      cdd_test_copy_request_body_fields(&rb_dst, &rb_src);
      cdd_test_free_request_body(&rb_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  cdd_test_free_request_body(&rb_src);

  /* 5. Server copy & free */
  srv_src = (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
  srv_dst = (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_server_object(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_server_object(srv_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_server_object(NULL, srv_src));

  if (srv_src && srv_dst) {
    srv_src->url = strdup("https://example.com");
    srv_src->description = strdup("prod server");
    srv_src->name = strdup("prod");
    srv_src->extensions_json = strdup("{\"x-srv\":1}");
    srv_src->variables = (struct OpenAPI_ServerVariable *)calloc(
        1, sizeof(struct OpenAPI_ServerVariable));
    if (srv_src->variables) {
      srv_src->variables[0].name = strdup("port");
      srv_src->variables[0].default_value = strdup("443");
      srv_src->variables[0].description = strdup("port var");
      srv_src->variables[0].extensions_json = strdup("{\"x-var\":1}");
      srv_src->variables[0].n_enum_values = 1;
      srv_src->variables[0].enum_values = (char **)calloc(1, sizeof(char *));
      if (srv_src->variables[0].enum_values) {
        srv_src->variables[0].enum_values[0] = strdup("443");
      }
      srv_src->n_variables = 1;
    }

    rc = cdd_test_copy_server_object(srv_dst, srv_src);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("https://example.com", srv_dst->url);
    ASSERT_STR_EQ("prod server", srv_dst->description);

    openapi_free_servers_array(srv_dst, 1);
    srv_dst = NULL;

    {
      int k;
      for (k = 1; k <= 15; ++k) {
        struct OpenAPI_Server *tmp_srv =
            (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
        g_cdd_strdup_fail = k;
        cdd_test_copy_server_object(tmp_srv, srv_src);
        openapi_free_servers_array(tmp_srv, 1);
      }
      g_cdd_strdup_fail = 0;
    }
  }

  openapi_free_servers_array(srv_src, 1);
  openapi_free_servers_array(NULL, 0);

  /* 6. Link copy & free */
  memset(&lnk_src, 0, sizeof(lnk_src));
  memset(&lnk_dst, 0, sizeof(lnk_dst));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_link_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_link_fields(&lnk_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_link_fields(NULL, &lnk_src));

  lnk_src.name = strdup("link1");
  lnk_src.ref = strdup("#/components/links/MyLink");
  lnk_src.operation_id = strdup("op1");
  lnk_src.operation_ref = strdup("#/paths/~1foo/get");
  lnk_src.summary = strdup("link summary");
  lnk_src.description = strdup("link description");
  lnk_src.extensions_json = strdup("{\"x-lnk\":1}");
  lnk_src.parameters =
      (struct OpenAPI_LinkParam *)calloc(1, sizeof(struct OpenAPI_LinkParam));
  if (lnk_src.parameters) {
    lnk_src.parameters[0].name = strdup("userId");
    lnk_src.parameters[0].value.type = OA_ANY_STRING;
    lnk_src.parameters[0].value.string = strdup("$response.body#/id");
    lnk_src.n_parameters = 1;
  }
  lnk_src.request_body_set = 1;
  lnk_src.request_body.type = OA_ANY_STRING;
  lnk_src.request_body.string = strdup("req");
  lnk_src.server_set = 1;
  lnk_src.server =
      (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
  if (lnk_src.server)
    lnk_src.server->url = strdup("http://srv2");

  rc = cdd_test_copy_link_fields(&lnk_dst, &lnk_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("link summary", lnk_dst.summary);
  ASSERT_STR_EQ("link description", lnk_dst.description);

  cdd_test_free_link(&lnk_dst);

  {
    int k;
    for (k = 1; k <= 15; ++k) {
      memset(&lnk_dst, 0, sizeof(lnk_dst));
      g_cdd_strdup_fail = k;
      cdd_test_copy_link_fields(&lnk_dst, &lnk_src);
      cdd_test_free_link(&lnk_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  cdd_test_free_link(&lnk_src);

  /* 7. Test schema boolean constraints parsing */
  {
    const char *excl_json =
        "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
        "\"paths\":{\"/"
        "items\":{\"get\":{\"responses\":{\"200\":{\"description\":"
        "\"OK\",\"content\":{\"application/json\":{\"schema\":{\"type\":"
        "\"integer\",\"minimum\":5,\"exclusiveMinimum\":true,\"maximum\":100,"
        "\"exclusiveMaximum\":true}}}}}}}}}";
    JSON_Value *jv = json_parse_string(excl_json);
    if (jv) {
      memset(&spec, 0, sizeof(spec));
      rc = openapi_load_from_json(jv, &spec);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      openapi_spec_free(&spec);
      json_value_free(jv);
    }
  }

  /* 8. openapi_doc_registry tests */
  {
    extern C_CDD_EXPORT int g_cdd_alloc_fail;
    struct OpenAPI_DocRegistry reg;
    struct OpenAPI_Spec s1;
    struct OpenAPI_Spec s2;

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, openapi_doc_registry_init(NULL));
    openapi_doc_registry_free(NULL);

    ASSERT_EQ(CDD_C_SUCCESS, openapi_doc_registry_init(&reg));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              openapi_doc_registry_add(NULL, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              openapi_doc_registry_add(&reg, NULL));

    memset(&s1, 0, sizeof(s1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              openapi_doc_registry_add(&reg, &s1));

    s1.document_uri = strdup("");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              openapi_doc_registry_add(&reg, &s1));
    free(s1.document_uri);

    s1.document_uri = strdup("#only_fragment");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              openapi_doc_registry_add(&reg, &s1));
    free(s1.document_uri);

    s1.document_uri = strdup("http://example.com/spec.json");
    ASSERT_EQ(CDD_C_SUCCESS, openapi_doc_registry_add(&reg, &s1));

    /* Duplicate base_uri returns invalid argument */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              openapi_doc_registry_add(&reg, &s1));

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

    openapi_doc_registry_free(&reg);
    openapi_spec_free(&s1);
  }

  /* 9. Extra free paths */
  {
    struct OpenAPI_Spec s_free;
    struct OpenAPI_SchemaRef ref_free;
    struct OpenAPI_Encoding enc_free;
    struct OpenAPI_MediaType mt_free;
    struct OpenAPI_Header hdr_free;
    struct OpenAPI_SecurityRequirementSet sec_set_free;

    openapi_spec_free(NULL);
    cdd_test_free_security_requirement_set(NULL);

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
    s_free.security_schemes[0].flows[0].extensions_json =
        strdup("{\"x-ext\":1}");
    openapi_spec_free(&s_free);

    memset(&ref_free, 0, sizeof(ref_free));
    ref_free.n_multipart_fields = 1;
    ref_free.multipart_fields = (struct OpenAPI_MultipartField *)calloc(
        1, sizeof(struct OpenAPI_MultipartField));
    ref_free.multipart_fields[0].name = strdup("f");
    ref_free.multipart_fields[0].type = strdup("t");
    cdd_test_free_schema_ref_content(&ref_free);

    memset(&enc_free, 0, sizeof(enc_free));
    enc_free.extensions_json = strdup("{\"x-ext\":1}");
    cdd_test_free_encoding(&enc_free);

    memset(&mt_free, 0, sizeof(mt_free));
    mt_free.extensions_json = strdup("{\"x-ext\":1}");
    cdd_test_free_media_type(&mt_free);

    memset(&hdr_free, 0, sizeof(hdr_free));
    hdr_free.extensions_json = strdup("{\"x-ext\":1}");
    cdd_test_free_header(&hdr_free);

    memset(&sec_set_free, 0, sizeof(sec_set_free));
    sec_set_free.n_requirements = 1;
    sec_set_free.requirements = (struct OpenAPI_SecurityRequirement *)calloc(
        1, sizeof(struct OpenAPI_SecurityRequirement));
    sec_set_free.requirements[0].scheme = strdup("Basic");
    sec_set_free.extensions_json = strdup("{\"x-ext\":1}");
    cdd_test_free_security_requirement_set(&sec_set_free);
  }

  /* 10. Specific branch coverage for openapi_free_components.c and
   * openapi_free.c */
  {
    struct OpenAPI_Any val_null_json;
    struct OpenAPI_Link lnk_null_srv;
    struct OpenAPI_Spec s_free2;
    char **null_arr;

    memset(&val_null_json, 0, sizeof(val_null_json));
    val_null_json.type = OA_ANY_JSON;
    val_null_json.json = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_free_any_value(&val_null_json));

    memset(&lnk_null_srv, 0, sizeof(lnk_null_srv));
    lnk_null_srv.server_set = 1;
    lnk_null_srv.server = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_free_link(&lnk_null_srv));

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
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_free_string_array(null_arr, 1));

    {
      struct OpenAPI_Spec s_dummy;
      extern C_CDD_EXPORT int g_openapi_spec_init_fail;
      struct OpenAPI_DocRegistry empty_reg;

      g_openapi_spec_init_fail = 2;
      ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&s_dummy));
      g_openapi_spec_init_fail = 0;

      memset(&empty_reg, 0, sizeof(empty_reg));
      openapi_doc_registry_free(&empty_reg);
    }
  }

  /* 11. Branch coverage for openapi_copy_schema.c */
  {
    struct OpenAPI_SchemaRef s_src, s_dst;
    struct OpenAPI_Any any_src, any_dst;
    char dummy_str_buf[8];
    char *dummy_str;
    struct OpenAPI_Any dummy_any;
    struct OpenAPI_DiscriminatorMap dummy_map;
    struct OpenAPI_SchemaRef dummy_ref;
    struct OpenAPI_MultipartField dummy_field;

    dummy_str_buf[0] = 'd';
    dummy_str_buf[1] = '\0';
    dummy_str = dummy_str_buf;
    memset(&dummy_any, 0, sizeof(dummy_any));
    memset(&dummy_map, 0, sizeof(dummy_map));
    memset(&dummy_ref, 0, sizeof(dummy_ref));
    memset(&dummy_field, 0, sizeof(dummy_field));

    /* A. Non-null array pointers with n == 0 */
    memset(&s_src, 0, sizeof(s_src));
    memset(&s_dst, 0, sizeof(s_dst));
    s_src.type_union = &dummy_str;
    s_src.n_type_union = 0;
    s_src.items_type_union = &dummy_str;
    s_src.n_items_type_union = 0;
    s_src.examples = &dummy_any;
    s_src.n_examples = 0;
    s_src.enum_values = &dummy_any;
    s_src.n_enum_values = 0;
    s_src.discriminator.mapping = &dummy_map;
    s_src.discriminator.n_mapping = 0;
    s_src.items_enum_values = &dummy_any;
    s_src.n_items_enum_values = 0;
    s_src.items_examples = &dummy_any;
    s_src.n_items_examples = 0;
    s_src.all_of = &dummy_ref;
    s_src.n_all_of = 0;
    s_src.any_of = &dummy_ref;
    s_src.n_any_of = 0;
    s_src.one_of = &dummy_ref;
    s_src.n_one_of = 0;
    s_src.multipart_fields = NULL;
    s_src.n_multipart_fields = 1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_schema_ref(&s_dst, &s_src));

    /* B. Discriminator mapping and multipart_fields with NULL subfields */
    memset(&s_src, 0, sizeof(s_src));
    memset(&s_dst, 0, sizeof(s_dst));
    s_src.discriminator.mapping = &dummy_map;
    s_src.discriminator.n_mapping = 1;
    dummy_map.value = NULL;
    dummy_map.schema = NULL;
    s_src.multipart_fields = &dummy_field;
    s_src.n_multipart_fields = 1;
    dummy_field.name = NULL;
    dummy_field.type = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_schema_ref(&s_dst, &s_src));
    cdd_test_free_schema_ref_content(&s_dst);

    /* C. copy_item_schema_as_array with non-boolean schema and NULL checks */
    memset(&s_src, 0, sizeof(s_src));
    memset(&s_dst, 0, sizeof(s_dst));
    s_src.schema_is_boolean = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_item_schema_as_array(NULL, &s_src));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_item_schema_as_array(&s_dst, NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_item_schema_as_array(&s_dst, &s_src));

    /* D. copy_any_value NULL checks and NULL string/json */
    memset(&any_src, 0, sizeof(any_src));
    memset(&any_dst, 0, sizeof(any_dst));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_any_value(NULL, &any_src));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_any_value(&any_dst, NULL));

    any_src.type = OA_ANY_STRING;
    any_src.string = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_any_value(&any_dst, &any_src));

    any_src.type = OA_ANY_JSON;
    any_src.json = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_any_value(&any_dst, &any_src));
  }

  /* 12. Branch coverage for openapi_copy_components.c and
   * openapi_copy_operations.c */
  {
    struct OpenAPI_Parameter param_src, param_dst;
    struct OpenAPI_Header hdr_src, hdr_dst;
    struct OpenAPI_Encoding enc_src, enc_dst;
    struct OpenAPI_MediaType mt_src, mt_dst;
    struct OpenAPI_Response resp_src, resp_dst;
    struct OpenAPI_Server s_srv_src;
    struct OpenAPI_ServerVariable svar_dummy;
    struct OpenAPI_Link s_lnk_src, s_lnk_dst;
    struct OpenAPI_SecurityRequirementSet sec_src, sec_dst;
    struct OpenAPI_SecurityRequirement sreq_dummy;
    struct OpenAPI_Callback s_cb_src, s_cb_dst;
    struct OpenAPI_Operation op_src, op_dst;
    struct OpenAPI_Path s_path_src, s_path_dst;
    struct OpenAPI_RequestBody s_rb_src, s_rb_dst;

    struct OpenAPI_MediaType dummy_mt;
    struct OpenAPI_Example dummy_ex;
    struct OpenAPI_Header dummy_hdr;
    struct OpenAPI_Encoding dummy_enc;
    struct OpenAPI_Link dummy_lnk;
    struct OpenAPI_LinkParam dummy_lp;
    struct OpenAPI_Path dummy_path;
    struct OpenAPI_Parameter dummy_param;
    struct OpenAPI_Server dummy_srv;
    struct OpenAPI_Operation dummy_op;
    char dummy_str_buf[8];
    char *dummy_str;
    char *dummy_str_arr[1];

    dummy_str_buf[0] = 'd';
    dummy_str_buf[1] = '\0';
    dummy_str = dummy_str_buf;
    dummy_str_arr[0] = NULL;

    memset(&dummy_mt, 0, sizeof(dummy_mt));
    memset(&dummy_ex, 0, sizeof(dummy_ex));
    memset(&dummy_hdr, 0, sizeof(dummy_hdr));
    memset(&dummy_enc, 0, sizeof(dummy_enc));
    memset(&dummy_lnk, 0, sizeof(dummy_lnk));
    memset(&dummy_lp, 0, sizeof(dummy_lp));
    memset(&dummy_path, 0, sizeof(dummy_path));
    memset(&dummy_param, 0, sizeof(dummy_param));
    memset(&dummy_srv, 0, sizeof(dummy_srv));
    memset(&dummy_op, 0, sizeof(dummy_op));
    memset(&svar_dummy, 0, sizeof(svar_dummy));
    memset(&sreq_dummy, 0, sizeof(sreq_dummy));

    /* 1. copy_parameter_fields & copy_header_fields with non-null pointer, n=0
     */
    memset(&param_src, 0, sizeof(param_src));
    memset(&param_dst, 0, sizeof(param_dst));
    param_src.content_media_types = &dummy_mt;
    param_src.n_content_media_types = 0;
    param_src.examples = &dummy_ex;
    param_src.n_examples = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_parameter_fields(&param_dst, &param_src));

    memset(&hdr_src, 0, sizeof(hdr_src));
    memset(&hdr_dst, 0, sizeof(hdr_dst));
    hdr_src.content_media_types = &dummy_mt;
    hdr_src.n_content_media_types = 0;
    hdr_src.examples = &dummy_ex;
    hdr_src.n_examples = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_header_fields(&hdr_dst, &hdr_src));

    /* 2. copy_encoding_fields branches */
    memset(&enc_src, 0, sizeof(enc_src));
    memset(&enc_dst, 0, sizeof(enc_dst));
    enc_src.name = dummy_str;
    enc_dst.name = strdup("name");
    enc_src.headers = &dummy_hdr;
    enc_src.n_headers = 0;
    enc_src.encoding = &dummy_enc;
    enc_src.n_encoding = 0;
    enc_src.prefix_encoding = &dummy_enc;
    enc_src.n_prefix_encoding = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_encoding_fields(&enc_dst, &enc_src));
    cdd_test_free_encoding(&enc_dst);

    memset(&enc_src, 0, sizeof(enc_src));
    memset(&enc_dst, 0, sizeof(enc_dst));
    enc_src.headers = &dummy_hdr;
    enc_src.n_headers = 1;
    dummy_hdr.name = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_encoding_fields(&enc_dst, &enc_src));
    cdd_test_free_encoding(&enc_dst);

    memset(&enc_src, 0, sizeof(enc_src));
    memset(&enc_dst, 0, sizeof(enc_dst));
    enc_src.n_encoding = 1;
    enc_src.encoding = &dummy_enc;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_encoding_fields(&enc_dst, &enc_src));
    cdd_test_free_encoding(&enc_dst);

    /* 3. copy_media_type_fields branches */
    memset(&mt_src, 0, sizeof(mt_src));
    memset(&mt_dst, 0, sizeof(mt_dst));
    mt_src.name = dummy_str;
    mt_dst.name = strdup("name");
    mt_src.ref = dummy_str;
    mt_dst.ref = strdup("ref");
    mt_src.schema.is_array = 1;
    mt_src.item_schema.is_array = 1;
    mt_src.examples = &dummy_ex;
    mt_src.n_examples = 0;
    mt_src.encoding = &dummy_enc;
    mt_src.n_encoding = 0;
    mt_src.prefix_encoding = &dummy_enc;
    mt_src.n_prefix_encoding = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_media_type_fields(&mt_dst, &mt_src));
    cdd_test_free_media_type(&mt_dst);

    memset(&mt_src, 0, sizeof(mt_src));
    memset(&mt_dst, 0, sizeof(mt_dst));
    mt_src.schema.ref_name = dummy_str;
    mt_src.item_schema.ref_name = dummy_str;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_media_type_fields(&mt_dst, &mt_src));
    cdd_test_free_media_type(&mt_dst);

    memset(&mt_src, 0, sizeof(mt_src));
    memset(&mt_dst, 0, sizeof(mt_dst));
    mt_src.schema.n_multipart_fields = 1;
    mt_src.item_schema.n_multipart_fields = 1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_media_type_fields(&mt_dst, &mt_src));
    cdd_test_free_media_type(&mt_dst);

    memset(&mt_src, 0, sizeof(mt_src));
    memset(&mt_dst, 0, sizeof(mt_dst));
    mt_src.schema.inline_type = dummy_str;
    mt_src.item_schema.inline_type = dummy_str;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_media_type_fields(&mt_dst, &mt_src));
    cdd_test_free_media_type(&mt_dst);

    /* 4. copy_media_type_array NULL checks */
    {
      struct OpenAPI_MediaType *mt_arr = NULL;
      size_t mt_cnt = 0;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_copy_media_type_array(&mt_arr, NULL, &dummy_mt, 1));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_copy_media_type_array(&mt_arr, &mt_cnt, &dummy_mt, 0));
    }

    /* 5. copy_response_fields branches */
    memset(&resp_src, 0, sizeof(resp_src));
    memset(&resp_dst, 0, sizeof(resp_dst));
    resp_src.content_media_types = &dummy_mt;
    resp_src.n_content_media_types = 0;
    resp_src.examples = &dummy_ex;
    resp_src.n_examples = 0;
    resp_src.headers = NULL;
    resp_src.n_headers = 1;
    resp_src.links = NULL;
    resp_src.n_links = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_response_fields(&resp_dst, &resp_src));

    memset(&resp_src, 0, sizeof(resp_src));
    memset(&resp_dst, 0, sizeof(resp_dst));
    resp_src.content_ref = dummy_str;
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_test_copy_response_fields(&resp_dst, &resp_src));
    g_cdd_strdup_fail = 0;

    memset(&resp_src, 0, sizeof(resp_src));
    memset(&resp_dst, 0, sizeof(resp_dst));
    resp_src.content_ref = dummy_str;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_response_fields(&resp_dst, &resp_src));
    cdd_test_free_response(&resp_dst);

    memset(&resp_src, 0, sizeof(resp_src));
    memset(&resp_dst, 0, sizeof(resp_dst));
    resp_src.examples = &dummy_ex;
    resp_src.n_examples = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_response_fields(&resp_dst, &resp_src));
    cdd_test_free_response(&resp_dst);

    memset(&resp_src, 0, sizeof(resp_src));
    memset(&resp_dst, 0, sizeof(resp_dst));
    resp_src.headers = &dummy_hdr;
    resp_src.n_headers = 1;
    dummy_hdr.name = NULL;
    resp_src.links = &dummy_lnk;
    resp_src.n_links = 1;
    dummy_lnk.name = dummy_str;
    dummy_lnk.ref = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_response_fields(&resp_dst, &resp_src));
    cdd_test_free_response(&resp_dst);

    memset(&resp_src, 0, sizeof(resp_src));
    memset(&resp_dst, 0, sizeof(resp_dst));
    resp_src.content_media_types = &dummy_mt;
    resp_src.n_content_media_types = 1;
    resp_src.headers = &dummy_hdr;
    resp_src.n_headers = 1;
    dummy_hdr.name = dummy_str;
    resp_src.links = &dummy_lnk;
    resp_src.n_links = 1;
    dummy_lnk.name = dummy_str;
    dummy_lnk.ref = dummy_str;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_response_fields(&resp_dst, &resp_src));
    cdd_test_free_response(&resp_dst);

    /* 6. copy_server_object branches */
    {
      struct OpenAPI_Server *p_srv =
          (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
      memset(&s_srv_src, 0, sizeof(s_srv_src));
      s_srv_src.variables = NULL;
      s_srv_src.n_variables = 1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_server_object(p_srv, &s_srv_src));
      openapi_free_servers_array(p_srv, 1);

      p_srv = (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
      memset(&s_srv_src, 0, sizeof(s_srv_src));
      s_srv_src.variables = &svar_dummy;
      s_srv_src.n_variables = 1;
      svar_dummy.name = NULL;
      svar_dummy.default_value = NULL;
      svar_dummy.enum_values = NULL;
      svar_dummy.n_enum_values = 1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_server_object(p_srv, &s_srv_src));
      openapi_free_servers_array(p_srv, 1);

      p_srv = (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
      memset(&s_srv_src, 0, sizeof(s_srv_src));
      s_srv_src.variables = &svar_dummy;
      s_srv_src.n_variables = 1;
      svar_dummy.name = dummy_str;
      svar_dummy.default_value = dummy_str;
      svar_dummy.enum_values = dummy_str_arr;
      svar_dummy.n_enum_values = 1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_server_object(p_srv, &s_srv_src));
      openapi_free_servers_array(p_srv, 1);

      p_srv = (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
      memset(&s_srv_src, 0, sizeof(s_srv_src));
      s_srv_src.variables = &svar_dummy;
      s_srv_src.n_variables = 1;
      svar_dummy.name = dummy_str;
      svar_dummy.default_value = dummy_str;
      svar_dummy.enum_values = &dummy_str;
      svar_dummy.n_enum_values = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_server_object(p_srv, &s_srv_src));
      openapi_free_servers_array(p_srv, 1);
    }

    /* 7. copy_link_fields branches */
    memset(&s_lnk_src, 0, sizeof(s_lnk_src));
    memset(&s_lnk_dst, 0, sizeof(s_lnk_dst));
    s_lnk_src.parameters = NULL;
    s_lnk_src.n_parameters = 1;
    s_lnk_src.server_set = 1;
    s_lnk_src.server = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_link_fields(&s_lnk_dst, &s_lnk_src));

    memset(&s_lnk_src, 0, sizeof(s_lnk_src));
    memset(&s_lnk_dst, 0, sizeof(s_lnk_dst));
    s_lnk_src.parameters = &dummy_lp;
    s_lnk_src.n_parameters = 1;
    dummy_lp.name = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_link_fields(&s_lnk_dst, &s_lnk_src));
    cdd_test_free_link(&s_lnk_dst);

    /* 8. copy_security_requirement_sets branches */
    memset(&sec_src, 0, sizeof(sec_src));
    memset(&sec_dst, 0, sizeof(sec_dst));
    {
      struct OpenAPI_SecurityRequirementSet *sec_arr = NULL;
      size_t sec_cnt = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_security_requirement_sets(
                                   &sec_arr, NULL, &sec_src, 1));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_security_requirement_sets(
                                   &sec_arr, &sec_cnt, &sec_src, 0));
    }
    sec_src.requirements = NULL;
    sec_src.n_requirements = 1;
    {
      struct OpenAPI_SecurityRequirementSet *dst_p = NULL;
      size_t cnt_p = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_security_requirement_sets(
                                   &dst_p, &cnt_p, &sec_src, 1));
      cdd_test_free_security_requirement_set(dst_p);
      free(dst_p);
    }
    memset(&sec_src, 0, sizeof(sec_src));
    sec_src.requirements = &sreq_dummy;
    sec_src.n_requirements = 1;
    sreq_dummy.scheme = NULL;
    sreq_dummy.scopes = NULL;
    sreq_dummy.n_scopes = 1;
    {
      struct OpenAPI_SecurityRequirementSet *dst_p = NULL;
      size_t cnt_p = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_security_requirement_sets(
                                   &dst_p, &cnt_p, &sec_src, 1));
      cdd_test_free_security_requirement_set(dst_p);
      free(dst_p);
    }
    sreq_dummy.scopes = dummy_str_arr;
    sreq_dummy.n_scopes = 1;
    {
      struct OpenAPI_SecurityRequirementSet *dst_p = NULL;
      size_t cnt_p = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_security_requirement_sets(
                                   &dst_p, &cnt_p, &sec_src, 1));
      cdd_test_free_security_requirement_set(dst_p);
      free(dst_p);
    }
    memset(&sec_src, 0, sizeof(sec_src));
    sec_src.requirements = &sreq_dummy;
    sec_src.n_requirements = 0;
    {
      struct OpenAPI_SecurityRequirementSet *dst_p = NULL;
      size_t cnt_p = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_security_requirement_sets(
                                   &dst_p, &cnt_p, &sec_src, 1));
      cdd_test_free_security_requirement_set(dst_p);
      free(dst_p);
    }

    /* 9. copy_callback_fields branches */
    memset(&s_cb_src, 0, sizeof(s_cb_src));
    memset(&s_cb_dst, 0, sizeof(s_cb_dst));
    s_cb_src.name = dummy_str;
    s_cb_dst.name = strdup("n");
    s_cb_src.ref = dummy_str;
    s_cb_dst.ref = strdup("r");
    s_cb_src.summary = dummy_str;
    s_cb_dst.summary = strdup("s");
    s_cb_src.description = dummy_str;
    s_cb_dst.description = strdup("d");
    s_cb_src.extensions_json = dummy_str;
    s_cb_dst.extensions_json = strdup("e");
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_callback_fields(&s_cb_dst, &s_cb_src));
    cdd_test_free_callback(&s_cb_dst);

    memset(&s_cb_src, 0, sizeof(s_cb_src));
    memset(&s_cb_dst, 0, sizeof(s_cb_dst));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_callback_fields(&s_cb_dst, &s_cb_src));

    memset(&s_cb_src, 0, sizeof(s_cb_src));
    memset(&s_cb_dst, 0, sizeof(s_cb_dst));
    s_cb_src.n_paths = 1;
    s_cb_src.paths = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_callback_fields(&s_cb_dst, &s_cb_src));

    memset(&s_cb_src, 0, sizeof(s_cb_src));
    memset(&s_cb_dst, 0, sizeof(s_cb_dst));
    s_cb_dst.paths = &dummy_path;
    s_cb_src.n_paths = 1;
    s_cb_src.paths = &dummy_path;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_callback_fields(&s_cb_dst, &s_cb_src));
    memset(&s_cb_dst, 0, sizeof(s_cb_dst));

    /* 10. copy_operation_fields branches */
    memset(&op_src, 0, sizeof(op_src));
    memset(&op_dst, 0, sizeof(op_dst));
    op_src.security = NULL;
    op_src.n_security = 1;
    op_src.parameters = NULL;
    op_src.n_parameters = 1;
    op_src.tags = NULL;
    op_src.n_tags = 1;
    op_src.req_body_media_types = NULL;
    op_src.n_req_body_media_types = 1;
    op_src.servers = NULL;
    op_src.n_servers = 1;
    op_src.responses = NULL;
    op_src.n_responses = 1;
    op_src.callbacks = NULL;
    op_src.n_callbacks = 1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_operation_fields(&op_dst, &op_src));

    memset(&op_src, 0, sizeof(op_src));
    memset(&op_dst, 0, sizeof(op_dst));
    op_src.tags = dummy_str_arr;
    op_src.n_tags = 1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_copy_operation_fields(&op_dst, &op_src));
    cdd_test_free_operation(&op_dst);

    /* 11. copy_path_fields branches */
    memset(&s_path_src, 0, sizeof(s_path_src));
    memset(&s_path_dst, 0, sizeof(s_path_dst));
    s_path_src.route = dummy_str;
    s_path_dst.route = strdup("rt");
    s_path_src.ref = dummy_str;
    s_path_dst.ref = strdup("rf");
    s_path_src.summary = dummy_str;
    s_path_dst.summary = strdup("sm");
    s_path_src.description = dummy_str;
    s_path_dst.description = strdup("ds");
    s_path_src.extensions_json = dummy_str;
    s_path_dst.extensions_json = strdup("ex");
    s_path_src.parameters = NULL;
    s_path_src.n_parameters = 1;
    s_path_src.servers = NULL;
    s_path_src.n_servers = 1;
    s_path_src.operations = NULL;
    s_path_src.n_operations = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_path_fields(&s_path_dst, &s_path_src));
    cdd_test_free_path_item(&s_path_dst);

    memset(&s_path_src, 0, sizeof(s_path_src));
    memset(&s_path_dst, 0, sizeof(s_path_dst));
    s_path_dst.parameters = &dummy_param;
    s_path_dst.servers = &dummy_srv;
    s_path_dst.operations = &dummy_op;
    s_path_dst.additional_operations = &dummy_op;
    s_path_src.n_parameters = 1;
    s_path_src.parameters = &dummy_param;
    s_path_src.n_servers = 1;
    s_path_src.servers = &dummy_srv;
    s_path_src.n_operations = 1;
    s_path_src.operations = &dummy_op;
    s_path_src.n_additional_operations = 1;
    s_path_src.additional_operations = &dummy_op;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_path_fields(&s_path_dst, &s_path_src));
    memset(&s_path_dst, 0, sizeof(s_path_dst));

    memset(&s_path_src, 0, sizeof(s_path_src));
    memset(&s_path_dst, 0, sizeof(s_path_dst));
    s_path_src.n_additional_operations = 1;
    s_path_src.additional_operations = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_path_fields(&s_path_dst, &s_path_src));

    /* 12. copy_request_body_fields branches */
    memset(&s_rb_src, 0, sizeof(s_rb_src));
    memset(&s_rb_dst, 0, sizeof(s_rb_dst));
    s_rb_src.ref = dummy_str;
    s_rb_dst.ref = strdup("old_ref");
    s_rb_src.description = dummy_str;
    s_rb_dst.description = strdup("old_desc");
    s_rb_src.content_media_types = &dummy_mt;
    s_rb_src.n_content_media_types = 0;
    s_rb_src.examples = &dummy_ex;
    s_rb_src.n_examples = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_request_body_fields(&s_rb_dst, &s_rb_src));
    cdd_test_free_request_body(&s_rb_dst);

    memset(&s_rb_src, 0, sizeof(s_rb_src));
    memset(&s_rb_dst, 0, sizeof(s_rb_dst));
    s_rb_src.description = dummy_str;
    s_rb_dst.description = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_request_body_fields(&s_rb_dst, &s_rb_src));
    cdd_test_free_request_body(&s_rb_dst);

    memset(&s_rb_src, 0, sizeof(s_rb_src));
    memset(&s_rb_dst, 0, sizeof(s_rb_dst));
    s_rb_src.description = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_copy_request_body_fields(&s_rb_dst, &s_rb_src));
    cdd_test_free_request_body(&s_rb_dst);
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_COPY_FREE_H */
