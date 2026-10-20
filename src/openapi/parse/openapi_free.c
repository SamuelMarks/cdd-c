/**
 * @file openapi_free.c
 * @brief Specification and path cleanup routines for OpenAPI spec.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Free servers array.
 */
void openapi_free_servers_array(struct OpenAPI_Server *servers,
                                size_t n_servers) {
  size_t i;
  if (!servers)
    return;
  for (i = 0; i < n_servers; ++i) {
    if (servers[i].url)
      C_CDD_FREE(servers[i].url);
    if (servers[i].description)
      C_CDD_FREE(servers[i].description);
    if (servers[i].name)
      C_CDD_FREE(servers[i].name);
    if (servers[i].extensions_json)
      C_CDD_FREE(servers[i].extensions_json);
    if (servers[i].variables) {
      size_t v;
      for (v = 0; v < servers[i].n_variables; ++v) {
        struct OpenAPI_ServerVariable *var = &servers[i].variables[v];
        if (var->name)
          C_CDD_FREE(var->name);
        if (var->default_value)
          C_CDD_FREE(var->default_value);
        if (var->description)
          C_CDD_FREE(var->description);
        if (var->extensions_json)
          C_CDD_FREE(var->extensions_json);
        if (var->enum_values) {
          size_t e;
          for (e = 0; e < var->n_enum_values; ++e) {
            if (var->enum_values[e])
              C_CDD_FREE(var->enum_values[e]);
          }
          C_CDD_FREE(var->enum_values);
        }
      }
      C_CDD_FREE(servers[i].variables);
    }
  }
  C_CDD_FREE(servers);
}

/**
 * @brief Frees the memory associated with path item.
 */
void free_path_item(struct OpenAPI_Path *p) {
  size_t i;
  if (!p)
    return;
  if (p->route)
    C_CDD_FREE(p->route);
  if (p->ref)
    C_CDD_FREE(p->ref);
  if (p->summary)
    C_CDD_FREE(p->summary);
  if (p->description)
    C_CDD_FREE(p->description);
  if (p->extensions_json)
    C_CDD_FREE(p->extensions_json);
  if (p->parameters) {
    for (i = 0; i < p->n_parameters; ++i) {
      free_parameter(&p->parameters[i]);
    }
    C_CDD_FREE(p->parameters);
  }
  if (p->servers) {
    openapi_free_servers_array(p->servers, p->n_servers);
    p->servers = NULL;
    p->n_servers = 0;
  }
  if (p->operations) {
    for (i = 0; i < p->n_operations; ++i) {
      free_operation(&p->operations[i]);
    }
    C_CDD_FREE(p->operations);
  }
  if (p->additional_operations) {
    for (i = 0; i < p->n_additional_operations; ++i) {
      free_operation(&p->additional_operations[i]);
    }
    C_CDD_FREE(p->additional_operations);
  }
  memset(p, 0, sizeof(*p));
}

/**
 * @brief Frees the memory associated with callback.
 */
void free_callback(struct OpenAPI_Callback *cb) {
  size_t i;
  if (!cb)
    return;
  if (cb->name)
    C_CDD_FREE(cb->name);
  if (cb->ref)
    C_CDD_FREE(cb->ref);
  if (cb->summary)
    C_CDD_FREE(cb->summary);
  if (cb->description)
    C_CDD_FREE(cb->description);
  if (cb->extensions_json)
    C_CDD_FREE(cb->extensions_json);
  if (cb->paths) {
    for (i = 0; i < cb->n_paths; ++i) {
      free_path_item(&cb->paths[i]);
    }
    C_CDD_FREE(cb->paths);
  }
  memset(cb, 0, sizeof(*cb));
}

/**
 * @brief Frees the memory associated with operation.
 */
void free_operation(struct OpenAPI_Operation *op) {
  size_t i;
  if (!op)
    return;
  if (op->method)
    C_CDD_FREE(op->method);
  if (op->operation_id)
    C_CDD_FREE(op->operation_id);
  if (op->summary)
    C_CDD_FREE(op->summary);
  if (op->description)
    C_CDD_FREE(op->description);
  if (op->extensions_json)
    C_CDD_FREE(op->extensions_json);
  if (op->responses_extensions_json)
    C_CDD_FREE(op->responses_extensions_json);

  if (op->tags) {
    for (i = 0; i < op->n_tags; ++i) {
      if (op->tags[i])
        C_CDD_FREE(op->tags[i]);
    }
    C_CDD_FREE(op->tags);
  }

  free_schema_ref_content(&op->req_body);
  if (op->req_body_media_types) {
    for (i = 0; i < op->n_req_body_media_types; ++i) {
      free_media_type(&op->req_body_media_types[i]);
    }
    C_CDD_FREE(op->req_body_media_types);
    op->req_body_media_types = NULL;
    op->n_req_body_media_types = 0;
  }
  if (op->req_body_description)
    C_CDD_FREE(op->req_body_description);
  if (op->req_body_extensions_json)
    C_CDD_FREE(op->req_body_extensions_json);
  if (op->req_body_ref)
    C_CDD_FREE(op->req_body_ref);
  if (op->external_docs.description)
    C_CDD_FREE(op->external_docs.description);
  if (op->external_docs.url)
    C_CDD_FREE(op->external_docs.url);
  if (op->external_docs.extensions_json)
    C_CDD_FREE(op->external_docs.extensions_json);

  if (op->servers) {
    openapi_free_servers_array(op->servers, op->n_servers);
    op->servers = NULL;
    op->n_servers = 0;
  }

  if (op->parameters) {
    for (i = 0; i < op->n_parameters; ++i) {
      free_parameter(&op->parameters[i]);
    }
    C_CDD_FREE(op->parameters);
  }

  if (op->responses) {
    for (i = 0; i < op->n_responses; ++i) {
      free_response(&op->responses[i]);
    }
    C_CDD_FREE(op->responses);
  }

  if (op->callbacks) {
    for (i = 0; i < op->n_callbacks; ++i) {
      free_callback(&op->callbacks[i]);
    }
    C_CDD_FREE(op->callbacks);
    op->callbacks = NULL;
    op->n_callbacks = 0;
  }

  if (op->security) {
    for (i = 0; i < op->n_security; ++i) {
      free_security_requirement_set(&op->security[i]);
    }
    C_CDD_FREE(op->security);
    op->security = NULL;
    op->n_security = 0;
    op->security_set = 0;
  }
  memset(op, 0, sizeof(*op));
}

/**
 * @brief Releases all memory owned by an OpenAPI_Spec structure.
 */
void openapi_spec_free(struct OpenAPI_Spec *spec) {
  size_t i;
  if (!spec)
    return;

  if (spec->openapi_version) {
    C_CDD_FREE(spec->openapi_version);
    spec->openapi_version = NULL;
  }
  if (spec->swagger_version) {
    C_CDD_FREE(spec->swagger_version);
    spec->swagger_version = NULL;
  }
  C_CDD_FREE(spec->host);
  C_CDD_FREE(spec->basePath);
  free_string_array(spec->schemes, spec->n_schemes);
  free_string_array(spec->consumes, spec->n_consumes);
  free_string_array(spec->produces, spec->n_produces);
  if (spec->schema_root_json) {
    C_CDD_FREE(spec->schema_root_json);
    spec->schema_root_json = NULL;
  }
  if (spec->self_uri) {
    C_CDD_FREE(spec->self_uri);
    spec->self_uri = NULL;
  }
  if (spec->retrieval_uri) {
    C_CDD_FREE(spec->retrieval_uri);
    spec->retrieval_uri = NULL;
  }
  if (spec->document_uri) {
    C_CDD_FREE(spec->document_uri);
    spec->document_uri = NULL;
  }
  if (spec->json_schema_dialect) {
    C_CDD_FREE(spec->json_schema_dialect);
    spec->json_schema_dialect = NULL;
  }
  if (spec->extensions_json) {
    C_CDD_FREE(spec->extensions_json);
    spec->extensions_json = NULL;
  }
  if (spec->paths_extensions_json) {
    C_CDD_FREE(spec->paths_extensions_json);
    spec->paths_extensions_json = NULL;
  }
  if (spec->webhooks_extensions_json) {
    C_CDD_FREE(spec->webhooks_extensions_json);
    spec->webhooks_extensions_json = NULL;
  }
  if (spec->components_extensions_json) {
    C_CDD_FREE(spec->components_extensions_json);
    spec->components_extensions_json = NULL;
  }
  if (spec->info.title) {
    C_CDD_FREE(spec->info.title);
    spec->info.title = NULL;
  }
  if (spec->info.summary) {
    C_CDD_FREE(spec->info.summary);
    spec->info.summary = NULL;
  }
  if (spec->info.description) {
    C_CDD_FREE(spec->info.description);
    spec->info.description = NULL;
  }
  if (spec->info.terms_of_service) {
    C_CDD_FREE(spec->info.terms_of_service);
    spec->info.terms_of_service = NULL;
  }
  if (spec->info.version) {
    C_CDD_FREE(spec->info.version);
    spec->info.version = NULL;
  }
  if (spec->info.extensions_json) {
    C_CDD_FREE(spec->info.extensions_json);
    spec->info.extensions_json = NULL;
  }
  if (spec->info.contact.name)
    C_CDD_FREE(spec->info.contact.name);
  if (spec->info.contact.url)
    C_CDD_FREE(spec->info.contact.url);
  if (spec->info.contact.email)
    C_CDD_FREE(spec->info.contact.email);
  if (spec->info.contact.extensions_json)
    C_CDD_FREE(spec->info.contact.extensions_json);
  if (spec->info.license.name)
    C_CDD_FREE(spec->info.license.name);
  if (spec->info.license.identifier)
    C_CDD_FREE(spec->info.license.identifier);
  if (spec->info.license.url)
    C_CDD_FREE(spec->info.license.url);
  if (spec->info.license.extensions_json)
    C_CDD_FREE(spec->info.license.extensions_json);
  if (spec->external_docs.description)
    C_CDD_FREE(spec->external_docs.description);
  if (spec->external_docs.url)
    C_CDD_FREE(spec->external_docs.url);
  if (spec->external_docs.extensions_json)
    C_CDD_FREE(spec->external_docs.extensions_json);

  if (spec->tags) {
    for (i = 0; i < spec->n_tags; ++i) {
      if (spec->tags[i].name)
        C_CDD_FREE(spec->tags[i].name);
      if (spec->tags[i].summary)
        C_CDD_FREE(spec->tags[i].summary);
      if (spec->tags[i].description)
        C_CDD_FREE(spec->tags[i].description);
      if (spec->tags[i].parent)
        C_CDD_FREE(spec->tags[i].parent);
      if (spec->tags[i].kind)
        C_CDD_FREE(spec->tags[i].kind);
      if (spec->tags[i].extensions_json)
        C_CDD_FREE(spec->tags[i].extensions_json);
      if (spec->tags[i].external_docs.description)
        C_CDD_FREE(spec->tags[i].external_docs.description);
      if (spec->tags[i].external_docs.url)
        C_CDD_FREE(spec->tags[i].external_docs.url);
      if (spec->tags[i].external_docs.extensions_json)
        C_CDD_FREE(spec->tags[i].external_docs.extensions_json);
    }
    C_CDD_FREE(spec->tags);
    spec->tags = NULL;
    spec->n_tags = 0;
  }

  if (spec->security) {
    for (i = 0; i < spec->n_security; ++i) {
      free_security_requirement_set(&spec->security[i]);
    }
    C_CDD_FREE(spec->security);
    spec->security = NULL;
    spec->n_security = 0;
    spec->security_set = 0;
  }

  if (spec->servers) {
    openapi_free_servers_array(spec->servers, spec->n_servers);
    spec->servers = NULL;
    spec->n_servers = 0;
  }

  if (spec->paths) {
    for (i = 0; i < spec->n_paths; ++i) {
      free_path_item(&spec->paths[i]);
    }
    C_CDD_FREE(spec->paths);
    spec->paths = NULL;
  }

  if (spec->webhooks) {
    for (i = 0; i < spec->n_webhooks; ++i) {
      free_path_item(&spec->webhooks[i]);
    }
    C_CDD_FREE(spec->webhooks);
    spec->webhooks = NULL;
  }

  if (spec->component_path_items) {
    for (i = 0; i < spec->n_component_path_items; ++i) {
      free_path_item(&spec->component_path_items[i]);
      if (spec->component_path_item_names)
        C_CDD_FREE(spec->component_path_item_names[i]);
    }
    C_CDD_FREE(spec->component_path_items);
    C_CDD_FREE(spec->component_path_item_names);
    spec->component_path_items = NULL;
    spec->component_path_item_names = NULL;
    spec->n_component_path_items = 0;
  }

  if (spec->security_schemes) {
    for (i = 0; i < spec->n_security_schemes; ++i) {
      if (spec->security_schemes[i].name)
        C_CDD_FREE(spec->security_schemes[i].name);
      if (spec->security_schemes[i].description)
        C_CDD_FREE(spec->security_schemes[i].description);
      if (spec->security_schemes[i].scheme)
        C_CDD_FREE(spec->security_schemes[i].scheme);
      if (spec->security_schemes[i].bearer_format)
        C_CDD_FREE(spec->security_schemes[i].bearer_format);
      if (spec->security_schemes[i].key_name)
        C_CDD_FREE(spec->security_schemes[i].key_name);
      if (spec->security_schemes[i].open_id_connect_url)
        C_CDD_FREE(spec->security_schemes[i].open_id_connect_url);
      if (spec->security_schemes[i].oauth2_metadata_url)
        C_CDD_FREE(spec->security_schemes[i].oauth2_metadata_url);
      if (spec->security_schemes[i].extensions_json)
        C_CDD_FREE(spec->security_schemes[i].extensions_json);
      if (spec->security_schemes[i].flows) {
        size_t f;
        for (f = 0; f < spec->security_schemes[i].n_flows; ++f) {
          struct OpenAPI_OAuthFlow *flow = &spec->security_schemes[i].flows[f];
          size_t s;
          if (flow->authorization_url)
            C_CDD_FREE(flow->authorization_url);
          if (flow->token_url)
            C_CDD_FREE(flow->token_url);
          if (flow->refresh_url)
            C_CDD_FREE(flow->refresh_url);
          if (flow->device_authorization_url)
            C_CDD_FREE(flow->device_authorization_url);
          if (flow->extensions_json)
            C_CDD_FREE(flow->extensions_json);
          if (flow->scopes) {
            for (s = 0; s < flow->n_scopes; ++s) {
              C_CDD_FREE(flow->scopes[s].name);
              if (flow->scopes[s].description)
                C_CDD_FREE(flow->scopes[s].description);
            }
            C_CDD_FREE(flow->scopes);
          }
        }
        C_CDD_FREE(spec->security_schemes[i].flows);
        spec->security_schemes[i].flows = NULL;
        spec->security_schemes[i].n_flows = 0;
      }
    }
    C_CDD_FREE(spec->security_schemes);
    spec->security_schemes = NULL;
  }

  if (spec->component_parameters || spec->component_parameter_names) {
    for (i = 0; i < spec->n_component_parameters; ++i) {
      if (spec->component_parameters)
        free_parameter(&spec->component_parameters[i]);
      if (spec->component_parameter_names)
        C_CDD_FREE(spec->component_parameter_names[i]);
    }
    C_CDD_FREE(spec->component_parameters);
    C_CDD_FREE(spec->component_parameter_names);
    spec->component_parameters = NULL;
    spec->component_parameter_names = NULL;
    spec->n_component_parameters = 0;
  }

  if (spec->component_responses || spec->component_response_names) {
    for (i = 0; i < spec->n_component_responses; ++i) {
      if (spec->component_responses)
        free_response(&spec->component_responses[i]);
      if (spec->component_response_names)
        C_CDD_FREE(spec->component_response_names[i]);
    }
    C_CDD_FREE(spec->component_responses);
    C_CDD_FREE(spec->component_response_names);
    spec->component_responses = NULL;
    spec->component_response_names = NULL;
    spec->n_component_responses = 0;
  }

  if (spec->component_headers || spec->component_header_names) {
    for (i = 0; i < spec->n_component_headers; ++i) {
      if (spec->component_headers)
        free_header(&spec->component_headers[i]);
      if (spec->component_header_names)
        C_CDD_FREE(spec->component_header_names[i]);
    }
    C_CDD_FREE(spec->component_headers);
    C_CDD_FREE(spec->component_header_names);
    spec->component_headers = NULL;
    spec->component_header_names = NULL;
    spec->n_component_headers = 0;
  }

  if (spec->component_request_bodies || spec->component_request_body_names) {
    for (i = 0; i < spec->n_component_request_bodies; ++i) {
      if (spec->component_request_bodies)
        free_request_body(&spec->component_request_bodies[i]);
      if (spec->component_request_body_names)
        C_CDD_FREE(spec->component_request_body_names[i]);
    }
    C_CDD_FREE(spec->component_request_bodies);
    C_CDD_FREE(spec->component_request_body_names);
    spec->component_request_bodies = NULL;
    spec->component_request_body_names = NULL;
    spec->n_component_request_bodies = 0;
  }

  if (spec->component_media_types || spec->component_media_type_names) {
    for (i = 0; i < spec->n_component_media_types; ++i) {
      if (spec->component_media_types) {
        struct OpenAPI_MediaType *mt = &spec->component_media_types[i];
        free_media_type(mt);
      }
      if (spec->component_media_type_names)
        C_CDD_FREE(spec->component_media_type_names[i]);
    }
    C_CDD_FREE(spec->component_media_types);
    C_CDD_FREE(spec->component_media_type_names);
    spec->component_media_types = NULL;
    spec->component_media_type_names = NULL;
    spec->n_component_media_types = 0;
  }

  if (spec->component_examples || spec->component_example_names) {
    for (i = 0; i < spec->n_component_examples; ++i) {
      if (spec->component_examples)
        free_example(&spec->component_examples[i]);
      if (spec->component_example_names)
        C_CDD_FREE(spec->component_example_names[i]);
    }
    C_CDD_FREE(spec->component_examples);
    C_CDD_FREE(spec->component_example_names);
    spec->component_examples = NULL;
    spec->component_example_names = NULL;
    spec->n_component_examples = 0;
  }

  if (spec->component_links) {
    for (i = 0; i < spec->n_component_links; ++i) {
      free_link(&spec->component_links[i]);
    }
    C_CDD_FREE(spec->component_links);
    spec->component_links = NULL;
    spec->n_component_links = 0;
  }

  if (spec->component_callbacks) {
    for (i = 0; i < spec->n_component_callbacks; ++i) {
      free_callback(&spec->component_callbacks[i]);
    }
    C_CDD_FREE(spec->component_callbacks);
    spec->component_callbacks = NULL;
    spec->n_component_callbacks = 0;
  }

  if (spec->raw_schema_names || spec->raw_schema_json) {
    for (i = 0; i < spec->n_raw_schemas; ++i) {
      if (spec->raw_schema_names)
        C_CDD_FREE(spec->raw_schema_names[i]);
      if (spec->raw_schema_json)
        C_CDD_FREE(spec->raw_schema_json[i]);
    }
    C_CDD_FREE(spec->raw_schema_names);
    C_CDD_FREE(spec->raw_schema_json);
    spec->raw_schema_names = NULL;
    spec->raw_schema_json = NULL;
    spec->n_raw_schemas = 0;
  }

  if (spec->defined_schemas || spec->defined_schema_names ||
      spec->defined_schema_ids || spec->defined_schema_anchors ||
      spec->defined_schema_dynamic_anchors) {
    for (i = 0; i < spec->n_defined_schemas; ++i) {
      if (spec->defined_schemas)
        struct_fields_free(&spec->defined_schemas[i]);
      if (spec->defined_schema_names)
        C_CDD_FREE(spec->defined_schema_names[i]);
      if (spec->defined_schema_ids)
        C_CDD_FREE(spec->defined_schema_ids[i]);
      if (spec->defined_schema_anchors)
        C_CDD_FREE(spec->defined_schema_anchors[i]);
      if (spec->defined_schema_dynamic_anchors)
        C_CDD_FREE(spec->defined_schema_dynamic_anchors[i]);
    }
    C_CDD_FREE(spec->defined_schemas);
    C_CDD_FREE(spec->defined_schema_names);
    C_CDD_FREE(spec->defined_schema_ids);
    C_CDD_FREE(spec->defined_schema_anchors);
    C_CDD_FREE(spec->defined_schema_dynamic_anchors);
    spec->defined_schemas = NULL;
    spec->defined_schema_names = NULL;
    spec->defined_schema_ids = NULL;
    spec->defined_schema_anchors = NULL;
    spec->defined_schema_dynamic_anchors = NULL;
  }

  spec->n_paths = 0;
  spec->n_webhooks = 0;
  spec->n_component_path_items = 0;
  spec->n_security_schemes = 0;
  spec->n_defined_schemas = 0;
  spec->n_raw_schemas = 0;
  spec->n_component_parameters = 0;
  spec->n_component_responses = 0;
  spec->n_component_headers = 0;
  spec->n_component_request_bodies = 0;
  spec->n_component_media_types = 0;
  spec->n_component_examples = 0;
  spec->n_component_links = 0;
  spec->n_component_callbacks = 0;
  spec->n_security = 0;
  spec->security_set = 0;
  memset(spec, 0, sizeof(*spec));
}

/**
 * @brief Frees the memory associated with string array.
 */
void free_string_array(char **arr, size_t n) {
  size_t i;
  if (!arr)
    return;
  for (i = 0; i < n; ++i) {
    if (arr[i])
      C_CDD_FREE(arr[i]);
  }
  C_CDD_FREE(arr);
}

/**
 * @brief Frees the memory associated with name list.
 */
void free_name_list(char **names, size_t count) {
  size_t i;
  if (!names)
    return;
  for (i = 0; i < count; ++i)
    C_CDD_FREE(names[i]);
  C_CDD_FREE(names);
}
