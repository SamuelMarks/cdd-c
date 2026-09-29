/**
 * @file openapi_uri.c
 * @brief URI resolution, path normalization, and document registry routing.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Executes the json pointer unescape operation.
 */
cdd_c_error_t json_pointer_unescape(const char *in, char **_out_val) {
  size_t len, i, j;
  char *out;
  if (!in) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  len = strlen(in);
  out = (char *)(size_t)C_CDD_MALLOC(len + 1);
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0, j = 0; i < len; ++i) {
    if (in[i] == '~' && i + 1 < len) {
      if (in[i + 1] == '0') {
        out[j++] = '~';
        i++;
        continue;
      }
      if (in[i + 1] == '1') {
        out[j++] = '/';
        i++;
        continue;
      }
    }
    out[j++] = in[i];
  }
  out[j] = '\0';
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the uri has scheme prefix operation.
 */
cdd_c_error_t uri_has_scheme_prefix(const char *uri, size_t len) {
  size_t i;
  if (!uri || len == 0)
    return CDD_C_SUCCESS;
  for (i = 0; i < len; ++i) {
    char c = uri[i];
    if (c == ':')
      return CDD_C_ERROR_UNKNOWN;
    if (c == '/' || c == '?' || c == '#')
      break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the uri base len operation.
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
 * @brief Executes the uri scheme len operation.
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
 * @brief Executes the dup substr operation.
 */
cdd_c_error_t dup_substr(const char *src, size_t len, char **_out_val) {
  char *out;
  if (!src) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  out = (char *)(size_t)C_CDD_MALLOC(len + 1);
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  memcpy(out, src, len);
  out[len] = '\0';
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the normalize path operation.
 */
cdd_c_error_t normalize_path(const char *path, char **_out_val) {
  char *_ast_dup_substr_7 = NULL;
  char *_ast_strdup_40 = NULL;
  char *_ast_strdup_41 = NULL;
  char *_ast_strdup_42 = NULL;
  size_t i = 0;
  int absolute = 0;
  int trailing = 0;
  char **segments = NULL;
  size_t count = 0;
  size_t cap = 0;
  char *out = NULL;
  size_t out_len = 0;

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
      if (path[i] == '/')
        ++i;
      continue;
    }
    if (seg_len == 1 && path[start] == '.') {
      if (path[i] == '/')
        ++i;
      continue;
    }
    if (seg_len == 2 && path[start] == '.' && path[start + 1] == '.') {
      if (count > 0 && strcmp(segments[count - 1], "..") != 0) {
        free(segments[count - 1]);
        count--;
      } else if (!absolute) {
        if (count == cap) {
          size_t new_cap = cap ? cap * 2 : 4;
          char **tmp = (char **)realloc(segments, new_cap * sizeof(*segments));
          if (!tmp)
            goto cleanup;
          segments = tmp;
          cap = new_cap;
        }
        segments[count++] =
            (c_cdd_strdup("..", &_ast_strdup_40), _ast_strdup_40);
      }
      if (path[i] == '/')
        ++i;
      continue;
    }
    if (count == cap) {
      size_t new_cap = cap ? cap * 2 : 4;
      char **tmp = (char **)realloc(segments, new_cap * sizeof(*segments));
      if (!tmp)
        goto cleanup;
      segments = tmp;
      cap = new_cap;
    }
    segments[count] = (dup_substr(path + start, seg_len, &_ast_dup_substr_7),
                       _ast_dup_substr_7);
    if (!segments[count])
      goto cleanup;
    count++;
    if (path[i] == '/')
      ++i;
  }

  if (count == 0) {
    if (absolute) {
      out = (c_cdd_strdup("/", &_ast_strdup_41), _ast_strdup_41);
      if (!out)
        goto cleanup;
      {
        *_out_val = out;
        return CDD_C_SUCCESS;
      }
    }
    out = (c_cdd_strdup("", &_ast_strdup_42), _ast_strdup_42);
    if (!out)
      goto cleanup;
    {
      *_out_val = out;
      return CDD_C_SUCCESS;
    }
  }

  out_len = absolute ? 1 : 0;
  for (i = 0; i < count; ++i) {
    out_len += strlen(segments[i]);
    if (i + 1 < count)
      out_len += 1;
  }
  if (trailing && out_len > 0 && (!absolute || out_len > 1))
    out_len += 1;

  out = (char *)(size_t)malloc(out_len + 1);
  if (!out)
    goto cleanup;
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
    if (trailing && pos > 0 && out[pos - 1] != '/')
      out[pos++] = '/';
    out[pos] = '\0';
  }

cleanup:
  if (segments) {
    for (i = 0; i < count; ++i)
      free(segments[i]);
    free(segments);
  }
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the resolve uri reference operation.
 */
cdd_c_error_t resolve_uri_reference(const char *base_uri, const char *ref,
                                    char **_out_val) {
  size_t _ast_uri_scheme_len_8 = 0;
  size_t _ast_uri_base_len_9 = 0;
  size_t _ast_uri_base_len_10 = 0;
  size_t _ast_uri_scheme_len_11 = 0;
  char *_ast_normalize_path_12 = NULL;
  char *_ast_normalize_path_13 = NULL;
  char *_ast_strdup_43 = NULL;
  char *_ast_strdup_44 = NULL;
  char *_ast_strdup_45 = NULL;
  char *_ast_strdup_46 = NULL;
  size_t ref_len;
  size_t base_len;
  size_t prefix_len = 0;
  size_t path_offset = 0;
  size_t path_len = 0;
  const char *base_path = NULL;
  const char *base_dir = NULL;
  size_t base_dir_len = 0;
  char *combined = NULL;
  char *normalized = NULL;
  char *out = NULL;

  if (!ref) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  ref_len = strlen(ref);
  if (ref_len == 0) {
    *_out_val = (c_cdd_strdup("", &_ast_strdup_43), _ast_strdup_43);
    return CDD_C_SUCCESS;
  }
  if (uri_has_scheme_prefix(ref, ref_len)) {
    *_out_val = (c_cdd_strdup(ref, &_ast_strdup_44), _ast_strdup_44);
    return CDD_C_SUCCESS;
  }
  if (!base_uri || !*base_uri) {
    *_out_val = (c_cdd_strdup(ref, &_ast_strdup_45), _ast_strdup_45);
    return CDD_C_SUCCESS;
  }

  if (ref_len >= 2 && ref[0] == '/' && ref[1] == '/') {
    size_t scheme_len =
        (uri_scheme_len(base_uri,
                        (uri_base_len(base_uri, &_ast_uri_base_len_9),
                         _ast_uri_base_len_9),
                        &_ast_uri_scheme_len_8),
         _ast_uri_scheme_len_8);
    if (scheme_len > 0) {
      size_t out_len = scheme_len + 1 + ref_len;
      out = (char *)(size_t)malloc(out_len + 1);
      if (!out) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      memcpy(out, base_uri, scheme_len + 1);
      memcpy(out + scheme_len + 1, ref, ref_len);
      out[out_len] = '\0';
      {
        *_out_val = out;
        return CDD_C_SUCCESS;
      }
    }
    {
      *_out_val = (c_cdd_strdup(ref, &_ast_strdup_46), _ast_strdup_46);
      return CDD_C_SUCCESS;
    }
  }

  base_len =
      (uri_base_len(base_uri, &_ast_uri_base_len_10), _ast_uri_base_len_10);
  if (uri_has_scheme_prefix(base_uri, base_len)) {
    size_t scheme_len =
        (uri_scheme_len(base_uri, base_len, &_ast_uri_scheme_len_11),
         _ast_uri_scheme_len_11);
    if (scheme_len > 0 && scheme_len + 2 < base_len &&
        base_uri[scheme_len + 1] == '/' && base_uri[scheme_len + 2] == '/') {
      size_t auth_start = scheme_len + 3;
      size_t i = auth_start;
      while (i < base_len && base_uri[i] != '/')
        ++i;
      prefix_len = i;
      path_offset = i;
    }
  }

  path_len = (base_len > path_offset) ? (base_len - path_offset) : 0;
  base_path = base_uri + path_offset;

  if (ref[0] == '/') {
    normalized =
        (normalize_path(ref, &_ast_normalize_path_12), _ast_normalize_path_12);
  } else {
    if (path_len == 0) {
      if (prefix_len > 0) {
        base_dir = "/";
        base_dir_len = 1;
      }
    } else {
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

    combined = (char *)(size_t)malloc(base_dir_len + ref_len + 1);
    if (!combined) {
      *_out_val = NULL;
      return CDD_C_SUCCESS;
    }
    if (base_dir_len > 0)
      memcpy(combined, base_dir, base_dir_len);
    memcpy(combined + base_dir_len, ref, ref_len);
    combined[base_dir_len + ref_len] = '\0';
    normalized = (normalize_path(combined, &_ast_normalize_path_13),
                  _ast_normalize_path_13);
  }

  free(combined);
  if (!normalized) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  {
    size_t norm_len = strlen(normalized);
    size_t out_len = prefix_len + norm_len;
    out = (char *)(size_t)malloc(out_len + 1);
    if (!out) {
      free(normalized);
      {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
    }
    if (prefix_len > 0)
      memcpy(out, base_uri, prefix_len);
    memcpy(out + prefix_len, normalized, norm_len);
    out[out_len] = '\0';
  }
  free(normalized);
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the compute document uri operation.
 */
cdd_c_error_t compute_document_uri(const char *self_uri,
                                   const char *retrieval_uri, char **_out_val) {
  char *_ast_resolve_uri_reference_14 = NULL;
  size_t _ast_uri_base_len_15 = 0;
  char *_ast_dup_substr_16 = NULL;
  char *_ast_strdup_47 = NULL;
  char *_ast_strdup_48 = NULL;
  char *resolved = NULL;
  char *out = NULL;

  if (self_uri && *self_uri) {
    if (retrieval_uri && *retrieval_uri) {
      resolved = (resolve_uri_reference(retrieval_uri, self_uri,
                                        &_ast_resolve_uri_reference_14),
                  _ast_resolve_uri_reference_14);
    } else {
      resolved = (c_cdd_strdup(self_uri, &_ast_strdup_47), _ast_strdup_47);
    }
  } else if (retrieval_uri && *retrieval_uri) {
    resolved = (c_cdd_strdup(retrieval_uri, &_ast_strdup_48), _ast_strdup_48);
  }

  if (!resolved) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  {
    size_t len =
        (uri_base_len(resolved, &_ast_uri_base_len_15), _ast_uri_base_len_15);
    out = (dup_substr(resolved, len, &_ast_dup_substr_16), _ast_dup_substr_16);
  }
  free(resolved);
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the root has openapi fields operation.
 */
cdd_c_error_t root_has_openapi_fields(const JSON_Object *root_obj) {
  if (!root_obj)
    return CDD_C_SUCCESS;
  return json_object_has_value(root_obj, "info") ||
         json_object_has_value(root_obj, "paths") ||
         json_object_has_value(root_obj, "components") ||
         json_object_has_value(root_obj, "servers") ||
         json_object_has_value(root_obj, "webhooks") ||
         json_object_has_value(root_obj, "tags") ||
         json_object_has_value(root_obj, "security") ||
         json_object_has_value(root_obj, "externalDocs") ||
         json_object_has_value(root_obj, "$self") ||
         json_object_has_value(root_obj, "jsonSchemaDialect");
}

/**
 * @brief Executes the root is schema document operation.
 */
cdd_c_error_t root_is_schema_document(const JSON_Value *root,
                                      const JSON_Object *root_obj) {
  JSON_Value_Type type;
  if (!root)
    return CDD_C_SUCCESS;
  type = json_value_get_type(root);
  if (type == JSONBoolean)
    return CDD_C_ERROR_UNKNOWN;
  if (type != JSONObject || !root_obj)
    return CDD_C_SUCCESS;
  if (json_object_has_value(root_obj, "openapi") ||
      json_object_has_value(root_obj, "swagger"))
    return CDD_C_SUCCESS;
  if (root_has_openapi_fields(root_obj))
    return CDD_C_SUCCESS;
  return CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief Executes the store schema root json operation.
 */
cdd_c_error_t store_schema_root_json(struct OpenAPI_Spec *spec,
                                     const JSON_Value *root) {
  char *_ast_strdup_49 = NULL;
  char *raw_json;
  if (!spec || !root)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (spec->schema_root_json)
    return CDD_C_SUCCESS;
  raw_json = json_serialize_to_string(root);
  if (!raw_json)
    return CDD_C_ERROR_MEMORY;
  spec->schema_root_json =
      (c_cdd_strdup(raw_json, &_ast_strdup_49), _ast_strdup_49);
  json_free_serialized_string(raw_json);
  if (!spec->schema_root_json)
    return CDD_C_ERROR_MEMORY;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the resolve ref target operation.
 */
cdd_c_error_t resolve_ref_target(const struct OpenAPI_Spec *spec,
                                 const char *ref,
                                 struct ResolvedRefTarget *_out_val) {
  char *_ast_dup_substr_17 = NULL;
  char *_ast_resolve_uri_reference_18 = NULL;
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
  base_part =
      (dup_substr(ref, base_len, &_ast_dup_substr_17), _ast_dup_substr_17);
  if (!base_part) {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }

  if (spec->document_uri && *spec->document_uri) {
    resolved_base = (resolve_uri_reference(spec->document_uri, base_part,
                                           &_ast_resolve_uri_reference_18),
                     _ast_resolve_uri_reference_18);
    if (resolved_base) {
      free(base_part);
    } else {
      resolved_base = base_part;
    }
  } else {
    resolved_base = base_part;
  }

  if (spec->doc_registry && resolved_base) {
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

  if (resolved_base) {
    size_t resolved_len = strlen(resolved_base);
    if (resolved_len != base_len ||
        strncmp(ref, resolved_base, base_len) != 0) {
      size_t hash_len = strlen(hash);
      out.resolved_ref = (char *)(size_t)malloc(resolved_len + hash_len + 1);
      if (out.resolved_ref) {
        memcpy(out.resolved_ref, resolved_base, resolved_len);
        memcpy(out.resolved_ref + resolved_len, hash, hash_len);
        out.resolved_ref[resolved_len + hash_len] = '\0';
        out.ref = out.resolved_ref;
      }
    }
    free(resolved_base);
  }

  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the ref base matches self operation.
 */
cdd_c_error_t ref_base_matches_self(const struct OpenAPI_Spec *spec,
                                    const char *ref, const char *hash) {
  size_t _ast_uri_base_len_21 = 0;
  size_t base_len;
  const char *self_uri;
  const char *self_hash;
  const char *self_base;
  size_t self_len;

  if (!ref || !hash)
    return CDD_C_SUCCESS;
  if (hash == ref)
    return CDD_C_ERROR_UNKNOWN;
  if (!spec)
    return CDD_C_SUCCESS;

  if (spec->document_uri && *spec->document_uri) {
    const char *base_uri = spec->document_uri;
    size_t uri_len =
        (uri_base_len(base_uri, &_ast_uri_base_len_21), _ast_uri_base_len_21);
    base_len = (size_t)(hash - ref);
    if (uri_len == base_len && strncmp(ref, base_uri, base_len) == 0)
      return CDD_C_ERROR_UNKNOWN;
    if (!uri_has_scheme_prefix(base_uri, uri_len)) {
      const char *rel = base_uri;
      size_t rel_len = uri_len;
      while (rel_len >= 2 && rel[0] == '.' && rel[1] == '/') {
        rel += 2;
        rel_len -= 2;
      }
      if (rel_len == 0)
        return CDD_C_SUCCESS;
      if (base_len >= rel_len &&
          strncmp(ref + (base_len - rel_len), rel, rel_len) == 0) {
        if (rel[0] == '/')
          return CDD_C_ERROR_UNKNOWN;
        if (base_len == rel_len)
          return CDD_C_ERROR_UNKNOWN;
        if (ref[base_len - rel_len - 1] == '/')
          return CDD_C_ERROR_UNKNOWN;
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
  if (base_len == self_len && strncmp(ref, self_base, base_len) == 0)
    return CDD_C_ERROR_UNKNOWN;

  /* Relative $self: allow refs whose base URI ends with the self path. */
  if (!uri_has_scheme_prefix(self_base, self_len)) {
    while (self_len >= 2 && self_base[0] == '.' && self_base[1] == '/') {
      self_base += 2;
      self_len -= 2;
    }
    if (self_len == 0)
      return CDD_C_SUCCESS;
    if (base_len >= self_len &&
        strncmp(ref + (base_len - self_len), self_base, self_len) == 0) {
      if (self_base[0] == '/')
        return CDD_C_ERROR_UNKNOWN;
      if (base_len == self_len)
        return CDD_C_ERROR_UNKNOWN;
      if (ref[base_len - self_len - 1] == '/')
        return CDD_C_ERROR_UNKNOWN;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the ref name from prefix operation.
 */
cdd_c_error_t ref_name_from_prefix(const struct OpenAPI_Spec *spec,
                                   const char *ref, const char *prefix,
                                   char **_out_val) {
  size_t prefix_len;
  const char *name;
  const char *hash;
  if (!ref || !prefix) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  prefix_len = strlen(prefix);
  if (strncmp(ref, prefix, prefix_len) == 0) {
    name = ref + prefix_len;
    if (!name || !*name) {
      *_out_val = NULL;
      return CDD_C_SUCCESS;
    }
    if (strchr(name, '/') != NULL) {
      *_out_val = NULL;
      return CDD_C_SUCCESS;
    }
    {
      *_out_val = (char *)(size_t)(name);
      return CDD_C_SUCCESS;
    }
  }
  hash = strchr(ref, '#');
  if (!hash) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (!ref_base_matches_self(spec, ref, hash)) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (strncmp(hash, prefix, prefix_len) != 0) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  name = hash + prefix_len;
  if (!name || !*name) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (strchr(name, '/') != NULL) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = (char *)(size_t)(name);
    return CDD_C_SUCCESS;
  }
}
