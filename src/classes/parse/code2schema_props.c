/**
 * @file code2schema_props.c
 * @brief Property and string array collection from JSON schema.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/*
 * @brief Collects an array of strings from a JSON array.
 *
 * @param[in] arr Source Parson JSON_Array of strings.
 * @param[out] out Output pointer receiving the newly allocated string array.
 * @param[out] out_count Output pointer receiving the number of strings.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c2s_collect_string_array(const JSON_Array *arr, char ***out,
                                       size_t *out_count) {
  size_t i, count = 0;
  char **vals;
  cdd_c_error_t rc;
  if (!out || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  *out_count = 0;
  if (!arr || arr->count == 0)
    return CDD_C_SUCCESS;
  count = arr->count;
  vals = (char **)C_CDD_CALLOC(count, sizeof(char *));
  if (!vals) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  for (i = 0; i < count; ++i) {
    if (arr->items[i]->type == JSONString) {
      rc = c_cdd_strdup(arr->items[i]->string_val, &vals[i]);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, free_string_array_code2schema(vals, count));
      }
    }
  }
  *out = vals;
  *out_count = count;

  return CDD_C_SUCCESS;
}

/*
 * @brief Collects property names from a JSON Schema object.
 *
 * @param[in] schema_obj JSON Schema object containing `properties`.
 * @param[out] out Output pointer receiving the newly allocated array of
 * property names.
 * @param[out] out_count Output pointer receiving the number of property names.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c2s_collect_property_names(const JSON_Object *schema_obj,
                                         char ***out, size_t *out_count) {
  size_t i, count = 0;
  char **vals;
  JSON_Object *props = NULL;
  cdd_c_error_t rc;
  if (!out || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  *out_count = 0;
  if (!schema_obj)
    return CDD_C_SUCCESS;
  rc = cdd_json_object_get_object(schema_obj, "properties", &props);
  if (props) {
    count = props->count;
    if (count == 0)
      return rc;
    vals = (char **)C_CDD_CALLOC(count, sizeof(char *));
    if (!vals) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return cdd_c_keep_err(CDD_C_ERROR_MEMORY, rc);
    }
    for (i = 0; i < count; ++i) {
      cdd_c_error_t rc_dup = c_cdd_strdup(props->names[i], &vals[i]);
      if (rc_dup != CDD_C_SUCCESS) {
        return cdd_c_keep_err(
            rc_dup,
            cdd_c_keep_err(rc, free_string_array_code2schema(vals, count)));
      }
    }
    *out = vals;
    *out_count = count;
    return rc;
  }

  return CDD_C_SUCCESS;
}
