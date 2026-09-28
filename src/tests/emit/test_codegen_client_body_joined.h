/**
 * @file test_codegen_client_body_joined.h
 * @brief Unit tests for client body joined form arrays and text plain.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_JOINED_H
#define TEST_CODEGEN_CLIENT_BODY_JOINED_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_body_common.h"
/* clang-format on */

TEST test_client_body_form_object_style_form_explode(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = (char *)(size_t)(size_t) "post";
  op.verb = OA_VERB_POST;
  op.operation_id = (char *)(size_t)(size_t) "testFormObjStyle";

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  c_cdd_strdup("MockSchemaFormObj", &spec.defined_schema_names[0]);

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "obj_prop");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "obj_prop");
#else
  strcpy(spec.defined_schemas[0].fields[0].name, "obj_prop");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "object");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "object");
#else
  strcpy(spec.defined_schemas[0].fields[0].type, "object");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].ref,
           sizeof(spec.defined_schemas[0].fields[0].ref), "MockSchemaFormObj");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].ref,
           sizeof(spec.defined_schemas[0].fields[0].ref), "MockSchemaFormObj");
#else
  strcpy(spec.defined_schemas[0].fields[0].ref, "MockSchemaFormObj");
#endif
#endif /* self-ref for test */

  op.req_body.ref_name = (char *)(size_t)(size_t) "MockSchemaFormObj";

  op.req_body.content_type =
      (char *)(size_t)(size_t) "application/x-www-form-urlencoded";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name =
      (char *)(size_t)(size_t) "application/x-www-form-urlencoded";

  memset(&enc, 0, sizeof(enc));
  enc.name = (char *)(size_t)(size_t) "obj_prop";
  enc.style_set = 1;
  enc.style = OA_STYLE_FORM;
  enc.explode_set = 1;
  enc.explode = 1;

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(spec.defined_schemas[0].fields);
  free(spec.defined_schemas);
  if (spec.defined_schema_names && spec.defined_schema_names[0])
    free(spec.defined_schema_names[0]);
  free(spec.defined_schema_names);
  free(op.req_body_media_types);
  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_cookie_object_style_form_explode(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = (char *)(size_t)(size_t) "get";
  op.verb = OA_VERB_GET;
  op.operation_id = (char *)(size_t)(size_t) "testCookieObjStyle";

  op.n_parameters = 1;
  op.parameters = calloc(1, sizeof(*op.parameters));
  op.parameters[0].name = (char *)(size_t)(size_t) "cookie_obj";
  op.parameters[0].in = OA_PARAM_IN_COOKIE;
  op.parameters[0].type = (char *)(size_t)(size_t) "object";
  op.parameters[0].style = OA_STYLE_FORM;
  op.parameters[0].explode_set = 1;
  op.parameters[0].explode = 1;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(op.parameters);
  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_response_is_textual_string_indirect(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = (char *)(size_t)(size_t) "get";
  op.verb = OA_VERB_GET;
  op.operation_id =
      (char *)(size_t)(size_t) "testResponseIsTextualStringIndirect";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = (char *)(size_t)(size_t) "200";

  /* Set up content type directly on the response to trigger */
  /* `response_is_textual_string` */
  resp.content_type = (char *)(size_t)(size_t) "text/plain";
  resp.schema.inline_type = (char *)(size_t)(size_t) "string";
  resp.schema.is_array = 0;

  /* Also we need to make sure the media_types don't override it in the new */
  /* parser logic, or maybe it's not even used? Let's just set it. */
  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Also hit schema_has_inline missing branches */
  resp.content_type = (char *)(size_t)(size_t) "text/plain";
  resp.schema.inline_type = (char *)(size_t)(size_t) "integer"; /* not string */
  resp.schema.is_array = 0;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_response_is_textual_string_success(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = (char *)(size_t)(size_t) "get";
  op.verb = OA_VERB_GET;
  op.operation_id = (char *)(size_t)(size_t) "testResponseTextualSuccess";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = (char *)(size_t)(size_t) "200";
  resp.content_type = (char *)(size_t)(size_t) "text/plain";
  resp.schema.inline_type = (char *)(size_t)(size_t) "string";
  resp.schema.is_array = 0;

  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_text_plain_success_indirect(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = (char *)(size_t)(size_t) "get";
  op.verb = OA_VERB_GET;
  op.operation_id =
      (char *)(size_t)(size_t) "testResponseTextualSuccessIndirect";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = (char *)(size_t)(size_t) "200";
  resp.content_type = (char *)(size_t)(size_t) "text/plain";
  resp.schema.inline_type = (char *)(size_t)(size_t) "string";
  resp.schema.is_array = 0;

  resp.n_content_media_types = 1;
  resp.content_media_types = calloc(1, sizeof(*resp.content_media_types));
  resp.content_media_types[0].name = (char *)(size_t)(size_t) "text/plain";
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "string";
  resp.content_media_types[0].schema.is_array = 0;

  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(resp.content_media_types);
  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_binary_success_indirect_real(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = (char *)(size_t)(size_t) "get";
  op.verb = OA_VERB_GET;
  op.operation_id =
      (char *)(size_t)(size_t) "testResponseBinarySuccessIndirectReal";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = (char *)(size_t)(size_t) "200";
  resp.content_type = (char *)(size_t)(size_t) "image/png"; /* Binary! */

  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_text_plain_success_indirect_real_fixed(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = (char *)(size_t)(size_t) "get";
  op.verb = OA_VERB_GET;
  op.operation_id =
      (char *)(size_t)(size_t) "testResponseTextualSuccessIndirectRealFixed";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = (char *)(size_t)(size_t) "200";
  resp.content_type = (char *)(size_t)(size_t) "text/plain";
  resp.schema.inline_type = (char *)(size_t)(size_t) "string";
  resp.schema.is_array = 0;

  /* Make sure it DOES NOT have a ref_name */
  resp.schema.ref_name = NULL;

  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_text_plain_success_indirect_real_fixed4(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = (char *)(size_t)(size_t) "get";
  op.verb = OA_VERB_GET;
  op.operation_id =
      (char *)(size_t)(size_t) "testResponseTextualSuccessIndirectRealFixed4";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = (char *)(size_t)(size_t) "200";

  /* Set up content type in the media_types map instead! */
  resp.n_content_media_types = 1;
  resp.content_media_types = calloc(1, sizeof(*resp.content_media_types));
  resp.content_media_types[0].name = (char *)(size_t)(size_t) "text/plain";
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "string";
  resp.content_media_types[0].schema.is_array = 0;

  /* Make sure to populate the root content_type so `response_is_textual_string`
   */
  /* works! */
  resp.content_type = (char *)(size_t)(size_t) "text/plain";

  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(resp.content_media_types);
  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_inline_json_parse_types_indirect_string(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = (char *)(size_t)(size_t) "get";
  op.verb = OA_VERB_GET;
  op.operation_id = (char *)(size_t)(size_t) "testInlineParseTypesString";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = (char *)(size_t)(size_t) "200";

  resp.n_content_media_types = 1;
  resp.content_media_types = calloc(1, sizeof(*resp.content_media_types));
  resp.content_media_types[0].name =
      (char *)(size_t)(size_t) "application/json";
  op.responses = &resp;

  /* Make sure it doesn't get blocked by success_schema_name! */
  resp.schema.ref_name = NULL;

  /* array of string */
  resp.schema.inline_type = (char *)(size_t)(size_t) "string";
  resp.schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of integer */
  resp.schema.inline_type = (char *)(size_t)(size_t) "integer";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of number */
  resp.schema.inline_type = (char *)(size_t)(size_t) "number";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of boolean */
  resp.schema.inline_type = (char *)(size_t)(size_t) "boolean";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of fallback */
  resp.schema.inline_type = (char *)(size_t)(size_t) "unknown_type";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* not an array boolean */
  resp.schema.inline_type = (char *)(size_t)(size_t) "boolean";
  resp.schema.is_array = 0;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(resp.content_media_types);
  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_joined_form_array_direct(void) {
  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif
  {
    struct OpenAPI_Operation op = {0};
    struct OpenAPI_Spec spec = {0};
    struct OpenAPI_Encoding enc = {0};
    struct StructFields sf = {0};
    struct StructField f = {0};
    struct OpenAPI_MediaType mt = {0};
    openapi_spec_init(&spec);
#if defined(_MSC_VER)
    strcpy_s(f.name, sizeof(f.name), "arr");
#else
#if defined(_MSC_VER)
    strcpy_s(f.name, sizeof(f.name), "arr");
#else
    strcpy(f.name, "arr");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(f.type, sizeof(f.type), "array");
#else
#if defined(_MSC_VER)
    strcpy_s(f.type, sizeof(f.type), "array");
#else
    strcpy(f.type, "array");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(f.ref, sizeof(f.ref), "MyOtherStruct");
#else
#if defined(_MSC_VER)
    strcpy_s(f.ref, sizeof(f.ref), "MyOtherStruct");
#else
    strcpy(f.ref, "MyOtherStruct");
#endif
#endif
    sf.size = 1;
    sf.capacity = 1;
    sf.fields = calloc(1, sizeof(struct StructField));
    sf.fields[0] = f;
    spec.defined_schemas = calloc(1, sizeof(struct StructFields));
    spec.defined_schemas[0] = sf;
    spec.defined_schema_names = calloc(1, sizeof(char *));
    spec.defined_schema_names[0] = strdup("MyStruct");
    spec.n_defined_schemas = 1;
    op.verb = OA_VERB_POST;
    op.req_body.ref_name = (char *)(size_t)(size_t) "MyStruct";
    op.req_body.content_type =
        (char *)(size_t)(size_t) "application/x-www-form-urlencoded";
    enc.name = (char *)(size_t)(size_t) "arr";
    enc.style = OA_STYLE_FORM;
    enc.style_set = 1;
    enc.explode = 0;
    enc.explode_set = 1;
    mt.name = (char *)(size_t)(size_t) "application/x-www-form-urlencoded";
    mt.encoding = calloc(1, sizeof(struct OpenAPI_Encoding));
    mt.encoding[0] = enc;
    mt.n_encoding = 1;
    op.req_body_media_types = calloc(1, sizeof(struct OpenAPI_MediaType));
    op.req_body_media_types[0] = mt;
    op.n_req_body_media_types = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              codegen_client_write_body(fp, &op, &spec, "/test", NULL));
    op.req_body_media_types[0].encoding[0].style = OA_STYLE_SPACE_DELIMITED;
    ASSERT_EQ(CDD_C_SUCCESS,
              codegen_client_write_body(fp, &op, &spec, "/test", NULL));
    op.req_body_media_types[0].encoding[0].style = OA_STYLE_PIPE_DELIMITED;
    ASSERT_EQ(CDD_C_SUCCESS,
              codegen_client_write_body(fp, &op, &spec, "/test", NULL));

    op.req_body_media_types[0].encoding[0].style = OA_STYLE_FORM;

#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].ref,
             sizeof(spec.defined_schemas[0].fields[0].ref), "integer");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].ref,
             sizeof(spec.defined_schemas[0].fields[0].ref), "integer");
#else
    strcpy(spec.defined_schemas[0].fields[0].ref, "integer");
#endif
#endif
    ASSERT_EQ(CDD_C_SUCCESS,
              codegen_client_write_body(fp, &op, &spec, "/test", NULL));

#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].ref,
             sizeof(spec.defined_schemas[0].fields[0].ref), "number");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].ref,
             sizeof(spec.defined_schemas[0].fields[0].ref), "number");
#else
    strcpy(spec.defined_schemas[0].fields[0].ref, "number");
#endif
#endif
    ASSERT_EQ(CDD_C_SUCCESS,
              codegen_client_write_body(fp, &op, &spec, "/test", NULL));

#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].ref,
             sizeof(spec.defined_schemas[0].fields[0].ref), "boolean");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].ref,
             sizeof(spec.defined_schemas[0].fields[0].ref), "boolean");
#else
    strcpy(spec.defined_schemas[0].fields[0].ref, "boolean");
#endif
#endif
    ASSERT_EQ(CDD_C_SUCCESS,
              codegen_client_write_body(fp, &op, &spec, "/test", NULL));

#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].ref,
             sizeof(spec.defined_schemas[0].fields[0].ref), "string");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].ref,
             sizeof(spec.defined_schemas[0].fields[0].ref), "string");
#else
    strcpy(spec.defined_schemas[0].fields[0].ref, "string");
#endif
#endif
    ASSERT_EQ(CDD_C_SUCCESS,
              codegen_client_write_body(fp, &op, &spec, "/test", NULL));

#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].ref,
             sizeof(spec.defined_schemas[0].fields[0].ref), "unsupported_type");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].ref,
             sizeof(spec.defined_schemas[0].fields[0].ref), "unsupported_type");
#else
    strcpy(spec.defined_schemas[0].fields[0].ref, "unsupported_type");
#endif
#endif
    ASSERT_EQ(CDD_C_SUCCESS,
              codegen_client_write_body(fp, &op, &spec, "/test", NULL));

    if (fp)
      fclose(fp);
    free(spec.defined_schemas[0].fields);
    free(spec.defined_schemas);
    free(spec.defined_schema_names[0]);
    free(spec.defined_schema_names);
    free(op.req_body_media_types[0].encoding);
    free(op.req_body_media_types);
    PASS();
  }
}

TEST test_client_body_write_joined_form_array_direct_io(void) {
  int i;
  for (i = 0; i < 50; ++i) {
    FILE *fp;
#if defined(_MSC_VER)
    if (((fp = cdd_test_tmpfile_global()) == NULL))
      fp = NULL;
#else
    fp = cdd_test_tmpfile_global();
#endif
    {
      struct OpenAPI_Operation op = {0};
      struct OpenAPI_Spec spec = {0};
      struct OpenAPI_Encoding enc = {0};
      struct StructFields sf = {0};
      struct StructField f = {0};
      struct OpenAPI_MediaType mt = {0};
      openapi_spec_init(&spec);
#if defined(_MSC_VER)
      strcpy_s(f.name, sizeof(f.name), "arr");
#else
#if defined(_MSC_VER)
      strcpy_s(f.name, sizeof(f.name), "arr");
#else
      strcpy(f.name, "arr");
#endif
#endif
#if defined(_MSC_VER)
      strcpy_s(f.type, sizeof(f.type), "array");
#else
#if defined(_MSC_VER)
      strcpy_s(f.type, sizeof(f.type), "array");
#else
      strcpy(f.type, "array");
#endif
#endif
#if defined(_MSC_VER)
      strcpy_s(f.ref, sizeof(f.ref), "MyOtherStruct");
#else
#if defined(_MSC_VER)
      strcpy_s(f.ref, sizeof(f.ref), "MyOtherStruct");
#else
      strcpy(f.ref, "MyOtherStruct");
#endif
#endif
      sf.size = 1;
      sf.capacity = 1;
      sf.fields = calloc(1, sizeof(struct StructField));
      sf.fields[0] = f;
      spec.defined_schemas = calloc(1, sizeof(struct StructFields));
      spec.defined_schemas[0] = sf;
      spec.defined_schema_names = calloc(1, sizeof(char *));
      spec.defined_schema_names[0] = strdup("MyStruct");
      spec.n_defined_schemas = 1;
      op.verb = OA_VERB_POST;
      op.req_body.ref_name = (char *)(size_t)(size_t) "MyStruct";
      op.req_body.content_type =
          (char *)(size_t)(size_t) "application/x-www-form-urlencoded";
      enc.name = (char *)(size_t)(size_t) "arr";
      enc.style = OA_STYLE_FORM;
      enc.style_set = 1;
      enc.explode = 0;
      enc.explode_set = 1;
      mt.name = (char *)(size_t)(size_t) "application/x-www-form-urlencoded";
      mt.encoding = calloc(1, sizeof(struct OpenAPI_Encoding));
      mt.encoding[0] = enc;
      mt.n_encoding = 1;
      op.req_body_media_types = calloc(1, sizeof(struct OpenAPI_MediaType));
      op.req_body_media_types[0] = mt;
      op.n_req_body_media_types = 1;
      g_io_calls = 0;
      g_fail_io_after = i;
      codegen_client_write_body(fp, &op, &spec, "/test", NULL);
      g_fail_io_after = -1;
      op.req_body_media_types[0].encoding[0].style = OA_STYLE_SPACE_DELIMITED;
      g_fail_io_after = i;
      codegen_client_write_body(fp, &op, &spec, "/test", NULL);
      g_fail_io_after = -1;
      op.req_body_media_types[0].encoding[0].style = OA_STYLE_PIPE_DELIMITED;
      g_fail_io_after = i;
      codegen_client_write_body(fp, &op, &spec, "/test", NULL);
      g_fail_io_after = -1;

      op.req_body_media_types[0].encoding[0].style = OA_STYLE_FORM;

#if defined(_MSC_VER)
      strcpy_s(spec.defined_schemas[0].fields[0].ref,
               sizeof(spec.defined_schemas[0].fields[0].ref), "integer");
#else
#if defined(_MSC_VER)
      strcpy_s(spec.defined_schemas[0].fields[0].ref,
               sizeof(spec.defined_schemas[0].fields[0].ref), "integer");
#else
      strcpy(spec.defined_schemas[0].fields[0].ref, "integer");
#endif
#endif
      g_fail_io_after = i;
      codegen_client_write_body(fp, &op, &spec, "/test", NULL);
      g_fail_io_after = -1;

#if defined(_MSC_VER)
      strcpy_s(spec.defined_schemas[0].fields[0].ref,
               sizeof(spec.defined_schemas[0].fields[0].ref), "number");
#else
#if defined(_MSC_VER)
      strcpy_s(spec.defined_schemas[0].fields[0].ref,
               sizeof(spec.defined_schemas[0].fields[0].ref), "number");
#else
      strcpy(spec.defined_schemas[0].fields[0].ref, "number");
#endif
#endif
      g_fail_io_after = i;
      codegen_client_write_body(fp, &op, &spec, "/test", NULL);
      g_fail_io_after = -1;

#if defined(_MSC_VER)
      strcpy_s(spec.defined_schemas[0].fields[0].ref,
               sizeof(spec.defined_schemas[0].fields[0].ref), "boolean");
#else
#if defined(_MSC_VER)
      strcpy_s(spec.defined_schemas[0].fields[0].ref,
               sizeof(spec.defined_schemas[0].fields[0].ref), "boolean");
#else
      strcpy(spec.defined_schemas[0].fields[0].ref, "boolean");
#endif
#endif
      g_fail_io_after = i;
      codegen_client_write_body(fp, &op, &spec, "/test", NULL);
      g_fail_io_after = -1;

#if defined(_MSC_VER)
      strcpy_s(spec.defined_schemas[0].fields[0].ref,
               sizeof(spec.defined_schemas[0].fields[0].ref), "string");
#else
#if defined(_MSC_VER)
      strcpy_s(spec.defined_schemas[0].fields[0].ref,
               sizeof(spec.defined_schemas[0].fields[0].ref), "string");
#else
      strcpy(spec.defined_schemas[0].fields[0].ref, "string");
#endif
#endif
      g_fail_io_after = i;
      codegen_client_write_body(fp, &op, &spec, "/test", NULL);
      g_fail_io_after = -1;

#if defined(_MSC_VER)
      strcpy_s(spec.defined_schemas[0].fields[0].ref,
               sizeof(spec.defined_schemas[0].fields[0].ref),
               "unsupported_type");
#else
#if defined(_MSC_VER)
      strcpy_s(spec.defined_schemas[0].fields[0].ref,
               sizeof(spec.defined_schemas[0].fields[0].ref),
               "unsupported_type");
#else
      strcpy(spec.defined_schemas[0].fields[0].ref, "unsupported_type");
#endif
#endif
      g_fail_io_after = i;
      codegen_client_write_body(fp, &op, &spec, "/test", NULL);
      g_fail_io_after = -1;

      if (fp)
        fclose(fp);
      free(spec.defined_schemas[0].fields);
      free(spec.defined_schemas);
      free(spec.defined_schema_names[0]);
      free(spec.defined_schema_names);
      free(op.req_body_media_types[0].encoding);
      free(op.req_body_media_types);
    }
  }
  PASS();
}

TEST test_client_body_write_joined_form_array(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = (char *)(size_t)(size_t) "get";
  op.verb = OA_VERB_GET;
  op.operation_id = (char *)(size_t)(size_t) "testJoinedFormArray";

  op.n_parameters = 1;
  op.parameters = calloc(1, sizeof(*op.parameters));
  op.parameters[0].name = (char *)(size_t)(size_t) "query_arr";
  op.parameters[0].in = OA_PARAM_IN_QUERY;
  op.parameters[0].type = (char *)(size_t)(size_t) "array";
  op.parameters[0].items_type = (char *)(size_t)(size_t) "object";
  op.parameters[0].style = OA_STYLE_FORM;
  op.parameters[0].explode_set = 1;
  op.parameters[0].explode = 0;

  /* Set array of strings basically via mock */
  op.parameters[0].items_type = (char *)(size_t)(size_t) "string";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.parameters[0].items_type = (char *)(size_t)(size_t) "integer";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.parameters[0].items_type = (char *)(size_t)(size_t) "number";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.parameters[0].items_type = (char *)(size_t)(size_t) "boolean";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.parameters[0].items_type = (char *)(size_t)(size_t) "unsupported";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Also try different delim via pipedd */
  op.parameters[0].style = OA_STYLE_PIPE_DELIMITED;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* space delimited */
  op.parameters[0].style = OA_STYLE_SPACE_DELIMITED;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(op.parameters);
  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_text_plain_success_indirect_real_fixed3(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp;
#if defined(_MSC_VER)
  if (((fp = cdd_test_tmpfile_global()) == NULL))
    fp = NULL;
#else
  fp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = (char *)(size_t)(size_t) "get";
  op.verb = OA_VERB_GET;
  op.operation_id =
      (char *)(size_t)(size_t) "testResponseTextualSuccessIndirectRealFixed3";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = (char *)(size_t)(size_t) "200";
  resp.content_type = (char *)(size_t)(size_t) "text/plain";
  resp.schema.inline_type = (char *)(size_t)(size_t) "string";
  resp.schema.is_array = 0;

  op.responses = &resp;

  /* Let's actually test response_is_textual_string AND schema_inline_is_string
   */
  /* indirectly The reason it wasn't hit previously is because */
  /* codegen_client_write_body skips generating the response code if we don't */
  /* have any parameters! No wait. Let's ensure the full operation is populated
   */
  /* properly. */
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

SUITE(client_body_joined_suite) {
  RUN_TEST(test_client_body_form_object_style_form_explode);
  RUN_TEST(test_client_body_cookie_object_style_form_explode);
  RUN_TEST(test_client_body_response_is_textual_string_indirect);
  RUN_TEST(test_client_body_response_is_textual_string_success);
  RUN_TEST(test_client_body_write_text_plain_success_indirect);
  RUN_TEST(test_client_body_write_binary_success_indirect_real);
  RUN_TEST(test_client_body_write_text_plain_success_indirect_real_fixed);
  RUN_TEST(test_client_body_write_text_plain_success_indirect_real_fixed4);
  RUN_TEST(test_client_body_write_inline_json_parse_types_indirect_string);
  RUN_TEST(test_client_body_write_joined_form_array_direct);
  RUN_TEST(test_client_body_write_joined_form_array_direct_io);
  RUN_TEST(test_client_body_write_joined_form_array);
  RUN_TEST(test_client_body_write_text_plain_success_indirect_real_fixed3);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_JOINED_H */
