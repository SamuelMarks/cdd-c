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

#if defined(_MSC_VER)
#define SNPRINTF _snprintf
#else
/** @brief SNPRINTF macro */
#define SNPRINTF snprintf
#endif

/**
 * @brief Generates a C HTTP server implementation and test stub from an OpenAPI
 * specification.
 */
cdd_c_error_t
openapi_server_generate(const struct OpenAPI_Spec *spec,
                        const struct OpenApiClientConfig *config) {
  char path[1024];
  FILE *fp = NULL;
  cdd_code_writer_t w;
  size_t i, j, k;

  if (!spec || !config || !config->filename_base)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  {
    cdd_c_error_t rc;
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

  cdd_code_writer_init_stream(&w, fp);

  cdd_code_writer_write(
      &w, "/* Generated Server from OpenAPI Specification */\n\n");
  cdd_code_writer_write(&w, "#include <stdio.h>\n");
  cdd_code_writer_write(&w, "#include <stdlib.h>\n");
  cdd_code_writer_write(&w, "#include <string.h>\n");
  cdd_code_writer_write(&w, "#include \"cdd_c_error.h\"\n");
  cdd_code_writer_write(&w, "#include <c_rest_request.h>\n");
  cdd_code_writer_write(&w, "#include <c_rest_response.h>\n");
  cdd_code_writer_write(&w, "#include <c_rest_router.h>\n\n");

  /* Print Server info for documentation */
  cdd_code_writer_printf(&w, "/* API Title: %s */\n",
                         spec->info.title ? spec->info.title : "API");
  cdd_code_writer_printf(&w, "/* API Version: %s */\n",
                         spec->info.version ? spec->info.version : "1.0");
  if (spec->info.description)
    cdd_code_writer_printf(&w, "/* Description: %s */\n",
                           spec->info.description);
  if (spec->info.contact.name)
    cdd_code_writer_printf(&w, "/* Contact: %s */\n", spec->info.contact.name);
  if (spec->info.license.name)
    cdd_code_writer_printf(&w, "/* License: %s */\n", spec->info.license.name);

  if (spec->n_servers > 0) {
    cdd_code_writer_printf(&w, "/* Base URL: %s */\n",
                           spec->servers[0].url ? spec->servers[0].url : "");
  }

  /* c-orm Database integration */
  cdd_code_writer_write(&w, "#include \"c_orm_db.h\"\n");
  cdd_code_writer_write(&w, "#include \"c_orm_api.h\"\n");
  cdd_code_writer_write(&w, "static c_orm_db_t *db_conn = NULL;\n");
  cdd_code_writer_write(&w, "static int g_use_concrete_daos = 0;\n");
  cdd_code_writer_write(&w, "static int g_is_ephemeral = 0;\n");
  cdd_code_writer_write(&w, "static int g_seed_active = 0;\n");
  cdd_code_writer_write(&w, "static int g_strict_validation = 0;\n");
  cdd_code_writer_write(&w, "static int g_enforce_auth = 0;\n");
  cdd_code_writer_write(&w, "static int g_start_auth_server = 0;\n\n");
  cdd_code_writer_write(
      &w, "static cdd_c_error_t resolve_daos(int is_ephemeral, const char "
          "*db_url) {\n"
          "    g_is_ephemeral = is_ephemeral;\n"
          "    g_use_concrete_daos = (is_ephemeral || (db_url && db_url[0] != "
          "'\\0')) ? 1 : 0;\n"
          "    return CDD_C_SUCCESS;\n}\n\n"
          "static cdd_c_error_t seed_database(void) {\n"
          "    if (!g_use_concrete_daos) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "    return CDD_C_SUCCESS;\n}\n\n");
  cdd_code_writer_write(
      &w, "static cdd_c_error_t cors_middleware(struct c_rest_request *req, "
          "struct c_rest_response *res) {\n"
          "    (void)req;\n"
          "    if (res) res->status_code = 204;\n"
          "    return CDD_C_SUCCESS;\n}\n\n"
          "static cdd_c_error_t strict_validation_middleware(struct "
          "c_rest_request *req, struct c_rest_response *res) {\n"
          "    (void)req;\n"
          "    if (g_strict_validation && res) res->status_code = 400;\n"
          "    return CDD_C_SUCCESS;\n}\n\n");
  cdd_code_writer_write(
      &w, "static cdd_c_error_t auth_middleware(struct c_rest_request *req, "
          "struct c_rest_response *res) {\n"
          "    (void)req;\n"
          "    if (g_enforce_auth && res) res->status_code = 401;\n"
          "    return CDD_C_SUCCESS;\n}\n\n"
          "static cdd_c_error_t handle_auth_idp(struct c_rest_request *req, "
          "struct c_rest_response *res, void *user_data) {\n"
          "    (void)req; (void)user_data;\n"
          "    if (res) res->status_code = 200;\n"
          "    return CDD_C_SUCCESS;\n}\n\n");
  cdd_code_writer_write(
      &w,
      "static cdd_c_error_t handle_mock_trigger_webhook(struct c_rest_request "
      "*req, struct c_rest_response *res, void *user_data) {\n"
      "    (void)req; (void)user_data;\n"
      "    if (res) res->status_code = 200;\n"
      "    return CDD_C_SUCCESS;\n}\n\n"
      "static cdd_c_error_t init_db(void) {\n");
  cdd_code_writer_write(
      &w, "    /* Initialize your c-orm database connection here */\n");
  cdd_code_writer_write(
      &w,
      "    db_conn = NULL; /* c_orm_sqlite_open(\"oauth.db\", &db_conn); */\n");
  cdd_code_writer_write(&w, "    return CDD_C_SUCCESS;\n}\n\n");

  /* Route handlers */
  for (i = 0; i < spec->n_paths; i++) {
    for (j = 0; j < spec->paths[i].n_operations; j++) {
      const struct OpenAPI_Operation *op = &spec->paths[i].operations[j];
      const char *opId = op->operation_id;
      if (opId) {
        cdd_code_writer_write(&w, "/**\n");
        cdd_code_writer_printf(&w, " * @brief %s handler\n",
                               op->summary ? op->summary : opId);
        if (op->description)
          cdd_code_writer_printf(&w, " * %s\n", op->description);

        for (k = 0; k < op->n_parameters; k++) {
          cdd_code_writer_printf(
              &w, " * @param %s (%s) %s\n", op->parameters[k].name,
              op->parameters[k].in == OA_PARAM_IN_QUERY ? "query" : "header",
              op->parameters[k].description ? op->parameters[k].description
                                            : "");
        }

        cdd_code_writer_write(&w, " * \\return HTTP Status Code\n");
        cdd_code_writer_write(&w, " */\n");
        cdd_code_writer_printf(
            &w,
            "static cdd_c_error_t handle_%s(struct c_rest_request *req, struct "
            "c_rest_response *res, void "
            "*user_data) {\n",
            opId);
        cdd_code_writer_printf(
            &w,
            "    const char *resp = \"{\\\"status\\\": \\\"%s called\\\"}\";\n",
            opId);
        cdd_code_writer_write(&w, "    (void)user_data;\n");
        cdd_code_writer_write(&w, "    (void)req;\n");
        cdd_code_writer_write(&w, "    (void)res;\n");

        if (op->deprecated) {
          cdd_code_writer_write(&w,
                                "    /* Note: Operation is deprecated */\n");
        }

        if (op->req_body.content_schema) {
          cdd_code_writer_printf(
              &w, "    /* Expecting requestBody schema: %s */\n",
              op->req_body.ref ? op->req_body.ref : "inline");
        }

        if (op->n_req_body_media_types > 0) {
          size_t m;
          for (m = 0; m < op->n_req_body_media_types; m++) {
            if (op->req_body_media_types[m].name &&
                strcmp(op->req_body_media_types[m].name,
                       "application/x-www-form-urlencoded") == 0) {
              cdd_code_writer_write(
                  &w, "    /* Parse form-urlencoded request body */\n");
              cdd_code_writer_write(&w, "    {\n");
              cdd_code_writer_write(
                  &w,
                  "        struct c_rest_urlencoded_data urlencoded_data;\n");
              if (op->req_body.ref_name) {
                cdd_code_writer_printf(&w, "        struct %s req_struct;\n",
                                       op->req_body.ref_name);
                cdd_code_writer_write(
                    &w,
                    "        memset(&req_struct, 0, sizeof(req_struct));\n");
              }
              cdd_code_writer_write(
                  &w, "        /* if (c_rest_request_parse_urlencoded(req, "
                      "&urlencoded_data) == 0) { ... } */\n");
              cdd_code_writer_write(&w, "    }\n");
              break;
            }
          }
        }

        if (op->n_responses > 0) {
          cdd_code_writer_printf(&w, "    /* Responses configured: %lu */\n",
                                 (unsigned long)op->n_responses);
        }

        if (op->n_callbacks > 0) {
          cdd_code_writer_printf(&w, "    /* Callbacks configured: %lu */\n",
                                 (unsigned long)op->n_callbacks);
        }
        {
          cdd_c_error_t rc = codegen_security_write_server_apply(fp, op, spec);
          if (rc != CDD_C_SUCCESS) {
            cdd_code_writer_free(&w);
            fclose(fp);
            return rc;
          }
        }

        cdd_code_writer_write(
            &w, "    /* c_rest_response_set_status(res, 200); */\n");
        cdd_code_writer_write(
            &w,
            "    /* c_rest_response_set_body(res, resp, strlen(resp)); */\n");
        cdd_code_writer_write(&w, "    return CDD_C_SUCCESS;\n}\n\n");
      }
    }
  }

  cdd_code_writer_write(
      &w, "/**\n"
          " * @brief Auto-generated code from OpenAPI specification\n"
          " */\n"
          "int main(int argc, char **argv) {\n");
  cdd_code_writer_write(&w, "    const char *options[15];\n");
  cdd_code_writer_write(&w, "    struct c_rest_context *ctx = NULL;\n");
  cdd_code_writer_write(&w, "    int i;\n");
  cdd_code_writer_write(&w, "    const char *port_str = \"8080\";\n");
  cdd_code_writer_write(&w, "    const char *db_path = \"oauth.db\";\n");
  cdd_code_writer_write(&w, "    const char *cert_path = NULL;\n");
  cdd_code_writer_write(&w, "    const char *key_path = NULL;\n");
  cdd_code_writer_write(&w, "    const char *threads_str = \"4\";\n");
  cdd_code_writer_write(&w, "    int opt_idx = 0;\n\n");
  cdd_code_writer_write(&w, "    for (i = 1; i < argc; i++) {\n");
  cdd_code_writer_write(
      &w, "        if (strcmp(argv[i], \"--port\") == 0 && i + 1 < argc) {\n");
  cdd_code_writer_write(&w, "            port_str = argv[++i];\n");
  cdd_code_writer_write(
      &w, "        } else if (strcmp(argv[i], \"--db-path\") == 0 "
          "&& i + 1 < argc) {\n");
  cdd_code_writer_write(&w, "            db_path = argv[++i];\n");
  cdd_code_writer_write(&w, "        } else if (strcmp(argv[i], \"--cert\") == "
                            "0 && i + 1 < argc) {\n");
  cdd_code_writer_write(&w, "            cert_path = argv[++i];\n");
  cdd_code_writer_write(&w, "        } else if (strcmp(argv[i], \"--key\") == "
                            "0 && i + 1 < argc) {\n");
  cdd_code_writer_write(&w, "            key_path = argv[++i];\n");
  cdd_code_writer_write(&w, "        } else if (strcmp(argv[i], \"--threads\") "
                            "== 0 && i + 1 < argc) {\n");
  cdd_code_writer_write(&w, "            threads_str = argv[++i];\n");
  cdd_code_writer_write(
      &w, "        } else if (strcmp(argv[i], \"--ephemeral\") == 0) {\n"
          "            g_is_ephemeral = 1;\n"
          "            db_path = \":memory:\";\n"
          "        } else if (strcmp(argv[i], \"--seed\") == 0) {\n"
          "            g_seed_active = 1;\n");
  cdd_code_writer_write(
      &w,
      "        } else if (strcmp(argv[i], \"--strict-validation\") == 0) {\n"
      "            g_strict_validation = 1;\n"
      "        } else if (strcmp(argv[i], \"--enforce-auth\") == 0) {\n"
      "            g_enforce_auth = 1;\n"
      "        } else if (strcmp(argv[i], \"--start-auth-server\") == 0) {\n"
      "            g_start_auth_server = 1;\n"
      "        }\n");
  cdd_code_writer_write(&w, "    }\n\n");
  cdd_code_writer_write(&w,
                        "    { cdd_c_error_t rc = resolve_daos(g_is_ephemeral, "
                        "db_path); if (rc != CDD_C_SUCCESS) return rc; }\n");
  cdd_code_writer_write(&w, "    if (g_use_concrete_daos) {\n"
                            "        cdd_c_error_t rc = init_db();\n"
                            "        if (rc != CDD_C_SUCCESS) return rc;\n"
                            "        if (g_seed_active) {\n"
                            "            rc = seed_database();\n"
                            "            if (rc != CDD_C_SUCCESS) return rc;\n"
                            "        }\n"
                            "    }\n\n");
  cdd_code_writer_write(&w, "    options[opt_idx++] = \"document_root\";\n");
  cdd_code_writer_write(&w, "    options[opt_idx++] = \".\";\n");
  cdd_code_writer_write(&w, "    options[opt_idx++] = \"listening_ports\";\n");
  cdd_code_writer_write(&w, "    options[opt_idx++] = port_str;\n");
  cdd_code_writer_write(&w, "    options[opt_idx++] = \"num_threads\";\n");
  cdd_code_writer_write(&w, "    options[opt_idx++] = threads_str;\n");
  cdd_code_writer_write(&w, "    if (cert_path && key_path) {\n");
  cdd_code_writer_write(&w,
                        "        options[opt_idx++] = \"ssl_certificate\";\n");
  cdd_code_writer_write(&w, "        options[opt_idx++] = cert_path;\n");
  cdd_code_writer_write(&w, "    }\n");
  cdd_code_writer_write(&w, "    options[opt_idx] = 0;\n\n");

  cdd_code_writer_write(&w, "    { cdd_c_error_t rc = init_db(); if(rc != "
                            "CDD_C_SUCCESS) return rc; }\n\n");

  cdd_code_writer_write(&w, "    cdd_c_error_t rc;\n");
  cdd_code_writer_write(&w,
                        "    rc = c_rest_init(C_REST_MODALITY_SYNC, &ctx);\n");
  cdd_code_writer_write(&w, "    if (rc != CDD_C_SUCCESS) {\n");
  cdd_code_writer_write(
      &w, "        fprintf(stderr, \"Failed to start c-rest server.\\n\");\n");
  cdd_code_writer_write(&w, "        return rc;\n");
  cdd_code_writer_write(&w, "    }\n\n");

  cdd_code_writer_write(&w, "    {\n");
  cdd_code_writer_write(&w, "        c_rest_router *router = NULL;\n");
  cdd_code_writer_write(&w, "        rc = c_rest_router_init(&router);\n");
  cdd_code_writer_write(&w, "        if (rc == CDD_C_SUCCESS) {\n");

  /* Register MCP endpoints */
  cdd_code_writer_write(&w,
                        "    /* MCP Transports: Server-Sent Events (sse) */\n");
  cdd_code_writer_write(
      &w,
      "static cdd_c_error_t handle_mcp_sse(struct c_rest_request *req, struct "
      "c_rest_response *res, void *user_data) {\n");
  cdd_code_writer_write(&w, "    (void)req;\n");
  cdd_code_writer_write(&w, "    (void)user_data;\n");
  cdd_code_writer_write(&w, "    res->status_code = 200;\n");
  cdd_code_writer_write(&w, "    /* c_rest_response_add_header is pseudo; just "
                            "printing headers manually or using standard HTTP "
                            "framework */\n");
  cdd_code_writer_write(&w, "    return CDD_C_SUCCESS;\n}\n\n");
  cdd_code_writer_write(
      &w, "static cdd_c_error_t handle_mcp_message(struct c_rest_request *req, "
          "struct c_rest_response *res, void *user_data) {\n");
  cdd_code_writer_write(&w, "    (void)req;\n");
  cdd_code_writer_write(&w, "    (void)user_data;\n");
  cdd_code_writer_write(&w, "    res->status_code = 202;\n");
  cdd_code_writer_write(&w, "    return CDD_C_SUCCESS;\n}\n\n");
  cdd_code_writer_write(&w,
                        "            /* MCP SSE Endpoint Registration */\n");
  cdd_code_writer_write(&w, "            c_rest_router_add(router, \"GET\", "
                            "\"/mcp/sse\", NULL, NULL);  \n");
  cdd_code_writer_write(
      &w, "            c_rest_router_add(router, \"POST\", \"/mcp/message\", "
          "NULL, NULL);  \n\n");

  /* Register handlers */
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
        cdd_code_writer_printf(&w,
                               "            c_rest_router_add(router, \"%s\", "
                               "\"%s\", handle_%s, NULL);\n",
                               method, spec->paths[i].route, opId);
      }
    }
  }

  cdd_code_writer_write(&w,
                        "            /* Bind Router to c-rest-framework */\n");
  cdd_code_writer_write(&w,
                        "            rc = c_rest_set_router(ctx, router);\n");
  cdd_code_writer_write(&w,
                        "            if (rc != CDD_C_SUCCESS) return rc;\n");
  cdd_code_writer_write(&w, "        }\n");
  cdd_code_writer_write(&w, "    }\n\n");

  cdd_code_writer_write(
      &w, "    printf(\"Server listening... Press enter to exit.\\n\");\n");
  cdd_code_writer_write(&w, "    rc = c_rest_run(ctx);\n");
  cdd_code_writer_write(&w, "    if (rc != CDD_C_SUCCESS) return rc;\n");
  cdd_code_writer_write(&w, "    getchar();\n");
  cdd_code_writer_write(&w, "    rc = c_rest_stop(ctx);\n");
  cdd_code_writer_write(&w, "    if (rc != CDD_C_SUCCESS) return rc;\n");
  cdd_code_writer_write(&w, "    rc = c_rest_destroy(ctx);\n");
  cdd_code_writer_write(&w, "    if (rc != CDD_C_SUCCESS) return rc;\n");
  cdd_code_writer_write(&w, "    return CDD_C_SUCCESS;\n");
  cdd_code_writer_write(&w, "}\n");

  cdd_code_writer_free(&w);
  fclose(fp);

  /* Test Stub Generation */
  {
    char test_path[1024];
    FILE *fp_test = NULL;
    CDD_SNPRINTF(test_path, sizeof(test_path), "test_%s_server.c",
                 config->filename_base);
#if defined(_MSC_VER)
    if (fopen_s(&fp_test, test_path, "w") != 0)
      fp_test = NULL;
#else
    fp_test = fopen(test_path, "w");
#endif
    if (fp_test) {
      cdd_code_writer_t tw;
      cdd_code_writer_init_stream(&tw, fp_test);
      cdd_code_writer_write(
          &tw,
          "/* Auto-generated greatest.h test stub for server endpoints */\n\n");
      cdd_code_writer_write(&tw, "#include <stdlib.h>\n");
      cdd_code_writer_write(&tw, "#include <string.h>\n");
      cdd_code_writer_write(&tw, "#include <greatest.h>\n");
      cdd_code_writer_write(&tw, "#include <c_rest_request.h>\n");
      cdd_code_writer_write(&tw, "#include <c_rest_response.h>\n\n ");

      for (i = 0; i < spec->n_paths; i++) {
        for (j = 0; j < spec->paths[i].n_operations; j++) {
          const struct OpenAPI_Operation *op = &spec->paths[i].operations[j];
          const char *opId = op->operation_id;
          if (opId) {
            cdd_code_writer_printf(&tw, "TEST test_server_handle_%s(void) {\n",
                                   opId);
            cdd_code_writer_write(&tw, "  struct c_rest_request req;\n");
            cdd_code_writer_write(&tw, "  struct c_rest_response res;\n");
            cdd_code_writer_write(&tw, "  cdd_c_error_t status;\n");
            cdd_code_writer_write(&tw, "  memset(&req, 0, sizeof(req));\n");
            cdd_code_writer_write(&tw, "  memset(&res, 0, sizeof(res));\n");
            cdd_code_writer_printf(
                &tw, "  status = handle_%s(&req, &res, NULL);\n", opId);
            cdd_code_writer_write(&tw, "  ASSERT_EQ(CDD_C_SUCCESS, status);\n");
            cdd_code_writer_write(&tw, "  PASS();\n");
            cdd_code_writer_write(&tw, "}\n\n");
          }
        }
      }

      cdd_code_writer_write(&tw, "SUITE(server_endpoints_suite) {\n");
      for (i = 0; i < spec->n_paths; i++) {
        for (j = 0; j < spec->paths[i].n_operations; j++) {
          if (spec->paths[i].operations[j].operation_id) {
            cdd_code_writer_printf(&tw, "  RUN_TEST(test_server_handle_%s);\n",
                                   spec->paths[i].operations[j].operation_id);
          }
        }
      }
      cdd_code_writer_write(&tw, "}\n");
      cdd_code_writer_free(&tw);
      fclose(fp_test);
    }
  }

  return CDD_C_SUCCESS;
}
