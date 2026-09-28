/**
 * @file openapi_headers_links.c
 * @brief Header, link, and encoding parsing.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Parses header object from the given input.
 */
cdd_c_error_t parse_header_object(const JSON_Object *hdr_obj,
                                  struct OpenAPI_Header *out_hdr,
                                  const struct OpenAPI_Spec *spec,
                                  int resolve_refs) {
  struct OpenAPI_Header *_ast_find_component_header_53;
  enum OpenAPI_Style _ast_parse_param_style_54;
  struct OpenAPI_MediaType *_ast_find_component_media_type_55;
  char *_ast_strdup_216 = NULL;
  char *_ast_strdup_217 = NULL;
  char *_ast_strdup_218 = NULL;
  char *_ast_strdup_219 = NULL;
  char *_ast_strdup_220 = NULL;
  char *_ast_strdup_221 = NULL;
  const char *ref;
  const char *desc;
  const char *style_str;
  int required_present;
  int required_val;
  int deprecated_present;
  int deprecated_val;
  int explode_present;
  int explode_val;
  const JSON_Value *schema_val;
  const JSON_Object *schema;
  const JSON_Object *content;
  const JSON_Object *effective_schema;
  const JSON_Value *effective_schema_val;
  const JSON_Object *media_obj;
  const JSON_Value *media_schema_val;
  const char *media_type;
  const char *media_ref;
  const struct OpenAPI_SchemaRef *resolved_schema;
  struct OpenAPI_SchemaRef parsed_schema = {0};
  int parsed_schema_set;
  const char *type;

  if (!hdr_obj || !out_hdr)
    return CDD_C_SUCCESS;

  ref = json_object_get_string(hdr_obj, "$ref");
  if (ref) {
    out_hdr->ref = (c_cdd_strdup(ref, &_ast_strdup_216), _ast_strdup_216);
    if (!out_hdr->ref)
      return CDD_C_ERROR_MEMORY;
    if (resolve_refs && spec) {
      const struct OpenAPI_Header *comp =
          (find_component_header(spec, ref, &_ast_find_component_header_53),
           _ast_find_component_header_53);
      if (comp) {
        {
          cdd_c_error_t _rc = copy_header_fields(out_hdr, comp);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }
    desc = json_object_get_string(hdr_obj, "description");
    if (desc) {
      out_hdr->description =
          (c_cdd_strdup(desc, &_ast_strdup_217), _ast_strdup_217);
      if (!out_hdr->description)
        return CDD_C_ERROR_MEMORY;
    }
    return CDD_C_SUCCESS;
  }

  desc = json_object_get_string(hdr_obj, "description");
  if (desc) {
    out_hdr->description =
        (c_cdd_strdup(desc, &_ast_strdup_218), _ast_strdup_218);
    if (!out_hdr->description)
      return CDD_C_ERROR_MEMORY;
  }

  required_present = json_object_has_value(hdr_obj, "required");
  required_val = json_object_get_boolean(hdr_obj, "required");
  if (required_present)
    out_hdr->required = (required_val == 1);

  deprecated_present = json_object_has_value(hdr_obj, "deprecated");
  deprecated_val = json_object_get_boolean(hdr_obj, "deprecated");
  if (deprecated_present) {
    out_hdr->deprecated_set = 1;
    out_hdr->deprecated = (deprecated_val == 1);
  }

  style_str = json_object_get_string(hdr_obj, "style");
  if (style_str) {
    enum OpenAPI_Style parsed_style =
        (parse_param_style(style_str, &_ast_parse_param_style_54),
         _ast_parse_param_style_54);
    if (parsed_style != OA_STYLE_SIMPLE)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    out_hdr->style_set = 1;
    out_hdr->style = parsed_style;
  } else {
    out_hdr->style = OA_STYLE_SIMPLE;
  }

  explode_present = json_object_has_value(hdr_obj, "explode");
  explode_val = json_object_get_boolean(hdr_obj, "explode");
  if (explode_present) {
    out_hdr->explode_set = 1;
    out_hdr->explode = explode_val;
  }

  schema_val = json_object_get_value(hdr_obj, "schema");
  schema = schema_val ? json_value_get_object(schema_val) : NULL;
  content = json_object_get_object(hdr_obj, "content");
  {
    const int has_schema = (schema_val != NULL);
    int has_content = (content != NULL);
    if (has_schema && has_content)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (content) {
    if (json_object_get_count(content) != 1)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    {
      cdd_c_error_t _rc = parse_content_object(
          content, &out_hdr->content_media_types,
          &out_hdr->n_content_media_types, spec, resolve_refs);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }
  effective_schema = schema;
  effective_schema_val = schema_val;
  if (!effective_schema && spec && spec->swagger_version &&
      json_object_has_value(hdr_obj, "type")) {
    effective_schema = hdr_obj;
    effective_schema_val = json_object_get_wrapping_value(hdr_obj);
  }
  media_obj = NULL;
  media_schema_val = NULL;
  media_type = NULL;
  media_ref = NULL;
  resolved_schema = NULL;
  parsed_schema_set = 0;

  if (content) {
    size_t ccount = json_object_get_count(content);
    if (ccount > 0) {
      media_type = json_object_get_name(content, 0);
      if (media_type) {
        media_obj = json_object_get_object(content, media_type);
      }
    }
    if (media_type) {
      out_hdr->content_type =
          (c_cdd_strdup(media_type, &_ast_strdup_219), _ast_strdup_219);
      if (!out_hdr->content_type)
        return CDD_C_ERROR_MEMORY;
    }
    if (media_obj) {
      media_ref = json_object_get_string(media_obj, "$ref");
      if (media_ref) {
        out_hdr->content_ref =
            (c_cdd_strdup(media_ref, &_ast_strdup_220), _ast_strdup_220);
        if (!out_hdr->content_ref)
          return CDD_C_ERROR_MEMORY;
        if (resolve_refs && spec) {
          const struct OpenAPI_MediaType *mt =
              (find_component_media_type(spec, media_ref,
                                         &_ast_find_component_media_type_55),
               _ast_find_component_media_type_55);
          if (mt) {
            if (mt->schema_set)
              resolved_schema = &mt->schema;
            else if (mt->item_schema_set)
              resolved_schema = &mt->item_schema;
          }
        }
      } else {
        media_schema_val = json_object_get_value(media_obj, "schema");
        effective_schema_val = media_schema_val;
        effective_schema =
            media_schema_val ? json_value_get_object(media_schema_val) : NULL;
      }
    }
  }

  type = effective_schema ? json_object_get_string(effective_schema, "type")
                          : NULL;

  if (resolved_schema) {
    {
      cdd_c_error_t _rc = apply_schema_ref_to_header(out_hdr, resolved_schema);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    {
      cdd_c_error_t _rc = copy_schema_ref(&out_hdr->schema, resolved_schema);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    out_hdr->schema_set = 1;
  } else if (effective_schema_val &&
             json_value_get_type(effective_schema_val) == JSONBoolean) {
    out_hdr->schema.schema_is_boolean = 1;
    out_hdr->schema.schema_boolean_value =
        json_value_get_boolean(effective_schema_val);
    out_hdr->schema_set = 1;
  } else if (effective_schema) {
    {
      cdd_c_error_t _rc =
          parse_schema_ref(effective_schema, &parsed_schema, spec);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    parsed_schema_set = 1;
    if (apply_schema_ref_to_header(out_hdr, &parsed_schema) != 0) {
      free_schema_ref_content(&parsed_schema);
      return CDD_C_ERROR_MEMORY;
    }
    if (copy_schema_ref(&out_hdr->schema, &parsed_schema) != 0) {
      free_schema_ref_content(&parsed_schema);
      return CDD_C_ERROR_MEMORY;
    }
    out_hdr->schema_set = 1;
  }

  if (!out_hdr->type) {
    out_hdr->type = (c_cdd_strdup(type ? type : "string", &_ast_strdup_221),
                     _ast_strdup_221);
    if (!out_hdr->type) {
      if (parsed_schema_set)
        free_schema_ref_content(&parsed_schema);
      return CDD_C_ERROR_MEMORY;
    }
  }

  if (object_has_example_and_examples(hdr_obj)) {
    if (parsed_schema_set)
      free_schema_ref_content(&parsed_schema);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  {
    cdd_c_error_t rc = parse_examples_object(
        json_object_get_object(hdr_obj, "examples"), &out_hdr->examples,
        &out_hdr->n_examples, spec, resolve_refs);
    if (rc != CDD_C_SUCCESS) {
      if (parsed_schema_set)
        free_schema_ref_content(&parsed_schema);
      return rc;
    }
  }
  if (out_hdr->n_examples == 0) {
    {
      cdd_c_error_t _rc = parse_any_field(hdr_obj, "example", &out_hdr->example,
                                          &out_hdr->example_set);
      if (_rc != CDD_C_SUCCESS) {
        if (parsed_schema_set)
          free_schema_ref_content(&parsed_schema);
        return _rc;
      }
    }
  }
  if (out_hdr->example_set || out_hdr->n_examples > 0) {
    out_hdr->example_location = OA_EXAMPLE_LOC_OBJECT;
  } else if (media_obj && !media_ref) {
    {
      cdd_c_error_t rc = parse_media_examples(
          media_obj, &out_hdr->example, &out_hdr->example_set,
          &out_hdr->examples, &out_hdr->n_examples, spec, resolve_refs);
      if (rc != CDD_C_SUCCESS) {
        if (parsed_schema_set)
          free_schema_ref_content(&parsed_schema);
        return rc;
      }
    }
    if (out_hdr->example_set || out_hdr->n_examples > 0) {
      out_hdr->example_location = OA_EXAMPLE_LOC_MEDIA;
    }
  }

  {
    cdd_c_error_t _rc = collect_extensions(hdr_obj, &out_hdr->extensions_json);
    if (_rc != CDD_C_SUCCESS) {
      if (parsed_schema_set)
        free_schema_ref_content(&parsed_schema);
      return _rc;
    }
  }

  if (parsed_schema_set)
    free_schema_ref_content(&parsed_schema);

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses link parameters from the given input.
 */
cdd_c_error_t parse_link_parameters(const JSON_Object *params_obj,
                                    struct OpenAPI_LinkParam **out_params,
                                    size_t *out_count) {
  char *_ast_strdup_222 = NULL;
  size_t count, i;
  if (!out_params || !out_count)
    return CDD_C_SUCCESS;
  *out_params = NULL;
  *out_count = 0;
  if (!params_obj)
    return CDD_C_SUCCESS;

  count = json_object_get_count(params_obj);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out_params = (struct OpenAPI_LinkParam *)calloc(
      count, sizeof(struct OpenAPI_LinkParam));
  if (!*out_params)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(params_obj, i);
    const JSON_Value *val = json_object_get_value_at(params_obj, i);
    if (name) {
      (*out_params)[i].name =
          (c_cdd_strdup(name, &_ast_strdup_222), _ast_strdup_222);
      if (!(*out_params)[i].name)
        return CDD_C_ERROR_MEMORY;
    }
    if (val) {
      if (parse_any_value(val, &(*out_params)[i].value) != 0)
        return CDD_C_ERROR_MEMORY;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses link object from the given input.
 */
cdd_c_error_t parse_link_object(const JSON_Object *link_obj,
                                struct OpenAPI_Link *out_link,
                                const struct OpenAPI_Spec *spec,
                                int resolve_refs) {
  struct OpenAPI_Link *_ast_find_component_link_56;
  char *_ast_strdup_223 = NULL;
  char *_ast_strdup_224 = NULL;
  char *_ast_strdup_225 = NULL;
  char *_ast_strdup_226 = NULL;
  char *_ast_strdup_227 = NULL;
  char *_ast_strdup_228 = NULL;
  char *_ast_strdup_229 = NULL;
  const char *ref;
  const char *summary;
  const char *desc;
  const char *op_ref;
  const char *op_id;
  const JSON_Object *params_obj;
  const JSON_Value *req_body_val;
  const JSON_Object *server_obj;

  if (!link_obj || !out_link)
    return CDD_C_SUCCESS;

  ref = json_object_get_string(link_obj, "$ref");
  if (ref) {
    out_link->ref = (c_cdd_strdup(ref, &_ast_strdup_223), _ast_strdup_223);
    if (!out_link->ref)
      return CDD_C_ERROR_MEMORY;
    if (resolve_refs && spec) {
      const struct OpenAPI_Link *comp =
          (find_component_link(spec, ref, &_ast_find_component_link_56),
           _ast_find_component_link_56);
      if (comp) {
        {
          cdd_c_error_t _rc = copy_link_fields(out_link, comp);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }
    summary = json_object_get_string(link_obj, "summary");
    if (summary) {
      out_link->summary =
          (c_cdd_strdup(summary, &_ast_strdup_224), _ast_strdup_224);
      if (!out_link->summary)
        return CDD_C_ERROR_MEMORY;
    }
    desc = json_object_get_string(link_obj, "description");
    if (desc) {
      out_link->description =
          (c_cdd_strdup(desc, &_ast_strdup_225), _ast_strdup_225);
      if (!out_link->description)
        return CDD_C_ERROR_MEMORY;
    }
    return CDD_C_SUCCESS;
  }

  summary = json_object_get_string(link_obj, "summary");
  if (summary) {
    out_link->summary =
        (c_cdd_strdup(summary, &_ast_strdup_226), _ast_strdup_226);
    if (!out_link->summary)
      return CDD_C_ERROR_MEMORY;
  }
  desc = json_object_get_string(link_obj, "description");
  if (desc) {
    out_link->description =
        (c_cdd_strdup(desc, &_ast_strdup_227), _ast_strdup_227);
    if (!out_link->description)
      return CDD_C_ERROR_MEMORY;
  }

  op_ref = json_object_get_string(link_obj, "operationRef");
  if (op_ref) {
    out_link->operation_ref =
        (c_cdd_strdup(op_ref, &_ast_strdup_228), _ast_strdup_228);
    if (!out_link->operation_ref)
      return CDD_C_ERROR_MEMORY;
  }
  op_id = json_object_get_string(link_obj, "operationId");
  if (op_id) {
    out_link->operation_id =
        (c_cdd_strdup(op_id, &_ast_strdup_229), _ast_strdup_229);
    if (!out_link->operation_id)
      return CDD_C_ERROR_MEMORY;
  }
  if ((op_ref && op_id) || (!op_ref && !op_id))
    return CDD_C_ERROR_INVALID_ARGUMENT;
  {
    cdd_c_error_t _rc =
        collect_extensions(link_obj, &out_link->extensions_json);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  params_obj = json_object_get_object(link_obj, "parameters");
  {
    cdd_c_error_t _rc = parse_link_parameters(params_obj, &out_link->parameters,
                                              &out_link->n_parameters);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  req_body_val = json_object_get_value(link_obj, "requestBody");
  if (req_body_val) {
    out_link->request_body_set = 1;
    {
      cdd_c_error_t _rc =
          parse_any_value(req_body_val, &out_link->request_body);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  server_obj = json_object_get_object(link_obj, "server");
  if (server_obj) {
    out_link->server =
        (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
    if (!out_link->server)
      return CDD_C_ERROR_MEMORY;
    out_link->server_set = 1;
    {
      cdd_c_error_t _rc = parse_server_object(server_obj, out_link->server);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses links object from the given input.
 */
cdd_c_error_t parse_links_object(const JSON_Object *links,
                                 struct OpenAPI_Link **out_links,
                                 size_t *out_count,
                                 const struct OpenAPI_Spec *spec,
                                 int resolve_refs) {
  char *_ast_strdup_230 = NULL;
  size_t i, count;
  if (!out_links || !out_count)
    return CDD_C_SUCCESS;

  *out_links = NULL;
  *out_count = 0;

  if (!links)
    return CDD_C_SUCCESS;

  count = json_object_get_count(links);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out_links =
      (struct OpenAPI_Link *)calloc(count, sizeof(struct OpenAPI_Link));
  if (!*out_links)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(links, i);
    const JSON_Object *link_obj =
        json_value_get_object(json_object_get_value_at(links, i));
    struct OpenAPI_Link *curr = &(*out_links)[i];
    if (name) {
      curr->name = (c_cdd_strdup(name, &_ast_strdup_230), _ast_strdup_230);
      if (!curr->name)
        return CDD_C_ERROR_MEMORY;
    }
    if (link_obj) {
      {
        cdd_c_error_t _rc =
            parse_link_object(link_obj, curr, spec, resolve_refs);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses headers object from the given input.
 */
cdd_c_error_t parse_headers_object(const JSON_Object *headers,
                                   struct OpenAPI_Header **out_headers,
                                   size_t *out_count,
                                   const struct OpenAPI_Spec *spec,
                                   int resolve_refs, int ignore_content_type) {
  char *_ast_strdup_231 = NULL;
  size_t i, count, valid = 0;
  if (!out_headers || !out_count)
    return CDD_C_SUCCESS;

  *out_headers = NULL;
  *out_count = 0;

  if (!headers)
    return CDD_C_SUCCESS;

  count = json_object_get_count(headers);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out_headers =
      (struct OpenAPI_Header *)calloc(count, sizeof(struct OpenAPI_Header));
  if (!*out_headers)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(headers, i);
    const JSON_Object *h_obj =
        json_value_get_object(json_object_get_value_at(headers, i));
    struct OpenAPI_Header *curr = &(*out_headers)[valid];
    if (ignore_content_type && header_name_is_content_type(name))
      continue;
    if (name) {
      curr->name = (c_cdd_strdup(name, &_ast_strdup_231), _ast_strdup_231);
      if (!curr->name) {
        *out_count = valid;
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (h_obj) {
      cdd_c_error_t rc = parse_header_object(h_obj, curr, spec, resolve_refs);
      if (rc != CDD_C_SUCCESS) {
        *out_count = valid + 1;
        return rc;
      }
    }
    valid++;
  }

  if (valid == 0) {
    free(*out_headers);
    *out_headers = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }
  if (valid < count) {
    struct OpenAPI_Header *tmp = (struct OpenAPI_Header *)realloc(
        *out_headers, valid * sizeof(struct OpenAPI_Header));
    if (tmp)
      *out_headers = tmp;
  }
  *out_count = valid;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses encoding object from the given input.
 */
cdd_c_error_t parse_encoding_object(const JSON_Object *enc_obj,
                                    struct OpenAPI_Encoding *out,
                                    const struct OpenAPI_Spec *spec,
                                    int resolve_refs) {
  enum OpenAPI_Style _ast_parse_param_style_57;
  char *_ast_strdup_232 = NULL;
  const char *content_type;
  const char *style_str;
  int explode_present;
  int explode_val;
  int allow_reserved_present;
  int allow_reserved_val;
  const JSON_Object *headers_obj;
  const JSON_Object *nested_encoding_obj;
  const JSON_Array *prefix_encoding_arr;
  const JSON_Object *item_encoding_obj;

  if (!enc_obj || !out)
    return CDD_C_SUCCESS;

  {
    const int has_encoding = json_object_has_value(enc_obj, "encoding");
    const int has_prefix = json_object_has_value(enc_obj, "prefixEncoding");
    const int has_item = json_object_has_value(enc_obj, "itemEncoding");
    if (has_encoding && (has_prefix || has_item))
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  content_type = json_object_get_string(enc_obj, "contentType");
  if (content_type) {
    out->content_type =
        (c_cdd_strdup(content_type, &_ast_strdup_232), _ast_strdup_232);
    if (!out->content_type)
      return CDD_C_ERROR_MEMORY;
  }

  style_str = json_object_get_string(enc_obj, "style");
  if (style_str) {
    out->style = (parse_param_style(style_str, &_ast_parse_param_style_57),
                  _ast_parse_param_style_57);
    out->style_set = 1;
  }
  explode_present = json_object_has_value(enc_obj, "explode");
  explode_val = json_object_get_boolean(enc_obj, "explode");
  if (explode_present) {
    out->explode_set = 1;
    out->explode = (explode_val == 1);
  }
  allow_reserved_present = json_object_has_value(enc_obj, "allowReserved");
  allow_reserved_val = json_object_get_boolean(enc_obj, "allowReserved");
  if (allow_reserved_present) {
    out->allow_reserved_set = 1;
    out->allow_reserved = (allow_reserved_val == 1);
  }

  headers_obj = json_object_get_object(enc_obj, "headers");
  if (headers_obj) {
    cdd_c_error_t rc = parse_headers_object(
        headers_obj, &out->headers, &out->n_headers, spec, resolve_refs, 1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  nested_encoding_obj = json_object_get_object(enc_obj, "encoding");
  if (nested_encoding_obj) {
    {
      cdd_c_error_t _rc =
          parse_encoding_map(nested_encoding_obj, &out->encoding,
                             &out->n_encoding, spec, resolve_refs);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  prefix_encoding_arr = json_object_get_array(enc_obj, "prefixEncoding");
  if (prefix_encoding_arr) {
    {
      cdd_c_error_t _rc =
          parse_encoding_array(prefix_encoding_arr, &out->prefix_encoding,
                               &out->n_prefix_encoding, spec, resolve_refs);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  item_encoding_obj = json_object_get_object(enc_obj, "itemEncoding");
  if (item_encoding_obj) {
    out->item_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    if (!out->item_encoding)
      return CDD_C_ERROR_MEMORY;
    out->item_encoding_set = 1;
    {
      cdd_c_error_t _rc = parse_encoding_object(
          item_encoding_obj, out->item_encoding, spec, resolve_refs);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  {
    cdd_c_error_t _rc = collect_extensions(enc_obj, &out->extensions_json);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses encoding map from the given input.
 */
cdd_c_error_t parse_encoding_map(const JSON_Object *enc_obj,
                                 struct OpenAPI_Encoding **out,
                                 size_t *out_count,
                                 const struct OpenAPI_Spec *spec,
                                 int resolve_refs) {
  char *_ast_strdup_233 = NULL;
  size_t i, count, valid = 0;
  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!enc_obj)
    return CDD_C_SUCCESS;
  count = json_object_get_count(enc_obj);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out =
      (struct OpenAPI_Encoding *)calloc(count, sizeof(struct OpenAPI_Encoding));
  if (!*out)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(enc_obj, i);
    const JSON_Object *enc_def =
        json_value_get_object(json_object_get_value_at(enc_obj, i));
    struct OpenAPI_Encoding *curr = &(*out)[valid];
    if (!name || !enc_def)
      continue;
    curr->name = (c_cdd_strdup(name, &_ast_strdup_233), _ast_strdup_233);
    if (!curr->name)
      return CDD_C_ERROR_MEMORY;
    {
      cdd_c_error_t rc =
          parse_encoding_object(enc_def, curr, spec, resolve_refs);
      if (rc != CDD_C_SUCCESS) {
        *out_count = valid + 1;
        return rc;
      }
    }
    valid++;
  }

  if (valid == 0) {
    free(*out);
    *out = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }
  *out_count = valid;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses encoding array from the given input.
 */
cdd_c_error_t parse_encoding_array(const JSON_Array *enc_arr,
                                   struct OpenAPI_Encoding **out,
                                   size_t *out_count,
                                   const struct OpenAPI_Spec *spec,
                                   int resolve_refs) {
  size_t i, count, valid = 0;
  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!enc_arr)
    return CDD_C_SUCCESS;
  count = json_array_get_count(enc_arr);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out =
      (struct OpenAPI_Encoding *)calloc(count, sizeof(struct OpenAPI_Encoding));
  if (!*out)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const JSON_Object *enc_def = json_array_get_object(enc_arr, i);
    struct OpenAPI_Encoding *curr = &(*out)[valid];
    if (!enc_def)
      continue;
    {
      cdd_c_error_t rc =
          parse_encoding_object(enc_def, curr, spec, resolve_refs);
      if (rc != CDD_C_SUCCESS) {
        *out_count = valid + 1;
        return rc;
      }
    }
    valid++;
  }

  if (valid == 0) {
    free(*out);
    *out = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }
  *out_count = valid;
  return CDD_C_SUCCESS;
}
