/**
 * @file openapi_schema_ref.c
 * @brief Schema reference parser implementation.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

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
                                           "writeOnly"};

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
                                          "writeOnly"};

/**
 * @brief Parses schema ref from the given input.
 */
cdd_c_error_t parse_schema_ref(const JSON_Object *schema,
                               struct OpenAPI_SchemaRef *out,
                               const struct OpenAPI_Spec *spec) {
  char *_ast_parse_schema_type_41 = NULL;
  struct ResolvedRefTarget _ast_resolve_ref_target_42;
  char *_ast_ref_name_from_prefix_43 = NULL;
  char *_ast_json_pointer_unescape_44 = NULL;
  char *_ast_parse_schema_type_45 = NULL;
  struct ResolvedRefTarget _ast_resolve_ref_target_46;
  char *_ast_ref_name_from_prefix_47 = NULL;
  char *_ast_json_pointer_unescape_48 = NULL;
  char *_ast_strdup_180 = NULL;
  char *_ast_strdup_181 = NULL;
  char *_ast_strdup_182 = NULL;
  char *_ast_strdup_183 = NULL;
  char *_ast_strdup_184 = NULL;
  char *_ast_strdup_185 = NULL;
  char *_ast_strdup_186 = NULL;
  char *_ast_strdup_187 = NULL;
  char *_ast_strdup_188 = NULL;
  char *_ast_strdup_189 = NULL;
  char *_ast_strdup_190 = NULL;
  char *_ast_strdup_191 = NULL;
  const char *ref = json_object_get_string(schema, "$ref");
  const char *dynamic_ref = json_object_get_string(schema, "$dynamicRef");
  const char *ref_val = ref ? ref : dynamic_ref;
  const int ref_is_dynamic = (!ref && dynamic_ref);
  const char *summary = json_object_get_string(schema, "summary");
  const char *desc = json_object_get_string(schema, "description");
  const char *format = json_object_get_string(schema, "format");
  const char *content_media_type =
      json_object_get_string(schema, "contentMediaType");
  const char *content_encoding =
      json_object_get_string(schema, "contentEncoding");
  const JSON_Array *type_arr;
  const JSON_Array *examples_arr;
  const JSON_Array *enum_arr;
  const char *type;
  int nullable = 0;
  int deprecated_present;
  int deprecated_val;
  int read_only_present;
  int read_only_val;
  int write_only_present;
  int write_only_val;

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!schema)
    return CDD_C_SUCCESS;

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
  out->items_schema_is_boolean = 0;
  out->items_schema_boolean_value = 0;
  out->multipart_fields = NULL;
  out->n_multipart_fields = 0;

  type_arr = json_object_get_array(schema, "type");
  if (type_arr) {
    {
      cdd_c_error_t _rc = parse_string_enum_array(type_arr, &out->type_union,
                                                  &out->n_type_union);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  type = (parse_schema_type(schema, &nullable, &_ast_parse_schema_type_41),
          _ast_parse_schema_type_41);
  out->nullable = nullable;

  if (json_object_has_value(schema, "allOf")) {
    cdd_c_error_t rc =
        parse_schema_array_ref(json_object_get_array(schema, "allOf"),
                               &out->all_of, &out->n_all_of, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (json_object_has_value(schema, "anyOf")) {
    cdd_c_error_t rc =
        parse_schema_array_ref(json_object_get_array(schema, "anyOf"),
                               &out->any_of, &out->n_any_of, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (json_object_has_value(schema, "oneOf")) {
    cdd_c_error_t rc =
        parse_schema_array_ref(json_object_get_array(schema, "oneOf"),
                               &out->one_of, &out->n_one_of, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (json_object_has_value(schema, "not")) {
    cdd_c_error_t rc = parse_schema_ref_ptr(
        json_object_get_object(schema, "not"), &out->not_schema, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (json_object_has_value(schema, "if")) {
    cdd_c_error_t rc = parse_schema_ref_ptr(
        json_object_get_object(schema, "if"), &out->if_schema, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (json_object_has_value(schema, "then")) {
    cdd_c_error_t rc = parse_schema_ref_ptr(
        json_object_get_object(schema, "then"), &out->then_schema, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (json_object_has_value(schema, "else")) {
    cdd_c_error_t rc = parse_schema_ref_ptr(
        json_object_get_object(schema, "else"), &out->else_schema, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (json_object_has_value(schema, "contentSchema")) {
    const JSON_Object *cs_obj = json_object_get_object(schema, "contentSchema");
    if (cs_obj) {
      cdd_c_error_t rc =
          parse_schema_ref_ptr(cs_obj, &out->content_schema, spec);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  {
    cdd_c_error_t _rc = parse_any_field(schema, "default", &out->default_value,
                                        &out->default_value_set);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }
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
    {
      cdd_c_error_t _rc = parse_schema_constraints(schema, &target);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  enum_arr = json_object_get_array(schema, "enum");
  if (enum_arr) {
    {
      cdd_c_error_t _rc =
          parse_any_array(enum_arr, &out->enum_values, &out->n_enum_values);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  if (format) {
    out->format = (c_cdd_strdup(format, &_ast_strdup_180), _ast_strdup_180);
    if (!out->format)
      return CDD_C_ERROR_MEMORY;
  }

  if (content_media_type) {
    out->content_media_type =
        (c_cdd_strdup(content_media_type, &_ast_strdup_181), _ast_strdup_181);
    if (!out->content_media_type)
      return CDD_C_ERROR_MEMORY;
  }
  if (content_encoding) {
    out->content_encoding =
        (c_cdd_strdup(content_encoding, &_ast_strdup_182), _ast_strdup_182);
    if (!out->content_encoding)
      return CDD_C_ERROR_MEMORY;
  }

  if (ref_val && summary) {
    out->summary = (c_cdd_strdup(summary, &_ast_strdup_183), _ast_strdup_183);
    if (!out->summary)
      return CDD_C_ERROR_MEMORY;
  }

  if (desc) {
    out->description = (c_cdd_strdup(desc, &_ast_strdup_184), _ast_strdup_184);
    if (!out->description)
      return CDD_C_ERROR_MEMORY;
  }

  {
    const JSON_Object *ext_docs =
        json_object_get_object(schema, "externalDocs");
    if (ext_docs) {
      out->external_docs_set = 1;
      {
        cdd_c_error_t rc = parse_external_docs(ext_docs, &out->external_docs);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }

  {
    const JSON_Object *disc_obj =
        json_object_get_object(schema, "discriminator");
    if (disc_obj) {
      out->discriminator_set = 1;
      {
        cdd_c_error_t _rc =
            parse_discriminator_object(disc_obj, &out->discriminator);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }

  {
    const JSON_Object *xml_obj = json_object_get_object(schema, "xml");
    if (xml_obj) {
      out->xml_set = 1;
      {
        cdd_c_error_t _rc = parse_xml_object(xml_obj, &out->xml);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }

  deprecated_present = json_object_has_value(schema, "deprecated");
  deprecated_val = json_object_get_boolean(schema, "deprecated");
  if (deprecated_present) {
    out->deprecated_set = 1;
    out->deprecated = (deprecated_val == 1);
  }
  read_only_present = json_object_has_value(schema, "readOnly");
  read_only_val = json_object_get_boolean(schema, "readOnly");
  if (read_only_present) {
    out->read_only_set = 1;
    out->read_only = (read_only_val == 1);
  }
  write_only_present = json_object_has_value(schema, "writeOnly");
  write_only_val = json_object_get_boolean(schema, "writeOnly");
  if (write_only_present) {
    out->write_only_set = 1;
    out->write_only = (write_only_val == 1);
  }

  {
    cdd_c_error_t _rc = parse_any_field(schema, "const", &out->const_value,
                                        &out->const_value_set);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  examples_arr = json_object_get_array(schema, "examples");
  if (examples_arr) {
    {
      cdd_c_error_t _rc =
          parse_any_array(examples_arr, &out->examples, &out->n_examples);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  if (ref_val) {
    struct ResolvedRefTarget resolved =
        (resolve_ref_target(spec, ref_val, &_ast_resolve_ref_target_42),
         _ast_resolve_ref_target_42);
    const struct OpenAPI_Spec *target = resolved.spec;
    const char *name_enc =
        (ref_name_from_prefix(target, resolved.ref, "#/components/schemas/",
                              &_ast_ref_name_from_prefix_43),
         _ast_ref_name_from_prefix_43);
    char *name_dec = NULL;
    out->ref = (c_cdd_strdup(ref_val, &_ast_strdup_185), _ast_strdup_185);
    if (!out->ref) {
      if (resolved.resolved_ref)
        free(resolved.resolved_ref);
      return CDD_C_ERROR_MEMORY;
    }
    out->ref_is_dynamic = ref_is_dynamic ? 1 : 0;
    if (name_enc) {
      name_dec =
          (json_pointer_unescape(name_enc, &_ast_json_pointer_unescape_44),
           _ast_json_pointer_unescape_44);
      if (!name_dec) {
        if (resolved.resolved_ref)
          free(resolved.resolved_ref);
        return CDD_C_ERROR_MEMORY;
      }
      out->ref_name = name_dec;
    }
    if (resolved.resolved_ref)
      free(resolved.resolved_ref);
    return CDD_C_SUCCESS;
  }

  if (type && strcmp(type, "array") == 0) {
    const JSON_Value *items_val = json_object_get_value(schema, "items");
    const JSON_Object *items =
        items_val ? json_value_get_object(items_val) : NULL;
    out->is_array = 1;
    if (items_val && json_value_get_type(items_val) == JSONBoolean) {
      out->items_schema_is_boolean = 1;
      out->items_schema_boolean_value = json_value_get_boolean(items_val);
      return CDD_C_SUCCESS;
    }
    if (items) {
      const char *item_ref = json_object_get_string(items, "$ref");
      const char *item_dynamic_ref =
          json_object_get_string(items, "$dynamicRef");
      const char *item_ref_val = item_ref ? item_ref : item_dynamic_ref;
      const int item_ref_is_dynamic = (!item_ref && item_dynamic_ref);
      const char *item_format = json_object_get_string(items, "format");
      const char *item_type = NULL;
      const JSON_Array *item_types_arr = NULL;
      const JSON_Array *item_enum = NULL;
      const JSON_Array *item_examples = NULL;
      int items_nullable = 0;
      const char *item_content_media_type =
          json_object_get_string(items, "contentMediaType");
      const char *item_content_encoding =
          json_object_get_string(items, "contentEncoding");
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
        {
          cdd_c_error_t _rc = parse_schema_constraints(items, &target);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
      item_type = (parse_schema_type(items, &items_nullable,
                                     &_ast_parse_schema_type_45),
                   _ast_parse_schema_type_45);
      out->items_nullable = items_nullable;
      item_types_arr = json_object_get_array(items, "type");
      if (item_types_arr) {
        {
          cdd_c_error_t _rc = parse_string_enum_array(
              item_types_arr, &out->items_type_union, &out->n_items_type_union);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
      if (item_format) {
        out->items_format =
            (c_cdd_strdup(item_format, &_ast_strdup_186), _ast_strdup_186);
        if (!out->items_format)
          return CDD_C_ERROR_MEMORY;
      }
      if (item_content_media_type) {
        out->items_content_media_type =
            (c_cdd_strdup(item_content_media_type, &_ast_strdup_187),
             _ast_strdup_187);
        if (!out->items_content_media_type)
          return CDD_C_ERROR_MEMORY;
      }
      if (item_content_encoding) {
        out->items_content_encoding =
            (c_cdd_strdup(item_content_encoding, &_ast_strdup_188),
             _ast_strdup_188);
        if (!out->items_content_encoding)
          return CDD_C_ERROR_MEMORY;
      }
      item_enum = json_object_get_array(items, "enum");
      if (item_enum) {
        {
          cdd_c_error_t _rc = parse_any_array(
              item_enum, &out->items_enum_values, &out->n_items_enum_values);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
      item_examples = json_object_get_array(items, "examples");
      if (item_examples) {
        {
          cdd_c_error_t _rc = parse_any_array(
              item_examples, &out->items_examples, &out->n_items_examples);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
      {
        cdd_c_error_t _rc =
            parse_any_field(items, "const", &out->items_const_value,
                            &out->items_const_value_set);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      {
        cdd_c_error_t _rc =
            parse_any_field(items, "default", &out->items_default_value,
                            &out->items_default_value_set);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      if (json_object_has_value(items, "contentSchema")) {
        const JSON_Object *cs_obj =
            json_object_get_object(items, "contentSchema");
        if (cs_obj) {
          cdd_c_error_t rc =
              parse_schema_ref_ptr(cs_obj, &out->items_content_schema, spec);
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
        struct ResolvedRefTarget resolved =
            (resolve_ref_target(spec, item_ref_val,
                                &_ast_resolve_ref_target_46),
             _ast_resolve_ref_target_46);
        const struct OpenAPI_Spec *target = resolved.spec;
        const char *name_enc =
            (ref_name_from_prefix(target, resolved.ref, "#/components/schemas/",
                                  &_ast_ref_name_from_prefix_47),
             _ast_ref_name_from_prefix_47);
        char *name_dec = NULL;
        out->items_ref =
            (c_cdd_strdup(item_ref_val, &_ast_strdup_189), _ast_strdup_189);
        if (!out->items_ref) {
          if (resolved.resolved_ref)
            free(resolved.resolved_ref);
          return CDD_C_ERROR_MEMORY;
        }
        out->items_ref_is_dynamic = item_ref_is_dynamic ? 1 : 0;
        if (name_enc) {
          name_dec =
              (json_pointer_unescape(name_enc, &_ast_json_pointer_unescape_48),
               _ast_json_pointer_unescape_48);
          if (!name_dec) {
            if (resolved.resolved_ref)
              free(resolved.resolved_ref);
            return CDD_C_ERROR_MEMORY;
          }
          out->ref_name = name_dec;
        }
        if (resolved.resolved_ref)
          free(resolved.resolved_ref);
        return CDD_C_SUCCESS;
      }
      if (item_type) {
        out->inline_type =
            (c_cdd_strdup(item_type, &_ast_strdup_190), _ast_strdup_190);
        return out->inline_type ? 0 : ENOMEM;
      }
    }
    return CDD_C_SUCCESS;
  }

  if (type) {
    out->inline_type = (c_cdd_strdup(type, &_ast_strdup_191), _ast_strdup_191);
    return out->inline_type ? 0 : ENOMEM;
  }

  return CDD_C_SUCCESS;
}
