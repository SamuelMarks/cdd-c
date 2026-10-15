/**
 * @file code2schema_props.c
 * @brief Property and string array collection from JSON schema.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Collects an array of strings from a JSON array.
 */
cdd_c_error_t c2s_collect_string_array(const JSON_Array *arr, char ***out,
                                       size_t *out_count) {
  size_t i, count;
  char **vals;
  if (!out || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  *out_count = 0;
  if (!arr)
    return CDD_C_SUCCESS;
  count = json_array_get_count(arr);
  if (count == 0)
    return CDD_C_SUCCESS;
  vals = (char **)C_CDD_CALLOC(count, sizeof(char *));
  if (!vals) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  for (i = 0; i < count; ++i) {
    const char *s = json_array_get_string(arr, i);
    if (s) {
      {
        cdd_c_error_t rc_c2s = c_cdd_strdup(s, &vals[i]);
        if (rc_c2s != CDD_C_SUCCESS) {
          free_string_array_code2schema(vals, count);
          return rc_c2s;
        }
      }
    }
  }
  *out = vals;
  *out_count = count;

  return CDD_C_SUCCESS;
}

/**
 * @brief Collects property names from a JSON Schema object.
 */
cdd_c_error_t c2s_collect_property_names(const JSON_Object *schema_obj,
                                         char ***out, size_t *out_count) {
  size_t i, count;
  char **vals;
  const JSON_Object *props;
  if (!out || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  *out_count = 0;
  if (!schema_obj)
    return CDD_C_SUCCESS;
  props = json_object_get_object(schema_obj, "properties");
  if (!props)
    return CDD_C_SUCCESS;
  count = json_object_get_count(props);
  if (count == 0)
    return CDD_C_SUCCESS;
  vals = (char **)C_CDD_CALLOC(count, sizeof(char *));
  if (!vals) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(props, i);
    {
      cdd_c_error_t rc_c2s = c_cdd_strdup(name, &vals[i]);
      if (rc_c2s != CDD_C_SUCCESS) {
        free_string_array_code2schema(vals, count);
        return rc_c2s;
      }
    }
  }
  *out = vals;
  *out_count = count;

  return CDD_C_SUCCESS;
}
