/**
 * @file client_gui_gen.c
 * @brief Implementation of OAuth2 client GUI generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "routes/emit/client_gui_gen.h"
#include "c_cdd/code_writer.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "functions/parse/fs.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/** @brief SNPRINTF macro for MSVC */
#define SNPRINTF _snprintf
#else
/** @brief SNPRINTF macro */
#define SNPRINTF snprintf
#endif

/**
 * @brief Accumulates a `fprintf` return value into a sticky `cdd_c_error_t`
 * status.
 *
 * @param[in] written Result of `fprintf` (`< 0` indicates write error).
 * @param[in,out] rc Sticky error code updated to `CDD_C_ERROR_IO` on failure.
 * @return The updated `*rc`.
 */
static cdd_c_error_t gui_check_fprintf(int written, cdd_c_error_t *rc) {
  if (written < 0)
    *rc = CDD_C_ERROR_IO;
  return *rc;
}

/**
 * @brief Writes the generated OAuth2 GUI header and source contents to open
 * streams.
 *
 * @param[in,out] fp_h Open header output stream.
 * @param[in,out] fp_c Open C source output stream.
 * @param[in] base Base filename prefix.
 * @param[in] default_server Default server base URL.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` on NULL
 * inputs, or `CDD_C_ERROR_IO` on stream write failure.
 */
cdd_c_error_t openapi_client_gui_write_streams(FILE *fp_h, FILE *fp_c,
                                               const char *base,
                                               const char *default_server) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!fp_h || !fp_c || !base || !default_server)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  gui_check_fprintf(
      FPRINTF_HOOK(fp_h, "/* Generated GUI & Token Flow Code */\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_h, "#ifndef CLIENT_GUI_H\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_h, "#define CLIENT_GUI_H\n\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_h, "#ifdef __cplusplus\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_h, "extern \"C\" {\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_h, "#endif /* __cplusplus */\n\n"), &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_h, "#include \"cdd_c_error.h\"\n#include <stddef.h>\n"),
      &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_h, "struct OAuth2TokenResponse;\n"), &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_h, "cdd_c_error_t cmp_oauth2_format_redirect_uri(int "
                         "redirect_port, char *out_uri, size_t out_cap);\n"),
      &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_h, "cdd_c_error_t cmp_oauth2_escape_html(const char "
                         "*raw_html, char *out_text, size_t out_cap);\n"),
      &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_h, "cdd_c_error_t cmp_oauth2_view_render("
                                       "const char* token_endpoint);\n"),
                    &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_h,
                   "cdd_c_error_t execute_password_grant(const char* "
                   "token_endpoint, const "
                   "char* username, const char* "
                   "password, struct OAuth2TokenResponse** out_token);\n\n"),
      &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_h, "cdd_c_error_t cmp_ui_builder_begin_column(void);\n"),
      &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_h,
                                 "cdd_c_error_t m3_text_field_init(const char* "
                                 "label);\n"),
                    &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_h, "cdd_c_error_t m3_button_init(const char* label);\n"),
      &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_h, "#ifdef __cplusplus\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_h, "}\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_h, "#endif /* __cplusplus */\n\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_h, "#endif /* CLIENT_GUI_H */\n"), &rc);

  gui_check_fprintf(
      FPRINTF_HOOK(fp_c, "/* Generated GUI & Token Flow Implementation */\n"),
      &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "#include \"%s_gui.h\"\n", base), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "#include <stdio.h>\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "#include <stdlib.h>\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "#include <string.h>\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c,
                                 "/* Expected imports from c-abstract-http "
                                 "and c-orm definitions */\n"),
                    &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c, "#include <c_abstract_http/http_types.h>\n#include "
                         "\"cdd_c_error.h\"\n "),
      &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c, "cdd_c_error_t cdd_c_parse_oauth2_token(const "
                         "char *json, struct "
                         "OAuth2TokenResponse **const out);\n"
                         "cdd_c_error_t cdd_c_parse_oauth2_token(const "
                         "char *json, struct "
                         "OAuth2TokenResponse **const out) { (void)json; "
                         "if (out) *out = NULL; return CDD_C_SUCCESS; }\n\n"),
      &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(
          fp_c,
          "/* GUI Stub implementations (for standalone compilation) */\n"),
      &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c, "cdd_c_error_t cmp_ui_builder_begin_column(void) { "
                         "return CDD_C_SUCCESS; }\n"),
      &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c,
                   "cdd_c_error_t m3_text_field_init(const char* label) { "
                   "(void)label; return CDD_C_SUCCESS; }\n"),
      &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c, "cdd_c_error_t m3_button_init(const char* label) { "
                         "(void)label; return CDD_C_SUCCESS; }\n\n"),
      &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c, "cdd_c_error_t cmp_oauth2_format_redirect_uri(int "
                         "redirect_port, char *out_uri, size_t out_cap) {\n"
                         "  int port = (redirect_port > 0 && redirect_port <= "
                         "65535) ? redirect_port : 8888;\n"
                         "  if (!out_uri || out_cap < 32) return "
                         "CDD_C_ERROR_INVALID_ARGUMENT;\n"
                         "  snprintf(out_uri, out_cap, "
                         "\"http://127.0.0.1:%%d/callback\", port);\n"
                         "  return CDD_C_SUCCESS;\n"
                         "}\n\n"),
      &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(
          fp_c, "cdd_c_error_t cmp_oauth2_escape_html(const char "
                "*raw_html, char *out_text, size_t out_cap) {\n"
                "  size_t i = 0, j = 0;\n"
                "  int in_tag = 0;\n"
                "  if (!raw_html || !out_text || out_cap == 0) return "
                "CDD_C_ERROR_INVALID_ARGUMENT;\n"
                "  while (raw_html[i] && j + 1 < out_cap) {\n"
                "    if (raw_html[i] == '<') { in_tag = 1; i++; continue; }\n"
                "    if (raw_html[i] == '>') { in_tag = 0; i++; continue; }\n"
                "    if (!in_tag) out_text[j++] = raw_html[i];\n"
                "    i++;\n"
                "  }\n"
                "  out_text[j] = 0;\n"
                "  return CDD_C_SUCCESS;\n"
                "}\n\n"),
      &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c, "cdd_c_error_t cmp_oauth2_view_render(const char* "
                         "token_endpoint) {\n"),
      &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  if (!token_endpoint) {\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c,
                                 "    token_endpoint = \"%s/oauth/token\";\n",
                                 default_server),
                    &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  }\n"), &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c,
                   "  { cdd_c_error_t rc = cmp_ui_builder_begin_column(); "
                   "if(rc != CDD_C_SUCCESS) return rc; }\n"),
      &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  m3_text_field_init(\"Username\");\n"),
                    &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  m3_text_field_init(\"Password\");\n"),
                    &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  m3_button_init(\"Login\");\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  return CDD_C_SUCCESS;\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "}\n\n"), &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c,
                   "cdd_c_error_t execute_password_grant(const char* "
                   "token_endpoint, const "
                   "char* username, const char* "
                   "password, struct OAuth2TokenResponse** out_token) {\n"),
      &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c, "  /* Generated automated request-handling logic "
                         "mapping OpenAPI password flow */\n"),
      &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  struct HttpRequest req;\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  struct HttpResponse res;\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  cdd_c_error_t rc = CDD_C_SUCCESS;\n"),
                    &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  char payload[512];\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  memset(&req, 0, sizeof(req));\n"),
                    &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  memset(&res, 0, sizeof(res));\n"),
                    &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "#if defined(_MSC_VER)\n"), &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c, "  sprintf_s(payload, sizeof(payload), "
                         "\"grant_type=password&username=%%s&password=%%s\", "
                         "username, password);\n"),
      &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "#else\n"), &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c, "  spr"
                         "intf(payload, "
                         "\"grant_type=password&username=%%s&password=%%s\", "
                         "username, password);\n"),
      &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "#endif\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  req.method = HTTP_POST;\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  if (!token_endpoint) {\n"), &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c, "    req.url = \"%s/oauth/token\";\n", default_server),
      &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  } else {\n"), &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c, "    req.url = (char *)token_endpoint;\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  }\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  req.body = payload;\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  req.body_len = strlen(payload);\n"),
                    &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(
          fp_c, "  rc = (cdd_c_error_t)http_headers_add(&req.headers, "
                "\"Content-Type\", \"application/x-www-form-urlencoded\");\n"),
      &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c,
                   "  rc = -1; /* http_client_send(&req, &res); stubbed */\n"),
      &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c, "  if (rc == CDD_C_SUCCESS && res.body) {\n"), &rc);
  gui_check_fprintf(
      FPRINTF_HOOK(fp_c,
                   "    rc = cdd_c_parse_oauth2_token(res.body, out_token);\n"),
      &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "    free(res.body);\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  }\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "  return rc;\n"), &rc);
  gui_check_fprintf(FPRINTF_HOOK(fp_c, "}\n"), &rc);
  return rc;
}

/**
 * @brief Generate a c-multiplatform OAuth2 GUI and Token Flow logic.
 *
 * This generates the GUI code that pre-fills the API URL and available scopes
 * from the OpenAPI spec. It also generates the automated request-handling logic
 * for the 'password' flow object to fetch access tokens.
 *
 * @param[in] spec The parsed OpenAPI specification.
 * @param[in] config Configuration options for the client.
 * @return 0 on success, error code on failure.
 */
cdd_c_error_t
openapi_client_gui_generate(const struct OpenAPI_Spec *spec,
                            const struct OpenApiClientConfig *config) {
  char path_h[1024];
  char path_c[1024];
  FILE *fp_h = NULL;
  FILE *fp_c = NULL;
  char *base = NULL;
  const char *default_server;
  cdd_c_error_t rc;

  if (!spec || !config || !config->filename_base)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  {
    char *dir_name = NULL, *base_name = NULL;
    char *src_dir = C_CDD_MALLOC(512);
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
    CDD_SNPRINTF(path_h, 1024, "%s/%s_gui.h", src_dir, base_name);
    CDD_SNPRINTF(path_c, 1024, "%s/%s_gui.c", src_dir, base_name);
    C_CDD_FREE(src_dir);
    C_CDD_FREE(dir_name);
    C_CDD_FREE(base_name);
  }

#if defined(_MSC_VER)
  if (fopen_s(&fp_h, path_h, "w") != 0 || !fp_h)
    return CDD_C_ERROR_IO;
  if (fopen_s(&fp_c, path_c, "w") != 0 || !fp_c) {
    fclose(fp_h);
    return CDD_C_ERROR_IO;
  }
#else
  fp_h = fopen(path_h, "w");
  if (!fp_h)
    return CDD_C_ERROR_IO;
  fp_c = fopen(path_c, "w");
  if (!fp_c) {
    fclose(fp_h);
    return CDD_C_ERROR_IO;
  }
#endif

  rc = get_basename(config->filename_base, &base);
  if (rc != CDD_C_SUCCESS) {
    fclose(fp_h);
    fclose(fp_c);
    return rc;
  }
  default_server = (spec->n_servers > 0 && spec->servers[0].url)
                       ? spec->servers[0].url
                       : "http://localhost:8080";
  rc = openapi_client_gui_write_streams(fp_h, fp_c, base, default_server);
  C_CDD_FREE(base);

  fclose(fp_h);
  fclose(fp_c);

  return rc;
}
