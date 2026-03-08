/**
 * @file test_http_apple.h
 * @brief Unit tests for the Apple Transport Backend.
 *
 * Verifies library initialization, context creation/destruction,
 * configuration mapping, and parameter validation.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_TEST_HTTP_APPLE_H
#define C_CDD_TEST_HTTP_APPLE_H

#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/http_types.h"
#include "functions/parse/http_apple.h"

TEST test_apple_lifecycle(void) {
  struct HttpTransportContext *ctx = NULL;

  ASSERT_EQ(0, http_apple_global_init());

  /* Init */
  ASSERT_EQ(EINVAL, http_apple_context_init(NULL));
  ASSERT_EQ(0, http_apple_context_init(&ctx));
  ASSERT(ctx != NULL);

  /* Free */
  http_apple_context_free(ctx);
  http_apple_context_free(NULL);

  http_apple_global_cleanup();

  PASS();
}

TEST test_apple_config(void) {
  struct HttpTransportContext *ctx = NULL;
  struct HttpConfig cfg;

  ASSERT_EQ(0, http_apple_context_init(&ctx));
  ASSERT(ctx != NULL);

  ASSERT_EQ(0, http_config_init(&cfg));

  ASSERT_EQ(EINVAL, http_apple_config_apply(NULL, &cfg));
  ASSERT_EQ(EINVAL, http_apple_config_apply(ctx, NULL));
  ASSERT_EQ(0, http_apple_config_apply(ctx, &cfg));

  http_config_free(&cfg);
  http_apple_context_free(ctx);
  PASS();
}

TEST test_apple_send_invalid(void) {
  struct HttpTransportContext *ctx = NULL;
  struct HttpRequest req;
  struct HttpResponse *res = NULL;

  ASSERT_EQ(0, http_apple_context_init(&ctx));
  ASSERT_EQ(0, http_request_init(&req));

  ASSERT_EQ(EINVAL, http_apple_send(NULL, &req, &res));
  ASSERT_EQ(EINVAL, http_apple_send(ctx, NULL, &res));
  ASSERT_EQ(EINVAL, http_apple_send(ctx, &req, NULL));

#if defined(__APPLE__)
  /* Valid input but we need to mock a real URL so it fails gracefully */
  req.url = "http://localhost:1";
  req.method = HTTP_GET;
  /* Might fail with EIO due to no connection or return an allocated res */
  {
    int rc = http_apple_send(ctx, &req, &res);
    if (rc == 0) {
      if (res) http_response_free(res);
      free(res);
    }
  }
#else
  /* Valid input but not implemented (or no Apple OS) should return ENOSYS */
  ASSERT_EQ(ENOSYS, http_apple_send(ctx, &req, &res));
#endif

  http_request_free(&req);
  http_apple_context_free(ctx);
  PASS();
}

SUITE(http_apple_suite) {
  RUN_TEST(test_apple_lifecycle);
  RUN_TEST(test_apple_config);
  RUN_TEST(test_apple_send_invalid);
}

#endif /* C_CDD_TEST_HTTP_APPLE_H */
