/**
 * @file openapi_responses.c
 * @brief Response, request body, and callback parsing.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Parses request body object from the given input.
 */
cdd_c_error_t parse_request_body_object(const JSON_Object *rb_obj,
                                        struct OpenAPI_RequestBody *out_rb,
                                        const struct OpenAPI_Spec *spec,
                                        int resolve_refs, const char *op_id) {
  struct OpenAPI_RequestBody *_ast_find_component_request_body_68;
  JSON_Object *_ast_find_media_object_by_name_69;
  char *_ast_build_inline_request_name_70 = NULL;
  char *_ast_build_inline_request_name_71 = NULL;
  char *_ast_build_inline_request_name_72 = NULL;
  char *_ast_strdup_243 = NULL;
  char *_ast_strdup_244 = NULL;
  char *_ast_strdup_245 = NULL;
  char *_ast_strdup_246 = NULL;
  char *_ast_strdup_247 = NULL;
  const char *ref;
  const char *desc;
  int required_present;
  int required_val;
  const JSON_Object *content;
  struct OpenAPI_MediaType *primary = NULL;
  int primary_idx = -1;

  if (!rb_obj || !out_rb)
    return CDD_C_SUCCESS;

  ref = json_object_get_string(rb_obj, "$ref");
  if (ref) {
    out_rb->ref = (c_cdd_strdup(ref, &_ast_strdup_243), _ast_strdup_243);
    if (!out_rb->ref)
      return CDD_C_ERROR_MEMORY;
    if (resolve_refs && spec) {
      const struct OpenAPI_RequestBody *comp =
          (find_component_request_body(spec, ref,
                                       &_ast_find_component_request_body_68),
           _ast_find_component_request_body_68);
      if (comp) {
        {
          cdd_c_error_t _rc = copy_request_body_fields(out_rb, comp);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }
    desc = json_object_get_string(rb_obj, "description");
    if (desc) {
      if (out_rb->description) {
        free(out_rb->description);
        out_rb->description = NULL;
      }
      out_rb->description =
          (c_cdd_strdup(desc, &_ast_strdup_244), _ast_strdup_244);
      if (!out_rb->description)
        return CDD_C_ERROR_MEMORY;
    }
    return CDD_C_SUCCESS;
  }

  desc = json_object_get_string(rb_obj, "description");
  if (desc) {
    if (out_rb->description) {
      free(out_rb->description);
      out_rb->description = NULL;
    }
    out_rb->description =
        (c_cdd_strdup(desc, &_ast_strdup_245), _ast_strdup_245);
    if (!out_rb->description)
      return CDD_C_ERROR_MEMORY;
  }

  required_present = json_object_has_value(rb_obj, "required");
  required_val = json_object_get_boolean(rb_obj, "required");
  if (required_present) {
    out_rb->required_set = 1;
    out_rb->required = (required_val == 1);
  }

  content = json_object_get_object(rb_obj, "content");
  if (!content || json_object_get_count(content) == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  {
    cdd_c_error_t rc = parse_content_object(
        content, &out_rb->content_media_types, &out_rb->n_content_media_types,
        spec, resolve_refs);
    if (rc != CDD_C_SUCCESS) {
      size_t k;
      for (k = 0; k < out_rb->n_content_media_types; k++) {
        free_media_type(&out_rb->content_media_types[k]);
      }
      free(out_rb->content_media_types);
      out_rb->content_media_types = NULL;
      out_rb->n_content_media_types = 0;
      return rc;
    }
    select_primary_media_type_index(out_rb->content_media_types,
                                    out_rb->n_content_media_types,
                                    &primary_idx);
    if (primary_idx >= 0) {
      primary = &out_rb->content_media_types[primary_idx];
    }
    if (primary) {
      if (spec && op_id) {
        const JSON_Object *media_obj =
            (find_media_object_by_name(content, primary->name,
                                       &_ast_find_media_object_by_name_69),
             _ast_find_media_object_by_name_69);
        const JSON_Value *schema_val =
            json_object_get_value(media_obj, "schema");
        const JSON_Value *item_schema_val =
            json_object_get_value(media_obj, "itemSchema");
        const JSON_Object *schema_obj =
            schema_val ? json_value_get_object(schema_val) : NULL;
        const JSON_Object *item_schema_obj =
            item_schema_val ? json_value_get_object(item_schema_val) : NULL;
        if (primary->schema_set && schema_obj) {
          if (primary->schema.is_array) {
            const JSON_Value *items_val =
                json_object_get_value(schema_obj, "items");
            const JSON_Object *items_obj =
                items_val ? json_value_get_object(items_val) : NULL;
            if (items_obj && schema_object_is_object_like(items_obj)) {
              char *base = (build_inline_request_name(
                                op_id, 1, &_ast_build_inline_request_name_70),
                            _ast_build_inline_request_name_70);
              char *registered = NULL;
              if (base) {
                if (register_inline_schema((struct OpenAPI_Spec *)spec, base,
                                           items_obj, items_val,
                                           &registered) == CDD_C_SUCCESS) {
                  free(primary->schema.inline_type);
                  primary->schema.inline_type = NULL;
                  {
                    cdd_c_error_t _rc =
                        assign_schema_ref_name(&primary->schema, registered);
                    if (_rc != CDD_C_SUCCESS)
                      return _rc;
                  }
                }
                free(base);
              }
            }
          } else if (schema_object_is_object_like(schema_obj)) {
            char *base = (build_inline_request_name(
                              op_id, 0, &_ast_build_inline_request_name_71),
                          _ast_build_inline_request_name_71);
            char *registered = NULL;
            if (base) {
              if (register_inline_schema((struct OpenAPI_Spec *)spec, base,
                                         schema_obj, schema_val,
                                         &registered) == CDD_C_SUCCESS) {
                if (primary->schema.inline_type) {
                  free(primary->schema.inline_type);
                  primary->schema.inline_type = NULL;
                }
                {
                  cdd_c_error_t _rc =
                      assign_schema_ref_name(&primary->schema, registered);
                  if (_rc != CDD_C_SUCCESS)
                    return _rc;
                }
              }
              free(base);
            }
          }
        }
        if (primary->item_schema_set && item_schema_obj &&
            schema_object_is_object_like(item_schema_obj)) {
          char *base = (build_inline_request_name(
                            op_id, 1, &_ast_build_inline_request_name_72),
                        _ast_build_inline_request_name_72);
          char *registered = NULL;
          if (base) {
            if (register_inline_schema((struct OpenAPI_Spec *)spec, base,
                                       item_schema_obj, item_schema_val,
                                       &registered) == CDD_C_SUCCESS) {
              if (primary->item_schema.inline_type) {
                free(primary->item_schema.inline_type);
                primary->item_schema.inline_type = NULL;
              }
              {
                cdd_c_error_t _rc =
                    assign_schema_ref_name(&primary->item_schema, registered);
                if (_rc != CDD_C_SUCCESS)
                  return _rc;
              }
            }
            free(base);
          }
        }
      }
      if (primary->ref) {
        out_rb->content_ref =
            (c_cdd_strdup(primary->ref, &_ast_strdup_246), _ast_strdup_246);
        if (!out_rb->content_ref)
          return CDD_C_ERROR_MEMORY;
      }
      if (primary->schema_set) {
        {
          cdd_c_error_t _rc =
              copy_schema_ref(&out_rb->schema, &primary->schema);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      } else if (primary->item_schema_set) {
        {
          cdd_c_error_t _rc =
              copy_item_schema_as_array(&out_rb->schema, &primary->item_schema);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
      if (primary->examples) {
        out_rb->examples = (struct OpenAPI_Example *)C_CDD_CALLOC(
            primary->n_examples, sizeof(struct OpenAPI_Example));
        if (!out_rb->examples)
          return CDD_C_ERROR_MEMORY;
        out_rb->n_examples = primary->n_examples;
        {
          size_t i;
          for (i = 0; i < primary->n_examples; ++i) {
            {
              cdd_c_error_t _rc = copy_example_fields(&out_rb->examples[i],
                                                      &primary->examples[i]);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
          }
        }
      } else if (primary->example_set) {
        {
          cdd_c_error_t _rc =
              copy_any_value(&out_rb->example, &primary->example);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        out_rb->example_set = 1;
      }
      out_rb->schema.content_type =
          (c_cdd_strdup(primary->name, &_ast_strdup_247), _ast_strdup_247);
      if (!out_rb->schema.content_type)
        return CDD_C_ERROR_MEMORY;
    }
  }

  {
    cdd_c_error_t _rc = collect_extensions(rb_obj, &out_rb->extensions_json);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses response object from the given input.
 */
cdd_c_error_t parse_response_object(const JSON_Object *resp_obj,
                                    struct OpenAPI_Response *out_resp,
                                    const struct OpenAPI_Spec *spec,
                                    int resolve_refs, const char *op_id,
                                    const char *resp_code) {
  struct OpenAPI_Response *_ast_find_component_response_73;
  JSON_Object *_ast_find_media_object_by_name_74;
  char *_ast_build_inline_response_name_75 = NULL;
  char *_ast_build_inline_response_name_76 = NULL;
  char *_ast_build_inline_response_name_77 = NULL;
  char *_ast_strdup_252 = NULL;
  char *_ast_strdup_253 = NULL;
  char *_ast_strdup_254 = NULL;
  char *_ast_strdup_255 = NULL;
  char *_ast_strdup_256 = NULL;
  const char *ref;
  const char *summary;
  const char *desc;
  const JSON_Object *content;
  const JSON_Object *headers;
  const JSON_Object *links;
  if (!resp_obj || !out_resp)
    return CDD_C_SUCCESS;

  ref = json_object_get_string(resp_obj, "$ref");
  if (ref) {
    out_resp->ref = (c_cdd_strdup(ref, &_ast_strdup_252), _ast_strdup_252);
    if (!out_resp->ref)
      return CDD_C_ERROR_MEMORY;
    if (resolve_refs && spec) {
      const struct OpenAPI_Response *comp =
          (find_component_response(spec, ref, &_ast_find_component_response_73),
           _ast_find_component_response_73);
      if (comp) {
        {
          cdd_c_error_t _rc = copy_response_fields(out_resp, comp);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }
  }

  summary = json_object_get_string(resp_obj, "summary");
  if (summary) {
    out_resp->summary =
        (c_cdd_strdup(summary, &_ast_strdup_253), _ast_strdup_253);
    if (!out_resp->summary)
      return CDD_C_ERROR_MEMORY;
  }

  desc = json_object_get_string(resp_obj, "description");
  if (desc) {
    out_resp->description =
        (c_cdd_strdup(desc, &_ast_strdup_254), _ast_strdup_254);
    if (!out_resp->description)
      return CDD_C_ERROR_MEMORY;
  }

  if (!ref) {
    {
      cdd_c_error_t _rc =
          collect_extensions(resp_obj, &out_resp->extensions_json);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  if (ref)
    return CDD_C_SUCCESS;

  if (spec && spec->swagger_version) {
    const JSON_Value *schema_val = json_object_get_value(resp_obj, "schema");
    const JSON_Object *schema_obj =
        schema_val ? json_value_get_object(schema_val) : NULL;
    if (schema_obj) {
      cdd_c_error_t rc = parse_schema_ref(schema_obj, &out_resp->schema, spec);
      if (rc != CDD_C_SUCCESS)
        return rc;
      out_resp->schema_set = 1;
    }
  }

  headers = json_object_get_object(resp_obj, "headers");
  if (headers) {
    cdd_c_error_t rc =
        parse_headers_object(headers, &out_resp->headers, &out_resp->n_headers,
                             spec, resolve_refs, 1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  links = json_object_get_object(resp_obj, "links");
  if (links) {
    {
      cdd_c_error_t _rc = parse_links_object(
          links, &out_resp->links, &out_resp->n_links, spec, resolve_refs);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  content = json_object_get_object(resp_obj, "content");
  if (content) {
    struct OpenAPI_MediaType *primary = NULL;
    int primary_idx = -1;
    {
      cdd_c_error_t _rc = parse_content_object(
          content, &out_resp->content_media_types,
          &out_resp->n_content_media_types, spec, resolve_refs);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    select_primary_media_type_index(out_resp->content_media_types,
                                    out_resp->n_content_media_types,
                                    &primary_idx);
    if (primary_idx >= 0) {
      primary = &out_resp->content_media_types[primary_idx];
    }
    if (primary) {
      if (spec && op_id && resp_code) {
        const JSON_Object *media_obj =
            (find_media_object_by_name(content, primary->name,
                                       &_ast_find_media_object_by_name_74),
             _ast_find_media_object_by_name_74);
        const JSON_Value *schema_val =
            json_object_get_value(media_obj, "schema");
        const JSON_Value *item_schema_val =
            json_object_get_value(media_obj, "itemSchema");
        const JSON_Object *schema_obj =
            schema_val ? json_value_get_object(schema_val) : NULL;
        const JSON_Object *item_schema_obj =
            item_schema_val ? json_value_get_object(item_schema_val) : NULL;
        if (primary->schema_set && schema_obj) {
          if (primary->schema.is_array) {
            const JSON_Value *items_val =
                json_object_get_value(schema_obj, "items");
            const JSON_Object *items_obj =
                items_val ? json_value_get_object(items_val) : NULL;
            if (items_obj && schema_object_is_object_like(items_obj)) {
              char *base = (build_inline_response_name(
                                op_id, resp_code, 1,
                                &_ast_build_inline_response_name_75),
                            _ast_build_inline_response_name_75);
              char *registered = NULL;
              if (base) {
                if (register_inline_schema((struct OpenAPI_Spec *)spec, base,
                                           items_obj, items_val,
                                           &registered) == CDD_C_SUCCESS) {
                  free(primary->schema.inline_type);
                  primary->schema.inline_type = NULL;
                  {
                    cdd_c_error_t _rc =
                        assign_schema_ref_name(&primary->schema, registered);
                    if (_rc != CDD_C_SUCCESS)
                      return _rc;
                  }
                }
                free(base);
              }
            }
          } else if (schema_object_is_object_like(schema_obj)) {
            char *base =
                (build_inline_response_name(
                     op_id, resp_code, 0, &_ast_build_inline_response_name_76),
                 _ast_build_inline_response_name_76);
            char *registered = NULL;
            if (base) {
              if (register_inline_schema((struct OpenAPI_Spec *)spec, base,
                                         schema_obj, schema_val,
                                         &registered) == CDD_C_SUCCESS) {
                if (primary->schema.inline_type) {
                  free(primary->schema.inline_type);
                  primary->schema.inline_type = NULL;
                }
                {
                  cdd_c_error_t _rc =
                      assign_schema_ref_name(&primary->schema, registered);
                  if (_rc != CDD_C_SUCCESS)
                    return _rc;
                }
              }
              free(base);
            }
          }
        }
        if (primary->item_schema_set && item_schema_obj &&
            schema_object_is_object_like(item_schema_obj)) {
          char *base =
              (build_inline_response_name(op_id, resp_code, 1,
                                          &_ast_build_inline_response_name_77),
               _ast_build_inline_response_name_77);
          char *registered = NULL;
          if (base) {
            if (register_inline_schema((struct OpenAPI_Spec *)spec, base,
                                       item_schema_obj, item_schema_val,
                                       &registered) == CDD_C_SUCCESS) {
              if (primary->item_schema.inline_type) {
                free(primary->item_schema.inline_type);
                primary->item_schema.inline_type = NULL;
              }
              {
                cdd_c_error_t _rc =
                    assign_schema_ref_name(&primary->item_schema, registered);
                if (_rc != CDD_C_SUCCESS)
                  return _rc;
              }
            }
            free(base);
          }
        }
      }
      out_resp->content_type =
          (c_cdd_strdup(primary->name, &_ast_strdup_255), _ast_strdup_255);
      if (!out_resp->content_type)
        return CDD_C_ERROR_MEMORY;
      if (primary->ref) {
        out_resp->content_ref =
            (c_cdd_strdup(primary->ref, &_ast_strdup_256), _ast_strdup_256);
        if (!out_resp->content_ref)
          return CDD_C_ERROR_MEMORY;
      }
      if (primary->schema_set) {
        {
          cdd_c_error_t _rc =
              copy_schema_ref(&out_resp->schema, &primary->schema);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      } else if (primary->item_schema_set) {
        {
          cdd_c_error_t _rc = copy_item_schema_as_array(&out_resp->schema,
                                                        &primary->item_schema);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
      if (primary->examples) {
        out_resp->examples = (struct OpenAPI_Example *)C_CDD_CALLOC(
            primary->n_examples, sizeof(struct OpenAPI_Example));
        if (!out_resp->examples)
          return CDD_C_ERROR_MEMORY;
        out_resp->n_examples = primary->n_examples;
        {
          size_t i;
          for (i = 0; i < primary->n_examples; ++i) {
            {
              cdd_c_error_t _rc = copy_example_fields(&out_resp->examples[i],
                                                      &primary->examples[i]);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
          }
        }
      } else if (primary->example_set) {
        {
          cdd_c_error_t _rc =
              copy_any_value(&out_resp->example, &primary->example);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
        out_resp->example_set = 1;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if valid response code key.
 */
cdd_c_error_t is_valid_response_code_key(const char *code) {
  size_t i;
  if (!code || !*code)
    return CDD_C_SUCCESS;
  if (strcmp(code, "default") == 0)
    return CDD_C_ERROR_UNKNOWN;
  if (strlen(code) != 3)
    return CDD_C_SUCCESS;
  if (code[1] == 'X' && code[2] == 'X')
    return (code[0] >= '1' && code[0] <= '5');
  for (i = 0; i < 3; ++i) {
    if (!isdigit((unsigned char)code[i]))
      return CDD_C_SUCCESS;
  }
  return CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief Parses responses from the given input.
 */
cdd_c_error_t parse_responses(const JSON_Object *responses,
                              struct OpenAPI_Operation *out_op,
                              const struct OpenAPI_Spec *spec,
                              const char *op_id) {
  char *_ast_strdup_257 = NULL;
  size_t i, count, valid = 0, resp_idx = 0;
  if (!responses || !out_op)
    return CDD_C_SUCCESS;

  count = json_object_get_count(responses);
  if (count == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  {
    cdd_c_error_t _rc =
        collect_extensions(responses, &out_op->responses_extensions_json);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  for (i = 0; i < count; ++i) {
    const char *code = json_object_get_name(responses, i);
    if (strncmp(code, "x-", 2) != 0)
      valid++;
  }
  if (valid == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  out_op->responses = (struct OpenAPI_Response *)C_CDD_CALLOC(
      valid, sizeof(struct OpenAPI_Response));
  if (!out_op->responses)
    return CDD_C_ERROR_MEMORY;
  out_op->n_responses = valid;

  for (i = 0; i < count; ++i) {
    const char *code = json_object_get_name(responses, i);
    const JSON_Value *val = json_object_get_value_at(responses, i);
    const JSON_Object *resp_obj = json_value_get_object(val);
    struct OpenAPI_Response *curr;

    if (strncmp(code, "x-", 2) == 0)
      continue;
    if (!is_valid_response_code_key(code))
      return CDD_C_ERROR_INVALID_ARGUMENT;
    curr = &out_op->responses[resp_idx++];
    curr->code = (c_cdd_strdup(code, &_ast_strdup_257), _ast_strdup_257);
    if (!curr->code)
      return CDD_C_ERROR_MEMORY;

    if (resp_obj) {
      cdd_c_error_t rc_resp =
          parse_response_object(resp_obj, curr, spec, 1, op_id, code);
      if (rc_resp != 0)
        return rc_resp;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses callback object from the given input.
 */
cdd_c_error_t parse_callback_object(const JSON_Object *cb_obj,
                                    struct OpenAPI_Callback *out_cb,
                                    const struct OpenAPI_Spec *spec,
                                    int resolve_refs) {
  struct OpenAPI_Callback *_ast_find_component_callback_78;
  char *_ast_strdup_258 = NULL;
  char *_ast_strdup_259 = NULL;
  char *_ast_strdup_260 = NULL;
  const char *ref;
  const char *summary;
  const char *desc;

  if (!cb_obj || !out_cb)
    return CDD_C_SUCCESS;

  ref = json_object_get_string(cb_obj, "$ref");
  if (ref) {
    out_cb->ref = (c_cdd_strdup(ref, &_ast_strdup_258), _ast_strdup_258);
    if (!out_cb->ref)
      return CDD_C_ERROR_MEMORY;
    if (resolve_refs && spec) {
      const struct OpenAPI_Callback *comp =
          (find_component_callback(spec, ref, &_ast_find_component_callback_78),
           _ast_find_component_callback_78);
      if (comp) {
        {
          cdd_c_error_t _rc = copy_callback_fields(out_cb, comp);
          if (_rc != CDD_C_SUCCESS)
            return _rc;
        }
      }
    }
    summary = json_object_get_string(cb_obj, "summary");
    if (summary) {
      if (out_cb->summary)
        free(out_cb->summary);
      out_cb->summary =
          (c_cdd_strdup(summary, &_ast_strdup_259), _ast_strdup_259);
      if (!out_cb->summary)
        return CDD_C_ERROR_MEMORY;
    }
    desc = json_object_get_string(cb_obj, "description");
    if (desc) {
      if (out_cb->description)
        free(out_cb->description);
      out_cb->description =
          (c_cdd_strdup(desc, &_ast_strdup_260), _ast_strdup_260);
      if (!out_cb->description)
        return CDD_C_ERROR_MEMORY;
    }
    return CDD_C_SUCCESS;
  }

  {
    cdd_c_error_t _rc = collect_extensions(cb_obj, &out_cb->extensions_json);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  return parse_paths_object(cb_obj, &out_cb->paths, &out_cb->n_paths, spec, 0,
                            resolve_refs);
}

/**
 * @brief Parses callbacks object from the given input.
 */
cdd_c_error_t parse_callbacks_object(const JSON_Object *callbacks,
                                     struct OpenAPI_Callback **out_callbacks,
                                     size_t *out_count,
                                     const struct OpenAPI_Spec *spec,
                                     int resolve_refs) {
  char *_ast_strdup_261 = NULL;
  size_t i, count;
  if (!out_callbacks || !out_count)
    return CDD_C_SUCCESS;

  *out_callbacks = NULL;
  *out_count = 0;

  if (!callbacks)
    return CDD_C_SUCCESS;

  count = json_object_get_count(callbacks);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out_callbacks = (struct OpenAPI_Callback *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_Callback));
  if (!*out_callbacks)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(callbacks, i);
    const JSON_Object *cb_obj =
        json_value_get_object(json_object_get_value_at(callbacks, i));
    struct OpenAPI_Callback *curr = &(*out_callbacks)[i];
    curr->name = (c_cdd_strdup(name, &_ast_strdup_261), _ast_strdup_261);
    if (!curr->name)
      return CDD_C_ERROR_MEMORY;
    if (cb_obj) {
      {
        cdd_c_error_t _rc =
            parse_callback_object(cb_obj, curr, spec, resolve_refs);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }

  return CDD_C_SUCCESS;
}
