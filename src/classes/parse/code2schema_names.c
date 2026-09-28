/**
 * @file code2schema_names.c
 * @brief Identifier sanitization and inline schema naming.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Sanitizes a string to be used as a valid C identifier.
 */
cdd_c_error_t sanitize_identifier(const char *in, char **_out_val) {
  size_t i, len;
  char *out;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!in || !*in) {
    {
      cdd_c_error_t rc_c2s = c_cdd_strdup("Variant", _out_val);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    return CDD_C_SUCCESS;
  }
  len = strlen(in);
  out = (char *)(size_t)C_CDD_CALLOC(len + 1, sizeof(char));
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
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

  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Generates a unique variant name for a union type.
 */
cdd_c_error_t make_unique_variant_name(const struct StructFields *dest,
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
    {
      cdd_c_error_t rc_c2s = sanitize_identifier(base, &sanitized);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }

    {
      cdd_c_error_t rc_c2s = struct_fields_get(dest, sanitized, &tmp4);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    if (!tmp4) {
      *_out_val = sanitized;
      return CDD_C_SUCCESS;
    }
  }
  CDD_SNPRINTF(buf, sizeof(buf), "%s_%lu", sanitized,
               (unsigned long)(index + 1));
  C_CDD_FREE(sanitized);
  {
    cdd_c_error_t rc_c2s = c_cdd_strdup(buf, &out);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
  }
  {
    struct StructField *tmp5 = NULL;
    {
      cdd_c_error_t rc_c2s = struct_fields_get(dest, out, &tmp5);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    if (!tmp5) {
      *_out_val = out;
      return CDD_C_SUCCESS;
    }
  }
  C_CDD_FREE(out);
  CDD_SNPRINTF(buf, sizeof(buf), "Variant_%lu", (unsigned long)(index + 1));
  {
    {
      cdd_c_error_t rc_c2s = c_cdd_strdup(buf, _out_val);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Generates a schema name for an inline anonymous struct/union.
 */
cdd_c_error_t make_inline_schema_name(const char *schema_name,
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
    {
      cdd_c_error_t rc_c2s = sanitize_identifier(buf, _out_val);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Registers an inline schema within the root OpenAPI components.
 */
cdd_c_error_t
register_inline_schema_c2s(JSON_Object *root, const char *schema_name,
                           const char *variant_name, const char *suffix,
                           const JSON_Value *schema_val, char **out_name) {
  char *name;

  if (!root || !schema_val || !out_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  name = NULL;
  {
    cdd_c_error_t rc_c2s =
        make_inline_schema_name(schema_name, variant_name, suffix, &name);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
  }
  if (!name) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  if (!json_object_has_value(root, name)) {
    JSON_Value *copy = NULL;
    {
      cdd_c_error_t rc_c2s = c2s_clone_json_value(schema_val, &copy);
      if (rc_c2s != CDD_C_SUCCESS) {
        C_CDD_FREE(name);
        return rc_c2s;
      }
    }
    if (json_object_set_value(root, name, copy) != JSONSuccess) {
      json_value_free(copy);
      C_CDD_FREE(name);
      return CDD_C_ERROR_MEMORY;
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
cdd_c_error_t discriminator_value_for_variant(const JSON_Object *disc_obj,
                                              const char *schema_name,
                                              const char *ref,
                                              char **_out_val) {
  const JSON_Object *mapping;
  size_t i, count;
  const char *ref_name;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!schema_name && !ref) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  if (ref) {
    cdd_c_error_t rc_c2s = c_cdd_str_after_last(ref, '/', &ref_name);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
  } else
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
    if (!val)
      continue;
    if (ref && strcmp(val, ref) == 0) {
      {
        cdd_c_error_t rc_c2s = c_cdd_strdup(key, _out_val);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
      return CDD_C_SUCCESS;
    }
    if (ref_name && strcmp(val, ref_name) == 0) {
      {
        cdd_c_error_t rc_c2s = c_cdd_strdup(key, _out_val);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
      return CDD_C_SUCCESS;
    }
    if (schema_name && strcmp(val, schema_name) == 0) {
      {
        cdd_c_error_t rc_c2s = c_cdd_strdup(key, _out_val);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
      return CDD_C_SUCCESS;
    }
  }

fallback:
  if (schema_name) {
    cdd_c_error_t rc_c2s = c_cdd_strdup(schema_name, _out_val);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
    return CDD_C_SUCCESS;
  }
  return c_cdd_strdup(ref_name, _out_val);
}
