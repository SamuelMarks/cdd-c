/* Generated Server from OpenAPI Specification */

#include <c_rest_request.h>
#include <c_rest_response.h>
#include <c_rest_router.h>
#include <civetweb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* API Title: A */
/* API Version: 1 */
/* Base URL:  */
#include "c_orm_api.h"
#include "c_orm_db.h"
static c_orm_db_t *db_conn = NULL;
/**
 * @brief Auto-generated code from OpenAPI specification
 */
static cdd_c_error_t init_db(void) {
  /* Initialize your c-orm database connection here */
  db_conn = NULL; /* c_orm_sqlite_open("oauth.db", &db_conn); */
  return CDD_C_SUCCESS;
}

/**
 * @brief MySummary handler
 * @param p1 (header)
 * @param p2 (query)
 * \return HTTP Status Code
 */
static int handle_doGetBranches(struct c_rest_request *req,
                                struct c_rest_response *res, void *user_data) {
  const char *resp = "{\"status\": \"doGetBranches called\"}";
  (void)user_data;
  (void)req;
  (void)res;
  /* Expecting requestBody schema: inline */
  /* c_rest_response_set_status(res, 200); */
  /* c_rest_response_set_body(res, resp, strlen(resp)); */
  return 200;
  return CDD_C_SUCCESS;
}

/**
 * @brief Auto-generated code from OpenAPI specification
 */
int main(int argc, char **argv) {
  const char *options[15];
  struct mg_callbacks callbacks;
  struct mg_context *ctx;
  int i;
  const char *port_str = "8080";
  const char *db_path = "oauth.db";
  const char *cert_path = NULL;
  const char *key_path = NULL;
  const char *threads_str = "4";
  int opt_idx = 0;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
      port_str = argv[++i];
    } else if (strcmp(argv[i], "--db-path") == 0 && i + 1 < argc) {
      db_path = argv[++i];
    } else if (strcmp(argv[i], "--cert") == 0 && i + 1 < argc) {
      cert_path = argv[++i];
    } else if (strcmp(argv[i], "--key") == 0 && i + 1 < argc) {
      key_path = argv[++i];
    } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
      threads_str = argv[++i];
    }
  }

  (void)db_path; /* Will be used in init_db() */

  options[opt_idx++] = "document_root";
  options[opt_idx++] = ".";
  options[opt_idx++] = "listening_ports";
  options[opt_idx++] = port_str;
  options[opt_idx++] = "num_threads";
  options[opt_idx++] = threads_str;
  if (cert_path && key_path) {
    options[opt_idx++] = "ssl_certificate";
    options[opt_idx++] = cert_path;
  }
  options[opt_idx] = 0;

  {
    cdd_c_error_t rc = init_db();
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  memset(&callbacks, 0, sizeof(callbacks));
  ctx = mg_start(&callbacks, 0, options);
  if (ctx == NULL) {
    fprintf(stderr, "Failed to start CivetWeb server.\n");
    return CDD_C_ERROR_UNKNOWN;
  }

  {
    c_rest_router *router = NULL;
    if (c_rest_router_init(&router) == 0) {
      /* MCP Transports: Server-Sent Events (sse) */
      static int handle_mcp_sse(struct c_rest_request * req,
                                struct c_rest_response * res, void *user_data) {
        (void)req;
        (void)user_data;
        res->status_code = 200;
        /* c_rest_response_add_header is pseudo; just printing headers manually
         * or using standard HTTP framework */
        return res->status_code;
        return CDD_C_SUCCESS;
      }

      static int handle_mcp_message(struct c_rest_request * req,
                                    struct c_rest_response * res,
                                    void *user_data) {
        (void)req;
        (void)user_data;
        res->status_code = 202;
        return res->status_code;
        return CDD_C_SUCCESS;
      }

      /* MCP SSE Endpoint Registration */
      c_rest_router_add(router, "GET", "/mcp/sse", NULL, NULL);
      c_rest_router_add(router, "POST", "/mcp/message", NULL, NULL);

      c_rest_router_add(router, "GET", "/test/route", handle_doGetBranches,
                        NULL);
      /* TODO: bind router to CivetWeb (e.g. via c_rest dispatch middleware) */
      c_rest_router_destroy(router);
    }
  }

  printf("Server listening on port 8080... Press enter to exit.\n");
  getchar();
  mg_stop(ctx);
  return CDD_C_SUCCESS;
}
