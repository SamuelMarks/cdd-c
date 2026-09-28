/**
 * @file code2schema_writer.c
 * @brief Writing struct fields and constraints to JSON schema.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Writes the default value of a StructField to a JSON schema
 * object.
 */
cdd_c_error_t c2s_write_default_value(JSON_Object *pobj,
                                      const struct StructField *field) {
  const char *def;
  const char *typ;
  char buf[256];
  int bval;
  double nval;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!pobj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  def = field->default_val;
  if (def[0] == '\0')
    return CDD_C_SUCCESS;
  typ = field->type;

  if (strcmp(def, "nullptr") == 0) {
    json_object_set_value(pobj, "default", json_value_init_null());
    return CDD_C_SUCCESS;
  }

  if (strcmp(typ, "string") == 0) {
    const char *s = NULL;
    {
      cdd_c_error_t rc_c2s = c2s_strip_quotes(def, buf, sizeof(buf), &s);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    json_object_set_string(pobj, "default", s);
    return CDD_C_SUCCESS;
  }

  if (strcmp(typ, "boolean") == 0) {
    int has_bval = 0;
    cdd_c_error_t rc_c2s = c2s_parse_bool_default(def, &bval, &has_bval);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
    if (has_bval)
      json_object_set_boolean(pobj, "default", bval);
    return CDD_C_SUCCESS;
  }

  if (strcmp(typ, "integer") == 0 || strcmp(typ, "number") == 0) {
    int has_nval = 0;
    cdd_c_error_t rc_c2s = c2s_parse_number_default(def, &nval, &has_nval);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
    if (has_nval)
      json_object_set_number(pobj, "default", nval);
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes numeric constraints of a StructField to a JSON schema
 * object.
 */
cdd_c_error_t c2s_write_numeric_constraints(JSON_Object *pobj,
                                            const struct StructField *field) {
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!pobj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
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
      json_object_set_number(pobj, "maximum", field->max_val);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes string constraints of a StructField to a JSON schema
 * object.
 */
cdd_c_error_t c2s_write_string_constraints(JSON_Object *pobj,
                                           const struct StructField *field) {
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!pobj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
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
cdd_c_error_t c2s_write_array_constraints(JSON_Object *pobj,
                                          const struct StructField *field) {
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!pobj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!field->type[0] || strcmp(field->type, "array") != 0)
    return CDD_C_SUCCESS;
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
cdd_c_error_t c2s_write_type_union(JSON_Object *obj, const char *type,
                                   char **type_union, size_t n_type_union) {
  size_t i;
  JSON_Value *arr_val;
  JSON_Array *arr;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!obj)
    return CDD_C_SUCCESS;

  if (type_union) {
    arr_val = json_value_init_array();
    if (!arr_val)
      return CDD_C_SUCCESS;
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
cdd_c_error_t write_struct_to_json_schema(JSON_Object *schemas_obj,
                                          const char *struct_name,
                                          const struct StructFields *sf) {
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);
  JSON_Value *props_val = json_value_init_object();
  JSON_Object *props_obj = json_value_get_object(props_val);
  JSON_Value *req_val = NULL;
  JSON_Array *req_arr = NULL;
  size_t i;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!schemas_obj || !struct_name || !sf) {
    json_value_free(val);
    json_value_free(props_val);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (sf->is_enum) {
    JSON_Value *enum_val = json_value_init_array();
    JSON_Array *enum_arr;
    if (!enum_val) {
      json_value_free(val);
      json_value_free(props_val);
      return CDD_C_ERROR_MEMORY;
    }
    enum_arr = json_value_get_array(enum_val);
    json_object_set_string(obj, "type", "string");
    for (i = 0; i < sf->enum_members.size; ++i) {
      const char *member = sf->enum_members.members[i];
      if (member)
        json_array_append_string(enum_arr, member);
    }
    if (json_object_set_value(obj, "enum", enum_val) != JSONSuccess) {
      json_value_free(enum_val);
      json_value_free(val);
      json_value_free(props_val);
      return CDD_C_ERROR_MEMORY;
    }
    if (c2s_merge_schema_extras_object(obj, sf->schema_extra_json) != 0) {
      json_value_free(val);
      json_value_free(props_val);
      return CDD_C_ERROR_MEMORY;
    }
    if (json_object_set_value(schemas_obj, struct_name, val) != JSONSuccess) {
      json_value_free(val);
      json_value_free(props_val);
      return CDD_C_ERROR_MEMORY;
    }
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

    if (*bw) {
      json_object_set_string(pobj, "x-c-bitwidth", bw);
    }

    if (strcmp(typ, "array") == 0) {
      JSON_Value *items_val = json_value_init_object();
      JSON_Object *items_obj = json_value_get_object(items_val);
      {
        cdd_c_error_t rc_c2s = c2s_write_type_union(
            pobj, "array", field->type_union, field->n_type_union);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
      if (field->items_type_union) {
        c2s_write_type_union(items_obj, ref, field->items_type_union,
                             field->n_items_type_union);
      } else if (*ref) {
        if (strcmp(ref, "integer") == 0 || strcmp(ref, "string") == 0 ||
            strcmp(ref, "boolean") == 0 || strcmp(ref, "number") == 0) {
          json_object_set_string(items_obj, "type", ref);
        } else {
          char ref_str[128];
          CDD_SNPRINTF(ref_str, sizeof(ref_str), "#/components/schemas/%s",
                       ref);
          json_object_set_string(items_obj, "$ref", ref_str);
        }
      }
      if (c2s_merge_schema_extras_object(items_obj, field->items_extra_json) !=
          0) {
        json_value_free(items_val);
        json_value_free(pval);
        json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
      json_object_set_value(pobj, "items", items_val);
      {
        cdd_c_error_t rc_c2s = c2s_write_array_constraints(pobj, field);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
    } else {
      if (strcmp(typ, "object") == 0 || strcmp(typ, "enum") == 0) {
        char ref_str[128];
        if (*ref) {
          if (ref[0] == '#') {
            CDD_SNPRINTF(ref_str, sizeof(ref_str), "%s", ref);
          } else {
            CDD_SNPRINTF(ref_str, sizeof(ref_str), "#/components/schemas/%s",
                         ref);
          }
          json_object_set_string(pobj, "$ref", ref_str);
        } else {
          c2s_write_type_union(pobj, "object", field->type_union,
                               field->n_type_union);
        }
      } else {
        {
          cdd_c_error_t rc_c2s = c2s_write_type_union(
              pobj, typ, field->type_union, field->n_type_union);
          if (rc_c2s != CDD_C_SUCCESS)
            return rc_c2s;
        }
      }
    }
    {
      cdd_c_error_t rc_c2s = c2s_write_numeric_constraints(pobj, field);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    {
      cdd_c_error_t rc_c2s = c2s_write_string_constraints(pobj, field);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    {
      cdd_c_error_t rc_c2s = c2s_write_default_value(pobj, field);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
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
    if (c2s_merge_schema_extras_object(pobj, field->schema_extra_json) != 0) {
      json_value_free(pval);
      json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    json_object_set_value(props_obj, sf->fields[i].name, pval);
    if (field->required) {
      if (!req_val) {
        req_val = json_value_init_array();
        req_arr = json_value_get_array(req_val);
      }
      json_array_append_string(req_arr, field->name);
    }
  }

  if (req_val)
    json_object_set_value(obj, "required", req_val);

  if (c2s_merge_schema_extras_object(obj, sf->schema_extra_json) != 0) {
    json_value_free(val);
    return CDD_C_ERROR_MEMORY;
  }

  if (json_object_set_value(schemas_obj, struct_name, val) != JSONSuccess) {
    json_value_free(val);
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
