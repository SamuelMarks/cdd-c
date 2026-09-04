/* clang-format off */
#ifndef TEST_CDD_C_ERROR_H
#define TEST_CDD_C_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "greatest.h"
#include "cdd_c_error.h"
#include <string.h>

TEST test_cdd_c_strerror(void) {
  char *out = NULL;
  cdd_c_error_t rc;

  rc = cdd_c_strerror(CDD_C_SUCCESS, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out != NULL);
  ASSERT_EQ(0, strcmp(out, "Success"));

  rc = cdd_c_strerror(CDD_C_ERROR_MEMORY, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, strcmp(out, "Memory allocation failed"));

  rc = cdd_c_strerror(CDD_C_ERROR_INVALID_ARGUMENT, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, strcmp(out, "Invalid argument"));

  rc = cdd_c_strerror(CDD_C_ERROR_IO, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, strcmp(out, "I/O error"));

  rc = cdd_c_strerror(CDD_C_ERROR_SYSTEM, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, strcmp(out, "System error"));

  rc = cdd_c_strerror(CDD_C_ERROR_NOT_FOUND, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, strcmp(out, "Not found"));

  rc = cdd_c_strerror(CDD_C_ERROR_PARSE, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, strcmp(out, "Parse error"));

  rc = cdd_c_strerror(CDD_C_ERROR_UNKNOWN, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, strcmp(out, "Unknown error"));

  /* Test out == NULL */
  rc = cdd_c_strerror(CDD_C_SUCCESS, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

SUITE(cdd_c_error_suite) {
  RUN_TEST(test_cdd_c_strerror);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_C_ERROR_H */
/* clang-format on */