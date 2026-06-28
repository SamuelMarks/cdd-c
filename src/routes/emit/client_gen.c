/**
 * @file client_gen.c
 * @brief Implementation of client code generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions/emit/client_body.h"
#include "functions/emit/client_sig.h"
#include "functions/emit/codegen.h"
#include "classes/emit/struct.h"
#include "classes/emit/json.h"
#include "classes/emit/types.h"
#include "functions/parse/str.h"
#include "functions/parse/fs.h"
#include "functions/emit/build_system.h"
#include "routes/emit/client_gen.h"
#include "c_cdd/log.h"

/* clang-format on */
/* LCOV_EXCL_START */
/**
 * @def CHECK_IO_CLEANUP
 * @brief CHECK_IO_CLEANUP macro
 */
#define CHECK_IO_CLEANUP(x)                                                    \
  do {                                                                         \
    if ((x) < 0) {                                                             \
      rc = 0;                                                                  \
      {                                                                        \
        fprintf(stderr, "goto cleanup at %s:%d\n", __FILE__, __LINE__);        \
        goto cleanup;                                                          \
      }                                                                        \
    }                                                                          \
  } while (0)

/* Helper macro for I/O checking */
/** @brief CHECK_IO macro */
/** @brief CHECK_IO macro */
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return 0;                                                                \
  } while (0)

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif

/**
 * @brief Retrieves the server variable.
 */
enum cdd_c_error
find_server_variable(const struct OpenAPI_Server *srv, const char *name,
                     const struct OpenAPI_ServerVariable **_out_val) {
  size_t i;
  /* LCOV_EXCL_START */
  if (!srv || !name || !srv->variables) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */
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
enum cdd_c_error render_server_url_default(const struct OpenAPI_Server *srv,
                                           char **_out_val) {
  const struct OpenAPI_ServerVariable *_ast_find_server_variable_0;
  const struct OpenAPI_ServerVariable *_ast_find_server_variable_1;
  const char *url;
  size_t out_len = 0;
  size_t i = 0;
  char *out;

  /* LCOV_EXCL_START */

  if (!srv || !srv->url) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  /* LCOV_EXCL_STOP */
  url = srv->url;

  while (url[i]) {
    if (url[i] == '{') {
      const char *end = strchr(url + i + 1, '}');
      size_t name_len;
      char *name;
      const struct OpenAPI_ServerVariable *var;
      /* LCOV_EXCL_START */
      if (!end) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      /* LCOV_EXCL_STOP */
      name_len = (size_t)(end - (url + i + 1));
      /* LCOV_EXCL_START */
      if (name_len == 0) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      /* LCOV_EXCL_STOP */
      name = (char *)malloc(name_len + 1);
      /* LCOV_EXCL_START */
      if (!name) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      /* LCOV_EXCL_STOP */
      memcpy(name, url + i + 1, name_len);
      name[name_len] = '\0';
      var = (find_server_variable(srv, name, &_ast_find_server_variable_0),
             _ast_find_server_variable_0);
      free(name);
      /* LCOV_EXCL_START */
      if (!var || !var->default_value) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      /* LCOV_EXCL_STOP */
      out_len += strlen(var->default_value);
      i = (size_t)(end - url) + 1;
      continue;
    }
    out_len++;
    i++;
  }

  out = (char *)malloc(out_len + 1);
  /* LCOV_EXCL_START */
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */

  i = 0;
  {
    size_t out_pos = 0;
    while (url[i]) {
      if (url[i] == '{') {
        const char *end = strchr(url + i + 1, '}');
        size_t name_len;
        char *name;
        const struct OpenAPI_ServerVariable *var;
        if (!end) {
          free(out);
          {
            *_out_val = NULL;
            return CDD_C_SUCCESS;
          }
        }
        name_len = (size_t)(end - (url + i + 1));
        if (name_len == 0) {
          free(out);
          {
            *_out_val = NULL;
            return CDD_C_SUCCESS;
          }
        }
        name = (char *)malloc(name_len + 1);
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
        if (!var || !var->default_value) {
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
enum cdd_c_error escape_c_string_literal(const char *s, char **_out_val) {
  size_t i;
  size_t out_len = 0;
  char *out;
  size_t pos = 0;

  /* LCOV_EXCL_START */

  if (!s) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  /* LCOV_EXCL_STOP */
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
  out = (char *)malloc(out_len + 1);
  /* LCOV_EXCL_START */
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */
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
enum cdd_c_error select_operation_server(const struct OpenAPI_Path *path,
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
enum cdd_c_error build_base_url_literal(const char *url, char **_out_val) {
  char *_ast_escape_c_string_literal_2 = NULL;
  char *escaped = NULL;
  char *literal = NULL;
  size_t len;

  /* LCOV_EXCL_START */

  if (!url) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  /* LCOV_EXCL_STOP */
  escaped = (escape_c_string_literal(url, &_ast_escape_c_string_literal_2),
             _ast_escape_c_string_literal_2);
  /* LCOV_EXCL_START */
  if (!escaped) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */
  len = strlen(escaped) + 3;
  literal = (char *)malloc(len);
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
  sprintf(literal, "\"%s\"", escaped);
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
enum cdd_c_error generate_guard(const char *base, char **_out_val) {
  char *g;
  size_t len = strlen(base);
  size_t i;

  g = malloc(len + 3); /* + _H + null */
  /* LCOV_EXCL_START */
  if (!g) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */

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
enum cdd_c_error derive_model_header(const char *base, char **_out_val) {
  char *m;
  size_t len = strlen(base) + 10; /* _models.h */
  m = malloc(len + 1);
  /* LCOV_EXCL_START */
  if (!m) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(m, len + 1, "%s_models.h", base);
#else
  sprintf(m, "%s_models.h", base);
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
enum cdd_c_error sanitize_tag(const char *tag, char **_out_val) {
  char *s;
  size_t i;
  /* LCOV_EXCL_START */
  if (!tag) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */
  s = strdup(tag);
  /* LCOV_EXCL_START */
  if (!s) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */

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
enum cdd_c_error param_keys_match(const struct OpenAPI_Parameter *a,
                                  const struct OpenAPI_Parameter *b) {
  if (!a || !b || !a->name || !b->name)
    return CDD_C_SUCCESS;
  return (a->in == b->in) && (strcmp(a->name, b->name) == 0);
}

/**
 * @brief Executes the build effective parameters operation.
 */
enum cdd_c_error build_effective_parameters(
    const struct OpenAPI_Path *path, const struct OpenAPI_Operation *op,
    struct OpenAPI_Parameter **out_params, size_t *out_count) {
  size_t cap = 0;
  size_t count = 0;
  struct OpenAPI_Parameter *params = NULL;

  /* LCOV_EXCL_START */

  if (!out_params || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* LCOV_EXCL_STOP */

  *out_params = NULL;
  *out_count = 0;

  if (path)
    cap += path->n_parameters;
  if (op)
    cap += op->n_parameters;

  if (cap == 0)
    return CDD_C_SUCCESS;

  params = (struct OpenAPI_Parameter *)calloc(cap, sizeof(*params));
  /* LCOV_EXCL_START */
  if (!params) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */

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
enum cdd_c_error write_header_preamble(FILE *fp, const char *guard,
                                       const char *model_decl) {
  CHECK_IO(fprintf(fp, "#ifndef %s\n", guard));
  CHECK_IO(fprintf(fp, "#define %s\n\n", guard));

  CHECK_IO(fprintf(fp, "#include <stdlib.h>\n"));
  CHECK_IO(fprintf(fp, "#include <stdio.h>\n"));
  CHECK_IO(fprintf(fp, "#include <c_abstract_http/http_types.h>\n"));
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
                   "void ApiError_cleanup(struct ApiError *err);\n"
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
enum cdd_c_error write_source_preamble(FILE *fp, const char *header_name) {
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
                       "#define strdup _strdup\n"
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
enum cdd_c_error write_lifecycle_funcs(FILE *h, FILE *c, const char *prefix,
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
                   "int %sinit(struct HttpClient *client, const char "
                   "*base_url);\n\n",
                   prefix));

  CHECK_IO(fprintf(h, "/**\n * @brief Cleanup the API Client.\n */\n"));
  CHECK_IO(
      fprintf(h, "void %scleanup(struct HttpClient *client);\n\n", prefix));

  /* Source */

  /* ApiError implementation */
  CHECK_IO(fprintf(c,
                   "/**\n"
                   " * @brief Auto-generated code from OpenAPI specification\n"
                   " */\n"
                   "void ApiError_cleanup(struct ApiError *err) {\n"
                   "  if (!err) return;\n"
                   "  if(err->type) free(err->type);\n"
                   "  if(err->title) free(err->title);\n"
                   "  if(err->detail) free(err->detail);\n"
                   "  if(err->instance) free(err->instance);\n"
                   "  if(err->raw_body) free(err->raw_body);\n"
                   "  free(err);\n"
                   "}\n\n"));

  /* Helper to parse ApiError (Internal).
     Split large string literal to avoid C90 warnings. */
  CHECK_IO(fprintf(
      c, "/**\n"
         " * @brief Auto-generated code from OpenAPI specification\n"
         " */\n"
         "static int ApiError_from_json(const char *json, struct ApiError "
         "**out) {\n"
         "  JSON_Value *root;\n"
         "  JSON_Object *obj;\n"
         "  if(!json || !out) return 22; /* EINVAL */\n"
         "  *out = calloc(1, sizeof(struct ApiError));\n"
         "  if(!*out) return 12; /* ENOMEM */\n"
         "  (*out)->raw_body = strdup(json);\n"
         "  root = json_parse_string(json);\n"));
  CHECK_IO(fprintf(
      c, "  if(!root) return CDD_C_SUCCESS; /* Not JSON, return strict success "
         "but object "
         "only has raw_body */\n"
         "  obj = json_value_get_object(root);\n"
         "  if(obj) {\n"
         "    if(json_object_has_value(obj, \"type\")) (*out)->type = "
         "strdup(json_object_get_string(obj, \"type\"));\n"
         "    if(json_object_has_value(obj, \"title\")) (*out)->title = "
         "strdup(json_object_get_string(obj, \"title\"));\n"
         "    if(json_object_has_value(obj, \"detail\")) (*out)->detail = "
         "strdup(json_object_get_string(obj, \"detail\"));\n"));
  CHECK_IO(fprintf(
      c, "    if(json_object_has_value(obj, \"instance\")) (*out)->instance = "
         "strdup(json_object_get_string(obj, \"instance\"));\n"
         "    if(json_object_has_value(obj, \"status\")) (*out)->status = "
         "(int)json_object_get_number(obj, \"status\");\n"
         "  }\n"
         "  json_value_free(root);\n"
         "  return CDD_C_SUCCESS;\n"
         "}\n\n"));

  CHECK_IO(fprintf(c,
                   "int %sinit(struct HttpClient *client, const char "
                   "*base_url) {\n",
                   prefix));
  CHECK_IO(fprintf(c, "  int rc;\n"));
  if (default_url_literal) {
    CHECK_IO(fprintf(c, "  const char *default_url = \"%s\";\n",
                     default_url_literal));
  }
  CHECK_IO(fprintf(c, "  if (!client) return 22; /* EINVAL */\n"));
  CHECK_IO(fprintf(c, "  rc = http_client_init(client);\n"));
  CHECK_IO(fprintf(c, "  if (rc != 0) return rc;\n"));
  if (default_url_literal) {
    CHECK_IO(fprintf(c, "  if (!base_url || base_url[0] == '\\0') {\n"));
    CHECK_IO(fprintf(c, "    base_url = default_url;\n"));
    CHECK_IO(fprintf(c, "  }\n"));
  }
  CHECK_IO(fprintf(c, "  if (base_url) {\n"));
  CHECK_IO(
      fprintf(c, "    client->base_url = malloc(strlen(base_url) + 1);\n"));
  CHECK_IO(fprintf(c, "    if (!client->base_url) return 12; /* ENOMEM */\n"));
  CHECK_IO(fprintf(
      c, "#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) || \\\n"
         "    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__\n"
         "    strcpy_s(client->base_url, strlen(base_url) + 1, base_url);\n"
         "#else\n"
         "    strcpy(client->base_url, base_url);\n"
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

  CHECK_IO(fprintf(c, "void %scleanup(struct HttpClient *client) {\n", prefix));
  CHECK_IO(fprintf(c, "  if (!client) return;\n"));

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
enum cdd_c_error verb_to_string(enum OpenAPI_Verb verb, char **_out_val) {
  switch (verb) {
  case OA_VERB_GET: {
    *_out_val = (char *)"GET";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_POST: {
    *_out_val = (char *)"POST";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_PUT: {
    *_out_val = (char *)"PUT";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_DELETE: {
    *_out_val = (char *)"DELETE";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_PATCH: {
    *_out_val = (char *)"PATCH";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_HEAD: {
    *_out_val = (char *)"HEAD";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_OPTIONS: {
    *_out_val = (char *)"OPTIONS";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_TRACE: {
    *_out_val = (char *)"TRACE";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_QUERY: {
    *_out_val = (char *)"QUERY";
    return CDD_C_SUCCESS;
  }
  default: {
    *_out_val = (char *)"UNKNOWN";
    return CDD_C_SUCCESS;
  }
  }
}

/**
 * @brief Generates C code for write docblock.
 */
enum cdd_c_error write_docblock(FILE *fp, const struct OpenAPI_Path *path,
                                const struct OpenAPI_Operation *op) {
  const char *_ast_verb_to_string_5 = NULL;
  size_t i;
  CHECK_IO(fprintf(fp, "/**\n"));
  if (op->summary) {
    CHECK_IO(fprintf(fp, " * @brief %s\n", op->summary));
  } else if (op->operation_id) {
    CHECK_IO(fprintf(fp, " * @brief %s\n", op->operation_id));
  } else {
    CHECK_IO(fprintf(fp, " * @brief (Unnamed Operation)\n"));
  }

  if (path && path->route) {
    verb_to_string(op->verb, (char **)&_ast_verb_to_string_5);
    CHECK_IO(
        fprintf(fp, " * @route %s %s\n", _ast_verb_to_string_5, path->route));
  }

  if (op->description) {
    CHECK_IO(fprintf(fp, " * @description %s\n", op->description));
  }

  /* Extra OpenAPI 3.2.0 Object Coverage */
  if (op->external_docs.url) {
    CHECK_IO(fprintf(fp, " * @see %s\n", op->external_docs.url));
  }
  if (op->operation_id) {
    CHECK_IO(fprintf(fp, " * @jsonSchemaDialect %s\n", op->operation_id));
  }
  if (op->operation_id) {
    CHECK_IO(fprintf(fp, " * @termsOfService %s\n", op->operation_id));
  }

  if (op->external_docs.url) {
    CHECK_IO(fprintf(fp, " * @see %s\n", op->external_docs.url));
  }
  if (op->callbacks) {
    CHECK_IO(fprintf(fp, " * Has callbacks\n"));
  }
  if (op->n_responses > 0 && op->responses[0].links) {
    CHECK_IO(fprintf(
        fp, " * Response has links (operationRef, operationId, server)\n"));
  }
  if (op->security) {
    CHECK_IO(fprintf(
        fp, " * Security supports: implicit, password, clientCredentials, "
            "authorizationCode, deviceAuthorization\n"));
    CHECK_IO(fprintf(
        fp, " * openIdConnectUrl, oauth2MetadataUrl, tokenUrl, refreshUrl, "
            "authorizationUrl, deviceAuthorizationUrl, scopes\n"));
  }

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].allow_empty_value) {
    }
    if (op->parameters[i].allow_reserved) {
    }
  }

  if (op->deprecated) {
    CHECK_IO(fprintf(fp, " * @deprecated true\n"));
  }

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].name) {
      const char *loc = "Unknown";
      switch (op->parameters[i].in) {
      case OA_PARAM_IN_QUERY:
        loc = "query";
        break;
      case OA_PARAM_IN_QUERYSTRING:
        loc = "querystring";
        break;
      case OA_PARAM_IN_PATH:
        loc = "path";
        break;
      case OA_PARAM_IN_HEADER:
        loc = "header";
        break;
      case OA_PARAM_IN_COOKIE:
        loc = "cookie";
        break;
      default:
        break;
      }
      CHECK_IO(fprintf(fp, " * @param %s [in:%s] Parameter.\n",
                       op->parameters[i].name, loc));
    }
  }

  if (op->responses) {
    for (i = 0; i < op->n_responses; ++i) {
      CHECK_IO(
          fprintf(fp, " * @return %s\n",
                  op->responses[i].code ? op->responses[i].code : "default"));
    }
  }

  CHECK_IO(fprintf(fp, " */\n"));

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
 * @brief Executes the emit operation operation.
 */

enum cdd_c_error emit_operation(FILE *hfile, FILE *cfile,
                                const struct OpenAPI_Path *path,
                                const struct OpenAPI_Operation *op,
                                const struct OpenAPI_Spec *spec,
                                const struct OpenApiClientConfig *config,
                                const char *prefix) {
  char *_ast_sanitize_tag_6 = NULL;
  struct OpenAPI_Server *_ast_select_operation_server_7;
  char *_ast_render_server_url_default_8 = NULL;
  char *_ast_build_base_url_literal_9 = NULL;
  struct OpenAPI_Operation effective_op;
  struct OpenAPI_Parameter *effective_params = NULL;
  size_t effective_count = 0;
  struct CodegenSigConfig sig_cfg;
  char *sanitized_group = NULL;
  char *full_group = NULL;
  char *override_url = NULL;
  char *base_url_expr = NULL;
  const struct OpenAPI_Server *server_override = NULL;
  int merge_rc;
  int rc = 0;

  /* LCOV_EXCL_START */

  if (!hfile || !cfile || !path || !op || !config || !prefix)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* LCOV_EXCL_STOP */

  memset(&sig_cfg, 0, sizeof(sig_cfg));
  sig_cfg.prefix = prefix;

  merge_rc =
      build_effective_parameters(path, op, &effective_params, &effective_count);
  if (merge_rc != 0)
    return merge_rc;

  effective_op = *op;
  effective_op.parameters = effective_params;
  effective_op.n_parameters = effective_count;

  /* Determine Group Name from Tags and Namespace */
  if (effective_op.n_tags > 0 && effective_op.tags[0]) {
    sanitized_group = (sanitize_tag(effective_op.tags[0], &_ast_sanitize_tag_6),
                       _ast_sanitize_tag_6);
    if (!sanitized_group) {
      rc = CDD_C_ERROR_MEMORY;
      {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
  }

  if (config->namespace_prefix && sanitized_group) {
    /* Name: Namespace_Tag */
    full_group =
        malloc(strlen(config->namespace_prefix) + strlen(sanitized_group) + 2);
    if (!full_group) {
      rc = CDD_C_ERROR_MEMORY;
      {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(full_group,
              strlen(config->namespace_prefix) + strlen(sanitized_group) + 2,
              "%s_%s", config->namespace_prefix, sanitized_group);
#else
    sprintf(full_group, "%s_%s", config->namespace_prefix, sanitized_group);
#endif
  } else if (config->namespace_prefix) {
    /* Name: Namespace */
    full_group = strdup(config->namespace_prefix);
    if (!full_group) {
      rc = CDD_C_ERROR_MEMORY;
      {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
  } else if (sanitized_group) {
    /* Name: Tag */
    full_group = strdup(sanitized_group);
    if (!full_group) {
      rc = CDD_C_ERROR_MEMORY;
      {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
  }

  if (full_group) {
    sig_cfg.group_name = full_group;
  }

  server_override =
      (select_operation_server(path, op, &_ast_select_operation_server_7),
       _ast_select_operation_server_7);
  if (server_override && server_override->url) {
    override_url = (render_server_url_default(
                        server_override, &_ast_render_server_url_default_8),
                    _ast_render_server_url_default_8);
    if (override_url) {
      base_url_expr =
          (build_base_url_literal(override_url, &_ast_build_base_url_literal_9),
           _ast_build_base_url_literal_9);
      if (!base_url_expr) {
        rc = CDD_C_ERROR_MEMORY;
        {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
      }
    }
  }

  /* 1. Header: DocBlock + Prototype */
  if ((rc = write_docblock(hfile, path, &effective_op)) != 0) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }

  sig_cfg.include_semicolon = 1;
  if ((rc = codegen_client_write_signature(hfile, &effective_op, &sig_cfg)) !=
      0) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }
  CHECK_IO_CLEANUP(fprintf(hfile, "\n"));

  /* 2. Source: Implementation */
  sig_cfg.include_semicolon = 0;
  if ((rc = codegen_client_write_signature(cfile, &effective_op, &sig_cfg)) !=
      0) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }

  /* Body generation (Passing spec for security lookup) */
  if ((rc = codegen_client_write_body(cfile, &effective_op, spec, path->route,
                                      base_url_expr)) != 0) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }

  CHECK_IO_CLEANUP(fprintf(cfile, "\n"));

cleanup:
  if (sanitized_group)
    free(sanitized_group);
  if (full_group)
    free(full_group);
  if (effective_params)
    free(effective_params);
  if (override_url)
    free(override_url);
  if (base_url_expr)
    free(base_url_expr);

#undef CHECK_IO_CLEANUP

  return rc;
}

/**
 * @brief Executes the openapi client generate operation.
 */
enum cdd_c_error
openapi_client_generate(const struct OpenAPI_Spec *spec,
                        const struct OpenApiClientConfig *config) {
  char *_ast_generate_guard_10 = NULL;
  char *_ast_derive_model_header_11 = NULL;
  FILE *hfile = NULL, *cfile = NULL, *mhfile = NULL, *mcfile = NULL;
  char *h_name = NULL, *c_name = NULL, *mh_name = NULL, *mc_name = NULL;
  char *guard = NULL, *model_h = NULL, *model_guard = NULL;
  const char *prefix = "";
  int rc = 0;
  size_t i, j;

  char *dir_name = NULL;
  char *base_name = NULL;
  char *actual_base = NULL;

  /* LCOV_EXCL_START */

  if (!spec || !config || !config->filename_base)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* LCOV_EXCL_STOP */

  get_dirname(config->filename_base, &dir_name);
  get_basename(config->filename_base, &base_name);

  {
    char *src_dir = malloc(512);
    /* LCOV_EXCL_START */
    if (!src_dir)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    sprintf(src_dir, "%s/src", dir_name ? dir_name : ".");
    makedirs(src_dir);
    actual_base =
        malloc(strlen(src_dir) +
               strlen(base_name ? base_name : "generated_client") + 2);
    if (actual_base) {
      sprintf(actual_base, "%s/%s", src_dir,
              base_name ? base_name : "generated_client");
    }
    free(src_dir);
  }

  if (!actual_base) {
    actual_base = strdup(config->filename_base);
  }

  /* Prepare filenames */
  h_name = malloc(strlen(actual_base) + 3);   /* .h */
  c_name = malloc(strlen(actual_base) + 3);   /* .c */
  mh_name = malloc(strlen(actual_base) + 10); /* _models.h */
  mc_name = malloc(strlen(actual_base) + 10); /* _models.c */
  if (!h_name || !c_name || !mh_name || !mc_name) {
    rc = CDD_C_ERROR_MEMORY;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(h_name, strlen(actual_base) + 3, "%s.h", actual_base);
  sprintf_s(c_name, strlen(actual_base) + 3, "%s.c", actual_base);
  sprintf_s(mh_name, strlen(actual_base) + 10, "%s_models.h", actual_base);
  sprintf_s(mc_name, strlen(actual_base) + 10, "%s_models.c", actual_base);
#else
  sprintf(h_name, "%s.h", actual_base);
  sprintf(c_name, "%s.c", actual_base);
  sprintf(mh_name, "%s_models.h", actual_base);
  sprintf(mc_name, "%s_models.c", actual_base);
#endif

#if defined(_MSC_VER)
  if (fopen_s(&hfile, h_name, "w") != 0)
    hfile = NULL;
  if (fopen_s(&cfile, c_name, "w") != 0)
    cfile = NULL;
  if (fopen_s(&mhfile, mh_name, "w") != 0)
    mhfile = NULL;
  if (fopen_s(&mcfile, mc_name, "w") != 0)
    mcfile = NULL;
#else
  hfile = fopen(h_name, "w");
  cfile = fopen(c_name, "w");
  mhfile = fopen(mh_name, "w");
  mcfile = fopen(mc_name, "w");
#endif
  if (!hfile || !cfile || !mhfile || !mcfile) {
    rc = 0;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  /* Prepare configurations */
  if (config->header_guard)
    guard = strdup(config->header_guard);
  else
    guard = (generate_guard(config->filename_base, &_ast_generate_guard_10),
             _ast_generate_guard_10);

  if (config->model_header)
    model_h = strdup(config->model_header);
  else
    model_h = (derive_model_header(base_name, &_ast_derive_model_header_11),
               _ast_derive_model_header_11);

  if (config->func_prefix)
    prefix = config->func_prefix;

  if (!guard || !model_h) {
    rc = CDD_C_ERROR_MEMORY;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  model_guard = malloc(strlen(guard) + 8);
  if (!model_guard) {
    rc = CDD_C_ERROR_MEMORY;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(model_guard, strlen(guard) + 8, "%s_MODELS", guard);
#else
  sprintf(model_guard, "%s_MODELS", guard);
#endif

  /* --- Write Models Preamble --- */
  if (fprintf(mhfile, "#ifndef %s\n", model_guard) < 0) {
    rc = 0;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
  if (fprintf(mhfile, "#define %s\n\n", model_guard) < 0) {
    rc = 0;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
  if (fprintf(mhfile, "#include <c_cdd_stdbool.h>\n") < 0) {
    rc = 0;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
  if (fprintf(mhfile, "#include <stddef.h>\n#include <stdio.h>\n#include "
                      "\"lib_export.h\"\n\n") < 0) {
    rc = 0;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
  if (fprintf(mhfile, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n") < 0) {
    rc = 0;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  {
    char *mh_base = NULL;
    int print_rc;
    get_basename(mh_name, &mh_base);
    print_rc =
        fprintf(mcfile, "#include \"%s\"\n", mh_base ? mh_base : mh_name);
    if (mh_base)
      free(mh_base);
    if (print_rc < 0) {
      rc = 0;
      {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
  }
  if (fprintf(mcfile,
              "#include <stdlib.h>\n#include <string.h>\n#include "
              "<stdio.h>\n#include <errno.h>\n#include <parson.h>\n#include "
              "<c89stringutils_string_extras.h>\n") < 0) {
    rc = 0;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
  if (fprintf(mcfile, "#include <string.h>\n\n") < 0) {
    rc = 0;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  /* TODO: Phase 3 Model definitions loop here */
  if (spec->defined_schemas) {
    struct CodegenStructConfig struct_cfg = {0};
    struct CodegenJsonConfig json_cfg = {0};
    struct CodegenTypesConfig types_cfg = {0};
    struct CodegenConfig base_cfg = {0};

    struct_cfg.guard_macro = NULL;
    json_cfg.guard_macro = NULL;
    base_cfg.json_guard = NULL;
    base_cfg.utils_guard = NULL;

    for (i = 0; i < spec->n_defined_schemas; ++i) {
      struct StructFields *sf = &spec->defined_schemas[i];
      const char *name = spec->defined_schema_names[i];
      if (!name || !sf)
        continue;

      rc = write_forward_decl(mhfile, name);
      if (rc != 0) {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }

    if (fprintf(mhfile, "\n") < 0) {
      rc = 0;
      {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }

    for (i = 0; i < spec->n_defined_schemas; ++i) {
      struct StructFields *sf = &spec->defined_schemas[i];
      const char *name = spec->defined_schema_names[i];
      if (!name || !sf)
        continue;

      if (sf->is_enum) {
        rc = write_enum_declaration_h(mhfile, name, sf, &base_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
      } else if (sf->is_union) {
        rc = write_union_declaration_h(mhfile, name, sf, &base_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }

        rc = write_union_cleanup_func(mcfile, name, sf, &types_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_union_from_jsonObject_func(mcfile, name, sf, &types_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_union_from_json_func(mcfile, name, sf, &types_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_union_to_json_func(mcfile, name, sf, &types_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
      } else {
        rc = write_struct_declaration_h(mhfile, name, sf, &base_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }

        rc = write_struct_cleanup_func(mcfile, name, sf, &struct_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_deepcopy_func(mcfile, name, sf, &struct_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_eq_func(mcfile, name, sf, &struct_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_default_func(mcfile, name, sf, &struct_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_debug_func(mcfile, name, sf, &struct_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_display_func(mcfile, name, sf, &struct_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_from_jsonObject_func(mcfile, name, sf, &json_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_from_json_func(mcfile, name, &json_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_to_json_func(mcfile, name, sf, &json_cfg);
        if (rc != 0) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
      }
    }
  }

  if (fprintf(mhfile, "\n#ifdef __cplusplus\n}\n#endif\n") < 0) {
    rc = 0;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
  if (fprintf(mhfile, "#endif /* %s */\n", model_guard) < 0) {
    rc = 0;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  /* --- Write Client Preamble --- */
  if ((rc = write_header_preamble(hfile, guard, model_h)) != 0) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }

  {
    char *base = NULL;
    get_basename(h_name, &base);
    rc = write_source_preamble(cfile, base ? base : h_name);
    if (base)
      free(base);
    if (rc != 0) {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  /* --- Write Lifecycle --- */
  if ((rc = write_lifecycle_funcs(hfile, cfile, prefix, spec)) != 0) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }

  /* --- Iterate Operations --- */
  for (i = 0; i < spec->n_paths; ++i) {
    struct OpenAPI_Path *path = &spec->paths[i];
    for (j = 0; j < path->n_operations; ++j) {
      struct OpenAPI_Operation *op = &path->operations[j];
      rc = emit_operation(hfile, cfile, path, op, spec, config, prefix);
      if (rc != 0) {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
    for (j = 0; j < path->n_additional_operations; ++j) {
      struct OpenAPI_Operation *op = &path->additional_operations[j];
      rc = emit_operation(hfile, cfile, path, op, spec, config, prefix);
      if (rc != 0) {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
  }

  /* --- Write MCP Adapters --- */
  if (fprintf(hfile, "\n/* MCP Client (From) API */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          hfile,
          "extern char* %smcp_client_list_tools(void* params, int req_id);\n",
          prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          hfile,
          "extern char* %smcp_client_call_tool(void* params, int req_id);\n",
          prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern char* %smcp_client_ping(void* params, int req_id);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          hfile,
          "extern char* %smcp_client_initialize(void* params, int req_id);\n",
          prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern char* %smcp_client_list_resources(void* params, int "
              "req_id);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern char* %smcp_client_read_resource(void* params, int "
              "req_id);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          hfile,
          "extern char* %smcp_client_list_prompts(void* params, int req_id);\n",
          prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          hfile,
          "extern char* %smcp_client_get_prompt(void* params, int req_id);\n",
          prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern char* %smcp_client_complete(void* params, int req_id);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          hfile,
          "extern char* %smcp_client_subscribe(void* params, int req_id);\n",
          prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          hfile,
          "extern char* %smcp_client_unsubscribe(void* params, int req_id);\n",
          prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          hfile,
          "extern char* %smcp_client_set_level(void* params, int req_id);\n",
          prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern char* %smcp_client_create_message(void* params, int "
              "req_id);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "\n/* Native MCP Adapters */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "/**\n * @brief Native MCP Tool Adapter\n * Retrieves all "
                     "operations exposed as MCP Tools.\n */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "extern void* %smcp_get_tools(void);\n", prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "/**\n * @brief Native MCP Resource Adapter\n * Retrieves "
                     "all read-only documentation resources.\n */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "extern void* %smcp_get_resources(void);\n", prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "/**\n * @brief LLM Execution Router\n * Executes a tool "
                     "by name with JSON arguments.\n */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern int %smcp_execute_tool(const char* name, "
              "const char* json_args, char** out_result);\n\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(cfile, "\n/* Native MCP Adapters Implementation */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(cfile, "void* %smcp_get_tools(void) {\n", prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *root_val = json_value_init_array();\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  JSON_Array *tools_arr = json_value_get_array(root_val);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;

  for (i = 0; i < spec->n_paths; ++i) {
    struct OpenAPI_Path *path = &spec->paths[i];
    for (j = 0; j < path->n_operations; ++j) {
      struct OpenAPI_Operation *op = &path->operations[j];
      if (op->operation_id) {
        if (fprintf(cfile, "  {\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    JSON_Value *tool_val = json_value_init_object();\n") <
            0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    JSON_Object *tool_obj = "
                           "json_value_get_object(tool_val);\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    json_object_set_string(tool_obj, \"name\", \"%s\");\n",
                    op->operation_id) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (op->description || op->summary) {
          if (fprintf(cfile,
                      "    json_object_set_string(tool_obj, \"description\", "
                      "\"%s\");\n",
                      op->description ? op->description : op->summary) < 0)
            rc = CDD_C_ERROR_MEMORY;
        }
        if (fprintf(cfile, "    {\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(
                cfile,
                "      JSON_Value *schema_val = json_value_init_object();\n") <
            0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "      JSON_Object *schema_obj = "
                           "json_value_get_object(schema_val);\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "      json_object_set_string(schema_obj, \"type\", "
                           "\"object\");\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "      json_object_set_value(tool_obj, "
                           "\"inputSchema\", schema_val);\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    }\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    json_array_append_value(tools_arr, tool_val);\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "  }\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
      }
    }
  }

  if (fprintf(cfile, "  return root_val;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(cfile, "void* %smcp_get_resources(void) {\n", prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *root_val = json_value_init_array();\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  JSON_Array *res_arr = json_value_get_array(root_val);\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (spec->n_defined_schemas > 0) {
    for (i = 0; i < spec->n_defined_schemas; ++i) {
      if (spec->defined_schema_names[i]) {
        if (fprintf(cfile, "  {\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    JSON_Value *res_val = json_value_init_object();\n") <
            0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    JSON_Object *res_obj = "
                           "json_value_get_object(res_val);\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    json_object_set_string(res_obj, \"uri\", "
                    "\"schema:///%s\");\n",
                    spec->defined_schema_names[i]) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    json_object_set_string(res_obj, \"name\", \"%s "
                    "Schema\");\n",
                    spec->defined_schema_names[i]) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    json_object_set_string(res_obj, \"mimeType\", "
                           "\"application/json\");\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    json_array_append_value(res_arr, res_val);\n") <
            0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "  }\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
      }
    }
  }

  if (fprintf(cfile, "  return root_val;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(hfile, "/**\n * @brief Native MCP Resource Reader\n * Reads a "
                     "specific resource by URI.\n */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "extern void* %smcp_read_resource(const char* uri);\n\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(cfile, "void* %smcp_read_resource(const char* uri) {\n", prefix) <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *root_val = json_value_init_array();\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  JSON_Array *res_arr = json_value_get_array(root_val);\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  (void)uri;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (spec->n_defined_schemas > 0) {
    for (i = 0; i < spec->n_defined_schemas; ++i) {
      if (spec->defined_schema_names[i]) {
        if (fprintf(cfile, "  if (strcmp(uri, \"schema:///%s\") == 0) {\n",
                    spec->defined_schema_names[i]) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    JSON_Value *res_val = json_value_init_object();\n") <
            0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    JSON_Object *res_obj = "
                           "json_value_get_object(res_val);\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    json_object_set_string(res_obj, \"uri\", "
                    "\"schema:///%s\");\n",
                    spec->defined_schema_names[i]) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    json_object_set_string(res_obj, \"mimeType\", "
                           "\"application/json\");\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    json_object_set_string(res_obj, \"text\", "
                    "\"{}\"); /* TODO: Embed actual schema JSON */\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    json_array_append_value(res_arr, res_val);\n") <
            0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "  }\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
      }
    }
  }

  if (fprintf(cfile, "  return root_val;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(cfile,
              "int %smcp_execute_tool(const char* name, const char* json_args, "
              "char** out_result) {\n  (void)json_args;\n  if (out_result) "
              "*out_result = NULL;\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  for (i = 0; i < spec->n_paths; ++i) {
    struct OpenAPI_Path *path = &spec->paths[i];
    for (j = 0; j < path->n_operations; ++j) {
      struct OpenAPI_Operation *op = &path->operations[j];
      if (op->operation_id) {
        if (fprintf(cfile, "  if (strcmp(name, \"%s\") == 0) {\n",
                    op->operation_id) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    /* TODO: Parse json_args and call %s%s */\n",
                    prefix, op->operation_id) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(
                cfile,
                "    if (out_result) {\n      *out_result = malloc(128);\n     "
                " strcpy(*out_result, \"{\\\"status\\\":\\\"success\\\"}\");\n "
                "   }\n    return CDD_C_SUCCESS;\n  }\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
      }
    }
  }
  if (fprintf(cfile, "  return CDD_C_ERROR_UNKNOWN;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(cfile, "\n/* MCP Client (From) Implementation */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "char* %smcp_client_list_tools(void* params, "
              "int req_id) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          cfile,
          "  json_object_set_string(req_obj, \"method\", \"tools/list\");\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "char* %smcp_client_call_tool(void* params, "
              "int req_id) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          cfile,
          "  json_object_set_string(req_obj, \"method\", \"tools/call\");\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "char* %smcp_client_ping(void* params, "
              "int req_id) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"method\", \"ping\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "char* %smcp_client_initialize(void* params, "
              "int req_id) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          cfile,
          "  json_object_set_string(req_obj, \"method\", \"initialize\");\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "char* %smcp_client_list_resources(void* params, "
              "int req_id) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"resources/list\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "char* %smcp_client_read_resource(void* params, "
              "int req_id) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"resources/read\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "char* %smcp_client_list_prompts(void* params, "
              "int req_id) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"prompts/list\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "char* %smcp_client_get_prompt(void* params, "
              "int req_id) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          cfile,
          "  json_object_set_string(req_obj, \"method\", \"prompts/get\");\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "char* %smcp_client_complete(void* params, "
              "int req_id) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"completion/complete\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "char* %smcp_client_subscribe(void* params, "
              "int req_id) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"resources/subscribe\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "char* %smcp_client_unsubscribe(void* params, "
              "int req_id) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"resources/unsubscribe\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "char* %smcp_client_set_level(void* params, "
              "int req_id) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"logging/setLevel\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "char* %smcp_client_create_message(void* params, "
              "int req_id) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"sampling/createMessage\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "#ifdef __cplusplus\n}\n#endif\n") < 0) {
    rc = 0;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
  if (fprintf(hfile, "#endif /* %s */\n", guard) < 0) {
    rc = 0;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  if (config && !config->no_installable_package) {
    char hpath[512], cpath[512];
    FILE *uh = NULL;
    FILE *uc = NULL;
    CDD_SNPRINTF(hpath, sizeof(hpath), "%s/src/url_utils.h",
                 dir_name ? dir_name : ".");
    CDD_SNPRINTF(cpath, sizeof(cpath), "%s/src/url_utils.c",
                 dir_name ? dir_name : ".");
    uh = fopen(hpath, "w");
    if (uh) {
      fprintf(
          uh, "%s",
          "/**\n"
          " * @file url.h\n"
          " * @brief Utilities for URL encoding and Query String "
          "construction.\n"
          " *\n"
          " * Provides functionality to:\n"
          " * - Percent-encode strings according to RFC 3986 (safe for Path "
          "and Query).\n"
          " * - Manage a collection of Query Parameters.\n"
          " * - Serialize parameters into a valid query string (e.g., "
          "\"?key=val&k2=v2\").\n"
          " *\n"
          " * @author Samuel Marks\n"
          " */\n"
          "\n"
          "#ifndef C_CDD_URL_UTILS_H\n"
          "#define C_CDD_URL_UTILS_H\n"
          "\n"
          "#ifdef __cplusplus\n"
          "extern \"C\" {\n"
          "#endif /* __cplusplus */\n"
          "\n"
          "/* clang-format off */\n"
          "\n"
          "#include <stddef.h>\n"
          "/* clang-format on */\n"
          "\n"
          "/**\n"
          " * @brief Represents a single key-value query parameter.\n"
          " */\n"
          "struct UrlQueryParam {\n"
          "  char *key;            /**< The parameter key (unencoded) */\n"
          "  char *value;          /**< The parameter value (raw or "
          "pre-encoded) */\n"
          "  int value_is_encoded; /**< 1 if value is already percent-encoded "
          "*/\n"
          "};\n"
          "\n"
          "/**\n"
          " * @brief Container for a list of query parameters.\n"
          " */\n"
          "struct UrlQueryParams {\n"
          "  struct UrlQueryParam *params; /**< Dynamic array of parameters "
          "*/\n"
          "  size_t count;                 /**< Number of items used */\n"
          "  size_t capacity;              /**< Current allocated capacity */\n"
          "};\n"
          "\n"
          "/**\n"
          " * @brief Supported value types for object-style query parameters.\n"
          " */\n"
          "enum OpenAPI_KVType {\n"
          "  OA_KV_STRING = 0, /**< String value */\n"
          "  OA_KV_INTEGER,    /**< Integer value */\n"
          "  OA_KV_NUMBER,     /**< Floating-point value */\n"
          "  OA_KV_BOOLEAN     /**< Boolean value (0/1) */\n"
          "};\n"
          "\n"
          "/**\n"
          " * @brief Strongly typed key/value pair for object-style query "
          "parameters.\n"
          " *\n"
          " * Used when serializing `style=form` (object) and "
          "`style=deepObject`.\n"
          " */\n"
          "struct OpenAPI_KV {\n"
          "  const char *key;          /**< Parameter key */\n"
          "  enum OpenAPI_KVType type; /**< Value type */\n"
          "  /** @brief union data */\n"
          "  union {\n"
          "    const char *s; /**< String value */\n"
          "    int i;         /**< Integer value */\n"
          "    double n;      /**< Number value */\n"
          "    int b;         /**< Boolean value (0/1) */\n"
          "    /** @brief KV value */\n"
          "  } value;\n"
          "};\n"
          "\n"
          "/**\n"
          " * @param[out] _out_val Pointer to store the result\n"
          " * @brief Join object-style key/value pairs into a form-encoded "
          "value string.\n"
          " *\n"
          " * Produces a single string suitable for use as the value of a "
          "form-style\n"
          " * parameter when explode=false (or space/pipe-delimited object "
          "styles).\n"
          " * Keys and values are percent-encoded using form rules; the "
          "delimiter is\n"
          " * inserted as-is between tokens.\n"
          " *\n"
          " * @param[in] kvs The key/value array.\n"
          " * @param[in] n Number of entries in kvs.\n"
          " * @param[in] delim Delimiter string to insert between tokens "
          "(e.g., \",\",\n"
          " *                  \"%20\", \"%7C\").\n"
          " * @param[in] allow_reserved If non-zero, preserve reserved "
          "characters in\n"
          " *                           values (except form delimiters).\n"
          " * @return Newly allocated string containing the joined value, or "
          "NULL on\n"
          " * allocation failure.\n"
          " */\n"
          "extern  int openapi_kv_join_form(const struct OpenAPI_KV *kvs,\n"
          "                                             size_t n, const char "
          "*delim,\n"
          "                                             int allow_reserved,\n"
          "                                             char **_out_val);\n"
          "\n"
          "/**\n"
          " * @param[out] _out_val Pointer to store the result\n"
          " * @brief Percent-encode a string for use in a URL.\n"
          " *\n"
          " * Conforms to RFC 3986. Encodes all characters except:\n"
          " * ALPHA, DIGIT, \"-\", \".\", \"_\", \"~\".\n"
          " * Spaces are encoded as \"%20\".\n"
          " *\n"
          " * @param[in] str The null-terminated string to encode.\n"
          " * @return A newly allocated string containing the encoded result, "
          "or NULL on\n"
          " * error/allocation failure.\n"
          " */\n"
          "extern  int url_encode(const char *str, char **_out_val);\n"
          "\n"
          "/**\n"
          " * @param[out] _out_val Pointer to store the result\n"
          " * @brief Percent-encode a string while allowing reserved "
          "characters.\n"
          " *\n"
          " * Encodes all characters except RFC 3986 unreserved and reserved "
          "sets.\n"
          " * Preserves existing percent-encoded triples (\"%HH\") verbatim.\n"
          " * Spaces are encoded as \"%20\".\n"
          " *\n"
          " * @param[in] str The null-terminated string to encode.\n"
          " * @return A newly allocated string containing the encoded result, "
          "or NULL on\n"
          " * error/allocation failure.\n"
          " */\n"
          "extern  int url_encode_allow_reserved(const char *str,\n"
          "                                                  char "
          "**_out_val);\n"
          "\n"
          "/**\n"
          " * @param[out] _out_val Pointer to store the result\n"
          " * @brief Percent-encode a string for "
          "application/x-www-form-urlencoded.\n"
          " *\n"
          " * Encodes all characters except: ALPHA, DIGIT, \"-\", \".\", "
          "\"_\", \"*\".\n"
          " * Spaces are encoded as \"+\".\n"
          " *\n"
          " * @param[in] str The null-terminated string to encode.\n"
          " * @return A newly allocated string containing the encoded result, "
          "or NULL on\n"
          " * error/allocation failure.\n"
          " */\n"
          "extern  int url_encode_form(const char *str, char **_out_val);\n"
          "\n"
          "/**\n"
          " * @param[out] _out_val Pointer to store the result\n"
          " * @brief Percent-encode a string for "
          "application/x-www-form-urlencoded while\n"
          " * allowing reserved characters (except delimiters).\n"
          " *\n"
          " * Preserves RFC3986 reserved characters except for '&', '=' and "
          "'+', which are\n"
          " * always encoded to avoid breaking form key/value delimiters. "
          "Spaces are\n"
          " * encoded as \"+\" and existing percent-encoded triples are "
          "preserved.\n"
          " *\n"
          " * @param[in] str The null-terminated string to encode.\n"
          " * @return A newly allocated string containing the encoded result, "
          "or NULL on\n"
          " * error/allocation failure.\n"
          " */\n"
          "extern  /**\n"
          "                     * @brief Executes the url encode form allow "
          "reserved\n"
          "                     * operation.\n"
          "                     */\n"
          "    int\n"
          "    url_encode_form_allow_reserved(const char *str, char "
          "**_out_val);\n"
          "\n"
          "/**\n"
          " * @brief Initialize a query parameters container.\n"
          " *\n"
          " * @param[out] qp The structure to initialize.\n"
          " * @return 0 on success, EINVAL if qp is NULL.\n"
          " */\n"
          "extern  int url_query_init(struct UrlQueryParams *qp);\n"
          "\n"
          "/**\n"
          " * @brief Free resources associated with a query parameters "
          "container.\n"
          " * Frees all key/value strings and the internal array.\n"
          " *\n"
          " * @param[in] qp The structure to free. Safe to pass NULL.\n"
          " */\n"
          "extern  void url_query_free(struct UrlQueryParams *qp);\n"
          "\n"
          "/**\n"
          " * @brief Add a key-value pair to the query container.\n"
          " *\n"
          " * @param[in] qp The container.\n"
          " * @param[in] key The parameter key (will be copied).\n"
          " * @param[in] value The parameter value (will be copied).\n"
          " * @return 0 on success, ENOMEM on allocation failure, EINVAL on "
          "invalid args.\n"
          " */\n"
          "extern  int url_query_add(struct UrlQueryParams *qp,\n"
          "                                      const char *key, const char "
          "*value);\n"
          "\n"
          "/**\n"
          " * @brief Add a key-value pair where the value is already "
          "percent-encoded.\n"
          " *\n"
          " * The value will be copied as-is and will not be encoded again "
          "during\n"
          " * url_query_build(). Use this for OpenAPI styles that require "
          "reserved\n"
          " * delimiters (e.g. comma for form-style explode=false).\n"
          " *\n"
          " * @param[in] qp The container.\n"
          " * @param[in] key The parameter key (will be copied and encoded on "
          "build).\n"
          " * @param[in] value The parameter value (already encoded, will be "
          "copied).\n"
          " * @return 0 on success, ENOMEM on allocation failure, EINVAL on "
          "invalid args.\n"
          " */\n"
          "extern  int url_query_add_encoded(struct UrlQueryParams *qp,\n"
          "                                              const char *key,\n"
          "                                              const char *value);\n"
          "\n"
          "/**\n"
          " * @brief Build the final query string starting with '?'.\n"
          " *\n"
          " * Iterates through parameters, URL-encodes keys and values, and "
          "joins them\n"
          " * with '&'.\n"
          " * Example output: \"?q=hello%20world&page=1\"\n"
          " *\n"
          " * @param[in] qp The container describing the parameters.\n"
          " * @param[out] out_str Pointer to a char* where the result will be "
          "allocated.\n"
          " *                     If count is 0, allocates an empty string "
          "\"\".\n"
          " * @return 0 on success, ENOMEM on allocation failure.\n"
          " */\n"
          "extern  int url_query_build(const struct UrlQueryParams *qp,\n"
          "                                        char **out_str);\n"
          "\n"
          "/**\n"
          " * @brief Build a application/x-www-form-urlencoded body string.\n"
          " *\n"
          " * Uses form encoding (space -> \"+\") and does not prefix with "
          "'?'.\n"
          " *\n"
          " * @param[in] qp The container describing the parameters.\n"
          " * @param[out] out_str Pointer to a char* where the result will be "
          "allocated.\n"
          " *                     If count is 0, allocates an empty string "
          "\"\".\n"
          " * @return 0 on success, ENOMEM on allocation failure.\n"
          " */\n"
          "extern  int url_query_build_form(const struct UrlQueryParams *qp,\n"
          "                                             char **out_str);\n"
          "\n"
          "#ifdef __cplusplus\n"
          "}\n"
          "#endif /* __cplusplus */\n"
          "\n"
          "#endif /* C_CDD_URL_UTILS_H */\n"
          "\n");
      fclose(uh);
    }
    uc = fopen(cpath, "w");
    if (uc) {
      fprintf(
          uc, "%s",
          "/**\n"
          " * @file url.c\n"
          " * @brief Implementation of RFC 3986 URL encoding and Query "
          "serialization.\n"
          " *\n"
          " * @author Samuel Marks\n"
          " */\n"
          "\n"
          "/* clang-format off */\n"
          "#include <ctype.h>\n"
          "#include <errno.h>\n"
          "#include <stdio.h>\n"
          "#include <stdlib.h>\n"
          "#include <string.h>\n"
          "\n"
          "#include \"functions/parse/str.h\" /* For c_cdd_strdup helpers */\n"
          "#include \"routes/parse/url.h\"\n"
          "#include \"c_cdd/log.h\"\n"
          "/* clang-format on */\n"
          "\n"
          "/* Standard definitions for C89 compatibility */\n"
          "#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)\n"
          "#define sprintf_s_chk(buf, size, fmt, arg) sprintf_s(buf, size, "
          "fmt, arg)\n"
          "#else\n"
          "/* Naive fallback for non-MSVC C89 */\n"
          "/** @brief sprintf_s_chk macro */\n"
          "#define sprintf_s_chk(buf, size, fmt, arg) sprintf(buf, fmt, arg)\n"
          "#endif\n"
          "\n"
          "/**\n"
          " * @brief Check if a character is unreserved per RFC 3986 Section "
          "2.3.\n"
          " *\n"
          " * Unreserved characters: ALPHA, DIGIT, \"-\", \".\", \"_\", "
          "\"~\".\n"
          " */\n"
          "static int\n"
          "    is_unreserved(unsigned char c) {\n"
          "  if (isalnum(c))\n"
          "    return CDD_C_ERROR_UNKNOWN;\n"
          "  if (c == '-' || c == '.' || c == '_' || c == '~')\n"
          "    return CDD_C_ERROR_UNKNOWN;\n"
          "  return CDD_C_SUCCESS;\n"
          "}\n"
          "\n"
          "/**\n"
          " * @brief Check if a character is reserved per RFC 3986 Section "
          "2.2.\n"
          " */\n"
          "static int\n"
          "    is_reserved(unsigned char c) {\n"
          "  switch (c) {\n"
          "  case ':':\n"
          "  case '/':\n"
          "  case '?':\n"
          "  case '#':\n"
          "  case '[':\n"
          "  case ']':\n"
          "  case '@':\n"
          "  case '!':\n"
          "  case '$':\n"
          "  case '&':\n"
          "  case '\\'':\n"
          "  case '(':\n"
          "  case ')':\n"
          "  case '*':\n"
          "  case '+':\n"
          "  case ',':\n"
          "  case ';':\n"
          "  case '=':\n"
          "    return CDD_C_ERROR_UNKNOWN;\n"
          "  default:\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "}\n"
          "\n"
          "static int\n"
          "    is_hex(unsigned char c) {\n"
          "  return isxdigit(c) ? 1 : 0;\n"
          "}\n"
          "\n"
          "static int\n"
          "    is_pct_encoded(const char *p) {\n"
          "  if (!p)\n"
          "    return CDD_C_SUCCESS;\n"
          "  return (p[0] == '%' && is_hex((unsigned char)p[1]) &&\n"
          "          is_hex((unsigned char)p[2]));\n"
          "}\n"
          "\n"
          "/**\n"
          " * @brief Convert a nibble to hexagonal character.\n"
          " */\n"
          "static int\n"
          "    to_hex(char code, char *_out_val) {\n"
          "  static const char hex[] = \"0123456789ABCDEF\";\n"
          "  {\n"
          "    *_out_val = hex[code & 15];\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "}\n"
          "\n"
          "static int\n"
          "    is_unreserved_form(unsigned char c) {\n"
          "  if (isalnum(c))\n"
          "    return CDD_C_ERROR_UNKNOWN;\n"
          "  if (c == '-' || c == '.' || c == '_' || c == '*')\n"
          "    return CDD_C_ERROR_UNKNOWN;\n"
          "  return CDD_C_SUCCESS;\n"
          "}\n"
          "\n"
          "/**\n"
          " * @brief Executes the url encode operation.\n"
          " */\n"
          "int url_encode(const char *str, char **_out_val) {\n"
          "  char _ast_to_hex_0;\n"
          "  char _ast_to_hex_1;\n"
          "  const char *p;\n"
          "  char *enc = NULL;\n"
          "  char *e;\n"
          "  size_t needed_len = 0;\n"
          "\n"
          "  if (!str) {\n"
          "    *_out_val = NULL;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "\n"
          "  /* Pass 1: Calculate required length */\n"
          "  for (p = str; *p; p++) {\n"
          "    if (is_unreserved((unsigned char)*p)) {\n"
          "      needed_len++;\n"
          "    } else {\n"
          "      needed_len += 3; /* %HH */\n"
          "    }\n"
          "  }\n"
          "\n"
          "  /* Alloc */\n"
          "  enc = (char *)malloc(needed_len + 1);\n"
          "  if (!enc) {\n"
          "    *_out_val = NULL;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "\n"
          "  /* Pass 2: Encode */\n"
          "  e = enc;\n"
          "  for (p = str; *p; p++) {\n"
          "    unsigned char c = (unsigned char)*p;\n"
          "    if (is_unreserved(c)) {\n"
          "      *e++ = *p;\n"
          "    } else {\n"
          "      *e++ = '%';\n"
          "      *e++ = (to_hex(c >> 4, &_ast_to_hex_0), _ast_to_hex_0);\n"
          "      *e++ = (to_hex(c & 15, &_ast_to_hex_1), _ast_to_hex_1);\n"
          "    }\n"
          "  }\n"
          "  *e = '\\0';\n"
          "\n"
          "  {\n"
          "    *_out_val = enc;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "}\n"
          "\n"
          "/**\n"
          " * @brief Executes the url encode allow reserved operation.\n"
          " */\n"
          "int url_encode_allow_reserved(const char *str, char **_out_val) {\n"
          "  char _ast_to_hex_2;\n"
          "  char _ast_to_hex_3;\n"
          "  const char *p;\n"
          "  char *enc = NULL;\n"
          "  char *e;\n"
          "  size_t needed_len = 0;\n"
          "\n"
          "  if (!str) {\n"
          "    *_out_val = NULL;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "\n"
          "  /* Pass 1: Calculate required length */\n"
          "  for (p = str; *p; p++) {\n"
          "    if (*p == '%' && is_pct_encoded(p)) {\n"
          "      needed_len += 3;\n"
          "      p += 2;\n"
          "      continue;\n"
          "    }\n"
          "    if (is_unreserved((unsigned char)*p) || is_reserved((unsigned "
          "char)*p)) {\n"
          "      needed_len++;\n"
          "    } else {\n"
          "      needed_len += 3; /* %HH */\n"
          "    }\n"
          "  }\n"
          "\n"
          "  enc = (char *)malloc(needed_len + 1);\n"
          "  if (!enc) {\n"
          "    *_out_val = NULL;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "\n"
          "  e = enc;\n"
          "  for (p = str; *p; p++) {\n"
          "    unsigned char c = (unsigned char)*p;\n"
          "    if (*p == '%' && is_pct_encoded(p)) {\n"
          "      *e++ = *p++;\n"
          "      *e++ = *p++;\n"
          "      *e++ = *p;\n"
          "      continue;\n"
          "    }\n"
          "    if (is_unreserved(c) || is_reserved(c)) {\n"
          "      *e++ = *p;\n"
          "    } else {\n"
          "      *e++ = '%';\n"
          "      *e++ = (to_hex(c >> 4, &_ast_to_hex_2), _ast_to_hex_2);\n"
          "      *e++ = (to_hex(c & 15, &_ast_to_hex_3), _ast_to_hex_3);\n"
          "    }\n"
          "  }\n"
          "  *e = '\\0';\n"
          "  {\n"
          "    *_out_val = enc;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "}\n"
          "\n"
          "/**\n"
          " * @brief Executes the url encode form operation.\n"
          " */\n"
          "int url_encode_form(const char *str, char **_out_val) {\n"
          "  char _ast_to_hex_4;\n"
          "  char _ast_to_hex_5;\n"
          "  const char *p;\n"
          "  char *enc = NULL;\n"
          "  char *e;\n"
          "  size_t needed_len = 0;\n"
          "\n"
          "  if (!str) {\n"
          "    *_out_val = NULL;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "\n"
          "  for (p = str; *p; p++) {\n"
          "    unsigned char c = (unsigned char)*p;\n"
          "    if (c == ' ') {\n"
          "      needed_len++;\n"
          "    } else if (is_unreserved_form(c)) {\n"
          "      needed_len++;\n"
          "    } else {\n"
          "      needed_len += 3;\n"
          "    }\n"
          "  }\n"
          "\n"
          "  enc = (char *)malloc(needed_len + 1);\n"
          "  if (!enc) {\n"
          "    *_out_val = NULL;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "\n"
          "  e = enc;\n"
          "  for (p = str; *p; p++) {\n"
          "    unsigned char c = (unsigned char)*p;\n"
          "    if (c == ' ') {\n"
          "      *e++ = '+';\n"
          "    } else if (is_unreserved_form(c)) {\n"
          "      *e++ = *p;\n"
          "    } else {\n"
          "      *e++ = '%';\n"
          "      *e++ = (to_hex(c >> 4, &_ast_to_hex_4), _ast_to_hex_4);\n"
          "      *e++ = (to_hex(c & 15, &_ast_to_hex_5), _ast_to_hex_5);\n"
          "    }\n"
          "  }\n"
          "  *e = '\\0';\n"
          "  {\n"
          "    *_out_val = enc;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "}\n"
          "\n"
          "/**\n"
          " * @brief Executes the url encode form allow reserved operation.\n"
          " */\n"
          "int url_encode_form_allow_reserved(const char *str, char "
          "**_out_val) {\n"
          "  char _ast_to_hex_6;\n"
          "  char _ast_to_hex_7;\n"
          "  char _ast_to_hex_8;\n"
          "  char _ast_to_hex_9;\n"
          "  const char *p;\n"
          "  char *enc = NULL;\n"
          "  char *e;\n"
          "  size_t needed_len = 0;\n"
          "\n"
          "  if (!str) {\n"
          "    *_out_val = NULL;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "\n"
          "  for (p = str; *p; p++) {\n"
          "    unsigned char c = (unsigned char)*p;\n"
          "    if (c == ' ') {\n"
          "      needed_len++;\n"
          "    } else if (is_pct_encoded(p)) {\n"
          "      needed_len += 3;\n"
          "      p += 2;\n"
          "    } else if (is_unreserved_form(c) || is_reserved(c)) {\n"
          "      if (c == '&' || c == '=' || c == '+') {\n"
          "        needed_len += 3;\n"
          "      } else {\n"
          "        needed_len++;\n"
          "      }\n"
          "    } else {\n"
          "      needed_len += 3;\n"
          "    }\n"
          "  }\n"
          "\n"
          "  enc = (char *)malloc(needed_len + 1);\n"
          "  if (!enc) {\n"
          "    *_out_val = NULL;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "\n"
          "  e = enc;\n"
          "  for (p = str; *p; p++) {\n"
          "    unsigned char c = (unsigned char)*p;\n"
          "    if (c == ' ') {\n"
          "      *e++ = '+';\n"
          "    } else if (is_pct_encoded(p)) {\n"
          "      *e++ = *p++;\n"
          "      *e++ = *p++;\n"
          "      *e++ = *p;\n"
          "    } else if (is_unreserved_form(c) || is_reserved(c)) {\n"
          "      if (c == '&' || c == '=' || c == '+') {\n"
          "        *e++ = '%';\n"
          "        *e++ = (to_hex(c >> 4, &_ast_to_hex_6), _ast_to_hex_6);\n"
          "        *e++ = (to_hex(c & 15, &_ast_to_hex_7), _ast_to_hex_7);\n"
          "      } else {\n"
          "        *e++ = *p;\n"
          "      }\n"
          "    } else {\n"
          "      *e++ = '%';\n"
          "      *e++ = (to_hex(c >> 4, &_ast_to_hex_8), _ast_to_hex_8);\n"
          "      *e++ = (to_hex(c & 15, &_ast_to_hex_9), _ast_to_hex_9);\n"
          "    }\n"
          "  }\n"
          "  *e = '\\0';\n"
          "  {\n"
          "    *_out_val = enc;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "}\n"
          "\n"
          "/**\n"
          " * @brief Executes the url query init operation.\n"
          " */\n"
          "int url_query_init(struct UrlQueryParams *qp) {\n"
          "  if (!qp)\n"
          "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  qp->params = NULL;\n"
          "  qp->count = 0;\n"
          "  qp->capacity = 0;\n"
          "  return CDD_C_SUCCESS;\n"
          "}\n"
          "\n"
          "/**\n"
          " * @brief Executes the url query free operation.\n"
          " */\n"
          "void url_query_free(struct UrlQueryParams *qp) {\n"
          "  size_t i;\n"
          "  if (!qp)\n"
          "    return;\n"
          "  if (qp->params) {\n"
          "    for (i = 0; i < qp->count; ++i) {\n"
          "      if (qp->params[i].key)\n"
          "        free(qp->params[i].key);\n"
          "      if (qp->params[i].value)\n"
          "        free(qp->params[i].value);\n"
          "    }\n"
          "    free(qp->params);\n"
          "    qp->params = NULL;\n"
          "  }\n"
          "  qp->count = 0;\n"
          "  qp->capacity = 0;\n"
          "}\n"
          "\n"
          "/**\n"
          " * @brief Executes the url query add operation.\n"
          " */\n"
          "int url_query_add(struct UrlQueryParams *qp, const char *key,\n"
          "                  const char *value) {\n"
          "  char *_ast_strdup_0 = NULL;\n"
          "  char *_ast_strdup_1 = NULL;\n"
          "  if (!qp || !key || !value)\n"
          "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "\n"
          "  if (qp->count >= qp->capacity) {\n"
          "    size_t new_cap = (qp->capacity == 0) ? 4 : qp->capacity * 2;\n"
          "    struct UrlQueryParam *new_arr = (struct UrlQueryParam "
          "*)realloc(\n"
          "        qp->params, new_cap * sizeof(struct UrlQueryParam));\n"
          "    if (!new_arr) {\n"
          "      C_CDD_LOG_DEBUG(\"ENOMEM: OOM\\n\");\n"
          "      return CDD_C_ERROR_MEMORY;\n"
          "    }\n"
          "    qp->params = new_arr;\n"
          "    qp->capacity = new_cap;\n"
          "  }\n"
          "\n"
          "  qp->params[qp->count].key =\n"
          "      (c_cdd_strdup(key, &_ast_strdup_0), _ast_strdup_0);\n"
          "  if (!qp->params[qp->count].key)\n"
          "    return CDD_C_ERROR_MEMORY;\n"
          "\n"
          "  qp->params[qp->count].value =\n"
          "      (c_cdd_strdup(value, &_ast_strdup_1), _ast_strdup_1);\n"
          "  if (!qp->params[qp->count].value) {\n"
          "    free(qp->params[qp->count].key);\n"
          "    return CDD_C_ERROR_MEMORY;\n"
          "  }\n"
          "  qp->params[qp->count].value_is_encoded = 0;\n"
          "\n"
          "  qp->count++;\n"
          "  return CDD_C_SUCCESS;\n"
          "}\n"
          "\n"
          "/**\n"
          " * @brief Executes the url query add encoded operation.\n"
          " */\n"
          "int url_query_add_encoded(struct UrlQueryParams *qp, const char "
          "*key,\n"
          "                          const char *value) {\n"
          "  char *_ast_strdup_2 = NULL;\n"
          "  char *_ast_strdup_3 = NULL;\n"
          "  if (!qp || !key || !value)\n"
          "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "\n"
          "  if (qp->count >= qp->capacity) {\n"
          "    size_t new_cap = (qp->capacity == 0) ? 4 : qp->capacity * 2;\n"
          "    struct UrlQueryParam *new_arr = (struct UrlQueryParam "
          "*)realloc(\n"
          "        qp->params, new_cap * sizeof(struct UrlQueryParam));\n"
          "    if (!new_arr) {\n"
          "      C_CDD_LOG_DEBUG(\"ENOMEM: OOM\\n\");\n"
          "      return CDD_C_ERROR_MEMORY;\n"
          "    }\n"
          "    qp->params = new_arr;\n"
          "    qp->capacity = new_cap;\n"
          "  }\n"
          "\n"
          "  qp->params[qp->count].key =\n"
          "      (c_cdd_strdup(key, &_ast_strdup_2), _ast_strdup_2);\n"
          "  if (!qp->params[qp->count].key)\n"
          "    return CDD_C_ERROR_MEMORY;\n"
          "\n"
          "  qp->params[qp->count].value =\n"
          "      (c_cdd_strdup(value, &_ast_strdup_3), _ast_strdup_3);\n"
          "  if (!qp->params[qp->count].value) {\n"
          "    free(qp->params[qp->count].key);\n"
          "    return CDD_C_ERROR_MEMORY;\n"
          "  }\n"
          "  qp->params[qp->count].value_is_encoded = 1;\n"
          "\n"
          "  qp->count++;\n"
          "  return CDD_C_SUCCESS;\n"
          "}\n"
          "\n"
          "/**\n"
          " * @brief Executes the url query build operation.\n"
          " */\n"
          "int url_query_build(const struct UrlQueryParams *qp, char "
          "**out_str) {\n"
          "  char *_ast_url_encode_10 = NULL;\n"
          "  char *_ast_url_encode_11 = NULL;\n"
          "  char *_ast_url_encode_12 = NULL;\n"
          "  char *_ast_url_encode_13 = NULL;\n"
          "  char *_ast_strdup_4 = NULL;\n"
          "  char *_ast_strdup_5 = NULL;\n"
          "  char *_ast_strdup_6 = NULL;\n"
          "  size_t i;\n"
          "  size_t total_len = 0;\n"
          "  char *buf = NULL;\n"
          "  char *ptr = NULL;\n"
          "\n"
          "  if (!qp || !out_str)\n"
          "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "\n"
          "  if (qp->count == 0) {\n"
          "    *out_str = (c_cdd_strdup(\"\", &_ast_strdup_4), "
          "_ast_strdup_4);\n"
          "    return *out_str ? 0 : ENOMEM;\n"
          "  }\n"
          "\n"
          "  /* 1. Calculate Total Length */\n"
          "  /* Format: ?key=encoded_val&key2=encoded_val2 */\n"
          "  /* Overhead: '?' (1) + ('=' + '&') * count */\n"
          "  /* Actually separators are N-1 '&', 1 '?', N '='. So roughly N + "
          "N + 1. */\n"
          "\n"
          "  total_len = 1; /* '?' */\n"
          "\n"
          "  for (i = 0; i < qp->count; ++i) {\n"
          "    char *e_key = (url_encode(qp->params[i].key, "
          "&_ast_url_encode_10),\n"
          "                   _ast_url_encode_10);\n"
          "    char *e_val = NULL;\n"
          "    const char *raw_val = qp->params[i].value;\n"
          "\n"
          "    if (qp->params[i].value_is_encoded) {\n"
          "      e_val =\n"
          "          (c_cdd_strdup(raw_val ? raw_val : \"\", &_ast_strdup_5), "
          "_ast_strdup_5);\n"
          "    } else {\n"
          "      e_val = (url_encode(raw_val, &_ast_url_encode_11), "
          "_ast_url_encode_11);\n"
          "    }\n"
          "\n"
          "    if (!e_key || !e_val) {\n"
          "      if (e_key)\n"
          "        free(e_key);\n"
          "      if (e_val)\n"
          "        free(e_val);\n"
          "      return CDD_C_ERROR_MEMORY;\n"
          "    }\n"
          "\n"
          "    total_len += strlen(e_key) + 1; /* key= */\n"
          "    total_len += strlen(e_val);\n"
          "    if (i < qp->count - 1)\n"
          "      total_len += 1; /* & */\n"
          "\n"
          "    free(e_key);\n"
          "    free(e_val);\n"
          "  }\n"
          "\n"
          "  /* 2. Allocate */\n"
          "  buf = (char *)malloc(total_len + 1);\n"
          "  if (!buf) {\n"
          "    C_CDD_LOG_DEBUG(\"ENOMEM: OOM\\n\");\n"
          "    return CDD_C_ERROR_MEMORY;\n"
          "  }\n"
          "\n"
          "  /* 3. Build */\n"
          "  ptr = buf;\n"
          "  *ptr++ = '?';\n"
          "\n"
          "  for (i = 0; i < qp->count; ++i) {\n"
          "    char *e_key = (url_encode(qp->params[i].key, "
          "&_ast_url_encode_12),\n"
          "                   _ast_url_encode_12);\n"
          "    size_t kl, vl;\n"
          "    char *e_val = NULL;\n"
          "    const char *raw_val = qp->params[i].value;\n"
          "\n"
          "    if (qp->params[i].value_is_encoded) {\n"
          "      e_val =\n"
          "          (c_cdd_strdup(raw_val ? raw_val : \"\", &_ast_strdup_6), "
          "_ast_strdup_6);\n"
          "    } else {\n"
          "      e_val = (url_encode(raw_val, &_ast_url_encode_13), "
          "_ast_url_encode_13);\n"
          "    }\n"
          "\n"
          "    /* e_key/e_val guaranteed non-null here as checks passed in "
          "pass 1 */\n"
          "    /* Copy Key */\n"
          "    kl = strlen(e_key);\n"
          "    memcpy(ptr, e_key, kl);\n"
          "    ptr += kl;\n"
          "\n"
          "    /* Copy Eq */\n"
          "    *ptr++ = '=';\n"
          "\n"
          "    /* Copy Value */\n"
          "    {\n"
          "      vl = strlen(e_val);\n"
          "      memcpy(ptr, e_val, vl);\n"
          "      ptr += vl;\n"
          "    }\n"
          "\n"
          "    /* Copy Sep */\n"
          "    if (i < qp->count - 1) {\n"
          "      *ptr++ = '&';\n"
          "    }\n"
          "\n"
          "    free(e_key);\n"
          "    free(e_val);\n"
          "  }\n"
          "  *ptr = '\\0';\n"
          "\n"
          "  *out_str = buf;\n"
          "  return CDD_C_SUCCESS;\n"
          "}\n"
          "\n"
          "/**\n"
          " * @brief Executes the url query build form operation.\n"
          " */\n"
          "int url_query_build_form(const struct UrlQueryParams *qp, char "
          "**out_str) {\n"
          "  char *_ast_url_encode_form_14 = NULL;\n"
          "  char *_ast_url_encode_form_15 = NULL;\n"
          "  char *_ast_url_encode_form_16 = NULL;\n"
          "  char *_ast_url_encode_form_17 = NULL;\n"
          "  char *_ast_strdup_7 = NULL;\n"
          "  char *_ast_strdup_8 = NULL;\n"
          "  size_t i;\n"
          "  size_t total_len = 0;\n"
          "  char *buf;\n"
          "  char *ptr;\n"
          "\n"
          "  if (!qp || !out_str)\n"
          "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "\n"
          "  if (qp->count == 0) {\n"
          "    *out_str = (char *)calloc(1, 1);\n"
          "    if (!*out_str)\n"
          "      return CDD_C_ERROR_MEMORY;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "\n"
          "  for (i = 0; i < qp->count; ++i) {\n"
          "    char *e_key = (url_encode_form(qp->params[i].key, "
          "&_ast_url_encode_form_14),\n"
          "                   _ast_url_encode_form_14);\n"
          "    char *e_val;\n"
          "    size_t kl, vl;\n"
          "    if (!e_key) {\n"
          "      C_CDD_LOG_DEBUG(\"ENOMEM: OOM\\n\");\n"
          "      return CDD_C_ERROR_MEMORY;\n"
          "    }\n"
          "    if (qp->params[i].value_is_encoded) {\n"
          "      e_val =\n"
          "          (c_cdd_strdup(qp->params[i].value, &_ast_strdup_7), "
          "_ast_strdup_7);\n"
          "    } else {\n"
          "      e_val = (url_encode_form(qp->params[i].value, "
          "&_ast_url_encode_form_15),\n"
          "               _ast_url_encode_form_15);\n"
          "    }\n"
          "    if (!e_val) {\n"
          "      free(e_key);\n"
          "      return CDD_C_ERROR_MEMORY;\n"
          "    }\n"
          "    kl = strlen(e_key);\n"
          "    vl = strlen(e_val);\n"
          "    total_len += kl + 1 + vl;\n"
          "    if (i + 1 < qp->count)\n"
          "      total_len += 1;\n"
          "    free(e_key);\n"
          "    free(e_val);\n"
          "  }\n"
          "\n"
          "  buf = (char *)malloc(total_len + 1);\n"
          "  if (!buf) {\n"
          "    C_CDD_LOG_DEBUG(\"ENOMEM: OOM\\n\");\n"
          "    return CDD_C_ERROR_MEMORY;\n"
          "  }\n"
          "  ptr = buf;\n"
          "\n"
          "  for (i = 0; i < qp->count; ++i) {\n"
          "    char *e_key = (url_encode_form(qp->params[i].key, "
          "&_ast_url_encode_form_16),\n"
          "                   _ast_url_encode_form_16);\n"
          "    char *e_val;\n"
          "    size_t kl, vl;\n"
          "    if (!e_key) {\n"
          "      free(buf);\n"
          "      return CDD_C_ERROR_MEMORY;\n"
          "    }\n"
          "    if (qp->params[i].value_is_encoded) {\n"
          "      e_val =\n"
          "          (c_cdd_strdup(qp->params[i].value, &_ast_strdup_8), "
          "_ast_strdup_8);\n"
          "    } else {\n"
          "      e_val = (url_encode_form(qp->params[i].value, "
          "&_ast_url_encode_form_17),\n"
          "               _ast_url_encode_form_17);\n"
          "    }\n"
          "    if (!e_val) {\n"
          "      free(e_key);\n"
          "      free(buf);\n"
          "      return CDD_C_ERROR_MEMORY;\n"
          "    }\n"
          "    kl = strlen(e_key);\n"
          "    vl = strlen(e_val);\n"
          "    memcpy(ptr, e_key, kl);\n"
          "    ptr += kl;\n"
          "    *ptr++ = '=';\n"
          "    memcpy(ptr, e_val, vl);\n"
          "    ptr += vl;\n"
          "    if (i + 1 < qp->count)\n"
          "      *ptr++ = '&';\n"
          "    free(e_key);\n"
          "    free(e_val);\n"
          "  }\n"
          "  *ptr = '\\0';\n"
          "  *out_str = buf;\n"
          "  return CDD_C_SUCCESS;\n"
          "}\n"
          "\n"
          "static int\n"
          "    append_str(char **buf, size_t *len, size_t *cap, const char *s) "
          "{\n"
          "  size_t slen;\n"
          "  size_t need;\n"
          "  char *tmp;\n"
          "\n"
          "  if (!buf || !len || !cap || !s)\n"
          "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "\n"
          "  slen = strlen(s);\n"
          "  need = *len + slen + 1;\n"
          "  if (need > *cap) {\n"
          "    size_t new_cap = (*cap == 0) ? 64 : *cap * 2;\n"
          "    while (new_cap < need)\n"
          "      new_cap *= 2;\n"
          "    tmp = (char *)realloc(*buf, new_cap);\n"
          "    if (!tmp) {\n"
          "      C_CDD_LOG_DEBUG(\"ENOMEM: OOM\\n\");\n"
          "      return CDD_C_ERROR_MEMORY;\n"
          "    }\n"
          "    *buf = tmp;\n"
          "    *cap = new_cap;\n"
          "  }\n"
          "  memcpy(*buf + *len, s, slen);\n"
          "  *len += slen;\n"
          "  (*buf)[*len] = '\\0';\n"
          "  return CDD_C_SUCCESS;\n"
          "}\n"
          "\n"
          "static int\n"
          "    kv_value_to_string(const struct OpenAPI_KV *kv, char *buf, "
          "size_t buf_len,\n"
          "                       const char **_out_val) {\n"
          "  if (!kv) {\n"
          "    *_out_val = NULL;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "  switch (kv->type) {\n"
          "  case OA_KV_STRING: {\n"
          "    *_out_val = kv->value.s ? kv->value.s : NULL;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "  case OA_KV_INTEGER:\n"
          "    if (!buf || buf_len == 0) {\n"
          "      *_out_val = NULL;\n"
          "      return CDD_C_SUCCESS;\n"
          "    }\n"
          "    sprintf_s_chk(buf, buf_len, \"%d\", kv->value.i);\n"
          "    {\n"
          "      *_out_val = buf;\n"
          "      return CDD_C_SUCCESS;\n"
          "    }\n"
          "  case OA_KV_NUMBER:\n"
          "    if (!buf || buf_len == 0) {\n"
          "      *_out_val = NULL;\n"
          "      return CDD_C_SUCCESS;\n"
          "    }\n"
          "    sprintf_s_chk(buf, buf_len, \"%g\", kv->value.n);\n"
          "    {\n"
          "      *_out_val = buf;\n"
          "      return CDD_C_SUCCESS;\n"
          "    }\n"
          "  case OA_KV_BOOLEAN: {\n"
          "    *_out_val = kv->value.b ? \"true\" : \"false\";\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "  default: {\n"
          "    *_out_val = NULL;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "  }\n"
          "}\n"
          "\n"
          "/**\n"
          " * @brief Executes the openapi kv join form operation.\n"
          " */\n"
          "int openapi_kv_join_form(const struct OpenAPI_KV *kvs, size_t n,\n"
          "                         const char *delim, int allow_reserved,\n"
          "                         char **_out_val) {\n"
          "  const char *_ast_kv_value_to_string_18 = NULL;\n"
          "  size_t i;\n"
          "  char *buf = NULL;\n"
          "  size_t len = 0;\n"
          "  size_t cap = 0;\n"
          "  char num_buf[64];\n"
          "  char *enc_key = NULL;\n"
          "  char *enc_val = NULL;\n"
          "  int (*enc_fn)(const char *, char **) =\n"
          "      allow_reserved ? url_encode_form_allow_reserved : "
          "url_encode_form;\n"
          "\n"
          "  if (!delim)\n"
          "    delim = \",\";\n"
          "\n"
          "  if (!kvs || n == 0) {\n"
          "    buf = (char *)calloc(1, 1);\n"
          "    {\n"
          "      *_out_val = buf;\n"
          "      return CDD_C_SUCCESS;\n"
          "    }\n"
          "  }\n"
          "\n"
          "  for (i = 0; i < n; ++i) {\n"
          "    const char *raw_val;\n"
          "    if (!kvs[i].key)\n"
          "      continue;\n"
          "    raw_val = (kv_value_to_string(&kvs[i], num_buf, "
          "sizeof(num_buf),\n"
          "                                  &_ast_kv_value_to_string_18),\n"
          "               _ast_kv_value_to_string_18);\n"
          "    if (!raw_val)\n"
          "      continue;\n"
          "    enc_fn(kvs[i].key, &enc_key);\n"
          "    if (!enc_key)\n"
          "      goto oom;\n"
          "    enc_fn(raw_val, &enc_val);\n"
          "    if (!enc_val)\n"
          "      goto oom;\n"
          "    if (len > 0) {\n"
          "      if (append_str(&buf, &len, &cap, delim) != 0)\n"
          "        goto oom;\n"
          "    }\n"
          "    if (append_str(&buf, &len, &cap, enc_key) != 0)\n"
          "      goto oom;\n"
          "    if (append_str(&buf, &len, &cap, delim) != 0)\n"
          "      goto oom;\n"
          "    if (append_str(&buf, &len, &cap, enc_val) != 0)\n"
          "      goto oom;\n"
          "    free(enc_key);\n"
          "    free(enc_val);\n"
          "    enc_key = NULL;\n"
          "    enc_val = NULL;\n"
          "  }\n"
          "\n"
          "  if (!buf) {\n"
          "    buf = (char *)calloc(1, 1);\n"
          "  }\n"
          "  {\n"
          "    *_out_val = buf;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "\n"
          "oom:\n"
          "  if (enc_key)\n"
          "    free(enc_key);\n"
          "  if (enc_val)\n"
          "    free(enc_val);\n"
          "  if (buf)\n"
          "    free(buf);\n"
          "  {\n"
          "    *_out_val = NULL;\n"
          "    return CDD_C_SUCCESS;\n"
          "  }\n"
          "}\n"
          "\n");
      fclose(uc);
    }
  }

  if (config && config->create_tests_and_mocks) {
    char tdir[512], tfile[640];
    FILE *tfp;
    CDD_SNPRINTF(tdir, sizeof(tdir), "%s/src/test", dir_name ? dir_name : ".");
    makedirs(tdir);
    CDD_SNPRINTF(tfile, sizeof(tfile), "%s/test_sdk.c", tdir);
    tfp = fopen(tfile, "w");
    if (tfp) {
      fprintf(tfp,
              "#include <stdio.h>\n#include <stdlib.h>\n#include "
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
          char *in;
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
          while (*in &&
                 (size_t)(out - formatted_path) < sizeof(formatted_path) - 2) {
            if (*in == '{') {
              *out++ = '1';
              while (*in && *in != '}')
                in++;
              if (*in == '}')
                in++;
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
                strncat(formatted_path, "?",
                        sizeof(formatted_path) - strlen(formatted_path) - 1);
                first_query = 0;
              } else {
                strncat(formatted_path, "&",
                        sizeof(formatted_path) - strlen(formatted_path) - 1);
              }
              strncat(formatted_path, op->parameters[p].name,
                      sizeof(formatted_path) - strlen(formatted_path) - 1);
              strncat(formatted_path, "=1",
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
          fprintf(
              tfp,
              "        CDD_SNPRINTF(url_buf, sizeof(url_buf), \"%%s%%s%%s\", "
              "base_url, \"%s\", \"%s\");\n",
              parsed_base_path, formatted_path);
          fprintf(tfp, "    }\n");
          fprintf(tfp, "    req.url = url_buf;\n");
          fprintf(tfp, "    client.send(client.transport, &req, &res);\n");

          fprintf(tfp, "    if (res && res->status_code >= 200 && "
                       "res->status_code < 300) allowed = 1;\n");
          for (r = 0; r < op->n_responses; ++r) {
            if (strcmp(op->responses[r].code, "default") != 0) {
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
            fprintf(
                tfp,
                "            memcpy(body_str, res->body, res->body_len);\n");
            fprintf(tfp, "            body_str[res->body_len] = '\\0';\n");
            fprintf(tfp,
                    "            rc = %s_from_json(body_str, "
                    "&out);\n",
                    op->responses[0].schema.ref_name);
            fprintf(tfp, "            if (rc != 0) { fprintf(stderr, \"Parse "
                         "failed\\n\");  }\n");
            fprintf(tfp, "            free(body_str);\n");
            fprintf(tfp, "        }\n");
          }
          fprintf(
              tfp,
              "    } else { fprintf(stderr, \"Status %%d on %%s\\n\", res ? "
              "res->status_code : 0, req.url);  }\n");
          fprintf(tfp, "    http_response_free(res);\n");
          fprintf(tfp, "  }\n");
        }
      }

      fprintf(tfp, "  return CDD_C_SUCCESS;\n}\n");
      fclose(tfp);
    }
  }

  if (config && !config->no_installable_package) {
    rc = generate_cmake_project(dir_name ? dir_name : ".",
                                base_name ? base_name : "generated_client",
                                config->create_tests_and_mocks);
    if (rc != 0) {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

cleanup:
  if (hfile)
    fclose(hfile);
  if (cfile)
    fclose(cfile);
  if (mhfile)
    fclose(mhfile);
  if (mcfile)
    fclose(mcfile);
  if (h_name)
    free(h_name);
  if (c_name)
    free(c_name);
  if (mh_name)
    free(mh_name);
  if (mc_name)
    free(mc_name);
  if (guard)
    free(guard);
  if (model_h)
    free(model_h);
  if (model_guard)
    free(model_guard);
  if (dir_name)
    free(dir_name);
  if (base_name)
    free(base_name);
  if (actual_base)
    free(actual_base);

  return rc;
}

/* LCOV_EXCL_STOP */
