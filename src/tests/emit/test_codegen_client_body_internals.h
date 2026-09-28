/**
 * @file test_codegen_client_body_internals.h
 * @brief Comprehensive tests for client_body helper functions and edge cases.
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

#ifndef C_CDD_STR_LIT
#define C_CDD_STR_LIT(s) ((char *)(size_t)(size_t)(s))
#endif

extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_cdd_fail_is_primitive_type;

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

#include "emit/test_codegen_client_body_coverage.h"
#include "emit/test_codegen_client_body_exhaustive.h"
#include "emit/test_codegen_client_body_helpers.h"
#include "emit/test_codegen_client_body_io_failures.h"
#include "emit/test_codegen_client_body_operations.h"
#include "emit/test_codegen_client_body_params.h"

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
  RUN_TEST(test_client_body_exhaustive_remaining);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_INTERNALS_H */
