/**
 * @file code2schema_unions.c
 * @brief Union composition (oneOf/anyOf) handling for struct fields.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/*
 * @brief Fallback method to apply a union (oneOf/anyOf) to StructFields.
 *
 * @param[in] union_arr JSON_Array of `oneOf` or `anyOf` sub-schemas.
 * @param[in,out] dest Target StructFields container.
 * @param[in] root Root `components.schemas` JSON_Object for `$ref` resolution.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t apply_union_to_struct_fields_fallback(const JSON_Array *union_arr,
                                                    struct StructFields *dest,
                                                    const JSON_Object *root) {
  size_t i, count = 0;
  cdd_c_error_t rc_top = CDD_C_SUCCESS;

  if (!union_arr || !dest)
    return CDD_C_SUCCESS;

  if (dest->size > 0)
    return CDD_C_SUCCESS;

  rc_top = cdd_json_array_get_count(union_arr, &count);
  for (i = 0; i < count; ++i) {
    JSON_Object *sub = NULL;
    const JSON_Object *resolved = NULL;
    const char *ref = NULL;
    struct StructFields tmp;
    cdd_c_error_t rc;

    if (cdd_json_array_get_object(union_arr, i, &sub) != CDD_C_SUCCESS)
      sub = NULL;
    resolved = sub;
    if (!sub)
      continue;

    if (cdd_json_object_get_string(sub, "$ref", &ref) != CDD_C_SUCCESS)
      ref = NULL;
    if (ref && root) {
      cdd_c_error_t rc_c2s = resolve_schema_ref_object(root, ref, &resolved);
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
      return (cdd_c_error_t)((int)rc | (int)struct_fields_free(&tmp));
    }

    if (tmp.size > 0) {
      rc = merge_struct_fields(dest, &tmp);
      return (cdd_c_error_t)((int)rc | (int)struct_fields_free(&tmp));
    }

    rc_top = struct_fields_free(&tmp);
  }

  return rc_top;
}

/*
 * @brief Checks if array items within a union are supported.
 *
 * @param[in] schema_obj Array variant JSON Schema object to inspect.
 * @param[in] root Root `components.schemas` JSON_Object.
 * @param[in] allow_inline Non-zero if inline item object schemas are permitted.
 * @param[out] out_supported Output pointer receiving 1 if supported, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c2s_union_array_items_supported(const JSON_Object *schema_obj,
                                              const JSON_Object *root,
                                              int allow_inline,
                                              int *out_supported) {
  JSON_Object *items = NULL;
  JSON_Array *item_type_arr = NULL;
  const char *item_ref = NULL;
  const char *item_type = NULL;
  char **items_type_union = NULL;
  size_t n_items_type_union = 0;
  const char *primary = NULL;
  if (!out_supported)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_supported = 0;

  if (!schema_obj)
    return CDD_C_SUCCESS;

  if (cdd_json_object_get_object(schema_obj, "items", &items) != CDD_C_SUCCESS)
    return CDD_C_SUCCESS;

  if (cdd_json_object_get_string(items, "$ref", &item_ref) != CDD_C_SUCCESS)
    item_ref = NULL;
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

  if (cdd_json_object_get_string(items, "type", &item_type) != CDD_C_SUCCESS)
    item_type = NULL;
  if (!item_ref && !item_type) {
    if (cdd_json_object_get_array(items, "type", &item_type_arr) !=
        CDD_C_SUCCESS)
      item_type_arr = NULL;
    if (item_type_arr) {
      cdd_c_error_t rc_tu = parse_type_union_array_code2schema(
          item_type_arr, &items_type_union, &n_items_type_union, &primary,
          NULL);
      if (rc_tu != CDD_C_SUCCESS)
        return rc_tu;
      item_type = primary;
    } else {
      JSON_Object *item_props = NULL;
      if (cdd_json_object_get_object(items, "properties", &item_props) ==
          CDD_C_SUCCESS)
        item_type = "object";
    }
  }

  if (item_ref) {
    *out_supported = 1;
    return free_string_array_code2schema(items_type_union, n_items_type_union);
  }
  if (!item_type) {
    return free_string_array_code2schema(items_type_union, n_items_type_union);
  }
  if (strcmp(item_type, "array") == 0) {
    return free_string_array_code2schema(items_type_union, n_items_type_union);
  }
  if (strcmp(item_type, "object") == 0 && (!allow_inline || !root)) {
    return free_string_array_code2schema(items_type_union, n_items_type_union);
  }

  *out_supported = 1;
  return free_string_array_code2schema(items_type_union, n_items_type_union);
}

/*
 * @brief Extended method to apply a union (oneOf/anyOf) to StructFields.
 *
 * @param[in] union_arr JSON_Array of `oneOf` or `anyOf` sub-schemas.
 * @param[in,out] dest Target StructFields container.
 * @param[in] root Read-only root `components.schemas` JSON_Object.
 * @param[in,out] root_mut Optional mutable root `components.schemas`
 * JSON_Object.
 * @param[in] schema_name Optional name of the enclosing union schema.
 * @param[in] is_anyof Non-zero if `anyOf`, 0 if `oneOf`.
 * @param[in] schema_obj Enclosing union JSON_Object (for `discriminator`
 * lookup).
 * @param[in] allow_inline Non-zero if inline variant schemas may be registered.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t apply_union_to_struct_fields_ex_mut(
    const JSON_Array *union_arr, struct StructFields *dest,
    const JSON_Object *root, JSON_Object *root_mut, const char *schema_name,
    int is_anyof, const JSON_Object *schema_obj, int allow_inline) {
  size_t i, count = 0;
  JSON_Object *disc_obj = NULL;
  cdd_c_error_t rc_top = CDD_C_SUCCESS;

  if (!union_arr || !dest)
    return CDD_C_SUCCESS;

  if (dest->size > 0)
    return CDD_C_SUCCESS;

  rc_top = cdd_json_array_get_count(union_arr, &count);
  if (count == 0)
    return rc_top;

  /* Validate variant support */
  for (i = 0; i < count; ++i) {
    JSON_Object *sub = NULL;
    const JSON_Object *resolved = NULL;
    const char *ref = NULL;
    enum UnionVariantJsonType jtype = UNION_JSON_UNKNOWN;
    if (cdd_json_array_get_object(union_arr, i, &sub) != CDD_C_SUCCESS)
      sub = NULL;
    resolved = sub;
    if (!sub)
      continue;
    if (cdd_json_object_get_string(sub, "$ref", &ref) != CDD_C_SUCCESS)
      ref = NULL;
    if (ref && root) {
      rc_top = resolve_schema_ref_object(root, ref, &resolved);
      if (rc_top != CDD_C_SUCCESS)
        return rc_top;
    }
    rc_top = c2s_detect_union_json_type(resolved, &jtype);
    if (jtype == UNION_JSON_ARRAY) {
      int supported = 0;
      if (!allow_inline)
        return CDD_C_SUCCESS;
      {
        cdd_c_error_t rc_c2s = c2s_union_array_items_supported(
            resolved, root_mut, allow_inline, &supported);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
      if (!supported)
        return CDD_C_SUCCESS;
    }
    if (jtype == UNION_JSON_OBJECT && !ref) {
      if (!allow_inline || !root_mut)
        return CDD_C_SUCCESS;
    }
    if (jtype == UNION_JSON_UNKNOWN)
      return CDD_C_SUCCESS;
  }

  dest->is_union = 1;
  dest->union_is_anyof = is_anyof ? 1 : 0;

  if (schema_obj) {
    if (cdd_json_object_get_object(schema_obj, "discriminator", &disc_obj) !=
        CDD_C_SUCCESS)
      disc_obj = NULL;
  }
  if (disc_obj) {
    const char *prop = NULL;
    if (cdd_json_object_get_string(disc_obj, "propertyName", &prop) !=
        CDD_C_SUCCESS)
      prop = NULL;
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
    JSON_Value *sub_val = NULL;
    JSON_Object *sub = NULL;
    const JSON_Object *resolved = NULL;
    const char *ref = NULL;
    enum UnionVariantJsonType jtype = UNION_JSON_UNKNOWN;
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

    rc = cdd_json_array_get_value(union_arr, i, &sub_val);
    if (cdd_json_value_get_object(sub_val, &sub) != CDD_C_SUCCESS)
      sub = NULL;
    resolved = sub;

    if (!sub)
      continue;

    if (cdd_json_object_get_string(sub, "$ref", &ref) != CDD_C_SUCCESS)
      ref = NULL;
    if (ref) {
      rc = resolve_schema_ref_object(root, ref, &resolved);
    }

    rc = c2s_detect_union_json_type(resolved, &jtype);
    meta->json_type = jtype;

    if (ref) {
      rc = c_cdd_str_after_last(ref, '/', &name_hint);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      if (cdd_json_object_get_string(resolved, "title", &name_hint) !=
          CDD_C_SUCCESS)
        name_hint = NULL;
      if (!name_hint) {
        if (cdd_json_object_get_string(resolved, "type", &name_hint) !=
            CDD_C_SUCCESS)
          name_hint = NULL;
      }
    }
    if (!name_hint)
      name_hint = schema_name ? schema_name : "Variant";

    {
      cdd_c_error_t rc_c2s =
          make_unique_variant_name(dest, name_hint, i, &variant_name);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }

    if (jtype == UNION_JSON_OBJECT && !ref) {
      rc = register_inline_schema_c2s(root_mut, schema_name, variant_name, NULL,
                                      sub_val, &inline_ref_name);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(variant_name);
        return rc;
      }
      ref = inline_ref_name;
    }

    if (jtype == UNION_JSON_ARRAY) {
      JSON_Object *items = NULL;
      JSON_Value *items_val = NULL;
      JSON_Array *item_type_arr = NULL;

      rc = cdd_json_object_get_object(resolved, "items", &items);
      rc = cdd_json_object_get_value(resolved, "items", &items_val);

      if (cdd_json_object_get_string(items, "$ref", &item_ref) != CDD_C_SUCCESS)
        item_ref = NULL;
      if (item_ref && root) {
        int is_enum = 0;
        rc = ref_points_to_string_enum(root, item_ref, &is_enum);
        if (is_enum) {
          item_ref = NULL;
          item_type = "string";
        }
      }
      if (!item_type) {
        if (cdd_json_object_get_string(items, "type", &item_type) !=
            CDD_C_SUCCESS)
          item_type = NULL;
      }
      if (!item_ref && !item_type) {
        if (cdd_json_object_get_array(items, "type", &item_type_arr) !=
            CDD_C_SUCCESS)
          item_type_arr = NULL;
        if (item_type_arr) {
          rc = parse_type_union_array_code2schema(
              item_type_arr, &items_type_union, &n_items_type_union, &item_type,
              NULL);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(variant_name);
            return rc;
          }
        } else {
          JSON_Object *item_props = NULL;
          rc = cdd_json_object_get_object(items, "properties", &item_props);
          item_type = "object";
        }
      }
      if (!item_ref && strcmp(item_type, "object") == 0) {
        rc = register_inline_schema_c2s(root_mut, schema_name, variant_name,
                                        "Item", items_val, &inline_item_ref);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(variant_name);
          return (cdd_c_error_t)((int)rc |
                                 (int)free_string_array_code2schema(
                                     items_type_union, n_items_type_union));
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
      return (cdd_c_error_t)((int)CDD_C_ERROR_MEMORY |
                             (int)free_string_array_code2schema(
                                 items_type_union, n_items_type_union));
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
      JSON_Array *required = NULL;
      if (cdd_json_object_get_array(resolved, "required", &required) !=
          CDD_C_SUCCESS)
        required = NULL;
      if (c2s_collect_string_array(required, &meta->required_props,
                                   &meta->n_required_props) != 0) {
        C_CDD_FREE(inline_ref_name);
        return CDD_C_ERROR_MEMORY;
      }
      if (c2s_collect_property_names(resolved, &meta->property_names,
                                     &meta->n_property_names) != 0) {
        C_CDD_FREE(inline_ref_name);
        return CDD_C_ERROR_MEMORY;
      }
    }

    {
      const char *ref_al = NULL;
      char *disc_val = NULL;
      if (ref) {
        rc = c_cdd_str_after_last(ref, '/', &ref_al);
      }
      rc = discriminator_value_for_variant(disc_obj, ref ? ref_al : name_hint,
                                           ref, &disc_val);
      if (rc != CDD_C_SUCCESS)
        disc_val = NULL;
      meta->disc_value = disc_val;
    }

    C_CDD_FREE(inline_ref_name);
    C_CDD_FREE(inline_item_ref);
    rc_top =
        free_string_array_code2schema(items_type_union, n_items_type_union);
  }

  return rc_top;
}

/*
 * @brief Applies a `oneOf`/`anyOf` JSON array to `StructFields` using `root` as
 * both read and mutable root.
 *
 * @param[in] union_arr JSON_Array of `oneOf` or `anyOf` sub-schemas.
 * @param[in,out] dest Target StructFields container.
 * @param[in,out] root Mutable root `components.schemas` JSON_Object.
 * @param[in] schema_name Optional name of the enclosing union schema.
 * @param[in] is_anyof Non-zero if `anyOf`, 0 if `oneOf`.
 * @param[in] schema_obj Enclosing union JSON_Object.
 * @param[in] allow_inline Non-zero if inline variant schemas may be registered.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t apply_union_to_struct_fields_ex(
    const JSON_Array *union_arr, struct StructFields *dest, JSON_Object *root,
    const char *schema_name, int is_anyof, const JSON_Object *schema_obj,
    int allow_inline) {
  return apply_union_to_struct_fields_ex_mut(union_arr, dest, root, root,
                                             schema_name, is_anyof, schema_obj,
                                             allow_inline);
}
