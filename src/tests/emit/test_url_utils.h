/**
 * @file test_url_utils.h
 * @brief Unit tests for URL encoding and query building.
 */

#ifndef TEST_URL_UTILS_H
#define TEST_URL_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

extern int g_io_calls;
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_cdd_strdup_fail;

extern enum cdd_c_error is_pct_encoded_test(const char *p);
extern enum cdd_c_error kv_value_to_string_test(const struct OpenAPI_KV *kv,
                                                char *buf, size_t buf_len,
                                                const char **_out_val);
extern enum cdd_c_error append_str_test(char **buf, size_t *len, size_t *cap,
                                        const char *s);

/* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "routes/parse/url.h"
/* clang-format on */

/* --- Encoding Tests --- */

TEST test_url_encode_simple(void) {
  char *_ast_url_encode_0 = NULL;
  char *res = (url_encode("hello", &_ast_url_encode_0), _ast_url_encode_0);
  ASSERT(res != NULL);
  ASSERT_STR_EQ("hello", res);
  free(res);
  g_fail_io_after = -1;
  PASS();
}

TEST test_url_encode_spaces(void) {
  char *_ast_url_encode_1 = NULL;
  char *res =
      (url_encode("hello world", &_ast_url_encode_1), _ast_url_encode_1);
  ASSERT(res != NULL);
  ASSERT_STR_EQ("hello%20world", res);
  free(res);
  g_fail_io_after = -1;
  PASS();
}

TEST test_url_encode_reserved(void) {
  char *_ast_url_encode_2 = NULL;
  /* Gen-delims: : / ? # [ ] @ */
  /* Sub-delims: ! $ & ' ( ) * + , ; = */
  /* These should ALL be encoded in component context (query param value) */
  const char *input = ":/?#[]@!$&'()*+,;=";
  const char *expected = "%3A%2F%3F%23%5B%5D%40%21%24%26%27%28%29%2A%2B%2C%3B%"
                         "3D";
  char *res = (url_encode(input, &_ast_url_encode_2), _ast_url_encode_2);

  ASSERT(res != NULL);
  ASSERT_STR_EQ(expected, res);
  free(res);
  g_fail_io_after = -1;
  PASS();
}

TEST test_url_encode_unreserved(void) {
  char *_ast_url_encode_3 = NULL;
  /* ALPHA, DIGIT, - . _ ~ */
  const char *input = "a-b.c_d~1";
  char *res = (url_encode(input, &_ast_url_encode_3), _ast_url_encode_3);
  ASSERT(res != NULL);
  ASSERT_STR_EQ("a-b.c_d~1", res);
  free(res);
  g_fail_io_after = -1;
  PASS();
}

TEST test_url_encode_allow_reserved(void) {
  char *_ast_url_encode_allow_reserved_4 = NULL;
  const char *input = ":/?#[]@!$&'()*+,;= %2F";
  const char *expected = ":/?#[]@!$&'()*+,;=%20%2F";
  char *res =
      (url_encode_allow_reserved(input, &_ast_url_encode_allow_reserved_4),
       _ast_url_encode_allow_reserved_4);
  ASSERT(res != NULL);
  ASSERT_STR_EQ(expected, res);
  free(res);
  g_fail_io_after = -1;
  PASS();
}

TEST test_url_encode_form_spaces(void) {
  char *_ast_url_encode_form_5 = NULL;
  char *res = (url_encode_form("hello world", &_ast_url_encode_form_5),
               _ast_url_encode_form_5);
  ASSERT(res != NULL);
  ASSERT_STR_EQ("hello+world", res);
  free(res);
  g_fail_io_after = -1;
  PASS();
}

TEST test_url_encode_form_reserved(void) {
  char *_ast_url_encode_form_6 = NULL;
  const char *input = "&=+";
  char *res =
      (url_encode_form(input, &_ast_url_encode_form_6), _ast_url_encode_form_6);
  ASSERT(res != NULL);
  ASSERT_STR_EQ("%26%3D%2B", res);
  free(res);
  g_fail_io_after = -1;
  PASS();
}

TEST test_url_encode_form_allow_reserved(void) {
  char *_ast_url_encode_form_allow_reserved_7 = NULL;
  const char *input = ":/?#[]@!$&'()*+,;= %2F";
  const char *expected = ":/?#[]@!$%26'()*%2B,;%3D+%2F";
  char *res = (url_encode_form_allow_reserved(
                   input, &_ast_url_encode_form_allow_reserved_7),
               _ast_url_encode_form_allow_reserved_7);
  ASSERT(res != NULL);
  ASSERT_STR_EQ(expected, res);
  free(res);
  g_fail_io_after = -1;
  PASS();
}

TEST test_url_encode_null(void) {
  char *_ast_url_encode_8 = NULL;
  ASSERT((url_encode(NULL, &_ast_url_encode_8), _ast_url_encode_8) == NULL);
  g_fail_io_after = -1;
  PASS();
}

/* --- Query Builder Tests --- */

TEST test_query_lifecycle(void) {
  struct UrlQueryParams qp;
  int rc;

  rc = url_query_init(&qp);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, qp.count);

  rc = url_query_add(&qp, "key", "val");
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, qp.count);
  ASSERT_STR_EQ("key", qp.params[0].key);
  ASSERT_STR_EQ("val", qp.params[0].value);

  url_query_free(&qp);
  ASSERT_EQ(0, qp.count);
  ASSERT(qp.params == NULL);
  g_fail_io_after = -1;

  PASS();
}

TEST test_query_build_empty(void) {
  struct UrlQueryParams qp;
  char *res = NULL;

  url_query_init(&qp);
  ASSERT_EQ(0, url_query_build(&qp, &res));
  ASSERT(res != NULL);
  ASSERT_STR_EQ("", res);

  free(res);
  url_query_free(&qp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_query_build_single(void) {
  struct UrlQueryParams qp;
  char *res = NULL;

  url_query_init(&qp);
  url_query_add(&qp, "q", "hello world");

  ASSERT_EQ(0, url_query_build(&qp, &res));
  ASSERT(res != NULL);
  ASSERT_STR_EQ("?q=hello%20world", res);

  free(res);
  url_query_free(&qp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_query_build_multiple(void) {
  struct UrlQueryParams qp;
  char *res = NULL;

  url_query_init(&qp);
  url_query_add(&qp, "a", "1");
  url_query_add(&qp, "b", "2+2"); /* + encodes */

  ASSERT_EQ(0, url_query_build(&qp, &res));
  ASSERT(res != NULL);
  /* Order is preserved */
  ASSERT_STR_EQ("?a=1&b=2%2B2", res);

  free(res);
  url_query_free(&qp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_query_build_form_single(void) {
  struct UrlQueryParams qp;
  char *res = NULL;

  url_query_init(&qp);
  url_query_add(&qp, "q", "hello world");

  ASSERT_EQ(0, url_query_build_form(&qp, &res));
  ASSERT(res != NULL);
  ASSERT_STR_EQ("q=hello+world", res);

  free(res);
  url_query_free(&qp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_query_build_form_multiple(void) {
  struct UrlQueryParams qp;
  char *res = NULL;

  url_query_init(&qp);
  url_query_add(&qp, "a", "1");
  url_query_add(&qp, "b", "2+2");

  ASSERT_EQ(0, url_query_build_form(&qp, &res));
  ASSERT(res != NULL);
  ASSERT_STR_EQ("a=1&b=2%2B2", res);

  free(res);
  url_query_free(&qp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_query_build_form_preserves_encoded_value(void) {
  struct UrlQueryParams qp;
  char *res = NULL;

  url_query_init(&qp);
  url_query_add_encoded(&qp, "color", "blue,black");

  ASSERT_EQ(0, url_query_build_form(&qp, &res));
  ASSERT(res != NULL);
  ASSERT_STR_EQ("color=blue,black", res);

  free(res);
  url_query_free(&qp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_openapi_kv_join_form_comma(void) {
  char *_ast_openapi_kv_join_form_9 = NULL;
  struct OpenAPI_KV kvs[2];
  char *res;

  kvs[0].key = "R";
  kvs[0].type = OA_KV_INTEGER;
  kvs[0].value.i = 100;
  kvs[1].key = "G";
  kvs[1].type = OA_KV_INTEGER;
  kvs[1].value.i = 200;

  res = (openapi_kv_join_form(kvs, 2, ",", 0, &_ast_openapi_kv_join_form_9),
         _ast_openapi_kv_join_form_9);
  ASSERT(res != NULL);
  ASSERT_STR_EQ("R,100,G,200", res);
  free(res);
  g_fail_io_after = -1;
  PASS();
}

TEST test_openapi_kv_join_form_space(void) {
  char *_ast_openapi_kv_join_form_10 = NULL;
  struct OpenAPI_KV kvs[2];
  char *res;

  kvs[0].key = "alpha";
  kvs[0].type = OA_KV_STRING;
  kvs[0].value.s = "a b";
  kvs[1].key = "beta";
  kvs[1].type = OA_KV_STRING;
  kvs[1].value.s = "c";

  res = (openapi_kv_join_form(kvs, 2, "%20", 0, &_ast_openapi_kv_join_form_10),
         _ast_openapi_kv_join_form_10);
  ASSERT(res != NULL);
  ASSERT_STR_EQ("alpha%20a+b%20beta%20c", res);
  free(res);
  g_fail_io_after = -1;
  PASS();
}

TEST test_openapi_kv_join_form_pipe_allow_reserved(void) {
  char *_ast_openapi_kv_join_form_11 = NULL;
  struct OpenAPI_KV kvs[1];
  char *res;

  kvs[0].key = "path";
  kvs[0].type = OA_KV_STRING;
  kvs[0].value.s = "a/b";

  res = (openapi_kv_join_form(kvs, 1, "%7C", 1, &_ast_openapi_kv_join_form_11),
         _ast_openapi_kv_join_form_11);
  ASSERT(res != NULL);
  ASSERT_STR_EQ("path%7Ca/b", res);
  free(res);
  g_fail_io_after = -1;
  PASS();
}

TEST test_query_build_preserves_encoded_value(void) {
  struct UrlQueryParams qp;
  char *res = NULL;

  url_query_init(&qp);
  url_query_add_encoded(&qp, "color", "blue,black");
  url_query_add(&qp, "q", "hello world");

  ASSERT_EQ(0, url_query_build(&qp, &res));
  ASSERT(res != NULL);
  /* Comma should remain, while space in other param is encoded */
  ASSERT_STR_EQ("?color=blue,black&q=hello%20world", res);

  free(res);
  url_query_free(&qp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_query_build_encoding_keys(void) {
  struct UrlQueryParams qp;
  char *res = NULL;

  url_query_init(&qp);
  url_query_add(&qp, "user id", "100");

  ASSERT_EQ(0, url_query_build(&qp, &res));
  /* Key should be encoded too */
  ASSERT_STR_EQ("?user%20id=100", res);

  free(res);
  url_query_free(&qp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_query_null_safety(void) {
  struct UrlQueryParams qp;
  char *res = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, url_query_init(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, url_query_add(NULL, "k", "v"));

  url_query_init(&qp);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, url_query_add(&qp, NULL, "v"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, url_query_add(&qp, "k", NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, url_query_build(NULL, &res));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, url_query_build(&qp, NULL));

  url_query_free(NULL); /* Safe */
  url_query_free(&qp);
  g_fail_io_after = -1;

  PASS();
}

TEST test_url_utils_write_query_json_param(void) {
  FILE *fp;
  struct OpenAPI_Parameter p;
  memset(&p, 0, sizeof(p));

  fp = fopen("test_url_json.txt", "w");
  ASSERT(fp != NULL);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_query_json_param(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_query_json_param(fp, NULL));

  p.name = "test";
  p.type = "array";
  p.is_array = 1;
  p.items_type = NULL;
  p.schema.inline_type = NULL;

  p.name = "test";

  /* unsupported array */

  fclose(fp);
  remove("test_url_json.txt");
  g_fail_io_after = -1;

  PASS();
}

TEST test_url_encode_all_null(void) {
  char *res = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, url_encode(NULL, &res));
  ASSERT(res == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, url_encode_allow_reserved(NULL, &res));
  ASSERT(res == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, url_encode_form(NULL, &res));
  ASSERT(res == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, url_encode_form_allow_reserved(NULL, &res));
  ASSERT(res == NULL);
  PASS();
}

TEST test_url_encode_oom(void) {
  char *res = NULL;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, url_encode("hello world", &res));
  ASSERT(res == NULL);
  g_fail_io_after = -1;
  g_cdd_strdup_fail = -1;
  PASS();
}

TEST test_url_encode_allow_reserved_oom(void) {
  char *res = NULL;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, url_encode_allow_reserved("hello world", &res));
  ASSERT(res == NULL);
  g_fail_io_after = -1;
  g_cdd_strdup_fail = -1;
  PASS();
}

TEST test_url_encode_form_oom(void) {
  char *res = NULL;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, url_encode_form("hello world", &res));
  ASSERT(res == NULL);
  g_fail_io_after = -1;
  g_cdd_strdup_fail = -1;
  PASS();
}

TEST test_url_encode_form_allow_reserved_oom(void) {
  char *res = NULL;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, url_encode_form_allow_reserved("hello world", &res));
  ASSERT(res == NULL);
  g_fail_io_after = -1;
  g_cdd_strdup_fail = -1;
  PASS();
}

TEST test_url_encode_form_allow_reserved_pct(void) {
  char *res = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, url_encode_form_allow_reserved("a b<", &res));
  ASSERT(res != NULL);
  ASSERT_STR_EQ("a+b%3C", res);
  free(res);
  PASS();
}

TEST test_url_encode_unreserved_form_asterisk(void) {
  char *res = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, url_encode_form("a*", &res));
  ASSERT(res != NULL);
  ASSERT_STR_EQ("a*", res);
  free(res);
  PASS();
}

TEST test_url_query_add_encoded_null(void) {
  struct UrlQueryParams qp;
  url_query_init(&qp);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            url_query_add_encoded(NULL, "k", "v"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            url_query_add_encoded(&qp, NULL, "v"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            url_query_add_encoded(&qp, "k", NULL));
  url_query_free(&qp);
  PASS();
}

TEST test_url_query_add_oom(void) {
  struct UrlQueryParams qp;
  int i;
  for (i = 1; i <= 3; ++i) {
    url_query_init(&qp);
    g_io_calls = 0;
    g_fail_io_after = i;
    g_cdd_strdup_fail = i;
    url_query_add(&qp, "key", "value");
    url_query_free(&qp);
    g_fail_io_after = -1;
    g_cdd_strdup_fail = -1;
  }

  url_query_init(&qp);
  url_query_add(&qp, "k1", "v1");
  url_query_add(&qp, "k2", "v2");
  url_query_add(&qp, "k3", "v3");
  url_query_add(&qp, "k4", "v4");
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_cdd_strdup_fail = 0;
  url_query_add(&qp, "k5", "v5");
  g_fail_io_after = -1;
  g_cdd_strdup_fail = -1;
  url_query_free(&qp);
  PASS();
}

TEST test_url_query_add_encoded_oom(void) {
  struct UrlQueryParams qp;
  int i;
  for (i = 1; i <= 3; ++i) {
    url_query_init(&qp);
    g_io_calls = 0;
    g_fail_io_after = i;
    g_cdd_strdup_fail = i;
    url_query_add_encoded(&qp, "key", "value");
    url_query_free(&qp);
    g_fail_io_after = -1;
    g_cdd_strdup_fail = -1;
  }

  url_query_init(&qp);
  url_query_add_encoded(&qp, "k1", "v1");
  url_query_add_encoded(&qp, "k2", "v2");
  url_query_add_encoded(&qp, "k3", "v3");
  url_query_add_encoded(&qp, "k4", "v4");
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_cdd_strdup_fail = 0;
  url_query_add_encoded(&qp, "k5", "v5");
  g_fail_io_after = -1;
  g_cdd_strdup_fail = -1;
  url_query_free(&qp);
  PASS();
}

TEST test_url_query_build_oom(void) {
  struct UrlQueryParams qp;
  char *res = NULL;
  int i;

  url_query_init(&qp);
  url_query_add(&qp, "k1", "v1");
  url_query_add_encoded(&qp, "k2", "v2");

  for (i = 0; i < 20; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    g_cdd_strdup_fail = i;
    url_query_build(&qp, &res);
    if (res) {
      free(res);
      res = NULL;
    }
  }
  g_fail_io_after = -1;
  g_cdd_strdup_fail = -1;
  url_query_free(&qp);

  url_query_init(&qp);
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_cdd_strdup_fail = 0;
  url_query_build(&qp, &res);
  g_fail_io_after = -1;
  g_cdd_strdup_fail = -1;
  url_query_free(&qp);
  PASS();
}

TEST test_url_query_build_form_oom(void) {
  struct UrlQueryParams qp;
  char *res = NULL;
  int i;

  url_query_init(&qp);
  url_query_add(&qp, "k1", "v1");
  url_query_add_encoded(&qp, "k2", "v2");

  for (i = 0; i < 20; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    g_cdd_strdup_fail = i;
    url_query_build_form(&qp, &res);
    if (res) {
      free(res);
      res = NULL;
    }
  }
  g_fail_io_after = -1;
  g_cdd_strdup_fail = -1;
  url_query_free(&qp);

  url_query_init(&qp);
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_cdd_strdup_fail = 0;
  url_query_build_form(&qp, &res);
  g_fail_io_after = -1;
  g_cdd_strdup_fail = -1;
  url_query_free(&qp);
  PASS();
}

TEST test_url_query_build_form_null(void) {
  char *res = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, url_query_build_form(NULL, &res));
  PASS();
}

TEST test_openapi_kv_join_form_types(void) {
  struct OpenAPI_KV kvs[5];
  char *res = NULL;

  kvs[0].key = "B";
  kvs[0].type = OA_KV_BOOLEAN;
  kvs[0].value.b = 0;
  kvs[1].key = "F";
  kvs[1].type = OA_KV_NUMBER;
  kvs[1].value.n = 3.14;
  kvs[2].key = "S";
  kvs[2].type = OA_KV_STRING;
  kvs[2].value.s = "str";
  kvs[3].key = "U";
  kvs[3].type = 999;
  kvs[4].key = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_kv_join_form(kvs, 5, ",", 0, &res));
  ASSERT(res != NULL);
  ASSERT_STR_EQ("B,false,F,3.14,S,str", res);
  free(res);
  PASS();
}

TEST test_openapi_kv_join_form_null(void) {
  char *res = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_kv_join_form(NULL, 0, ",", 0, &res));
  ASSERT(res != NULL);
  ASSERT_STR_EQ("", res);
  free(res);
  PASS();
}

TEST test_openapi_kv_join_form_delim_null(void) {
  struct OpenAPI_KV kvs[1];
  char *res = NULL;
  kvs[0].key = "k";
  kvs[0].type = OA_KV_STRING;
  kvs[0].value.s = "v";
  ASSERT_EQ(CDD_C_SUCCESS, openapi_kv_join_form(kvs, 1, NULL, 0, &res));
  ASSERT(res != NULL);
  ASSERT_STR_EQ("k,v", res);
  free(res);
  PASS();
}

TEST test_openapi_kv_join_form_oom(void) {
  struct OpenAPI_KV kvs[2];
  char *res = NULL;
  int i;

  kvs[0].key = "S1";
  kvs[0].type = OA_KV_STRING;
  kvs[0].value.s =
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  kvs[1].key =
      "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
      "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC";
  kvs[1].type = OA_KV_STRING;
  kvs[1].value.s = "BBBBBBBBBB";

  for (i = 0; i < 20; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    g_cdd_strdup_fail = i;
    openapi_kv_join_form(kvs, 2, ",", 0, &res);
    if (res) {
      free(res);
      res = NULL;
    }
  }
  g_fail_io_after = -1;
  g_cdd_strdup_fail = -1;

  g_io_calls = 0;
  g_fail_io_after = 0;
  g_cdd_strdup_fail = 0;
  openapi_kv_join_form(NULL, 0, ",", 0, &res);
  g_fail_io_after = -1;
  g_cdd_strdup_fail = -1;
  if (res) {
    free(res);
    res = NULL;
  }
  PASS();
}

TEST test_openapi_kv_join_form_large_string(void) {
  struct OpenAPI_KV kvs[1];
  char *res = NULL;
  kvs[0].key = "large";
  kvs[0].type = OA_KV_STRING;
  kvs[0].value.s =
      "very_large_string_that_exceeds_sixty_four_bytes_to_trigger_the_while_"
      "loop_in_append_str_for_realloc_capacity_doubling_very_large_indeed";
  ASSERT_EQ(CDD_C_SUCCESS, openapi_kv_join_form(kvs, 1, ",", 0, &res));
  ASSERT(res != NULL);
  free(res);
  PASS();
}

TEST test_kv_value_to_string_null(void) {
  struct OpenAPI_KV kv;
  char *res = NULL;
  char buf[32];
  kv.key = "x";
  kv.type = OA_KV_STRING;
  kv.value.s = "y";
  ASSERT_EQ(CDD_C_SUCCESS,
            kv_value_to_string_test(NULL, NULL, 0, (const char **)&res));
  ASSERT_EQ(CDD_C_SUCCESS,
            kv_value_to_string_test(&kv, NULL, 0, (const char **)&res));
  kv.type = OA_KV_NUMBER;
  ASSERT_EQ(CDD_C_SUCCESS,
            kv_value_to_string_test(&kv, NULL, 0, (const char **)&res));
  ASSERT_EQ(CDD_C_SUCCESS,
            kv_value_to_string_test(&kv, buf, 0, (const char **)&res));
  kv.type = OA_KV_INTEGER;
  ASSERT_EQ(CDD_C_SUCCESS,
            kv_value_to_string_test(&kv, buf, 0, (const char **)&res));
  kv.type = OA_KV_BOOLEAN;
  kv.value.b = 1;
  ASSERT_EQ(CDD_C_SUCCESS, kv_value_to_string_test(&kv, buf, sizeof(buf),
                                                   (const char **)&res));
  ASSERT_EQ(CDD_C_SUCCESS,
            kv_value_to_string_test(&kv, buf, 0, (const char **)&res));
  kv.type = OA_KV_INTEGER;
  ASSERT_EQ(CDD_C_SUCCESS,
            kv_value_to_string_test(&kv, buf, 0, (const char **)&res));
  PASS();
}

TEST test_append_str_nulls(void) {
  char *buf = NULL;
  size_t len = 0;
  size_t cap = 0;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            append_str_test(NULL, &len, &cap, "s"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            append_str_test(&buf, NULL, &cap, "s"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            append_str_test(&buf, &len, NULL, "s"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            append_str_test(&buf, &len, &cap, NULL));
  PASS();
}

TEST test_is_pct_encoded_null(void) {
  ASSERT_EQ(CDD_C_SUCCESS, is_pct_encoded_test(NULL));
  PASS();
}

TEST test_openapi_kv_join_form_all_skipped(void) {
  struct OpenAPI_KV kvs[1];
  char *res = NULL;
  kvs[0].key = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_kv_join_form(kvs, 1, ",", 0, &res));
  ASSERT(res != NULL);
  ASSERT_STR_EQ("", res);
  free(res);
  PASS();
}

TEST test_url_query_build_form_empty(void) {
  struct UrlQueryParams qp;
  char *res = NULL;
  url_query_init(&qp);
  ASSERT_EQ(CDD_C_SUCCESS, url_query_build_form(&qp, &res));
  ASSERT(res != NULL);
  ASSERT_STR_EQ("", res);
  free(res);
  url_query_free(&qp);
  PASS();
}

TEST test_openapi_kv_join_form_all_skipped_oom(void) {
  struct OpenAPI_KV kvs[1];
  char *res = NULL;
  kvs[0].key = NULL;
  g_io_calls = 0;
  g_fail_io_after = 0;
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_kv_join_form(kvs, 1, ",", 0, &res));
  g_fail_io_after = -1;
  g_cdd_strdup_fail = -1;
  PASS();
}

SUITE(url_utils_suite) {

  RUN_TEST(test_url_encode_all_null);
  RUN_TEST(test_url_encode_oom);
  RUN_TEST(test_url_encode_allow_reserved_oom);
  RUN_TEST(test_url_encode_form_oom);
  RUN_TEST(test_url_encode_form_allow_reserved_oom);
  RUN_TEST(test_url_encode_form_allow_reserved_pct);
  RUN_TEST(test_url_encode_unreserved_form_asterisk);
  RUN_TEST(test_url_query_add_encoded_null);
  RUN_TEST(test_url_query_add_oom);
  RUN_TEST(test_url_query_add_encoded_oom);
  RUN_TEST(test_url_query_build_oom);
  RUN_TEST(test_url_query_build_form_oom);
  RUN_TEST(test_url_query_build_form_null);
  RUN_TEST(test_openapi_kv_join_form_types);
  RUN_TEST(test_openapi_kv_join_form_null);
  RUN_TEST(test_openapi_kv_join_form_delim_null);
  RUN_TEST(test_openapi_kv_join_form_oom);
  RUN_TEST(test_openapi_kv_join_form_large_string);
  RUN_TEST(test_openapi_kv_join_form_all_skipped);
  RUN_TEST(test_openapi_kv_join_form_all_skipped_oom);
  RUN_TEST(test_url_query_build_form_empty);
  RUN_TEST(test_kv_value_to_string_null);
  RUN_TEST(test_append_str_nulls);
  RUN_TEST(test_is_pct_encoded_null);

  RUN_TEST(test_url_encode_simple);
  RUN_TEST(test_url_encode_spaces);
  RUN_TEST(test_url_encode_reserved);
  RUN_TEST(test_url_encode_unreserved);
  RUN_TEST(test_url_encode_allow_reserved);
  RUN_TEST(test_url_encode_form_spaces);
  RUN_TEST(test_url_encode_form_reserved);
  RUN_TEST(test_url_encode_form_allow_reserved);
  RUN_TEST(test_url_encode_null);

  RUN_TEST(test_query_lifecycle);
  RUN_TEST(test_query_build_empty);
  RUN_TEST(test_query_build_single);
  RUN_TEST(test_query_build_multiple);
  RUN_TEST(test_query_build_form_single);
  RUN_TEST(test_query_build_form_multiple);
  RUN_TEST(test_query_build_form_preserves_encoded_value);
  RUN_TEST(test_openapi_kv_join_form_comma);
  RUN_TEST(test_openapi_kv_join_form_space);
  RUN_TEST(test_openapi_kv_join_form_pipe_allow_reserved);
  RUN_TEST(test_query_build_preserves_encoded_value);
  RUN_TEST(test_query_build_encoding_keys);
  RUN_TEST(test_query_null_safety);
  RUN_TEST(test_url_utils_write_query_json_param);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_URL_UTILS_H */
