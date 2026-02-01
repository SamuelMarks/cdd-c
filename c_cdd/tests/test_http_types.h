#ifndef TEST_HTTP_TYPES_H
#define TEST_HTTP_TYPES_H

#include <errno.h>
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "http_types.h"

TEST test_multipart_lifecycle(void) {
  struct HttpRequest req;
  http_request_init(&req);

  ASSERT_EQ(0, req.parts.count);

  /* Add text part */
  ASSERT_EQ(0, http_request_add_part(&req, "field", NULL, NULL, "value", 5));
  ASSERT_EQ(1, req.parts.count);
  ASSERT_STR_EQ("field", req.parts.parts[0].name);
  ASSERT_EQ(NULL, req.parts.parts[0].filename);

  /* Add file part */
  ASSERT_EQ(0, http_request_add_part(&req, "file", "pic.jpg", "image/jpeg",
                                     "DATA", 4));
  ASSERT_EQ(2, req.parts.count);
  ASSERT_STR_EQ("pic.jpg", req.parts.parts[1].filename);

  http_request_free(&req);
  PASS();
}

TEST test_multipart_flatten(void) {
  struct HttpRequest req;
  char *content;

  http_request_init(&req);
  http_request_add_part(&req, "f1", NULL, NULL, "v1", 2);
  http_request_add_part(&req, "f2", "a.txt", "text/plain", "v2", 2);

  ASSERT_EQ(0, http_request_flatten_parts(&req));
  ASSERT(req.body != NULL);
  ASSERT(req.body_len > 0);

  content = (char *)req.body;
  /* Basic sanity check of content */
  ASSERT(strstr(content, "Content-Disposition: form-data; name=\"f1\""));
  ASSERT(strstr(
      content,
      "Content-Disposition: form-data; name=\"f2\"; filename=\"a.txt\""));
  ASSERT(strstr(content, "Content-Type: text/plain"));
  ASSERT(strstr(content, "v2"));         /* Data */
  ASSERT(strstr(content, "--cddbound")); /* Boundary */

  http_request_free(&req);
  PASS();
}

SUITE(http_types_suite) {
  RUN_TEST(test_multipart_lifecycle);
  RUN_TEST(test_multipart_flatten);
}

#endif
