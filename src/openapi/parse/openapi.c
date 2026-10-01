/**
 * @file openapi.c
 * @brief OpenAPI specification top-level loader and schema lookup.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Executes the openapi load from json internal operation.
 */
cdd_c_error_t openapi_load_from_json_internal(
    const JSON_Value *root, struct OpenAPI_Spec *out, const char *retrieval_uri,
    struct OpenAPI_DocRegistry *registry) {
  const JSON_Object *root_obj;
  const JSON_Object *paths_obj;
  const JSON_Object *webhooks_obj;
  const JSON_Object *comps_obj;
  cdd_c_error_t rc;

  if (!root || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  root_obj = json_value_get_object(root);
  if (!root_obj && json_value_get_type(root) != JSONBoolean)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  out->doc_registry = registry;
  if (retrieval_uri && *retrieval_uri) {
    rc = c_cdd_strdup(retrieval_uri, &out->retrieval_uri);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  {
    const char *version =
        root_obj ? json_object_get_string(root_obj, "openapi") : NULL;
    const char *swagger_version =
        root_obj ? json_object_get_string(root_obj, "swagger") : NULL;
    if (!version && !swagger_version) {
      if (!root_is_schema_document(root, root_obj))
        return CDD_C_ERROR_INVALID_ARGUMENT;
      out->is_schema_document = 1;
      {
        const char *schema_id =
            root_obj ? json_object_get_string(root_obj, "$id") : NULL;
        if ((schema_id && *schema_id) ||
            (out->retrieval_uri && *out->retrieval_uri)) {
          rc = compute_document_uri(schema_id, out->retrieval_uri,
                                    &out->document_uri);
          if (rc != CDD_C_SUCCESS)
            return CDD_C_ERROR_MEMORY;
        }
      }
      rc = store_schema_root_json(out, root);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(out);
        return rc;
      }
      if (registry) {
        rc = openapi_doc_registry_add(registry, out);
        if (rc != CDD_C_SUCCESS) {
          openapi_spec_free(out);
          return rc;
        }
      }
      return CDD_C_SUCCESS;
    }
    if (version) {
      if (!openapi_version_supported(version))
        return CDD_C_ERROR_INVALID_ARGUMENT;
      rc = c_cdd_strdup(version, &out->openapi_version);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (swagger_version) {
      rc = c_cdd_strdup(swagger_version, &out->swagger_version);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }

    if (out->swagger_version) {
      const char *host;
      const char *basePath;
      JSON_Array *schemes_arr;
      JSON_Array *consumes_arr;
      JSON_Array *produces_arr;
      size_t i;

      host = json_object_get_string(root_obj, "host");
      if (host) {
        rc = c_cdd_strdup(host, &out->host);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      basePath = json_object_get_string(root_obj, "basePath");
      if (basePath) {
        rc = c_cdd_strdup(basePath, &out->basePath);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }

      schemes_arr = json_object_get_array(root_obj, "schemes");
      if (schemes_arr) {
        out->n_schemes = json_array_get_count(schemes_arr);
        out->schemes = (char **)c_cdd_calloc(out->n_schemes, sizeof(char *));
        if (!out->schemes)
          return CDD_C_ERROR_MEMORY;
        for (i = 0; i < out->n_schemes; i++) {
          rc = c_cdd_strdup(json_array_get_string(schemes_arr, i),
                            &out->schemes[i]);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
      consumes_arr = json_object_get_array(root_obj, "consumes");
      if (consumes_arr) {
        out->n_consumes = json_array_get_count(consumes_arr);
        out->consumes = (char **)c_cdd_calloc(out->n_consumes, sizeof(char *));
        if (!out->consumes)
          return CDD_C_ERROR_MEMORY;
        for (i = 0; i < out->n_consumes; i++) {
          rc = c_cdd_strdup(json_array_get_string(consumes_arr, i),
                            &out->consumes[i]);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
      produces_arr = json_object_get_array(root_obj, "produces");
      if (produces_arr) {
        out->n_produces = json_array_get_count(produces_arr);
        out->produces = (char **)c_cdd_calloc(out->n_produces, sizeof(char *));
        if (!out->produces)
          return CDD_C_ERROR_MEMORY;
        for (i = 0; i < out->n_produces; i++) {
          rc = c_cdd_strdup(json_array_get_string(produces_arr, i),
                            &out->produces[i]);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }

    {
      const char *self_uri = json_object_get_string(root_obj, "$self");
      if (self_uri) {
        rc = c_cdd_strdup(self_uri, &out->self_uri);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    if (out->self_uri || out->retrieval_uri) {
      rc = compute_document_uri(out->self_uri, out->retrieval_uri,
                                &out->document_uri);
      if (rc != CDD_C_SUCCESS)
        return CDD_C_ERROR_MEMORY;
    }
    {
      const char *dialect =
          json_object_get_string(root_obj, "jsonSchemaDialect");
      if (dialect) {
        rc = c_cdd_strdup(dialect, &out->json_schema_dialect);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    rc = collect_extensions(root_obj, &out->extensions_json);
    if (rc != CDD_C_SUCCESS)
      return rc;

    rc = parse_info(root_obj, out);
    if (rc != CDD_C_SUCCESS) {
      openapi_spec_free(out);
      return rc;
    }
    /* info check removed */
    {
      const JSON_Object *ext_docs =
          json_object_get_object(root_obj, "externalDocs");
      if (ext_docs) {
        cdd_c_error_t rc_ext =
            parse_external_docs(ext_docs, &out->external_docs);
        if (rc_ext != 0) {
          openapi_spec_free(out);
          return rc_ext;
        }
      }
    }
    rc = parse_tags(root_obj, out);
    if (rc != CDD_C_SUCCESS) {
      openapi_spec_free(out);
      return rc;
    }
    rc = validate_tag_parents(out);
    if (rc != CDD_C_SUCCESS) {
      openapi_spec_free(out);
      return rc;
    }

    rc = parse_security_field(root_obj, "security", &out->security,
                              &out->n_security, &out->security_set);
    if (rc != CDD_C_SUCCESS) {
      openapi_spec_free(out);
      return rc;
    }

    rc = parse_servers(root_obj, out);
    if (rc != CDD_C_SUCCESS) {
      openapi_spec_free(out);
      return rc;
    }

    paths_obj = json_object_get_object(root_obj, "paths");
    webhooks_obj = json_object_get_object(root_obj, "webhooks");
    comps_obj = out->swagger_version
                    ? root_obj
                    : json_object_get_object(root_obj, "components");
    if (paths_obj) {
      {
        cdd_c_error_t _rc =
            collect_extensions(paths_obj, &out->paths_extensions_json);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
    if (webhooks_obj) {
      {
        cdd_c_error_t _rc =
            collect_extensions(webhooks_obj, &out->webhooks_extensions_json);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
    if (comps_obj) {
      {
        cdd_c_error_t _rc =
            collect_extensions(comps_obj, &out->components_extensions_json);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
    if (!paths_obj && !webhooks_obj && !comps_obj) {
      openapi_spec_free(out);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }

    /* Load Schemas First */
    if (comps_obj) {
      int has_oauth2 = 0;
      const JSON_Object *schemes =
          json_object_get_object(comps_obj, "securitySchemes");
      if (schemes) {
        size_t scount = json_object_get_count(schemes);
        size_t sk;
        for (sk = 0; sk < scount; ++sk) {
          const JSON_Object *sec_obj =
              json_value_get_object(json_object_get_value_at(schemes, sk));
          const char *type = json_object_get_string(sec_obj, "type");
          if (type && strcmp(type, "oauth2") == 0) {
            has_oauth2 = 1;
            break;
          }
        }
      }
      if (has_oauth2) {
        JSON_Object *mut_comps = (JSON_Object *)comps_obj;
        JSON_Object *schemas = json_object_get_object(mut_comps, "schemas");
        if (!schemas) {
          JSON_Value *schemas_val;
#ifdef CDD_BUILD_TESTS
          if (g_cdd_alloc_fail && --g_cdd_alloc_fail == 0)
            schemas_val = NULL;
          else
#endif
            schemas_val = json_value_init_object();
          if (!schemas_val)
            return CDD_C_ERROR_MEMORY;
          json_object_set_value(mut_comps, "schemas", schemas_val);
          schemas = json_object_get_object(mut_comps, "schemas");
        }
        if (!json_object_has_value(schemas, "OAuth2TokenRequest")) {
          json_object_set_value(
              schemas, "OAuth2TokenRequest",
              json_parse_string("{\"type\":\"object\",\"properties\":{"
                                "\"grant_type\":{\"type\":"
                                "\"string\",\"enum\":[\"password\","
                                "\"authorization_code\","
                                "\"client_credentials\",\"refresh_"
                                "token\"]},\"username\":{"
                                "\"type\":\"string\"},\"password\":{"
                                "\"type\":\"string\",\"writeOnly\":true},"
                                "\"scope\":{\"type\":"
                                "\"string\"},\"client_"
                                "id\":{\"type\":\"string\"},\"client_"
                                "secret\":{\"type\":"
                                "\"string\",\"writeOnly\":true},"
                                "\"refresh_token\":{\"type\":"
                                "\"string\"},\"code\":{"
                                "\"type\":\"string\"},\"redirect_uri\":{"
                                "\"type\":\"string\"}},"
                                "\"required\":[\"grant_type\"]}"));
        }
        if (!json_object_has_value(schemas, "OAuth2TokenResponse")) {
          json_object_set_value(
              schemas, "OAuth2TokenResponse",
              json_parse_string(
                  "{\"type\":\"object\",\"properties\":{\"access_token\":"
                  "{"
                  "\"type\":\"string\"},\"token_type\":{\"type\":"
                  "\"string\"},"
                  "\"expires_in\":{\"type\":\"integer\"},\"refresh_"
                  "token\":{"
                  "\"type\":\"string\"},\"scope\":{\"type\":\"string\"}},"
                  "\"required\":[\"access_token\",\"token_type\"]}"));
        }
        if (!json_object_has_value(schemas, "OAuth2Client")) {
          json_object_set_value(
              schemas, "OAuth2Client",
              json_parse_string(
                  "{\"type\":\"object\",\"properties\":{\"client_id\":{"
                  "\"type\":"
                  "\"string\",\"description\":\"[UNIQUE]\"},\"client_"
                  "secret_"
                  "hash\":{\"type\":\"string\",\"writeOnly\":true},"
                  "\"redirect_uris\":{\"type\":\"array\",\"items\":{"
                  "\"type\":"
                  "\"string\"}},\"grant_types\":{\"type\":\"array\","
                  "\"items\":{"
                  "\"type\":\"string\"}}},\"required\":[\"client_id\","
                  "\"client_"
                  "secret_hash\"]"
                  "}"));
        }
        if (!json_object_has_value(schemas, "OAuth2User")) {
          json_object_set_value(
              schemas, "OAuth2User",
              json_parse_string(
                  "{\"type\":\"object\",\"properties\":{\"username\":{"
                  "\"type\":"
                  "\"string\",\"description\":\"[UNIQUE]\"},\"password_"
                  "hash\":{"
                  "\"type\":\"string\",\"writeOnly\":true},"
                  "\"created_at\":{\"type\":\"integer\",\"format\":"
                  "\"int64\"}},"
                  "\"required\":[\"username\",\"password_hash\"]"
                  "}"));
        }
        if (!json_object_has_value(schemas, "OAuth2Token")) {
          json_object_set_value(
              schemas, "OAuth2Token",
              json_parse_string(
                  "{\"type\":\"object\",\"properties\":{\"access_token\":"
                  "{"
                  "\"type\":"
                  "\"string\",\"description\":\"[UNIQUE]\"},\"refresh_"
                  "token\":{"
                  "\"type\":\"string\",\"description\":\"[UNIQUE]\"},"
                  "\"expires_at\":{\"type\":\"integer\",\"format\":"
                  "\"int64\"},"
                  "\"client_id\":{\"type\":\"integer\",\"description\":"
                  "\"[FK="
                  "OAuth2Client]\"},\"user_id\":{\"type\":\"integer\","
                  "\"description\":\"[FK=OAuth2User]\"}},\"required\":["
                  "\"access_"
                  "token\"]"
                  "}"));
        }
      }
      rc = parse_components(comps_obj, out);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(out);
        return rc;
      }
    }

    if (paths_obj) {
      rc = parse_paths_object(paths_obj, &out->paths, &out->n_paths, out, 1, 1);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(out);
        return rc;
      }
      rc = validate_path_templates(out->paths, out->n_paths);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(out);
        return rc;
      }
      rc = validate_path_template_collisions(out->paths, out->n_paths);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(out);
        return rc;
      }
      rc = validate_querystring_usage(out->paths, out->n_paths);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(out);
        return rc;
      }
      rc = validate_querystring_usage_in_paths_callbacks(out->paths,
                                                         out->n_paths);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(out);
        return rc;
      }
    }

    if (webhooks_obj) {
      rc = parse_paths_object(webhooks_obj, &out->webhooks, &out->n_webhooks,
                              out, 0, 1);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(out);
        return rc;
      }
      rc = validate_querystring_usage(out->webhooks, out->n_webhooks);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(out);
        return rc;
      }
      rc = validate_querystring_usage_in_paths_callbacks(out->webhooks,
                                                         out->n_webhooks);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(out);
        return rc;
      }
    }

    if (out->n_component_path_items > 0) {
      rc = validate_querystring_usage(out->component_path_items,
                                      out->n_component_path_items);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(out);
        return rc;
      }
      rc = validate_querystring_usage_in_paths_callbacks(
          out->component_path_items, out->n_component_path_items);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(out);
        return rc;
      }
    }

    rc = validate_querystring_usage_in_component_callbacks(out);
    if (rc != CDD_C_SUCCESS) {
      openapi_spec_free(out);
      return rc;
    }

    rc = validate_unique_operation_ids(out);
    if (rc != CDD_C_SUCCESS) {
      openapi_spec_free(out);
      return rc;
    }

    if (registry) {
      rc = openapi_doc_registry_add(registry, out);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(out);
        return rc;
      }
    }

    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the openapi load from json operation.
 */
cdd_c_error_t openapi_load_from_json(const JSON_Value *root,
                                     struct OpenAPI_Spec *out) {
  return openapi_load_from_json_internal(root, out, NULL, NULL);
}

/**
 * @brief Executes the openapi load from json with context operation.
 */
cdd_c_error_t openapi_load_from_json_with_context(
    const JSON_Value *root, const char *retrieval_uri, struct OpenAPI_Spec *out,
    struct OpenAPI_DocRegistry *registry) {
  return openapi_load_from_json_internal(root, out, retrieval_uri, registry);
}

/**
 * @brief Executes the openapi spec find schema operation.
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
      {
        *_out_val = &spec->defined_schemas[i];
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
 * @brief Executes the openapi spec find schema by id operation.
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
    /* Fragmented refs may target subschemas; do not map to root
     * structs. */
    {
      *_out_val = NULL;
      return CDD_C_SUCCESS;
    }
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
      {
        *_out_val = &spec->defined_schemas[i];
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
 * @brief Executes the openapi spec find schema by anchor operation.
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

  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the openapi spec find schema for ref operation.
 */
cdd_c_error_t
openapi_spec_find_schema_for_ref(const struct OpenAPI_Spec *spec,
                                 const struct OpenAPI_SchemaRef *ref,
                                 struct StructFields **_out_val) {
  struct ResolvedRefTarget resolved;
  const struct OpenAPI_Spec *target;
  cdd_c_error_t rc;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!spec || !ref) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  if (ref->all_of) {
    size_t i;
    for (i = 0; i < ref->n_all_of; ++i)
      free_schema_ref_content(&ref->all_of[i]);
    free(ref->all_of);
  }
  if (ref->any_of) {
    size_t i;
    for (i = 0; i < ref->n_any_of; ++i)
      free_schema_ref_content(&ref->any_of[i]);
    free(ref->any_of);
  }
  if (ref->one_of) {
    size_t i;
    for (i = 0; i < ref->n_one_of; ++i)
      free_schema_ref_content(&ref->one_of[i]);
    free(ref->one_of);
  }
  if (ref->not_schema) {
    free_schema_ref_content(ref->not_schema);
    free(ref->not_schema);
  }
  if (ref->if_schema) {
    free_schema_ref_content(ref->if_schema);
    free(ref->if_schema);
  }
  if (ref->then_schema) {
    free_schema_ref_content(ref->then_schema);
    free(ref->then_schema);
  }
  if (ref->else_schema) {
    free_schema_ref_content(ref->else_schema);
    free(ref->else_schema);
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
        free(resolved.resolved_ref);
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
          free(resolved.resolved_ref);
        *_out_val = NULL;
        return rc;
      }
      if (!found) {
        rc =
            openapi_spec_find_schema_by_anchor(target, resolved.ref, 0, &found);
        if (rc != CDD_C_SUCCESS) {
          if (resolved.resolved_ref)
            free(resolved.resolved_ref);
          *_out_val = NULL;
          return rc;
        }
      }
    } else {
      rc = openapi_spec_find_schema_by_anchor(target, resolved.ref, 0, &found);
      if (rc != CDD_C_SUCCESS) {
        if (resolved.resolved_ref)
          free(resolved.resolved_ref);
        *_out_val = NULL;
        return rc;
      }
      if (!found) {
        rc =
            openapi_spec_find_schema_by_anchor(target, resolved.ref, 1, &found);
        if (rc != CDD_C_SUCCESS) {
          if (resolved.resolved_ref)
            free(resolved.resolved_ref);
          *_out_val = NULL;
          return rc;
        }
      }
    }
    if (!found) {
      rc = openapi_spec_find_schema_by_id(target, resolved.ref, &found);
      if (rc != CDD_C_SUCCESS) {
        if (resolved.resolved_ref)
          free(resolved.resolved_ref);
        *_out_val = NULL;
        return rc;
      }
    }
    if (resolved.resolved_ref)
      free(resolved.resolved_ref);
    *_out_val = found;
    return CDD_C_SUCCESS;
  }

  *_out_val = NULL;
  return CDD_C_SUCCESS;
}
