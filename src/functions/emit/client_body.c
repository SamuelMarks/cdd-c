/**
 * @file client_body.c
 * @brief Client Function Body Generator Implementation.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "functions/emit/client_body_internal.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern int g_fail_io_after;
extern int g_io_calls;
#include <stdarg.h>
static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)))
#endif
    ;
static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...) {
  int ret;
  va_list args;
  if (g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after)
    return -1;
  va_start(args, format);
  ret = vfprintf(stream, format, args);
  va_end(args);
  return ret;
}
#define fprintf test_cdd_fprintf_hook
#endif

/**
 * @brief Checks if status range code.
 */
cdd_c_error_t client_body_is_status_range_code(const char *code,
                                               int *out_is_range) {
  if (!out_is_range)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_range = 0;
  if (!code)
    return CDD_C_SUCCESS;

  *out_is_range = (strlen(code) == 3 && code[0] >= '1' && code[0] <= '5' &&
                   code[1] == 'X' && code[2] == 'X');
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the status range prefix operation.
 */
cdd_c_error_t client_body_status_range_prefix(const char *code,
                                              int *out_prefix) {
  int is_range = 0;

  if (!out_prefix)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_prefix = 0;
  client_body_is_status_range_code(code, &is_range);
  if (!is_range)
    return CDD_C_SUCCESS;

  *out_prefix = (int)(code[0] - '0');
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if status code literal.
 */
cdd_c_error_t client_body_is_status_code_literal(const char *code,
                                                 int *out_is_lit) {
  if (!out_is_lit)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_lit = 0;
  if (!code || strlen(code) != 3)
    return CDD_C_SUCCESS;

  *out_is_lit = (code[0] >= '0' && code[0] <= '9' && code[1] >= '0' &&
                 code[1] <= '9' && code[2] >= '0' && code[2] <= '9');
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for codegen client write body.
 */
cdd_c_error_t codegen_client_write_body(FILE *fp,
                                        const struct OpenAPI_Operation *op,
                                        const struct OpenAPI_Spec *spec,
                                        const char *path_template,
                                        const char *base_url_expr) {
  const char *_ast_verb_to_enum_str_16 = NULL;
  const char *_ast_method_str_to_enum_str_17 = NULL;
  const char *err_code_str = NULL;
  int query_exists = 0;
  int cookie_exists = 0;
  int has_querystring = 0;
  int security_query = 0;
  int security_cookie = 0;
  int req_is_json = 0;
  int req_is_form = 0;
  int req_is_mp = 0;
  int req_is_mp_form = 0;
  int req_is_textual = 0;
  int req_is_binary = 0;
  size_t i;
  struct CodegenUrlConfig url_cfg;
  const struct OpenAPI_Response *default_resp = NULL;
  const struct OpenAPI_Response *range_resp[6] = {0};
  int has_range = 0;
  int has_success = 0;
  const char *success_schema_name = NULL;
  const char *success_inline_type = NULL;
  int success_inline_is_array = 0;
  int is_range_code = 0;
  int is_lit_code = 0;
  int range_bkt = 0;
  int def_has_pl = 0;
  int is_bin_resp = 0;
  int is_text_str_resp = 0;
  int req_body_has_inline = 0;

  if (!fp || !op || !path_template)

    return CDD_C_ERROR_INVALID_ARGUMENT;

  client_body_schema_has_inline(&op->req_body, &req_body_has_inline);

  if (op->req_body.content_type) {
    media_type_is_json(op->req_body.content_type, &req_is_json);
    media_type_is_form(op->req_body.content_type, &req_is_form);
    media_type_is_multipart(op->req_body.content_type, &req_is_mp);
    media_type_is_multipart_form(op->req_body.content_type, &req_is_mp_form);
    media_type_is_textual(op->req_body.content_type, &req_is_textual);
    media_type_is_binary(op->req_body.content_type, &req_is_binary);
  }

  if (spec) {
    security_query = (int)codegen_security_requires_query(op, spec);
    security_cookie = (int)codegen_security_requires_cookie(op, spec);
  }

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_QUERY ||
        op->parameters[i].in == OA_PARAM_IN_QUERYSTRING) {
      query_exists = 1;
      if (op->parameters[i].in == OA_PARAM_IN_QUERYSTRING)
        has_querystring = 1;
    }
    if (op->parameters[i].in == OA_PARAM_IN_COOKIE)
      cookie_exists = 1;
  }
  if (has_querystring && security_query)

    security_query = 0;

  if (security_query)
    query_exists = 1;
  if (security_cookie)
    cookie_exists = 1;

  /* --- 1. Declarations --- */
  CHECK_IO(fprintf(fp, "  struct HttpRequest req;\n"));
  CHECK_IO(fprintf(fp, "  struct HttpResponse *res = NULL;\n"));
  CHECK_IO(fprintf(fp, "  cdd_c_error_t rc = CDD_C_SUCCESS;\n"));
  CHECK_IO(fprintf(fp, "  int attempt = 0;\n"));
  CHECK_IO(fprintf(fp, "  int handled = 0;\n"));

  if (query_exists) {
    CHECK_IO(fprintf(fp, "  struct UrlQueryParams qp = {0};\n"));
    CHECK_IO(fprintf(fp, "  char *query_str = NULL;\n"));
    CHECK_IO(fprintf(fp, "  char *path_str = NULL;\n"));
    CHECK_IO(fprintf(fp, "  int qp_initialized = 0;\n"));
  } else {
    CHECK_IO(fprintf(fp, "  char *url = NULL;\n"));
  }
  if (cookie_exists) {
    CHECK_IO(fprintf(fp, "  char *cookie_str = NULL;\n"));
    CHECK_IO(fprintf(fp, "  size_t cookie_len = 0;\n"));
  }

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_PATH && op->parameters[i].name) {

      CHECK_IO(

          fprintf(fp, "  char *path_%s = NULL;\n", op->parameters[i].name));
    }
  }

  if (op->req_body.content_type && req_is_json &&
      (op->req_body.ref_name || req_body_has_inline)) {
    CHECK_IO(fprintf(fp, "  char *req_json = NULL;\n"));
  }
  if (op->req_body.ref_name && op->req_body.content_type && req_is_form) {
    CHECK_IO(fprintf(fp, "  struct UrlQueryParams form_qp;\n"));
    CHECK_IO(fprintf(fp, "  char *form_body = NULL;\n"));
  }

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].name) {
      CHECK_IO(fprintf(fp, "  (void)%s;\n", op->parameters[i].name));
      if (op->parameters[i].is_array ||
          (op->parameters[i].type &&
           strcmp(op->parameters[i].type, "array") == 0)) {
        CHECK_IO(fprintf(fp, "  (void)%s_len;\n", op->parameters[i].name));
      }
    }
  }
  if ((op->req_body.ref_name && op->req_body.is_array) ||
      (op->req_body.is_array && op->req_body.inline_type) ||
      (op->req_body.content_type && req_is_binary)) {
    CHECK_IO(fprintf(fp, "  (void)body;\n"));
    CHECK_IO(fprintf(fp, "  (void)body_len;\n"));
  } else if (op->req_body.ref_name || req_body_has_inline ||
             (op->req_body.content_type && req_is_textual)) {
    CHECK_IO(fprintf(fp, "  (void)req_body;\n"));
  }

  /* Ensure ApiError out is initialized */
  CHECK_IO(fprintf(fp, "  if (api_error) *api_error = NULL;\n\n"));

  {
    const struct OpenAPI_SchemaRef *success_schema = NULL;
    int success_is_binary = 0;

    /* Replicate client_sig.c logic for out param detection */
    const struct OpenAPI_Response *d_resp = NULL;
    size_t _i;
    for (_i = 0; _i < op->n_responses; ++_i) {
      if (!op->responses[_i].code)

        continue;

      if (strcmp(op->responses[_i].code, "default") == 0) {
        d_resp = &op->responses[_i];
        continue;
      }
      if (op->responses[_i].code[0] == '2') {
        int is_bin = 0;
        int resp_has_inline = 0;
        response_is_binary(&op->responses[_i], &is_bin);
        if (is_bin) {
          success_is_binary = 1;
        }
        client_body_schema_has_inline(&op->responses[_i].schema,
                                      &resp_has_inline);
        if (op->responses[_i].schema.ref_name || resp_has_inline ||
            op->responses[_i].schema.is_array) {
          success_schema = &op->responses[_i].schema;
        }
        break;
      }
    }
    if (!success_is_binary && !success_schema && d_resp) {
      int is_bin = 0;
      int d_resp_has_inline = 0;
      response_is_binary(d_resp, &is_bin);
      client_body_schema_has_inline(&d_resp->schema, &d_resp_has_inline);
      if (is_bin) {

        success_is_binary = 1;

      } else if (d_resp->schema.ref_name || d_resp_has_inline ||
                 d_resp->schema.is_array) {

        success_schema = &d_resp->schema;
      }
    }
    if (!success_is_binary && !success_schema) {
      success_schema = &op->req_body;
    }

    if (success_is_binary) {
      CHECK_IO(fprintf(fp, "  (void)out;\n  (void)out_len;\n"));
    } else {
      int has_out = 0;
      int succ_has_inline = 0;
      if (success_schema->is_array) {
        if (success_schema->ref_name || success_schema->inline_type) {
          has_out = 1;
        }
      } else {
        client_body_schema_has_inline(success_schema, &succ_has_inline);
        if (success_schema->ref_name || succ_has_inline) {
          has_out = 1;
        }
      }
      if (has_out) {
        CHECK_IO(fprintf(fp, "  (void)out;\n"));
        if (success_schema->is_array) {
          CHECK_IO(fprintf(fp, "  (void)out_len;\n"));
        }
      }
    }
  }

  /* --- 2. Init & Security --- */
  CHECK_IO(fprintf(
      fp, "  if (!ctx || !ctx->send) return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  if (op->req_body.is_array) {
    CHECK_IO(fprintf(fp, "  /* Array serialization not supported by cdd-c yet "
                         "*/\n  return CDD_C_ERROR_SYSTEM; /* ENOTSUP */\n"));
  }
  CHECK_IO(fprintf(fp, "  rc = http_request_init(&req);\n"));
  CHECK_IO(fprintf(fp, "  if (rc != CDD_C_SUCCESS) return rc;\n\n"));

  if (spec) {
    codegen_security_write_apply(fp, op, spec);
  }

  /* --- 3. Header Param Logic --- */
  if (write_header_param_logic(fp, op) != 0)

    return CDD_C_ERROR_IO;

  /* --- 4. Cookie Param Logic --- */
  if (write_cookie_param_logic(fp, op) != 0)

    return CDD_C_ERROR_IO;

  /* --- 5. Query Param Logic --- */
  if (codegen_url_write_query_params(fp, op, query_exists ? 1 : 0) != 0)

    return CDD_C_ERROR_IO;

  /* --- 6. Body Serialization --- */
  {
    const char *ct = op->req_body.content_type;
    if (ct) {
      if (req_is_mp_form) {
        if (write_multipart_body(fp, op, spec) != 0)

          return CDD_C_ERROR_IO;

      } else if (req_is_form) {
        if (write_form_urlencoded_body(fp, op, spec) != 0)

          return CDD_C_ERROR_IO;

      } else if (req_is_json && op->req_body.ref_name) {

        CHECK_IO(fprintf(

            fp, "  rc = %s_to_json((const struct %s *)%s, &req_json);\n",
            op->req_body.ref_name, op->req_body.ref_name,
            op->req_body.is_array ? "body" : "req_body"));

        CHECK_IO(fprintf(fp, "  if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "  req.body = req_json;\n"));
        CHECK_IO(fprintf(fp, "  req.body_len = strlen(req_json);\n"));
        CHECK_IO(fprintf(fp, "  http_headers_add(&req.headers, "

                             "\"Content-Type\", \"application/json\");\n\n"));
      } else if (req_is_json && req_body_has_inline) {
        CHECK_IO(fprintf(fp, "  {\n"));
        CHECK_IO(fprintf(fp, "    JSON_Value *req_val = NULL;\n"));
        CHECK_IO(fprintf(fp, "    char *tmp_json = NULL;\n"));
        if (op->req_body.is_array) {
          CHECK_IO(fprintf(fp, "    JSON_Array *req_arr = NULL;\n"));
          CHECK_IO(fprintf(fp, "    size_t i;\n"));
          CHECK_IO(fprintf(fp, "    req_val = json_value_init_array();\n"));
          CHECK_IO(fprintf(fp, "    if (!req_val) { rc = CDD_C_ERROR_MEMORY; "
                               "goto cleanup; }\n"));
          CHECK_IO(
              fprintf(fp, "    req_arr = json_value_get_array(req_val);\n"));
          CHECK_IO(fprintf(fp,
                           "    if (!req_arr) { rc = "
                           "CDD_C_ERROR_INVALID_ARGUMENT; goto cleanup; }\n"));
          CHECK_IO(fprintf(fp, "    for (i = 0; i < body_len; ++i) {\n"));
          if (strcmp(op->req_body.inline_type, "string") == 0) {

            CHECK_IO(fprintf(fp, "      if (!body[i]) {\n"));
            CHECK_IO(fprintf(

                fp,
                "        if (json_array_append_null(req_arr) != "
                "JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

            CHECK_IO(fprintf(fp, "      } else {\n"));
            CHECK_IO(fprintf(

                fp, "        if (json_array_append_string(req_arr, "
                    "body[i]) != JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto "
                    "cleanup; }\n"));

            CHECK_IO(fprintf(fp, "      }\n"));

          } else if (strcmp(op->req_body.inline_type, "integer") == 0) {
            CHECK_IO(fprintf(
                fp,
                "      if (json_array_append_number(req_arr, "
                "(double)body[i]) != JSONSuccess) { rc = CDD_C_ERROR_MEMORY; "
                "goto cleanup; }\n"));

          } else if (strcmp(op->req_body.inline_type, "number") == 0) {
            CHECK_IO(fprintf(

                fp, "      if (json_array_append_number(req_arr, "
                    "body[i]) != JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto "
                    "cleanup; }\n"));

          } else if (strcmp(op->req_body.inline_type, "boolean") == 0) {
            CHECK_IO(fprintf(

                fp,
                "      if (json_array_append_boolean(req_arr, "
                "body[i] ? 1 : 0) != JSONSuccess) { rc = CDD_C_ERROR_MEMORY; "
                "goto cleanup; }\n"));
          } else {

            CHECK_IO(fprintf(

                fp,
                "      rc = CDD_C_ERROR_INVALID_ARGUMENT; goto cleanup;\n"));
          }
          CHECK_IO(fprintf(fp, "    }\n"));
        } else if (strcmp(op->req_body.inline_type, "string") == 0) {
          CHECK_IO(fprintf(fp,
                           "    if (!req_body) { rc = "
                           "CDD_C_ERROR_INVALID_ARGUMENT; goto cleanup; }\n"));
          CHECK_IO(
              fprintf(fp, "    req_val = json_value_init_string(req_body);\n"));

        } else if (strcmp(op->req_body.inline_type, "integer") == 0) {
          CHECK_IO(fprintf(fp, "    req_val = json_value_init_number((double)"

                               "req_body);\n"));

        } else if (strcmp(op->req_body.inline_type, "number") == 0) {
          CHECK_IO(

              fprintf(fp, "    req_val = json_value_init_number(req_body);\n"));

        } else if (strcmp(op->req_body.inline_type, "boolean") == 0) {
          CHECK_IO(fprintf(

              fp, "    req_val = json_value_init_boolean(req_body ? 1 : "
                  "0);\n"));
        } else {

          CHECK_IO(fprintf(

              fp, "    rc = CDD_C_ERROR_INVALID_ARGUMENT; goto cleanup;\n"));
        }
        CHECK_IO(fprintf(
            fp,
            "    if (!req_val) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
        CHECK_IO(
            fprintf(fp, "    tmp_json = json_serialize_to_string(req_val);\n"));
        CHECK_IO(fprintf(fp,
                         "    if (!tmp_json) { json_value_free(req_val); rc = "
                         "ENOMEM; goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "    req_json = strdup(tmp_json);\n"));
        CHECK_IO(fprintf(fp, "    json_free_serialized_string(tmp_json);\n"));
        CHECK_IO(fprintf(fp, "    json_value_free(req_val);\n"));
        CHECK_IO(fprintf(
            fp,
            "    if (!req_json) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "    req.body = req_json;\n"));
        CHECK_IO(fprintf(fp, "    req.body_len = strlen(req_json);\n"));
        CHECK_IO(fprintf(fp, "    http_headers_add(&req.headers, "
                             "\"Content-Type\", \"application/json\");\n"));
        CHECK_IO(fprintf(fp, "  }\n\n"));
      } else if (req_is_textual) {
        CHECK_IO(fprintf(fp, "  if (req_body) {\n"));
        CHECK_IO(fprintf(fp, "    req.body = (void *)req_body;\n"));
        CHECK_IO(fprintf(fp, "    req.body_len = strlen(req_body);\n"));
        CHECK_IO(fprintf(fp,
                         "    http_headers_add(&req.headers, "
                         "\"Content-Type\", \"%s\");\n",
                         ct));
        CHECK_IO(fprintf(fp, "  }\n\n"));
      } else if (req_is_binary || req_is_mp) {
        CHECK_IO(fprintf(fp, "  req.body = (void *)body;\n"));
        CHECK_IO(fprintf(fp, "  req.body_len = body_len;\n"));
        CHECK_IO(fprintf(fp,
                         "  http_headers_add(&req.headers, "
                         "\"Content-Type\", \"%s\");\n\n",
                         ct));
      }
    }
  }

  /* --- 7. URL Construction --- */
  memset(&url_cfg, 0, sizeof(url_cfg));
  url_cfg.out_variable = query_exists ? "path_str" : "url";
  url_cfg.base_variable = base_url_expr ? base_url_expr : "ctx->base_url";

  if (codegen_url_write_builder(fp, path_template, op->parameters,
                                op->n_parameters, &url_cfg) != 0) {

    return CDD_C_ERROR_IO;
  }

  if (query_exists) {
    CHECK_IO(fprintf(
        fp, "  if (asprintf(&req.url, \"%%s%%s\", path_str, "
            "query_str) == -1) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
  } else {
    CHECK_IO(fprintf(fp, "  req.url = url;\n"));
  }

  {
    const char *method_enum =
        (verb_to_enum_str(op->verb, &_ast_verb_to_enum_str_16),
         _ast_verb_to_enum_str_16);
    if (op->is_additional) {
      const char *mapped =
          (method_str_to_enum_str(op->method, &_ast_method_str_to_enum_str_17),
           _ast_method_str_to_enum_str_17);
      if (mapped) {
        method_enum = mapped;
      } else if (op->method && op->method[0] != '\0') {
        CHECK_IO(fprintf(fp,
                         "  /* Warning: unsupported HTTP method '%s', "
                         "defaulting to GET */\n",
                         op->method));
        method_enum = "HTTP_GET";
      }
    }
    CHECK_IO(fprintf(fp, "  req.method = %s;\n\n", method_enum));
  }

  /* --- 8. Send with Retry Logic --- */
  CHECK_IO(fprintf(fp, "  do {\n"));
  CHECK_IO(fprintf(fp, "    if(attempt > 0) {\n"));
  CHECK_IO(fprintf(fp, "      /* Implement backoff delay here if needed */\n"));
  CHECK_IO(fprintf(fp, "    }\n"));

  CHECK_IO(fprintf(fp, "    rc = ctx->send(ctx->transport, &req, &res);\n"));
  CHECK_IO(fprintf(fp, "    attempt++;\n"));
  CHECK_IO(fprintf(fp, "  } while (rc != CDD_C_SUCCESS && attempt <= "
                       "ctx->config.retry_count);\n\n"));

  CHECK_IO(fprintf(fp, "  if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
  CHECK_IO(
      fprintf(fp, "  if (!res) { rc = CDD_C_ERROR_IO; goto cleanup; }\n\n"));

  for (i = 0; i < op->n_responses; ++i) {
    const struct OpenAPI_Response *resp = &op->responses[i];
    if (!resp->code)

      continue;

    if (strcmp(resp->code, "default") == 0) {
      default_resp = resp;
      continue;
    }
    is_status_range_code(resp->code, &is_range_code);
    if (is_range_code) {
      status_range_prefix(resp->code, &range_bkt);
      range_resp[range_bkt] = resp;
      has_range = 1;
      if (range_bkt == 2) {
        int r2_has_inline = 0;
        has_success = 1;
        if (!success_schema_name && resp->schema.ref_name)
          success_schema_name = resp->schema.ref_name;
        client_body_schema_has_inline(&resp->schema, &r2_has_inline);
        if (!success_inline_type && r2_has_inline) {
          success_inline_type = resp->schema.inline_type;
          success_inline_is_array = resp->schema.is_array ? 1 : 0;
        }
      }
      continue;
    }
    if (resp->code[0] == '2') {
      int r2_has_inline = 0;
      has_success = 1;
      if (!success_schema_name && resp->schema.ref_name)

        success_schema_name = resp->schema.ref_name;

      client_body_schema_has_inline(&resp->schema, &r2_has_inline);
      if (!success_inline_type && r2_has_inline) {
        success_inline_type = resp->schema.inline_type;
        success_inline_is_array = resp->schema.is_array ? 1 : 0;
      }
    }
  }
  if (!success_schema_name && !success_inline_type && default_resp &&
      !has_success) {
    schema_has_payload(&default_resp->schema, &def_has_pl);
    if (def_has_pl) {
      int def_has_inline = 0;
      client_body_schema_has_inline(&default_resp->schema, &def_has_inline);
      if (default_resp->schema.ref_name) {
        success_schema_name = default_resp->schema.ref_name;
      } else {
        success_inline_type = default_resp->schema.inline_type;
        success_inline_is_array = default_resp->schema.is_array ? 1 : 0;
      }
    }
  }

  /* --- 9. Responses --- */

  CHECK_IO(fprintf(fp, "  switch (res->status_code) {\n"));
  for (i = 0; i < op->n_responses; ++i) {
    const struct OpenAPI_Response *resp = &op->responses[i];
    if (!resp->code)

      continue;

    if (strcmp(resp->code, "default") == 0)
      continue;
    is_status_range_code(resp->code, &is_range_code);
    if (is_range_code)
      continue;
    is_status_code_literal(resp->code, &is_lit_code);
    if (!is_lit_code)
      continue;

    CHECK_IO(fprintf(fp, "    case %s:\n", resp->code));
    CHECK_IO(fprintf(fp, "      handled = 1;\n"));
    if (resp->code[0] == '2') {
      int resp_has_inline = 0;
      response_is_binary(resp, &is_bin_resp);
      response_is_textual_string(resp, &is_text_str_resp);

      if (is_bin_resp) {
        if (write_binary_success(fp) != 0)

          return CDD_C_ERROR_IO;

      } else if (is_text_str_resp) {
        if (write_text_plain_success(fp) != 0)

          return CDD_C_ERROR_IO;

      } else if (resp->schema.ref_name) {

        CHECK_IO(fprintf(fp, "      if (res->body && out) {\n"));
        if (resp->schema.is_array) {
          CHECK_IO(fprintf(fp,

                           "        rc = %s_array_from_json((const "
                           "char*)res->body, out, out_len);\n",
                           resp->schema.ref_name));
        } else {

          CHECK_IO(fprintf(

              fp, "        rc = %s_from_json((const char*)res->body, out);\n",
              resp->schema.ref_name));
        }

        CHECK_IO(fprintf(fp, "      }\n"));

      } else {
        client_body_schema_has_inline(&resp->schema, &resp_has_inline);
        if (resp_has_inline) {
          if (write_inline_json_parse(fp, &resp->schema) != 0)

            return CDD_C_ERROR_IO;
        }
      }
      CHECK_IO(fprintf(fp, "      break;\n"));
    } else {
      mapped_err_code(atoi(resp->code), &err_code_str);
      CHECK_IO(fprintf(fp, "      rc = %s;\n", err_code_str));
      CHECK_IO(fprintf(fp, "      if (res->body && api_error) {\n"));
      CHECK_IO(fprintf(
          fp, "        cdd_c_error_t api_rc = ApiError_from_json((const "
              "char*)res->body, api_error);\n"));
      CHECK_IO(fprintf(fp, "        if (api_rc != CDD_C_SUCCESS) { "
                           "C_CDD_LOG_DEBUG(\"Failed to parse ApiError: "
                           "%%d\\n\", api_rc); }\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
      CHECK_IO(fprintf(fp, "      break;\n"));
    }
  }
  CHECK_IO(fprintf(fp, "    default:\n"));
  CHECK_IO(fprintf(fp, "      break;\n"));
  CHECK_IO(fprintf(fp, "  }\n"));

  if (has_range) {
    CHECK_IO(fprintf(fp, "  if (!handled) {\n"));
    for (i = 1; i <= 5; ++i) {
      const struct OpenAPI_Response *resp = range_resp[i];
      if (!resp)
        continue;
      if (i == 2) {
        int resp_has_inline = 0;
        response_is_binary(resp, &is_bin_resp);
        response_is_textual_string(resp, &is_text_str_resp);

        CHECK_IO(fprintf(fp, "    if (res->status_code >= 200 && "
                             "res->status_code < 300) {\n"));
        CHECK_IO(fprintf(fp, "      handled = 1;\n"));
        if (is_bin_resp) {

          if (write_binary_success(fp) != 0)
            return CDD_C_ERROR_IO;

        } else if (is_text_str_resp) {
          if (write_text_plain_success(fp) != 0)

            return CDD_C_ERROR_IO;

        } else if (resp->schema.ref_name) {
          CHECK_IO(fprintf(fp, "      if (res->body && out) {\n"));
          CHECK_IO(fprintf(
              fp, "        rc = %s_from_json((const char*)res->body, out);\n",
              resp->schema.ref_name));
          CHECK_IO(fprintf(fp, "      }\n"));

        } else {
          client_body_schema_has_inline(&resp->schema, &resp_has_inline);
          if (resp_has_inline) {
            if (write_inline_json_parse(fp, &resp->schema) != 0)
              return CDD_C_ERROR_IO;
          }
        }
        CHECK_IO(fprintf(fp, "    }\n"));
      } else {

        CHECK_IO(fprintf(fp,

                         "    if (res->status_code >= %d && "
                         "res->status_code < %d) {\n",
                         (int)(i * 100), (int)((i + 1) * 100)));

        CHECK_IO(fprintf(fp, "      handled = 1;\n"));
        mapped_err_code((int)(i * 100), &err_code_str);
        CHECK_IO(

            fprintf(fp, "      rc = %s;\n", err_code_str));

        CHECK_IO(fprintf(fp, "      if (res->body && api_error) {\n"));
        CHECK_IO(fprintf(fp, "        cdd_c_error_t api_rc = "

                             "ApiError_from_json((const char*)res->body, "
                             "api_error);\n"));

        CHECK_IO(fprintf(fp, "        if (api_rc != CDD_C_SUCCESS) { "

                             "C_CDD_LOG_DEBUG(\"Failed to parse ApiError: "
                             "%%d\\n\", api_rc); }\n"));

        CHECK_IO(fprintf(fp, "      }\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
      }
    }
    CHECK_IO(fprintf(fp, "  }\n"));
  }

  CHECK_IO(fprintf(fp, "  if (!handled) {\n"));
  if (default_resp) {
    int default_is_success = 0;
    int default_matches_success = 0;
    int def_has_inline = 0;

    schema_has_payload(&default_resp->schema, &def_has_pl);
    response_is_binary(default_resp, &is_bin_resp);
    response_is_textual_string(default_resp, &is_text_str_resp);

    default_is_success = (!has_success && (def_has_pl || is_bin_resp));

    client_body_schema_has_inline(&default_resp->schema, &def_has_inline);

    if (success_schema_name && default_resp->schema.ref_name &&
        strcmp(success_schema_name, default_resp->schema.ref_name) == 0) {
      default_matches_success = 1;
    }
    if (success_inline_type && def_has_inline &&
        strcmp(success_inline_type, default_resp->schema.inline_type) == 0 &&
        success_inline_is_array == (default_resp->schema.is_array ? 1 : 0)) {
      default_matches_success = 1;
    }
    CHECK_IO(fprintf(fp, "    /* default response */\n"));
    if (default_is_success || default_matches_success) {
      if (is_bin_resp) {

        if (write_binary_success(fp) != 0)
          return CDD_C_ERROR_IO;

      } else if (is_text_str_resp) {
        if (write_text_plain_success(fp) != 0)

          return CDD_C_ERROR_IO;

      } else if (default_resp->schema.ref_name) {
        CHECK_IO(fprintf(fp, "    if (res->body && out) {\n"));
        if (default_resp->schema.is_array) {

          CHECK_IO(fprintf(fp,

                           "      rc = %s_array_from_json((const "
                           "char*)res->body, out, out_len);\n",
                           default_resp->schema.ref_name));
        } else {
          CHECK_IO(fprintf(
              fp, "      rc = %s_from_json((const char*)res->body, out);\n",
              default_resp->schema.ref_name));
        }
        CHECK_IO(fprintf(fp, "    }\n"));

      } else {
        if (write_inline_json_parse(fp, &default_resp->schema) != 0)
          return CDD_C_ERROR_IO;
      }
    } else {

      CHECK_IO(fprintf(fp, "    rc = CDD_C_ERROR_IO;\n"));
      CHECK_IO(fprintf(fp, "    if (res->body && api_error) {\n"));
      CHECK_IO(fprintf(

          fp, "      cdd_c_error_t api_rc = ApiError_from_json((const "
              "char*)res->body, api_error);\n"));

      CHECK_IO(fprintf(fp, "      if (api_rc != CDD_C_SUCCESS) { "

                           "C_CDD_LOG_DEBUG(\"Failed to parse ApiError: "
                           "%%d\\n\", api_rc); }\n"));

      CHECK_IO(fprintf(fp, "    }\n"));
    }
  } else {
    CHECK_IO(fprintf(fp, "    rc = CDD_C_ERROR_IO;\n"));
    CHECK_IO(fprintf(fp, "    if (res->body && api_error) {\n"));
    CHECK_IO(fprintf(fp,
                     "      cdd_c_error_t api_rc = ApiError_from_json((const "
                     "char*)res->body, api_error);\n"));
    CHECK_IO(fprintf(fp, "      if (api_rc != CDD_C_SUCCESS) { "
                         "C_CDD_LOG_DEBUG(\"Failed to parse ApiError: "
                         "%%d\\n\", api_rc); }\n"));
    CHECK_IO(fprintf(fp, "    }\n"));
  }
  CHECK_IO(fprintf(fp, "  }\n\n"));

  /* --- 10. Cleanup --- */
  CHECK_IO(fprintf(fp, "cleanup:\n"));

  if (op->req_body.content_type && req_is_json &&
      (op->req_body.ref_name || req_body_has_inline)) {
    CHECK_IO(fprintf(fp, "  if (req_json) free(req_json);\n"));
  }
  if (op->req_body.ref_name && op->req_body.content_type && req_is_form) {
    CHECK_IO(fprintf(fp, "  if (form_body) free(form_body);\n"));
    CHECK_IO(fprintf(fp, "  url_query_free(&form_qp);\n"));
  }
  if (query_exists) {
    CHECK_IO(fprintf(fp, "  if (path_str) free(path_str);\n"));
    CHECK_IO(fprintf(fp, "  if (query_str) free(query_str);\n"));
    CHECK_IO(fprintf(fp, "  url_query_free(&qp);\n"));
  }
  if (cookie_exists) {
    CHECK_IO(fprintf(fp, "  if (cookie_str) free(cookie_str);\n"));
  }
  CHECK_IO(fprintf(fp, "  http_request_free(&req);\n"));
  CHECK_IO(fprintf(fp, "  if (res) { http_response_free(res); free(res); }\n"));
  CHECK_IO(fprintf(fp, "  return rc;\n}\n"));

  return CDD_C_SUCCESS;
}
