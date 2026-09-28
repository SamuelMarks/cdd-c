/**
 * @file test_codegen_client_body_media.h
 * @brief Unit tests for client body media types and inline parsing.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_MEDIA_H
#define TEST_CODEGEN_CLIENT_BODY_MEDIA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_body_common.h"
/* clang-format on */

TEST test_client_body_verb_enum_indirect(void) {
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

  op.operation_id = (char *)(size_t)(size_t) "testVerb";

  op.verb = OA_VERB_PUT;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.verb = OA_VERB_DELETE;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.verb = OA_VERB_HEAD;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.verb = OA_VERB_PATCH;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_header_formatting_indirect(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Header hdr;
  struct OpenAPI_MultipartField mf;
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
  op.operation_id = (char *)(size_t)(size_t) "testHdrFormat";

  /* Set up global schemas to bypass properties missing error */
  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  c_cdd_strdup("MockSchemaHdr", &spec.defined_schema_names[0]);

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "1test_prop");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "1test_prop");
#else
  strcpy(spec.defined_schemas[0].fields[0].name, "1test_prop");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
  strcpy(spec.defined_schemas[0].fields[0].type, "string");
#endif
#endif

  op.req_body.ref_name = (char *)(size_t)(size_t) "MockSchemaHdr";

  /* Multipart data with specific encodings to hit header name formatting */
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/mixed";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = (char *)(size_t)(size_t) "multipart/mixed";

  memset(&enc, 0, sizeof(enc));
  enc.name = (char *)(size_t)(size_t) "1test_prop";

  memset(&hdr, 0, sizeof(hdr));
  hdr.name = (char *)(size_t)(size_t) "1Content-Type"; /* hit sanitize starting
                                                  with number, and it isn't */
  /* Content-Type exact */

  enc.n_headers = 1;
  enc.headers = &hdr;

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  memset(&mf, 0, sizeof(mf));
  mf.name = (char *)(size_t)(size_t) "1test_prop";
  mf.type = (char *)(size_t)(size_t) "string";
  mf.is_binary = 0;
  op.req_body.n_multipart_fields = 1;
  op.req_body.multipart_fields = &mf;

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

TEST test_client_body_media_types_textual_binary_indirect(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MultipartField mf;
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
  op.operation_id = (char *)(size_t)(size_t) "testMediaTypeClassifiers";

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  c_cdd_strdup("MockSchemaTxtBin", &spec.defined_schema_names[0]);

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "test_prop");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "test_prop");
#else
  strcpy(spec.defined_schemas[0].fields[0].name, "test_prop");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
  strcpy(spec.defined_schemas[0].fields[0].type, "string");
#endif
#endif

  op.req_body.ref_name = (char *)(size_t)(size_t) "MockSchemaTxtBin";

  /* Hit textual formatters indirectly via multipart field generation */
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/mixed";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = (char *)(size_t)(size_t) "multipart/mixed";

  memset(&enc, 0, sizeof(enc));
  enc.name = (char *)(size_t)(size_t) "test_prop";

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  memset(&mf, 0, sizeof(mf));
  mf.name = (char *)(size_t)(size_t) "test_prop";
  mf.type = (char *)(size_t)(size_t) "string";
  mf.is_binary = 0;
  op.req_body.n_multipart_fields = 1;
  op.req_body.multipart_fields = &mf;

  /* text/html */
  enc.content_type = (char *)(size_t)(size_t) "text/html";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* application/xml */
  enc.content_type = (char *)(size_t)(size_t) "application/xml";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* application/rss+xml */
  enc.content_type = (char *)(size_t)(size_t) "application/rss+xml";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* image/png (binary) */
  enc.content_type = (char *)(size_t)(size_t) "image/png";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* audio/mp3 (binary) */
  enc.content_type = (char *)(size_t)(size_t) "audio/mp3";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* video/mp4 (binary) */
  enc.content_type = (char *)(size_t)(size_t) "video/mp4";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* application/pdf (binary) */
  enc.content_type = (char *)(size_t)(size_t) "application/pdf";
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

TEST test_client_body_media_types_textual_binary_missing_branches_indirect(
    void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MultipartField mf;
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
  op.operation_id = (char *)(size_t)(size_t) "testMediaTypeClassifiersMissing";

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  c_cdd_strdup("MockSchemaMissing", &spec.defined_schema_names[0]);

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "test_prop");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "test_prop");
#else
  strcpy(spec.defined_schemas[0].fields[0].name, "test_prop");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
  strcpy(spec.defined_schemas[0].fields[0].type, "string");
#endif
#endif

  op.req_body.ref_name = (char *)(size_t)(size_t) "MockSchemaMissing";

  /* Hit textual formatters indirectly via multipart field generation */
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/mixed";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = (char *)(size_t)(size_t) "multipart/mixed";

  memset(&enc, 0, sizeof(enc));
  enc.name = (char *)(size_t)(size_t) "test_prop";

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  memset(&mf, 0, sizeof(mf));
  mf.name = (char *)(size_t)(size_t) "test_prop";
  mf.type = (char *)(size_t)(size_t) "string";
  mf.is_binary = 0;
  op.req_body.n_multipart_fields = 1;
  op.req_body.multipart_fields = &mf;

  /* text/css (textual prefix) */
  enc.content_type = (char *)(size_t)(size_t) "text/css";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* application/atom+xml (textual suffix) */
  enc.content_type = (char *)(size_t)(size_t) "application/atom+xml";
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

TEST test_client_body_media_type_caps_indirect(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MultipartField mf;
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
  op.operation_id = (char *)(size_t)(size_t) "testMediaCapsIndirect";

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  c_cdd_strdup("MockSchemaCaps", &spec.defined_schema_names[0]);

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "test_prop");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "test_prop");
#else
  strcpy(spec.defined_schemas[0].fields[0].name, "test_prop");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
  strcpy(spec.defined_schemas[0].fields[0].type, "string");
#endif
#endif

  op.req_body.ref_name = (char *)(size_t)(size_t) "MockSchemaCaps";

  /* Mixed upper and lower caps */
  op.req_body.content_type = (char *)(size_t)(size_t) "MULTIPART/MIXED";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = (char *)(size_t)(size_t) "multipart/mixed";

  memset(&enc, 0, sizeof(enc));
  enc.name = (char *)(size_t)(size_t) "test_prop";

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  memset(&mf, 0, sizeof(mf));
  mf.name = (char *)(size_t)(size_t) "test_prop";
  mf.type = (char *)(size_t)(size_t) "string";
  mf.is_binary = 0;
  op.req_body.n_multipart_fields = 1;
  op.req_body.multipart_fields = &mf;

  /* text/plain in caps */
  enc.content_type = (char *)(size_t)(size_t) "TEXT/PLAIN";
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

TEST test_client_body_media_type_prefix_caps_indirect(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MultipartField mf;
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
  op.operation_id = (char *)(size_t)(size_t) "testMediaPrefixCapsIndirect";

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  c_cdd_strdup("MockSchemaPrefixCaps", &spec.defined_schema_names[0]);

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "test_prop");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "test_prop");
#else
  strcpy(spec.defined_schemas[0].fields[0].name, "test_prop");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
  strcpy(spec.defined_schemas[0].fields[0].type, "string");
#endif
#endif

  op.req_body.ref_name = (char *)(size_t)(size_t) "MockSchemaPrefixCaps";

  /* Hit prefix upper caps */
  op.req_body.content_type = (char *)(size_t)(size_t) "MULTIPART/MIXED";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = (char *)(size_t)(size_t) "multipart/mixed";

  memset(&enc, 0, sizeof(enc));
  enc.name = (char *)(size_t)(size_t) "test_prop";

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  memset(&mf, 0, sizeof(mf));
  mf.name = (char *)(size_t)(size_t) "test_prop";
  mf.type = (char *)(size_t)(size_t) "string";
  mf.is_binary = 0;
  op.req_body.n_multipart_fields = 1;
  op.req_body.multipart_fields = &mf;

  /* text/plain in mixed caps to hit prefix */
  enc.content_type = (char *)(size_t)(size_t) "TeXt/PlaIN";
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

TEST test_client_body_media_type_prefix_suffix_short(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MultipartField mf;
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
  op.operation_id = (char *)(size_t)(size_t) "testMediaShort";

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  c_cdd_strdup("MockSchemaShort", &spec.defined_schema_names[0]);

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "test_prop");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].name,
           sizeof(spec.defined_schemas[0].fields[0].name), "test_prop");
#else
  strcpy(spec.defined_schemas[0].fields[0].name, "test_prop");
#endif
#endif
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
#if defined(_MSC_VER)
  strcpy_s(spec.defined_schemas[0].fields[0].type,
           sizeof(spec.defined_schemas[0].fields[0].type), "string");
#else
  strcpy(spec.defined_schemas[0].fields[0].type, "string");
#endif
#endif

  op.req_body.ref_name = (char *)(size_t)(size_t) "MockSchemaShort";

  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/mixed";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = (char *)(size_t)(size_t) "multipart/mixed";

  memset(&enc, 0, sizeof(enc));
  enc.name = (char *)(size_t)(size_t) "test_prop";

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  memset(&mf, 0, sizeof(mf));
  mf.name = (char *)(size_t)(size_t) "test_prop";
  mf.type = (char *)(size_t)(size_t) "string";
  mf.is_binary = 0;
  op.req_body.n_multipart_fields = 1;
  op.req_body.multipart_fields = &mf;

  /* text/ (len < pre_len = 5) -> e.g. "tex" */
  enc.content_type = (char *)(size_t)(size_t) "tex";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* +xml (len < suf_len = 4) -> e.g. "xm" */
  enc.content_type = (char *)(size_t)(size_t) "xm";
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

TEST test_client_body_write_inline_json_parse_indirect(void) {
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
  op.operation_id = (char *)(size_t)(size_t) "testInlineParseIndirect";

  /* We need to hit write_response_read -> write_inline_json_parse */
  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = (char *)(size_t)(size_t) "200";

  /* Setup media type with inline schema */
  resp.n_content_media_types = 1;
  resp.content_media_types = calloc(1, sizeof(*resp.content_media_types));
  resp.content_media_types[0].name =
      (char *)(size_t)(size_t) "application/json";

  /* Test array of string */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "string";
  resp.content_media_types[0].schema.is_array = 1;
  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Test array of integer */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "integer";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Test array of number */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "number";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Test array of boolean */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "boolean";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Test raw boolean */
  resp.content_media_types[0].schema.is_array = 0;
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "boolean";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Test unhandled / unknown (e.g., fallback) */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "unknown_type_test";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Add missing branch from write_response_read (raw array of non-json) */
  resp.content_media_types[0].name = (char *)(size_t)(size_t) "text/plain";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(resp.content_media_types);
  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_inline_json_parse_types(void) {
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
  op.operation_id = (char *)(size_t)(size_t) "testInlineParseTypes";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = (char *)(size_t)(size_t) "200";

  resp.n_content_media_types = 1;
  resp.content_media_types = calloc(1, sizeof(*resp.content_media_types));
  resp.content_media_types[0].name =
      (char *)(size_t)(size_t) "application/json";
  op.responses = &resp;

  /* array of string */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "string";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of int */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "integer";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of number */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "number";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of boolean */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "boolean";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of fallback/unknown */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "unknown_test";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* raw boolean */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "boolean";
  resp.content_media_types[0].schema.is_array = 0;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of string (non-json) */
  resp.content_media_types[0].name = (char *)(size_t)(size_t) "text/plain";
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "string";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(resp.content_media_types);
  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_inline_json_parse_types_indirect(void) {
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
  op.operation_id = (char *)(size_t)(size_t) "testInlineParseTypes";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = (char *)(size_t)(size_t) "200";

  resp.n_content_media_types = 1;
  resp.content_media_types = calloc(1, sizeof(*resp.content_media_types));
  resp.content_media_types[0].name =
      (char *)(size_t)(size_t) "application/json";
  op.responses = &resp;

  /* Need to provide a valid body payload via dummy schema */
  /* so the codegen logic is fully hit and NOT skipped! */

  /* array of string */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "string";
  resp.content_media_types[0].schema.is_array = 1;
  /* Make it think the response is an object with an inline schema so it hits */
  /* the array codegen! No, write_inline_json_parse writes code! We just need it
   */
  /* to emit the C code blocks! */
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of int */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "integer";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of number */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "number";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of boolean */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "boolean";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of fallback/unknown */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "unknown_test";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* raw boolean */
  resp.content_media_types[0].schema.inline_type =
      (char *)(size_t)(size_t) "boolean";
  resp.content_media_types[0].schema.is_array = 0;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(resp.content_media_types);
  if (fp)
    fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

SUITE(client_body_media_suite) {
  RUN_TEST(test_client_body_verb_enum_indirect);
  RUN_TEST(test_client_body_header_formatting_indirect);
  RUN_TEST(test_client_body_media_types_textual_binary_indirect);
  RUN_TEST(
      test_client_body_media_types_textual_binary_missing_branches_indirect);
  RUN_TEST(test_client_body_media_type_caps_indirect);
  RUN_TEST(test_client_body_media_type_prefix_caps_indirect);
  RUN_TEST(test_client_body_media_type_prefix_suffix_short);
  RUN_TEST(test_client_body_write_inline_json_parse_indirect);
  RUN_TEST(test_client_body_write_inline_json_parse_types);
  RUN_TEST(test_client_body_write_inline_json_parse_types_indirect);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_MEDIA_H */
