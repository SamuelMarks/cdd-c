/**
 * @file openapi_headers_links.c
 * @brief Header, link, and encoding parsing.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Frees `OpenAPI_SchemaRef` content via a `*_free` wrapper.
 *
 * @param[in,out] sr SchemaRef structure to release.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t oa_hdr_schema_ref_free(struct OpenAPI_SchemaRef *sr) {
  return free_schema_ref_content(sr);
}

/**
 * @brief Parses header object from the given input.
 *
 * @param[in] hdr_obj The source header JSON object.
 * @param[out] out_hdr Output OpenAPI_Header structure.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_header_object(const JSON_Object *hdr_obj,
                                               struct OpenAPI_Header *out_hdr,
                                               const struct OpenAPI_Spec *spec,
                                               int resolve_refs) {
  const char *ref = NULL;
  const char *desc = NULL;
  const char *style_str = NULL;
  const char *collection_format = NULL;
  int required_present = 0;
  int required_val = 0;
  int deprecated_present = 0;
  int deprecated_val = 0;
  int explode_present = 0;
  int explode_val = 0;
  JSON_Value *schema_val = NULL;
  JSON_Object *schema = NULL;
  JSON_Object *content = NULL;
  const JSON_Object *effective_schema = NULL;
  const JSON_Value *effective_schema_val = NULL;
  JSON_Object *media_obj = NULL;
  JSON_Value *media_schema_val = NULL;
  const char *media_type = NULL;
  const char *media_ref = NULL;
  const struct OpenAPI_SchemaRef *resolved_schema = NULL;
  struct OpenAPI_SchemaRef parsed_schema = {0};
  JSON_Object *examples_obj = NULL;
  cdd_c_error_t rc;

  if (!hdr_obj || !out_hdr)
    return CDD_C_SUCCESS;

  rc = oa_json_opt_string(hdr_obj, "$ref", &ref);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (ref) {
    rc = c_cdd_strdup(ref, &out_hdr->ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (resolve_refs && spec) {
      struct OpenAPI_Header *comp = NULL;
      rc = find_component_header(spec, ref, &comp);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (comp) {
        rc = copy_header_fields(out_hdr, comp);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    rc = oa_json_opt_string(hdr_obj, "description", &desc);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (desc) {
      C_CDD_FREE(out_hdr->description);
      out_hdr->description = NULL;
      rc = c_cdd_strdup(desc, &out_hdr->description);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return CDD_C_SUCCESS;
  }

  rc = oa_json_opt_string(hdr_obj, "description", &desc);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (desc) {
    rc = c_cdd_strdup(desc, &out_hdr->description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = oa_json_opt_bool(hdr_obj, "required", &required_present, &required_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (required_present)
    out_hdr->required = required_val;

  rc = oa_json_opt_bool(hdr_obj, "deprecated", &deprecated_present,
                        &deprecated_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (deprecated_present) {
    out_hdr->deprecated_set = 1;
    out_hdr->deprecated = deprecated_val;
  }

  rc = oa_json_opt_string(hdr_obj, "style", &style_str);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_json_opt_string(hdr_obj, "collectionFormat", &collection_format);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (collection_format) {
    rc = c_cdd_strdup(collection_format, &out_hdr->collection_format);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (style_str) {
    enum OpenAPI_Style parsed_style = OA_STYLE_UNKNOWN;
    rc = parse_param_style(style_str, &parsed_style);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (parsed_style != OA_STYLE_SIMPLE)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    out_hdr->style_set = 1;
    out_hdr->style = parsed_style;
  } else {
    out_hdr->style = OA_STYLE_SIMPLE;
  }

  rc = oa_json_opt_bool(hdr_obj, "explode", &explode_present, &explode_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (explode_present) {
    out_hdr->explode_set = 1;
    out_hdr->explode = explode_val;
  } else if (out_hdr->collection_format) {
    out_hdr->explode_set = 1;
    out_hdr->explode = 0;
  }

  rc = oa_json_opt_value(hdr_obj, "schema", &schema_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (schema_val) {
    rc = oa_json_val_opt_object(schema_val, &schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  rc = oa_json_opt_object(hdr_obj, "content", &content);
  if (rc != CDD_C_SUCCESS)
    return rc;
  {
    const int has_schema = (schema_val != NULL);
    int has_content = (content != NULL);
    if (has_schema && has_content)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (content) {
    size_t content_count = 0;
    rc = cdd_json_object_get_count(content, &content_count);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (content_count != 1)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = parse_content_object(content, &out_hdr->content_media_types,
                              &out_hdr->n_content_media_types, spec,
                              resolve_refs);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  effective_schema = schema;
  effective_schema_val = schema_val;
  if (!effective_schema && spec && spec->swagger_version) {
    int has_type = 0;
    rc = oa_json_has_value(hdr_obj, "type", &has_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (has_type) {
      effective_schema = hdr_obj;
      effective_schema_val = hdr_obj->wrapping_value;
    }
  }
  media_obj = NULL;
  media_schema_val = NULL;
  media_type = NULL;
  media_ref = NULL;
  resolved_schema = NULL;

  if (content) {
    rc = cdd_json_object_get_name(content, 0, &media_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = oa_json_opt_object(content, media_type, &media_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = c_cdd_strdup(media_type, &out_hdr->content_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (media_obj) {
      rc = oa_json_opt_string(media_obj, "$ref", &media_ref);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (media_ref) {
        rc = c_cdd_strdup(media_ref, &out_hdr->content_ref);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (resolve_refs && spec) {
          struct OpenAPI_MediaType *mt = NULL;
          rc = find_component_media_type(spec, media_ref, &mt);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (mt) {
            if (mt->schema_set)
              resolved_schema = &mt->schema;
            else if (mt->item_schema_set)
              resolved_schema = &mt->item_schema;
          }
        }
      } else {
        JSON_Object *ms_obj = NULL;
        rc = oa_json_opt_value(media_obj, "schema", &media_schema_val);
        if (rc != CDD_C_SUCCESS)
          return rc;
        effective_schema_val = media_schema_val;
        if (media_schema_val) {
          rc = oa_json_val_opt_object(media_schema_val, &ms_obj);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        effective_schema = ms_obj;
      }
    }
  }

  {
    JSON_Value_Type eff_vtype = JSONError;
    if (effective_schema_val) {
      rc = cdd_json_value_get_type(effective_schema_val, &eff_vtype);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (resolved_schema) {
      rc = apply_schema_ref_to_header(out_hdr, resolved_schema);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = copy_schema_ref(&out_hdr->schema, resolved_schema);
      if (rc != CDD_C_SUCCESS)
        return rc;
      out_hdr->schema_set = 1;
    } else if (effective_schema_val && eff_vtype == JSONBoolean) {
      out_hdr->schema.schema_is_boolean = 1;
      rc = cdd_json_value_get_boolean(effective_schema_val,
                                      &out_hdr->schema.schema_boolean_value);
      if (rc != CDD_C_SUCCESS)
        return rc;
      out_hdr->schema_set = 1;
    } else if (effective_schema) {
      rc = parse_schema_ref(effective_schema, &parsed_schema, spec);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = apply_schema_ref_to_header(out_hdr, &parsed_schema);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = copy_schema_ref(&out_hdr->schema, &parsed_schema);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      out_hdr->schema_set = 1;
    }
  }

  if (!out_hdr->type) {
    rc = c_cdd_strdup("string", &out_hdr->type);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  {
    int has_both = 0;
    rc = object_has_example_and_examples(hdr_obj, &has_both);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (has_both) {
      rc = CDD_C_ERROR_INVALID_ARGUMENT;
      goto cleanup;
    }
  }

  rc = oa_json_opt_object(hdr_obj, "examples", &examples_obj);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = parse_examples_object(examples_obj, &out_hdr->examples,
                             &out_hdr->n_examples, spec, resolve_refs);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  if (out_hdr->n_examples == 0) {
    rc = parse_any_field(hdr_obj, "example", &out_hdr->example,
                         &out_hdr->example_set);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (out_hdr->example_set || out_hdr->n_examples > 0) {
    out_hdr->example_location = OA_EXAMPLE_LOC_OBJECT;
  } else if (media_obj && !media_ref) {
    rc = parse_media_examples(media_obj, &out_hdr->example,
                              &out_hdr->example_set, &out_hdr->examples,
                              &out_hdr->n_examples, spec, resolve_refs);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (out_hdr->example_set || out_hdr->n_examples > 0) {
      out_hdr->example_location = OA_EXAMPLE_LOC_MEDIA;
    }
  }

  rc = collect_extensions(hdr_obj, &out_hdr->extensions_json);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = CDD_C_SUCCESS;
cleanup: {
  cdd_c_error_t clean_rc = oa_hdr_schema_ref_free(&parsed_schema);
  return cdd_c_keep_err(rc, clean_rc);
}
}

/**
 * @brief Parses link parameters from the given input.
 *
 * @param[in] params_obj The source link parameters JSON object.
 * @param[out] out_params Output array of OpenAPI_LinkParam structures.
 * @param[out] out_count Output count of link parameters.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_link_parameters(
    const JSON_Object *params_obj, struct OpenAPI_LinkParam **out_params,
    size_t *out_count) {
  size_t count = 0, i;
  cdd_c_error_t rc;
  if (!out_params || !out_count)
    return CDD_C_SUCCESS;
  *out_params = NULL;
  *out_count = 0;
  if (!params_obj)
    return CDD_C_SUCCESS;

  rc = cdd_json_object_get_count(params_obj, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (count == 0)
    return CDD_C_SUCCESS;

  *out_params = (struct OpenAPI_LinkParam *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_LinkParam));
  if (!*out_params)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val = NULL;
    rc = cdd_json_object_get_name(params_obj, i, &name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_object_get_value_at(params_obj, i, &val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = c_cdd_strdup(name, &(*out_params)[i].name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_any_value(val, &(*out_params)[i].value);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses link object from the given input.
 *
 * @param[in] link_obj The source link JSON object.
 * @param[out] out_link Output OpenAPI_Link structure.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_link_object(const JSON_Object *link_obj,
                                             struct OpenAPI_Link *out_link,
                                             const struct OpenAPI_Spec *spec,
                                             int resolve_refs) {
  const char *ref = NULL;
  const char *summary = NULL;
  const char *desc = NULL;
  JSON_Object *params_obj = NULL;
  JSON_Value *req_body_val = NULL;
  JSON_Object *server_obj = NULL;
  cdd_c_error_t rc;

  if (!link_obj || !out_link)
    return CDD_C_SUCCESS;

  rc = oa_json_opt_string(link_obj, "$ref", &ref);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (ref) {
    rc = c_cdd_strdup(ref, &out_link->ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (resolve_refs && spec) {
      struct OpenAPI_Link *comp = NULL;
      rc = find_component_link(spec, ref, &comp);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (comp) {
        rc = copy_link_fields(out_link, comp);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    rc = oa_json_opt_string(link_obj, "summary", &summary);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (summary) {
      C_CDD_FREE(out_link->summary);
      out_link->summary = NULL;
      rc = c_cdd_strdup(summary, &out_link->summary);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    rc = oa_json_opt_string(link_obj, "description", &desc);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (desc) {
      C_CDD_FREE(out_link->description);
      out_link->description = NULL;
      rc = c_cdd_strdup(desc, &out_link->description);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return CDD_C_SUCCESS;
  }

  rc = openapi_parse_reflected_strings(link_obj, &OpenAPI_Link_type_info,
                                       out_link);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if ((out_link->operation_ref && out_link->operation_id) ||
      (!out_link->operation_ref && !out_link->operation_id))
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = collect_extensions(link_obj, &out_link->extensions_json);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = oa_json_opt_object(link_obj, "parameters", &params_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_link_parameters(params_obj, &out_link->parameters,
                             &out_link->n_parameters);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = oa_json_opt_value(link_obj, "requestBody", &req_body_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (req_body_val) {
    out_link->request_body_set = 1;
    rc = parse_any_value(req_body_val, &out_link->request_body);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = oa_json_opt_object(link_obj, "server", &server_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (server_obj) {
    out_link->server =
        (struct OpenAPI_Server *)C_CDD_CALLOC(1, sizeof(struct OpenAPI_Server));
    if (!out_link->server)
      return CDD_C_ERROR_MEMORY;
    out_link->server_set = 1;
    rc = parse_server_object(server_obj, out_link->server);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses links object from the given input.
 *
 * @param[in] links The source links map JSON object.
 * @param[out] out_links Output array of OpenAPI_Link structures.
 * @param[out] out_count Output count of links.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_links_object(const JSON_Object *links,
                                              struct OpenAPI_Link **out_links,
                                              size_t *out_count,
                                              const struct OpenAPI_Spec *spec,
                                              int resolve_refs) {
  size_t i, count = 0;
  cdd_c_error_t rc;
  if (!out_links || !out_count)
    return CDD_C_SUCCESS;

  *out_links = NULL;
  *out_count = 0;

  if (!links)
    return CDD_C_SUCCESS;

  rc = cdd_json_object_get_count(links, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (count == 0)
    return CDD_C_SUCCESS;

  *out_links =
      (struct OpenAPI_Link *)C_CDD_CALLOC(count, sizeof(struct OpenAPI_Link));
  if (!*out_links)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *link_obj = NULL;
    struct OpenAPI_Link *curr = &(*out_links)[i];
    rc = cdd_json_object_get_name(links, i, &name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_object_get_value_at(links, i, &val_at);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = oa_json_val_opt_object(val_at, &link_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = c_cdd_strdup(name, &curr->name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (link_obj) {
      rc = parse_link_object(link_obj, curr, spec, resolve_refs);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses headers object from the given input.
 *
 * @param[in] headers The source headers map JSON object.
 * @param[out] out_headers Output array of OpenAPI_Header structures.
 * @param[out] out_count Output count of headers.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @param[in] ignore_content_type Non-zero to skip Content-Type headers.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_headers_object(
    const JSON_Object *headers, struct OpenAPI_Header **out_headers,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs,
    int ignore_content_type) {
  size_t i, count = 0, valid = 0;
  cdd_c_error_t rc;
  if (!out_headers || !out_count)
    return CDD_C_SUCCESS;

  *out_headers = NULL;
  *out_count = 0;

  if (!headers)
    return CDD_C_SUCCESS;

  rc = cdd_json_object_get_count(headers, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (count == 0)
    return CDD_C_SUCCESS;

  *out_headers = (struct OpenAPI_Header *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_Header));
  if (!*out_headers)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *h_obj = NULL;
    struct OpenAPI_Header *curr = &(*out_headers)[valid];
    rc = cdd_json_object_get_name(headers, i, &name);
    if (rc != CDD_C_SUCCESS) {
      *out_count = valid;
      return rc;
    }
    rc = cdd_json_object_get_value_at(headers, i, &val_at);
    if (rc != CDD_C_SUCCESS) {
      *out_count = valid;
      return rc;
    }
    rc = oa_json_val_opt_object(val_at, &h_obj);
    if (rc != CDD_C_SUCCESS) {
      *out_count = valid;
      return rc;
    }
    if (ignore_content_type) {
      int is_ct = 0;
      rc = header_name_is_content_type(name, &is_ct);
      if (rc != CDD_C_SUCCESS) {
        *out_count = valid;
        return rc;
      }
      if (is_ct)
        continue;
    }
    rc = c_cdd_strdup(name, &curr->name);
    if (rc != CDD_C_SUCCESS) {
      *out_count = valid;
      return rc;
    }
    if (h_obj) {
      rc = parse_header_object(h_obj, curr, spec, resolve_refs);
      if (rc != CDD_C_SUCCESS) {
        *out_count = valid + 1;
        return rc;
      }
    }
    valid++;
  }

  if (valid == 0) {
    C_CDD_FREE(*out_headers);
    *out_headers = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }
  if (valid < count) {
    struct OpenAPI_Header *tmp = (struct OpenAPI_Header *)C_CDD_REALLOC(
        *out_headers, valid * sizeof(struct OpenAPI_Header));
    if (tmp)
      *out_headers = tmp;
  }
  *out_count = valid;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses an OpenAPI Encoding object.
 *
 * @param[in] enc_obj JSON object representing the Encoding.
 * @param[out] out Destination encoding structure.
 * @param[in] spec Parent OpenAPI specification context.
 * @param[in] resolve_refs Non-zero to resolve `$ref` against components.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
parse_encoding_object(const JSON_Object *enc_obj, struct OpenAPI_Encoding *out,
                      const struct OpenAPI_Spec *spec, int resolve_refs) {
  const char *content_type = NULL;
  const char *style_str = NULL;
  int explode_present = 0;
  int explode_val = 0;
  int allow_reserved_present = 0;
  int allow_reserved_val = 0;
  JSON_Object *headers_obj = NULL;
  JSON_Object *nested_encoding_obj = NULL;
  JSON_Array *prefix_encoding_arr = NULL;
  JSON_Object *item_encoding_obj = NULL;
  cdd_c_error_t rc;

  if (!enc_obj || !out)
    return CDD_C_SUCCESS;

  {
    int has_encoding = 0;
    int has_prefix = 0;
    int has_item = 0;
    rc = oa_json_has_value(enc_obj, "encoding", &has_encoding);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = oa_json_has_value(enc_obj, "prefixEncoding", &has_prefix);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = oa_json_has_value(enc_obj, "itemEncoding", &has_item);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (has_encoding && (has_prefix || has_item))
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = oa_json_opt_string(enc_obj, "contentType", &content_type);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (content_type) {
    rc = c_cdd_strdup(content_type, &out->content_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = oa_json_opt_string(enc_obj, "style", &style_str);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (style_str) {
    rc = parse_param_style(style_str, &out->style);
    if (rc != CDD_C_SUCCESS)
      return rc;
    out->style_set = 1;
  }
  rc = oa_json_opt_bool(enc_obj, "explode", &explode_present, &explode_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (explode_present) {
    out->explode_set = 1;
    out->explode = explode_val;
  }
  rc = oa_json_opt_bool(enc_obj, "allowReserved", &allow_reserved_present,
                        &allow_reserved_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (allow_reserved_present) {
    out->allow_reserved_set = 1;
    out->allow_reserved = allow_reserved_val;
  }

  rc = oa_json_opt_object(enc_obj, "headers", &headers_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (headers_obj) {
    rc = parse_headers_object(headers_obj, &out->headers, &out->n_headers, spec,
                              resolve_refs, 1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = oa_json_opt_object(enc_obj, "encoding", &nested_encoding_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (nested_encoding_obj) {
    rc = parse_encoding_map(nested_encoding_obj, &out->encoding,
                            &out->n_encoding, spec, resolve_refs);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = oa_json_opt_array(enc_obj, "prefixEncoding", &prefix_encoding_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (prefix_encoding_arr) {
    rc = parse_encoding_array(prefix_encoding_arr, &out->prefix_encoding,
                              &out->n_prefix_encoding, spec, resolve_refs);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = oa_json_opt_object(enc_obj, "itemEncoding", &item_encoding_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (item_encoding_obj) {
    out->item_encoding = (struct OpenAPI_Encoding *)C_CDD_CALLOC(
        1, sizeof(struct OpenAPI_Encoding));
    if (!out->item_encoding)
      return CDD_C_ERROR_MEMORY;
    out->item_encoding_set = 1;
    rc = parse_encoding_object(item_encoding_obj, out->item_encoding, spec,
                               resolve_refs);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return collect_extensions(enc_obj, &out->extensions_json);
}

/**
 * @brief Parses an `encoding` map of Encoding objects.
 *
 * @param[in] enc_obj JSON object mapping property names to Encoding objects.
 * @param[out] out Output array of parsed encodings.
 * @param[out] out_count Output number of parsed encodings.
 * @param[in] spec Parent OpenAPI specification context.
 * @param[in] resolve_refs Non-zero to resolve `$ref` against components.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_encoding_map(const JSON_Object *enc_obj,
                                              struct OpenAPI_Encoding **out,
                                              size_t *out_count,
                                              const struct OpenAPI_Spec *spec,
                                              int resolve_refs) {
  size_t i, count = 0, valid = 0;
  cdd_c_error_t rc;
  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!enc_obj)
    return CDD_C_SUCCESS;
  rc = cdd_json_object_get_count(enc_obj, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (count == 0)
    return CDD_C_SUCCESS;

  *out = (struct OpenAPI_Encoding *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_Encoding));
  if (!*out)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    JSON_Value *val_at = NULL;
    JSON_Object *enc_def = NULL;
    struct OpenAPI_Encoding *curr = &(*out)[valid];
    rc = cdd_json_object_get_name(enc_obj, i, &name);
    if (rc != CDD_C_SUCCESS) {
      *out_count = valid;
      return rc;
    }
    rc = cdd_json_object_get_value_at(enc_obj, i, &val_at);
    if (rc != CDD_C_SUCCESS) {
      *out_count = valid;
      return rc;
    }
    rc = oa_json_val_opt_object(val_at, &enc_def);
    if (rc != CDD_C_SUCCESS) {
      *out_count = valid;
      return rc;
    }
    if (!enc_def)
      continue;
    rc = c_cdd_strdup(name, &curr->name);
    if (rc != CDD_C_SUCCESS) {
      *out_count = valid;
      return rc;
    }
    rc = parse_encoding_object(enc_def, curr, spec, resolve_refs);
    if (rc != CDD_C_SUCCESS) {
      *out_count = valid + 1;
      return rc;
    }
    valid++;
  }

  if (valid == 0) {
    C_CDD_FREE(*out);
    *out = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }
  *out_count = valid;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a `prefixEncoding` array of Encoding objects.
 *
 * @param[in] enc_arr JSON array of Encoding objects.
 * @param[out] out Output array of parsed encodings.
 * @param[out] out_count Output number of parsed encodings.
 * @param[in] spec Parent OpenAPI specification context.
 * @param[in] resolve_refs Non-zero to resolve `$ref` against components.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_encoding_array(const JSON_Array *enc_arr,
                                                struct OpenAPI_Encoding **out,
                                                size_t *out_count,
                                                const struct OpenAPI_Spec *spec,
                                                int resolve_refs) {
  size_t i, count = 0, valid = 0;
  cdd_c_error_t rc;
  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!enc_arr)
    return CDD_C_SUCCESS;
  rc = cdd_json_array_get_count(enc_arr, &count);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (count == 0)
    return CDD_C_SUCCESS;

  *out = (struct OpenAPI_Encoding *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_Encoding));
  if (!*out)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    JSON_Object *enc_def = NULL;
    struct OpenAPI_Encoding *curr = &(*out)[valid];
    rc = oa_json_array_opt_object(enc_arr, i, &enc_def);
    if (rc != CDD_C_SUCCESS) {
      *out_count = valid;
      return rc;
    }
    if (!enc_def)
      continue;
    rc = parse_encoding_object(enc_def, curr, spec, resolve_refs);
    if (rc != CDD_C_SUCCESS) {
      *out_count = valid + 1;
      return rc;
    }
    valid++;
  }

  if (valid == 0) {
    C_CDD_FREE(*out);
    *out = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }
  *out_count = valid;
  return CDD_C_SUCCESS;
}
