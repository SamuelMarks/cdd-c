#ifdef _MSC_VER
#ifndef strdup
#define strdup _strdup
#endif
#endif
/**
 * @file test_c2openapi_op.h
 * @brief Unit tests for the Operation Builder.
 */

#ifndef TEST_C2OPENAPI_OP_H
#define TEST_C2OPENAPI_OP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/parse/inspector.h"
#include "docstrings/parse/doc.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/operation.h"
/* clang-format on */

/* --- Helpers --- */

static void reset_op(struct OpenAPI_Operation *op) {
  size_t i;
  /* Use openapi_spec_free or manual free?
     The op inside spec relies on arrays. Loader's free logic is complex.
     We use a simplified free here since we only populate one op.
  */
  /* Call module internal logic if available or reimplement basic cleanup */
  /* Reimplementing basics for test safety */
  if (op->operation_id)
    free(op->operation_id);
  if (op->summary)
    free(op->summary);
  if (op->description)
    free(op->description);
  if (op->method)
    free(op->method);
  if (op->parameters) {
    for (i = 0; i < op->n_parameters; i++) {
      free(op->parameters[i].name);
      /* op->parameters[i].in is an enum, nothing to free */
      free(op->parameters[i].type);
      if (op->parameters[i].description)
        free(op->parameters[i].description);
      if (op->parameters[i].content_type)
        free(op->parameters[i].content_type);
      if (op->parameters[i].items_type)
        free(op->parameters[i].items_type);
      if (op->parameters[i].example.type == OA_ANY_STRING &&
          op->parameters[i].example.string)
        free(op->parameters[i].example.string);
      if (op->parameters[i].example.type == OA_ANY_JSON &&
          op->parameters[i].example.json)
        free(op->parameters[i].example.json);
      if (op->parameters[i].schema.ref_name)
        free(op->parameters[i].schema.ref_name);
      if (op->parameters[i].schema.ref)
        free(op->parameters[i].schema.ref);
      if (op->parameters[i].schema.inline_type)
        free(op->parameters[i].schema.inline_type);
      if (op->parameters[i].schema.items_ref)
        free(op->parameters[i].schema.items_ref);
      if (op->parameters[i].schema.format)
        free(op->parameters[i].schema.format);
      if (op->parameters[i].schema.items_format)
        free(op->parameters[i].schema.items_format);
      if (op->parameters[i].schema.content_media_type)
        free(op->parameters[i].schema.content_media_type);
      if (op->parameters[i].schema.content_encoding)
        free(op->parameters[i].schema.content_encoding);
      if (op->parameters[i].schema.items_content_media_type)
        free(op->parameters[i].schema.items_content_media_type);
      if (op->parameters[i].schema.items_content_encoding)
        free(op->parameters[i].schema.items_content_encoding);
    }
    free(op->parameters);
  }
  if (op->tags) {
    if (op->n_tags > 0) {

      for (i = 0; i < op->n_tags; ++i) {
        if (op->tags[i])
          free(op->tags[i]);
      }
    }
    free(op->tags);
  }
  if (op->external_docs.url)
    free(op->external_docs.url);
  if (op->external_docs.description)
    free(op->external_docs.description);
  if (op->req_body.ref_name)
    free(op->req_body.ref_name);
  if (op->req_body.inline_type)
    free(op->req_body.inline_type);
  if (op->req_body.content_type)
    free(op->req_body.content_type);
  if (op->req_body_media_types) {
    for (i = 0; i < op->n_req_body_media_types; ++i) {
      struct OpenAPI_MediaType *mt = &op->req_body_media_types[i];
      if (mt->name)
        free(mt->name);
      if (mt->ref)
        free(mt->ref);
      if (mt->extensions_json)
        free(mt->extensions_json);
      if (mt->schema.ref_name)
        free(mt->schema.ref_name);
      if (mt->schema.ref)
        free(mt->schema.ref);
      if (mt->schema.inline_type)
        free(mt->schema.inline_type);
      if (mt->schema.items_ref)
        free(mt->schema.items_ref);
      if (mt->schema.format)
        free(mt->schema.format);
      if (mt->schema.items_format)
        free(mt->schema.items_format);
      if (mt->schema.content_media_type)
        free(mt->schema.content_media_type);
      if (mt->schema.content_encoding)
        free(mt->schema.content_encoding);
      if (mt->schema.items_content_media_type)
        free(mt->schema.items_content_media_type);
      if (mt->schema.items_content_encoding)
        free(mt->schema.items_content_encoding);
      if (mt->example.type == OA_ANY_STRING && mt->example.string)
        free(mt->example.string);
      if (mt->example.type == OA_ANY_JSON && mt->example.json)
        free(mt->example.json);
    }
    free(op->req_body_media_types);
  }
  if (op->req_body_description)
    free(op->req_body_description);
  if (op->req_body_extensions_json)
    free(op->req_body_extensions_json);
  if (op->req_body_ref)
    free(op->req_body_ref);
  if (op->req_body.example.type == OA_ANY_STRING && op->req_body.example.string)
    free(op->req_body.example.string);
  if (op->req_body.example.type == OA_ANY_JSON && op->req_body.example.json)
    free(op->req_body.example.json);
  if (op->responses) {
    for (i = 0; i < op->n_responses; i++) {
      free(op->responses[i].code);
      if (op->responses[i].summary)
        free(op->responses[i].summary);
      if (op->responses[i].description)
        free(op->responses[i].description);
      if (op->responses[i].content_type)
        free(op->responses[i].content_type);
      if (op->responses[i].example.type == OA_ANY_STRING &&
          op->responses[i].example.string)
        free(op->responses[i].example.string);
      if (op->responses[i].example.type == OA_ANY_JSON &&
          op->responses[i].example.json)
        free(op->responses[i].example.json);
      if (op->responses[i].schema.ref_name)
        free(op->responses[i].schema.ref_name);
      if (op->responses[i].schema.inline_type)
        free(op->responses[i].schema.inline_type);
      if (op->responses[i].content_media_types) {
        size_t j;
        for (j = 0; j < op->responses[i].n_content_media_types; ++j) {
          struct OpenAPI_MediaType *mt =
              &op->responses[i].content_media_types[j];
          if (mt->name)
            free(mt->name);
          if (mt->ref)
            free(mt->ref);
          if (mt->extensions_json)
            free(mt->extensions_json);
          if (mt->schema.ref_name)
            free(mt->schema.ref_name);
          if (mt->schema.ref)
            free(mt->schema.ref);
          if (mt->schema.inline_type)
            free(mt->schema.inline_type);
          if (mt->schema.items_ref)
            free(mt->schema.items_ref);
          if (mt->schema.format)
            free(mt->schema.format);
          if (mt->schema.items_format)
            free(mt->schema.items_format);
          if (mt->schema.content_media_type)
            free(mt->schema.content_media_type);
          if (mt->schema.content_encoding)
            free(mt->schema.content_encoding);
          if (mt->schema.items_content_media_type)
            free(mt->schema.items_content_media_type);
          if (mt->schema.items_content_encoding)
            free(mt->schema.items_content_encoding);
          if (mt->example.type == OA_ANY_STRING && mt->example.string)
            free(mt->example.string);
          if (mt->example.type == OA_ANY_JSON && mt->example.json)
            free(mt->example.json);
        }
        free(op->responses[i].content_media_types);
      }
      if (op->responses[i].headers) {
        size_t h;
        for (h = 0; h < op->responses[i].n_headers; ++h) {
          struct OpenAPI_Header *hdr = &op->responses[i].headers[h];
          free(hdr->name);
          if (hdr->ref)
            free(hdr->ref);
          if (hdr->description)
            free(hdr->description);
          if (hdr->content_type)
            free(hdr->content_type);
          if (hdr->content_ref)
            free(hdr->content_ref);
          if (hdr->type)
            free(hdr->type);
          if (hdr->items_type)
            free(hdr->items_type);
          if (hdr->schema.ref_name)
            free(hdr->schema.ref_name);
          if (hdr->schema.ref)
            free(hdr->schema.ref);
          if (hdr->schema.inline_type)
            free(hdr->schema.inline_type);
          if (hdr->schema.items_ref)
            free(hdr->schema.items_ref);
          if (hdr->schema.format)
            free(hdr->schema.format);
          if (hdr->schema.items_format)
            free(hdr->schema.items_format);
          if (hdr->schema.content_media_type)
            free(hdr->schema.content_media_type);
          if (hdr->schema.content_encoding)
            free(hdr->schema.content_encoding);
          if (hdr->schema.items_content_media_type)
            free(hdr->schema.items_content_media_type);
          if (hdr->schema.items_content_encoding)
            free(hdr->schema.items_content_encoding);
          if (hdr->example.type == OA_ANY_STRING && hdr->example.string)
            free(hdr->example.string);
          if (hdr->example.type == OA_ANY_JSON && hdr->example.json)
            free(hdr->example.json);
        }
        free(op->responses[i].headers);
      }
      if (op->responses[i].links) {
        size_t l;
        for (l = 0; l < op->responses[i].n_links; ++l) {
          struct OpenAPI_Link *link = &op->responses[i].links[l];
          size_t p;
          free(link->name);
          if (link->ref)
            free(link->ref);
          if (link->summary)
            free(link->summary);
          if (link->description)
            free(link->description);
          if (link->operation_ref)
            free(link->operation_ref);
          if (link->operation_id)
            free(link->operation_id);
          if (link->parameters) {
            for (p = 0; p < link->n_parameters; ++p) {
              struct OpenAPI_LinkParam *lp = &link->parameters[p];
              free(lp->name);
              if (lp->value.type == OA_ANY_STRING && lp->value.string)
                free(lp->value.string);
              if (lp->value.type == OA_ANY_JSON && lp->value.json)
                free(lp->value.json);
            }
            free(link->parameters);
          }
          if (link->request_body_set) {
            if (link->request_body.type == OA_ANY_STRING &&
                link->request_body.string)
              free(link->request_body.string);
            if (link->request_body.type == OA_ANY_JSON &&
                link->request_body.json)
              free(link->request_body.json);
          }
          if (link->server_set && link->server) {
            if (link->server->url)
              free(link->server->url);
            if (link->server->name)
              free(link->server->name);
            if (link->server->description)
              free(link->server->description);
            free(link->server);
          }
        }
        free(op->responses[i].links);
      }
    }
    free(op->responses);
  }
  if (op->security) {

    for (i = 0; i < op->n_security; ++i) {
      struct OpenAPI_SecurityRequirementSet *set = &op->security[i];
      if (set->requirements) {
        size_t r;
        for (r = 0; r < set->n_requirements; ++r) {
          size_t s;
          free(set->requirements[r].scheme);
          if (set->requirements[r].scopes) {
            for (s = 0; s < set->requirements[r].n_scopes; ++s) {
              free(set->requirements[r].scopes[s]);
            }
            free(set->requirements[r].scopes);
          }
        }
        free(set->requirements);
      }
    }
    free(op->security);
  }
  if (op->servers) {

    for (i = 0; i < op->n_servers; ++i) {
      size_t v;
      free(op->servers[i].url);
      free(op->servers[i].name);
      free(op->servers[i].description);
      if (op->servers[i].variables) {
        for (v = 0; v < op->servers[i].n_variables; ++v) {
          size_t e;
          struct OpenAPI_ServerVariable *var = &op->servers[i].variables[v];
          free(var->name);
          free(var->default_value);
          free(var->description);
          if (var->enum_values) {
            for (e = 0; e < var->n_enum_values; ++e) {
              free(var->enum_values[e]);
            }
            free(var->enum_values);
          }
        }
        free(op->servers[i].variables);
      }
    }
    free(op->servers);
  }
  memset(op, 0, sizeof(*op));
}

static cdd_c_error_t
find_response_media_type(const struct OpenAPI_Response *resp, const char *name,
                         const struct OpenAPI_MediaType **_out_val) {
  size_t i;
  if (!resp || !name || !resp->content_media_types) {
    *_out_val = NULL;
    return 0;
  }
  for (i = 0; i < resp->n_content_media_types; ++i) {
    const struct OpenAPI_MediaType *mt = &resp->content_media_types[i];
    if (mt->name && strcmp(mt->name, name) == 0) {
      *_out_val = mt;
      return 0;
    }
  }
  {
    *_out_val = NULL;
    return 0;
  }
}

/* --- Tests --- */

TEST test_build_simple_get(void) {
  /*
   * Case: int api_user_get(int id);
   * Doc: @route GET /user/{id}
   */
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int rc;

  /* Setup Signature */
  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";

  /* Setup Doc */
  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/user/{id}");
  doc.verb = strdup("GET");
  doc.summary = (char *)(size_t)(size_t) "Get a user";

  /* Setup Context */
  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  /* Verify Basic */
  ASSERT_EQ(OA_VERB_GET, op.verb);
  ASSERT_STR_EQ("api_user_get", op.operation_id);
  ASSERT_STR_EQ("Get a user", op.summary);

  /* Verify Parameter */
  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("id", op.parameters[0].name);
  ASSERT_EQ(OA_PARAM_IN_PATH, op.parameters[0].in);
  ASSERT(op.parameters[0].required);
  ASSERT_STR_EQ("integer", op.parameters[0].type);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_param_format_from_mapping(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "long";

  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/user/{id}");
  doc.verb = strdup("GET");

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT(op.parameters[0].schema_set);
  ASSERT_STR_EQ("integer", op.parameters[0].schema.inline_type);
  ASSERT_STR_EQ("int64", op.parameters[0].schema.format);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_param_format_override(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam *params = NULL;
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/user/{id}");
  doc.verb = strdup("GET");
  params = (struct DocParam *)calloc(1, sizeof(struct DocParam));
  ASSERT(params != NULL);
  params[0].name = strdup("id");
  params[0].format = strdup("int64");
  doc.params = params;
  doc.n_params = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT(op.parameters[0].schema_set);
  ASSERT_STR_EQ("int64", op.parameters[0].schema.format);

  reset_op(&op);
  doc_metadata_free(&doc);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_response_header_format(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocResponseHeader *headers = NULL;
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_ping";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/ping");
  doc.verb = strdup("GET");
  headers = (struct DocResponseHeader *)calloc(1, sizeof(*headers));
  ASSERT(headers != NULL);
  headers[0].code = strdup("200");
  headers[0].name = strdup("X-Rate");
  headers[0].type = strdup("integer");
  headers[0].format = strdup("int64");
  headers[0].description = strdup("Rate limit");
  doc.response_headers = headers;
  doc.n_response_headers = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_EQ(1, op.responses[0].n_headers);
  ASSERT(op.responses[0].headers[0].schema_set);
  ASSERT_STR_EQ("int64", op.responses[0].headers[0].schema.format);

  reset_op(&op);
  doc_metadata_free(&doc);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_default_response_when_missing(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_ping";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/ping");
  doc.verb = strdup("GET");

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("200", op.responses[0].code);
  ASSERT_STR_EQ("Success", op.responses[0].description);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_operation_id_override(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/user/{id}");
  doc.verb = strdup("GET");
  doc.operation_id = (char *)(size_t)(size_t) "getUserById";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("getUserById", op.operation_id);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_param_content_type(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam params[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(params, 0, sizeof(params));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_search";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "payload";
  args[0].type = (char *)(size_t)(size_t) "const char *";

  memset(&doc, 0, sizeof(doc));
  memset(params, 0, sizeof(params));
  doc.route = strdup("/user/search");
  doc.verb = strdup("GET");
  doc.params = params;
  doc.n_params = 1;
  doc.params[0].name = (char *)(size_t)(size_t) "payload";
  doc.params[0].in_loc = (char *)(size_t)(size_t) "query";
  doc.params[0].content_type = (char *)(size_t)(size_t) "application/json";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("application/json", op.parameters[0].content_type);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_param_example(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam params[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(params, 0, sizeof(params));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  memset(params, 0, sizeof(params));
  doc.route = strdup("/user/{id}");
  doc.verb = strdup("GET");
  doc.params = params;
  doc.n_params = 1;
  doc.params[0].name = (char *)(size_t)(size_t) "id";
  doc.params[0].in_loc = (char *)(size_t)(size_t) "path";
  doc.params[0].example = (char *)(size_t)(size_t) "123";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_parameters);
  ASSERT(op.parameters[0].example_set);
  ASSERT_EQ(OA_ANY_NUMBER, op.parameters[0].example.type);
  ASSERT_EQ(OA_EXAMPLE_LOC_OBJECT, op.parameters[0].example_location);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_return_content_type(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocResponse returns[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(returns, 0, sizeof(returns));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_status";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  memset(returns, 0, sizeof(returns));
  doc.route = (char *)(size_t)(size_t) "/status";
  doc.verb = strdup("GET");
  doc.returns = returns;
  doc.n_returns = 1;
  doc.returns[0].code = (char *)(size_t)(size_t) "200";
  doc.returns[0].summary = (char *)(size_t)(size_t) "Status";
  doc.returns[0].description = (char *)(size_t)(size_t) "OK";
  doc.returns[0].content_type = (char *)(size_t)(size_t) "text/plain";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("Status", op.responses[0].summary);
  ASSERT_STR_EQ("text/plain", op.responses[0].content_type);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_response_example(void) {
  /* */
  /* */

  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocResponse returns[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(returns, 0, sizeof(returns));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  memset(returns, 0, sizeof(returns));
  doc.route = strdup("/user/{id}");
  doc.verb = strdup("GET");
  doc.returns = returns;
  doc.n_returns = 1;
  doc.returns[0].code = (char *)(size_t)(size_t) "200";
  doc.returns[0].description = (char *)(size_t)(size_t) "OK";
  doc.returns[0].content_type = (char *)(size_t)(size_t) "application/json";
  doc.returns[0].example = (char *)(size_t)(size_t) "{\"ok\":true}";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_EQ(1, op.responses[0].n_content_media_types);
  ASSERT(op.responses[0].content_media_types[0].example_set);
  ASSERT_EQ(OA_ANY_JSON, op.responses[0].content_media_types[0].example.type);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_post_with_body(void) {
  /*
   * Case: int api_pet_create(const struct Pet *p);
   * Implicit POST from name.
   */
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct OpenAPI_Operation op;
  int rc;

  /* Sig */
  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_pet_create";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "p";
  args[0].type = (char *)(size_t)(size_t) "const struct Pet *";

  /* Doc (minimal) */
  ctx.sig = &sig;
  ctx.doc = NULL; /* No explicit doc to test implicit logic */
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  /* Implicit Verb */
  ASSERT_EQ(OA_VERB_POST, op.verb);

  /* Parameter becomes Body */
  ASSERT_EQ(0, op.n_parameters); /* Should NOT be a parameter */
  ASSERT_STR_EQ("Pet", op.req_body.ref_name);
  ASSERT_STR_EQ("application/json", op.req_body.content_type);
  ASSERT_EQ(1, op.req_body_required_set);
  ASSERT_EQ(1, op.req_body_required);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_params_explicit(void) {
  /*
   * Case: int list(int limit);
   * Doc: @param limit [in:query]
   */
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op;

  /* Sig */
  memset(args, 0, sizeof(args));
  memset(dparams, 0, sizeof(dparams));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "list";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "limit";
  args[0].type = (char *)(size_t)(size_t) "int";

  /* Doc */
  memset(&doc, 0, sizeof(doc));
  doc.params = dparams;
  doc.n_params = 1;
  dparams[0].name = (char *)(size_t)(size_t) "limit";
  dparams[0].in_loc = (char *)(size_t)(size_t) "query";
  dparams[0].description = (char *)(size_t)(size_t) "Max items";
  dparams[0].required = 0;

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  c2openapi_build_operation(&ctx, &op);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("limit", op.parameters[0].name);
  ASSERT_EQ(OA_PARAM_IN_QUERY, op.parameters[0].in);
  /* Default required for query is 0 unless specified */
  ASSERT_EQ(0, op.parameters[0].required);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_param_style_flags(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(dparams, 0, sizeof(dparams));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "search";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "tags";
  args[0].type = (char *)(size_t)(size_t) "char **";

  memset(&doc, 0, sizeof(doc));
  memset(dparams, 0, sizeof(dparams));
  doc.params = dparams;
  doc.n_params = 1;
  dparams[0].name = (char *)(size_t)(size_t) "tags";
  dparams[0].in_loc = (char *)(size_t)(size_t) "query";
  dparams[0].style = DOC_PARAM_STYLE_SPACE_DELIMITED;
  dparams[0].style_set = 1;
  dparams[0].explode = 0;
  dparams[0].explode_set = 1;
  dparams[0].allow_reserved = 1;
  dparams[0].allow_reserved_set = 1;
  dparams[0].allow_empty_value = 1;
  dparams[0].allow_empty_value_set = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT_EQ(OA_STYLE_SPACE_DELIMITED, op.parameters[0].style);
  ASSERT_EQ(1, op.parameters[0].explode_set);
  ASSERT_EQ(0, op.parameters[0].explode);
  ASSERT_EQ(1, op.parameters[0].allow_reserved_set);
  ASSERT_EQ(1, op.parameters[0].allow_reserved);
  ASSERT_EQ(1, op.parameters[0].allow_empty_value_set);
  ASSERT_EQ(1, op.parameters[0].allow_empty_value);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_param_default_styles(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[2];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(dparams, 0, sizeof(dparams));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "get_item";
  sig.n_args = 2;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";
  args[1].name = (char *)(size_t)(size_t) "token";
  args[1].type = (char *)(size_t)(size_t) "char *";

  memset(&doc, 0, sizeof(doc));
  doc.route = (char *)(size_t)(size_t) "/items/{id}";
  doc.params = dparams;
  doc.n_params = 1;
  memset(dparams, 0, sizeof(dparams));
  dparams[0].name = (char *)(size_t)(size_t) "token";
  dparams[0].in_loc = (char *)(size_t)(size_t) "header";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(2, op.n_parameters);
  ASSERT_EQ(OA_PARAM_IN_PATH, op.parameters[0].in);
  ASSERT_EQ(OA_STYLE_SIMPLE, op.parameters[0].style);
  ASSERT_EQ(OA_PARAM_IN_HEADER, op.parameters[1].in);
  ASSERT_EQ(OA_STYLE_SIMPLE, op.parameters[1].style);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_reserved_header_param_ignored(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[2];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(dparams, 0, sizeof(dparams));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "get_item";
  sig.n_args = 2;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";
  args[1].name = (char *)(size_t)(size_t) "Accept";
  args[1].type = (char *)(size_t)(size_t) "char *";

  memset(&doc, 0, sizeof(doc));
  doc.route = (char *)(size_t)(size_t) "/items/{id}";
  doc.params = dparams;
  doc.n_params = 1;
  memset(dparams, 0, sizeof(dparams));
  dparams[0].name = (char *)(size_t)(size_t) "Accept";
  dparams[0].in_loc = (char *)(size_t)(size_t) "header";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("id", op.parameters[0].name);
  ASSERT_EQ(OA_PARAM_IN_PATH, op.parameters[0].in);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_with_tags_description_and_deprecated(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_list";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "limit";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  doc.summary = (char *)(size_t)(size_t) "List users";
  doc.description = (char *)(size_t)(size_t) "Longer description text";
  doc.deprecated_set = 1;
  doc.deprecated = 1;
  doc.external_docs_url = (char *)(size_t)(size_t) "https://example.com/docs";
  doc.external_docs_description = (char *)(size_t)(size_t) "External docs";
  {
    static char *tags[] = {(char *)(size_t)(size_t) "users",
                           (char *)(size_t)(size_t) "admin"};
    doc.tags = tags;
    doc.n_tags = 2;
  }

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("List users", op.summary);
  ASSERT_STR_EQ("Longer description text", op.description);
  ASSERT_EQ(1, op.deprecated);
  ASSERT_EQ(2, op.n_tags);
  ASSERT_STR_EQ("users", op.tags[0]);
  ASSERT_STR_EQ("admin", op.tags[1]);
  ASSERT_STR_EQ("https://example.com/docs", op.external_docs.url);
  ASSERT_STR_EQ("External docs", op.external_docs.description);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_params_querystring(void) {
  /*
   * Case: int search(const char *qs);
   * Doc: @param qs [in:querystring]
   */
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op;

  memset(args, 0, sizeof(args));
  memset(dparams, 0, sizeof(dparams));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "search";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "qs";
  args[0].type = (char *)(size_t)(size_t) "const char *";

  memset(&doc, 0, sizeof(doc));
  doc.params = dparams;
  doc.n_params = 1;
  dparams[0].name = (char *)(size_t)(size_t) "qs";
  dparams[0].in_loc = (char *)(size_t)(size_t) "querystring";
  dparams[0].description = (char *)(size_t)(size_t) "Serialized query string";

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  c2openapi_build_operation(&ctx, &op);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("qs", op.parameters[0].name);
  ASSERT_EQ(OA_PARAM_IN_QUERYSTRING, op.parameters[0].in);
  ASSERT_STR_EQ("string", op.parameters[0].type);
  ASSERT_STR_EQ("application/x-www-form-urlencoded",
                op.parameters[0].content_type);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_params_querystring_json_struct(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op;

  memset(args, 0, sizeof(args));
  memset(dparams, 0, sizeof(dparams));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "search_query";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "qs";
  args[0].type = (char *)(size_t)(size_t) "struct Query *";

  memset(&doc, 0, sizeof(doc));
  doc.params = dparams;
  doc.n_params = 1;
  dparams[0].name = (char *)(size_t)(size_t) "qs";
  dparams[0].in_loc = (char *)(size_t)(size_t) "querystring";
  dparams[0].content_type = (char *)(size_t)(size_t) "application/json";

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  c2openapi_build_operation(&ctx, &op);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT_EQ(OA_PARAM_IN_QUERYSTRING, op.parameters[0].in);
  ASSERT_STR_EQ("application/json", op.parameters[0].content_type);
  ASSERT(op.parameters[0].schema_set);
  ASSERT_STR_EQ("Query", op.parameters[0].schema.ref_name);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_custom_verb_additional(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "copy_user";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  doc.route = (char *)(size_t)(size_t) "/users/{id}";
  doc.verb = strdup("COPY");

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(OA_VERB_UNKNOWN, op.verb);
  ASSERT_EQ(1, op.is_additional);
  ASSERT_STR_EQ("COPY", op.method);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_response_multi_content(void) {
  const struct OpenAPI_MediaType *_ast_find_response_media_type_0;
  const struct OpenAPI_MediaType *_ast_find_response_media_type_1;
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocResponse resps[2];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(resps, 0, sizeof(resps));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "get_report";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/report");
  doc.verb = strdup("GET");
  doc.returns = resps;
  doc.n_returns = 2;

  resps[0].code = (char *)(size_t)(size_t) "200";
  resps[0].description = (char *)(size_t)(size_t) "OK json";
  resps[0].content_type = (char *)(size_t)(size_t) "application/json";
  resps[1].code = (char *)(size_t)(size_t) "200";
  resps[1].description = (char *)(size_t)(size_t) "OK text";
  resps[1].content_type = (char *)(size_t)(size_t) "text/plain";

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_EQ(2, op.responses[0].n_content_media_types);
  ASSERT((find_response_media_type(&op.responses[0], "application/json",
                                   &_ast_find_response_media_type_0),
          _ast_find_response_media_type_0));
  ASSERT((find_response_media_type(&op.responses[0], "text/plain",
                                   &_ast_find_response_media_type_1),
          _ast_find_response_media_type_1));

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_response_headers(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocResponse resps[1];
  struct DocResponseHeader hdrs[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(resps, 0, sizeof(resps));
  memset(hdrs, 0, sizeof(hdrs));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "get_user";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.returns = resps;
  doc.n_returns = 1;
  resps[0].code = (char *)(size_t)(size_t) "200";
  resps[0].description = (char *)(size_t)(size_t) "OK";

  doc.response_headers = hdrs;
  doc.n_response_headers = 1;
  hdrs[0].code = (char *)(size_t)(size_t) "200";
  hdrs[0].name = (char *)(size_t)(size_t) "X-Request-Id";
  hdrs[0].type = (char *)(size_t)(size_t) "string";
  hdrs[0].content_type = (char *)(size_t)(size_t) "application/xml";
  hdrs[0].description = (char *)(size_t)(size_t) "Request identifier";
  hdrs[0].example = (char *)(size_t)(size_t) "42";
  hdrs[0].required_set = 1;
  hdrs[0].required = 1;

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("200", op.responses[0].code);
  ASSERT_STR_EQ("OK", op.responses[0].description);
  ASSERT_EQ(1, op.responses[0].n_headers);
  ASSERT_STR_EQ("X-Request-Id", op.responses[0].headers[0].name);
  ASSERT_STR_EQ("string", op.responses[0].headers[0].type);
  ASSERT_STR_EQ("application/xml", op.responses[0].headers[0].content_type);
  ASSERT_STR_EQ("Request identifier", op.responses[0].headers[0].description);
  ASSERT_EQ(1, op.responses[0].headers[0].required);
  ASSERT(op.responses[0].headers[0].example_set);
  ASSERT_EQ(OA_ANY_NUMBER, op.responses[0].headers[0].example.type);
  ASSERT_EQ(42, (int)op.responses[0].headers[0].example.number);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_response_links(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocLink links[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "get_page";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.route = (char *)(size_t)(size_t) "/pages";
  doc.verb = strdup("GET");
  doc.links = links;
  doc.n_links = 1;

  memset(links, 0, sizeof(links));
  links[0].code = (char *)(size_t)(size_t) "200";
  links[0].name = (char *)(size_t)(size_t) "next";
  links[0].operation_id = (char *)(size_t)(size_t) "getNextPage";
  links[0].summary = (char *)(size_t)(size_t) "Next page";
  links[0].description = (char *)(size_t)(size_t) "Fetch next page";
  links[0].parameters_json =
      (char *)(size_t)(size_t) "{\"cursor\":\"$response.body#/next\"}";
  links[0].request_body_json = (char *)(size_t)(size_t) "{\"foo\":1}";
  links[0].server_url = (char *)(size_t)(size_t) "https://example.com";
  links[0].server_name = (char *)(size_t)(size_t) "prod";
  links[0].server_description = (char *)(size_t)(size_t) "Primary server";

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_EQ(1, op.responses[0].n_links);
  ASSERT_STR_EQ("next", op.responses[0].links[0].name);
  ASSERT_STR_EQ("getNextPage", op.responses[0].links[0].operation_id);
  ASSERT_STR_EQ("Next page", op.responses[0].links[0].summary);
  ASSERT_STR_EQ("Fetch next page", op.responses[0].links[0].description);
  ASSERT_EQ(1, op.responses[0].links[0].n_parameters);
  ASSERT_STR_EQ("cursor", op.responses[0].links[0].parameters[0].name);
  ASSERT_EQ(OA_ANY_STRING, op.responses[0].links[0].parameters[0].value.type);
  ASSERT_STR_EQ("$response.body#/next",
                op.responses[0].links[0].parameters[0].value.string);
  ASSERT(op.responses[0].links[0].request_body_set);
  ASSERT_EQ(OA_ANY_JSON, op.responses[0].links[0].request_body.type);
  ASSERT(op.responses[0].links[0].server_set);
  ASSERT(op.responses[0].links[0].server != NULL);
  ASSERT_STR_EQ("https://example.com", op.responses[0].links[0].server->url);
  ASSERT_STR_EQ("prod", op.responses[0].links[0].server->name);
  ASSERT_STR_EQ("Primary server", op.responses[0].links[0].server->description);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_response_output_arg(void) {
  /*
   * Case: int get_obj(struct Obj **out);
   * Heuristic: Double pointer -> Output parameter -> 200 Response
   */
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct OpenAPI_Operation op;

  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "get_obj";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "out";
  args[0].type = (char *)(size_t)(size_t) "struct Obj **";

  ctx.sig = &sig;
  ctx.doc = NULL;
  ctx.func_name = sig.name;

  c2openapi_build_operation(&ctx, &op);

  /* Should skip parameters */
  ASSERT_EQ(0, op.n_parameters);

  /* Check Responses */
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("200", op.responses[0].code);
  ASSERT_STR_EQ("Obj", op.responses[0].schema.ref_name);
  ASSERT_STR_EQ("Success", op.responses[0].description);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_op_security_servers_request_body(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocSecurityRequirement sec[2];
  char *scopes1[] = {(char *)(size_t)(size_t) "write:pets",
                     (char *)(size_t)(size_t) "read:pets"};
  struct DocServer servers[1];
  struct DocServerVar server_vars[1];
  char *server_enum[] = {(char *)(size_t)(size_t) "prod",
                         (char *)(size_t)(size_t) "staging"};
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_upload";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "payload";
  args[0].type = (char *)(size_t)(size_t) "const struct Payload *";

  memset(&doc, 0, sizeof(doc));
  doc.verb = strdup("POST");
  doc.route = (char *)(size_t)(size_t) "/upload";
  doc.request_body_description = (char *)(size_t)(size_t) "Upload payload";
  doc.request_body_required_set = 1;
  doc.request_body_required = 0;
  doc.request_body_content_type = (char *)(size_t)(size_t) "application/xml";

  memset(sec, 0, sizeof(sec));
  sec[0].scheme = (char *)(size_t)(size_t) "api_key";
  sec[1].scheme = (char *)(size_t)(size_t) "petstore_auth";
  sec[1].scopes = scopes1;
  sec[1].n_scopes = 2;
  doc.security = sec;
  doc.n_security = 2;

  memset(servers, 0, sizeof(servers));
  servers[0].url = (char *)(size_t)(size_t) "https://api.example.com";
  servers[0].name = (char *)(size_t)(size_t) "prod";
  servers[0].description = (char *)(size_t)(size_t) "Production API";
  memset(server_vars, 0, sizeof(server_vars));
  server_vars[0].name = (char *)(size_t)(size_t) "env";
  server_vars[0].default_value = (char *)(size_t)(size_t) "prod";
  server_vars[0].enum_values = server_enum;
  server_vars[0].n_enum_values = 2;
  servers[0].variables = server_vars;
  servers[0].n_variables = 1;
  doc.servers = servers;
  doc.n_servers = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("Upload payload", op.req_body_description);
  ASSERT_EQ(1, op.req_body_required_set);
  ASSERT_EQ(0, op.req_body_required);
  ASSERT_STR_EQ("application/xml", op.req_body.content_type);

  ASSERT_EQ(1, op.security_set);
  ASSERT_EQ(2, op.n_security);
  ASSERT_STR_EQ("api_key", op.security[0].requirements[0].scheme);
  ASSERT_EQ(0, op.security[0].requirements[0].n_scopes);
  ASSERT_STR_EQ("petstore_auth", op.security[1].requirements[0].scheme);
  ASSERT_EQ(2, op.security[1].requirements[0].n_scopes);
  ASSERT_STR_EQ("write:pets", op.security[1].requirements[0].scopes[0]);
  ASSERT_STR_EQ("read:pets", op.security[1].requirements[0].scopes[1]);

  ASSERT_EQ(1, op.n_servers);
  ASSERT_STR_EQ("https://api.example.com", op.servers[0].url);
  ASSERT_STR_EQ("prod", op.servers[0].name);
  ASSERT_STR_EQ("Production API", op.servers[0].description);
  ASSERT_EQ(1, op.servers[0].n_variables);
  ASSERT_STR_EQ("env", op.servers[0].variables[0].name);
  ASSERT_STR_EQ("prod", op.servers[0].variables[0].default_value);
  ASSERT_EQ(2, op.servers[0].variables[0].n_enum_values);
  ASSERT_STR_EQ("prod", op.servers[0].variables[0].enum_values[0]);
  ASSERT_STR_EQ("staging", op.servers[0].variables[0].enum_values[1]);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_op_param_deprecated(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam params[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(params, 0, sizeof(params));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_get_legacy";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "legacyId";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  doc.verb = strdup("GET");
  doc.route = (char *)(size_t)(size_t) "/legacy/{legacyId}";
  memset(params, 0, sizeof(params));
  params[0].name = (char *)(size_t)(size_t) "legacyId";
  params[0].in_loc = (char *)(size_t)(size_t) "path";
  params[0].deprecated_set = 1;
  params[0].deprecated = 1;
  doc.params = params;
  doc.n_params = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("legacyId", op.parameters[0].name);
  ASSERT_EQ(1, op.parameters[0].deprecated_set);
  ASSERT_EQ(1, op.parameters[0].deprecated);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_request_body_example(void) {
  /* */
  /* */

  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocRequestBody bodies[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_post";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "user";
  args[0].type = (char *)(size_t)(size_t) "struct User *";

  memset(&doc, 0, sizeof(doc));
  doc.verb = strdup("POST");
  doc.route = (char *)(size_t)(size_t) "/user";
  memset(bodies, 0, sizeof(bodies));
  bodies[0].content_type = (char *)(size_t)(size_t) "application/json";
  bodies[0].description = (char *)(size_t)(size_t) "User";
  bodies[0].example = (char *)(size_t)(size_t) "{\"name\":\"x\"}";
  doc.request_bodies = bodies;
  doc.n_request_bodies = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, op.n_req_body_media_types);
  ASSERT(op.req_body_media_types[0].example_set);
  ASSERT_EQ(OA_ANY_JSON, op.req_body_media_types[0].example.type);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_request_body_default_content_type(void) {
  /* */
  /* */

  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocRequestBody bodies[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_post";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "user";
  args[0].type = (char *)(size_t)(size_t) "struct User *";

  memset(&doc, 0, sizeof(doc));
  doc.verb = strdup("POST");
  doc.route = (char *)(size_t)(size_t) "/user";
  memset(bodies, 0, sizeof(bodies));
  bodies[0].description = (char *)(size_t)(size_t) "User";
  doc.request_bodies = bodies;
  doc.n_request_bodies = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("application/json", op.req_body.content_type);
  ASSERT_EQ(1, op.n_req_body_media_types);
  ASSERT_STR_EQ("application/json", op.req_body_media_types[0].name);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_op_request_body_multi_content(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocRequestBody bodies[2];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_upload_multi";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "payload";
  args[0].type = (char *)(size_t)(size_t) "const struct Payload *";

  memset(&doc, 0, sizeof(doc));
  doc.verb = strdup("POST");
  doc.route = (char *)(size_t)(size_t) "/upload";
  memset(bodies, 0, sizeof(bodies));
  bodies[0].content_type = (char *)(size_t)(size_t) "application/json";
  bodies[0].description = (char *)(size_t)(size_t) "JSON body";
  bodies[1].content_type = (char *)(size_t)(size_t) "application/xml";
  bodies[1].description = (char *)(size_t)(size_t) "XML body";
  doc.request_bodies = bodies;
  doc.n_request_bodies = 2;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("application/json", op.req_body.content_type);
  ASSERT_EQ(2, op.n_req_body_media_types);
  ASSERT_STR_EQ("application/json", op.req_body_media_types[0].name);
  ASSERT_STR_EQ("application/xml", op.req_body_media_types[1].name);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

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

SUITE(c2openapi_op_suite) {
  RUN_TEST(test_build_simple_get);
  RUN_TEST(test_build_param_format_from_mapping);
  RUN_TEST(test_build_param_format_override);
  RUN_TEST(test_build_response_header_format);
  RUN_TEST(test_build_default_response_when_missing);
  RUN_TEST(test_build_operation_id_override);
  RUN_TEST(test_build_param_content_type);
  RUN_TEST(test_build_param_example);
  RUN_TEST(test_build_return_content_type);
  RUN_TEST(test_build_response_example);
  RUN_TEST(test_build_post_with_body);
  RUN_TEST(test_build_params_explicit);
  RUN_TEST(test_build_param_default_styles);
  RUN_TEST(test_build_reserved_header_param_ignored);
  RUN_TEST(test_build_with_tags_description_and_deprecated);
  RUN_TEST(test_build_params_querystring);
  RUN_TEST(test_build_params_querystring_json_struct);
  RUN_TEST(test_build_custom_verb_additional);
  RUN_TEST(test_build_response_multi_content);
  RUN_TEST(test_build_response_headers);
  RUN_TEST(test_build_response_links);
  RUN_TEST(test_build_response_output_arg);
  RUN_TEST(test_build_op_security_servers_request_body);
  RUN_TEST(test_build_op_param_deprecated);
  RUN_TEST(test_build_param_style_flags);
  RUN_TEST(test_build_request_body_example);
  RUN_TEST(test_build_request_body_default_content_type);
  RUN_TEST(test_build_op_request_body_multi_content);
  RUN_TEST(test_reset_op_coverage);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C2OPENAPI_OP_H */
