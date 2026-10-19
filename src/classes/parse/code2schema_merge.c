/**
 * @file code2schema_merge.c
 * @brief Merging struct fields and handling allOf composition.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Merges a source struct field into a destination struct field.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t merge_struct_field(struct StructField *dest,
                                 const struct StructField *src) {
  if (!dest || !src)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!dest->default_val[0] && src->default_val[0]) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(dest->default_val, sizeof(dest->default_val), src->default_val,
              sizeof(dest->default_val) - 1);
#else
    strncpy(dest->default_val, src->default_val, sizeof(dest->default_val) - 1);
#endif
    dest->default_val[sizeof(dest->default_val) - 1] = '\0';
  }

  if (src->required)
    dest->required = 1;

  if (src->has_min) {
    if (!dest->has_min || src->min_val > dest->min_val ||
        (src->min_val == dest->min_val && src->exclusive_min &&
         !dest->exclusive_min)) {
      dest->has_min = 1;
      dest->min_val = src->min_val;
      dest->exclusive_min = src->exclusive_min;
    }
  }

  if (src->has_max) {
    if (!dest->has_max || src->max_val < dest->max_val ||
        (src->max_val == dest->max_val && src->exclusive_max &&
         !dest->exclusive_max)) {
      dest->has_max = 1;
      dest->max_val = src->max_val;
      dest->exclusive_max = src->exclusive_max;
    }
  }

  if (src->has_min_len) {
    if (!dest->has_min_len || src->min_len > dest->min_len) {
      dest->has_min_len = 1;
      dest->min_len = src->min_len;
    }
  }

  if (src->has_max_len) {
    if (!dest->has_max_len || src->max_len < dest->max_len) {
      dest->has_max_len = 1;
      dest->max_len = src->max_len;
    }
  }

  if (src->has_min_items) {
    if (!dest->has_min_items || src->min_items > dest->min_items) {
      dest->has_min_items = 1;
      dest->min_items = src->min_items;
    }
  }

  if (src->has_max_items) {
    if (!dest->has_max_items || src->max_items < dest->max_items) {
      dest->has_max_items = 1;
      dest->max_items = src->max_items;
    }
  }

  if (src->unique_items)
    dest->unique_items = 1;

  if (!dest->pattern[0] && src->pattern[0]) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(dest->pattern, sizeof(dest->pattern), src->pattern,
              sizeof(dest->pattern) - 1);
#else
    strncpy(dest->pattern, src->pattern, sizeof(dest->pattern) - 1);
#endif
    dest->pattern[sizeof(dest->pattern) - 1] = '\0';
  }

  if (src->is_flexible_array)
    dest->is_flexible_array = 1;

  if (!dest->bit_width[0] && src->bit_width[0]) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(dest->bit_width, sizeof(dest->bit_width), src->bit_width,
              sizeof(dest->bit_width) - 1);
#else
    strncpy(dest->bit_width, src->bit_width, sizeof(dest->bit_width) - 1);
#endif
    dest->bit_width[sizeof(dest->bit_width) - 1] = '\0';
  }

  if (c2s_merge_schema_extras_strings(&dest->schema_extra_json,
                                      src->schema_extra_json) != 0) {
    /* Best-effort: ignore merge failures */
  }
  if (c2s_merge_schema_extras_strings(&dest->items_extra_json,
                                      src->items_extra_json) != 0) {
    /* Best-effort: ignore merge failures */
  }

  if (!dest->type_union && src->type_union) {
    if (copy_string_array_code2schema(&dest->type_union, &dest->n_type_union,
                                      src->type_union,
                                      src->n_type_union) != 0) {
      /* Best-effort: ignore copy failures */
    }
  }
  if (!dest->items_type_union && src->items_type_union) {
    if (copy_string_array_code2schema(
            &dest->items_type_union, &dest->n_items_type_union,
            src->items_type_union, src->n_items_type_union) != 0) {
      /* Best-effort: ignore copy failures */
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Merges source struct fields into destination struct fields.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t merge_struct_fields(struct StructFields *dest,
                                  const struct StructFields *src) {
  size_t i;

  if (!dest || !src)
    return CDD_C_SUCCESS;

  if (src->is_enum)
    return CDD_C_SUCCESS;

  if (c2s_merge_schema_extras_strings(&dest->schema_extra_json,
                                      src->schema_extra_json) != 0)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < src->size; ++i) {
    const struct StructField *src_field = &src->fields[i];
    struct StructField *dest_field = NULL;
    struct_fields_get(dest, src_field->name, &dest_field);

    if (!dest_field) {
      const char *ref = src_field->ref[0] ? src_field->ref : NULL;
      const char *def =
          src_field->default_val[0] ? src_field->default_val : NULL;
      const char *bw = src_field->bit_width[0] ? src_field->bit_width : NULL;
      if (struct_fields_add(dest, src_field->name, src_field->type, ref, def,
                            bw) != 0)
        return CDD_C_ERROR_MEMORY;
      dest_field = NULL;
      struct_fields_get(dest, src_field->name, &dest_field);

      {
        struct StructField tmp = *src_field;
        *dest_field = tmp;
        dest_field->schema_extra_json = NULL;
        dest_field->items_extra_json = NULL;
        dest_field->type_union = NULL;
        dest_field->items_type_union = NULL;
        dest_field->items_extra_json = NULL;
        dest_field->type_union = NULL;
        dest_field->n_type_union = 0;
        dest_field->items_type_union = NULL;
        dest_field->n_items_type_union = 0;
        if (src_field->schema_extra_json) {
          cdd_c_error_t rc_c2s = c_cdd_strdup(src_field->schema_extra_json,
                                              &dest_field->schema_extra_json);
          if (rc_c2s != CDD_C_SUCCESS)
            return rc_c2s;
        }
        if (src_field->items_extra_json) {
          cdd_c_error_t rc_c2s = c_cdd_strdup(src_field->items_extra_json,
                                              &dest_field->items_extra_json);
          if (rc_c2s != CDD_C_SUCCESS)
            return rc_c2s;
        }
        if (src_field->type_union) {
          if (copy_string_array_code2schema(
                  &dest_field->type_union, &dest_field->n_type_union,
                  src_field->type_union, src_field->n_type_union) != 0)
            return CDD_C_ERROR_MEMORY;
        }
        if (src_field->items_type_union) {
          if (copy_string_array_code2schema(&dest_field->items_type_union,
                                            &dest_field->n_items_type_union,
                                            src_field->items_type_union,
                                            src_field->n_items_type_union) != 0)
            return CDD_C_ERROR_MEMORY;
        }
      }
      continue;
    }

    merge_struct_field(dest_field, src_field);
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Applies an allOf JSON Schema array to a StructFields object.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t apply_allof_to_struct_fields(const JSON_Array *all_of,
                                           struct StructFields *dest,
                                           const JSON_Object *root) {
  size_t i, count;

  if (!all_of || !dest)
    return CDD_C_SUCCESS;

  count = json_array_get_count(all_of);
  for (i = 0; i < count; ++i) {
    const JSON_Object *sub = json_array_get_object(all_of, i);
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

    rc = merge_struct_fields(dest, &tmp);
    struct_fields_free(&tmp);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}
