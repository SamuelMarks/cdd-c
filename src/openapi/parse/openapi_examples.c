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
  char *_ast_strdup_20 = NULL;
  char *_ast_strdup_21 = NULL;
  char *_ast_strdup_22 = NULL;
  char *_ast_strdup_23 = NULL;
  char *_ast_strdup_24 = NULL;
  char *_ast_strdup_26 = NULL;
  char *_ast_strdup_27 = NULL;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  if (src->name && !dst->name) {
    dst->name = (c_cdd_strdup(src->name, &_ast_strdup_20), _ast_strdup_20);
    if (!dst->name)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->ref && !dst->ref) {
    dst->ref = (c_cdd_strdup(src->ref, &_ast_strdup_21), _ast_strdup_21);
    if (!dst->ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->extensions_json && !dst->extensions_json) {
    dst->extensions_json =
        (c_cdd_strdup(src->extensions_json, &_ast_strdup_22), _ast_strdup_22);
    if (!dst->extensions_json)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->summary && !dst->summary) {
    dst->summary =
        (c_cdd_strdup(src->summary, &_ast_strdup_23), _ast_strdup_23);
    if (!dst->summary)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->description && !dst->description) {
    dst->description =
        (c_cdd_strdup(src->description, &_ast_strdup_24), _ast_strdup_24);
    if (!dst->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->data_value_set && !dst->data_value_set) {
    {
      cdd_c_error_t _rc = copy_any_value(&dst->data_value, &src->data_value);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    dst->data_value_set = 1;
  }
  if (src->value_set && !dst->value_set) {
    {
      cdd_c_error_t _rc = copy_any_value(&dst->value, &src->value);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    dst->value_set = 1;
  }
  if (src->serialized_value && !dst->serialized_value) {
    dst->serialized_value =
        (c_cdd_strdup(src->serialized_value, &_ast_strdup_26), _ast_strdup_26);
    if (!dst->serialized_value)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->external_value && !dst->external_value) {
    dst->external_value =
        (c_cdd_strdup(src->external_value, &_ast_strdup_27), _ast_strdup_27);
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
  struct ResolvedRefTarget _ast_resolve_ref_target_2;
  char *_ast_ref_name_from_prefix_3 = NULL;
  char *_ast_json_pointer_unescape_4 = NULL;
  size_t i;
  struct ResolvedRefTarget resolved =
      (resolve_ref_target(spec, ref, &_ast_resolve_ref_target_2),
       _ast_resolve_ref_target_2);
  const struct OpenAPI_Spec *target = resolved.spec;
  const char *name_enc =
      (ref_name_from_prefix(target, resolved.ref, "#/components/examples/",
                            &_ast_ref_name_from_prefix_3),
       _ast_ref_name_from_prefix_3);
  char *name_dec;
  if (!target || !name_enc) {
    if (resolved.resolved_ref)
      free(resolved.resolved_ref);
    {
      *_out_val = NULL;
      return CDD_C_SUCCESS;
    }
  }
  name_dec = (json_pointer_unescape(name_enc, &_ast_json_pointer_unescape_4),
              _ast_json_pointer_unescape_4);
  if (name_dec && target->component_example_names) {
    for (i = 0; i < target->n_component_examples; ++i) {
      if (target->component_example_names[i] &&
          strcmp(target->component_example_names[i], name_dec) == 0) {
        free(name_dec);
        if (resolved.resolved_ref)
          free(resolved.resolved_ref);
        {
          *_out_val = &target->component_examples[i];
          return CDD_C_SUCCESS;
        }
      }
    }
  }
  free(name_dec);
  if (resolved.resolved_ref)
    free(resolved.resolved_ref);
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses example object from the given input.
 */
cdd_c_error_t parse_example_object(const JSON_Object *ex_obj, const char *name,
                                   struct OpenAPI_Example *out,
                                   const struct OpenAPI_Spec *spec,
                                   int resolve_refs) {
  struct OpenAPI_Example *_ast_find_component_example_5;
  char *_ast_strdup_28 = NULL;
  char *_ast_strdup_29 = NULL;
  char *_ast_strdup_30 = NULL;
  char *_ast_strdup_31 = NULL;
  char *_ast_strdup_32 = NULL;
  char *_ast_strdup_33 = NULL;
  const char *ref;
  const char *summary;
  const char *desc;
  const char *serialized;
  const char *external;

  if (!ex_obj || !out)
    return CDD_C_SUCCESS;

  if (name) {
    out->name = (c_cdd_strdup(name, &_ast_strdup_28), _ast_strdup_28);
    if (!out->name)
      return CDD_C_ERROR_MEMORY;
  }

  ref = json_object_get_string(ex_obj, "$ref");
  if (ref) {
    out->ref = (c_cdd_strdup(ref, &_ast_strdup_29), _ast_strdup_29);
    if (!out->ref)
      return CDD_C_ERROR_MEMORY;
    if (resolve_refs && spec) {
      const struct OpenAPI_Example *comp =
          (find_component_example(spec, ref, &_ast_find_component_example_5),
           _ast_find_component_example_5);
      if (comp) {
        {
          cdd_c_error_t _rc = copy_example_fields(out, comp);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }
  }

  summary = json_object_get_string(ex_obj, "summary");
  if (summary) {
    out->summary = (c_cdd_strdup(summary, &_ast_strdup_30), _ast_strdup_30);
    if (!out->summary)
      return CDD_C_ERROR_MEMORY;
  }
  desc = json_object_get_string(ex_obj, "description");
  if (desc) {
    out->description = (c_cdd_strdup(desc, &_ast_strdup_31), _ast_strdup_31);
    if (!out->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (!ref) {
    {
      cdd_c_error_t _rc = collect_extensions(ex_obj, &out->extensions_json);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  {
    cdd_c_error_t _rc = parse_any_field(ex_obj, "dataValue", &out->data_value,
                                        &out->data_value_set);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }
  {
    cdd_c_error_t _rc =
        parse_any_field(ex_obj, "value", &out->value, &out->value_set);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  serialized = json_object_get_string(ex_obj, "serializedValue");
  if (serialized) {
    out->serialized_value =
        (c_cdd_strdup(serialized, &_ast_strdup_32), _ast_strdup_32);
    if (!out->serialized_value)
      return CDD_C_ERROR_MEMORY;
  }
  external = json_object_get_string(ex_obj, "externalValue");
  if (external) {
    out->external_value =
        (c_cdd_strdup(external, &_ast_strdup_33), _ast_strdup_33);
    if (!out->external_value)
      return CDD_C_ERROR_MEMORY;
  }

  if (!out->ref && !example_fields_valid(out))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  return CDD_C_SUCCESS;
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
      {
        cdd_c_error_t rc =
            parse_example_object(ex_obj, name, &(*out)[i], spec, resolve_refs);
        if (rc != CDD_C_SUCCESS)
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

  if (!media_obj)
    return CDD_C_SUCCESS;
  if (object_has_example_and_examples(media_obj))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  examples_obj = json_object_get_object(media_obj, "examples");
  if (examples_obj) {
    return parse_examples_object(examples_obj, examples, n_examples, spec,
                                 resolve_refs);
  }
  return parse_any_field(media_obj, "example", example, example_set);
}

/**
 * @brief Parses oauth scopes from the given input.
 */
cdd_c_error_t parse_oauth_scopes(const JSON_Object *scopes_obj,
                                 struct OpenAPI_OAuthScope **out,
                                 size_t *out_count) {
  char *_ast_strdup_34 = NULL;
  char *_ast_strdup_35 = NULL;
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
    (*out)[i].name = (c_cdd_strdup(name, &_ast_strdup_34), _ast_strdup_34);
    if (!(*out)[i].name)
      return CDD_C_ERROR_MEMORY;
    if (desc) {
      (*out)[i].description =
          (c_cdd_strdup(desc, &_ast_strdup_35), _ast_strdup_35);
      if (!(*out)[i].description)
        return CDD_C_ERROR_MEMORY;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses oauth flows from the given input.
 */
cdd_c_error_t parse_oauth_flows(const JSON_Object *flows_obj,
                                struct OpenAPI_SecurityScheme *out) {
  enum OpenAPI_OAuthFlowType _ast_parse_oauth_flow_type_6;
  char *_ast_strdup_36 = NULL;
  char *_ast_strdup_37 = NULL;
  char *_ast_strdup_38 = NULL;
  char *_ast_strdup_39 = NULL;
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
    flow->type = (parse_oauth_flow_type(name, &_ast_parse_oauth_flow_type_6),
                  _ast_parse_oauth_flow_type_6);
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
        flow->authorization_url =
            (c_cdd_strdup(authorization_url, &_ast_strdup_36), _ast_strdup_36);
        if (!flow->authorization_url)
          return CDD_C_ERROR_MEMORY;
      }
      if (token_url) {
        flow->token_url =
            (c_cdd_strdup(token_url, &_ast_strdup_37), _ast_strdup_37);
        if (!flow->token_url)
          return CDD_C_ERROR_MEMORY;
      }
      if (refresh_url) {
        flow->refresh_url =
            (c_cdd_strdup(refresh_url, &_ast_strdup_38), _ast_strdup_38);
        if (!flow->refresh_url)
          return CDD_C_ERROR_MEMORY;
      }
      if (device_authorization_url) {
        flow->device_authorization_url =
            (c_cdd_strdup(device_authorization_url, &_ast_strdup_39),
             _ast_strdup_39);
        if (!flow->device_authorization_url)
          return CDD_C_ERROR_MEMORY;
      }
      {
        cdd_c_error_t _rc =
            parse_oauth_scopes(scopes_obj, &flow->scopes, &flow->n_scopes);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      {
        cdd_c_error_t _rc =
            collect_extensions(flow_obj, &flow->extensions_json);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }
  return CDD_C_SUCCESS;
}
