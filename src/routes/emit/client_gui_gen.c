/**
 * @file client_gui_gen.c
 * @brief Implementation of OAuth2 client GUI generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt.h"
#include "routes/emit/client_gui_gen.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions/parse/fs.h"
/* clang-format on */

#if defined(_MSC_VER)
/** @brief SNPRINTF macro for MSVC */
#define SNPRINTF _snprintf
#else
/** @brief SNPRINTF macro */
#define SNPRINTF snprintf
#endif

/**
 * @brief Generates client GUI bindings based on OpenAPI specs.
 *
 */
enum cdd_c_error
openapi_client_gui_generate(const struct OpenAPI_Spec *spec,
                            const struct OpenApiClientConfig *config) {
  char path_h[1024];
  char path_c[1024];
  FILE *fp_h = NULL;
  FILE *fp_c = NULL;

  if (!spec || !config || !config->filename_base)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  {
    char *dir_name = NULL, *base_name = NULL;
    char *src_dir = malloc(512);
    if (!src_dir)
      return CDD_C_ERROR_MEMORY;
    get_dirname(config->filename_base, &dir_name);
    get_basename(config->filename_base, &base_name);
    sprintf(src_dir, "%s/src", dir_name ? dir_name : ".");
    makedirs(src_dir);
    CDD_SNPRINTF(path_h, sizeof(path_h), "%s/%s_gui.h", src_dir,
                 base_name ? base_name : "generated_client");
    CDD_SNPRINTF(path_c, sizeof(path_c), "%s/%s_gui.c", src_dir,
                 base_name ? base_name : "generated_client");
    free(src_dir);
    if (dir_name)
      free(dir_name);
    if (base_name)
      free(base_name);
  }

#if defined(_MSC_VER)
  if (fopen_s(&fp_h, path_h, "w") != 0)
    fp_h = NULL;
  if (fopen_s(&fp_c, path_c, "w") != 0)
    fp_c = NULL;
#else
  fp_h = fopen(path_h, "w");
  fp_c = fopen(path_c, "w");
#endif

  if (!fp_h || !fp_c) {
    if (fp_h)
      fclose(fp_h);
    if (fp_c)
      fclose(fp_c);
    return CDD_C_SUCCESS;
  }

  /* Duplicate check inside the original file; left as is to match logic but we
   * don't strictly need it. */
  if (!fp_h || !fp_c) {
    if (fp_h)
      fclose(fp_h);
    if (fp_c)
      fclose(fp_c);
    return CDD_C_SUCCESS;
  }

  /* Header Generation */
  fprintf(fp_h, "/* Generated GUI & Token Flow Code */\n");
  fprintf(fp_h, "#ifndef CLIENT_GUI_H\n");
  fprintf(fp_h, "#define CLIENT_GUI_H\n\n");
  fprintf(fp_h, "#ifdef __cplusplus\n");
  fprintf(fp_h, "extern \"C\" {\n");
  fprintf(fp_h, "#endif /* __cplusplus */\n\n");
  fprintf(fp_h, "struct OAuth2TokenResponse;\n");
  fprintf(
      fp_h,
      "enum cdd_c_error cmp_oauth2_view_render(const char* token_endpoint);\n");
  fprintf(fp_h, "enum cdd_c_error execute_password_grant(const char* "
                "token_endpoint, const "
                "char* username, const char* "
                "password, struct OAuth2TokenResponse** out_token);\n\n");
  fprintf(fp_h, "enum cdd_c_error cmp_ui_builder_begin_column(void);\n");
  fprintf(fp_h, "enum cdd_c_error m3_text_field_init(const char* label);\n");
  fprintf(fp_h, "enum cdd_c_error m3_button_init(const char* label);\n");
  fprintf(fp_h, "#ifdef __cplusplus\n");
  fprintf(fp_h, "}\n");
  fprintf(fp_h, "#endif /* __cplusplus */\n\n");
  fprintf(fp_h, "#endif /* CLIENT_GUI_H */\n");

  /* Source Generation */
  fprintf(fp_c, "/* Generated GUI & Token Flow Implementation */\n");
  {
    char *base = NULL;
    get_basename(config->filename_base, &base);
    fprintf(fp_c, "#include \"%s_gui.h\"\n",
            base ? base : config->filename_base);
    if (base)
      free(base);
  }
  fprintf(fp_c, "#include <stdio.h>\n");
  fprintf(fp_c, "#include <stdlib.h>\n");
  fprintf(fp_c, "#include <string.h>\n");
  fprintf(
      fp_c,
      "/* Expected imports from c-abstract-http and c-orm definitions */\n");
  fprintf(
      fp_c,
      "#include <c_abstract_http/http_types.h>\n#include <cdd_c_error.h>\n ");
  fprintf(fp_c, "extern enum cdd_c_error cdd_c_parse_oauth2_token(const char "
                "*json, struct "
                "OAuth2TokenResponse **const out);\n\n");

  fprintf(fp_c,
          "/* GUI Stub implementations (for standalone compilation) */\n");
  fprintf(fp_c, "enum cdd_c_error cmp_ui_builder_begin_column(void) { return "
                "CDD_C_SUCCESS; }\n");
  fprintf(fp_c, "enum cdd_c_error m3_text_field_init(const char* label) { "
                "(void)label; return CDD_C_SUCCESS; }\n");
  fprintf(fp_c, "enum cdd_c_error m3_button_init(const char* label) { "
                "(void)label; return CDD_C_SUCCESS; }\n\n");

  fprintf(fp_c, "enum cdd_c_error cmp_oauth2_view_render(const char* "
                "token_endpoint) {\n");
  fprintf(fp_c, "  if (!token_endpoint) {\n");
  if (spec->n_servers > 0 && spec->servers[0].url) {
    fprintf(fp_c, "    token_endpoint = \"%s/oauth/token\";\n",
            spec->servers[0].url);
  } else {
    fprintf(fp_c,
            "    token_endpoint = \"http://localhost:8080/oauth/token\";\n");
  }
  fprintf(fp_c, "  }\n");

  fprintf(fp_c, "  { enum cdd_c_error rc = cmp_ui_builder_begin_column(); "
                "if(rc != CDD_C_SUCCESS) return rc; }\n");
  fprintf(fp_c, "  m3_text_field_init(\"Username\");\n");
  fprintf(fp_c, "  m3_text_field_init(\"Password\");\n");
  fprintf(fp_c, "  m3_button_init(\"Login\");\n");
  fprintf(fp_c, "  return CDD_C_SUCCESS;\n");
  fprintf(fp_c, "}\n\n");

  fprintf(fp_c, "enum cdd_c_error execute_password_grant(const char* "
                "token_endpoint, const "
                "char* username, const char* "
                "password, struct OAuth2TokenResponse** out_token) {\n");
  fprintf(fp_c, "  /* Generated automated request-handling logic mapping "
                "OpenAPI password flow */\n");
  fprintf(fp_c, "  struct HttpRequest req;\n");
  fprintf(fp_c, "  struct HttpResponse res;\n");
  fprintf(fp_c, "  enum cdd_c_error rc = CDD_C_SUCCESS;\n");
  fprintf(fp_c, "  char payload[512];\n");
  fprintf(fp_c, "  memset(&req, 0, sizeof(req));\n");
  fprintf(fp_c, "  memset(&res, 0, sizeof(res));\n");
  fprintf(fp_c, "#if defined(_MSC_VER)\n");
  fprintf(fp_c, "  sprintf_s(payload, sizeof(payload), "
                "\"grant_type=password&username=%%s&password=%%s\", username, "
                "password);\n");
  fprintf(fp_c, "#else\n");
  fprintf(fp_c, "  sprintf(payload, "
                "\"grant_type=password&username=%%s&password=%%s\", username, "
                "password);\n");
  fprintf(fp_c, "#endif\n");
  fprintf(fp_c, "  req.method = HTTP_POST;\n");
  fprintf(fp_c, "  if (!token_endpoint) {\n");
  if (spec->n_servers > 0 && spec->servers[0].url) {
    fprintf(fp_c, "    req.url = \"%s/oauth/token\";\n", spec->servers[0].url);
  } else {
    fprintf(fp_c, "    req.url = \"http://localhost:8080/oauth/token\";\n");
  }
  fprintf(fp_c, "  } else {\n");
  fprintf(fp_c, "    req.url = (char *)token_endpoint;\n");
  fprintf(fp_c, "  }\n");
  fprintf(fp_c, "  req.body = payload;\n");
  fprintf(fp_c, "  req.body_len = strlen(payload);\n");
  fprintf(fp_c, "  http_headers_add(&req.headers, \"Content-Type\", "
                "\"application/x-www-form-urlencoded\");\n");
  fprintf(fp_c, "  rc = -1; /* http_client_send(&req, &res); stubbed */\n");
  fprintf(fp_c, "  if (rc == 0 && res.body) {\n");
  fprintf(fp_c, "    rc = cdd_c_parse_oauth2_token(res.body, out_token);\n");
  fprintf(fp_c, "    free(res.body);\n");
  fprintf(fp_c, "  }\n");
  fprintf(fp_c, "  return rc;\n");
  fprintf(fp_c, "}\n");

  fclose(fp_h);
  fclose(fp_c);

  return CDD_C_SUCCESS;
}
