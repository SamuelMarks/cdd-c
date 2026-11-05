/**
 * @file test_openapi_copy_components.h
 * @brief Unit tests for OpenAPI component deep-copy routines and OOM branches.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_COPY_COMPONENTS_H
#define TEST_OPENAPI_COPY_COMPONENTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Unit test for component copy and free helpers (parameter, header,
 * encoding, media type, response, example).
 * @return Greatest test result.
 */
TEST test_openapi_copy_components_helpers(void) {
  struct OpenAPI_Parameter p_src, p_dst;
  struct OpenAPI_Header h_src, h_dst;
  struct OpenAPI_Encoding enc_src, enc_dst;
  struct OpenAPI_MediaType mt_src, mt_dst;
  struct OpenAPI_Response r_src, r_dst;
  struct OpenAPI_Example ex_src, ex_dst;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  /* 1. Parameter copy & free */
  memset(&p_src, 0, sizeof(p_src));
  memset(&p_dst, 0, sizeof(p_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_parameter_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_parameter_fields(&p_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_parameter_fields(NULL, &p_src));

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

  rc = copy_parameter_fields(&p_dst, &p_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("param1", p_dst.name);
  ASSERT_STR_EQ("param desc", p_dst.description);
  ASSERT_STR_EQ("application/json", p_dst.content_type);
  ASSERT_STR_EQ("#/components/mediaTypes/Json", p_dst.content_ref);
  ASSERT_STR_EQ("string", p_dst.items_type);
  ASSERT_EQ(1, p_dst.example_set);
  ASSERT_STR_EQ("ex_val", p_dst.example.string);

  free_parameter(&p_dst);

  {
    int k;
    for (k = 1; k <= 15; ++k) {
      memset(&p_dst, 0, sizeof(p_dst));
      g_cdd_strdup_fail = k;
      rc = copy_parameter_fields(&p_dst, &p_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      free_parameter(&p_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  free_parameter(&p_src);
  free_parameter(NULL);

  /* 2. Header copy & free */
  memset(&h_src, 0, sizeof(h_src));
  memset(&h_dst, 0, sizeof(h_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_header_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_header_fields(&h_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_header_fields(NULL, &h_src));

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

  rc = copy_header_fields(&h_dst, &h_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("hdr desc", h_dst.description);
  ASSERT_STR_EQ("text/plain", h_dst.content_type);
  ASSERT_STR_EQ("#/components/mediaTypes/Text", h_dst.content_ref);
  ASSERT_STR_EQ("integer", h_dst.items_type);
  ASSERT_EQ(1, h_dst.example_set);

  free_header(&h_dst);

  {
    int k;
    for (k = 1; k <= 15; ++k) {
      memset(&h_dst, 0, sizeof(h_dst));
      g_cdd_strdup_fail = k;
      rc = copy_header_fields(&h_dst, &h_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      free_header(&h_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  free_header(&h_src);
  free_header(NULL);

  /* 3. Encoding copy & free */
  memset(&enc_src, 0, sizeof(enc_src));
  memset(&enc_dst, 0, sizeof(enc_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_encoding_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_encoding_fields(&enc_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_encoding_fields(NULL, &enc_src));

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

  rc = copy_encoding_fields(&enc_dst, &enc_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("enc1", enc_dst.name);
  ASSERT_STR_EQ("image/png", enc_dst.content_type);

  free_encoding(&enc_dst);

  {
    int k;
    for (k = 1; k <= 20; ++k) {
      memset(&enc_dst, 0, sizeof(enc_dst));
      g_cdd_strdup_fail = k;
      rc = copy_encoding_fields(&enc_dst, &enc_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      free_encoding(&enc_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  free_encoding(&enc_src);
  free_encoding(NULL);

  /* 4. Media type copy & free */
  memset(&mt_src, 0, sizeof(mt_src));
  memset(&mt_dst, 0, sizeof(mt_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_media_type_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_media_type_fields(&mt_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_media_type_fields(NULL, &mt_src));

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

  rc = copy_media_type_fields(&mt_dst, &mt_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("application/json", mt_dst.name);
  ASSERT_STR_EQ("#/components/mediaTypes/Json", mt_dst.ref);
  ASSERT_EQ(1, mt_dst.example_set);

  free_media_type(&mt_dst);

  {
    int k;
    for (k = 1; k <= 20; ++k) {
      memset(&mt_dst, 0, sizeof(mt_dst));
      g_cdd_strdup_fail = k;
      rc = copy_media_type_fields(&mt_dst, &mt_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      free_media_type(&mt_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  free_media_type(&mt_src);
  free_media_type(NULL);

  /* Copy media type array */
  {
    struct OpenAPI_MediaType *mt_arr_dst = NULL;
    size_t mt_count = 0;
    struct OpenAPI_MediaType mt_arr_src[1];
    memset(mt_arr_src, 0, sizeof(mt_arr_src));
    mt_arr_src[0].name = strdup("app/json");

    ASSERT_EQ(CDD_C_SUCCESS, copy_media_type_array(NULL, NULL, NULL, 0));
    ASSERT_EQ(CDD_C_SUCCESS,
              copy_media_type_array(&mt_arr_dst, &mt_count, NULL, 0));

    rc = copy_media_type_array(&mt_arr_dst, &mt_count, mt_arr_src, 1);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, mt_count);
    ASSERT(mt_arr_dst != NULL);

    {
      size_t m;
      for (m = 0; m < mt_count; ++m) {
        free_media_type(&mt_arr_dst[m]);
      }
      free(mt_arr_dst);
    }
    free_media_type(&mt_arr_src[0]);
  }

  /* 5. Response copy & free */
  memset(&r_src, 0, sizeof(r_src));
  memset(&r_dst, 0, sizeof(r_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_response_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_response_fields(&r_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_response_fields(NULL, &r_src));

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

  rc = copy_response_fields(&r_dst, &r_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("OK summary", r_dst.summary);
  ASSERT_STR_EQ("OK response", r_dst.description);
  ASSERT_EQ(1, r_dst.example_set);

  free_response(&r_dst);

  {
    int k;
    for (k = 1; k <= 25; ++k) {
      memset(&r_dst, 0, sizeof(r_dst));
      g_cdd_strdup_fail = k;
      rc = copy_response_fields(&r_dst, &r_src);
      if (rc != CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
      free_response(&r_dst);
    }
    g_cdd_strdup_fail = 0;
  }

  free_response(&r_src);
  free_response(NULL);

  /* 6. Example copy & free */
  memset(&ex_src, 0, sizeof(ex_src));
  memset(&ex_dst, 0, sizeof(ex_dst));
  ASSERT_EQ(CDD_C_SUCCESS, copy_example_fields(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_example_fields(&ex_dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_example_fields(NULL, &ex_src));

  ex_src.name = strdup("ex_name");
  ex_src.summary = strdup("ex summary");
  ex_src.description = strdup("ex description");
  ex_src.external_value = strdup("http://ex.json");
  ex_src.serialized_value = strdup("raw_ex");
  ex_src.extensions_json = strdup("{\"x-e\":1}");

  rc = copy_example_fields(&ex_dst, &ex_src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("ex_name", ex_dst.name);
  ASSERT_STR_EQ("ex summary", ex_dst.summary);
  ASSERT_STR_EQ("ex description", ex_dst.description);
  ASSERT_STR_EQ("http://ex.json", ex_dst.external_value);

  free_example(&ex_src);
  free_example(&ex_dst);
  free_example(NULL);

  PASS();
}

/**
 * @brief Unit test for component copy OOM branches.
 * @return Greatest test result.
 */
TEST test_openapi_copy_components_oom(void) {
  int k;
  struct OpenAPI_Parameter p_src, p_dst;
  struct OpenAPI_Header h_src, h_dst;
  struct OpenAPI_Encoding enc_src, enc_dst;
  struct OpenAPI_Response r_src, r_dst;
  struct OpenAPI_Example ex_src, ex_dst;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  for (k = 1; k <= 8; ++k) {
    memset(&p_src, 0, sizeof(p_src));
    memset(&p_dst, 0, sizeof(p_dst));
    p_src.name = cdd_test_mut_str("p");
    p_src.description = cdd_test_mut_str("d");
    p_src.content_type = cdd_test_mut_str("t");
    p_src.content_ref = cdd_test_mut_str("r");
    p_src.items_type = cdd_test_mut_str("i");
    g_cdd_alloc_fail = k;
    rc = copy_parameter_fields(&p_dst, &p_src);
    g_cdd_alloc_fail = 0;
    if (rc != CDD_C_SUCCESS) {
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    free_parameter(&p_dst);
  }

  for (k = 1; k <= 8; ++k) {
    memset(&h_src, 0, sizeof(h_src));
    memset(&h_dst, 0, sizeof(h_dst));
    h_src.description = cdd_test_mut_str("d");
    h_src.content_type = cdd_test_mut_str("t");
    h_src.content_ref = cdd_test_mut_str("r");
    h_src.items_type = cdd_test_mut_str("i");
    g_cdd_alloc_fail = k;
    rc = copy_header_fields(&h_dst, &h_src);
    g_cdd_alloc_fail = 0;
    if (rc != CDD_C_SUCCESS) {
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    free_header(&h_dst);
  }

  for (k = 1; k <= 6; ++k) {
    memset(&enc_src, 0, sizeof(enc_src));
    memset(&enc_dst, 0, sizeof(enc_dst));
    enc_src.name = cdd_test_mut_str("n");
    enc_src.content_type = cdd_test_mut_str("t");
    g_cdd_alloc_fail = k;
    rc = copy_encoding_fields(&enc_dst, &enc_src);
    g_cdd_alloc_fail = 0;
    if (rc != CDD_C_SUCCESS) {
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    free_encoding(&enc_dst);
  }

  for (k = 1; k <= 6; ++k) {
    memset(&r_src, 0, sizeof(r_src));
    memset(&r_dst, 0, sizeof(r_dst));
    r_src.code = cdd_test_mut_str("200");
    r_src.description = cdd_test_mut_str("d");
    g_cdd_alloc_fail = k;
    rc = copy_response_fields(&r_dst, &r_src);
    g_cdd_alloc_fail = 0;
    if (rc != CDD_C_SUCCESS) {
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    free_response(&r_dst);
  }

  for (k = 1; k <= 6; ++k) {
    memset(&ex_src, 0, sizeof(ex_src));
    memset(&ex_dst, 0, sizeof(ex_dst));
    ex_src.name = cdd_test_mut_str("n");
    ex_src.summary = cdd_test_mut_str("s");
    ex_src.description = cdd_test_mut_str("d");
    ex_src.external_value = cdd_test_mut_str("ev");
    ex_src.serialized_value = cdd_test_mut_str("sv");
    g_cdd_alloc_fail = k;
    rc = copy_example_fields(&ex_dst, &ex_src);
    g_cdd_alloc_fail = 0;
    if (rc != CDD_C_SUCCESS) {
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    free_example(&ex_dst);
  }

  g_cdd_alloc_fail = 0;
  PASS();
}

/**
 * @brief Unit test for focused sub-object failure branches in
 * openapi_copy_components.c.
 * @return Greatest test result.
 */
TEST test_openapi_copy_components_subobject_failures(void) {
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
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_header_fields(&th_dst, &th_src));
  g_cdd_strdup_fail = 0;
  free_header(&th_src);
  free_header(&th_dst);

  /* 2. Encoding headers name and fields copy failure */
  memset(&te_src, 0, sizeof(te_src));
  memset(&te_dst, 0, sizeof(te_dst));
  te_src.n_headers = 1;
  te_src.headers =
      (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
  te_src.headers[0].name = strdup("n");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_encoding_fields(&te_dst, &te_src));
  g_cdd_strdup_fail = 0;
  free_encoding(&te_src);
  free_encoding(&te_dst);

  memset(&te_src, 0, sizeof(te_src));
  memset(&te_dst, 0, sizeof(te_dst));
  te_src.n_headers = 1;
  te_src.headers =
      (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
  te_src.headers[0].name = strdup("n");
  te_src.headers[0].description = strdup("d");
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_encoding_fields(&te_dst, &te_src));
  g_cdd_strdup_fail = 0;
  free_encoding(&te_src);
  free_encoding(&te_dst);

  /* 3. Encoding nested encoding copy failure */
  memset(&te_src, 0, sizeof(te_src));
  memset(&te_dst, 0, sizeof(te_dst));
  te_src.n_encoding = 1;
  te_src.encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  te_src.encoding[0].content_type = strdup("c");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_encoding_fields(&te_dst, &te_src));
  g_cdd_strdup_fail = 0;
  free_encoding(&te_src);
  free_encoding(&te_dst);

  /* 4. Encoding prefix_encoding copy failure */
  memset(&te_src, 0, sizeof(te_src));
  memset(&te_dst, 0, sizeof(te_dst));
  te_src.n_prefix_encoding = 1;
  te_src.prefix_encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  te_src.prefix_encoding[0].content_type = strdup("c");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_encoding_fields(&te_dst, &te_src));
  g_cdd_strdup_fail = 0;
  free_encoding(&te_src);
  free_encoding(&te_dst);

  /* 5. Encoding item_encoding copy failure */
  memset(&te_src, 0, sizeof(te_src));
  memset(&te_dst, 0, sizeof(te_dst));
  te_src.item_encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  te_src.item_encoding->content_type = strdup("c");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_encoding_fields(&te_dst, &te_src));
  g_cdd_strdup_fail = 0;
  free_encoding(&te_src);
  free_encoding(&te_dst);

  /* 6. Media type nested encoding copy failure */
  memset(&tm_src, 0, sizeof(tm_src));
  memset(&tm_dst, 0, sizeof(tm_dst));
  tm_src.n_encoding = 1;
  tm_src.encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  tm_src.encoding[0].content_type = strdup("c");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_media_type_fields(&tm_dst, &tm_src));
  g_cdd_strdup_fail = 0;
  free_media_type(&tm_src);
  free_media_type(&tm_dst);

  /* 7. Media type prefix_encoding copy failure */
  memset(&tm_src, 0, sizeof(tm_src));
  memset(&tm_dst, 0, sizeof(tm_dst));
  tm_src.n_prefix_encoding = 1;
  tm_src.prefix_encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  tm_src.prefix_encoding[0].content_type = strdup("c");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_media_type_fields(&tm_dst, &tm_src));
  g_cdd_strdup_fail = 0;
  free_media_type(&tm_src);
  free_media_type(&tm_dst);

  /* 8. Media type item_encoding copy failure */
  memset(&tm_src, 0, sizeof(tm_src));
  memset(&tm_dst, 0, sizeof(tm_dst));
  tm_src.item_encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  tm_src.item_encoding->content_type = strdup("c");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_media_type_fields(&tm_dst, &tm_src));
  g_cdd_strdup_fail = 0;
  free_media_type(&tm_src);
  free_media_type(&tm_dst);

  /* 9. Response headers copy failure */
  memset(&tr_src, 0, sizeof(tr_src));
  memset(&tr_dst, 0, sizeof(tr_dst));
  tr_src.n_headers = 1;
  tr_src.headers =
      (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
  tr_src.headers[0].description = strdup("d");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_response_fields(&tr_dst, &tr_src));
  g_cdd_strdup_fail = 0;
  free_response(&tr_src);
  free_response(&tr_dst);

  /* 10. Response links copy failure */
  memset(&tr_src, 0, sizeof(tr_src));
  memset(&tr_dst, 0, sizeof(tr_dst));
  tr_src.n_links = 1;
  tr_src.links = (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
  tr_src.links[0].summary = strdup("s");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_response_fields(&tr_dst, &tr_src));
  g_cdd_strdup_fail = 0;
  free_response(&tr_src);
  free_response(&tr_dst);

  /* 11. Calloc failures across components */
  memset(&th_src, 0, sizeof(th_src));
  memset(&th_dst, 0, sizeof(th_dst));
  th_src.n_examples = 1;
  th_src.examples =
      (struct OpenAPI_Example *)calloc(1, sizeof(struct OpenAPI_Example));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_header_fields(&th_dst, &th_src));
  g_cdd_alloc_fail = 0;
  free_header(&th_src);
  free_header(&th_dst);

  memset(&te_src, 0, sizeof(te_src));
  memset(&te_dst, 0, sizeof(te_dst));
  te_src.n_headers = 1;
  te_src.headers =
      (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_encoding_fields(&te_dst, &te_src));
  g_cdd_alloc_fail = 0;
  free_encoding(&te_src);
  free_encoding(&te_dst);

  memset(&te_src, 0, sizeof(te_src));
  memset(&te_dst, 0, sizeof(te_dst));
  te_src.n_encoding = 1;
  te_src.encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_encoding_fields(&te_dst, &te_src));
  g_cdd_alloc_fail = 0;
  free_encoding(&te_src);
  free_encoding(&te_dst);

  memset(&te_src, 0, sizeof(te_src));
  memset(&te_dst, 0, sizeof(te_dst));
  te_src.n_prefix_encoding = 1;
  te_src.prefix_encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_encoding_fields(&te_dst, &te_src));
  g_cdd_alloc_fail = 0;
  free_encoding(&te_src);
  free_encoding(&te_dst);

  memset(&te_src, 0, sizeof(te_src));
  memset(&te_dst, 0, sizeof(te_dst));
  te_src.item_encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_encoding_fields(&te_dst, &te_src));
  g_cdd_alloc_fail = 0;
  free_encoding(&te_src);
  free_encoding(&te_dst);

  memset(&tm_src, 0, sizeof(tm_src));
  memset(&tm_dst, 0, sizeof(tm_dst));
  tm_src.n_examples = 1;
  tm_src.examples =
      (struct OpenAPI_Example *)calloc(1, sizeof(struct OpenAPI_Example));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_media_type_fields(&tm_dst, &tm_src));
  g_cdd_alloc_fail = 0;
  free_media_type(&tm_src);
  free_media_type(&tm_dst);

  memset(&tm_src, 0, sizeof(tm_src));
  memset(&tm_dst, 0, sizeof(tm_dst));
  tm_src.item_encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_media_type_fields(&tm_dst, &tm_src));
  g_cdd_alloc_fail = 0;
  free_media_type(&tm_src);
  free_media_type(&tm_dst);

  memset(&tm_src, 0, sizeof(tm_src));
  memset(&tm_dst, 0, sizeof(tm_dst));
  tm_src.item_schema_set = 1;
  tm_src.item_schema.inline_type = strdup("s");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_media_type_fields(&tm_dst, &tm_src));
  g_cdd_strdup_fail = 0;
  free_media_type(&tm_src);
  free_media_type(&tm_dst);

  memset(&tm_src, 0, sizeof(tm_src));
  memset(&tm_dst, 0, sizeof(tm_dst));
  tm_src.n_encoding = 1;
  tm_src.encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_media_type_fields(&tm_dst, &tm_src));
  g_cdd_alloc_fail = 0;
  free_media_type(&tm_src);
  free_media_type(&tm_dst);

  memset(&tm_src, 0, sizeof(tm_src));
  memset(&tm_dst, 0, sizeof(tm_dst));
  tm_src.n_prefix_encoding = 1;
  tm_src.prefix_encoding =
      (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_media_type_fields(&tm_dst, &tm_src));
  g_cdd_alloc_fail = 0;
  free_media_type(&tm_src);
  free_media_type(&tm_dst);

  memset(&tr_src, 0, sizeof(tr_src));
  memset(&tr_dst, 0, sizeof(tr_dst));
  tr_src.n_content_media_types = 1;
  tr_src.content_media_types =
      (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
  tr_src.content_media_types[0].name = strdup("s");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_response_fields(&tr_dst, &tr_src));
  g_cdd_alloc_fail = 0;
  free_response(&tr_src);
  free_response(&tr_dst);

  memset(&tr_src, 0, sizeof(tr_src));
  memset(&tr_dst, 0, sizeof(tr_dst));
  tr_src.n_links = 1;
  tr_src.links = (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
  tr_src.links[0].summary = strdup("s");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_response_fields(&tr_dst, &tr_src));
  g_cdd_strdup_fail = 0;
  free_response(&tr_src);
  free_response(&tr_dst);

  memset(&tr_src, 0, sizeof(tr_src));
  memset(&tr_dst, 0, sizeof(tr_dst));
  tr_src.n_examples = 1;
  tr_src.examples =
      (struct OpenAPI_Example *)calloc(1, sizeof(struct OpenAPI_Example));
  tr_src.examples[0].name = strdup("n");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_response_fields(&tr_dst, &tr_src));
  g_cdd_strdup_fail = 0;
  free_response(&tr_src);
  free_response(&tr_dst);

  memset(&tr_src, 0, sizeof(tr_src));
  memset(&tr_dst, 0, sizeof(tr_dst));
  tr_src.n_headers = 1;
  tr_src.headers =
      (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
  tr_src.headers[0].name = strdup("n");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_response_fields(&tr_dst, &tr_src));
  g_cdd_strdup_fail = 0;
  free_response(&tr_src);
  free_response(&tr_dst);

  memset(&tr_src, 0, sizeof(tr_src));
  memset(&tr_dst, 0, sizeof(tr_dst));
  tr_src.n_links = 1;
  tr_src.links = (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
  tr_src.links[0].name = strdup("n");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_response_fields(&tr_dst, &tr_src));
  g_cdd_strdup_fail = 0;
  free_response(&tr_src);
  free_response(&tr_dst);

  /* Test content_ref copy failure in response */
  memset(&tr_src, 0, sizeof(tr_src));
  memset(&tr_dst, 0, sizeof(tr_dst));
  tr_src.content_ref = strdup("#/components/mediaTypes/Mt");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_response_fields(&tr_dst, &tr_src));
  g_cdd_strdup_fail = 0;
  free_response(&tr_src);
  free_response(&tr_dst);

  /* Test link ref copy failure in response */
  memset(&tr_src, 0, sizeof(tr_src));
  memset(&tr_dst, 0, sizeof(tr_dst));
  tr_src.n_links = 1;
  tr_src.links = (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
  tr_src.links[0].ref = strdup("#/components/links/L1");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_response_fields(&tr_dst, &tr_src));
  g_cdd_strdup_fail = 0;
  free_response(&tr_src);
  free_response(&tr_dst);

  memset(&tr_src, 0, sizeof(tr_src));
  memset(&tr_dst, 0, sizeof(tr_dst));
  tr_src.n_examples = 1;
  tr_src.examples =
      (struct OpenAPI_Example *)calloc(1, sizeof(struct OpenAPI_Example));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_response_fields(&tr_dst, &tr_src));
  g_cdd_alloc_fail = 0;
  free_response(&tr_src);
  free_response(&tr_dst);

  memset(&tr_src, 0, sizeof(tr_src));
  memset(&tr_dst, 0, sizeof(tr_dst));
  tr_src.n_headers = 1;
  tr_src.headers =
      (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_response_fields(&tr_dst, &tr_src));
  g_cdd_alloc_fail = 0;
  free_response(&tr_src);
  free_response(&tr_dst);

  memset(&tr_src, 0, sizeof(tr_src));
  memset(&tr_dst, 0, sizeof(tr_dst));
  tr_src.n_links = 1;
  tr_src.links = (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_response_fields(&tr_dst, &tr_src));
  g_cdd_alloc_fail = 0;
  free_response(&tr_src);
  free_response(&tr_dst);

  PASS();
}

/**
 * @brief Unit test for edge-case branch coverage in openapi_copy_components.c.
 * @return Greatest test result.
 */
TEST test_openapi_copy_components_branches(void) {
  struct OpenAPI_Parameter param_src, param_dst;
  struct OpenAPI_Header hdr_src, hdr_dst;
  struct OpenAPI_Encoding enc_src, enc_dst;
  struct OpenAPI_MediaType mt_src, mt_dst;
  struct OpenAPI_Response resp_src, resp_dst;

  struct OpenAPI_MediaType dummy_mt;
  struct OpenAPI_Example dummy_ex;
  struct OpenAPI_Header dummy_hdr;
  struct OpenAPI_Encoding dummy_enc;
  struct OpenAPI_Link dummy_lnk;
  char dummy_str_buf[8];
  char *dummy_str;

  dummy_str_buf[0] = 'd';
  dummy_str_buf[1] = '\0';
  dummy_str = dummy_str_buf;

  memset(&dummy_mt, 0, sizeof(dummy_mt));
  memset(&dummy_ex, 0, sizeof(dummy_ex));
  memset(&dummy_hdr, 0, sizeof(dummy_hdr));
  memset(&dummy_enc, 0, sizeof(dummy_enc));
  memset(&dummy_lnk, 0, sizeof(dummy_lnk));

  /* 1. copy_parameter_fields & copy_header_fields with non-null pointer, n=0 */
  memset(&param_src, 0, sizeof(param_src));
  memset(&param_dst, 0, sizeof(param_dst));
  param_src.content_media_types = &dummy_mt;
  param_src.n_content_media_types = 0;
  param_src.examples = &dummy_ex;
  param_src.n_examples = 0;
  ASSERT_EQ(CDD_C_SUCCESS, copy_parameter_fields(&param_dst, &param_src));

  memset(&hdr_src, 0, sizeof(hdr_src));
  memset(&hdr_dst, 0, sizeof(hdr_dst));
  hdr_src.content_media_types = &dummy_mt;
  hdr_src.n_content_media_types = 0;
  hdr_src.examples = &dummy_ex;
  hdr_src.n_examples = 0;
  ASSERT_EQ(CDD_C_SUCCESS, copy_header_fields(&hdr_dst, &hdr_src));

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
  ASSERT_EQ(CDD_C_SUCCESS, copy_encoding_fields(&enc_dst, &enc_src));
  free_encoding(&enc_dst);

  memset(&enc_src, 0, sizeof(enc_src));
  memset(&enc_dst, 0, sizeof(enc_dst));
  enc_src.headers = &dummy_hdr;
  enc_src.n_headers = 1;
  dummy_hdr.name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, copy_encoding_fields(&enc_dst, &enc_src));
  free_encoding(&enc_dst);

  memset(&enc_src, 0, sizeof(enc_src));
  memset(&enc_dst, 0, sizeof(enc_dst));
  enc_src.n_encoding = 1;
  enc_src.encoding = &dummy_enc;
  ASSERT_EQ(CDD_C_SUCCESS, copy_encoding_fields(&enc_dst, &enc_src));
  free_encoding(&enc_dst);

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
  ASSERT_EQ(CDD_C_SUCCESS, copy_media_type_fields(&mt_dst, &mt_src));
  free_media_type(&mt_dst);

  memset(&mt_src, 0, sizeof(mt_src));
  memset(&mt_dst, 0, sizeof(mt_dst));
  mt_src.schema.ref_name = dummy_str;
  mt_src.item_schema.ref_name = dummy_str;
  ASSERT_EQ(CDD_C_SUCCESS, copy_media_type_fields(&mt_dst, &mt_src));
  free_media_type(&mt_dst);

  memset(&mt_src, 0, sizeof(mt_src));
  memset(&mt_dst, 0, sizeof(mt_dst));
  mt_src.schema.n_multipart_fields = 1;
  mt_src.item_schema.n_multipart_fields = 1;
  ASSERT_EQ(CDD_C_SUCCESS, copy_media_type_fields(&mt_dst, &mt_src));
  free_media_type(&mt_dst);

  memset(&mt_src, 0, sizeof(mt_src));
  memset(&mt_dst, 0, sizeof(mt_dst));
  mt_src.schema.inline_type = dummy_str;
  mt_src.item_schema.inline_type = dummy_str;
  ASSERT_EQ(CDD_C_SUCCESS, copy_media_type_fields(&mt_dst, &mt_src));
  free_media_type(&mt_dst);

  /* 4. copy_media_type_array NULL checks */
  {
    struct OpenAPI_MediaType *mt_arr = NULL;
    size_t mt_cnt = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              copy_media_type_array(&mt_arr, NULL, &dummy_mt, 1));
    ASSERT_EQ(CDD_C_SUCCESS,
              copy_media_type_array(&mt_arr, &mt_cnt, &dummy_mt, 0));
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
  ASSERT_EQ(CDD_C_SUCCESS, copy_response_fields(&resp_dst, &resp_src));

  memset(&resp_src, 0, sizeof(resp_src));
  memset(&resp_dst, 0, sizeof(resp_dst));
  resp_src.content_ref = dummy_str;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_response_fields(&resp_dst, &resp_src));
  g_cdd_strdup_fail = 0;

  memset(&resp_src, 0, sizeof(resp_src));
  memset(&resp_dst, 0, sizeof(resp_dst));
  resp_src.content_ref = dummy_str;
  ASSERT_EQ(CDD_C_SUCCESS, copy_response_fields(&resp_dst, &resp_src));
  free_response(&resp_dst);

  memset(&resp_src, 0, sizeof(resp_src));
  memset(&resp_dst, 0, sizeof(resp_dst));
  resp_src.examples = &dummy_ex;
  resp_src.n_examples = 1;
  ASSERT_EQ(CDD_C_SUCCESS, copy_response_fields(&resp_dst, &resp_src));
  free_response(&resp_dst);

  memset(&resp_src, 0, sizeof(resp_src));
  memset(&resp_dst, 0, sizeof(resp_dst));
  resp_src.headers = &dummy_hdr;
  resp_src.n_headers = 1;
  dummy_hdr.name = NULL;
  resp_src.links = &dummy_lnk;
  resp_src.n_links = 1;
  dummy_lnk.name = dummy_str;
  dummy_lnk.ref = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, copy_response_fields(&resp_dst, &resp_src));
  free_response(&resp_dst);

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
  ASSERT_EQ(CDD_C_SUCCESS, copy_response_fields(&resp_dst, &resp_src));
  free_response(&resp_dst);

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_COPY_COMPONENTS_H */
