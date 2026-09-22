/**
 * @file test_codegen_client_body_internals.h
 * @brief Comprehensive tests for client_body helper functions and edge cases.
 *
 * Exercises all parameter validations, predicates, media type parsers,
 * sub-emitters, and I/O failure branches to achieve 100% test coverage.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_INTERNALS_H
#define TEST_CODEGEN_CLIENT_BODY_INTERNALS_H

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

#define C_CDD_STR_LIT(s) ((char *)(size_t)(size_t)(s))

extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_cdd_fail_is_primitive_type;

/**
 * @brief Test client_body verb and method string to enum mappings.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_verb_and_method_helpers(void) {
  const char *val = NULL;
  cdd_c_error_t rc;

  /* Null argument checks */
  rc = client_body_verb_to_enum_str(OA_VERB_GET, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_method_str_to_enum_str("get", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_mapped_err_code(400, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Verb mappings */
  rc = client_body_verb_to_enum_str(OA_VERB_GET, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_GET", val);

  rc = client_body_verb_to_enum_str(OA_VERB_POST, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_POST", val);

  rc = client_body_verb_to_enum_str(OA_VERB_PUT, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_PUT", val);

  rc = client_body_verb_to_enum_str(OA_VERB_DELETE, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_DELETE", val);

  rc = client_body_verb_to_enum_str(OA_VERB_HEAD, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_HEAD", val);

  rc = client_body_verb_to_enum_str(OA_VERB_PATCH, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_PATCH", val);

  rc = client_body_verb_to_enum_str(OA_VERB_OPTIONS, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_OPTIONS", val);

  rc = client_body_verb_to_enum_str(OA_VERB_TRACE, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_TRACE", val);

  rc = client_body_verb_to_enum_str(OA_VERB_QUERY, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_QUERY", val);

  rc = client_body_verb_to_enum_str((enum OpenAPI_Verb)9999, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_GET", val);

  /* Method string mappings */
  rc = client_body_method_str_to_enum_str(NULL, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(val == NULL);

  rc = client_body_method_str_to_enum_str("get", &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_GET", val);

  rc = client_body_method_str_to_enum_str("post", &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_POST", val);

  rc = client_body_method_str_to_enum_str("put", &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_PUT", val);

  rc = client_body_method_str_to_enum_str("delete", &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_DELETE", val);

  rc = client_body_method_str_to_enum_str("head", &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_HEAD", val);

  rc = client_body_method_str_to_enum_str("patch", &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_PATCH", val);

  rc = client_body_method_str_to_enum_str("options", &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_OPTIONS", val);

  rc = client_body_method_str_to_enum_str("trace", &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_TRACE", val);

  rc = client_body_method_str_to_enum_str("query", &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_QUERY", val);

  rc = client_body_method_str_to_enum_str("connect", &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("HTTP_CONNECT", val);

  rc = client_body_method_str_to_enum_str("nonexistent_verb", &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(val == NULL);

  /* Error code mappings */
  rc = client_body_mapped_err_code(400, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("CDD_C_ERROR_INVALID_ARGUMENT", val);

  rc = client_body_mapped_err_code(401, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("CDD_C_ERROR_SYSTEM", val);

  rc = client_body_mapped_err_code(403, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("CDD_C_ERROR_SYSTEM", val);

  rc = client_body_mapped_err_code(404, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("CDD_C_ERROR_NOT_FOUND", val);

  rc = client_body_mapped_err_code(500, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("CDD_C_ERROR_IO", val);

  rc = client_body_mapped_err_code(200, &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("CDD_C_ERROR_IO", val);

  PASS();
}

/**
 * @brief Test find_media_type and find_encoding helper routines.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_find_helpers(void) {
  struct OpenAPI_MediaType mts[2];
  struct OpenAPI_Encoding encs[2];
  const struct OpenAPI_MediaType *found_mt = NULL;
  struct OpenAPI_Encoding *found_enc = NULL;
  cdd_c_error_t rc;

  memset(mts, 0, sizeof(mts));
  memset(encs, 0, sizeof(encs));

  mts[0].name = C_CDD_STR_LIT("application/json");
  mts[1].name = C_CDD_STR_LIT("text/plain");
  mts[0].encoding = encs;
  mts[0].n_encoding = 2;
  encs[0].name = C_CDD_STR_LIT("profile");
  encs[1].name = C_CDD_STR_LIT("avatar");

  /* find_media_type validations */
  rc = client_body_find_media_type(mts, 2, "application/json", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_find_media_type(NULL, 2, "application/json", &found_mt);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_mt == NULL);

  rc = client_body_find_media_type(mts, 2, NULL, &found_mt);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_mt == NULL);

  rc = client_body_find_media_type(mts, 2, "application/json", &found_mt);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_mt == &mts[0]);

  rc = client_body_find_media_type(mts, 2, "text/plain", &found_mt);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_mt == &mts[1]);

  rc = client_body_find_media_type(mts, 2, "image/png", &found_mt);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_mt == NULL);

  /* find_encoding validations */
  rc = client_body_find_encoding(&mts[0], "profile", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_find_encoding(NULL, "profile", &found_enc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_enc == NULL);

  rc = client_body_find_encoding(&mts[0], NULL, &found_enc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_enc == NULL);

  rc = client_body_find_encoding(&mts[1], "profile", &found_enc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_enc == NULL);

  rc = client_body_find_encoding(&mts[0], "profile", &found_enc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_enc == &encs[0]);

  rc = client_body_find_encoding(&mts[0], "avatar", &found_enc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_enc == &encs[1]);

  rc = client_body_find_encoding(&mts[0], "missing", &found_enc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_enc == NULL);

  PASS();
}

/**
 * @brief Test type classifications and struct fields predicates.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_type_predicates(void) {
  int flag = 0;
  struct StructFields sf;
  struct OpenAPI_SchemaRef schema;
  cdd_c_error_t rc;

  /* is_primitive_type */
  rc = client_body_is_primitive_type("string", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_is_primitive_type(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_primitive_type("string", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_is_primitive_type("integer", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_is_primitive_type("number", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_is_primitive_type("boolean", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_is_primitive_type("object", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_primitive_type("array", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_primitive_type("custom_ref", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  /* is_object_ref_type */
  rc = client_body_is_object_ref_type("MyType", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_is_object_ref_type(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_object_ref_type("string", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_object_ref_type("object", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_object_ref_type("array", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_object_ref_type("enum", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_object_ref_type("MyCustomModel", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  /* struct_fields_all_primitive */
  rc = client_body_struct_fields_all_primitive(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_struct_fields_all_primitive(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  struct_fields_init(&sf);
  struct_fields_add(&sf, "name", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "age", "integer", NULL, NULL, NULL);
  rc = client_body_struct_fields_all_primitive(&sf, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  struct_fields_add(&sf, "sub", "object", NULL, NULL, NULL);
  rc = client_body_struct_fields_all_primitive(&sf, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);
  struct_fields_free(&sf);

  /* schema_has_inline */
  rc = client_body_schema_has_inline(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_schema_has_inline(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  memset(&schema, 0, sizeof(schema));
  rc = client_body_schema_has_inline(&schema, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  schema.inline_type = C_CDD_STR_LIT("string");
  rc = client_body_schema_has_inline(&schema, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  /* schema_inline_is_string */
  rc = client_body_schema_inline_is_string(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_schema_inline_is_string(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  memset(&schema, 0, sizeof(schema));
  schema.inline_type = C_CDD_STR_LIT("string");
  schema.is_array = 1;
  rc = client_body_schema_inline_is_string(&schema, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  schema.is_array = 0;
  schema.inline_type = C_CDD_STR_LIT("integer");
  rc = client_body_schema_inline_is_string(&schema, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  schema.inline_type = C_CDD_STR_LIT("string");
  rc = client_body_schema_inline_is_string(&schema, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  /* schema_has_payload */
  rc = client_body_schema_has_payload(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_schema_has_payload(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  memset(&schema, 0, sizeof(schema));
  rc = client_body_schema_has_payload(&schema, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  schema.ref_name = C_CDD_STR_LIT("UploadModel");
  rc = client_body_schema_has_payload(&schema, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  schema.ref_name = NULL;
  schema.inline_type = C_CDD_STR_LIT("string");
  rc = client_body_schema_has_payload(&schema, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  PASS();
}

/**
 * @brief Test media type parsing and matching functions.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_media_type_helpers(void) {
  size_t sz = 0;
  int flag = 0;
  char buf[64];
  const char *val = NULL;
  struct OpenAPI_Response resp;
  struct OpenAPI_Header hdr;
  cdd_c_error_t rc;

  /* media_type_base_len */
  rc = client_body_media_type_base_len(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_media_type_base_len(NULL, &sz);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, sz);

  rc = client_body_media_type_base_len("application/json; charset=utf-8", &sz);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(16, sz);

  rc = client_body_media_type_base_len("text/plain", &sz);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(10, sz);

  /* media_type_has_prefix */
  rc = client_body_media_type_has_prefix(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_media_type_has_prefix(NULL, "text/", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_has_prefix("text/plain", NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_has_prefix("app", "application/", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_has_prefix("TEXT/PLAIN", "text/", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_has_prefix("text/plain", "TEXT/", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_has_prefix("application/json", "apple/", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_has_prefix("application/json", "text/", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  /* media_type_has_suffix */
  rc = client_body_media_type_has_suffix(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_media_type_has_suffix(NULL, "+json", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_has_suffix("app", "+json_long", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_has_suffix("application/problem+JSON", "+json",
                                         &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_has_suffix("application/problem+json", "+JSON",
                                         &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_has_suffix("application/json", "+xml", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_has_suffix("application/xml", "+json", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  /* media_type_ieq */
  rc = client_body_media_type_ieq(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_media_type_ieq(NULL, "application/json", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_ieq("application/json", NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_ieq("app; foo", "different_len", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_ieq("APPLICATION/JSON; charset=utf-8",
                                  "application/json", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_ieq("text/plain", "TEXT/PLAIN", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_ieq("text/plain", "text/html", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_ieq("application/json", "application/xml", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  /* media_type_is_json / form / text_plain / multipart */
  rc = client_body_media_type_is_json(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_media_type_is_json(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_is_json("application/json", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_is_json("application/problem+json", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_is_json("text/plain", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_is_form(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_media_type_is_form(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_is_form("application/x-www-form-urlencoded",
                                      &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_is_text_plain(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_media_type_is_text_plain(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_is_text_plain("text/plain", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_is_multipart(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_media_type_is_multipart(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_is_multipart("multipart/form-data", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_is_multipart_form(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_media_type_is_multipart_form(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_is_multipart_form("multipart/form-data", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_is_multipart_form("multipart/mixed", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  /* first_content_type_entry */
  rc = client_body_first_content_type_entry(NULL, NULL, 0, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_first_content_type_entry("   text/plain, application/json",
                                            buf, sizeof(buf), &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("text/plain", val);

  /* sanitize_ident */
  rc = client_body_sanitize_ident(NULL, 0, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_sanitize_ident(buf, sizeof(buf), "my-var.v1/name");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("my_var_v1_name", buf);

  /* multipart_header_param_name */
  memset(&hdr, 0, sizeof(hdr));
  hdr.name = C_CDD_STR_LIT("X-Rate-Limit");
  rc = client_body_multipart_header_param_name(NULL, 0, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_multipart_header_param_name(buf, sizeof(buf), "part",
                                               hdr.name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("part_hdr_X_Rate_Limit", buf);

  /* header_name_is_content_type */
  rc = client_body_header_name_is_content_type(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_header_name_is_content_type(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_header_name_is_content_type("Content-Type", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_header_name_is_content_type("CONTENT-TYPE", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_header_name_is_content_type("Authorization", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  /* media_type_is_textual */
  rc = client_body_media_type_is_textual(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_media_type_is_textual(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_is_textual("text/html", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_is_textual("application/xml", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_is_textual("application/soap+xml", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_is_textual("image/jpeg", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  /* media_type_is_binary */
  rc = client_body_media_type_is_binary(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_media_type_is_binary(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_is_binary("application/json", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_is_binary("application/x-www-form-urlencoded",
                                        &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_is_binary("multipart/form-data", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_is_binary("text/plain", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_media_type_is_binary("application/octet-stream", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_media_type_is_binary("image/png", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  /* response_is_textual_string */
  rc = client_body_response_is_textual_string(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_response_is_textual_string(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  memset(&resp, 0, sizeof(resp));
  rc = client_body_response_is_textual_string(&resp, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  resp.content_type = C_CDD_STR_LIT("application/json");
  rc = client_body_response_is_textual_string(&resp, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  resp.content_type = C_CDD_STR_LIT("text/plain");
  resp.schema.inline_type = C_CDD_STR_LIT("string");
  rc = client_body_response_is_textual_string(&resp, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  resp.schema.inline_type = C_CDD_STR_LIT("integer");
  rc = client_body_response_is_textual_string(&resp, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  /* response_is_binary */
  rc = client_body_response_is_binary(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_response_is_binary(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  memset(&resp, 0, sizeof(resp));
  resp.content_type = C_CDD_STR_LIT("application/octet-stream");
  rc = client_body_response_is_binary(&resp, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  resp.content_type = C_CDD_STR_LIT("application/json");
  rc = client_body_response_is_binary(&resp, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  /* status code classification */
  rc = client_body_is_status_range_code(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_is_status_range_code(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_status_range_code("2XX", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_is_status_range_code("4XX", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_is_status_range_code("200", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_status_range_code("XX", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_status_range_prefix(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_status_range_prefix(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_status_range_prefix("2XX", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, flag);

  rc = client_body_status_range_prefix("5XX", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(5, flag);

  rc = client_body_status_range_prefix("200", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_status_code_literal(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_is_status_code_literal(NULL, &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_status_code_literal("200", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_is_status_code_literal("404", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, flag);

  rc = client_body_is_status_code_literal("2XX", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_status_code_literal("20", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  rc = client_body_is_status_code_literal("2000", &flag);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, flag);

  PASS();
}

/**
 * @brief Test failure branches when is_primitive_type fails via test hook.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_fail_is_primitive_type_branches(void) {
  int flag = 0;
  struct StructFields sf;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  FILE *fp;
  cdd_c_error_t rc;

  /* is_primitive_type failure */
  g_cdd_fail_is_primitive_type = 1;
  rc = client_body_is_primitive_type("string", &flag);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* is_object_ref_type failure */
  g_cdd_fail_is_primitive_type = 1;
  rc = client_body_is_object_ref_type("string", &flag);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* struct_fields_all_primitive failure */
  struct_fields_init(&sf);
  struct_fields_add(&sf, "name", "string", NULL, NULL, NULL);
  g_cdd_fail_is_primitive_type = 1;
  rc = client_body_struct_fields_all_primitive(&sf, &flag);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  struct_fields_free(&sf);

  /* write_header_param_logic array primitive failure */
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  param.name = C_CDD_STR_LIT("hdr_arr");
  param.in = OA_PARAM_IN_HEADER;
  param.content_type = C_CDD_STR_LIT("application/json");
  param.is_array = 1;
  param.items_type = C_CDD_STR_LIT("string");
  op.parameters = &param;
  op.n_parameters = 1;
  g_cdd_fail_is_primitive_type = 1;
  rc = client_body_write_header_param_logic(fp, &op);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* write_header_param_logic non-ref param type failure */
  param.is_array = 0;
  param.type = C_CDD_STR_LIT("string");
  g_cdd_fail_is_primitive_type = 1;
  rc = client_body_write_header_param_logic(fp, &op);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* write_header_param_logic primitive type failure */
  param.type = NULL;
  param.schema.inline_type = C_CDD_STR_LIT("string");
  g_cdd_fail_is_primitive_type = 1;
  rc = client_body_write_header_param_logic(fp, &op);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  g_cdd_fail_is_primitive_type = 0;
  fclose(fp);
  PASS();
}

/**
 * @brief Test direct calls to body writer sub-emitters and validation.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_writer_sub_emitters(void) {
  FILE *fp;
  struct OpenAPI_SchemaRef schema;
  struct OpenAPI_Operation op;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Encoding enc;
  cdd_c_error_t rc;

  /* Null argument assertions */
  rc = client_body_write_text_plain_success(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_binary_success(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_inline_json_parse(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_joined_form_array(NULL, NULL, NULL, NULL, '&', NULL, 0,
                                           0);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_header_param_logic(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_form_urlencoded_body(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_cookie_param_logic(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_multipart_part_headers(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_multipart_body(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = codegen_client_write_body(NULL, NULL, NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Direct writer calls with temp file */
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);

  rc = client_body_write_text_plain_success(fp);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_binary_success(fp);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  memset(&schema, 0, sizeof(schema));
  schema.inline_type = C_CDD_STR_LIT("string");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("integer");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("number");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("boolean");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.is_array = 1;
  schema.inline_type = C_CDD_STR_LIT("string");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("integer");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("number");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("boolean");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("CustomUnknown");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.is_array = 0;
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* joined form array variants */
  rc = client_body_write_joined_form_array(fp, "tags", "n_tags", "string", '|',
                                           NULL, 0, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_joined_form_array(fp, "tags", "n_tags", "string", ' ',
                                           "url_encode", 1, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_joined_form_array(fp, "items", "n_items", "Item", '&',
                                           NULL, 0, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_joined_form_array(fp, "nums", "n_nums", "integer", ',',
                                           NULL, 0, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_joined_form_array(fp, "rates", "n_rates", "number",
                                           ',', NULL, 0, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_joined_form_array(fp, "flags", "n_flags", "boolean",
                                           ',', NULL, 0, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* multipart part headers */
  memset(&enc, 0, sizeof(enc));
  rc = client_body_write_multipart_part_headers(fp, &enc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  memset(&op, 0, sizeof(op));
  rc = client_body_write_cookie_param_logic(fp, &op);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_header_param_logic(fp, &op);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  memset(&spec, 0, sizeof(spec));
  rc = client_body_write_form_urlencoded_body(fp, &op, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_multipart_body(fp, &op, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  fclose(fp);
  PASS();
}

/**
 * @brief Comprehensive operation test covering form urlencoded, cookie, and
 * multipart body edge cases.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_all_operation_patterns(void) {
  FILE *fp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter params[25];
  struct OpenAPI_Response responses[8];
  struct OpenAPI_Encoding encs[8];
  struct OpenAPI_Header hdrs[6];
  struct OpenAPI_MediaType mt_form;
  struct OpenAPI_MediaType mt_mp;
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(2, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(4, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);

  /* Schema 0: Complex form schema */
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "str_val", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "res_val", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "int_val", "integer", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "num_val", "number", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "bool_val", "boolean", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_matrix", "array", "string",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_pipe", "array", "string",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_space", "array", "string",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_num", "array", "number",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_bool", "array", "boolean",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_obj", "array", "SubModel",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_unsup", "array",
                    "custom_unknown", NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "obj_deep", "object", "SubModel",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "obj_space", "object", "SubModel",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "obj_pipe", "object", "SubModel",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "obj_matrix", "object",
                    "SubModel", NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "obj_val", "object", "SubModel",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "obj_noref", "object", "", NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "unsupported", "custom_blob",
                    NULL, NULL, NULL);
  c_cdd_strdup("ComplexForm", &spec.defined_schema_names[0]);

  /* Schema 1: SubModel */
  struct_fields_init(&spec.defined_schemas[1]);
  struct_fields_add(&spec.defined_schemas[1], "p_str", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[1], "p_int", "integer", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[1], "p_num", "number", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[1], "p_bool", "boolean", NULL, NULL,
                    NULL);
  c_cdd_strdup("SubModel", &spec.defined_schema_names[1]);
  spec.n_defined_schemas = 2;

  /* Setup operation */
  memset(&op, 0, sizeof(op));
  memset(params, 0, sizeof(params));
  memset(responses, 0, sizeof(responses));
  memset(encs, 0, sizeof(encs));
  memset(hdrs, 0, sizeof(hdrs));

  op.operation_id = C_CDD_STR_LIT("testComplexAll");
  op.verb = OA_VERB_POST;
  op.method = C_CDD_STR_LIT("post");

  /* Parameters: path, query, header, cookie */
  params[0].name = C_CDD_STR_LIT("path_id");
  params[0].in = OA_PARAM_IN_PATH;
  params[0].type = C_CDD_STR_LIT("string");

  params[1].name = C_CDD_STR_LIT("filter");
  params[1].in = OA_PARAM_IN_QUERY;
  params[1].type = C_CDD_STR_LIT("string");

  params[2].name = C_CDD_STR_LIT("X-Api-Version");
  params[2].in = OA_PARAM_IN_HEADER;
  params[2].type = C_CDD_STR_LIT("string");

  params[3].name = C_CDD_STR_LIT("X-Json-Primitive");
  params[3].in = OA_PARAM_IN_HEADER;
  params[3].content_type = C_CDD_STR_LIT("application/json");
  params[3].type = C_CDD_STR_LIT("integer");

  params[4].name = C_CDD_STR_LIT("X-Json-Array-Prim");
  params[4].in = OA_PARAM_IN_HEADER;
  params[4].content_type = C_CDD_STR_LIT("application/json");
  params[4].is_array = 1;
  params[4].items_type = C_CDD_STR_LIT("string");

  params[5].name = C_CDD_STR_LIT("cookie_str");
  params[5].in = OA_PARAM_IN_COOKIE;
  params[5].type = C_CDD_STR_LIT("string");

  params[6].name = C_CDD_STR_LIT("cookie_int");
  params[6].in = OA_PARAM_IN_COOKIE;
  params[6].type = C_CDD_STR_LIT("integer");

  params[7].name = C_CDD_STR_LIT("cookie_arr");
  params[7].in = OA_PARAM_IN_COOKIE;
  params[7].is_array = 1;
  params[7].items_type = C_CDD_STR_LIT("integer");
  params[7].explode_set = 1;
  params[7].explode = 0;

  params[8].name = C_CDD_STR_LIT("cookie_no_enc_str");
  params[8].in = OA_PARAM_IN_COOKIE;
  params[8].style = OA_STYLE_COOKIE;
  params[8].type = C_CDD_STR_LIT("string");

  params[9].name = C_CDD_STR_LIT("cookie_no_enc_num");
  params[9].in = OA_PARAM_IN_COOKIE;
  params[9].style = OA_STYLE_COOKIE;
  params[9].type = C_CDD_STR_LIT("number");

  params[10].name = C_CDD_STR_LIT("cookie_no_enc_bool");
  params[10].in = OA_PARAM_IN_COOKIE;
  params[10].style = OA_STYLE_COOKIE;
  params[10].type = C_CDD_STR_LIT("boolean");

  params[11].name = C_CDD_STR_LIT("cookie_no_enc_arr");
  params[11].in = OA_PARAM_IN_COOKIE;
  params[11].style = OA_STYLE_COOKIE;
  params[11].is_array = 1;
  params[11].items_type = C_CDD_STR_LIT("string");

  op.parameters = params;
  op.n_parameters = 12;

  /* Request body: form urlencoded */
  op.req_body.ref_name = C_CDD_STR_LIT("ComplexForm");
  op.req_body.content_type = C_CDD_STR_LIT("application/x-www-form-urlencoded");

  memset(&mt_form, 0, sizeof(mt_form));
  mt_form.name = C_CDD_STR_LIT("application/x-www-form-urlencoded");
  mt_form.encoding = encs;
  mt_form.n_encoding = 6;

  encs[0].name = C_CDD_STR_LIT("res_val");
  encs[0].allow_reserved_set = 1;
  encs[0].allow_reserved = 1;

  encs[1].name = C_CDD_STR_LIT("arr_matrix");
  encs[1].style_set = 1;
  encs[1].style = OA_STYLE_MATRIX;

  encs[2].name = C_CDD_STR_LIT("obj_deep");
  encs[2].style_set = 1;
  encs[2].style = OA_STYLE_DEEP_OBJECT;
  encs[2].explode_set = 1;
  encs[2].explode = 1;
  encs[2].allow_reserved_set = 1;
  encs[2].allow_reserved = 1;

  encs[3].name = C_CDD_STR_LIT("obj_space");
  encs[3].style_set = 1;
  encs[3].style = OA_STYLE_SPACE_DELIMITED;

  encs[4].name = C_CDD_STR_LIT("obj_pipe");
  encs[4].style_set = 1;
  encs[4].style = OA_STYLE_PIPE_DELIMITED;

  encs[5].name = C_CDD_STR_LIT("obj_matrix");
  encs[5].style_set = 1;
  encs[5].style = OA_STYLE_MATRIX;

  encs[6].name = C_CDD_STR_LIT("arr_pipe");
  encs[6].style_set = 1;
  encs[6].style = OA_STYLE_PIPE_DELIMITED;

  encs[7].name = C_CDD_STR_LIT("arr_space");
  encs[7].style_set = 1;
  encs[7].style = OA_STYLE_SPACE_DELIMITED;

  op.req_body_media_types = &mt_form;
  op.n_req_body_media_types = 1;

  /* Responses: 200, 204, 1XX, 2XX, 3XX, 4XX, 5XX, default */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.ref_name = C_CDD_STR_LIT("SubModel");

  responses[1].code = C_CDD_STR_LIT("204");

  responses[2].code = C_CDD_STR_LIT("1XX");

  responses[3].code = C_CDD_STR_LIT("2XX");
  responses[3].content_type = C_CDD_STR_LIT("text/plain");
  responses[3].schema.inline_type = C_CDD_STR_LIT("string");

  responses[4].code = C_CDD_STR_LIT("3XX");

  responses[5].code = C_CDD_STR_LIT("4XX");

  responses[6].code = C_CDD_STR_LIT("5XX");

  responses[7].code = C_CDD_STR_LIT("default");
  responses[7].content_type = C_CDD_STR_LIT("application/octet-stream");

  op.responses = responses;
  op.n_responses = 8;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{path_id}",
                                 "\"https://override.api.com\"");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* Now test multipart body with part headers */
  op.req_body.ref_name = C_CDD_STR_LIT("ComplexForm");
  op.req_body.content_type = C_CDD_STR_LIT("multipart/form-data");

  memset(&mt_mp, 0, sizeof(mt_mp));
  mt_mp.name = C_CDD_STR_LIT("multipart/form-data");
  mt_mp.encoding = encs;
  mt_mp.n_encoding = 7;

  hdrs[0].name = C_CDD_STR_LIT("X-Part-Hdr");
  hdrs[0].type = C_CDD_STR_LIT("string");

  hdrs[1].name = C_CDD_STR_LIT("Content-Type");
  hdrs[1].type = C_CDD_STR_LIT("string");

  hdrs[2].name = C_CDD_STR_LIT("X-Array-Num");
  hdrs[2].type = C_CDD_STR_LIT("number");
  hdrs[2].is_array = 1;

  hdrs[3].name = C_CDD_STR_LIT("X-Array-Bool");
  hdrs[3].type = C_CDD_STR_LIT("boolean");
  hdrs[3].is_array = 1;

  hdrs[4].name = C_CDD_STR_LIT("X-Scalar-Bool");
  hdrs[4].type = C_CDD_STR_LIT("boolean");

  hdrs[5].name = C_CDD_STR_LIT("X-Obj-Hdr-Unexp");
  hdrs[5].type = C_CDD_STR_LIT("object");
  hdrs[5].explode_set = 1;
  hdrs[5].explode = 0;

  encs[0].name = C_CDD_STR_LIT("str_val");
  encs[0].headers = hdrs;
  encs[0].n_headers = 6;
  encs[0].content_type = C_CDD_STR_LIT("text/plain");

  encs[1].name = C_CDD_STR_LIT("obj_deep");
  encs[1].content_type = C_CDD_STR_LIT("application/json");

  encs[2].name = C_CDD_STR_LIT("int_val");
  encs[2].content_type = C_CDD_STR_LIT("text/plain");

  encs[3].name = C_CDD_STR_LIT("num_val");
  encs[3].content_type = C_CDD_STR_LIT("text/plain");

  encs[4].name = C_CDD_STR_LIT("bool_val");
  encs[4].content_type = C_CDD_STR_LIT("text/plain");

  encs[5].name = C_CDD_STR_LIT("arr_num");
  encs[5].content_type = C_CDD_STR_LIT("text/plain");

  encs[6].name = C_CDD_STR_LIT("arr_bool");
  encs[6].content_type = C_CDD_STR_LIT("text/plain");

  op.req_body_media_types = &mt_mp;
  op.n_req_body_media_types = 1;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{path_id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* Test cookie object with explode=0 and explode=1, with/without encoding */
  params[6].name = C_CDD_STR_LIT("cookie_obj_no_enc");
  params[6].in = OA_PARAM_IN_COOKIE;
  params[6].style = OA_STYLE_COOKIE;
  params[6].type = C_CDD_STR_LIT("object");
  params[6].is_array = 0;
  params[6].schema.ref_name = C_CDD_STR_LIT("SubModel");
  params[6].explode_set = 1;
  params[6].explode = 0;

  params[7].name = C_CDD_STR_LIT("cookie_obj_exp_no_enc");
  params[7].in = OA_PARAM_IN_COOKIE;
  params[7].style = OA_STYLE_COOKIE;
  params[7].type = C_CDD_STR_LIT("object");
  params[7].is_array = 0;
  params[7].schema.ref_name = C_CDD_STR_LIT("SubModel");
  params[7].explode_set = 1;
  params[7].explode = 1;

  params[8].name = C_CDD_STR_LIT("cookie_arr_num_unexp");
  params[8].in = OA_PARAM_IN_COOKIE;
  params[8].style = OA_STYLE_COOKIE;
  params[8].is_array = 1;
  params[8].items_type = C_CDD_STR_LIT("number");
  params[8].explode_set = 1;
  params[8].explode = 0;

  params[9].name = C_CDD_STR_LIT("cookie_arr_bool_unexp");
  params[9].in = OA_PARAM_IN_COOKIE;
  params[9].style = OA_STYLE_COOKIE;
  params[9].is_array = 1;
  params[9].items_type = C_CDD_STR_LIT("boolean");
  params[9].explode_set = 1;
  params[9].explode = 0;

  params[10].name = C_CDD_STR_LIT("cookie_arr_str_enc_unexp");
  params[10].in = OA_PARAM_IN_COOKIE;
  params[10].style = OA_STYLE_FORM;
  params[10].allow_reserved_set = 1;
  params[10].allow_reserved = 1;
  params[10].is_array = 1;
  params[10].items_type = C_CDD_STR_LIT("string");
  params[10].explode_set = 1;
  params[10].explode = 0;

  params[11].name = C_CDD_STR_LIT("cookie_str_enc");
  params[11].in = OA_PARAM_IN_COOKIE;
  params[11].style = OA_STYLE_FORM;
  params[11].type = C_CDD_STR_LIT("string");
  params[11].is_array = 0;
  params[11].allow_reserved_set = 1;
  params[11].allow_reserved = 1;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{path_id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* Systematic IO failure loop on form urlencoded */
  op.req_body.content_type = C_CDD_STR_LIT("application/x-www-form-urlencoded");
  op.req_body_media_types = &mt_form;
  op.n_req_body_media_types = 1;
  {
    int io_f;
    for (io_f = 0; io_f < 600; ++io_f) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_f;
      rc = codegen_client_write_body(fp, &op, &spec, "/items/{path_id}", NULL);
      g_fail_io_after = -1;
      fclose(fp);
      if (rc == CDD_C_SUCCESS)
        break;
    }
  }

  /* Systematic IO failure loop on multipart */
  op.req_body.content_type = C_CDD_STR_LIT("multipart/form-data");
  op.req_body_media_types = &mt_mp;
  op.n_req_body_media_types = 1;
  {
    int io_f;
    for (io_f = 0; io_f < 600; ++io_f) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_f;
      rc = codegen_client_write_body(fp, &op, &spec, "/items/{path_id}", NULL);
      g_fail_io_after = -1;
      fclose(fp);
      if (rc == CDD_C_SUCCESS)
        break;
    }
  }

  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Test additional coverage branches: default responses, inline types.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_default_responses_and_inlines(void) {
  FILE *fp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response responses[3];
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  memset(responses, 0, sizeof(responses));

  op.operation_id = C_CDD_STR_LIT("testDefaults");
  op.verb = OA_VERB_GET;
  op.method = C_CDD_STR_LIT("get");

  param.name = C_CDD_STR_LIT("id");
  param.in = OA_PARAM_IN_PATH;
  param.type = C_CDD_STR_LIT("string");
  op.parameters = &param;
  op.n_parameters = 1;

  /* default response with text/plain */
  responses[0].code = C_CDD_STR_LIT("default");
  responses[0].content_type = C_CDD_STR_LIT("text/plain");
  responses[0].schema.inline_type = C_CDD_STR_LIT("string");
  op.responses = responses;
  op.n_responses = 1;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* default response with inline non-string json */
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.inline_type = C_CDD_STR_LIT("integer");
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* default response error (generic) */
  responses[0].content_type = NULL;
  responses[0].schema.inline_type = NULL;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* 200 with inline non-string json */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.inline_type = C_CDD_STR_LIT("integer");
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* 2XX range response with inline non-string json */
  responses[0].code = C_CDD_STR_LIT("2XX");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.inline_type = C_CDD_STR_LIT("integer");
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* 2XX range response with binary */
  responses[0].code = C_CDD_STR_LIT("2XX");
  responses[0].content_type = C_CDD_STR_LIT("application/octet-stream");
  responses[0].schema.inline_type = NULL;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* 200 response with array schema ref */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[0].schema.is_array = 1;
  responses[0].schema.inline_type = NULL;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* default response matching 200 array schema */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[0].schema.is_array = 1;
  responses[1].code = C_CDD_STR_LIT("default");
  responses[1].content_type = C_CDD_STR_LIT("application/json");
  responses[1].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[1].schema.is_array = 1;
  op.n_responses = 2;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* only default response binary (no 2xx) */
  responses[0].code = C_CDD_STR_LIT("default");
  responses[0].content_type = C_CDD_STR_LIT("application/octet-stream");
  responses[0].schema.ref_name = NULL;
  responses[0].schema.is_array = 0;
  op.n_responses = 1;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* default response matching 200 scalar schema */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[0].schema.is_array = 0;
  responses[1].code = C_CDD_STR_LIT("default");
  responses[1].content_type = C_CDD_STR_LIT("application/json");
  responses[1].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[1].schema.is_array = 0;
  op.n_responses = 2;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* default response matching 200 inline type */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].schema.ref_name = NULL;
  responses[0].schema.inline_type = C_CDD_STR_LIT("integer");
  responses[0].schema.is_array = 0;
  responses[1].code = C_CDD_STR_LIT("default");
  responses[1].schema.ref_name = NULL;
  responses[1].schema.inline_type = C_CDD_STR_LIT("integer");
  responses[1].schema.is_array = 0;
  op.n_responses = 2;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* 200 success with default error */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[0].schema.is_array = 0;
  responses[1].code = C_CDD_STR_LIT("default");
  responses[1].schema.ref_name = NULL;
  responses[1].schema.inline_type = NULL;
  responses[1].content_type = NULL;
  op.n_responses = 2;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* 2XX with schema ref */
  responses[0].code = C_CDD_STR_LIT("2XX");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[0].schema.inline_type = NULL;
  responses[0].schema.is_array = 0;
  op.n_responses = 1;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* Non-literal, non-range response code */
  responses[0].code = C_CDD_STR_LIT("INVALID_CODE");
  op.n_responses = 1;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Test direct calls for part headers and form edge cases.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_direct_part_headers_and_form_edge_cases(void) {
  FILE *fp;
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Header hdrs[12];
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  cdd_c_error_t rc;

  memset(&enc, 0, sizeof(enc));
  memset(hdrs, 0, sizeof(hdrs));
  enc.name = C_CDD_STR_LIT("part");
  enc.headers = hdrs;
  enc.n_headers = 11;

  /* 0: null name */
  hdrs[0].name = NULL;

  /* 1: array integer */
  hdrs[1].name = C_CDD_STR_LIT("h_arr_int");
  hdrs[1].type = C_CDD_STR_LIT("array");
  hdrs[1].is_array = 1;
  hdrs[1].items_type = C_CDD_STR_LIT("integer");

  /* 2: array number */
  hdrs[2].name = C_CDD_STR_LIT("h_arr_num");
  hdrs[2].type = C_CDD_STR_LIT("array");
  hdrs[2].is_array = 1;
  hdrs[2].items_type = C_CDD_STR_LIT("number");

  /* 3: array boolean */
  hdrs[3].name = C_CDD_STR_LIT("h_arr_bool");
  hdrs[3].type = C_CDD_STR_LIT("array");
  hdrs[3].is_array = 1;
  hdrs[3].items_type = C_CDD_STR_LIT("boolean");

  /* 4: array string */
  hdrs[4].name = C_CDD_STR_LIT("h_arr_str");
  hdrs[4].type = C_CDD_STR_LIT("array");
  hdrs[4].is_array = 1;
  hdrs[4].items_type = C_CDD_STR_LIT("string");

  /* 5: object unexploded */
  hdrs[5].name = C_CDD_STR_LIT("h_obj_unexp");
  hdrs[5].type = C_CDD_STR_LIT("object");
  hdrs[5].is_array = 0;
  hdrs[5].explode_set = 1;
  hdrs[5].explode = 0;

  /* 6: scalar integer */
  hdrs[6].name = C_CDD_STR_LIT("h_sc_int");
  hdrs[6].type = C_CDD_STR_LIT("integer");
  hdrs[6].is_array = 0;

  /* 7: scalar number */
  hdrs[7].name = C_CDD_STR_LIT("h_sc_num");
  hdrs[7].type = C_CDD_STR_LIT("number");
  hdrs[7].is_array = 0;

  /* 8: scalar boolean */
  hdrs[8].name = C_CDD_STR_LIT("h_sc_bool");
  hdrs[8].type = C_CDD_STR_LIT("boolean");
  hdrs[8].is_array = 0;

  /* 9: object exploded */
  hdrs[9].name = C_CDD_STR_LIT("h_obj_exp");
  hdrs[9].type = C_CDD_STR_LIT("object");
  hdrs[9].is_array = 0;
  hdrs[9].explode_set = 1;
  hdrs[9].explode = 1;

  /* 10: scalar string */
  hdrs[10].name = C_CDD_STR_LIT("h_sc_str");
  hdrs[10].type = C_CDD_STR_LIT("string");
  hdrs[10].is_array = 0;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = client_body_write_multipart_part_headers(fp, &enc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* Loop I/O failures across write_multipart_part_headers */
  {
    int io_fail;
    for (io_fail = 0; io_fail < 50; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_multipart_part_headers(fp, &enc);
      g_fail_io_after = -1;
      fclose(fp);
    }
  }

  /* Test cookie array unencoded string */
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  param.name = C_CDD_STR_LIT("cookie_str_arr");
  param.in = OA_PARAM_IN_COOKIE;
  param.style = OA_STYLE_COOKIE;
  param.is_array = 1;
  param.items_type = C_CDD_STR_LIT("string");
  param.explode_set = 1;
  param.explode = 0;
  op.parameters = &param;
  op.n_parameters = 1;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = client_body_write_cookie_param_logic(fp, &op);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* Test form urlencoded object field with empty ref, missing schema, and
   * allow_reserved_set only */
  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "empty_obj", "object", "", NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "unknown_obj", "object",
                    "UnknownSchemaRef", NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "res_only_obj", "object",
                    "EmptyObjForm", NULL, NULL);
  c_cdd_strdup("EmptyObjForm", &spec.defined_schema_names[0]);
  spec.n_defined_schemas = 1;

  memset(&op, 0, sizeof(op));
  op.req_body.ref_name = C_CDD_STR_LIT("EmptyObjForm");
  op.req_body.content_type = C_CDD_STR_LIT("application/x-www-form-urlencoded");

  {
    struct OpenAPI_Encoding res_enc;
    struct OpenAPI_MediaType res_mt;
    memset(&res_enc, 0, sizeof(res_enc));
    memset(&res_mt, 0, sizeof(res_mt));
    res_enc.name = C_CDD_STR_LIT("res_only_obj");
    res_enc.allow_reserved_set = 1;
    res_enc.allow_reserved = 1;
    res_mt.name = C_CDD_STR_LIT("application/x-www-form-urlencoded");
    res_mt.encoding = &res_enc;
    res_mt.n_encoding = 1;
    op.req_body_media_types = &res_mt;
    op.n_req_body_media_types = 1;

    fp = cdd_test_tmpfile_global();
    ASSERT(fp != NULL);
    rc = client_body_write_form_urlencoded_body(fp, &op, &spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    fclose(fp);
  }

  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Test all inline JSON request body types (scalars and arrays).
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_inline_req_body_json_types(void) {
  FILE *fp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  cdd_c_error_t rc;
  const char *types[5];
  size_t t_idx;

  types[0] = "string";
  types[1] = "integer";
  types[2] = "number";
  types[3] = "boolean";
  types[4] = "custom_unknown";

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  op.operation_id = C_CDD_STR_LIT("testInlineJson");
  op.verb = OA_VERB_POST;
  op.method = C_CDD_STR_LIT("post");
  op.req_body.content_type = C_CDD_STR_LIT("application/json");

  resp.code = C_CDD_STR_LIT("200");
  op.responses = &resp;
  op.n_responses = 1;

  for (t_idx = 0; t_idx < 5; ++t_idx) {
    /* scalar */
    op.req_body.is_array = 0;
    op.req_body.inline_type = C_CDD_STR_LIT(types[t_idx]);
    fp = cdd_test_tmpfile_global();
    ASSERT(fp != NULL);
    rc = codegen_client_write_body(fp, &op, &spec, "/items", NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    fclose(fp);

    /* array */
    op.req_body.is_array = 1;
    fp = cdd_test_tmpfile_global();
    ASSERT(fp != NULL);
    rc = codegen_client_write_body(fp, &op, &spec, "/items", NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    fclose(fp);
  }

  /* Response with NULL code branch */
  resp.code = NULL;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Test querystring combined with security query.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_querystring_and_security_query(void) {
  FILE *fp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp;
  struct OpenAPI_SecurityScheme sch;
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  memset(&resp, 0, sizeof(resp));
  memset(&sch, 0, sizeof(sch));

  sch.type = OA_SEC_APIKEY;
  sch.in = OA_SEC_IN_QUERY;
  c_cdd_strdup("query_key", &sch.name);
  spec.security_schemes =
      (struct OpenAPI_SecurityScheme *)calloc(1, sizeof(sch));
  ASSERT(spec.security_schemes);
  spec.security_schemes[0] = sch;
  spec.n_security_schemes = 1;
  op.security_set = 0;
  spec.security_set = 0;

  op.operation_id = C_CDD_STR_LIT("testSecQuery");
  op.verb = OA_VERB_GET;
  op.method = C_CDD_STR_LIT("get");

  param.name = C_CDD_STR_LIT("qs");
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = C_CDD_STR_LIT("string");
  op.parameters = &param;
  op.n_parameters = 1;

  resp.code = C_CDD_STR_LIT("200");
  op.responses = &resp;
  op.n_responses = 1;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Systematic I/O failure loops to exercise all CHECK_IO branches.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_systematic_io_failures(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter params[4];
  struct OpenAPI_Response responses[4];
  int io_fail;
  FILE *fp;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);

  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "title", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "count", "integer", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "flag", "boolean", NULL, NULL,
                    NULL);
  c_cdd_strdup("Item", &spec.defined_schema_names[0]);
  spec.n_defined_schemas = 1;

  memset(&op, 0, sizeof(op));
  memset(params, 0, sizeof(params));
  memset(responses, 0, sizeof(responses));

  op.operation_id = C_CDD_STR_LIT("testIoSim");
  op.verb = OA_VERB_POST;
  op.method = C_CDD_STR_LIT("post");

  params[0].name = C_CDD_STR_LIT("id");
  params[0].in = OA_PARAM_IN_PATH;
  params[0].type = C_CDD_STR_LIT("string");

  params[1].name = C_CDD_STR_LIT("q");
  params[1].in = OA_PARAM_IN_QUERY;
  params[1].type = C_CDD_STR_LIT("string");

  params[2].name = C_CDD_STR_LIT("h");
  params[2].in = OA_PARAM_IN_HEADER;
  params[2].type = C_CDD_STR_LIT("string");

  params[3].name = C_CDD_STR_LIT("c");
  params[3].in = OA_PARAM_IN_COOKIE;
  params[3].type = C_CDD_STR_LIT("string");

  op.parameters = params;
  op.n_parameters = 4;

  op.req_body.ref_name = C_CDD_STR_LIT("Item");
  op.req_body.content_type = C_CDD_STR_LIT("application/json");

  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");

  responses[1].code = C_CDD_STR_LIT("2XX");
  responses[1].content_type = C_CDD_STR_LIT("text/plain");
  responses[1].schema.inline_type = C_CDD_STR_LIT("string");

  responses[2].code = C_CDD_STR_LIT("400");
  responses[3].code = C_CDD_STR_LIT("default");
  responses[3].content_type = C_CDD_STR_LIT("application/octet-stream");

  op.responses = responses;
  op.n_responses = 4;

  /* Run I/O failure loop until success */
  for (io_fail = 0; io_fail < 300; ++io_fail) {
    cdd_c_error_t rc;
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* Now repeat with form urlencoded body to trip form CHECK_IO branches */
  op.req_body.content_type = C_CDD_STR_LIT("application/x-www-form-urlencoded");
  for (io_fail = 0; io_fail < 300; ++io_fail) {
    cdd_c_error_t rc;
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* Now repeat with multipart body to trip multipart CHECK_IO branches */
  op.req_body.content_type = C_CDD_STR_LIT("multipart/form-data");
  for (io_fail = 0; io_fail < 300; ++io_fail) {
    cdd_c_error_t rc;
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* Loop with 200 binary response */
  op.req_body.content_type = C_CDD_STR_LIT("application/json");
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].content_type = C_CDD_STR_LIT("application/octet-stream");
  responses[0].schema.ref_name = NULL;
  responses[0].schema.inline_type = NULL;
  for (io_fail = 0; io_fail < 100; ++io_fail) {
    cdd_c_error_t rc;
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* Loop with 200 text/plain response */
  responses[0].content_type = C_CDD_STR_LIT("text/plain");
  responses[0].schema.inline_type = C_CDD_STR_LIT("string");
  for (io_fail = 0; io_fail < 100; ++io_fail) {
    cdd_c_error_t rc;
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* Loop with 2XX binary and default text/plain */
  responses[0].code = C_CDD_STR_LIT("404");
  responses[1].code = C_CDD_STR_LIT("2XX");
  responses[1].content_type = C_CDD_STR_LIT("application/octet-stream");
  responses[1].schema.inline_type = NULL;
  responses[3].code = C_CDD_STR_LIT("default");
  responses[3].content_type = C_CDD_STR_LIT("text/plain");
  responses[3].schema.inline_type = C_CDD_STR_LIT("string");
  for (io_fail = 0; io_fail < 100; ++io_fail) {
    cdd_c_error_t rc;
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* Loop with default response array schema */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[0].schema.is_array = 1;
  responses[3].code = C_CDD_STR_LIT("default");
  responses[3].content_type = C_CDD_STR_LIT("application/json");
  responses[3].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[3].schema.is_array = 1;
  for (io_fail = 0; io_fail < 100; ++io_fail) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
  }

  /* Loop form urlencoded with arr_pipe */
  {
    struct OpenAPI_Encoding pipe_enc;
    struct OpenAPI_MediaType pipe_mt;
    struct OpenAPI_Spec pipe_spec;
    struct OpenAPI_Operation pipe_op;

    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&pipe_spec));
    pipe_spec.defined_schemas =
        (struct StructFields *)calloc(1, sizeof(struct StructFields));
    pipe_spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
    ASSERT(pipe_spec.defined_schemas);
    ASSERT(pipe_spec.defined_schema_names);
    struct_fields_init(&pipe_spec.defined_schemas[0]);
    struct_fields_add(&pipe_spec.defined_schemas[0], "arr_p", "array", "string",
                      NULL, NULL);
    c_cdd_strdup("PipeSchema", &pipe_spec.defined_schema_names[0]);
    pipe_spec.n_defined_schemas = 1;

    memset(&pipe_op, 0, sizeof(pipe_op));
    memset(&pipe_enc, 0, sizeof(pipe_enc));
    memset(&pipe_mt, 0, sizeof(pipe_mt));

    pipe_op.req_body.ref_name = C_CDD_STR_LIT("PipeSchema");
    pipe_op.req_body.content_type =
        C_CDD_STR_LIT("application/x-www-form-urlencoded");

    pipe_enc.name = C_CDD_STR_LIT("arr_p");
    pipe_enc.style_set = 1;
    pipe_enc.style = OA_STYLE_PIPE_DELIMITED;

    pipe_mt.name = C_CDD_STR_LIT("application/x-www-form-urlencoded");
    pipe_mt.encoding = &pipe_enc;
    pipe_mt.n_encoding = 1;
    pipe_op.req_body_media_types = &pipe_mt;
    pipe_op.n_req_body_media_types = 1;

    for (io_fail = 0; io_fail < 40; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_form_urlencoded_body(fp, &pipe_op, &pipe_spec);
      g_fail_io_after = -1;
      fclose(fp);
    }
    openapi_spec_free(&pipe_spec);
  }

  /* Loop with security header scheme */
  {
    struct OpenAPI_SecurityScheme s_sch;
    memset(&s_sch, 0, sizeof(s_sch));
    s_sch.type = OA_SEC_APIKEY;
    s_sch.in = OA_SEC_IN_HEADER;
    c_cdd_strdup("s_key", &s_sch.name);
    spec.security_schemes =
        (struct OpenAPI_SecurityScheme *)calloc(1, sizeof(s_sch));
    ASSERT(spec.security_schemes);
    spec.security_schemes[0] = s_sch;
    spec.n_security_schemes = 1;
    spec.security_set = 0;
    op.security_set = 0;
    for (io_fail = 0; io_fail < 40; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
      g_fail_io_after = -1;
      fclose(fp);
    }
  }

  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Test targeted remaining branches to reach 100% coverage.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_targeted_remaining_branches(void) {
  FILE *fp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Encoding encs[15];
  struct OpenAPI_Header hdrs[4];
  struct OpenAPI_MediaType mt;
  int io_fail;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(2, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(4, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);

  /* Schema with all multipart types */
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "f_str", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_int", "integer", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_num", "number", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_bool", "boolean", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_obj", "object", "SubM", NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_arr_str", "array", "string",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_arr_int", "array", "integer",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_arr_num", "array", "number",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_arr_bool", "array", "boolean",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_arr_obj", "array", "SubM",
                    NULL, NULL);
  c_cdd_strdup("MultiAll", &spec.defined_schema_names[0]);

  struct_fields_init(&spec.defined_schemas[1]);
  struct_fields_add(&spec.defined_schemas[1], "id", "integer", NULL, NULL,
                    NULL);
  c_cdd_strdup("SubM", &spec.defined_schema_names[1]);
  spec.n_defined_schemas = 2;

  memset(&op, 0, sizeof(op));
  memset(encs, 0, sizeof(encs));
  memset(hdrs, 0, sizeof(hdrs));
  memset(&mt, 0, sizeof(mt));

  hdrs[0].name = C_CDD_STR_LIT("X-Custom-Hdr");
  hdrs[0].type = C_CDD_STR_LIT("string");

  encs[0].name = C_CDD_STR_LIT("f_str");
  encs[0].headers = hdrs;
  encs[0].n_headers = 1;

  encs[1].name = C_CDD_STR_LIT("f_int");
  encs[1].headers = hdrs;
  encs[1].n_headers = 1;

  encs[2].name = C_CDD_STR_LIT("f_num");
  encs[2].headers = hdrs;
  encs[2].n_headers = 1;

  encs[3].name = C_CDD_STR_LIT("f_bool");
  encs[3].headers = hdrs;
  encs[3].n_headers = 1;

  encs[4].name = C_CDD_STR_LIT("f_obj");
  encs[4].headers = hdrs;
  encs[4].n_headers = 1;

  encs[5].name = C_CDD_STR_LIT("f_arr_str");
  encs[5].headers = hdrs;
  encs[5].n_headers = 1;

  encs[6].name = C_CDD_STR_LIT("f_arr_int");
  encs[6].headers = hdrs;
  encs[6].n_headers = 1;

  encs[7].name = C_CDD_STR_LIT("f_arr_num");
  encs[7].headers = hdrs;
  encs[7].n_headers = 1;

  encs[8].name = C_CDD_STR_LIT("f_arr_bool");
  encs[8].headers = hdrs;
  encs[8].n_headers = 1;

  encs[9].name = C_CDD_STR_LIT("f_arr_obj");
  encs[9].headers = hdrs;
  encs[9].n_headers = 1;

  mt.name = C_CDD_STR_LIT("multipart/form-data");
  mt.encoding = encs;
  mt.n_encoding = 10;

  op.operation_id = C_CDD_STR_LIT("testMultiAll");
  op.verb = OA_VERB_POST;
  op.method = C_CDD_STR_LIT("post");
  op.req_body.ref_name = C_CDD_STR_LIT("MultiAll");
  op.req_body.content_type = C_CDD_STR_LIT("multipart/form-data");
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  /* Run I/O loop across client_body_write_multipart_body to trip every single
   * write_multipart_part_headers return error */
  for (io_fail = 0; io_fail < 350; ++io_fail) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    client_body_write_multipart_body(fp, &op, &spec);
    g_fail_io_after = -1;
    fclose(fp);
  }

  /* Run I/O loop on operation with req_json, cookie_str, 200 binary, 2XX text,
   * default */
  {
    struct OpenAPI_Parameter t_params[3];
    struct OpenAPI_Response t_responses[4];
    struct OpenAPI_Operation t_op;

    memset(&t_op, 0, sizeof(t_op));
    memset(t_params, 0, sizeof(t_params));
    memset(t_responses, 0, sizeof(t_responses));

    t_op.operation_id = C_CDD_STR_LIT("testCleanupIo");
    t_op.verb = OA_VERB_POST;
    t_op.method = C_CDD_STR_LIT("post");
    t_op.req_body.ref_name = C_CDD_STR_LIT("MultiAll");
    t_op.req_body.content_type = C_CDD_STR_LIT("application/json");

    t_params[0].name = C_CDD_STR_LIT("cookie_c");
    t_params[0].in = OA_PARAM_IN_COOKIE;
    t_params[0].type = C_CDD_STR_LIT("string");

    t_params[1].name = C_CDD_STR_LIT("query_q");
    t_params[1].in = OA_PARAM_IN_QUERY;
    t_params[1].type = C_CDD_STR_LIT("string");

    t_op.parameters = t_params;
    t_op.n_parameters = 2;

    t_responses[0].code = C_CDD_STR_LIT("200");
    t_responses[0].content_type = C_CDD_STR_LIT("application/octet-stream");

    t_responses[1].code = C_CDD_STR_LIT("2XX");
    t_responses[1].content_type = C_CDD_STR_LIT("text/plain");
    t_responses[1].schema.inline_type = C_CDD_STR_LIT("string");

    t_responses[2].code = C_CDD_STR_LIT("default");
    t_responses[2].content_type = C_CDD_STR_LIT("application/octet-stream");

    t_op.responses = t_responses;
    t_op.n_responses = 3;

    for (io_fail = 0; io_fail < 150; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      codegen_client_write_body(fp, &t_op, &spec, "/test", NULL);
      g_fail_io_after = -1;
      fclose(fp);
    }

    /* Form body cleanup loop */
    t_op.req_body.content_type =
        C_CDD_STR_LIT("application/x-www-form-urlencoded");
    for (io_fail = 0; io_fail < 150; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      codegen_client_write_body(fp, &t_op, &spec, "/test", NULL);
      g_fail_io_after = -1;
      fclose(fp);
    }
  }

  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Test all remaining sub-writer IO failures to achieve 100% coverage.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_all_remaining_sub_writer_io_failures(void) {
  FILE *fp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response responses[2];
  struct OpenAPI_SecurityScheme sch;
  int io_fail;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  memset(&op, 0, sizeof(op));
  memset(responses, 0, sizeof(responses));
  memset(&sch, 0, sizeof(sch));

  op.operation_id = C_CDD_STR_LIT("testSubIo");
  op.verb = OA_VERB_GET;
  op.method = C_CDD_STR_LIT("get");

  /* 1. Security header apply IO failure (hits line 4393) */
  sch.type = OA_SEC_APIKEY;
  sch.in = OA_SEC_IN_HEADER;
  c_cdd_strdup("sec_key", &sch.name);
  c_cdd_strdup("X-Api-Key", &sch.key_name);
  spec.security_schemes =
      (struct OpenAPI_SecurityScheme *)calloc(1, sizeof(sch));
  ASSERT(spec.security_schemes);
  spec.security_schemes[0] = sch;
  spec.n_security_schemes = 1;
  op.security_set = 0;
  spec.security_set = 0;

  responses[0].code = C_CDD_STR_LIT("200");
  op.responses = responses;
  op.n_responses = 1;

  for (io_fail = 0; io_fail < 30; ++io_fail) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    codegen_client_write_body(fp, &op, &spec, "/test", NULL);
    g_fail_io_after = -1;
    fclose(fp);
  }

  /* Clear security schemes */
  openapi_spec_free(&spec);
  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));

  /* 2. Range 2XX binary write_binary_success failure (hits line 4789) */
  responses[0].code = C_CDD_STR_LIT("2XX");
  responses[0].content_type = C_CDD_STR_LIT("application/octet-stream");
  responses[0].schema.ref_name = NULL;
  responses[0].schema.inline_type = NULL;
  op.responses = responses;
  op.n_responses = 1;

  for (io_fail = 0; io_fail < 40; ++io_fail) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    codegen_client_write_body(fp, &op, &spec, "/test", NULL);
    g_fail_io_after = -1;
    fclose(fp);
  }

  /* 3. Range 2XX inline json write_inline_json_parse failure (hits line 4808)
   */
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.inline_type = C_CDD_STR_LIT("integer");
  for (io_fail = 0; io_fail < 40; ++io_fail) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    codegen_client_write_body(fp, &op, &spec, "/test", NULL);
    g_fail_io_after = -1;
    fclose(fp);
  }

  /* 4. Default response binary failure when no 2xx exists (hits line 4874) */
  responses[0].code = C_CDD_STR_LIT("400");
  responses[0].content_type = NULL;
  responses[0].schema.inline_type = NULL;
  responses[1].code = C_CDD_STR_LIT("default");
  responses[1].content_type = C_CDD_STR_LIT("application/octet-stream");
  responses[1].schema.ref_name = NULL;
  responses[1].schema.inline_type = NULL;
  op.n_responses = 2;

  for (io_fail = 0; io_fail < 50; ++io_fail) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    codegen_client_write_body(fp, &op, &spec, "/test", NULL);
    g_fail_io_after = -1;
    fclose(fp);
  }

  /* 5. Default response text/plain failure when no 2xx exists (hits line 4879)
   */
  responses[1].content_type = C_CDD_STR_LIT("text/plain");
  responses[1].schema.inline_type = C_CDD_STR_LIT("string");
  for (io_fail = 0; io_fail < 50; ++io_fail) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    codegen_client_write_body(fp, &op, &spec, "/test", NULL);
    g_fail_io_after = -1;
    fclose(fp);
  }

  /* 6. Default response inline json failure when no 2xx exists (hits line 4899)
   */
  responses[1].content_type = C_CDD_STR_LIT("application/json");
  responses[1].schema.inline_type = C_CDD_STR_LIT("integer");
  for (io_fail = 0; io_fail < 50; ++io_fail) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    codegen_client_write_body(fp, &op, &spec, "/test", NULL);
    g_fail_io_after = -1;
    fclose(fp);
  }

  /* 7. Inline json parse IO failures */
  {
    struct OpenAPI_SchemaRef sch_ref;
    const char *types[4];
    size_t k;
    types[0] = "string";
    types[1] = "integer";
    types[2] = "number";
    types[3] = "boolean";
    memset(&sch_ref, 0, sizeof(sch_ref));
    for (k = 0; k < 4; ++k) {
      sch_ref.inline_type = C_CDD_STR_LIT(types[k]);
      sch_ref.is_array = 0;
      for (io_fail = 0; io_fail < 25; ++io_fail) {
        fp = cdd_test_tmpfile_global();
        if (!fp)
          break;
        g_io_calls = 0;
        g_fail_io_after = io_fail;
        client_body_write_inline_json_parse(fp, &sch_ref);
        g_fail_io_after = -1;
        fclose(fp);
      }
      sch_ref.is_array = 1;
      for (io_fail = 0; io_fail < 50; ++io_fail) {
        fp = cdd_test_tmpfile_global();
        if (!fp)
          break;
        g_io_calls = 0;
        g_fail_io_after = io_fail;
        client_body_write_inline_json_parse(fp, &sch_ref);
        g_fail_io_after = -1;
        fclose(fp);
      }
    }
  }

  /* 8. Joined form array IO failures */
  {
    for (io_fail = 0; io_fail < 70; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_joined_form_array(fp, "tags", "n_tags", "string", ',',
                                          "url_encode", 1, 0);
      g_fail_io_after = -1;
      fclose(fp);
    }
    for (io_fail = 0; io_fail < 70; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_joined_form_array(fp, "tags", "n_tags", "string", ',',
                                          NULL, 0, 0);
      g_fail_io_after = -1;
      fclose(fp);
    }
    for (io_fail = 0; io_fail < 70; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_joined_form_array(fp, "items", "n_items", "Item", '&',
                                          NULL, 0, 1);
      g_fail_io_after = -1;
      fclose(fp);
    }
    for (io_fail = 0; io_fail < 70; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_joined_form_array(fp, "ints", "n_ints", "integer", '|',
                                          "url_encode", 0, 0);
      g_fail_io_after = -1;
      fclose(fp);
    }
    for (io_fail = 0; io_fail < 70; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_joined_form_array(fp, "nums", "n_nums", "number", '|',
                                          "url_encode", 0, 0);
      g_fail_io_after = -1;
      fclose(fp);
    }
    for (io_fail = 0; io_fail < 70; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_joined_form_array(fp, "bools", "n_bools", "boolean",
                                          '|', "url_encode", 0, 0);
      g_fail_io_after = -1;
      fclose(fp);
    }
    for (io_fail = 0; io_fail < 70; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_joined_form_array(fp, "str_raw", "n_str_raw", "string",
                                          '|', "url_encode", 0, 0);
      g_fail_io_after = -1;
      fclose(fp);
    }
  }

  /* 9. Header param logic IO failures */
  {
    struct OpenAPI_Operation hdr_op;
    struct OpenAPI_Parameter hdr_params[14];
    memset(&hdr_op, 0, sizeof(hdr_op));
    memset(hdr_params, 0, sizeof(hdr_params));

    /* JSON array types */
    hdr_params[0].name = C_CDD_STR_LIT("h_j_arr_str");
    hdr_params[0].in = OA_PARAM_IN_HEADER;
    hdr_params[0].content_type = C_CDD_STR_LIT("application/json");
    hdr_params[0].is_array = 1;
    hdr_params[0].items_type = C_CDD_STR_LIT("string");

    hdr_params[1].name = C_CDD_STR_LIT("h_j_arr_int");
    hdr_params[1].in = OA_PARAM_IN_HEADER;
    hdr_params[1].content_type = C_CDD_STR_LIT("application/json");
    hdr_params[1].is_array = 1;
    hdr_params[1].items_type = C_CDD_STR_LIT("integer");

    hdr_params[2].name = C_CDD_STR_LIT("h_j_arr_num");
    hdr_params[2].in = OA_PARAM_IN_HEADER;
    hdr_params[2].content_type = C_CDD_STR_LIT("application/json");
    hdr_params[2].is_array = 1;
    hdr_params[2].items_type = C_CDD_STR_LIT("number");

    hdr_params[3].name = C_CDD_STR_LIT("h_j_arr_bool");
    hdr_params[3].in = OA_PARAM_IN_HEADER;
    hdr_params[3].content_type = C_CDD_STR_LIT("application/json");
    hdr_params[3].is_array = 1;
    hdr_params[3].items_type = C_CDD_STR_LIT("boolean");

    hdr_params[4].name = C_CDD_STR_LIT("h_j_arr_obj");
    hdr_params[4].in = OA_PARAM_IN_HEADER;
    hdr_params[4].content_type = C_CDD_STR_LIT("application/json");
    hdr_params[4].is_array = 1;
    hdr_params[4].items_type = C_CDD_STR_LIT("Item");

    /* JSON scalar types */
    hdr_params[5].name = C_CDD_STR_LIT("h_j_sc_str");
    hdr_params[5].in = OA_PARAM_IN_HEADER;
    hdr_params[5].content_type = C_CDD_STR_LIT("application/json");
    hdr_params[5].type = C_CDD_STR_LIT("string");

    hdr_params[6].name = C_CDD_STR_LIT("h_j_sc_int");
    hdr_params[6].in = OA_PARAM_IN_HEADER;
    hdr_params[6].content_type = C_CDD_STR_LIT("application/json");
    hdr_params[6].type = C_CDD_STR_LIT("integer");

    hdr_params[7].name = C_CDD_STR_LIT("h_j_sc_num");
    hdr_params[7].in = OA_PARAM_IN_HEADER;
    hdr_params[7].content_type = C_CDD_STR_LIT("application/json");
    hdr_params[7].type = C_CDD_STR_LIT("number");

    hdr_params[8].name = C_CDD_STR_LIT("h_j_sc_bool");
    hdr_params[8].in = OA_PARAM_IN_HEADER;
    hdr_params[8].content_type = C_CDD_STR_LIT("application/json");
    hdr_params[8].type = C_CDD_STR_LIT("boolean");

    hdr_params[9].name = C_CDD_STR_LIT("h_j_sc_obj");
    hdr_params[9].in = OA_PARAM_IN_HEADER;
    hdr_params[9].content_type = C_CDD_STR_LIT("application/json");
    hdr_params[9].type = C_CDD_STR_LIT("Item");

    /* Standard header types */
    hdr_params[10].name = C_CDD_STR_LIT("h_std_sc_str");
    hdr_params[10].in = OA_PARAM_IN_HEADER;
    hdr_params[10].type = C_CDD_STR_LIT("string");

    hdr_params[11].name = C_CDD_STR_LIT("h_std_sc_int");
    hdr_params[11].in = OA_PARAM_IN_HEADER;
    hdr_params[11].type = C_CDD_STR_LIT("integer");

    hdr_params[12].name = C_CDD_STR_LIT("h_std_sc_num");
    hdr_params[12].in = OA_PARAM_IN_HEADER;
    hdr_params[12].type = C_CDD_STR_LIT("number");

    hdr_params[13].name = C_CDD_STR_LIT("h_std_sc_bool");
    hdr_params[13].in = OA_PARAM_IN_HEADER;
    hdr_params[13].type = C_CDD_STR_LIT("boolean");

    hdr_op.parameters = hdr_params;
    hdr_op.n_parameters = 14;

    for (io_fail = 0; io_fail < 150; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_header_param_logic(fp, &hdr_op);
      g_fail_io_after = -1;
      fclose(fp);
    }
  }

  /* 10. Cookie param logic IO failures */
  {
    struct OpenAPI_Operation ck_op;
    struct OpenAPI_Parameter ck_params[10];
    memset(&ck_op, 0, sizeof(ck_op));
    memset(ck_params, 0, sizeof(ck_params));

    ck_params[0].name = C_CDD_STR_LIT("ck1");
    ck_params[0].in = OA_PARAM_IN_COOKIE;
    ck_params[0].type = C_CDD_STR_LIT("string");
    ck_params[0].style = OA_STYLE_FORM;

    ck_params[1].name = C_CDD_STR_LIT("ck2");
    ck_params[1].in = OA_PARAM_IN_COOKIE;
    ck_params[1].type = C_CDD_STR_LIT("integer");

    ck_params[2].name = C_CDD_STR_LIT("ck3");
    ck_params[2].in = OA_PARAM_IN_COOKIE;
    ck_params[2].is_array = 1;
    ck_params[2].items_type = C_CDD_STR_LIT("string");
    ck_params[2].style = OA_STYLE_FORM;

    ck_params[3].name = C_CDD_STR_LIT("ck4");
    ck_params[3].in = OA_PARAM_IN_COOKIE;
    ck_params[3].is_array = 1;
    ck_params[3].items_type = C_CDD_STR_LIT("integer");
    ck_params[3].explode_set = 1;
    ck_params[3].explode = 0;

    ck_params[4].name = C_CDD_STR_LIT("ck5");
    ck_params[4].in = OA_PARAM_IN_COOKIE;
    ck_params[4].type = C_CDD_STR_LIT("object");
    ck_params[4].schema.ref_name = C_CDD_STR_LIT("Item");
    ck_params[4].style = OA_STYLE_FORM;
    ck_params[4].explode_set = 1;
    ck_params[4].explode = 1;

    ck_params[5].name = C_CDD_STR_LIT("ck6");
    ck_params[5].in = OA_PARAM_IN_COOKIE;
    ck_params[5].type = C_CDD_STR_LIT("object");
    ck_params[5].schema.ref_name = C_CDD_STR_LIT("Item");
    ck_params[5].style = OA_STYLE_FORM;
    ck_params[5].explode_set = 1;
    ck_params[5].explode = 0;

    ck_params[6].name = C_CDD_STR_LIT("ck7");
    ck_params[6].in = OA_PARAM_IN_COOKIE;
    ck_params[6].type = C_CDD_STR_LIT("number");

    ck_params[7].name = C_CDD_STR_LIT("ck8");
    ck_params[7].in = OA_PARAM_IN_COOKIE;
    ck_params[7].type = C_CDD_STR_LIT("boolean");

    ck_params[8].name = C_CDD_STR_LIT("ck9");
    ck_params[8].in = OA_PARAM_IN_COOKIE;
    ck_params[8].is_array = 1;
    ck_params[8].items_type = C_CDD_STR_LIT("number");
    ck_params[8].explode_set = 1;
    ck_params[8].explode = 0;

    ck_params[9].name = C_CDD_STR_LIT("ck10");
    ck_params[9].in = OA_PARAM_IN_COOKIE;
    ck_params[9].is_array = 1;
    ck_params[9].items_type = C_CDD_STR_LIT("boolean");
    ck_params[9].explode_set = 1;
    ck_params[9].explode = 0;

    ck_op.parameters = ck_params;
    ck_op.n_parameters = 10;

    for (io_fail = 0; io_fail < 200; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      if (!fp)
        break;
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_cookie_param_logic(fp, &ck_op);
      g_fail_io_after = -1;
      fclose(fp);
    }
  }

  openapi_spec_free(&spec);
  PASS();
}

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
  if (!fp)
    return;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  codegen_client_write_body(fp, op, spec, path, NULL);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
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
  if (!fp)
    return;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_header_param_logic(fp, op);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
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
  if (!fp)
    return;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_cookie_param_logic(fp, op);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
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
  if (!fp)
    return;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_form_urlencoded_body(fp, op, spec);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
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
  if (!fp)
    return;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_multipart_body(fp, op, spec);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
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
  if (!fp)
    return;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_multipart_part_headers(fp, enc);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
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
  if (!fp)
    return;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_joined_form_array(fp, field, len_field, items_type, delim,
                                      encode_fn, add_encoded, is_object);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
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
  if (!fp)
    return;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  client_body_write_inline_json_parse(fp, schema);
  total_calls = g_io_calls;
  fclose(fp);
  for (i = 0; i <= total_calls; ++i) {
    fp = cdd_test_tmpfile_global();
    if (!fp)
      break;
    g_io_calls = 0;
    g_fail_io_after = i;
    client_body_write_inline_json_parse(fp, schema);
    g_fail_io_after = -1;
    fclose(fp);
  }
}

TEST test_client_body_exhaustive_100_percent_coverage(void) {
  FILE *fp;
  cdd_c_error_t rc;

  /* Section 1: Helper predicates and edge cases */
  {
    struct OpenAPI_MediaType mts[2];
    const struct OpenAPI_MediaType *found_mt = NULL;
    struct OpenAPI_MediaType mt_single;
    struct OpenAPI_Encoding encs[2];
    struct OpenAPI_Encoding *found_enc = NULL;
    int has = 0;
    char ct_buf[64];
    const char *out_val = NULL;
    char san_buf[64];
    char mhp_buf[128];
    struct OpenAPI_SchemaRef empty_sch;
    int is_range = 0;
    int is_lit = 0;
    int pfx = 0;

    /* find_media_type branches */
    memset(mts, 0, sizeof(mts));
    mts[0].name = NULL;
    mts[1].name = C_CDD_STR_LIT("target_mt");
    rc = client_body_find_media_type(mts, 2, "target_mt", &found_mt);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(&mts[1], found_mt);

    /* find_encoding branches */
    memset(&mt_single, 0, sizeof(mt_single));
    memset(encs, 0, sizeof(encs));
    encs[0].name = NULL;
    encs[1].name = C_CDD_STR_LIT("target_enc");
    mt_single.encoding = encs;
    mt_single.n_encoding = 2;
    rc = client_body_find_encoding(&mt_single, "target_enc", &found_enc);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(&encs[1], found_enc);

    /* media_type_has_suffix branches */
    rc = client_body_media_type_has_suffix("application/json", NULL, &has);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, has);

    /* first_content_type_entry branches */
    rc = client_body_first_content_type_entry("app/json", NULL, sizeof(ct_buf),
                                              &out_val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = client_body_first_content_type_entry("app/json", ct_buf, 0, &out_val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = client_body_first_content_type_entry(NULL, ct_buf, sizeof(ct_buf),
                                              &out_val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = client_body_first_content_type_entry("   ", ct_buf, sizeof(ct_buf),
                                              &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("", ct_buf);
    rc = client_body_first_content_type_entry("abcdef", ct_buf, 2, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* sanitize_ident branches */
    rc = client_body_sanitize_ident(san_buf, 0, "test");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = client_body_sanitize_ident(san_buf, sizeof(san_buf), NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = client_body_sanitize_ident(san_buf, sizeof(san_buf), "{foo|bar~}");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = client_body_sanitize_ident(san_buf, 3, "abcdef");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("ab", san_buf);

    /* multipart_header_param_name branches */
    rc = client_body_multipart_header_param_name(mhp_buf, 0, "f", "h");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = client_body_multipart_header_param_name(mhp_buf, sizeof(mhp_buf), NULL,
                                                 "h");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = client_body_multipart_header_param_name(mhp_buf, sizeof(mhp_buf), "f",
                                                 NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* write_inline_json_parse null branches */
    memset(&empty_sch, 0, sizeof(empty_sch));
    fp = cdd_test_tmpfile_global();
    ASSERT(fp);
    rc = client_body_write_inline_json_parse(fp, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = client_body_write_inline_json_parse(fp, &empty_sch);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    fclose(fp);

    /* write_joined_form_array branches */
    fp = cdd_test_tmpfile_global();
    ASSERT(fp);
    rc = client_body_write_joined_form_array(fp, NULL, "n", "string", ',', NULL,
                                             0, 0);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = client_body_write_joined_form_array(fp, "f", NULL, "string", ',', NULL,
                                             0, 0);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = client_body_write_joined_form_array(fp, "f", "n", NULL, ',', NULL, 0,
                                             0);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    /* empty encode_fn */
    rc = client_body_write_joined_form_array(fp, "f", "n", "string", ',', "", 0,
                                             0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    /* integer items_type with encode_fn and without */
    rc = client_body_write_joined_form_array(fp, "f", "n", "integer", ',', NULL,
                                             0, 0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = client_body_write_joined_form_array(fp, "f", "n", "integer", ',',
                                             "url_encode", 1, 0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    /* number items_type with encode_fn and without */
    rc = client_body_write_joined_form_array(fp, "f", "n", "number", ',', NULL,
                                             0, 0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = client_body_write_joined_form_array(fp, "f", "n", "number", ',',
                                             "url_encode", 1, 0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    /* boolean items_type with encode_fn and without */
    rc = client_body_write_joined_form_array(fp, "f", "n", "boolean", ',', NULL,
                                             0, 0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = client_body_write_joined_form_array(fp, "f", "n", "boolean", ',',
                                             "url_encode", 1, 0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    /* custom / fallback items_type */
    rc = client_body_write_joined_form_array(fp, "f", "n", "CustomType", ',',
                                             NULL, 0, 0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    fclose(fp);

    /* Exhaustive IO loop on joined form array */
    test_helper_run_io_joined_form_array("f", "n", "integer", ',', NULL, 0, 0);
    test_helper_run_io_joined_form_array("f", "n", "integer", ',', "url_encode",
                                         1, 0);
    test_helper_run_io_joined_form_array("f", "n", "number", ',', NULL, 0, 0);
    test_helper_run_io_joined_form_array("f", "n", "number", ',', "url_encode",
                                         1, 0);
    test_helper_run_io_joined_form_array("f", "n", "boolean", ',', NULL, 0, 0);
    test_helper_run_io_joined_form_array("f", "n", "boolean", ',', "url_encode",
                                         1, 0);

    /* Bad inline schema type IO loop for line 941 */
    {
      struct OpenAPI_SchemaRef bad_inl_sch;
      memset(&bad_inl_sch, 0, sizeof(bad_inl_sch));
      bad_inl_sch.inline_type = C_CDD_STR_LIT("unsupported");
      bad_inl_sch.is_array = 1;
      test_helper_run_io_inline_json_parse(&bad_inl_sch);
    }

    /* status range and literal codes */
    rc = client_body_is_status_range_code("20", &is_range);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_range);
    rc = client_body_is_status_range_code("2000", &is_range);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_range);
    rc = client_body_is_status_range_code("0XX", &is_range);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_range);
    rc = client_body_is_status_range_code("6XX", &is_range);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_range);
    rc = client_body_is_status_range_code("2AX", &is_range);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_range);
    rc = client_body_is_status_range_code("2XA", &is_range);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_range);
    rc = client_body_is_status_range_code("2XX", &is_range);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, is_range);

    rc = client_body_status_range_prefix("2XX", &pfx);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(2, pfx);

    rc = client_body_is_status_code_literal("/00", &is_lit);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_lit);
    rc = client_body_is_status_code_literal(":00", &is_lit);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_lit);
    rc = client_body_is_status_code_literal("2/0", &is_lit);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_lit);
    rc = client_body_is_status_code_literal("2:0", &is_lit);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_lit);
    rc = client_body_is_status_code_literal("20/", &is_lit);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_lit);
    rc = client_body_is_status_code_literal("20:", &is_lit);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_lit);
    rc = client_body_is_status_code_literal("200", &is_lit);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, is_lit);
  }

  /* Section 2: write_header_param_logic full branches */
  {
    struct OpenAPI_Operation op;
    struct OpenAPI_Parameter params[25];
    memset(&op, 0, sizeof(op));
    memset(params, 0, sizeof(params));

    /* NULL check */
    fp = cdd_test_tmpfile_global();
    ASSERT(fp);
    rc = client_body_write_header_param_logic(fp, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    fclose(fp);

    /* Param 0: JSON non-json content_type */
    params[0].name = C_CDD_STR_LIT("h_non_json");
    params[0].in = OA_PARAM_IN_HEADER;
    params[0].content_type = C_CDD_STR_LIT("text/plain");
    params[0].type = C_CDD_STR_LIT("string");

    /* Param 1: JSON array boolean */
    params[1].name = C_CDD_STR_LIT("h_arr_bool");
    params[1].in = OA_PARAM_IN_HEADER;
    params[1].content_type = C_CDD_STR_LIT("application/json");
    params[1].is_array = 1;
    params[1].items_type = C_CDD_STR_LIT("boolean");

    /* Param 2: JSON array non-primitive, non-object */
    params[2].name = C_CDD_STR_LIT("h_arr_custom");
    params[2].in = OA_PARAM_IN_HEADER;
    params[2].content_type = C_CDD_STR_LIT("application/json");
    params[2].is_array = 1;
    params[2].items_type = C_CDD_STR_LIT("CustomStruct");

    /* Param 3: JSON array items_type NULL, schema.inline_type string */
    params[3].name = C_CDD_STR_LIT("h_arr_inl_str");
    params[3].in = OA_PARAM_IN_HEADER;
    params[3].content_type = C_CDD_STR_LIT("application/json");
    params[3].is_array = 1;
    params[3].items_type = NULL;
    params[3].schema.inline_type = C_CDD_STR_LIT("string");

    /* Param 4: JSON array items_type NULL, schema.inline_type NULL */
    params[4].name = C_CDD_STR_LIT("h_arr_null");
    params[4].in = OA_PARAM_IN_HEADER;
    params[4].content_type = C_CDD_STR_LIT("application/json");
    params[4].is_array = 1;
    params[4].items_type = NULL;

    /* Param 5: JSON scalar boolean */
    params[5].name = C_CDD_STR_LIT("h_sc_bool");
    params[5].in = OA_PARAM_IN_HEADER;
    params[5].content_type = C_CDD_STR_LIT("application/json");
    params[5].type = C_CDD_STR_LIT("boolean");

    /* Param 6: JSON scalar ref_name */
    params[6].name = C_CDD_STR_LIT("h_sc_ref");
    params[6].in = OA_PARAM_IN_HEADER;
    params[6].content_type = C_CDD_STR_LIT("application/json");
    params[6].schema.ref_name = C_CDD_STR_LIT("HeaderModel");

    /* Param 7: JSON scalar object type */
    params[7].name = C_CDD_STR_LIT("h_sc_obj");
    params[7].in = OA_PARAM_IN_HEADER;
    params[7].content_type = C_CDD_STR_LIT("application/json");
    params[7].type = C_CDD_STR_LIT("object");

    /* Param 8: Standard header array with items_type NULL */
    params[8].name = C_CDD_STR_LIT("h_std_arr_null");
    params[8].in = OA_PARAM_IN_HEADER;
    params[8].is_array = 1;
    params[8].items_type = NULL;
    params[8].type = C_CDD_STR_LIT("array");

    /* Param 9: Standard header explode_set 0 */
    params[9].name = C_CDD_STR_LIT("h_std_no_exp");
    params[9].in = OA_PARAM_IN_HEADER;
    params[9].is_array = 1;
    params[9].items_type = C_CDD_STR_LIT("string");
    params[9].explode_set = 0;

    /* Param 10: Standard header custom type */
    params[10].name = C_CDD_STR_LIT("h_std_custom");
    params[10].in = OA_PARAM_IN_HEADER;
    params[10].type = C_CDD_STR_LIT("CustomType");

    /* Param 11: Non-header parameter */
    params[11].name = C_CDD_STR_LIT("q_param");
    params[11].in = OA_PARAM_IN_QUERY;

    /* Param 12: Object header with explode_set 0 */
    params[12].name = C_CDD_STR_LIT("h_obj_no_exp");
    params[12].in = OA_PARAM_IN_HEADER;
    params[12].type = C_CDD_STR_LIT("object");
    params[12].explode_set = 0;

    /* Param 13: Object header with explode_set 1 */
    params[13].name = C_CDD_STR_LIT("h_obj_exp");
    params[13].in = OA_PARAM_IN_HEADER;
    params[13].type = C_CDD_STR_LIT("object");
    params[13].explode_set = 1;
    params[13].explode = 1;

    /* Param 14: Standard header array integer */
    params[14].name = C_CDD_STR_LIT("h_arr_int");
    params[14].in = OA_PARAM_IN_HEADER;
    params[14].is_array = 1;
    params[14].items_type = C_CDD_STR_LIT("integer");

    /* Param 15: Standard header array number */
    params[15].name = C_CDD_STR_LIT("h_arr_num");
    params[15].in = OA_PARAM_IN_HEADER;
    params[15].is_array = 1;
    params[15].items_type = C_CDD_STR_LIT("number");

    /* Param 16: Standard header array boolean */
    params[16].name = C_CDD_STR_LIT("h_arr_bool");
    params[16].in = OA_PARAM_IN_HEADER;
    params[16].is_array = 1;
    params[16].items_type = C_CDD_STR_LIT("boolean");

    /* Param 17: Standard header scalar integer */
    params[17].name = C_CDD_STR_LIT("h_sc_int");
    params[17].in = OA_PARAM_IN_HEADER;
    params[17].type = C_CDD_STR_LIT("integer");

    /* Param 18: Standard header scalar number */
    params[18].name = C_CDD_STR_LIT("h_sc_num");
    params[18].in = OA_PARAM_IN_HEADER;
    params[18].type = C_CDD_STR_LIT("number");

    /* Param 19: JSON scalar number */
    params[19].name = C_CDD_STR_LIT("h_sc_num_json");
    params[19].in = OA_PARAM_IN_HEADER;
    params[19].content_type = C_CDD_STR_LIT("application/json");
    params[19].type = C_CDD_STR_LIT("number");

    /* Param 20: JSON unsupported scalar */
    params[20].name = C_CDD_STR_LIT("h_sc_unsupp_json");
    params[20].in = OA_PARAM_IN_HEADER;
    params[20].content_type = C_CDD_STR_LIT("application/json");
    params[20].type = NULL;
    params[20].schema.inline_type = C_CDD_STR_LIT("unsupported_inline");

    op.parameters = params;
    op.n_parameters = 21;

    test_helper_run_io_hdr(&op);
  }

  /* Section 3: client_body_write_form_urlencoded_body full branches */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation op;
    struct OpenAPI_Encoding encs[15];
    struct OpenAPI_MediaType mt;
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    spec.defined_schemas =
        (struct StructFields *)calloc(4, sizeof(struct StructFields));
    spec.defined_schema_names = (char **)calloc(4, sizeof(char *));
    ASSERT(spec.defined_schemas);
    ASSERT(spec.defined_schema_names);
    memset(&op, 0, sizeof(op));
    memset(encs, 0, sizeof(encs));
    memset(&mt, 0, sizeof(mt));

    /* NULL check */
    fp = cdd_test_tmpfile_global();
    ASSERT(fp);
    rc = client_body_write_form_urlencoded_body(fp, NULL, &spec);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = client_body_write_form_urlencoded_body(fp, &op, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    fclose(fp);

    /* Defined schema: ObjPrim with boolean, integer, number, string */
    struct_fields_init(&spec.defined_schemas[0]);
    struct_fields_add(&spec.defined_schemas[0], "f_bool", "boolean", NULL, NULL,
                      NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_int", "integer", NULL, NULL,
                      NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_num", "number", NULL, NULL,
                      NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_str", "string", NULL, NULL,
                      NULL);
    c_cdd_strdup("ObjPrim", &spec.defined_schema_names[0]);

    /* Defined schema: MainReq with object and array fields */
    struct_fields_init(&spec.defined_schemas[1]);
    struct_fields_add(&spec.defined_schemas[1], "field_form_no_exp", "object",
                      "ObjPrim", NULL, NULL);
    struct_fields_add(&spec.defined_schemas[1], "field_deep_obj", "object",
                      "ObjPrim", NULL, NULL);
    struct_fields_add(&spec.defined_schemas[1], "field_deep_res", "object",
                      "ObjPrim", NULL, NULL);
    struct_fields_add(&spec.defined_schemas[1], "field_deep_no_exp", "object",
                      "ObjPrim", NULL, NULL);
    struct_fields_add(&spec.defined_schemas[1], "field_arr_res_exp", "array",
                      "ObjPrim", NULL, NULL);
    struct_fields_add(&spec.defined_schemas[1], "field_arr_res_no_exp", "array",
                      "ObjPrim", NULL, NULL);
    struct_fields_add(&spec.defined_schemas[1], "field_arr_prim_exp", "array",
                      "string", NULL, NULL);
    struct_fields_add(&spec.defined_schemas[1], "field_arr_unsupp", "array",
                      "enum", NULL, NULL);
    struct_fields_add(&spec.defined_schemas[1], "field_res_no_style", "object",
                      "ObjPrim", NULL, NULL);
    struct_fields_add(&spec.defined_schemas[1], "field_exp_no_style", "object",
                      "ObjPrim", NULL, NULL);
    struct_fields_add(&spec.defined_schemas[1], "field_bad_schema", "object",
                      "NonExistentSchema", NULL, NULL);
    struct_fields_add(&spec.defined_schemas[1], "field_empty_model", "object",
                      "EmptyModel", NULL, NULL);
    c_cdd_strdup("MainReq", &spec.defined_schema_names[1]);

    struct_fields_init(&spec.defined_schemas[2]);
    c_cdd_strdup("EmptyModel", &spec.defined_schema_names[2]);
    spec.n_defined_schemas = 3;

    /* Encodings */
    encs[0].name = C_CDD_STR_LIT("field_form_no_exp");
    encs[0].style_set = 1;
    encs[0].style = OA_STYLE_FORM;
    encs[0].explode_set = 1;
    encs[0].explode = 0;

    encs[1].name = C_CDD_STR_LIT("field_deep_obj");
    encs[1].style_set = 1;
    encs[1].style = OA_STYLE_DEEP_OBJECT;
    encs[1].explode_set = 1;
    encs[1].explode = 1;

    encs[2].name = C_CDD_STR_LIT("field_deep_res");
    encs[2].style_set = 1;
    encs[2].style = OA_STYLE_DEEP_OBJECT;
    encs[2].explode_set = 1;
    encs[2].explode = 1;
    encs[2].allow_reserved_set = 1;
    encs[2].allow_reserved = 1;

    encs[3].name = C_CDD_STR_LIT("field_arr_res_exp");
    encs[3].style_set = 1;
    encs[3].style = OA_STYLE_FORM;
    encs[3].explode_set = 1;
    encs[3].explode = 1;
    encs[3].allow_reserved_set = 1;
    encs[3].allow_reserved = 1;

    encs[4].name = C_CDD_STR_LIT("field_arr_res_no_exp");
    encs[4].style_set = 1;
    encs[4].style = OA_STYLE_FORM;
    encs[4].explode_set = 1;
    encs[4].explode = 0;
    encs[4].allow_reserved_set = 1;
    encs[4].allow_reserved = 1;

    encs[5].name = C_CDD_STR_LIT("field_arr_prim_exp");
    encs[5].style_set = 1;
    encs[5].style = OA_STYLE_FORM;
    encs[5].explode_set = 1;
    encs[5].explode = 1;

    encs[6].name = C_CDD_STR_LIT("field_res_no_style");

    encs[7].name = C_CDD_STR_LIT("field_exp_no_style");
    encs[7].explode_set = 1;
    encs[7].explode = 1;

    encs[8].name = C_CDD_STR_LIT("field_bad_schema");
    encs[8].style_set = 1;
    encs[8].style = OA_STYLE_FORM;

    encs[9].name = C_CDD_STR_LIT("field_deep_no_exp");
    encs[9].style_set = 1;
    encs[9].style = OA_STYLE_DEEP_OBJECT;
    encs[9].explode_set = 1;
    encs[9].explode = 0;

    encs[10].name = C_CDD_STR_LIT("field_empty_model");
    encs[10].style_set = 1;
    encs[10].style = OA_STYLE_FORM;

    encs[11].name = C_CDD_STR_LIT("field_arr_unsupp");
    encs[11].style_set = 1;
    encs[11].style = OA_STYLE_FORM;
    encs[11].explode_set = 1;
    encs[11].explode = 1;

    mt.name = C_CDD_STR_LIT("application/x-www-form-urlencoded");
    mt.encoding = encs;
    mt.n_encoding = 12;

    op.operation_id = C_CDD_STR_LIT("testFormFull");
    op.verb = OA_VERB_POST;
    op.method = C_CDD_STR_LIT("post");
    op.req_body.ref_name = C_CDD_STR_LIT("MainReq");
    op.req_body.content_type =
        C_CDD_STR_LIT("application/x-www-form-urlencoded");
    op.req_body_media_types = &mt;
    op.n_req_body_media_types = 1;

    test_helper_run_io_form(&op, &spec);
    openapi_spec_free(&spec);
  }

  /* Section 4: client_body_write_cookie_param_logic full branches */
  {
    struct OpenAPI_Operation op;
    struct OpenAPI_Parameter params[15];
    memset(&op, 0, sizeof(op));
    memset(params, 0, sizeof(params));

    /* NULL check */
    fp = cdd_test_tmpfile_global();
    ASSERT(fp);
    rc = client_body_write_cookie_param_logic(fp, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    fclose(fp);

    /* Param 0: style COOKIE */
    params[0].name = C_CDD_STR_LIT("ck_cookie");
    params[0].in = OA_PARAM_IN_COOKIE;
    params[0].type = C_CDD_STR_LIT("string");
    params[0].style = OA_STYLE_COOKIE;

    /* Param 1: style MATRIX */
    params[1].name = C_CDD_STR_LIT("ck_matrix");
    params[1].in = OA_PARAM_IN_COOKIE;
    params[1].type = C_CDD_STR_LIT("string");
    params[1].style = OA_STYLE_MATRIX;

    /* Param 2: object explode 1 */
    params[2].name = C_CDD_STR_LIT("ck_obj_exp");
    params[2].in = OA_PARAM_IN_COOKIE;
    params[2].type = C_CDD_STR_LIT("object");
    params[2].explode_set = 1;
    params[2].explode = 1;

    /* Param 3: object explode 0 */
    params[3].name = C_CDD_STR_LIT("ck_obj_no_exp");
    params[3].in = OA_PARAM_IN_COOKIE;
    params[3].type = C_CDD_STR_LIT("object");
    params[3].explode_set = 1;
    params[3].explode = 0;

    /* Param 4: boolean */
    params[4].name = C_CDD_STR_LIT("ck_bool");
    params[4].in = OA_PARAM_IN_COOKIE;
    params[4].type = C_CDD_STR_LIT("boolean");

    /* Param 5: non-cookie param */
    params[5].name = C_CDD_STR_LIT("hdr_param");
    params[5].in = OA_PARAM_IN_HEADER;

    /* Param 6: style UNKNOWN */
    params[6].name = C_CDD_STR_LIT("ck_unk");
    params[6].in = OA_PARAM_IN_COOKIE;
    params[6].type = C_CDD_STR_LIT("string");
    params[6].style = OA_STYLE_UNKNOWN;

    /* Param 7: object array */
    params[7].name = C_CDD_STR_LIT("ck_obj_arr");
    params[7].in = OA_PARAM_IN_COOKIE;
    params[7].type = C_CDD_STR_LIT("object");
    params[7].is_array = 1;

    /* Param 8: custom type */
    params[8].name = C_CDD_STR_LIT("ck_custom");
    params[8].in = OA_PARAM_IN_COOKIE;
    params[8].type = C_CDD_STR_LIT("CustomType");

    /* Param 9: cookie array integer */
    params[9].name = C_CDD_STR_LIT("ck_arr_int");
    params[9].in = OA_PARAM_IN_COOKIE;
    params[9].is_array = 1;
    params[9].items_type = C_CDD_STR_LIT("integer");

    /* Param 10: cookie array number */
    params[10].name = C_CDD_STR_LIT("ck_arr_num");
    params[10].in = OA_PARAM_IN_COOKIE;
    params[10].is_array = 1;
    params[10].items_type = C_CDD_STR_LIT("number");

    /* Param 11: cookie array boolean */
    params[11].name = C_CDD_STR_LIT("ck_arr_bool");
    params[11].in = OA_PARAM_IN_COOKIE;
    params[11].is_array = 1;
    params[11].items_type = C_CDD_STR_LIT("boolean");

    /* Param 12: cookie array string */
    params[12].name = C_CDD_STR_LIT("ck_arr_str");
    params[12].in = OA_PARAM_IN_COOKIE;
    params[12].is_array = 1;
    params[12].items_type = C_CDD_STR_LIT("string");
    params[12].style = OA_STYLE_COOKIE;
    params[12].explode_set = 1;
    params[12].explode = 1;

    /* Param 13: cookie array string no explode */
    params[13].name = C_CDD_STR_LIT("ck_arr_str_no_exp");
    params[13].in = OA_PARAM_IN_COOKIE;
    params[13].is_array = 1;
    params[13].items_type = C_CDD_STR_LIT("string");
    params[13].style = OA_STYLE_COOKIE;
    params[13].explode_set = 1;
    params[13].explode = 0;

    op.parameters = params;
    op.n_parameters = 14;

    test_helper_run_io_cookie(&op);
  }

  /* Section 5: client_body_write_multipart_part_headers full branches */
  {
    struct OpenAPI_Encoding enc;
    struct OpenAPI_Header hdrs[4];
    memset(&enc, 0, sizeof(enc));
    memset(hdrs, 0, sizeof(hdrs));

    /* NULL / empty enc checks */
    fp = cdd_test_tmpfile_global();
    ASSERT(fp);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              client_body_write_multipart_part_headers(NULL, &enc));
    ASSERT_EQ(CDD_C_SUCCESS,
              client_body_write_multipart_part_headers(fp, NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              client_body_write_multipart_part_headers(fp, &enc));
    enc.name = C_CDD_STR_LIT("part1");
    ASSERT_EQ(CDD_C_SUCCESS,
              client_body_write_multipart_part_headers(fp, &enc));
    enc.headers = hdrs;
    enc.n_headers = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              client_body_write_multipart_part_headers(fp, &enc));
    enc.n_headers = 1;
    enc.name = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              client_body_write_multipart_part_headers(fp, &enc));
    enc.name = C_CDD_STR_LIT("part1");
    fclose(fp);

    /* Headers with full branch coverage */
    hdrs[0].name = C_CDD_STR_LIT("X-Hdr-Arr-Bool");
    hdrs[0].is_array = 0;
    hdrs[0].type = C_CDD_STR_LIT("array");
    hdrs[0].items_type = C_CDD_STR_LIT("boolean");

    hdrs[1].name = C_CDD_STR_LIT("X-Hdr-Obj-Exp");
    hdrs[1].type = C_CDD_STR_LIT("object");
    hdrs[1].explode_set = 1;
    hdrs[1].explode = 1;

    hdrs[2].name = C_CDD_STR_LIT("X-Hdr-Int");
    hdrs[2].type = C_CDD_STR_LIT("integer");

    hdrs[3].name = C_CDD_STR_LIT("X-Hdr-Num");
    hdrs[3].type = C_CDD_STR_LIT("number");

    enc.headers = hdrs;
    enc.n_headers = 4;

    test_helper_run_io_part_headers(&enc);
  }

  /* Section 6: client_body_write_multipart_body empty content_type */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation op;
    struct OpenAPI_MediaType mt;
    struct OpenAPI_Encoding encs[6];
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    spec.defined_schemas =
        (struct StructFields *)calloc(4, sizeof(struct StructFields));
    spec.defined_schema_names = (char **)calloc(4, sizeof(char *));
    ASSERT(spec.defined_schemas);
    ASSERT(spec.defined_schema_names);
    memset(&op, 0, sizeof(op));
    memset(&mt, 0, sizeof(mt));
    memset(encs, 0, sizeof(encs));

    /* NULL checks */
    fp = cdd_test_tmpfile_global();
    ASSERT(fp);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              client_body_write_multipart_body(NULL, &op, &spec));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              client_body_write_multipart_body(fp, NULL, &spec));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              client_body_write_multipart_body(fp, &op, NULL));
    fclose(fp);

    /* Schema with all types */
    struct_fields_init(&spec.defined_schemas[0]);
    struct_fields_add(&spec.defined_schemas[0], "f_arr", "array", "string",
                      NULL, NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_arr_unsupp", "array", "enum",
                      NULL, NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_str", "string", NULL, NULL,
                      NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_int", "integer", NULL, NULL,
                      NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_num", "number", NULL, NULL,
                      NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_bool", "boolean", NULL, NULL,
                      NULL);
    struct_fields_add(&spec.defined_schemas[0], "f_obj", "object", "SubM", NULL,
                      NULL);
    c_cdd_strdup("MultiEmptyCT", &spec.defined_schema_names[0]);

    struct_fields_init(&spec.defined_schemas[1]);
    struct_fields_add(&spec.defined_schemas[1], "id", "integer", NULL, NULL,
                      NULL);
    c_cdd_strdup("SubM", &spec.defined_schema_names[1]);
    spec.n_defined_schemas = 2;

    /* Encodings with empty string content_type */
    encs[0].name = C_CDD_STR_LIT("f_arr");
    encs[0].content_type = C_CDD_STR_LIT("");
    encs[1].name = C_CDD_STR_LIT("f_str");
    encs[1].content_type = C_CDD_STR_LIT("");
    encs[2].name = C_CDD_STR_LIT("f_int");
    encs[2].content_type = C_CDD_STR_LIT("");
    encs[3].name = C_CDD_STR_LIT("f_num");
    encs[3].content_type = C_CDD_STR_LIT("");
    encs[4].name = C_CDD_STR_LIT("f_bool");
    encs[4].content_type = C_CDD_STR_LIT("");
    encs[5].name = C_CDD_STR_LIT("f_obj");
    encs[5].content_type = C_CDD_STR_LIT("");

    mt.name = C_CDD_STR_LIT("multipart/form-data");
    mt.encoding = encs;
    mt.n_encoding = 6;

    op.operation_id = C_CDD_STR_LIT("testMultiEmptyCT");
    op.verb = OA_VERB_POST;
    op.method = C_CDD_STR_LIT("post");
    op.req_body.ref_name = C_CDD_STR_LIT("MultiEmptyCT");
    op.req_body.content_type = C_CDD_STR_LIT("multipart/form-data");
    op.req_body_media_types = &mt;
    op.n_req_body_media_types = 1;

    test_helper_run_io_multipart(&op, &spec);
    openapi_spec_free(&spec);
  }

  /* Section 7: codegen_client_write_body all remaining branches */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation op;
    struct OpenAPI_Response responses[3];
    struct OpenAPI_SecurityScheme sch_q;
    struct OpenAPI_SecurityScheme sch_c;
    struct OpenAPI_Parameter params[3];

    /* 7a: spec == NULL */
    memset(&op, 0, sizeof(op));
    op.operation_id = C_CDD_STR_LIT("testNoSpec");
    op.verb = OA_VERB_GET;
    op.method = C_CDD_STR_LIT("get");
    test_helper_run_io_client_body(&op, NULL, "/no_spec");

    /* 7b: security_query without query params, security_cookie without cookie
     * params */
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&sch_q, 0, sizeof(sch_q));
    sch_q.type = OA_SEC_APIKEY;
    sch_q.in = OA_SEC_IN_QUERY;
    c_cdd_strdup("sec_query", &sch_q.name);
    c_cdd_strdup("api_key", &sch_q.key_name);

    memset(&sch_c, 0, sizeof(sch_c));
    sch_c.type = OA_SEC_APIKEY;
    sch_c.in = OA_SEC_IN_COOKIE;
    c_cdd_strdup("sec_cookie", &sch_c.name);
    c_cdd_strdup("session_id", &sch_c.key_name);

    spec.security_schemes =
        (struct OpenAPI_SecurityScheme *)calloc(2, sizeof(sch_q));
    ASSERT(spec.security_schemes);
    spec.security_schemes[0] = sch_q;
    spec.security_schemes[1] = sch_c;
    spec.n_security_schemes = 2;
    spec.security_set = 0;
    op.security_set = 0;

    /* Also path parameter with name NULL, and regular parameter with name NULL
     */
    memset(params, 0, sizeof(params));
    params[0].in = OA_PARAM_IN_PATH;
    params[0].name = NULL;
    params[1].in = OA_PARAM_IN_UNKNOWN;
    params[1].name = NULL;
    op.parameters = params;
    op.n_parameters = 2;

    /* req_body with ref_name but content_type NULL, is_array 0 */
    op.req_body.ref_name = C_CDD_STR_LIT("ReqModel");
    op.req_body.content_type = NULL;
    op.req_body.is_array = 0;
    test_helper_run_io_client_body(&op, &spec, "/sec_and_null_names");

    /* Also test req_body.ref_name with is_array 1 for line 4285 and req_is_json
     * for line 4418 */
    op.req_body.is_array = 1;
    op.req_body.content_type = C_CDD_STR_LIT("application/json");
    test_helper_run_io_client_body(&op, &spec, "/ref_arr_body");

    /* Responses with is_array 0 */
    memset(responses, 0, sizeof(responses));
    responses[0].code = C_CDD_STR_LIT("200");
    responses[0].schema.ref_name = C_CDD_STR_LIT("ResModel");
    responses[0].schema.is_array = 0;
    op.responses = responses;
    op.n_responses = 1;

    openapi_spec_free(&spec);

    /* 7c: Inline req_body JSON array & scalar types (string, integer, number,
     * boolean, custom) */
    {
      const char *inl_types[5];
      size_t t;
      inl_types[0] = "string";
      inl_types[1] = "integer";
      inl_types[2] = "number";
      inl_types[3] = "boolean";
      inl_types[4] = "custom_unsupported";

      for (t = 0; t < 5; ++t) {
        ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
        memset(&op, 0, sizeof(op));
        op.operation_id = C_CDD_STR_LIT("testInlReq");
        op.verb = OA_VERB_POST;
        op.method = C_CDD_STR_LIT("post");
        op.req_body.content_type = C_CDD_STR_LIT("application/json");
        op.req_body.inline_type = C_CDD_STR_LIT(inl_types[t]);
        op.req_body.is_array = 1;
        test_helper_run_io_client_body(&op, &spec, "/inl_arr");

        op.req_body.is_array = 0;
        test_helper_run_io_client_body(&op, &spec, "/inl_sc");
        openapi_spec_free(&spec);
      }
    }

    /* 7d: op.method empty string and custom unsupported method */
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&op, 0, sizeof(op));
    op.operation_id = C_CDD_STR_LIT("testEmptyMethod");
    op.verb = OA_VERB_UNKNOWN;
    op.is_additional = 1;
    op.method = C_CDD_STR_LIT("");
    test_helper_run_io_client_body(&op, &spec, "/empty_method");

    op.method = C_CDD_STR_LIT("CUSTOM");
    test_helper_run_io_client_body(&op, &spec, "/custom_method");
    openapi_spec_free(&spec);

    /* 7e: Default response mismatch checks and def_has_inline */
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&op, 0, sizeof(op));
    memset(responses, 0, sizeof(responses));
    op.operation_id = C_CDD_STR_LIT("testDefMismatch");
    op.verb = OA_VERB_GET;
    op.method = C_CDD_STR_LIT("get");

    /* Success 200 response */
    responses[0].code = C_CDD_STR_LIT("200");
    responses[0].schema.ref_name = C_CDD_STR_LIT("ModelA");
    responses[0].schema.inline_type = C_CDD_STR_LIT("string");
    responses[0].schema.is_array = 0;

    /* Default response with different ref_name and different inline_type */
    responses[1].code = C_CDD_STR_LIT("default");
    responses[1].schema.ref_name = C_CDD_STR_LIT("ModelB");
    responses[1].schema.inline_type = C_CDD_STR_LIT("integer");
    responses[1].schema.is_array = 1;
    op.responses = responses;
    op.n_responses = 2;
    test_helper_run_io_client_body(&op, &spec, "/def_mismatch");

    /* Response schema has is_array 1 but ref_name and inline_type NULL (L4324)
     */
    responses[0].code = C_CDD_STR_LIT("200");
    responses[0].schema.ref_name = NULL;
    responses[0].schema.inline_type = NULL;
    responses[0].schema.is_array = 1;
    responses[1].code = C_CDD_STR_LIT("default");
    responses[1].schema.ref_name = NULL;
    responses[1].schema.inline_type = NULL;
    responses[1].schema.is_array = 0;
    test_helper_run_io_client_body(&op, &spec, "/null_ref_arr_resps");

    /* Default response schema has is_array 1 with no success response (L4340)
     */
    responses[0].code = C_CDD_STR_LIT("404");
    responses[0].schema.is_array = 0;
    responses[1].code = C_CDD_STR_LIT("default");
    responses[1].schema.is_array = 1;
    test_helper_run_io_client_body(&op, &spec, "/null_def_arr_resps");

    /* Two 2xx responses with inline types (L4651) */
    responses[0].code = C_CDD_STR_LIT("200");
    responses[0].schema.inline_type = C_CDD_STR_LIT("string");
    responses[0].schema.is_array = 0;
    responses[1].code = C_CDD_STR_LIT("201");
    responses[1].schema.inline_type = C_CDD_STR_LIT("integer");
    responses[1].schema.is_array = 0;
    test_helper_run_io_client_body(&op, &spec, "/two_2xx_inlines");

    /* Default response matching inline type but differing is_array (L4844) */
    responses[0].code = C_CDD_STR_LIT("200");
    responses[0].schema.inline_type = C_CDD_STR_LIT("integer");
    responses[0].schema.is_array = 0;
    responses[1].code = C_CDD_STR_LIT("default");
    responses[1].schema.inline_type = C_CDD_STR_LIT("integer");
    responses[1].schema.is_array = 1;
    test_helper_run_io_client_body(&op, &spec, "/def_arr_mismatch");

    /* Default response matches success with def_has_inline when
     * default_is_success */
    responses[0].code = C_CDD_STR_LIT("400");
    responses[0].schema.ref_name = NULL;
    responses[0].schema.inline_type = NULL;
    responses[1].code = C_CDD_STR_LIT("default");
    responses[1].content_type = C_CDD_STR_LIT("application/json");
    responses[1].schema.ref_name = NULL;
    responses[1].schema.inline_type = C_CDD_STR_LIT("string");
    responses[1].schema.is_array = 0;
    test_helper_run_io_client_body(&op, &spec, "/def_inline_success");

    openapi_spec_free(&spec);
  }
  /* 7f: Form body in codegen_client_write_body */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation f_op;
    struct OpenAPI_MediaType f_mt;
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    spec.defined_schemas =
        (struct StructFields *)calloc(1, sizeof(struct StructFields));
    spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
    struct_fields_init(&spec.defined_schemas[0]);
    struct_fields_add(&spec.defined_schemas[0], "title", "string", NULL, NULL,
                      NULL);
    c_cdd_strdup("FormModel", &spec.defined_schema_names[0]);
    spec.n_defined_schemas = 1;

    memset(&f_op, 0, sizeof(f_op));
    memset(&f_mt, 0, sizeof(f_mt));
    f_op.operation_id = C_CDD_STR_LIT("testFormBody");
    f_op.verb = OA_VERB_POST;
    f_op.method = C_CDD_STR_LIT("post");
    f_op.req_body.ref_name = C_CDD_STR_LIT("FormModel");
    f_op.req_body.content_type =
        C_CDD_STR_LIT("application/x-www-form-urlencoded");
    f_mt.name = C_CDD_STR_LIT("application/x-www-form-urlencoded");
    f_op.req_body_media_types = &f_mt;
    f_op.n_req_body_media_types = 1;
    test_helper_run_io_client_body(&f_op, &spec, "/form_body");
    openapi_spec_free(&spec);
  }

  /* 7g: Text plain body in codegen_client_write_body */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation txt_op;
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&txt_op, 0, sizeof(txt_op));
    txt_op.operation_id = C_CDD_STR_LIT("testTxtBody");
    txt_op.verb = OA_VERB_POST;
    txt_op.method = C_CDD_STR_LIT("post");
    txt_op.req_body.content_type = C_CDD_STR_LIT("text/plain");
    test_helper_run_io_client_body(&txt_op, &spec, "/txt_body");
    openapi_spec_free(&spec);
  }

  /* 7h: No success responses (only 404), hitting line 4351 success_schema ==
   * NULL */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation no_succ_op;
    struct OpenAPI_Response no_succ_res;
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&no_succ_op, 0, sizeof(no_succ_op));
    memset(&no_succ_res, 0, sizeof(no_succ_res));
    no_succ_op.operation_id = C_CDD_STR_LIT("testNoSuccess");
    no_succ_op.verb = OA_VERB_GET;
    no_succ_op.method = C_CDD_STR_LIT("get");
    no_succ_res.code = C_CDD_STR_LIT("404");
    no_succ_op.responses = &no_succ_res;
    no_succ_op.n_responses = 1;
    test_helper_run_io_client_body(&no_succ_op, &spec, "/no_success");
    openapi_spec_free(&spec);
  }

  /* 7i: 2XX has inline type but 200 does not, hitting line 4659 */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation r2_op;
    struct OpenAPI_Response r2_res[2];
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&r2_op, 0, sizeof(r2_op));
    memset(r2_res, 0, sizeof(r2_res));
    r2_op.operation_id = C_CDD_STR_LIT("testR2Inline");
    r2_op.verb = OA_VERB_GET;
    r2_op.method = C_CDD_STR_LIT("get");
    r2_res[0].code = C_CDD_STR_LIT("200");
    r2_res[0].schema.ref_name = C_CDD_STR_LIT("ModelA");
    r2_res[1].code = C_CDD_STR_LIT("2XX");
    r2_res[1].schema.inline_type = C_CDD_STR_LIT("integer");
    r2_op.responses = r2_res;
    r2_op.n_responses = 2;
    test_helper_run_io_client_body(&r2_op, &spec, "/r2_inline");
    openapi_spec_free(&spec);
  }

  /* 7j: Default response has ref_name when success has matching ref_name,
   * hitting lines 4869-4882 */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation def_ref_op;
    struct OpenAPI_Response def_ref_res[2];
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&def_ref_op, 0, sizeof(def_ref_op));
    memset(def_ref_res, 0, sizeof(def_ref_res));
    def_ref_op.operation_id = C_CDD_STR_LIT("testDefRef");
    def_ref_op.verb = OA_VERB_GET;
    def_ref_op.method = C_CDD_STR_LIT("get");
    def_ref_res[0].code = C_CDD_STR_LIT("200");
    def_ref_res[0].schema.ref_name = C_CDD_STR_LIT("CommonModel");
    def_ref_res[1].code = C_CDD_STR_LIT("default");
    def_ref_res[1].schema.ref_name = C_CDD_STR_LIT("CommonModel");
    def_ref_res[1].schema.is_array = 0;
    def_ref_op.responses = def_ref_res;
    def_ref_op.n_responses = 2;
    test_helper_run_io_client_body(&def_ref_op, &spec, "/def_ref");

    /* Also default response array with ref_name for line 4863 */
    def_ref_res[1].schema.is_array = 1;
    test_helper_run_io_client_body(&def_ref_op, &spec, "/def_ref_arr");
    openapi_spec_free(&spec);
  }
  /* Binary request body (lines 4550-4552) */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation bin_op;
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&bin_op, 0, sizeof(bin_op));
    bin_op.operation_id = C_CDD_STR_LIT("testBinBody");
    bin_op.verb = OA_VERB_POST;
    bin_op.method = C_CDD_STR_LIT("post");
    bin_op.req_body.content_type = C_CDD_STR_LIT("application/octet-stream");
    test_helper_run_io_client_body(&bin_op, &spec, "/bin_body");
    openapi_spec_free(&spec);
  }

  /* Range 2XX empty schema (line 4786) and 2XX text/plain (lines 4778-4782) */
  {
    struct OpenAPI_Spec spec;
    struct OpenAPI_Operation r_empty_op;
    struct OpenAPI_Response r_resps[2];
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
    memset(&r_empty_op, 0, sizeof(r_empty_op));
    memset(r_resps, 0, sizeof(r_resps));
    r_empty_op.operation_id = C_CDD_STR_LIT("testRangeEmpty");
    r_empty_op.verb = OA_VERB_GET;
    r_empty_op.method = C_CDD_STR_LIT("get");
    r_resps[0].code = C_CDD_STR_LIT("2XX");
    r_empty_op.responses = r_resps;
    r_empty_op.n_responses = 1;
    test_helper_run_io_client_body(&r_empty_op, &spec, "/r_empty");

    r_resps[0].content_type = C_CDD_STR_LIT("application/octet-stream");
    test_helper_run_io_client_body(&r_empty_op, &spec, "/r_bin");

    /* 2XX with ref_name for lines 4778-4782 */
    r_resps[0].content_type = NULL;
    r_resps[0].schema.ref_name = C_CDD_STR_LIT("CommonModel");
    test_helper_run_io_client_body(&r_empty_op, &spec, "/r_ref");
    openapi_spec_free(&spec);
  }

  PASS();
}

SUITE(client_body_internals_suite) {
  RUN_TEST(test_client_body_verb_and_method_helpers);
  RUN_TEST(test_client_body_find_helpers);
  RUN_TEST(test_client_body_type_predicates);
  RUN_TEST(test_client_body_media_type_helpers);
  RUN_TEST(test_client_body_fail_is_primitive_type_branches);
  RUN_TEST(test_client_body_writer_sub_emitters);
  RUN_TEST(test_client_body_all_operation_patterns);
  RUN_TEST(test_client_body_default_responses_and_inlines);
  RUN_TEST(test_client_body_direct_part_headers_and_form_edge_cases);
  RUN_TEST(test_client_body_inline_req_body_json_types);
  RUN_TEST(test_client_body_querystring_and_security_query);
  RUN_TEST(test_client_body_systematic_io_failures);
  RUN_TEST(test_client_body_targeted_remaining_branches);
  RUN_TEST(test_client_body_all_remaining_sub_writer_io_failures);
  RUN_TEST(test_client_body_exhaustive_100_percent_coverage);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_INTERNALS_H */
