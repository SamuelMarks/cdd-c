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

#include <parson.h>

#include "c_cdd/log.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include "openapi/emit/openapi.h"
/* clang-format on */

/**
 * @brief Write security schemes to components.
 */
C_CDD_EXPORT cdd_c_error_t write_security_schemes(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  char *_ast_oauth_flow_type_to_str_22 = NULL;
  JSON_Value *sec_val;
  JSON_Object *sec_obj;
  size_t i;

  if (spec->n_security_schemes == 0)
    return CDD_C_SUCCESS;

  sec_val = json_value_init_object();
  if (!sec_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  sec_obj = json_value_get_object(sec_val);

  for (i = 0; i < spec->n_security_schemes; ++i) {
    const struct OpenAPI_SecurityScheme *s = &spec->security_schemes[i];
    JSON_Value *s_val = json_value_init_object();
    JSON_Object *s_obj = json_value_get_object(s_val);

    if (s->description)
      json_object_set_string(s_obj, "description", s->description);
    if (s->deprecated_set)
      json_object_set_boolean(s_obj, "deprecated", s->deprecated ? 1 : 0);

    switch (s->type) {
    case OA_SEC_APIKEY:
      json_object_set_string(s_obj, "type", "apiKey");
      if (s->in == OA_SEC_IN_HEADER)
        json_object_set_string(s_obj, "in", "header");
      else if (s->in == OA_SEC_IN_QUERY)
        json_object_set_string(s_obj, "in", "query");
      else if (s->in == OA_SEC_IN_COOKIE)
        json_object_set_string(s_obj, "in", "cookie");

      if (s->key_name)
        json_object_set_string(s_obj, "name", s->key_name);
      break;

    case OA_SEC_HTTP:
      json_object_set_string(s_obj, "type", "http");
      if (s->scheme)
        json_object_set_string(s_obj, "scheme", s->scheme);
      if (s->scheme && strcmp(s->scheme, "bearer") == 0) {
        if (s->bearer_format) {
          json_object_set_string(s_obj, "bearerFormat", s->bearer_format);
        } else {
          json_object_set_string(s_obj, "bearerFormat", "JWT"); /* Common */
        }
      }
      break;

    case OA_SEC_MUTUALTLS:
      json_object_set_string(s_obj, "type", "mutualTLS");
      break;

    case OA_SEC_OAUTH2:
      json_object_set_string(s_obj, "type", "oauth2");
      if (s->oauth2_metadata_url) {
        json_object_set_string(s_obj, "oauth2MetadataUrl",
                               s->oauth2_metadata_url);
      }
      if (s->flows && s->n_flows > 0) {
        JSON_Value *flows_val = json_value_init_object();
        JSON_Object *flows_obj = json_value_get_object(flows_val);
        size_t f;
        for (f = 0; f < s->n_flows; ++f) {
          const struct OpenAPI_OAuthFlow *flow = &s->flows[f];
          const char *flow_key =
              (oauth_flow_type_to_str_openapi(flow->type,
                                              &_ast_oauth_flow_type_to_str_22),
               _ast_oauth_flow_type_to_str_22);
          JSON_Value *flow_val;
          JSON_Object *flow_obj;
          JSON_Value *scopes_val;
          JSON_Object *scopes_obj;
          size_t sc;
          if (!flow_key)
            continue;
          flow_val = json_value_init_object();
          flow_obj = json_value_get_object(flow_val);
          if (flow->authorization_url)
            json_object_set_string(flow_obj, "authorizationUrl",
                                   flow->authorization_url);
          if (flow->token_url)
            json_object_set_string(flow_obj, "tokenUrl", flow->token_url);
          if (flow->refresh_url)
            json_object_set_string(flow_obj, "refreshUrl", flow->refresh_url);
          if (flow->device_authorization_url)
            json_object_set_string(flow_obj, "deviceAuthorizationUrl",
                                   flow->device_authorization_url);
          scopes_val = json_value_init_object();
          scopes_obj = json_value_get_object(scopes_val);
          if (flow->scopes) {
            for (sc = 0; sc < flow->n_scopes; ++sc) {
              const char *scope_name = flow->scopes[sc].name;
              const char *scope_desc = flow->scopes[sc].description;
              if (scope_name)
                json_object_set_string(scopes_obj, scope_name,
                                       scope_desc ? scope_desc : "");
            }
          }
          json_object_set_value(flow_obj, "scopes", scopes_val);
          if (flow->extensions_json)
            merge_schema_extras_object_openapi(flow_obj, flow->extensions_json);
          json_object_set_value(flows_obj, flow_key, flow_val);
        }
        json_object_set_value(s_obj, "flows", flows_val);
      }
      break;

    case OA_SEC_OPENID:
      json_object_set_string(s_obj, "type", "openIdConnect");
      if (s->open_id_connect_url) {
        json_object_set_string(s_obj, "openIdConnectUrl",
                               s->open_id_connect_url);
      }
      break;

    default:
      break;
    }

    if (s->extensions_json)
      merge_schema_extras_object_openapi(s_obj, s->extensions_json);

    json_object_set_value(sec_obj, s->name ? s->name : "unknown", s_val);
  }

  json_object_set_value(components, "securitySchemes", sec_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write component parameters.
 */
C_CDD_EXPORT cdd_c_error_t write_component_parameters(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *params_val;
  JSON_Object *params_obj;
  size_t i;

  if (!spec || spec->n_component_parameters == 0)
    return CDD_C_SUCCESS;

  params_val = json_value_init_object();
  if (!params_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  params_obj = json_value_get_object(params_val);

  for (i = 0; i < spec->n_component_parameters; ++i) {
    const char *name = spec->component_parameter_names[i];
    const struct OpenAPI_Parameter *param = &spec->component_parameters[i];
    JSON_Value *p_val = json_value_init_object();
    JSON_Object *p_obj = json_value_get_object(p_val);
    if (!name) {
      json_value_free(p_val);
      continue;
    }
    write_parameter_object(p_obj, param);
    json_object_set_value(params_obj, name, p_val);
  }

  json_object_set_value(components, "parameters", params_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write component responses.
 */
C_CDD_EXPORT cdd_c_error_t write_component_responses(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *resp_val;
  JSON_Object *resp_obj;
  size_t i;

  if (!spec || spec->n_component_responses == 0)
    return CDD_C_SUCCESS;

  resp_val = json_value_init_object();
  if (!resp_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  resp_obj = json_value_get_object(resp_val);

  for (i = 0; i < spec->n_component_responses; ++i) {
    const char *name = spec->component_response_names[i];
    const struct OpenAPI_Response *resp = &spec->component_responses[i];
    JSON_Value *r_val = json_value_init_object();
    JSON_Object *r_obj = json_value_get_object(r_val);
    if (!name) {
      json_value_free(r_val);
      continue;
    }
    write_response_object(r_obj, resp);
    json_object_set_value(resp_obj, name, r_val);
  }

  json_object_set_value(components, "responses", resp_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write component headers.
 */
C_CDD_EXPORT cdd_c_error_t write_component_headers(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *hdrs_val;
  JSON_Object *hdrs_obj;
  size_t i;

  if (!spec || spec->n_component_headers == 0)
    return CDD_C_SUCCESS;

  hdrs_val = json_value_init_object();
  if (!hdrs_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  hdrs_obj = json_value_get_object(hdrs_val);

  for (i = 0; i < spec->n_component_headers; ++i) {
    const char *name = spec->component_header_names[i];
    const struct OpenAPI_Header *hdr = &spec->component_headers[i];
    JSON_Value *h_val = json_value_init_object();
    JSON_Object *h_obj = json_value_get_object(h_val);
    if (!name) {
      json_value_free(h_val);
      continue;
    }
    write_header_object(h_obj, hdr);
    json_object_set_value(hdrs_obj, name, h_val);
  }

  json_object_set_value(components, "headers", hdrs_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write component media types.
 */
C_CDD_EXPORT cdd_c_error_t write_component_media_types(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *media_val;
  JSON_Object *media_obj;
  size_t i;

  if (!spec || spec->n_component_media_types == 0)
    return CDD_C_SUCCESS;

  media_val = json_value_init_object();
  if (!media_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  media_obj = json_value_get_object(media_val);

  for (i = 0; i < spec->n_component_media_types; ++i) {
    const char *name = spec->component_media_type_names[i];
    const struct OpenAPI_MediaType *mt = &spec->component_media_types[i];
    JSON_Value *mt_val = json_value_init_object();
    JSON_Object *mt_obj = json_value_get_object(mt_val);

    if (!name) {
      json_value_free(mt_val);
      continue;
    }
    if (write_media_type_object(mt_obj, mt) != 0) {
      json_value_free(mt_val);
      return CDD_C_ERROR_MEMORY;
    }

    json_object_set_value(media_obj, name, mt_val);
  }

  json_object_set_value(components, "mediaTypes", media_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write component examples.
 */
C_CDD_EXPORT cdd_c_error_t write_component_examples(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *examples_val;
  JSON_Object *examples_obj;
  size_t i;

  if (!spec || spec->n_component_examples == 0 || !spec->component_examples)
    return CDD_C_SUCCESS;

  examples_val = json_value_init_object();
  if (!examples_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  examples_obj = json_value_get_object(examples_val);

  for (i = 0; i < spec->n_component_examples; ++i) {
    const char *name = spec->component_example_names[i];
    const struct OpenAPI_Example *ex = &spec->component_examples[i];
    JSON_Value *ex_val;
    JSON_Object *ex_obj;
    if (!name)
      continue;
    ex_val = json_value_init_object();
    ex_obj = json_value_get_object(ex_val);
    write_example_object(ex_obj, ex);
    json_object_set_value(examples_obj, name, ex_val);
  }

  json_object_set_value(components, "examples", examples_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write component links.
 */
C_CDD_EXPORT cdd_c_error_t write_component_links(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *links_val;
  JSON_Object *links_obj;
  size_t i;

  if (!spec || spec->n_component_links == 0 || !spec->component_links)
    return CDD_C_SUCCESS;

  links_val = json_value_init_object();
  if (!links_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  links_obj = json_value_get_object(links_val);

  for (i = 0; i < spec->n_component_links; ++i) {
    const struct OpenAPI_Link *link = &spec->component_links[i];
    const char *name = link->name ? link->name : "link";
    JSON_Value *l_val = json_value_init_object();
    JSON_Object *l_obj = json_value_get_object(l_val);

    write_link_object(l_obj, link);
    json_object_set_value(links_obj, name, l_val);
  }

  json_object_set_value(components, "links", links_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write component callbacks.
 */
C_CDD_EXPORT cdd_c_error_t write_component_callbacks(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *cbs_val;
  JSON_Object *cbs_obj;
  size_t i;

  if (!spec || spec->n_component_callbacks == 0 || !spec->component_callbacks)
    return CDD_C_SUCCESS;

  cbs_val = json_value_init_object();
  if (!cbs_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  cbs_obj = json_value_get_object(cbs_val);

  for (i = 0; i < spec->n_component_callbacks; ++i) {
    const struct OpenAPI_Callback *cb = &spec->component_callbacks[i];
    const char *name = cb->name ? cb->name : "callback";
    JSON_Value *cb_val = json_value_init_object();
    JSON_Object *cb_obj = json_value_get_object(cb_val);

    write_callback_object(cb_obj, cb);
    json_object_set_value(cbs_obj, name, cb_val);
  }

  json_object_set_value(components, "callbacks", cbs_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write component path items.
 */
C_CDD_EXPORT cdd_c_error_t write_component_path_items(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *paths_val;
  JSON_Object *paths_obj;
  size_t i;
  cdd_c_error_t rc;

  if (!spec || spec->n_component_path_items == 0)
    return CDD_C_SUCCESS;

  paths_val = json_value_init_object();
  if (!paths_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  paths_obj = json_value_get_object(paths_val);

  for (i = 0; i < spec->n_component_path_items; ++i) {
    const struct OpenAPI_Path *p = &spec->component_path_items[i];
    const char *name = spec->component_path_item_names
                           ? spec->component_path_item_names[i]
                           : p->route;
    JSON_Value *item_val = json_value_init_object();
    JSON_Object *item_obj = json_value_get_object(item_val);

    if (!name) {
      json_value_free(item_val);
      continue;
    }

    rc = write_path_item_object(item_obj, p);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(item_val);
      json_value_free(paths_val);
      return rc;
    }

    json_object_set_value(paths_obj, name, item_val);
  }

  json_object_set_value(components, "pathItems", paths_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write component request bodies.
 */
C_CDD_EXPORT cdd_c_error_t write_component_request_bodies(
    JSON_Object *components, const struct OpenAPI_Spec *spec) {
  JSON_Value *rbs_val;
  JSON_Object *rbs_obj;
  size_t i;

  if (spec->n_component_request_bodies == 0)
    return CDD_C_SUCCESS;

  rbs_val = json_value_init_object();
  if (!rbs_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rbs_obj = json_value_get_object(rbs_val);

  for (i = 0; i < spec->n_component_request_bodies; ++i) {
    const char *name = spec->component_request_body_names[i];
    const struct OpenAPI_RequestBody *rb = &spec->component_request_bodies[i];
    JSON_Value *rb_val = json_value_init_object();
    JSON_Object *rb_obj = json_value_get_object(rb_val);
    if (!name) {
      json_value_free(rb_val);
      continue;
    }
    if (rb->ref) {
      json_object_set_string(rb_obj, "$ref", rb->ref);
    } else {
      if (write_request_body_object(rb_obj, rb) != 0) {
        json_value_free(rb_val);
        json_value_free(rbs_val);
        return CDD_C_ERROR_MEMORY;
      }
    }
    json_object_set_value(rbs_obj, name, rb_val);
  }

  json_object_set_value(components, "requestBodies", rbs_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write components.
 */
C_CDD_EXPORT cdd_c_error_t write_components(JSON_Object *root_obj,
                                            const struct OpenAPI_Spec *spec) {
  JSON_Value *comps_val;
  JSON_Object *comps_obj;
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

  comps_val = json_value_init_object();
  if (!comps_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  comps_obj = json_value_get_object(comps_val);

  if (spec->components_extensions_json)
    merge_schema_extras_object_openapi(comps_obj,
                                       spec->components_extensions_json);

  /* Schemas */
  if (spec->n_defined_schemas > 0 || spec->n_raw_schemas > 0) {
    JSON_Value *schemas_val = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(schemas_val);
    size_t i;

    for (i = 0; i < spec->n_defined_schemas; ++i) {
      if (spec->defined_schema_names[i]) {
        rc = write_struct_to_json_schema(schemas_obj,
                                         spec->defined_schema_names[i],
                                         &spec->defined_schemas[i]);
        if (rc != CDD_C_SUCCESS) {
          json_value_free(comps_val);
          json_value_free(schemas_val);
          return rc;
        }
      }
    }

    for (i = 0; i < spec->n_raw_schemas; ++i) {
      JSON_Value *raw_val;
      if (!spec->raw_schema_names[i] || !spec->raw_schema_json[i])
        continue;
      raw_val = json_parse_string(spec->raw_schema_json[i]);
      if (!raw_val) {
        printf("FAILED parsing %s\n", spec->raw_schema_json[i]);
        json_value_free(comps_val);
        json_value_free(schemas_val);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      json_object_set_value(schemas_obj, spec->raw_schema_names[i], raw_val);
    }
    json_object_set_value(comps_obj, "schemas", schemas_val);
  }

  /* Security Schemes */
  if (spec->n_security_schemes > 0) {
    rc = write_security_schemes(comps_obj, spec);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(comps_val);
      return rc;
    }
  }

  /* Parameters */
  rc = write_component_parameters(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(comps_val);
    return rc;
  }

  /* Responses */
  rc = write_component_responses(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(comps_val);
    return rc;
  }

  /* Headers */
  rc = write_component_headers(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(comps_val);
    return rc;
  }

  /* Request Bodies */
  rc = write_component_request_bodies(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(comps_val);
    return rc;
  }

  /* Media Types */
  rc = write_component_media_types(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(comps_val);
    return rc;
  }

  /* Examples */
  rc = write_component_examples(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(comps_val);
    return rc;
  }

  /* Links */
  rc = write_component_links(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(comps_val);
    return rc;
  }

  /* Callbacks */
  rc = write_component_callbacks(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(comps_val);
    return rc;
  }

  /* Path Items */
  rc = write_component_path_items(comps_obj, spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(comps_val);
    return rc;
  }

  json_object_set_value(root_obj, "components", comps_val);
  return CDD_C_SUCCESS;
}
