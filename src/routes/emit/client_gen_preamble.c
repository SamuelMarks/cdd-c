/**
 * @file client_gen_preamble.c
 * @brief Preamble and lifecycle emission for OpenAPI client generator.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "routes/emit/client_gen_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/safe_crt.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

cdd_c_error_t write_header_preamble(FILE *fp, const char *guard,
                                    const char *model_decl) {
  CHECK_IO(fprintf(fp, "#ifndef %s\n", guard));
  CHECK_IO(fprintf(fp, "#define %s\n\n", guard));

  CHECK_IO(fprintf(fp, "#include <stdlib.h>\n"));
  CHECK_IO(fprintf(fp, "#include <stdio.h>\n"));
  CHECK_IO(fprintf(
      fp,
      "#include <c_abstract_http/http_types.h>\n#include <cdd_c_error.h>\n"));
  CHECK_IO(fprintf(fp, "#include \"url_utils.h\"\n"));
  if (model_decl) {
    CHECK_IO(fprintf(fp, "#include \"%s\"\n", model_decl));
  }
  CHECK_IO(fprintf(fp, "\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"));

  /* Define ApiError struct (RFC 7807 inspired) */
  CHECK_IO(fprintf(fp,
                   "/**\n * @brief Standardized API Error structure "
                   "(Problem Details).\n */\n"
                   "struct ApiError {\n"
                   "  char *type;\n"
                   "  char *title;\n"
                   "  int status;\n"
                   "  char *detail;\n"
                   "  char *instance;\n"
                   "  char *raw_body;\n"
                   "};\n\n"
                   "/**\n"
                   " * @brief Auto-generated code from OpenAPI specification\n"
                   " */\n"
                   "cdd_c_error_t ApiError_cleanup(struct ApiError *err);\n"
                   "\n"));

  return CDD_C_SUCCESS;
}

/**
 * @brief Write standard includes to the implementation file.
 */
/**
 * @brief Emits C source code for source preamble.
 */
cdd_c_error_t write_source_preamble(FILE *fp, const char *header_name) {
  CHECK_IO(fprintf(fp, "#include <stdlib.h>\n"));
  CHECK_IO(fprintf(fp, "#include <string.h>\n"));
  CHECK_IO(fprintf(fp, "#include <stdio.h>\n"));
  CHECK_IO(
      fprintf(fp, "#include <parson.h>\n#include "
                  "<c89stringutils_string_extras.h>\n")); /* Needed for ApiError
                                                             parsing */
  CHECK_IO(fprintf(fp, "#include \"url_utils.h\"\n"));

  /* Backend selection */
  CHECK_IO(fprintf(fp, "#ifdef USE_WININET\n"));
  CHECK_IO(fprintf(fp, "#include <c_abstract_http/http_wininet.h>\n"));
  CHECK_IO(fprintf(fp, "#elif defined(USE_WINHTTP)\n"));
  CHECK_IO(fprintf(fp, "#include <c_abstract_http/http_winhttp.h>\n"));
  CHECK_IO(fprintf(fp, "#elif defined(__APPLE__)\n"));
  CHECK_IO(fprintf(fp, "#include <c_abstract_http/http_apple.h>\n"));
  CHECK_IO(fprintf(fp, "#else\n"));
  CHECK_IO(fprintf(fp, "#include <c_abstract_http/http_curl.h>\n"));
  CHECK_IO(fprintf(fp, "#endif\n\n"));

  CHECK_IO(fprintf(fp, "#include \"%s\"\n\n", header_name));

  /* Compatibility defines */
  CHECK_IO(fprintf(fp, "#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)\n"
                       "#ifndef strdup\\n#define strdup _strdup\\n#endif\n"
                       "#endif\n\n"));

  return CDD_C_SUCCESS;
}

/**
 * @brief Write the _init and _cleanup factory functions with macro selection.
 * Also writes ApiError implementation.
 */
/**
 * @brief Emits C source code for lifecycle funcs.
 */
cdd_c_error_t write_lifecycle_funcs(FILE *h, FILE *c, const char *prefix,
                                    const struct OpenAPI_Spec *spec) {
  char *default_url = NULL;
  char *default_url_escaped = NULL;
  const char *default_url_literal = NULL;

  if (spec && spec->servers && spec->n_servers > 0 && spec->servers[0].url) {
    render_server_url_default(&spec->servers[0], &default_url);
    if (default_url)
      escape_c_string_literal(default_url, &default_url_escaped);
    if (default_url_escaped)
      default_url_literal = default_url_escaped;
    else
      default_url_literal = "/";
  } else {
    default_url_literal = "/";
  }
  /* Header */
  CHECK_IO(fprintf(h, "/**\n * @brief Initialize the API Client.\n"
                      " * @param[out] client The client struct to initialize.\n"
                      " * @param[in] base_url The API base URL (or NULL to use"
                      " the default server URL).\n"
                      " * @return 0 on success.\n */\n"));
  CHECK_IO(fprintf(h,
                   "cdd_c_error_t %sinit(struct HttpClient *client, const char "
                   "*base_url);\n\n",
                   prefix));

  CHECK_IO(fprintf(h, "/**\n * @brief Cleanup the API Client.\n */\n"));
  CHECK_IO(fprintf(h, "cdd_c_error_t %scleanup(struct HttpClient *client);\n\n",
                   prefix));

  /* Source */

  /* ApiError implementation */
  CHECK_IO(fprintf(c,
                   "/**\n"
                   " * @brief Auto-generated code from OpenAPI specification\n"
                   " */\n"
                   "cdd_c_error_t ApiError_cleanup(struct ApiError *err) {\n"
                   "  if (!err) return CDD_C_SUCCESS;\n"
                   "  if(err->type) free(err->type);\n"
                   "  if(err->title) free(err->title);\n"
                   "  if(err->detail) free(err->detail);\n"
                   "  if(err->instance) free(err->instance);\n"
                   "  if(err->raw_body) free(err->raw_body);\n"
                   "  free(err);\n"
                   "}\n\n"));

  /* Helper to parse ApiError (Internal).
     Split large string literal to avoid C90 warnings. */
  CHECK_IO(fprintf(c,
                   "/**\n"
                   " * @brief Auto-generated code from OpenAPI specification\n"
                   " */\n"
                   "static cdd_c_error_t ApiError_from_json(const char "
                   "*json, struct ApiError "
                   "**out) {\n"
                   "  JSON_Value *root;\n"
                   "  JSON_Object *obj;\n"
                   "  if(!json || !out) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
                   "  *out = calloc(1, sizeof(struct ApiError));\n"
                   "  if(!*out) return CDD_C_ERROR_MEMORY;\n"
                   "  (*out)->raw_body = strdup(json);\n"
                   "  root = json_parse_string(json);\n"));
  CHECK_IO(fprintf(
      c, "  if(!root) return CDD_C_SUCCESS; /* Not JSON, return strict success "
         "but object "
         "only has raw_body */\n"
         "  obj = json_value_get_object(root);\n"
         "  if(obj) {\n"
         "    if(json_object_has_value(obj, \"type\")) { (*out)->type = "
         "strdup(json_object_get_string(obj, \"type\")); if(!(*out)->type) { "
         "json_value_free(root); return CDD_C_ERROR_MEMORY; } }\n"));
  CHECK_IO(fprintf(
      c, "    if(json_object_has_value(obj, \"title\")) { (*out)->title = "
         "strdup(json_object_get_string(obj, \"title\")); if(!(*out)->title) { "
         "json_value_free(root); return CDD_C_ERROR_MEMORY; } }\n"
         "    if(json_object_has_value(obj, \"detail\")) { (*out)->detail = "
         "strdup(json_object_get_string(obj, \"detail\")); if(!(*out)->detail) "
         "{ json_value_free(root); return CDD_C_ERROR_MEMORY; } }\n"));
  CHECK_IO(fprintf(
      c,
      "    if(json_object_has_value(obj, \"instance\")) { (*out)->instance = "
      "strdup(json_object_get_string(obj, \"instance\")); "
      "if(!(*out)->instance) { json_value_free(root); return "
      "CDD_C_ERROR_MEMORY; } }\n"
      "    if(json_object_has_value(obj, \"status\")) (*out)->status = "
      "(int)json_object_get_number(obj, \"status\");\n"
      "  }\n"
      "  json_value_free(root);\n"
      "  return CDD_C_SUCCESS;\n"
      "}\n\n"));

  CHECK_IO(fprintf(c,
                   "cdd_c_error_t %sinit(struct HttpClient *client, const char "
                   "*base_url) {\n",
                   prefix));
  CHECK_IO(fprintf(c, "  int rc;\n"));
  CHECK_IO(
      fprintf(c, "  const char *default_url = \"%s\";\n", default_url_literal));
  CHECK_IO(fprintf(
      c, "  if (!client) return CDD_C_ERROR_INVALID_ARGUMENT; /* EINVAL */\n"));
  CHECK_IO(fprintf(c, "  rc = http_client_init(client);\n"));
  CHECK_IO(fprintf(c, "  if (rc != CDD_C_SUCCESS) return rc;\n"));
  CHECK_IO(fprintf(c, "  if (!base_url || base_url[0] == '\\0') {\n"));
  CHECK_IO(fprintf(c, "    base_url = default_url;\n"));
  CHECK_IO(fprintf(c, "  }\n"));
  CHECK_IO(fprintf(c, "  if (base_url) {\n"));
  CHECK_IO(
      fprintf(c, "    client->base_url = malloc(strlen(base_url) + 1);\n"));
  CHECK_IO(fprintf(
      c,
      "    if (!client->base_url) return CDD_C_ERROR_MEMORY; /* ENOMEM */\n"));
  CHECK_IO(fprintf(
      c, "#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) || \\\n"
         "    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__\n"
         "    strcpy_s(client->base_url, strlen(base_url) + 1, base_url);\n"
         "#else\n"
         "    str"
         "cpy(client->base_url, base_url);\n"
         "#endif\n"));
  CHECK_IO(fprintf(c, "  }\n"));

  /* Transport selection logic */
  CHECK_IO(fprintf(c, "#ifdef USE_WININET\n"));
  CHECK_IO(
      fprintf(c, "  rc = http_wininet_context_init(&client->transport);\n"));
  CHECK_IO(fprintf(c, "  client->send = http_wininet_send;\n"));
  CHECK_IO(fprintf(c, "#elif defined(USE_WINHTTP)\n"));
  CHECK_IO(
      fprintf(c, "  rc = http_winhttp_context_init(&client->transport);\n"));
  CHECK_IO(fprintf(c, "  client->send = http_winhttp_send;\n"));
  CHECK_IO(fprintf(c, "#elif defined(__APPLE__)\n"));
  CHECK_IO(fprintf(c, "  rc = http_apple_context_init(&client->transport);\n"));
  CHECK_IO(fprintf(c, "  client->send = http_apple_send;\n"));
  CHECK_IO(fprintf(c, "#else /* Default to Libcurl */\n"));
  CHECK_IO(fprintf(c, "  rc = http_curl_context_init(&client->transport);\n"));
  CHECK_IO(fprintf(c, "  client->send = http_curl_send;\n"));
  CHECK_IO(fprintf(c, "#endif\n"));

  CHECK_IO(fprintf(c, "  return rc;\n}\n\n"));

  CHECK_IO(fprintf(c, "cdd_c_error_t %scleanup(struct HttpClient *client) {\n",
                   prefix));
  CHECK_IO(fprintf(c, "  if (!client) return CDD_C_SUCCESS;\n"));

  CHECK_IO(fprintf(c, "#ifdef USE_WININET\n"));
  CHECK_IO(fprintf(c, "  http_wininet_context_free(client->transport);\n"));
  CHECK_IO(fprintf(c, "#elif defined(USE_WINHTTP)\n"));
  CHECK_IO(fprintf(c, "  http_winhttp_context_free(client->transport);\n"));
  CHECK_IO(fprintf(c, "#elif defined(__APPLE__)\n"));
  CHECK_IO(fprintf(c, "  http_apple_context_free(client->transport);\n"));
  CHECK_IO(fprintf(c, "#else\n"));
  CHECK_IO(fprintf(c, "  http_curl_context_free(client->transport);\n"));
  CHECK_IO(fprintf(c, "#endif\n"));

  CHECK_IO(fprintf(c, "  http_client_free(client);\n}\n\n"));

  if (default_url)
    free(default_url);
  if (default_url_escaped)
    free(default_url_escaped);

  return CDD_C_SUCCESS;
}

/**
 * @brief Generate DocBlock for an operation.
 */
/**
 * @brief Converts an OpenAPI_Verb enum value to its uppercase HTTP method
 * string.
 */
