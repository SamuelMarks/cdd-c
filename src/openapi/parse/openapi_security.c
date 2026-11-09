/**
 * @file openapi_security.c
 * @brief Security schemes and security requirements parsing.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Parses servers from the given input.
 *
 * @param[in] root_obj The root OpenAPI JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_servers(const JSON_Object *root_obj,
                            struct OpenAPI_Spec *out) {
  return parse_servers_array(root_obj, "servers", &out->servers,
                             &out->n_servers);
}

/**
 * @brief Internal helper to parse securitySchemes or securityDefinitions.
 *
 * @param[in] components The components or root JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t parse_security_schemes_inner(const JSON_Object *components,
                                                  struct OpenAPI_Spec *out) {
  JSON_Object *schemes = NULL;
  size_t count = 0, i;

  if (!components || !out)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_object_get_object(
      components,
      out->swagger_version ? "securityDefinitions" : "securitySchemes",
      &schemes);
  if (!schemes)
    return CDD_C_SUCCESS;

  if (validate_component_key_map(schemes) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  g_cdd_last_alloc_err = cdd_json_object_get_count(schemes, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  out->security_schemes = (struct OpenAPI_SecurityScheme *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_SecurityScheme));
  if (!out->security_schemes)
    return CDD_C_ERROR_MEMORY;
  out->n_security_schemes = count;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *sec_obj = NULL;
    int has_dep = 0;
    int dep_val = -1;
    cdd_c_error_t rc;
    g_cdd_last_alloc_err = cdd_json_object_get_name(schemes, i, &name);
    g_cdd_last_alloc_err = cdd_json_object_get_value_at(schemes, i, &val_at);
    g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &sec_obj);
    g_cdd_last_alloc_err = c_cdd_strdup(name, &out->security_schemes[i].name);
    if (!out->security_schemes[i].name)
      return CDD_C_ERROR_MEMORY;
    out->security_schemes[i].type = OA_SEC_UNKNOWN;

    if (!sec_obj)
      continue;

    {
      const char *type = NULL;
      g_cdd_last_alloc_err = cdd_json_object_get_string(sec_obj, "type", &type);
      if (type && strcmp(type, "basic") == 0) {
        out->security_schemes[i].type = OA_SEC_HTTP;
        g_cdd_last_alloc_err =
            c_cdd_strdup("basic", &out->security_schemes[i].scheme);
        if (!out->security_schemes[i].scheme)
          return CDD_C_ERROR_MEMORY;
      } else {
        g_cdd_last_alloc_err =
            parse_security_type(type, &out->security_schemes[i].type);
        if (out->security_schemes[i].type == OA_SEC_UNKNOWN)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }

    {
      const char *desc = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(sec_obj, "description", &desc);
      if (desc) {
        g_cdd_last_alloc_err =
            c_cdd_strdup(desc, &out->security_schemes[i].description);
        if (!out->security_schemes[i].description)
          return CDD_C_ERROR_MEMORY;
      }
    }
    g_cdd_last_alloc_err =
        cdd_json_object_has_value(sec_obj, "deprecated", &has_dep);
    if (has_dep) {
      out->security_schemes[i].deprecated_set = 1;
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(sec_obj, "deprecated", &dep_val);
      out->security_schemes[i].deprecated = (dep_val == 1);
    }
    rc = collect_extensions(sec_obj, &out->security_schemes[i].extensions_json);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (out->security_schemes[i].type == OA_SEC_APIKEY) {
      const char *in = NULL;
      const char *key_name = NULL;
      g_cdd_last_alloc_err = cdd_json_object_get_string(sec_obj, "in", &in);
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(sec_obj, "name", &key_name);
      g_cdd_last_alloc_err =
          parse_security_in(in, &out->security_schemes[i].in);
      if (!key_name || !*key_name ||
          out->security_schemes[i].in == OA_SEC_IN_UNKNOWN)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      g_cdd_last_alloc_err =
          c_cdd_strdup(key_name, &out->security_schemes[i].key_name);
      if (!out->security_schemes[i].key_name)
        return CDD_C_ERROR_MEMORY;
    } else if (out->security_schemes[i].type == OA_SEC_HTTP) {
      if (!out->security_schemes[i].scheme) {
        const char *scheme = NULL;
        const char *bearer_format = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(sec_obj, "scheme", &scheme);
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(sec_obj, "bearerFormat", &bearer_format);
        if (!scheme || !*scheme)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        g_cdd_last_alloc_err =
            c_cdd_strdup(scheme, &out->security_schemes[i].scheme);
        if (!out->security_schemes[i].scheme)
          return CDD_C_ERROR_MEMORY;
        if (bearer_format) {
          g_cdd_last_alloc_err = c_cdd_strdup(
              bearer_format, &out->security_schemes[i].bearer_format);
          if (!out->security_schemes[i].bearer_format)
            return CDD_C_ERROR_MEMORY;
        }
      }
    } else if (out->security_schemes[i].type == OA_SEC_OPENID) {
      const char *oid_url = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(sec_obj, "openIdConnectUrl", &oid_url);
      if (!oid_url || !*oid_url)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      g_cdd_last_alloc_err =
          c_cdd_strdup(oid_url, &out->security_schemes[i].open_id_connect_url);
      if (!out->security_schemes[i].open_id_connect_url)
        return CDD_C_ERROR_MEMORY;
    } else if (out->security_schemes[i].type == OA_SEC_OAUTH2) {
      cdd_c_error_t flow_rc;
      const char *meta_url = NULL;
      JSON_Object *flows_obj = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(sec_obj, "oauth2MetadataUrl", &meta_url);
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(sec_obj, "flows", &flows_obj);
      if (meta_url) {
        g_cdd_last_alloc_err = c_cdd_strdup(
            meta_url, &out->security_schemes[i].oauth2_metadata_url);
        if (!out->security_schemes[i].oauth2_metadata_url)
          return CDD_C_ERROR_MEMORY;
      }
      if (!flows_obj) {
        if (!out->swagger_version)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      flow_rc = parse_oauth_flows(flows_obj, &out->security_schemes[i]);
      if (flow_rc != CDD_C_SUCCESS)
        return flow_rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses security schemes from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_security_schemes(const JSON_Object *components,
                                     struct OpenAPI_Spec *out) {
  cdd_c_error_t rc = parse_security_schemes_inner(components, out);
  if (rc != CDD_C_SUCCESS)
    g_cdd_last_alloc_err = openapi_spec_free(out);
  return rc;
}

/**
 * @brief Parses security requirements from the given input.
 *
 * @param[in] arr The source security requirements JSON array.
 * @param[out] out Output array of OpenAPI_SecurityRequirementSet structures.
 * @param[out] out_count Output count of requirement sets.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t
parse_security_requirements(const JSON_Array *arr,
                            struct OpenAPI_SecurityRequirementSet **out,
                            size_t *out_count) {
  size_t i, count = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!arr || !out || !out_count)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_array_get_count(arr, &count);
  if (count == 0) {
    *out = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }

  *out = (struct OpenAPI_SecurityRequirementSet *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_SecurityRequirementSet));
  if (!*out)
    return CDD_C_ERROR_MEMORY;

  *out_count = count;

  for (i = 0; i < count; ++i) {
    JSON_Object *sec_obj = NULL;
    struct OpenAPI_SecurityRequirementSet *set = &(*out)[i];
    size_t j, req_count = 0;
    size_t key_count = 0;

    g_cdd_last_alloc_err = cdd_json_array_get_object(arr, i, &sec_obj);
    if (sec_obj) {
      g_cdd_last_alloc_err = cdd_json_object_get_count(sec_obj, &key_count);
      for (j = 0; j < key_count; ++j) {
        const char *name = NULL;
        g_cdd_last_alloc_err = cdd_json_object_get_name(sec_obj, j, &name);
        if (strncmp(name, "x-", 2) != 0)
          req_count++;
      }
      if (collect_extensions(sec_obj, &set->extensions_json) != 0) {
        rc = CDD_C_ERROR_MEMORY;
        goto fail;
      }
    }

    if (req_count == 0) {
      set->requirements = NULL;
      set->n_requirements = 0;
      continue;
    }

    set->requirements = (struct OpenAPI_SecurityRequirement *)C_CDD_CALLOC(
        req_count, sizeof(struct OpenAPI_SecurityRequirement));
    if (!set->requirements) {
      rc = CDD_C_ERROR_MEMORY;
      goto fail;
    }
    set->n_requirements = req_count;
    {
      size_t req_idx = 0;
      for (j = 0; j < key_count; ++j) {
        size_t k, n_scopes = 0;
        const char *scheme = NULL;
        JSON_Array *scopes_arr = NULL;
        struct OpenAPI_SecurityRequirement *req;
        g_cdd_last_alloc_err = cdd_json_object_get_name(sec_obj, j, &scheme);
        if (strncmp(scheme, "x-", 2) == 0)
          continue;
        g_cdd_last_alloc_err =
            cdd_json_object_get_array(sec_obj, scheme, &scopes_arr);
        req = &set->requirements[req_idx++];

        g_cdd_last_alloc_err = c_cdd_strdup(scheme, &req->scheme);
        if (!req->scheme) {
          rc = CDD_C_ERROR_MEMORY;
          goto fail;
        }

        if (scopes_arr)
          g_cdd_last_alloc_err =
              cdd_json_array_get_count(scopes_arr, &n_scopes);

        if (n_scopes == 0) {
          req->scopes = NULL;
          req->n_scopes = 0;
          continue;
        }

        req->scopes = (char **)C_CDD_CALLOC(n_scopes, sizeof(char *));
        if (!req->scopes) {
          rc = CDD_C_ERROR_MEMORY;
          goto fail;
        }
        req->n_scopes = n_scopes;

        for (k = 0; k < n_scopes; ++k) {
          const char *scope = NULL;
          g_cdd_last_alloc_err =
              cdd_json_array_get_string(scopes_arr, k, &scope);
          g_cdd_last_alloc_err =
              c_cdd_strdup(scope ? scope : "", &req->scopes[k]);
          if (!req->scopes[k]) {
            rc = CDD_C_ERROR_MEMORY;
            goto fail;
          }
        }
      }
    }
  }

  return CDD_C_SUCCESS;

fail:
  for (i = 0; i < count; ++i) {
    g_cdd_last_alloc_err = free_security_requirement_set(&(*out)[i]);
  }
  C_CDD_FREE(*out);
  *out = NULL;
  *out_count = 0;
  return rc;
}

/**
 * @brief Parses security field from the given input.
 *
 * @param[in] obj The parent JSON object.
 * @param[in] key The security property key name.
 * @param[out] out Output array of OpenAPI_SecurityRequirementSet structures.
 * @param[out] out_count Output count of requirement sets.
 * @param[out] out_set Output flag set to 1 if security field was present.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_security_field(const JSON_Object *obj, const char *key,
                                   struct OpenAPI_SecurityRequirementSet **out,
                                   size_t *out_count, int *out_set) {
  JSON_Array *arr = NULL;
  int has_val = 0;
  if (!obj || !key || !out || !out_count || !out_set)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_object_has_value(obj, key, &has_val);
  if (!has_val) {
    *out_set = 0;
    return CDD_C_SUCCESS;
  }

  *out_set = 1;
  g_cdd_last_alloc_err = cdd_json_object_get_array(obj, key, &arr);
  if (!arr) {
    *out = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }

  return parse_security_requirements(arr, out, out_count);
}
