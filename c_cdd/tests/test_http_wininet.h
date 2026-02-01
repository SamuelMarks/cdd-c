/**
 * @file test_http_wininet.h
 * @brief Unit tests for the WinInet Transport Backend.
 *
 * Verifies library initialization, context creation/destruction,
 * configuration mapping, and parameter validation.
 * Note: Actual network tests are skipped on non-Windows platforms.
 *
 * @author Samuel Marks
 */

#ifndef TEST_HTTP_WININET_H
#define TEST_HTTP_WININET_H

#include <errno.h>
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "http_types.h"
#include "http_wininet.h"
#include "str_utils.h"

TEST test_wininet_lifecycle(void) {
#ifdef _WIN32
  struct HttpTransportContext *ctx = NULL;
  int rc;

  /* Init */
  rc = http_wininet_global_init();
  ASSERT_EQ(0, rc);

  rc = http_wininet_context_init(&ctx);
  ASSERT_EQ(0, rc);
  ASSERT(ctx != NULL);

  /* Cleanup */
  http_wininet_context_free(ctx);
  http_wininet_global_cleanup();
#else
  SKIPm("WinInet not supported on this platform");
#endif
  PASS();
}

TEST test_wininet_config_apply(void) {
#ifdef _WIN32
  struct HttpTransportContext *ctx = NULL;
  struct HttpConfig config;
  int rc;

  http_wininet_context_init(&ctx);
  http_config_init(&config);

  /* Customize */
  config.timeout_ms = 500;
  config.verify_peer = 0; /* Insecure */

  rc = http_wininet_config_apply(ctx, &config);
  ASSERT_EQ(0, rc);

  /* Invalid args */
  ASSERT_EQ(EINVAL, http_wininet_config_apply(NULL, &config));
  ASSERT_EQ(EINVAL, http_wininet_config_apply(ctx, NULL));

  http_config_free(&config);
  http_wininet_context_free(ctx);
#else
  SKIPm("WinInet not supported on this platform");
#endif
  PASS();
}

TEST test_wininet_send_validation(void) {
#ifdef _WIN32
  struct HttpTransportContext *ctx = NULL;
  struct HttpRequest req;
  struct HttpResponse *res = NULL;
  int rc;

  http_wininet_context_init(&ctx);
  http_request_init(&req);

  /* Null checks */
  rc = http_wininet_send(NULL, &req, &res);
  ASSERT_EQ(EINVAL, rc);

  rc = http_wininet_send(ctx, NULL, &res);
  ASSERT_EQ(EINVAL, rc);

  rc = http_wininet_send(ctx, &req, NULL);
  ASSERT_EQ(EINVAL, rc);

  /* Malformed URL handling in local testing (CrackUrl check) */
  req.url = c_cdd_strdup("not-a-valid-url");
  rc = http_wininet_send(ctx, &req, &res);
  ASSERT_EQ(EINVAL, rc);

  http_request_free(&req);
  http_wininet_context_free(ctx);
#else
  SKIPm("WinInet not supported on this platform");
#endif
  PASS();
}

SUITE(http_wininet_suite) {
  RUN_TEST(test_wininet_lifecycle);
  RUN_TEST(test_wininet_config_apply);
  RUN_TEST(test_wininet_send_validation);
}

#endif /* TEST_HTTP_WININET_H */
