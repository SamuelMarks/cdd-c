/**
 * @file code2schema_mapper.c
 * @brief Mapping JSON schema properties to struct fields.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

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
 * @brief Internal helper to convert a JSON Schema object to
 * StructFields.
 */
cdd_c_error_t c2s_json_object_to_struct_fields_internal(
    const JSON_Object *o, struct StructFields *f, JSON_Object *root,
    const char *schema_name, int allow_inline_union) {
  JSON_Object *props;
  size_t i, count;
  const JSON_Array *required;
  const JSON_Array *enum_arr;
  const JSON_Array *all_of;
  const JSON_Array *any_of;
  const JSON_Array *one_of;
  cdd_c_error_t rc;

  if (!o || !f)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (c2s_collect_schema_extras(o, k_schema_skip_keys,
                                sizeof(k_schema_skip_keys) /
                                    sizeof(k_schema_skip_keys[0]),
                                &f->schema_extra_json) != 0)
    return CDD_C_ERROR_MEMORY;

  {
    int is_enum = 0;
    cdd_c_error_t rc_c2s = schema_object_is_string_enum(o, &enum_arr, &is_enum);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
    if (is_enum) {
      f->is_enum = 1;
      if (enum_members_init(&f->enum_members) != 0)
        return CDD_C_ERROR_MEMORY;
      if (json_array_to_enum_members(enum_arr, &f->enum_members) != 0)
        return CDD_C_ERROR_MEMORY;
      return CDD_C_SUCCESS;
    }
  }

  all_of = json_object_get_array(o, "allOf");
  if (all_of) {
    rc = apply_allof_to_struct_fields(all_of, f, root);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  any_of = json_object_get_array(o, "anyOf");
  if (any_of) {
    rc = apply_union_to_struct_fields_ex(any_of, f, root, schema_name, 1, o,
                                         allow_inline_union);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!f->is_union) {
      rc = apply_union_to_struct_fields_fallback(any_of, f, root);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  one_of = json_object_get_array(o, "oneOf");
  if (one_of) {
    rc = apply_union_to_struct_fields_ex(one_of, f, root, schema_name, 0, o,
                                         allow_inline_union);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!f->is_union) {
      rc = apply_union_to_struct_fields_fallback(one_of, f, root);
      if (rc != CDD_C_SUCCESS)
        return rc;
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
      CDD_SNPRINTF(default_buf, sizeof(default_buf), "\"%s\"", s);
      d_val = default_buf;
    } else if (json_object_has_value_of_type(prop, "default", JSONNumber)) {
      double d = json_object_get_number(prop, "default");
      if (type && strcmp(type, "integer") == 0)
        CDD_SNPRINTF(default_buf, sizeof(default_buf), "%d", (int)d);
      else
        CDD_SNPRINTF(default_buf, sizeof(default_buf), "%f", d);
      d_val = default_buf;
    } else if (json_object_has_value_of_type(prop, "default", JSONBoolean)) {
      int b = json_object_get_boolean(prop, "default");
      CDD_SNPRINTF(default_buf, sizeof(default_buf), "%d", b);
      d_val = default_buf;
    }

    if (!type) {
      type_arr = json_object_get_array(prop, "type");
      if (type_arr) {
        const char *primary = NULL;
        rc = parse_type_union_array_code2schema(type_arr, &type_union,
                                                &n_type_union, &primary, NULL);
        if (rc != CDD_C_SUCCESS) {
          free_string_array_code2schema(type_union, n_type_union);
          return rc;
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
        if (item_ref && root) {
          int is_enum = 0;
          cdd_c_error_t rc_c2s =
              ref_points_to_string_enum(root, item_ref, &is_enum);
          if (rc_c2s != CDD_C_SUCCESS)
            return rc_c2s;
          if (is_enum) {
            /* Enum arrays are not strongly typed yet; treat as string arrays */
            item_ref = NULL;
            item_type = "string";
          }
        }
        if (!item_ref && !item_type && items) {
          item_type_arr = json_object_get_array(items, "type");
          if (item_type_arr) {
            const char *primary = NULL;
            rc = parse_type_union_array_code2schema(
                item_type_arr, &items_type_union, &n_items_type_union, &primary,
                NULL);
            if (rc != CDD_C_SUCCESS) {
              free_string_array_code2schema(type_union, n_type_union);
              free_string_array_code2schema(items_type_union,
                                            n_items_type_union);
              return rc;
            }
            item_type = primary;
          }
        }
        if (struct_fields_add(f, name, "array", item_ref ? item_ref : item_type,
                              NULL, bw) != 0) {
          free_string_array_code2schema(type_union, n_type_union);
          free_string_array_code2schema(items_type_union, n_items_type_union);
          return CDD_C_ERROR_MEMORY;
        }
        field = &f->fields[f->size - 1];
        field_added = 1;
        if (type_union) {
          field->type_union = type_union;
          field->n_type_union = n_type_union;
          type_union = NULL;
          n_type_union = 0;
        }
        if (items_type_union) {
          field->items_type_union = items_type_union;
          field->n_items_type_union = n_items_type_union;
          items_type_union = NULL;
          n_items_type_union = 0;
        }
        if (items) {
          if (c2s_collect_schema_extras(items, k_items_skip_keys,
                                        sizeof(k_items_skip_keys) /
                                            sizeof(k_items_skip_keys[0]),
                                        &field->items_extra_json) != 0)
            return CDD_C_ERROR_MEMORY;
        }
        free_string_array_code2schema(items_type_union, n_items_type_union);
      } else {
        if (struct_fields_add(f, name, type, ref, d_val, bw) != 0) {
          free_string_array_code2schema(type_union, n_type_union);
          return CDD_C_ERROR_MEMORY;
        }
        field = &f->fields[f->size - 1];
        field_added = 1;
        if (type_union) {
          field->type_union = type_union;
          field->n_type_union = n_type_union;
          type_union = NULL;
          n_type_union = 0;
        }
      }
    } else if (ref) {
      int is_enum = 0;
      const char *field_type;
      if (root) {
        cdd_c_error_t rc_c2s = ref_points_to_string_enum(root, ref, &is_enum);
        if (rc_c2s != CDD_C_SUCCESS) {
          free_string_array_code2schema(type_union, n_type_union);
          return rc_c2s;
        }
      }
      field_type = is_enum ? "enum" : "object";
      if (struct_fields_add(f, name, field_type, ref, NULL, bw) != 0) {
        free_string_array_code2schema(type_union, n_type_union);
        return CDD_C_ERROR_MEMORY;
      }
      field = &f->fields[f->size - 1];
      field_added = 1;
    }

    if (!field_added) {
      free_string_array_code2schema(type_union, n_type_union);
      continue;
    }

    {
      int is_req = 0;
      cdd_c_error_t rc_c2s = required_name_in_list(required, name, &is_req);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
      if (is_req)
        field->required = 1;
    }
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
                                               JSONBoolean) &&
                 json_object_get_boolean(prop, "exclusiveMinimum")) {
        field->exclusive_min = 1;
      }

      if (json_object_has_value_of_type(prop, "maximum", JSONNumber)) {
        field->has_max = 1;
        field->max_val = json_object_get_number(prop, "maximum");
      }

      if (json_object_has_value_of_type(prop, "exclusiveMaximum", JSONNumber)) {
        field->has_max = 1;
        field->max_val = json_object_get_number(prop, "exclusiveMaximum");
        field->exclusive_max = 1;
      } else if (json_object_has_value_of_type(prop, "exclusiveMaximum",
                                               JSONBoolean) &&
                 json_object_get_boolean(prop, "exclusiveMaximum")) {
        field->exclusive_max = 1;
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

    if (c2s_collect_schema_extras(prop, k_property_skip_keys,
                                  sizeof(k_property_skip_keys) /
                                      sizeof(k_property_skip_keys[0]),
                                  &field->schema_extra_json) != 0)
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

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_json_object_to_struct_fields_fail = 0;
#endif

/**
 * @brief Extended function to convert a JSON Schema object to StructFields.
 */
cdd_c_error_t json_object_to_struct_fields_ex(
    const JSON_Object *schema_obj, struct StructFields *fields,
    const JSON_Object *schemas_obj_root, const char *schema_name) {
#ifdef CDD_BUILD_TESTS
  if (g_json_object_to_struct_fields_fail &&
      --g_json_object_to_struct_fields_fail == 0) {
    return CDD_C_ERROR_MEMORY;
  }
#endif
  return c2s_json_object_to_struct_fields_internal(
      schema_obj, fields, (JSON_Object *)schemas_obj_root, schema_name, 0);
}

/**
 * @brief Extended code generation function to convert JSON Schema to
 * StructFields.
 */
cdd_c_error_t json_object_to_struct_fields_ex_codegen(
    const JSON_Object *schema_obj, struct StructFields *fields,
    JSON_Object *schemas_obj_root, const char *schema_name) {
#ifdef CDD_BUILD_TESTS
  if (g_json_object_to_struct_fields_fail &&
      --g_json_object_to_struct_fields_fail == 0) {
    return CDD_C_ERROR_MEMORY;
  }
#endif
  return c2s_json_object_to_struct_fields_internal(
      schema_obj, fields, schemas_obj_root, schema_name, 1);
}

/**
 * @brief Converts a JSON Schema object to a StructFields structure.
 */
cdd_c_error_t
json_object_to_struct_fields(const JSON_Object *schema_obj,
                             struct StructFields *fields,
                             const JSON_Object *schemas_obj_root) {
  return json_object_to_struct_fields_ex(schema_obj, fields, schemas_obj_root,
                                         NULL);
}
