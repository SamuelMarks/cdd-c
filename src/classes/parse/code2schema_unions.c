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
      parse_type_union_array_code2schema(item_type_arr, &items_type_union,
                                         &n_items_type_union, &primary, NULL);
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
cdd_c_error_t apply_union_to_struct_fields_ex_mut(
    const JSON_Array *union_arr, struct StructFields *dest,
    const JSON_Object *root, JSON_Object *root_mut, const char *schema_name,
    int is_anyof, const JSON_Object *schema_obj, int allow_inline) {
  size_t i, count;
  const JSON_Object *disc_obj;

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
      cdd_c_error_t rc_c2s = resolve_schema_ref_object(root, ref, &resolved);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    c2s_detect_union_json_type(resolved, &jtype);
    if (jtype == UNION_JSON_ARRAY) {
      int supported = 0;
      if (!allow_inline)
        return CDD_C_SUCCESS;
      {
        cdd_c_error_t rc_c2s = c2s_union_array_items_supported(
            resolved, root_mut ? root_mut : NULL, allow_inline, &supported);
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
      cdd_c_error_t rc_c2s = resolve_schema_ref_object(root, ref, &resolved);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }

    c2s_detect_union_json_type(resolved, &jtype);
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
      const JSON_Object *items = json_object_get_object(resolved, "items");
      const JSON_Value *items_val = json_object_get_value(resolved, "items");
      const JSON_Array *item_type_arr = NULL;

      item_ref = json_object_get_string(items, "$ref");
      if (item_ref && root) {
        int is_enum = 0;
        ref_points_to_string_enum(root, item_ref, &is_enum);
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
        rc = register_inline_schema_c2s(root_mut, schema_name, variant_name,
                                        "Item", items_val, &inline_item_ref);
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
                                   &meta->n_required_props) != 0) {
        C_CDD_FREE(inline_ref_name);
        C_CDD_FREE(inline_item_ref);
        free_string_array_code2schema(items_type_union, n_items_type_union);
        return CDD_C_ERROR_MEMORY;
      }
      if (c2s_collect_property_names(resolved, &meta->property_names,
                                     &meta->n_property_names) != 0) {
        C_CDD_FREE(inline_ref_name);
        C_CDD_FREE(inline_item_ref);
        free_string_array_code2schema(items_type_union, n_items_type_union);
        return CDD_C_ERROR_MEMORY;
      }
    }

    {
      const char *ref_al = NULL;
      char *disc_val = NULL;
      if (ref) {
        cdd_c_error_t rc_c2s = c_cdd_str_after_last(ref, '/', &ref_al);
        if (rc_c2s != CDD_C_SUCCESS) {
          C_CDD_FREE(inline_ref_name);
          C_CDD_FREE(inline_item_ref);
          free_string_array_code2schema(items_type_union, n_items_type_union);
          return rc_c2s;
        }
      }
      discriminator_value_for_variant(disc_obj, ref ? ref_al : name_hint, ref,
                                      &disc_val);
      meta->disc_value = disc_val;
    }

    C_CDD_FREE(inline_ref_name);
    C_CDD_FREE(inline_item_ref);
    free_string_array_code2schema(items_type_union, n_items_type_union);
  }

  return CDD_C_SUCCESS;
}

cdd_c_error_t apply_union_to_struct_fields_ex(
    const JSON_Array *union_arr, struct StructFields *dest, JSON_Object *root,
    const char *schema_name, int is_anyof, const JSON_Object *schema_obj,
    int allow_inline) {
  return apply_union_to_struct_fields_ex_mut(union_arr, dest, root, root,
                                             schema_name, is_anyof, schema_obj,
                                             allow_inline);
}
