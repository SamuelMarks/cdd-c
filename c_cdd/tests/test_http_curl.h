/**
 * @file test_http_curl.h
 * @brief Integration tests for Libcurl Backend.
 *
 * Verifies that the Curl wrapper correctly initializes, handles configuration,
 * sends requests, and maps failures to errno.
 *
 * @author Samuel Marks
 */

#ifndef TEST_HTTP_CURL_H
#define TEST_HTTP_CURL_H

#include <errno.h>
#include <greatest.h>
#include <stdio.h>
#include <string.h>

#include "http_curl.h"
#include "http_types.h"
#include "str_utils.h"

/* Helper: Build a request to localhost on a port likely to be closed */
static int setup_request(struct HttpRequest *req, int port) {
  int rc;
  char url[64];

  rc = http_request_init(req);
  if (rc != 0)
    return rc;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(url, sizeof(url), "http://127.0.0.1:%d/test", port);
#else
  sprintf(url, "http://127.0.0.1:%d/test", port);
#endif

  req->url = c_cdd_strdup(url);
  return 0;
}

TEST test_curl_global_lifecycle(void) {
  /* Should succeed and track ref count internally */
  ASSERT_EQ(0, http_curl_global_init());
  ASSERT_EQ(0, http_curl_global_init()); /* Re-entrant check */

  http_curl_global_cleanup();
  http_curl_global_cleanup();
  PASS();
}

TEST test_curl_context_lifecycle(void) {
  struct HttpTransportContext *ctx = NULL;
  int rc;

  http_curl_global_init();

  rc = http_curl_context_init(&ctx);
  ASSERT_EQ(0, rc);
  ASSERT(ctx != NULL);

  http_curl_context_free(ctx);
  /* Double free safety check (should be safe with NULL) */
  http_curl_context_free(NULL);

  http_curl_global_cleanup();
  PASS();
}

TEST test_curl_config_application(void) {
  struct HttpTransportContext *ctx = NULL;
  struct HttpConfig config;
  int rc;

  http_curl_global_init();
  http_curl_context_init(&ctx);
  http_config_init(&config);

  /* Set some values */
  config.timeout_ms = 500;
  config.verify_peer = 0; /* Insecure for testing logic */

  rc = http_curl_config_apply(ctx, &config);
  ASSERT_EQ(0, rc);

  http_config_free(&config);
  http_curl_context_free(ctx);
  http_curl_global_cleanup();
  PASS();
}

TEST test_curl_send_connection_failure(void) {
  /* Expect mapped error (ECONNREFUSED or ETIMEDOUT or EHOSTUNREACH) */
  struct HttpTransportContext *ctx = NULL;
  struct HttpRequest req;
  struct HttpResponse *res = NULL;
  struct HttpConfig config;
  int rc;

  http_curl_global_init();
  http_curl_context_init(&ctx);
  http_config_init(&config);

  /* Fast timeout for test speed */
  config.timeout_ms = 50;
  http_curl_config_apply(ctx, &config);

  /* Use an invalid port */
  setup_request(&req, 59999);

  rc = http_curl_send(ctx, &req, &res);

  /* Verify mapping logic.
     Note: On some systems connection refused happens instanly (ECONNREFUSED),
     on others it times out (ETIMEDOUT). Both are valid error mappings
     for this test. */
  if (rc != ECONNREFUSED && rc != ETIMEDOUT && rc != EHOSTUNREACH &&
      rc != EIO) {
    fprintf(stderr, "Unexpected return code: %d (%s)\n", rc, strerror(rc));
    FAIL();
  }

  ASSERT(res == NULL); /* Should not be allocated on failure */

  http_config_free(&config);
  http_request_free(&req);
  http_curl_context_free(ctx);
  http_curl_global_cleanup();
  PASS();
}

TEST test_curl_send_invalid_arguments(void) {
  struct HttpTransportContext *ctx = NULL;
  struct HttpResponse *res = NULL;
  struct HttpRequest req;

  http_curl_global_init();
  http_curl_context_init(&ctx);
  http_request_init(&req);

  /* NULL ctx */
  ASSERT_EQ(EINVAL, http_curl_send(NULL, &req, &res));

  /* NULL req */
  ASSERT_EQ(EINVAL, http_curl_send(ctx, NULL, &res));

  /* NULL res pointer */
  ASSERT_EQ(EINVAL, http_curl_send(ctx, &req, NULL));

  /* NULL internal config application */
  ASSERT_EQ(EINVAL, http_curl_config_apply(NULL, NULL));
  ASSERT_EQ(EINVAL, http_curl_config_apply(ctx, NULL));

  http_request_free(&req);
  http_curl_context_free(ctx);
  http_curl_global_cleanup();
  PASS();
}

/*
 * NOTE: Testing a successful request requires a running server.
 * We skip strictly specific success tests here (mocking/stubbing libcurl
 * internals requires complex LD_PRELOAD or weak symbols which is beyond
 * standard C89 unit test scope without heavy frameworks).
 * The failure cases prove the logic integration.
 */

SUITE(http_curl_suite) {
  RUN_TEST(test_curl_global_lifecycle);
  RUN_TEST(test_curl_context_lifecycle);
  RUN_TEST(test_curl_config_application);
  RUN_TEST(test_curl_send_connection_failure);
  RUN_TEST(test_curl_send_invalid_arguments);
}

#endif /* TEST_HTTP_CURL_H */
