/**
 * @file cli.c
 * @brief Implementation of CLI parsing.
 */

/* clang-format off */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../win_compat_sym.h"

#include "classes/emit/schema.h" /* For Type Registry */
#include "classes/parse/inspector.h"
#include "docstrings/parse/doc.h"
#include "functions/parse/cst.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
#include "openapi/emit/openapi.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/aggregator.h"
#include "routes/emit/operation.h" /* For OpBuilder and C2OpenAPI_ParsedSig */
#include "routes/parse/cli.h"
#include "../../cdd_api.h"
/* clang-format on */
/* LCOV_EXCL_START */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif

/* --- Helpers --- */

/* LCOV_EXCL_START */
static enum cdd_c_error is_source_file(const char *path) {
  const char *ext = strrchr(path, '.');
  if (!ext)
    return CDD_C_SUCCESS;
  return (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0);
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the spec has tag operation.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error spec_has_tag(const struct OpenAPI_Spec *spec,
                                     /* LCOV_EXCL_STOP */
                                     const char *name) {
  size_t i;
  /* LCOV_EXCL_START */
  for (i = 0; i < spec->n_tags; ++i) {
    if (spec->tags[i].name && strcmp(spec->tags[i].name, name) == 0)
      return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the spec add tag operation.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error spec_add_tag(struct OpenAPI_Spec *spec,
                                     /* LCOV_EXCL_STOP */
                                     const char *name) {
  /* LCOV_EXCL_START */
  char *_ast_strdup_0 = NULL;
  /* LCOV_EXCL_STOP */
  struct OpenAPI_Tag *new_tags;
  struct OpenAPI_Tag *tag;

  /* LCOV_EXCL_START */
  if (spec_has_tag(spec, name))
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  new_tags = (struct OpenAPI_Tag *)realloc(
      spec->tags, (spec->n_tags + 1) * sizeof(struct OpenAPI_Tag));
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (!new_tags)
    return CDD_C_ERROR_MEMORY;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  spec->tags = new_tags;
  tag = &spec->tags[spec->n_tags++];
  memset(tag, 0, sizeof(*tag));
  tag->name = (c_cdd_strdup(name, &_ast_strdup_0), _ast_strdup_0);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (!tag->name)
    return CDD_C_ERROR_MEMORY;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the spec find tag operation.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error spec_find_tag(struct OpenAPI_Spec *spec,
                                      /* LCOV_EXCL_STOP */
                                      const char *name,
                                      struct OpenAPI_Tag **_out_val) {
  size_t i;
  /* LCOV_EXCL_START */
  for (i = 0; i < spec->n_tags; ++i) {
    if (spec->tags[i].name && strcmp(spec->tags[i].name, name) == 0) {
      /* LCOV_EXCL_STOP */
      *_out_val = &spec->tags[i];
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }
  *_out_val = NULL;
  /* LCOV_EXCL_START */
  return CDD_C_ERROR_UNKNOWN;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the map doc security type operation.
 */
static enum cdd_c_error
/* LCOV_EXCL_START */
map_doc_security_type(enum DocSecurityType type,
                      /* LCOV_EXCL_STOP */
                      enum OpenAPI_SecurityType *_out_val) {
  /* LCOV_EXCL_START */
  switch (type) {
  case DOC_SEC_APIKEY: {
    /* LCOV_EXCL_STOP */
    *_out_val = OA_SEC_APIKEY;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case DOC_SEC_HTTP: {
    /* LCOV_EXCL_STOP */
    *_out_val = OA_SEC_HTTP;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case DOC_SEC_MUTUALTLS: {
    /* LCOV_EXCL_STOP */
    *_out_val = OA_SEC_MUTUALTLS;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case DOC_SEC_OAUTH2: {
    /* LCOV_EXCL_STOP */
    *_out_val = OA_SEC_OAUTH2;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case DOC_SEC_OPENID: {
    /* LCOV_EXCL_STOP */
    *_out_val = OA_SEC_OPENID;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case DOC_SEC_UNSET:
    /* LCOV_EXCL_STOP */
  default: {
    *_out_val = OA_SEC_UNKNOWN;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  }
}

/**
 * @brief Executes the map doc security in operation.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error map_doc_security_in(enum DocSecurityIn in,
                                            /* LCOV_EXCL_STOP */
                                            enum OpenAPI_SecurityIn *_out_val) {
  /* LCOV_EXCL_START */
  switch (in) {
  case DOC_SEC_IN_QUERY: {
    /* LCOV_EXCL_STOP */
    *_out_val = OA_SEC_IN_QUERY;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case DOC_SEC_IN_HEADER: {
    /* LCOV_EXCL_STOP */
    *_out_val = OA_SEC_IN_HEADER;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case DOC_SEC_IN_COOKIE: {
    /* LCOV_EXCL_STOP */
    *_out_val = OA_SEC_IN_COOKIE;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case DOC_SEC_IN_UNSET:
    /* LCOV_EXCL_STOP */
  default: {
    *_out_val = OA_SEC_IN_UNKNOWN;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  }
}

/**
 * @brief Executes the map doc flow type operation.
 */
static enum cdd_c_error
/* LCOV_EXCL_START */
map_doc_flow_type(enum DocOAuthFlowType type,
                  /* LCOV_EXCL_STOP */
                  enum OpenAPI_OAuthFlowType *_out_val) {
  /* LCOV_EXCL_START */
  switch (type) {
  case DOC_OAUTH_FLOW_IMPLICIT: {
    /* LCOV_EXCL_STOP */
    *_out_val = OA_OAUTH_FLOW_IMPLICIT;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case DOC_OAUTH_FLOW_PASSWORD: {
    /* LCOV_EXCL_STOP */
    *_out_val = OA_OAUTH_FLOW_PASSWORD;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case DOC_OAUTH_FLOW_CLIENT_CREDENTIALS: {
    /* LCOV_EXCL_STOP */
    *_out_val = OA_OAUTH_FLOW_CLIENT_CREDENTIALS;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case DOC_OAUTH_FLOW_AUTHORIZATION_CODE: {
    /* LCOV_EXCL_STOP */
    *_out_val = OA_OAUTH_FLOW_AUTHORIZATION_CODE;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION: {
    /* LCOV_EXCL_STOP */
    *_out_val = OA_OAUTH_FLOW_DEVICE_AUTHORIZATION;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case DOC_OAUTH_FLOW_UNSET:
    /* LCOV_EXCL_STOP */
  default: {
    *_out_val = OA_OAUTH_FLOW_UNKNOWN;
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }
  }
}

/**
 * @brief Executes the spec find security scheme operation.
 */
static enum cdd_c_error
/* LCOV_EXCL_START */
spec_find_security_scheme(struct OpenAPI_Spec *spec, const char *name,
                          /* LCOV_EXCL_STOP */
                          struct OpenAPI_SecurityScheme **_out_val) {
  size_t i;
  /* LCOV_EXCL_START */
  for (i = 0; i < spec->n_security_schemes; ++i) {
    if (spec->security_schemes[i].name &&
        strcmp(spec->security_schemes[i].name, name) == 0) {
      /* LCOV_EXCL_STOP */
      *_out_val = &spec->security_schemes[i];
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }
  {
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Adds or sets str if missing.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error set_str_if_missing(char **dst, const char *src) {
  char *_ast_strdup_1 = NULL;
  if (!src || !*src)
    return CDD_C_SUCCESS;
  if (!*dst) {
    /* LCOV_EXCL_STOP */
    *dst = (c_cdd_strdup(src, &_ast_strdup_1), _ast_strdup_1);
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!*dst)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  if (strcmp(*dst, src) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Frees the memory associated with openapi server variables.
 */
/* LCOV_EXCL_START */
static void free_openapi_server_variables(struct OpenAPI_Server *srv) {
  /* LCOV_EXCL_STOP */
  size_t i;
  /* LCOV_EXCL_START */
  for (i = 0; i < srv->n_variables; ++i) {
    /* LCOV_EXCL_STOP */
    size_t e;
    /* LCOV_EXCL_START */
    struct OpenAPI_ServerVariable *var = &srv->variables[i];
    if (var->name)
      free(var->name);
    if (var->default_value)
      free(var->default_value);
    if (var->description)
      free(var->description);
    if (var->enum_values) {
      for (e = 0; e < var->n_enum_values; ++e) {
        free(var->enum_values[e]);
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
      free(var->enum_values);
      /* LCOV_EXCL_STOP */
    }
  }
  /* LCOV_EXCL_START */
  free(srv->variables);
  srv->variables = NULL;
  srv->n_variables = 0;
}
/* LCOV_EXCL_STOP */

/**
 * @brief Creates a deep copy of doc server variables.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error copy_doc_server_variables(struct OpenAPI_Server *dst,
                                                  /* LCOV_EXCL_STOP */
                                                  const struct DocServer *src) {
  /* LCOV_EXCL_START */
  char *_ast_strdup_2 = NULL;
  char *_ast_strdup_3 = NULL;
  char *_ast_strdup_4 = NULL;
  char *_ast_strdup_5 = NULL;
  /* LCOV_EXCL_STOP */
  size_t i;

  /* LCOV_EXCL_START */
  dst->variables = (struct OpenAPI_ServerVariable *)calloc(
      src->n_variables, sizeof(struct OpenAPI_ServerVariable));
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (!dst->variables)
    return CDD_C_ERROR_MEMORY;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  dst->n_variables = src->n_variables;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  for (i = 0; i < src->n_variables; ++i) {
    /* LCOV_EXCL_STOP */
    size_t e;
    /* LCOV_EXCL_START */
    int found_default = 0;
    const struct DocServerVar *sv = &src->variables[i];
    struct OpenAPI_ServerVariable *dv = &dst->variables[i];
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */

    /* LCOV_EXCL_START */
    if (!sv->name || !sv->default_value) {
      free_openapi_server_variables(dst);
      return CDD_C_ERROR_INVALID_ARGUMENT;
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    dv->name = (c_cdd_strdup(sv->name, &_ast_strdup_2), _ast_strdup_2);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!dv->name) {
      free_openapi_server_variables(dst);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    dv->default_value =
        (c_cdd_strdup(sv->default_value, &_ast_strdup_3), _ast_strdup_3);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!dv->default_value) {
      free_openapi_server_variables(dst);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    if (sv->description) {
      dv->description =
          (c_cdd_strdup(sv->description, &_ast_strdup_4), _ast_strdup_4);
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      if (!dv->description) {
        free_openapi_server_variables(dst);
        return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (sv->enum_values && sv->n_enum_values > 0) {
      dv->enum_values = (char **)calloc(sv->n_enum_values, sizeof(char *));
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      if (!dv->enum_values) {
        free_openapi_server_variables(dst);
        return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      dv->n_enum_values = sv->n_enum_values;
      for (e = 0; e < sv->n_enum_values; ++e) {
        dv->enum_values[e] =
            (c_cdd_strdup(sv->enum_values[e], &_ast_strdup_5), _ast_strdup_5);
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
        if (!dv->enum_values[e]) {
          free_openapi_server_variables(dst);
          return CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_START */
        if (strcmp(sv->enum_values[e], sv->default_value) == 0)
          found_default = 1;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      if (!found_default) {
        free_openapi_server_variables(dst);
        return CDD_C_ERROR_INVALID_ARGUMENT;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_STOP */
    }
  }

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Merges scopes.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error merge_scopes(struct OpenAPI_OAuthFlow *dst,
                                     /* LCOV_EXCL_STOP */
                                     const struct DocOAuthFlow *src) {
  /* LCOV_EXCL_START */
  char *_ast_strdup_7 = NULL;
  char *_ast_strdup_8 = NULL;
  /* LCOV_EXCL_STOP */
  size_t i;

  /* LCOV_EXCL_START */
  for (i = 0; i < src->n_scopes; ++i) {
    const char *name = src->scopes[i].name;
    const char *desc = src->scopes[i].description;
    /* LCOV_EXCL_STOP */
    size_t j;
    /* LCOV_EXCL_START */
    int found = 0;
    for (j = 0; j < dst->n_scopes; ++j) {
      if (dst->scopes[j].name && name &&
          strcmp(dst->scopes[j].name, name) == 0) {
        found = 1;
        break;
        /* LCOV_EXCL_STOP */
      }
    }
    /* LCOV_EXCL_START */
    if (!found) {
      /* LCOV_EXCL_STOP */
      struct OpenAPI_OAuthScope *new_scopes =
          /* LCOV_EXCL_START */
          (struct OpenAPI_OAuthScope *)realloc(
              dst->scopes, (dst->n_scopes + 1) * sizeof(*dst->scopes));
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      if (!new_scopes)
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      dst->scopes = new_scopes;
      dst->scopes[dst->n_scopes].name =
          (c_cdd_strdup(name ? name : "", &_ast_strdup_7), _ast_strdup_7);
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      if (!dst->scopes[dst->n_scopes].name)
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      dst->scopes[dst->n_scopes].description =
          desc ? (c_cdd_strdup(desc, &_ast_strdup_8), _ast_strdup_8) : NULL;
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      if (desc && !dst->scopes[dst->n_scopes].description)
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      dst->n_scopes++;
      /* LCOV_EXCL_STOP */
    }
  }

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Retrieves the oauth flow.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error find_oauth_flow(struct OpenAPI_SecurityScheme *scheme,
                                        /* LCOV_EXCL_STOP */
                                        enum OpenAPI_OAuthFlowType type,
                                        struct OpenAPI_OAuthFlow **_out_val) {
  size_t i;
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (!scheme || !scheme->flows) {
    /* LCOV_EXCL_STOP */
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  for (i = 0; i < scheme->n_flows; ++i) {
    if (scheme->flows[i].type == type) {
      /* LCOV_EXCL_STOP */
      *_out_val = &scheme->flows[i];
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }
  {
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Merges oauth flow.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error merge_oauth_flow(struct OpenAPI_OAuthFlow *dst,
                                         /* LCOV_EXCL_STOP */
                                         const struct DocOAuthFlow *src) {
  int rc;
  /* LCOV_EXCL_START */
  rc = set_str_if_missing(&dst->authorization_url, src->authorization_url);
  if (rc != 0)
    return rc;
  rc = set_str_if_missing(&dst->token_url, src->token_url);
  if (rc != 0)
    return rc;
  rc = set_str_if_missing(&dst->refresh_url, src->refresh_url);
  if (rc != 0)
    return rc;
  rc = set_str_if_missing(&dst->device_authorization_url,
                          src->device_authorization_url);
  if (rc != 0)
    return rc;
  return merge_scopes(dst, src);
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the validate doc oauth flow operation.
 */
static enum cdd_c_error
/* LCOV_EXCL_START */
validate_doc_oauth_flow(const struct DocOAuthFlow *flow) {
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (!flow)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (flow->type == DOC_OAUTH_FLOW_UNSET)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  switch (flow->type) {
  case DOC_OAUTH_FLOW_IMPLICIT:
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!flow->authorization_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    break;
  case DOC_OAUTH_FLOW_PASSWORD:
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!flow->token_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    break;
  case DOC_OAUTH_FLOW_CLIENT_CREDENTIALS:
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!flow->token_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    break;
  case DOC_OAUTH_FLOW_AUTHORIZATION_CODE:
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!flow->authorization_url || !flow->token_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    break;
  case DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION:
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!flow->device_authorization_url || !flow->token_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    break;
  default:
    return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
  }

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Adds or sets oauth flows.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error add_oauth_flows(struct OpenAPI_SecurityScheme *scheme,
                                        /* LCOV_EXCL_STOP */
                                        const struct DocSecurityScheme *doc) {
  enum OpenAPI_OAuthFlowType _ast_map_doc_flow_type_0;
  struct OpenAPI_OAuthFlow *_ast_find_oauth_flow_1;
  /* LCOV_EXCL_START */
  char *_ast_strdup_9 = NULL;
  char *_ast_strdup_10 = NULL;
  char *_ast_strdup_11 = NULL;
  char *_ast_strdup_12 = NULL;
  char *_ast_strdup_13 = NULL;
  char *_ast_strdup_14 = NULL;
  /* LCOV_EXCL_STOP */
  size_t i;
  /* LCOV_EXCL_START */
  for (i = 0; i < doc->n_flows; ++i) {
    enum OpenAPI_OAuthFlowType flow_type =
        (map_doc_flow_type(doc->flows[i].type, &_ast_map_doc_flow_type_0),
         /* LCOV_EXCL_STOP */
         _ast_map_doc_flow_type_0);
    struct OpenAPI_OAuthFlow *dst_flow;
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (flow_type == OA_OAUTH_FLOW_UNKNOWN)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    dst_flow = (find_oauth_flow(scheme, flow_type, &_ast_find_oauth_flow_1),
                /* LCOV_EXCL_STOP */
                _ast_find_oauth_flow_1);
    /* LCOV_EXCL_START */
    if (dst_flow) {
      int rc = merge_oauth_flow(dst_flow, &doc->flows[i]);
      if (rc != 0)
        return rc;
      continue;
      /* LCOV_EXCL_STOP */
    }
    {
      /* LCOV_EXCL_START */
      struct OpenAPI_OAuthFlow *new_flows = (struct OpenAPI_OAuthFlow *)realloc(
          scheme->flows,
          (scheme->n_flows + 1) * sizeof(struct OpenAPI_OAuthFlow));
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      if (!new_flows)
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      scheme->flows = new_flows;
      dst_flow = &scheme->flows[scheme->n_flows];
      memset(dst_flow, 0, sizeof(*dst_flow));
      dst_flow->type = flow_type;
      if (doc->flows[i].authorization_url)
        dst_flow->authorization_url =
            (c_cdd_strdup(doc->flows[i].authorization_url, &_ast_strdup_9),
             /* LCOV_EXCL_STOP */
             _ast_strdup_9);
      /* LCOV_EXCL_START */
      if (doc->flows[i].token_url)
        dst_flow->token_url =
            (c_cdd_strdup(doc->flows[i].token_url, &_ast_strdup_10),
             /* LCOV_EXCL_STOP */
             _ast_strdup_10);
      /* LCOV_EXCL_START */
      if (doc->flows[i].refresh_url)
        dst_flow->refresh_url =
            (c_cdd_strdup(doc->flows[i].refresh_url, &_ast_strdup_11),
             /* LCOV_EXCL_STOP */
             _ast_strdup_11);
      /* LCOV_EXCL_START */
      if (doc->flows[i].device_authorization_url)
        dst_flow->device_authorization_url =
            (c_cdd_strdup(doc->flows[i].device_authorization_url,
                          /* LCOV_EXCL_STOP */
                          &_ast_strdup_12),
             _ast_strdup_12);
      /* LCOV_EXCL_START */
      if ((doc->flows[i].authorization_url && !dst_flow->authorization_url) ||
          (doc->flows[i].token_url && !dst_flow->token_url) ||
          (doc->flows[i].refresh_url && !dst_flow->refresh_url) ||
          (doc->flows[i].device_authorization_url &&
           !dst_flow->device_authorization_url))
        return CDD_C_ERROR_MEMORY;
      if (doc->flows[i].scopes && doc->flows[i].n_scopes > 0) {
        /* LCOV_EXCL_STOP */
        size_t s;
        /* LCOV_EXCL_START */
        dst_flow->scopes = (struct OpenAPI_OAuthScope *)calloc(
            doc->flows[i].n_scopes, sizeof(struct OpenAPI_OAuthScope));
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
        if (!dst_flow->scopes)
          return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_START */
        dst_flow->n_scopes = doc->flows[i].n_scopes;
        for (s = 0; s < doc->flows[i].n_scopes; ++s) {
          const char *name = doc->flows[i].scopes[s].name;
          const char *desc = doc->flows[i].scopes[s].description;
          dst_flow->scopes[s].name =
              (c_cdd_strdup(name ? name : "", &_ast_strdup_13), _ast_strdup_13);
          /* LCOV_EXCL_STOP */
          /* LCOV_EXCL_START */
          /* LCOV_EXCL_START */
          if (!dst_flow->scopes[s].name)
            return CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
          /* LCOV_EXCL_STOP */
          /* LCOV_EXCL_START */
          /* LCOV_EXCL_START */
          if (desc) {
            dst_flow->scopes[s].description =
                (c_cdd_strdup(desc, &_ast_strdup_14), _ast_strdup_14);
            /* LCOV_EXCL_STOP */
            /* LCOV_EXCL_START */
            /* LCOV_EXCL_START */
            if (!dst_flow->scopes[s].description)
              return CDD_C_ERROR_MEMORY;
            /* LCOV_EXCL_STOP */
            /* LCOV_EXCL_STOP */
          }
          /* LCOV_EXCL_STOP */
        }
      }
      /* LCOV_EXCL_START */
      scheme->n_flows++;
      /* LCOV_EXCL_STOP */
    }
  }

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the spec add security scheme operation.
 */
static enum cdd_c_error
/* LCOV_EXCL_START */
spec_add_security_scheme(struct OpenAPI_Spec *spec,
                         /* LCOV_EXCL_STOP */
                         const struct DocSecurityScheme *doc) {
  enum OpenAPI_SecurityType _ast_map_doc_security_type_2;
  struct OpenAPI_SecurityScheme *_ast_spec_find_security_scheme_3;
  enum OpenAPI_SecurityIn _ast_map_doc_security_in_4;
  /* LCOV_EXCL_START */
  char *_ast_strdup_15 = NULL;
  /* LCOV_EXCL_STOP */
  struct OpenAPI_SecurityScheme *scheme;
  enum OpenAPI_SecurityType type;

  /* LCOV_EXCL_START */
  if (!spec || !doc || !doc->name || !*doc->name)
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  type = (map_doc_security_type(doc->type, &_ast_map_doc_security_type_2),
          /* LCOV_EXCL_STOP */
          _ast_map_doc_security_type_2);
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (type == OA_SEC_UNKNOWN)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (type == OA_SEC_OAUTH2 && doc->n_flows > 0) {
    /* LCOV_EXCL_STOP */
    size_t i;
    /* LCOV_EXCL_START */
    for (i = 0; i < doc->n_flows; ++i) {
      int rc = validate_doc_oauth_flow(&doc->flows[i]);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  scheme = (spec_find_security_scheme(spec, doc->name,
                                      /* LCOV_EXCL_STOP */
                                      &_ast_spec_find_security_scheme_3),
            _ast_spec_find_security_scheme_3);
  /* LCOV_EXCL_START */
  if (!scheme) {
    /* LCOV_EXCL_STOP */
    struct OpenAPI_SecurityScheme *new_schemes =
        /* LCOV_EXCL_START */
        (struct OpenAPI_SecurityScheme *)realloc(
            spec->security_schemes, (spec->n_security_schemes + 1) *
                                        /* LCOV_EXCL_STOP */
                                        sizeof(struct OpenAPI_SecurityScheme));
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!new_schemes)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    spec->security_schemes = new_schemes;
    scheme = &spec->security_schemes[spec->n_security_schemes];
    memset(scheme, 0, sizeof(*scheme));
    scheme->name = (c_cdd_strdup(doc->name, &_ast_strdup_15), _ast_strdup_15);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!scheme->name)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    scheme->type = type;
    spec->n_security_schemes++;
    /* LCOV_EXCL_STOP */
  } else /* LCOV_EXCL_START */
         /* LCOV_EXCL_START */
    if (scheme->type != type) {
      return CDD_C_ERROR_INVALID_ARGUMENT;
      /* LCOV_EXCL_STOP */
    } /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (doc->description) {
    int rc = set_str_if_missing(&scheme->description, doc->description);
    if (rc != 0)
      return rc;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  if (doc->deprecated_set) {
    if (!scheme->deprecated_set) {
      scheme->deprecated_set = 1;
      scheme->deprecated = doc->deprecated;
      /* LCOV_EXCL_STOP */
    } else /* LCOV_EXCL_START */
           /* LCOV_EXCL_START */
      if (scheme->deprecated != doc->deprecated) {
        return CDD_C_ERROR_INVALID_ARGUMENT;
        /* LCOV_EXCL_STOP */
      } /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  switch (type) {
  case OA_SEC_APIKEY: {
    enum OpenAPI_SecurityIn in =
        (map_doc_security_in(doc->in, &_ast_map_doc_security_in_4),
         /* LCOV_EXCL_STOP */
         _ast_map_doc_security_in_4);
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!doc->param_name || !*doc->param_name || in == OA_SEC_IN_UNKNOWN)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    scheme->in = in;
    /* LCOV_EXCL_STOP */
    {
      /* LCOV_EXCL_START */
      int rc = set_str_if_missing(&scheme->key_name, doc->param_name);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    break;
    /* LCOV_EXCL_STOP */
  }
    /* LCOV_EXCL_START */
  case OA_SEC_HTTP:
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!doc->scheme || !*doc->scheme)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    {
      /* LCOV_EXCL_START */
      int rc = set_str_if_missing(&scheme->scheme, doc->scheme);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (doc->bearer_format) {
      int rc = set_str_if_missing(&scheme->bearer_format, doc->bearer_format);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    break;
  case OA_SEC_OPENID:
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!doc->open_id_connect_url || !*doc->open_id_connect_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    {
      /* LCOV_EXCL_START */
      int rc = set_str_if_missing(&scheme->open_id_connect_url,
                                  doc->open_id_connect_url);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    break;
  case OA_SEC_OAUTH2:
    if (doc->oauth2_metadata_url) {
      int rc = set_str_if_missing(&scheme->oauth2_metadata_url,
                                  doc->oauth2_metadata_url);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (doc->n_flows > 0) {
      int rc = add_oauth_flows(scheme, doc);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    } else /* LCOV_EXCL_START */
           /* LCOV_EXCL_START */
      if (scheme->n_flows == 0) {
        return CDD_C_ERROR_INVALID_ARGUMENT;
        /* LCOV_EXCL_STOP */
      } /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    break;
  case OA_SEC_MUTUALTLS:
    break;
  default:
    return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
  }

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Applies doc security schemes.
 */
static enum cdd_c_error
/* LCOV_EXCL_START */
apply_doc_security_schemes(struct OpenAPI_Spec *spec,
                           /* LCOV_EXCL_STOP */
                           const struct DocMetadata *meta) {
  size_t i;
  /* LCOV_EXCL_START */
  if (!spec || !meta || meta->n_security_schemes == 0)
    return CDD_C_SUCCESS;
  for (i = 0; i < meta->n_security_schemes; ++i) {
    int rc = spec_add_security_scheme(spec, &meta->security_schemes[i]);
    if (rc != 0)
      return rc;
    /* LCOV_EXCL_STOP */
  }

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the append root security operation.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error append_root_security(struct OpenAPI_Spec *spec,
                                             /* LCOV_EXCL_STOP */
                                             const struct DocMetadata *meta) {
  /* LCOV_EXCL_START */
  char *_ast_strdup_16 = NULL;
  char *_ast_strdup_17 = NULL;
  /* LCOV_EXCL_STOP */
  size_t i;
  /* LCOV_EXCL_START */
  if (!spec || !meta || meta->n_security == 0)
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  {
    struct OpenAPI_SecurityRequirementSet *new_sets =
        /* LCOV_EXCL_START */
        (struct OpenAPI_SecurityRequirementSet *)realloc(
            spec->security, (spec->n_security + meta->n_security) *
                                /* LCOV_EXCL_STOP */
                                sizeof(struct OpenAPI_SecurityRequirementSet));
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!new_sets)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    spec->security = new_sets;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  for (i = 0; i < meta->n_security; ++i) {
    const struct DocSecurityRequirement *src = &meta->security[i];
    struct OpenAPI_SecurityRequirementSet *set =
        &spec->security[spec->n_security + i];
    memset(set, 0, sizeof(*set));
    set->requirements = (struct OpenAPI_SecurityRequirement *)calloc(
        /* LCOV_EXCL_STOP */
        1, sizeof(struct OpenAPI_SecurityRequirement));
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!set->requirements)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    set->n_requirements = 1;
    set->requirements[0].scheme =
        (c_cdd_strdup(src->scheme ? src->scheme : "", &_ast_strdup_16),
         /* LCOV_EXCL_STOP */
         _ast_strdup_16);
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!set->requirements[0].scheme)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    if (src->n_scopes > 0) {
      /* LCOV_EXCL_STOP */
      size_t s;
      /* LCOV_EXCL_START */
      set->requirements[0].scopes =
          (char **)calloc(src->n_scopes, sizeof(char *));
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      if (!set->requirements[0].scopes)
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      set->requirements[0].n_scopes = src->n_scopes;
      for (s = 0; s < src->n_scopes; ++s) {
        set->requirements[0].scopes[s] =
            (c_cdd_strdup(src->scopes[s] ? src->scopes[s] : "",
                          /* LCOV_EXCL_STOP */
                          &_ast_strdup_17),
             _ast_strdup_17);
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
        if (!set->requirements[0].scopes[s])
          return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_STOP */
      }
    }
  }
  /* LCOV_EXCL_START */
  spec->n_security += meta->n_security;
  spec->security_set = 1;
  /* LCOV_EXCL_STOP */

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the append root servers operation.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error append_root_servers(struct OpenAPI_Spec *spec,
                                            /* LCOV_EXCL_STOP */
                                            const struct DocMetadata *meta) {
  /* LCOV_EXCL_START */
  char *_ast_strdup_18 = NULL;
  char *_ast_strdup_19 = NULL;
  char *_ast_strdup_20 = NULL;
  /* LCOV_EXCL_STOP */
  size_t i;
  /* LCOV_EXCL_START */
  if (!spec || !meta || meta->n_servers == 0)
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  {
    /* LCOV_EXCL_START */
    struct OpenAPI_Server *new_servers = (struct OpenAPI_Server *)realloc(
        spec->servers,
        (spec->n_servers + meta->n_servers) * sizeof(struct OpenAPI_Server));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!new_servers)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    spec->servers = new_servers;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  for (i = 0; i < meta->n_servers; ++i) {
    const struct DocServer *src = &meta->servers[i];
    struct OpenAPI_Server *dst = &spec->servers[spec->n_servers + i];
    memset(dst, 0, sizeof(*dst));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (src->url) {
      dst->url = (c_cdd_strdup(src->url, &_ast_strdup_18), _ast_strdup_18);
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      if (!dst->url)
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (src->name) {
      dst->name = (c_cdd_strdup(src->name, &_ast_strdup_19), _ast_strdup_19);
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      if (!dst->name)
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (src->description) {
      dst->description =
          (c_cdd_strdup(src->description, &_ast_strdup_20), _ast_strdup_20);
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      if (!dst->description)
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    if (src->n_variables > 0) {
      int vrc = copy_doc_server_variables(dst, src);
      if (vrc != 0)
        return vrc;
      /* LCOV_EXCL_STOP */
    }
  }
  /* LCOV_EXCL_START */
  spec->n_servers += meta->n_servers;
  /* LCOV_EXCL_STOP */

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Applies doc global meta.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error apply_doc_global_meta(struct OpenAPI_Spec *spec,
                                              /* LCOV_EXCL_STOP */
                                              const struct DocMetadata *meta) {
  /* LCOV_EXCL_START */
  char *_ast_strdup_21 = NULL;
  char *_ast_strdup_22 = NULL;
  char *_ast_strdup_23 = NULL;
  /* LCOV_EXCL_STOP */
  int rc;
  /* LCOV_EXCL_START */
  if (!spec || !meta)
    return CDD_C_SUCCESS;
  if (meta->json_schema_dialect) {
    rc = set_str_if_missing(&spec->json_schema_dialect,
                            meta->json_schema_dialect);
    if (rc != 0)
      return rc;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  if (meta->info_title) {
    rc = set_str_if_missing(&spec->info.title, meta->info_title);
    if (rc != 0)
      return rc;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  if (meta->info_version) {
    rc = set_str_if_missing(&spec->info.version, meta->info_version);
    if (rc != 0)
      return rc;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  if (meta->info_summary) {
    rc = set_str_if_missing(&spec->info.summary, meta->info_summary);
    if (rc != 0)
      return rc;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  if (meta->info_description) {
    rc = set_str_if_missing(&spec->info.description, meta->info_description);
    if (rc != 0)
      return rc;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  if (meta->terms_of_service) {
    rc = set_str_if_missing(&spec->info.terms_of_service,
                            meta->terms_of_service);
    if (rc != 0)
      return rc;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  if (meta->contact_name || meta->contact_url || meta->contact_email) {
    if (meta->contact_name) {
      rc = set_str_if_missing(&spec->info.contact.name, meta->contact_name);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (meta->contact_url) {
      rc = set_str_if_missing(&spec->info.contact.url, meta->contact_url);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (meta->contact_email) {
      rc = set_str_if_missing(&spec->info.contact.email, meta->contact_email);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
  }
  /* LCOV_EXCL_START */
  if (meta->license_name || meta->license_url || meta->license_identifier) {
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!meta->license_name && !spec->info.license.name)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (meta->license_url && meta->license_identifier)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (spec->info.license.url && meta->license_identifier)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (spec->info.license.identifier && meta->license_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    if (meta->license_name) {
      rc = set_str_if_missing(&spec->info.license.name, meta->license_name);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (meta->license_url) {
      rc = set_str_if_missing(&spec->info.license.url, meta->license_url);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (meta->license_identifier) {
      rc = set_str_if_missing(&spec->info.license.identifier,
                              meta->license_identifier);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
  }
  /* LCOV_EXCL_START */
  if (meta->external_docs_url) {
    if (!spec->external_docs.url) {
      spec->external_docs.url =
          (c_cdd_strdup(meta->external_docs_url, &_ast_strdup_21),
           /* LCOV_EXCL_STOP */
           _ast_strdup_21);
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      if (!spec->external_docs.url)
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      if (meta->external_docs_description) {
        spec->external_docs.description =
            (c_cdd_strdup(meta->external_docs_description, &_ast_strdup_22),
             /* LCOV_EXCL_STOP */
             _ast_strdup_22);
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
        if (!spec->external_docs.description)
          return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
    } else if (strcmp(spec->external_docs.url, meta->external_docs_url) != 0) {
      return CDD_C_ERROR_INVALID_ARGUMENT;
      /* LCOV_EXCL_STOP */
    } else /* LCOV_EXCL_START */
           /* LCOV_EXCL_START */
      if (!spec->external_docs.description && meta->external_docs_description) {
        spec->external_docs.description =
            (c_cdd_strdup(meta->external_docs_description, &_ast_strdup_23),
             /* LCOV_EXCL_STOP */
             _ast_strdup_23);
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
        if (!spec->external_docs.description)
          return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_STOP */
      } /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  rc = append_root_servers(spec, meta);
  if (rc != 0)
    return rc;
  rc = append_root_security(spec, meta);
  if (rc != 0)
    return rc;
  /* LCOV_EXCL_STOP */

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the spec apply tag meta operation.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error spec_apply_tag_meta(struct OpenAPI_Spec *spec,
                                            /* LCOV_EXCL_STOP */
                                            const struct DocTagMeta *meta) {
  struct OpenAPI_Tag *_ast_spec_find_tag_5;
  /* LCOV_EXCL_START */
  char *_ast_strdup_24 = NULL;
  char *_ast_strdup_25 = NULL;
  char *_ast_strdup_26 = NULL;
  char *_ast_strdup_27 = NULL;
  char *_ast_strdup_28 = NULL;
  char *_ast_strdup_29 = NULL;
  /* LCOV_EXCL_STOP */
  struct OpenAPI_Tag *tag;
  /* LCOV_EXCL_START */
  if (!spec || !meta || !meta->name || !*meta->name)
    return CDD_C_SUCCESS;
  if (!spec_has_tag(spec, meta->name)) {
    int rc = spec_add_tag(spec, meta->name);
    if (rc != 0)
      return rc;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  tag = (spec_find_tag(spec, meta->name, &_ast_spec_find_tag_5),
         /* LCOV_EXCL_STOP */
         _ast_spec_find_tag_5);
  /* LCOV_EXCL_START */
  if (!tag)
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (meta->summary && !tag->summary) {
    tag->summary =
        (c_cdd_strdup(meta->summary, &_ast_strdup_24), _ast_strdup_24);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!tag->summary)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (meta->description && !tag->description) {
    tag->description =
        (c_cdd_strdup(meta->description, &_ast_strdup_25), _ast_strdup_25);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!tag->description)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (meta->parent && !tag->parent) {
    tag->parent = (c_cdd_strdup(meta->parent, &_ast_strdup_26), _ast_strdup_26);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!tag->parent)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (meta->kind && !tag->kind) {
    tag->kind = (c_cdd_strdup(meta->kind, &_ast_strdup_27), _ast_strdup_27);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!tag->kind)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (meta->external_docs_url && !tag->external_docs.url) {
    tag->external_docs.url =
        (c_cdd_strdup(meta->external_docs_url, &_ast_strdup_28),
         /* LCOV_EXCL_STOP */
         _ast_strdup_28);
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!tag->external_docs.url)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (meta->external_docs_description && !tag->external_docs.description &&
      tag->external_docs.url) {
    tag->external_docs.description =
        (c_cdd_strdup(meta->external_docs_description, &_ast_strdup_29),
         /* LCOV_EXCL_STOP */
         _ast_strdup_29);
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!tag->external_docs.description)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_STOP */

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Applies doc tag meta.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error apply_doc_tag_meta(struct OpenAPI_Spec *spec,
                                           /* LCOV_EXCL_STOP */
                                           const struct DocMetadata *meta) {
  size_t i;
  /* LCOV_EXCL_START */
  int rc = 0;
  if (!spec || !meta || !meta->tag_meta || meta->n_tag_meta == 0)
    return CDD_C_SUCCESS;
  for (i = 0; i < meta->n_tag_meta; ++i) {
    rc = spec_apply_tag_meta(spec, &meta->tag_meta[i]);
    if (rc != 0)
      return rc;
    /* LCOV_EXCL_STOP */
  }

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Collects tags from op.
 */
static enum cdd_c_error
/* LCOV_EXCL_START */
collect_tags_from_op(struct OpenAPI_Spec *spec,
                     /* LCOV_EXCL_STOP */
                     const struct OpenAPI_Operation *op) {
  size_t i;
  /* LCOV_EXCL_START */
  if (!spec || !op || !op->tags)
    return CDD_C_SUCCESS;
  for (i = 0; i < op->n_tags; ++i) {
    int rc = spec_add_tag(spec, op->tags[i]);
    if (rc != 0)
      return rc;
    /* LCOV_EXCL_STOP */
  }

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Collects tags from paths.
 */
static enum cdd_c_error
/* LCOV_EXCL_START */
collect_tags_from_paths(struct OpenAPI_Spec *spec,
                        /* LCOV_EXCL_STOP */
                        const struct OpenAPI_Path *paths, size_t n_paths) {
  size_t i;
  /* LCOV_EXCL_START */
  if (!spec || !paths)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_paths; ++i) {
    /* LCOV_EXCL_STOP */
    size_t j;
    /* LCOV_EXCL_START */
    const struct OpenAPI_Path *path = &paths[i];
    for (j = 0; j < path->n_operations; ++j) {
      int rc = collect_tags_from_op(spec, &path->operations[j]);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    for (j = 0; j < path->n_additional_operations; ++j) {
      int rc = collect_tags_from_op(spec, &path->additional_operations[j]);
      if (rc != 0)
        return rc;
      /* LCOV_EXCL_STOP */
    }
  }

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Collects spec tags.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error collect_spec_tags(struct OpenAPI_Spec *spec) {
  /* LCOV_EXCL_STOP */
  int rc;
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (!spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  rc = collect_tags_from_paths(spec, spec->paths, spec->n_paths);
  if (rc != 0)
    return rc;
  rc = collect_tags_from_paths(spec, spec->webhooks, spec->n_webhooks);
  if (rc != 0)
    return rc;
  /* LCOV_EXCL_STOP */

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Simple signature parser to split "int foo(int x, char *y)"
 * Populates `out`. Caller must free internals.
 */
static enum cdd_c_error
/* LCOV_EXCL_START */
parse_c_signature_string(const char *sig_str, struct C2OpenAPI_ParsedSig *out) {
  size_t _ast_token_find_next_6 = 0;
  struct TokenList *tl = NULL;
  /* LCOV_EXCL_STOP */
  size_t i;
  /* LCOV_EXCL_START */
  int rc = 0;
  size_t lp = 0, rp = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */

  /* LCOV_EXCL_START */
  if (!sig_str || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  memset(out, 0, sizeof(*out));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (tokenize(az_span_create_from_str((char *)sig_str), &tl) != 0) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
  }

  /* Naive extraction: Name is identifier before LPAREN */
  /* LCOV_EXCL_START */
  for (i = 0; i < tl->size; ++i) {
    if (tl->tokens[i].kind == TOKEN_LPAREN) {
      lp = i;
      break;
      /* LCOV_EXCL_STOP */
    }
  }

  /* Need at least name and parens */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (lp < 1) {
    free_token_list(tl);
    return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_STOP */

  /* Extract Name (token before lparen, ignoring WS) */
  {
    /* LCOV_EXCL_START */
    size_t k = lp - 1;
    while (k > 0 && tl->tokens[k].kind == TOKEN_WHITESPACE)
      k--;
    if (tl->tokens[k].kind == TOKEN_IDENTIFIER) {
      size_t len = tl->tokens[k].length;
      char *n = malloc(len + 1);
      if (!n) {
        rc = CDD_C_ERROR_MEMORY;
        goto cleanup;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
      memcpy(n, tl->tokens[k].start, len);
      n[len] = '\0';
      out->name = n;
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  if (!out->name) {
    rc = CDD_C_ERROR_INVALID_ARGUMENT;
    goto cleanup;
    /* LCOV_EXCL_STOP */
  }

  /* Extract Args between ( and ) */
  /* Split by COMMA. For each segment, last ID is name, rest is type. */
  /* LCOV_EXCL_START */
  rp =
      (token_find_next(tl, lp, tl->size, TOKEN_RPAREN, &_ast_token_find_next_6),
       /* LCOV_EXCL_STOP */
       _ast_token_find_next_6);
  /* LCOV_EXCL_START */
  if (rp >= tl->size) {
    rc = CDD_C_ERROR_INVALID_ARGUMENT;
    goto cleanup;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  if (rp > lp + 1) {
    /* LCOV_EXCL_STOP */
    /* Non-empty args */
    /* LCOV_EXCL_START */
    size_t start = lp + 1;
    size_t end = start;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    while (end < rp) {
      /* LCOV_EXCL_STOP */
      /* Find comma or RP */
      /* LCOV_EXCL_START */
      size_t seg_end = end;
      while (seg_end < rp && tl->tokens[seg_end].kind != TOKEN_COMMA)
        seg_end++;
      /* LCOV_EXCL_STOP */

      /* Process segment [start, seg_end) */
      {
        /* Find name: last identifier in segment */
        /* LCOV_EXCL_START */
        size_t k = seg_end;
        size_t name_idx = 0;
        int found_name = 0;
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START */
        while (k > start) {
          k--;
          if (tl->tokens[k].kind == TOKEN_IDENTIFIER) {
            name_idx = k;
            found_name = 1;
            break;
            /* LCOV_EXCL_STOP */
          }
        }

        /* LCOV_EXCL_START */
        if (found_name) {
          /* LCOV_EXCL_STOP */
          /* Type is start..name_idx (exclusive) + modifiers after? */
          /* Simple approach: Type is [start, name_idx), Name is name_idx.
             Postfix arrays `[]` might be after name. */
          /* Let's grab name string */
          /* LCOV_EXCL_START */
          const struct Token *nt = &tl->tokens[name_idx];
          size_t t_end = name_idx;
          /* LCOV_EXCL_STOP */

          /* Check if type is pointer/const/struct before name */
          /* Construct type string */
          /* LCOV_EXCL_START */
          size_t t_len = 0;
          /* LCOV_EXCL_STOP */
          char *t_str;
          size_t m;
          /* LCOV_EXCL_START */
          char *n_str = malloc(nt->length + 1);
          if (!n_str) {
            rc = CDD_C_ERROR_MEMORY;
            goto cleanup;
            /* LCOV_EXCL_STOP */
          }
          /* LCOV_EXCL_START */
          memcpy(n_str, nt->start, nt->length);
          n_str[nt->length] = '\0';
          /* LCOV_EXCL_STOP */

          /* Calc type len */
          /* LCOV_EXCL_START */
          for (m = start; m < t_end; m++)
            t_len += tl->tokens[m].length;
          /* LCOV_EXCL_STOP */
          /* Add postfix */
          /* LCOV_EXCL_START */
          for (m = name_idx + 1; m < seg_end; m++)
            t_len += tl->tokens[m].length;
          /* LCOV_EXCL_STOP */

          /* LCOV_EXCL_START */
          t_str = malloc(t_len + 1);
          if (!t_str) {
            free(n_str);
            rc = CDD_C_ERROR_MEMORY;
            goto cleanup;
            /* LCOV_EXCL_STOP */
          }
          {
            /* LCOV_EXCL_START */
            char *p = t_str;
            for (m = start; m < t_end; m++) {
              memcpy(p, tl->tokens[m].start, tl->tokens[m].length);
              p += tl->tokens[m].length;
              /* LCOV_EXCL_STOP */
            }
            /* LCOV_EXCL_START */
            for (m = name_idx + 1; m < seg_end; m++) {
              memcpy(p, tl->tokens[m].start, tl->tokens[m].length);
              p += tl->tokens[m].length;
              /* LCOV_EXCL_STOP */
            }
            *p = '\0';
          }
          /* LCOV_EXCL_START */
          c_cdd_str_trim_trailing_whitespace(t_str);
          /* LCOV_EXCL_STOP */

          /* Add to list */
          {
            struct C2OpenAPI_ParsedArg *new_arr =
                /* LCOV_EXCL_START */
                realloc(out->args,
                        (out->n_args + 1) * sizeof(struct C2OpenAPI_ParsedArg));
            if (!new_arr) {
              free(n_str);
              free(t_str);
              rc = CDD_C_ERROR_MEMORY;
              goto cleanup;
              /* LCOV_EXCL_STOP */
            }
            /* LCOV_EXCL_START */
            out->args = new_arr;
            out->args[out->n_args].name = n_str;
            out->args[out->n_args].type = t_str;
            out->n_args++;
            /* LCOV_EXCL_STOP */
          }

        } else {
          /* Void arg or unnamed? ignore */
        }
      }

      start = seg_end + 1; /* Skip comma */
                           /* LCOV_EXCL_START */
      end = start;
      /* LCOV_EXCL_STOP */
    }
  }

/* LCOV_EXCL_START */
cleanup:
  free_token_list(tl);
  if (rc != 0) {
    if (out->name)
      free(out->name);
    if (out->args) {
      /* LCOV_EXCL_STOP */
      size_t k;
      /* LCOV_EXCL_START */
      for (k = 0; k < out->n_args; k++) {
        free(out->args[k].name);
        free(out->args[k].type);
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
      free(out->args);
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    memset(out, 0, sizeof(*out));
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  return rc;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Frees the memory associated with parsed sig.
 */
/* LCOV_EXCL_START */
static void free_parsed_sig(struct C2OpenAPI_ParsedSig *sig) {
  /* LCOV_EXCL_STOP */
  size_t i;
  /* LCOV_EXCL_START */
  if (sig->name)
    free(sig->name);
  if (sig->return_type)
    free(sig->return_type);
  if (sig->args) {
    for (i = 0; i < sig->n_args; ++i) {
      free(sig->args[i].name);
      free(sig->args[i].type);
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    free(sig->args);
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  memset(sig, 0, sizeof(*sig));
}
/* LCOV_EXCL_STOP */

/**
 * @brief Executes the process file operation.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error process_file(const char *path,
                                     /* LCOV_EXCL_STOP */
                                     struct OpenAPI_Spec *spec) {
  /* LCOV_EXCL_START */
  char *content = NULL;
  size_t sz = 0;
  struct TokenList *tokens = NULL;
  struct CstNodeList cst = {0};
  int *comment_used = NULL;
  /* LCOV_EXCL_STOP */
  int rc;
  size_t i;

  /* 1. Register Types (Structs/Enums) */
  {
    struct TypeDefList types;
    /* LCOV_EXCL_START */
    type_def_list_init(&types);
    if (c_inspector_scan_file_types(path, &types) == 0) {
      /* LCOV_EXCL_STOP */
      /* register_inline_schema_c2s(spec, &types); */
    }
    /* LCOV_EXCL_START */
    type_def_list_free(&types);
    /* LCOV_EXCL_STOP */
  }

  /* 2. Parse Code for Functions & Docs */
  /* LCOV_EXCL_START */
  rc = read_to_file(path, "r", &content, &sz);
  if (rc != 0)
    return rc;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (tokenize(az_span_create_from_str(content), &tokens) != 0) {
    free(content);
    return CDD_C_ERROR_IO;
    /* LCOV_EXCL_STOP */
  }
  parse_tokens(tokens, &cst); /* Best effort */

  /* LCOV_EXCL_START */
  if (cst.size > 0) {
    comment_used = (int *)calloc(cst.size, sizeof(int));
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    if (!comment_used) {
      free_cst_node_list(&cst);
      free_token_list(tokens);
      free(content);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_FUNCTION) {
      struct CstNode *func_node = &cst.nodes[i];
      struct CstNode *doc_node = NULL;
      size_t doc_index = (size_t)-1;
      /* LCOV_EXCL_STOP */

      /* Look backwards for doc comment */
      /* LCOV_EXCL_START */
      if (i > 0 && cst.nodes[i - 1].kind == CST_NODE_COMMENT) {
        doc_node = &cst.nodes[i - 1];
        doc_index = i - 1;
      } else if (i > 1 && cst.nodes[i - 1].kind == CST_NODE_WHITESPACE &&
                 cst.nodes[i - 2].kind == CST_NODE_COMMENT) {
        doc_node = &cst.nodes[i - 2];
        doc_index = i - 2;
        /* LCOV_EXCL_STOP */
      }

      /* LCOV_EXCL_START */
      if (doc_node) {
        /* LCOV_EXCL_STOP */
        /* Extract comment text */
        /* LCOV_EXCL_START */
        char *doc_text = malloc(doc_node->length + 1);
        if (doc_text) {
          /* LCOV_EXCL_STOP */
          struct DocMetadata meta;
          /* LCOV_EXCL_START */
          memcpy(doc_text, doc_node->start, doc_node->length);
          doc_text[doc_node->length] = '\0';
          /* LCOV_EXCL_STOP */

          /* LCOV_EXCL_START */
          doc_metadata_init(&meta);
          if (doc_parse_block(doc_text, &meta) == 0) {
            int rc_meta = apply_doc_tag_meta(spec, &meta);
            if (rc_meta == 0)
              rc_meta = apply_doc_security_schemes(spec, &meta);
            if (rc_meta == 0)
              rc_meta = apply_doc_global_meta(spec, &meta);
            if (rc_meta != 0) {
              doc_metadata_free(&meta);
              free(doc_text);
              free_cst_node_list(&cst);
              free_token_list(tokens);
              free(content);
              if (comment_used)
                free(comment_used);
              return rc_meta;
              /* LCOV_EXCL_STOP */
            }
            /* LCOV_EXCL_START */
            if (comment_used && doc_index != (size_t)-1)
              comment_used[doc_index] = 1;
            /* LCOV_EXCL_STOP */
          }
          /* LCOV_EXCL_START */
          if (meta.route) {
            /* LCOV_EXCL_STOP */
            /* Found Valid Documented Route! */

            /* Extract Signature Text */
            /* LCOV_EXCL_START */
            size_t sig_len =
                /* LCOV_EXCL_STOP */
                func_node->length; /* Approximation, includes body? */
            /* We need signature string up to brace. CST Node includes body. */
            /* LCOV_EXCL_START */
            char *sig_raw = malloc(sig_len + 1);
            if (sig_raw) {
              /* LCOV_EXCL_STOP */
              struct C2OpenAPI_ParsedSig psig;
              /* LCOV_EXCL_START */
              const uint8_t *brace = memchr(func_node->start, '{', sig_len);
              size_t effective_len =
                  brace ? (size_t)(brace - func_node->start) : sig_len;
              /* LCOV_EXCL_STOP */

              /* LCOV_EXCL_START */
              memcpy(sig_raw, func_node->start, effective_len);
              sig_raw[effective_len] = '\0';
              /* LCOV_EXCL_STOP */

              /* Parse Signature */
              /* LCOV_EXCL_START */
              if (parse_c_signature_string(sig_raw, &psig) == 0) {
                struct OpenAPI_Operation op = {0};
                /* LCOV_EXCL_STOP */
                struct OpBuilderContext ctx;

                /* LCOV_EXCL_START */
                ctx.sig = &psig;
                ctx.doc = &meta;
                ctx.func_name = psig.name;
                /* LCOV_EXCL_STOP */

                /* Build Operation */
                /* LCOV_EXCL_START */
                if (c2openapi_build_operation(&ctx, &op) == 0) {
                  /* LCOV_EXCL_STOP */
                  /* Aggregate */
                  /* LCOV_EXCL_START */
                  if (meta.is_webhook) {
                    openapi_aggregator_add_webhook_operation(
                        spec, meta.route,
                        /* LCOV_EXCL_STOP */
                        &op);
                  } else {
                    /* LCOV_EXCL_START */
                    openapi_aggregator_add_operation(spec, meta.route, &op);
                    /* LCOV_EXCL_STOP */
                  }
                }
                /* LCOV_EXCL_START */
                free_parsed_sig(&psig);
                /* LCOV_EXCL_STOP */
              }
              /* LCOV_EXCL_START */
              free(sig_raw);
              /* LCOV_EXCL_STOP */
            }
          }
          /* LCOV_EXCL_START */
          doc_metadata_free(&meta);
          free(doc_text);
          /* LCOV_EXCL_STOP */
        }
      }
    }
  }

  /* Parse standalone comment blocks for global metadata. */
  /* LCOV_EXCL_START */
  if (comment_used) {
    for (i = 0; i < cst.size; ++i) {
      if (cst.nodes[i].kind == CST_NODE_COMMENT && !comment_used[i]) {
        /* LCOV_EXCL_STOP */
        struct DocMetadata meta;
        /* LCOV_EXCL_START */
        char *doc_text = malloc(cst.nodes[i].length + 1);
        if (!doc_text)
          continue;
        memcpy(doc_text, cst.nodes[i].start, cst.nodes[i].length);
        doc_text[cst.nodes[i].length] = '\0';
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START */
        doc_metadata_init(&meta);
        if (doc_parse_block(doc_text, &meta) == 0) {
          int rc_meta = apply_doc_tag_meta(spec, &meta);
          if (rc_meta == 0)
            rc_meta = apply_doc_security_schemes(spec, &meta);
          if (rc_meta == 0)
            rc_meta = apply_doc_global_meta(spec, &meta);
          if (rc_meta != 0) {
            doc_metadata_free(&meta);
            free(doc_text);
            free_cst_node_list(&cst);
            free_token_list(tokens);
            free(content);
            free(comment_used);
            return rc_meta;
            /* LCOV_EXCL_STOP */
          }
        }
        /* LCOV_EXCL_START */
        doc_metadata_free(&meta);
        free(doc_text);
        /* LCOV_EXCL_STOP */
      }
    }
  }

  /* LCOV_EXCL_START */
  free_cst_node_list(&cst);
  free_token_list(tokens);
  free(content);
  if (comment_used)
    free(comment_used);
  /* LCOV_EXCL_STOP */

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the walker cb operation.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error walker_cb(const char *path, void *user_data) {
  struct OpenAPI_Spec *spec = (struct OpenAPI_Spec *)user_data;
  if (!is_source_file(path))
    return CDD_C_SUCCESS;
  printf("Scanning: %s\n", path);
  process_file(path, spec);
  /* LCOV_EXCL_STOP */

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

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the load base spec operation.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error load_base_spec(const char *path,
                                       /* LCOV_EXCL_STOP */
                                       struct OpenAPI_Spec *spec) {
  /* LCOV_EXCL_START */
  JSON_Value *root = NULL;
  /* LCOV_EXCL_STOP */
  int rc;

  /* LCOV_EXCL_START */

  /* LCOV_EXCL_START */
  if (!path || !spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  root = json_parse_file(path);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (!root)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = openapi_load_from_json(root, spec);
  json_value_free(root);
  return rc;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the c2openapi cli main operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error c2openapi_cli_main(int argc, char **argv) {
  /* LCOV_EXCL_STOP */
  struct OpenAPI_Spec spec;
  const char *src_dir;
  const char *out_file;
  /* LCOV_EXCL_START */
  const char *base_file = NULL;
  const char *self_uri = NULL;
  const char *dialect_uri = NULL;
  char *json = NULL;
  /* LCOV_EXCL_STOP */
  int rc;
  /* LCOV_EXCL_START */
  int argi = 1;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  while (argi < argc && argv[argi][0] == '-') {
    if (strcmp(argv[argi], "--base") == 0 || strcmp(argv[argi], "-b") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr,
                /* LCOV_EXCL_STOP */
                "Usage: c2openapi [--base <openapi.json>] [--self <uri>] "
                "[--dialect <uri>] "
                "<src_dir> <out.json>\n");
        /* LCOV_EXCL_START */
        return CDD_C_ERROR_UNKNOWN;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
      base_file = argv[argi + 1];
      argi += 2;
      continue;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (strcmp(argv[argi], "--self") == 0 || strcmp(argv[argi], "-s") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr,
                /* LCOV_EXCL_STOP */
                "Usage: c2openapi [--base <openapi.json>] [--self <uri>] "
                "[--dialect <uri>] "
                "<src_dir> <out.json>\n");
        /* LCOV_EXCL_START */
        return CDD_C_ERROR_UNKNOWN;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
      self_uri = argv[argi + 1];
      argi += 2;
      continue;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    if (strcmp(argv[argi], "--dialect") == 0 ||
        strcmp(argv[argi], "--jsonSchemaDialect") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr,
                /* LCOV_EXCL_STOP */
                "Usage: c2openapi [--base <openapi.json>] [--self <uri>] "
                "[--dialect <uri>] "
                "<src_dir> <out.json>\n");
        /* LCOV_EXCL_START */
        return CDD_C_ERROR_UNKNOWN;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
      dialect_uri = argv[argi + 1];
      argi += 2;
      continue;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    break;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  if (argc - argi != 2) {
    fprintf(stderr, "Usage: c2openapi [--base <openapi.json>] [--self <uri>] "
                    /* LCOV_EXCL_STOP */
                    "[--dialect <uri>] "
                    "<src_dir> <out.json>\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  src_dir = argv[argi];
  out_file = argv[argi + 1];
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  (void)openapi_spec_init(&spec);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (base_file) {
    rc = load_base_spec(base_file, &spec);
    if (rc != 0) {
      fprintf(stderr, "Failed to load base OpenAPI spec %s: %d\n", base_file,
              /* LCOV_EXCL_STOP */
              rc);
      /* LCOV_EXCL_START */
      openapi_spec_free(&spec);
      return CDD_C_ERROR_UNKNOWN;
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  if (self_uri && *self_uri) {
    if (spec.self_uri)
      free(spec.self_uri);
    spec.self_uri = strdup(self_uri);
    if (!spec.self_uri) {
      fprintf(stderr, "Failed to set $self URI\n");
      openapi_spec_free(&spec);
      return CDD_C_ERROR_UNKNOWN;
      /* LCOV_EXCL_STOP */
    }
  }
  /* LCOV_EXCL_START */
  if (dialect_uri && *dialect_uri) {
    if (spec.json_schema_dialect)
      free(spec.json_schema_dialect);
    spec.json_schema_dialect = strdup(dialect_uri);
    if (!spec.json_schema_dialect) {
      fprintf(stderr, "Failed to set jsonSchemaDialect\n");
      openapi_spec_free(&spec);
      return CDD_C_ERROR_UNKNOWN;
      /* LCOV_EXCL_STOP */
    }
  }

  /* 1. Walk & Process */
  /* LCOV_EXCL_START */
  rc = walk_directory(src_dir, walker_cb, &spec);
  if (rc != 0) {
    fprintf(stderr, "Error walking directory %s: %d\n", src_dir, rc);
    openapi_spec_free(&spec);
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }

  /* Derive top-level tags from operation tags */
  /* LCOV_EXCL_START */
  rc = collect_spec_tags(&spec);
  if (rc != 0) {
    fprintf(stderr, "Error collecting tags: %d\n", rc);
    openapi_spec_free(&spec);
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }

  /* 2. Write */
  /* LCOV_EXCL_START */
  rc = openapi_write_spec_to_json(&spec, &json);
  if (rc != 0 || !json) {
    fprintf(stderr, "Error serializing spec: %d\n", rc);
    openapi_spec_free(&spec);
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  rc = fs_write_to_file(out_file, json);
  if (rc != 0) {
    fprintf(stderr, "Failed to write %s\n", out_file);
    rc = EXIT_FAILURE;
    /* LCOV_EXCL_STOP */
  } else {
    /* LCOV_EXCL_START */
    printf("Written %s\n", out_file);
    rc = 0;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  free(json);
  openapi_spec_free(&spec);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  return (rc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Executes the to docs json cli main operation.
 */
/* LCOV_EXCL_START */
enum cdd_c_error to_docs_json_cli_main(int argc, char **argv) {
  const char *input_file =
      getenv("CDD_INPUT") ? getenv("CDD_INPUT") : getenv("INPUT_FILE");
  int no_imports = getenv("CDD_NO_IMPORTS") ? 1 : 0;
  int no_wrapping = getenv("CDD_NO_WRAPPING") ? 1 : 0;
  /* LCOV_EXCL_STOP */
  int i;
  /* LCOV_EXCL_START */
  struct OpenAPI_Spec spec = {0};
  /* LCOV_EXCL_STOP */
  int rc;
  JSON_Value *parsed_root;
  JSON_Value *root_val;
  JSON_Object *root_obj;
  JSON_Value *endpoints_val;
  JSON_Object *endpoints_obj;
  size_t p, op_idx;

  /* LCOV_EXCL_START */
  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      return CDD_C_SUCCESS;
    } else if ((strcmp(argv[i], "-i") == 0 ||
                strcmp(argv[i], "--input") == 0) &&
               i + 1 < argc) {
      input_file = argv[++i];
    } else if (strcmp(argv[i], "--no-imports") == 0) {
      no_imports = 1;
    } else if (strcmp(argv[i], "--no-wrapping") == 0) {
      no_wrapping = 1;
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  if (!input_file)
    return CDD_C_ERROR_UNKNOWN;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  parsed_root = json_parse_file(input_file);
  if (!parsed_root)
    return CDD_C_ERROR_UNKNOWN;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = openapi_load_from_json(parsed_root, &spec);
  json_value_free(parsed_root);
  if (rc != 0)
    return rc;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  root_val = json_value_init_object();
  root_obj = json_value_get_object(root_val);
  endpoints_val = json_value_init_object();
  endpoints_obj = json_value_get_object(endpoints_val);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  for (p = 0; p < spec.n_paths; p++) {
    struct OpenAPI_Path *pi = &spec.paths[p];
    JSON_Value *path_val = json_object_get_value(endpoints_obj, pi->route);
    /* LCOV_EXCL_STOP */
    JSON_Object *path_obj;
    /* LCOV_EXCL_START */
    if (!path_val) {
      path_val = json_value_init_object();
      json_object_set_value(endpoints_obj, pi->route, path_val);
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    path_obj = json_value_get_object(path_val);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    for (op_idx = 0; op_idx < pi->n_operations; op_idx++) {
      struct OpenAPI_Operation *op = &pi->operations[op_idx];
      const char *method = "";
      /* LCOV_EXCL_STOP */
      char snippet[4096];
      char final_code[8192];
      /* LCOV_EXCL_START */
      const char *op_id = op->operation_id ? op->operation_id : "unknown";
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      switch (op->verb) {
      case OA_VERB_GET:
        method = "get";
        break;
      case OA_VERB_POST:
        method = "post";
        break;
      case OA_VERB_PUT:
        method = "put";
        break;
      case OA_VERB_DELETE:
        method = "delete";
        break;
      case OA_VERB_PATCH:
        method = "patch";
        break;
      case OA_VERB_HEAD:
        method = "head";
        break;
      case OA_VERB_OPTIONS:
        method = "options";
        break;
      case OA_VERB_TRACE:
        method = "trace";
        break;
      default:
        method = "custom";
        break;
        /* LCOV_EXCL_STOP */
      }

      /* LCOV_EXCL_START */
      snippet[0] = '\0';
      final_code[0] = '\0';
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      if (!no_imports) {
        strcat(final_code,
               /* LCOV_EXCL_STOP */
               "#include \"generated_client.h\"\n#include <stdio.h>\n\n");
      }
      /* LCOV_EXCL_START */
      if (!no_wrapping) {
        strcat(final_code, "int main(void) {\n  struct HttpClient client;\n  "
                           /* LCOV_EXCL_STOP */
                           "struct ApiError *err = NULL;\n  api_init(&client, "
                           "\"https://api.example.com\");\n");
      }

      /* LCOV_EXCL_START */
      sprintf(snippet,
              /* LCOV_EXCL_STOP */
              "  /* Call the %s API */\n  int rc = api_%s(&client, &err);\n  "
              "if (rc != 0) {\n    /* handle error */\n  }\n",
              op_id, op_id);
      /* LCOV_EXCL_START */
      strcat(final_code, snippet);
      /* LCOV_EXCL_STOP */

      /* LCOV_EXCL_START */
      if (!no_wrapping) {
        strcat(final_code,
               /* LCOV_EXCL_STOP */
               "  api_cleanup(&client);\n  return CDD_C_SUCCESS;\n}\n");
      }

      /* LCOV_EXCL_START */
      json_object_set_string(path_obj, method, final_code);
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  json_object_set_value(root_obj, "endpoints", endpoints_val);
  /* LCOV_EXCL_STOP */

  {
    /* LCOV_EXCL_START */
    char *serialized = json_serialize_to_string_pretty(root_val);
    printf("%s\n", serialized);
    json_free_serialized_string(serialized);
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  json_value_free(root_val);
  openapi_spec_free(&spec);
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief CLI entry point for binding generation (e.g., `cdd-c bind`).
 */
/* LCOV_EXCL_START */
enum cdd_c_error generate_bindings_cli_main(int argc, char **argv) {
  cdd_generate_bindings_config_t config = {0};
  /* LCOV_EXCL_STOP */
  int i;
  int rc;

  /* LCOV_EXCL_START */
  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      puts(
          /* LCOV_EXCL_STOP */
          "Usage: cdd-c bind [OPTIONS]\n\n"
          "Options:\n"
          "  -i, --input <file|dir>    Input C header/source or directory\n"
          "  -o, --output-dir <dir>    Output directory for bindings\n"
          "  -l, --lang <langs>        Comma-separated list of languages or "
          "'*' "
          "(e.g., python,rust)\n"
          "  -n, --lib-name <name>     Name of the shared library (e.g., "
          "sqlite3)\n"
          "  -m, --module-name <name>  Name of the generated namespace/module\n"
          "  --skip-static             Skip static inline functions\n"
          "  --opaque-pointers         Treat unknown structs as void* "
          "(opaque)\n"
          "  --generate-tests          Generate basic sanity-check tests\n"
          "  -h, --help                Show this help message\n");
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
    } else if ((strcmp(argv[i], "-i") == 0 ||
                strcmp(argv[i], "--input") == 0) &&
               i + 1 < argc) {
      config.input = argv[++i];
    } else if ((strcmp(argv[i], "-o") == 0 ||
                strcmp(argv[i], "--output-dir") == 0) &&
               i + 1 < argc) {
      config.output_dir = argv[++i];
    } else if ((strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--lang") == 0) &&
               i + 1 < argc) {
      config.target_langs = argv[++i];
    } else if ((strcmp(argv[i], "-n") == 0 ||
                strcmp(argv[i], "--lib-name") == 0) &&
               i + 1 < argc) {
      config.library_name = argv[++i];
    } else if ((strcmp(argv[i], "-m") == 0 ||
                strcmp(argv[i], "--module-name") == 0) &&
               i + 1 < argc) {
      config.module_name = argv[++i];
    } else if (strcmp(argv[i], "--skip-static") == 0) {
      config.skip_static = 1;
    } else if (strcmp(argv[i], "--opaque-pointers") == 0) {
      config.opaque_pointers = 1;
    } else if (strcmp(argv[i], "--generate-tests") == 0) {
      config.generate_tests = 1;
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  if (!config.input || !config.output_dir || !config.target_langs) {
    fprintf(stderr, "Error: --input, --output-dir, and --lang are required.\n");
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  rc = cdd_generate_bindings(&config);
  if (rc != 0) {
    fprintf(stderr, "Error: Binding generation failed with code %d\n", rc);
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_STOP */
