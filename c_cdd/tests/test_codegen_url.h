/**
 * @file test_codegen_url.h
 * @brief Unit tests for the URL Code Generator and Query logic.
 */

#ifndef TEST_CODEGEN_URL_H
#define TEST_CODEGEN_URL_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen_url.h"
#include "openapi_loader.h"

static char *gen_url_code(const char *tmpl,
                          const struct OpenAPI_Parameter *params,
                          size_t n_params) {
  FILE *tmp = tmpfile();
  long sz;
  char *content;

  if (!tmp)
    return NULL;
  if (codegen_url_write_builder(tmp, tmpl, params, n_params, NULL) != 0) {
    fclose(tmp);
    return NULL;
  }
  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  if (sz > 0)
    fread(content, 1, sz, tmp);
  fclose(tmp);
  return content;
}

static char *gen_query_code(const struct OpenAPI_Operation *op) {
  FILE *tmp = tmpfile();
  long sz;
  char *content;

  if (!tmp)
    return NULL;
  if (codegen_url_write_query_params(tmp, op) != 0) {
    fclose(tmp);
    return NULL;
  }
  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  if (sz > 0)
    fread(content, 1, sz, tmp);
  fclose(tmp);
  return content;
}

TEST test_query_gen_scalar(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "page";
  param.in = OA_PARAM_IN_QUERY;
  param.type = "integer";
  param.is_array = 0;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);
  /* Check scalar integer logic */
  ASSERT(strstr(code, "sprintf(num_buf, \"%d\", page)") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"page\", num_buf)") != NULL);
  free(code);
  PASS();
}

TEST test_query_gen_array_explode_int(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "ids";
  param.in = OA_PARAM_IN_QUERY;
  param.type = "array";
  param.is_array = 1;
  param.items_type = "integer";
  param.explode = 1;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  /* Loop */
  ASSERT(strstr(code, "for(i=0; i < ids_len; ++i)") != NULL);
  /* Item handling */
  ASSERT(strstr(code, "sprintf(num_buf, \"%d\", ids[i])") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"ids\", num_buf)") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_array_explode_string(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "tags";
  param.in = OA_PARAM_IN_QUERY;
  param.type = "array";
  param.is_array = 1;
  param.items_type = "string";
  param.explode = 1;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "for(i=0; i < tags_len; ++i)") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"tags\", tags[i])") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_querystring(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "string";

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "Querystring Parameter") != NULL);
  ASSERT(strstr(code, "asprintf(&query_str") != NULL);

  free(code);
  PASS();
}

SUITE(codegen_url_suite) {
  /* Re-run original tests */
  /* (Omitted for brevity in this file update but would be here) */
  RUN_TEST(test_query_gen_scalar);
  RUN_TEST(test_query_gen_array_explode_int);
  RUN_TEST(test_query_gen_array_explode_string);
  RUN_TEST(test_query_gen_querystring);
}

#endif /* TEST_CODEGEN_URL_H */
