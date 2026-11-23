/**
 * @file server_gen.c
 * @brief Implementation of server code generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "server_gen.h"
#include "c_cdd/code_writer.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "functions/parse/fs.h"
#include "routes/emit/security.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Emits the `greatest.h` server endpoint test stub file when writable.
 *
 * @param[in] spec Parsed OpenAPI specification.
 * @param[in] config Client/Server generator configuration.
 * @return CDD_C_SUCCESS on success, or error code on emission failure.
 */
static cdd_c_error_t
emit_server_test_stub(const struct OpenAPI_Spec *spec,
                      const struct OpenApiClientConfig *config) {
  char test_path[1024];
  FILE *fp_test = NULL;
  cdd_code_writer_t tw;
  size_t i, j;
  cdd_c_error_t rc;

#if defined(_MSC_VER)
  sprintf_s(test_path, sizeof(test_path), "test_%s_server.c",
            config->filename_base);
  if (fopen_s(&fp_test, test_path, "w") != 0)
    fp_test = NULL;
#else
  CDD_SNPRINTF(test_path, sizeof(test_path), "test_%s_server.c",
               config->filename_base);
  fp_test = fopen(test_path, "w");
#endif
  if (!fp_test)
    return CDD_C_SUCCESS;

  memset(&tw, 0, sizeof(tw));
  rc = cdd_code_writer_init_stream(&tw, fp_test);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &tw, "/* Auto-generated greatest.h test stub for server endpoints */\n\n"
           "#include <stdlib.h>\n"
           "#include <string.h>\n"
           "#include <greatest.h>\n"
           "#include <c_rest_request.h>\n"
           "#include <c_rest_response.h>\n\n ");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (i = 0; i < spec->n_paths; i++) {
    for (j = 0; j < spec->paths[i].n_operations; j++) {
      const struct OpenAPI_Operation *op = &spec->paths[i].operations[j];
      const char *opId = op->operation_id;
      if (opId) {
        rc = cdd_code_writer_printf(&tw,
                                    "TEST test_server_handle_%s(void) {\n"
                                    "  struct c_rest_request req;\n"
                                    "  struct c_rest_response res;\n"
                                    "  cdd_c_error_t status;\n"
                                    "  memset(&req, 0, sizeof(req));\n"
                                    "  memset(&res, 0, sizeof(res));\n"
                                    "  status = handle_%s(&req, &res, NULL);\n"
                                    "  ASSERT_EQ(CDD_C_SUCCESS, status);\n"
                                    "  PASS();\n"
                                    "}\n\n",
                                    opId, opId);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
  }

  rc = cdd_code_writer_write(&tw, "SUITE(server_endpoints_suite) {\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  for (i = 0; i < spec->n_paths; i++) {
    for (j = 0; j < spec->paths[i].n_operations; j++) {
      if (spec->paths[i].operations[j].operation_id) {
        rc = cdd_code_writer_printf(&tw, "  RUN_TEST(test_server_handle_%s);\n",
                                    spec->paths[i].operations[j].operation_id);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
  }
  rc = cdd_code_writer_write(&tw, "}\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

cleanup:
  fclose(fp_test);
  return cdd_c_keep_err(rc, cdd_code_writer_free(&tw));
}

/**
 * @brief Generates a standalone C HTTP server implementation from an OpenAPI
 * specification.
 *
 * @param[in] spec The parsed OpenAPI specification.
 * @param[in] config Client/Server generator configuration.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t
openapi_server_generate(const struct OpenAPI_Spec *spec,
                        const struct OpenApiClientConfig *config) {
  char path[1024];
  FILE *fp = NULL;
  cdd_code_writer_t w;
  size_t i, j, k;
  cdd_c_error_t rc;

  if (!spec || !config || !config->filename_base)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  {
    char *dir_name = NULL, *base_name = NULL;
    char *src_dir = (char *)C_CDD_MALLOC(512);
    if (!src_dir)
      return CDD_C_ERROR_MEMORY;
    rc = get_dirname(config->filename_base, &dir_name);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(src_dir);
      return rc;
    }
    rc = get_basename(config->filename_base, &base_name);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(dir_name);
      C_CDD_FREE(src_dir);
      return rc;
    }
#if defined(_MSC_VER)
    sprintf_s(src_dir, 512, "%s/src", dir_name);
#else
    CDD_SNPRINTF(src_dir, 512, "%s/src", dir_name);
#endif
    rc = makedirs(src_dir);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(src_dir);
      C_CDD_FREE(dir_name);
      C_CDD_FREE(base_name);
      return rc;
    }
    CDD_SNPRINTF(path, 1024, "%s/%s_server.c", src_dir, base_name);
    C_CDD_FREE(src_dir);
    C_CDD_FREE(dir_name);
    C_CDD_FREE(base_name);
  }
#if defined(_MSC_VER)
  if (fopen_s(&fp, path, "w") != 0)
    fp = NULL;
#else
  fp = fopen(path, "w");
#endif
  if (!fp) {
    return CDD_C_ERROR_IO;
  }

  memset(&w, 0, sizeof(w));
  rc = cdd_code_writer_init_stream(&w, fp);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w, "/* Generated Server from OpenAPI Specification */\n\n"
          "#include <stdio.h>\n"
          "#include <stdlib.h>\n"
          "#include <string.h>\n"
          "#include \"cdd_c_error.h\"\n"
          "#include <c_rest_request.h>\n"
          "#include <c_rest_response.h>\n"
          "#include <c_rest_router.h>\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_printf(&w,
                              "/* API Title: %s */\n"
                              "/* API Version: %s */\n",
                              spec->info.title ? spec->info.title : "API",
                              spec->info.version ? spec->info.version : "1.0");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  if (spec->info.description) {
    rc = cdd_code_writer_printf(&w, "/* Description: %s */\n",
                                spec->info.description);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (spec->info.contact.name) {
    rc = cdd_code_writer_printf(&w, "/* Contact: %s */\n",
                                spec->info.contact.name);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (spec->info.license.name) {
    rc = cdd_code_writer_printf(&w, "/* License: %s */\n",
                                spec->info.license.name);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (spec->n_servers > 0) {
    rc = cdd_code_writer_printf(&w, "/* Base URL: %s */\n",
                                spec->servers[0].url ? spec->servers[0].url
                                                     : "");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  rc = cdd_code_writer_write(&w, "#include \"c_orm_db.h\"\n"
                                 "#include \"c_orm_api.h\"\n"
                                 "static c_orm_db_t *db_conn = NULL;\n"
                                 "static int g_use_concrete_daos = 0;\n"
                                 "static int g_is_ephemeral = 0;\n"
                                 "static int g_seed_active = 0;\n"
                                 "static int g_strict_validation = 0;\n"
                                 "static int g_enforce_auth = 0;\n"
                                 "static int g_start_auth_server = 0;\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w, "static cdd_c_error_t resolve_daos(int is_ephemeral, const char "
          "*db_url) {\n"
          "    g_is_ephemeral = is_ephemeral;\n"
          "    g_use_concrete_daos = (is_ephemeral || (db_url && db_url[0] != "
          "'\\0')) ? 1 : 0;\n"
          "    return CDD_C_SUCCESS;\n}\n\n"
          "static cdd_c_error_t seed_database(void) {\n"
          "    if (!g_use_concrete_daos) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "    return CDD_C_SUCCESS;\n}\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w, "static cdd_c_error_t cors_middleware(struct c_rest_request *req, "
          "struct c_rest_response *res) {\n"
          "    if (!req) {}\n"
          "    if (res) res->status_code = 204;\n"
          "    return CDD_C_SUCCESS;\n}\n\n"
          "static cdd_c_error_t strict_validation_middleware(struct "
          "c_rest_request *req, struct c_rest_response *res) {\n"
          "    if (!req) {}\n"
          "    if (g_strict_validation && res) res->status_code = 400;\n"
          "    return CDD_C_SUCCESS;\n}\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w, "static cdd_c_error_t auth_middleware(struct c_rest_request *req, "
          "struct c_rest_response *res) {\n"
          "    if (!req) {}\n"
          "    if (g_enforce_auth && res) res->status_code = 401;\n"
          "    return CDD_C_SUCCESS;\n}\n\n"
          "static cdd_c_error_t handle_auth_idp(struct c_rest_request *req, "
          "struct c_rest_response *res, void *user_data) {\n"
          "    if (!req && !user_data) {}\n"
          "    if (res) res->status_code = 200;\n"
          "    return CDD_C_SUCCESS;\n}\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w, "static cdd_c_error_t handle_mock_trigger_webhook(struct "
          "c_rest_request *req, struct c_rest_response *res, void *user_data) "
          "{\n"
          "    if (!req && !user_data) {}\n"
          "    if (res) res->status_code = 200;\n"
          "    return CDD_C_SUCCESS;\n}\n\n"
          "static cdd_c_error_t init_db(void) {\n"
          "    db_conn = NULL;\n"
          "    return CDD_C_SUCCESS;\n}\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (i = 0; i < spec->n_paths; i++) {
    for (j = 0; j < spec->paths[i].n_operations; j++) {
      const struct OpenAPI_Operation *op = &spec->paths[i].operations[j];
      const char *opId = op->operation_id;
      if (opId) {
        rc = cdd_code_writer_printf(&w, "/**\n * @brief %s handler\n",
                                    op->summary ? op->summary : opId);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        if (op->description) {
          rc = cdd_code_writer_printf(&w, " * %s\n", op->description);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        for (k = 0; k < op->n_parameters; k++) {
          rc = cdd_code_writer_printf(
              &w, " * @param %s (%s) %s\n", op->parameters[k].name,
              op->parameters[k].in == OA_PARAM_IN_QUERY ? "query" : "header",
              op->parameters[k].description ? op->parameters[k].description
                                            : "");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        rc = cdd_code_writer_printf(
            &w,
            " * \\return HTTP Status Code\n */\n"
            "static cdd_c_error_t handle_%s(struct c_rest_request *req, "
            "struct c_rest_response *res, void *user_data) {\n"
            "    const char *resp = \"{\\\"status\\\": \\\"%s called\\\"}\";\n"
            "    if (!user_data) {}\n"
            "    if (!req) {}\n"
            "    if (!res && !resp) {}\n",
            opId, opId);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;

        if (op->deprecated) {
          rc = cdd_code_writer_write(
              &w, "    /* Note: Operation is deprecated */\n");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        if (op->req_body.content_schema) {
          rc = cdd_code_writer_printf(
              &w, "    /* Expecting requestBody schema: %s */\n",
              op->req_body.ref ? op->req_body.ref : "inline");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        if (op->n_req_body_media_types > 0) {
          size_t m;
          for (m = 0; m < op->n_req_body_media_types; m++) {
            if (op->req_body_media_types[m].name &&
                strcmp(op->req_body_media_types[m].name,
                       "application/x-www-form-urlencoded") == 0) {
              rc = cdd_code_writer_write(
                  &w,
                  "    /* Parse form-urlencoded request body */\n"
                  "    {\n"
                  "        struct c_rest_urlencoded_data urlencoded_data;\n");
              if (rc != CDD_C_SUCCESS)
                goto cleanup;
              if (op->req_body.ref_name) {
                rc = cdd_code_writer_printf(
                    &w,
                    "        struct %s req_struct;\n"
                    "        memset(&req_struct, 0, sizeof(req_struct));\n",
                    op->req_body.ref_name);
                if (rc != CDD_C_SUCCESS)
                  goto cleanup;
              }
              rc = cdd_code_writer_write(
                  &w, "        /* if (c_rest_request_parse_urlencoded(req, "
                      "&urlencoded_data) == 0) { ... } */\n    }\n");
              if (rc != CDD_C_SUCCESS)
                goto cleanup;
              break;
            }
          }
        }
        if (op->n_responses > 0) {
          rc = cdd_code_writer_printf(&w,
                                      "    /* Responses configured: %lu */\n",
                                      (unsigned long)op->n_responses);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        if (op->n_callbacks > 0) {
          rc = cdd_code_writer_printf(&w,
                                      "    /* Callbacks configured: %lu */\n",
                                      (unsigned long)op->n_callbacks);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        rc = codegen_security_write_server_apply(fp, op, spec);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = cdd_code_writer_write(
            &w, "    /* c_rest_response_set_status(res, 200); */\n"
                "    /* c_rest_response_set_body(res, resp, strlen(resp)); */\n"
                "    return CDD_C_SUCCESS;\n}\n\n");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
  }

  rc = cdd_code_writer_write(
      &w, "/**\n"
          " * @brief Auto-generated code from OpenAPI specification\n"
          " */\n"
          "int main(int argc, char **argv) {\n"
          "    const char *options[15];\n"
          "    struct c_rest_context *ctx = NULL;\n"
          "    int i;\n"
          "    const char *port_str = \"8080\";\n"
          "    const char *db_path = \"oauth.db\";\n"
          "    const char *cert_path = NULL;\n"
          "    const char *key_path = NULL;\n"
          "    const char *threads_str = \"4\";\n"
          "    int opt_idx = 0;\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w, "    for (i = 1; i < argc; i++) {\n"
          "        if (strcmp(argv[i], \"--port\") == 0 && i + 1 < argc) {\n"
          "            port_str = argv[++i];\n"
          "        } else if (strcmp(argv[i], \"--db-path\") == 0 && i + 1 < "
          "argc) {\n"
          "            db_path = argv[++i];\n"
          "        } else if (strcmp(argv[i], \"--cert\") == 0 && i + 1 < "
          "argc) {\n"
          "            cert_path = argv[++i];\n"
          "        } else if (strcmp(argv[i], \"--key\") == 0 && i + 1 < "
          "argc) {\n"
          "            key_path = argv[++i];\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w, "        } else if (strcmp(argv[i], \"--threads\") == 0 && i + 1 < "
          "argc) {\n"
          "            threads_str = argv[++i];\n"
          "        } else if (strcmp(argv[i], \"--ephemeral\") == 0) {\n"
          "            g_is_ephemeral = 1;\n"
          "            db_path = \":memory:\";\n"
          "        } else if (strcmp(argv[i], \"--seed\") == 0) {\n"
          "            g_seed_active = 1;\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w,
      "        } else if (strcmp(argv[i], \"--strict-validation\") == 0) {\n"
      "            g_strict_validation = 1;\n"
      "        } else if (strcmp(argv[i], \"--enforce-auth\") == 0) {\n"
      "            g_enforce_auth = 1;\n"
      "        } else if (strcmp(argv[i], \"--start-auth-server\") == 0) {\n"
      "            g_start_auth_server = 1;\n"
      "        }\n    }\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w, "    { cdd_c_error_t rc = resolve_daos(g_is_ephemeral, db_path); if "
          "(rc != CDD_C_SUCCESS) return rc; }\n"
          "    if (g_use_concrete_daos) {\n"
          "        cdd_c_error_t rc = init_db();\n"
          "        if (rc != CDD_C_SUCCESS) return rc;\n"
          "        if (g_seed_active) {\n"
          "            rc = seed_database();\n"
          "            if (rc != CDD_C_SUCCESS) return rc;\n"
          "        }\n    }\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w, "    options[opt_idx++] = \"document_root\";\n"
          "    options[opt_idx++] = \".\";\n"
          "    options[opt_idx++] = \"listening_ports\";\n"
          "    options[opt_idx++] = port_str;\n"
          "    options[opt_idx++] = \"num_threads\";\n"
          "    options[opt_idx++] = threads_str;\n"
          "    if (cert_path && key_path) {\n"
          "        options[opt_idx++] = \"ssl_certificate\";\n"
          "        options[opt_idx++] = cert_path;\n"
          "    }\n"
          "    options[opt_idx] = 0;\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w, "    { cdd_c_error_t rc = init_db(); if(rc != CDD_C_SUCCESS) return "
          "rc; }\n\n"
          "    cdd_c_error_t rc;\n"
          "    rc = c_rest_init(C_REST_MODALITY_SYNC, &ctx);\n"
          "    if (rc != CDD_C_SUCCESS) {\n"
          "        fprintf(stderr, \"Failed to start c-rest server.\\n\");\n"
          "        return rc;\n"
          "    }\n\n"
          "    {\n"
          "        c_rest_router *router = NULL;\n"
          "        rc = c_rest_router_init(&router);\n"
          "        if (rc == CDD_C_SUCCESS) {\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w,
      "    /* MCP Transports: Server-Sent Events (sse) */\n"
      "static cdd_c_error_t handle_mcp_sse(struct c_rest_request *req, struct "
      "c_rest_response *res, void *user_data) {\n"
      "    if (!req) {}\n"
      "    if (!user_data) {}\n"
      "    res->status_code = 200;\n"
      "    return CDD_C_SUCCESS;\n}\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_code_writer_write(
      &w,
      "static cdd_c_error_t handle_mcp_message(struct c_rest_request *req, "
      "struct c_rest_response *res, void *user_data) {\n"
      "    if (!req) {}\n"
      "    if (!user_data) {}\n"
      "    res->status_code = 202;\n"
      "    return CDD_C_SUCCESS;\n}\n\n"
      "            /* MCP SSE Endpoint Registration */\n"
      "            c_rest_router_add(router, \"GET\", \"/mcp/sse\", NULL, "
      "NULL);\n"
      "            c_rest_router_add(router, \"POST\", \"/mcp/message\", NULL, "
      "NULL);\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (i = 0; i < spec->n_paths; i++) {
    for (j = 0; j < spec->paths[i].n_operations; j++) {
      const struct OpenAPI_Operation *op = &spec->paths[i].operations[j];
      const char *opId = op->operation_id;
      if (opId) {
        const char *method = "GET";
        if (op->verb == OA_VERB_POST)
          method = "POST";
        else if (op->verb == OA_VERB_PUT)
          method = "PUT";
        else if (op->verb == OA_VERB_DELETE)
          method = "DELETE";
        else if (op->verb == OA_VERB_OPTIONS)
          method = "OPTIONS";
        else if (op->verb == OA_VERB_HEAD)
          method = "HEAD";
        else if (op->verb == OA_VERB_PATCH)
          method = "PATCH";
        else if (op->verb == OA_VERB_TRACE)
          method = "TRACE";
        rc = cdd_code_writer_printf(
            &w,
            "            c_rest_router_add(router, \"%s\", \"%s\", handle_%s, "
            "NULL);\n",
            method, spec->paths[i].route, opId);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
  }

  rc = cdd_code_writer_write(
      &w, "            /* Bind Router to c-rest-framework */\n"
          "            rc = c_rest_set_router(ctx, router);\n"
          "            if (rc != CDD_C_SUCCESS) return rc;\n"
          "        }\n"
          "    }\n\n"
          "    printf(\"Server listening... Press enter to exit.\\n\");\n"
          "    rc = c_rest_run(ctx);\n"
          "    if (rc != CDD_C_SUCCESS) return rc;\n"
          "    getchar();\n"
          "    rc = c_rest_stop(ctx);\n"
          "    if (rc != CDD_C_SUCCESS) return rc;\n"
          "    rc = c_rest_destroy(ctx);\n"
          "    if (rc != CDD_C_SUCCESS) return rc;\n"
          "    return CDD_C_SUCCESS;\n}\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  fclose(fp);
  return cdd_c_keep_err(emit_server_test_stub(spec, config),
                        cdd_code_writer_free(&w));

cleanup:
  fclose(fp);
  return cdd_c_keep_err(rc, cdd_code_writer_free(&w));
}
