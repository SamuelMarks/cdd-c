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
  cdd_c_error_t rc;

  if (!components || !out)
    return CDD_C_SUCCESS;

  rc = oa_json_opt_object(components,
                          out->swagger_version ? "securityDefinitions"
                                               : "securitySchemes",
                          &schemes);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!schemes)
    return CDD_C_SUCCESS;

  rc = validate_component_key_map(schemes);
  if (rc != CDD_C_SUCCESS)
    return rc;

  count = schemes->count;
  if (count == 0)
    return CDD_C_SUCCESS;

  out->security_schemes = (struct OpenAPI_SecurityScheme *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_SecurityScheme));
  if (!out->security_schemes)
    return CDD_C_ERROR_MEMORY;
  out->n_security_schemes = count;

  for (i = 0; i < count; ++i) {
    const char *name = schemes->names[i];
    JSON_Value *val_at = schemes->values[i];
    JSON_Object *sec_obj =
        (val_at->type == JSONObject) ? val_at->object_val : NULL;
    int has_dep = 0;
    int dep_val = 0;
    rc = c_cdd_strdup(name, &out->security_schemes[i].name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    out->security_schemes[i].type = OA_SEC_UNKNOWN;

    if (!sec_obj)
      continue;

    {
      const char *type = NULL;
      rc = oa_json_opt_string(sec_obj, "type", &type);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (type && strcmp(type, "basic") == 0) {
        out->security_schemes[i].type = OA_SEC_HTTP;
        rc = c_cdd_strdup("basic", &out->security_schemes[i].scheme);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else {
        rc = parse_security_type(type, &out->security_schemes[i].type);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (out->security_schemes[i].type == OA_SEC_UNKNOWN)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }

    rc = openapi_parse_reflected_strings(
        sec_obj, &OpenAPI_SecurityScheme_type_info, &out->security_schemes[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = oa_json_opt_bool(sec_obj, "deprecated", &has_dep, &dep_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (has_dep) {
      out->security_schemes[i].deprecated_set = 1;
      out->security_schemes[i].deprecated = dep_val;
    }
    rc = collect_extensions(sec_obj, &out->security_schemes[i].extensions_json);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (out->security_schemes[i].type == OA_SEC_APIKEY) {
      const char *in = NULL;
      const char *key_name = NULL;
      rc = oa_json_opt_string(sec_obj, "in", &in);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = oa_json_opt_string(sec_obj, "name", &key_name);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = parse_security_in(in, &out->security_schemes[i].in);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (!key_name || !*key_name ||
          out->security_schemes[i].in == OA_SEC_IN_UNKNOWN)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      rc = c_cdd_strdup(key_name, &out->security_schemes[i].key_name);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (out->security_schemes[i].type == OA_SEC_HTTP) {
      if (!out->security_schemes[i].scheme) {
        const char *scheme = NULL;
        rc = oa_json_opt_string(sec_obj, "scheme", &scheme);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (!scheme || !*scheme)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        rc = c_cdd_strdup(scheme, &out->security_schemes[i].scheme);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (out->security_schemes[i].type == OA_SEC_OPENID) {
      if (!out->security_schemes[i].open_id_connect_url ||
          !*out->security_schemes[i].open_id_connect_url)
        return CDD_C_ERROR_INVALID_ARGUMENT;
    } else if (out->security_schemes[i].type == OA_SEC_OAUTH2) {
      cdd_c_error_t flow_rc;
      JSON_Object *flows_obj = NULL;
      rc = oa_json_opt_object(sec_obj, "flows", &flows_obj);
      if (rc != CDD_C_SUCCESS)
        return rc;
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
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, openapi_spec_free(out));
  }
  return CDD_C_SUCCESS;
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

  count = arr->count;
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
    JSON_Object *sec_obj =
        (arr->items[i]->type == JSONObject) ? arr->items[i]->object_val : NULL;
    struct OpenAPI_SecurityRequirementSet *set = &(*out)[i];
    size_t j, req_count = 0;
    size_t key_count = 0;

    if (sec_obj) {
      key_count = sec_obj->count;
      for (j = 0; j < key_count; ++j) {
        const char *name = sec_obj->names[j];
        if (strncmp(name, "x-", 2) != 0)
          req_count++;
      }
      rc = collect_extensions(sec_obj, &set->extensions_json);
      if (rc != CDD_C_SUCCESS)
        goto fail;
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
        const char *scheme = sec_obj->names[j];
        JSON_Array *scopes_arr = NULL;
        struct OpenAPI_SecurityRequirement *req;
        if (strncmp(scheme, "x-", 2) == 0)
          continue;
        scopes_arr = (sec_obj->values[j]->type == JSONArray)
                         ? sec_obj->values[j]->array_val
                         : NULL;
        req = &set->requirements[req_idx++];

        rc = c_cdd_strdup(scheme, &req->scheme);
        if (rc != CDD_C_SUCCESS)
          goto fail;

        if (scopes_arr)
          n_scopes = scopes_arr->count;

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
          const char *scope = (scopes_arr->items[k]->type == JSONString)
                                  ? scopes_arr->items[k]->string_val
                                  : NULL;
          rc = c_cdd_strdup(scope ? scope : "", &req->scopes[k]);
          if (rc != CDD_C_SUCCESS)
            goto fail;
        }
      }
    }
  }

  return CDD_C_SUCCESS;

fail: {
  cdd_c_error_t f_rc = CDD_C_SUCCESS;
  for (i = 0; i < count; ++i) {
    f_rc =
        (cdd_c_error_t)((int)f_rc |
                        (int)openapi_security_requirement_set_free(&(*out)[i]));
  }
  C_CDD_FREE(*out);
  *out = NULL;
  *out_count = 0;
  return cdd_c_keep_err(rc, f_rc);
}
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
  size_t k;
  cdd_c_error_t rc;
  if (!obj || !key || !out || !out_count || !out_set)
    return CDD_C_SUCCESS;

  for (k = 0; k < obj->count; ++k) {
    if (strcmp(obj->names[k], key) == 0) {
      has_val = 1;
      break;
    }
  }
  if (!has_val) {
    *out_set = 0;
    return CDD_C_SUCCESS;
  }

  *out_set = 1;
  rc = oa_json_opt_array(obj, "security", &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!arr) {
    *out = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }

  return parse_security_requirements(arr, out, out_count);
}
