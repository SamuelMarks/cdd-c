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
  size_t i, k, count;
  char *serialized = NULL;
  cdd_c_error_t rc;

  if (out_json)
    *out_json = NULL;
  if (!obj || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_json_value_init_object(&extras_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  extras_obj = extras_val->object_val;
  count = obj->count;
  for (i = 0; i < count; ++i) {
    const char *key = obj->names[i];
    JSON_Value *val = obj->values[i];
    JSON_Value *copy = NULL;
    int is_skip = 0;

    for (k = 0; k < skip_count; ++k) {
      if (strcmp(key, skip_keys[k]) == 0) {
        is_skip = 1;
        break;
      }
    }
    if (is_skip)
      continue;

    rc = c2s_clone_json_value(val, &copy);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(extras_val));
    }
    rc = cdd_json_object_set_value(extras_obj, key, copy);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc,
                            cdd_c_keep_err(cdd_json_value_free(copy),
                                           cdd_json_value_free(extras_val)));
    }
  }

  if (extras_obj->count == 0) {
    return cdd_json_value_free(extras_val);
  }

  rc = cdd_json_serialize_to_string(extras_val, &serialized);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(extras_val));
  }
  rc = c_cdd_strdup(serialized, out_json);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(
        rc, cdd_c_keep_err(cdd_json_free_serialized_string(serialized),
                           cdd_json_value_free(extras_val)));
  }
  return cdd_c_keep_err(cdd_json_free_serialized_string(serialized),
                        cdd_json_value_free(extras_val));
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
  size_t i, k, count;
  cdd_c_error_t rc;

  if (!target || !extras_json || !*extras_json)
    return CDD_C_SUCCESS;

  rc = cdd_json_parse_string(extras_json, &extras_val);
  if (extras_val) {
    extras_obj =
        (extras_val->type == JSONObject) ? extras_val->object_val : NULL;
    if (!extras_obj) {
      return cdd_c_keep_err(rc, cdd_json_value_free(extras_val));
    }

    count = extras_obj->count;
    for (i = 0; i < count; ++i) {
      const char *key = extras_obj->names[i];
      JSON_Value *val = extras_obj->values[i];
      JSON_Value *copy = NULL;
      int has_val = 0;
      cdd_c_error_t rc_c2s, rc_set;

      for (k = 0; k < target->count; ++k) {
        if (strcmp(target->names[k], key) == 0) {
          has_val = 1;
          break;
        }
      }
      if (has_val)
        continue;

      rc_c2s = c2s_clone_json_value(val, &copy);
      if (rc_c2s != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc_c2s, cdd_json_value_free(extras_val));
      }

#ifdef CDD_BUILD_TESTS
      if (g_cdd_fail_json_set_value) {
        g_cdd_fail_json_set_value = 0;
        return cdd_c_keep_err(CDD_C_ERROR_MEMORY,
                              cdd_c_keep_err(cdd_json_value_free(copy),
                                             cdd_json_value_free(extras_val)));
      }
#endif
      rc_set = cdd_json_object_set_value(target, key, copy);
      if (rc_set != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc_set,
                              cdd_c_keep_err(cdd_json_value_free(copy),
                                             cdd_json_value_free(extras_val)));
      }
    }

    return cdd_c_keep_err(rc, cdd_json_value_free(extras_val));
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Merges extra attributes from `src_json` into `*dest_json`.
 *
 * @param[in,out] dest_json Pointer to destination serialized JSON extras
 * string.
 * @param[in] src_json Source serialized JSON extras string to merge in.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c2s_merge_schema_extras_strings(char **dest_json,
                                              const char *src_json) {
  JSON_Value *dest_val = NULL;
  JSON_Value *src_val = NULL;
  JSON_Object *dest_obj = NULL;
  JSON_Object *src_obj = NULL;
  size_t i, count;
  char *serialized = NULL;
  cdd_c_error_t rc_d, rc_s, rc;

  if (!dest_json || !src_json || !*src_json)
    return CDD_C_SUCCESS;

  if (!*dest_json) {
    return c_cdd_strdup(src_json, dest_json);
  }

  rc_d = cdd_json_parse_string(*dest_json, &dest_val);
  if (dest_val) {
    rc_s = cdd_json_parse_string(src_json, &src_val);
    if (src_val) {
      dest_obj = (dest_val->type == JSONObject) ? dest_val->object_val : NULL;
      src_obj = (src_val->type == JSONObject) ? src_val->object_val : NULL;
      if (!dest_obj || !src_obj) {
        return cdd_c_keep_err(
            rc_d,
            cdd_c_keep_err(rc_s, cdd_c_keep_err(cdd_json_value_free(dest_val),
                                                cdd_json_value_free(src_val))));
      }

      count = src_obj->count;
      for (i = 0; i < count; ++i) {
        const char *key = src_obj->names[i];
        JSON_Value *val = src_obj->values[i];
        JSON_Value *copy = NULL;
        cdd_c_error_t rc_c2s, rc_set;

        rc_c2s = c2s_clone_json_value(val, &copy);
        if (rc_c2s != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc_c2s,
                                cdd_c_keep_err(cdd_json_value_free(dest_val),
                                               cdd_json_value_free(src_val)));
        }

#ifdef CDD_BUILD_TESTS
        if (g_cdd_fail_json_set_value) {
          g_cdd_fail_json_set_value = 0;
          return cdd_c_keep_err(
              CDD_C_ERROR_MEMORY,
              cdd_c_keep_err(cdd_json_value_free(copy),
                             cdd_c_keep_err(cdd_json_value_free(dest_val),
                                            cdd_json_value_free(src_val))));
        }
#endif
        rc_set = cdd_json_object_set_value(dest_obj, key, copy);
        if (rc_set != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc_set,
              cdd_c_keep_err(cdd_json_value_free(copy),
                             cdd_c_keep_err(cdd_json_value_free(dest_val),
                                            cdd_json_value_free(src_val))));
        }
      }

#ifdef CDD_BUILD_TESTS
      if (g_cdd_fail_json_serialize) {
        g_cdd_fail_json_serialize = 0;
        return cdd_c_keep_err(CDD_C_ERROR_MEMORY,
                              cdd_c_keep_err(cdd_json_value_free(dest_val),
                                             cdd_json_value_free(src_val)));
      }
#endif
      rc = cdd_json_serialize_to_string(dest_val, &serialized);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(dest_val),
                                                 cdd_json_value_free(src_val)));
      }
      {
        char *dup = NULL;
        rc = c_cdd_strdup(serialized, &dup);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, cdd_c_keep_err(cdd_json_free_serialized_string(serialized),
                                 cdd_c_keep_err(cdd_json_value_free(dest_val),
                                                cdd_json_value_free(src_val))));
        }
        C_CDD_FREE(*dest_json);
        *dest_json = dup;
      }

      return cdd_c_keep_err(
          rc_d, cdd_c_keep_err(
                    rc_s, cdd_c_keep_err(
                              cdd_json_free_serialized_string(serialized),
                              cdd_c_keep_err(cdd_json_value_free(dest_val),
                                             cdd_json_value_free(src_val)))));
    }
    return cdd_c_keep_err(rc_d, cdd_json_value_free(dest_val));
  }
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
