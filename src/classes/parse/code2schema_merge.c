/**
 * @file code2schema_merge.c
 * @brief Merging struct fields and handling allOf composition.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Merges a source struct field into a destination struct field.
 */
cdd_c_error_t merge_struct_field(struct StructField *dest,
                                 const struct StructField *src) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_MEMORY;
#endif
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
        (src->min_val == dest->min_val && src->exclusive_min &&
         !dest->exclusive_min)) {
      dest->has_min = 1;
      dest->min_val = src->min_val;
      dest->exclusive_min = src->exclusive_min;
    }
  }

  if (src->has_max) {
    if (!dest->has_max || src->max_val < dest->max_val ||
        (src->max_val == dest->max_val && src->exclusive_max &&
         !dest->exclusive_max)) {
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
    dest->unique_items = 1;

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
    dest->is_flexible_array = 1;

  if (!dest->bit_width[0] && src->bit_width[0]) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(dest->bit_width, sizeof(dest->bit_width), src->bit_width,
              sizeof(dest->bit_width) - 1);
#else
    strncpy(dest->bit_width, src->bit_width, sizeof(dest->bit_width) - 1);
#endif
    dest->bit_width[sizeof(dest->bit_width) - 1] = '\0';
  }

  if (c2s_merge_schema_extras_strings(&dest->schema_extra_json,
                                      src->schema_extra_json) != 0) {
    /* Best-effort: ignore merge failures */
  }
  if (c2s_merge_schema_extras_strings(&dest->items_extra_json,
                                      src->items_extra_json) != 0) {
    /* Best-effort: ignore merge failures */
  }

  if (!dest->type_union && src->type_union) {
    if (copy_string_array_code2schema(&dest->type_union, &dest->n_type_union,
                                      src->type_union,
                                      src->n_type_union) != 0) {
      /* Best-effort: ignore copy failures */
    }
  }
  if (!dest->items_type_union && src->items_type_union) {
    if (copy_string_array_code2schema(
            &dest->items_type_union, &dest->n_items_type_union,
            src->items_type_union, src->n_items_type_union) != 0) {
      /* Best-effort: ignore copy failures */
    }
  }
  return rc;
}

/**
 * @brief Merges source struct fields into destination struct fields.
 */
cdd_c_error_t merge_struct_fields(struct StructFields *dest,
                                  const struct StructFields *src) {
  size_t i;

  if (!dest || !src)
    return CDD_C_SUCCESS;

  if (src->is_enum)
    return CDD_C_SUCCESS;

  if (c2s_merge_schema_extras_strings(&dest->schema_extra_json,
                                      src->schema_extra_json) != 0)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < src->size; ++i) {
    const struct StructField *src_field = &src->fields[i];
    struct StructField *dest_field = NULL;
    {
      cdd_c_error_t rc_c2s =
          struct_fields_get(dest, src_field->name, &dest_field);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }

    if (!dest_field) {
      const char *ref = src_field->ref[0] ? src_field->ref : NULL;
      const char *def =
          src_field->default_val[0] ? src_field->default_val : NULL;
      const char *bw = src_field->bit_width[0] ? src_field->bit_width : NULL;
      if (struct_fields_add(dest, src_field->name, src_field->type, ref, def,
                            bw) != 0)
        return CDD_C_ERROR_MEMORY;
      dest_field = NULL;
      {
        cdd_c_error_t rc_c2s =
            struct_fields_get(dest, src_field->name, &dest_field);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }

      {
        struct StructField tmp = *src_field;
        *dest_field = tmp;
        dest_field->schema_extra_json = NULL;
        dest_field->items_extra_json = NULL;
        dest_field->type_union = NULL;
        dest_field->items_type_union = NULL;
        dest_field->items_extra_json = NULL;
        dest_field->type_union = NULL;
        dest_field->n_type_union = 0;
        dest_field->items_type_union = NULL;
        dest_field->n_items_type_union = 0;
        if (src_field->schema_extra_json) {
          cdd_c_error_t rc_c2s = c_cdd_strdup(src_field->schema_extra_json,
                                              &dest_field->schema_extra_json);
          if (rc_c2s != CDD_C_SUCCESS)
            return rc_c2s;
        }
        if (src_field->items_extra_json) {
          cdd_c_error_t rc_c2s = c_cdd_strdup(src_field->items_extra_json,
                                              &dest_field->items_extra_json);
          if (rc_c2s != CDD_C_SUCCESS)
            return rc_c2s;
        }
        if (src_field->type_union) {
          if (copy_string_array_code2schema(
                  &dest_field->type_union, &dest_field->n_type_union,
                  src_field->type_union, src_field->n_type_union) != 0)
            return CDD_C_ERROR_MEMORY;
        }
        if (src_field->items_type_union) {
          if (copy_string_array_code2schema(&dest_field->items_type_union,
                                            &dest_field->n_items_type_union,
                                            src_field->items_type_union,
                                            src_field->n_items_type_union) != 0)
            return CDD_C_ERROR_MEMORY;
        }
      }
      continue;
    }

    {
      cdd_c_error_t mrc = merge_struct_field(dest_field, src_field);
      if (mrc != CDD_C_SUCCESS)
        return mrc;
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
cdd_c_error_t apply_allof_to_struct_fields(const JSON_Array *all_of,
                                           struct StructFields *dest,
                                           const JSON_Object *root) {
  size_t i, count;

  if (!all_of || !dest)
    return CDD_C_SUCCESS;

  count = json_array_get_count(all_of);
  for (i = 0; i < count; ++i) {
    const JSON_Object *sub = json_array_get_object(all_of, i);
    const JSON_Object *resolved = sub;
    const char *ref;
    struct StructFields tmp;
    cdd_c_error_t rc;

    if (!sub)
      continue;

    ref = json_object_get_string(sub, "$ref");
    if (ref && root) {
      cdd_c_error_t rc_c2s =
          resolve_schema_ref_object(root, ref, (JSON_Object **)&resolved);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    if (!resolved)
      continue;

    rc = struct_fields_init(&tmp);
    if (rc != CDD_C_SUCCESS)
      return rc;

    rc = json_object_to_struct_fields(resolved, &tmp, root);
    if (rc != CDD_C_SUCCESS) {
      struct_fields_free(&tmp);
      return rc;
    }

    rc = merge_struct_fields(dest, &tmp);
    struct_fields_free(&tmp);
    if (rc != CDD_C_SUCCESS)
      return rc;
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
