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
 *
 * @param[in,out] ref The OpenAPI_SchemaRef structure to free.
 */
void free_schema_ref_content(struct OpenAPI_SchemaRef *ref) {
  if (!ref)
    return;

  if (ref->all_of) {
    size_t i;
    for (i = 0; i < ref->n_all_of; ++i)
      free_schema_ref_content(&ref->all_of[i]);
    C_CDD_FREE(ref->all_of);
  }
  if (ref->any_of) {
    size_t i;
    for (i = 0; i < ref->n_any_of; ++i)
      free_schema_ref_content(&ref->any_of[i]);
    C_CDD_FREE(ref->any_of);
  }
  if (ref->one_of) {
    size_t i;
    for (i = 0; i < ref->n_one_of; ++i)
      free_schema_ref_content(&ref->one_of[i]);
    C_CDD_FREE(ref->one_of);
  }
  if (ref->not_schema) {
    free_schema_ref_content(ref->not_schema);
    C_CDD_FREE(ref->not_schema);
  }
  if (ref->if_schema) {
    free_schema_ref_content(ref->if_schema);
    C_CDD_FREE(ref->if_schema);
  }
  if (ref->then_schema) {
    free_schema_ref_content(ref->then_schema);
    C_CDD_FREE(ref->then_schema);
  }
  if (ref->else_schema) {
    free_schema_ref_content(ref->else_schema);
    C_CDD_FREE(ref->else_schema);
  }
  if (ref->ref_name)
    C_CDD_FREE(ref->ref_name);
  if (ref->ref)
    C_CDD_FREE(ref->ref);
  if (ref->inline_type)
    C_CDD_FREE(ref->inline_type);
  if (ref->type_union)
    free_string_array(ref->type_union, ref->n_type_union);
  if (ref->format)
    C_CDD_FREE(ref->format);
  if (ref->content_type)
    C_CDD_FREE(ref->content_type);
  if (ref->content_media_type)
    C_CDD_FREE(ref->content_media_type);
  if (ref->content_encoding)
    C_CDD_FREE(ref->content_encoding);
  if (ref->content_schema) {
    free_schema_ref_content(ref->content_schema);
    C_CDD_FREE(ref->content_schema);
  }
  if (ref->items_format)
    C_CDD_FREE(ref->items_format);
  if (ref->items_type_union)
    free_string_array(ref->items_type_union, ref->n_items_type_union);
  if (ref->items_ref)
    C_CDD_FREE(ref->items_ref);
  if (ref->items_content_media_type)
    C_CDD_FREE(ref->items_content_media_type);
  if (ref->items_content_encoding)
    C_CDD_FREE(ref->items_content_encoding);
  if (ref->items_content_schema) {
    free_schema_ref_content(ref->items_content_schema);
    C_CDD_FREE(ref->items_content_schema);
  }
  if (ref->summary)
    C_CDD_FREE(ref->summary);
  if (ref->description)
    C_CDD_FREE(ref->description);
  if (ref->const_value_set)
    free_any_value(&ref->const_value);
  if (ref->examples) {
    size_t i;
    for (i = 0; i < ref->n_examples; ++i)
      free_any_value(&ref->examples[i]);
    C_CDD_FREE(ref->examples);
  }
  if (ref->example_set)
    free_any_value(&ref->example);
  if (ref->default_value_set)
    free_any_value(&ref->default_value);
  if (ref->enum_values) {
    size_t i;
    for (i = 0; i < ref->n_enum_values; ++i)
      free_any_value(&ref->enum_values[i]);
    C_CDD_FREE(ref->enum_values);
  }
  if (ref->schema_extra_json)
    C_CDD_FREE(ref->schema_extra_json);
  if (ref->external_docs.description)
    C_CDD_FREE(ref->external_docs.description);
  if (ref->external_docs.url)
    C_CDD_FREE(ref->external_docs.url);
  if (ref->external_docs.extensions_json)
    C_CDD_FREE(ref->external_docs.extensions_json);
  if (ref->discriminator.property_name)
    C_CDD_FREE(ref->discriminator.property_name);
  if (ref->discriminator.default_mapping)
    C_CDD_FREE(ref->discriminator.default_mapping);
  if (ref->discriminator.extensions_json)
    C_CDD_FREE(ref->discriminator.extensions_json);
  if (ref->discriminator.mapping) {
    size_t i;
    for (i = 0; i < ref->discriminator.n_mapping; ++i) {
      if (ref->discriminator.mapping[i].value)
        C_CDD_FREE(ref->discriminator.mapping[i].value);
      if (ref->discriminator.mapping[i].schema)
        C_CDD_FREE(ref->discriminator.mapping[i].schema);
    }
    C_CDD_FREE(ref->discriminator.mapping);
  }
  if (ref->xml.name)
    C_CDD_FREE(ref->xml.name);
  if (ref->xml.namespace_uri)
    C_CDD_FREE(ref->xml.namespace_uri);
  if (ref->xml.prefix)
    C_CDD_FREE(ref->xml.prefix);
  if (ref->xml.extensions_json)
    C_CDD_FREE(ref->xml.extensions_json);
  if (ref->items_enum_values) {
    size_t i;
    for (i = 0; i < ref->n_items_enum_values; ++i)
      free_any_value(&ref->items_enum_values[i]);
    C_CDD_FREE(ref->items_enum_values);
  }
  if (ref->pattern)
    C_CDD_FREE(ref->pattern);
  if (ref->items_pattern)
    C_CDD_FREE(ref->items_pattern);
  if (ref->items_example_set)
    free_any_value(&ref->items_example);
  if (ref->items_examples) {
    size_t i;
    for (i = 0; i < ref->n_items_examples; ++i)
      free_any_value(&ref->items_examples[i]);
    C_CDD_FREE(ref->items_examples);
  }
  if (ref->items_const_value_set)
    free_any_value(&ref->items_const_value);
  if (ref->items_default_value_set)
    free_any_value(&ref->items_default_value);
  if (ref->items_extra_json)
    C_CDD_FREE(ref->items_extra_json);
  if (ref->collection_format)
    C_CDD_FREE(ref->collection_format);
  if (ref->items_collection_format)
    C_CDD_FREE(ref->items_collection_format);
  if (ref->multipart_fields) {
    size_t i;
    for (i = 0; i < ref->n_multipart_fields; ++i) {
      if (ref->multipart_fields[i].name)
        C_CDD_FREE(ref->multipart_fields[i].name);
      if (ref->multipart_fields[i].type)
        C_CDD_FREE(ref->multipart_fields[i].type);
    }
    C_CDD_FREE(ref->multipart_fields);
  }
  memset(ref, 0, sizeof(*ref));
}

/**
 * @brief Frees the memory associated with encoding.
 *
 * @param[in,out] enc The OpenAPI_Encoding structure to free.
 */
void free_encoding(struct OpenAPI_Encoding *enc) {
  size_t i;
  if (!enc)
    return;
  if (enc->name)
    C_CDD_FREE(enc->name);
  if (enc->content_type)
    C_CDD_FREE(enc->content_type);
  if (enc->extensions_json)
    C_CDD_FREE(enc->extensions_json);
  if (enc->headers) {
    for (i = 0; i < enc->n_headers; ++i) {
      free_header(&enc->headers[i]);
    }
    C_CDD_FREE(enc->headers);
    enc->headers = NULL;
    enc->n_headers = 0;
  }
  if (enc->encoding) {
    for (i = 0; i < enc->n_encoding; ++i) {
      free_encoding(&enc->encoding[i]);
    }
    C_CDD_FREE(enc->encoding);
    enc->encoding = NULL;
    enc->n_encoding = 0;
  }
  if (enc->prefix_encoding) {
    for (i = 0; i < enc->n_prefix_encoding; ++i) {
      free_encoding(&enc->prefix_encoding[i]);
    }
    C_CDD_FREE(enc->prefix_encoding);
    enc->prefix_encoding = NULL;
    enc->n_prefix_encoding = 0;
  }
  if (enc->item_encoding) {
    free_encoding(enc->item_encoding);
    C_CDD_FREE(enc->item_encoding);
    enc->item_encoding = NULL;
    enc->item_encoding_set = 0;
  }
  memset(enc, 0, sizeof(*enc));
}

/**
 * @brief Frees the memory associated with media type.
 *
 * @param[in,out] mt The OpenAPI_MediaType structure to free.
 */
void free_media_type(struct OpenAPI_MediaType *mt) {
  size_t e;
  if (!mt)
    return;
  if (mt->name)
    C_CDD_FREE(mt->name);
  if (mt->ref)
    C_CDD_FREE(mt->ref);
  if (mt->extensions_json)
    C_CDD_FREE(mt->extensions_json);
  free_schema_ref_content(&mt->schema);
  free_schema_ref_content(&mt->item_schema);
  if (mt->example_set)
    free_any_value(&mt->example);
  if (mt->examples) {
    for (e = 0; e < mt->n_examples; ++e)
      free_example(&mt->examples[e]);
    C_CDD_FREE(mt->examples);
    mt->examples = NULL;
    mt->n_examples = 0;
  }
  if (mt->encoding) {
    for (e = 0; e < mt->n_encoding; ++e) {
      free_encoding(&mt->encoding[e]);
    }
    C_CDD_FREE(mt->encoding);
    mt->encoding = NULL;
    mt->n_encoding = 0;
  }
  if (mt->prefix_encoding) {
    for (e = 0; e < mt->n_prefix_encoding; ++e) {
      free_encoding(&mt->prefix_encoding[e]);
    }
    C_CDD_FREE(mt->prefix_encoding);
    mt->prefix_encoding = NULL;
    mt->n_prefix_encoding = 0;
  }
  if (mt->item_encoding) {
    free_encoding(mt->item_encoding);
    C_CDD_FREE(mt->item_encoding);
    mt->item_encoding = NULL;
    mt->item_encoding_set = 0;
  }
  memset(mt, 0, sizeof(*mt));
}

/**
 * @brief Frees the memory associated with parameter.
 *
 * @param[in,out] param The OpenAPI_Parameter structure to free.
 */
void free_parameter(struct OpenAPI_Parameter *param) {
  if (!param)
    return;
  if (param->name)
    C_CDD_FREE(param->name);
  if (param->type)
    C_CDD_FREE(param->type);
  if (param->content_type)
    C_CDD_FREE(param->content_type);
  if (param->content_ref)
    C_CDD_FREE(param->content_ref);
  if (param->content_media_types) {
    size_t i;
    for (i = 0; i < param->n_content_media_types; ++i) {
      free_media_type(&param->content_media_types[i]);
    }
    C_CDD_FREE(param->content_media_types);
    param->content_media_types = NULL;
    param->n_content_media_types = 0;
  }
  free_schema_ref_content(&param->schema);
  if (param->description)
    C_CDD_FREE(param->description);
  if (param->extensions_json)
    C_CDD_FREE(param->extensions_json);
  if (param->items_type)
    C_CDD_FREE(param->items_type);
  if (param->collection_format)
    C_CDD_FREE(param->collection_format);
  if (param->ref)
    C_CDD_FREE(param->ref);
  if (param->example_set)
    free_any_value(&param->example);
  if (param->examples) {
    size_t i;
    for (i = 0; i < param->n_examples; ++i) {
      free_example(&param->examples[i]);
    }
    C_CDD_FREE(param->examples);
    param->examples = NULL;
    param->n_examples = 0;
  }
  memset(param, 0, sizeof(*param));
}

/**
 * @brief Frees the memory associated with header.
 *
 * @param[in,out] hdr The OpenAPI_Header structure to free.
 */
void free_header(struct OpenAPI_Header *hdr) {
  if (!hdr)
    return;
  if (hdr->name)
    C_CDD_FREE(hdr->name);
  if (hdr->ref)
    C_CDD_FREE(hdr->ref);
  if (hdr->description)
    C_CDD_FREE(hdr->description);
  if (hdr->extensions_json)
    C_CDD_FREE(hdr->extensions_json);
  if (hdr->content_type)
    C_CDD_FREE(hdr->content_type);
  if (hdr->content_ref)
    C_CDD_FREE(hdr->content_ref);
  if (hdr->content_media_types) {
    size_t i;
    for (i = 0; i < hdr->n_content_media_types; ++i) {
      free_media_type(&hdr->content_media_types[i]);
    }
    C_CDD_FREE(hdr->content_media_types);
    hdr->content_media_types = NULL;
    hdr->n_content_media_types = 0;
  }
  free_schema_ref_content(&hdr->schema);
  if (hdr->type)
    C_CDD_FREE(hdr->type);
  if (hdr->items_type)
    C_CDD_FREE(hdr->items_type);
  if (hdr->collection_format)
    C_CDD_FREE(hdr->collection_format);
  if (hdr->example_set)
    free_any_value(&hdr->example);
  if (hdr->examples) {
    size_t i;
    for (i = 0; i < hdr->n_examples; ++i) {
      free_example(&hdr->examples[i]);
    }
    C_CDD_FREE(hdr->examples);
    hdr->examples = NULL;
    hdr->n_examples = 0;
  }
  memset(hdr, 0, sizeof(*hdr));
}

/**
 * @brief Frees the memory associated with response.
 *
 * @param[in,out] resp The OpenAPI_Response structure to free.
 */
void free_response(struct OpenAPI_Response *resp) {
  size_t i;
  if (!resp)
    return;
  if (resp->code)
    C_CDD_FREE(resp->code);
  if (resp->summary)
    C_CDD_FREE(resp->summary);
  if (resp->description)
    C_CDD_FREE(resp->description);
  if (resp->extensions_json)
    C_CDD_FREE(resp->extensions_json);
  if (resp->content_type)
    C_CDD_FREE(resp->content_type);
  if (resp->content_ref)
    C_CDD_FREE(resp->content_ref);
  if (resp->content_media_types) {
    for (i = 0; i < resp->n_content_media_types; ++i) {
      free_media_type(&resp->content_media_types[i]);
    }
    C_CDD_FREE(resp->content_media_types);
    resp->content_media_types = NULL;
    resp->n_content_media_types = 0;
  }
  if (resp->ref)
    C_CDD_FREE(resp->ref);
  if (resp->headers) {
    for (i = 0; i < resp->n_headers; ++i) {
      free_header(&resp->headers[i]);
    }
    C_CDD_FREE(resp->headers);
    resp->headers = NULL;
    resp->n_headers = 0;
  }
  if (resp->links) {
    for (i = 0; i < resp->n_links; ++i) {
      free_link(&resp->links[i]);
    }
    C_CDD_FREE(resp->links);
    resp->links = NULL;
    resp->n_links = 0;
  }
  if (resp->example_set)
    free_any_value(&resp->example);
  if (resp->examples) {
    for (i = 0; i < resp->n_examples; ++i) {
      free_example(&resp->examples[i]);
    }
    C_CDD_FREE(resp->examples);
    resp->examples = NULL;
    resp->n_examples = 0;
  }
  free_schema_ref_content(&resp->schema);
  memset(resp, 0, sizeof(*resp));
}

/**
 * @brief Frees the memory associated with request body.
 *
 * @param[in,out] rb The OpenAPI_RequestBody structure to free.
 */
void free_request_body(struct OpenAPI_RequestBody *rb) {
  if (!rb)
    return;
  if (rb->ref)
    C_CDD_FREE(rb->ref);
  if (rb->description)
    C_CDD_FREE(rb->description);
  if (rb->extensions_json)
    C_CDD_FREE(rb->extensions_json);
  if (rb->content_ref)
    C_CDD_FREE(rb->content_ref);
  if (rb->content_media_types) {
    size_t i;
    for (i = 0; i < rb->n_content_media_types; ++i) {
      free_media_type(&rb->content_media_types[i]);
    }
    C_CDD_FREE(rb->content_media_types);
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
    C_CDD_FREE(rb->examples);
    rb->examples = NULL;
    rb->n_examples = 0;
  }
  free_schema_ref_content(&rb->schema);
  memset(rb, 0, sizeof(*rb));
}

/**
 * @brief Frees the memory associated with any value.
 *
 * @param[in,out] val The OpenAPI_Any value to free.
 */
void free_any_value(struct OpenAPI_Any *val) {
  if (!val)
    return;
  if (val->type == OA_ANY_STRING && val->string) {
    C_CDD_FREE(val->string);
  } else if (val->type == OA_ANY_JSON && val->json) {
    C_CDD_FREE(val->json);
  }
  val->type = OA_ANY_UNSET;
  val->string = NULL;
  val->json = NULL;
}

/**
 * @brief Frees the memory associated with link.
 *
 * @param[in,out] link The OpenAPI_Link structure to free.
 */
void free_link(struct OpenAPI_Link *link) {
  size_t i;
  if (!link)
    return;
  if (link->name)
    C_CDD_FREE(link->name);
  if (link->ref)
    C_CDD_FREE(link->ref);
  if (link->summary)
    C_CDD_FREE(link->summary);
  if (link->description)
    C_CDD_FREE(link->description);
  if (link->extensions_json)
    C_CDD_FREE(link->extensions_json);
  if (link->operation_ref)
    C_CDD_FREE(link->operation_ref);
  if (link->operation_id)
    C_CDD_FREE(link->operation_id);
  if (link->parameters) {
    for (i = 0; i < link->n_parameters; ++i) {
      if (link->parameters[i].name)
        C_CDD_FREE(link->parameters[i].name);
      free_any_value(&link->parameters[i].value);
    }
    C_CDD_FREE(link->parameters);
  }
  if (link->request_body_set)
    free_any_value(&link->request_body);
  if (link->server_set && link->server) {
    openapi_free_servers_array(link->server, 1);
    link->server = NULL;
    link->server_set = 0;
  }
  memset(link, 0, sizeof(*link));
}

/**
 * @brief Frees the memory associated with security requirement.
 *
 * @param[in,out] req The OpenAPI_SecurityRequirement structure to free.
 */
void free_security_requirement(struct OpenAPI_SecurityRequirement *req) {
  size_t i;
  if (!req)
    return;
  if (req->scheme)
    C_CDD_FREE(req->scheme);
  if (req->scopes) {
    for (i = 0; i < req->n_scopes; ++i) {
      if (req->scopes[i])
        C_CDD_FREE(req->scopes[i]);
    }
    C_CDD_FREE(req->scopes);
  }
  memset(req, 0, sizeof(*req));
}

/**
 * @brief Frees the memory associated with security requirement set.
 *
 * @param[in,out] set The OpenAPI_SecurityRequirementSet structure to free.
 */
void free_security_requirement_set(struct OpenAPI_SecurityRequirementSet *set) {
  size_t i;
  if (!set)
    return;
  if (set->requirements) {
    for (i = 0; i < set->n_requirements; ++i) {
      free_security_requirement(&set->requirements[i]);
    }
    C_CDD_FREE(set->requirements);
  }
  if (set->extensions_json)
    C_CDD_FREE(set->extensions_json);
  memset(set, 0, sizeof(*set));
}

/**
 * @brief Frees the memory associated with example.
 *
 * @param[in,out] ex The OpenAPI_Example structure to free.
 */
void free_example(struct OpenAPI_Example *ex) {
  if (!ex)
    return;
  if (ex->name)
    C_CDD_FREE(ex->name);
  if (ex->ref)
    C_CDD_FREE(ex->ref);
  if (ex->summary)
    C_CDD_FREE(ex->summary);
  if (ex->description)
    C_CDD_FREE(ex->description);
  if (ex->extensions_json)
    C_CDD_FREE(ex->extensions_json);
  if (ex->serialized_value)
    C_CDD_FREE(ex->serialized_value);
  if (ex->external_value)
    C_CDD_FREE(ex->external_value);
  if (ex->data_value_set)
    free_any_value(&ex->data_value);
  if (ex->value_set)
    free_any_value(&ex->value);
  memset(ex, 0, sizeof(*ex));
}
