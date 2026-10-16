/**
 * @file openapi_copy_schema.c
 * @brief Deep copy routines for schema references and any values.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Creates a deep copy of schema ref.
 */
cdd_c_error_t copy_schema_ref(struct OpenAPI_SchemaRef *dst,
                              const struct OpenAPI_SchemaRef *src) {
  size_t i;
  cdd_c_error_t rc;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  dst->schema_is_boolean = src->schema_is_boolean;
  dst->schema_boolean_value = src->schema_boolean_value;
  dst->is_array = src->is_array;
  dst->ref_is_dynamic = src->ref_is_dynamic;
  if (src->ref_name) {
    c_cdd_strdup(src->ref_name, &dst->ref_name);
    if (!dst->ref_name)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->ref) {
    c_cdd_strdup(src->ref, &dst->ref);
    if (!dst->ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->inline_type) {
    c_cdd_strdup(src->inline_type, &dst->inline_type);
    if (!dst->inline_type)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->type_union && src->n_type_union > 0) {
    rc = copy_string_array(&dst->type_union, &dst->n_type_union,
                           src->type_union, src->n_type_union);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->format) {
    c_cdd_strdup(src->format, &dst->format);
    if (!dst->format)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_type) {
    c_cdd_strdup(src->content_type, &dst->content_type);
    if (!dst->content_type)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_media_type) {
    c_cdd_strdup(src->content_media_type, &dst->content_media_type);
    if (!dst->content_media_type)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_encoding) {
    c_cdd_strdup(src->content_encoding, &dst->content_encoding);
    if (!dst->content_encoding)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->items_format) {
    c_cdd_strdup(src->items_format, &dst->items_format);
    if (!dst->items_format)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->items_type_union && src->n_items_type_union > 0) {
    rc = copy_string_array(&dst->items_type_union, &dst->n_items_type_union,
                           src->items_type_union, src->n_items_type_union);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->items_ref) {
    c_cdd_strdup(src->items_ref, &dst->items_ref);
    if (!dst->items_ref)
      return CDD_C_ERROR_MEMORY;
  }
  dst->items_ref_is_dynamic = src->items_ref_is_dynamic;
  if (src->items_content_media_type) {
    c_cdd_strdup(src->items_content_media_type, &dst->items_content_media_type);
    if (!dst->items_content_media_type)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->items_content_encoding) {
    c_cdd_strdup(src->items_content_encoding, &dst->items_content_encoding);
    if (!dst->items_content_encoding)
      return CDD_C_ERROR_MEMORY;
  }
  dst->nullable = src->nullable;
  dst->items_nullable = src->items_nullable;
  if (src->summary) {
    c_cdd_strdup(src->summary, &dst->summary);
    if (!dst->summary)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->description) {
    c_cdd_strdup(src->description, &dst->description);
    if (!dst->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->deprecated_set) {
    dst->deprecated_set = 1;
    dst->deprecated = src->deprecated;
  }
  if (src->read_only_set) {
    dst->read_only_set = 1;
    dst->read_only = src->read_only;
  }
  if (src->write_only_set) {
    dst->write_only_set = 1;
    dst->write_only = src->write_only;
  }
  if (src->const_value_set) {
    rc = copy_any_value(&dst->const_value, &src->const_value);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->const_value_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    dst->examples = (struct OpenAPI_Any *)C_CDD_CALLOC(
        src->n_examples, sizeof(struct OpenAPI_Any));
    if (!dst->examples)
      return CDD_C_ERROR_MEMORY;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      rc = copy_any_value(&dst->examples[i], &src->examples[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->example_set) {
    rc = copy_any_value(&dst->example, &src->example);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->example_set = 1;
  }
  if (src->default_value_set) {
    rc = copy_any_value(&dst->default_value, &src->default_value);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->default_value_set = 1;
  }
  if (src->enum_values && src->n_enum_values > 0) {
    dst->enum_values = (struct OpenAPI_Any *)C_CDD_CALLOC(
        src->n_enum_values, sizeof(struct OpenAPI_Any));
    if (!dst->enum_values)
      return CDD_C_ERROR_MEMORY;
    dst->n_enum_values = src->n_enum_values;
    for (i = 0; i < src->n_enum_values; ++i) {
      rc = copy_any_value(&dst->enum_values[i], &src->enum_values[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->schema_extra_json) {
    c_cdd_strdup(src->schema_extra_json, &dst->schema_extra_json);
    if (!dst->schema_extra_json)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->external_docs.description) {
    c_cdd_strdup(src->external_docs.description,
                 &dst->external_docs.description);
    if (!dst->external_docs.description)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->external_docs.url) {
    c_cdd_strdup(src->external_docs.url, &dst->external_docs.url);
    if (!dst->external_docs.url)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->external_docs.extensions_json) {
    c_cdd_strdup(src->external_docs.extensions_json,
                 &dst->external_docs.extensions_json);
    if (!dst->external_docs.extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  dst->external_docs_set = src->external_docs_set;
  if (src->discriminator.property_name) {
    c_cdd_strdup(src->discriminator.property_name,
                 &dst->discriminator.property_name);
    if (!dst->discriminator.property_name)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->discriminator.default_mapping) {
    c_cdd_strdup(src->discriminator.default_mapping,
                 &dst->discriminator.default_mapping);
    if (!dst->discriminator.default_mapping)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->discriminator.extensions_json) {
    c_cdd_strdup(src->discriminator.extensions_json,
                 &dst->discriminator.extensions_json);
    if (!dst->discriminator.extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->discriminator.mapping && src->discriminator.n_mapping > 0) {
    dst->discriminator.mapping =
        (struct OpenAPI_DiscriminatorMap *)C_CDD_CALLOC(
            src->discriminator.n_mapping,
            sizeof(struct OpenAPI_DiscriminatorMap));
    if (!dst->discriminator.mapping)
      return CDD_C_ERROR_MEMORY;
    dst->discriminator.n_mapping = src->discriminator.n_mapping;
    for (i = 0; i < src->discriminator.n_mapping; ++i) {
      if (src->discriminator.mapping[i].value) {
        c_cdd_strdup(src->discriminator.mapping[i].value,
                     &dst->discriminator.mapping[i].value);
        if (!dst->discriminator.mapping[i].value)
          return CDD_C_ERROR_MEMORY;
      }
      if (src->discriminator.mapping[i].schema) {
        c_cdd_strdup(src->discriminator.mapping[i].schema,
                     &dst->discriminator.mapping[i].schema);
        if (!dst->discriminator.mapping[i].schema)
          return CDD_C_ERROR_MEMORY;
      }
    }
  }
  dst->discriminator_set = src->discriminator_set;
  dst->xml.node_type = src->xml.node_type;
  dst->xml.node_type_set = src->xml.node_type_set;
  if (src->xml.name) {
    c_cdd_strdup(src->xml.name, &dst->xml.name);
    if (!dst->xml.name)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->xml.namespace_uri) {
    c_cdd_strdup(src->xml.namespace_uri, &dst->xml.namespace_uri);
    if (!dst->xml.namespace_uri)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->xml.prefix) {
    c_cdd_strdup(src->xml.prefix, &dst->xml.prefix);
    if (!dst->xml.prefix)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->xml.extensions_json) {
    c_cdd_strdup(src->xml.extensions_json, &dst->xml.extensions_json);
    if (!dst->xml.extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  dst->xml.attribute = src->xml.attribute;
  dst->xml.attribute_set = src->xml.attribute_set;
  dst->xml.wrapped = src->xml.wrapped;
  dst->xml.wrapped_set = src->xml.wrapped_set;
  dst->xml_set = src->xml_set;
  if (src->items_enum_values && src->n_items_enum_values > 0) {
    dst->items_enum_values = (struct OpenAPI_Any *)C_CDD_CALLOC(
        src->n_items_enum_values, sizeof(struct OpenAPI_Any));
    if (!dst->items_enum_values)
      return CDD_C_ERROR_MEMORY;
    dst->n_items_enum_values = src->n_items_enum_values;
    for (i = 0; i < src->n_items_enum_values; ++i) {
      rc = copy_any_value(&dst->items_enum_values[i],
                          &src->items_enum_values[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  dst->has_min = src->has_min;
  dst->min_val = src->min_val;
  dst->exclusive_min = src->exclusive_min;
  dst->has_max = src->has_max;
  dst->max_val = src->max_val;
  dst->exclusive_max = src->exclusive_max;
  dst->has_min_len = src->has_min_len;
  dst->min_len = src->min_len;
  dst->has_max_len = src->has_max_len;
  dst->max_len = src->max_len;
  if (src->pattern) {
    c_cdd_strdup(src->pattern, &dst->pattern);
    if (!dst->pattern)
      return CDD_C_ERROR_MEMORY;
  }
  dst->has_min_items = src->has_min_items;
  dst->min_items = src->min_items;
  dst->has_max_items = src->has_max_items;
  dst->max_items = src->max_items;
  dst->unique_items = src->unique_items;
  dst->items_has_min = src->items_has_min;
  dst->items_min_val = src->items_min_val;
  dst->items_exclusive_min = src->items_exclusive_min;
  dst->items_has_max = src->items_has_max;
  dst->items_max_val = src->items_max_val;
  dst->items_exclusive_max = src->items_exclusive_max;
  dst->items_has_min_len = src->items_has_min_len;
  dst->items_min_len = src->items_min_len;
  dst->items_has_max_len = src->items_has_max_len;
  dst->items_max_len = src->items_max_len;
  if (src->items_pattern) {
    c_cdd_strdup(src->items_pattern, &dst->items_pattern);
    if (!dst->items_pattern)
      return CDD_C_ERROR_MEMORY;
  }
  dst->items_has_min_items = src->items_has_min_items;
  dst->items_min_items = src->items_min_items;
  dst->items_has_max_items = src->items_has_max_items;
  dst->items_max_items = src->items_max_items;
  dst->items_unique_items = src->items_unique_items;
  if (src->items_example_set) {
    rc = copy_any_value(&dst->items_example, &src->items_example);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->items_example_set = 1;
  }
  if (src->items_examples && src->n_items_examples > 0) {
    dst->items_examples = (struct OpenAPI_Any *)C_CDD_CALLOC(
        src->n_items_examples, sizeof(struct OpenAPI_Any));
    if (!dst->items_examples)
      return CDD_C_ERROR_MEMORY;
    dst->n_items_examples = src->n_items_examples;
    for (i = 0; i < src->n_items_examples; ++i) {
      rc = copy_any_value(&dst->items_examples[i], &src->items_examples[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->items_const_value_set) {
    rc = copy_any_value(&dst->items_const_value, &src->items_const_value);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->items_const_value_set = 1;
  }
  if (src->items_default_value_set) {
    rc = copy_any_value(&dst->items_default_value, &src->items_default_value);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->items_default_value_set = 1;
  }
  if (src->items_extra_json) {
    c_cdd_strdup(src->items_extra_json, &dst->items_extra_json);
    if (!dst->items_extra_json)
      return CDD_C_ERROR_MEMORY;
  }
  dst->items_schema_is_boolean = src->items_schema_is_boolean;
  dst->items_schema_boolean_value = src->items_schema_boolean_value;
  dst->has_multiple_of = src->has_multiple_of;
  dst->multiple_of = src->multiple_of;
  dst->has_min_properties = src->has_min_properties;
  dst->min_properties = src->min_properties;
  dst->has_max_properties = src->has_max_properties;
  dst->max_properties = src->max_properties;

  if (src->all_of && src->n_all_of > 0) {
    dst->all_of = (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
        src->n_all_of, sizeof(struct OpenAPI_SchemaRef));
    if (!dst->all_of)
      return CDD_C_ERROR_MEMORY;
    dst->n_all_of = src->n_all_of;
    for (i = 0; i < src->n_all_of; ++i) {
      rc = copy_schema_ref(&dst->all_of[i], &src->all_of[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->any_of && src->n_any_of > 0) {
    dst->any_of = (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
        src->n_any_of, sizeof(struct OpenAPI_SchemaRef));
    if (!dst->any_of)
      return CDD_C_ERROR_MEMORY;
    dst->n_any_of = src->n_any_of;
    for (i = 0; i < src->n_any_of; ++i) {
      rc = copy_schema_ref(&dst->any_of[i], &src->any_of[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->one_of && src->n_one_of > 0) {
    dst->one_of = (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
        src->n_one_of, sizeof(struct OpenAPI_SchemaRef));
    if (!dst->one_of)
      return CDD_C_ERROR_MEMORY;
    dst->n_one_of = src->n_one_of;
    for (i = 0; i < src->n_one_of; ++i) {
      rc = copy_schema_ref(&dst->one_of[i], &src->one_of[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->not_schema) {
    dst->not_schema = (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
        1, sizeof(struct OpenAPI_SchemaRef));
    if (!dst->not_schema)
      return CDD_C_ERROR_MEMORY;
    rc = copy_schema_ref(dst->not_schema, src->not_schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->if_schema) {
    dst->if_schema = (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
        1, sizeof(struct OpenAPI_SchemaRef));
    if (!dst->if_schema)
      return CDD_C_ERROR_MEMORY;
    rc = copy_schema_ref(dst->if_schema, src->if_schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->then_schema) {
    dst->then_schema = (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
        1, sizeof(struct OpenAPI_SchemaRef));
    if (!dst->then_schema)
      return CDD_C_ERROR_MEMORY;
    rc = copy_schema_ref(dst->then_schema, src->then_schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->else_schema) {
    dst->else_schema = (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
        1, sizeof(struct OpenAPI_SchemaRef));
    if (!dst->else_schema)
      return CDD_C_ERROR_MEMORY;
    rc = copy_schema_ref(dst->else_schema, src->else_schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->content_schema) {
    dst->content_schema = (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
        1, sizeof(struct OpenAPI_SchemaRef));
    if (!dst->content_schema)
      return CDD_C_ERROR_MEMORY;
    rc = copy_schema_ref(dst->content_schema, src->content_schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->items_content_schema) {
    dst->items_content_schema = (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
        1, sizeof(struct OpenAPI_SchemaRef));
    if (!dst->items_content_schema)
      return CDD_C_ERROR_MEMORY;
    rc = copy_schema_ref(dst->items_content_schema, src->items_content_schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (src->n_multipart_fields > 0 && src->multipart_fields) {
    dst->multipart_fields = (struct OpenAPI_MultipartField *)C_CDD_CALLOC(
        src->n_multipart_fields, sizeof(struct OpenAPI_MultipartField));
    if (!dst->multipart_fields)
      return CDD_C_ERROR_MEMORY;
    dst->n_multipart_fields = src->n_multipart_fields;
    for (i = 0; i < src->n_multipart_fields; ++i) {
      const struct OpenAPI_MultipartField *src_field =
          &src->multipart_fields[i];
      struct OpenAPI_MultipartField *dst_field = &dst->multipart_fields[i];
      dst_field->is_binary = src_field->is_binary;
      if (src_field->name) {
        c_cdd_strdup(src_field->name, &dst_field->name);
        if (!dst_field->name)
          return CDD_C_ERROR_MEMORY;
      }
      if (src_field->type) {
        c_cdd_strdup(src_field->type, &dst_field->type);
        if (!dst_field->type)
          return CDD_C_ERROR_MEMORY;
      }
    }
  }
  return CDD_C_SUCCESS;
}

static cdd_c_error_t
copy_item_schema_as_array_inner(struct OpenAPI_SchemaRef *dst,
                                const struct OpenAPI_SchemaRef *item) {
  cdd_c_error_t rc;
  if (!dst || !item)
    return CDD_C_SUCCESS;
  rc = copy_schema_ref(dst, item);
  if (rc != CDD_C_SUCCESS)
    return rc;
  dst->is_array = 1;
  if (dst->schema_is_boolean) {
    dst->items_schema_is_boolean = 1;
    dst->items_schema_boolean_value = dst->schema_boolean_value;
    dst->schema_is_boolean = 0;
    dst->schema_boolean_value = 0;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of item schema as array.
 */
cdd_c_error_t copy_item_schema_as_array(struct OpenAPI_SchemaRef *dst,
                                        const struct OpenAPI_SchemaRef *item) {
  cdd_c_error_t rc = copy_item_schema_as_array_inner(dst, item);
  if (rc != CDD_C_SUCCESS)
    free_schema_ref_content(dst);
  return rc;
}

/**
 * @brief Creates a deep copy of any value.
 */
cdd_c_error_t copy_any_value(struct OpenAPI_Any *dst,
                             const struct OpenAPI_Any *src) {
  if (!dst || !src)
    return CDD_C_SUCCESS;
  dst->type = src->type;
  dst->number = src->number;
  dst->boolean = src->boolean;
  if (src->type == OA_ANY_STRING && src->string) {
    c_cdd_strdup(src->string, &dst->string);
    if (!dst->string)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->type == OA_ANY_JSON && src->json) {
    c_cdd_strdup(src->json, &dst->json);
    if (!dst->json)
      return CDD_C_ERROR_MEMORY;
  }
  return CDD_C_SUCCESS;
}
