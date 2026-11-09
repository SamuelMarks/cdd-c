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
 *
 * @param[out] dst Destination OpenAPI_Example structure.
 * @param[in] src Source OpenAPI_Example structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_example_fields(struct OpenAPI_Example *dst,
                                  const struct OpenAPI_Example *src) {
  if (!dst || !src)
    return CDD_C_SUCCESS;
  if (src->name && !dst->name) {
    g_cdd_last_alloc_err = c_cdd_strdup(src->name, &dst->name);
    if (!dst->name)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->ref && !dst->ref) {
    g_cdd_last_alloc_err = c_cdd_strdup(src->ref, &dst->ref);
    if (!dst->ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->extensions_json && !dst->extensions_json) {
    g_cdd_last_alloc_err =
        c_cdd_strdup(src->extensions_json, &dst->extensions_json);
    if (!dst->extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->summary && !dst->summary) {
    g_cdd_last_alloc_err = c_cdd_strdup(src->summary, &dst->summary);
    if (!dst->summary)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->description && !dst->description) {
    g_cdd_last_alloc_err = c_cdd_strdup(src->description, &dst->description);
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
    g_cdd_last_alloc_err =
        c_cdd_strdup(src->serialized_value, &dst->serialized_value);
    if (!dst->serialized_value)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->external_value && !dst->external_value) {
    g_cdd_last_alloc_err =
        c_cdd_strdup(src->external_value, &dst->external_value);
    if (!dst->external_value)
      return CDD_C_ERROR_MEMORY;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the component example.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found example.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t find_component_example(const struct OpenAPI_Spec *spec,
                                     const char *ref,
                                     struct OpenAPI_Example **_out_val) {
  size_t i;
  struct ResolvedRefTarget resolved;
  const struct OpenAPI_Spec *target;
  const char *name_enc = NULL;
  char *name_dec = NULL;
  g_cdd_last_alloc_err = resolve_ref_target(spec, ref, &resolved);
  target = resolved.spec;
  g_cdd_last_alloc_err = ref_name_from_prefix(
      target, resolved.ref, "#/components/examples/", &name_enc);
  if (!target || !name_enc) {
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  g_cdd_last_alloc_err = json_pointer_unescape(name_enc, &name_dec);
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
  const char *ref = NULL;
  cdd_c_error_t rc;

  if (!ex_obj || !out)
    return CDD_C_SUCCESS;

  if (name) {
    g_cdd_last_alloc_err = c_cdd_strdup(name, &out->name);
    if (!out->name)
      return CDD_C_ERROR_MEMORY;
  }

  g_cdd_last_alloc_err = cdd_json_object_get_string(ex_obj, "$ref", &ref);
  if (ref) {
    g_cdd_last_alloc_err = c_cdd_strdup(ref, &out->ref);
    if (!out->ref)
      return CDD_C_ERROR_MEMORY;
    if (resolve_refs && spec) {
      struct OpenAPI_Example *comp = NULL;
      g_cdd_last_alloc_err = find_component_example(spec, ref, &comp);
      if (comp) {
        rc = copy_example_fields(out, comp);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }

  rc = openapi_parse_reflected_strings(ex_obj, &OpenAPI_Example_type_info, out);
  if (rc != CDD_C_SUCCESS)
    return rc;
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

  if (!out->ref) {
    int is_valid = 0;
    g_cdd_last_alloc_err = example_fields_valid(out, &is_valid);
    if (!is_valid)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses example object from the given input.
 *
 * @param[in] ex_obj The source JSON example object.
 * @param[in] name The example key name.
 * @param[out] out Output OpenAPI_Example structure.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_example_object(const JSON_Object *ex_obj, const char *name,
                                   struct OpenAPI_Example *out,
                                   const struct OpenAPI_Spec *spec,
                                   int resolve_refs) {
  cdd_c_error_t rc =
      parse_example_object_inner(ex_obj, name, out, spec, resolve_refs);
  if (rc != CDD_C_SUCCESS)
    g_cdd_last_alloc_err = free_example(out);
  return rc;
}

/**
 * @brief Parses examples object from the given input.
 *
 * @param[in] examples The source JSON examples map object.
 * @param[out] out Output array of OpenAPI_Example structures.
 * @param[out] out_count Output number of parsed examples.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_examples_object(const JSON_Object *examples,
                                    struct OpenAPI_Example **out,
                                    size_t *out_count,
                                    const struct OpenAPI_Spec *spec,
                                    int resolve_refs) {
  size_t count = 0, i;

  if (!out || !out_count) {
    return CDD_C_SUCCESS;
  }
  *out = NULL;
  *out_count = 0;

  if (!examples)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_object_get_count(examples, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out = (struct OpenAPI_Example *)C_CDD_CALLOC(count,
                                                sizeof(struct OpenAPI_Example));
  if (!*out)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *ex_obj = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_name(examples, i, &name);
    g_cdd_last_alloc_err = cdd_json_object_get_value_at(examples, i, &val_at);
    g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &ex_obj);
    if (ex_obj) {
      cdd_c_error_t rc =
          parse_example_object(ex_obj, name, &(*out)[i], spec, resolve_refs);
      if (rc != CDD_C_SUCCESS) {
        size_t j;
        for (j = 0; j < count; ++j)
          g_cdd_last_alloc_err = free_example(&(*out)[j]);
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
 *
 * @param[in] media_obj The source media type JSON object.
 * @param[out] example Output single example value.
 * @param[out] example_set Output flag set to 1 if single example is present.
 * @param[out] examples Output array of named examples.
 * @param[out] n_examples Output count of named examples.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t
parse_media_examples(const JSON_Object *media_obj, struct OpenAPI_Any *example,
                     int *example_set, struct OpenAPI_Example **examples,
                     size_t *n_examples, const struct OpenAPI_Spec *spec,
                     int resolve_refs) {
  JSON_Object *examples_obj = NULL;
  cdd_c_error_t rc;

  if (!media_obj)
    return CDD_C_SUCCESS;
  {
    int has_both = 0;
    g_cdd_last_alloc_err =
        object_has_example_and_examples(media_obj, &has_both);
    if (has_both)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  g_cdd_last_alloc_err =
      cdd_json_object_get_object(media_obj, "examples", &examples_obj);
  if (examples_obj) {
    return parse_examples_object(examples_obj, examples, n_examples, spec,
                                 resolve_refs);
  }
  rc = parse_any_field(media_obj, "example", example, example_set);
  if (rc != CDD_C_SUCCESS)
    g_cdd_last_alloc_err = free_any_value(example);
  return rc;
}

/**
 * @brief Parses oauth scopes from the given input.
 *
 * @param[in] scopes_obj The source scopes JSON object.
 * @param[out] out Output array of OpenAPI_OAuthScope structures.
 * @param[out] out_count Output count of scopes.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_oauth_scopes(const JSON_Object *scopes_obj,
                                 struct OpenAPI_OAuthScope **out,
                                 size_t *out_count) {
  size_t count = 0, i;
  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!scopes_obj)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_object_get_count(scopes_obj, &count);
  if (count == 0)
    return CDD_C_SUCCESS;
  *out = (struct OpenAPI_OAuthScope *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_OAuthScope));
  if (!*out)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;
  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    const char *desc = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_name(scopes_obj, i, &name);
    g_cdd_last_alloc_err = cdd_json_object_get_string(scopes_obj, name, &desc);
    g_cdd_last_alloc_err = c_cdd_strdup(name, &(*out)[i].name);
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
      g_cdd_last_alloc_err = c_cdd_strdup(desc, &(*out)[i].description);
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
  size_t count = 0, i;
  if (!flows_obj || !out)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_object_get_count(flows_obj, &count);
  if (count == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  out->flows = (struct OpenAPI_OAuthFlow *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_OAuthFlow));
  if (!out->flows)
    return CDD_C_ERROR_MEMORY;
  out->n_flows = count;
  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *flow_obj = NULL;
    struct OpenAPI_OAuthFlow *flow = &out->flows[i];
    g_cdd_last_alloc_err = cdd_json_object_get_name(flows_obj, i, &name);
    g_cdd_last_alloc_err = cdd_json_object_get_value_at(flows_obj, i, &val_at);
    g_cdd_last_alloc_err = cdd_json_value_get_object(val_at, &flow_obj);
    g_cdd_last_alloc_err = parse_oauth_flow_type(name, &flow->type);
    if (!flow_obj)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    {
      const char *authorization_url = NULL;
      const char *token_url = NULL;
      const char *refresh_url = NULL;
      const char *device_authorization_url = NULL;
      int scopes_present = 0;
      JSON_Object *scopes_obj = NULL;
      cdd_c_error_t rc;

      g_cdd_last_alloc_err = cdd_json_object_get_string(
          flow_obj, "authorizationUrl", &authorization_url);
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(flow_obj, "tokenUrl", &token_url);
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(flow_obj, "refreshUrl", &refresh_url);
      g_cdd_last_alloc_err = cdd_json_object_get_string(
          flow_obj, "deviceAuthorizationUrl", &device_authorization_url);
      g_cdd_last_alloc_err =
          cdd_json_object_has_value(flow_obj, "scopes", &scopes_present);
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(flow_obj, "scopes", &scopes_obj);

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

      rc = openapi_parse_reflected_strings(flow_obj,
                                           &OpenAPI_OAuthFlow_type_info, flow);
      if (rc != CDD_C_SUCCESS)
        return rc;
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
 *
 * @param[in] flows_obj The source OAuth flows JSON object.
 * @param[in,out] out The target OpenAPI_SecurityScheme structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
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
