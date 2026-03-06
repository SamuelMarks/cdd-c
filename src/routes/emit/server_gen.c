/**
 * @file server_gen.c
 * @brief Implementation of server wrapper generation.
 */

#include "server_gen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int openapi_server_generate(const struct OpenAPI_Spec *spec,
                            const struct OpenApiClientConfig *config) {
  char path[1024];
  FILE *fp;
  size_t i, j;

  snprintf(path, sizeof(path), "%s_server.c", config->filename_base);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  if (fopen_s(&fp, path, "w") != 0)
    fp = NULL;
#else
#if defined(_MSC_VER)
  fopen_s(&fp, path, "w");
#else
  fp = fopen(path, "w");
#endif
#endif
  if (!fp)
    return -1;

  fprintf(fp, "#include <stdio.h>\n");
  fprintf(fp, "#include <stdlib.h>\n");
  fprintf(fp, "#include <string.h>\n");
  fprintf(fp, "#include <civetweb.h>\n\n");

  /* PostgreSQL includes */
  fprintf(fp, "#ifdef USE_LIBPQ\n");
  fprintf(fp, "#include <libpq-fe.h>\n");
  fprintf(fp, "static PGconn *db_conn = NULL;\n");
  fprintf(fp, "static void init_db(void) {\n");
  fprintf(fp, "    db_conn = PQconnectdb(\"user=postgres dbname=postgres\");\n");
  fprintf(fp, "    if (PQstatus(db_conn) != CONNECTION_OK) {\n");
  fprintf(fp, "        fprintf(stderr, \"Connection to database failed: %%s\", PQerrorMessage(db_conn));\n");
  fprintf(fp, "        PQfinish(db_conn);\n");
  fprintf(fp, "        exit(1);\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "}\n");
  fprintf(fp, "#endif\n\n");

  /* Route handlers */
  for (i = 0; i < spec->n_paths; i++) {
    for (j = 0; j < spec->paths[i].n_operations; j++) {
      const char *opId = spec->paths[i].operations[j].operation_id;
      if (opId) {
        fprintf(fp, "/**\n");
        fprintf(fp, " * @brief %s handler\n", spec->paths[i].operations[j].summary ? spec->paths[i].operations[j].summary : opId);
        fprintf(fp, " * \\return HTTP Status Code\n");
        fprintf(fp, " */\n");
        fprintf(fp, "static int handle_%s(struct mg_connection *conn, void *cbdata) {\n", opId);
        fprintf(fp, "    const char *resp = \"{\\\"status\\\": \\\"%s called\\\"}\";\n", opId);
        fprintf(fp, "    (void)cbdata;\n");
        fprintf(fp, "    mg_printf(conn, \"HTTP/1.1 200 OK\\r\\nContent-Type: application/json\\r\\nContent-Length: %%d\\r\\n\\r\\n%%s\", (int)strlen(resp), resp);\n");
        fprintf(fp, "    return 200;\n");
        fprintf(fp, "}\n\n");
      }
    }
  }

  fprintf(fp, "int main(int argc, char **argv) {\n");
  fprintf(fp, "    const char *options[] = {\"document_root\", \".\", \"listening_ports\", \"8080\", 0};\n");
  fprintf(fp, "    struct mg_callbacks callbacks;\n");
  fprintf(fp, "    struct mg_context *ctx;\n");
  fprintf(fp, "    (void)argc;\n");
  fprintf(fp, "    (void)argv;\n\n");

  fprintf(fp, "#ifdef USE_LIBPQ\n");
  fprintf(fp, "    init_db();\n");
  fprintf(fp, "#endif\n\n");

  fprintf(fp, "    memset(&callbacks, 0, sizeof(callbacks));\n");
  fprintf(fp, "    ctx = mg_start(&callbacks, 0, options);\n");
  fprintf(fp, "    if (ctx == NULL) {\n");
  fprintf(fp, "        fprintf(stderr, \"Failed to start CivetWeb server.\\n\");\n");
  fprintf(fp, "        return 1;\n");
  fprintf(fp, "    }\n\n");

  /* Register handlers */
  for (i = 0; i < spec->n_paths; i++) {
    for (j = 0; j < spec->paths[i].n_operations; j++) {
      const char *opId = spec->paths[i].operations[j].operation_id;
      if (opId) {
        fprintf(fp, "    mg_set_request_handler(ctx, \"%s\", handle_%s, 0);\n", spec->paths[i].route, opId);
      }
    }
  }

  fprintf(fp, "    printf(\"Server listening on port 8080... Press enter to exit.\\n\");\n");
  fprintf(fp, "    getchar();\n");
  fprintf(fp, "    mg_stop(ctx);\n");
  fprintf(fp, "    return 0;\n");
  fprintf(fp, "}\n");

  fclose(fp);
  return 0;
}
