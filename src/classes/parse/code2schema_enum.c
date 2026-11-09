/**
 * @file code2schema_enum.c
 * @brief Enum conversion and reference resolution for code2schema.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/*
 * @brief Converts a JSON array of strings to an EnumMembers structure.
 *
 * @param[in] enum_arr Parson JSON_Array of enum member strings.
 * @param[in,out] em Target EnumMembers container to populate.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t json_array_to_enum_members(const JSON_Array *enum_arr,
                                         struct EnumMembers *em) {
  size_t i, count = 0;
  if (!enum_arr || !em)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  g_cdd_last_alloc_err = cdd_json_array_get_count(enum_arr, &count);
  for (i = 0; i < count; i++) {
    const char *s = NULL;
    char *clean = NULL;
    char final_name[64];
    size_t k;
    int dup = 0;
    int needs_san = 0;
    const char *wire_override = NULL;
    g_cdd_last_alloc_err = cdd_json_array_get_string(enum_arr, i, &s);
    if (!s)
      continue;
    g_cdd_last_alloc_err = c2s_needs_identifier_sanitization(s, &needs_san);
    if (needs_san) {
      if (sanitize_identifier(s, &clean) != CDD_C_SUCCESS) {
        return CDD_C_ERROR_MEMORY;
      }
      CDD_STRCPY(final_name, sizeof(final_name), clean);
      C_CDD_FREE(clean);
    } else if (strcmp(s, "UNKNOWN") == 0) {
      CDD_STRCPY(final_name, sizeof(final_name), "UNKNOWN_VAL");
    } else {
      CDD_STRCPY(final_name, sizeof(final_name), s);
    }

    for (k = 0; k < em->size; ++k) {
      if (em->members[k] && strcmp(em->members[k], final_name) == 0) {
        dup = 1;
        break;
      }
    }
    if (dup) {
      char base_nm[52];
      unsigned int sfx = 2;
      int dup_cand = 1;
      CDD_STRCPY(base_nm, sizeof(base_nm), final_name);
      while (dup_cand) {
        dup_cand = 0;
        CDD_SNPRINTF(final_name, sizeof(final_name), "%s_%u", base_nm, sfx++);
        for (k = 0; k < em->size; ++k) {
          if (em->members[k] && strcmp(em->members[k], final_name) == 0) {
            dup_cand = 1;
            break;
          }
        }
      }
    }

    if (strcmp(final_name, s) != 0) {
      wire_override = s;
    }
    if (enum_members_add_ex(em, final_name, wire_override, 0) != 0)
      return CDD_C_ERROR_MEMORY;
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Determines if a JSON Schema object represents a string enum.
 *
 * @param[in] schema_obj JSON Schema object to inspect.
 * @param[out] enum_arr_out Optional output pointer receiving the `enum`
 * JSON_Array.
 * @param[out] out_is_enum Output pointer receiving 1 if string enum, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `out_is_enum` is NULL.
 */
cdd_c_error_t schema_object_is_string_enum(const JSON_Object *schema_obj,
                                           const JSON_Array **enum_arr_out,
                                           int *out_is_enum) {
  JSON_Array *enum_arr = NULL;
  size_t i, count = 0;
  const char *type = NULL;

  if (enum_arr_out)
    *enum_arr_out = NULL;
  if (!out_is_enum)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_enum = 0;
  if (!schema_obj)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err =
      cdd_json_object_get_array(schema_obj, "enum", &enum_arr);
  if (!enum_arr)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_array_get_count(enum_arr, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_object_get_string(schema_obj, "type", &type);
  if (type && strcmp(type, "string") != 0)
    return CDD_C_SUCCESS;

  for (i = 0; i < count; ++i) {
    const char *s = NULL;
    g_cdd_last_alloc_err = cdd_json_array_get_string(enum_arr, i, &s);
    if (!s)
      return CDD_C_SUCCESS;
  }

  if (enum_arr_out)
    *enum_arr_out = enum_arr;
  *out_is_enum = 1;
  return CDD_C_SUCCESS;
}

/*
 * @brief Checks if a JSON Schema $ref points to a string enum.
 *
 * @param[in] root Root `components.schemas` JSON_Object.
 * @param[in] ref JSON Schema `$ref` path string.
 * @param[out] out_points_to_enum Output pointer receiving 1 if target is string
 * enum, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t ref_points_to_string_enum(const JSON_Object *root,
                                        const char *ref,
                                        int *out_points_to_enum) {
  const char *name;
  JSON_Object *schema_obj = NULL;
  if (!out_points_to_enum)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_points_to_enum = 0;
  if (!root || !ref)
    return CDD_C_SUCCESS;
  name = NULL;
  {
    cdd_c_error_t rc_c2s = c_cdd_str_after_last(ref, '/', &name);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
  }
  if (!*name)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_object_get_object(root, name, &schema_obj);
  if (!schema_obj)
    return CDD_C_SUCCESS;
  return schema_object_is_string_enum(schema_obj, NULL, out_points_to_enum);
}

/*
 * @brief Checks if a given property name is present in a required
 * properties list.
 *
 * @param[in] required JSON_Array of required property names.
 * @param[in] name Property name to look for.
 * @param[out] out_in_list Output pointer receiving 1 if present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `out_in_list` is NULL.
 */
cdd_c_error_t required_name_in_list(const JSON_Array *required,
                                    const char *name, int *out_in_list) {
  size_t i, count = 0;
  if (!out_in_list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_in_list = 0;
  if (!required || !name)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_array_get_count(required, &count);
  for (i = 0; i < count; ++i) {
    const char *req_name = NULL;
    g_cdd_last_alloc_err = cdd_json_array_get_string(required, i, &req_name);
    if (req_name && strcmp(req_name, name) == 0) {
      *out_in_list = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Resolves a JSON Schema $ref to its corresponding object.
 *
 * @param[in] root Root `components.schemas` JSON_Object.
 * @param[in] ref JSON Schema `$ref` path string.
 * @param[out] _out_val Output pointer receiving the resolved schema
 * JSON_Object.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t resolve_schema_ref_object(const JSON_Object *root,
                                        const char *ref,
                                        const JSON_Object **_out_val) {
  const char *name;
  JSON_Object *found_obj = NULL;
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!root || !ref) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  name = NULL;
  {
    cdd_c_error_t rc_c2s = c_cdd_str_after_last(ref, '/', &name);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
  }
  if (!*name) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  g_cdd_last_alloc_err = cdd_json_object_get_object(root, name, &found_obj);
  *_out_val = found_obj;
  return CDD_C_SUCCESS;
}
