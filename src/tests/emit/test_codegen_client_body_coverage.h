/**
 * @file test_codegen_client_body_coverage.h
 * @brief Exhaustive coverage tests for client body (part 1).
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_COVERAGE_H
#define TEST_CODEGEN_CLIENT_BODY_COVERAGE_H

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

#include "emit/test_codegen_client_body_helpers.h"

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

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_COVERAGE_H */
