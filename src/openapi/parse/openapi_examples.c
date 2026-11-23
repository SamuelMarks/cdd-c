/**
 * @file openapi_examples.c
 * @brief Example object and OAuth flow parsing.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Frees an `OpenAPI_Example` item.
 *
 * @param[in,out] ex Example item to release.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t oa_example_item_free(struct OpenAPI_Example *ex) {
  return free_example(ex);
}

/**
 * @brief Frees an `OpenAPI_Any` item.
 *
 * @param[in,out] val Any value to release.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t oa_example_any_free(struct OpenAPI_Any *val) {
  return free_any_value(val);
}

/*
 * @brief Creates a deep copy of example fields.
 *
 * @param[out] dst Destination OpenAPI_Example structure.
 * @param[in] src Source OpenAPI_Example structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_example_fields(struct OpenAPI_Example *dst,
                                  const struct OpenAPI_Example *src) {
  cdd_c_error_t rc;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  if (src->name && !dst->name) {
    rc = c_cdd_strdup(src->name, &dst->name);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->ref && !dst->ref) {
    rc = c_cdd_strdup(src->ref, &dst->ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->extensions_json && !dst->extensions_json) {
    rc = c_cdd_strdup(src->extensions_json, &dst->extensions_json);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->summary && !dst->summary) {
    rc = c_cdd_strdup(src->summary, &dst->summary);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->description && !dst->description) {
    rc = c_cdd_strdup(src->description, &dst->description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->data_value_set && !dst->data_value_set) {
    rc = copy_any_value(&dst->data_value, &src->data_value);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->data_value_set = 1;
  }
  if (src->value_set && !dst->value_set) {
    rc = copy_any_value(&dst->value, &src->value);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->value_set = 1;
  }
  if (src->serialized_value && !dst->serialized_value) {
    rc = c_cdd_strdup(src->serialized_value, &dst->serialized_value);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->external_value && !dst->external_value) {
    rc = c_cdd_strdup(src->external_value, &dst->external_value);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/*
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
  cdd_c_error_t rc;
  rc = resolve_ref_target(spec, ref, &resolved);
  if (rc != CDD_C_SUCCESS) {
    *_out_val = NULL;
    return rc;
  }
  target = resolved.spec;
  rc = ref_name_from_prefix(target, resolved.ref, "#/components/examples/",
                            &name_enc);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return rc;
  }
  if (!target || !name_enc) {
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  rc = json_pointer_unescape(name_enc, &name_dec);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return rc;
  }
  if (target->component_example_names) {
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
    rc = c_cdd_strdup(name, &out->name);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = oa_json_opt_string(ex_obj, "$ref", &ref);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (ref) {
    rc = c_cdd_strdup(ref, &out->ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (resolve_refs && spec) {
      struct OpenAPI_Example *comp = NULL;
      rc = find_component_example(spec, ref, &comp);
      if (rc != CDD_C_SUCCESS)
        return rc;
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
    rc = example_fields_valid(out, &is_valid);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!is_valid)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  return CDD_C_SUCCESS;
}

/*
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
  if (rc != CDD_C_SUCCESS) {
    cdd_c_error_t rc_f = oa_example_item_free(out);
    return (cdd_c_error_t)((int)rc | (int)rc_f);
  }
  return CDD_C_SUCCESS;
}

/*
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
  cdd_c_error_t rc;

  if (!out || !out_count) {
    return CDD_C_SUCCESS;
  }
  *out = NULL;
  *out_count = 0;

  if (!examples)
    return CDD_C_SUCCESS;

  rc = oa_json_object_count(examples, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (count == 0)
    return CDD_C_SUCCESS;

  *out = (struct OpenAPI_Example *)C_CDD_CALLOC(count,
                                                sizeof(struct OpenAPI_Example));
  if (!*out)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Object *ex_obj = NULL;
    rc = oa_json_object_name(examples, i, &name);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = oa_json_opt_object(examples, name, &ex_obj);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (ex_obj) {
      rc = parse_example_object(ex_obj, name, &(*out)[i], spec, resolve_refs);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }
  return CDD_C_SUCCESS;

cleanup: {
  size_t j;
  for (j = 0; j < count; ++j) {
    rc = (cdd_c_error_t)((int)rc | (int)oa_example_item_free(&(*out)[j]));
  }
  C_CDD_FREE(*out);
  *out = NULL;
  *out_count = 0;
}
  return rc;
}

/*
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
  int has_both = 0;
  cdd_c_error_t rc;

  if (!media_obj)
    return CDD_C_SUCCESS;

  rc = object_has_example_and_examples(media_obj, &has_both);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (has_both)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = oa_json_opt_object(media_obj, "examples", &examples_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (examples_obj) {
    return parse_examples_object(examples_obj, examples, n_examples, spec,
                                 resolve_refs);
  }
  rc = parse_any_field(media_obj, "example", example, example_set);
  if (rc != CDD_C_SUCCESS) {
    cdd_c_error_t rc_f = oa_example_any_free(example);
    return (cdd_c_error_t)((int)rc | (int)rc_f);
  }
  return CDD_C_SUCCESS;
}

/*
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
  cdd_c_error_t rc;
  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!scopes_obj)
    return CDD_C_SUCCESS;
  rc = oa_json_object_count(scopes_obj, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
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
    rc = oa_json_object_name(scopes_obj, i, &name);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = oa_json_opt_string(scopes_obj, name, &desc);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = c_cdd_strdup(name, &(*out)[i].name);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (desc) {
      rc = c_cdd_strdup(desc, &(*out)[i].description);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }
  return CDD_C_SUCCESS;

cleanup: {
  size_t j;
  for (j = 0; j < count; ++j) {
    C_CDD_FREE((*out)[j].name);
    C_CDD_FREE((*out)[j].description);
  }
  C_CDD_FREE(*out);
  *out = NULL;
  *out_count = 0;
}
  return rc;
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
  cdd_c_error_t rc;
  if (!flows_obj || !out)
    return CDD_C_SUCCESS;
  rc = oa_json_object_count(flows_obj, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (count == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  out->flows = (struct OpenAPI_OAuthFlow *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_OAuthFlow));
  if (!out->flows)
    return CDD_C_ERROR_MEMORY;
  out->n_flows = count;
  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Object *flow_obj = NULL;
    struct OpenAPI_OAuthFlow *flow = &out->flows[i];
    const char *authorization_url = NULL;
    const char *token_url = NULL;
    const char *device_authorization_url = NULL;
    int scopes_present = 0;
    JSON_Object *scopes_obj = NULL;

    rc = oa_json_object_name(flows_obj, i, &name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = oa_json_opt_object(flows_obj, name, &flow_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_oauth_flow_type(name, &flow->type);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!flow_obj)
      return CDD_C_ERROR_INVALID_ARGUMENT;

    rc = oa_json_opt_string(flow_obj, "authorizationUrl", &authorization_url);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = oa_json_opt_string(flow_obj, "tokenUrl", &token_url);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = oa_json_opt_string(flow_obj, "deviceAuthorizationUrl",
                            &device_authorization_url);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = oa_json_has_value(flow_obj, "scopes", &scopes_present);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = oa_json_opt_object(flow_obj, "scopes", &scopes_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;

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

    rc = openapi_parse_reflected_strings(flow_obj, &OpenAPI_OAuthFlow_type_info,
                                         flow);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_oauth_scopes(scopes_obj, &flow->scopes, &flow->n_scopes);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = collect_extensions(flow_obj, &flow->extensions_json);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Parses oauth flows from the given input.
 *
 * @param[in] flows_obj The source OAuth flows JSON object.
 * @param[in,out] out The target OpenAPI_SecurityScheme structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_oauth_flows(const JSON_Object *flows_obj,
                                struct OpenAPI_SecurityScheme *out) {
  cdd_c_error_t rc = parse_oauth_flows_inner(flows_obj, out);
  if (rc != CDD_C_SUCCESS) {
    if (out->flows) {
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
  return CDD_C_SUCCESS;
}
