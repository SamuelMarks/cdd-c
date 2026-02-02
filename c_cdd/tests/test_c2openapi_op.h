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
  /* Use openapi_spec_free or manual free?
     The op inside spec relies on arrays. Loader's free logic is complex.
     We use a simplified free here since we only populate one op.
  */
  /* Call module internal logic if available or reimplement basic cleanup */
  /* Reimplementing basics for test safety */
  if (op->operation_id)
    free(op->operation_id);
  if (op->parameters) {
    size_t i;
    for (i = 0; i < op->n_parameters; i++) {
      free(op->parameters[i].name);
      /* op->parameters[i].in is an enum, nothing to free */
      free(op->parameters[i].type);
      if (op->parameters[i].items_type)
        free(op->parameters[i].items_type);
    }
    free(op->parameters);
  }
  if (op->tags) {
    if (op->n_tags > 0) {
      /* In tests we usually allocate tags[0] */
      if (op->tags[0])
        free(op->tags[0]);
    }
    free(op->tags);
  }
  if (op->req_body.ref_name)
    free(op->req_body.ref_name);
  if (op->req_body.content_type)
    free(op->req_body.content_type);
  if (op->responses) {
    size_t i;
    for (i = 0; i < op->n_responses; i++) {
      free(op->responses[i].code);
      if (op->responses[i].schema.ref_name)
        free(op->responses[i].schema.ref_name);
    }
    free(op->responses);
  }
  memset(op, 0, sizeof(*op));
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

  /* Verify Parameter */
  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("id", op.parameters[0].name);
  ASSERT_EQ(OA_PARAM_IN_PATH, op.parameters[0].in);
  ASSERT(op.parameters[0].required);
  ASSERT_STR_EQ("integer", op.parameters[0].type);

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

  reset_op(&op);
  PASS();
}

SUITE(c2openapi_op_suite) {
  RUN_TEST(test_build_simple_get);
  RUN_TEST(test_build_post_with_body);
  RUN_TEST(test_build_params_explicit);
  RUN_TEST(test_build_response_output_arg);
}

#endif /* TEST_C2OPENAPI_OP_H */
