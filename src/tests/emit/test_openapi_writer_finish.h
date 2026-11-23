/**
 * @file test_openapi_writer_finish.h
 * @brief Unit tests for OpenAPI Writer (finish).
 */

#ifndef TEST_OPENAPI_WRITER_FINISH_H
#define TEST_OPENAPI_WRITER_FINISH_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

/**
 * @brief Unit test test_openapi_writer_close_100_percent.
 * @return Greatest test result.
 */
TEST test_openapi_writer_close_100_percent(void) {
  JSON_Value *val;
  JSON_Object *obj;
  JSON_Value *parent_val;
  JSON_Object *parent;
  struct OpenAPI_Any any_vals[2];
  struct OpenAPI_Example ex;
  struct OpenAPI_Example ex_arr[1];
  struct OpenAPI_ExternalDocs ed;
  struct OpenAPI_Discriminator disc;
  struct OpenAPI_DiscriminatorMap dmap[1];
  struct OpenAPI_Xml xml;
  struct OpenAPI_Server srv[1];
  struct OpenAPI_ServerVariable svar;
  struct OpenAPI_SchemaRef sr;
  struct OpenAPI_SchemaRef sub_sr[1];
  struct OpenAPI_Parameter p;
  struct OpenAPI_Header h;
  struct OpenAPI_Header h_arr[1];
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MediaType mt_dummy;
  struct OpenAPI_Link lk;
  struct OpenAPI_LinkParam lp[2];
  struct OpenAPI_Response r_dummy;
  struct OpenAPI_RequestBody rb_d;
  struct OpenAPI_Operation op;
  struct OpenAPI_Callback cb_d;
  struct OpenAPI_Path path;
  struct OpenAPI_SecurityRequirementSet sec_set;
  struct OpenAPI_SecurityScheme ss;
  struct OpenAPI_OAuthFlow fl;
  struct OpenAPI_OAuthScope scp[1];
  struct OpenAPI_Spec sp_d;
  struct StructFields sf[1];
  char *dn[1];
  char *tu[1];
  char *json = NULL;
  int k;

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
    parent_val = _cdd_jv_1;
  }
  parent = json_value_get_object(parent_val);
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
    val = _cdd_jv_1;
  }
  obj = json_value_get_object(val);

  /* 1. L367: duplicate key in merge_schema_extras_object_openapi */
  json_object_set_string(obj, "k", "orig");
  ASSERT_EQ(CDD_C_SUCCESS, merge_schema_extras_object_openapi(
                               obj, "{\"k\": \"dup\", \"k2\": \"new\"}"));

  /* 2. L422: write_schema_type with type == NULL */
  g_cdd_last_alloc_err = write_schema_type(obj, NULL, 0);

  /* 3. L463: write_schema_type_union with type_union != NULL && n_type_union ==
   * 0 */
  tu[0] = cdd_test_mut_str("string");
  g_cdd_last_alloc_err = write_schema_type_union(obj, "string", 0, tu, 0);

  /* 4. L490, L503: write_enum_any_values variations */
  memset(any_vals, 0, sizeof(any_vals));
  any_vals[0].type = (enum OpenAPI_AnyType)999;
  g_cdd_last_alloc_err = write_enum_any_values(obj, "k", NULL, 1);
  g_cdd_last_alloc_err = write_enum_any_values(obj, "k", any_vals, 0);
  g_cdd_last_alloc_err = write_enum_any_values(obj, NULL, any_vals, 1);
  g_cdd_last_alloc_err = write_enum_any_values(NULL, "k", any_vals, 1);
  g_cdd_last_alloc_err = write_enum_any_values(obj, "bad_enum", any_vals, 1);
  any_vals[0].type = OA_ANY_STRING;
  any_vals[0].string = cdd_test_mut_str("good");
  g_cdd_last_alloc_err = write_enum_any_values(obj, "good_enum", any_vals, 1);
  any_vals[0].type = OA_ANY_JSON;
  any_vals[0].json = cdd_test_mut_str("{invalid_json");
  g_cdd_last_alloc_err =
      write_enum_any_values(obj, "raw_json_enum", any_vals, 1);

  /* 5. L579, L581, L594, L599: write_example_object branches */
  memset(&ex, 0, sizeof(ex));
  ex.ref = cdd_test_mut_str("#/ref");
  ex.summary = NULL;
  ex.description = NULL;
  g_cdd_last_alloc_err = write_example_object(obj, &ex);

  memset(&ex, 0, sizeof(ex));
  ex.data_value_set = 1;
  ex.data_value.type = (enum OpenAPI_AnyType)999;
  g_cdd_last_alloc_err = write_example_object(obj, &ex);

  memset(&ex, 0, sizeof(ex));
  ex.value_set = 1;
  ex.value.type = (enum OpenAPI_AnyType)999;
  g_cdd_last_alloc_err = write_example_object(obj, &ex);

  /* 6. L621: write_examples_object null/empty checks */
  memset(ex_arr, 0, sizeof(ex_arr));
  ex_arr[0].name = cdd_test_mut_str("ex1");
  ASSERT_EQ(CDD_C_SUCCESS, write_examples_object(parent, "k", NULL, 1));
  ASSERT_EQ(CDD_C_SUCCESS, write_examples_object(parent, "k", ex_arr, 0));
  ASSERT_EQ(CDD_C_SUCCESS, write_examples_object(parent, NULL, ex_arr, 1));
  ASSERT_EQ(CDD_C_SUCCESS, write_examples_object(NULL, "k", ex_arr, 1));
  ASSERT_EQ(CDD_C_SUCCESS,
            write_examples_object(parent, "valid_ex", ex_arr, 1));

  /* 7. L658, L665, L668: write_example_fields branches */
  g_cdd_last_alloc_err = write_example_fields(parent, NULL, 0, ex_arr, 0);
  g_cdd_last_alloc_err = write_example_fields(parent, NULL, 1, NULL, 0);
  memset(&any_vals[0], 0, sizeof(any_vals[0]));
  any_vals[0].type = (enum OpenAPI_AnyType)999;
  g_cdd_last_alloc_err = write_example_fields(parent, &any_vals[0], 1, NULL, 0);

  /* 8. L681: write_external_docs with docs->url == NULL and docs == NULL */
  memset(&ed, 0, sizeof(ed));
  ed.url = NULL;
  g_cdd_last_alloc_err = write_external_docs(parent, "ext", &ed);
  g_cdd_last_alloc_err = write_external_docs(parent, "ext", NULL);

  /* 9. L711, L719, L721, L724, L729: write_discriminator_object branches */
  memset(&disc, 0, sizeof(disc));
  disc.property_name = NULL;
  disc.n_mapping = 0;
  disc.default_mapping = NULL;
  g_cdd_last_alloc_err = write_discriminator_object(parent, &disc, 1);

  memset(&disc, 0, sizeof(disc));
  disc.property_name = cdd_test_mut_str("type");
  disc.n_mapping = 0;
  disc.default_mapping = NULL;
  g_cdd_last_alloc_err = write_discriminator_object(parent, &disc, 1);

  memset(&disc, 0, sizeof(disc));
  memset(dmap, 0, sizeof(dmap));
  dmap[0].value = cdd_test_mut_str("val");
  dmap[0].schema = cdd_test_mut_str("sch");
  disc.property_name = NULL;
  disc.mapping = dmap;
  disc.n_mapping = 1;
  disc.default_mapping = NULL;
  g_cdd_last_alloc_err = write_discriminator_object(parent, &disc, 1);

  memset(&disc, 0, sizeof(disc));
  disc.property_name = NULL;
  disc.default_mapping = cdd_test_mut_str("def");
  g_cdd_last_alloc_err = write_discriminator_object(parent, &disc, 1);

  memset(&disc, 0, sizeof(disc));
  disc.property_name = cdd_test_mut_str("type");
  disc.mapping = NULL;
  disc.n_mapping = 1;
  g_cdd_last_alloc_err = write_discriminator_object(parent, &disc, 1);

  disc.mapping = dmap;
  disc.n_mapping = 0;
  g_cdd_last_alloc_err = write_discriminator_object(parent, &disc, 1);

  dmap[0].value = cdd_test_mut_str("val");
  dmap[0].schema = NULL;
  disc.mapping = dmap;
  disc.n_mapping = 1;
  g_cdd_last_alloc_err = write_discriminator_object(parent, &disc, 1);

  /* 10. L762: write_xml_object with invalid node_type */
  memset(&xml, 0, sizeof(xml));
  xml.node_type_set = 1;
  xml.node_type = (enum OpenAPI_XmlNodeType)999;
  g_cdd_last_alloc_err = write_xml_object(parent, &xml, 1);

  /* 11. L856, L868: write_server_object branches */
  memset(srv, 0, sizeof(srv));
  srv[0].n_variables = 1;
  srv[0].variables = NULL;
  g_cdd_last_alloc_err = write_server_object(obj, &srv[0]);

  memset(&svar, 0, sizeof(svar));
  svar.n_enum_values = 1;
  svar.enum_values = NULL;
  srv[0].variables = &svar;
  srv[0].n_variables = 1;
  g_cdd_last_alloc_err = write_server_object(obj, &srv[0]);

  /* 12. L937: write_schema_example with invalid any */
  memset(&sr, 0, sizeof(sr));
  sr.example_set = 1;
  sr.example.type = (enum OpenAPI_AnyType)999;
  g_cdd_last_alloc_err = write_schema_ref(parent, "sch_ex", &sr);

  /* 13. L1009, L1044, L1051: write_items_schema_fields branches */
  g_cdd_last_alloc_err = write_items_schema_fields(obj, NULL);
  memset(&sr, 0, sizeof(sr));
  sr.is_array = 1;
  sr.inline_type = cdd_test_mut_str("string");
  sr.items_const_value_set = 1;
  sr.items_const_value.type = (enum OpenAPI_AnyType)999;
  g_cdd_last_alloc_err = write_schema_ref(parent, "items_const", &sr);

  memset(&sr, 0, sizeof(sr));
  sr.is_array = 1;
  sr.inline_type = cdd_test_mut_str("string");
  sr.items_default_value_set = 1;
  sr.items_default_value.type = (enum OpenAPI_AnyType)999;
  g_cdd_last_alloc_err = write_schema_ref(parent, "items_def", &sr);

  /* 14. L1085, L1127, L1207, L1224, L1241, L1263, L1282, L1286, L1310 */
  g_cdd_last_alloc_err = write_schema_ref(parent, NULL, &sr);
  g_cdd_last_alloc_err = write_schema_ref(parent, "sch", NULL);

  memset(&sr, 0, sizeof(sr));
  sr.is_array = 1;
  g_cdd_last_alloc_err = write_schema_ref(parent, "arr_empty", &sr);

  memset(&sr, 0, sizeof(sr));
  sr.summary = cdd_test_mut_str("sum");
  sr.ref_name = NULL;
  sr.ref = NULL;
  g_cdd_last_alloc_err = write_schema_ref(parent, "sch_sum_no_ref", &sr);

  memset(&sr, 0, sizeof(sr));
  sr.summary = cdd_test_mut_str("sum");
  sr.ref_name = NULL;
  sr.ref = cdd_test_mut_str("#/ref");
  g_cdd_last_alloc_err = write_schema_ref(parent, "sch_sum_ref", &sr);

  memset(&sr, 0, sizeof(sr));
  sr.const_value_set = 1;
  sr.const_value.type = (enum OpenAPI_AnyType)999;
  g_cdd_last_alloc_err = write_schema_ref(parent, "sch_const", &sr);

  memset(&sr, 0, sizeof(sr));
  sr.n_examples = 1;
  sr.examples = NULL;
  g_cdd_last_alloc_err = write_schema_ref(parent, "sch_ex_null", &sr);

  memset(&sr, 0, sizeof(sr));
  sr.default_value_set = 1;
  sr.default_value.type = (enum OpenAPI_AnyType)999;
  g_cdd_last_alloc_err = write_schema_ref(parent, "sch_def", &sr);

  memset(sub_sr, 0, sizeof(sub_sr));
  sub_sr[0].ref_name = cdd_test_mut_str("Sub");
  memset(&sr, 0, sizeof(sr));
  sr.all_of = sub_sr;
  sr.n_all_of = 1;
  sr.any_of = sub_sr;
  sr.n_any_of = 1;
  sr.one_of = sub_sr;
  sr.n_one_of = 1;
  for (k = 0; k < 25; ++k) {
    JSON_Value *oom_val = NULL;
    JSON_Object *oom_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      oom_val = _cdd_jv_1;
    }
    oom_obj = json_value_get_object(oom_val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    g_cdd_last_alloc_err = write_schema_ref(oom_obj, "sch_composed", &sr);
    json_set_allocation_functions(malloc, free);
    g_cdd_last_alloc_err = json_value_free(oom_val);
  }

  memset(&sr, 0, sizeof(sr));
  sr.any_of = sub_sr;
  sr.n_any_of = 1;
  for (k = 0; k < 25; ++k) {
    JSON_Value *oom_val = NULL;
    JSON_Object *oom_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      oom_val = _cdd_jv_1;
    }
    oom_obj = json_value_get_object(oom_val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    g_cdd_last_alloc_err = write_schema_ref(oom_obj, "sch_any", &sr);
    json_set_allocation_functions(malloc, free);
    g_cdd_last_alloc_err = json_value_free(oom_val);
  }

  memset(&sr, 0, sizeof(sr));
  sr.one_of = sub_sr;
  sr.n_one_of = 1;
  for (k = 0; k < 25; ++k) {
    JSON_Value *oom_val = NULL;
    JSON_Object *oom_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      oom_val = _cdd_jv_1;
    }
    oom_obj = json_value_get_object(oom_val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    g_cdd_last_alloc_err = write_schema_ref(oom_obj, "sch_one", &sr);
    json_set_allocation_functions(malloc, free);
    g_cdd_last_alloc_err = json_value_free(oom_val);
  }

  /* 15. L1398, L1417, L1428, L1453, L1455, L1458: write_parameter_object */
  memset(&p, 0, sizeof(p));
  p.in = (enum OpenAPI_ParamIn)999;
  p.style = (enum OpenAPI_Style)999;
  g_cdd_last_alloc_err = write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.content_media_types = (struct OpenAPI_MediaType *)(size_t)0x1;
  p.n_content_media_types = 0;
  g_cdd_last_alloc_err = write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.content_type = cdd_test_mut_str("application/json");
  p.schema_set = 1;
  g_cdd_last_alloc_err = write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.content_type = cdd_test_mut_str("application/json");
  p.schema_set = 0;
  p.type = NULL;
  p.is_array = 0;
  g_cdd_last_alloc_err = write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.content_type = cdd_test_mut_str("application/json");
  p.schema_set = 0;
  p.type = NULL;
  p.is_array = 1;
  g_cdd_last_alloc_err = write_parameter_object(obj, &p);

  /* 16. L1508, L1518, L1529, L1538, L1540, L1552: write_header_object */
  memset(&h, 0, sizeof(h));
  h.style_set = 1;
  h.style = (enum OpenAPI_Style)999;
  g_cdd_last_alloc_err = write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.content_media_types = (struct OpenAPI_MediaType *)(size_t)0x1;
  h.n_content_media_types = 0;
  g_cdd_last_alloc_err = write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.content_ref = cdd_test_mut_str("#/ref");
  h.content_type = cdd_test_mut_str("application/json");
  g_cdd_last_alloc_err = write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.content_type = cdd_test_mut_str("application/json");
  h.schema_set = 1;
  g_cdd_last_alloc_err = write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.content_type = cdd_test_mut_str("application/json");
  h.schema_set = 0;
  h.type = NULL;
  h.is_array = 0;
  g_cdd_last_alloc_err = write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.content_type = NULL;
  h.schema_set = 1;
  g_cdd_last_alloc_err = write_header_object(obj, &h);

  /* 17. L1578, L1586, L1590, L1594, L1599, L1629, L1670: write_encoding_* */
  memset(&enc, 0, sizeof(enc));
  enc.style_set = 1;
  enc.style = (enum OpenAPI_Style)999;
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_object(obj, &enc));

  memset(&enc, 0, sizeof(enc));
  enc.headers = (struct OpenAPI_Header *)(size_t)0x1;
  enc.n_headers = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_object(obj, &enc));

  memset(&enc, 0, sizeof(enc));
  enc.encoding = (struct OpenAPI_Encoding *)(size_t)0x1;
  enc.n_encoding = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_object(obj, &enc));

  memset(&enc, 0, sizeof(enc));
  enc.prefix_encoding = (struct OpenAPI_Encoding *)(size_t)0x1;
  enc.n_prefix_encoding = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_object(obj, &enc));

  memset(&enc, 0, sizeof(enc));
  enc.item_encoding = &enc;
  enc.item_encoding_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_object(obj, &enc));

  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_map(obj, NULL, 1));
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_map(obj, &enc, 0));
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_array(parent, NULL, &enc, 1));
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_array(parent, "k", NULL, 1));
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_array(parent, "k", &enc, 0));

  /* 18. L1709, L1712, L1717, L1721, L1726, L1754: write_media_type_* */
  memset(&mt_dummy, 0, sizeof(mt_dummy));
  mt_dummy.encoding = &enc;
  mt_dummy.n_encoding = 0;
  mt_dummy.prefix_encoding = &enc;
  mt_dummy.n_prefix_encoding = 0;
  mt_dummy.item_encoding = &enc;
  mt_dummy.item_encoding_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_media_type_object(obj, &mt_dummy));
  ASSERT_EQ(CDD_C_SUCCESS, write_media_type_map(parent, NULL, &mt_dummy, 1));
  ASSERT_EQ(CDD_C_SUCCESS, write_media_type_map(parent, "k", NULL, 1));
  ASSERT_EQ(CDD_C_SUCCESS, write_media_type_map(parent, "k", &mt_dummy, 0));

  memset(&mt_dummy, 0, sizeof(mt_dummy));
  mt_dummy.schema_set = 0;
  mt_dummy.schema.ref_name = cdd_test_mut_str("Model");
  ASSERT_EQ(CDD_C_SUCCESS, write_media_type_object(obj, &mt_dummy));

  memset(&mt_dummy, 0, sizeof(mt_dummy));
  mt_dummy.item_schema_set = 0;
  mt_dummy.item_schema.ref_name = cdd_test_mut_str("ItemModel");
  ASSERT_EQ(CDD_C_SUCCESS, write_media_type_object(obj, &mt_dummy));

  /* 19. L1808, L1817, L1819, L1830, L1834, L1856, L1894, L1909, L1955, L1958,
   * L1962 */
  memset(&lk, 0, sizeof(lk));
  lk.n_parameters = 1;
  lk.parameters = NULL;
  g_cdd_last_alloc_err = write_link_object(obj, &lk);

  memset(&lk, 0, sizeof(lk));
  memset(lp, 0, sizeof(lp));
  lp[0].value.type = OA_ANY_STRING;
  lp[0].value.string = cdd_test_mut_str("val");
  lp[0].name = NULL;
  lp[1].value.type = (enum OpenAPI_AnyType)999;
  lp[1].name = cdd_test_mut_str("p");
  lk.parameters = lp;
  lk.n_parameters = 2;
  lk.request_body_set = 1;
  lk.request_body.type = (enum OpenAPI_AnyType)999;
  lk.server_set = 1;
  lk.server = NULL;
  g_cdd_last_alloc_err = write_link_object(obj, &lk);

  memset(h_arr, 0, sizeof(h_arr));
  h_arr[0].name = cdd_test_mut_str("X-H");
  ASSERT_EQ(CDD_C_SUCCESS, write_headers_map(parent, NULL, h_arr, 1, 0));
  ASSERT_EQ(CDD_C_SUCCESS, write_headers_map(parent, "k", NULL, 1, 0));
  ASSERT_EQ(CDD_C_SUCCESS, write_headers_map(parent, "k", h_arr, 0, 0));

  memset(&r_dummy, 0, sizeof(r_dummy));
  r_dummy.n_headers = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_headers(parent, &r_dummy));
  r_dummy.n_headers = 1;
  r_dummy.headers = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, write_headers(parent, &r_dummy));
  ASSERT_EQ(CDD_C_SUCCESS, write_headers(obj, NULL));

  ASSERT_EQ(CDD_C_SUCCESS, write_links(NULL, &r_dummy));
  r_dummy.n_links = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_links(parent, &r_dummy));
  r_dummy.n_links = 1;
  r_dummy.links = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, write_links(parent, &r_dummy));
  ASSERT_EQ(CDD_C_SUCCESS, write_links(obj, NULL));

  memset(&r_dummy, 0, sizeof(r_dummy));
  r_dummy.headers = h_arr;
  r_dummy.n_headers = 0;
  r_dummy.links = &lk;
  r_dummy.n_links = 0;
  r_dummy.content_media_types = &mt_dummy;
  r_dummy.n_content_media_types = 0;
  g_cdd_last_alloc_err = write_response_object(obj, &r_dummy);

  /* 20. L2009, L2054, L2076, L2127, L2148: parameters & request_body */
  ASSERT_EQ(CDD_C_SUCCESS, write_parameters(NULL, &p, 1));
  ASSERT_EQ(CDD_C_SUCCESS, write_parameters(parent, &p, 0));

  memset(&rb_d, 0, sizeof(rb_d));
  rb_d.content_media_types = &mt_dummy;
  rb_d.n_content_media_types = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_request_body_object(obj, &rb_d));

  memset(&rb_d, 0, sizeof(rb_d));
  rb_d.content_ref = cdd_test_mut_str("#/ref");
  rb_d.schema.content_type = cdd_test_mut_str("application/json");
  ASSERT_EQ(CDD_C_SUCCESS, write_request_body_object(obj, &rb_d));

  memset(&op, 0, sizeof(op));
  ASSERT_EQ(CDD_C_SUCCESS, write_request_body(NULL, &op));

  memset(&op, 0, sizeof(op));
  op.req_body.ref_name = cdd_test_mut_str("Body");
  op.req_body.content_type = NULL;
  op.n_req_body_media_types = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_request_body(obj, &op));

  memset(&op, 0, sizeof(op));
  op.req_body.content_type = cdd_test_mut_str("application/json");
  ASSERT_EQ(CDD_C_SUCCESS, write_request_body(obj, &op));

  /* 21. L2191, L2193, L2198, L2228, L2240, L2261: callbacks & responses */
  memset(&cb_d, 0, sizeof(cb_d));
  cb_d.ref = cdd_test_mut_str("#/ref");
  cb_d.summary = NULL;
  cb_d.description = NULL;
  g_cdd_last_alloc_err = write_callback_object(obj, &cb_d);

  memset(&cb_d, 0, sizeof(cb_d));
  memset(&path, 0, sizeof(path));
  cb_d.paths = &path;
  cb_d.n_paths = 0;
  g_cdd_last_alloc_err = write_callback_object(obj, &cb_d);

  memset(&op, 0, sizeof(op));
  ASSERT_EQ(CDD_C_SUCCESS, write_callbacks(NULL, &op));
  ASSERT_EQ(CDD_C_SUCCESS, write_callbacks(obj, NULL));
  op.n_callbacks = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_callbacks(obj, &op));
  op.n_callbacks = 1;
  op.callbacks = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, write_callbacks(obj, &op));

  cb_d.name = NULL;
  op.callbacks = &cb_d;
  op.n_callbacks = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_callbacks(obj, &op));

  ASSERT_EQ(CDD_C_SUCCESS, write_responses(obj, NULL));

  /* 22. L2354, L2411, L2472, L2514: operations & paths */
  memset(&op, 0, sizeof(op));
  op.n_servers = 1;
  op.servers = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, write_operation_object(obj, &op));

  ASSERT_EQ(CDD_C_SUCCESS, write_additional_operations(NULL, &path));
  ASSERT_EQ(CDD_C_SUCCESS, write_additional_operations(obj, NULL));
  memset(&path, 0, sizeof(path));
  path.n_additional_operations = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_additional_operations(obj, &path));
  path.n_additional_operations = 1;
  path.additional_operations = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, write_additional_operations(obj, &path));

  memset(&path, 0, sizeof(path));
  path.n_servers = 1;
  path.servers = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, write_path_item_object(obj, &path));

  memset(&sp_d, 0, sizeof(sp_d));
  memset(&path, 0, sizeof(path));
  path.route = NULL;
  sp_d.paths = &path;
  sp_d.n_paths = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_paths(obj, &sp_d));

  /* 23. L2560, L2598, L2695: server array, tags, security requirements */
  ASSERT_EQ(CDD_C_SUCCESS, write_server_array(NULL, "k", srv, 1));
  ASSERT_EQ(CDD_C_SUCCESS, write_server_array(parent, NULL, srv, 1));
  ASSERT_EQ(CDD_C_SUCCESS, write_tags(obj, NULL));
  memset(&sec_set, 0, sizeof(sec_set));
  ASSERT_EQ(CDD_C_SUCCESS,
            write_security_requirements(NULL, "k", &sec_set, 1, 1));
  ASSERT_EQ(CDD_C_SUCCESS,
            write_security_requirements(parent, NULL, &sec_set, 1, 1));

  /* 24. L2799, L2818, L2853, L2869: security schemes */
  memset(&sp_d, 0, sizeof(sp_d));
  memset(&ss, 0, sizeof(ss));
  ss.type = OA_SEC_HTTP;
  ss.scheme = cdd_test_mut_str("basic");
  sp_d.security_schemes = &ss;
  sp_d.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_security_schemes(obj, &sp_d));

  memset(&ss, 0, sizeof(ss));
  ss.type = OA_SEC_OAUTH2;
  ss.flows = (struct OpenAPI_OAuthFlow *)(size_t)0x1;
  ss.n_flows = 0;
  sp_d.security_schemes = &ss;
  sp_d.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_security_schemes(obj, &sp_d));

  memset(&ss, 0, sizeof(ss));
  memset(&fl, 0, sizeof(fl));
  memset(scp, 0, sizeof(scp));
  scp[0].name = NULL;
  fl.scopes = scp;
  fl.n_scopes = 1;
  fl.type = OA_OAUTH_FLOW_IMPLICIT;
  ss.type = OA_SEC_OAUTH2;
  ss.flows = &fl;
  ss.n_flows = 1;
  sp_d.security_schemes = &ss;
  sp_d.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_security_schemes(obj, &sp_d));

  memset(&ss, 0, sizeof(ss));
  ss.type = OA_SEC_OPENID;
  ss.open_id_connect_url = NULL;
  sp_d.security_schemes = &ss;
  sp_d.n_security_schemes = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_security_schemes(obj, &sp_d));

  /* 25. L2898, L2934, L2970, L3006, L3047, L3083, L3116, L3150: component_* */
  ASSERT_EQ(CDD_C_SUCCESS, write_component_parameters(obj, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, write_component_responses(obj, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, write_component_headers(obj, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, write_component_media_types(obj, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, write_component_examples(obj, NULL));
  memset(&sp_d, 0, sizeof(sp_d));
  sp_d.n_component_examples = 1;
  sp_d.component_examples = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, write_component_examples(obj, &sp_d));

  ASSERT_EQ(CDD_C_SUCCESS, write_component_links(obj, NULL));
  memset(&sp_d, 0, sizeof(sp_d));
  sp_d.n_component_links = 1;
  sp_d.component_links = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, write_component_links(obj, &sp_d));

  ASSERT_EQ(CDD_C_SUCCESS, write_component_callbacks(obj, NULL));
  memset(&sp_d, 0, sizeof(sp_d));
  sp_d.n_component_callbacks = 1;
  sp_d.component_callbacks = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, write_component_callbacks(obj, &sp_d));

  ASSERT_EQ(CDD_C_SUCCESS, write_component_path_items(obj, NULL));

  /* 26. L3269: write_components with NULL name */
  memset(&sp_d, 0, sizeof(sp_d));
  dn[0] = NULL;
  memset(sf, 0, sizeof(sf));
  sp_d.defined_schemas = sf;
  sp_d.defined_schema_names = dn;
  sp_d.n_defined_schemas = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_components(obj, &sp_d));

  /* 27. L3448: openapi_write_spec_to_json with n_paths == 0 and
   * paths_extensions_json */
  memset(&sp_d, 0, sizeof(sp_d));
  sp_d.n_paths = 0;
  sp_d.paths_extensions_json = cdd_test_mut_str("{\"x-paths\": 1}");
  ASSERT_EQ(CDD_C_SUCCESS, openapi_write_spec_to_json(&sp_d, &json));
  ASSERT(json != NULL);
  free(json);
  json = NULL;

  g_cdd_last_alloc_err = json_value_free(val);
  g_cdd_last_alloc_err = json_value_free(parent_val);
  PASS();
}

/**
 * @brief Sweeps Parson OOM and step-fail injection on an emitter call taking a
 * fresh `JSON_Object *`.
 * @param[in] obj_var Identifier bound to the fresh `JSON_Object *`.
 * @param[in] call_expr Expression using `obj_var` returning `cdd_c_error_t`.
 */
#define SWEEP_OA_OBJ(obj_var, call_expr)                                       \
  do {                                                                         \
    int _k;                                                                    \
    for (_k = 0; _k < 500; ++_k) {                                             \
      JSON_Value *_tv = NULL;                                                  \
      JSON_Object *obj_var = NULL;                                             \
      int _rem;                                                                \
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&_tv));              \
      obj_var = json_value_get_object(_tv);                                    \
      json_set_allocation_functions(mock_parson_oom_malloc,                    \
                                    mock_parson_oom_free);                     \
      g_parson_oom_fail_at = _k;                                               \
      g_cdd_last_alloc_err = (call_expr);                                      \
      _rem = g_parson_oom_fail_at;                                             \
      g_parson_oom_fail_at = -1;                                               \
      json_set_allocation_functions(malloc, free);                             \
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(_tv));                      \
      if (_rem >= 0)                                                           \
        break;                                                                 \
    }                                                                          \
    for (_k = 1; _k < 500; ++_k) {                                             \
      JSON_Value *_tv = NULL;                                                  \
      JSON_Object *obj_var = NULL;                                             \
      int _rem;                                                                \
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&_tv));              \
      obj_var = json_value_get_object(_tv);                                    \
      g_cdd_step_fail = _k;                                                    \
      g_cdd_last_alloc_err = (call_expr);                                      \
      _rem = g_cdd_step_fail;                                                  \
      g_cdd_step_fail = 0;                                                     \
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(_tv));                      \
      if (_rem > 0)                                                            \
        break;                                                                 \
    }                                                                          \
  } while (0)

/**
 * @brief Unit test test_openapi_writer_percolation_exhaustive.
 * @return Greatest test result.
 */
TEST test_openapi_writer_percolation_exhaustive(void) {
  const char *out_s = NULL;
  int out_b = 0;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  JSON_Array *ja = NULL;
  char *tu[2];
  struct OpenAPI_Any any_v;
  struct OpenAPI_Any any_arr[2];
  struct OpenAPI_Example ex_arr[1];
  struct OpenAPI_Xml xml;
  struct OpenAPI_ExternalDocs ext_docs;
  struct OpenAPI_Discriminator disc;
  struct OpenAPI_DiscriminatorMap dmap[1];
  struct OpenAPI_MultipartField mp[2];
  struct OpenAPI_SchemaRef sub_ref;
  struct OpenAPI_SchemaRef ref;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Header hdr;
  struct OpenAPI_Encoding sub_enc;
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Link link;
  struct OpenAPI_LinkParam lparam[1];
  struct OpenAPI_Server srv;
  struct OpenAPI_ServerVariable svar[1];
  char *svar_enums[1];
  struct OpenAPI_RequestBody rb;
  struct OpenAPI_Response resp;
  struct OpenAPI_Callback cb;
  struct OpenAPI_Path cb_path;
  struct OpenAPI_Path main_path;
  struct OpenAPI_Operation op;
  char *op_tags[1];
  struct OpenAPI_SecurityRequirement sreq[1];
  char *sscopes[1];
  struct OpenAPI_SecurityRequirementSet sset[1];
  struct OpenAPI_Tag tag;
  struct OpenAPI_Info info;
  struct OpenAPI_OAuthScope oscope[2];
  struct OpenAPI_OAuthFlow oflow[1];
  struct OpenAPI_SecurityScheme sschemes[5];
  struct StructFields def_sf[1];
  struct StructField def_flds[1];
  char *def_names[1];
  char *def_ids[1];
  char *def_anchors[1];
  char *def_dyn_anchors[1];
  char *raw_names[1];
  char *raw_jsons[1];
  char *comp_names[1];
  struct OpenAPI_Spec spec;
  char *spec_json = NULL;
  int k;

  /* 1. Direct step-fail checks on json_dom and OpenAPI emitter helpers */
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_json_value_get_array(NULL, &ja));
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_json_array_append_value(NULL, NULL));
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, verb_to_str_openapi(OA_VERB_GET, &out_s));
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            param_in_to_str_openapi(OA_PARAM_IN_QUERY, &out_s));
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, style_to_str_openapi(OA_STYLE_FORM, &out_s));
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            oauth_flow_type_to_str_openapi(OA_OAUTH_FLOW_IMPLICIT, &out_s));
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, is_schema_primitive_openapi("string", &out_b));
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            server_url_has_query_or_fragment("https://a", &out_b));
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, schema_ref_keyword(0, &out_s));
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            header_name_is_content_type_openapi("Content-Type", &out_b));
  g_cdd_step_fail = 0;
  {
    extern C_CDD_EXPORT int g_cdd_fail_str_iequal;
    g_cdd_fail_str_iequal = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              header_name_is_content_type_openapi("X-Test", &out_b));
    memset(&param, 0, sizeof(param));
    param.in = OA_PARAM_IN_HEADER;
    param.name = cdd_test_mut_str("X-Test");
    g_cdd_step_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              param_is_reserved_header_openapi(&param, &out_b));
    g_cdd_step_fail = 0;
    for (k = 1; k <= 3; ++k) {
      g_cdd_fail_str_iequal = k;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                param_is_reserved_header_openapi(&param, &out_b));
    }
    g_cdd_fail_str_iequal = 0;
  }
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, schema_ref_has_data(NULL, &out_b));
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, type_union_contains(NULL, 0, "a", &out_b));
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, any_to_json_value(NULL, &jv));
  g_cdd_step_fail = 0;

  /* 2. any_to_json_value OOM on OA_ANY_JSON */
  memset(&any_v, 0, sizeof(any_v));
  any_v.type = OA_ANY_JSON;
  any_v.json = cdd_test_mut_str("{\"x\": 1}");
  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  g_parson_oom_fail_at = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, any_to_json_value(&any_v, &jv));
  g_parson_oom_fail_at = -1;
  json_set_allocation_functions(malloc, free);

  /* 3. Defensive NULL / empty guard checks */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&jv));
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_array(jv, &ja));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_array_append_value(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, write_component_request_bodies(jo, NULL));
  memset(&srv, 0, sizeof(srv));
  srv.url = cdd_test_mut_str("https://api.example.com");
  ASSERT_EQ(CDD_C_SUCCESS, write_server_array(jo, NULL, &srv, 1));
  ASSERT_EQ(CDD_C_SUCCESS, write_server_array(jo, "servers", NULL, 1));
  ASSERT_EQ(CDD_C_SUCCESS, write_server_array(jo, "servers", &srv, 0));
  memset(&spec, 0, sizeof(spec));
  spec.n_servers = 1;
  spec.servers = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, write_servers(jo, &spec));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(jv));
  jv = NULL;

  /* 4. Schema primitives, unions, multipart, array items, and full SchemaRef */
  tu[0] = cdd_test_mut_str("string");
  tu[1] = cdd_test_mut_str("integer");
  SWEEP_OA_OBJ(o, write_schema_type(o, "string", 1));
  SWEEP_OA_OBJ(o, write_schema_type_union(o, "string", 1, tu, 2));
  SWEEP_OA_OBJ(o, write_schema_from_type_fields(o, "s", "string", 1, "string"));
  SWEEP_OA_OBJ(o, write_schema_from_type_fields(o, "s", "string", 1, "Model"));
  SWEEP_OA_OBJ(o, write_schema_from_type_fields(o, "s", "string", 0, NULL));
  SWEEP_OA_OBJ(o, write_schema_from_type_fields(o, "s", "Model", 0, NULL));
  SWEEP_OA_OBJ(o, write_schema_from_type_fields(o, "s", NULL, 0, NULL));

  memset(mp, 0, sizeof(mp));
  mp[0].name = cdd_test_mut_str("file");
  mp[0].is_binary = 1;
  mp[1].name = cdd_test_mut_str("meta");
  mp[1].type = cdd_test_mut_str("string");
  memset(&ref, 0, sizeof(ref));
  ref.multipart_fields = mp;
  ref.n_multipart_fields = 2;
  SWEEP_OA_OBJ(o, write_schema_ref(o, "mp", &ref));

  /* Array schema ref variants */
  memset(&sub_ref, 0, sizeof(sub_ref));
  sub_ref.inline_type = cdd_test_mut_str("string");
  memset(any_arr, 0, sizeof(any_arr));
  any_arr[0].type = OA_ANY_STRING;
  any_arr[0].string = cdd_test_mut_str("v1");
  any_arr[1].type = OA_ANY_NUMBER;
  any_arr[1].number = 2.0;

  memset(&ref, 0, sizeof(ref));
  ref.is_array = 1;
  ref.items_schema_is_boolean = 1;
  ref.items_schema_boolean_value = 1;
  SWEEP_OA_OBJ(o, write_schema_ref(o, "arr_bool", &ref));

  memset(&ref, 0, sizeof(ref));
  ref.is_array = 1;
  ref.inline_type = cdd_test_mut_str("string");
  ref.items_nullable = 1;
  ref.items_format = cdd_test_mut_str("byte");
  ref.items_collection_format = cdd_test_mut_str("csv");
  ref.items_content_media_type = cdd_test_mut_str("text/plain");
  ref.items_content_encoding = cdd_test_mut_str("base64");
  ref.items_content_schema = &sub_ref;
  ref.items_has_min = 1;
  ref.items_min_val = 1.0;
  ref.items_exclusive_min = 1;
  ref.items_has_max = 1;
  ref.items_max_val = 9.0;
  ref.items_exclusive_max = 1;
  ref.items_has_min_len = 1;
  ref.items_min_len = 1;
  ref.items_has_max_len = 1;
  ref.items_max_len = 5;
  ref.items_pattern = cdd_test_mut_str("^a");
  ref.items_has_min_items = 1;
  ref.items_min_items = 1;
  ref.items_has_max_items = 1;
  ref.items_max_items = 3;
  ref.items_unique_items = 1;
  ref.items_example = any_arr[0];
  ref.items_example_set = 1;
  ref.items_examples = any_arr;
  ref.n_items_examples = 2;
  ref.items_enum_values = any_arr;
  ref.n_items_enum_values = 2;
  ref.items_const_value = any_arr[0];
  ref.items_const_value_set = 1;
  ref.items_default_value = any_arr[0];
  ref.items_default_value_set = 1;
  ref.items_extra_json = cdd_test_mut_str("{\"x-it\": 1}");
  SWEEP_OA_OBJ(o, write_schema_ref(o, "arr_inl", &ref));

  ref.inline_type = NULL;
  ref.items_ref = cdd_test_mut_str("#/components/schemas/Item");
  ref.items_ref_is_dynamic = 1;
  SWEEP_OA_OBJ(o, write_schema_ref(o, "arr_iref", &ref));

  ref.items_ref = NULL;
  ref.ref_name = cdd_test_mut_str("string");
  SWEEP_OA_OBJ(o, write_schema_ref(o, "arr_prim", &ref));

  ref.ref_name = cdd_test_mut_str("CustomItem");
  SWEEP_OA_OBJ(o, write_schema_ref(o, "arr_mod", &ref));

  /* Non-array rich SchemaRef variants */
  memset(&xml, 0, sizeof(xml));
  xml.name = cdd_test_mut_str("Node");
  xml.namespace_uri = cdd_test_mut_str("https://ns");
  xml.prefix = cdd_test_mut_str("p");
  xml.node_type = OA_XML_NODE_ELEMENT;
  xml.node_type_set = 1;
  xml.attribute = 1;
  xml.attribute_set = 1;
  xml.wrapped = 1;
  xml.wrapped_set = 1;
  xml.extensions_json = cdd_test_mut_str("{\"x-xml\": 1}");

  memset(&ext_docs, 0, sizeof(ext_docs));
  ext_docs.url = cdd_test_mut_str("https://docs");
  ext_docs.description = cdd_test_mut_str("Docs");
  ext_docs.extensions_json = cdd_test_mut_str("{\"x-ed\": 1}");

  memset(dmap, 0, sizeof(dmap));
  dmap[0].value = cdd_test_mut_str("dog");
  dmap[0].schema = cdd_test_mut_str("#/components/schemas/Dog");
  memset(&disc, 0, sizeof(disc));
  disc.property_name = cdd_test_mut_str("petType");
  disc.default_mapping = cdd_test_mut_str("Other");
  disc.mapping = dmap;
  disc.n_mapping = 1;
  disc.extensions_json = cdd_test_mut_str("{\"x-disc\": 1}");

  memset(&ref, 0, sizeof(ref));
  ref.type_union = tu;
  ref.n_type_union = 2;
  ref.nullable = 1;
  ref.format = cdd_test_mut_str("email");
  ref.collection_format = cdd_test_mut_str("pipes");
  ref.has_min = 1;
  ref.min_val = 0.0;
  ref.exclusive_min = 1;
  ref.has_max = 1;
  ref.max_val = 10.0;
  ref.exclusive_max = 1;
  ref.has_min_len = 1;
  ref.min_len = 1;
  ref.has_max_len = 1;
  ref.max_len = 20;
  ref.pattern = cdd_test_mut_str("^[a-z]+$");
  ref.has_min_items = 1;
  ref.min_items = 0;
  ref.has_max_items = 1;
  ref.max_items = 5;
  ref.unique_items = 1;
  ref.has_multiple_of = 1;
  ref.multiple_of = 2.0;
  ref.has_min_properties = 1;
  ref.min_properties = 1;
  ref.has_max_properties = 1;
  ref.max_properties = 10;
  ref.all_of = &sub_ref;
  ref.n_all_of = 1;
  ref.any_of = &sub_ref;
  ref.n_any_of = 1;
  ref.one_of = &sub_ref;
  ref.n_one_of = 1;
  ref.not_schema = &sub_ref;
  ref.if_schema = &sub_ref;
  ref.then_schema = &sub_ref;
  ref.else_schema = &sub_ref;
  ref.summary = cdd_test_mut_str("Sum");
  ref.description = cdd_test_mut_str("Desc");
  ref.deprecated = 1;
  ref.deprecated_set = 1;
  ref.read_only = 1;
  ref.read_only_set = 1;
  ref.write_only = 1;
  ref.write_only_set = 1;
  ref.default_value = any_arr[0];
  ref.default_value_set = 1;
  ref.const_value = any_arr[0];
  ref.const_value_set = 1;
  ref.enum_values = any_arr;
  ref.n_enum_values = 2;
  ref.example = any_arr[0];
  ref.example_set = 1;
  ref.examples = any_arr;
  ref.n_examples = 2;
  ref.content_media_type = cdd_test_mut_str("application/json");
  ref.content_encoding = cdd_test_mut_str("utf-8");
  ref.content_schema = &sub_ref;
  ref.external_docs = ext_docs;
  ref.external_docs_set = 1;
  ref.discriminator = disc;
  ref.discriminator_set = 1;
  ref.xml = xml;
  ref.xml_set = 1;
  ref.schema_extra_json = cdd_test_mut_str("{\"x-sch\": 1}");
  SWEEP_OA_OBJ(o, write_schema_ref(o, "rich_union", &ref));

  ref.n_type_union = 0;
  ref.type_union = NULL;
  ref.ref = cdd_test_mut_str("#/components/schemas/Ext");
  SWEEP_OA_OBJ(o, write_schema_ref(o, "rich_ref", &ref));

  ref.ref = NULL;
  ref.ref_name = cdd_test_mut_str("string");
  SWEEP_OA_OBJ(o, write_schema_ref(o, "rich_prim", &ref));

  ref.ref_name = cdd_test_mut_str("ModelRef");
  SWEEP_OA_OBJ(o, write_schema_ref(o, "rich_mod", &ref));

  /* 5. Parameters, headers, encodings, media types, links, responses */
  memset(ex_arr, 0, sizeof(ex_arr));
  ex_arr[0].name = cdd_test_mut_str("ex1");
  ex_arr[0].summary = cdd_test_mut_str("esum");
  ex_arr[0].description = cdd_test_mut_str("edesc");
  ex_arr[0].value = any_arr[0];
  ex_arr[0].value_set = 1;
  ex_arr[0].data_value = any_arr[0];
  ex_arr[0].data_value_set = 1;
  ex_arr[0].serialized_value = cdd_test_mut_str("raw");
  ex_arr[0].external_value = cdd_test_mut_str("https://ex");
  ex_arr[0].extensions_json = cdd_test_mut_str("{\"x-ex\": 1}");

  memset(&param, 0, sizeof(param));
  param.ref = cdd_test_mut_str("#/components/parameters/PRef");
  param.summary = cdd_test_mut_str("PSum");
  param.description = cdd_test_mut_str("PDesc");
  SWEEP_OA_OBJ(o, write_parameter_object(o, &param));

  memset(&param, 0, sizeof(param));
  param.name = cdd_test_mut_str("q");
  param.in = OA_PARAM_IN_QUERY;
  param.required = 1;
  param.description = cdd_test_mut_str("QDesc");
  param.deprecated = 1;
  param.deprecated_set = 1;
  param.allow_empty_value = 1;
  param.allow_empty_value_set = 1;
  param.collection_format = cdd_test_mut_str("multi");
  param.style = OA_STYLE_FORM;
  param.explode = 1;
  param.explode_set = 1;
  param.allow_reserved = 1;
  param.allow_reserved_set = 1;
  param.example_location = OA_EXAMPLE_LOC_OBJECT;
  param.example = any_arr[0];
  param.example_set = 1;
  param.examples = ex_arr;
  param.n_examples = 1;
  param.schema = sub_ref;
  param.schema_set = 1;
  param.extensions_json = cdd_test_mut_str("{\"x-p\": 1}");
  SWEEP_OA_OBJ(o, write_parameter_object(o, &param));

  param.explode_set = 0;
  param.explode = 1;
  param.item_schema_set = 1;
  SWEEP_OA_OBJ(o, write_parameter_object(o, &param));

  param.content_type = cdd_test_mut_str("application/json");
  param.example_location = OA_EXAMPLE_LOC_MEDIA;
  SWEEP_OA_OBJ(o, write_parameter_object(o, &param));

  param.item_schema_set = 0;
  param.schema_set = 1;
  SWEEP_OA_OBJ(o, write_parameter_object(o, &param));

  param.schema_set = 0;
  memset(&param.schema, 0, sizeof(param.schema));
  param.type = cdd_test_mut_str("string");
  SWEEP_OA_OBJ(o, write_parameter_object(o, &param));

  param.content_ref = cdd_test_mut_str("#/components/mediaTypes/MRef");
  SWEEP_OA_OBJ(o, write_parameter_object(o, &param));

  /* Header variants */
  memset(&hdr, 0, sizeof(hdr));
  hdr.name = cdd_test_mut_str("X-Hdr");
  hdr.ref = cdd_test_mut_str("#/components/headers/HRef");
  hdr.description = cdd_test_mut_str("HDesc");
  SWEEP_OA_OBJ(o, write_header_object(o, &hdr));

  memset(&hdr, 0, sizeof(hdr));
  hdr.name = cdd_test_mut_str("X-Hdr");
  hdr.description = cdd_test_mut_str("HDesc");
  hdr.required = 1;
  hdr.deprecated = 1;
  hdr.deprecated_set = 1;
  hdr.style = OA_STYLE_SIMPLE;
  hdr.style_set = 1;
  hdr.collection_format = cdd_test_mut_str("csv");
  hdr.explode = 1;
  hdr.explode_set = 1;
  hdr.example_location = OA_EXAMPLE_LOC_OBJECT;
  hdr.example = any_arr[0];
  hdr.example_set = 1;
  hdr.examples = ex_arr;
  hdr.n_examples = 1;
  hdr.schema = sub_ref;
  hdr.schema_set = 1;
  hdr.extensions_json = cdd_test_mut_str("{\"x-h\": 1}");
  SWEEP_OA_OBJ(o, write_header_object(o, &hdr));

  hdr.schema_set = 0;
  memset(&hdr.schema, 0, sizeof(hdr.schema));
  hdr.type = cdd_test_mut_str("string");
  SWEEP_OA_OBJ(o, write_header_object(o, &hdr));

  hdr.content_type = cdd_test_mut_str("application/json");
  hdr.example_location = OA_EXAMPLE_LOC_MEDIA;
  SWEEP_OA_OBJ(o, write_header_object(o, &hdr));

  hdr.schema = sub_ref;
  hdr.schema_set = 1;
  SWEEP_OA_OBJ(o, write_header_object(o, &hdr));

  hdr.content_ref = cdd_test_mut_str("#/components/mediaTypes/MRef");
  SWEEP_OA_OBJ(o, write_header_object(o, &hdr));

  /* Encoding and MediaType */
  memset(&sub_enc, 0, sizeof(sub_enc));
  sub_enc.name = cdd_test_mut_str("part");
  sub_enc.content_type = cdd_test_mut_str("text/plain");
  memset(&enc, 0, sizeof(enc));
  enc.name = cdd_test_mut_str("main");
  enc.content_type = cdd_test_mut_str("application/json");
  enc.headers = &hdr;
  enc.n_headers = 1;
  enc.style = OA_STYLE_FORM;
  enc.style_set = 1;
  enc.explode = 1;
  enc.explode_set = 1;
  enc.allow_reserved = 1;
  enc.allow_reserved_set = 1;
  enc.encoding = &sub_enc;
  enc.n_encoding = 1;
  enc.prefix_encoding = &sub_enc;
  enc.n_prefix_encoding = 1;
  enc.item_encoding = &sub_enc;
  enc.item_encoding_set = 1;
  enc.extensions_json = cdd_test_mut_str("{\"x-enc\": 1}");
  SWEEP_OA_OBJ(o, write_encoding_object(o, &enc));

  memset(&mt, 0, sizeof(mt));
  mt.name = cdd_test_mut_str("application/json");
  mt.schema = sub_ref;
  mt.schema_set = 1;
  mt.item_schema = sub_ref;
  mt.item_schema_set = 1;
  mt.example = any_arr[0];
  mt.example_set = 1;
  mt.examples = ex_arr;
  mt.n_examples = 1;
  mt.encoding = &enc;
  mt.n_encoding = 1;
  mt.prefix_encoding = &sub_enc;
  mt.n_prefix_encoding = 1;
  mt.item_encoding = &sub_enc;
  mt.item_encoding_set = 1;
  mt.extensions_json = cdd_test_mut_str("{\"x-mt\": 1}");
  SWEEP_OA_OBJ(o, write_media_type_object(o, &mt));

  /* Server, Link, RequestBody, Response, Callback */
  svar_enums[0] = cdd_test_mut_str("v1");
  memset(svar, 0, sizeof(svar));
  svar[0].name = cdd_test_mut_str("ver");
  svar[0].default_value = cdd_test_mut_str("v1");
  svar[0].description = cdd_test_mut_str("Version");
  svar[0].enum_values = svar_enums;
  svar[0].n_enum_values = 1;
  svar[0].extensions_json = cdd_test_mut_str("{\"x-sv\": 1}");
  memset(&srv, 0, sizeof(srv));
  srv.url = cdd_test_mut_str("https://api.example.com/{ver}");
  srv.name = cdd_test_mut_str("prod");
  srv.description = cdd_test_mut_str("Prod");
  srv.variables = svar;
  srv.n_variables = 1;
  srv.extensions_json = cdd_test_mut_str("{\"x-srv\": 1}");
  SWEEP_OA_OBJ(o, write_server_array(o, "servers", &srv, 1));

  memset(lparam, 0, sizeof(lparam));
  lparam[0].name = cdd_test_mut_str("id");
  lparam[0].value = any_arr[0];
  memset(&link, 0, sizeof(link));
  link.name = cdd_test_mut_str("GetUser");
  link.ref = cdd_test_mut_str("#/components/links/LRef");
  link.summary = cdd_test_mut_str("LSum");
  link.description = cdd_test_mut_str("LDesc");
  memset(&spec, 0, sizeof(spec));
  spec.component_links = &link;
  spec.n_component_links = 1;
  SWEEP_OA_OBJ(o, write_component_links(o, &spec));

  link.ref = NULL;
  link.operation_ref = cdd_test_mut_str("#/paths/~1users/get");
  link.operation_id = cdd_test_mut_str("getUser");
  link.parameters = lparam;
  link.n_parameters = 1;
  link.request_body = any_arr[0];
  link.request_body_set = 1;
  link.server = &srv;
  link.server_set = 1;
  link.extensions_json = cdd_test_mut_str("{\"x-lnk\": 1}");

  memset(&rb, 0, sizeof(rb));
  rb.description = cdd_test_mut_str("ReqBody");
  rb.required = 1;
  rb.required_set = 1;
  rb.content_media_types = &mt;
  rb.n_content_media_types = 1;
  rb.extensions_json = cdd_test_mut_str("{\"x-rb\": 1}");
  SWEEP_OA_OBJ(o, write_request_body_object(o, &rb));

  memset(&resp, 0, sizeof(resp));
  resp.code = cdd_test_mut_str("200");
  resp.summary = cdd_test_mut_str("RSum");
  resp.description = cdd_test_mut_str("RDesc");
  resp.headers = &hdr;
  resp.n_headers = 1;
  resp.content_media_types = &mt;
  resp.n_content_media_types = 1;
  resp.links = &link;
  resp.n_links = 1;
  resp.extensions_json = cdd_test_mut_str("{\"x-rsp\": 1}");

  /* 6. Operation, Path, Info, Tags, Security, Components, Full Spec */
  op_tags[0] = cdd_test_mut_str("users");
  sscopes[0] = cdd_test_mut_str("read");
  memset(sreq, 0, sizeof(sreq));
  sreq[0].scheme = cdd_test_mut_str("oauth");
  sreq[0].scopes = sscopes;
  sreq[0].n_scopes = 1;
  memset(sset, 0, sizeof(sset));
  sset[0].requirements = sreq;
  sset[0].n_requirements = 1;

  memset(&cb_path, 0, sizeof(cb_path));
  cb_path.route = cdd_test_mut_str("{$request.body#/url}");
  cb_path.summary = cdd_test_mut_str("CBSum");
  memset(&cb, 0, sizeof(cb));
  cb.name = cdd_test_mut_str("onEvent");
  cb.paths = &cb_path;
  cb.n_paths = 1;
  cb.extensions_json = cdd_test_mut_str("{\"x-cb\": 1}");

  memset(&op, 0, sizeof(op));
  op.verb = OA_VERB_POST;
  op.tags = op_tags;
  op.n_tags = 1;
  op.summary = cdd_test_mut_str("OpSum");
  op.description = cdd_test_mut_str("OpDesc");
  op.external_docs = ext_docs;
  op.operation_id = cdd_test_mut_str("createUser");
  op.parameters = &param;
  op.n_parameters = 1;
  op.req_body = sub_ref;
  op.req_body_description = cdd_test_mut_str("Body");
  op.req_body_required = 1;
  op.req_body_required_set = 1;
  op.req_body_extensions_json = cdd_test_mut_str("{\"x-oprb\": 1}");
  op.responses = &resp;
  op.n_responses = 1;
  op.callbacks = &cb;
  op.n_callbacks = 1;
  op.deprecated = 1;
  op.security = sset;
  op.n_security = 1;
  op.security_set = 1;
  op.servers = &srv;
  op.n_servers = 1;
  op.extensions_json = cdd_test_mut_str("{\"x-op\": 1}");
  SWEEP_OA_OBJ(o, write_operation_object(o, &op));

  /* Security schemes covering all 5 types + scope description NULL/non-NULL */
  memset(oscope, 0, sizeof(oscope));
  oscope[0].name = cdd_test_mut_str("read");
  oscope[0].description = NULL;
  oscope[1].name = cdd_test_mut_str("write");
  oscope[1].description = cdd_test_mut_str("Write access");
  memset(oflow, 0, sizeof(oflow));
  oflow[0].type = OA_OAUTH_FLOW_AUTHORIZATION_CODE;
  oflow[0].authorization_url = cdd_test_mut_str("https://auth");
  oflow[0].token_url = cdd_test_mut_str("https://tok");
  oflow[0].refresh_url = cdd_test_mut_str("https://ref");
  oflow[0].device_authorization_url = cdd_test_mut_str("https://dev");
  oflow[0].scopes = oscope;
  oflow[0].n_scopes = 2;
  oflow[0].extensions_json = cdd_test_mut_str("{\"x-fl\": 1}");

  memset(sschemes, 0, sizeof(sschemes));
  sschemes[0].name = cdd_test_mut_str("ak_q");
  sschemes[0].type = OA_SEC_APIKEY;
  sschemes[0].in = OA_SEC_IN_QUERY;
  sschemes[0].key_name = cdd_test_mut_str("api_key");
  sschemes[0].deprecated = 1;
  sschemes[0].deprecated_set = 1;
  sschemes[0].description = cdd_test_mut_str("Query key");
  sschemes[0].extensions_json = cdd_test_mut_str("{\"x-ss\": 1}");

  sschemes[1].name = cdd_test_mut_str("http_jwt");
  sschemes[1].type = OA_SEC_HTTP;
  sschemes[1].scheme = cdd_test_mut_str("bearer");
  sschemes[1].bearer_format = NULL;

  sschemes[2].name = cdd_test_mut_str("mtls");
  sschemes[2].type = OA_SEC_MUTUALTLS;

  sschemes[3].name = cdd_test_mut_str("oa2");
  sschemes[3].type = OA_SEC_OAUTH2;
  sschemes[3].oauth2_metadata_url = cdd_test_mut_str("https://meta");
  sschemes[3].flows = oflow;
  sschemes[3].n_flows = 1;

  sschemes[4].name = cdd_test_mut_str("oidc");
  sschemes[4].type = OA_SEC_OPENID;
  sschemes[4].open_id_connect_url = cdd_test_mut_str("https://oidc");

  /* Defined schema + $id/$anchor/$dynamicAnchor + raw schema + full spec */
  memset(def_flds, 0, sizeof(def_flds));
  memcpy(def_flds[0].name, "id", 3);
  memcpy(def_flds[0].type, "integer", 8);
  memset(def_sf, 0, sizeof(def_sf));
  def_sf[0].fields = def_flds;
  def_sf[0].size = 1;
  def_names[0] = cdd_test_mut_str("User");
  def_ids[0] = cdd_test_mut_str("https://example.com/user.json");
  def_anchors[0] = cdd_test_mut_str("UserAnchor");
  def_dyn_anchors[0] = cdd_test_mut_str("UserDyn");
  raw_names[0] = cdd_test_mut_str("RawModel");
  raw_jsons[0] = cdd_test_mut_str("{\"type\": \"object\"}");
  comp_names[0] = cdd_test_mut_str("CompItem");

  memset(&info, 0, sizeof(info));
  info.title = cdd_test_mut_str("API");
  info.summary = cdd_test_mut_str("Sum");
  info.description = cdd_test_mut_str("Desc");
  info.terms_of_service = cdd_test_mut_str("https://tos");
  info.version = cdd_test_mut_str("1.0.0");
  info.contact.name = cdd_test_mut_str("Support");
  info.contact.url = cdd_test_mut_str("https://support");
  info.contact.email = cdd_test_mut_str("a@b.c");
  info.contact.extensions_json = cdd_test_mut_str("{\"x-ct\": 1}");
  info.license.name = cdd_test_mut_str("MIT");
  info.license.identifier = cdd_test_mut_str("MIT");
  info.license.extensions_json = cdd_test_mut_str("{\"x-lic\": 1}");
  info.extensions_json = cdd_test_mut_str("{\"x-inf\": 1}");

  memset(&tag, 0, sizeof(tag));
  tag.name = cdd_test_mut_str("users");
  tag.summary = cdd_test_mut_str("Users");
  tag.description = cdd_test_mut_str("User ops");
  tag.parent = cdd_test_mut_str("root");
  tag.kind = cdd_test_mut_str("nav");
  tag.external_docs = ext_docs;
  tag.extensions_json = cdd_test_mut_str("{\"x-tag\": 1}");

  memset(&main_path, 0, sizeof(main_path));
  main_path.route = cdd_test_mut_str("/users");
  main_path.summary = cdd_test_mut_str("PathSum");
  main_path.description = cdd_test_mut_str("PathDesc");
  main_path.operations = &op;
  main_path.n_operations = 1;
  main_path.parameters = &param;
  main_path.n_parameters = 1;
  main_path.servers = &srv;
  main_path.n_servers = 1;
  main_path.extensions_json = cdd_test_mut_str("{\"x-pi\": 1}");

  memset(&spec, 0, sizeof(spec));
  spec.openapi_version = cdd_test_mut_str("3.2.0");
  spec.self_uri = cdd_test_mut_str("https://example.com/openapi.json");
  spec.json_schema_dialect =
      cdd_test_mut_str("https://spec.openapis.org/oas/3.1/dialect/base");
  spec.info = info;
  spec.servers = &srv;
  spec.n_servers = 1;
  spec.paths = &main_path;
  spec.n_paths = 1;
  spec.paths_extensions_json = cdd_test_mut_str("{\"x-paths\": 1}");
  spec.webhooks = &main_path;
  spec.n_webhooks = 1;
  spec.webhooks_extensions_json = cdd_test_mut_str("{\"x-wh\": 1}");
  spec.security = sset;
  spec.n_security = 1;
  spec.security_set = 1;
  spec.tags = &tag;
  spec.n_tags = 1;
  spec.external_docs = ext_docs;
  spec.extensions_json = cdd_test_mut_str("{\"x-root\": 1}");
  spec.components_extensions_json = cdd_test_mut_str("{\"x-comp\": 1}");
  spec.defined_schemas = def_sf;
  spec.defined_schema_names = def_names;
  spec.defined_schema_ids = def_ids;
  spec.defined_schema_anchors = def_anchors;
  spec.defined_schema_dynamic_anchors = def_dyn_anchors;
  spec.n_defined_schemas = 1;
  spec.raw_schema_names = raw_names;
  spec.raw_schema_json = raw_jsons;
  spec.n_raw_schemas = 1;
  spec.security_schemes = sschemes;
  spec.n_security_schemes = 5;
  spec.component_parameters = &param;
  spec.component_parameter_names = comp_names;
  spec.n_component_parameters = 1;
  spec.component_responses = &resp;
  spec.component_response_names = comp_names;
  spec.n_component_responses = 1;
  spec.component_headers = &hdr;
  spec.component_header_names = comp_names;
  spec.n_component_headers = 1;
  spec.component_request_bodies = &rb;
  spec.component_request_body_names = comp_names;
  spec.n_component_request_bodies = 1;
  spec.component_media_types = &mt;
  spec.component_media_type_names = comp_names;
  spec.n_component_media_types = 1;
  spec.component_examples = ex_arr;
  spec.component_example_names = comp_names;
  spec.n_component_examples = 1;
  spec.component_links = &link;
  spec.n_component_links = 1;
  spec.component_callbacks = &cb;
  spec.n_component_callbacks = 1;
  spec.component_path_items = &main_path;
  spec.component_path_item_names = comp_names;
  spec.n_component_path_items = 1;

  SWEEP_OA_OBJ(o, write_components(o, &spec));
  SWEEP_OA_OBJ(o, write_component_responses(o, &spec));
  SWEEP_OA_OBJ(o, write_component_headers(o, &spec));
  SWEEP_OA_OBJ(o, write_component_request_bodies(o, &spec));
  SWEEP_OA_OBJ(o, write_callbacks(o, &op));
  SWEEP_OA_OBJ(o, write_operations(o, &main_path));

  for (k = 0; k < 600; ++k) {
    int rem;
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    g_cdd_last_alloc_err = openapi_write_spec_to_json(&spec, &spec_json);
    rem = g_parson_oom_fail_at;
    g_parson_oom_fail_at = -1;
    json_set_allocation_functions(malloc, free);
    if (spec_json) {
      free(spec_json);
      spec_json = NULL;
    }
    if (rem >= 0)
      break;
  }
  for (k = 1; k < 600; ++k) {
    int rem;
    g_cdd_step_fail = k;
    g_cdd_last_alloc_err = openapi_write_spec_to_json(&spec, &spec_json);
    rem = g_cdd_step_fail;
    g_cdd_step_fail = 0;
    if (spec_json) {
      free(spec_json);
      spec_json = NULL;
    }
    if (rem > 0)
      break;
  }

  /* 7. Targeted sweeps for remaining variant branches */
  {
    struct OpenAPI_Example ex_v;
    struct OpenAPI_SecurityScheme ss_var[5];
    struct OpenAPI_Path dup_paths[2];
    struct OpenAPI_Operation addl_op;
    struct OpenAPI_Callback cb_ref;
    struct OpenAPI_Response rsp_v;
    struct OpenAPI_RequestBody rb_v;

    /* exclusiveMinimum boolean without minimum */
    memset(&ref, 0, sizeof(ref));
    ref.inline_type = cdd_test_mut_str("number");
    ref.exclusive_min = 1;
    SWEEP_OA_OBJ(o, write_schema_ref(o, "exmin", &ref));

    /* write_example_object with $ref, and with value_set only */
    memset(&ex_v, 0, sizeof(ex_v));
    ex_v.ref = cdd_test_mut_str("#/components/examples/ERef");
    ex_v.summary = cdd_test_mut_str("ESum");
    ex_v.description = cdd_test_mut_str("EDesc");
    SWEEP_OA_OBJ(o, write_example_object(o, &ex_v));

    memset(&ex_v, 0, sizeof(ex_v));
    ex_v.value = any_arr[0];
    ex_v.value_set = 1;
    SWEEP_OA_OBJ(o, write_example_object(o, &ex_v));

    /* write_example_fields single example */
    SWEEP_OA_OBJ(o, write_example_fields(o, &any_arr[0], 1, NULL, 0));

    /* write_parameter_object non-content type fallback */
    memset(&param, 0, sizeof(param));
    param.name = cdd_test_mut_str("p2");
    param.in = OA_PARAM_IN_QUERY;
    param.type = cdd_test_mut_str("string");
    SWEEP_OA_OBJ(o, write_parameter_object(o, &param));

    /* write_component_links with $ref only (summary=NULL, description=NULL) */
    memset(&link, 0, sizeof(link));
    link.name = cdd_test_mut_str("LOnly");
    link.ref = cdd_test_mut_str("#/components/links/LRef");
    memset(&spec, 0, sizeof(spec));
    spec.component_links = &link;
    spec.n_component_links = 1;
    SWEEP_OA_OBJ(o, write_component_links(o, &spec));

    /* write_response_object $ref, content_ref, and inline schema */
    memset(&rsp_v, 0, sizeof(rsp_v));
    rsp_v.ref = cdd_test_mut_str("#/components/responses/RRef");
    rsp_v.summary = cdd_test_mut_str("RS");
    rsp_v.description = cdd_test_mut_str("RD");
    SWEEP_OA_OBJ(o, write_response_object(o, &rsp_v));

    memset(&rsp_v, 0, sizeof(rsp_v));
    rsp_v.description = cdd_test_mut_str("RD");
    rsp_v.content_ref = cdd_test_mut_str("#/components/mediaTypes/MRef");
    SWEEP_OA_OBJ(o, write_response_object(o, &rsp_v));

    memset(&rsp_v, 0, sizeof(rsp_v));
    rsp_v.description = cdd_test_mut_str("RD");
    rsp_v.schema = sub_ref;
    rsp_v.examples = ex_arr;
    rsp_v.n_examples = 1;
    SWEEP_OA_OBJ(o, write_response_object(o, &rsp_v));

    /* write_request_body_object inline schema + examples, and component $ref */
    memset(&rb_v, 0, sizeof(rb_v));
    rb_v.schema = sub_ref;
    rb_v.examples = ex_arr;
    rb_v.n_examples = 1;
    SWEEP_OA_OBJ(o, write_request_body_object(o, &rb_v));

    memset(&rb_v, 0, sizeof(rb_v));
    rb_v.ref = cdd_test_mut_str("#/components/requestBodies/RBRef");
    memset(&spec, 0, sizeof(spec));
    spec.component_request_bodies = &rb_v;
    spec.component_request_body_names = comp_names;
    spec.n_component_request_bodies = 1;
    SWEEP_OA_OBJ(o, write_component_request_bodies(o, &spec));

    /* write_request_body on op with req_body_ref and minimal write_operations
     */
    memset(&addl_op, 0, sizeof(addl_op));
    addl_op.verb = OA_VERB_POST;
    addl_op.req_body_ref = cdd_test_mut_str("#/components/requestBodies/RBRef");
    addl_op.req_body_description = cdd_test_mut_str("BDesc");
    addl_op.req_body_extensions_json = cdd_test_mut_str("{\"x-b\": 1}");
    addl_op.responses = &rsp_v;
    addl_op.n_responses = 1;
    addl_op.responses_extensions_json = cdd_test_mut_str("{\"x-re\": 1}");
    SWEEP_OA_OBJ(o, write_request_body(o, &addl_op));
    SWEEP_OA_OBJ(o, write_responses(o, &addl_op));

    memset(&addl_op, 0, sizeof(addl_op));
    addl_op.verb = OA_VERB_GET;
    memset(&main_path, 0, sizeof(main_path));
    main_path.operations = &addl_op;
    main_path.n_operations = 1;
    SWEEP_OA_OBJ(o, write_operations(o, &main_path));

    /* write_callback_object $ref, write_additional_operations, path $ref,
     * duplicate routes, and security requirement extensions */
    memset(&cb_ref, 0, sizeof(cb_ref));
    cb_ref.name = cdd_test_mut_str("cbRef");
    cb_ref.ref = cdd_test_mut_str("#/components/callbacks/CBRef");
    cb_ref.summary = cdd_test_mut_str("CS");
    cb_ref.description = cdd_test_mut_str("CD");
    SWEEP_OA_OBJ(o, write_callback_object(o, &cb_ref));

    memset(&main_path, 0, sizeof(main_path));
    main_path.route = cdd_test_mut_str("/items");
    main_path.summary = cdd_test_mut_str("PS");
    main_path.description = cdd_test_mut_str("PD");
    main_path.ref = cdd_test_mut_str("#/components/pathItems/PIRef");
    main_path.additional_operations = &addl_op;
    main_path.n_additional_operations = 1;
    SWEEP_OA_OBJ(o, write_path_item_object(o, &main_path));
    SWEEP_OA_OBJ(o, write_additional_operations(o, &main_path));

    dup_paths[0] = main_path;
    dup_paths[1] = main_path;
    memset(&spec, 0, sizeof(spec));
    spec.paths = dup_paths;
    spec.n_paths = 2;
    SWEEP_OA_OBJ(o, write_paths(o, &spec));

    sset[0].extensions_json = cdd_test_mut_str("{\"x-sr\": 1}");
    SWEEP_OA_OBJ(o, write_security_requirements(o, "security", sset, 1, 1));

    /* Security scheme variants: header/cookie apiKey, null/custom-bearer http,
     * and empty-flows oauth2 */
    memset(ss_var, 0, sizeof(ss_var));
    ss_var[0].name = cdd_test_mut_str("ak_h");
    ss_var[0].type = OA_SEC_APIKEY;
    ss_var[0].in = OA_SEC_IN_HEADER;
    ss_var[1].name = cdd_test_mut_str("ak_c");
    ss_var[1].type = OA_SEC_APIKEY;
    ss_var[1].in = OA_SEC_IN_COOKIE;
    ss_var[2].name = cdd_test_mut_str("http_null");
    ss_var[2].type = OA_SEC_HTTP;
    ss_var[2].scheme = NULL;
    ss_var[3].name = cdd_test_mut_str("http_b2");
    ss_var[3].type = OA_SEC_HTTP;
    ss_var[3].scheme = cdd_test_mut_str("bearer");
    ss_var[3].bearer_format = cdd_test_mut_str("Paseto");
    ss_var[4].name = cdd_test_mut_str("oa2_empty");
    ss_var[4].type = OA_SEC_OAUTH2;
    ss_var[4].flows = NULL;
    ss_var[4].n_flows = 0;
    memset(&spec, 0, sizeof(spec));
    spec.security_schemes = ss_var;
    spec.n_security_schemes = 5;
    SWEEP_OA_OBJ(o, write_security_schemes(o, &spec));
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_FINISH_H */
