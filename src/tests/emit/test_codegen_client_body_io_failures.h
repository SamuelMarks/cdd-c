/**
 * @file test_codegen_client_body_io_failures.h
 * @brief Unit tests for client body IO failures.
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_IO_FAILURES_H
#define TEST_CODEGEN_CLIENT_BODY_IO_FAILURES_H

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
        g_io_calls = 0;
        g_fail_io_after = io_fail;
        client_body_write_inline_json_parse(fp, &sch_ref);
        g_fail_io_after = -1;
        fclose(fp);
      }
      sch_ref.is_array = 1;
      for (io_fail = 0; io_fail < 50; ++io_fail) {
        fp = cdd_test_tmpfile_global();
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
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_joined_form_array(fp, "tags", "n_tags", "string", ',',
                                          "url_encode", 1, 0);
      g_fail_io_after = -1;
      fclose(fp);
    }
    for (io_fail = 0; io_fail < 70; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_joined_form_array(fp, "tags", "n_tags", "string", ',',
                                          NULL, 0, 0);
      g_fail_io_after = -1;
      fclose(fp);
    }
    for (io_fail = 0; io_fail < 70; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_joined_form_array(fp, "items", "n_items", "Item", '&',
                                          NULL, 0, 1);
      g_fail_io_after = -1;
      fclose(fp);
    }
    for (io_fail = 0; io_fail < 70; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_joined_form_array(fp, "ints", "n_ints", "integer", '|',
                                          "url_encode", 0, 0);
      g_fail_io_after = -1;
      fclose(fp);
    }
    for (io_fail = 0; io_fail < 70; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_joined_form_array(fp, "nums", "n_nums", "number", '|',
                                          "url_encode", 0, 0);
      g_fail_io_after = -1;
      fclose(fp);
    }
    for (io_fail = 0; io_fail < 70; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_joined_form_array(fp, "bools", "n_bools", "boolean",
                                          '|', "url_encode", 0, 0);
      g_fail_io_after = -1;
      fclose(fp);
    }
    for (io_fail = 0; io_fail < 70; ++io_fail) {
      fp = cdd_test_tmpfile_global();
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_IO_FAILURES_H */
