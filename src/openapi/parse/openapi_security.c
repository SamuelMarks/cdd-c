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
 */
cdd_c_error_t parse_servers(const JSON_Object *root_obj,
                            struct OpenAPI_Spec *out) {
  return parse_servers_array(root_obj, "servers", &out->servers,
                             &out->n_servers);
}

/**
 * @brief Parses security schemes from the given input.
 */
cdd_c_error_t parse_security_schemes(const JSON_Object *components,
                                     struct OpenAPI_Spec *out) {
  enum OpenAPI_SecurityType _ast_parse_security_type_51;
  enum OpenAPI_SecurityIn _ast_parse_security_in_52;
  char *_ast_strdup_209 = NULL;
  char *_ast_strdup_210 = NULL;
  char *_ast_strdup_211 = NULL;
  char *_ast_strdup_212 = NULL;
  char *_ast_strdup_213 = NULL;
  char *_ast_strdup_214 = NULL;
  char *_ast_strdup_215 = NULL;
  const JSON_Object *schemes;
  size_t count, i;

  if (!components || !out)
    return CDD_C_SUCCESS;

  schemes = json_object_get_object(components, out->swagger_version
                                                   ? "securityDefinitions"
                                                   : "securitySchemes");
  if (!schemes)
    return CDD_C_SUCCESS;

  if (validate_component_key_map(schemes) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  count = json_object_get_count(schemes);
  if (count == 0)
    return CDD_C_SUCCESS;

  out->security_schemes = (struct OpenAPI_SecurityScheme *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_SecurityScheme));
  if (!out->security_schemes)
    return CDD_C_ERROR_MEMORY;
  out->n_security_schemes = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(schemes, i);
    const JSON_Object *sec_obj =
        json_value_get_object(json_object_get_value_at(schemes, i));
    out->security_schemes[i].name =
        (c_cdd_strdup(name, &_ast_strdup_209), _ast_strdup_209);
    if (!out->security_schemes[i].name)
      return CDD_C_ERROR_MEMORY;
    out->security_schemes[i].type = OA_SEC_UNKNOWN;

    if (!sec_obj)
      continue;

    {
      const char *type = json_object_get_string(sec_obj, "type");
      if (type && strcmp(type, "basic") == 0) {
        out->security_schemes[i].type = OA_SEC_HTTP;
        out->security_schemes[i].scheme =
            (c_cdd_strdup("basic", &_ast_strdup_210), _ast_strdup_210);
        if (!out->security_schemes[i].scheme)
          return CDD_C_ERROR_MEMORY;
      } else {
        out->security_schemes[i].type =
            (parse_security_type(type, &_ast_parse_security_type_51),
             _ast_parse_security_type_51);
        if (out->security_schemes[i].type == OA_SEC_UNKNOWN)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }

    {
      const char *desc = json_object_get_string(sec_obj, "description");
      if (desc) {
        out->security_schemes[i].description =
            (c_cdd_strdup(desc, &_ast_strdup_210), _ast_strdup_210);
        if (!out->security_schemes[i].description)
          return CDD_C_ERROR_MEMORY;
      }
    }
    if (json_object_has_value(sec_obj, "deprecated")) {
      out->security_schemes[i].deprecated_set = 1;
      out->security_schemes[i].deprecated =
          json_object_get_boolean(sec_obj, "deprecated") == 1;
    }
    {
      cdd_c_error_t _rc = collect_extensions(
          sec_obj, &out->security_schemes[i].extensions_json);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }

    if (out->security_schemes[i].type == OA_SEC_APIKEY) {
      const char *in = json_object_get_string(sec_obj, "in");
      const char *key_name = json_object_get_string(sec_obj, "name");
      out->security_schemes[i].in =
          (parse_security_in(in, &_ast_parse_security_in_52),
           _ast_parse_security_in_52);
      if (!key_name || !*key_name ||
          out->security_schemes[i].in == OA_SEC_IN_UNKNOWN)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      out->security_schemes[i].key_name =
          (c_cdd_strdup(key_name, &_ast_strdup_211), _ast_strdup_211);
      if (!out->security_schemes[i].key_name)
        return CDD_C_ERROR_MEMORY;
    } else if (out->security_schemes[i].type == OA_SEC_HTTP) {
      if (!out->security_schemes[i].scheme) {
        const char *scheme = json_object_get_string(sec_obj, "scheme");
        const char *bearer_format =
            json_object_get_string(sec_obj, "bearerFormat");
        if (!scheme || !*scheme)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        out->security_schemes[i].scheme =
            (c_cdd_strdup(scheme, &_ast_strdup_212), _ast_strdup_212);
        if (!out->security_schemes[i].scheme)
          return CDD_C_ERROR_MEMORY;
        if (bearer_format) {
          out->security_schemes[i].bearer_format =
              (c_cdd_strdup(bearer_format, &_ast_strdup_213), _ast_strdup_213);
          if (!out->security_schemes[i].bearer_format)
            return CDD_C_ERROR_MEMORY;
        }
      }
    } else if (out->security_schemes[i].type == OA_SEC_OPENID) {
      const char *oid_url = json_object_get_string(sec_obj, "openIdConnectUrl");
      if (!oid_url || !*oid_url)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      out->security_schemes[i].open_id_connect_url =
          (c_cdd_strdup(oid_url, &_ast_strdup_214), _ast_strdup_214);
      if (!out->security_schemes[i].open_id_connect_url)
        return CDD_C_ERROR_MEMORY;
    } else if (out->security_schemes[i].type == OA_SEC_OAUTH2) {
      cdd_c_error_t flow_rc;
      const char *meta_url =
          json_object_get_string(sec_obj, "oauth2MetadataUrl");
      const JSON_Object *flows_obj = json_object_get_object(sec_obj, "flows");
      if (meta_url) {
        out->security_schemes[i].oauth2_metadata_url =
            (c_cdd_strdup(meta_url, &_ast_strdup_215), _ast_strdup_215);
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
 * @brief Parses security requirements from the given input.
 */
cdd_c_error_t
parse_security_requirements(const JSON_Array *arr,
                            struct OpenAPI_SecurityRequirementSet **out,
                            size_t *out_count) {
  char *_ast_strdup_178 = NULL;
  char *_ast_strdup_179 = NULL;
  size_t i, count;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!arr || !out || !out_count)
    return CDD_C_SUCCESS;

  count = json_array_get_count(arr);
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
    const JSON_Object *sec_obj = json_array_get_object(arr, i);
    struct OpenAPI_SecurityRequirementSet *set = &(*out)[i];
    size_t j, req_count = 0;
    size_t key_count = 0;

    if (sec_obj) {
      key_count = json_object_get_count(sec_obj);
      for (j = 0; j < key_count; ++j) {
        const char *name = json_object_get_name(sec_obj, j);
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
        const char *scheme = json_object_get_name(sec_obj, j);
        const JSON_Array *scopes_arr = json_object_get_array(sec_obj, scheme);
        struct OpenAPI_SecurityRequirement *req;
        if (strncmp(scheme, "x-", 2) == 0)
          continue;
        req = &set->requirements[req_idx++];

        req->scheme = (c_cdd_strdup(scheme, &_ast_strdup_178), _ast_strdup_178);
        if (!req->scheme) {
          rc = CDD_C_ERROR_MEMORY;
          goto fail;
        }

        if (scopes_arr)
          n_scopes = json_array_get_count(scopes_arr);

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
          const char *scope = json_array_get_string(scopes_arr, k);
          req->scopes[k] = (c_cdd_strdup(scope ? scope : "", &_ast_strdup_179),
                            _ast_strdup_179);
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
    free_security_requirement_set(&(*out)[i]);
  }
  free(*out);
  *out = NULL;
  *out_count = 0;
  return rc;
}

/**
 * @brief Parses security field from the given input.
 */
cdd_c_error_t parse_security_field(const JSON_Object *obj, const char *key,
                                   struct OpenAPI_SecurityRequirementSet **out,
                                   size_t *out_count, int *out_set) {
  const JSON_Array *arr;
  if (!obj || !key || !out || !out_count || !out_set)
    return CDD_C_SUCCESS;

  if (!json_object_has_value(obj, key)) {
    *out_set = 0;
    return CDD_C_SUCCESS;
  }

  *out_set = 1;
  arr = json_object_get_array(obj, key);
  if (!arr) {
    *out = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }

  return parse_security_requirements(arr, out, out_count);
}
