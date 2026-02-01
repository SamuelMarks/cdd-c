/**
 * @file test_url_utils.h
 * @brief Unit tests for URL encoding and query building.
 *
 * Verifies RFC 3986 compliance for encoding and correct structural assembly
 * of query strings.
 *
 * @author Samuel Marks
 */

#ifndef TEST_URL_UTILS_H
#define TEST_URL_UTILS_H

#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "url_utils.h"

/* --- Encoding Tests --- */

TEST test_url_encode_simple(void) {
  char *res = url_encode("hello");
  ASSERT(res != NULL);
  ASSERT_STR_EQ("hello", res);
  free(res);
  PASS();
}

TEST test_url_encode_spaces(void) {
  char *res = url_encode("hello world");
  ASSERT(res != NULL);
  ASSERT_STR_EQ("hello%20world", res);
  free(res);
  PASS();
}

TEST test_url_encode_reserved(void) {
  /* Gen-delims: : / ? # [ ] @ */
  /* Sub-delims: ! $ & ' ( ) * + , ; = */
  /* These should ALL be encoded in component context (query param value) */
  const char *input = ":/?#[]@!$&'()*+,;=";
  const char *expected = "%3A%2F%3F%23%5B%5D%40%21%24%26%27%28%29%2A%2B%2C%3B%"
                         "3D";
  char *res = url_encode(input);

  ASSERT(res != NULL);
  ASSERT_STR_EQ(expected, res);
  free(res);
  PASS();
}

TEST test_url_encode_unreserved(void) {
  /* ALPHA, DIGIT, - . _ ~ */
  const char *input = "a-b.c_d~1";
  char *res = url_encode(input);
  ASSERT(res != NULL);
  ASSERT_STR_EQ("a-b.c_d~1", res);
  free(res);
  PASS();
}

TEST test_url_encode_null(void) {
  ASSERT(url_encode(NULL) == NULL);
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
  PASS();
}

TEST test_query_null_safety(void) {
  struct UrlQueryParams qp;
  char *res = NULL;

  ASSERT_EQ(EINVAL, url_query_init(NULL));
  ASSERT_EQ(EINVAL, url_query_add(NULL, "k", "v"));

  url_query_init(&qp);
  ASSERT_EQ(EINVAL, url_query_add(&qp, NULL, "v"));
  ASSERT_EQ(EINVAL, url_query_add(&qp, "k", NULL));

  ASSERT_EQ(EINVAL, url_query_build(NULL, &res));
  ASSERT_EQ(EINVAL, url_query_build(&qp, NULL));

  url_query_free(NULL); /* Safe */
  url_query_free(&qp);

  PASS();
}

SUITE(url_utils_suite) {
  RUN_TEST(test_url_encode_simple);
  RUN_TEST(test_url_encode_spaces);
  RUN_TEST(test_url_encode_reserved);
  RUN_TEST(test_url_encode_unreserved);
  RUN_TEST(test_url_encode_null);

  RUN_TEST(test_query_lifecycle);
  RUN_TEST(test_query_build_empty);
  RUN_TEST(test_query_build_single);
  RUN_TEST(test_query_build_multiple);
  RUN_TEST(test_query_build_encoding_keys);
  RUN_TEST(test_query_null_safety);
}

#endif /* TEST_URL_UTILS_H */
