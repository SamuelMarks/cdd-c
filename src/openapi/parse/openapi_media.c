/**
 * @file openapi_media.c
 * @brief Media type parsing, negotiation, and MIME type helpers.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Frees an `OpenAPI_MediaType` item.
 *
 * @param[in,out] mt Media type item to release.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t oa_media_type_item_free(struct OpenAPI_MediaType *mt) {
  return free_media_type(mt);
}

/**
 * @brief Parses an optional `schema` or `itemSchema` JSON_Value (boolean or
 * object) into an `OpenAPI_SchemaRef`.
 *
 * @param[in] val Optional JSON value (`NULL` if absent).
 * @param[out] out_schema Destination `OpenAPI_SchemaRef`.
 * @param[out] out_set Flag set to 1 when a boolean or object schema is parsed.
 * @param[in] spec Parent OpenAPI specification context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t
parse_media_schema_value(const JSON_Value *val,
                         struct OpenAPI_SchemaRef *out_schema, int *out_set,
                         const struct OpenAPI_Spec *spec) {
  JSON_Value_Type vtype = JSONError;
  JSON_Object *schema_obj = NULL;
  cdd_c_error_t rc;
  if (!val)
    return CDD_C_SUCCESS;
  rc = cdd_json_value_get_type(val, &vtype);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (vtype == JSONBoolean) {
    out_schema->schema_is_boolean = 1;
    rc = cdd_json_value_get_boolean(val, &out_schema->schema_boolean_value);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_set = 1;
    return CDD_C_SUCCESS;
  }
  if (vtype == JSONObject) {
    rc = cdd_json_value_get_object(val, &schema_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_schema_ref(schema_obj, out_schema, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_set = 1;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Internal helper to parse a Media Type object.
 *
 * @param[in] media_obj The source media type JSON object.
 * @param[out] out Output OpenAPI_MediaType structure.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t parse_media_type_object_inner(
    const JSON_Object *media_obj, struct OpenAPI_MediaType *out,
    const struct OpenAPI_Spec *spec, int resolve_refs) {
  JSON_Value *schema_val = NULL;
  JSON_Value *item_schema_val = NULL;
  JSON_Object *encoding_obj = NULL;
  JSON_Array *prefix_encoding_arr = NULL;
  JSON_Object *item_encoding_obj = NULL;
  const char *ref = NULL;
  int has_encoding = 0;
  int has_prefix = 0;
  int has_item = 0;
  cdd_c_error_t rc;

  if (!media_obj || !out)
    return CDD_C_SUCCESS;

  rc = oa_json_has_value(media_obj, "encoding", &has_encoding);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_json_has_value(media_obj, "prefixEncoding", &has_prefix);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_json_has_value(media_obj, "itemEncoding", &has_item);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_encoding && (has_prefix || has_item))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = oa_json_opt_string(media_obj, "$ref", &ref);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (ref) {
    rc = c_cdd_strdup(ref, &out->ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (resolve_refs && spec) {
      struct OpenAPI_MediaType *mt = NULL;
      rc = find_component_media_type(spec, ref, &mt);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (mt) {
        rc = copy_media_type_fields(out, mt);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    return CDD_C_SUCCESS;
  }

  rc = oa_json_opt_value(media_obj, "schema", &schema_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_media_schema_value(schema_val, &out->schema, &out->schema_set,
                                spec);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = oa_json_opt_value(media_obj, "itemSchema", &item_schema_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_media_schema_value(item_schema_val, &out->item_schema,
                                &out->item_schema_set, spec);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = oa_json_opt_object(media_obj, "encoding", &encoding_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (encoding_obj) {
    rc = parse_encoding_map(encoding_obj, &out->encoding, &out->n_encoding,
                            spec, resolve_refs);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = oa_json_opt_array(media_obj, "prefixEncoding", &prefix_encoding_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (prefix_encoding_arr) {
    rc = parse_encoding_array(prefix_encoding_arr, &out->prefix_encoding,
                              &out->n_prefix_encoding, spec, resolve_refs);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = oa_json_opt_object(media_obj, "itemEncoding", &item_encoding_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (item_encoding_obj) {
    out->item_encoding = (struct OpenAPI_Encoding *)C_CDD_CALLOC(
        1, sizeof(struct OpenAPI_Encoding));
    if (!out->item_encoding)
      return CDD_C_ERROR_MEMORY;
    out->item_encoding_set = 1;
    rc = parse_encoding_object(item_encoding_obj, out->item_encoding, spec,
                               resolve_refs);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = parse_media_examples(media_obj, &out->example, &out->example_set,
                            &out->examples, &out->n_examples, spec,
                            resolve_refs);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return collect_extensions(media_obj, &out->extensions_json);
}

/*
 * @brief Parses an OpenAPI Media Type object.
 *
 * @param[in] media_obj JSON object representing the Media Type.
 * @param[out] out Destination media type structure.
 * @param[in] spec Parent OpenAPI specification context.
 * @param[in] resolve_refs Non-zero to resolve `$ref` against components.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t parse_media_type_object(const JSON_Object *media_obj,
                                      struct OpenAPI_MediaType *out,
                                      const struct OpenAPI_Spec *spec,
                                      int resolve_refs) {
  cdd_c_error_t rc =
      parse_media_type_object_inner(media_obj, out, spec, resolve_refs);
  if (rc != CDD_C_SUCCESS) {
    cdd_c_error_t rc_f = oa_media_type_item_free(out);
    return (cdd_c_error_t)((int)rc | (int)rc_f);
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Computes the length of the media type essence (`type/subtype`) before
 * any `;` parameters.
 *
 * @param[in] media_type Media type string.
 * @param[out] _out_val Base length in bytes.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t openapi_media_type_base_len(const char *media_type,
                                          size_t *_out_val) {
  size_t len = 0;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!media_type) {
    *_out_val = 0;
    return CDD_C_SUCCESS;
  }
  while (media_type[len] && media_type[len] != ';') {
    len++;
  }
  *_out_val = len;
  return CDD_C_SUCCESS;
}

/*
 * @brief Compares two media type strings ignoring `;` parameters.
 *
 * @param[in] a First media type string.
 * @param[in] b Second media type string.
 * @param[out] out_equal Set to 1 if base essences match, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t media_type_base_equal(const char *a, const char *b,
                                    int *out_equal) {
  size_t alen = 0;
  size_t blen = 0;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!out_equal)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_equal = 0;
  if (!a || !b)
    return CDD_C_SUCCESS;
  rc = media_type_base_len(a, &alen);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = media_type_base_len(b, &blen);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (alen != blen)
    return CDD_C_SUCCESS;
  *out_equal = (strncmp(a, b, alen) == 0) ? 1 : 0;
  return CDD_C_SUCCESS;
}

/*
 * @brief Checks if a media type string represents a JSON media type
 * (`application/json` or `+json`).
 *
 * @param[in] name Media type string.
 * @param[out] out_is_json Set to 1 if JSON media type, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t openapi_media_type_is_json(const char *name, int *out_is_json) {
  size_t len = 0;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!out_is_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_json = 0;
  if (!name)
    return CDD_C_SUCCESS;
  rc = media_type_base_len(name, &len);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (len == strlen("application/json") &&
      strncmp(name, "application/json", len) == 0) {
    *out_is_json = 1;
    return CDD_C_SUCCESS;
  }
  if (len >= 5 && strncmp(name + (len - 5), "+json", 5) == 0) {
    *out_is_json = 1;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Computes media type specificity (`0` for wildcard, `1` for type
 * wildcard, `2` for exact `type/subtype`).
 *
 * @param[in] name Media type string.
 * @param[out] out_spec Specificity score.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t media_type_specificity(const char *name, int *out_spec) {
  const char *slash;
  size_t len = 0;
  size_t type_len;
  size_t sub_len;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (out_spec)
    *out_spec = 0;
  if (!name)
    return CDD_C_SUCCESS;
  rc = media_type_base_len(name, &len);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (len == 0)
    return CDD_C_SUCCESS;
  slash = (const char *)memchr(name, '/', len);
  if (!slash) {
    if (out_spec)
      *out_spec = 2;
    return CDD_C_SUCCESS;
  }
  type_len = (size_t)(slash - name);
  if (type_len == 0)
    return CDD_C_SUCCESS;
  sub_len = len - type_len - 1;
  if (type_len == 1 && name[0] == '*' && sub_len == 1 && slash[1] == '*') {
    if (out_spec)
      *out_spec = 0;
    return CDD_C_SUCCESS;
  }
  if (sub_len == 1 && slash[1] == '*') {
    if (out_spec)
      *out_spec = 1;
    return CDD_C_SUCCESS;
  }
  if (out_spec)
    *out_spec = 2;
  return CDD_C_SUCCESS;
}

/*
 * @brief Computes media type preference rank (`3` JSON, `2` form-urlencoded,
 * `1` multipart, `0` other).
 *
 * @param[in] name Media type string.
 * @param[out] out_rank Preference rank.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t media_type_preference_rank(const char *name, int *out_rank) {
  int is_json = 0;
  int is_eq = 0;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (out_rank)
    *out_rank = 0;
  if (!name)
    return CDD_C_SUCCESS;
  rc = media_type_is_json(name, &is_json);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (is_json) {
    if (out_rank)
      *out_rank = 3;
    return CDD_C_SUCCESS;
  }
  rc = media_type_base_equal(name, "application/x-www-form-urlencoded", &is_eq);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (is_eq) {
    if (out_rank)
      *out_rank = 2;
    return CDD_C_SUCCESS;
  }
  rc = media_type_base_equal(name, "multipart/form-data", &is_eq);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (is_eq) {
    if (out_rank)
      *out_rank = 1;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Selects the preferred primary media type index from an array of media
 * types.
 *
 * @param[in] mts Array of media types.
 * @param[in] n Number of media types.
 * @param[out] out_idx Selected index, or -1 if none.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t
select_primary_media_type_index(const struct OpenAPI_MediaType *mts, size_t n,
                                int *out_idx) {
  size_t i;
  int best_idx = -1;
  int best_spec = -1;
  int best_rank = -1;
  cdd_c_error_t rc;

  if (out_idx)
    *out_idx = -1;

  for (i = 0; i < n; ++i) {
    int spec = 0;
    int rank = 0;
    rc = media_type_specificity(mts[i].name, &spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = media_type_preference_rank(mts[i].name, &rank);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (spec > best_spec) {
      best_spec = spec;
      best_rank = rank;
      best_idx = (int)i;
      continue;
    }
    if (spec == best_spec && rank > best_rank) {
      best_rank = rank;
      best_idx = (int)i;
    }
  }
  if (out_idx)
    *out_idx = best_idx;
  return CDD_C_SUCCESS;
}

/*
 * @brief Finds a media type JSON object in a `content` map by media type name.
 *
 * @param[in] content JSON object representing the `content` map.
 * @param[in] media_name Media type key to look up.
 * @param[out] _out_val Matching JSON_Object pointer, or NULL if not found.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t find_media_object_by_name(const JSON_Object *content,
                                        const char *media_name,
                                        const JSON_Object **_out_val) {
  size_t i, count = 0;
  JSON_Object *exact = NULL;
  cdd_c_error_t rc;
  if (!content || !media_name) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  rc = oa_json_opt_object(content, media_name, &exact);
  if (rc != CDD_C_SUCCESS) {
    *_out_val = NULL;
    return rc;
  }
  if (exact) {
    *_out_val = exact;
    return CDD_C_SUCCESS;
  }
  rc = oa_json_object_count(content, &count);
  if (rc != CDD_C_SUCCESS) {
    *_out_val = NULL;
    return rc;
  }
  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    int is_eq = 0;
    rc = oa_json_object_name(content, i, &name);
    if (rc != CDD_C_SUCCESS) {
      *_out_val = NULL;
      return rc;
    }
    rc = media_type_base_equal(name, media_name, &is_eq);
    if (rc != CDD_C_SUCCESS) {
      *_out_val = NULL;
      return rc;
    }
    if (is_eq) {
      JSON_Object *found_obj = NULL;
      rc = oa_json_opt_object(content, name, &found_obj);
      if (rc != CDD_C_SUCCESS) {
        *_out_val = NULL;
        return rc;
      }
      *_out_val = found_obj;
      return CDD_C_SUCCESS;
    }
  }
  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/*
 * @brief Parses a `content` map of media type objects.
 *
 * @param[in] content JSON object mapping media type ranges to Media Type
 * objects.
 * @param[out] out Output array of parsed media types.
 * @param[out] out_count Output number of parsed media types.
 * @param[in] spec Parent OpenAPI specification context.
 * @param[in] resolve_refs Non-zero to resolve `$ref` against components.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t parse_content_object(const JSON_Object *content,
                                   struct OpenAPI_MediaType **out,
                                   size_t *out_count,
                                   const struct OpenAPI_Spec *spec,
                                   int resolve_refs) {
  size_t i, count = 0, valid = 0;
  cdd_c_error_t rc;
  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!content)
    return CDD_C_SUCCESS;
  rc = oa_json_object_count(content, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (count == 0)
    return CDD_C_SUCCESS;

  *out_count = count;
  *out = (struct OpenAPI_MediaType *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_MediaType));
  if (!*out)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Object *media_obj = NULL;
    struct OpenAPI_MediaType *curr = &(*out)[valid];
    rc = oa_json_object_name(content, i, &name);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = oa_json_opt_object(content, name, &media_obj);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (!media_obj)
      continue;
    rc = c_cdd_strdup(name, &curr->name);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = parse_media_type_object(media_obj, curr, spec, resolve_refs);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    valid++;
  }

  if (valid == 0) {
    C_CDD_FREE(*out);
    *out = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }
  *out_count = valid;
  return CDD_C_SUCCESS;

cleanup: {
  size_t j;
  for (j = 0; j < count; ++j) {
    rc = (cdd_c_error_t)((int)rc | (int)oa_media_type_item_free(&(*out)[j]));
  }
  C_CDD_FREE(*out);
  *out = NULL;
  *out_count = 0;
}
  return rc;
}
