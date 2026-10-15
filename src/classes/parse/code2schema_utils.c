/**
 * @file code2schema_utils.c
 * @brief Utility functions for code2schema parsing and cloning.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Reads a line from a file pointer and strips trailing newlines.
 *
 * Reads up to `bufsz - 1` characters from the file into the buffer
 * `buf`. Any trailing carriage return or newline characters are removed.
 */
cdd_c_error_t c2s_read_line(FILE *fp, char *buf, size_t bufsz, int *has_line) {
  size_t len;
  if (!has_line)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *has_line = 0;
  if (!fp || !buf || bufsz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!fgets(buf, (int)bufsz, fp))
    return CDD_C_SUCCESS;
  len = strlen(buf);
  while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
    buf[--len] = '\0';
  *has_line = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Trim trailing whitespace and semicolons from a string in place.
 *
 * @param[in,out] str The string to trim.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t trim_trailing(char *str) {
  size_t len;
  if (!str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  c_cdd_str_trim_trailing_whitespace(str);
  len = strlen(str);
  while (len > 0 &&
         (str[len - 1] == ';' || isspace((unsigned char)str[len - 1]))) {
    str[--len] = '\0';
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Check if string starts with prefix.
 *
 * @param[in] str The string to check.
 * @param[in] prefix The prefix.
 * @param[out] _out_val Pointer to store the result (1 if matches, 0 otherwise).
 * @return CDD_C_SUCCESS on success, or error code.
 */
cdd_c_error_t str_starts_with(const char *str, const char *prefix,
                              int *_out_val) {
  {
    int b = 0;
    {
      cdd_c_error_t rc_c2s = c_cdd_str_starts_with(str, prefix, &b);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    if (_out_val)
      *_out_val = b;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Checks if a given key exists in an array of strings.
 *
 * Iterates linearly over a null-safe list to find a string-matched key.
 * Returns early if either the key or the list are NULL.
 */
cdd_c_error_t c2s_key_in_list(const char *key, const char **list, size_t count,
                              int *out_found) {
  size_t i;
  if (!out_found)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_found = 0;
  if (!key || !list)
    return CDD_C_SUCCESS;
  for (i = 0; i < count; ++i) {
    if (list[i] && strcmp(list[i], key) == 0) {
      *out_found = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Clones a parson JSON_Value safely via serialization roundtrip.
 *
 * Provides deep cloning of JSON values. Will fail (set _out_val to NULL)
 * if the input value is NULL or serialization fails.
 */
cdd_c_error_t c2s_clone_json_value(const JSON_Value *val,
                                   JSON_Value **_out_val) {
  char *serialized;
  JSON_Value *copy;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;
  if (!val)
    return CDD_C_SUCCESS;
  serialized = json_serialize_to_string(val);
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_json_serialize) {
    g_cdd_fail_json_serialize = 0;
    json_free_serialized_string(serialized);
    serialized = NULL;
  }
#endif
  if (!serialized)
    return CDD_C_ERROR_MEMORY;
  copy = json_parse_string(serialized);
  json_free_serialized_string(serialized);
  if (!copy)
    return CDD_C_ERROR_MEMORY;
  *_out_val = copy;
  return CDD_C_SUCCESS;
}

/**
 * @brief Safely frees an array of dynamically allocated string pointers.
 *
 * Loops over the first `n` elements of `arr` and frees them before
 * freeing the parent array pointer itself. Handles NULLs defensively.
 */
void free_string_array_code2schema(char **arr, size_t n) {
  size_t i;
  if (!arr)
    return;
  for (i = 0; i < n; ++i) {
    if (arr[i]) {
      C_CDD_FREE(arr[i]);
      arr[i] = NULL;
    }
  }
  C_CDD_FREE(arr);
}

/**
 * @brief Deep copies an array of string pointers into a new dynamically
 * allocated array.
 *
 * Allocates memory for both the array pointers and the underlying
 * strings. Handles cleanup via `free_string_array` if memory exhaustion
 * occurs.
 */
cdd_c_error_t copy_string_array_code2schema(char ***dst, size_t *dst_count,
                                            char **src, size_t src_count) {
  size_t i;
  char **out;
  if (!dst || !dst_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *dst = NULL;
  *dst_count = 0;
  if (!src || src_count == 0)
    return CDD_C_SUCCESS;
  out = (char **)C_CDD_CALLOC(src_count, sizeof(char *));
  if (!out) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  for (i = 0; i < src_count; ++i) {
    if (src[i]) {

      {
        cdd_c_error_t rc_c2s = c_cdd_strdup(src[i], &out[i]);
        if (rc_c2s != CDD_C_SUCCESS) {
          free_string_array_code2schema(out, src_count);
          return rc_c2s;
        }
      }
    }
  }
  *dst = out;
  *dst_count = src_count;

  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts valid C types from an array of JSON types.
 *
 * In OpenAPI/JSON schema, `type` can be an array like `["string",
 * "null"]`. Extracts each distinct type, determining the primary C type,
 * while flagging nullability if `"null"` is encountered in the list.
 */
cdd_c_error_t parse_type_union_array_code2schema(const JSON_Array *arr,
                                                 char ***out_union,
                                                 size_t *out_count,
                                                 const char **out_primary,
                                                 int *out_nullable) {
  size_t i, count, n = 0;
  char **types;
  const char *primary = NULL;

  if (out_union)
    *out_union = NULL;
  if (out_count)
    *out_count = 0;
  if (out_primary)
    *out_primary = NULL;
  if (out_nullable)
    *out_nullable = 0;

  if (!arr)
    return CDD_C_SUCCESS;

  count = json_array_get_count(arr);
  if (count == 0)
    return CDD_C_SUCCESS;

  types = (char **)C_CDD_CALLOC(count, sizeof(char *));
  if (!types) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  for (i = 0; i < count; ++i) {
    const char *t = json_array_get_string(arr, i);
    if (!t)
      continue;
    {
      cdd_c_error_t rc_c2s = c_cdd_strdup(t, &types[n]);
      if (rc_c2s != CDD_C_SUCCESS) {
        free_string_array_code2schema(types, count);
        return rc_c2s;
      }
    }
    if (strcmp(t, "null") == 0) {
      if (out_nullable)
        *out_nullable = 1;
    } else if (!primary) {
      primary = types[n];
    }
    n++;
  }

  if (n == 0) {
    C_CDD_FREE(types);
    return CDD_C_SUCCESS;
  }

  if (!primary)
    primary = "null";

  if (out_primary)
    *out_primary = primary;
  if (out_union)
    *out_union = types;
  else
    free_string_array_code2schema(types, n);
  if (out_count)
    *out_count = n;

  return CDD_C_SUCCESS;
}

/**
 * @brief Collects unstructured extra properties from a JSON Object.
 *
 * Loops over the properties of `obj` and copies any that do not match
 * keys found within the provided `skip_keys` list into a newly allocated
 * JSON string representing those leftover (extra) attributes.
 */
