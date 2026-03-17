/**
 * @file client_gui_gen.c
 * @brief Implementation of OAuth2 client GUI generation.
 */

/* clang-format off */
#include "routes/emit/client_gui_gen.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
#define SNPRINTF _snprintf
#else
#define SNPRINTF snprintf
#endif

int openapi_client_gui_generate(const struct OpenAPI_Spec *spec,
                                const struct OpenApiClientConfig *config) {
  char path_h[1024];
  char path_c[1024];
  FILE *fp_h = NULL;
  FILE *fp_c = NULL;

  if (!spec || !config || !config->filename_base)
    return EINVAL;

  SNPRINTF(path_h, sizeof(path_h), "%s_gui.h", config->filename_base);
  SNPRINTF(path_c, sizeof(path_c), "%s_gui.c", config->filename_base);

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
    return EIO;
  }

  /* Header Generation */
  fprintf(fp_h, "/* Generated GUI & Token Flow Code */\n");
  fprintf(fp_h, "#ifndef CLIENT_GUI_H\n");
  fprintf(fp_h, "#define CLIENT_GUI_H\n\n");
  fprintf(fp_h, "#ifdef __cplusplus\n");
  fprintf(fp_h, "extern \"C\" {\n");
  fprintf(fp_h, "#endif /* __cplusplus */\n\n");
  fprintf(fp_h, "struct OAuth2TokenResponse;\n");
  fprintf(fp_h, "int cmp_oauth2_view_render(const char* token_endpoint);\n");
  fprintf(fp_h, "int execute_password_grant(const char* token_endpoint, const "
                "char* username, const char* "
                "password, struct OAuth2TokenResponse** out_token);\n\n");
  fprintf(fp_h, "void cmp_ui_builder_begin_column(void);\n");
  fprintf(fp_h, "void m3_text_field_init(const char* label);\n");
  fprintf(fp_h, "void m3_button_init(const char* label);\n");
  fprintf(fp_h, "#ifdef __cplusplus\n");
  fprintf(fp_h, "}\n");
  fprintf(fp_h, "#endif /* __cplusplus */\n\n");
  fprintf(fp_h, "#endif /* CLIENT_GUI_H */\n");

  /* Source Generation */
  fprintf(fp_c, "/* Generated GUI & Token Flow Implementation */\n");
  fprintf(fp_c, "#include \"%s_gui.h\"\n", config->filename_base);
  fprintf(fp_c, "#include <stdio.h>\n");
  fprintf(fp_c, "#include <stdlib.h>\n");
  fprintf(fp_c, "#include <string.h>\n");
  fprintf(
      fp_c,
      "/* Expected imports from c-abstract-http and c-orm definitions */\n");
  fprintf(fp_c, "#include \"c_abstract_http_client.h\"\n");
  fprintf(fp_c, "extern int cdd_c_parse_oauth2_token(const char *json, struct "
                "OAuth2TokenResponse **const out);\n\n");

  fprintf(fp_c,
          "/* GUI Stub implementations (for standalone compilation) */\n");
  fprintf(fp_c, "void cmp_ui_builder_begin_column(void) {}\n");
  fprintf(fp_c,
          "void m3_text_field_init(const char* label) { (void)label; }\n");
  fprintf(fp_c, "void m3_button_init(const char* label) { (void)label; }\n\n");

  fprintf(fp_c, "int cmp_oauth2_view_render(const char* token_endpoint) {\n");
  fprintf(fp_c, "  if (!token_endpoint) {\n");
  if (spec->n_servers > 0 && spec->servers[0].url) {
    fprintf(fp_c, "    token_endpoint = \"%s/oauth/token\";\n",
            spec->servers[0].url);
  } else {
    fprintf(fp_c,
            "    token_endpoint = \"http://localhost:8080/oauth/token\";\n");
  }
  fprintf(fp_c, "  }\n");

  fprintf(fp_c, "  cmp_ui_builder_begin_column();\n");
  fprintf(fp_c, "  m3_text_field_init(\"Username\");\n");
  fprintf(fp_c, "  m3_text_field_init(\"Password\");\n");
  fprintf(fp_c, "  m3_button_init(\"Login\");\n");
  fprintf(fp_c, "  return 0;\n");
  fprintf(fp_c, "}\n\n");

  fprintf(fp_c, "int execute_password_grant(const char* token_endpoint, const "
                "char* username, const char* "
                "password, struct OAuth2TokenResponse** out_token) {\n");
  fprintf(fp_c, "  /* Generated automated request-handling logic mapping "
                "OpenAPI password flow */\n");
  fprintf(fp_c, "  struct HttpRequest req;\n");
  fprintf(fp_c, "  struct HttpResponse res;\n");
  fprintf(fp_c, "  int rc;\n");
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
  fprintf(fp_c, "    req.url = token_endpoint;\n");
  fprintf(fp_c, "  }\n");
  fprintf(fp_c, "  req.body = payload;\n");
  fprintf(fp_c, "  req.body_len = strlen(payload);\n");
  fprintf(fp_c,
          "  req.content_type = \"application/x-www-form-urlencoded\";\n");
  fprintf(fp_c, "  rc = http_client_send(&req, &res);\n");
  fprintf(fp_c, "  if (rc == 0 && res.body) {\n");
  fprintf(fp_c, "    rc = cdd_c_parse_oauth2_token(res.body, out_token);\n");
  fprintf(fp_c, "    free(res.body);\n");
  fprintf(fp_c, "  }\n");
  fprintf(fp_c, "  return rc;\n");
  fprintf(fp_c, "}\n");

  fclose(fp_h);
  fclose(fp_c);

  return 0;
}
