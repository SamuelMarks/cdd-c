/**
 * @file test_codegen_client_body_common.h
 * @brief Common test fixture and helper functions for client body generator
 * tests.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_COMMON_H
#define TEST_CODEGEN_CLIENT_BODY_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_test_helpers_export.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/struct.h"
#include "functions/emit/client_body.h"
#include "functions/parse/str.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);

extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_fail_io_after;

static int g_client_body_fail_tmpfile = 0;

static cdd_c_error_t gen_body(const struct OpenAPI_Operation *op,
                              const struct OpenAPI_Spec *spec, const char *tmpl,
                              const char *base_url_expr, char **_out_val) {
  FILE *tmp;
  long sz;
  char *content = NULL;

  cdd_c_error_t rc = 0;

  if (g_client_body_fail_tmpfile) {
    tmp = NULL;
  } else {
#if defined(_MSC_VER)
    if (((tmp = cdd_test_tmpfile_global()) == NULL))
      tmp = NULL;
#else
    tmp = cdd_test_tmpfile_global();
#endif
  }
  if (!tmp)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = codegen_client_write_body(tmp, op, spec, tmpl, base_url_expr);
  if (rc != CDD_C_SUCCESS) {
    if (tmp)
      fclose(tmp);
    return rc;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = (char *)(size_t)calloc(1, (size_t)sz + 1);
  if (sz > 0)
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

  if (tmp)
    fclose(tmp);
  *_out_val = content;
  return CDD_C_SUCCESS;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_COMMON_H */
