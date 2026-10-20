/**
 * @file openapi_examples.c
 * @brief Example object and OAuth flow parsing.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Creates a deep copy of example fields.
 */
cdd_c_error_t copy_example_fields(struct OpenAPI_Example *dst,
                                  const struct OpenAPI_Example *src) {
  if (!dst || !src)
    return CDD_C_SUCCESS;
  if (src->name && !dst->name) {
    c_cdd_strdup(src->name, &dst->name);
    if (!dst->name)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->ref && !dst->ref) {
    c_cdd_strdup(src->ref, &dst->ref);
    if (!dst->ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->extensions_json && !dst->extensions_json) {
    c_cdd_strdup(src->extensions_json, &dst->extensions_json);
    if (!dst->extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->summary && !dst->summary) {
    c_cdd_strdup(src->summary, &dst->summary);
    if (!dst->summary)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->description && !dst->description) {
    c_cdd_strdup(src->description, &dst->description);
    if (!dst->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->data_value_set && !dst->data_value_set) {
    cdd_c_error_t rc = copy_any_value(&dst->data_value, &src->data_value);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->data_value_set = 1;
  }
  if (src->value_set && !dst->value_set) {
    cdd_c_error_t rc = copy_any_value(&dst->value, &src->value);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->value_set = 1;
  }
  if (src->serialized_value && !dst->serialized_value) {
    c_cdd_strdup(src->serialized_value, &dst->serialized_value);
    if (!dst->serialized_value)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->external_value && !dst->external_value) {
    c_cdd_strdup(src->external_value, &dst->external_value);
    if (!dst->external_value)
      return CDD_C_ERROR_MEMORY;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the component example.
 */
cdd_c_error_t find_component_example(const struct OpenAPI_Spec *spec,
                                     const char *ref,
                                     struct OpenAPI_Example **_out_val) {
  size_t i;
  struct ResolvedRefTarget resolved;
  const struct OpenAPI_Spec *target;
  const char *name_enc = NULL;
  char *name_dec = NULL;
  resolve_ref_target(spec, ref, &resolved);
  target = resolved.spec;
  ref_name_from_prefix(target, resolved.ref, "#/components/examples/",
                       &name_enc);
  if (!target || !name_enc) {
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  json_pointer_unescape(name_enc, &name_dec);
  if (name_dec && target->component_example_names) {
    for (i = 0; i < target->n_component_examples; ++i) {
      if (target->component_example_names[i] &&
          strcmp(target->component_example_names[i], name_dec) == 0) {
        C_CDD_FREE(name_dec);
        if (resolved.resolved_ref)
          C_CDD_FREE(resolved.resolved_ref);
        *_out_val = &target->component_examples[i];
        return CDD_C_SUCCESS;
      }
    }
  }
  C_CDD_FREE(name_dec);
  if (resolved.resolved_ref)
    C_CDD_FREE(resolved.resolved_ref);
  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Internal helper to parse an OpenAPI Example object.
 *
 * @param[in] ex_obj The source JSON example object.
 * @param[in] name The example key name.
 * @param[out] out Output OpenAPI_Example structure.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t parse_example_object_inner(const JSON_Object *ex_obj,
                                                const char *name,
                                                struct OpenAPI_Example *out,
                                                const struct OpenAPI_Spec *spec,
                                                int resolve_refs) {
  const char *ref;
  const char *summary;
  const char *desc;
  const char *serialized;
  const char *external;
  cdd_c_error_t rc;

  if (!ex_obj || !out)
    return CDD_C_SUCCESS;

  if (name) {
    c_cdd_strdup(name, &out->name);
    if (!out->name)
      return CDD_C_ERROR_MEMORY;
  }

  ref = json_object_get_string(ex_obj, "$ref");
  if (ref) {
    c_cdd_strdup(ref, &out->ref);
    if (!out->ref)
      return CDD_C_ERROR_MEMORY;
    if (resolve_refs && spec) {
      struct OpenAPI_Example *comp = NULL;
      find_component_example(spec, ref, &comp);
      if (comp) {
        rc = copy_example_fields(out, comp);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }

  summary = json_object_get_string(ex_obj, "summary");
  if (summary) {
    C_CDD_FREE(out->summary);
    out->summary = NULL;
    c_cdd_strdup(summary, &out->summary);
    if (!out->summary)
      return CDD_C_ERROR_MEMORY;
  }
  desc = json_object_get_string(ex_obj, "description");
  if (desc) {
    C_CDD_FREE(out->description);
    out->description = NULL;
    c_cdd_strdup(desc, &out->description);
    if (!out->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (!ref) {
    rc = collect_extensions(ex_obj, &out->extensions_json);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = parse_any_field(ex_obj, "dataValue", &out->data_value,
                       &out->data_value_set);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_any_field(ex_obj, "value", &out->value, &out->value_set);
  if (rc != CDD_C_SUCCESS)
    return rc;

  serialized = json_object_get_string(ex_obj, "serializedValue");
  if (serialized) {
    C_CDD_FREE(out->serialized_value);
    out->serialized_value = NULL;
    c_cdd_strdup(serialized, &out->serialized_value);
    if (!out->serialized_value)
      return CDD_C_ERROR_MEMORY;
  }
  external = json_object_get_string(ex_obj, "externalValue");
  if (external) {
    C_CDD_FREE(out->external_value);
    out->external_value = NULL;
    c_cdd_strdup(external, &out->external_value);
    if (!out->external_value)
      return CDD_C_ERROR_MEMORY;
  }

  if (!out->ref && !example_fields_valid(out))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses example object from the given input.
 */
cdd_c_error_t parse_example_object(const JSON_Object *ex_obj, const char *name,
                                   struct OpenAPI_Example *out,
                                   const struct OpenAPI_Spec *spec,
                                   int resolve_refs) {
  cdd_c_error_t rc =
      parse_example_object_inner(ex_obj, name, out, spec, resolve_refs);
  if (rc != CDD_C_SUCCESS)
    free_example(out);
  return rc;
}

/**
 * @brief Parses examples object from the given input.
 */
cdd_c_error_t parse_examples_object(const JSON_Object *examples,
                                    struct OpenAPI_Example **out,
                                    size_t *out_count,
                                    const struct OpenAPI_Spec *spec,
                                    int resolve_refs) {
  size_t count, i;

  if (!out || !out_count) {
    return CDD_C_SUCCESS;
  }
  *out = NULL;
  *out_count = 0;

  if (!examples)
    return CDD_C_SUCCESS;

  count = json_object_get_count(examples);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out = (struct OpenAPI_Example *)C_CDD_CALLOC(count,
                                                sizeof(struct OpenAPI_Example));
  if (!*out)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(examples, i);
    const JSON_Object *ex_obj =
        json_value_get_object(json_object_get_value_at(examples, i));
    if (ex_obj) {
      cdd_c_error_t rc =
          parse_example_object(ex_obj, name, &(*out)[i], spec, resolve_refs);
      if (rc != CDD_C_SUCCESS) {
        size_t j;
        for (j = 0; j < count; ++j)
          free_example(&(*out)[j]);
        C_CDD_FREE(*out);
        *out = NULL;
        *out_count = 0;
        return rc;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses media examples from the given input.
 */
cdd_c_error_t
parse_media_examples(const JSON_Object *media_obj, struct OpenAPI_Any *example,
                     int *example_set, struct OpenAPI_Example **examples,
                     size_t *n_examples, const struct OpenAPI_Spec *spec,
                     int resolve_refs) {
  const JSON_Object *examples_obj;
  cdd_c_error_t rc;

  if (!media_obj)
    return CDD_C_SUCCESS;
  if (object_has_example_and_examples(media_obj))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  examples_obj = json_object_get_object(media_obj, "examples");
  if (examples_obj) {
    return parse_examples_object(examples_obj, examples, n_examples, spec,
                                 resolve_refs);
  }
  rc = parse_any_field(media_obj, "example", example, example_set);
  if (rc != CDD_C_SUCCESS)
    free_any_value(example);
  return rc;
}

/**
 * @brief Parses oauth scopes from the given input.
 */
cdd_c_error_t parse_oauth_scopes(const JSON_Object *scopes_obj,
                                 struct OpenAPI_OAuthScope **out,
                                 size_t *out_count) {
  size_t count, i;
  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!scopes_obj)
    return CDD_C_SUCCESS;
  count = json_object_get_count(scopes_obj);
  if (count == 0)
    return CDD_C_SUCCESS;
  *out = (struct OpenAPI_OAuthScope *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_OAuthScope));
  if (!*out)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;
  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(scopes_obj, i);
    const char *desc = json_object_get_string(scopes_obj, name);
    c_cdd_strdup(name, &(*out)[i].name);
    if (!(*out)[i].name) {
      size_t j;
      for (j = 0; j < count; ++j) {
        C_CDD_FREE((*out)[j].name);
        C_CDD_FREE((*out)[j].description);
      }
      C_CDD_FREE(*out);
      *out = NULL;
      *out_count = 0;
      return CDD_C_ERROR_MEMORY;
    }
    if (desc) {
      c_cdd_strdup(desc, &(*out)[i].description);
      if (!(*out)[i].description) {
        size_t j;
        for (j = 0; j < count; ++j) {
          C_CDD_FREE((*out)[j].name);
          C_CDD_FREE((*out)[j].description);
        }
        C_CDD_FREE(*out);
        *out = NULL;
        *out_count = 0;
        return CDD_C_ERROR_MEMORY;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Internal helper to parse OAuth flows into a security scheme.
 *
 * @param[in] flows_obj The source OAuth flows JSON object.
 * @param[in,out] out The target OpenAPI_SecurityScheme structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
parse_oauth_flows_inner(const JSON_Object *flows_obj,
                        struct OpenAPI_SecurityScheme *out) {
  size_t count, i;
  if (!flows_obj || !out)
    return CDD_C_SUCCESS;
  count = json_object_get_count(flows_obj);
  if (count == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  out->flows = (struct OpenAPI_OAuthFlow *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_OAuthFlow));
  if (!out->flows)
    return CDD_C_ERROR_MEMORY;
  out->n_flows = count;
  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(flows_obj, i);
    const JSON_Object *flow_obj =
        json_value_get_object(json_object_get_value_at(flows_obj, i));
    struct OpenAPI_OAuthFlow *flow = &out->flows[i];
    parse_oauth_flow_type(name, &flow->type);
    if (!flow_obj)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    {
      const char *authorization_url =
          json_object_get_string(flow_obj, "authorizationUrl");
      const char *token_url = json_object_get_string(flow_obj, "tokenUrl");
      const char *refresh_url = json_object_get_string(flow_obj, "refreshUrl");
      const char *device_authorization_url =
          json_object_get_string(flow_obj, "deviceAuthorizationUrl");
      const int scopes_present = json_object_has_value(flow_obj, "scopes");
      const JSON_Object *scopes_obj =
          json_object_get_object(flow_obj, "scopes");
      cdd_c_error_t rc;

      if (!scopes_present || !scopes_obj)
        return CDD_C_ERROR_INVALID_ARGUMENT;

      switch (flow->type) {
      case OA_OAUTH_FLOW_IMPLICIT:
        if (!authorization_url)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        break;
      case OA_OAUTH_FLOW_PASSWORD:
        if (!token_url)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        break;
      case OA_OAUTH_FLOW_CLIENT_CREDENTIALS:
        if (!token_url)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        break;
      case OA_OAUTH_FLOW_AUTHORIZATION_CODE:
        if (!authorization_url || !token_url)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        break;
      case OA_OAUTH_FLOW_DEVICE_AUTHORIZATION:
        if (!device_authorization_url || !token_url)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        break;
      default:
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }

      if (authorization_url) {
        c_cdd_strdup(authorization_url, &flow->authorization_url);
        if (!flow->authorization_url)
          return CDD_C_ERROR_MEMORY;
      }
      if (token_url) {
        c_cdd_strdup(token_url, &flow->token_url);
        if (!flow->token_url)
          return CDD_C_ERROR_MEMORY;
      }
      if (refresh_url) {
        c_cdd_strdup(refresh_url, &flow->refresh_url);
        if (!flow->refresh_url)
          return CDD_C_ERROR_MEMORY;
      }
      if (device_authorization_url) {
        c_cdd_strdup(device_authorization_url, &flow->device_authorization_url);
        if (!flow->device_authorization_url)
          return CDD_C_ERROR_MEMORY;
      }
      rc = parse_oauth_scopes(scopes_obj, &flow->scopes, &flow->n_scopes);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = collect_extensions(flow_obj, &flow->extensions_json);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses oauth flows from the given input.
 */
cdd_c_error_t parse_oauth_flows(const JSON_Object *flows_obj,
                                struct OpenAPI_SecurityScheme *out) {
  cdd_c_error_t rc = parse_oauth_flows_inner(flows_obj, out);
  if (rc != CDD_C_SUCCESS && out->flows) {
    size_t f, s;
    for (f = 0; f < out->n_flows; ++f) {
      C_CDD_FREE(out->flows[f].authorization_url);
      C_CDD_FREE(out->flows[f].token_url);
      C_CDD_FREE(out->flows[f].refresh_url);
      C_CDD_FREE(out->flows[f].device_authorization_url);
      C_CDD_FREE(out->flows[f].extensions_json);
      if (out->flows[f].scopes) {
        for (s = 0; s < out->flows[f].n_scopes; ++s) {
          C_CDD_FREE(out->flows[f].scopes[s].name);
          C_CDD_FREE(out->flows[f].scopes[s].description);
        }
        C_CDD_FREE(out->flows[f].scopes);
      }
    }
    C_CDD_FREE(out->flows);
    out->flows = NULL;
    out->n_flows = 0;
  }
  return rc;
}
