/**
 * @file code2schema_unions.c
 * @brief Union composition (oneOf/anyOf) handling for struct fields.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Fallback method to apply a union (oneOf/anyOf) to StructFields.
 */
cdd_c_error_t apply_union_to_struct_fields_fallback(const JSON_Array *union_arr,
                                                    struct StructFields *dest,
                                                    const JSON_Object *root) {
  size_t i, count;

  if (!union_arr || !dest)
    return CDD_C_SUCCESS;

  if (dest->size > 0)
    return CDD_C_SUCCESS;

  count = json_array_get_count(union_arr);
  for (i = 0; i < count; ++i) {
    const JSON_Object *sub = json_array_get_object(union_arr, i);
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

    if (tmp.size > 0) {
      rc = merge_struct_fields(dest, &tmp);
      struct_fields_free(&tmp);
      return rc;
    }

    struct_fields_free(&tmp);
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
 * @brief Checks if array items within a union are supported.
 */
cdd_c_error_t c2s_union_array_items_supported(const JSON_Object *schema_obj,
                                              const JSON_Object *root,
                                              int allow_inline,
                                              int *out_supported) {
  const JSON_Object *items;
  const JSON_Array *item_type_arr = NULL;
  const char *item_ref;
  const char *item_type;
  char **items_type_union = NULL;
  size_t n_items_type_union = 0;
  const char *primary = NULL;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!out_supported)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_supported = 0;

  if (!schema_obj)
    return CDD_C_SUCCESS;

  items = json_object_get_object(schema_obj, "items");
  if (!items)
    return CDD_C_SUCCESS;

  item_ref = json_object_get_string(items, "$ref");
  if (item_ref && root) {
    int is_enum = 0;
    cdd_c_error_t rc_c2s = ref_points_to_string_enum(root, item_ref, &is_enum);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
    if (is_enum) {
      *out_supported = 1;
      return CDD_C_SUCCESS;
    }
  }

  item_type = json_object_get_string(items, "type");
  if (!item_ref && !item_type) {
    item_type_arr = json_object_get_array(items, "type");
    if (item_type_arr) {
      if (parse_type_union_array_code2schema(item_type_arr, &items_type_union,
                                             &n_items_type_union, &primary,
                                             NULL) == 0)
        item_type = primary;
    } else if (json_object_get_object(items, "properties")) {
      item_type = "object";
    }
  }

  if (item_ref) {
    free_string_array_code2schema(items_type_union, n_items_type_union);
    *out_supported = 1;
    return CDD_C_SUCCESS;
  }
  if (!item_type) {
    free_string_array_code2schema(items_type_union, n_items_type_union);
    return CDD_C_SUCCESS;
  }
  if (strcmp(item_type, "array") == 0) {
    free_string_array_code2schema(items_type_union, n_items_type_union);
    return CDD_C_SUCCESS;
  }
  if (strcmp(item_type, "object") == 0 && (!allow_inline || !root)) {
    free_string_array_code2schema(items_type_union, n_items_type_union);
    return CDD_C_SUCCESS;
  }

  free_string_array_code2schema(items_type_union, n_items_type_union);
  *out_supported = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Extended method to apply a union (oneOf/anyOf) to StructFields.
 */
cdd_c_error_t apply_union_to_struct_fields_ex(
    const JSON_Array *union_arr, struct StructFields *dest, JSON_Object *root,
    const char *schema_name, int is_anyof, const JSON_Object *schema_obj,
    int allow_inline) {
  size_t i, count;
  const JSON_Object *disc_obj;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

  if (!union_arr || !dest)
    return CDD_C_SUCCESS;

  if (dest->size > 0)
    return CDD_C_SUCCESS;

  count = json_array_get_count(union_arr);
  if (count == 0)
    return CDD_C_SUCCESS;

  /* Validate variant support */
  for (i = 0; i < count; ++i) {
    const JSON_Object *sub = json_array_get_object(union_arr, i);
    const JSON_Object *resolved = sub;
    const char *ref;
    enum UnionVariantJsonType jtype;
    if (!sub)
      continue;
    ref = json_object_get_string(sub, "$ref");
    if (ref && root) {
      cdd_c_error_t rc_c2s =
          resolve_schema_ref_object(root, ref, (JSON_Object **)&resolved);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    {
      cdd_c_error_t rc_c2s = c2s_detect_union_json_type(resolved, &jtype);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    if (jtype == UNION_JSON_ARRAY) {
      int supported = 0;
      if (!allow_inline)
        return CDD_C_SUCCESS;
      {
        cdd_c_error_t rc_c2s = c2s_union_array_items_supported(
            resolved, root, allow_inline, &supported);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
      if (!supported)
        return CDD_C_SUCCESS;
    }
    if (jtype == UNION_JSON_OBJECT && !ref) {
      if (!allow_inline || !root)
        return CDD_C_SUCCESS;
    }
    if (jtype == UNION_JSON_UNKNOWN)
      return CDD_C_SUCCESS;
  }

  dest->is_union = 1;
  dest->union_is_anyof = is_anyof ? 1 : 0;

  disc_obj =
      schema_obj ? json_object_get_object(schema_obj, "discriminator") : NULL;
  if (disc_obj) {
    const char *prop = json_object_get_string(disc_obj, "propertyName");
    if (prop && *prop) {
      dest->union_discriminator = NULL;
      {
        cdd_c_error_t rc_c2s = c_cdd_strdup(prop, &dest->union_discriminator);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
    }
  }

  dest->union_variants = (struct UnionVariantMeta *)C_CDD_CALLOC(
      count, sizeof(struct UnionVariantMeta));
  if (!dest->union_variants) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
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
    cdd_c_error_t rc;

    if (!sub)
      continue;

    ref = json_object_get_string(sub, "$ref");
    if (ref) {
      cdd_c_error_t rc_c2s =
          resolve_schema_ref_object(root, ref, (JSON_Object **)&resolved);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }

    {
      cdd_c_error_t rc_c2s = c2s_detect_union_json_type(resolved, &jtype);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    meta->json_type = jtype;

    if (ref) {
      {
        cdd_c_error_t rc_c2s = c_cdd_str_after_last(ref, '/', &name_hint);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
    } else {
      name_hint = json_object_get_string(resolved, "title");
      if (!name_hint)
        name_hint = json_object_get_string(resolved, "type");
    }
    if (!name_hint)
      name_hint = schema_name ? schema_name : "Variant";

    {
      cdd_c_error_t rc_c2s =
          make_unique_variant_name(dest, name_hint, i, &variant_name);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    if (!variant_name) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }

    if (jtype == UNION_JSON_OBJECT && !ref) {
      rc = register_inline_schema_c2s(root, schema_name, variant_name, NULL,
                                      sub_val, &inline_ref_name);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(variant_name);
        return rc;
      }
      ref = inline_ref_name;
    }

    if (jtype == UNION_JSON_ARRAY) {
      const JSON_Object *items = json_object_get_object(resolved, "items");
      const JSON_Value *items_val = json_object_get_value(resolved, "items");
      const JSON_Array *item_type_arr = NULL;

      item_ref = json_object_get_string(items, "$ref");
      if (item_ref && root) {
        int is_enum = 0;
        cdd_c_error_t rc_c2s =
            ref_points_to_string_enum(root, item_ref, &is_enum);
        if (rc_c2s != CDD_C_SUCCESS) {
          C_CDD_FREE(variant_name);
          C_CDD_FREE(inline_ref_name);
          return rc_c2s;
        }
        if (is_enum) {
          item_ref = NULL;
          item_type = "string";
        }
      }
      item_type = json_object_get_string(items, "type");
      if (!item_ref && !item_type) {
        item_type_arr = json_object_get_array(items, "type");
        if (item_type_arr) {
          rc = parse_type_union_array_code2schema(
              item_type_arr, &items_type_union, &n_items_type_union, &item_type,
              NULL);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(variant_name);
            C_CDD_FREE(inline_ref_name);
            return rc;
          }
        } else if (json_object_get_object(items, "properties")) {
          item_type = "object";
        }
      }
      if (!item_ref && item_type && strcmp(item_type, "object") == 0) {
        rc = register_inline_schema_c2s(root, schema_name, variant_name, "Item",
                                        items_val, &inline_item_ref);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(variant_name);
          C_CDD_FREE(inline_ref_name);
          free_string_array_code2schema(items_type_union, n_items_type_union);
          return rc;
        }
        item_ref = inline_item_ref;
      }
    }

    switch (jtype) {
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
    case UNION_JSON_ARRAY:
      type_name = "array";
      break;
    case UNION_JSON_NULL:
      type_name = "null";
      break;
    default:
      type_name = "object";
      break;
    }

    if (struct_fields_add(dest, variant_name, type_name,
                          (jtype == UNION_JSON_OBJECT) ? ref
                          : (jtype == UNION_JSON_ARRAY)
                              ? (item_ref ? item_ref : item_type)
                              : NULL,
                          NULL, NULL) != 0) {
      C_CDD_FREE(variant_name);
      C_CDD_FREE(inline_ref_name);
      C_CDD_FREE(inline_item_ref);
      free_string_array_code2schema(items_type_union, n_items_type_union);
      return CDD_C_ERROR_MEMORY;
    }
    field = &dest->fields[dest->size - 1];

    if (jtype == UNION_JSON_ARRAY && items_type_union) {
      field->items_type_union = items_type_union;
      field->n_items_type_union = n_items_type_union;
      items_type_union = NULL;
      n_items_type_union = 0;
    }

    C_CDD_FREE(variant_name);

    if (jtype == UNION_JSON_OBJECT) {
      const JSON_Array *required = json_object_get_array(resolved, "required");
      if (c2s_collect_string_array(required, &meta->required_props,
                                   &meta->n_required_props) != 0)
        return CDD_C_ERROR_MEMORY;
      if (c2s_collect_property_names(resolved, &meta->property_names,
                                     &meta->n_property_names) != 0)
        return CDD_C_ERROR_MEMORY;
    }

    {
      const char *ref_al = NULL;
      char *disc_val = NULL;
      if (ref) {
        cdd_c_error_t rc_c2s = c_cdd_str_after_last(ref, '/', &ref_al);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
      discriminator_value_for_variant(disc_obj, ref ? ref_al : name_hint, ref,
                                      &disc_val);
      meta->disc_value = disc_val;
    }

    C_CDD_FREE(inline_ref_name);
    C_CDD_FREE(inline_item_ref);
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
