/**
 * @file openapi_uri.c
 * @brief URI resolution, path normalization, and document registry routing.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Unescapes a JSON Pointer token (`~1` -> `/`, `~0` -> `~`).
 *
 * @param[in] in Escaped JSON Pointer token string.
 * @param[out] _out_val Newly allocated unescaped string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t json_pointer_unescape(const char *in, char **_out_val) {
  if (!in) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  return cdd_json_pointer_unescape(in, strlen(in), _out_val);
}

/**
 * @brief Checks whether a URI string starts with an RFC 3986 scheme prefix
 * within `len` bytes.
 *
 * @param[in] uri URI string to inspect.
 * @param[in] len Number of characters to inspect.
 * @param[out] out_has Set to 1 if scheme is present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t uri_has_scheme_prefix(const char *uri, size_t len, int *out_has) {
  size_t i;
  if (!out_has)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has = 0;
  if (!uri || len == 0)
    return CDD_C_SUCCESS;
  for (i = 0; i < len; ++i) {
    char c = uri[i];
    if (c == ':') {
      *out_has = 1;
      return CDD_C_SUCCESS;
    }
    if (c == '/' || c == '?' || c == '#')
      break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Computes the length of the base URI before any `#` fragment.
 *
 * @param[in] uri Input URI string.
 * @param[out] _out_val Length of the base URI portion.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t uri_base_len(const char *uri, size_t *_out_val) {
  if (!uri) {
    *_out_val = 0;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = strcspn(uri, "#");
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Computes the length of the scheme prefix in a URI.
 *
 * @param[in] uri Input URI string.
 * @param[in] len Length of URI to inspect.
 * @param[out] _out_val Length of scheme prefix, or 0 if none.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t uri_scheme_len(const char *uri, size_t len, size_t *_out_val) {
  size_t i;
  if (!uri || len == 0) {
    *_out_val = 0;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < len; ++i) {
    if (uri[i] == ':') {
      *_out_val = i;
      return CDD_C_SUCCESS;
    }
    if (uri[i] == '/' || uri[i] == '?' || uri[i] == '#')
      break;
  }
  {
    *_out_val = 0;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Duplicates a substring of length `len`.
 *
 * @param[in] src Source string.
 * @param[in] len Number of bytes to copy.
 * @param[out] _out_val Newly allocated null-terminated string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
cdd_c_error_t dup_substr(const char *src, size_t len, char **_out_val) {
  char *out;
  if (!src) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  out = (char *)C_CDD_MALLOC(len + 1);
  if (!out) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
  memcpy(out, src, len);
  out[len] = '\0';
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Normalizes dot-segments (`.` and `..`) in a URI path.
 *
 * @param[in] path Input URI path string.
 * @param[out] _out_val Newly allocated normalized path string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t normalize_path(const char *path, char **_out_val) {
  size_t i = 0;
  int absolute = 0;
  int trailing = 0;
  char **segments = NULL;
  size_t count = 0;
  size_t cap = 0;
  char *out = NULL;
  size_t out_len = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!path) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (path[0] == '/')
    absolute = 1;
  if (path[0] && path[strlen(path) - 1] == '/')
    trailing = 1;

  while (path[i]) {
    size_t start = i;
    size_t seg_len;
    while (path[i] && path[i] != '/')
      ++i;
    seg_len = i - start;
    if (seg_len == 0) {
      ++i;
      continue;
    }
    if (seg_len == 1 && path[start] == '.') {
      continue;
    }
    if (seg_len == 2 && memcmp(path + start, "..", 2) == 0) {
      if (count > 0 && strcmp(segments[count - 1], "..") != 0) {
        C_CDD_FREE(segments[count - 1]);
        count--;
        continue;
      }
      if (absolute) {
        continue;
      }
    }
    if (count == cap) {
      size_t new_cap = cap ? cap * 2 : 4;
      char **tmp =
          (char **)C_CDD_REALLOC(segments, new_cap * sizeof(*segments));
      if (!tmp) {
        rc = CDD_C_ERROR_MEMORY;
        goto cleanup;
      }
      segments = tmp;
      cap = new_cap;
    }
    segments[count] = NULL;
    rc = dup_substr(path + start, seg_len, &segments[count]);
    if (rc != CDD_C_SUCCESS) {
      rc = CDD_C_ERROR_MEMORY;
      goto cleanup;
    }
    count++;
  }

  if (count == 0) {
    if (absolute) {
      rc = c_cdd_strdup("/", &out);
      if (rc != CDD_C_SUCCESS) {
        rc = CDD_C_ERROR_MEMORY;
        goto cleanup;
      }
      *_out_val = out;
      return CDD_C_SUCCESS;
    }
    rc = c_cdd_strdup("", &out);
    if (rc != CDD_C_SUCCESS) {
      rc = CDD_C_ERROR_MEMORY;
      goto cleanup;
    }
    *_out_val = out;
    return CDD_C_SUCCESS;
  }

  out_len = absolute ? 1 : 0;
  for (i = 0; i < count; ++i) {
    out_len += strlen(segments[i]);
    if (i + 1 < count)
      out_len += 1;
  }
  if (trailing)
    out_len += 1;

  out = (char *)C_CDD_MALLOC(out_len + 1);
  if (!out) {
    rc = CDD_C_ERROR_MEMORY;
    goto cleanup;
  }
  {
    size_t pos = 0;
    if (absolute)
      out[pos++] = '/';
    for (i = 0; i < count; ++i) {
      size_t seg_len = strlen(segments[i]);
      memcpy(out + pos, segments[i], seg_len);
      pos += seg_len;
      if (i + 1 < count)
        out[pos++] = '/';
    }
    if (trailing)
      out[pos++] = '/';
    out[pos] = '\0';
  }

cleanup:
  if (segments) {
    for (i = 0; i < count; ++i)
      C_CDD_FREE(segments[i]);
    C_CDD_FREE(segments);
  }
  *_out_val = out;
  return rc;
}

/**
 * @brief Resolves a relative URI reference against a base URI per RFC 3986.
 *
 * @param[in] base_uri Base URI string.
 * @param[in] ref Relative or absolute URI reference.
 * @param[out] _out_val Newly allocated resolved URI string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t resolve_uri_reference(const char *base_uri, const char *ref,
                                    char **_out_val) {
  size_t ref_len;
  size_t base_len = 0;
  size_t prefix_len = 0;
  size_t path_offset = 0;
  size_t path_len = 0;
  const char *base_path = NULL;
  const char *base_dir = NULL;
  size_t base_dir_len = 0;
  char *combined = NULL;
  char *normalized = NULL;
  char *out = NULL;
  cdd_c_error_t rc;

  if (!ref) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  ref_len = strlen(ref);
  if (ref_len == 0) {
    *_out_val = NULL;
    return c_cdd_strdup("", _out_val);
  }
  {
    int has_scheme = 0;
    g_cdd_last_alloc_err = uri_has_scheme_prefix(ref, ref_len, &has_scheme);
    if (has_scheme) {
      *_out_val = NULL;
      return c_cdd_strdup(ref, _out_val);
    }
  }
  if (!base_uri || !*base_uri) {
    *_out_val = NULL;
    return c_cdd_strdup(ref, _out_val);
  }

  if (ref_len >= 2 && memcmp(ref, "//", 2) == 0) {
    size_t b_len = 0;
    size_t scheme_len = 0;
    g_cdd_last_alloc_err = uri_base_len(base_uri, &b_len);
    g_cdd_last_alloc_err = uri_scheme_len(base_uri, b_len, &scheme_len);
    if (scheme_len > 0) {
      size_t out_len = scheme_len + 1 + ref_len;
      out = (char *)C_CDD_MALLOC(out_len + 1);
      if (!out) {
        *_out_val = NULL;
        return CDD_C_ERROR_MEMORY;
      }
      memcpy(out, base_uri, scheme_len + 1);
      memcpy(out + scheme_len + 1, ref, ref_len);
      out[out_len] = '\0';
      *_out_val = out;
      return CDD_C_SUCCESS;
    }
    *_out_val = NULL;
    return c_cdd_strdup(ref, _out_val);
  }

  g_cdd_last_alloc_err = uri_base_len(base_uri, &base_len);
  {
    int has_scheme = 0;
    g_cdd_last_alloc_err =
        uri_has_scheme_prefix(base_uri, base_len, &has_scheme);
    if (has_scheme) {
      size_t scheme_len = 0;
      g_cdd_last_alloc_err = uri_scheme_len(base_uri, base_len, &scheme_len);
      if (scheme_len + 2 < base_len &&
          memcmp(base_uri + scheme_len + 1, "//", 2) == 0) {
        size_t auth_start = scheme_len + 3;
        size_t i = auth_start;
        while (i < base_len && base_uri[i] != '/')
          ++i;
        prefix_len = i;
        path_offset = i;
      }
    }
  }

  path_len = (base_len > path_offset) ? (base_len - path_offset) : 0;
  base_path = base_uri + path_offset;

  if (ref[0] == '/') {
    rc = normalize_path(ref, &normalized);
    if (rc != CDD_C_SUCCESS) {
      *_out_val = NULL;
      return rc;
    }
  } else {
    {
      size_t i = path_len;
      while (i > 0 && base_path[i - 1] != '/')
        --i;
      base_dir = base_path;
      base_dir_len = i;
      if (base_dir_len == 0 && prefix_len > 0) {
        base_dir = "/";
        base_dir_len = 1;
      }
    }

    combined = (char *)C_CDD_MALLOC(base_dir_len + ref_len + 1);
    if (!combined) {
      *_out_val = NULL;
      return CDD_C_ERROR_MEMORY;
    }
    if (base_dir_len > 0)
      memcpy(combined, base_dir, base_dir_len);
    memcpy(combined + base_dir_len, ref, ref_len);
    combined[base_dir_len + ref_len] = '\0';
    rc = normalize_path(combined, &normalized);
  }

  C_CDD_FREE(combined);
  if (!normalized) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }

  {
    size_t norm_len = strlen(normalized);
    size_t out_len = prefix_len + norm_len;
    out = (char *)C_CDD_MALLOC(out_len + 1);
    if (!out) {
      C_CDD_FREE(normalized);
      *_out_val = NULL;
      return CDD_C_ERROR_MEMORY;
    }
    if (prefix_len > 0)
      memcpy(out, base_uri, prefix_len);
    memcpy(out + prefix_len, normalized, norm_len);
    out[out_len] = '\0';
  }
  C_CDD_FREE(normalized);
  *_out_val = out;
  return CDD_C_SUCCESS;
}

/**
 * @brief Computes the canonical document base URI from `$self` or retrieval
 * URI.
 *
 * @param[in] self_uri Optional `$self` URI declared in the document.
 * @param[in] retrieval_uri Optional retrieval URI where the document was
 * loaded.
 * @param[out] _out_val Newly allocated canonical document URI.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t compute_document_uri(const char *self_uri,
                                   const char *retrieval_uri, char **_out_val) {
  char *resolved = NULL;
  char *out = NULL;

  if ((!self_uri || !*self_uri) && (!retrieval_uri || !*retrieval_uri)) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  if (self_uri && *self_uri) {
    if (retrieval_uri && *retrieval_uri) {
      g_cdd_last_alloc_err =
          resolve_uri_reference(retrieval_uri, self_uri, &resolved);
    } else {
      g_cdd_last_alloc_err = c_cdd_strdup(self_uri, &resolved);
    }
  } else {
    g_cdd_last_alloc_err = c_cdd_strdup(retrieval_uri, &resolved);
  }

  if (!resolved) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }

  {
    size_t len = 0;
    g_cdd_last_alloc_err = uri_base_len(resolved, &len);
    g_cdd_last_alloc_err = dup_substr(resolved, len, &out);
  }
  C_CDD_FREE(resolved);
  if (!out) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
  *_out_val = out;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a root JSON object contains top-level OpenAPI document
 * fields.
 *
 * @param[in] root_obj Root JSON object.
 * @param[out] out_has Set to 1 if OpenAPI document fields are present, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t root_has_openapi_fields(const JSON_Object *root_obj,
                                      int *out_has) {
  int has_info = 0, has_paths = 0, has_comps = 0, has_servers = 0;
  int has_webhooks = 0, has_tags = 0, has_sec = 0, has_ext = 0;
  int has_self = 0, has_dialect = 0;
  if (!out_has)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has = 0;
  if (!root_obj)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_object_has_value(root_obj, "info", &has_info);
  if (has_info) {
    *out_has = 1;
    return CDD_C_SUCCESS;
  }
  g_cdd_last_alloc_err =
      cdd_json_object_has_value(root_obj, "paths", &has_paths);
  if (has_paths) {
    *out_has = 1;
    return CDD_C_SUCCESS;
  }
  g_cdd_last_alloc_err =
      cdd_json_object_has_value(root_obj, "components", &has_comps);
  if (has_comps) {
    *out_has = 1;
    return CDD_C_SUCCESS;
  }
  g_cdd_last_alloc_err =
      cdd_json_object_has_value(root_obj, "servers", &has_servers);
  if (has_servers) {
    *out_has = 1;
    return CDD_C_SUCCESS;
  }
  g_cdd_last_alloc_err =
      cdd_json_object_has_value(root_obj, "webhooks", &has_webhooks);
  if (has_webhooks) {
    *out_has = 1;
    return CDD_C_SUCCESS;
  }
  g_cdd_last_alloc_err = cdd_json_object_has_value(root_obj, "tags", &has_tags);
  if (has_tags) {
    *out_has = 1;
    return CDD_C_SUCCESS;
  }
  g_cdd_last_alloc_err =
      cdd_json_object_has_value(root_obj, "security", &has_sec);
  if (has_sec) {
    *out_has = 1;
    return CDD_C_SUCCESS;
  }
  g_cdd_last_alloc_err =
      cdd_json_object_has_value(root_obj, "externalDocs", &has_ext);
  if (has_ext) {
    *out_has = 1;
    return CDD_C_SUCCESS;
  }
  g_cdd_last_alloc_err =
      cdd_json_object_has_value(root_obj, "$self", &has_self);
  g_cdd_last_alloc_err =
      cdd_json_object_has_value(root_obj, "jsonSchemaDialect", &has_dialect);
  *out_has = (has_self | has_dialect) ? 1 : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a root JSON value/object is a standalone JSON Schema
 * document.
 *
 * @param[in] root Root JSON value.
 * @param[in] root_obj Root JSON object (if object).
 * @param[out] out_is_schema Set to 1 if standalone schema document, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t root_is_schema_document(const JSON_Value *root,
                                      const JSON_Object *root_obj,
                                      int *out_is_schema) {
  JSON_Value_Type type = JSONError;
  int has_oa = 0, has_sw = 0, has_oa_fields = 0;
  if (!out_is_schema)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_schema = 0;
  if (!root)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_value_get_type(root, &type);
  if (type == JSONBoolean) {
    *out_is_schema = 1;
    return CDD_C_SUCCESS;
  }
  if (type != JSONObject)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err =
      cdd_json_object_has_value(root_obj, "openapi", &has_oa);
  g_cdd_last_alloc_err =
      cdd_json_object_has_value(root_obj, "swagger", &has_sw);
  if (has_oa || has_sw)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = root_has_openapi_fields(root_obj, &has_oa_fields);
  if (has_oa_fields)
    return CDD_C_SUCCESS;
  *out_is_schema = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Stores the serialized root JSON of a standalone schema document in
 * `spec`.
 *
 * @param[in,out] spec Target OpenAPI specification.
 * @param[in] root Root JSON value.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
cdd_c_error_t store_schema_root_json(struct OpenAPI_Spec *spec,
                                     const JSON_Value *root) {
  char *raw_json = NULL;
  if (!spec || !root)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (spec->schema_root_json)
    return CDD_C_SUCCESS;
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_json_serialize;
    if (g_cdd_fail_json_serialize) {
      g_cdd_fail_json_serialize = 0;
      raw_json = NULL;
    } else {
      g_cdd_last_alloc_err = cdd_json_serialize_to_string(root, &raw_json);
    }
  }
#else
  cdd_json_serialize_to_string(root, &raw_json);
#endif
  if (!raw_json)
    return CDD_C_ERROR_MEMORY;
  g_cdd_last_alloc_err = c_cdd_strdup(raw_json, &spec->schema_root_json);
  g_cdd_last_alloc_err = cdd_json_free_serialized_string(raw_json);
  if (!spec->schema_root_json)
    return CDD_C_ERROR_MEMORY;
  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves a `$ref` against a specification and its document registry.
 *
 * @param[in] spec Source OpenAPI specification.
 * @param[in] ref Reference string to resolve.
 * @param[out] _out_val Resolved target specification and reference structure.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t resolve_ref_target(const struct OpenAPI_Spec *spec,
                                 const char *ref,
                                 struct ResolvedRefTarget *_out_val) {
  struct ResolvedRefTarget out;
  const char *hash;
  size_t base_len;
  char *base_part = NULL;
  char *resolved_base = NULL;

  out.spec = spec;
  out.ref = ref;
  out.resolved_ref = NULL;

  if (!spec || !ref) {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }

  hash = strchr(ref, '#');
  if (!hash || hash == ref) {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }

  base_len = (size_t)(hash - ref);
  g_cdd_last_alloc_err = dup_substr(ref, base_len, &base_part);
  if (!base_part) {
    *_out_val = out;
    return CDD_C_ERROR_MEMORY;
  }

  if (spec->document_uri && *spec->document_uri) {
    g_cdd_last_alloc_err =
        resolve_uri_reference(spec->document_uri, base_part, &resolved_base);
    if (resolved_base) {
      C_CDD_FREE(base_part);
    } else {
      resolved_base = base_part;
    }
  } else {
    resolved_base = base_part;
  }

  if (spec->doc_registry) {
    size_t i;
    for (i = 0; i < spec->doc_registry->count; ++i) {
      const struct OpenAPI_DocRegistryEntry *entry =
          &spec->doc_registry->entries[i];
      if (entry->base_uri && strcmp(entry->base_uri, resolved_base) == 0) {
        out.spec = entry->spec;
        break;
      }
    }
  }

  {
    if (resolved_base != base_part) {
      size_t resolved_len = strlen(resolved_base);
      size_t hash_len = strlen(hash);
      out.resolved_ref = (char *)C_CDD_MALLOC(resolved_len + hash_len + 1);
      if (!out.resolved_ref) {
        C_CDD_FREE(resolved_base);
        *_out_val = out;
        return CDD_C_ERROR_MEMORY;
      }
      memcpy(out.resolved_ref, resolved_base, resolved_len);
      memcpy(out.resolved_ref + resolved_len, hash, hash_len);
      out.resolved_ref[resolved_len + hash_len] = '\0';
      out.ref = out.resolved_ref;
    }
    C_CDD_FREE(resolved_base);
  }

  *_out_val = out;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a `$ref`'s base URI matches the specification's `$self` or
 * `document_uri`.
 *
 * @param[in] spec OpenAPI specification context.
 * @param[in] ref Full reference string.
 * @param[in] hash Pointer to the `#` character within `ref`.
 * @param[out] out_matches Set to 1 if base matches self, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t ref_base_matches_self(const struct OpenAPI_Spec *spec,
                                    const char *ref, const char *hash,
                                    int *out_matches) {
  size_t base_len;
  const char *self_uri;
  const char *self_hash;
  const char *self_base;
  size_t self_len;
  int has_scheme = 0;

  if (!out_matches)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_matches = 0;

  if (!ref || !hash)
    return CDD_C_SUCCESS;
  if (hash == ref) {
    *out_matches = 1;
    return CDD_C_SUCCESS;
  }
  if (!spec)
    return CDD_C_SUCCESS;

  if (spec->document_uri && *spec->document_uri) {
    const char *base_uri = spec->document_uri;
    size_t uri_len = 0;
    g_cdd_last_alloc_err = uri_base_len(base_uri, &uri_len);
    base_len = (size_t)(hash - ref);
    if (uri_len == base_len && strncmp(ref, base_uri, base_len) == 0) {
      *out_matches = 1;
      return CDD_C_SUCCESS;
    }
    g_cdd_last_alloc_err =
        uri_has_scheme_prefix(base_uri, uri_len, &has_scheme);
    if (!has_scheme) {
      const char *rel = base_uri;
      size_t rel_len = uri_len;
      while (rel_len >= 2 && ((rel[0] == '.') & (rel[1] == '/'))) {
        rel += 2;
        rel_len -= 2;
      }
      if (rel_len == 0)
        return CDD_C_SUCCESS;
      if (base_len >= rel_len &&
          strncmp(ref + (base_len - rel_len), rel, rel_len) == 0 &&
          (rel[0] == '/' || base_len == rel_len ||
           ref[base_len - rel_len - 1] == '/')) {
        *out_matches = 1;
        return CDD_C_SUCCESS;
      }
    }
    return CDD_C_SUCCESS;
  }

  if (!spec->self_uri || !*spec->self_uri)
    return CDD_C_SUCCESS;

  base_len = (size_t)(hash - ref);
  self_uri = spec->self_uri;
  self_hash = strchr(self_uri, '#');
  self_base = self_uri;
  self_len = self_hash ? (size_t)(self_hash - self_uri) : strlen(self_uri);

  /* Exact base match (absolute $self). */
  if (base_len == self_len && strncmp(ref, self_base, base_len) == 0) {
    *out_matches = 1;
    return CDD_C_SUCCESS;
  }

  /* Relative $self: allow refs whose base URI ends with the self path. */
  g_cdd_last_alloc_err =
      uri_has_scheme_prefix(self_base, self_len, &has_scheme);
  if (!has_scheme) {
    while (self_len >= 2 && ((self_base[0] == '.') & (self_base[1] == '/'))) {
      self_base += 2;
      self_len -= 2;
    }
    if (self_len == 0)
      return CDD_C_SUCCESS;
    if (base_len >= self_len &&
        strncmp(ref + (base_len - self_len), self_base, self_len) == 0) {
      if (self_base[0] == '/' || base_len == self_len ||
          ref[base_len - self_len - 1] == '/') {
        *out_matches = 1;
        return CDD_C_SUCCESS;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts the trailing component name from a `$ref` matching a prefix.
 *
 * @param[in] spec OpenAPI specification context.
 * @param[in] ref Reference string to match.
 * @param[in] prefix Component path prefix (e.g. `"#/components/schemas/"`).
 * @param[out] _out_val Pointer into `ref` after the prefix, or NULL if not
 * matching.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t ref_name_from_prefix(const struct OpenAPI_Spec *spec,
                                   const char *ref, const char *prefix,
                                   const char **_out_val) {
  size_t prefix_len;
  const char *name;
  const char *hash;
  int matches_self = 0;
  if (!ref || !prefix) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  prefix_len = strlen(prefix);
  if (strncmp(ref, prefix, prefix_len) == 0) {
    name = ref + prefix_len;
    if (!*name) {
      *_out_val = NULL;
      return CDD_C_SUCCESS;
    }
    if (strchr(name, '/') != NULL) {
      *_out_val = NULL;
      return CDD_C_SUCCESS;
    }
    {
      *_out_val = name;
      return CDD_C_SUCCESS;
    }
  }
  hash = strchr(ref, '#');
  if (!hash) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  g_cdd_last_alloc_err = ref_base_matches_self(spec, ref, hash, &matches_self);
  if (!matches_self) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (strncmp(hash, prefix, prefix_len) != 0) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  name = hash + prefix_len;
  if (!*name) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (strchr(name, '/') != NULL) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = name;
    return CDD_C_SUCCESS;
  }
}
