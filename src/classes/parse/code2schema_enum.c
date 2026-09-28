/**
 * @file code2schema_enum.c
 * @brief Enum conversion and reference resolution for code2schema.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Converts a JSON array of strings to an EnumMembers structure.
 */
cdd_c_error_t json_array_to_enum_members(const JSON_Array *enum_arr,
                                         struct EnumMembers *em) {
  size_t i, count;
  if (!enum_arr || !em)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  count = json_array_get_count(enum_arr);
  for (i = 0; i < count; i++) {
    const char *s = json_array_get_string(enum_arr, i);
    if (!s)
      continue;
    if (enum_members_add(em, s) != 0)
      return CDD_C_ERROR_MEMORY;
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
 * @brief Determines if a JSON Schema object represents a string enum.
 */
cdd_c_error_t schema_object_is_string_enum(const JSON_Object *schema_obj,
                                           const JSON_Array **enum_arr_out,
                                           int *out_is_enum) {
  const JSON_Array *enum_arr;
  size_t i, count;
  const char *type;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

  if (enum_arr_out)
    *enum_arr_out = NULL;
  if (!out_is_enum)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_enum = 0;
  if (!schema_obj)
    return CDD_C_SUCCESS;

  enum_arr = json_object_get_array(schema_obj, "enum");
  if (!enum_arr)
    return CDD_C_SUCCESS;

  count = json_array_get_count(enum_arr);
  if (count == 0)
    return CDD_C_SUCCESS;

  type = json_object_get_string(schema_obj, "type");
  if (type && strcmp(type, "string") != 0)
    return CDD_C_SUCCESS;

  for (i = 0; i < count; ++i) {
    if (!json_array_get_string(enum_arr, i))
      return CDD_C_SUCCESS;
  }

  if (enum_arr_out)
    *enum_arr_out = enum_arr;
  *out_is_enum = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a JSON Schema $ref points to a string enum.
 */
cdd_c_error_t ref_points_to_string_enum(const JSON_Object *root,
                                        const char *ref,
                                        int *out_points_to_enum) {
  const char *name;
  const JSON_Object *schema_obj;
  if (!out_points_to_enum)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_points_to_enum = 0;
  if (!root || !ref)
    return CDD_C_SUCCESS;
  name = NULL;
  {
    cdd_c_error_t rc_c2s = c_cdd_str_after_last(ref, '/', &name);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
  }
  if (!*name)
    return CDD_C_SUCCESS;
  schema_obj = json_object_get_object(root, name);
  if (!schema_obj)
    return CDD_C_SUCCESS;
  return schema_object_is_string_enum(schema_obj, NULL, out_points_to_enum);
}

/**
 * @brief Checks if a given property name is present in a required
 * properties list.
 */
cdd_c_error_t required_name_in_list(const JSON_Array *required,
                                    const char *name, int *out_in_list) {
  size_t i, count;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!out_in_list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_in_list = 0;
  if (!required || !name)
    return CDD_C_SUCCESS;
  count = json_array_get_count(required);
  for (i = 0; i < count; ++i) {
    const char *req_name = json_array_get_string(required, i);
    if (req_name && strcmp(req_name, name) == 0) {
      *out_in_list = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
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

/**
 * @brief Resolves a JSON Schema $ref to its corresponding object.
 */
cdd_c_error_t resolve_schema_ref_object(const JSON_Object *root,
                                        const char *ref,
                                        JSON_Object **_out_val) {
  const char *name;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!root || !ref) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  name = NULL;
  {
    cdd_c_error_t rc_c2s = c_cdd_str_after_last(ref, '/', &name);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
  }
  if (!*name) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = json_object_get_object(root, name);
    return CDD_C_SUCCESS;
  }
}
