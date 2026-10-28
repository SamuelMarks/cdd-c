/**
 * @file code2schema_mapper.c
 * @brief Mapping JSON schema properties to struct fields.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/** @brief Keys skipped when collecting extra root schema fields */
static const char *k_schema_skip_keys[] = {
    "type", "$ref", "properties", "required", "allOf", "anyOf", "oneOf"};

/** @brief Keys skipped when collecting extra property schema fields */
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

/** @brief Keys skipped when collecting extra array items schema fields */
static const char *k_items_skip_keys[] = {"type", "$ref"};

/**
 * @brief Internal helper to convert a JSON Schema object to
 * StructFields.
 *
 * @param[in] o JSON Schema object to convert.
 * @param[in,out] f Target StructFields container to populate.
 * @param[in] root Read-only root `components.schemas` JSON_Object for `$ref`
 * resolution.
 * @param[in,out] root_mut Optional mutable root `components.schemas`
 * JSON_Object for inline schema registration.
 * @param[in] schema_name Optional name of the current schema.
 * @param[in] allow_inline_union Non-zero to permit synthesizing inline union
 * schemas.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c2s_json_object_to_struct_fields_internal_mut(
    const JSON_Object *o, struct StructFields *f, const JSON_Object *root,
    JSON_Object *root_mut, const char *schema_name, int allow_inline_union) {
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
    schema_object_is_string_enum(o, &enum_arr, &is_enum);
    if (is_enum) {
      f->is_enum = 1;
      if (enum_members_init(&f->enum_members) != CDD_C_SUCCESS)
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
    rc = apply_union_to_struct_fields_ex_mut(
        any_of, f, root, root_mut, schema_name, 1, o, allow_inline_union);
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
    rc = apply_union_to_struct_fields_ex_mut(
        one_of, f, root, root_mut, schema_name, 0, o, allow_inline_union);
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
      required_name_in_list(required, name, &is_req);
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

  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
/** @brief Mock failure counter for json_object_to_struct_fields */
C_CDD_EXPORT int g_json_object_to_struct_fields_fail = 0;
#endif

/**
 * @brief Extended function to convert a JSON Schema object to StructFields.
 *
 * @param[in] o JSON Schema object to convert.
 * @param[in,out] f Target StructFields container to populate.
 * @param[in] root Root `components.schemas` JSON_Object for `$ref` resolution.
 * @param[in] schema_name Optional name of the current schema.
 * @param[in] allow_inline_union Non-zero to permit inline union handling.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c2s_json_object_to_struct_fields_internal(
    const JSON_Object *o, struct StructFields *f, const JSON_Object *root,
    const char *schema_name, int allow_inline_union) {
  return c2s_json_object_to_struct_fields_internal_mut(
      o, f, root, NULL, schema_name, allow_inline_union);
}

/**
 * @brief Converts a JSON Schema object to `StructFields` with an explicit
 * schema name.
 *
 * @param[in] schema_obj JSON Schema object to convert.
 * @param[in,out] fields Target StructFields container to populate.
 * @param[in] schemas_obj_root Root `components.schemas` JSON_Object for `$ref`
 * resolution.
 * @param[in] schema_name Optional name of the current schema.
 * @return CDD_C_SUCCESS on success, or an error code.
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
      schema_obj, fields, schemas_obj_root, schema_name, 0);
}

/**
 * @brief Extended code generation function to convert JSON Schema to
 * StructFields.
 *
 * @param[in] schema_obj JSON Schema object to convert.
 * @param[in,out] fields Target StructFields container to populate.
 * @param[in,out] schemas_obj_root Mutable root `components.schemas`
 * JSON_Object.
 * @param[in] schema_name Optional name of the current schema.
 * @return CDD_C_SUCCESS on success, or an error code.
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
  return c2s_json_object_to_struct_fields_internal_mut(
      schema_obj, fields, schemas_obj_root, schemas_obj_root, schema_name, 1);
}

/**
 * @brief Converts a JSON Schema object to a StructFields structure.
 *
 * @param[in] schema_obj JSON Schema object to convert.
 * @param[in,out] fields Target StructFields container to populate.
 * @param[in] schemas_obj_root Root `components.schemas` JSON_Object for `$ref`
 * resolution.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t
json_object_to_struct_fields(const JSON_Object *schema_obj,
                             struct StructFields *fields,
                             const JSON_Object *schemas_obj_root) {
  return json_object_to_struct_fields_ex(schema_obj, fields, schemas_obj_root,
                                         NULL);
}
