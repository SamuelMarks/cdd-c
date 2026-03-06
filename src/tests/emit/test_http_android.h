/**
 * @file test_http_android.h
 * @brief Unit tests for the Android Transport Backend.
 *
 * Verifies library initialization, context creation/destruction,
 * configuration mapping, and parameter validation.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_TEST_HTTP_ANDROID_H
#define C_CDD_TEST_HTTP_ANDROID_H

#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/http_types.h"
#include "functions/parse/http_android.h"

TEST test_android_lifecycle(void) {
  struct HttpTransportContext *ctx = NULL;

  ASSERT_EQ(0, http_android_global_init());

  /* Init */
  ASSERT_EQ(EINVAL, http_android_context_init(NULL));
  ASSERT_EQ(0, http_android_context_init(&ctx));
  ASSERT(ctx != NULL);

  /* Free */
  http_android_context_free(ctx);
  http_android_context_free(NULL);

  http_android_global_cleanup();

  PASS();
}

TEST test_android_config(void) {
  struct HttpTransportContext *ctx = NULL;
  struct HttpConfig cfg;

  ASSERT_EQ(0, http_android_context_init(&ctx));
  ASSERT(ctx != NULL);

  ASSERT_EQ(0, http_config_init(&cfg));

  ASSERT_EQ(EINVAL, http_android_config_apply(NULL, &cfg));
  ASSERT_EQ(EINVAL, http_android_config_apply(ctx, NULL));
  ASSERT_EQ(0, http_android_config_apply(ctx, &cfg));

  http_config_free(&cfg);
  http_android_context_free(ctx);
  PASS();
}

TEST test_android_send_invalid(void) {
  struct HttpTransportContext *ctx = NULL;
  struct HttpRequest req;
  struct HttpResponse *res = NULL;

  ASSERT_EQ(0, http_android_context_init(&ctx));
  ASSERT_EQ(0, http_request_init(&req));

  ASSERT_EQ(EINVAL, http_android_send(NULL, &req, &res));
  ASSERT_EQ(EINVAL, http_android_send(ctx, NULL, &res));
  ASSERT_EQ(EINVAL, http_android_send(ctx, &req, NULL));

  /* Valid input but not implemented (or no JVM) should return ENOSYS */
  ASSERT_EQ(ENOSYS, http_android_send(ctx, &req, &res));

  http_request_free(&req);
  http_android_context_free(ctx);
  PASS();
}

SUITE(http_android_suite) {
  RUN_TEST(test_android_lifecycle);
  RUN_TEST(test_android_config);
  RUN_TEST(test_android_send_invalid);
}

#endif /* C_CDD_TEST_HTTP_ANDROID_H */