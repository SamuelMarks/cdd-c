/**
 * @file code2schema_extras.c
 * @brief Schema extras collection and merging routines.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/*
 * @brief Collects non-standard/extra JSON Schema keys not present in
 * `skip_keys` and serializes them to a JSON string.
 *
 * @param[in] obj Source JSON_Object to inspect.
 * @param[in] skip_keys Array of standard key names to ignore.
 * @param[in] skip_count Number of entries in `skip_keys`.
 * @param[out] out_json Output pointer receiving the serialized JSON extras
 * string (or NULL if none).
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c2s_collect_schema_extras(const JSON_Object *obj,
                                        const char **skip_keys,
                                        size_t skip_count, char **out_json) {
  JSON_Value *extras_val = NULL;
  JSON_Object *extras_obj = NULL;
  size_t i, count = 0, extras_count = 0;
  char *serialized = NULL;

  if (out_json)
    *out_json = NULL;
  if (!obj || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&extras_val);
  if (!extras_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  g_cdd_last_alloc_err = cdd_json_value_get_object(extras_val, &extras_obj);

  g_cdd_last_alloc_err = cdd_json_object_get_count(obj, &count);
  for (i = 0; i < count; ++i) {
    const char *key = NULL;
    JSON_Value *val = NULL;
    JSON_Value *copy = NULL;
    int is_skip = 0;

    g_cdd_last_alloc_err = cdd_json_object_get_name(obj, i, &key);
    g_cdd_last_alloc_err =
        c2s_key_in_list(key, skip_keys, skip_count, &is_skip);
    if (is_skip)
      continue;
    g_cdd_last_alloc_err = cdd_json_object_get_value(obj, key, &val);
    {
      cdd_c_error_t rc_c2s = c2s_clone_json_value(val, &copy);
      if (rc_c2s != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
        return rc_c2s;
      }
    }
    if (cdd_json_object_set_value(extras_obj, key, copy) != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(copy);
      g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  g_cdd_last_alloc_err = cdd_json_object_get_count(extras_obj, &extras_count);
  if (extras_count == 0) {
    g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
    return CDD_C_SUCCESS;
  }

  g_cdd_last_alloc_err = cdd_json_serialize_to_string(extras_val, &serialized);
  if (!serialized) {
    g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
    return CDD_C_ERROR_MEMORY;
  }
  {
    cdd_c_error_t rc_c2s = c_cdd_strdup(serialized, out_json);
    if (rc_c2s != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_free_serialized_string(serialized);
      g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
      return rc_c2s;
    }
  }
  g_cdd_last_alloc_err = cdd_json_free_serialized_string(serialized);
  g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
  return *out_json ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/*
 * @brief Merges extra attributes described by a JSON string into a
 * target parson JSON Object.
 *
 * Parses the JSON string `extras_json`, clones each value, and sets them
 * directly on `target`. Returns early if there are no extras. Does not
 * override existing keys on `target`.
 *
 * @param[in,out] target Target JSON_Object to merge extra properties into.
 * @param[in] extras_json Serialized JSON object string containing extra
 * properties.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c2s_merge_schema_extras_object(JSON_Object *target,
                                             const char *extras_json) {
  JSON_Value *extras_val = NULL;
  JSON_Object *extras_obj = NULL;
  size_t i, count = 0;
  if (!target || !extras_json || extras_json[0] == '\0')
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_parse_string(extras_json, &extras_val);
  if (!extras_val)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_value_get_object(extras_val, &extras_obj);
  if (!extras_obj) {
    g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
    return CDD_C_SUCCESS;
  }

  g_cdd_last_alloc_err = cdd_json_object_get_count(extras_obj, &count);
  for (i = 0; i < count; ++i) {
    const char *key = NULL;
    JSON_Value *val = NULL;
    JSON_Value *copy = NULL;
    int has_val = 0;

    g_cdd_last_alloc_err = cdd_json_object_get_name(extras_obj, i, &key);
    g_cdd_last_alloc_err = cdd_json_object_has_value(target, key, &has_val);
    if (has_val)
      continue;
    g_cdd_last_alloc_err = cdd_json_object_get_value(extras_obj, key, &val);
    {
      cdd_c_error_t rc_c2s = c2s_clone_json_value(val, &copy);
      if (rc_c2s != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
        return rc_c2s;
      }
    }
    if (
#ifdef CDD_BUILD_TESTS
        g_cdd_fail_json_set_value ||
#endif
        cdd_json_object_set_value(target, key, copy) != CDD_C_SUCCESS) {
#ifdef CDD_BUILD_TESTS
      g_cdd_fail_json_set_value = 0;
#endif
      g_cdd_last_alloc_err = cdd_json_value_free(copy);
      g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  g_cdd_last_alloc_err = cdd_json_value_free(extras_val);

  return CDD_C_SUCCESS;
}

/*
 * @brief Merges two serialized JSON objects containing extra schemas
 * together.
 *
 * Parses `dest_json` and `src_json`. Clones the properties from the
 * `src` object into `dest`, serializing the result back out into
 * `dest_json`. If `dest_json` was NULL, `src_json` is effectively
 * duplicated into it.
 *
 * @param[in,out] dest_json Pointer to destination JSON string pointer to
 * update.
 * @param[in] src_json Source serialized JSON object string to merge in.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c2s_merge_schema_extras_strings(char **dest_json,
                                              const char *src_json) {
  JSON_Value *dest_val = NULL;
  JSON_Value *src_val = NULL;
  JSON_Object *dest_obj = NULL;
  JSON_Object *src_obj = NULL;
  size_t i, count = 0;
  char *serialized = NULL;

  if (!dest_json || !src_json || src_json[0] == '\0')
    return CDD_C_SUCCESS;
  if (!*dest_json) {
    {
      cdd_c_error_t rc_c2s = c_cdd_strdup(src_json, dest_json);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    return *dest_json ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }

  g_cdd_last_alloc_err = cdd_json_parse_string(*dest_json, &dest_val);
  g_cdd_last_alloc_err = cdd_json_parse_string(src_json, &src_val);
  if (!dest_val || !src_val) {
    if (dest_val)
      g_cdd_last_alloc_err = cdd_json_value_free(dest_val);
    if (src_val)
      g_cdd_last_alloc_err = cdd_json_value_free(src_val);
    return CDD_C_SUCCESS;
  }
  g_cdd_last_alloc_err = cdd_json_value_get_object(dest_val, &dest_obj);
  g_cdd_last_alloc_err = cdd_json_value_get_object(src_val, &src_obj);
  if (!dest_obj || !src_obj) {
    g_cdd_last_alloc_err = cdd_json_value_free(dest_val);
    g_cdd_last_alloc_err = cdd_json_value_free(src_val);
    return CDD_C_SUCCESS;
  }

  g_cdd_last_alloc_err = cdd_json_object_get_count(src_obj, &count);
  for (i = 0; i < count; ++i) {
    const char *key = NULL;
    JSON_Value *val = NULL;
    JSON_Value *copy = NULL;
    int has_val = 0;

    g_cdd_last_alloc_err = cdd_json_object_get_name(src_obj, i, &key);
    g_cdd_last_alloc_err = cdd_json_object_has_value(dest_obj, key, &has_val);
    if (has_val)
      g_cdd_last_alloc_err = cdd_json_object_remove(dest_obj, key);
    g_cdd_last_alloc_err = cdd_json_object_get_value(src_obj, key, &val);
    {
      cdd_c_error_t rc_c2s = c2s_clone_json_value(val, &copy);
      if (rc_c2s != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_json_value_free(dest_val);
        g_cdd_last_alloc_err = cdd_json_value_free(src_val);
        return rc_c2s;
      }
    }
#ifdef CDD_BUILD_TESTS
    if (g_cdd_fail_json_set_value) {
      g_cdd_fail_json_set_value = 0;
      g_cdd_last_alloc_err = cdd_json_value_free(copy);
      g_cdd_last_alloc_err = cdd_json_value_free(dest_val);
      g_cdd_last_alloc_err = cdd_json_value_free(src_val);
      return CDD_C_ERROR_MEMORY;
    }
#endif
    g_cdd_last_alloc_err = cdd_json_object_set_value(dest_obj, key, copy);
  }

  g_cdd_last_alloc_err = cdd_json_serialize_to_string(dest_val, &serialized);
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_json_serialize) {
    g_cdd_fail_json_serialize = 0;
    g_cdd_last_alloc_err = cdd_json_free_serialized_string(serialized);
    serialized = NULL;
  }
#endif
  if (!serialized) {
    g_cdd_last_alloc_err = cdd_json_value_free(dest_val);
    g_cdd_last_alloc_err = cdd_json_value_free(src_val);
    return CDD_C_ERROR_MEMORY;
  }
  {
    char *dup = NULL;
    cdd_c_error_t rc = c_cdd_strdup(serialized, &dup);
    g_cdd_last_alloc_err = cdd_json_free_serialized_string(serialized);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(dest_val);
      g_cdd_last_alloc_err = cdd_json_value_free(src_val);
      return CDD_C_ERROR_MEMORY;
    }
    C_CDD_FREE(*dest_json);
    *dest_json = dup;
  }

  g_cdd_last_alloc_err = cdd_json_value_free(dest_val);
  g_cdd_last_alloc_err = cdd_json_value_free(src_val);

  return CDD_C_SUCCESS;
}

/*
 * @brief Checks if an OpenAPI type is a primitive type.
 *
 * @param[in] type OpenAPI type name string.
 * @param[out] out_is_primitive Output pointer receiving 1 if primitive, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `out_is_primitive` is NULL.
 */
cdd_c_error_t c2s_openapi_type_is_primitive(const char *type,
                                            int *out_is_primitive) {
  if (!out_is_primitive)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_primitive = 0;
  if (!type)
    return CDD_C_SUCCESS;
  if (strcmp(type, "integer") == 0 || strcmp(type, "number") == 0 ||
      strcmp(type, "string") == 0 || strcmp(type, "boolean") == 0) {
    *out_is_primitive = 1;
  }
  return CDD_C_SUCCESS;
}
