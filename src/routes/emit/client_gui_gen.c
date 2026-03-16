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
  fprintf(fp_h, "int cmp_oauth2_view_render(void);\n");
  fprintf(fp_h, "int execute_password_grant(const char* username, const char* "
                "password);\n\n");
  fprintf(fp_h, "#ifdef __cplusplus\n");
  fprintf(fp_h, "}\n");
  fprintf(fp_h, "#endif /* __cplusplus */\n\n");
  fprintf(fp_h, "#endif /* CLIENT_GUI_H */\n");

  /* Source Generation */
  fprintf(fp_c, "/* Generated GUI & Token Flow Implementation */\n");
  fprintf(fp_c, "#include \"%s_gui.h\"\n", config->filename_base);
  fprintf(fp_c, "#include <stdio.h>\n\n");

  /* Locate OAuth2 password flow info in spec (mock up for now) */
  fprintf(fp_c, "int cmp_oauth2_view_render(void) {\n");
  fprintf(fp_c, "  /* Pre-filled from OpenAPI spec */\n");

  if (spec->n_servers > 0 && spec->servers[0].url) {
    fprintf(fp_c, "  const char* api_url = \"%s\";\n", spec->servers[0].url);
  } else {
    fprintf(fp_c, "  const char* api_url = \"http://localhost:8080\";\n");
  }

  fprintf(fp_c, "  printf(\"Rendering OAuth2 View for %%s\\n\", api_url);\n");

  /* If spec has security schemes, we could iterate scopes here. For now we use
   * generic. */
  fprintf(fp_c, "  /* Available scopes: */\n");
  fprintf(fp_c, "  printf(\"Scope: read\\n\");\n");
  fprintf(fp_c, "  printf(\"Scope: write\\n\");\n");
  fprintf(fp_c, "  return 0;\n");
  fprintf(fp_c, "}\n\n");

  fprintf(fp_c, "int execute_password_grant(const char* username, const char* "
                "password) {\n");
  fprintf(fp_c, "  /* Generated automated request-handling logic mapping "
                "OpenAPI password flow */\n");
  fprintf(fp_c, "  printf(\"Executing OAuth2 password grant for user: "
                "%%s\\n\", username);\n");
  fprintf(fp_c, "  /* Perform HTTP call... */\n");
  fprintf(fp_c, "  return 0;\n");
  fprintf(fp_c, "}\n");

  fclose(fp_h);
  fclose(fp_c);

  return 0;
}
