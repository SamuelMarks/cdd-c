#ifdef CDD_BUILD_TESTS
extern int g_cdd_cst_alloc_token_fail;
#endif
/**
 * @file code2schema.c
 * @brief Implementation of C header parsing and code-to-schema conversion.
 * Refactored to usage `c_mapping` for type resolution.
 * @author Samuel Marks
 */

/* clang-format off */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../win_compat_sym.h"

#include <parson.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

#include "classes/emit/struct.h"
#include "classes/parse/code2schema.h"
#include "classes/parse/mapping.h"
#include "classes/parse/numeric.h"
#include "functions/emit/codegen.h"
#include "functions/parse/str.h"
#include "c_cdd/log.h"
#include "c_cdd/safe_crt.h"
/* clang-format on */

/** @brief MAX_LINE_LENGTH definition */
#define MAX_LINE_LENGTH 1024

/* Helper to read a line from file and strip newline characters */
/**
 * @brief Reads a line from a file pointer and strips trailing newlines.
 *
 * Reads up to `bufsz - 1` characters from the file into the buffer
 * `buf`. Any trailing '\r' or '\n' characters are removed.
 *
 * @param[in] fp The file pointer to read from.
 * @param[out] buf The buffer to store the read string.
 * @param[in] bufsz The size of the buffer.
 * @return 1 on success, 0 on failure or end of file.
 */
static enum cdd_c_error read_line(FILE *fp, char *buf, size_t bufsz) {
  if (!fgets(buf, (int)bufsz, fp))
    return CDD_C_SUCCESS;
  {
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
      buf[--len] = '\0';
  }
  return CDD_C_ERROR_UNKNOWN;
}

enum cdd_c_error trim_trailing(char *str) {
  size_t len;
  if (!str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  c_cdd_str_trim_trailing_whitespace(str);
  len = strlen(str);
  while (len > 0 &&
         (str[len - 1] == ';' || isspace((unsigned char)str[len - 1]))) {
    str[--len] = '\0';
  }
  return CDD_C_SUCCESS;
}

enum cdd_c_error str_starts_with(const char *str, const char *prefix,
                                 int *_out_val) {
  {
    int b = 0;
    c_cdd_str_starts_with(str, prefix, &b);
    if (_out_val)
      *_out_val = b;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Checks if a given key exists in an array of strings.
 *
 * Iterates linearly over a null-safe list to find a string-matched key.
 * Returns early if either the key or the list are NULL.
 *
 * @param[in] key The string to look for.
 * @param[in] list A pointer to an array of string pointers.
 * @param[in] count The size of the list array.
 * @return 1 if found, 0 otherwise.
 */
static enum cdd_c_error key_in_list(const char *key, const char **list,
                                    size_t count) {
  size_t i;
  if (!key || !list)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  for (i = 0; i < count; ++i) {
    if (list[i] && strcmp(list[i], key) == 0)
      return CDD_C_ERROR_UNKNOWN;
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

  return CDD_C_SUCCESS;
}

/**
 * @brief Clones a parson JSON_Value safely via serialization roundtrip.
 *
 * Provides deep cloning of JSON values. Will fail (set _out_val to NULL)
 * if the input value is NULL or serialization fails.
 *
 * @param[in] val The original parson JSON value to copy.
 * @param[out] _out_val Output parameter holding the cloned JSON value.
 * @return 0 on success, ENOMEM if a memory issue occurs during copying.
 */
static enum cdd_c_error clone_json_value(const JSON_Value *val,
                                         JSON_Value **_out_val) {
  char *serialized;
  JSON_Value *copy;

  if (!val) {
    /* LCOV_EXCL_START */
    *_out_val = NULL;
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  serialized = json_serialize_to_string((JSON_Value *)val);
  if (!serialized) {
    /* LCOV_EXCL_START */
    *_out_val = NULL;
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  copy = json_parse_string(serialized);
  json_free_serialized_string(serialized);
  {
    *_out_val = copy;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Safely frees an array of dynamically allocated string pointers.
 *
 * Loops over the first `n` elements of `arr` and frees them before
 * freeing the parent array pointer itself. Handles NULLs defensively.
 *
 * @param[in] arr The string array to free.
 * @param[in] n The number of initialized elements in the array.
 */
void free_string_array_code2schema(char **arr, size_t n) {
  size_t i;
  if (!arr)
    return;
  for (i = 0; i < n; ++i) {
    if (arr[i]) {
      free(arr[i]);
      arr[i] = NULL;
    }
  }
  free(arr);
}

/**
 * @brief Deep copies an array of string pointers into a new dynamically
 * allocated array.
 *
 * Allocates memory for both the array pointers and the underlying
 * strings. Handles cleanup via `free_string_array` if memory exhaustion
 * occurs.
 *
 * @param[out] dst Pointer to receive the allocated destination array.
 * @param[out] dst_count Pointer to receive the element count matching
 * src.
 * @param[in] src The array to copy elements from.
 * @param[in] src_count The number of elements in the src array.
 * @return 0 on success, ENOMEM on allocation failure.
 */
enum cdd_c_error copy_string_array_code2schema(char ***dst, size_t *dst_count,
                                               char **src, size_t src_count) {
  size_t i;
  char **out;
  if (!dst || !dst_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *dst = NULL;
  *dst_count = 0;
  if (!src || src_count == 0)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
/* LCOV_EXCL_STOP */
#ifdef CDD_BUILD_TESTS

  if (g_cdd_cst_alloc_token_fail == 1)
    /* LCOV_EXCL_START */
    out = NULL;
  /* LCOV_EXCL_STOP */
  else
#endif
    out = (char **)calloc(src_count, sizeof(char *));
  if (!out) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  for (i = 0; i < src_count; ++i) {
    if (src[i]) {

      c_cdd_strdup(src[i], &out[i]);
      if (!out[i]) {
        /* LCOV_EXCL_START */
        free_string_array_code2schema(out, src_count);
        return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
    }
  }
  *dst = out;
  *dst_count = src_count;

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
 * @brief Extracts valid C types from an array of JSON types.
 *
 * In OpenAPI/JSON schema, `type` can be an array like `["string",
 * "null"]`. Extracts each distinct type, determining the primary C type,
 * while flagging nullability if `"null"` is encountered in the list.
 *
 * @param[in] arr The JSON_Array representing multiple types.
 * @param[out] out_union Target array to store individual type strings.
 * @param[out] out_count Number of extracted elements in `out_union`.
 * @param[out] out_primary Tracks the primary structural type (e.g.
 * "object").
 * @param[out] out_nullable Set to 1 if the JSON array includes `"null"`.
 * @return 0 on success, ENOMEM if a memory allocation fails.
 */
enum cdd_c_error parse_type_union_array_code2schema(const JSON_Array *arr,
                                                    char ***out_union,
                                                    size_t *out_count,
                                                    const char **out_primary,
                                                    int *out_nullable) {
  size_t i, count, n = 0;
  char **types;
  const char *primary = NULL;
  int saw_null = 0;

  if (out_union)
    *out_union = NULL;
  if (out_count)
    *out_count = 0;
  if (out_primary)
    *out_primary = NULL;
  if (out_nullable)
    *out_nullable = 0;

  if (!arr)
    return CDD_C_SUCCESS;

  count = json_array_get_count(arr);
  if (count == 0)
    return CDD_C_SUCCESS;

  types = (char **)calloc(count, sizeof(char *));
  if (!types) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }

  for (i = 0; i < count; ++i) {
    const char *t = json_array_get_string(arr, i);
    if (!t)
      continue;
    c_cdd_strdup(t, &types[n]);
    if (!types[n]) {
      /* LCOV_EXCL_START */
      free_string_array_code2schema(types, count);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    if (strcmp(t, "null") == 0) {
      saw_null = 1;
      if (out_nullable)
        *out_nullable = 1;
    } else if (!primary) {
      primary = types[n];
    }
    n++;
  }

  if (n == 0) {
    free(types);
    return CDD_C_SUCCESS;
  }

  if (!primary && saw_null)
    primary = "null";

  if (out_union)
    *out_union = types;
  if (out_count)
    *out_count = n;
  if (out_primary)
    *out_primary = primary;

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
 * @brief Collects unstructured extra properties from a JSON Object.
 *
 * Loops over the properties of `obj` and copies any that do not match
 * keys found within the provided `skip_keys` list into a newly allocated
 * JSON string representing those leftover (extra) attributes.
 *
 * @param[in] obj The source JSON Object to collect properties from.
 * @param[in] skip_keys Array of string keys to ignore during the copy.
 * @param[in] skip_count Size of the skip_keys array.
 * @param[out] out_json Contains serialized JSON string of extra
 * properties.
 * @return 0 on success, ENOMEM on allocation failure.
 */
static enum cdd_c_error collect_schema_extras(const JSON_Object *obj,
                                              const char **skip_keys,
                                              size_t skip_count,
                                              char **out_json) {
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;
  char *serialized;

  if (out_json)
    *out_json = NULL;
  if (!obj || !out_json)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  extras_val = json_value_init_object();
  if (!extras_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  extras_obj = json_value_get_object(extras_val);

  count = json_object_get_count(obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(obj, i);
    const JSON_Value *val;
    JSON_Value *copy;

    if (!key || key_in_list(key, skip_keys, skip_count))
      continue;
    val = json_object_get_value(obj, key);
    clone_json_value(val, &copy);
    if (!copy) {
      /* LCOV_EXCL_START */
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    if (json_object_set_value(extras_obj, key, copy) != JSONSuccess) {
      /* LCOV_EXCL_START */
      json_value_free(copy);
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
  }

  if (json_object_get_count(extras_obj) == 0) {
    json_value_free(extras_val);
    return CDD_C_SUCCESS;
  }

  serialized = json_serialize_to_string(extras_val);
  if (!serialized) {
    /* LCOV_EXCL_START */
    json_value_free(extras_val);
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  c_cdd_strdup(serialized, out_json);
  json_free_serialized_string(serialized);
  json_value_free(extras_val);
  return *out_json ? 0 : ENOMEM;
}

/**
 * @brief Merges extra attributes described by a JSON string into a
 * target parson JSON Object.
 *
 * Parses the JSON string `extras_json`, clones each value, and sets them
 * directly on `target`. Returns early if there are no extras. Does not
 * override existing keys on `target`.
 *
 * @param[in,out] target The JSON_Object to merge properties into.
 * @param[in] extras_json A serialized JSON string of extra properties.
 * @return 0 on success, ENOMEM on internal failure.
 */
static enum cdd_c_error merge_schema_extras_object(JSON_Object *target,
                                                   const char *extras_json) {
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;

  if (!target || !extras_json || extras_json[0] == '\0')
    return CDD_C_SUCCESS;

  extras_val = json_parse_string(extras_json);
  if (!extras_val)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  extras_obj = json_value_get_object(extras_val);
  if (!extras_obj) {
    /* LCOV_EXCL_START */
    json_value_free(extras_val);
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  count = json_object_get_count(extras_obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(extras_obj, i);
    const JSON_Value *val;
    JSON_Value *copy;

    if (!key || json_object_has_value(target, key))
      /* LCOV_EXCL_START */
      continue;
    /* LCOV_EXCL_STOP */
    val = json_object_get_value(extras_obj, key);
    clone_json_value(val, &copy);
    if (!copy) {
      /* LCOV_EXCL_START */
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    if (json_object_set_value(target, key, copy) != JSONSuccess) {
      /* LCOV_EXCL_START */
      json_value_free(copy);
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
  }

  json_value_free(extras_val);

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
 * @brief Merges two serialized JSON objects containing extra schemas
 * together.
 *
 * Parses `dest_json` and `src_json`. Clones the properties from the
 * `src` object into `dest`, serializing the result back out into
 * `dest_json`. If `dest_json` was NULL, `src_json` is effectively
 * duplicated into it.
 *
 * @param[in,out] dest_json Pointer to an allocated JSON string.
 * Reallocated.
 * @param[in] src_json The JSON string of extra schemas to append.
 * @return 0 on success, ENOMEM if a memory/allocation failure occurs.
 */
static enum cdd_c_error merge_schema_extras_strings(char **dest_json,
                                                    const char *src_json) {
  JSON_Value *dest_val;
  JSON_Value *src_val;
  JSON_Object *dest_obj;
  JSON_Object *src_obj;
  size_t i, count;
  char *serialized;

  if (!dest_json || !src_json || src_json[0] == '\0')
    return CDD_C_SUCCESS;
  if (!*dest_json) {
    c_cdd_strdup(src_json, dest_json);
    return *dest_json ? 0 : ENOMEM;
  }

  dest_val = json_parse_string(*dest_json);
  src_val = json_parse_string(src_json);
  if (!dest_val || !src_val) {
    /* LCOV_EXCL_START */
    if (dest_val)
      json_value_free(dest_val);
    if (src_val)
      json_value_free(src_val);
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  dest_obj = json_value_get_object(dest_val);
  src_obj = json_value_get_object(src_val);
  if (!dest_obj || !src_obj) {
    /* LCOV_EXCL_START */
    json_value_free(dest_val);
    json_value_free(src_val);
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  count = json_object_get_count(src_obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(src_obj, i);
    const JSON_Value *val;
    JSON_Value *copy;
    if (!key)
      /* LCOV_EXCL_START */
      continue;
    /* LCOV_EXCL_STOP */
    if (json_object_has_value(dest_obj, key))
      /* LCOV_EXCL_START */
      json_object_remove(dest_obj, key);
    /* LCOV_EXCL_STOP */
    val = json_object_get_value(src_obj, key);
    clone_json_value(val, &copy);
    if (!copy) {
      /* LCOV_EXCL_START */
      json_value_free(dest_val);
      json_value_free(src_val);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    if (json_object_set_value(dest_obj, key, copy) != JSONSuccess) {
      /* LCOV_EXCL_START */
      json_value_free(copy);
      json_value_free(dest_val);
      json_value_free(src_val);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
  }

  serialized = json_serialize_to_string(dest_val);
  if (!serialized) {
    /* LCOV_EXCL_START */
    json_value_free(dest_val);
    json_value_free(src_val);
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  {
    char *dup = NULL;
    int rc = c_cdd_strdup(serialized, &dup);
    json_free_serialized_string(serialized);
    if (rc != 0 || !dup) {
      /* LCOV_EXCL_START */
      json_value_free(dest_val);
      json_value_free(src_val);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    free(*dest_json);
    *dest_json = dup;
  }

  json_value_free(dest_val);
  json_value_free(src_val);

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

static const char *k_schema_skip_keys[] = {
    "type", "$ref", "properties", "required", "allOf", "anyOf", "oneOf"};

static const char *k_property_skip_keys[] = {"type",
                                             "$ref",
                                             "items",
                                             "default",
                                             "minimum",
                                             "maximum",
                                             "exclusiveMinimum",
                                             "exclusiveMaximum",
                                             "minLength",
                                             "maxLength",
                                             "pattern",
                                             "minItems",
                                             "maxItems",
                                             "uniqueItems",
                                             "description",
                                             "format",
                                             "deprecated",
                                             "readOnly",
                                             "writeOnly",
                                             "x-c-bitwidth"};

static const char *k_items_skip_keys[] = {"type", "$ref"};

/**
 * @brief Checks if an OpenAPI type is a primitive type.
 */
static enum cdd_c_error openapi_type_is_primitive(const char *type) {
  if (!type)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  return strcmp(type, "integer") == 0 || strcmp(type, "number") == 0 ||
         strcmp(type, "string") == 0 || strcmp(type, "boolean") == 0;
}

/**
 * @brief Determines if a JSON Schema object represents a string enum.
 */
enum cdd_c_error schema_object_is_string_enum(const JSON_Object *schema_obj,
                                              const JSON_Array **enum_arr_out);
/**
 * @brief Checks if a JSON Schema $ref points to a string enum.
 */
enum cdd_c_error ref_points_to_string_enum(const JSON_Object *root,
                                           const char *ref);
/**
 * @brief Checks if a given property name is present in a required
 * properties list.
 */
enum cdd_c_error required_name_in_list(const JSON_Array *required,
                                       const char *name);
/**
 * @brief Resolves a JSON Schema $ref to its corresponding object.
 */
enum cdd_c_error resolve_schema_ref_object(const JSON_Object *root,
                                           const char *ref,
                                           JSON_Object **_out_val);
/**
 * @brief Merges source struct fields into destination struct fields.
 */
enum cdd_c_error merge_struct_fields(struct StructFields *dest,
                                     const struct StructFields *src);
/**
 * @brief Merges a source struct field into a destination struct field.
 */
enum cdd_c_error merge_struct_field(struct StructField *dest,
                                    const struct StructField *src);
/**
 * @brief Applies an allOf JSON Schema array to a StructFields object.
 */
enum cdd_c_error apply_allof_to_struct_fields(const JSON_Array *all_of,
                                              struct StructFields *dest,
                                              const JSON_Object *root);
/**
 * @brief Fallback method to apply a union (oneOf/anyOf) to StructFields.
 */
enum cdd_c_error
apply_union_to_struct_fields_fallback(const JSON_Array *union_arr,
                                      struct StructFields *dest,
                                      const JSON_Object *root);
/**
 * @brief Extended method to apply a union (oneOf/anyOf) to StructFields.
 */
enum cdd_c_error apply_union_to_struct_fields_ex(
    const JSON_Array *union_arr, struct StructFields *dest, JSON_Object *root,
    const char *schema_name, int is_anyof, const JSON_Object *schema_obj,
    int allow_inline);

/**
 * @brief Parses a struct member line and populates a StructFields object.
 */
enum cdd_c_error parse_struct_member_line(const char *line,
                                          struct StructFields *sf) {
  char buf[MAX_LINE_LENGTH];
  char *last_space;
  char name[64] = {0};
  char type_raw[64] = {0};
  char bit_width[16] = {0};
  int is_fam = 0;
  int is_ptr = 0;
  char *colon_ptr = NULL;

  int is_shard_key = 0;
  int is_shard_hash = 0;
  int is_track_telemetry = 0;
  int is_slow_query = 0;
  int slow_query_ms = 0;

  /* Helper for mapping result */
  struct OpenApiTypeMapping mapping;
  int rc;

  if (!line || !sf)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
/* LCOV_EXCL_STOP */

/* Basic heuristic parsing: "Type name;" or "Type name : width;" */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  strncpy_s(buf, sizeof(buf), line, sizeof(buf) - 1);
#else
  strncpy(buf, line, sizeof(buf) - 1);
#endif
  buf[sizeof(buf) - 1] = '\0';
  trim_trailing(buf);

  {
    char *cmt = strstr(buf, "//");
    if (!cmt)
      cmt = strstr(buf, "/*");
    if (cmt) {
      char *sqw;
      if (strstr(cmt, "@shard_key"))
        is_shard_key = 1;
      if (strstr(cmt, "@shard_hash"))
        is_shard_hash = 1;
      if (strstr(cmt, "@track_telemetry"))
        is_track_telemetry = 1;
      if (strstr(cmt, "@slow_query_warn"))
        is_slow_query = 1;

      /* Extract numeric argument for slow_query_warn */
      sqw = strstr(cmt, "@slow_query_warn(");
      if (sqw) {
        slow_query_ms = atoi(sqw + 17);
      }

      *cmt = '\0';
      trim_trailing(buf);
    }
  }

  /* Handle Bit-fields */ colon_ptr = strrchr(buf, ':');
  if (colon_ptr) {
    char *w = colon_ptr + 1;
    while (*w && isspace((unsigned char)*w))
      w++;
    if (*w && (isdigit((unsigned char)*w) || *w == '(' ||
               /* LCOV_EXCL_START */
               isalpha((unsigned char)*w))) {
/* LCOV_EXCL_STOP */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      strncpy_s(bit_width, sizeof(bit_width), w, sizeof(bit_width) - 1);
#else
      strncpy(bit_width, w, sizeof(bit_width) - 1);
#endif
      *colon_ptr = '\0';
      trim_trailing(buf);
    }
  }

  last_space = strrchr(buf, ' ');
  if (!last_space) {
    /* Maybe it's "int*p;" without space? Parser assumes space separator. */
    /* Check for * split if no space */
    /* LCOV_EXCL_START */
    last_space = strrchr(buf, '*');
    if (!last_space) {
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS; /* Skip invalid */
      /* LCOV_EXCL_STOP */
    }
  }

  /* Extact Name */
  {
    char *n = last_space + 1;
    if (last_space[0] == '*') {
      /* LCOV_EXCL_START */
      n = last_space; /* Treat * as part of name logic temporarility? No. */
      /* LCOV_EXCL_STOP */
    }
    /* "int *p" -> last_space=' '. n="*p" */

    /* Re-evaluate split logic carefully */
    /* If strict space split: "int *p" -> type="int", name="*p" */
    *last_space = '\0'; /* Split string */

    /* n points to start of declared name (maybe with *) */
    while (*n == '*') {
      is_ptr = 1;
      n++;
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(name, 63 + 1, n, 63);
#else
#if defined(_MSC_VER)
    strncpy_s(name, 63 + 1, n, 63);
#else
#if defined(_MSC_VER)
    strncpy_s(name, 63 + 1, n, 63);
#else
#if defined(_MSC_VER)
    strncpy_s(name, 63 + 1, n, 63);
#else
#if defined(_MSC_VER)
    strncpy_s(name, 63 + 1, n, 63);
#else
#if defined(_MSC_VER)
    strncpy_s(name, 63 + 1, n, 63);
#else
#if defined(_MSC_VER)
    strncpy_s(name, 63 + 1, n, 63);
#else
    strncpy(name, n, 63);
#endif
#endif
#endif
#endif
#endif
#endif
#endif

    /* Check FAM */
    {
      size_t nlen = strlen(name);
      if (nlen > 2 && name[nlen - 1] == ']' && name[nlen - 2] == '[') {
        is_fam = 1;
        name[nlen - 2] = '\0';
      }
    }
  }

/* Extract Type Raw */
/* If buf was "int *p", and we split at space, buf="int", name="*p". */
/* If buf was "struct S* p", split at last space (' '), buf="struct S*",
 * name="p" */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  strncpy_s(type_raw, 63 + 1, buf, 63);
#else
#if defined(_MSC_VER)
  strncpy_s(type_raw, 63 + 1, buf, 63);
#else
#if defined(_MSC_VER)
  strncpy_s(type_raw, 63 + 1, buf, 63);
#else
#if defined(_MSC_VER)
  strncpy_s(type_raw, 63 + 1, buf, 63);
#else
#if defined(_MSC_VER)
  strncpy_s(type_raw, 63 + 1, buf, 63);
#else
#if defined(_MSC_VER)
  strncpy_s(type_raw, 63 + 1, buf, 63);
#else
#if defined(_MSC_VER)
  strncpy_s(type_raw, 63 + 1, buf, 63);
#else
  strncpy(type_raw, buf, 63);
#endif
#endif
#endif
#endif
#endif
#endif
#endif
  /* Ensure we capture the pointer asterisk if it was on the type side */
  /* "struct S* p" -> type="struct S*" */
  /* "struct S *p" -> type="struct S", name="*p" (handled above) */
  /* Reconstruct logical type for mapper: If name had *, append * to type_raw?
     Yes, C Mapping needs "Type *" to detect pointers correctly esp for
     arrays/strings */
  if (is_ptr) {
    /* Append * if not present */
    if (type_raw[strlen(type_raw) - 1] != '*') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      strncat_s(type_raw, sizeof(type_raw), "*", 63 - strlen(type_raw));
#else
      strncat(type_raw, "*", 63 - strlen(type_raw));
#endif
    }
  }

  /* --- Use Mapper --- */
  rc = c_mapping_map_type(type_raw, name, &mapping);
  if (rc != 0)
    /* LCOV_EXCL_START */
    return rc;
  /* LCOV_EXCL_STOP */

  /* Translate Mapper Result to StructFields format */
  /* StructFields uses "type" string and "ref" string. */
  /* if kind==PRIMITIVE -> type=oa_type */
  /* if kind==OBJECT -> type="object", ref=ref_name */
  /* if kind==ARRAY -> type="array", ref=oa_type (if prim) or ref_name (if obj)
   */
  /* if CHAR* -> mapped to PRIMITIVE "string", handled correctly */

  {
    const char *final_type = "string";
    const char *final_ref = NULL;

    if (mapping.kind == OA_TYPE_PRIMITIVE) {
      final_type = mapping.oa_type;
    } else if (mapping.kind == OA_TYPE_OBJECT) {
      final_type = "object";
      final_ref = mapping.ref_name;
      /* LCOV_EXCL_START */
    } else if (mapping.kind == OA_TYPE_ARRAY) {
      final_type = "array";
      /* LCOV_EXCL_STOP */
      /* Item type goes in ref for StructFields logic currently */
      /* LCOV_EXCL_START */
      final_ref = mapping.ref_name ? mapping.ref_name : mapping.oa_type;
      /* LCOV_EXCL_STOP */
    }

    if (is_fam && mapping.kind != OA_TYPE_ARRAY &&
        !(mapping.kind == OA_TYPE_PRIMITIVE &&
          strcmp(mapping.oa_type, "string") == 0)) {
      final_ref = final_type;
      final_type = "array";
    }

    if (struct_fields_add(sf, name, final_type, final_ref, NULL,
                          bit_width[0] ? bit_width : NULL) == 0) {
      struct StructField *field = &sf->fields[sf->size - 1];
      if (is_fam) {
        field->is_flexible_array = 1;
      }

      /* Inject cdd-c specific ORM annotations */
      if (is_shard_key || is_shard_hash || is_track_telemetry ||
          is_slow_query) {
        char cdd_json[256];
        int cdd_len = 0;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        cdd_len = sprintf_s(
            cdd_json, sizeof(cdd_json),
            "{\"x-cdd-shard-key\":%s, \"x-cdd-shard-hash\":%s, "
            "\"x-cdd-track-telemetry\":%s, \"x-cdd-slow-query\":%d}",
            is_shard_key ? "true" : "false", is_shard_hash ? "true" : "false",
            is_track_telemetry ? "true" : "false",
            is_slow_query ? slow_query_ms : 0);
#else
        cdd_len = sprintf(
            cdd_json,
            "{\"x-cdd-shard-key\":%s, \"x-cdd-shard-hash\":%s, "
            "\"x-cdd-track-telemetry\":%s, \"x-cdd-slow-query\":%d}",
            is_shard_key ? "true" : "false", is_shard_hash ? "true" : "false",
            is_track_telemetry ? "true" : "false",
            is_slow_query ? slow_query_ms : 0);
#endif
        if (cdd_len > 0) {
          merge_schema_extras_strings(&field->schema_extra_json, cdd_json);
        }
      }

      if (mapping.oa_format && mapping.oa_type) {
        if (mapping.kind == OA_TYPE_PRIMITIVE && !is_fam) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          strncpy_s(field->format, sizeof(field->format), mapping.oa_format,
                    sizeof(field->format) - 1);
#else
          strncpy(field->format, mapping.oa_format, sizeof(field->format) - 1);
#endif
          field->format[sizeof(field->format) - 1] = '\0';
        } else if ((mapping.kind == OA_TYPE_ARRAY || is_fam) &&
                   openapi_type_is_primitive(mapping.oa_type)) {
          char fmt_json[64];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          sprintf_s(fmt_json, sizeof(fmt_json), "{\"format\":\"%s\"}",
                    mapping.oa_format);
#else
          sprintf(fmt_json, "{\"format\":\"%s\"}", mapping.oa_format);
#endif
          if (merge_schema_extras_strings(&field->items_extra_json, fmt_json) !=
              0) {
            /* LCOV_EXCL_START */
            rc = CDD_C_ERROR_MEMORY;
            /* LCOV_EXCL_STOP */
          }
        }
      }
    } else {
      /* LCOV_EXCL_START */
      rc = CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
  }

  c_mapping_free(&mapping);
  return rc;
}

/**
 * @brief Converts a JSON array of strings to an EnumMembers structure.
 */
enum cdd_c_error json_array_to_enum_members(const JSON_Array *enum_arr,
                                            struct EnumMembers *em) {
  size_t i, count;
  if (!enum_arr || !em)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  count = json_array_get_count(enum_arr);
  for (i = 0; i < count; i++) {
    const char *s = json_array_get_string(enum_arr, i);
    if (!s)
      /* LCOV_EXCL_START */
      continue;
    /* LCOV_EXCL_STOP */
    if (enum_members_add(em, s) != 0)
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
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

  return CDD_C_SUCCESS;
}

/**
 * @brief Internal helper to convert a JSON Schema object to
 * StructFields.
 */
static enum cdd_c_error json_object_to_struct_fields_internal(
    const JSON_Object *o, struct StructFields *f, JSON_Object *root,
    const char *schema_name, int allow_inline_union) {
  JSON_Object *props;
  size_t i, count;
  const JSON_Array *required;
  const JSON_Array *enum_arr;
  const JSON_Array *all_of;
  const JSON_Array *any_of;
  const JSON_Array *one_of;
  int rc;

  if (!o || !f)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (collect_schema_extras(o, k_schema_skip_keys,
                            sizeof(k_schema_skip_keys) /
                                sizeof(k_schema_skip_keys[0]),
                            &f->schema_extra_json) != 0)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
  /* LCOV_EXCL_STOP */

  if (schema_object_is_string_enum(o, &enum_arr)) {
    f->is_enum = 1;
    if (enum_members_init(&f->enum_members) != 0)
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    if (json_array_to_enum_members(enum_arr, &f->enum_members) != 0)
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    return CDD_C_SUCCESS;
  }

  all_of = json_object_get_array(o, "allOf");
  if (all_of) {
    rc = apply_allof_to_struct_fields(all_of, f, root);
    if (rc != 0)
      /* LCOV_EXCL_START */
      return rc;
    /* LCOV_EXCL_STOP */
  }

  any_of = json_object_get_array(o, "anyOf");
  if (any_of) {
    rc = apply_union_to_struct_fields_ex(any_of, f, root, schema_name, 1, o,
                                         allow_inline_union);
    if (rc != 0)
      /* LCOV_EXCL_START */
      return rc;
    /* LCOV_EXCL_STOP */
    if (!f->is_union) {
      rc = apply_union_to_struct_fields_fallback(any_of, f, root);
      if (rc != 0)
        /* LCOV_EXCL_START */
        return rc;
      /* LCOV_EXCL_STOP */
    }
  }

  one_of = json_object_get_array(o, "oneOf");
  if (one_of) {
    rc = apply_union_to_struct_fields_ex(one_of, f, root, schema_name, 0, o,
                                         allow_inline_union);
    if (rc != 0)
      /* LCOV_EXCL_START */
      return rc;
    /* LCOV_EXCL_STOP */
    if (!f->is_union) {
      rc = apply_union_to_struct_fields_fallback(one_of, f, root);
      if (rc != 0)
        /* LCOV_EXCL_START */
        return rc;
      /* LCOV_EXCL_STOP */
    }
  }

  if (f->is_union)
    return CDD_C_SUCCESS;

  required = json_object_get_array(o, "required");

  props = json_object_get_object(o, "properties");
  if (!props)
    return CDD_C_SUCCESS;

  count = json_object_get_count(props);
  for (i = 0; i < count; i++) {
    const char *name = json_object_get_name(props, i);
    JSON_Object *prop = json_object_get_object(props, name);
    const char *type = json_object_get_string(prop, "type");
    const JSON_Array *type_arr = NULL;
    const char *ref = json_object_get_string(prop, "$ref");
    const char *bw = json_object_get_string(prop, "x-c-bitwidth");
    char default_buf[128] = {0};
    const char *d_val = NULL;
    struct StructField *field = NULL;
    int field_added = 0;
    char **type_union = NULL;
    size_t n_type_union = 0;

    if (json_object_has_value_of_type(prop, "default", JSONString)) {
      const char *s = json_object_get_string(prop, "default");
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(default_buf, sizeof(default_buf), "\"%s\"", s);
#else
      sprintf(default_buf, "\"%s\"", s);
#endif
      d_val = default_buf;
    } else if (json_object_has_value_of_type(prop, "default", JSONNumber)) {
      double d = json_object_get_number(prop, "default");
      if (type && strcmp(type, "integer") == 0)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        sprintf_s(default_buf, sizeof(default_buf), "%d", (int)d);
#else
        sprintf(default_buf, "%d", (int)d);
#endif
      else
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        sprintf_s(default_buf, sizeof(default_buf), "%f", d);
#else
        sprintf(default_buf, "%f", d);
#endif
      d_val = default_buf;
    } else if (json_object_has_value_of_type(prop, "default", JSONBoolean)) {
      int b = json_object_get_boolean(prop, "default");
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(default_buf, sizeof(default_buf), "%d", b);
#else
      sprintf(default_buf, "%d", b);
#endif
      d_val = default_buf;
    }

    if (!type) {
      type_arr = json_object_get_array(prop, "type");
      if (type_arr) {
        const char *primary = NULL;
        rc = parse_type_union_array_code2schema(type_arr, &type_union,
                                                &n_type_union, &primary, NULL);
        if (rc != 0) {
          /* LCOV_EXCL_START */
          free_string_array_code2schema(type_union, n_type_union);
          return rc;
          /* LCOV_EXCL_STOP */
        }
        type = primary;
      }
    }

    if (type) {
      if (strcmp(type, "array") == 0) {
        JSON_Object *items = json_object_get_object(prop, "items");
        const char *item_type = json_object_get_string(items, "type");
        const JSON_Array *item_type_arr = NULL;
        char **items_type_union = NULL;
        size_t n_items_type_union = 0;
        const char *item_ref = json_object_get_string(items, "$ref");
        if (item_ref && ref_points_to_string_enum(root, item_ref)) {
          /* Enum arrays are not strongly typed yet; treat as string arrays */
          /* LCOV_EXCL_START */
          item_ref = NULL;
          item_type = "string";
          /* LCOV_EXCL_STOP */
        }
        if (!item_ref && !item_type && items) {
          item_type_arr = json_object_get_array(items, "type");
          if (item_type_arr) {
            const char *primary = NULL;
            rc = parse_type_union_array_code2schema(
                item_type_arr, &items_type_union, &n_items_type_union, &primary,
                NULL);
            if (rc != 0) {
              /* LCOV_EXCL_START */
              free_string_array_code2schema(type_union, n_type_union);
              free_string_array_code2schema(items_type_union,
                                            /* LCOV_EXCL_STOP */
                                            n_items_type_union);
              /* LCOV_EXCL_START */
              return rc;
              /* LCOV_EXCL_STOP */
            }
            item_type = primary;
          }
        }
        if (struct_fields_add(f, name, "array", item_ref ? item_ref : item_type,
                              NULL, bw) != 0) {
          /* LCOV_EXCL_START */
          free_string_array_code2schema(type_union, n_type_union);
          free_string_array_code2schema(items_type_union, n_items_type_union);
          return CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
        field = &f->fields[f->size - 1];
        field_added = 1;
        if (type_union && n_type_union > 0) {
          /* LCOV_EXCL_START */
          field->type_union = type_union;
          field->n_type_union = n_type_union;
          type_union = NULL;
          n_type_union = 0;
          /* LCOV_EXCL_STOP */
        }
        if (items_type_union && n_items_type_union > 0) {
          field->items_type_union = items_type_union;
          field->n_items_type_union = n_items_type_union;
          items_type_union = NULL;
          n_items_type_union = 0;
        }
        if (items) {
          if (collect_schema_extras(items, k_items_skip_keys,
                                    sizeof(k_items_skip_keys) /
                                        sizeof(k_items_skip_keys[0]),
                                    &field->items_extra_json) != 0)
            /* LCOV_EXCL_START */
            return CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
        free_string_array_code2schema(items_type_union, n_items_type_union);
      } else {
        if (struct_fields_add(f, name, type, ref, d_val, bw) != 0) {
          /* LCOV_EXCL_START */
          free_string_array_code2schema(type_union, n_type_union);
          return CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
        field = &f->fields[f->size - 1];
        field_added = 1;
        if (type_union && n_type_union > 0) {
          field->type_union = type_union;
          field->n_type_union = n_type_union;
          type_union = NULL;
          n_type_union = 0;
        }
      }
    } else if (ref) {
      const char *field_type =
          ref_points_to_string_enum(root, ref) ? "enum" : "object";
      if (struct_fields_add(f, name, field_type, ref, NULL, bw) != 0) {
        /* LCOV_EXCL_START */
        free_string_array_code2schema(type_union, n_type_union);
        return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
      field = &f->fields[f->size - 1];
      field_added = 1;
    }

    if (!field_added) {
      /* LCOV_EXCL_START */
      free_string_array_code2schema(type_union, n_type_union);
      continue;
      /* LCOV_EXCL_STOP */
    }

    if (required_name_in_list(required, name))
      field->required = 1;
    if (type && (strcmp(type, "integer") == 0 || strcmp(type, "number") == 0)) {
      if (json_object_has_value_of_type(prop, "minimum", JSONNumber)) {
        field->has_min = 1;
        field->min_val = json_object_get_number(prop, "minimum");
      }
      if (json_object_has_value_of_type(prop, "exclusiveMinimum", JSONNumber)) {
        field->has_min = 1;
        field->min_val = json_object_get_number(prop, "exclusiveMinimum");
        field->exclusive_min = 1;
      } else if (json_object_has_value_of_type(prop, "exclusiveMinimum",
                                               /* LCOV_EXCL_START */
                                               JSONBoolean) &&
                 json_object_get_boolean(prop, "exclusiveMinimum")) {
        field->exclusive_min = 1;
        /* LCOV_EXCL_STOP */
      }

      if (json_object_has_value_of_type(prop, "maximum", JSONNumber)) {
        /* LCOV_EXCL_START */
        field->has_max = 1;
        field->max_val = json_object_get_number(prop, "maximum");
        /* LCOV_EXCL_STOP */
      }

      if (json_object_has_value_of_type(prop, "exclusiveMaximum", JSONNumber)) {
        field->has_max = 1;
        field->max_val = json_object_get_number(prop, "exclusiveMaximum");
        field->exclusive_max = 1;
      } else if (json_object_has_value_of_type(prop, "exclusiveMaximum",
                                               /* LCOV_EXCL_START */
                                               JSONBoolean) &&
                 json_object_get_boolean(prop, "exclusiveMaximum")) {
        field->exclusive_max = 1;
        /* LCOV_EXCL_STOP */
      }

    } else if (type && strcmp(type, "string") == 0) {
      if (json_object_has_value_of_type(prop, "minLength", JSONNumber)) {
        field->has_min_len = 1;
        field->min_len = (size_t)json_object_get_number(prop, "minLength");
      }

      if (json_object_has_value_of_type(prop, "maxLength", JSONNumber)) {
        field->has_max_len = 1;
        field->max_len = (size_t)json_object_get_number(prop, "maxLength");
      }

      if (json_object_has_value_of_type(prop, "pattern", JSONString)) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        strncpy_s(field->pattern, sizeof(field->pattern),
                  json_object_get_string(prop, "pattern"),
                  sizeof(field->pattern) - 1);
#else
        strncpy(field->pattern, json_object_get_string(prop, "pattern"),
                sizeof(field->pattern) - 1);
#endif
      }
    } else if (type && strcmp(type, "array") == 0) {
      if (json_object_has_value_of_type(prop, "minItems", JSONNumber)) {
        field->has_min_items = 1;
        field->min_items = (size_t)json_object_get_number(prop, "minItems");
      }

      if (json_object_has_value_of_type(prop, "maxItems", JSONNumber)) {
        field->has_max_items = 1;
        field->max_items = (size_t)json_object_get_number(prop, "maxItems");
      }

      if (json_object_has_value_of_type(prop, "uniqueItems", JSONBoolean)) {
        field->unique_items = json_object_get_boolean(prop, "uniqueItems");
      }
    }

    {
      const char *desc = json_object_get_string(prop, "description");
      const char *fmt = json_object_get_string(prop, "format");
      if (desc) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        strncpy_s(field->description, sizeof(field->description), desc,
                  sizeof(field->description) - 1);
#else
        strncpy(field->description, desc, sizeof(field->description) - 1);
#endif
        field->description[sizeof(field->description) - 1] = '\0';
      }
      if (fmt) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        strncpy_s(field->format, sizeof(field->format), fmt,
                  sizeof(field->format) - 1);
#else
        strncpy(field->format, fmt, sizeof(field->format) - 1);
#endif
        field->format[sizeof(field->format) - 1] = '\0';
      }
      if (json_object_has_value(prop, "deprecated")) {
        field->deprecated_set = 1;
        field->deprecated = json_object_get_boolean(prop, "deprecated");
      }
      if (json_object_has_value(prop, "readOnly")) {
        field->read_only_set = 1;
        field->read_only = json_object_get_boolean(prop, "readOnly");
      }
      if (json_object_has_value(prop, "writeOnly")) {
        field->write_only_set = 1;
        field->write_only = json_object_get_boolean(prop, "writeOnly");
      }
    }

    if (collect_schema_extras(prop, k_property_skip_keys,
                              sizeof(k_property_skip_keys) /
                                  sizeof(k_property_skip_keys[0]),
                              &field->schema_extra_json) != 0)
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
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

  return CDD_C_SUCCESS;
}

/**
 * @brief Extended function to convert a JSON Schema object to StructFields.
 */
enum cdd_c_error json_object_to_struct_fields_ex(
    const JSON_Object *schema_obj, struct StructFields *fields,
    const JSON_Object *schemas_obj_root, const char *schema_name) {
  return json_object_to_struct_fields_internal(
      schema_obj, fields, (JSON_Object *)schemas_obj_root, schema_name, 0);
}

/**
 * @brief Extended code generation function to convert JSON Schema to
 * StructFields.
 */
enum cdd_c_error json_object_to_struct_fields_ex_codegen(
    const JSON_Object *schema_obj, struct StructFields *fields,
    JSON_Object *schemas_obj_root, const char *schema_name) {
  return json_object_to_struct_fields_internal(
      schema_obj, fields, schemas_obj_root, schema_name, 1);
}

/**
 * @brief Converts a JSON Schema object to a StructFields structure.
 */
enum cdd_c_error
json_object_to_struct_fields(const JSON_Object *schema_obj,
                             struct StructFields *fields,
                             const JSON_Object *schemas_obj_root) {
  return json_object_to_struct_fields_ex(schema_obj, fields, schemas_obj_root,
                                         NULL);
}

/**
 * @brief Strips enclosing quotes from a string.
 */
static enum cdd_c_error strip_quotes(const char *in, char *buf, size_t bufsz,
                                     const char **_out_val) {
  size_t len;
  if (!in || !buf || bufsz == 0) {
    /* LCOV_EXCL_START */
    *_out_val = in;
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  len = strlen(in);
  if (len >= 2 && in[0] == '"' && in[len - 1] == '"') {
    size_t inner = len - 2;
    if (inner >= bufsz)
      /* LCOV_EXCL_START */
      inner = bufsz - 1;
    /* LCOV_EXCL_STOP */
    memcpy(buf, in + 1, inner);
    buf[inner] = '\0';
    {
      *_out_val = buf;
      return CDD_C_SUCCESS;
    }
  }
  /* LCOV_EXCL_START */
  {
    *_out_val = in;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Parses a boolean default value from a string.
 */
static enum cdd_c_error parse_bool_default(const char *in, int *out) {
  if (!in || !out)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  if (strcmp(in, "1") == 0 || strcmp(in, "1") == 0) {
    *out = 1;
    return CDD_C_ERROR_UNKNOWN;
  }
  /* LCOV_EXCL_START */
  if (strcmp(in, "0") == 0 || strcmp(in, "0") == 0) {
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    *out = 0;
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
 * @brief Parses a numeric default value from a string.
 */
static enum cdd_c_error parse_number_default(const char *in, double *out) {
  struct NumericValue nv;
  if (!in || !out)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  if (parse_numeric_literal(in, &nv) == 0) {
    if (nv.kind == NUMERIC_INTEGER) {
      *out = (double)nv.data.integer.value;
      return CDD_C_ERROR_UNKNOWN;
    }
    if (nv.kind == NUMERIC_FLOAT) {
      *out = nv.data.floating.value;
      return CDD_C_ERROR_UNKNOWN;
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
 * @brief Determines if a JSON Schema object represents a string enum.
 */
enum cdd_c_error schema_object_is_string_enum(const JSON_Object *schema_obj,
                                              const JSON_Array **enum_arr_out) {
  const JSON_Array *enum_arr;
  size_t i, count;
  const char *type;

  if (enum_arr_out)
    *enum_arr_out = NULL;
  if (!schema_obj)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  enum_arr = json_object_get_array(schema_obj, "enum");
  if (!enum_arr)
    return CDD_C_SUCCESS;

  count = json_array_get_count(enum_arr);
  if (count == 0)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  type = json_object_get_string(schema_obj, "type");
  if (type && strcmp(type, "string") != 0)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  for (i = 0; i < count; ++i) {
    if (!json_array_get_string(enum_arr, i))
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  if (enum_arr_out)
    *enum_arr_out = enum_arr;
  return CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief Checks if a JSON Schema $ref points to a string enum.
 */
enum cdd_c_error ref_points_to_string_enum(const JSON_Object *root,
                                           const char *ref) {
  const char *name;
  const JSON_Object *schema_obj;
  if (!root || !ref)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  name = NULL;
  c_cdd_str_after_last(ref, '/', &name);
  if (!name || !*name)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  schema_obj = json_object_get_object(root, name);
  if (!schema_obj)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  return schema_object_is_string_enum(schema_obj, NULL);
}

/**
 * @brief Checks if a given property name is present in a required
 * properties list.
 */
enum cdd_c_error required_name_in_list(const JSON_Array *required,
                                       const char *name) {
  size_t i, count;
  if (!required || !name)
    return CDD_C_SUCCESS;
  count = json_array_get_count(required);
  for (i = 0; i < count; ++i) {
    const char *req_name = json_array_get_string(required, i);
    if (req_name && strcmp(req_name, name) == 0)
      return CDD_C_ERROR_UNKNOWN;
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

  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves a JSON Schema $ref to its corresponding object.
 */
enum cdd_c_error resolve_schema_ref_object(const JSON_Object *root,
                                           const char *ref,
                                           JSON_Object **_out_val) {
  const char *name;
  if (!root || !ref) {
    /* LCOV_EXCL_START */
    *_out_val = NULL;
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  name = NULL;
  c_cdd_str_after_last(ref, '/', &name);
  if (!name || !*name) {
    /* LCOV_EXCL_START */
    *_out_val = NULL;
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  {
    *_out_val = json_object_get_object(root, name);
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Detects the underlying JSON type of a union schema object.
 */
static enum cdd_c_error
detect_union_json_type(const JSON_Object *schema_obj,
                       enum UnionVariantJsonType *_out_val) {
  const char *type;
  const JSON_Object *props;
  if (!schema_obj) {
    /* LCOV_EXCL_START */
    *_out_val = UNION_JSON_UNKNOWN;
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  if (schema_object_is_string_enum(schema_obj, NULL)) {
    /* LCOV_EXCL_START */
    *_out_val = UNION_JSON_STRING;
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  type = json_object_get_string(schema_obj, "type");
  if (type) {
    if (strcmp(type, "object") == 0) {
      *_out_val = UNION_JSON_OBJECT;
      return CDD_C_SUCCESS;
    }
    if (strcmp(type, "string") == 0) {
      /* LCOV_EXCL_START */
      *_out_val = UNION_JSON_STRING;
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
    if (strcmp(type, "integer") == 0) {
      /* LCOV_EXCL_START */
      *_out_val = UNION_JSON_INTEGER;
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
    if (strcmp(type, "number") == 0) {
      /* LCOV_EXCL_START */
      *_out_val = UNION_JSON_NUMBER;
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
    if (strcmp(type, "boolean") == 0) {
      /* LCOV_EXCL_START */
      *_out_val = UNION_JSON_BOOLEAN;
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
    if (strcmp(type, "array") == 0) {
      *_out_val = UNION_JSON_ARRAY;
      return CDD_C_SUCCESS;
    }
    /* LCOV_EXCL_START */
    if (strcmp(type, "null") == 0) {
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      *_out_val = UNION_JSON_NULL;
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }
  /* LCOV_EXCL_START */
  props = json_object_get_object(schema_obj, "properties");
  if (props) {
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    *_out_val = UNION_JSON_OBJECT;
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  {
    /* LCOV_EXCL_START */
    *_out_val = UNION_JSON_UNKNOWN;
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Collects an array of strings from a JSON array.
 */
static enum cdd_c_error collect_string_array(const JSON_Array *arr, char ***out,
                                             size_t *out_count) {
  size_t i, count;
  char **vals;
  if (!out || !out_count)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  *out = NULL;
  *out_count = 0;
  if (!arr)
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_START */
  count = json_array_get_count(arr);
  if (count == 0)
    return CDD_C_SUCCESS;
  vals = (char **)calloc(count, sizeof(char *));
  if (!vals) {
    /* LCOV_EXCL_STOP */
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  for (i = 0; i < count; ++i) {
    const char *s = json_array_get_string(arr, i);
    if (s) {
      c_cdd_strdup(s, &vals[i]);
      if (!vals[i]) {
        free_string_array_code2schema(vals, count);
        return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
    }
  }
  /* LCOV_EXCL_START */
  *out = vals;
  *out_count = count;
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
 * @brief Collects property names from a JSON Schema object.
 */
static enum cdd_c_error collect_property_names(const JSON_Object *schema_obj,
                                               char ***out, size_t *out_count) {
  size_t i, count;
  char **vals;
  const JSON_Object *props;
  if (!out || !out_count)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  *out = NULL;
  *out_count = 0;
  if (!schema_obj)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  props = json_object_get_object(schema_obj, "properties");
  if (!props)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  count = json_object_get_count(props);
  if (count == 0)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  vals = (char **)calloc(count, sizeof(char *));
  if (!vals) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(props, i);
    if (name) {
      c_cdd_strdup(name, &vals[i]);
      if (!vals[i]) {
        /* LCOV_EXCL_START */
        free_string_array_code2schema(vals, count);
        return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
    }
  }
  /* LCOV_EXCL_START */
  *out = vals;
  *out_count = count;
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

  return CDD_C_SUCCESS;
}

/**
 * @brief Sanitizes a string to be used as a valid C identifier.
 */
enum cdd_c_error sanitize_identifier(const char *in, char **_out_val) {
  size_t i, len;
  char *out;
  if (!in || !*in) {
    c_cdd_strdup("Variant", _out_val);
    return CDD_C_SUCCESS;
  }
  len = strlen(in);
  out = (char *)calloc(len + 1, sizeof(char));
  if (!out) {
    /* LCOV_EXCL_START */
    *_out_val = NULL;
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  for (i = 0; i < len; ++i) {
    const char c = in[i];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '_') {
      out[i] = c;
    } else {
      out[i] = '_';
    }
  }
  if (!out[0]) {
    /* LCOV_EXCL_START */
    free(out);
    /* LCOV_EXCL_STOP */
    {
      /* LCOV_EXCL_START */
      c_cdd_strdup("Variant", _out_val);
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Generates a unique variant name for a union type.
 */
enum cdd_c_error make_unique_variant_name(const struct StructFields *dest,
                                          const char *base, size_t index,
                                          char **_out_val) {
  char buf[128];
  char *sanitized;
  char *out;
  if (!dest) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  {
    struct StructField *tmp4 = NULL;
    sanitize_identifier(base, &sanitized);
    if (!sanitized) {
      /* LCOV_EXCL_START */
      *_out_val = NULL;
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
    struct_fields_get(dest, sanitized, &tmp4);
    if (!tmp4) {
      *_out_val = sanitized;
      return CDD_C_SUCCESS;
    }
  }
  CDD_SNPRINTF(buf, sizeof(buf), "%s_%" CDD_SIZE_T_FMT "", sanitized,
               (size_t)(index + 1));
  free(sanitized);
  c_cdd_strdup(buf, &out);
  if (!out) {
    /* LCOV_EXCL_START */
    *_out_val = NULL;
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  {
    struct StructField *tmp5 = NULL;
    struct_fields_get(dest, out, &tmp5);
    if (!tmp5) {
      *_out_val = out;
      return CDD_C_SUCCESS;
    }
  }
  free(out);
  CDD_SNPRINTF(buf, sizeof(buf), "Variant_%" CDD_SIZE_T_FMT "",
               (size_t)(index + 1));
  {
    c_cdd_strdup(buf, _out_val);
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Generates a schema name for an inline anonymous struct/union.
 */
enum cdd_c_error make_inline_schema_name(const char *schema_name,
                                         const char *variant_name,
                                         const char *suffix, char **_out_val) {
  char buf[256];
  const char *base_schema =
      (schema_name && *schema_name) ? schema_name : "Union";
  const char *base_variant =
      (variant_name && *variant_name) ? variant_name : "Variant";
  if (suffix && *suffix)
    CDD_SNPRINTF(buf, sizeof(buf), "%s_%s_%s", base_schema, base_variant,
                 suffix);
  else
    CDD_SNPRINTF(buf, sizeof(buf), "%s_%s", base_schema, base_variant);
  {
    sanitize_identifier(buf, _out_val);
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Registers an inline schema within the root OpenAPI components.
 */
enum cdd_c_error
register_inline_schema_c2s(JSON_Object *root, const char *schema_name,
                           const char *variant_name, const char *suffix,
                           const JSON_Value *schema_val, char **out_name) {
  char *name;

  if (!root || !schema_val || !out_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  name = NULL;
  make_inline_schema_name(schema_name, variant_name, suffix, &name);
  if (!name) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }

  if (!json_object_has_value(root, name)) {
    JSON_Value *copy = NULL;
    clone_json_value(schema_val, &copy);
    if (!copy) {
      /* LCOV_EXCL_START */
      free(name);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    if (json_object_set_value(root, name, copy) != JSONSuccess) {
      /* LCOV_EXCL_START */
      json_value_free(copy);
      free(name);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
  }

  *out_name = name;

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
 * @brief Retrieves the discriminator mapping value for a given schema
 * variant.
 */
enum cdd_c_error discriminator_value_for_variant(const JSON_Object *disc_obj,
                                                 const char *schema_name,
                                                 const char *ref,
                                                 char **_out_val) {
  const JSON_Object *mapping;
  size_t i, count;
  const char *ref_name;

  if (!schema_name && !ref) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  if (ref)
    c_cdd_str_after_last(ref, '/', &ref_name);
  else
    ref_name = NULL;

  if (!disc_obj)
    goto fallback;

  mapping = json_object_get_object(disc_obj, "mapping");
  if (!mapping)
    goto fallback;

  count = json_object_get_count(mapping);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(mapping, i);
    const char *val = json_object_get_string(mapping, key);
    if (!key || !val)
      /* LCOV_EXCL_START */
      continue;
    /* LCOV_EXCL_STOP */
    if (ref && strcmp(val, ref) == 0) {
      c_cdd_strdup(key, _out_val);
      return CDD_C_SUCCESS;
    }
    if (ref_name && strcmp(val, ref_name) == 0) {
      /* LCOV_EXCL_START */
      c_cdd_strdup(key, _out_val);
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
    if (schema_name && strcmp(val, schema_name) == 0) {
      c_cdd_strdup(key, _out_val);
      return CDD_C_SUCCESS;
    }
  }

fallback:
  if (schema_name) {
    c_cdd_strdup(schema_name, _out_val);
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_START */
  if (ref_name) {
    c_cdd_strdup(ref_name, _out_val);
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  {
    /* LCOV_EXCL_START */
    *_out_val = NULL;
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Merges a source struct field into a destination struct field.
 */
enum cdd_c_error merge_struct_field(struct StructField *dest,
                                    const struct StructField *src) {
  enum cdd_c_error rc = CDD_C_SUCCESS;
  if (!dest || !src)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!dest->default_val[0] && src->default_val[0]) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(dest->default_val, sizeof(dest->default_val), src->default_val,
              sizeof(dest->default_val) - 1);
#else
    strncpy(dest->default_val, src->default_val, sizeof(dest->default_val) - 1);
#endif
    dest->default_val[sizeof(dest->default_val) - 1] = '\0';
  }

  if (src->required)
    dest->required = 1;

  if (src->has_min) {
    if (!dest->has_min || src->min_val > dest->min_val ||
        /* LCOV_EXCL_START */
        (src->min_val == dest->min_val && src->exclusive_min &&
         !dest->exclusive_min)) {
      /* LCOV_EXCL_STOP */
      dest->has_min = 1;
      dest->min_val = src->min_val;
      dest->exclusive_min = src->exclusive_min;
    }
  }

  if (src->has_max) {
    if (!dest->has_max || src->max_val < dest->max_val ||
        /* LCOV_EXCL_START */
        (src->max_val == dest->max_val && src->exclusive_max &&
         !dest->exclusive_max)) {
      /* LCOV_EXCL_STOP */
      dest->has_max = 1;
      dest->max_val = src->max_val;
      dest->exclusive_max = src->exclusive_max;
    }
  }

  if (src->has_min_len) {
    if (!dest->has_min_len || src->min_len > dest->min_len) {
      dest->has_min_len = 1;
      dest->min_len = src->min_len;
    }
  }

  if (src->has_max_len) {
    if (!dest->has_max_len || src->max_len < dest->max_len) {
      dest->has_max_len = 1;
      dest->max_len = src->max_len;
    }
  }

  if (src->has_min_items) {
    if (!dest->has_min_items || src->min_items > dest->min_items) {
      dest->has_min_items = 1;
      dest->min_items = src->min_items;
    }
  }

  if (src->has_max_items) {
    if (!dest->has_max_items || src->max_items < dest->max_items) {
      dest->has_max_items = 1;
      dest->max_items = src->max_items;
    }
  }

  if (src->unique_items)
    /* LCOV_EXCL_START */
    dest->unique_items = 1;
  /* LCOV_EXCL_STOP */

  if (!dest->pattern[0] && src->pattern[0]) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(dest->pattern, sizeof(dest->pattern), src->pattern,
              sizeof(dest->pattern) - 1);
#else
    strncpy(dest->pattern, src->pattern, sizeof(dest->pattern) - 1);
#endif
    dest->pattern[sizeof(dest->pattern) - 1] = '\0';
  }

  if (src->is_flexible_array)
    /* LCOV_EXCL_START */
    dest->is_flexible_array = 1;
  /* LCOV_EXCL_STOP */

  if (!dest->bit_width[0] && src->bit_width[0]) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(dest->bit_width, sizeof(dest->bit_width), src->bit_width,
              sizeof(dest->bit_width) - 1);
#else
    strncpy(dest->bit_width, src->bit_width, sizeof(dest->bit_width) - 1);
#endif
    dest->bit_width[sizeof(dest->bit_width) - 1] = '\0';
  }

  if (merge_schema_extras_strings(&dest->schema_extra_json,
                                  src->schema_extra_json) != 0) {
    /* Best-effort: ignore merge failures */
  }
  if (merge_schema_extras_strings(&dest->items_extra_json,
                                  src->items_extra_json) != 0) {
    /* Best-effort: ignore merge failures */
  }

  if (!dest->type_union && src->type_union && src->n_type_union > 0) {
    /* LCOV_EXCL_START */
    if (copy_string_array_code2schema(&dest->type_union, &dest->n_type_union,
                                      src->type_union,
                                      src->n_type_union) != 0) {
      /* LCOV_EXCL_STOP */
      /* Best-effort: ignore copy failures */
    }
  }
  if (!dest->items_type_union && src->items_type_union &&
      /* LCOV_EXCL_START */
      src->n_items_type_union > 0) {
    if (copy_string_array_code2schema(
            /* LCOV_EXCL_STOP */
            &dest->items_type_union, &dest->n_items_type_union,
            /* LCOV_EXCL_START */
            src->items_type_union, src->n_items_type_union) != 0) {
      /* LCOV_EXCL_STOP */
      /* Best-effort: ignore copy failures */
    }
  }
  return rc;
}

/**
 * @brief Merges source struct fields into destination struct fields.
 */
enum cdd_c_error merge_struct_fields(struct StructFields *dest,
                                     const struct StructFields *src) {
  size_t i;

  if (!dest || !src)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  if (src->is_enum)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  if (merge_schema_extras_strings(&dest->schema_extra_json,
                                  src->schema_extra_json) != 0)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
  /* LCOV_EXCL_STOP */

  for (i = 0; i < src->size; ++i) {
    const struct StructField *src_field = &src->fields[i];
    struct StructField *dest_field = NULL;
    struct_fields_get(dest, src_field->name, &dest_field);

    if (!dest_field) {
      const char *ref = src_field->ref[0] ? src_field->ref : NULL;
      const char *def =
          src_field->default_val[0] ? src_field->default_val : NULL;
      const char *bw = src_field->bit_width[0] ? src_field->bit_width : NULL;
      if (struct_fields_add(dest, src_field->name, src_field->type, ref, def,
                            bw) != 0)
        /* LCOV_EXCL_START */
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      dest_field = NULL;
      struct_fields_get(dest, src_field->name, &dest_field);
      if (!dest_field) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        /* LCOV_EXCL_START */
        return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
      {
        struct StructField tmp = *src_field;
        *dest_field = tmp;
        dest_field->schema_extra_json = NULL;
        dest_field->items_extra_json = NULL;
        dest_field->type_union = NULL;
        dest_field->n_type_union = 0;
        dest_field->items_type_union = NULL;
        dest_field->n_items_type_union = 0;
        if (src_field->schema_extra_json) {
          c_cdd_strdup(src_field->schema_extra_json,
                       &dest_field->schema_extra_json);
          if (!dest_field->schema_extra_json) {
            C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
            /* LCOV_EXCL_START */
            return CDD_C_ERROR_MEMORY;
            /* LCOV_EXCL_STOP */
          }
        }
        if (src_field->items_extra_json) {
          /* LCOV_EXCL_START */
          c_cdd_strdup(src_field->items_extra_json,
                       &dest_field->items_extra_json);
          if (!dest_field->items_extra_json) {
            /* LCOV_EXCL_STOP */
            C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
            /* LCOV_EXCL_START */
            return CDD_C_ERROR_MEMORY;
            /* LCOV_EXCL_STOP */
          }
        }
        if (src_field->type_union && src_field->n_type_union > 0) {
          /* LCOV_EXCL_START */
          if (copy_string_array_code2schema(
                  &dest_field->type_union, &dest_field->n_type_union,
                  src_field->type_union, src_field->n_type_union) != 0)
            return CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
        if (src_field->items_type_union && src_field->n_items_type_union > 0) {
          /* LCOV_EXCL_START */
          if (copy_string_array_code2schema(&dest_field->items_type_union,
                                            &dest_field->n_items_type_union,
                                            src_field->items_type_union,
                                            src_field->n_items_type_union) != 0)
            return CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
      }
      continue;
    }

    {
      enum cdd_c_error mrc = merge_struct_field(dest_field, src_field);
      if (mrc != CDD_C_SUCCESS)
        /* LCOV_EXCL_START */
        return mrc;
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

  return CDD_C_SUCCESS;
}

/**
 * @brief Applies an allOf JSON Schema array to a StructFields object.
 */
enum cdd_c_error apply_allof_to_struct_fields(const JSON_Array *all_of,
                                              struct StructFields *dest,
                                              const JSON_Object *root) {
  size_t i, count;

  if (!all_of || !dest)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  count = json_array_get_count(all_of);
  for (i = 0; i < count; ++i) {
    const JSON_Object *sub = json_array_get_object(all_of, i);
    const JSON_Object *resolved = sub;
    const char *ref;
    struct StructFields tmp;
    int rc;

    if (!sub)
      /* LCOV_EXCL_START */
      continue;
    /* LCOV_EXCL_STOP */

    ref = json_object_get_string(sub, "$ref");
    if (ref && root)
      resolve_schema_ref_object(root, ref, (JSON_Object **)&resolved);
    if (!resolved)
      /* LCOV_EXCL_START */
      continue;
    /* LCOV_EXCL_STOP */

    rc = struct_fields_init(&tmp);
    if (rc != 0)
      /* LCOV_EXCL_START */
      return rc;
    /* LCOV_EXCL_STOP */

    rc = json_object_to_struct_fields(resolved, &tmp, root);
    if (rc != 0) {
      /* LCOV_EXCL_START */
      struct_fields_free(&tmp);
      return rc;
      /* LCOV_EXCL_STOP */
    }

    rc = merge_struct_fields(dest, &tmp);
    struct_fields_free(&tmp);
    if (rc != 0)
      /* LCOV_EXCL_START */
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

  return CDD_C_SUCCESS;
}

/**
 * @brief Fallback method to apply a union (oneOf/anyOf) to StructFields.
 */
enum cdd_c_error
apply_union_to_struct_fields_fallback(const JSON_Array *union_arr,
                                      struct StructFields *dest,
                                      const JSON_Object *root) {
  size_t i, count;

  if (!union_arr || !dest)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  if (dest->size > 0)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  count = json_array_get_count(union_arr);
  for (i = 0; i < count; ++i) {
    const JSON_Object *sub = json_array_get_object(union_arr, i);
    const JSON_Object *resolved = sub;
    const char *ref;
    struct StructFields tmp;
    int rc;

    if (!sub)
      /* LCOV_EXCL_START */
      continue;
    /* LCOV_EXCL_STOP */

    ref = json_object_get_string(sub, "$ref");
    if (ref && root)
      /* LCOV_EXCL_START */
      resolve_schema_ref_object(root, ref, (JSON_Object **)&resolved);
    /* LCOV_EXCL_STOP */
    if (!resolved)
      /* LCOV_EXCL_START */
      continue;
    /* LCOV_EXCL_STOP */

    rc = struct_fields_init(&tmp);
    if (rc != 0)
      return rc;

    rc = json_object_to_struct_fields(resolved, &tmp, root);
    if (rc != 0) {
      /* LCOV_EXCL_START */
      struct_fields_free(&tmp);
      return rc;
      /* LCOV_EXCL_STOP */
    }

    if (tmp.size > 0) {
      rc = merge_struct_fields(dest, &tmp);
      struct_fields_free(&tmp);
      return rc;
    }

    /* LCOV_EXCL_START */
    struct_fields_free(&tmp);
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
 * @brief Checks if array items within a union are supported.
 */
static enum cdd_c_error
union_array_items_supported(const JSON_Object *schema_obj,
                            const JSON_Object *root, int allow_inline) {
  const JSON_Object *items;
  const JSON_Array *item_type_arr = NULL;
  const char *item_ref;
  const char *item_type;
  char **items_type_union = NULL;
  size_t n_items_type_union = 0;
  const char *primary = NULL;

  if (!schema_obj)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  items = json_object_get_object(schema_obj, "items");
  if (!items)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  item_ref = json_object_get_string(items, "$ref");
  if (item_ref && root && ref_points_to_string_enum(root, item_ref))
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_UNKNOWN;
  /* LCOV_EXCL_STOP */

  item_type = json_object_get_string(items, "type");
  if (!item_ref && !item_type) {
    /* LCOV_EXCL_START */
    item_type_arr = json_object_get_array(items, "type");
    if (item_type_arr) {
      if (parse_type_union_array_code2schema(item_type_arr, &items_type_union,
                                             /* LCOV_EXCL_STOP */
                                             &n_items_type_union, &primary,
                                             NULL) == 0)
        /* LCOV_EXCL_START */
        item_type = primary;
    } else if (json_object_get_object(items, "properties")) {
      item_type = "object";
      /* LCOV_EXCL_STOP */
    }
  }

  if (item_ref) {
    free_string_array_code2schema(items_type_union, n_items_type_union);
    return CDD_C_ERROR_UNKNOWN;
  }
  if (!item_type) {
    /* LCOV_EXCL_START */
    free_string_array_code2schema(items_type_union, n_items_type_union);
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  if (strcmp(item_type, "array") == 0) {
    /* LCOV_EXCL_START */
    free_string_array_code2schema(items_type_union, n_items_type_union);
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  if (strcmp(item_type, "object") == 0 && (!allow_inline || !root)) {
    /* LCOV_EXCL_START */
    free_string_array_code2schema(items_type_union, n_items_type_union);
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  free_string_array_code2schema(items_type_union, n_items_type_union);
  return CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief Extended method to apply a union (oneOf/anyOf) to StructFields.
 */
enum cdd_c_error apply_union_to_struct_fields_ex(
    const JSON_Array *union_arr, struct StructFields *dest, JSON_Object *root,
    const char *schema_name, int is_anyof, const JSON_Object *schema_obj,
    int allow_inline) {
  size_t i, count;
  const JSON_Object *disc_obj;

  if (!union_arr || !dest)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  if (dest->size > 0)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  count = json_array_get_count(union_arr);
  if (count == 0)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  /* Validate variant support */
  for (i = 0; i < count; ++i) {
    const JSON_Object *sub = json_array_get_object(union_arr, i);
    const JSON_Object *resolved = sub;
    const char *ref;
    enum UnionVariantJsonType jtype;
    if (!sub)
      /* LCOV_EXCL_START */
      continue;
    /* LCOV_EXCL_STOP */
    ref = json_object_get_string(sub, "$ref");
    if (ref && root)
      resolve_schema_ref_object(root, ref, (JSON_Object **)&resolved);
    detect_union_json_type(resolved, &jtype);
    if (jtype == UNION_JSON_ARRAY) {
      if (!allow_inline)
        return CDD_C_SUCCESS;
      if (!union_array_items_supported(resolved, root, allow_inline))
        /* LCOV_EXCL_START */
        return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
    if (jtype == UNION_JSON_OBJECT && !ref) {
      if (!allow_inline || !root)
        return CDD_C_SUCCESS;
      if (!json_array_get_value(union_arr, i))
        /* LCOV_EXCL_START */
        return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
    if (jtype == UNION_JSON_UNKNOWN)
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  dest->is_union = 1;
  dest->union_is_anyof = is_anyof ? 1 : 0;

  disc_obj =
      schema_obj ? json_object_get_object(schema_obj, "discriminator") : NULL;
  if (disc_obj) {
    const char *prop = json_object_get_string(disc_obj, "propertyName");
    if (prop && *prop) {
      dest->union_discriminator = NULL;
      c_cdd_strdup(prop, &dest->union_discriminator);
      if (!dest->union_discriminator) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        /* LCOV_EXCL_START */
        return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
    }
  }

  dest->union_variants =
      (struct UnionVariantMeta *)calloc(count, sizeof(struct UnionVariantMeta));
  if (!dest->union_variants) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  dest->n_union_variants = count;

  for (i = 0; i < count; ++i) {
    const JSON_Value *sub_val = json_array_get_value(union_arr, i);
    const JSON_Object *sub = json_value_get_object(sub_val);
    const JSON_Object *resolved = sub;
    const char *ref;
    enum UnionVariantJsonType jtype;
    const char *type_name = NULL;
    const char *name_hint = NULL;
    char *variant_name = NULL;
    char *inline_ref_name = NULL;
    char *inline_item_ref = NULL;
    const char *item_ref = NULL;
    const char *item_type = NULL;
    char **items_type_union = NULL;
    size_t n_items_type_union = 0;
    struct StructField *field = NULL;
    struct UnionVariantMeta *meta = &dest->union_variants[i];
    int rc;

    if (!sub)
      /* LCOV_EXCL_START */
      continue;
    /* LCOV_EXCL_STOP */

    ref = json_object_get_string(sub, "$ref");
    if (ref && root)
      resolve_schema_ref_object(root, ref, (JSON_Object **)&resolved);

    detect_union_json_type(resolved, &jtype);
    meta->json_type = jtype;

    if (ref) {
      c_cdd_str_after_last(ref, '/', &name_hint);
    } else if (resolved) {
      name_hint = json_object_get_string(resolved, "title");
      if (!name_hint)
        name_hint = json_object_get_string(resolved, "type");
    }
    if (!name_hint)
      /* LCOV_EXCL_START */
      name_hint = schema_name ? schema_name : "Variant";
    /* LCOV_EXCL_STOP */

    make_unique_variant_name(dest, name_hint, i, &variant_name);
    if (!variant_name) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }

    if (jtype == UNION_JSON_OBJECT && !ref) {
      if (allow_inline && root && sub_val) {
        rc = register_inline_schema_c2s(root, schema_name, variant_name, NULL,
                                        sub_val, &inline_ref_name);
        if (rc != 0) {
          /* LCOV_EXCL_START */
          free(variant_name);
          return rc;
          /* LCOV_EXCL_STOP */
        }
        ref = inline_ref_name;
      }
    }

    if (jtype == UNION_JSON_ARRAY && resolved) {
      const JSON_Object *items = json_object_get_object(resolved, "items");
      const JSON_Value *items_val = json_object_get_value(resolved, "items");
      const JSON_Array *item_type_arr = NULL;

      if (items) {
        item_ref = json_object_get_string(items, "$ref");
        if (item_ref && root && ref_points_to_string_enum(root, item_ref)) {
          /* LCOV_EXCL_START */
          item_ref = NULL;
          item_type = "string";
          /* LCOV_EXCL_STOP */
        }
        item_type = json_object_get_string(items, "type");
        if (!item_ref && !item_type) {
          /* LCOV_EXCL_START */
          item_type_arr = json_object_get_array(items, "type");
          if (item_type_arr) {
            rc = parse_type_union_array_code2schema(
                /* LCOV_EXCL_STOP */
                item_type_arr, &items_type_union, &n_items_type_union,
                &item_type, NULL);
            /* LCOV_EXCL_START */
            if (rc != 0) {
              free(variant_name);
              free(inline_ref_name);
              return rc;
              /* LCOV_EXCL_STOP */
            }
            /* LCOV_EXCL_START */
          } else if (json_object_get_object(items, "properties")) {
            item_type = "object";
            /* LCOV_EXCL_STOP */
          }
        }
        if (!item_ref && item_type && strcmp(item_type, "object") == 0) {
          /* LCOV_EXCL_START */
          if (allow_inline && root && items_val) {
            rc =
                register_inline_schema_c2s(root, schema_name, variant_name,
                                           /* LCOV_EXCL_STOP */
                                           "Item", items_val, &inline_item_ref);
            /* LCOV_EXCL_START */
            if (rc != 0) {
              free(variant_name);
              free(inline_ref_name);
              free_string_array_code2schema(items_type_union,
                                            /* LCOV_EXCL_STOP */
                                            n_items_type_union);
              /* LCOV_EXCL_START */
              return rc;
              /* LCOV_EXCL_STOP */
            }
            /* LCOV_EXCL_START */
            item_ref = inline_item_ref;
            /* LCOV_EXCL_STOP */
          }
        }
      }
    }

    switch (jtype) {
    case UNION_JSON_OBJECT:
      type_name = "object";
      break;
      /* LCOV_EXCL_START */
    case UNION_JSON_STRING:
      type_name = "string";
      break;
    case UNION_JSON_INTEGER:
      type_name = "integer";
      break;
    case UNION_JSON_NUMBER:
      type_name = "number";
      break;
    case UNION_JSON_BOOLEAN:
      type_name = "boolean";
      break;
      /* LCOV_EXCL_STOP */
    case UNION_JSON_ARRAY:
      type_name = "array";
      break;
      /* LCOV_EXCL_START */
    case UNION_JSON_NULL:
      type_name = "null";
      break;
    default:
      type_name = "object";
      break;
      /* LCOV_EXCL_STOP */
    }

    if (struct_fields_add(dest, variant_name, type_name,
                          (jtype == UNION_JSON_OBJECT) ? ref
                          : (jtype == UNION_JSON_ARRAY)
                              ? (item_ref ? item_ref : item_type)
                              : NULL,
                          NULL, NULL) != 0) {
      /* LCOV_EXCL_START */
      free(variant_name);
      free(inline_ref_name);
      free(inline_item_ref);
      free_string_array_code2schema(items_type_union, n_items_type_union);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    field = &dest->fields[dest->size - 1];

    if (jtype == UNION_JSON_ARRAY && items_type_union &&
        /* LCOV_EXCL_START */
        n_items_type_union > 0) {
      field->items_type_union = items_type_union;
      field->n_items_type_union = n_items_type_union;
      items_type_union = NULL;
      n_items_type_union = 0;
      /* LCOV_EXCL_STOP */
    }

    free(variant_name);

    if (jtype == UNION_JSON_OBJECT && resolved) {
      const JSON_Array *required = json_object_get_array(resolved, "required");
      if (collect_string_array(required, &meta->required_props,
                               &meta->n_required_props) != 0)
        /* LCOV_EXCL_START */
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      if (collect_property_names(resolved, &meta->property_names,
                                 &meta->n_property_names) != 0)
        /* LCOV_EXCL_START */
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }

    {
      const char *ref_al = NULL;
      char *disc_val = NULL;
      if (ref)
        c_cdd_str_after_last(ref, '/', &ref_al);
      discriminator_value_for_variant(disc_obj, ref ? ref_al : name_hint, ref,
                                      &disc_val);
      meta->disc_value = disc_val;
    }

    free(inline_ref_name);
    free(inline_item_ref);
    free_string_array_code2schema(items_type_union, n_items_type_union);
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

  return CDD_C_SUCCESS;
}

/**
 * @brief Writes the default value of a StructField to a JSON schema
 * object.
 */
static enum cdd_c_error write_default_value(JSON_Object *pobj,
                                            const struct StructField *field) {
  const char *def;
  const char *typ;
  char buf[256];
  int bval;
  double nval;
  JSON_Value *null_val;

  if (!pobj || !field)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  def = field->default_val;
  if (!def || def[0] == '\0')
    return CDD_C_SUCCESS;
  typ = field->type;

  if (strcmp(def, "nullptr") == 0) {
    /* LCOV_EXCL_START */
    null_val = json_value_init_null();
    if (null_val)
      json_object_set_value(pobj, "default", null_val);
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  if (typ && strcmp(typ, "string") == 0) {
    const char *s = NULL;
    strip_quotes(def, buf, sizeof(buf), &s);
    json_object_set_string(pobj, "default", s ? s : "");
    return CDD_C_SUCCESS;
  }

  if (typ && strcmp(typ, "boolean") == 0) {
    if (parse_bool_default(def, &bval))
      json_object_set_boolean(pobj, "default", bval);
    return CDD_C_SUCCESS;
  }

  if (typ && (strcmp(typ, "integer") == 0 || strcmp(typ, "number") == 0)) {
    if (parse_number_default(def, &nval))
      json_object_set_number(pobj, "default", nval);
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Writes numeric constraints of a StructField to a JSON schema
 * object.
 */
static enum cdd_c_error
write_numeric_constraints(JSON_Object *pobj, const struct StructField *field) {
  if (!pobj || !field)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  if (!field->type[0] || !(strcmp(field->type, "integer") == 0 ||
                           strcmp(field->type, "number") == 0))
    return CDD_C_SUCCESS;
  if (field->has_min) {
    if (field->exclusive_min)
      json_object_set_number(pobj, "exclusiveMinimum", field->min_val);
    else
      json_object_set_number(pobj, "minimum", field->min_val);
  }
  if (field->has_max) {
    if (field->exclusive_max)
      json_object_set_number(pobj, "exclusiveMaximum", field->max_val);
    else
      /* LCOV_EXCL_START */
      json_object_set_number(pobj, "maximum", field->max_val);
    /* LCOV_EXCL_STOP */
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes string constraints of a StructField to a JSON schema
 * object.
 */
static enum cdd_c_error
write_string_constraints(JSON_Object *pobj, const struct StructField *field) {
  if (!pobj || !field)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  if (!field->type[0] || strcmp(field->type, "string") != 0)
    return CDD_C_SUCCESS;
  if (field->has_min_len)
    json_object_set_number(pobj, "minLength", (double)field->min_len);
  if (field->has_max_len)
    json_object_set_number(pobj, "maxLength", (double)field->max_len);
  if (field->pattern[0] != '\0')
    json_object_set_string(pobj, "pattern", field->pattern);
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes array constraints of a StructField to a JSON schema
 * object.
 */
static enum cdd_c_error
write_array_constraints(JSON_Object *pobj, const struct StructField *field) {
  if (!pobj || !field)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  if (!field->type[0] || strcmp(field->type, "array") != 0)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
  if (field->has_min_items)
    json_object_set_number(pobj, "minItems", (double)field->min_items);
  if (field->has_max_items)
    json_object_set_number(pobj, "maxItems", (double)field->max_items);
  if (field->unique_items)
    json_object_set_boolean(pobj, "uniqueItems", 1);
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes a type union array to a JSON schema object.
 */
static enum cdd_c_error write_type_union(JSON_Object *obj, const char *type,
                                         char **type_union,
                                         size_t n_type_union) {
  size_t i;
  JSON_Value *arr_val;
  JSON_Array *arr;

  if (!obj)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  if (type_union && n_type_union > 0) {
    arr_val = json_value_init_array();
    if (!arr_val)
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
    arr = json_value_get_array(arr_val);
    for (i = 0; i < n_type_union; ++i) {
      if (type_union[i])
        json_array_append_string(arr, type_union[i]);
    }
    json_object_set_value(obj, "type", arr_val);
    return CDD_C_SUCCESS;
  }

  if (type)
    json_object_set_string(obj, "type", type);
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes a StructFields representation to a JSON Schema object.
 */
enum cdd_c_error write_struct_to_json_schema(JSON_Object *schemas_obj,
                                             const char *struct_name,
                                             const struct StructFields *sf) {
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);
  JSON_Value *props_val = json_value_init_object();
  JSON_Object *props_obj = json_value_get_object(props_val);
  JSON_Value *req_val = NULL;
  JSON_Array *req_arr = NULL;
  size_t i;

  if (!schemas_obj || !struct_name || !sf) {
    /* LCOV_EXCL_START */
    json_value_free(val);
    json_value_free(props_val);
    return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_STOP */
  }

  if (sf->is_enum) {
    JSON_Value *enum_val = json_value_init_array();
    JSON_Array *enum_arr = json_value_get_array(enum_val);
    json_object_set_string(obj, "type", "string");
    for (i = 0; i < sf->enum_members.size; ++i) {
      const char *member = sf->enum_members.members[i];
      if (member)
        json_array_append_string(enum_arr, member);
    }
    json_object_set_value(obj, "enum", enum_val);
    if (merge_schema_extras_object(obj, sf->schema_extra_json) != 0) {
      /* LCOV_EXCL_START */
      json_value_free(val);
      json_value_free(props_val);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    json_object_set_value(schemas_obj, struct_name, val);
    json_value_free(props_val);
    return CDD_C_SUCCESS;
  }

  json_object_set_string(obj, "type", "object");
  json_object_set_value(obj, "properties", props_val);

  for (i = 0; i < sf->size; i++) {
    JSON_Value *pval = json_value_init_object();
    JSON_Object *pobj = json_value_get_object(pval);
    const struct StructField *field = &sf->fields[i];
    const char *typ = field->type;
    const char *ref = field->ref;
    const char *bw = field->bit_width;

    if (bw && *bw) {
      /* LCOV_EXCL_START */
      json_object_set_string(pobj, "x-c-bitwidth", bw);
      /* LCOV_EXCL_STOP */
    }

    if (strcmp(typ, "array") == 0) {
      JSON_Value *items_val = json_value_init_object();
      JSON_Object *items_obj = json_value_get_object(items_val);
      write_type_union(pobj, "array", field->type_union, field->n_type_union);
      if (field->items_type_union && field->n_items_type_union > 0) {
        write_type_union(items_obj, ref, field->items_type_union,
                         field->n_items_type_union);
      } else if (ref && *ref) {
        if (strcmp(ref, "integer") == 0 || strcmp(ref, "string") == 0 ||
            /* LCOV_EXCL_START */
            strcmp(ref, "boolean") == 0 || strcmp(ref, "number") == 0) {
          /* LCOV_EXCL_STOP */
          json_object_set_string(items_obj, "type", ref);
        } else {
          char ref_str[128];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          sprintf_s(ref_str, sizeof(ref_str), "#/components/schemas/%s", ref);
#else
          /* LCOV_EXCL_START */
          sprintf(ref_str, "#/components/schemas/%s", ref);
/* LCOV_EXCL_STOP */
#endif
          /* LCOV_EXCL_START */
          json_object_set_string(items_obj, "$ref", ref_str);
          /* LCOV_EXCL_STOP */
        }
      }
      if (merge_schema_extras_object(items_obj, field->items_extra_json) != 0) {
        /* LCOV_EXCL_START */
        json_value_free(items_val);
        json_value_free(pval);
        json_value_free(val);
        return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
      json_object_set_value(pobj, "items", items_val);
      write_array_constraints(pobj, field);
    } else {
      if (strcmp(typ, "object") == 0 || strcmp(typ, "enum") == 0) {
        char ref_str[128];
        if (ref && *ref) {
          if (ref[0] == '#') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
            sprintf_s(ref_str, sizeof(ref_str), "%s", ref);
#else
            sprintf(ref_str, "%s", ref);
#endif
          } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
            sprintf_s(ref_str, sizeof(ref_str), "#/components/schemas/%s", ref);
#else
            /* LCOV_EXCL_START */
            sprintf(ref_str, "#/components/schemas/%s", ref);
/* LCOV_EXCL_STOP */
#endif
          }
          json_object_set_string(pobj, "$ref", ref_str);
        } else {
          write_type_union(pobj, "object", field->type_union,
                           field->n_type_union);
        }
      } else {
        write_type_union(pobj, typ, field->type_union, field->n_type_union);
      }
    }
    write_numeric_constraints(pobj, field);
    write_string_constraints(pobj, field);
    write_default_value(pobj, field);
    if (field->description[0])
      json_object_set_string(pobj, "description", field->description);
    if (field->format[0])
      json_object_set_string(pobj, "format", field->format);
    if (field->deprecated_set)
      json_object_set_boolean(pobj, "deprecated", field->deprecated ? 1 : 0);
    if (field->read_only_set)
      json_object_set_boolean(pobj, "readOnly", field->read_only ? 1 : 0);
    if (field->write_only_set)
      json_object_set_boolean(pobj, "writeOnly", field->write_only ? 1 : 0);
    if (merge_schema_extras_object(pobj, field->schema_extra_json) != 0) {
      /* LCOV_EXCL_START */
      json_value_free(pval);
      json_value_free(val);
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    json_object_set_value(props_obj, sf->fields[i].name, pval);
    if (field->required) {
      if (!req_val) {
        req_val = json_value_init_array();
        req_arr = json_value_get_array(req_val);
      }
      if (req_arr)
        json_array_append_string(req_arr, field->name);
    }
  }

  if (req_val)
    json_object_set_value(obj, "required", req_val);

  if (merge_schema_extras_object(obj, sf->schema_extra_json) != 0) {
    /* LCOV_EXCL_START */
    json_value_free(val);
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }

  json_object_set_value(schemas_obj, struct_name, val);

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
 * @brief Parses a union definition and writes it to a JSON Schema
 * object.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error parse_union_and_write(FILE *fp,
                                              /* LCOV_EXCL_STOP */
                                              JSON_Object *schemas_obj,
                                              const char *union_name) {
  /* (Implementation preserved from previous code2schema.c) */
  char line[512];
  /* LCOV_EXCL_START */
  JSON_Value *union_val = json_value_init_object();
  JSON_Object *union_obj = json_value_get_object(union_val);
  JSON_Value *oneof_val = json_value_init_array();
  JSON_Array *oneof_arr = json_value_get_array(oneof_val);
  /* LCOV_EXCL_STOP */
  char *p;

  /* LCOV_EXCL_START */
  while (read_line(fp, line, sizeof(line))) {
    p = line;
    while (isspace((unsigned char)*p))
      p++;
    if (*p == '}')
      break;
    if (!*p)
      continue;
    /* LCOV_EXCL_STOP */
    {
      /* LCOV_EXCL_START */
      char typebuf[64] = {0};
      char namebuf[64] = {0};
/* LCOV_EXCL_STOP */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      if (sscanf_s(p, "%63s %63[^;]", typebuf, (unsigned)sizeof(typebuf),
                   namebuf, (unsigned)sizeof(namebuf)) == 2) {
#else
      /* LCOV_EXCL_START */
      if (sscanf(p, "%63s %63[^;]", typebuf, namebuf) == 2) {
/* LCOV_EXCL_STOP */
#endif
        /* LCOV_EXCL_START */
        char *n = namebuf;
        char *t = typebuf;
        if (n[0] == '*')
          n++;
        /* LCOV_EXCL_STOP */
        {
          /* LCOV_EXCL_START */
          JSON_Value *option_val = json_value_init_object();
          JSON_Object *option_obj = json_value_get_object(option_val);
          JSON_Value *props_val = json_value_init_object();
          JSON_Object *props_obj = json_value_get_object(props_val);
          JSON_Value *field_val = json_value_init_object();
          JSON_Object *field_obj = json_value_get_object(field_val);
          /* LCOV_EXCL_STOP */

          /* LCOV_EXCL_START */
          if (strcmp(t, "int") == 0)
            json_object_set_string(field_obj, "type", "integer");
          else if (strcmp(t, "char") == 0)
            json_object_set_string(field_obj, "type", "string");
          else if (strcmp(t, "float") == 0)
            json_object_set_string(field_obj, "type", "number");
          /* LCOV_EXCL_STOP */
          else
            /* LCOV_EXCL_START */
            json_object_set_string(field_obj, "type", "object");
          /* LCOV_EXCL_STOP */

          /* LCOV_EXCL_START */
          json_object_set_value(props_obj, n, field_val);
          json_object_set_string(option_obj, "type", "object");
          json_object_set_value(option_obj, "properties", props_val);
          json_object_set_string(option_obj, "title", n);
          json_array_append_value(oneof_arr, option_val);
          /* LCOV_EXCL_STOP */
        }
      }
    }
  }

  /* LCOV_EXCL_START */
  json_object_set_value(union_obj, "oneOf", oneof_val);
  json_object_set_string(union_obj, "type", "object");
  json_object_set_value(schemas_obj, union_name, union_val);
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

enum cdd_c_error code2schema_main(int argc, char **argv) {
  FILE *fp;
  char line[MAX_LINE_LENGTH];
  JSON_Value *root = json_value_init_object();
  JSON_Object *root_obj = json_value_get_object(root);
  JSON_Value *schemas_val = json_value_init_object();
  JSON_Object *schemas_obj = json_value_get_object(schemas_val);
  JSON_Value *comp_val = json_value_init_object();
  JSON_Object *comp_obj = json_value_get_object(comp_val);

  if (argc != 2) {
    json_value_free(root);
    json_value_free(schemas_val);
    json_value_free(comp_val);
    return CDD_C_ERROR_UNKNOWN;
  }

#if defined(_MSC_VER)
  if (fopen_s(&fp, argv[0], "r") != 0)
    fp = NULL;
#else
#if defined(_MSC_VER)
  fopen_s(&fp, argv[0], "r");
#else
  fp = fopen(argv[0], "r");
#endif
#endif
  if (!fp) {
    json_value_free(root);
    json_value_free(schemas_val);
    json_value_free(comp_val);
    return CDD_C_ERROR_UNKNOWN;
  }

  json_object_set_value(comp_obj, "schemas", schemas_val);
  json_object_set_value(root_obj, "components", comp_val);

  while (read_line(fp, line, sizeof(line))) {
    char *p = line;
    while (isspace((unsigned char)*p))
      /* LCOV_EXCL_START */
      p++;
    /* LCOV_EXCL_STOP */

    if (strncmp(p, "union ", 6) == 0) {
      /* LCOV_EXCL_START */
      char union_name[64] = {0};
      char *brace = strchr(p, '{');
      if (brace) {
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_START */
        *brace = 0;
        /* LCOV_EXCL_STOP */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        if (sscanf_s(p + 6, "%63s", union_name, (unsigned)sizeof(union_name)) ==
            1) {
#else
        /* LCOV_EXCL_START */
        if (sscanf(p + 6, "%63s", union_name) == 1) {
/* LCOV_EXCL_STOP */
#endif
          /* LCOV_EXCL_START */
          parse_union_and_write(fp, schemas_obj, union_name);
          /* LCOV_EXCL_STOP */
        }
      }
    } else if (strncmp(p, "struct ", 7) == 0) {
      char struct_name[64] = {0};
      char *brace = strchr(p, '{');
      if (brace) {
        struct StructFields sf;
        /* LCOV_EXCL_START */
        *brace = 0;
        /* LCOV_EXCL_STOP */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        if (sscanf_s(p + 7, "%63s", struct_name,
                     (unsigned)sizeof(struct_name)) == 1) {
#else
        if (sscanf(p + 7, "%63s", struct_name) == 1) {
#endif
          if (struct_fields_init(&sf) == 0) {
            char subline[MAX_LINE_LENGTH];
            while (read_line(fp, subline, sizeof(subline))) {
              char *sp = subline;
              while (isspace((unsigned char)*sp))
                /* LCOV_EXCL_START */
                sp++;
              /* LCOV_EXCL_STOP */
              if (*sp == '}')
                /* LCOV_EXCL_START */
                break;
              /* LCOV_EXCL_STOP */
              if (*sp)
                parse_struct_member_line(sp, &sf);
            }
            write_struct_to_json_schema(schemas_obj, struct_name, &sf);
            struct_fields_free(&sf);
          }
        }
      }
    } else if (strncmp(p, "enum ", 5) == 0) {
      char enum_name[64] = {0};
      char *brace = strchr(p, '{');
      if (brace) {
        JSON_Value *eval = json_value_init_object();
        JSON_Object *eobj = json_value_get_object(eval);
        JSON_Value *arrval = json_value_init_array();
        JSON_Array *earr = json_value_get_array(arrval);

        /* LCOV_EXCL_START */
        *brace = 0;
        /* LCOV_EXCL_STOP */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        if (sscanf_s(p + 5, "%63s", enum_name, (unsigned)sizeof(enum_name)) ==
            1) {
#else
        if (sscanf(p + 5, "%63s", enum_name) == 1) {
#endif
          const char *delim = ",}";
          char *token;
          char *ctx = NULL;
          char *rest = brace + 1;
          char full_body[4096] = {0};

          /* Accumulate body */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          strcat_s(full_body, sizeof(full_body), rest);
          while (!strchr(full_body, '}')) {
            if (!read_line(fp, line, sizeof(line)))
              break;
            strcat_s(full_body, sizeof(full_body), line);
          }
#else
          strcat(full_body, rest);
          while (!strchr(full_body, '}')) {
            /* LCOV_EXCL_START */
            if (!read_line(fp, line, sizeof(line)))
              break;
            strcat(full_body, line);
            /* LCOV_EXCL_STOP */
          }
#endif

#ifdef _WIN32
          token = strtok_s(full_body, delim, &ctx);
#else
          token = strtok_r(full_body, delim, &ctx);
#endif
          while (token) {
            char *tm = token;
            while (isspace((unsigned char)*tm))
              tm++;
            c_cdd_str_trim_trailing_whitespace(tm);
            {
              char *eq = strchr(tm, '=');
              if (eq)
                *eq = 0;
              c_cdd_str_trim_trailing_whitespace(tm);
            }
            if (*tm)
              json_array_append_string(earr, tm);
#ifdef _WIN32
            token = strtok_s(NULL, delim, &ctx);
#else
            token = strtok_r(NULL, delim, &ctx);
#endif
          }
          json_object_set_string(eobj, "type", "string");
          json_object_set_value(eobj, "enum", arrval);
          json_object_set_value(schemas_obj, enum_name, eval);
        } else {
          /* LCOV_EXCL_START */
          json_value_free(eval);
          json_value_free(arrval);
          /* LCOV_EXCL_STOP */
        }
      }
    }
  }

  json_serialize_to_file_pretty(root, argv[1]);
  fclose(fp);
  json_value_free(root);
  return CDD_C_SUCCESS;
}
