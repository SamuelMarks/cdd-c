/**
 * @file code2schema_enum.c
 * @brief Enum conversion and reference resolution for code2schema.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Converts a JSON array of strings to an EnumMembers structure.
 */
cdd_c_error_t json_array_to_enum_members(const JSON_Array *enum_arr,
                                         struct EnumMembers *em) {
  size_t i, count;
  if (!enum_arr || !em)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  count = json_array_get_count(enum_arr);
  for (i = 0; i < count; i++) {
    const char *s = json_array_get_string(enum_arr, i);
    if (!s)
      continue;
    if (enum_members_add(em, s) != 0)
      return CDD_C_ERROR_MEMORY;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Determines if a JSON Schema object represents a string enum.
 */
cdd_c_error_t schema_object_is_string_enum(const JSON_Object *schema_obj,
                                           const JSON_Array **enum_arr_out,
                                           int *out_is_enum) {
  const JSON_Array *enum_arr;
  size_t i, count;
  const char *type;

  if (enum_arr_out)
    *enum_arr_out = NULL;
  if (!out_is_enum)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_enum = 0;
  if (!schema_obj)
    return CDD_C_SUCCESS;

  enum_arr = json_object_get_array(schema_obj, "enum");
  if (!enum_arr)
    return CDD_C_SUCCESS;

  count = json_array_get_count(enum_arr);
  if (count == 0)
    return CDD_C_SUCCESS;

  type = json_object_get_string(schema_obj, "type");
  if (type && strcmp(type, "string") != 0)
    return CDD_C_SUCCESS;

  for (i = 0; i < count; ++i) {
    if (!json_array_get_string(enum_arr, i))
      return CDD_C_SUCCESS;
  }

  if (enum_arr_out)
    *enum_arr_out = enum_arr;
  *out_is_enum = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a JSON Schema $ref points to a string enum.
 */
cdd_c_error_t ref_points_to_string_enum(const JSON_Object *root,
                                        const char *ref,
                                        int *out_points_to_enum) {
  const char *name;
  const JSON_Object *schema_obj;
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
  schema_obj = json_object_get_object(root, name);
  if (!schema_obj)
    return CDD_C_SUCCESS;
  return schema_object_is_string_enum(schema_obj, NULL, out_points_to_enum);
}

/**
 * @brief Checks if a given property name is present in a required
 * properties list.
 */
cdd_c_error_t required_name_in_list(const JSON_Array *required,
                                    const char *name, int *out_in_list) {
  size_t i, count;
  if (!out_in_list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_in_list = 0;
  if (!required || !name)
    return CDD_C_SUCCESS;
  count = json_array_get_count(required);
  for (i = 0; i < count; ++i) {
    const char *req_name = json_array_get_string(required, i);
    if (req_name && strcmp(req_name, name) == 0) {
      *out_in_list = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves a JSON Schema $ref to its corresponding object.
 */
cdd_c_error_t resolve_schema_ref_object(const JSON_Object *root,
                                        const char *ref,
                                        const JSON_Object **_out_val) {
  const char *name;
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
  {
    *_out_val = json_object_get_object(root, name);
    return CDD_C_SUCCESS;
  }
}
