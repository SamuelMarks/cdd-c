/**
 * @file codegen_client_body.c
 * @brief Implementation of the Request Orchestrator.
 *
 * Generates C implementation for API client functions including retry logic
 * and standardized error parsing via `ApiError`.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd_stdbool.h"
#include "codegen_client_body.h"
#include "codegen_security.h"
#include "codegen_struct.h"
#include "codegen_url.h"
#include "openapi_loader.h"
#include "str_utils.h"

#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif

static const char *verb_to_enum_str(enum OpenAPI_Verb v) {
  switch (v) {
  case OA_VERB_GET:
    return "HTTP_GET";
  case OA_VERB_POST:
    return "HTTP_POST";
  case OA_VERB_PUT:
    return "HTTP_PUT";
  case OA_VERB_DELETE:
    return "HTTP_DELETE";
  case OA_VERB_HEAD:
    return "HTTP_HEAD";
  case OA_VERB_PATCH:
    return "HTTP_PATCH";
  case OA_VERB_OPTIONS:
    return "HTTP_OPTIONS";
  case OA_VERB_TRACE:
    return "HTTP_TRACE";
  case OA_VERB_QUERY:
    return "HTTP_QUERY";
  default:
    return "HTTP_GET";
  }
}

static int mapped_err_code(int status) {
  if (status == 400)
    return 22; /* EINVAL */
  if (status == 401 || status == 403)
    return 13; /* EACCES */
  if (status == 404)
    return 2; /* ENOENT */
  return 5;   /* EIO generic */
}

static int write_header_param_logic(FILE *fp,
                                    const struct OpenAPI_Operation *op) {
  size_t i;
  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_HEADER) {
      const struct OpenAPI_Parameter *p = &op->parameters[i];
      CHECK_IO(fprintf(fp, "  /* Header Parameter: %s */\n", p->name));
      if (strcmp(p->type, "string") == 0) {
        CHECK_IO(fprintf(fp, "  if (%s) {\n", p->name));
        CHECK_IO(fprintf(
            fp, "    rc = http_headers_add(&req.headers, \"%s\", %s);\n",
            p->name, p->name));
        CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "  }\n"));
      } else if (strcmp(p->type, "integer") == 0) {
        CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
        CHECK_IO(fprintf(fp, "    sprintf(num_buf, \"%%d\", %s);\n", p->name));
        CHECK_IO(fprintf(
            fp, "    rc = http_headers_add(&req.headers, \"%s\", num_buf);\n",
            p->name));
        CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n  }\n"));
      } else if (strcmp(p->type, "boolean") == 0) {
        CHECK_IO(fprintf(fp,
                         "  rc = http_headers_add(&req.headers, \"%s\", %s ? "
                         "\"true\" : \"false\");\n",
                         p->name, p->name));
        CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));
      }
    }
  }
  return 0;
}

static int write_multipart_body(FILE *fp, const struct OpenAPI_Operation *op,
                                const struct OpenAPI_Spec *spec) {
  const struct StructFields *sf;
  size_t i;

  sf = openapi_spec_find_schema(spec, op->req_body.ref_name);
  if (!sf) {
    CHECK_IO(fprintf(
        fp,
        "  /* Warning: Schema %s definition not found, skipping multipart */\n",
        op->req_body.ref_name));
    return 0;
  }

  CHECK_IO(fprintf(fp, "  /* Multipart Body Construction */\n"));
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    if (strcmp(f->type, "string") == 0) {
      CHECK_IO(fprintf(fp, "    if (req_body->%s) {\n", f->name));
      CHECK_IO(fprintf(fp,
                       "      rc = http_request_add_part(&req, \"%s\", NULL, "
                       "NULL, req_body->%s, strlen(req_body->%s));\n",
                       f->name, f->name, f->name));
      CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
    } else if (strcmp(f->type, "integer") == 0) {
      CHECK_IO(fprintf(fp, "    {\n      char num_buf[32];\n"));
      CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%d\", req_body->%s);\n",
                       f->name));
      CHECK_IO(fprintf(fp,
                       "      rc = http_request_add_part(&req, \"%s\", NULL, "
                       "NULL, num_buf, strlen(num_buf));\n",
                       f->name));
      CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n    }\n"));
    }
  }
  CHECK_IO(fprintf(fp, "  rc = http_request_flatten_parts(&req);\n"));
  CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n\n"));
  return 0;
}

int codegen_client_write_body(FILE *const fp,
                              const struct OpenAPI_Operation *const op,
                              const struct OpenAPI_Spec *const spec,
                              const char *const path_template) {
  int query_exists = 0;
  size_t i;
  struct CodegenUrlConfig url_cfg;

  if (!fp || !op || !path_template)
    return EINVAL;

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_QUERY ||
        op->parameters[i].in == OA_PARAM_IN_QUERYSTRING)
      query_exists = 1;
  }

  /* --- 1. Declarations --- */
  CHECK_IO(fprintf(fp, "  struct HttpRequest req;\n"));
  CHECK_IO(fprintf(fp, "  struct HttpResponse *res = NULL;\n"));
  CHECK_IO(fprintf(fp, "  int rc = 0;\n"));
  CHECK_IO(fprintf(fp, "  int attempt = 0;\n"));

  if (query_exists) {
    CHECK_IO(fprintf(fp, "  struct UrlQueryParams qp;\n"));
    CHECK_IO(fprintf(fp, "  char *query_str = NULL;\n"));
    CHECK_IO(fprintf(fp, "  char *path_str = NULL;\n"));
  } else {
    CHECK_IO(fprintf(fp, "  char *url = NULL;\n"));
  }

  if (op->req_body.ref_name && op->req_body.content_type &&
      strcmp(op->req_body.content_type, "application/json") == 0) {
    CHECK_IO(fprintf(fp, "  char *req_json = NULL;\n"));
  }

  /* Ensure ApiError out is initialized */
  CHECK_IO(fprintf(fp, "  if (api_error) *api_error = NULL;\n\n"));

  /* --- 2. Init & Security --- */
  CHECK_IO(fprintf(fp, "  if (!ctx || !ctx->send) return EINVAL;\n"));
  CHECK_IO(fprintf(fp, "  rc = http_request_init(&req);\n"));
  CHECK_IO(fprintf(fp, "  if (rc != 0) return rc;\n\n"));

  if (spec) {
    if (codegen_security_write_apply(fp, op, spec) != 0)
      return EIO;
  }

  /* --- 3. Header Param Logic --- */
  if (write_header_param_logic(fp, op) != 0)
    return EIO;

  /* --- 4. Query Param Logic --- */
  if (codegen_url_write_query_params(fp, op) != 0)
    return EIO;

  /* --- 5. Body Serialization --- */
  if (op->req_body.ref_name) {
    if (op->req_body.content_type) {
      if (strcmp(op->req_body.content_type, "multipart/form-data") == 0) {
        if (write_multipart_body(fp, op, spec) != 0)
          return EIO;
      } else if (strcmp(op->req_body.content_type, "application/json") == 0) {
        CHECK_IO(fprintf(fp, "  rc = %s_to_json(req_body, &req_json);\n",
                         op->req_body.ref_name));
        CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "  req.body = req_json;\n"));
        CHECK_IO(fprintf(fp, "  req.body_len = strlen(req_json);\n"));
        CHECK_IO(fprintf(fp, "  http_headers_add(&req.headers, "
                             "\"Content-Type\", \"application/json\");\n\n"));
      }
    }
  }

  /* --- 6. URL Construction --- */
  memset(&url_cfg, 0, sizeof(url_cfg));
  url_cfg.out_variable = query_exists ? "path_str" : "url";
  url_cfg.base_variable = "ctx->base_url";

  if (codegen_url_write_builder(fp, path_template, op->parameters,
                                op->n_parameters, &url_cfg) != 0) {
    return EIO;
  }

  if (query_exists) {
    CHECK_IO(fprintf(fp, "  if (asprintf(&req.url, \"%%s%%s\", path_str, "
                         "query_str) == -1) { rc = ENOMEM; goto cleanup; }\n"));
  } else {
    CHECK_IO(fprintf(fp, "  req.url = url;\n"));
  }

  CHECK_IO(fprintf(fp, "  req.method = %s;\n\n", verb_to_enum_str(op->verb)));

  /* --- 7. Send with Retry Logic --- */
  CHECK_IO(fprintf(fp, "  do {\n"));
  CHECK_IO(fprintf(fp, "    if(attempt > 0) {\n"));
  CHECK_IO(fprintf(fp, "      /* Implement backoff delay here if needed */\n"));
  CHECK_IO(fprintf(fp, "    }\n"));

  CHECK_IO(fprintf(fp, "    rc = ctx->send(ctx->transport, &req, &res);\n"));
  CHECK_IO(fprintf(fp, "    attempt++;\n"));
  CHECK_IO(fprintf(
      fp, "  } while (rc != 0 && attempt <= ctx->config.retry_count);\n\n"));

  CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));
  CHECK_IO(fprintf(fp, "  if (!res) { rc = EIO; goto cleanup; }\n\n"));

  /* --- 8. Responses --- */
  CHECK_IO(fprintf(fp, "  switch (res->status_code) {\n"));
  for (i = 0; i < op->n_responses; ++i) {
    const struct OpenAPI_Response *resp = &op->responses[i];
    if (strcmp(resp->code, "default") == 0)
      continue;
    CHECK_IO(fprintf(fp, "    case %s:\n", resp->code));
    if (resp->code[0] == '2') {
      if (resp->schema.ref_name) {
        CHECK_IO(fprintf(fp, "      if (res->body && out) {\n"));
        CHECK_IO(fprintf(
            fp, "        rc = %s_from_json((const char*)res->body, out%s);\n",
            resp->schema.ref_name, resp->schema.is_array ? ", out_len" : ""));
        CHECK_IO(fprintf(fp, "      }\n"));
      }
      CHECK_IO(fprintf(fp, "      break;\n"));
    } else {
      /* Map specific error code first */
      CHECK_IO(
          fprintf(fp, "      rc = %d;\n", mapped_err_code(atoi(resp->code))));

      /* Then attempt to parse global ApiError */
      CHECK_IO(fprintf(fp, "      if (res->body && api_error) {\n"));
      CHECK_IO(fprintf(
          fp,
          "        ApiError_from_json((const char*)res->body, api_error);\n"));
      CHECK_IO(fprintf(fp, "      }\n"));

      CHECK_IO(fprintf(fp, "      break;\n"));
    }
  }
  CHECK_IO(fprintf(fp, "    default:\n"));
  CHECK_IO(fprintf(fp, "      rc = EIO;\n"));
  CHECK_IO(fprintf(fp, "      if (res->body && api_error) {\n"));
  CHECK_IO(fprintf(
      fp, "        ApiError_from_json((const char*)res->body, api_error);\n"));
  CHECK_IO(fprintf(fp, "      }\n"));
  CHECK_IO(fprintf(fp, "      break;\n"));
  CHECK_IO(fprintf(fp, "  }\n\n"));

  /* --- 9. Cleanup --- */
  CHECK_IO(fprintf(fp, "cleanup:\n"));
  if (op->req_body.ref_name && op->req_body.content_type &&
      strcmp(op->req_body.content_type, "application/json") == 0) {
    CHECK_IO(fprintf(fp, "  if (req_json) free(req_json);\n"));
  }
  if (query_exists) {
    CHECK_IO(fprintf(fp, "  if (path_str) free(path_str);\n"));
    CHECK_IO(fprintf(fp, "  if (query_str) free(query_str);\n"));
    CHECK_IO(fprintf(fp, "  url_query_free(&qp);\n"));
  }
  CHECK_IO(fprintf(fp, "  http_request_free(&req);\n"));
  CHECK_IO(fprintf(fp, "  if (res) { http_response_free(res); free(res); }\n"));
  CHECK_IO(fprintf(fp, "  return rc;\n}\n"));

  return 0;
}
