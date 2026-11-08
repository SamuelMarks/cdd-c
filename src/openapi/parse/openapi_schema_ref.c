/**
 * @file openapi_schema_ref.c
 * @brief Schema reference parser implementation.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/** @brief Keys skipped when collecting extra schema reference fields */
static const char *k_schema_skip_keys[] = {"$ref",
                                           "$dynamicRef",
                                           "$anchor",
                                           "$dynamicAnchor",
                                           "type",
                                           "items",
                                           "format",
                                           "contentMediaType",
                                           "contentEncoding",
                                           "contentSchema",
                                           "externalDocs",
                                           "discriminator",
                                           "xml",
                                           "enum",
                                           "const",
                                           "default",
                                           "examples",
                                           "example",
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
                                           "multipleOf",
                                           "minProperties",
                                           "maxProperties",
                                           "allOf",
                                           "anyOf",
                                           "oneOf",
                                           "not",
                                           "if",
                                           "then",
                                           "else",
                                           "summary",
                                           "description",
                                           "deprecated",
                                           "readOnly",
                                           "writeOnly",
                                           "collectionFormat"};

/** @brief Keys skipped when collecting extra array items schema reference
 * fields */
static const char *k_items_skip_keys[] = {"$ref",
                                          "$dynamicRef",
                                          "$anchor",
                                          "$dynamicAnchor",
                                          "type",
                                          "format",
                                          "contentMediaType",
                                          "contentEncoding",
                                          "contentSchema",
                                          "enum",
                                          "const",
                                          "default",
                                          "examples",
                                          "example",
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
                                          "summary",
                                          "description",
                                          "deprecated",
                                          "readOnly",
                                          "writeOnly",
                                          "collectionFormat"};

/**
 * @brief Internal helper to parse an OpenAPI Schema or Reference object.
 *
 * @param[in] schema The source schema JSON object.
 * @param[out] out Output OpenAPI_SchemaRef structure.
 * @param[in] spec The parent OpenAPI specification.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t parse_schema_ref_inner(const JSON_Object *schema,
                                            struct OpenAPI_SchemaRef *out,
                                            const struct OpenAPI_Spec *spec) {
  const char *ref = NULL;
  const char *dynamic_ref = NULL;
  const char *ref_val = NULL;
  int ref_is_dynamic = 0;
  const char *summary = NULL;
  const char *desc = NULL;
  const char *format = NULL;
  const char *collection_format = NULL;
  const char *content_media_type = NULL;
  const char *content_encoding = NULL;
  JSON_Array *type_arr = NULL;
  JSON_Array *examples_arr = NULL;
  JSON_Array *enum_arr = NULL;
  const char *type = NULL;
  int nullable = 0;
  int deprecated_present = 0;
  int deprecated_val = -1;
  int read_only_present = 0;
  int read_only_val = -1;
  int write_only_present = 0;
  int write_only_val = -1;
  int has_val = 0;
  cdd_c_error_t rc;

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!schema)
    return CDD_C_SUCCESS;

  cdd_json_object_get_string(schema, "$ref", &ref);
  cdd_json_object_get_string(schema, "$dynamicRef", &dynamic_ref);
  ref_val = ref ? ref : dynamic_ref;
  ref_is_dynamic = (!ref && dynamic_ref);
  cdd_json_object_get_string(schema, "summary", &summary);
  cdd_json_object_get_string(schema, "description", &desc);
  cdd_json_object_get_string(schema, "format", &format);
  cdd_json_object_get_string(schema, "collectionFormat", &collection_format);
  cdd_json_object_get_string(schema, "contentMediaType", &content_media_type);
  cdd_json_object_get_string(schema, "contentEncoding", &content_encoding);

  out->schema_is_boolean = 0;
  out->schema_boolean_value = 0;
  out->is_array = 0;
  out->ref_name = NULL;
  out->ref = NULL;
  out->ref_is_dynamic = 0;
  out->inline_type = NULL;
  out->type_union = NULL;
  out->n_type_union = 0;
  out->format = NULL;
  out->items_format = NULL;
  out->items_ref = NULL;
  out->items_ref_is_dynamic = 0;
  out->items_type_union = NULL;
  out->n_items_type_union = 0;
  out->content_type = NULL;
  out->content_media_type = NULL;
  out->content_encoding = NULL;
  out->items_content_media_type = NULL;
  out->items_content_encoding = NULL;
  out->has_multiple_of = 0;
  out->multiple_of = 0;
  out->has_min_properties = 0;
  out->min_properties = 0;
  out->has_max_properties = 0;
  out->max_properties = 0;
  out->all_of = NULL;
  out->n_all_of = 0;
  out->any_of = NULL;
  out->n_any_of = 0;
  out->one_of = NULL;
  out->n_one_of = 0;
  out->not_schema = NULL;
  out->if_schema = NULL;
  out->then_schema = NULL;
  out->else_schema = NULL;
  out->nullable = 0;
  out->items_nullable = 0;
  out->summary = NULL;
  out->description = NULL;
  out->deprecated = 0;
  out->deprecated_set = 0;
  out->read_only = 0;
  out->read_only_set = 0;
  out->write_only = 0;
  out->write_only_set = 0;
  out->const_value_set = 0;
  memset(&out->const_value, 0, sizeof(out->const_value));
  out->examples = NULL;
  out->n_examples = 0;
  out->example_set = 0;
  memset(&out->example, 0, sizeof(out->example));
  out->default_value_set = 0;
  memset(&out->default_value, 0, sizeof(out->default_value));
  out->enum_values = NULL;
  out->n_enum_values = 0;
  out->schema_extra_json = NULL;
  memset(&out->external_docs, 0, sizeof(out->external_docs));
  out->external_docs_set = 0;
  memset(&out->discriminator, 0, sizeof(out->discriminator));
  out->discriminator_set = 0;
  memset(&out->xml, 0, sizeof(out->xml));
  out->xml_set = 0;
  out->items_enum_values = NULL;
  out->n_items_enum_values = 0;
  out->has_min = 0;
  out->min_val = 0;
  out->exclusive_min = 0;
  out->has_max = 0;
  out->max_val = 0;
  out->exclusive_max = 0;
  out->has_min_len = 0;
  out->min_len = 0;
  out->has_max_len = 0;
  out->max_len = 0;
  out->pattern = NULL;
  out->has_min_items = 0;
  out->min_items = 0;
  out->has_max_items = 0;
  out->max_items = 0;
  out->unique_items = 0;
  out->items_has_min = 0;
  out->items_min_val = 0;
  out->items_exclusive_min = 0;
  out->items_has_max = 0;
  out->items_max_val = 0;
  out->items_exclusive_max = 0;
  out->items_has_min_len = 0;
  out->items_min_len = 0;
  out->items_has_max_len = 0;
  out->items_max_len = 0;
  out->items_pattern = NULL;
  out->items_has_min_items = 0;
  out->items_min_items = 0;
  out->items_has_max_items = 0;
  out->items_max_items = 0;
  out->items_unique_items = 0;
  out->items_example_set = 0;
  memset(&out->items_example, 0, sizeof(out->items_example));
  out->items_examples = NULL;
  out->n_items_examples = 0;
  out->items_const_value_set = 0;
  memset(&out->items_const_value, 0, sizeof(out->items_const_value));
  out->items_default_value_set = 0;
  memset(&out->items_default_value, 0, sizeof(out->items_default_value));
  out->items_extra_json = NULL;
  out->collection_format = NULL;
  out->items_collection_format = NULL;
  out->items_schema_is_boolean = 0;
  out->items_schema_boolean_value = 0;
  out->multipart_fields = NULL;
  out->n_multipart_fields = 0;

  cdd_json_object_get_array(schema, "type", &type_arr);
  if (type_arr) {
    rc =
        parse_string_enum_array(type_arr, &out->type_union, &out->n_type_union);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  parse_schema_type(schema, &nullable, &type);
  out->nullable = nullable;

  has_val = 0;
  cdd_json_object_has_value(schema, "allOf", &has_val);
  if (has_val) {
    JSON_Array *comp_arr = NULL;
    cdd_json_object_get_array(schema, "allOf", &comp_arr);
    rc = parse_schema_array_ref(comp_arr, &out->all_of, &out->n_all_of, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  has_val = 0;
  cdd_json_object_has_value(schema, "anyOf", &has_val);
  if (has_val) {
    JSON_Array *comp_arr = NULL;
    cdd_json_object_get_array(schema, "anyOf", &comp_arr);
    rc = parse_schema_array_ref(comp_arr, &out->any_of, &out->n_any_of, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  has_val = 0;
  cdd_json_object_has_value(schema, "oneOf", &has_val);
  if (has_val) {
    JSON_Array *comp_arr = NULL;
    cdd_json_object_get_array(schema, "oneOf", &comp_arr);
    rc = parse_schema_array_ref(comp_arr, &out->one_of, &out->n_one_of, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  has_val = 0;
  cdd_json_object_has_value(schema, "not", &has_val);
  if (has_val) {
    JSON_Object *sub_obj = NULL;
    cdd_json_object_get_object(schema, "not", &sub_obj);
    rc = parse_schema_ref_ptr(sub_obj, &out->not_schema, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  has_val = 0;
  cdd_json_object_has_value(schema, "if", &has_val);
  if (has_val) {
    JSON_Object *sub_obj = NULL;
    cdd_json_object_get_object(schema, "if", &sub_obj);
    rc = parse_schema_ref_ptr(sub_obj, &out->if_schema, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  has_val = 0;
  cdd_json_object_has_value(schema, "then", &has_val);
  if (has_val) {
    JSON_Object *sub_obj = NULL;
    cdd_json_object_get_object(schema, "then", &sub_obj);
    rc = parse_schema_ref_ptr(sub_obj, &out->then_schema, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  has_val = 0;
  cdd_json_object_has_value(schema, "else", &has_val);
  if (has_val) {
    JSON_Object *sub_obj = NULL;
    cdd_json_object_get_object(schema, "else", &sub_obj);
    rc = parse_schema_ref_ptr(sub_obj, &out->else_schema, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  has_val = 0;
  cdd_json_object_has_value(schema, "contentSchema", &has_val);
  if (has_val) {
    JSON_Object *cs_obj = NULL;
    cdd_json_object_get_object(schema, "contentSchema", &cs_obj);
    if (cs_obj) {
      rc = parse_schema_ref_ptr(cs_obj, &out->content_schema, spec);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  rc = parse_any_field(schema, "default", &out->default_value,
                       &out->default_value_set);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (collect_schema_extras(schema, k_schema_skip_keys,
                            sizeof(k_schema_skip_keys) /
                                sizeof(k_schema_skip_keys[0]),
                            &out->schema_extra_json) != 0)
    return CDD_C_ERROR_MEMORY;
  {
    struct SchemaConstraintTarget target;
    target.has_multiple_of = &out->has_multiple_of;
    target.multiple_of = &out->multiple_of;
    target.has_min_properties = &out->has_min_properties;
    target.min_properties = &out->min_properties;
    target.has_max_properties = &out->has_max_properties;
    target.max_properties = &out->max_properties;
    target.has_min = &out->has_min;
    target.min_val = &out->min_val;
    target.exclusive_min = &out->exclusive_min;
    target.has_max = &out->has_max;
    target.max_val = &out->max_val;
    target.exclusive_max = &out->exclusive_max;
    target.has_min_len = &out->has_min_len;
    target.min_len = &out->min_len;
    target.has_max_len = &out->has_max_len;
    target.max_len = &out->max_len;
    target.pattern = &out->pattern;
    target.has_min_items = &out->has_min_items;
    target.min_items = &out->min_items;
    target.has_max_items = &out->has_max_items;
    target.max_items = &out->max_items;
    target.unique_items = &out->unique_items;
    target.example = &out->example;
    target.example_set = &out->example_set;
    rc = parse_schema_constraints(schema, &target);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  cdd_json_object_get_array(schema, "enum", &enum_arr);
  if (enum_arr) {
    rc = parse_any_array(enum_arr, &out->enum_values, &out->n_enum_values);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (format) {
    c_cdd_strdup(format, &out->format);
    if (!out->format)
      return CDD_C_ERROR_MEMORY;
  }

  if (collection_format) {
    c_cdd_strdup(collection_format, &out->collection_format);
    if (!out->collection_format)
      return CDD_C_ERROR_MEMORY;
  }

  if (content_media_type) {
    c_cdd_strdup(content_media_type, &out->content_media_type);
    if (!out->content_media_type)
      return CDD_C_ERROR_MEMORY;
  }
  if (content_encoding) {
    c_cdd_strdup(content_encoding, &out->content_encoding);
    if (!out->content_encoding)
      return CDD_C_ERROR_MEMORY;
  }

  if (ref_val && summary) {
    c_cdd_strdup(summary, &out->summary);
    if (!out->summary)
      return CDD_C_ERROR_MEMORY;
  }

  if (desc) {
    c_cdd_strdup(desc, &out->description);
    if (!out->description)
      return CDD_C_ERROR_MEMORY;
  }

  {
    JSON_Object *ext_docs = NULL;
    cdd_json_object_get_object(schema, "externalDocs", &ext_docs);
    if (ext_docs) {
      out->external_docs_set = 1;
      rc = parse_external_docs(ext_docs, &out->external_docs);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  {
    JSON_Object *disc_obj = NULL;
    cdd_json_object_get_object(schema, "discriminator", &disc_obj);
    if (disc_obj) {
      out->discriminator_set = 1;
      rc = parse_discriminator_object(disc_obj, &out->discriminator);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  {
    JSON_Object *xml_obj = NULL;
    cdd_json_object_get_object(schema, "xml", &xml_obj);
    if (xml_obj) {
      out->xml_set = 1;
      rc = parse_xml_object(xml_obj, &out->xml);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  cdd_json_object_has_value(schema, "deprecated", &deprecated_present);
  cdd_json_object_get_boolean(schema, "deprecated", &deprecated_val);
  if (deprecated_present) {
    out->deprecated_set = 1;
    out->deprecated = (deprecated_val == 1);
  }
  cdd_json_object_has_value(schema, "readOnly", &read_only_present);
  cdd_json_object_get_boolean(schema, "readOnly", &read_only_val);
  if (read_only_present) {
    out->read_only_set = 1;
    out->read_only = (read_only_val == 1);
  }
  cdd_json_object_has_value(schema, "writeOnly", &write_only_present);
  cdd_json_object_get_boolean(schema, "writeOnly", &write_only_val);
  if (write_only_present) {
    out->write_only_set = 1;
    out->write_only = (write_only_val == 1);
  }

  rc = parse_any_field(schema, "const", &out->const_value,
                       &out->const_value_set);
  if (rc != CDD_C_SUCCESS)
    return rc;

  cdd_json_object_get_array(schema, "examples", &examples_arr);
  if (examples_arr) {
    rc = parse_any_array(examples_arr, &out->examples, &out->n_examples);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (ref_val) {
    struct ResolvedRefTarget resolved;
    const struct OpenAPI_Spec *target;
    const char *name_enc = NULL;
    char *name_dec = NULL;
    resolve_ref_target(spec, ref_val, &resolved);
    target = resolved.spec;
    ref_name_from_prefix(target, resolved.ref, "#/components/schemas/",
                         &name_enc);
    c_cdd_strdup(ref_val, &out->ref);
    if (!out->ref) {
      if (resolved.resolved_ref)
        C_CDD_FREE(resolved.resolved_ref);
      return CDD_C_ERROR_MEMORY;
    }
    out->ref_is_dynamic = ref_is_dynamic ? 1 : 0;
    if (name_enc) {
      json_pointer_unescape(name_enc, &name_dec);
      if (!name_dec) {
        if (resolved.resolved_ref)
          C_CDD_FREE(resolved.resolved_ref);
        return CDD_C_ERROR_MEMORY;
      }
      out->ref_name = name_dec;
    }
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    return CDD_C_SUCCESS;
  }

  if (type && strcmp(type, "array") == 0) {
    JSON_Value *items_val = NULL;
    JSON_Object *items = NULL;
    JSON_Value_Type items_vtype = JSONError;
    cdd_json_object_get_value(schema, "items", &items_val);
    if (items_val) {
      cdd_json_value_get_object(items_val, &items);
      cdd_json_value_get_type(items_val, &items_vtype);
    }
    out->is_array = 1;
    if (items_val && items_vtype == JSONBoolean) {
      out->items_schema_is_boolean = 1;
      cdd_json_value_get_boolean(items_val, &out->items_schema_boolean_value);
      return CDD_C_SUCCESS;
    }
    if (items) {
      const char *item_ref = NULL;
      const char *item_dynamic_ref = NULL;
      const char *item_ref_val = NULL;
      int item_ref_is_dynamic = 0;
      const char *item_format = NULL;
      const char *item_collection_format = NULL;
      const char *item_type = NULL;
      JSON_Array *item_types_arr = NULL;
      JSON_Array *item_enum = NULL;
      JSON_Array *item_examples = NULL;
      int items_nullable = 0;
      const char *item_content_media_type = NULL;
      const char *item_content_encoding = NULL;
      int has_cs = 0;
      cdd_json_object_get_string(items, "$ref", &item_ref);
      cdd_json_object_get_string(items, "$dynamicRef", &item_dynamic_ref);
      item_ref_val = item_ref ? item_ref : item_dynamic_ref;
      item_ref_is_dynamic = (!item_ref && item_dynamic_ref);
      cdd_json_object_get_string(items, "format", &item_format);
      cdd_json_object_get_string(items, "collectionFormat",
                                 &item_collection_format);
      cdd_json_object_get_string(items, "contentMediaType",
                                 &item_content_media_type);
      cdd_json_object_get_string(items, "contentEncoding",
                                 &item_content_encoding);
      {
        struct SchemaConstraintTarget target;
        target.has_multiple_of = NULL;
        target.multiple_of = NULL;
        target.has_min_properties = NULL;
        target.min_properties = NULL;
        target.has_max_properties = NULL;
        target.max_properties = NULL;
        target.has_min = &out->items_has_min;
        target.min_val = &out->items_min_val;
        target.exclusive_min = &out->items_exclusive_min;
        target.has_max = &out->items_has_max;
        target.max_val = &out->items_max_val;
        target.exclusive_max = &out->items_exclusive_max;
        target.has_min_len = &out->items_has_min_len;
        target.min_len = &out->items_min_len;
        target.has_max_len = &out->items_has_max_len;
        target.max_len = &out->items_max_len;
        target.pattern = &out->items_pattern;
        target.has_min_items = &out->items_has_min_items;
        target.min_items = &out->items_min_items;
        target.has_max_items = &out->items_has_max_items;
        target.max_items = &out->items_max_items;
        target.unique_items = &out->items_unique_items;
        target.example = &out->items_example;
        target.example_set = &out->items_example_set;
        rc = parse_schema_constraints(items, &target);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      parse_schema_type(items, &items_nullable, &item_type);
      out->items_nullable = items_nullable;
      cdd_json_object_get_array(items, "type", &item_types_arr);
      if (item_types_arr) {
        rc = parse_string_enum_array(item_types_arr, &out->items_type_union,
                                     &out->n_items_type_union);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (item_format) {
        c_cdd_strdup(item_format, &out->items_format);
        if (!out->items_format)
          return CDD_C_ERROR_MEMORY;
      }
      if (item_collection_format) {
        c_cdd_strdup(item_collection_format, &out->items_collection_format);
        if (!out->items_collection_format)
          return CDD_C_ERROR_MEMORY;
      }
      if (item_content_media_type) {
        c_cdd_strdup(item_content_media_type, &out->items_content_media_type);
        if (!out->items_content_media_type)
          return CDD_C_ERROR_MEMORY;
      }
      if (item_content_encoding) {
        c_cdd_strdup(item_content_encoding, &out->items_content_encoding);
        if (!out->items_content_encoding)
          return CDD_C_ERROR_MEMORY;
      }
      cdd_json_object_get_array(items, "enum", &item_enum);
      if (item_enum) {
        rc = parse_any_array(item_enum, &out->items_enum_values,
                             &out->n_items_enum_values);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      cdd_json_object_get_array(items, "examples", &item_examples);
      if (item_examples) {
        rc = parse_any_array(item_examples, &out->items_examples,
                             &out->n_items_examples);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      rc = parse_any_field(items, "const", &out->items_const_value,
                           &out->items_const_value_set);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = parse_any_field(items, "default", &out->items_default_value,
                           &out->items_default_value_set);
      if (rc != CDD_C_SUCCESS)
        return rc;
      cdd_json_object_has_value(items, "contentSchema", &has_cs);
      if (has_cs) {
        JSON_Object *cs_obj = NULL;
        cdd_json_object_get_object(items, "contentSchema", &cs_obj);
        if (cs_obj) {
          rc = parse_schema_ref_ptr(cs_obj, &out->items_content_schema, spec);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
      if (collect_schema_extras(items, k_items_skip_keys,
                                sizeof(k_items_skip_keys) /
                                    sizeof(k_items_skip_keys[0]),
                                &out->items_extra_json) != 0)
        return CDD_C_ERROR_MEMORY;
      if (item_ref_val) {
        struct ResolvedRefTarget resolved;
        const struct OpenAPI_Spec *target;
        const char *name_enc = NULL;
        char *name_dec = NULL;
        resolve_ref_target(spec, item_ref_val, &resolved);
        target = resolved.spec;
        ref_name_from_prefix(target, resolved.ref, "#/components/schemas/",
                             &name_enc);
        c_cdd_strdup(item_ref_val, &out->items_ref);
        if (!out->items_ref) {
          if (resolved.resolved_ref)
            C_CDD_FREE(resolved.resolved_ref);
          return CDD_C_ERROR_MEMORY;
        }
        out->items_ref_is_dynamic = item_ref_is_dynamic ? 1 : 0;
        if (name_enc) {
          json_pointer_unescape(name_enc, &name_dec);
          if (!name_dec) {
            if (resolved.resolved_ref)
              C_CDD_FREE(resolved.resolved_ref);
            return CDD_C_ERROR_MEMORY;
          }
          out->ref_name = name_dec;
        }
        if (resolved.resolved_ref)
          C_CDD_FREE(resolved.resolved_ref);
        return CDD_C_SUCCESS;
      }
      if (item_type) {
        c_cdd_strdup(item_type, &out->inline_type);
        return out->inline_type ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
      }
    }
    return CDD_C_SUCCESS;
  }

  if (type) {
    c_cdd_strdup(type, &out->inline_type);
    return out->inline_type ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a JSON schema object into an OpenAPI_SchemaRef structure.
 *
 * @param[in] schema JSON object representing the schema.
 * @param[out] out Destination OpenAPI_SchemaRef structure.
 * @param[in] spec Parent OpenAPI specification context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t parse_schema_ref(const JSON_Object *schema,
                               struct OpenAPI_SchemaRef *out,
                               const struct OpenAPI_Spec *spec) {
  cdd_c_error_t rc = parse_schema_ref_inner(schema, out, spec);
  if (rc != CDD_C_SUCCESS && out)
    free_schema_ref_content(out);
  return rc;
}
