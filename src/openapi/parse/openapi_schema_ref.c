/**
 * @file openapi_schema_ref.c
 * @brief Schema reference parser implementation.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Frees `OpenAPI_SchemaRef` content via a `*_free` wrapper.
 *
 * @param[in,out] sr SchemaRef structure to release.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t oa_schema_ref_content_free(struct OpenAPI_SchemaRef *sr) {
  return free_schema_ref_content(sr);
}

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
 * @brief Parses an optional composition array keyword (`allOf`, `anyOf`,
 * `oneOf`) on `schema`.
 *
 * @param[in] schema Source schema JSON object.
 * @param[in] key Keyword name.
 * @param[out] out_arr Output array of `OpenAPI_SchemaRef`.
 * @param[out] out_count Output element count.
 * @param[in] spec Parent OpenAPI specification context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t
parse_opt_schema_comp_array(const JSON_Object *schema, const char *key,
                            struct OpenAPI_SchemaRef **out_arr,
                            size_t *out_count,
                            const struct OpenAPI_Spec *spec) {
  int has_val = 0;
  JSON_Array *comp_arr = NULL;
  cdd_c_error_t rc;
  rc = oa_json_has_value(schema, key, &has_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!has_val)
    return CDD_C_SUCCESS;
  rc = oa_json_opt_array(schema, key, &comp_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return parse_schema_array_ref(comp_arr, out_arr, out_count, spec);
}

/**
 * @brief Parses an optional sub-schema keyword (`not`, `if`, `then`, `else`) on
 * `schema`.
 *
 * @param[in] schema Source schema JSON object.
 * @param[in] key Keyword name.
 * @param[out] out_ptr Output heap-allocated `OpenAPI_SchemaRef` pointer.
 * @param[in] spec Parent OpenAPI specification context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t
parse_opt_sub_schema_ptr(const JSON_Object *schema, const char *key,
                         struct OpenAPI_SchemaRef **out_ptr,
                         const struct OpenAPI_Spec *spec) {
  int has_val = 0;
  JSON_Object *sub_obj = NULL;
  cdd_c_error_t rc;
  rc = oa_json_has_value(schema, key, &has_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!has_val)
    return CDD_C_SUCCESS;
  rc = oa_json_opt_object(schema, key, &sub_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return parse_schema_ref_ptr(sub_obj, out_ptr, spec);
}

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
  int deprecated_val = 0;
  int read_only_present = 0;
  int read_only_val = 0;
  int write_only_present = 0;
  int write_only_val = 0;
  int has_val = 0;
  cdd_c_error_t rc;

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!schema)
    return CDD_C_SUCCESS;

  rc = oa_json_opt_string(schema, "$ref", &ref);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_json_opt_string(schema, "$dynamicRef", &dynamic_ref);
  if (rc != CDD_C_SUCCESS)
    return rc;
  ref_val = ref ? ref : dynamic_ref;
  ref_is_dynamic = (!ref && dynamic_ref);
  rc = oa_json_opt_string(schema, "summary", &summary);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_json_opt_string(schema, "description", &desc);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_json_opt_string(schema, "format", &format);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_json_opt_string(schema, "collectionFormat", &collection_format);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_json_opt_string(schema, "contentMediaType", &content_media_type);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_json_opt_string(schema, "contentEncoding", &content_encoding);
  if (rc != CDD_C_SUCCESS)
    return rc;

  memset(out, 0, sizeof(*out));

  rc = oa_json_opt_array(schema, "type", &type_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (type_arr) {
    rc =
        parse_string_enum_array(type_arr, &out->type_union, &out->n_type_union);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = parse_schema_type(schema, &nullable, &type);
  if (rc != CDD_C_SUCCESS)
    return rc;
  out->nullable = nullable;

  rc = parse_opt_schema_comp_array(schema, "allOf", &out->all_of,
                                   &out->n_all_of, spec);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_opt_schema_comp_array(schema, "anyOf", &out->any_of,
                                   &out->n_any_of, spec);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_opt_schema_comp_array(schema, "oneOf", &out->one_of,
                                   &out->n_one_of, spec);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_opt_sub_schema_ptr(schema, "not", &out->not_schema, spec);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_opt_sub_schema_ptr(schema, "if", &out->if_schema, spec);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_opt_sub_schema_ptr(schema, "then", &out->then_schema, spec);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_opt_sub_schema_ptr(schema, "else", &out->else_schema, spec);
  if (rc != CDD_C_SUCCESS)
    return rc;

  has_val = 0;
  rc = oa_json_has_value(schema, "contentSchema", &has_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_val) {
    JSON_Object *cs_obj = NULL;
    rc = oa_json_opt_object(schema, "contentSchema", &cs_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
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
  rc = collect_schema_extras(schema, k_schema_skip_keys,
                             sizeof(k_schema_skip_keys) /
                                 sizeof(k_schema_skip_keys[0]),
                             &out->schema_extra_json);
  if (rc != CDD_C_SUCCESS)
    return rc;
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

  rc = oa_json_opt_array(schema, "enum", &enum_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (enum_arr) {
    rc = parse_any_array(enum_arr, &out->enum_values, &out->n_enum_values);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (format) {
    rc = c_cdd_strdup(format, &out->format);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (collection_format) {
    rc = c_cdd_strdup(collection_format, &out->collection_format);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (content_media_type) {
    rc = c_cdd_strdup(content_media_type, &out->content_media_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (content_encoding) {
    rc = c_cdd_strdup(content_encoding, &out->content_encoding);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (ref_val && summary) {
    rc = c_cdd_strdup(summary, &out->summary);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (desc) {
    rc = c_cdd_strdup(desc, &out->description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  {
    JSON_Object *ext_docs = NULL;
    rc = oa_json_opt_object(schema, "externalDocs", &ext_docs);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (ext_docs) {
      out->external_docs_set = 1;
      rc = parse_external_docs(ext_docs, &out->external_docs);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  {
    JSON_Object *disc_obj = NULL;
    rc = oa_json_opt_object(schema, "discriminator", &disc_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (disc_obj) {
      out->discriminator_set = 1;
      rc = parse_discriminator_object(disc_obj, &out->discriminator);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  {
    JSON_Object *xml_obj = NULL;
    rc = oa_json_opt_object(schema, "xml", &xml_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (xml_obj) {
      out->xml_set = 1;
      rc = parse_xml_object(xml_obj, &out->xml);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  rc = oa_json_opt_bool(schema, "deprecated", &deprecated_present,
                        &deprecated_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (deprecated_present) {
    out->deprecated_set = 1;
    out->deprecated = deprecated_val;
  }
  rc = oa_json_opt_bool(schema, "readOnly", &read_only_present, &read_only_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (read_only_present) {
    out->read_only_set = 1;
    out->read_only = read_only_val;
  }
  rc = oa_json_opt_bool(schema, "writeOnly", &write_only_present,
                        &write_only_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (write_only_present) {
    out->write_only_set = 1;
    out->write_only = write_only_val;
  }

  rc = parse_any_field(schema, "const", &out->const_value,
                       &out->const_value_set);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = oa_json_opt_array(schema, "examples", &examples_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
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
    rc = resolve_ref_target(spec, ref_val, &resolved);
    if (rc != CDD_C_SUCCESS)
      return rc;
    target = resolved.spec;
    rc = ref_name_from_prefix(target, resolved.ref, "#/components/schemas/",
                              &name_enc);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(resolved.resolved_ref);
      return rc;
    }
    rc = c_cdd_strdup(ref_val, &out->ref);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(resolved.resolved_ref);
      return rc;
    }
    out->ref_is_dynamic = ref_is_dynamic ? 1 : 0;
    if (name_enc) {
      rc = json_pointer_unescape(name_enc, &name_dec);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(resolved.resolved_ref);
        return rc;
      }
      out->ref_name = name_dec;
    }
    C_CDD_FREE(resolved.resolved_ref);
    return CDD_C_SUCCESS;
  }

  if (type && strcmp(type, "array") == 0) {
    JSON_Value *items_val = NULL;
    JSON_Object *items = NULL;
    JSON_Value_Type items_vtype = JSONError;
    rc = oa_json_opt_value(schema, "items", &items_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (items_val) {
      rc = oa_json_val_opt_object(items_val, &items);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = cdd_json_value_get_type(items_val, &items_vtype);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    out->is_array = 1;
    if (items_val && items_vtype == JSONBoolean) {
      out->items_schema_is_boolean = 1;
      return cdd_json_value_get_boolean(items_val,
                                        &out->items_schema_boolean_value);
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
      rc = oa_json_opt_string(items, "$ref", &item_ref);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = oa_json_opt_string(items, "$dynamicRef", &item_dynamic_ref);
      if (rc != CDD_C_SUCCESS)
        return rc;
      item_ref_val = item_ref ? item_ref : item_dynamic_ref;
      item_ref_is_dynamic = (!item_ref && item_dynamic_ref);
      rc = oa_json_opt_string(items, "format", &item_format);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = oa_json_opt_string(items, "collectionFormat",
                              &item_collection_format);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = oa_json_opt_string(items, "contentMediaType",
                              &item_content_media_type);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = oa_json_opt_string(items, "contentEncoding", &item_content_encoding);
      if (rc != CDD_C_SUCCESS)
        return rc;
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
      rc = parse_schema_type(items, &items_nullable, &item_type);
      if (rc != CDD_C_SUCCESS)
        return rc;
      out->items_nullable = items_nullable;
      rc = oa_json_opt_array(items, "type", &item_types_arr);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (item_types_arr) {
        rc = parse_string_enum_array(item_types_arr, &out->items_type_union,
                                     &out->n_items_type_union);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (item_format) {
        rc = c_cdd_strdup(item_format, &out->items_format);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (item_collection_format) {
        rc =
            c_cdd_strdup(item_collection_format, &out->items_collection_format);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (item_content_media_type) {
        rc = c_cdd_strdup(item_content_media_type,
                          &out->items_content_media_type);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (item_content_encoding) {
        rc = c_cdd_strdup(item_content_encoding, &out->items_content_encoding);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      rc = oa_json_opt_array(items, "enum", &item_enum);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (item_enum) {
        rc = parse_any_array(item_enum, &out->items_enum_values,
                             &out->n_items_enum_values);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      rc = oa_json_opt_array(items, "examples", &item_examples);
      if (rc != CDD_C_SUCCESS)
        return rc;
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
      rc = oa_json_has_value(items, "contentSchema", &has_cs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (has_cs) {
        JSON_Object *cs_obj = NULL;
        rc = oa_json_opt_object(items, "contentSchema", &cs_obj);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (cs_obj) {
          rc = parse_schema_ref_ptr(cs_obj, &out->items_content_schema, spec);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
      rc = collect_schema_extras(items, k_items_skip_keys,
                                 sizeof(k_items_skip_keys) /
                                     sizeof(k_items_skip_keys[0]),
                                 &out->items_extra_json);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (item_ref_val) {
        struct ResolvedRefTarget resolved;
        const struct OpenAPI_Spec *target;
        const char *name_enc = NULL;
        char *name_dec = NULL;
        rc = resolve_ref_target(spec, item_ref_val, &resolved);
        if (rc != CDD_C_SUCCESS)
          return rc;
        target = resolved.spec;
        rc = ref_name_from_prefix(target, resolved.ref, "#/components/schemas/",
                                  &name_enc);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(resolved.resolved_ref);
          return rc;
        }
        rc = c_cdd_strdup(item_ref_val, &out->items_ref);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(resolved.resolved_ref);
          return rc;
        }
        out->items_ref_is_dynamic = item_ref_is_dynamic ? 1 : 0;
        if (name_enc) {
          rc = json_pointer_unescape(name_enc, &name_dec);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(resolved.resolved_ref);
            return rc;
          }
          out->ref_name = name_dec;
        }
        C_CDD_FREE(resolved.resolved_ref);
        return CDD_C_SUCCESS;
      }
      if (item_type) {
        return c_cdd_strdup(item_type, &out->inline_type);
      }
    }
    return CDD_C_SUCCESS;
  }

  if (type) {
    return c_cdd_strdup(type, &out->inline_type);
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
  if (rc != CDD_C_SUCCESS && out) {
    return cdd_c_keep_err(rc, oa_schema_ref_content_free(out));
  }
  return rc;
}
