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

  rc = cdd_test_copy_parameter_fields(&p_dst, &p_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("param1", p_dst.name);
  ASSERT_STR_EQ("param desc", p_dst.description);
  ASSERT_STR_EQ("application/json", p_dst.content_type);
  ASSERT_STR_EQ("#/components/mediaTypes/Json", p_dst.content_ref);
  ASSERT_STR_EQ("string", p_dst.items_type);
  ASSERT_EQ(1, p_dst.example_set);
  ASSERT_STR_EQ("ex_val", p_dst.example.string);

  cdd_test_free_parameter(&p_src);
  cdd_test_free_parameter(&p_dst);
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

  rc = cdd_test_copy_header_fields(&h_dst, &h_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("hdr desc", h_dst.description);
  ASSERT_STR_EQ("text/plain", h_dst.content_type);
  ASSERT_STR_EQ("#/components/mediaTypes/Text", h_dst.content_ref);
  ASSERT_STR_EQ("integer", h_dst.items_type);
  ASSERT_EQ(1, h_dst.example_set);

  cdd_test_free_header(&h_src);
  cdd_test_free_header(&h_dst);
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

  rc = cdd_test_copy_encoding_fields(&enc_dst, &enc_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("enc1", enc_dst.name);
  ASSERT_STR_EQ("image/png", enc_dst.content_type);

  cdd_test_free_encoding(&enc_src);
  cdd_test_free_encoding(&enc_dst);
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

  rc = cdd_test_copy_media_type_fields(&mt_dst, &mt_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("application/json", mt_dst.name);
  ASSERT_STR_EQ("#/components/mediaTypes/Json", mt_dst.ref);
  ASSERT_EQ(1, mt_dst.example_set);

  cdd_test_free_media_type(&mt_src);
  cdd_test_free_media_type(&mt_dst);
  cdd_test_free_media_type(NULL);

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

  rc = cdd_test_copy_response_fields(&r_dst, &r_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("OK summary", r_dst.summary);
  ASSERT_STR_EQ("OK response", r_dst.description);
  ASSERT_EQ(1, r_dst.example_set);

  cdd_test_free_response(&r_src);
  cdd_test_free_response(&r_dst);
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

  rc = cdd_test_copy_operation_fields(&op_dst, &op_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("op1", op_dst.operation_id);
  ASSERT_STR_EQ("op summary", op_dst.summary);
  ASSERT_STR_EQ("op description", op_dst.description);
  ASSERT_STR_EQ("#/components/requestBodies/Body1", op_dst.req_body_ref);
  ASSERT_STR_EQ("{\"x-rb\":1}", op_dst.req_body_extensions_json);

  cdd_test_free_operation(&op_src);
  cdd_test_free_operation(&op_dst);
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

  rc = cdd_test_copy_schema_ref(&sr_dst, &sr_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("string", sr_dst.inline_type);
  ASSERT_STR_EQ("date-time", sr_dst.format);
  ASSERT_STR_EQ("sr desc", sr_dst.description);
  ASSERT_STR_EQ("sr sum", sr_dst.summary);
  ASSERT_STR_EQ("#/components/schemas/MyString", sr_dst.ref);

  cdd_test_free_schema_ref_content(&sr_src);
  cdd_test_free_schema_ref_content(&sr_dst);

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

  rc = cdd_test_copy_path_fields(&path_dst, &path_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("/test", path_dst.route);
  ASSERT_STR_EQ("#/components/pathItems/MyPath", path_dst.ref);
  ASSERT_STR_EQ("path summary", path_dst.summary);
  ASSERT_STR_EQ("path description", path_dst.description);

  cdd_test_free_path_item(&path_src);
  cdd_test_free_path_item(&path_dst);

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

  cdd_test_free_callback(&cb_src);
  cdd_test_free_callback(&cb_dst);

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

  rc = cdd_test_copy_request_body_fields(&rb_dst, &rb_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("#/components/requestBodies/MyRb", rb_dst.ref);
  ASSERT_STR_EQ("rb description", rb_dst.description);

  cdd_test_free_request_body(&rb_src);
  cdd_test_free_request_body(&rb_dst);

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
      srv_src->n_variables = 1;
    }

    rc = cdd_test_copy_server_object(srv_dst, srv_src);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("https://example.com", srv_dst->url);
    ASSERT_STR_EQ("prod server", srv_dst->description);
  }

  openapi_free_servers_array(srv_src, 1);
  openapi_free_servers_array(srv_dst, 1);
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

  cdd_test_free_link(&lnk_src);
  cdd_test_free_link(&lnk_dst);

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

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_COPY_FREE_H */
