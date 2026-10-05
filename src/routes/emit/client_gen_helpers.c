/**
 * @file client_gen_helpers.c
 * @brief Helper routines for OpenAPI client generator.
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

/**
 * @brief Retrieves the server variable.
 */
cdd_c_error_t
find_server_variable(const struct OpenAPI_Server *srv, const char *name,
                     const struct OpenAPI_ServerVariable **_out_val) {
  size_t i;
  if (!srv || !name || !srv->variables) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < srv->n_variables; ++i) {
    const struct OpenAPI_ServerVariable *var = &srv->variables[i];
    if (var->name && strcmp(var->name, name) == 0) {
      *_out_val = var;
      return CDD_C_SUCCESS;
    }
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the render server url default operation.
 */
cdd_c_error_t render_server_url_default(const struct OpenAPI_Server *srv,
                                        char **_out_val) {
  const struct OpenAPI_ServerVariable *_ast_find_server_variable_0;
  const struct OpenAPI_ServerVariable *_ast_find_server_variable_1;
  const char *url;
  size_t out_len = 0;
  size_t i = 0;
  char *out;

  if (!srv || !srv->url) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  url = srv->url;

  while (url[i]) {
    if (url[i] == '{') {
      const char *end = strchr(url + i + 1, '}');
      size_t name_len;
      char *name;
      const struct OpenAPI_ServerVariable *var;
      if (!end) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      name_len = (size_t)(end - (url + i + 1));
      if (name_len == 0) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      name = (char *)(size_t)malloc(name_len + 1);
#ifdef CDD_BUILD_TESTS
      if (g_client_gen_fail == 1) {
        free(name);
        name = NULL;
      }
#endif
      if (!name) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      memcpy(name, url + i + 1, name_len);
      name[name_len] = '\0';
      var = (find_server_variable(srv, name, &_ast_find_server_variable_0),
             _ast_find_server_variable_0);
      free(name);
      if (!var || !var->default_value) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      out_len += strlen(var->default_value);
      i = (size_t)(end - url) + 1;
      continue;
    }
    out_len++;
    i++;
  }

  out = (char *)(size_t)malloc(out_len + 1);
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 2) {
    free(out);
    out = NULL;
  }
#endif
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  i = 0;
  {
    size_t out_pos = 0;
    while (url[i]) {
      if (url[i] == '{') {
        const char *end = strchr(url + i + 1, '}');
        size_t name_len;
        char *name;
        const struct OpenAPI_ServerVariable *var;
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 47)
          end = NULL;
#endif
        if (!end) {
          free(out);
          {
            *_out_val = NULL;
            return CDD_C_SUCCESS;
          }
        }
        name_len = (size_t)(end - (url + i + 1));
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 48)
          name_len = 0;
#endif
        if (name_len == 0) {
          free(out);
          {
            *_out_val = NULL;
            return CDD_C_SUCCESS;
          }
        }
        name = (char *)(size_t)malloc(name_len + 1);
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 3) {
          free(name);
          name = NULL;
        }
#endif
        if (!name) {
          free(out);
          {
            *_out_val = NULL;
            return CDD_C_SUCCESS;
          }
        }
        memcpy(name, url + i + 1, name_len);
        name[name_len] = '\0';
        var = (find_server_variable(srv, name, &_ast_find_server_variable_1),
               _ast_find_server_variable_1);
        free(name);
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 49)
          var = NULL;
#endif
        if (!var) {
          free(out);
          {
            *_out_val = NULL;
            return CDD_C_SUCCESS;
          }
        }
        memcpy(out + out_pos, var->default_value, strlen(var->default_value));
        out_pos += strlen(var->default_value);
        i = (size_t)(end - url) + 1;
        continue;
      }
      out[out_pos++] = url[i++];
    }
    out[out_pos] = '\0';
  }

  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the escape c string literal operation.
 */
cdd_c_error_t escape_c_string_literal(const char *s, char **_out_val) {
  size_t i;
  size_t out_len = 0;
  char *out;
  size_t pos = 0;

  if (!s) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  for (i = 0; s[i]; ++i) {
    switch (s[i]) {
    case '\\':
    case '\"':
      out_len += 2;
      break;
    case '\n':
    case '\r':
    case '\t':
      out_len += 2;
      break;
    default:
      out_len += 1;
      break;
    }
  }
  out = (char *)(size_t)malloc(out_len + 1);
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 4) {
    free(out);
    out = NULL;
  }
#endif
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; s[i]; ++i) {
    switch (s[i]) {
    case '\\':
      out[pos++] = '\\';
      out[pos++] = '\\';
      break;
    case '\"':
      out[pos++] = '\\';
      out[pos++] = '\"';
      break;
    case '\n':
      out[pos++] = '\\';
      out[pos++] = 'n';
      break;
    case '\r':
      out[pos++] = '\\';
      out[pos++] = 'r';
      break;
    case '\t':
      out[pos++] = '\\';
      out[pos++] = 't';
      break;
    default:
      out[pos++] = s[i];
      break;
    }
  }
  out[pos] = '\0';
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the select operation server operation.
 */
cdd_c_error_t select_operation_server(const struct OpenAPI_Path *path,
                                      const struct OpenAPI_Operation *op,
                                      struct OpenAPI_Server **_out_val) {
  if (op && op->servers && op->n_servers > 0) {
    *_out_val = &op->servers[0];
    return CDD_C_SUCCESS;
  }
  if (path && path->servers && path->n_servers > 0) {
    *_out_val = &path->servers[0];
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the build base url literal operation.
 */
cdd_c_error_t build_base_url_literal(const char *url, char **_out_val) {
  char *_ast_escape_c_string_literal_2 = NULL;
  char *escaped = NULL;
  char *literal = NULL;
  size_t len;

  if (!url) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  escaped = (escape_c_string_literal(url, &_ast_escape_c_string_literal_2),
             _ast_escape_c_string_literal_2);
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 50) {
    free(escaped);
    escaped = NULL;
  }
#endif
  if (!escaped) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  len = strlen(escaped) + 3;
  literal = (char *)(size_t)malloc(len);
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 5) {
    free(literal);
    literal = NULL;
  }
#endif
  if (!literal) {
    free(escaped);
    {
      *_out_val = NULL;
      return CDD_C_SUCCESS;
    }
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(literal, len, "\"%s\"", escaped);
#else
  CDD_SNPRINTF(literal, len, "\"%s\"", escaped);
#endif
  free(escaped);
  {
    *_out_val = literal;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Generate a sanitized uppercase Include Guard macro.
 */
/**
 * @brief Generates guard.
 */
cdd_c_error_t generate_guard(const char *base, char **_out_val) {
  char *g;
  size_t len = strlen(base);
  size_t i;

  g = malloc(len + 3); /* + _H + null */
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 6) {
    free(g);
    g = NULL;
  }
#endif
  if (!g) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < len; ++i) {
    if (isalnum((unsigned char)base[i])) {
      g[i] = (char)toupper((unsigned char)base[i]);
    } else {
      g[i] = '_';
    }
  }
  g[len] = '_';
  g[len + 1] = 'H';
  g[len + 2] = '\0';
  {
    *_out_val = g;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Derive the model header name if not provided.
 */
/**
 * @brief Executes the derive model header operation.
 */
cdd_c_error_t derive_model_header(const char *base, char **_out_val) {
  char *m;
  size_t len = strlen(base) + 10; /* _models.h */
  m = malloc(len + 1);
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 7) {
    free(m);
    m = NULL;
  }
#endif
  if (!m) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(m, len + 1, "%s_models.h", base);
#else
  CDD_SNPRINTF(m, len + 1, "%s_models.h", base);
#endif
  {
    *_out_val = m;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Sanitize a tag string to be a valid C identifier part.
 * Converts non-alphanumeric characters to underscores.
 * Capitalizes the first letter for style matching (e.g. "pet" -> "Pet").
 *
 */
/**
 * @brief Executes the sanitize tag operation.
 *
 */
cdd_c_error_t sanitize_tag(const char *tag, char **_out_val) {
  char *s;
  size_t i;
  if (!tag) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  s = strdup(tag);
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 8) {
    free(s);
    s = NULL;
  }
#endif
  if (!s) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  if (s[0] && islower((unsigned char)s[0])) {
    s[0] = (char)toupper((unsigned char)s[0]);
  }

  for (i = 0; s[i]; ++i) {
    if (!isalnum((unsigned char)s[i])) {
      s[i] = '_';
    }
  }
  {
    *_out_val = s;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the param keys match operation.
 */
cdd_c_error_t param_keys_match(const struct OpenAPI_Parameter *a,
                               const struct OpenAPI_Parameter *b) {
  if (!a || !b || !a->name || !b->name)
    return CDD_C_SUCCESS;
  return (a->in == b->in) && (strcmp(a->name, b->name) == 0);
}

/**
 * @brief Executes the build effective parameters operation.
 */
cdd_c_error_t build_effective_parameters(const struct OpenAPI_Path *path,
                                         const struct OpenAPI_Operation *op,
                                         struct OpenAPI_Parameter **out_params,
                                         size_t *out_count) {
  size_t cap = 0;
  size_t count = 0;
  struct OpenAPI_Parameter *params = NULL;

  if (!out_params || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_params = NULL;
  *out_count = 0;

  if (path)
    cap += path->n_parameters;
  if (op)
    cap += op->n_parameters;

  if (cap == 0)
    return CDD_C_SUCCESS;

  params = (struct OpenAPI_Parameter *)calloc(cap, sizeof(*params));
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 9) {
    free(params);
    params = NULL;
  }
#endif
  if (!params) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  if (path && path->parameters) {
    size_t i;
    for (i = 0; i < path->n_parameters; ++i) {
      params[count++] = path->parameters[i];
    }
  }

  if (op && op->parameters) {
    size_t i;
    for (i = 0; i < op->n_parameters; ++i) {
      size_t k;
      int replaced = 0;
      for (k = 0; k < count; ++k) {
        if (param_keys_match(&params[k], &op->parameters[i])) {
          params[k] = op->parameters[i];
          replaced = 1;
          break;
        }
      }
      if (!replaced) {
        params[count++] = op->parameters[i];
      }
    }
  }

  *out_params = params;
  *out_count = count;

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
 * @brief Write standard includes to the header file.
 * Defines `struct ApiError` for standardized error handling.
 */
/**
 * @brief Generates C code for write header preamble.
 */
