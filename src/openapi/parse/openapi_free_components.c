/**
 * @file openapi_free_components.c
 * @brief Component resource cleanup routines for OpenAPI spec.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Frees the memory associated with schema ref content.
 */
void free_schema_ref_content(struct OpenAPI_SchemaRef *ref) {
  if (!ref)
    return;

  if (ref->all_of) {
    size_t i;
    for (i = 0; i < ref->n_all_of; ++i)
      free_schema_ref_content(&ref->all_of[i]);
    free(ref->all_of);
  }
  if (ref->any_of) {
    size_t i;
    for (i = 0; i < ref->n_any_of; ++i)
      free_schema_ref_content(&ref->any_of[i]);
    free(ref->any_of);
  }
  if (ref->one_of) {
    size_t i;
    for (i = 0; i < ref->n_one_of; ++i)
      free_schema_ref_content(&ref->one_of[i]);
    free(ref->one_of);
  }
  if (ref->not_schema) {
    free_schema_ref_content(ref->not_schema);
    free(ref->not_schema);
  }
  if (ref->if_schema) {
    free_schema_ref_content(ref->if_schema);
    free(ref->if_schema);
  }
  if (ref->then_schema) {
    free_schema_ref_content(ref->then_schema);
    free(ref->then_schema);
  }
  if (ref->else_schema) {
    free_schema_ref_content(ref->else_schema);
    free(ref->else_schema);
  }
  if (ref->ref_name)
    free(ref->ref_name);
  if (ref->ref)
    free(ref->ref);
  if (ref->inline_type)
    free(ref->inline_type);
  if (ref->type_union)
    free_string_array(ref->type_union, ref->n_type_union);
  if (ref->format)
    free(ref->format);
  if (ref->content_type)
    free(ref->content_type);
  if (ref->content_media_type)
    free(ref->content_media_type);
  if (ref->content_encoding)
    free(ref->content_encoding);
  if (ref->content_schema) {
    free_schema_ref_content(ref->content_schema);
    free(ref->content_schema);
  }
  if (ref->items_format)
    free(ref->items_format);
  if (ref->items_type_union)
    free_string_array(ref->items_type_union, ref->n_items_type_union);
  if (ref->items_ref)
    free(ref->items_ref);
  if (ref->items_content_media_type)
    free(ref->items_content_media_type);
  if (ref->items_content_encoding)
    free(ref->items_content_encoding);
  if (ref->items_content_schema) {
    free_schema_ref_content(ref->items_content_schema);
    free(ref->items_content_schema);
  }
  if (ref->summary)
    free(ref->summary);
  if (ref->description)
    free(ref->description);
  if (ref->const_value_set)
    free_any_value(&ref->const_value);
  if (ref->examples) {
    size_t i;
    for (i = 0; i < ref->n_examples; ++i)
      free_any_value(&ref->examples[i]);
    free(ref->examples);
  }
  if (ref->example_set)
    free_any_value(&ref->example);
  if (ref->default_value_set)
    free_any_value(&ref->default_value);
  if (ref->enum_values) {
    size_t i;
    for (i = 0; i < ref->n_enum_values; ++i)
      free_any_value(&ref->enum_values[i]);
    free(ref->enum_values);
  }
  if (ref->schema_extra_json)
    free(ref->schema_extra_json);
  if (ref->external_docs.description)
    free(ref->external_docs.description);
  if (ref->external_docs.url)
    free(ref->external_docs.url);
  if (ref->external_docs.extensions_json)
    free(ref->external_docs.extensions_json);
  if (ref->discriminator.property_name)
    free(ref->discriminator.property_name);
  if (ref->discriminator.default_mapping)
    free(ref->discriminator.default_mapping);
  if (ref->discriminator.extensions_json)
    free(ref->discriminator.extensions_json);
  if (ref->discriminator.mapping) {
    size_t i;
    for (i = 0; i < ref->discriminator.n_mapping; ++i) {
      if (ref->discriminator.mapping[i].value)
        free(ref->discriminator.mapping[i].value);
      if (ref->discriminator.mapping[i].schema)
        free(ref->discriminator.mapping[i].schema);
    }
    free(ref->discriminator.mapping);
  }
  if (ref->xml.name)
    free(ref->xml.name);
  if (ref->xml.namespace_uri)
    free(ref->xml.namespace_uri);
  if (ref->xml.prefix)
    free(ref->xml.prefix);
  if (ref->xml.extensions_json)
    free(ref->xml.extensions_json);
  if (ref->items_enum_values) {
    size_t i;
    for (i = 0; i < ref->n_items_enum_values; ++i)
      free_any_value(&ref->items_enum_values[i]);
    free(ref->items_enum_values);
  }
  if (ref->pattern)
    free(ref->pattern);
  if (ref->items_pattern)
    free(ref->items_pattern);
  if (ref->items_example_set)
    free_any_value(&ref->items_example);
  if (ref->items_examples) {
    size_t i;
    for (i = 0; i < ref->n_items_examples; ++i)
      free_any_value(&ref->items_examples[i]);
    free(ref->items_examples);
  }
  if (ref->items_const_value_set)
    free_any_value(&ref->items_const_value);
  if (ref->items_default_value_set)
    free_any_value(&ref->items_default_value);
  if (ref->items_extra_json)
    free(ref->items_extra_json);
  if (ref->multipart_fields) {
    size_t i;
    for (i = 0; i < ref->n_multipart_fields; ++i) {
      if (ref->multipart_fields[i].name)
        free(ref->multipart_fields[i].name);
      if (ref->multipart_fields[i].type)
        free(ref->multipart_fields[i].type);
    }
    free(ref->multipart_fields);
  }
}

/**
 * @brief Frees the memory associated with encoding.
 */
void free_encoding(struct OpenAPI_Encoding *enc) {
  size_t i;
  if (!enc)
    return;
  if (enc->name)
    free(enc->name);
  if (enc->content_type)
    free(enc->content_type);
  if (enc->extensions_json)
    free(enc->extensions_json);
  if (enc->headers) {
    for (i = 0; i < enc->n_headers; ++i) {
      free_header(&enc->headers[i]);
    }
    free(enc->headers);
    enc->headers = NULL;
    enc->n_headers = 0;
  }
  if (enc->encoding) {
    for (i = 0; i < enc->n_encoding; ++i) {
      free_encoding(&enc->encoding[i]);
    }
    free(enc->encoding);
    enc->encoding = NULL;
    enc->n_encoding = 0;
  }
  if (enc->prefix_encoding) {
    for (i = 0; i < enc->n_prefix_encoding; ++i) {
      free_encoding(&enc->prefix_encoding[i]);
    }
    free(enc->prefix_encoding);
    enc->prefix_encoding = NULL;
    enc->n_prefix_encoding = 0;
  }
  if (enc->item_encoding) {
    free_encoding(enc->item_encoding);
    free(enc->item_encoding);
    enc->item_encoding = NULL;
    enc->item_encoding_set = 0;
  }
}

/**
 * @brief Frees the memory associated with media type.
 */
void free_media_type(struct OpenAPI_MediaType *mt) {
  size_t e;
  if (!mt)
    return;
  if (mt->name)
    free(mt->name);
  if (mt->ref)
    free(mt->ref);
  if (mt->extensions_json)
    free(mt->extensions_json);
  free_schema_ref_content(&mt->schema);
  free_schema_ref_content(&mt->item_schema);
  if (mt->example_set)
    free_any_value(&mt->example);
  if (mt->examples) {
    for (e = 0; e < mt->n_examples; ++e)
      free_example(&mt->examples[e]);
    free(mt->examples);
    mt->examples = NULL;
    mt->n_examples = 0;
  }
  if (mt->encoding) {
    for (e = 0; e < mt->n_encoding; ++e) {
      free_encoding(&mt->encoding[e]);
    }
    free(mt->encoding);
    mt->encoding = NULL;
    mt->n_encoding = 0;
  }
  if (mt->prefix_encoding) {
    for (e = 0; e < mt->n_prefix_encoding; ++e) {
      free_encoding(&mt->prefix_encoding[e]);
    }
    free(mt->prefix_encoding);
    mt->prefix_encoding = NULL;
    mt->n_prefix_encoding = 0;
  }
  if (mt->item_encoding) {
    free_encoding(mt->item_encoding);
    free(mt->item_encoding);
    mt->item_encoding = NULL;
    mt->item_encoding_set = 0;
  }
}

/**
 * @brief Frees the memory associated with parameter.
 */
void free_parameter(struct OpenAPI_Parameter *param) {
  if (!param)
    return;
  if (param->name)
    free(param->name);
  if (param->type)
    free(param->type);
  if (param->content_type)
    free(param->content_type);
  if (param->content_ref)
    free(param->content_ref);
  if (param->content_media_types) {
    size_t i;
    for (i = 0; i < param->n_content_media_types; ++i) {
      free_media_type(&param->content_media_types[i]);
    }
    free(param->content_media_types);
    param->content_media_types = NULL;
    param->n_content_media_types = 0;
  }
  free_schema_ref_content(&param->schema);
  if (param->description)
    free(param->description);
  if (param->extensions_json)
    free(param->extensions_json);
  if (param->items_type)
    free(param->items_type);
  if (param->ref)
    free(param->ref);
  if (param->example_set)
    free_any_value(&param->example);
  if (param->examples) {
    size_t i;
    for (i = 0; i < param->n_examples; ++i) {
      free_example(&param->examples[i]);
    }
    free(param->examples);
    param->examples = NULL;
    param->n_examples = 0;
  }
}

/**
 * @brief Frees the memory associated with header.
 */
void free_header(struct OpenAPI_Header *hdr) {
  if (!hdr)
    return;
  if (hdr->name)
    free(hdr->name);
  if (hdr->ref)
    free(hdr->ref);
  if (hdr->description)
    free(hdr->description);
  if (hdr->extensions_json)
    free(hdr->extensions_json);
  if (hdr->content_type)
    free(hdr->content_type);
  if (hdr->content_ref)
    free(hdr->content_ref);
  if (hdr->content_media_types) {
    size_t i;
    for (i = 0; i < hdr->n_content_media_types; ++i) {
      free_media_type(&hdr->content_media_types[i]);
    }
    free(hdr->content_media_types);
    hdr->content_media_types = NULL;
    hdr->n_content_media_types = 0;
  }
  free_schema_ref_content(&hdr->schema);
  if (hdr->type)
    free(hdr->type);
  if (hdr->items_type)
    free(hdr->items_type);
  if (hdr->example_set)
    free_any_value(&hdr->example);
  if (hdr->examples) {
    size_t i;
    for (i = 0; i < hdr->n_examples; ++i) {
      free_example(&hdr->examples[i]);
    }
    free(hdr->examples);
    hdr->examples = NULL;
    hdr->n_examples = 0;
  }
}

/**
 * @brief Frees the memory associated with response.
 */
void free_response(struct OpenAPI_Response *resp) {
  size_t i;
  if (!resp)
    return;
  if (resp->code)
    free(resp->code);
  if (resp->summary)
    free(resp->summary);
  if (resp->description)
    free(resp->description);
  if (resp->extensions_json)
    free(resp->extensions_json);
  if (resp->content_type)
    free(resp->content_type);
  if (resp->content_ref)
    free(resp->content_ref);
  if (resp->content_media_types) {
    for (i = 0; i < resp->n_content_media_types; ++i) {
      free_media_type(&resp->content_media_types[i]);
    }
    free(resp->content_media_types);
    resp->content_media_types = NULL;
    resp->n_content_media_types = 0;
  }
  if (resp->ref)
    free(resp->ref);
  if (resp->headers) {
    for (i = 0; i < resp->n_headers; ++i) {
      free_header(&resp->headers[i]);
    }
    free(resp->headers);
    resp->headers = NULL;
    resp->n_headers = 0;
  }
  if (resp->links) {
    for (i = 0; i < resp->n_links; ++i) {
      free_link(&resp->links[i]);
    }
    free(resp->links);
    resp->links = NULL;
    resp->n_links = 0;
  }
  if (resp->example_set)
    free_any_value(&resp->example);
  if (resp->examples) {
    for (i = 0; i < resp->n_examples; ++i) {
      free_example(&resp->examples[i]);
    }
    free(resp->examples);
    resp->examples = NULL;
    resp->n_examples = 0;
  }
  free_schema_ref_content(&resp->schema);
}

/**
 * @brief Frees the memory associated with request body.
 */
void free_request_body(struct OpenAPI_RequestBody *rb) {
  if (!rb)
    return;
  if (rb->ref)
    free(rb->ref);
  if (rb->description)
    free(rb->description);
  if (rb->extensions_json)
    free(rb->extensions_json);
  if (rb->content_ref)
    free(rb->content_ref);
  if (rb->content_media_types) {
    size_t i;
    for (i = 0; i < rb->n_content_media_types; ++i) {
      free_media_type(&rb->content_media_types[i]);
    }
    free(rb->content_media_types);
    rb->content_media_types = NULL;
    rb->n_content_media_types = 0;
  }
  if (rb->example_set)
    free_any_value(&rb->example);
  if (rb->examples) {
    size_t i;
    for (i = 0; i < rb->n_examples; ++i) {
      free_example(&rb->examples[i]);
    }
    free(rb->examples);
    rb->examples = NULL;
    rb->n_examples = 0;
  }
  free_schema_ref_content(&rb->schema);
}

/**
 * @brief Frees the memory associated with any value.
 */
void free_any_value(struct OpenAPI_Any *val) {
  if (!val)
    return;
  if (val->type == OA_ANY_STRING && val->string) {
    free(val->string);
  } else if (val->type == OA_ANY_JSON && val->json) {
    free(val->json);
  }
  val->type = OA_ANY_UNSET;
  val->string = NULL;
  val->json = NULL;
}

/**
 * @brief Frees the memory associated with link.
 */
void free_link(struct OpenAPI_Link *link) {
  size_t i;
  if (!link)
    return;
  if (link->name)
    free(link->name);
  if (link->ref)
    free(link->ref);
  if (link->summary)
    free(link->summary);
  if (link->description)
    free(link->description);
  if (link->extensions_json)
    free(link->extensions_json);
  if (link->operation_ref)
    free(link->operation_ref);
  if (link->operation_id)
    free(link->operation_id);
  if (link->parameters) {
    for (i = 0; i < link->n_parameters; ++i) {
      if (link->parameters[i].name)
        free(link->parameters[i].name);
      free_any_value(&link->parameters[i].value);
    }
    free(link->parameters);
  }
  if (link->request_body_set)
    free_any_value(&link->request_body);
  if (link->server_set && link->server) {
    openapi_free_servers_array(link->server, 1);
    link->server = NULL;
    link->server_set = 0;
  }
}

/**
 * @brief Frees the memory associated with security requirement.
 */
void free_security_requirement(struct OpenAPI_SecurityRequirement *req) {
  size_t i;
  if (!req)
    return;
  if (req->scheme)
    free(req->scheme);
  if (req->scopes) {
    for (i = 0; i < req->n_scopes; ++i) {
      if (req->scopes[i])
        free(req->scopes[i]);
    }
    free(req->scopes);
  }
}

/**
 * @brief Frees the memory associated with security requirement set.
 */
void free_security_requirement_set(struct OpenAPI_SecurityRequirementSet *set) {
  size_t i;
  if (!set)
    return;
  if (set->requirements) {
    for (i = 0; i < set->n_requirements; ++i) {
      free_security_requirement(&set->requirements[i]);
    }
    free(set->requirements);
  }
  if (set->extensions_json)
    free(set->extensions_json);
}

/**
 * @brief Frees the memory associated with example.
 */
void free_example(struct OpenAPI_Example *ex) {
  if (!ex)
    return;
  if (ex->name)
    free(ex->name);
  if (ex->ref)
    free(ex->ref);
  if (ex->summary)
    free(ex->summary);
  if (ex->description)
    free(ex->description);
  if (ex->extensions_json)
    free(ex->extensions_json);
  if (ex->serialized_value)
    free(ex->serialized_value);
  if (ex->external_value)
    free(ex->external_value);
  if (ex->data_value_set)
    free_any_value(&ex->data_value);
  if (ex->value_set)
    free_any_value(&ex->value);
}
