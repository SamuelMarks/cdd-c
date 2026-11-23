/**
 * @file openapi.c
 * @brief OpenAPI specification top-level loader and schema lookup.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Parses an optional Swagger 2.0 string list (`schemes`, `consumes`, or
 * `produces`).
 *
 * @param[in] root_obj Root Swagger 2.0 JSON object.
 * @param[in] key Property name.
 * @param[out] out_arr Destination allocated string array.
 * @param[out] out_count Destination array element count.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t parse_swagger_str_list(const JSON_Object *root_obj,
                                            const char *key, char ***out_arr,
                                            size_t *out_count) {
  JSON_Array *arr = NULL;
  size_t count = 0, i;
  cdd_c_error_t rc;
  rc = oa_json_opt_array(root_obj, key, &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!arr)
    return CDD_C_SUCCESS;
  rc = cdd_json_array_get_count(arr, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out_arr = (char **)C_CDD_CALLOC(count, sizeof(char *));
  if (!*out_arr)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;
  for (i = 0; i < count; ++i) {
    const char *s_str = NULL;
    rc = oa_json_array_opt_string(arr, i, &s_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = c_cdd_strdup(s_str, &(*out_arr)[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Injects a built-in OAuth2 schema into `schemas` when absent.
 *
 * @param[in,out] schemas Target `components.schemas` JSON_Object.
 * @param[in] name Schema name.
 * @param[in] schema_json Raw JSON schema literal (<509 chars).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t inject_oauth2_schema_if_absent(JSON_Object *schemas,
                                                    const char *name,
                                                    const char *schema_json) {
  int has_val = 0;
  JSON_Value *parsed_val = NULL;
  cdd_c_error_t rc;
  rc = oa_json_has_value(schemas, name, &has_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_val)
    return CDD_C_SUCCESS;
  rc = cdd_json_parse_string(schema_json, &parsed_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_object_set_value_safe(schemas, name, parsed_val);
}

/**
 * @brief Injects default OAuth2 schemas into `comps_obj` when an `oauth2`
 * security scheme is declared.
 *
 * @param[in,out] comps_obj Components JSON object.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t ensure_oauth2_default_schemas(JSON_Object *comps_obj) {
  int has_oauth2 = 0;
  JSON_Object *schemes = NULL;
  JSON_Object *schemas = NULL;
  cdd_c_error_t rc;

  rc = oa_json_opt_object(comps_obj, "securitySchemes", &schemes);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (schemes) {
    size_t scount = 0, sk;
    rc = oa_json_object_count(schemes, &scount);
    if (rc != CDD_C_SUCCESS)
      return rc;
    for (sk = 0; sk < scount; ++sk) {
      const char *sname = NULL;
      JSON_Object *sec_obj = NULL;
      const char *type = NULL;
      rc = oa_json_object_name(schemes, sk, &sname);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = oa_json_opt_object(schemes, sname, &sec_obj);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = oa_json_opt_string(sec_obj, "type", &type);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (type && strcmp(type, "oauth2") == 0) {
        has_oauth2 = 1;
        break;
      }
    }
  }
  if (!has_oauth2)
    return CDD_C_SUCCESS;

  rc = oa_json_opt_object(comps_obj, "schemas", &schemas);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!schemas) {
    JSON_Value *schemas_val = NULL;
#ifdef CDD_BUILD_TESTS
    if (g_cdd_alloc_fail && --g_cdd_alloc_fail == 0)
      return CDD_C_ERROR_MEMORY;
#endif
    rc = cdd_json_value_init_object(&schemas_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_object_set_value_safe(comps_obj, "schemas", schemas_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = oa_json_opt_object(comps_obj, "schemas", &schemas);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = inject_oauth2_schema_if_absent(
      schemas, "OAuth2TokenRequest",
      "{\"type\":\"object\",\"properties\":{\"grant_type\":{\"type\":"
      "\"string\",\"enum\":[\"password\",\"authorization_code\","
      "\"client_credentials\",\"refresh_token\"]},\"username\":{\"type\":"
      "\"string\"},\"password\":{\"type\":\"string\",\"writeOnly\":true},"
      "\"scope\":{\"type\":\"string\"},\"client_id\":{\"type\":\"string\"},"
      "\"client_secret\":{\"type\":\"string\",\"writeOnly\":true},"
      "\"refresh_token\":{\"type\":\"string\"},\"code\":{\"type\":\"string\"},"
      "\"redirect_uri\":{\"type\":\"string\"}},\"required\":[\"grant_type\"]}");
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = inject_oauth2_schema_if_absent(
      schemas, "OAuth2TokenResponse",
      "{\"type\":\"object\",\"properties\":{\"access_token\":{\"type\":"
      "\"string\"},\"token_type\":{\"type\":\"string\"},\"expires_in\":{"
      "\"type\":\"integer\"},\"refresh_token\":{\"type\":\"string\"},"
      "\"scope\":{\"type\":\"string\"}},\"required\":[\"access_token\","
      "\"token_type\"]}");
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = inject_oauth2_schema_if_absent(
      schemas, "OAuth2Client",
      "{\"type\":\"object\",\"properties\":{\"client_id\":{\"type\":\"string\","
      "\"description\":\"[UNIQUE]\"},\"client_secret_hash\":{\"type\":"
      "\"string\",\"writeOnly\":true},\"redirect_uris\":{\"type\":\"array\","
      "\"items\":{\"type\":\"string\"}},\"grant_types\":{\"type\":\"array\","
      "\"items\":{\"type\":\"string\"}}},\"required\":[\"client_id\","
      "\"client_secret_hash\"]}");
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = inject_oauth2_schema_if_absent(
      schemas, "OAuth2User",
      "{\"type\":\"object\",\"properties\":{\"username\":{\"type\":\"string\","
      "\"description\":\"[UNIQUE]\"},\"password_hash\":{\"type\":\"string\","
      "\"writeOnly\":true},\"created_at\":{\"type\":\"integer\",\"format\":"
      "\"int64\"}},\"required\":[\"username\",\"password_hash\"]}");
  if (rc != CDD_C_SUCCESS)
    return rc;
  return inject_oauth2_schema_if_absent(
      schemas, "OAuth2Token",
      "{\"type\":\"object\",\"properties\":{\"access_token\":{\"type\":"
      "\"string\",\"description\":\"[UNIQUE]\"},\"refresh_token\":{\"type\":"
      "\"string\",\"description\":\"[UNIQUE]\"},\"expires_at\":{\"type\":"
      "\"integer\",\"format\":\"int64\"},\"client_id\":{\"type\":\"integer\","
      "\"description\":\"[FK=OAuth2Client]\"},\"user_id\":{\"type\":"
      "\"integer\",\"description\":\"[FK=OAuth2User]\"}},\"required\":["
      "\"access_token\"]}");
}

/*
 * @brief Parses a Parson JSON object into an OpenAPI_Spec with document
 * context.
 *
 * @param[in] root The root Parson JSON_Value.
 * @param[out] out The target OpenAPI_Spec structure.
 * @param[in] retrieval_uri Optional document retrieval URI.
 * @param[in,out] registry Optional document registry for multi-file resolution.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t openapi_load_from_json_internal(
    const JSON_Value *root, struct OpenAPI_Spec *out, const char *retrieval_uri,
    struct OpenAPI_DocRegistry *registry) {
  JSON_Object *root_obj = NULL;
  JSON_Value_Type root_type = JSONError;
  JSON_Object *paths_obj = NULL;
  JSON_Object *webhooks_obj = NULL;
  JSON_Object *comps_obj = NULL;
  const char *version = NULL;
  const char *swagger_version = NULL;
  cdd_c_error_t rc;

  if (!root || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_json_value_get_type(root, &root_type);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (root_type == JSONObject) {
    rc = cdd_json_value_get_object(root, &root_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (root_type != JSONBoolean) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  out->doc_registry = registry;
  if (retrieval_uri && *retrieval_uri) {
    rc = c_cdd_strdup(retrieval_uri, &out->retrieval_uri);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (root_obj) {
    rc = oa_json_opt_string(root_obj, "openapi", &version);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = oa_json_opt_string(root_obj, "swagger", &swagger_version);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  if (!version && !swagger_version) {
    int is_schema = 0;
    const char *schema_id = NULL;
    rc = root_is_schema_document(root, root_obj, &is_schema);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (!is_schema) {
      rc = CDD_C_ERROR_INVALID_ARGUMENT;
      goto cleanup;
    }
    out->is_schema_document = 1;
    if (root_obj) {
      rc = oa_json_opt_string(root_obj, "$id", &schema_id);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    if ((schema_id && *schema_id) || out->retrieval_uri != NULL) {
      rc = compute_document_uri(schema_id, out->retrieval_uri,
                                &out->document_uri);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    rc = store_schema_root_json(out, root);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (registry) {
      rc = openapi_doc_registry_add(registry, out);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    return CDD_C_SUCCESS;
  }

  if (version) {
    int is_supported = 0;
    rc = openapi_version_supported(version, &is_supported);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (!is_supported) {
      rc = CDD_C_ERROR_INVALID_ARGUMENT;
      goto cleanup;
    }
    rc = c_cdd_strdup(version, &out->openapi_version);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (swagger_version) {
    rc = c_cdd_strdup(swagger_version, &out->swagger_version);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  if (out->swagger_version) {
    const char *host = NULL;
    const char *basePath = NULL;

    rc = oa_json_opt_string(root_obj, "host", &host);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (host) {
      rc = c_cdd_strdup(host, &out->host);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    rc = oa_json_opt_string(root_obj, "basePath", &basePath);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (basePath) {
      rc = c_cdd_strdup(basePath, &out->basePath);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    rc = parse_swagger_str_list(root_obj, "schemes", &out->schemes,
                                &out->n_schemes);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = parse_swagger_str_list(root_obj, "consumes", &out->consumes,
                                &out->n_consumes);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = parse_swagger_str_list(root_obj, "produces", &out->produces,
                                &out->n_produces);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  {
    const char *self_uri = NULL;
    const char *dialect = NULL;
    JSON_Object *ext_docs = NULL;

    rc = oa_json_opt_string(root_obj, "$self", &self_uri);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (self_uri) {
      rc = c_cdd_strdup(self_uri, &out->self_uri);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    if (out->self_uri || out->retrieval_uri) {
      rc = compute_document_uri(out->self_uri, out->retrieval_uri,
                                &out->document_uri);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    rc = oa_json_opt_string(root_obj, "jsonSchemaDialect", &dialect);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (dialect) {
      rc = c_cdd_strdup(dialect, &out->json_schema_dialect);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    rc = collect_extensions(root_obj, &out->extensions_json);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = parse_info(root_obj, out);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = oa_json_opt_object(root_obj, "externalDocs", &ext_docs);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (ext_docs) {
      rc = parse_external_docs(ext_docs, &out->external_docs);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }

  rc = parse_tags(root_obj, out);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = validate_tag_parents(out);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = parse_security_field(root_obj, "security", &out->security,
                            &out->n_security, &out->security_set);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = parse_servers(root_obj, out);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = oa_json_opt_object(root_obj, "paths", &paths_obj);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = oa_json_opt_object(root_obj, "webhooks", &webhooks_obj);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  if (out->swagger_version) {
    comps_obj = root_obj;
  } else {
    rc = oa_json_opt_object(root_obj, "components", &comps_obj);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (paths_obj) {
    rc = collect_extensions(paths_obj, &out->paths_extensions_json);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (webhooks_obj) {
    rc = collect_extensions(webhooks_obj, &out->webhooks_extensions_json);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (comps_obj) {
    rc = collect_extensions(comps_obj, &out->components_extensions_json);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (!paths_obj && !webhooks_obj && !comps_obj) {
    rc = CDD_C_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  if (comps_obj) {
    rc = ensure_oauth2_default_schemas(comps_obj);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = parse_components(comps_obj, out);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  if (paths_obj) {
    rc = parse_paths_object(paths_obj, &out->paths, &out->n_paths, out, 1, 1);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = validate_path_templates(out->paths, out->n_paths);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = validate_path_template_collisions(out->paths, out->n_paths);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = validate_querystring_usage(out->paths, out->n_paths);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc =
        validate_querystring_usage_in_paths_callbacks(out->paths, out->n_paths);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  if (webhooks_obj) {
    rc = parse_paths_object(webhooks_obj, &out->webhooks, &out->n_webhooks, out,
                            0, 1);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = validate_querystring_usage(out->webhooks, out->n_webhooks);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = validate_querystring_usage_in_paths_callbacks(out->webhooks,
                                                       out->n_webhooks);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  if (out->n_component_path_items > 0) {
    rc = validate_querystring_usage(out->component_path_items,
                                    out->n_component_path_items);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = validate_querystring_usage_in_paths_callbacks(
        out->component_path_items, out->n_component_path_items);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  rc = validate_querystring_usage_in_component_callbacks(out);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = validate_unique_operation_ids(out);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  if (registry) {
    rc = openapi_doc_registry_add(registry, out);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  return CDD_C_SUCCESS;

cleanup: {
  cdd_c_error_t rc_f = openapi_spec_free(out);
  return (cdd_c_error_t)((int)rc | (int)rc_f);
}
}

/*
 * @brief Parse an OpenAPI or Schema document from a JSON Value.
 *
 * @param[in] root The root JSON Value of the OpenAPI or Schema document.
 * @param[out] out Destination structure to populate.
 * @return 0 on success, error code (EINVAL/ENOMEM) on failure.
 */
cdd_c_error_t openapi_load_from_json(const JSON_Value *root,
                                     struct OpenAPI_Spec *out) {
  return openapi_load_from_json_internal(root, out, NULL, NULL);
}

/*
 * @brief Parse a JSON Value with document context for multi-doc resolution.
 *
 * @param[in] root The root JSON Value of the OpenAPI file.
 * @param[in] retrieval_uri Retrieval/base URI for this document (optional).
 * @param[out] out The spec structure to populate (must be initialized).
 * @param[in,out] registry Optional registry for multi-doc resolution.
 * @return 0 on success, EINVAL on invalid input, ENOMEM on allocation failure.
 */
cdd_c_error_t openapi_load_from_json_with_context(
    const JSON_Value *root, const char *retrieval_uri, struct OpenAPI_Spec *out,
    struct OpenAPI_DocRegistry *registry) {
  return openapi_load_from_json_internal(root, out, retrieval_uri, registry);
}

/*
 * @param[out] _out_val Pointer to store the result
 * @brief Look up a schema definition by name in the loaded spec.
 *
 * @param[in] spec The spec to search.
 * @param[in] name The schema name (e.g. "LoginRequest").
 * @return Pointer to StructFields if found, NULL otherwise.
 */
cdd_c_error_t openapi_spec_find_schema(const struct OpenAPI_Spec *spec,
                                       const char *name,
                                       struct StructFields **_out_val) {
  size_t i;
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!spec || !name || !spec->defined_schema_names) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < spec->n_defined_schemas; ++i) {
    if (spec->defined_schema_names[i] &&
        strcmp(spec->defined_schema_names[i], name) == 0) {
      *_out_val = &spec->defined_schemas[i];
      return CDD_C_SUCCESS;
    }
  }
  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/*
 * @brief Looks up a schema in an OpenAPI_Spec by its JSON Schema $id URI.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The $id reference string.
 * @param[out] _out_val Output pointer to the found StructFields.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t openapi_spec_find_schema_by_id(const struct OpenAPI_Spec *spec,
                                             const char *ref,
                                             struct StructFields **_out_val) {
  size_t i;
  const char *hash;
  size_t base_len;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_find_schema_by_id && --g_cdd_fail_find_schema_by_id == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  if (!spec || !ref || !spec->defined_schema_ids) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  hash = strchr(ref, '#');
  if (hash && hash[1] != '\0') {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  base_len = hash ? (size_t)(hash - ref) : strlen(ref);
  if (base_len == 0) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < spec->n_defined_schemas; ++i) {
    const char *id = spec->defined_schema_ids[i];
    if (!id)
      continue;
    if (strlen(id) == base_len && strncmp(id, ref, base_len) == 0) {
      *_out_val = &spec->defined_schemas[i];
      return CDD_C_SUCCESS;
    }
  }

  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/*
 * @brief Looks up a schema in an OpenAPI_Spec by its JSON Schema $anchor.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The anchor reference string.
 * @param[in] dynamic_anchor Non-zero to search $dynamicAnchor instead of
 * $anchor.
 * @param[out] _out_val Output pointer to the found StructFields.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t
openapi_spec_find_schema_by_anchor(const struct OpenAPI_Spec *spec,
                                   const char *ref, int dynamic_anchor,
                                   struct StructFields **_out_val) {
  size_t i;
  const char *hash;
  const char *anchor;
  char **anchors;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_find_schema_by_anchor &&
      --g_cdd_fail_find_schema_by_anchor == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  if (!spec || !ref) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  hash = strchr(ref, '#');
  if (!hash || hash[1] == '\0') {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  anchor = hash + 1;
  if (anchor[0] == '/') {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  anchors = dynamic_anchor ? spec->defined_schema_dynamic_anchors
                           : spec->defined_schema_anchors;
  if (!anchors) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < spec->n_defined_schemas; ++i) {
    const char *cand = anchors[i];
    if (!cand)
      continue;
    if (strcmp(cand, anchor) == 0) {
      *_out_val = &spec->defined_schemas[i];
      return CDD_C_SUCCESS;
    }
  }

  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/*
 * @param[out] _out_val Pointer to store the result
 * @brief Find a schema definition by following a SchemaRef.
 *
 * Uses the document registry when present to resolve external component refs.
 *
 * @param[in] spec The current specification.
 * @param[in] ref The schema reference.
 * @return Pointer to StructFields if found, NULL otherwise.
 */
cdd_c_error_t
openapi_spec_find_schema_for_ref(const struct OpenAPI_Spec *spec,
                                 const struct OpenAPI_SchemaRef *ref,
                                 struct StructFields **_out_val) {
  struct ResolvedRefTarget resolved;
  const struct OpenAPI_Spec *target;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!spec || !ref) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  if (ref->ref_name) {
    if (ref->ref) {
      rc = resolve_ref_target(spec, ref->ref, &resolved);
      if (rc != CDD_C_SUCCESS) {
        *_out_val = NULL;
        return rc;
      }
      target = resolved.spec;
      if (resolved.resolved_ref)
        C_CDD_FREE(resolved.resolved_ref);
    } else {
      target = spec;
    }
    return openapi_spec_find_schema(target, ref->ref_name, _out_val);
  }

  if (ref->ref) {
    struct StructFields *found = NULL;
    rc = resolve_ref_target(spec, ref->ref, &resolved);
    if (rc != CDD_C_SUCCESS) {
      *_out_val = NULL;
      return rc;
    }
    target = resolved.spec;
    if (ref->ref_is_dynamic) {
      rc = openapi_spec_find_schema_by_anchor(target, resolved.ref, 1, &found);
      if (rc != CDD_C_SUCCESS) {
        if (resolved.resolved_ref)
          C_CDD_FREE(resolved.resolved_ref);
        *_out_val = NULL;
        return rc;
      }
      if (!found) {
        rc =
            openapi_spec_find_schema_by_anchor(target, resolved.ref, 0, &found);
        if (rc != CDD_C_SUCCESS) {
          if (resolved.resolved_ref)
            C_CDD_FREE(resolved.resolved_ref);
          *_out_val = NULL;
          return rc;
        }
      }
    } else {
      rc = openapi_spec_find_schema_by_anchor(target, resolved.ref, 0, &found);
      if (rc != CDD_C_SUCCESS) {
        if (resolved.resolved_ref)
          C_CDD_FREE(resolved.resolved_ref);
        *_out_val = NULL;
        return rc;
      }
      if (!found) {
        rc =
            openapi_spec_find_schema_by_anchor(target, resolved.ref, 1, &found);
        if (rc != CDD_C_SUCCESS) {
          if (resolved.resolved_ref)
            C_CDD_FREE(resolved.resolved_ref);
          *_out_val = NULL;
          return rc;
        }
      }
    }
    if (!found) {
      rc = openapi_spec_find_schema_by_id(target, resolved.ref, &found);
      if (rc != CDD_C_SUCCESS) {
        if (resolved.resolved_ref)
          C_CDD_FREE(resolved.resolved_ref);
        *_out_val = NULL;
        return rc;
      }
    }
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = found;
    return CDD_C_SUCCESS;
  }

  *_out_val = NULL;
  return CDD_C_SUCCESS;
}
