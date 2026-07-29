/* Auto-generated greatest.h test stub for server endpoints */
/* clang-format off */
#include "c_cdd_export.h"
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
#include <c_rest_request.h>
#include <c_rest_response.h>
#include <greatest.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */
TEST test_server_handle_doGet(void) {
  /* TODO: Implement test for doGet */
  struct c_rest_request req;
  struct c_rest_response res;
  memset(&req, 0, sizeof(req));
  memset(&res, 0, sizeof(res));
  /* int status = handle_doGet(&req, &res, NULL); */
  /* ASSERT_EQ(200, status); */
  g_fail_io_after = -1;
  PASS();
}

TEST test_server_handle_doPost(void) {
  /* TODO: Implement test for doPost */
  struct c_rest_request req;
  struct c_rest_response res;
  memset(&req, 0, sizeof(req));
  memset(&res, 0, sizeof(res));
  /* int status = handle_doPost(&req, &res, NULL); */
  /* ASSERT_EQ(200, status); */
  g_fail_io_after = -1;
  PASS();
}

TEST test_server_handle_doPut(void) {
  /* TODO: Implement test for doPut */
  struct c_rest_request req;
  struct c_rest_response res;
  memset(&req, 0, sizeof(req));
  memset(&res, 0, sizeof(res));
  /* int status = handle_doPut(&req, &res, NULL); */
  /* ASSERT_EQ(200, status); */
  g_fail_io_after = -1;
  PASS();
}

TEST test_server_handle_doDelete(void) {
  /* TODO: Implement test for doDelete */
  struct c_rest_request req;
  struct c_rest_response res;
  memset(&req, 0, sizeof(req));
  memset(&res, 0, sizeof(res));
  /* int status = handle_doDelete(&req, &res, NULL); */
  /* ASSERT_EQ(200, status); */
  g_fail_io_after = -1;
  PASS();
}

TEST test_server_handle_doOptions(void) {
  /* TODO: Implement test for doOptions */
  struct c_rest_request req;
  struct c_rest_response res;
  memset(&req, 0, sizeof(req));
  memset(&res, 0, sizeof(res));
  /* int status = handle_doOptions(&req, &res, NULL); */
  /* ASSERT_EQ(200, status); */
  g_fail_io_after = -1;
  PASS();
}

TEST test_server_handle_doHead(void) {
  /* TODO: Implement test for doHead */
  struct c_rest_request req;
  struct c_rest_response res;
  memset(&req, 0, sizeof(req));
  memset(&res, 0, sizeof(res));
  /* int status = handle_doHead(&req, &res, NULL); */
  /* ASSERT_EQ(200, status); */
  g_fail_io_after = -1;
  PASS();
}

TEST test_server_handle_doPatch(void) {
  /* TODO: Implement test for doPatch */
  struct c_rest_request req;
  struct c_rest_response res;
  memset(&req, 0, sizeof(req));
  memset(&res, 0, sizeof(res));
  /* int status = handle_doPatch(&req, &res, NULL); */
  /* ASSERT_EQ(200, status); */
  g_fail_io_after = -1;
  PASS();
}

TEST test_server_handle_doTrace(void) {
  /* TODO: Implement test for doTrace */
  struct c_rest_request req;
  struct c_rest_response res;
  memset(&req, 0, sizeof(req));
  memset(&res, 0, sizeof(res));
  /* int status = handle_doTrace(&req, &res, NULL); */
  /* ASSERT_EQ(200, status); */
  g_fail_io_after = -1;
  PASS();
}

TEST test_server_handle_doUnknown(void) {
  /* TODO: Implement test for doUnknown */
  struct c_rest_request req;
  struct c_rest_response res;
  memset(&req, 0, sizeof(req));
  memset(&res, 0, sizeof(res));
  /* int status = handle_doUnknown(&req, &res, NULL); */
  /* ASSERT_EQ(200, status); */
  g_fail_io_after = -1;
  PASS();
}

SUITE(server_endpoints_suite) {
  RUN_TEST(test_server_handle_doGet);
  RUN_TEST(test_server_handle_doPost);
  RUN_TEST(test_server_handle_doPut);
  RUN_TEST(test_server_handle_doDelete);
  RUN_TEST(test_server_handle_doOptions);
  RUN_TEST(test_server_handle_doHead);
  RUN_TEST(test_server_handle_doPatch);
  RUN_TEST(test_server_handle_doTrace);
  RUN_TEST(test_server_handle_doUnknown);
}
