/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/oauth2_error.h"
#include "classes/emit/struct.h"
#include "win_compat_sym.h"
/* clang-format on */

int write_oauth2_error_parser_func(FILE *fp, const char *struct_name,
                                   const struct StructFields *sf) {
  size_t i;
  if (!fp || !struct_name || !sf)
    return EINVAL;

  /* Generate Enum */
  fprintf(fp, "/**\n * @brief Standard OAuth2 Errors for %s\n */\n",
          struct_name);
  fprintf(fp, "enum %sErrorType {\n", struct_name);
  fprintf(fp, "  %s_ERROR_NONE = 0,\n", struct_name);
  fprintf(fp, "  %s_ERROR_INVALID_REQUEST,\n", struct_name);
  fprintf(fp, "  %s_ERROR_INVALID_CLIENT,\n", struct_name);
  fprintf(fp, "  %s_ERROR_INVALID_GRANT,\n", struct_name);
  fprintf(fp, "  %s_ERROR_UNAUTHORIZED_CLIENT,\n", struct_name);
  fprintf(fp, "  %s_ERROR_UNSUPPORTED_GRANT_TYPE,\n", struct_name);
  fprintf(fp, "  %s_ERROR_UNKNOWN\n", struct_name);
  fprintf(fp, "};\n\n");

  /* Parse function (Zero-allocation style, similar to standalone_json) */
  fprintf(fp,
          "/**\n * @brief Parse RFC6749 OAuth2 Error response into %s.\n */\n",
          struct_name);
  fprintf(fp,
          "int %s_parse_oauth2_error(char *json, struct %s **const out, enum "
          "%sErrorType *out_err) {\n",
          struct_name, struct_name, struct_name);
  fprintf(fp, "  struct %s *ret;\n", struct_name);
  fprintf(fp, "  char *p = json;\n");
  fprintf(fp, "  char *key;\n");
  fprintf(fp, "  char *val;\n\n");
  fprintf(fp, "  if (!json || !out || !out_err) return 1;\n");
  fprintf(fp, "  *out_err = %s_ERROR_NONE;\n", struct_name);
  fprintf(fp, "  ret = (struct %s *)calloc(1, sizeof(struct %s));\n",
          struct_name, struct_name);
  fprintf(fp, "  if (!ret) return 1;\n\n");

  fprintf(fp, "  while (*p) {\n");
  fprintf(fp, "    while (*p && *p != '\"') p++;\n");
  fprintf(fp, "    if (!*p) break;\n");
  fprintf(fp, "    p++;\n");
  fprintf(fp, "    key = p;\n");
  fprintf(fp, "    while (*p && *p != '\"') p++;\n");
  fprintf(fp, "    if (!*p) break;\n");
  fprintf(fp, "    *p++ = '\\0';\n");

  fprintf(fp, "    while (*p && *p != ':') p++;\n");
  fprintf(fp, "    if (!*p) break;\n");
  fprintf(fp, "    p++;\n");
  fprintf(fp, "    while (*p && (*p == ' ' || *p == '\\t' || *p == '\\n' || *p "
              "== '\\r')) p++;\n\n");

  fprintf(fp, "    if (*p == '\"') {\n");
  fprintf(fp, "      p++;\n");
  fprintf(fp, "      val = p;\n");
  fprintf(fp, "      while (*p && *p != '\"') {\n");
  fprintf(fp, "        if (*p == '\\\\' && *(p+1)) p += 2;\n");
  fprintf(fp, "        else p++;\n");
  fprintf(fp, "      }\n");
  fprintf(fp, "      if (*p) *p++ = '\\0';\n\n");

  fprintf(fp, "      if (strcmp(key, \"error\") == 0) {\n");
  fprintf(fp,
          "        if (strcmp(val, \"invalid_request\") == 0) *out_err = "
          "%s_ERROR_INVALID_REQUEST;\n",
          struct_name);
  fprintf(fp,
          "        else if (strcmp(val, \"invalid_client\") == 0) *out_err = "
          "%s_ERROR_INVALID_CLIENT;\n",
          struct_name);
  fprintf(fp,
          "        else if (strcmp(val, \"invalid_grant\") == 0) *out_err = "
          "%s_ERROR_INVALID_GRANT;\n",
          struct_name);
  fprintf(fp,
          "        else if (strcmp(val, \"unauthorized_client\") == 0) "
          "*out_err = %s_ERROR_UNAUTHORIZED_CLIENT;\n",
          struct_name);
  fprintf(fp,
          "        else if (strcmp(val, \"unsupported_grant_type\") == 0) "
          "*out_err = %s_ERROR_UNSUPPORTED_GRANT_TYPE;\n",
          struct_name);
  fprintf(fp, "        else *out_err = %s_ERROR_UNKNOWN;\n", struct_name);

  /* Assign if struct has an error field */
  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].name, "error") == 0 &&
        strcmp(sf->fields[i].type, "string") == 0) {
      fprintf(fp, "        ret->error = val;\n");
    }
  }

  fprintf(fp, "      }\n");

  fprintf(fp, "      else if (strcmp(key, \"error_description\") == 0) {\n");
  /* Assign if struct has an error_description field */
  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].name, "error_description") == 0 &&
        strcmp(sf->fields[i].type, "string") == 0) {
      fprintf(fp, "        ret->error_description = val;\n");
    }
  }
  fprintf(fp, "      }\n");

  fprintf(fp, "    } else {\n");
  fprintf(fp, "      while (*p && *p != ',' && *p != '}' && *p != ' ' && *p != "
              "'\\n' && *p != '\\r') p++;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "  }\n\n");

  fprintf(fp, "  *out = ret;\n");
  fprintf(fp, "  return 0;\n");
  fprintf(fp, "}\n");
  return 0;
}