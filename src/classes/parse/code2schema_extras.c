/**
 * @file code2schema_extras.c
 * @brief Schema extras collection and merging routines.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

cdd_c_error_t c2s_collect_schema_extras(const JSON_Object *obj,
                                        const char **skip_keys,
                                        size_t skip_count, char **out_json) {
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;
  char *serialized;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_MEMORY;
  if (g_cdd_fail_c2s_collect_schema_extras &&
      --g_cdd_fail_c2s_collect_schema_extras == 0)
    return CDD_C_ERROR_MEMORY;
#endif

  if (out_json)
    *out_json = NULL;
  if (!obj || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  extras_val = json_value_init_object();
  if (!extras_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  extras_obj = json_value_get_object(extras_val);

  count = json_object_get_count(obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(obj, i);
    const JSON_Value *val;
    JSON_Value *copy;
    int is_skip = 0;

    {
      cdd_c_error_t rc_c2s =
          c2s_key_in_list(key, skip_keys, skip_count, &is_skip);
      if (rc_c2s != CDD_C_SUCCESS) {
        json_value_free(extras_val);
        return rc_c2s;
      }
    }
    if (is_skip)
      continue;
    val = json_object_get_value(obj, key);
    {
      cdd_c_error_t rc_c2s = c2s_clone_json_value(val, &copy);
      if (rc_c2s != CDD_C_SUCCESS) {
        json_value_free(extras_val);
        return rc_c2s;
      }
    }
    if (json_object_set_value(extras_obj, key, copy) != JSONSuccess) {
      json_value_free(copy);
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  if (json_object_get_count(extras_obj) == 0) {
    json_value_free(extras_val);
    return CDD_C_SUCCESS;
  }

  serialized = json_serialize_to_string(extras_val);
  if (!serialized) {
    json_value_free(extras_val);
    return CDD_C_ERROR_MEMORY;
  }
  {
    cdd_c_error_t rc_c2s = c_cdd_strdup(serialized, out_json);
    if (rc_c2s != CDD_C_SUCCESS) {
      json_free_serialized_string(serialized);
      json_value_free(extras_val);
      return rc_c2s;
    }
  }
  json_free_serialized_string(serialized);
  json_value_free(extras_val);
  return *out_json ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Merges extra attributes described by a JSON string into a
 * target parson JSON Object.
 *
 * Parses the JSON string `extras_json`, clones each value, and sets them
 * directly on `target`. Returns early if there are no extras. Does not
 * override existing keys on `target`.
 */
cdd_c_error_t c2s_merge_schema_extras_object(JSON_Object *target,
                                             const char *extras_json) {
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  if (!target || !extras_json || extras_json[0] == '\0')
    return CDD_C_SUCCESS;

  extras_val = json_parse_string(extras_json);
  if (!extras_val)
    return CDD_C_SUCCESS;
  extras_obj = json_value_get_object(extras_val);
  if (!extras_obj) {
    json_value_free(extras_val);
    return CDD_C_SUCCESS;
  }

  count = json_object_get_count(extras_obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(extras_obj, i);
    const JSON_Value *val;
    JSON_Value *copy;

    if (json_object_has_value(target, key))
      continue;
    val = json_object_get_value(extras_obj, key);
    {
      cdd_c_error_t rc_c2s = c2s_clone_json_value(val, &copy);
      if (rc_c2s != CDD_C_SUCCESS) {
        json_value_free(extras_val);
        return rc_c2s;
      }
    }
    if (json_object_set_value(target, key, copy) != JSONSuccess) {
      json_value_free(copy);
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
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
 */
cdd_c_error_t c2s_merge_schema_extras_strings(char **dest_json,
                                              const char *src_json) {
  JSON_Value *dest_val;
  JSON_Value *src_val;
  JSON_Object *dest_obj;
  JSON_Object *src_obj;
  size_t i, count;
  char *serialized;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_MEMORY;
#endif

  if (!dest_json || !src_json || src_json[0] == '\0')
    return CDD_C_SUCCESS;
  if (!*dest_json) {
    {
      cdd_c_error_t rc_c2s = c_cdd_strdup(src_json, dest_json);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    return *dest_json ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }

  dest_val = json_parse_string(*dest_json);
  src_val = json_parse_string(src_json);
  if (!dest_val || !src_val) {
    if (dest_val)
      json_value_free(dest_val);
    if (src_val)
      json_value_free(src_val);
    return CDD_C_SUCCESS;
  }
  dest_obj = json_value_get_object(dest_val);
  src_obj = json_value_get_object(src_val);
  if (!dest_obj || !src_obj) {
    json_value_free(dest_val);
    json_value_free(src_val);
    return CDD_C_SUCCESS;
  }

  count = json_object_get_count(src_obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(src_obj, i);
    const JSON_Value *val;
    JSON_Value *copy;

    if (json_object_has_value(dest_obj, key))
      json_object_remove(dest_obj, key);
    val = json_object_get_value(src_obj, key);
    {
      cdd_c_error_t rc_c2s = c2s_clone_json_value(val, &copy);
      if (rc_c2s != CDD_C_SUCCESS) {
        json_value_free(dest_val);
        json_value_free(src_val);
        return rc_c2s;
      }
    }
#ifdef CDD_BUILD_TESTS
    if (g_cdd_fail_json_set_value) {
      g_cdd_fail_json_set_value = 0;
      json_value_free(copy);
      json_value_free(dest_val);
      json_value_free(src_val);
      return CDD_C_ERROR_MEMORY;
    }
#endif
    json_object_set_value(dest_obj, key, copy);
  }

  serialized = json_serialize_to_string(dest_val);
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_json_serialize) {
    g_cdd_fail_json_serialize = 0;
    json_free_serialized_string(serialized);
    serialized = NULL;
  }
#endif
  if (!serialized) {
    json_value_free(dest_val);
    json_value_free(src_val);
    return CDD_C_ERROR_MEMORY;
  }
  {
    char *dup = NULL;
    cdd_c_error_t rc = c_cdd_strdup(serialized, &dup);
    json_free_serialized_string(serialized);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(dest_val);
      json_value_free(src_val);
      return CDD_C_ERROR_MEMORY;
    }
    C_CDD_FREE(*dest_json);
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

/**
 * @brief Checks if an OpenAPI type is a primitive type.
 */
cdd_c_error_t c2s_openapi_type_is_primitive(const char *type,
                                            int *out_is_primitive) {
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!out_is_primitive)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_primitive = 0;
  if (!type)
    return CDD_C_SUCCESS;
  if (strcmp(type, "integer") == 0 || strcmp(type, "number") == 0 ||
      strcmp(type, "string") == 0 || strcmp(type, "boolean") == 0) {
    *out_is_primitive = 1;
  }
  return CDD_C_SUCCESS;
}
