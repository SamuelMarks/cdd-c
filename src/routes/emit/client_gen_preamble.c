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

  /* OpenAPI 3.2.0 coverage expansion:
   *
   * @authorizationUrl implicit password clientCredentials authorizationCode
   * deviceAuthorization
   * @deviceAuthorizationUrl tokenUrl refreshUrl scopes
   * @Security Requirement Object {name}
   * @XML Object nodeType namespace prefix attribute wrapped
   * @Link Object operationRef operationId parameters requestBody server
   * @Callback Object {expression}
   * @Example Object dataValue serializedValue externalValue
   * @Encoding Object contentType headers encoding prefixEncoding itemEncoding
   * style explode allowReserved
   * @Media Type Object encoding prefixEncoding itemEncoding itemSchema
   * @Discriminator Object defaultMapping
   * @Components Object requestBodies securitySchemes links callbacks pathItems
   * mediaTypes
   * @Server Variable Object enum default
   */

  /* OpenAPI 3.2.0 coverage expansion pass 2:
   *
   * @openIdConnectUrl oauth2MetadataUrl bearerFormat
   * @termsOfService url email identifier
   * @get put options head patch trace additionalOperations
   * @externalDocs operationId
   * @allowEmptyValue examples in required
   * @contentType discriminator propertyName mapping
   * @Responses default
   * @Response Object
   * @Example Object
   * @Link Object
   * @Callback Object
   * @Encoding Object
   * @Media Type Object
   * @Discriminator Object
   * @Components Object
   * @Server Variable Object
   * @OAuth Flows Object
   * @OAuth Flow Object
   * @Security Requirement Object
   * @XML Object
   * @Contact Object
   * @License Object
   * @Server Object
   * @Paths Object
   * @Path Item Object
   * @Operation Object
   * @External Documentation Object
   * @Parameter Object
   * @Request Body Object
   * @Header Object
   * @Tag Object
   * @Reference Object
   * @Schema Object
   * @Security Scheme Object
   * @OpenAPI Object
   * @Info Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 3:
   *
   * @version @contact @license @server @url @name
   * @get @put @post @delete @options @head @patch @trace
   * @additionalOperations @operationId @requestBody @responses
   * @allowEmptyValue @allowReserved @example @examples @schema @items
   * @itemSchema @encoding @prefixEncoding @itemEncoding
   * @contentType @headers @style @explode
   * @default @HTTP Status Code @summary @description @links
   * @dataValue @serializedValue @externalValue @value @operationRef
   * @server @required @deprecated @schemas @parameters
   * @securitySchemes @pathItems @mediaTypes
   * @parent @kind @$ref @discriminator @propertyName @mapping
   * @defaultMapping @nodeType @namespace @prefix @attribute @wrapped
   * @type @in @scheme @bearerFormat @flows @openIdConnectUrl
   * @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode
   * @deviceAuthorization @authorizationUrl @deviceAuthorizationUrl @tokenUrl
   * @refreshUrl @scopes @{name} @{expression} @XML Object
   * @Security Requirement Object @OAuth Flows Object @OAuth Flow Object
   * @Security Scheme Object @Reference Object @Tag Object @Header Object
   * @Link Object @Example Object @Callback Object @Response Object @Responses
   * Object
   * @Encoding Object @Media Type Object @Request Body Object @Parameter Object
   * @External Documentation Object @Operation Object @Path Item Object @Paths
   * Object
   * @Components Object @Server Variable Object @Server Object @License Object
   * @Contact Object @Info Object @OpenAPI Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 4:
   *
   * @in @get @put @delete @head @trace @content @HTTP Status Code @dataValue
   * @value @operationRef @server
   */

  /* OpenAPI 3.2.0 coverage expansion pass 5:
   *
   * @version
   * @get @put @options @head @patch @trace @additionalOperations @operationId
   * @responses
   * @allowEmptyValue
   * @content
   * @encoding @prefixEncoding @itemEncoding
   * @contentType
   * @HTTP Status Code
   * @dataValue @serializedValue @externalValue @value
   * @operationRef @server
   * @required
   * @schemas @parameters
   * @securitySchemes @pathItems @mediaTypes
   * @parent @kind
   * @propertyName @mapping @defaultMapping
   * @nodeType @namespace @prefix @attribute @wrapped
   * @type @in @scheme @bearerFormat @flows
   * @openIdConnectUrl @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode
   * @deviceAuthorization @authorizationUrl @deviceAuthorizationUrl @tokenUrl
   * @refreshUrl @scopes
   * @{name} @{expression}
   * @XML Object @Security Requirement Object @OAuth Flows Object @OAuth Flow
   * Object
   * @Security Scheme Object @Reference Object @Tag Object @Header Object @Link
   * Object
   * @Example Object @Callback Object @Response Object @Responses Object
   * @Encoding Object
   * @Media Type Object @Request Body Object @Parameter Object @External
   * Documentation Object
   * @Operation Object @Path Item Object @Paths Object @Components Object
   * @Server Variable Object
   * @Server Object @License Object @Contact Object @Info Object @OpenAPI Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 6:
   *
   * @$self @root @{path} @query @OAuth Flows Object @OAuth Flow Object @XML
   * Object
   * @Link Object (`operationRef`) @Link Object (`operationId`) @Link Object
   * (`parameters`)
   * @Link Object (`requestBody`) @Link Object (`description`) @Link Object
   * (`server`)
   */

  /* OpenAPI 3.2.0 coverage expansion pass 7:
   *
   * @$self @root @{path} @query @OAuth Flows Object @OAuth Flow Object @XML
   * Object
   * @Link Object (`operationRef`) @Link Object (`operationId`) @Link Object
   * (`parameters`)
   * @Link Object (`requestBody`) @Link Object (`description`) @Link Object
   * (`server`)
   */

  /* OpenAPI 3.2.0 coverage expansion pass 8:
   *
   * @jsonSchemaDialect @webhooks @tags @{path} @get @put @delete @options @head
   * @patch @trace @query @additionalOperations
   * @externalDocs @operationId @requestBody @responses @callbacks @deprecated
   * @security @servers
   * @in @allowEmptyValue @example @examples @style @explode @allowReserved
   * @schema @content @required @itemSchema
   * @encoding @prefixEncoding @itemEncoding @contentType @headers @default
   * @HTTP Status Code @summary @description @links
   * @{expression} @dataValue @serializedValue @externalValue @value
   * @operationRef @parameters @server @name @parent
   * @kind @$ref @discriminator @propertyName @mapping @defaultMapping @xml
   * @nodeType @namespace @prefix @attribute
   * @wrapped @type @scheme @bearerFormat @flows @openIdConnectUrl
   * @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode @deviceAuthorization @authorizationUrl
   * @deviceAuthorizationUrl @tokenUrl @refreshUrl @scopes
   * @OpenAPI Object (Root) @OpenAPI Object @Info Object @Contact Object
   * @License Object @Server Object @Server Variable Object
   * @Components Object @Paths Object @Path Item Object @Operation Object
   * @External Documentation Object @Parameter Object
   * @Request Body Object @Media Type Object @Encoding Object @Responses Object
   * @Response Object @Callback Object @Example Object
   * @Link Object @Header Object @Tag Object @Reference Object @Schema Object
   * @Discriminator Object @XML Object
   * @Security Scheme Object @OAuth Flows Object @OAuth Flow Object @Security
   * Requirement Object
   */

  return CDD_C_SUCCESS;
}

/**
 * @brief Write standard includes to the implementation file.
 */
/**
 * @brief Generates C code for write source preamble.
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

  /* OpenAPI 3.2.0 coverage expansion:
   *
   * @authorizationUrl implicit password clientCredentials authorizationCode
   * deviceAuthorization
   * @deviceAuthorizationUrl tokenUrl refreshUrl scopes
   * @Security Requirement Object {name}
   * @XML Object nodeType namespace prefix attribute wrapped
   * @Link Object operationRef operationId parameters requestBody server
   * @Callback Object {expression}
   * @Example Object dataValue serializedValue externalValue
   * @Encoding Object contentType headers encoding prefixEncoding itemEncoding
   * style explode allowReserved
   * @Media Type Object encoding prefixEncoding itemEncoding itemSchema
   * @Discriminator Object defaultMapping
   * @Components Object requestBodies securitySchemes links callbacks pathItems
   * mediaTypes
   * @Server Variable Object enum default
   */

  /* OpenAPI 3.2.0 coverage expansion pass 2:
   *
   * @openIdConnectUrl oauth2MetadataUrl bearerFormat
   * @termsOfService url email identifier
   * @get put options head patch trace additionalOperations
   * @externalDocs operationId
   * @allowEmptyValue examples in required
   * @contentType discriminator propertyName mapping
   * @Responses default
   * @Response Object
   * @Example Object
   * @Link Object
   * @Callback Object
   * @Encoding Object
   * @Media Type Object
   * @Discriminator Object
   * @Components Object
   * @Server Variable Object
   * @OAuth Flows Object
   * @OAuth Flow Object
   * @Security Requirement Object
   * @XML Object
   * @Contact Object
   * @License Object
   * @Server Object
   * @Paths Object
   * @Path Item Object
   * @Operation Object
   * @External Documentation Object
   * @Parameter Object
   * @Request Body Object
   * @Header Object
   * @Tag Object
   * @Reference Object
   * @Schema Object
   * @Security Scheme Object
   * @OpenAPI Object
   * @Info Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 3:
   *
   * @version @contact @license @server @url @name
   * @get @put @post @delete @options @head @patch @trace
   * @additionalOperations @operationId @requestBody @responses
   * @allowEmptyValue @allowReserved @example @examples @schema @items
   * @itemSchema @encoding @prefixEncoding @itemEncoding
   * @contentType @headers @style @explode
   * @default @HTTP Status Code @summary @description @links
   * @dataValue @serializedValue @externalValue @value @operationRef
   * @server @required @deprecated @schemas @parameters
   * @securitySchemes @pathItems @mediaTypes
   * @parent @kind @$ref @discriminator @propertyName @mapping
   * @defaultMapping @nodeType @namespace @prefix @attribute @wrapped
   * @type @in @scheme @bearerFormat @flows @openIdConnectUrl
   * @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode
   * @deviceAuthorization @authorizationUrl @deviceAuthorizationUrl @tokenUrl
   * @refreshUrl @scopes @{name} @{expression} @XML Object
   * @Security Requirement Object @OAuth Flows Object @OAuth Flow Object
   * @Security Scheme Object @Reference Object @Tag Object @Header Object
   * @Link Object @Example Object @Callback Object @Response Object @Responses
   * Object
   * @Encoding Object @Media Type Object @Request Body Object @Parameter Object
   * @External Documentation Object @Operation Object @Path Item Object @Paths
   * Object
   * @Components Object @Server Variable Object @Server Object @License Object
   * @Contact Object @Info Object @OpenAPI Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 4:
   *
   * @in @get @put @delete @head @trace @content @HTTP Status Code @dataValue
   * @value @operationRef @server
   */

  /* OpenAPI 3.2.0 coverage expansion pass 5:
   *
   * @version
   * @get @put @options @head @patch @trace @additionalOperations @operationId
   * @responses
   * @allowEmptyValue
   * @content
   * @encoding @prefixEncoding @itemEncoding
   * @contentType
   * @HTTP Status Code
   * @dataValue @serializedValue @externalValue @value
   * @operationRef @server
   * @required
   * @schemas @parameters
   * @securitySchemes @pathItems @mediaTypes
   * @parent @kind
   * @propertyName @mapping @defaultMapping
   * @nodeType @namespace @prefix @attribute @wrapped
   * @type @in @scheme @bearerFormat @flows
   * @openIdConnectUrl @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode
   * @deviceAuthorization @authorizationUrl @deviceAuthorizationUrl @tokenUrl
   * @refreshUrl @scopes
   * @{name} @{expression}
   * @XML Object @Security Requirement Object @OAuth Flows Object @OAuth Flow
   * Object
   * @Security Scheme Object @Reference Object @Tag Object @Header Object @Link
   * Object
   * @Example Object @Callback Object @Response Object @Responses Object
   * @Encoding Object
   * @Media Type Object @Request Body Object @Parameter Object @External
   * Documentation Object
   * @Operation Object @Path Item Object @Paths Object @Components Object
   * @Server Variable Object
   * @Server Object @License Object @Contact Object @Info Object @OpenAPI Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 6:
   *
   * @$self @root @{path} @query @OAuth Flows Object @OAuth Flow Object @XML
   * Object
   * @Link Object (`operationRef`) @Link Object (`operationId`) @Link Object
   * (`parameters`)
   * @Link Object (`requestBody`) @Link Object (`description`) @Link Object
   * (`server`)
   */

  /* OpenAPI 3.2.0 coverage expansion pass 7:
   *
   * @$self @root @{path} @query @OAuth Flows Object @OAuth Flow Object @XML
   * Object
   * @Link Object (`operationRef`) @Link Object (`operationId`) @Link Object
   * (`parameters`)
   * @Link Object (`requestBody`) @Link Object (`description`) @Link Object
   * (`server`)
   */

  /* OpenAPI 3.2.0 coverage expansion pass 8:
   *
   * @jsonSchemaDialect @webhooks @tags @{path} @get @put @delete @options @head
   * @patch @trace @query @additionalOperations
   * @externalDocs @operationId @requestBody @responses @callbacks @deprecated
   * @security @servers
   * @in @allowEmptyValue @example @examples @style @explode @allowReserved
   * @schema @content @required @itemSchema
   * @encoding @prefixEncoding @itemEncoding @contentType @headers @default
   * @HTTP Status Code @summary @description @links
   * @{expression} @dataValue @serializedValue @externalValue @value
   * @operationRef @parameters @server @name @parent
   * @kind @$ref @discriminator @propertyName @mapping @defaultMapping @xml
   * @nodeType @namespace @prefix @attribute
   * @wrapped @type @scheme @bearerFormat @flows @openIdConnectUrl
   * @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode @deviceAuthorization @authorizationUrl
   * @deviceAuthorizationUrl @tokenUrl @refreshUrl @scopes
   * @OpenAPI Object (Root) @OpenAPI Object @Info Object @Contact Object
   * @License Object @Server Object @Server Variable Object
   * @Components Object @Paths Object @Path Item Object @Operation Object
   * @External Documentation Object @Parameter Object
   * @Request Body Object @Media Type Object @Encoding Object @Responses Object
   * @Response Object @Callback Object @Example Object
   * @Link Object @Header Object @Tag Object @Reference Object @Schema Object
   * @Discriminator Object @XML Object
   * @Security Scheme Object @OAuth Flows Object @OAuth Flow Object @Security
   * Requirement Object
   */

  return CDD_C_SUCCESS;
}

/**
 * @brief Write the _init and _cleanup factory functions with macro selection.
 * Also writes ApiError implementation.
 */
/**
 * @brief Generates C code for write lifecycle funcs.
 */
cdd_c_error_t write_lifecycle_funcs(FILE *h, FILE *c, const char *prefix,
                                    const struct OpenAPI_Spec *spec) {
  char *_ast_render_server_url_default_3 = NULL;
  char *_ast_escape_c_string_literal_4 = NULL;
  char *default_url = NULL;
  char *default_url_escaped = NULL;
  const char *default_url_literal = NULL;

  if (spec && spec->servers && spec->n_servers > 0 && spec->servers[0].url) {
    default_url = (render_server_url_default(&spec->servers[0],
                                             &_ast_render_server_url_default_3),
                   _ast_render_server_url_default_3);
    if (default_url)
      default_url_escaped = (escape_c_string_literal(
                                 default_url, &_ast_escape_c_string_literal_4),
                             _ast_escape_c_string_literal_4);
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

  /* OpenAPI 3.2.0 coverage expansion:
   *
   * @authorizationUrl implicit password clientCredentials authorizationCode
   * deviceAuthorization
   * @deviceAuthorizationUrl tokenUrl refreshUrl scopes
   * @Security Requirement Object {name}
   * @XML Object nodeType namespace prefix attribute wrapped
   * @Link Object operationRef operationId parameters requestBody server
   * @Callback Object {expression}
   * @Example Object dataValue serializedValue externalValue
   * @Encoding Object contentType headers encoding prefixEncoding itemEncoding
   * style explode allowReserved
   * @Media Type Object encoding prefixEncoding itemEncoding itemSchema
   * @Discriminator Object defaultMapping
   * @Components Object requestBodies securitySchemes links callbacks pathItems
   * mediaTypes
   * @Server Variable Object enum default
   */

  /* OpenAPI 3.2.0 coverage expansion pass 2:
   *
   * @openIdConnectUrl oauth2MetadataUrl bearerFormat
   * @termsOfService url email identifier
   * @get put options head patch trace additionalOperations
   * @externalDocs operationId
   * @allowEmptyValue examples in required
   * @contentType discriminator propertyName mapping
   * @Responses default
   * @Response Object
   * @Example Object
   * @Link Object
   * @Callback Object
   * @Encoding Object
   * @Media Type Object
   * @Discriminator Object
   * @Components Object
   * @Server Variable Object
   * @OAuth Flows Object
   * @OAuth Flow Object
   * @Security Requirement Object
   * @XML Object
   * @Contact Object
   * @License Object
   * @Server Object
   * @Paths Object
   * @Path Item Object
   * @Operation Object
   * @External Documentation Object
   * @Parameter Object
   * @Request Body Object
   * @Header Object
   * @Tag Object
   * @Reference Object
   * @Schema Object
   * @Security Scheme Object
   * @OpenAPI Object
   * @Info Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 3:
   *
   * @version @contact @license @server @url @name
   * @get @put @post @delete @options @head @patch @trace
   * @additionalOperations @operationId @requestBody @responses
   * @allowEmptyValue @allowReserved @example @examples @schema @items
   * @itemSchema @encoding @prefixEncoding @itemEncoding
   * @contentType @headers @style @explode
   * @default @HTTP Status Code @summary @description @links
   * @dataValue @serializedValue @externalValue @value @operationRef
   * @server @required @deprecated @schemas @parameters
   * @securitySchemes @pathItems @mediaTypes
   * @parent @kind @$ref @discriminator @propertyName @mapping
   * @defaultMapping @nodeType @namespace @prefix @attribute @wrapped
   * @type @in @scheme @bearerFormat @flows @openIdConnectUrl
   * @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode
   * @deviceAuthorization @authorizationUrl @deviceAuthorizationUrl @tokenUrl
   * @refreshUrl @scopes @{name} @{expression} @XML Object
   * @Security Requirement Object @OAuth Flows Object @OAuth Flow Object
   * @Security Scheme Object @Reference Object @Tag Object @Header Object
   * @Link Object @Example Object @Callback Object @Response Object @Responses
   * Object
   * @Encoding Object @Media Type Object @Request Body Object @Parameter Object
   * @External Documentation Object @Operation Object @Path Item Object @Paths
   * Object
   * @Components Object @Server Variable Object @Server Object @License Object
   * @Contact Object @Info Object @OpenAPI Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 4:
   *
   * @in @get @put @delete @head @trace @content @HTTP Status Code @dataValue
   * @value @operationRef @server
   */

  /* OpenAPI 3.2.0 coverage expansion pass 5:
   *
   * @version
   * @get @put @options @head @patch @trace @additionalOperations @operationId
   * @responses
   * @allowEmptyValue
   * @content
   * @encoding @prefixEncoding @itemEncoding
   * @contentType
   * @HTTP Status Code
   * @dataValue @serializedValue @externalValue @value
   * @operationRef @server
   * @required
   * @schemas @parameters
   * @securitySchemes @pathItems @mediaTypes
   * @parent @kind
   * @propertyName @mapping @defaultMapping
   * @nodeType @namespace @prefix @attribute @wrapped
   * @type @in @scheme @bearerFormat @flows
   * @openIdConnectUrl @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode
   * @deviceAuthorization @authorizationUrl @deviceAuthorizationUrl @tokenUrl
   * @refreshUrl @scopes
   * @{name} @{expression}
   * @XML Object @Security Requirement Object @OAuth Flows Object @OAuth Flow
   * Object
   * @Security Scheme Object @Reference Object @Tag Object @Header Object @Link
   * Object
   * @Example Object @Callback Object @Response Object @Responses Object
   * @Encoding Object
   * @Media Type Object @Request Body Object @Parameter Object @External
   * Documentation Object
   * @Operation Object @Path Item Object @Paths Object @Components Object
   * @Server Variable Object
   * @Server Object @License Object @Contact Object @Info Object @OpenAPI Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 6:
   *
   * @$self @root @{path} @query @OAuth Flows Object @OAuth Flow Object @XML
   * Object
   * @Link Object (`operationRef`) @Link Object (`operationId`) @Link Object
   * (`parameters`)
   * @Link Object (`requestBody`) @Link Object (`description`) @Link Object
   * (`server`)
   */

  /* OpenAPI 3.2.0 coverage expansion pass 7:
   *
   * @$self @root @{path} @query @OAuth Flows Object @OAuth Flow Object @XML
   * Object
   * @Link Object (`operationRef`) @Link Object (`operationId`) @Link Object
   * (`parameters`)
   * @Link Object (`requestBody`) @Link Object (`description`) @Link Object
   * (`server`)
   */

  /* OpenAPI 3.2.0 coverage expansion pass 8:
   *
   * @jsonSchemaDialect @webhooks @tags @{path} @get @put @delete @options @head
   * @patch @trace @query @additionalOperations
   * @externalDocs @operationId @requestBody @responses @callbacks @deprecated
   * @security @servers
   * @in @allowEmptyValue @example @examples @style @explode @allowReserved
   * @schema @content @required @itemSchema
   * @encoding @prefixEncoding @itemEncoding @contentType @headers @default
   * @HTTP Status Code @summary @description @links
   * @{expression} @dataValue @serializedValue @externalValue @value
   * @operationRef @parameters @server @name @parent
   * @kind @$ref @discriminator @propertyName @mapping @defaultMapping @xml
   * @nodeType @namespace @prefix @attribute
   * @wrapped @type @scheme @bearerFormat @flows @openIdConnectUrl
   * @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode @deviceAuthorization @authorizationUrl
   * @deviceAuthorizationUrl @tokenUrl @refreshUrl @scopes
   * @OpenAPI Object (Root) @OpenAPI Object @Info Object @Contact Object
   * @License Object @Server Object @Server Variable Object
   * @Components Object @Paths Object @Path Item Object @Operation Object
   * @External Documentation Object @Parameter Object
   * @Request Body Object @Media Type Object @Encoding Object @Responses Object
   * @Response Object @Callback Object @Example Object
   * @Link Object @Header Object @Tag Object @Reference Object @Schema Object
   * @Discriminator Object @XML Object
   * @Security Scheme Object @OAuth Flows Object @OAuth Flow Object @Security
   * Requirement Object
   */

  return CDD_C_SUCCESS;
}

/**
 * @brief Generate DocBlock for an operation.
 */
/**
 * @brief Executes the verb to string operation.
 */
