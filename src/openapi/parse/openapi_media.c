/**
 * @file openapi_media.c
 * @brief Media type parsing, negotiation, and MIME type helpers.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Parses media type object from the given input.
 */
cdd_c_error_t parse_media_type_object(const JSON_Object *media_obj,
                                      struct OpenAPI_MediaType *out,
                                      const struct OpenAPI_Spec *spec,
                                      int resolve_refs) {
  struct OpenAPI_MediaType *_ast_find_component_media_type_63;
  char *_ast_strdup_241 = NULL;
  const JSON_Value *schema_val;
  const JSON_Object *schema_obj;
  const JSON_Value *item_schema_val;
  const JSON_Object *item_schema_obj;
  const JSON_Object *encoding_obj;
  const JSON_Array *prefix_encoding_arr;
  const JSON_Object *item_encoding_obj;
  const char *ref;

  if (!media_obj || !out)
    return CDD_C_SUCCESS;

  {
    const int has_encoding = json_object_has_value(media_obj, "encoding");
    const int has_prefix = json_object_has_value(media_obj, "prefixEncoding");
    const int has_item = json_object_has_value(media_obj, "itemEncoding");
    if (has_encoding && (has_prefix || has_item))
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  ref = json_object_get_string(media_obj, "$ref");
  if (ref) {
    out->ref = (c_cdd_strdup(ref, &_ast_strdup_241), _ast_strdup_241);
    if (!out->ref)
      return CDD_C_ERROR_MEMORY;
    if (resolve_refs && spec) {
      const struct OpenAPI_MediaType *mt =
          (find_component_media_type(spec, ref,
                                     &_ast_find_component_media_type_63),
           _ast_find_component_media_type_63);
      if (mt) {
        {
          cdd_c_error_t _rc = copy_media_type_fields(out, mt);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }
    return CDD_C_SUCCESS;
  }

  schema_val = json_object_get_value(media_obj, "schema");
  schema_obj = schema_val ? json_value_get_object(schema_val) : NULL;
  if (schema_val) {
    if (json_value_get_type(schema_val) == JSONBoolean) {
      out->schema.schema_is_boolean = 1;
      out->schema.schema_boolean_value = json_value_get_boolean(schema_val);
      out->schema_set = 1;
    } else if (schema_obj) {
      {
        cdd_c_error_t _rc = parse_schema_ref(schema_obj, &out->schema, spec);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      out->schema_set = 1;
    }
  }

  item_schema_val = json_object_get_value(media_obj, "itemSchema");
  item_schema_obj =
      item_schema_val ? json_value_get_object(item_schema_val) : NULL;
  if (item_schema_val) {
    if (json_value_get_type(item_schema_val) == JSONBoolean) {
      out->item_schema.schema_is_boolean = 1;
      out->item_schema.schema_boolean_value =
          json_value_get_boolean(item_schema_val);
      out->item_schema_set = 1;
    } else if (item_schema_obj) {
      {
        cdd_c_error_t _rc =
            parse_schema_ref(item_schema_obj, &out->item_schema, spec);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      out->item_schema_set = 1;
    }
  }

  encoding_obj = json_object_get_object(media_obj, "encoding");
  if (encoding_obj) {
    cdd_c_error_t rc = parse_encoding_map(encoding_obj, &out->encoding,
                                          &out->n_encoding, spec, resolve_refs);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  prefix_encoding_arr = json_object_get_array(media_obj, "prefixEncoding");
  if (prefix_encoding_arr) {
    cdd_c_error_t rc =
        parse_encoding_array(prefix_encoding_arr, &out->prefix_encoding,
                             &out->n_prefix_encoding, spec, resolve_refs);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  item_encoding_obj = json_object_get_object(media_obj, "itemEncoding");
  if (item_encoding_obj) {
    out->item_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    if (!out->item_encoding)
      return CDD_C_ERROR_MEMORY;
    out->item_encoding_set = 1;
    {
      cdd_c_error_t rc = parse_encoding_object(
          item_encoding_obj, out->item_encoding, spec, resolve_refs);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  {
    cdd_c_error_t rc = parse_media_examples(
        media_obj, &out->example, &out->example_set, &out->examples,
        &out->n_examples, spec, resolve_refs);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  {
    cdd_c_error_t _rc = collect_extensions(media_obj, &out->extensions_json);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the media type base len operation.
 */
cdd_c_error_t media_type_base_len(const char *name, size_t *_out_val) {
  size_t len = 0;
  if (!name) {
    *_out_val = 0;
    return CDD_C_SUCCESS;
  }
  while (name[len] && name[len] != ';')
    ++len;
  {
    *_out_val = len;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the media type base equal operation.
 */
cdd_c_error_t media_type_base_equal(const char *a, const char *b) {
  size_t _ast_media_type_base_len_64 = 0;
  size_t _ast_media_type_base_len_65 = 0;
  size_t alen;
  size_t blen;
  if (!a || !b)
    return CDD_C_SUCCESS;
  alen = (media_type_base_len(a, &_ast_media_type_base_len_64),
          _ast_media_type_base_len_64);
  blen = (media_type_base_len(b, &_ast_media_type_base_len_65),
          _ast_media_type_base_len_65);
  if (alen != blen)
    return CDD_C_SUCCESS;
  return strncmp(a, b, alen) == 0;
}

/**
 * @brief Executes the media type is json operation.
 */
cdd_c_error_t media_type_is_json(const char *name) {
  size_t _ast_media_type_base_len_66 = 0;
  size_t len;
  if (!name)
    return CDD_C_SUCCESS;
  len = (media_type_base_len(name, &_ast_media_type_base_len_66),
         _ast_media_type_base_len_66);
  if (len == strlen("application/json") &&
      strncmp(name, "application/json", len) == 0)
    return CDD_C_ERROR_UNKNOWN;
  if (len >= 5 && strncmp(name + (len - 5), "+json", 5) == 0)
    return CDD_C_ERROR_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the media type specificity operation.
 */
cdd_c_error_t media_type_specificity(const char *name, int *out_spec) {
  size_t _ast_media_type_base_len_67 = 0;
  const char *slash;
  size_t len;
  size_t type_len;
  size_t sub_len;
  if (out_spec)
    *out_spec = 0;
  if (!name)
    return CDD_C_SUCCESS;
  len = (media_type_base_len(name, &_ast_media_type_base_len_67),
         _ast_media_type_base_len_67);
  if (len == 0)
    return CDD_C_SUCCESS;
  slash = (const char *)memchr(name, '/', len);
  if (!slash) {
    if (out_spec)
      *out_spec = 2; /* Treat unknown as exact */
    return CDD_C_SUCCESS;
  }
  type_len = (size_t)(slash - name);
  if (type_len == 0 || type_len >= len)
    return CDD_C_SUCCESS;
  sub_len = len - type_len - 1;
  if (type_len == 1 && name[0] == '*' && sub_len == 1 && slash[1] == '*') {
    if (out_spec)
      *out_spec = 0; /* */
    return CDD_C_SUCCESS;
  }
  if (sub_len == 1 && slash[1] == '*') {
    if (out_spec)
      *out_spec = 1;
    return CDD_C_SUCCESS;
  }
  if (out_spec)
    *out_spec = 2; /* type/subtype */
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the media type preference rank operation.
 */
cdd_c_error_t media_type_preference_rank(const char *name, int *out_rank) {
  if (out_rank)
    *out_rank = 0;
  if (!name)
    return CDD_C_SUCCESS;
  if (media_type_is_json(name)) {
    if (out_rank)
      *out_rank = 3;
    return CDD_C_SUCCESS;
  }
  if (media_type_base_equal(name, "application/x-www-form-urlencoded")) {
    if (out_rank)
      *out_rank = 2;
    return CDD_C_SUCCESS;
  }
  if (media_type_base_equal(name, "multipart/form-data")) {
    if (out_rank)
      *out_rank = 1;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the select primary media type index operation.
 */
cdd_c_error_t
select_primary_media_type_index(const struct OpenAPI_MediaType *mts, size_t n,
                                int *out_idx) {
  size_t i;
  int best_idx = -1;
  int best_spec = -1;
  int best_rank = -1;

  if (out_idx)
    *out_idx = -1;

  for (i = 0; i < n; ++i) {
    int spec = 0;
    int rank = 0;
    media_type_specificity(mts[i].name, &spec);
    media_type_preference_rank(mts[i].name, &rank);
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

/**
 * @brief Retrieves the media object by name.
 */
cdd_c_error_t find_media_object_by_name(const JSON_Object *content,
                                        const char *media_name,
                                        JSON_Object **_out_val) {
  size_t i, count;
  if (!content || !media_name) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  {
    const JSON_Object *exact = json_object_get_object(content, media_name);
    if (exact) {
      *_out_val = (JSON_Object *)(exact);
      return CDD_C_SUCCESS;
    }
  }
  count = json_object_get_count(content);
  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(content, i);
    if (!name)
      continue;
    if (media_type_base_equal(name, media_name)) {
      {
        *_out_val = json_object_get_object(content, name);
        return CDD_C_SUCCESS;
      }
    }
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses content object from the given input.
 */
cdd_c_error_t parse_content_object(const JSON_Object *content,
                                   struct OpenAPI_MediaType **out,
                                   size_t *out_count,
                                   const struct OpenAPI_Spec *spec,
                                   int resolve_refs) {
  char *_ast_strdup_242 = NULL;
  size_t i, count, valid = 0;
  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!content)
    return CDD_C_SUCCESS;
  count = json_object_get_count(content);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out_count = count;
  *out = (struct OpenAPI_MediaType *)calloc(count,
                                            sizeof(struct OpenAPI_MediaType));
  if (!*out)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(content, i);
    const JSON_Object *media_obj =
        json_value_get_object(json_object_get_value_at(content, i));
    struct OpenAPI_MediaType *curr = &(*out)[valid];
    if (!name || !media_obj)
      continue;
    curr->name = (c_cdd_strdup(name, &_ast_strdup_242), _ast_strdup_242);
    if (!curr->name)
      return CDD_C_ERROR_MEMORY;
    {
      cdd_c_error_t rc =
          parse_media_type_object(media_obj, curr, spec, resolve_refs);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    valid++;
  }

  if (valid == 0) {
    free(*out);
    *out = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }
  *out_count = valid;
  return CDD_C_SUCCESS;
}
