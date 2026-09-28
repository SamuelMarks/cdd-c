/**
 * @file test_codegen_client_body_helpers.h
 * @brief Test helper functions for exhaustive client body testing.
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_HELPERS_H
#define TEST_CODEGEN_CLIENT_BODY_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd_stdbool.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "classes/emit/struct.h"
#include "functions/emit/client_body.h"
#include "functions/parse/str.h"
#include "greatest.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

#ifndef C_CDD_STR_LIT
#define C_CDD_STR_LIT(s) ((char *)(size_t)(size_t)(s))
#endif

extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_cdd_fail_is_primitive_type;

/**
 * @brief Register test suite for client body internals and edge cases.
 */

/**
 * @brief Helper for exhaustive IO failure testing on codegen_client_write_body.
 *
 * @param[in] op OpenAPI Operation.
 * @param[in] spec OpenAPI Spec.
 * @param[in] path Path template string.
 */
static void test_helper_run_io_client_body(const struct OpenAPI_Operation *op,
                                           const struct OpenAPI_Spec *spec,
                                           const char *path) {
  FILE *fp;
  int total_calls;
  int i;
  fp = cdd_test_tmpfile_global();
  g_fail_io_after = 100000;
  g_io_calls = 0;
  codegen_client_write_body(fp, op, spec, path, NULL);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = i;
    codegen_client_write_body(fp, op, spec, path, NULL);
    g_fail_io_after = -1;
    fclose(fp);
  }
}

/**
 * @brief Helper for exhaustive IO failure testing on
 * client_body_write_header_param_logic.
 *
 * @param[in] op OpenAPI Operation.
 */
static void test_helper_run_io_hdr(const struct OpenAPI_Operation *op) {
  FILE *fp;
  int total_calls;
  int i;
  fp = cdd_test_tmpfile_global();
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_header_param_logic(fp, op);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = i;
    client_body_write_header_param_logic(fp, op);
    g_fail_io_after = -1;
    fclose(fp);
  }
}

/**
 * @brief Helper for exhaustive IO failure testing on
 * client_body_write_cookie_param_logic.
 *
 * @param[in] op OpenAPI Operation.
 */
static void test_helper_run_io_cookie(const struct OpenAPI_Operation *op) {
  FILE *fp;
  int total_calls;
  int i;
  fp = cdd_test_tmpfile_global();
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_cookie_param_logic(fp, op);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = i;
    client_body_write_cookie_param_logic(fp, op);
    g_fail_io_after = -1;
    fclose(fp);
  }
}

/**
 * @brief Helper for exhaustive IO failure testing on
 * client_body_write_form_urlencoded_body.
 *
 * @param[in] op OpenAPI Operation.
 * @param[in] spec OpenAPI Spec.
 */
static void test_helper_run_io_form(const struct OpenAPI_Operation *op,
                                    const struct OpenAPI_Spec *spec) {
  FILE *fp;
  int total_calls;
  int i;
  fp = cdd_test_tmpfile_global();
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_form_urlencoded_body(fp, op, spec);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = i;
    client_body_write_form_urlencoded_body(fp, op, spec);
    g_fail_io_after = -1;
    fclose(fp);
  }
}

/**
 * @brief Helper for exhaustive IO failure testing on
 * client_body_write_multipart_body.
 *
 * @param[in] op OpenAPI Operation.
 * @param[in] spec OpenAPI Spec.
 */
static void test_helper_run_io_multipart(const struct OpenAPI_Operation *op,
                                         const struct OpenAPI_Spec *spec) {
  FILE *fp;
  int total_calls;
  int i;
  fp = cdd_test_tmpfile_global();
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_multipart_body(fp, op, spec);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = i;
    client_body_write_multipart_body(fp, op, spec);
    g_fail_io_after = -1;
    fclose(fp);
  }
}

/**
 * @brief Helper for exhaustive IO failure testing on
 * client_body_write_multipart_part_headers.
 *
 * @param[in] enc OpenAPI Encoding.
 */
static void
test_helper_run_io_part_headers(const struct OpenAPI_Encoding *enc) {
  FILE *fp;
  int total_calls;
  int i;
  fp = cdd_test_tmpfile_global();
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_multipart_part_headers(fp, enc);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = i;
    client_body_write_multipart_part_headers(fp, enc);
    g_fail_io_after = -1;
    fclose(fp);
  }
}

/**
 * @brief Helper for exhaustive IO failure testing on
 * client_body_write_joined_form_array.
 *
 * @param[in] field Field name.
 * @param[in] len_field Len field name.
 * @param[in] items_type Items type name.
 * @param[in] delim Delimiter.
 * @param[in] encode_fn Encode function.
 * @param[in] add_encoded Add encoded flag.
 * @param[in] is_object Object flag.
 */
static void test_helper_run_io_joined_form_array(
    const char *field, const char *len_field, const char *items_type,
    char delim, const char *encode_fn, int add_encoded, int is_object) {
  FILE *fp;
  int total_calls;
  int i;
  fp = cdd_test_tmpfile_global();
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_joined_form_array(fp, field, len_field, items_type, delim,
                                      encode_fn, add_encoded, is_object);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = i;
    client_body_write_joined_form_array(fp, field, len_field, items_type, delim,
                                        encode_fn, add_encoded, is_object);
    g_fail_io_after = -1;
    fclose(fp);
  }
}

/**
 * @brief Comprehensive test to achieve 100% line, function, and branch coverage
 * for client_body.
 *
 * @return GREATEST_TEST_RES.
 */
/**
 * @brief Helper for exhaustive IO failure testing on
 * client_body_write_inline_json_parse.
 *
 * @param[in] schema OpenAPI SchemaRef.
 */
static void
test_helper_run_io_inline_json_parse(const struct OpenAPI_SchemaRef *schema) {
  FILE *fp;
  int total_calls;
  int i;
  fp = cdd_test_tmpfile_global();
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_inline_json_parse(fp, schema);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = i;
    client_body_write_inline_json_parse(fp, schema);
    g_fail_io_after = -1;
    fclose(fp);
  }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_HELPERS_H */
