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
 * @param servers Array of servers
 * @param n_servers Number of servers
 */
void openapi_free_servers_array(struct OpenAPI_Server *servers,
                                size_t n_servers) {
  size_t i;
  if (!servers)
    return;
  for (i = 0; i < n_servers; ++i) {
    if (servers[i].url)
      free(servers[i].url);
    if (servers[i].description)
      free(servers[i].description);
    if (servers[i].name)
      free(servers[i].name);
    if (servers[i].extensions_json)
      free(servers[i].extensions_json);
    if (servers[i].variables) {
      size_t v;
      for (v = 0; v < servers[i].n_variables; ++v) {
        struct OpenAPI_ServerVariable *var = &servers[i].variables[v];
        if (var->name)
          free(var->name);
        if (var->default_value)
          free(var->default_value);
        if (var->description)
          free(var->description);
        if (var->extensions_json)
          free(var->extensions_json);
        if (var->enum_values) {
          size_t e;
          for (e = 0; e < var->n_enum_values; ++e) {
            if (var->enum_values[e])
              free(var->enum_values[e]);
          }
          free(var->enum_values);
        }
      }
      free(servers[i].variables);
    }
  }
  free(servers);
}

/**
 * @brief Frees the memory associated with path item.
 */
void free_path_item(struct OpenAPI_Path *p) {
  size_t i;
  if (!p)
    return;
  if (p->route)
    free(p->route);
  if (p->ref)
    free(p->ref);
  if (p->summary)
    free(p->summary);
  if (p->description)
    free(p->description);
  if (p->extensions_json)
    free(p->extensions_json);
  if (p->parameters) {
    for (i = 0; i < p->n_parameters; ++i) {
      free_parameter(&p->parameters[i]);
    }
    free(p->parameters);
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
    free(p->operations);
  }
  if (p->additional_operations) {
    for (i = 0; i < p->n_additional_operations; ++i) {
      free_operation(&p->additional_operations[i]);
    }
    free(p->additional_operations);
  }
}

/**
 * @brief Frees the memory associated with callback.
 */
void free_callback(struct OpenAPI_Callback *cb) {
  size_t i;
  if (!cb)
    return;
  if (cb->name)
    free(cb->name);
  if (cb->ref)
    free(cb->ref);
  if (cb->summary)
    free(cb->summary);
  if (cb->description)
    free(cb->description);
  if (cb->extensions_json)
    free(cb->extensions_json);
  if (cb->paths) {
    for (i = 0; i < cb->n_paths; ++i) {
      free_path_item(&cb->paths[i]);
    }
    free(cb->paths);
  }
}

/**
 * @brief Frees the memory associated with operation.
 */
void free_operation(struct OpenAPI_Operation *op) {
  size_t i;
  if (!op)
    return;
  if (op->method)
    free(op->method);
  if (op->operation_id)
    free(op->operation_id);
  if (op->summary)
    free(op->summary);
  if (op->description)
    free(op->description);
  if (op->extensions_json)
    free(op->extensions_json);
  if (op->responses_extensions_json)
    free(op->responses_extensions_json);

  if (op->tags) {
    for (i = 0; i < op->n_tags; ++i) {
      if (op->tags[i])
        free(op->tags[i]);
    }
    free(op->tags);
  }

  free_schema_ref_content(&op->req_body);
  if (op->req_body_media_types) {
    for (i = 0; i < op->n_req_body_media_types; ++i) {
      free_media_type(&op->req_body_media_types[i]);
    }
    free(op->req_body_media_types);
    op->req_body_media_types = NULL;
    op->n_req_body_media_types = 0;
  }
  if (op->req_body_description)
    free(op->req_body_description);
  if (op->req_body_extensions_json)
    free(op->req_body_extensions_json);
  if (op->req_body_ref)
    free(op->req_body_ref);
  if (op->external_docs.description)
    free(op->external_docs.description);
  if (op->external_docs.url)
    free(op->external_docs.url);
  if (op->external_docs.extensions_json)
    free(op->external_docs.extensions_json);

  if (op->servers) {
    openapi_free_servers_array(op->servers, op->n_servers);
    op->servers = NULL;
    op->n_servers = 0;
  }

  if (op->parameters) {
    for (i = 0; i < op->n_parameters; ++i) {
      free_parameter(&op->parameters[i]);
    }
    free(op->parameters);
  }

  if (op->responses) {
    for (i = 0; i < op->n_responses; ++i) {
      free_response(&op->responses[i]);
    }
    free(op->responses);
  }

  if (op->callbacks) {
    for (i = 0; i < op->n_callbacks; ++i) {
      free_callback(&op->callbacks[i]);
    }
    free(op->callbacks);
    op->callbacks = NULL;
    op->n_callbacks = 0;
  }

  if (op->security) {
    for (i = 0; i < op->n_security; ++i) {
      free_security_requirement_set(&op->security[i]);
    }
    free(op->security);
    op->security = NULL;
    op->n_security = 0;
    op->security_set = 0;
  }
}

/**
 * @brief Executes the openapi spec free operation.
 */
void openapi_spec_free(struct OpenAPI_Spec *spec) {
  size_t i, j = 0;
  (void)j;
  if (!spec)
    return;

  if (spec->openapi_version) {
    free(spec->openapi_version);
    spec->openapi_version = NULL;
  }
  if (spec->swagger_version) {
    free(spec->swagger_version);
    spec->swagger_version = NULL;
  }
  if (spec->schema_root_json) {
    free(spec->schema_root_json);
    spec->schema_root_json = NULL;
  }
  if (spec->self_uri) {
    free(spec->self_uri);
    spec->self_uri = NULL;
  }
  if (spec->retrieval_uri) {
    free(spec->retrieval_uri);
    spec->retrieval_uri = NULL;
  }
  if (spec->document_uri) {
    free(spec->document_uri);
    spec->document_uri = NULL;
  }
  if (spec->json_schema_dialect) {
    free(spec->json_schema_dialect);
    spec->json_schema_dialect = NULL;
  }
  if (spec->extensions_json) {
    free(spec->extensions_json);
    spec->extensions_json = NULL;
  }
  if (spec->paths_extensions_json) {
    free(spec->paths_extensions_json);
    spec->paths_extensions_json = NULL;
  }
  if (spec->webhooks_extensions_json) {
    free(spec->webhooks_extensions_json);
    spec->webhooks_extensions_json = NULL;
  }
  if (spec->components_extensions_json) {
    free(spec->components_extensions_json);
    spec->components_extensions_json = NULL;
  }
  if (spec->info.title) {
    free(spec->info.title);
    spec->info.title = NULL;
  }
  if (spec->info.summary) {
    free(spec->info.summary);
    spec->info.summary = NULL;
  }
  if (spec->info.description) {
    free(spec->info.description);
    spec->info.description = NULL;
  }
  if (spec->info.terms_of_service) {
    free(spec->info.terms_of_service);
    spec->info.terms_of_service = NULL;
  }
  if (spec->info.version) {
    free(spec->info.version);
    spec->info.version = NULL;
  }
  if (spec->info.extensions_json) {
    free(spec->info.extensions_json);
    spec->info.extensions_json = NULL;
  }
  if (spec->info.contact.name)
    free(spec->info.contact.name);
  if (spec->info.contact.url)
    free(spec->info.contact.url);
  if (spec->info.contact.email)
    free(spec->info.contact.email);
  if (spec->info.contact.extensions_json)
    free(spec->info.contact.extensions_json);
  if (spec->info.license.name)
    free(spec->info.license.name);
  if (spec->info.license.identifier)
    free(spec->info.license.identifier);
  if (spec->info.license.url)
    free(spec->info.license.url);
  if (spec->info.license.extensions_json)
    free(spec->info.license.extensions_json);
  if (spec->external_docs.description)
    free(spec->external_docs.description);
  if (spec->external_docs.url)
    free(spec->external_docs.url);
  if (spec->external_docs.extensions_json)
    free(spec->external_docs.extensions_json);

  if (spec->tags) {
    for (i = 0; i < spec->n_tags; ++i) {
      if (spec->tags[i].name)
        free(spec->tags[i].name);
      if (spec->tags[i].summary)
        free(spec->tags[i].summary);
      if (spec->tags[i].description)
        free(spec->tags[i].description);
      if (spec->tags[i].parent)
        free(spec->tags[i].parent);
      if (spec->tags[i].kind)
        free(spec->tags[i].kind);
      if (spec->tags[i].extensions_json)
        free(spec->tags[i].extensions_json);
      if (spec->tags[i].external_docs.description)
        free(spec->tags[i].external_docs.description);
      if (spec->tags[i].external_docs.url)
        free(spec->tags[i].external_docs.url);
      if (spec->tags[i].external_docs.extensions_json)
        free(spec->tags[i].external_docs.extensions_json);
    }
    free(spec->tags);
    spec->tags = NULL;
    spec->n_tags = 0;
  }

  if (spec->security) {
    for (i = 0; i < spec->n_security; ++i) {
      free_security_requirement_set(&spec->security[i]);
    }
    free(spec->security);
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
    free(spec->paths);
    spec->paths = NULL;
  }

  if (spec->webhooks) {
    for (i = 0; i < spec->n_webhooks; ++i) {
      free_path_item(&spec->webhooks[i]);
    }
    free(spec->webhooks);
    spec->webhooks = NULL;
  }

  if (spec->component_path_items) {
    for (i = 0; i < spec->n_component_path_items; ++i) {
      free_path_item(&spec->component_path_items[i]);
      if (spec->component_path_item_names)
        free(spec->component_path_item_names[i]);
    }
    free(spec->component_path_items);
    free(spec->component_path_item_names);
    spec->component_path_items = NULL;
    spec->component_path_item_names = NULL;
    spec->n_component_path_items = 0;
  }

  if (spec->security_schemes) {
    for (i = 0; i < spec->n_security_schemes; ++i) {
      if (spec->security_schemes[i].name)
        free(spec->security_schemes[i].name);
      if (spec->security_schemes[i].description)
        free(spec->security_schemes[i].description);
      if (spec->security_schemes[i].scheme)
        free(spec->security_schemes[i].scheme);
      if (spec->security_schemes[i].bearer_format)
        free(spec->security_schemes[i].bearer_format);
      if (spec->security_schemes[i].key_name)
        free(spec->security_schemes[i].key_name);
      if (spec->security_schemes[i].open_id_connect_url)
        free(spec->security_schemes[i].open_id_connect_url);
      if (spec->security_schemes[i].oauth2_metadata_url)
        free(spec->security_schemes[i].oauth2_metadata_url);
      if (spec->security_schemes[i].extensions_json)
        free(spec->security_schemes[i].extensions_json);
      if (spec->security_schemes[i].flows) {
        size_t f;
        for (f = 0; f < spec->security_schemes[i].n_flows; ++f) {
          struct OpenAPI_OAuthFlow *flow = &spec->security_schemes[i].flows[f];
          size_t s;
          if (flow->authorization_url)
            free(flow->authorization_url);
          if (flow->token_url)
            free(flow->token_url);
          if (flow->refresh_url)
            free(flow->refresh_url);
          if (flow->device_authorization_url)
            free(flow->device_authorization_url);
          if (flow->extensions_json)
            free(flow->extensions_json);
          if (flow->scopes) {
            for (s = 0; s < flow->n_scopes; ++s) {
              if (flow->scopes[s].name)
                free(flow->scopes[s].name);
              if (flow->scopes[s].description)
                free(flow->scopes[s].description);
            }
            free(flow->scopes);
          }
        }
        free(spec->security_schemes[i].flows);
        spec->security_schemes[i].flows = NULL;
        spec->security_schemes[i].n_flows = 0;
      }
    }
    free(spec->security_schemes);
    spec->security_schemes = NULL;
  }

  if (spec->component_parameters) {
    for (i = 0; i < spec->n_component_parameters; ++i) {
      free_parameter(&spec->component_parameters[i]);
      free(spec->component_parameter_names[i]);
    }
    free(spec->component_parameters);
    free(spec->component_parameter_names);
    spec->component_parameters = NULL;
    spec->component_parameter_names = NULL;
    spec->n_component_parameters = 0;
  }

  if (spec->component_responses) {
    for (i = 0; i < spec->n_component_responses; ++i) {
      free_response(&spec->component_responses[i]);
      free(spec->component_response_names[i]);
    }
    free(spec->component_responses);
    free(spec->component_response_names);
    spec->component_responses = NULL;
    spec->component_response_names = NULL;
    spec->n_component_responses = 0;
  }

  if (spec->component_headers) {
    for (i = 0; i < spec->n_component_headers; ++i) {
      free_header(&spec->component_headers[i]);
      free(spec->component_header_names[i]);
    }
    free(spec->component_headers);
    free(spec->component_header_names);
    spec->component_headers = NULL;
    spec->component_header_names = NULL;
    spec->n_component_headers = 0;
  }

  if (spec->component_request_bodies) {
    for (i = 0; i < spec->n_component_request_bodies; ++i) {
      free_request_body(&spec->component_request_bodies[i]);
      free(spec->component_request_body_names[i]);
    }
    free(spec->component_request_bodies);
    free(spec->component_request_body_names);
    spec->component_request_bodies = NULL;
    spec->component_request_body_names = NULL;
    spec->n_component_request_bodies = 0;
  }

  if (spec->component_media_types) {
    for (i = 0; i < spec->n_component_media_types; ++i) {
      struct OpenAPI_MediaType *mt = &spec->component_media_types[i];
      free_media_type(mt);
      if (spec->component_media_type_names)
        free(spec->component_media_type_names[i]);
    }
    free(spec->component_media_types);
    free(spec->component_media_type_names);
    spec->component_media_types = NULL;
    spec->component_media_type_names = NULL;
    spec->n_component_media_types = 0;
  }

  if (spec->component_examples) {
    for (i = 0; i < spec->n_component_examples; ++i) {
      free_example(&spec->component_examples[i]);
      if (spec->component_example_names)
        free(spec->component_example_names[i]);
    }
    free(spec->component_examples);
    free(spec->component_example_names);
    spec->component_examples = NULL;
    spec->component_example_names = NULL;
    spec->n_component_examples = 0;
  }

  if (spec->component_links) {
    for (i = 0; i < spec->n_component_links; ++i) {
      free_link(&spec->component_links[i]);
    }
    free(spec->component_links);
    spec->component_links = NULL;
    spec->n_component_links = 0;
  }

  if (spec->component_callbacks) {
    for (i = 0; i < spec->n_component_callbacks; ++i) {
      free_callback(&spec->component_callbacks[i]);
    }
    free(spec->component_callbacks);
    spec->component_callbacks = NULL;
    spec->n_component_callbacks = 0;
  }

  if (spec->raw_schema_names || spec->raw_schema_json) {
    for (i = 0; i < spec->n_raw_schemas; ++i) {
      if (spec->raw_schema_names)
        free(spec->raw_schema_names[i]);
      if (spec->raw_schema_json)
        free(spec->raw_schema_json[i]);
    }
    free(spec->raw_schema_names);
    free(spec->raw_schema_json);
    spec->raw_schema_names = NULL;
    spec->raw_schema_json = NULL;
    spec->n_raw_schemas = 0;
  }

  if (spec->defined_schemas) {
    for (i = 0; i < spec->n_defined_schemas; ++i) {
      struct_fields_free(&spec->defined_schemas[i]);
      free(spec->defined_schema_names[i]);
      if (spec->defined_schema_ids)
        free(spec->defined_schema_ids[i]);
      if (spec->defined_schema_anchors)
        free(spec->defined_schema_anchors[i]);
      if (spec->defined_schema_dynamic_anchors)
        free(spec->defined_schema_dynamic_anchors[i]);
    }
    free(spec->defined_schemas);
    free(spec->defined_schema_names);
    free(spec->defined_schema_ids);
    free(spec->defined_schema_anchors);
    free(spec->defined_schema_dynamic_anchors);
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
      free(arr[i]);
  }
  free(arr);
}

/**
 * @brief Frees the memory associated with name list.
 */
void free_name_list(char **names, size_t count) {
  size_t i;
  if (!names)
    return;
  for (i = 0; i < count; ++i)
    free(names[i]);
  free(names);
}
