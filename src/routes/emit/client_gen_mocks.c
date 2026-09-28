/**
 * @file client_gen_mocks.c
 * @brief Test and mock generation for OpenAPI client generator.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "routes/emit/client_gen_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/safe_crt.h"
#include "functions/parse/fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Emit test_sdk.c mocks for testing client.
 */
cdd_c_error_t client_gen_emit_mocks(const char *dir_name,
                                    const struct OpenAPI_Spec *spec) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  size_t i, j;

  char tdir[512], tfile[640];
  FILE *tfp;
  CDD_SNPRINTF(tdir, sizeof(tdir), "%s/src/test", dir_name);
  {
    cdd_c_error_t rc_cg;
    rc_cg = makedirs(tdir);
#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 51)
      rc_cg = CDD_C_ERROR_IO;
#endif
    if (rc_cg != CDD_C_SUCCESS) {
      rc = rc_cg;
      goto cleanup;
    }
  }
  CDD_SNPRINTF(tfile, sizeof(tfile), "%s/test_sdk.c", tdir);
#if defined(_MSC_VER)
  if (fopen_s(&tfp, tfile, "w") != 0)
    tfp = NULL;
#else
  tfp = fopen(tfile, "w");
#endif
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 52) {
    fclose(tfp);
    tfp = NULL;
  }
#endif
  if (tfp) {
    fprintf(tfp, "#include <stdio.h>\n#include <stdlib.h>\n#include "
                 "<string.h>\n#include \"generated_client_models.h\"\n#include "
                 "\"c_cdd/safe_crt.h\"\n");
    fprintf(tfp, "#if defined(__APPLE__)\n#include "
                 "<c_abstract_http/http_apple.h>\n#define "
                 "C_ABSTRACT_HTTP_INIT http_apple_context_init\n#define "
                 "C_ABSTRACT_HTTP_SEND http_apple_send\n#else\n#include "
                 "<c_abstract_http/http_curl.h>\n#define "
                 "C_ABSTRACT_HTTP_INIT http_curl_context_init\n#define "
                 "C_ABSTRACT_HTTP_SEND http_curl_send\n#endif\n");
    fprintf(tfp, "int main(void) {\n  struct HttpClient client;\n  "
                 "C_ABSTRACT_HTTP_INIT(&client.transport);\n  client.send = "
                 "C_ABSTRACT_HTTP_SEND;\n");

    for (i = 0; i < spec->n_paths; ++i) {
      for (j = 0; j < spec->paths[i].n_operations; ++j) {
        const struct OpenAPI_Operation *op = &spec->paths[i].operations[j];
        const char *method = "";
        const char *parsed_base_path = "";
        char formatted_path[512];
        const char *in;
        char *out;
        int first_query;
        size_t p, r;
        if (op->verb == OA_VERB_GET)
          method = "GET";
        else if (op->verb == OA_VERB_POST)
          method = "POST";
        else if (op->verb == OA_VERB_PUT)
          method = "PUT";
        else if (op->verb == OA_VERB_DELETE)
          method = "DELETE";
        else
          method = "GET";

        fprintf(tfp,
                "  {\n    struct HttpRequest req;\n    struct HttpHeader "
                "*hdrs = NULL;\n"
                "    const char *base_url = NULL;\n    char url_buf[1024];\n"
                "    struct HttpResponse *res = NULL;\n"
                "    int allowed = 0;\n    "
                "http_request_init(&req);\n");
        fprintf(tfp, "    req.method = HTTP_%s;\n", method);
        if (op->verb == OA_VERB_POST || op->verb == OA_VERB_PUT ||
            op->verb == OA_VERB_PATCH) {
          const char *body_str =
              op->req_body.is_array
                  ? "[]"
                  : "{\\\"name\\\":\\\"doggie\\\",\\\"photoUrls\\\":["
                    "\\\"http://"
                    "a.com\\\"],\\\"id\\\":1,\\\"petId\\\":1,"
                    "\\\"quantity\\\":1,\\\"username\\\":\\\"testuser\\\","
                    "\\\"password\\\":\\\"123\\\",\\\"status\\\":"
                    "\\\"available\\\"}";
          fprintf(tfp,
                  "    req.body = (uint8_t*)\"%s\";\n    req.body_len = "
                  "strlen(\"%s\");\n",
                  body_str, body_str);
          fprintf(tfp,
                  "    hdrs = malloc(sizeof(struct "
                  "HttpHeader) * 4);\n"
                  "    hdrs[0].key = \"Content-Type\";\n"
                  "    hdrs[0].value = \"%s\";\n"
                  "    hdrs[1].key = \"api_key\";\n"
                  "    hdrs[1].value = \"special-key\";\n"
                  "    hdrs[2].key = \"Authorization\";\n"
                  "    hdrs[2].value = \"Bearer special-key\";\n"
                  "    hdrs[3].key = \"Accept\";\n"
                  "    hdrs[3].value = \"application/json\";\n"
                  "    req.headers.headers = hdrs;\n"
                  "    req.headers.count = 4;\n",
                  op->n_req_body_media_types > 0
                      ? op->req_body_media_types[0].name
                      : "application/json");
        } else {
          fprintf(tfp,
                  "    hdrs = malloc(sizeof(struct "
                  "HttpHeader) * 3);\n    hdrs[0].key = \"api_key\";\n    "
                  "hdrs[0].value = \"special-key\";\n    hdrs[1].key = "
                  "\"Authorization\";\n"
                  "    hdrs[1].value = \"Bearer special-key\";\n"
                  "    hdrs[2].key = \"Accept\";\n"
                  "    hdrs[2].value = \"application/json\";\n"
                  "    req.headers.headers = hdrs;\n    req.headers.count = "
                  "3;\n");
        }

        in = spec->paths[i].route;
        out = formatted_path;
        while (*in) {
          if (*in == '{') {
            *out++ = '1';
            in = strchr(in, '}') + 1;
          } else {
            *out++ = *in++;
          }
        }
        *out = '\0';
        first_query = 1;
        for (p = 0; p < op->n_parameters; p++) {
          if (op->parameters[p].in == OA_PARAM_IN_QUERY ||
              op->parameters[p].in == OA_PARAM_IN_QUERYSTRING) {
            if (first_query) {
              CDD_STRNCAT(formatted_path, sizeof(formatted_path), "?",
                          sizeof(formatted_path) - strlen(formatted_path) - 1);
              first_query = 0;
            } else {
              CDD_STRNCAT(formatted_path, sizeof(formatted_path), "&",
                          sizeof(formatted_path) - strlen(formatted_path) - 1);
            }
            CDD_STRNCAT(formatted_path, sizeof(formatted_path),
                        op->parameters[p].name,
                        sizeof(formatted_path) - strlen(formatted_path) - 1);
            CDD_STRNCAT(formatted_path, sizeof(formatted_path), "=1",
                        sizeof(formatted_path) - strlen(formatted_path) - 1);
          }
        }
        parsed_base_path = "";
        if (spec->n_servers > 0 && spec->servers[0].url) {
          const char *ptr = strstr(spec->servers[0].url, "://");
          if (ptr) {
            ptr = strchr(ptr + 3, '/');
            if (ptr)
              parsed_base_path = ptr;
          } else {
            parsed_base_path = spec->servers[0].url;
          }
        } else if (spec->basePath) {
          parsed_base_path = spec->basePath;
        }
        if (strcmp(parsed_base_path, "/") == 0)
          parsed_base_path = "";

        fprintf(tfp, "    base_url = getenv(\"BASE_URL\");\n");
        fprintf(tfp, "    if (!base_url) {\n");
        fprintf(tfp,
                "        CDD_SNPRINTF(url_buf, sizeof(url_buf), "
                "\"http://localhost:8080/v2%%s\", \"%s\");\n",
                formatted_path);
        fprintf(tfp, "    } else {\n");
        fprintf(tfp,
                "        CDD_SNPRINTF(url_buf, sizeof(url_buf), \"%%s%%s%%s\", "
                "base_url, \"%s\", \"%s\");\n",
                parsed_base_path, formatted_path);
        fprintf(tfp, "    }\n");
        fprintf(tfp, "    req.url = url_buf;\n");
        fprintf(tfp, "    client.send(client.transport, &req, &res);\n");

        fprintf(tfp, "    if (res && res->status_code >= 200 && "
                     "res->status_code < 300) allowed = 1;\n");
        for (r = 0; r < op->n_responses; ++r) {
          if (op->responses[r].code &&
              strcmp(op->responses[r].code, "default") != 0) {
            fprintf(tfp,
                    "    if (res && res->status_code == %d) allowed = 1;\n",
                    atoi(op->responses[r].code));
          }
        }
        fprintf(tfp, "    if (allowed) {\n");

        if (op->n_responses > 0 && op->responses[0].schema.ref_name &&
            !op->responses[0].schema.is_array) {
          fprintf(tfp, "        struct %s *out = NULL;\n",
                  op->responses[0].schema.ref_name);
          fprintf(tfp, "        if (res->body && res->status_code >= 200 && "
                       "res->status_code < 300) {\n");
          fprintf(tfp,
                  "            char *body_str = malloc(res->body_len + 1);\n"
                  "            int rc;\n");
          fprintf(tfp,
                  "            memcpy(body_str, res->body, res->body_len);\n");
          fprintf(tfp, "            body_str[res->body_len] = '\\0';\n");
          fprintf(tfp,
                  "            rc = %s_from_json(body_str, "
                  "&out);\n",
                  op->responses[0].schema.ref_name);
          fprintf(tfp, "            if (rc != CDD_C_SUCCESS) { "
                       "fprintf(stderr, \"Parse "
                       "failed\\n\");  }\n");
          fprintf(tfp, "            free(body_str);\n");
          fprintf(tfp, "        }\n");
        }
        fprintf(tfp,
                "    } else { fprintf(stderr, \"Status %%d on %%s\\n\", res ? "
                "res->status_code : 0, req.url);  }\n");
        fprintf(tfp, "    http_response_free(res);\n");
        fprintf(tfp, "  }\n");
      }
    }

    fprintf(tfp, "  return CDD_C_SUCCESS;\n}\n");
    fclose(tfp);
  }

  return CDD_C_SUCCESS;

cleanup:
  return rc;
}
