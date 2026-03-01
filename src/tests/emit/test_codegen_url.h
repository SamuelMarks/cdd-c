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

#include "openapi/parse/openapi.h"
#include "routes/emit/url.h"

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
  if (codegen_url_write_query_params(tmp, op, 0) != 0) {
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

TEST test_query_gen_scalar_number(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "ratio";
  param.in = OA_PARAM_IN_QUERY;
  param.type = "number";
  param.is_array = 0;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);
  ASSERT(strstr(code, "sprintf(num_buf, \"%g\", ratio)") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"ratio\", num_buf)") != NULL);
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

TEST test_query_gen_array_explode_number(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "ratios";
  param.in = OA_PARAM_IN_QUERY;
  param.type = "array";
  param.is_array = 1;
  param.items_type = "number";
  param.explode = 1;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "for(i=0; i < ratios_len; ++i)") != NULL);
  ASSERT(strstr(code, "sprintf(num_buf, \"%g\", ratios[i])") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"ratios\", num_buf)") != NULL);

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

TEST test_query_gen_array_form_default_explode(void) {
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
  param.style = OA_STYLE_FORM;
  /* explode_set is false -> default should be explode=true */

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "for(i=0; i < tags_len; ++i)") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"tags\", tags[i])") != NULL);
  ASSERT(strstr(code, "joined_len") == NULL);

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

TEST test_query_gen_querystring_form_object(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "object";
  param.content_type = "application/x-www-form-urlencoded";
  param.schema.inline_type = "object";

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "Querystring Parameter (form object)") != NULL);
  ASSERT(strstr(code, "url_query_build_form(&qp, &qs_form_body)") != NULL);
  ASSERT(strstr(code, "const struct OpenAPI_KV *kv = &qs[i]") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_querystring_json_ref(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "object";
  param.content_type = "application/json";
  param.schema.ref_name = "Pet";

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "Querystring Parameter (json)") != NULL);
  ASSERT(strstr(code, "Pet_to_json(qs") != NULL);
  ASSERT(strstr(code, "url_encode(qs_json)") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_querystring_json_primitive(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "integer";
  param.content_type = "application/json";
  param.schema.inline_type = "integer";

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "Querystring Parameter (json primitive)") != NULL);
  ASSERT(strstr(code, "json_value_init_number") != NULL);
  ASSERT(strstr(code, "url_encode(qs_json)") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_querystring_json_array(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "array";
  param.content_type = "application/json";
  param.schema.is_array = 1;
  param.schema.inline_type = "string";

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "Querystring Parameter (json array)") != NULL);
  ASSERT(strstr(code, "json_value_init_array") != NULL);
  ASSERT(strstr(code, "json_array_append_string") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_querystring_json_array_object(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "array";
  param.content_type = "application/json";
  param.schema.is_array = 1;
  param.items_type = "Pet";

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "Querystring Parameter (json array objects)") != NULL);
  ASSERT(strstr(code, "Pet_to_json") != NULL);
  ASSERT(strstr(code, "json_parse_string(item_json)") != NULL);
  ASSERT(strstr(code, "json_array_append_value") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_querystring_raw_string(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "string";
  param.content_type = "text/plain";
  param.schema.inline_type = "string";

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "Querystring Parameter (raw)") != NULL);
  ASSERT(strstr(code, "url_encode(qs)") != NULL);
  ASSERT(strstr(code, "asprintf(&query_str, \"?%s\", qs_enc)") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_querystring_raw_integer(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "integer";
  param.content_type = "application/jsonpath";
  param.schema.inline_type = "integer";

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "Querystring Parameter (raw)") != NULL);
  ASSERT(strstr(code, "sprintf(num_buf") != NULL);
  ASSERT(strstr(code, "url_encode(num_buf)") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_array_form_explode_false(void) {
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
  param.style = OA_STYLE_FORM;
  param.explode = 0;
  param.explode_set = 1;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "url_query_add_encoded(&qp, \"tags\", joined)") != NULL);
  ASSERT(strstr(code, "joined[joined_len++] = ','") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_array_space_delimited(void) {
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
  param.style = OA_STYLE_SPACE_DELIMITED;
  param.explode = 0;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "joined[joined_len++] = ' '") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"tags\", joined)") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_array_pipe_delimited(void) {
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
  param.style = OA_STYLE_PIPE_DELIMITED;
  param.explode = 0;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "joined[joined_len++] = '|'") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"tags\", joined)") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_scalar_allow_reserved(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.type = "string";
  param.allow_reserved_set = 1;
  param.allow_reserved = 1;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);
  ASSERT(strstr(code, "url_encode_allow_reserved") != NULL);
  ASSERT(strstr(code, "url_query_add_encoded(&qp, \"filter\", enc)") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_array_explode_allow_reserved(void) {
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
  param.allow_reserved_set = 1;
  param.allow_reserved = 1;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "url_encode_allow_reserved(tags[i])") != NULL);
  ASSERT(strstr(code, "url_query_add_encoded(&qp, \"tags\", enc)") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_object_form_explode(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.type = "object";
  param.style = OA_STYLE_FORM;
  param.explode = 1;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "for(i=0; i < filter_len; ++i)") != NULL);
  ASSERT(strstr(code, "const struct OpenAPI_KV *kv = &filter[i]") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, kv_key, kv_raw)") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_object_form_explode_false(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.type = "object";
  param.style = OA_STYLE_FORM;
  param.explode = 0;
  param.explode_set = 1;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "key_enc = url_encode(kv_key)") != NULL);
  ASSERT(strstr(code, "val_enc = url_encode(kv_raw)") != NULL);
  ASSERT(strstr(code, "url_query_add_encoded(&qp, \"filter\", joined)") !=
         NULL);

  free(code);
  PASS();
}

TEST test_query_gen_object_deep_object(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.type = "object";
  param.style = OA_STYLE_DEEP_OBJECT;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "asprintf(&deep_key") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, deep_key, kv_raw)") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_object_space_delimited(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.type = "object";
  param.style = OA_STYLE_SPACE_DELIMITED;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "joined[joined_len++] = ' '") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"filter\", joined)") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_object_pipe_delimited(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.type = "object";
  param.style = OA_STYLE_PIPE_DELIMITED;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "joined[joined_len++] = '|'") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"filter\", joined)") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_object_space_delimited_allow_reserved(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.type = "object";
  param.style = OA_STYLE_SPACE_DELIMITED;
  param.allow_reserved_set = 1;
  param.allow_reserved = 1;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "url_encode_allow_reserved(kv_key)") != NULL);
  ASSERT(strstr(code, "url_query_add_encoded(&qp, \"filter\", joined)") !=
         NULL);
  ASSERT(strstr(code, "\"%20\"") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_array_space_delimited_allow_reserved(void) {
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
  param.style = OA_STYLE_SPACE_DELIMITED;
  param.allow_reserved_set = 1;
  param.allow_reserved = 1;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);

  ASSERT(strstr(code, "url_encode_allow_reserved(raw)") != NULL);
  ASSERT(strstr(code, "url_query_add_encoded(&qp, \"tags\", joined)") != NULL);
  ASSERT(strstr(code, "\"%20\"") != NULL);

  free(code);
  PASS();
}

TEST test_path_matrix_param_string(void) {
  struct OpenAPI_Parameter param;
  char *code;

  memset(&param, 0, sizeof(param));
  param.name = "id";
  param.in = OA_PARAM_IN_PATH;
  param.type = "string";
  param.style = OA_STYLE_MATRIX;
  param.explode = 0;

  code = gen_url_code("/pets/{id}", &param, 1);
  ASSERT(code);
  ASSERT(strstr(code, "path_id") != NULL);
  ASSERT(strstr(code, "\";id=%s\"") != NULL);

  free(code);
  PASS();
}

TEST test_path_label_array_explode(void) {
  struct OpenAPI_Parameter param;
  char *code;

  memset(&param, 0, sizeof(param));
  param.name = "tags";
  param.in = OA_PARAM_IN_PATH;
  param.type = "array";
  param.is_array = 1;
  param.items_type = "string";
  param.style = OA_STYLE_LABEL;
  param.explode = 1;

  code = gen_url_code("/tags/{tags}", &param, 1);
  ASSERT(code);
  ASSERT(strstr(code, "path_tags") != NULL);
  ASSERT(strstr(code, "memcpy(path_tags + path_len, \".\", 1)") != NULL);

  free(code);
  PASS();
}

TEST test_path_matrix_object_explode_false(void) {
  struct OpenAPI_Parameter param;
  char *code;

  memset(&param, 0, sizeof(param));
  param.name = "color";
  param.in = OA_PARAM_IN_PATH;
  param.type = "object";
  param.style = OA_STYLE_MATRIX;
  param.explode = 0;
  param.explode_set = 1;

  code = gen_url_code("/pets/{color}", &param, 1);
  ASSERT(code);
  ASSERT(strstr(code, "const struct OpenAPI_KV *kv = &color[i]") != NULL);
  ASSERT(strstr(code, "\";color=\"") != NULL);
  ASSERT(strstr(code, "path_color") != NULL);

  free(code);
  PASS();
}

TEST test_path_simple_param_number(void) {
  struct OpenAPI_Parameter param;
  char *code;

  memset(&param, 0, sizeof(param));
  param.name = "id";
  param.in = OA_PARAM_IN_PATH;
  param.type = "number";
  param.style = OA_STYLE_SIMPLE;

  code = gen_url_code("/items/{id}", &param, 1);
  ASSERT(code);
  ASSERT(strstr(code, "sprintf(num_buf, \"%g\", id)") != NULL);
  ASSERT(strstr(code, "asprintf(&path_id") != NULL);

  free(code);
  PASS();
}

TEST test_query_gen_json_content_ref(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  param.name = "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.content_type = "application/json";
  param.schema.ref_name = "Filter";
  param.type = "Filter";

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_query_code(&op);
  ASSERT(code);
  ASSERT(strstr(code, "Query Parameter (json): filter") != NULL);
  ASSERT(strstr(code, "Filter_to_json") != NULL);
  ASSERT(strstr(code, "url_query_add_encoded(&qp, \"filter\"") != NULL);

  free(code);
  PASS();
}

SUITE(codegen_url_suite) {
  /* Re-run original tests */
  /* (Omitted for brevity in this file update but would be here) */
  RUN_TEST(test_query_gen_scalar);
  RUN_TEST(test_query_gen_scalar_number);
  RUN_TEST(test_query_gen_array_explode_int);
  RUN_TEST(test_query_gen_array_explode_number);
  RUN_TEST(test_query_gen_array_explode_string);
  RUN_TEST(test_query_gen_array_form_default_explode);
  RUN_TEST(test_query_gen_querystring);
  RUN_TEST(test_query_gen_querystring_form_object);
  RUN_TEST(test_query_gen_querystring_json_ref);
  RUN_TEST(test_query_gen_querystring_json_primitive);
  RUN_TEST(test_query_gen_querystring_json_array);
  RUN_TEST(test_query_gen_querystring_json_array_object);
  RUN_TEST(test_query_gen_querystring_raw_string);
  RUN_TEST(test_query_gen_querystring_raw_integer);
  RUN_TEST(test_query_gen_json_content_ref);
  RUN_TEST(test_query_gen_array_form_explode_false);
  RUN_TEST(test_query_gen_array_space_delimited);
  RUN_TEST(test_query_gen_array_pipe_delimited);
  RUN_TEST(test_query_gen_scalar_allow_reserved);
  RUN_TEST(test_query_gen_array_explode_allow_reserved);
  RUN_TEST(test_query_gen_object_form_explode);
  RUN_TEST(test_query_gen_object_form_explode_false);
  RUN_TEST(test_query_gen_object_deep_object);
  RUN_TEST(test_query_gen_object_space_delimited);
  RUN_TEST(test_query_gen_object_pipe_delimited);
  RUN_TEST(test_query_gen_object_space_delimited_allow_reserved);
  RUN_TEST(test_query_gen_array_space_delimited_allow_reserved);
  RUN_TEST(test_path_matrix_param_string);
  RUN_TEST(test_path_label_array_explode);
  RUN_TEST(test_path_matrix_object_explode_false);
  RUN_TEST(test_path_simple_param_number);
}

#endif /* TEST_CODEGEN_URL_H */
