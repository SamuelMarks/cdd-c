/**
 * @file test_c2openapi_op.h
 * @brief Unit tests for the Operation Builder.
 *
 * Verifies that the builder correctly:
 * - Identifies HTTP verbs from documentation or naming conventions.
 * - Categorizes parameters as Path, Query, Header, Body, or Response.
 * - Extracts types using the C mapper.
 *
 * @author Samuel Marks
 */

#ifndef TEST_C2OPENAPI_OP_H
#define TEST_C2OPENAPI_OP_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c2openapi_operation.h"
#include "c_inspector.h"
#include "doc_parser.h"
#include "openapi_loader.h"

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
      size_t i;
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
    size_t i;
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
    size_t i;
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

static const struct OpenAPI_MediaType *
find_response_media_type(const struct OpenAPI_Response *resp,
                         const char *name) {
  size_t i;
  if (!resp || !name || !resp->content_media_types)
    return NULL;
  for (i = 0; i < resp->n_content_media_types; ++i) {
    const struct OpenAPI_MediaType *mt = &resp->content_media_types[i];
    if (mt->name && strcmp(mt->name, name) == 0)
      return mt;
  }
  return NULL;
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
  struct OpenAPI_Operation op = {0};
  int rc;

  /* Setup Signature */
  sig.name = "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "id";
  args[0].type = "int";

  /* Setup Doc */
  memset(&doc, 0, sizeof(doc));
  doc.route = "/user/{id}";
  doc.verb = "GET";
  doc.summary = "Get a user";

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

  reset_op(&op);
  PASS();
}

TEST test_build_param_format_from_mapping(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "id";
  args[0].type = "long";

  memset(&doc, 0, sizeof(doc));
  doc.route = "/user/{id}";
  doc.verb = "GET";

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

  reset_op(&op);
  PASS();
}

TEST test_build_param_format_override(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam *params = NULL;
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "id";
  args[0].type = "int";

  memset(&doc, 0, sizeof(doc));
  doc.route = "/user/{id}";
  doc.verb = "GET";
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
  PASS();
}

TEST test_build_response_header_format(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocResponseHeader *headers = NULL;
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_ping";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.route = "/ping";
  doc.verb = "GET";
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
  PASS();
}

TEST test_build_default_response_when_missing(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_ping";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.route = "/ping";
  doc.verb = "GET";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("200", op.responses[0].code);
  ASSERT_STR_EQ("Success", op.responses[0].description);

  reset_op(&op);
  PASS();
}

TEST test_build_operation_id_override(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "id";
  args[0].type = "int";

  memset(&doc, 0, sizeof(doc));
  doc.route = "/user/{id}";
  doc.verb = "GET";
  doc.operation_id = "getUserById";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("getUserById", op.operation_id);

  reset_op(&op);
  PASS();
}

TEST test_build_param_content_type(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam params[1];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_user_search";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "payload";
  args[0].type = "const char *";

  memset(&doc, 0, sizeof(doc));
  memset(params, 0, sizeof(params));
  doc.route = "/user/search";
  doc.verb = "GET";
  doc.params = params;
  doc.n_params = 1;
  doc.params[0].name = "payload";
  doc.params[0].in_loc = "query";
  doc.params[0].content_type = "application/json";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("application/json", op.parameters[0].content_type);

  reset_op(&op);
  PASS();
}

TEST test_build_param_example(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam params[1];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "id";
  args[0].type = "int";

  memset(&doc, 0, sizeof(doc));
  memset(params, 0, sizeof(params));
  doc.route = "/user/{id}";
  doc.verb = "GET";
  doc.params = params;
  doc.n_params = 1;
  doc.params[0].name = "id";
  doc.params[0].in_loc = "path";
  doc.params[0].example = "123";

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

  reset_op(&op);
  PASS();
}

TEST test_build_return_content_type(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocResponse returns[1];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_status";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  memset(returns, 0, sizeof(returns));
  doc.route = "/status";
  doc.verb = "GET";
  doc.returns = returns;
  doc.n_returns = 1;
  doc.returns[0].code = "200";
  doc.returns[0].summary = "Status";
  doc.returns[0].description = "OK";
  doc.returns[0].content_type = "text/plain";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("Status", op.responses[0].summary);
  ASSERT_STR_EQ("text/plain", op.responses[0].content_type);

  reset_op(&op);
  PASS();
}

TEST test_build_response_example(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocResponse returns[1];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "id";
  args[0].type = "int";

  memset(&doc, 0, sizeof(doc));
  memset(returns, 0, sizeof(returns));
  doc.route = "/user/{id}";
  doc.verb = "GET";
  doc.returns = returns;
  doc.n_returns = 1;
  doc.returns[0].code = "200";
  doc.returns[0].description = "OK";
  doc.returns[0].content_type = "application/json";
  doc.returns[0].example = "{\"ok\":true}";

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

  reset_op(&op);
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
  struct OpenAPI_Operation op = {0};
  int rc;

  /* Sig */
  sig.name = "api_pet_create";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "p";
  args[0].type = "const struct Pet *";

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
  struct OpenAPI_Operation op = {0};

  /* Sig */
  sig.name = "list";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "limit";
  args[0].type = "int";

  /* Doc */
  memset(&doc, 0, sizeof(doc));
  doc.params = dparams;
  doc.n_params = 1;
  dparams[0].name = "limit";
  dparams[0].in_loc = "query";
  dparams[0].description = "Max items";
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
  PASS();
}

TEST test_build_param_style_flags(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "search";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "tags";
  args[0].type = "char **";

  memset(&doc, 0, sizeof(doc));
  memset(dparams, 0, sizeof(dparams));
  doc.params = dparams;
  doc.n_params = 1;
  dparams[0].name = "tags";
  dparams[0].in_loc = "query";
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
  PASS();
}

TEST test_build_param_default_styles(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[2];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "get_item";
  sig.n_args = 2;
  sig.args = args;
  args[0].name = "id";
  args[0].type = "int";
  args[1].name = "token";
  args[1].type = "char *";

  memset(&doc, 0, sizeof(doc));
  doc.route = "/items/{id}";
  doc.params = dparams;
  doc.n_params = 1;
  memset(dparams, 0, sizeof(dparams));
  dparams[0].name = "token";
  dparams[0].in_loc = "header";

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
  PASS();
}

TEST test_build_reserved_header_param_ignored(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[2];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "get_item";
  sig.n_args = 2;
  sig.args = args;
  args[0].name = "id";
  args[0].type = "int";
  args[1].name = "Accept";
  args[1].type = "char *";

  memset(&doc, 0, sizeof(doc));
  doc.route = "/items/{id}";
  doc.params = dparams;
  doc.n_params = 1;
  memset(dparams, 0, sizeof(dparams));
  dparams[0].name = "Accept";
  dparams[0].in_loc = "header";

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
  PASS();
}

TEST test_build_with_tags_description_and_deprecated(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_user_list";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "limit";
  args[0].type = "int";

  memset(&doc, 0, sizeof(doc));
  doc.summary = "List users";
  doc.description = "Longer description text";
  doc.deprecated_set = 1;
  doc.deprecated = 1;
  doc.external_docs_url = "https://example.com/docs";
  doc.external_docs_description = "External docs";
  {
    static char *tags[] = {"users", "admin"};
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
  struct OpenAPI_Operation op = {0};

  sig.name = "search";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "qs";
  args[0].type = "const char *";

  memset(&doc, 0, sizeof(doc));
  doc.params = dparams;
  doc.n_params = 1;
  dparams[0].name = "qs";
  dparams[0].in_loc = "querystring";
  dparams[0].description = "Serialized query string";

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
  PASS();
}

TEST test_build_params_querystring_json_struct(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op = {0};

  sig.name = "search_query";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "qs";
  args[0].type = "struct Query *";

  memset(&doc, 0, sizeof(doc));
  doc.params = dparams;
  doc.n_params = 1;
  dparams[0].name = "qs";
  dparams[0].in_loc = "querystring";
  dparams[0].content_type = "application/json";

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
  PASS();
}

TEST test_build_custom_verb_additional(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "copy_user";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "id";
  args[0].type = "int";

  memset(&doc, 0, sizeof(doc));
  doc.route = "/users/{id}";
  doc.verb = "COPY";

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(OA_VERB_UNKNOWN, op.verb);
  ASSERT_EQ(1, op.is_additional);
  ASSERT_STR_EQ("COPY", op.method);

  reset_op(&op);
  PASS();
}

TEST test_build_response_multi_content(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocResponse resps[2];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "get_report";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.route = "/report";
  doc.verb = "GET";
  doc.returns = resps;
  doc.n_returns = 2;

  resps[0].code = "200";
  resps[0].description = "OK json";
  resps[0].content_type = "application/json";
  resps[1].code = "200";
  resps[1].description = "OK text";
  resps[1].content_type = "text/plain";

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_EQ(2, op.responses[0].n_content_media_types);
  ASSERT(find_response_media_type(&op.responses[0], "application/json"));
  ASSERT(find_response_media_type(&op.responses[0], "text/plain"));

  reset_op(&op);
  PASS();
}

TEST test_build_response_headers(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocResponse resps[1];
  struct DocResponseHeader hdrs[1];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "get_user";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.returns = resps;
  doc.n_returns = 1;
  resps[0].code = "200";
  resps[0].description = "OK";

  doc.response_headers = hdrs;
  doc.n_response_headers = 1;
  hdrs[0].code = "200";
  hdrs[0].name = "X-Request-Id";
  hdrs[0].type = "string";
  hdrs[0].content_type = "application/xml";
  hdrs[0].description = "Request identifier";
  hdrs[0].example = "42";
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
  PASS();
}

TEST test_build_response_links(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocLink links[1];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "get_page";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.route = "/pages";
  doc.verb = "GET";
  doc.links = links;
  doc.n_links = 1;

  memset(links, 0, sizeof(links));
  links[0].code = "200";
  links[0].name = "next";
  links[0].operation_id = "getNextPage";
  links[0].summary = "Next page";
  links[0].description = "Fetch next page";
  links[0].parameters_json = "{\"cursor\":\"$response.body#/next\"}";
  links[0].request_body_json = "{\"foo\":1}";
  links[0].server_url = "https://example.com";
  links[0].server_name = "prod";
  links[0].server_description = "Primary server";

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

  reset_op(&op);
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
  struct OpenAPI_Operation op = {0};

  sig.name = "get_obj";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "out";
  args[0].type = "struct Obj **";

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
  PASS();
}

TEST test_build_op_security_servers_request_body(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocSecurityRequirement sec[2];
  char *scopes1[] = {"write:pets", "read:pets"};
  struct DocServer servers[1];
  struct DocServerVar server_vars[1];
  char *server_enum[] = {"prod", "staging"};
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_upload";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "payload";
  args[0].type = "const struct Payload *";

  memset(&doc, 0, sizeof(doc));
  doc.verb = "POST";
  doc.route = "/upload";
  doc.request_body_description = "Upload payload";
  doc.request_body_required_set = 1;
  doc.request_body_required = 0;
  doc.request_body_content_type = "application/xml";

  memset(sec, 0, sizeof(sec));
  sec[0].scheme = "api_key";
  sec[1].scheme = "petstore_auth";
  sec[1].scopes = scopes1;
  sec[1].n_scopes = 2;
  doc.security = sec;
  doc.n_security = 2;

  memset(servers, 0, sizeof(servers));
  servers[0].url = "https://api.example.com";
  servers[0].name = "prod";
  servers[0].description = "Production API";
  memset(server_vars, 0, sizeof(server_vars));
  server_vars[0].name = "env";
  server_vars[0].default_value = "prod";
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

  reset_op(&op);
  PASS();
}

TEST test_build_op_param_deprecated(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam params[1];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_get_legacy";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "legacyId";
  args[0].type = "int";

  memset(&doc, 0, sizeof(doc));
  doc.verb = "GET";
  doc.route = "/legacy/{legacyId}";
  memset(params, 0, sizeof(params));
  params[0].name = "legacyId";
  params[0].in_loc = "path";
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

  reset_op(&op);
  PASS();
}

TEST test_build_request_body_example(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocRequestBody bodies[1];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_user_post";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "user";
  args[0].type = "struct User *";

  memset(&doc, 0, sizeof(doc));
  doc.verb = "POST";
  doc.route = "/user";
  memset(bodies, 0, sizeof(bodies));
  bodies[0].content_type = "application/json";
  bodies[0].description = "User";
  bodies[0].example = "{\"name\":\"x\"}";
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

  reset_op(&op);
  PASS();
}

TEST test_build_request_body_default_content_type(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocRequestBody bodies[1];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_user_post";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "user";
  args[0].type = "struct User *";

  memset(&doc, 0, sizeof(doc));
  doc.verb = "POST";
  doc.route = "/user";
  memset(bodies, 0, sizeof(bodies));
  bodies[0].description = "User";
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

  reset_op(&op);
  PASS();
}

TEST test_build_op_request_body_multi_content(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocRequestBody bodies[2];
  struct OpenAPI_Operation op = {0};
  int rc;

  sig.name = "api_upload_multi";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = "payload";
  args[0].type = "const struct Payload *";

  memset(&doc, 0, sizeof(doc));
  doc.verb = "POST";
  doc.route = "/upload";
  memset(bodies, 0, sizeof(bodies));
  bodies[0].content_type = "application/json";
  bodies[0].description = "JSON body";
  bodies[1].content_type = "application/xml";
  bodies[1].description = "XML body";
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
  RUN_TEST(test_build_request_body_example);
  RUN_TEST(test_build_request_body_default_content_type);
  RUN_TEST(test_build_op_request_body_multi_content);
}

#endif /* TEST_C2OPENAPI_OP_H */
