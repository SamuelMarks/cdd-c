/**
 * @file test_http_winhttp.h
 * @brief Integration tests for WinHTTP Backend.
 *
 * Verifies initialization, handle lifecycle management, configuration
 * application, and error handling.
 *
 * @author Samuel Marks
 */

#ifndef TEST_HTTP_WINHTTP_H
#define TEST_HTTP_WINHTTP_H

#include <errno.h>
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/http_types.h"
#include "functions/parse/http_winhttp.h"
#include "functions/parse/str.h"

TEST test_winhttp_lifecycle(void) {
#ifdef _WIN32
  struct HttpTransportContext *ctx = NULL;
  int rc;

  /* Global init */
  rc = http_winhttp_global_init();
  ASSERT_EQ(0, rc);

  /* Context init */
  rc = http_winhttp_context_init(&ctx);
  ASSERT_EQ(0, rc);
  ASSERT(ctx != NULL);

  /* Cleanup */
  http_winhttp_context_free(ctx);
  http_winhttp_global_cleanup();
#else
  SKIPm("WinHTTP not supported on this platform");
#endif
  PASS();
}

TEST test_winhttp_config_usage(void) {
#ifdef _WIN32
  struct HttpTransportContext *ctx = NULL;
  struct HttpConfig cfg;
  int rc;

  http_winhttp_context_init(&ctx);
  http_config_init(&cfg);

  /* Set some values */
  cfg.timeout_ms = 5000;
  /* Disable SSL verification to test flag caching logic */
  cfg.verify_peer = 0;
  cfg.verify_host = 0; /* Note: Implementation needs to map this */

  rc = http_winhttp_config_apply(ctx, &cfg);
  ASSERT_EQ(0, rc);

  /* Test proxy configuration */
  if (cfg.proxy_url)
    free(cfg.proxy_url);
  cfg.proxy_url = c_cdd_strdup("http://127.0.0.1:8888");

  rc = http_winhttp_config_apply(ctx, &cfg);
  ASSERT_EQ(0, rc);

  http_config_free(&cfg);
  http_winhttp_context_free(ctx);
#else
  SKIPm("WinHTTP not supported on this platform");
#endif
  PASS();
}

TEST test_winhttp_send_fail(void) {
#ifdef _WIN32
  struct HttpTransportContext *ctx = NULL;
  struct HttpRequest req;
  struct HttpResponse *res = NULL;
  int rc;

  rc = http_winhttp_context_init(&ctx);
  ASSERT_EQ(0, rc);

  /* Initialize request */
  http_request_init(&req);
  /* Invalid URL syntax to trigger immediate failure in CrackUrl */
  req.url = c_cdd_strdup("not_a_url");

  rc = http_winhttp_send(ctx, &req, &res);

  /* Should fail at CrackUrl stage with EINVAL */
  ASSERT(rc != 0);
  ASSERT(res == NULL);

  http_request_free(&req);
  http_winhttp_context_free(ctx);
#else
  SKIPm("WinHTTP not supported on this platform");
#endif
  PASS();
}

TEST test_winhttp_send_null_checks(void) {
#ifdef _WIN32
  struct HttpTransportContext *ctx = NULL;
  struct HttpRequest req;
  struct HttpResponse *res = NULL;
  struct HttpConfig cfg;
  int rc;

  http_winhttp_context_init(&ctx);
  http_config_init(&cfg);

  /* Send with NULLs */
  rc = http_winhttp_send(ctx, NULL, &res);
  ASSERT_EQ(EINVAL, rc);

  rc = http_winhttp_send(NULL, &req, &res);
  ASSERT_EQ(EINVAL, rc);

  rc = http_winhttp_send(ctx, &req, NULL);
  ASSERT_EQ(EINVAL, rc);

  /* Config Apply with NULLs */
  ASSERT_EQ(EINVAL, http_winhttp_config_apply(NULL, &cfg));
  ASSERT_EQ(EINVAL, http_winhttp_config_apply(ctx, NULL));

  http_config_free(&cfg);
  http_winhttp_context_free(ctx);
#else
  SKIPm("WinHTTP not supported on this platform");
#endif
  PASS();
}

SUITE(http_winhttp_suite) {
  RUN_TEST(test_winhttp_lifecycle);
  RUN_TEST(test_winhttp_config_usage);
  RUN_TEST(test_winhttp_send_fail);
  RUN_TEST(test_winhttp_send_null_checks);
}

#endif /* TEST_HTTP_WINHTTP_H */
