/**
 * @file openapi_components.c
 * @brief OpenAPI emitter security and components generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/json_dom.h"
#include "c_cdd/log.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include "openapi/emit/openapi.h"
/* clang-format on */

/**
 * @brief Writes security schemes to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_security_schemes(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *sec_val = NULL;
  JSON_Object *sec_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (spec->n_security_schemes == 0)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&sec_val);
  if (!sec_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(sec_val, &sec_obj);

  for (i = 0; i < spec->n_security_schemes; ++i) {
    const struct OpenAPI_SecurityScheme *s = &spec->security_schemes[i];
    JSON_Value *s_val = NULL;
    JSON_Object *s_obj = NULL;
    rc = cdd_json_value_init_object(&s_val);
    rc = cdd_json_value_get_object(s_val, &s_obj);

    rc = openapi_emit_reflected_strings(s_obj,
                                        &OpenAPI_SecurityScheme_type_info, s);
    if (s->deprecated_set)
      rc = cdd_json_object_set_boolean(s_obj, "deprecated",
                                       s->deprecated ? 1 : 0);

    switch (s->type) {
    case OA_SEC_APIKEY:
      rc = cdd_json_object_set_string(s_obj, "type", "apiKey");
      if (s->in == OA_SEC_IN_HEADER)
        rc = cdd_json_object_set_string(s_obj, "in", "header");
      else if (s->in == OA_SEC_IN_QUERY)
        rc = cdd_json_object_set_string(s_obj, "in", "query");
      else if (s->in == OA_SEC_IN_COOKIE)
        rc = cdd_json_object_set_string(s_obj, "in", "cookie");

      if (s->key_name)
        rc = cdd_json_object_set_string(s_obj, "name", s->key_name);
      break;

    case OA_SEC_HTTP:
      rc = cdd_json_object_set_string(s_obj, "type", "http");
      if (s->scheme)
        rc = cdd_json_object_set_string(s_obj, "scheme", s->scheme);
      if (s->scheme && strcmp(s->scheme, "bearer") == 0 && !s->bearer_format) {
        rc = cdd_json_object_set_string(s_obj, "bearerFormat",
                                        "JWT"); /* Common */
      }
      break;

    case OA_SEC_MUTUALTLS:
      rc = cdd_json_object_set_string(s_obj, "type", "mutualTLS");
      break;

    case OA_SEC_OAUTH2:
      rc = cdd_json_object_set_string(s_obj, "type", "oauth2");
      if (s->flows && s->n_flows > 0) {
        JSON_Value *flows_val = NULL;
        JSON_Object *flows_obj = NULL;
        size_t f;
        rc = cdd_json_value_init_object(&flows_val);
        rc = cdd_json_value_get_object(flows_val, &flows_obj);
        for (f = 0; f < s->n_flows; ++f) {
          const struct OpenAPI_OAuthFlow *flow = &s->flows[f];
          char *flow_key = NULL;
          JSON_Value *flow_val = NULL;
          JSON_Object *flow_obj = NULL;
          JSON_Value *scopes_val = NULL;
          JSON_Object *scopes_obj = NULL;
          size_t sc;
          rc = oauth_flow_type_to_str_openapi(flow->type, &flow_key);
          if (!flow_key)
            continue;
          rc = cdd_json_value_init_object(&flow_val);
          rc = cdd_json_value_get_object(flow_val, &flow_obj);
          rc = openapi_emit_reflected_strings(
              flow_obj, &OpenAPI_OAuthFlow_type_info, flow);
          rc = cdd_json_value_init_object(&scopes_val);
          rc = cdd_json_value_get_object(scopes_val, &scopes_obj);
          if (flow->scopes) {
            for (sc = 0; sc < flow->n_scopes; ++sc) {
              const char *scope_name = flow->scopes[sc].name;
              const char *scope_desc = flow->scopes[sc].description;
              if (scope_name)
                rc = cdd_json_object_set_string(scopes_obj, scope_name,
                                                scope_desc ? scope_desc : "");
            }
          }
          rc = cdd_json_object_set_value_safe(flow_obj, "scopes", scopes_val);
          if (flow->extensions_json)
            rc = merge_schema_extras_object_openapi(flow_obj,
                                                    flow->extensions_json);
          rc = cdd_json_object_set_value_safe(flows_obj, flow_key, flow_val);
        }
        rc = cdd_json_object_set_value_safe(s_obj, "flows", flows_val);
      }
      break;

    case OA_SEC_OPENID:
      rc = cdd_json_object_set_string(s_obj, "type", "openIdConnect");
      break;

    default:
      break;
    }

    if (s->extensions_json)
      rc = merge_schema_extras_object_openapi(s_obj, s->extensions_json);

    rc = cdd_json_object_set_value_safe(sec_obj, s->name ? s->name : "unknown",
                                        s_val);
  }

  rc = cdd_json_object_set_value_safe(components, "securitySchemes", sec_val);
  return rc;
}

/**
 * @brief Writes component parameters to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_component_parameters(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *params_val = NULL;
  JSON_Object *params_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!spec || spec->n_component_parameters == 0)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&params_val);
  if (!params_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(params_val, &params_obj);

  for (i = 0; i < spec->n_component_parameters; ++i) {
    const char *name = spec->component_parameter_names[i];
    const struct OpenAPI_Parameter *param = &spec->component_parameters[i];
    JSON_Value *p_val = NULL;
    JSON_Object *p_obj = NULL;
    rc = cdd_json_value_init_object(&p_val);
    rc = cdd_json_value_get_object(p_val, &p_obj);
    if (!name) {
      rc = cdd_json_value_free(p_val);
      continue;
    }
    rc = write_parameter_object(p_obj, param);
    rc = cdd_json_object_set_value_safe(params_obj, name, p_val);
  }

  rc = cdd_json_object_set_value_safe(components, "parameters", params_val);
  return rc;
}

/**
 * @brief Writes component responses to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_component_responses(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *resp_val = NULL;
  JSON_Object *resp_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!spec || spec->n_component_responses == 0)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&resp_val);
  if (!resp_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(resp_val, &resp_obj);

  for (i = 0; i < spec->n_component_responses; ++i) {
    const char *name = spec->component_response_names[i];
    const struct OpenAPI_Response *resp = &spec->component_responses[i];
    JSON_Value *r_val = NULL;
    JSON_Object *r_obj = NULL;
    rc = cdd_json_value_init_object(&r_val);
    rc = cdd_json_value_get_object(r_val, &r_obj);
    if (!name) {
      rc = cdd_json_value_free(r_val);
      continue;
    }
    rc = write_response_object(r_obj, resp);
    rc = cdd_json_object_set_value_safe(resp_obj, name, r_val);
  }

  rc = cdd_json_object_set_value_safe(components, "responses", resp_val);
  return rc;
}

/**
 * @brief Writes component headers to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_component_headers(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *hdrs_val = NULL;
  JSON_Object *hdrs_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!spec || spec->n_component_headers == 0)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&hdrs_val);
  if (!hdrs_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(hdrs_val, &hdrs_obj);

  for (i = 0; i < spec->n_component_headers; ++i) {
    const char *name = spec->component_header_names[i];
    const struct OpenAPI_Header *hdr = &spec->component_headers[i];
    JSON_Value *h_val = NULL;
    JSON_Object *h_obj = NULL;
    rc = cdd_json_value_init_object(&h_val);
    rc = cdd_json_value_get_object(h_val, &h_obj);
    if (!name) {
      rc = cdd_json_value_free(h_val);
      continue;
    }
    rc = write_header_object(h_obj, hdr);
    rc = cdd_json_object_set_value_safe(hdrs_obj, name, h_val);
  }

  rc = cdd_json_object_set_value_safe(components, "headers", hdrs_val);
  return rc;
}

/**
 * @brief Writes component media types to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_component_media_types(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *media_val = NULL;
  JSON_Object *media_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!spec || spec->n_component_media_types == 0)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&media_val);
  if (!media_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(media_val, &media_obj);

  for (i = 0; i < spec->n_component_media_types; ++i) {
    const char *name = spec->component_media_type_names[i];
    const struct OpenAPI_MediaType *mt = &spec->component_media_types[i];
    JSON_Value *mt_val = NULL;
    JSON_Object *mt_obj = NULL;
    rc = cdd_json_value_init_object(&mt_val);
    rc = cdd_json_value_get_object(mt_val, &mt_obj);

    if (!name) {
      rc = cdd_json_value_free(mt_val);
      continue;
    }
    if (write_media_type_object(mt_obj, mt) != 0) {
      rc = cdd_json_value_free(mt_val);
      rc = cdd_json_value_free(media_val);
      return CDD_C_ERROR_MEMORY;
    }

    rc = cdd_json_object_set_value_safe(media_obj, name, mt_val);
  }

  rc = cdd_json_object_set_value_safe(components, "mediaTypes", media_val);
  return rc;
}

/**
 * @brief Writes component examples to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_component_examples(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *examples_val = NULL;
  JSON_Object *examples_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!spec || spec->n_component_examples == 0 || !spec->component_examples)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&examples_val);
  if (!examples_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(examples_val, &examples_obj);

  for (i = 0; i < spec->n_component_examples; ++i) {
    const char *name = spec->component_example_names[i];
    const struct OpenAPI_Example *ex = &spec->component_examples[i];
    JSON_Value *ex_val = NULL;
    JSON_Object *ex_obj = NULL;
    if (!name)
      continue;
    rc = cdd_json_value_init_object(&ex_val);
    rc = cdd_json_value_get_object(ex_val, &ex_obj);
    rc = write_example_object(ex_obj, ex);
    rc = cdd_json_object_set_value_safe(examples_obj, name, ex_val);
  }

  rc = cdd_json_object_set_value_safe(components, "examples", examples_val);
  return rc;
}

/**
 * @brief Writes component links to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_component_links(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *links_val = NULL;
  JSON_Object *links_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!spec || spec->n_component_links == 0 || !spec->component_links)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&links_val);
  if (!links_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(links_val, &links_obj);

  for (i = 0; i < spec->n_component_links; ++i) {
    const struct OpenAPI_Link *link = &spec->component_links[i];
    const char *name = link->name ? link->name : "link";
    JSON_Value *l_val = NULL;
    JSON_Object *l_obj = NULL;
    rc = cdd_json_value_init_object(&l_val);
    rc = cdd_json_value_get_object(l_val, &l_obj);

    rc = write_link_object(l_obj, link);
    rc = cdd_json_object_set_value_safe(links_obj, name, l_val);
  }

  rc = cdd_json_object_set_value_safe(components, "links", links_val);
  return rc;
}

/**
 * @brief Writes component callbacks to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_component_callbacks(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *cbs_val = NULL;
  JSON_Object *cbs_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!spec || spec->n_component_callbacks == 0 || !spec->component_callbacks)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&cbs_val);
  if (!cbs_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(cbs_val, &cbs_obj);

  for (i = 0; i < spec->n_component_callbacks; ++i) {
    const struct OpenAPI_Callback *cb = &spec->component_callbacks[i];
    const char *name = cb->name ? cb->name : "callback";
    JSON_Value *cb_val = NULL;
    JSON_Object *cb_obj = NULL;
    rc = cdd_json_value_init_object(&cb_val);
    rc = cdd_json_value_get_object(cb_val, &cb_obj);

    rc = write_callback_object(cb_obj, cb);
    rc = cdd_json_object_set_value_safe(cbs_obj, name, cb_val);
  }

  rc = cdd_json_object_set_value_safe(components, "callbacks", cbs_val);
  return rc;
}

/**
 * @brief Writes component path items to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_component_path_items(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *paths_val = NULL;
  JSON_Object *paths_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!spec || spec->n_component_path_items == 0)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&paths_val);
  if (!paths_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(paths_val, &paths_obj);

  for (i = 0; i < spec->n_component_path_items; ++i) {
    const struct OpenAPI_Path *p = &spec->component_path_items[i];
    const char *name = spec->component_path_item_names
                           ? spec->component_path_item_names[i]
                           : p->route;
    JSON_Value *item_val = NULL;
    JSON_Object *item_obj = NULL;
    rc = cdd_json_value_init_object(&item_val);
    rc = cdd_json_value_get_object(item_val, &item_obj);

    if (!name) {
      rc = cdd_json_value_free(item_val);
      continue;
    }

    rc = write_path_item_object(item_obj, p);
    if (rc != CDD_C_SUCCESS) {
      rc = (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(item_val));
      return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(paths_val));
    }

    rc = cdd_json_object_set_value_safe(paths_obj, name, item_val);
  }

  rc = cdd_json_object_set_value_safe(components, "pathItems", paths_val);
  return rc;
}

/**
 * @brief Writes component request bodies to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_component_request_bodies(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *rbs_val = NULL;
  JSON_Object *rbs_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (spec->n_component_request_bodies == 0)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&rbs_val);
  if (!rbs_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(rbs_val, &rbs_obj);

  for (i = 0; i < spec->n_component_request_bodies; ++i) {
    const char *name = spec->component_request_body_names[i];
    const struct OpenAPI_RequestBody *rb = &spec->component_request_bodies[i];
    JSON_Value *rb_val = NULL;
    JSON_Object *rb_obj = NULL;
    rc = cdd_json_value_init_object(&rb_val);
    rc = cdd_json_value_get_object(rb_val, &rb_obj);
    if (!name) {
      rc = cdd_json_value_free(rb_val);
      continue;
    }
    if (rb->ref) {
      rc = cdd_json_object_set_string(rb_obj, "$ref", rb->ref);
    } else {
      if (write_request_body_object(rb_obj, rb) != 0) {
        rc = cdd_json_value_free(rb_val);
        rc = cdd_json_value_free(rbs_val);
        return CDD_C_ERROR_MEMORY;
      }
    }
    rc = cdd_json_object_set_value_safe(rbs_obj, name, rb_val);
  }

  rc = cdd_json_object_set_value_safe(components, "requestBodies", rbs_val);
  return rc;
}

/**
 * @brief Writes components object to root JSON object.
 *
 * @param[in,out] root_obj Root JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_components(JSON_Object *root_obj,
                                            const struct OpenAPI_Spec *spec) {
  JSON_Value *comps_val = NULL;
  JSON_Object *comps_obj = NULL;
  cdd_c_error_t rc;

  /* Only create components block if there is something to write */
  if (spec->n_defined_schemas == 0 && spec->n_raw_schemas == 0 &&
      spec->n_security_schemes == 0 && spec->n_component_parameters == 0 &&
      spec->n_component_responses == 0 && spec->n_component_headers == 0 &&
      spec->n_component_request_bodies == 0 &&
      spec->n_component_media_types == 0 && spec->n_component_examples == 0 &&
      spec->n_component_links == 0 && spec->n_component_callbacks == 0 &&
      spec->n_component_path_items == 0 && !spec->components_extensions_json) {
    return CDD_C_SUCCESS;
  }

  rc = cdd_json_value_init_object(&comps_val);
  if (!comps_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(comps_val, &comps_obj);

  if (spec->components_extensions_json)
    rc = merge_schema_extras_object_openapi(comps_obj,
                                            spec->components_extensions_json);

  /* Schemas */
  if (spec->n_defined_schemas > 0 || spec->n_raw_schemas > 0) {
    JSON_Value *schemas_val = NULL;
    JSON_Object *schemas_obj = NULL;
    size_t i;
    rc = cdd_json_value_init_object(&schemas_val);
    rc = cdd_json_value_get_object(schemas_val, &schemas_obj);

    for (i = 0; i < spec->n_defined_schemas; ++i) {
      if (spec->defined_schema_names[i]) {
        rc = write_struct_to_json_schema(schemas_obj,
                                         spec->defined_schema_names[i],
                                         &spec->defined_schemas[i]);
        if (rc != CDD_C_SUCCESS) {
          rc = (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(comps_val));
          return (cdd_c_error_t)((int)rc |
                                 (int)cdd_json_value_free(schemas_val));
        }
      }
    }

    for (i = 0; i < spec->n_raw_schemas; ++i) {
      JSON_Value *raw_val = NULL;
      if (!spec->raw_schema_names[i] || !spec->raw_schema_json[i])
        continue;
      rc = cdd_json_parse_string(spec->raw_schema_json[i], &raw_val);
      if (!raw_val) {
        printf("FAILED parsing %s\n", spec->raw_schema_json[i]);
        rc = cdd_json_value_free(comps_val);
        rc = cdd_json_value_free(schemas_val);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      rc = cdd_json_object_set_value_safe(schemas_obj,
                                          spec->raw_schema_names[i], raw_val);
    }
    rc = cdd_json_object_set_value_safe(comps_obj, "schemas", schemas_val);
  }

  /* Security Schemes */
  if (spec->n_security_schemes > 0) {
    rc = write_security_schemes(comps_obj, spec);
    if (rc != CDD_C_SUCCESS) {
      return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(comps_val));
    }
  }

  /* Parameters */
  rc = write_component_parameters(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(comps_val));
  }

  /* Responses */
  rc = write_component_responses(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(comps_val));
  }

  /* Headers */
  rc = write_component_headers(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(comps_val));
  }

  /* Request Bodies */
  rc = write_component_request_bodies(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(comps_val));
  }

  /* Media Types */
  rc = write_component_media_types(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(comps_val));
  }

  /* Examples */
  rc = write_component_examples(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(comps_val));
  }

  /* Links */
  rc = write_component_links(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(comps_val));
  }

  /* Callbacks */
  rc = write_component_callbacks(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(comps_val));
  }

  /* Path Items */
  rc = write_component_path_items(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    return (cdd_c_error_t)((int)rc | (int)cdd_json_value_free(comps_val));
  }

  rc = cdd_json_object_set_value_safe(root_obj, "components", comps_val);
  return rc;
}
