/**
 * @file test_c2openapi_op_coverage.h
 * @brief Cleanup and reset coverage tests for Operation Builder.
 */

#ifndef TEST_C2OPENAPI_OP_COVERAGE_H
#define TEST_C2OPENAPI_OP_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "parse/test_c2openapi_op_helpers.h"

TEST test_reset_op_coverage(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Response resp;
  struct OpenAPI_Link link;
  struct OpenAPI_LinkParam lp;
  char *tag;
  const struct OpenAPI_MediaType *found_mt = NULL;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  memset(&mt, 0, sizeof(mt));
  memset(&resp, 0, sizeof(resp));
  memset(&link, 0, sizeof(link));
  memset(&lp, 0, sizeof(lp));

  op.operation_id = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.operation_id, 32, "op_id");
#else
  strcpy(op.operation_id, "op_id");
#endif
  op.summary = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.summary, 32, "summary");
#else
  strcpy(op.summary, "summary");
#endif
  op.description = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.description, 32, "desc");
#else
  strcpy(op.description, "desc");
#endif
  op.method = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.method, 32, "GET");
#else
  strcpy(op.method, "GET");
#endif

  /* Parameter coverage */
  param.name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.name, 32, "param");
#else
  strcpy(param.name, "param");
#endif
  param.type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.type, 32, "string");
#else
  strcpy(param.type, "string");
#endif
  param.description = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.description, 32, "pdesc");
#else
  strcpy(param.description, "pdesc");
#endif
  param.content_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.content_type, 32, "text/plain");
#else
  strcpy(param.content_type, "text/plain");
#endif
  param.items_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.items_type, 32, "int");
#else
  strcpy(param.items_type, "int");
#endif
  param.example.type = OA_ANY_STRING;
  param.example.string = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.example.string, 32, "ex_str");
#else
  strcpy(param.example.string, "ex_str");
#endif
  param.schema.ref_name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.schema.ref_name, 32, "RefN");
#else
  strcpy(param.schema.ref_name, "RefN");
#endif
  param.schema.ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.schema.ref, 32, "Ref");
#else
  strcpy(param.schema.ref, "Ref");
#endif
  param.schema.inline_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.schema.inline_type, 32, "inline");
#else
  strcpy(param.schema.inline_type, "inline");
#endif
  param.schema.items_ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.schema.items_ref, 32, "itemsRef");
#else
  strcpy(param.schema.items_ref, "itemsRef");
#endif
  param.schema.format = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.schema.format, 32, "fmt");
#else
  strcpy(param.schema.format, "fmt");
#endif
  param.schema.items_format = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.schema.items_format, 32, "ifmt");
#else
  strcpy(param.schema.items_format, "ifmt");
#endif
  param.schema.content_media_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.schema.content_media_type, 32, "cmt");
#else
  strcpy(param.schema.content_media_type, "cmt");
#endif
  param.schema.content_encoding = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.schema.content_encoding, 32, "ce");
#else
  strcpy(param.schema.content_encoding, "ce");
#endif
  param.schema.items_content_media_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.schema.items_content_media_type, 32, "icmt");
#else
  strcpy(param.schema.items_content_media_type, "icmt");
#endif
  param.schema.items_content_encoding = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(param.schema.items_content_encoding, 32, "ice");
#else
  strcpy(param.schema.items_content_encoding, "ice");
#endif

  op.parameters =
      (struct OpenAPI_Parameter *)malloc(sizeof(struct OpenAPI_Parameter) * 2);
  op.parameters[0] = param;
  memset(&op.parameters[1], 0, sizeof(struct OpenAPI_Parameter));
  op.parameters[1].example.type = OA_ANY_JSON;
  op.parameters[1].example.json = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.parameters[1].example.json, 32, "{}");
#else
  strcpy(op.parameters[1].example.json, "{}");
#endif
  op.n_parameters = 2;

  /* Tags coverage */
  tag = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(tag, 32, "tag1");
#else
  strcpy(tag, "tag1");
#endif
  op.tags = (char **)malloc(sizeof(char *));
  op.tags[0] = tag;
  op.n_tags = 1;

  /* External docs */
  op.external_docs.url = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.external_docs.url, 32, "http");
#else
  strcpy(op.external_docs.url, "http");
#endif
  op.external_docs.description = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.external_docs.description, 32, "doc");
#else
  strcpy(op.external_docs.description, "doc");
#endif

  /* Request body */
  op.req_body_extensions_json = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.req_body_extensions_json, 32, "{}");
#else
  strcpy(op.req_body_extensions_json, "{}");
#endif
  op.req_body_ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.req_body_ref, 32, "rbRef");
#else
  strcpy(op.req_body_ref, "rbRef");
#endif
  op.req_body.ref_name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.req_body.ref_name, 32, "rbRefN");
#else
  strcpy(op.req_body.ref_name, "rbRefN");
#endif
  op.req_body.inline_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.req_body.inline_type, 32, "rbInl");
#else
  strcpy(op.req_body.inline_type, "rbInl");
#endif
  op.req_body.content_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.req_body.content_type, 32, "rbCt");
#else
  strcpy(op.req_body.content_type, "rbCt");
#endif
  op.req_body.example.type = OA_ANY_STRING;
  op.req_body.example.string = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.req_body.example.string, 32, "rbEx");
#else
  strcpy(op.req_body.example.string, "rbEx");
#endif
  reset_op(&op);
  memset(&op, 0, sizeof(op));
  op.req_body.example.type = OA_ANY_JSON;
  op.req_body.example.json = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.req_body.example.json, 32, "{}");
#else
  strcpy(op.req_body.example.json, "{}");
#endif

  /* req_body_media_types */
  mt.name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.name, 32, "app/json");
#else
  strcpy(mt.name, "app/json");
#endif
  mt.ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.ref, 32, "ref");
#else
  strcpy(mt.ref, "ref");
#endif
  mt.extensions_json = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.extensions_json, 32, "ext");
#else
  strcpy(mt.extensions_json, "ext");
#endif
  mt.schema.ref_name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.schema.ref_name, 32, "rn");
#else
  strcpy(mt.schema.ref_name, "rn");
#endif
  mt.schema.ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.schema.ref, 32, "r");
#else
  strcpy(mt.schema.ref, "r");
#endif
  mt.schema.inline_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.schema.inline_type, 32, "it");
#else
  strcpy(mt.schema.inline_type, "it");
#endif
  mt.schema.items_ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.schema.items_ref, 32, "ir");
#else
  strcpy(mt.schema.items_ref, "ir");
#endif
  mt.schema.format = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.schema.format, 32, "f");
#else
  strcpy(mt.schema.format, "f");
#endif
  mt.schema.items_format = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.schema.items_format, 32, "if");
#else
  strcpy(mt.schema.items_format, "if");
#endif
  mt.schema.content_media_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.schema.content_media_type, 32, "cmt");
#else
  strcpy(mt.schema.content_media_type, "cmt");
#endif
  mt.schema.content_encoding = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.schema.content_encoding, 32, "ce");
#else
  strcpy(mt.schema.content_encoding, "ce");
#endif
  mt.schema.items_content_media_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.schema.items_content_media_type, 32, "icmt");
#else
  strcpy(mt.schema.items_content_media_type, "icmt");
#endif
  mt.schema.items_content_encoding = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.schema.items_content_encoding, 32, "ice");
#else
  strcpy(mt.schema.items_content_encoding, "ice");
#endif
  mt.example.type = OA_ANY_STRING;
  mt.example.string = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(mt.example.string, 32, "ex");
#else
  strcpy(mt.example.string, "ex");
#endif

  op.req_body_media_types =
      (struct OpenAPI_MediaType *)malloc(sizeof(struct OpenAPI_MediaType) * 2);
  op.req_body_media_types[0] = mt;
  memset(&op.req_body_media_types[1], 0, sizeof(struct OpenAPI_MediaType));
  op.req_body_media_types[1].example.type = OA_ANY_JSON;
  op.req_body_media_types[1].example.json = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.req_body_media_types[1].example.json, 32, "{}");
#else
  strcpy(op.req_body_media_types[1].example.json, "{}");
#endif
  op.n_req_body_media_types = 2;

  /* Responses with content_media_types, headers, and links */
  resp.description = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.description, 32, "rdesc");
#else
  strcpy(resp.description, "rdesc");
#endif
  resp.summary = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.summary, 32, "rsum");
#else
  strcpy(resp.summary, "rsum");
#endif
  resp.code = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.code, 32, "200");
#else
  strcpy(resp.code, "200");
#endif
  resp.content_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_type, 32, "rCt");
#else
  strcpy(resp.content_type, "rCt");
#endif
  resp.example.type = OA_ANY_STRING;
  resp.example.string = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.example.string, 32, "re_str");
#else
  strcpy(resp.example.string, "re_str");
#endif
  resp.schema.ref_name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.schema.ref_name, 32, "rsrn");
#else
  strcpy(resp.schema.ref_name, "rsrn");
#endif
  resp.schema.inline_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.schema.inline_type, 32, "rsit");
#else
  strcpy(resp.schema.inline_type, "rsit");
#endif

  resp.content_media_types =
      (struct OpenAPI_MediaType *)malloc(sizeof(struct OpenAPI_MediaType) * 2);
  memset(&resp.content_media_types[0], 0, sizeof(struct OpenAPI_MediaType));
  resp.content_media_types[0].name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].name, 32, "text/csv");
#else
  strcpy(resp.content_media_types[0].name, "text/csv");
#endif
  resp.content_media_types[0].ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].ref, 32, "cmt_ref");
#else
  strcpy(resp.content_media_types[0].ref, "cmt_ref");
#endif
  resp.content_media_types[0].extensions_json = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].extensions_json, 32, "ext");
#else
  strcpy(resp.content_media_types[0].extensions_json, "ext");
#endif
  resp.content_media_types[0].schema.ref_name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].schema.ref_name, 32, "srn");
#else
  strcpy(resp.content_media_types[0].schema.ref_name, "srn");
#endif
  resp.content_media_types[0].schema.ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].schema.ref, 32, "sr");
#else
  strcpy(resp.content_media_types[0].schema.ref, "sr");
#endif
  resp.content_media_types[0].schema.inline_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].schema.inline_type, 32, "sit");
#else
  strcpy(resp.content_media_types[0].schema.inline_type, "sit");
#endif
  resp.content_media_types[0].schema.items_ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].schema.items_ref, 32, "sir");
#else
  strcpy(resp.content_media_types[0].schema.items_ref, "sir");
#endif
  resp.content_media_types[0].schema.format = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].schema.format, 32, "sf");
#else
  strcpy(resp.content_media_types[0].schema.format, "sf");
#endif
  resp.content_media_types[0].schema.items_format = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].schema.items_format, 32, "sif");
#else
  strcpy(resp.content_media_types[0].schema.items_format, "sif");
#endif
  resp.content_media_types[0].schema.content_media_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].schema.content_media_type, 32, "scmt");
#else
  strcpy(resp.content_media_types[0].schema.content_media_type, "scmt");
#endif
  resp.content_media_types[0].schema.content_encoding = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].schema.content_encoding, 32, "sce");
#else
  strcpy(resp.content_media_types[0].schema.content_encoding, "sce");
#endif
  resp.content_media_types[0].schema.items_content_media_type =
      (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].schema.items_content_media_type, 32,
           "sicmt");
#else
  strcpy(resp.content_media_types[0].schema.items_content_media_type, "sicmt");
#endif
  resp.content_media_types[0].schema.items_content_encoding =
      (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].schema.items_content_encoding, 32,
           "sice");
#else
  strcpy(resp.content_media_types[0].schema.items_content_encoding, "sice");
#endif
  resp.content_media_types[0].example.type = OA_ANY_STRING;
  resp.content_media_types[0].example.string = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[0].example.string, 32, "cmt_str");
#else
  strcpy(resp.content_media_types[0].example.string, "cmt_str");
#endif

  memset(&resp.content_media_types[1], 0, sizeof(struct OpenAPI_MediaType));
  resp.content_media_types[1].example.type = OA_ANY_JSON;
  resp.content_media_types[1].example.json = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.content_media_types[1].example.json, 32, "{}");
#else
  strcpy(resp.content_media_types[1].example.json, "{}");
#endif
  resp.n_content_media_types = 2;

  resp.headers = (struct OpenAPI_Header *)malloc(sizeof(struct OpenAPI_Header));
  memset(&resp.headers[0], 0, sizeof(struct OpenAPI_Header));
  resp.headers[0].name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].name, 32, "X-Hdr");
#else
  strcpy(resp.headers[0].name, "X-Hdr");
#endif
  resp.headers[0].ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].ref, 32, "href");
#else
  strcpy(resp.headers[0].ref, "href");
#endif
  resp.headers[0].description = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].description, 32, "HdrDesc");
#else
  strcpy(resp.headers[0].description, "HdrDesc");
#endif
  resp.headers[0].content_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].content_type, 32, "hct");
#else
  strcpy(resp.headers[0].content_type, "hct");
#endif
  resp.headers[0].content_ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].content_ref, 32, "hcr");
#else
  strcpy(resp.headers[0].content_ref, "hcr");
#endif
  resp.headers[0].type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].type, 32, "int");
#else
  strcpy(resp.headers[0].type, "int");
#endif
  resp.headers[0].items_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].items_type, 32, "item");
#else
  strcpy(resp.headers[0].items_type, "item");
#endif
  resp.headers[0].example.type = OA_ANY_STRING;
  resp.headers[0].example.string = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].example.string, 32, "hex");
#else
  strcpy(resp.headers[0].example.string, "hex");
#endif
  resp.headers[0].schema.ref_name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].schema.ref_name, 32, "hsrn");
#else
  strcpy(resp.headers[0].schema.ref_name, "hsrn");
#endif
  resp.headers[0].schema.ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].schema.ref, 32, "hsr");
#else
  strcpy(resp.headers[0].schema.ref, "hsr");
#endif
  resp.headers[0].schema.inline_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].schema.inline_type, 32, "hsi");
#else
  strcpy(resp.headers[0].schema.inline_type, "hsi");
#endif
  resp.headers[0].schema.items_ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].schema.items_ref, 32, "hsir");
#else
  strcpy(resp.headers[0].schema.items_ref, "hsir");
#endif
  resp.headers[0].schema.format = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].schema.format, 32, "hsf");
#else
  strcpy(resp.headers[0].schema.format, "hsf");
#endif
  resp.headers[0].schema.items_format = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].schema.items_format, 32, "hsif");
#else
  strcpy(resp.headers[0].schema.items_format, "hsif");
#endif
  resp.headers[0].schema.content_media_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].schema.content_media_type, 32, "hscmt");
#else
  strcpy(resp.headers[0].schema.content_media_type, "hscmt");
#endif
  resp.headers[0].schema.content_encoding = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].schema.content_encoding, 32, "hsce");
#else
  strcpy(resp.headers[0].schema.content_encoding, "hsce");
#endif
  resp.headers[0].schema.items_content_media_type = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].schema.items_content_media_type, 32, "hsicmt");
#else
  strcpy(resp.headers[0].schema.items_content_media_type, "hsicmt");
#endif
  resp.headers[0].schema.items_content_encoding = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.headers[0].schema.items_content_encoding, 32, "hsice");
#else
  strcpy(resp.headers[0].schema.items_content_encoding, "hsice");
#endif
  resp.n_headers = 1;

  /* links */
  link.name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(link.name, 32, "L1");
#else
  strcpy(link.name, "L1");
#endif
  link.ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(link.ref, 32, "lref");
#else
  strcpy(link.ref, "lref");
#endif
  link.summary = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(link.summary, 32, "lsum");
#else
  strcpy(link.summary, "lsum");
#endif
  link.operation_ref = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(link.operation_ref, 32, "opRef");
#else
  strcpy(link.operation_ref, "opRef");
#endif
  link.operation_id = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(link.operation_id, 32, "opId");
#else
  strcpy(link.operation_id, "opId");
#endif
  link.description = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(link.description, 32, "ldesc");
#else
  strcpy(link.description, "ldesc");
#endif

  lp.name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(lp.name, 32, "lp1");
#else
  strcpy(lp.name, "lp1");
#endif
  lp.value.type = OA_ANY_STRING;
  lp.value.string = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(lp.value.string, 32, "val_str");
#else
  strcpy(lp.value.string, "val_str");
#endif

  link.parameters =
      (struct OpenAPI_LinkParam *)malloc(sizeof(struct OpenAPI_LinkParam) * 2);
  link.parameters[0] = lp;
  memset(&link.parameters[1], 0, sizeof(struct OpenAPI_LinkParam));
  link.parameters[1].name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(link.parameters[1].name, 32, "lp2");
#else
  strcpy(link.parameters[1].name, "lp2");
#endif
  link.parameters[1].value.type = OA_ANY_JSON;
  link.parameters[1].value.json = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(link.parameters[1].value.json, 32, "{}");
#else
  strcpy(link.parameters[1].value.json, "{}");
#endif
  link.n_parameters = 2;

  link.request_body_set = 1;
  link.request_body.type = OA_ANY_STRING;
  link.request_body.string = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(link.request_body.string, 32, "req_body_str");
#else
  strcpy(link.request_body.string, "req_body_str");
#endif

  link.server_set = 1;
  link.server = (struct OpenAPI_Server *)malloc(sizeof(struct OpenAPI_Server));
  memset(link.server, 0, sizeof(struct OpenAPI_Server));
  link.server->url = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(link.server->url, 32, "http");
#else
  strcpy(link.server->url, "http");
#endif
  link.server->name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(link.server->name, 32, "srv");
#else
  strcpy(link.server->name, "srv");
#endif
  link.server->description = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(link.server->description, 32, "sdesc");
#else
  strcpy(link.server->description, "sdesc");
#endif

  resp.links = (struct OpenAPI_Link *)malloc(sizeof(struct OpenAPI_Link) * 2);
  resp.links[0] = link;
  memset(&resp.links[1], 0, sizeof(struct OpenAPI_Link));
  resp.links[1].name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.links[1].name, 32, "L2");
#else
  strcpy(resp.links[1].name, "L2");
#endif
  resp.links[1].request_body_set = 1;
  resp.links[1].request_body.type = OA_ANY_JSON;
  resp.links[1].request_body.json = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(resp.links[1].request_body.json, 32, "{}");
#else
  strcpy(resp.links[1].request_body.json, "{}");
#endif
  resp.n_links = 2;

  op.responses =
      (struct OpenAPI_Response *)malloc(sizeof(struct OpenAPI_Response) * 2);
  op.responses[0] = resp;
  memset(&op.responses[1], 0, sizeof(struct OpenAPI_Response));
  op.responses[1].code = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.responses[1].code, 32, "default");
#else
  strcpy(op.responses[1].code, "default");
#endif
  op.responses[1].example.type = OA_ANY_JSON;
  op.responses[1].example.json = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.responses[1].example.json, 32, "{}");
#else
  strcpy(op.responses[1].example.json, "{}");
#endif
  op.responses[1].headers =
      (struct OpenAPI_Header *)malloc(sizeof(struct OpenAPI_Header));
  memset(&op.responses[1].headers[0], 0, sizeof(struct OpenAPI_Header));
  op.responses[1].headers[0].name = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.responses[1].headers[0].name, 32, "X-Hdr2");
#else
  strcpy(op.responses[1].headers[0].name, "X-Hdr2");
#endif
  op.responses[1].headers[0].example.type = OA_ANY_JSON;
  op.responses[1].headers[0].example.json = (char *)malloc(32);
#if defined(_MSC_VER)
  strcpy_s(op.responses[1].headers[0].example.json, 32, "{}");
#else
  strcpy(op.responses[1].headers[0].example.json, "{}");
#endif
  op.responses[1].n_headers = 1;
  op.n_responses = 2;

  /* find_response_media_type null cases */
  find_response_media_type(NULL, "app/json", &found_mt);
  ASSERT_EQ(NULL, found_mt);
  find_response_media_type(&op.responses[0], NULL, &found_mt);
  ASSERT_EQ(NULL, found_mt);
  find_response_media_type(&op.responses[1], "app/json", &found_mt);
  ASSERT_EQ(NULL, found_mt);
  find_response_media_type(&op.responses[0], "nonexistent", &found_mt);
  ASSERT_EQ(NULL, found_mt);

  reset_op(&op);
  PASS();
}

SUITE(c2openapi_op_coverage_suite) { RUN_TEST(test_reset_op_coverage); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C2OPENAPI_OP_COVERAGE_H */
