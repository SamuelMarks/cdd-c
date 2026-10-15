/**
 * @file code2schema_extras.c
 * @brief Schema extras collection and merging routines.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

cdd_c_error_t c2s_collect_schema_extras(const JSON_Object *obj,
                                        const char **skip_keys,
                                        size_t skip_count, char **out_json) {
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;
  char *serialized;

  if (out_json)
    *out_json = NULL;
  if (!obj || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  extras_val = json_value_init_object();
  if (!extras_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  extras_obj = json_value_get_object(extras_val);

  count = json_object_get_count(obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(obj, i);
    const JSON_Value *val;
    JSON_Value *copy;
    int is_skip = 0;

    c2s_key_in_list(key, skip_keys, skip_count, &is_skip);
    if (is_skip)
      continue;
    val = json_object_get_value(obj, key);
    {
      cdd_c_error_t rc_c2s = c2s_clone_json_value(val, &copy);
      if (rc_c2s != CDD_C_SUCCESS) {
        json_value_free(extras_val);
        return rc_c2s;
      }
    }
    if (json_object_set_value(extras_obj, key, copy) != JSONSuccess) {
      json_value_free(copy);
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  if (json_object_get_count(extras_obj) == 0) {
    json_value_free(extras_val);
    return CDD_C_SUCCESS;
  }

  serialized = json_serialize_to_string(extras_val);
  if (!serialized) {
    json_value_free(extras_val);
    return CDD_C_ERROR_MEMORY;
  }
  {
    cdd_c_error_t rc_c2s = c_cdd_strdup(serialized, out_json);
    if (rc_c2s != CDD_C_SUCCESS) {
      json_free_serialized_string(serialized);
      json_value_free(extras_val);
      return rc_c2s;
    }
  }
  json_free_serialized_string(serialized);
  json_value_free(extras_val);
  return *out_json ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Merges extra attributes described by a JSON string into a
 * target parson JSON Object.
 *
 * Parses the JSON string `extras_json`, clones each value, and sets them
 * directly on `target`. Returns early if there are no extras. Does not
 * override existing keys on `target`.
 */
cdd_c_error_t c2s_merge_schema_extras_object(JSON_Object *target,
                                             const char *extras_json) {
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;
  if (!target || !extras_json || extras_json[0] == '\0')
    return CDD_C_SUCCESS;

  extras_val = json_parse_string(extras_json);
  if (!extras_val)
    return CDD_C_SUCCESS;
  extras_obj = json_value_get_object(extras_val);
  if (!extras_obj) {
    json_value_free(extras_val);
    return CDD_C_SUCCESS;
  }

  count = json_object_get_count(extras_obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(extras_obj, i);
    const JSON_Value *val;
    JSON_Value *copy;

    if (json_object_has_value(target, key))
      continue;
    val = json_object_get_value(extras_obj, key);
    {
      cdd_c_error_t rc_c2s = c2s_clone_json_value(val, &copy);
      if (rc_c2s != CDD_C_SUCCESS) {
        json_value_free(extras_val);
        return rc_c2s;
      }
    }
    if (json_object_set_value(target, key, copy) != JSONSuccess) {
      json_value_free(copy);
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  json_value_free(extras_val);

  return CDD_C_SUCCESS;
}

/**
 * @brief Merges two serialized JSON objects containing extra schemas
 * together.
 *
 * Parses `dest_json` and `src_json`. Clones the properties from the
 * `src` object into `dest`, serializing the result back out into
 * `dest_json`. If `dest_json` was NULL, `src_json` is effectively
 * duplicated into it.
 */
cdd_c_error_t c2s_merge_schema_extras_strings(char **dest_json,
                                              const char *src_json) {
  JSON_Value *dest_val;
  JSON_Value *src_val;
  JSON_Object *dest_obj;
  JSON_Object *src_obj;
  size_t i, count;
  char *serialized;

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

  dest_val = json_parse_string(*dest_json);
  src_val = json_parse_string(src_json);
  if (!dest_val || !src_val) {
    if (dest_val)
      json_value_free(dest_val);
    if (src_val)
      json_value_free(src_val);
    return CDD_C_SUCCESS;
  }
  dest_obj = json_value_get_object(dest_val);
  src_obj = json_value_get_object(src_val);
  if (!dest_obj || !src_obj) {
    json_value_free(dest_val);
    json_value_free(src_val);
    return CDD_C_SUCCESS;
  }

  count = json_object_get_count(src_obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(src_obj, i);
    const JSON_Value *val;
    JSON_Value *copy;

    if (json_object_has_value(dest_obj, key))
      json_object_remove(dest_obj, key);
    val = json_object_get_value(src_obj, key);
    {
      cdd_c_error_t rc_c2s = c2s_clone_json_value(val, &copy);
      if (rc_c2s != CDD_C_SUCCESS) {
        json_value_free(dest_val);
        json_value_free(src_val);
        return rc_c2s;
      }
    }
#ifdef CDD_BUILD_TESTS
    if (g_cdd_fail_json_set_value) {
      g_cdd_fail_json_set_value = 0;
      json_value_free(copy);
      json_value_free(dest_val);
      json_value_free(src_val);
      return CDD_C_ERROR_MEMORY;
    }
#endif
    json_object_set_value(dest_obj, key, copy);
  }

  serialized = json_serialize_to_string(dest_val);
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_json_serialize) {
    g_cdd_fail_json_serialize = 0;
    json_free_serialized_string(serialized);
    serialized = NULL;
  }
#endif
  if (!serialized) {
    json_value_free(dest_val);
    json_value_free(src_val);
    return CDD_C_ERROR_MEMORY;
  }
  {
    char *dup = NULL;
    cdd_c_error_t rc = c_cdd_strdup(serialized, &dup);
    json_free_serialized_string(serialized);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(dest_val);
      json_value_free(src_val);
      return CDD_C_ERROR_MEMORY;
    }
    C_CDD_FREE(*dest_json);
    *dest_json = dup;
  }

  json_value_free(dest_val);
  json_value_free(src_val);

  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if an OpenAPI type is a primitive type.
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
