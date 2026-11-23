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
 * @brief Serializes a single `OpenAPI_OAuthFlow` into `flows_obj`.
 *
 * @param[in,out] flows_obj Parent flows JSON object.
 * @param[in] flow OAuth flow definition.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t
write_single_oauth_flow(JSON_Object *flows_obj,
                        const struct OpenAPI_OAuthFlow *flow) {
  const char *flow_key = NULL;
  JSON_Value *flow_val = NULL;
  JSON_Object *flow_obj = NULL;
  JSON_Value *scopes_val = NULL;
  JSON_Object *scopes_obj = NULL;
  size_t sc;
  cdd_c_error_t rc;

  rc = oauth_flow_type_to_str_openapi(flow->type, &flow_key);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!flow_key)
    return CDD_C_SUCCESS;

  rc = oa_emit_new_object(&flow_val, &flow_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = openapi_emit_reflected_strings(flow_obj, &OpenAPI_OAuthFlow_type_info,
                                      flow);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_flow;

  rc = oa_emit_new_object(&scopes_val, &scopes_obj);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_flow;

  if (flow->scopes) {
    for (sc = 0; sc < flow->n_scopes; ++sc) {
      const struct OpenAPI_OAuthScope *scope = &flow->scopes[sc];
      if (scope->name) {
        rc = cdd_json_object_set_string(scopes_obj, scope->name,
                                        scope->description ? scope->description
                                                           : "");
        if (rc != CDD_C_SUCCESS) {
          cdd_c_error_t f1 = cdd_json_value_free(scopes_val);
          cdd_c_error_t f2 = cdd_json_value_free(flow_val);
          return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
        }
      }
    }
  }
  rc = cdd_json_object_set_value_safe(flow_obj, "scopes", scopes_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_flow;

  if (flow->extensions_json) {
    rc = merge_schema_extras_object_openapi(flow_obj, flow->extensions_json);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_flow;
  }
  return cdd_json_object_set_value_safe(flows_obj, flow_key, flow_val);

cleanup_flow: { return cdd_c_keep_err(rc, cdd_json_value_free(flow_val)); }
}

/**
 * @brief Serializes a single `OpenAPI_SecurityScheme` into `s_obj`.
 *
 * @param[in,out] s_obj Target security scheme JSON object.
 * @param[in] s Source security scheme structure.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t
write_single_security_scheme(JSON_Object *s_obj,
                             const struct OpenAPI_SecurityScheme *s) {
  cdd_c_error_t rc;

  rc = openapi_emit_reflected_strings(s_obj, &OpenAPI_SecurityScheme_type_info,
                                      s);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (s->deprecated_set) {
    rc =
        cdd_json_object_set_boolean(s_obj, "deprecated", s->deprecated ? 1 : 0);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  switch (s->type) {
  case OA_SEC_APIKEY: {
    rc = cdd_json_object_set_string(s_obj, "type", "apiKey");
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (s->in == OA_SEC_IN_HEADER) {
      rc = cdd_json_object_set_string(s_obj, "in", "header");
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (s->in == OA_SEC_IN_QUERY) {
      rc = cdd_json_object_set_string(s_obj, "in", "query");
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (s->in == OA_SEC_IN_COOKIE) {
      rc = cdd_json_object_set_string(s_obj, "in", "cookie");
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (s->key_name) {
      rc = cdd_json_object_set_string(s_obj, "name", s->key_name);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    break;
  }

  case OA_SEC_HTTP: {
    rc = cdd_json_object_set_string(s_obj, "type", "http");
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (s->scheme) {
      rc = cdd_json_object_set_string(s_obj, "scheme", s->scheme);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (s->scheme && strcmp(s->scheme, "bearer") == 0 && !s->bearer_format) {
      rc = cdd_json_object_set_string(s_obj, "bearerFormat", "JWT");
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    break;
  }

  case OA_SEC_MUTUALTLS: {
    rc = cdd_json_object_set_string(s_obj, "type", "mutualTLS");
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  }

  case OA_SEC_OAUTH2: {
    rc = cdd_json_object_set_string(s_obj, "type", "oauth2");
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (s->flows && s->n_flows > 0) {
      JSON_Value *flows_val = NULL;
      JSON_Object *flows_obj = NULL;
      size_t f;
      rc = oa_emit_new_object(&flows_val, &flows_obj);
      if (rc != CDD_C_SUCCESS)
        return rc;
      for (f = 0; f < s->n_flows; ++f) {
        rc = write_single_oauth_flow(flows_obj, &s->flows[f]);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc, cdd_json_value_free(flows_val));
        }
      }
      rc = cdd_json_object_set_value_safe(s_obj, "flows", flows_val);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    break;
  }

  case OA_SEC_OPENID: {
    rc = cdd_json_object_set_string(s_obj, "type", "openIdConnect");
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  }

  default:
    break;
  }

  if (s->extensions_json) {
    return merge_schema_extras_object_openapi(s_obj, s->extensions_json);
  }
  return CDD_C_SUCCESS;
}

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

  rc = oa_emit_new_object(&sec_val, &sec_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < spec->n_security_schemes; ++i) {
    const struct OpenAPI_SecurityScheme *s = &spec->security_schemes[i];
    JSON_Value *s_val = NULL;
    JSON_Object *s_obj = NULL;
    rc = oa_emit_new_object(&s_val, &s_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(sec_val));
    }

    rc = write_single_security_scheme(s_obj, s);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(s_val);
      cdd_c_error_t f2 = cdd_json_value_free(sec_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }

    rc = cdd_json_object_set_value_safe(sec_obj, s->name ? s->name : "unknown",
                                        s_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(sec_val));
    }
  }

  return cdd_json_object_set_value_safe(components, "securitySchemes", sec_val);
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

  rc = oa_emit_new_object(&params_val, &params_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < spec->n_component_parameters; ++i) {
    const char *name = spec->component_parameter_names[i];
    const struct OpenAPI_Parameter *param = &spec->component_parameters[i];
    JSON_Value *p_val = NULL;
    JSON_Object *p_obj = NULL;
    if (!name)
      continue;
    rc = oa_emit_new_object(&p_val, &p_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(params_val));
    }
    rc = write_parameter_object(p_obj, param);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(p_val);
      cdd_c_error_t f2 = cdd_json_value_free(params_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }
    rc = cdd_json_object_set_value_safe(params_obj, name, p_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(params_val));
    }
  }

  return cdd_json_object_set_value_safe(components, "parameters", params_val);
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

  rc = oa_emit_new_object(&resp_val, &resp_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < spec->n_component_responses; ++i) {
    const char *name = spec->component_response_names[i];
    const struct OpenAPI_Response *resp = &spec->component_responses[i];
    JSON_Value *r_val = NULL;
    JSON_Object *r_obj = NULL;
    if (!name)
      continue;
    rc = oa_emit_new_object(&r_val, &r_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(resp_val));
    }
    rc = write_response_object(r_obj, resp);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(r_val);
      cdd_c_error_t f2 = cdd_json_value_free(resp_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }
    rc = cdd_json_object_set_value_safe(resp_obj, name, r_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(resp_val));
    }
  }

  return cdd_json_object_set_value_safe(components, "responses", resp_val);
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

  rc = oa_emit_new_object(&hdrs_val, &hdrs_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < spec->n_component_headers; ++i) {
    const char *name = spec->component_header_names[i];
    const struct OpenAPI_Header *hdr = &spec->component_headers[i];
    JSON_Value *h_val = NULL;
    JSON_Object *h_obj = NULL;
    if (!name)
      continue;
    rc = oa_emit_new_object(&h_val, &h_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(hdrs_val));
    }
    rc = write_header_object(h_obj, hdr);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(h_val);
      cdd_c_error_t f2 = cdd_json_value_free(hdrs_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }
    rc = cdd_json_object_set_value_safe(hdrs_obj, name, h_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(hdrs_val));
    }
  }

  return cdd_json_object_set_value_safe(components, "headers", hdrs_val);
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

  rc = oa_emit_new_object(&media_val, &media_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < spec->n_component_media_types; ++i) {
    const char *name = spec->component_media_type_names[i];
    const struct OpenAPI_MediaType *mt = &spec->component_media_types[i];
    JSON_Value *mt_val = NULL;
    JSON_Object *mt_obj = NULL;
    if (!name)
      continue;
    rc = oa_emit_new_object(&mt_val, &mt_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(media_val));
    }
    rc = write_media_type_object(mt_obj, mt);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(mt_val);
      cdd_c_error_t f2 = cdd_json_value_free(media_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }

    rc = cdd_json_object_set_value_safe(media_obj, name, mt_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(media_val));
    }
  }

  return cdd_json_object_set_value_safe(components, "mediaTypes", media_val);
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

  rc = oa_emit_new_object(&examples_val, &examples_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < spec->n_component_examples; ++i) {
    const char *name = spec->component_example_names[i];
    const struct OpenAPI_Example *ex = &spec->component_examples[i];
    JSON_Value *ex_val = NULL;
    JSON_Object *ex_obj = NULL;
    if (!name)
      continue;
    rc = oa_emit_new_object(&ex_val, &ex_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(examples_val));
    }
    rc = write_example_object(ex_obj, ex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(ex_val);
      cdd_c_error_t f2 = cdd_json_value_free(examples_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }
    rc = cdd_json_object_set_value_safe(examples_obj, name, ex_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(examples_val));
    }
  }

  return cdd_json_object_set_value_safe(components, "examples", examples_val);
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

  rc = oa_emit_new_object(&links_val, &links_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < spec->n_component_links; ++i) {
    const struct OpenAPI_Link *link = &spec->component_links[i];
    const char *name = link->name ? link->name : "link";
    JSON_Value *l_val = NULL;
    JSON_Object *l_obj = NULL;
    rc = oa_emit_new_object(&l_val, &l_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(links_val));
    }

    rc = write_link_object(l_obj, link);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(l_val);
      cdd_c_error_t f2 = cdd_json_value_free(links_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }
    rc = cdd_json_object_set_value_safe(links_obj, name, l_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(links_val));
    }
  }

  return cdd_json_object_set_value_safe(components, "links", links_val);
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

  rc = oa_emit_new_object(&cbs_val, &cbs_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < spec->n_component_callbacks; ++i) {
    const struct OpenAPI_Callback *cb = &spec->component_callbacks[i];
    const char *name = cb->name ? cb->name : "callback";
    JSON_Value *cb_val = NULL;
    JSON_Object *cb_obj = NULL;
    rc = oa_emit_new_object(&cb_val, &cb_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(cbs_val));
    }

    rc = write_callback_object(cb_obj, cb);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(cb_val);
      cdd_c_error_t f2 = cdd_json_value_free(cbs_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }
    rc = cdd_json_object_set_value_safe(cbs_obj, name, cb_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(cbs_val));
    }
  }

  return cdd_json_object_set_value_safe(components, "callbacks", cbs_val);
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

  rc = oa_emit_new_object(&paths_val, &paths_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < spec->n_component_path_items; ++i) {
    const struct OpenAPI_Path *p = &spec->component_path_items[i];
    const char *name = spec->component_path_item_names
                           ? spec->component_path_item_names[i]
                           : p->route;
    JSON_Value *item_val = NULL;
    JSON_Object *item_obj = NULL;
    if (!name)
      continue;
    rc = oa_emit_new_object(&item_val, &item_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(paths_val));
    }

    rc = write_path_item_object(item_obj, p);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(item_val);
      cdd_c_error_t f2 = cdd_json_value_free(paths_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }

    rc = cdd_json_object_set_value_safe(paths_obj, name, item_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(paths_val));
    }
  }

  return cdd_json_object_set_value_safe(components, "pathItems", paths_val);
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

  if (!spec || spec->n_component_request_bodies == 0)
    return CDD_C_SUCCESS;

  rc = oa_emit_new_object(&rbs_val, &rbs_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < spec->n_component_request_bodies; ++i) {
    const char *name = spec->component_request_body_names[i];
    const struct OpenAPI_RequestBody *rb = &spec->component_request_bodies[i];
    JSON_Value *rb_val = NULL;
    JSON_Object *rb_obj = NULL;
    if (!name)
      continue;
    rc = oa_emit_new_object(&rb_val, &rb_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(rbs_val));
    }
    if (rb->ref) {
      rc = cdd_json_object_set_string(rb_obj, "$ref", rb->ref);
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t f1 = cdd_json_value_free(rb_val);
        cdd_c_error_t f2 = cdd_json_value_free(rbs_val);
        return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
      }
    } else {
      rc = write_request_body_object(rb_obj, rb);
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t f1 = cdd_json_value_free(rb_val);
        cdd_c_error_t f2 = cdd_json_value_free(rbs_val);
        return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
      }
    }
    rc = cdd_json_object_set_value_safe(rbs_obj, name, rb_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(rbs_val));
    }
  }

  return cdd_json_object_set_value_safe(components, "requestBodies", rbs_val);
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

  if (spec->n_defined_schemas == 0 && spec->n_raw_schemas == 0 &&
      spec->n_security_schemes == 0 && spec->n_component_parameters == 0 &&
      spec->n_component_responses == 0 && spec->n_component_headers == 0 &&
      spec->n_component_request_bodies == 0 &&
      spec->n_component_media_types == 0 && spec->n_component_examples == 0 &&
      spec->n_component_links == 0 && spec->n_component_callbacks == 0 &&
      spec->n_component_path_items == 0 && !spec->components_extensions_json) {
    return CDD_C_SUCCESS;
  }

  rc = oa_emit_new_object(&comps_val, &comps_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (spec->components_extensions_json) {
    rc = merge_schema_extras_object_openapi(comps_obj,
                                            spec->components_extensions_json);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_comps;
  }

  /* Schemas */
  if (spec->n_defined_schemas > 0 || spec->n_raw_schemas > 0) {
    JSON_Value *schemas_val = NULL;
    JSON_Object *schemas_obj = NULL;
    size_t i;
    rc = oa_emit_new_object(&schemas_val, &schemas_obj);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_comps;

    for (i = 0; i < spec->n_defined_schemas; ++i) {
      if (spec->defined_schema_names[i]) {
        rc = write_struct_to_json_schema(schemas_obj,
                                         spec->defined_schema_names[i],
                                         &spec->defined_schemas[i]);
        if (rc != CDD_C_SUCCESS) {
          cdd_c_error_t f1 = cdd_json_value_free(schemas_val);
          cdd_c_error_t f2 = cdd_json_value_free(comps_val);
          return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
        }
        if (spec->defined_schema_ids && spec->defined_schema_ids[i]) {
          JSON_Object *so = NULL;
          rc = cdd_json_object_get_object(schemas_obj,
                                          spec->defined_schema_names[i], &so);
          if (rc != CDD_C_SUCCESS) {
            cdd_c_error_t f1 = cdd_json_value_free(schemas_val);
            cdd_c_error_t f2 = cdd_json_value_free(comps_val);
            return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
          }
          rc = cdd_json_object_set_string(so, "$id",
                                          spec->defined_schema_ids[i]);
          if (rc != CDD_C_SUCCESS) {
            cdd_c_error_t f1 = cdd_json_value_free(schemas_val);
            cdd_c_error_t f2 = cdd_json_value_free(comps_val);
            return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
          }
        }
        if (spec->defined_schema_anchors && spec->defined_schema_anchors[i]) {
          JSON_Object *so = NULL;
          rc = cdd_json_object_get_object(schemas_obj,
                                          spec->defined_schema_names[i], &so);
          if (rc != CDD_C_SUCCESS) {
            cdd_c_error_t f1 = cdd_json_value_free(schemas_val);
            cdd_c_error_t f2 = cdd_json_value_free(comps_val);
            return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
          }
          rc = cdd_json_object_set_string(so, "$anchor",
                                          spec->defined_schema_anchors[i]);
          if (rc != CDD_C_SUCCESS) {
            cdd_c_error_t f1 = cdd_json_value_free(schemas_val);
            cdd_c_error_t f2 = cdd_json_value_free(comps_val);
            return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
          }
        }
        if (spec->defined_schema_dynamic_anchors &&
            spec->defined_schema_dynamic_anchors[i]) {
          JSON_Object *so = NULL;
          rc = cdd_json_object_get_object(schemas_obj,
                                          spec->defined_schema_names[i], &so);
          if (rc != CDD_C_SUCCESS) {
            cdd_c_error_t f1 = cdd_json_value_free(schemas_val);
            cdd_c_error_t f2 = cdd_json_value_free(comps_val);
            return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
          }
          rc = cdd_json_object_set_string(
              so, "$dynamicAnchor", spec->defined_schema_dynamic_anchors[i]);
          if (rc != CDD_C_SUCCESS) {
            cdd_c_error_t f1 = cdd_json_value_free(schemas_val);
            cdd_c_error_t f2 = cdd_json_value_free(comps_val);
            return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
          }
        }
      }
    }

    for (i = 0; i < spec->n_raw_schemas; ++i) {
      JSON_Value *raw_val = NULL;
      if (!spec->raw_schema_names[i] || !spec->raw_schema_json[i])
        continue;
      rc = cdd_json_parse_string(spec->raw_schema_json[i], &raw_val);
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t f1 = cdd_json_value_free(schemas_val);
        cdd_c_error_t f2 = cdd_json_value_free(comps_val);
        return cdd_c_keep_err(
            rc == CDD_C_ERROR_MEMORY ? rc : CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_c_keep_err(f1, f2));
      }
      rc = cdd_json_object_set_value_safe(schemas_obj,
                                          spec->raw_schema_names[i], raw_val);
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t f1 = cdd_json_value_free(schemas_val);
        cdd_c_error_t f2 = cdd_json_value_free(comps_val);
        return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
      }
    }
    rc = cdd_json_object_set_value_safe(comps_obj, "schemas", schemas_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_comps;
  }

  if (spec->n_security_schemes > 0) {
    rc = write_security_schemes(comps_obj, spec);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_comps;
  }

  rc = write_component_parameters(comps_obj, spec);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_comps;

  rc = write_component_responses(comps_obj, spec);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_comps;

  rc = write_component_headers(comps_obj, spec);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_comps;

  rc = write_component_request_bodies(comps_obj, spec);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_comps;

  rc = write_component_media_types(comps_obj, spec);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_comps;

  rc = write_component_examples(comps_obj, spec);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_comps;

  rc = write_component_links(comps_obj, spec);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_comps;

  rc = write_component_callbacks(comps_obj, spec);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_comps;

  rc = write_component_path_items(comps_obj, spec);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_comps;

  return cdd_json_object_set_value_safe(root_obj, "components", comps_val);

cleanup_comps: { return cdd_c_keep_err(rc, cdd_json_value_free(comps_val)); }
}
