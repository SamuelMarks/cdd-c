/**
 * @file openapi_paths.c
 * @brief Path item and operation parsing and routing.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Parses paths object from the given input.
 */
cdd_c_error_t parse_paths_object(const JSON_Object *paths_obj,
                                 struct OpenAPI_Path **out_paths,
                                 size_t *out_count,
                                 const struct OpenAPI_Spec *spec,
                                 int require_leading_slash, int resolve_refs) {
  struct OpenAPI_Path *_ast_find_component_path_item_80;
  char *_ast_strdup_286 = NULL;
  char *_ast_strdup_287 = NULL;
  char *_ast_strdup_288 = NULL;
  char *_ast_strdup_289 = NULL;
  char *_ast_strdup_290 = NULL;
  char *_ast_strdup_291 = NULL;
  size_t i, n_paths;
  size_t raw_count;
  size_t out_idx;

  if (!paths_obj || !out_paths || !out_count)
    return CDD_C_SUCCESS;

  raw_count = json_object_get_count(paths_obj);
  n_paths = 0;
  for (i = 0; i < raw_count; ++i) {
    const char *name = json_object_get_name(paths_obj, i);
    if (name && strncmp(name, "x-", 2) == 0)
      continue;
    n_paths++;
  }
  if (n_paths == 0) {
    *out_paths = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }

  *out_paths =
      (struct OpenAPI_Path *)calloc(n_paths, sizeof(struct OpenAPI_Path));
  if (!*out_paths)
    return CDD_C_ERROR_MEMORY;
  *out_count = n_paths;

  out_idx = 0;
  for (i = 0; i < raw_count; ++i) {
    const char *route = json_object_get_name(paths_obj, i);
    const JSON_Value *p_val = json_object_get_value_at(paths_obj, i);
    const JSON_Object *p_obj = json_value_get_object(p_val);
    struct OpenAPI_Path *curr_path;
    size_t n_ops_in_obj = p_obj ? json_object_get_count(p_obj) : 0;
    size_t k, valid_ops = 0;

    if (route && strncmp(route, "x-", 2) == 0)
      continue;
    if (out_idx >= n_paths)
      break;
    curr_path = &(*out_paths)[out_idx++];

    if (require_leading_slash && (!route || route[0] != '/'))
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (route) {
      curr_path->route =
          (c_cdd_strdup(route, &_ast_strdup_286), _ast_strdup_286);
      if (!curr_path->route)
        return CDD_C_ERROR_MEMORY;
    }

    if (p_obj) {
      const char *path_ref = json_object_get_string(p_obj, "$ref");
      const char *path_summary = json_object_get_string(p_obj, "summary");
      const char *path_description =
          json_object_get_string(p_obj, "description");
      const JSON_Array *path_params =
          json_object_get_array(p_obj, "parameters");

      if (path_ref) {
        curr_path->ref =
            (c_cdd_strdup(path_ref, &_ast_strdup_287), _ast_strdup_287);
        if (!curr_path->ref)
          return CDD_C_ERROR_MEMORY;
        if (resolve_refs && spec) {
          const struct OpenAPI_Path *comp =
              (find_component_path_item(spec, path_ref,
                                        &_ast_find_component_path_item_80),
               _ast_find_component_path_item_80);
          if (comp) {
            {
              cdd_c_error_t _rc = copy_path_fields(curr_path, comp);
              if (_rc != CDD_C_SUCCESS)
                return _rc;
            }
          }
        }
        if (path_summary) {
          if (curr_path->summary)
            free(curr_path->summary);
          curr_path->summary =
              (c_cdd_strdup(path_summary, &_ast_strdup_288), _ast_strdup_288);
          if (!curr_path->summary)
            return CDD_C_ERROR_MEMORY;
        }
        if (path_description) {
          if (curr_path->description)
            free(curr_path->description);
          curr_path->description =
              (c_cdd_strdup(path_description, &_ast_strdup_289),
               _ast_strdup_289);
          if (!curr_path->description)
            return CDD_C_ERROR_MEMORY;
        }
        /* Path Item refs do not process sibling fields */
        continue;
      }
      if (path_summary) {
        curr_path->summary =
            (c_cdd_strdup(path_summary, &_ast_strdup_290), _ast_strdup_290);
        if (!curr_path->summary)
          return CDD_C_ERROR_MEMORY;
      }
      if (path_description) {
        curr_path->description =
            (c_cdd_strdup(path_description, &_ast_strdup_291), _ast_strdup_291);
        if (!curr_path->description)
          return CDD_C_ERROR_MEMORY;
      }
      {
        cdd_c_error_t _rc =
            collect_extensions(p_obj, &curr_path->extensions_json);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      {
        cdd_c_error_t rc =
            parse_parameters_array(path_params, &curr_path->parameters,
                                   &curr_path->n_parameters, spec);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      {
        cdd_c_error_t _rc = parse_servers_array(
            p_obj, "servers", &curr_path->servers, &curr_path->n_servers);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
      {
        cdd_c_error_t _rc = parse_additional_operations(p_obj, curr_path, spec);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }

    if (n_ops_in_obj > 0) {
      curr_path->operations = (struct OpenAPI_Operation *)calloc(
          n_ops_in_obj, sizeof(struct OpenAPI_Operation));

      curr_path->n_operations = n_ops_in_obj;
      if (!curr_path->operations) {
        return CDD_C_ERROR_MEMORY;
      }

      if (p_obj) {
        for (k = 0; k < n_ops_in_obj; ++k) {
          cdd_c_error_t err;
          const char *verb = json_object_get_name(p_obj, k);
          const JSON_Object *op_obj =
              json_value_get_object(json_object_get_value_at(p_obj, k));
          if (!verb)
            continue;
          if (strcmp(verb, "parameters") == 0 || strcmp(verb, "servers") == 0 ||
              strcmp(verb, "summary") == 0 ||
              strcmp(verb, "description") == 0 || strcmp(verb, "$ref") == 0 ||
              strcmp(verb, "additionalOperations") == 0 ||
              strncmp(verb, "x-", 2) == 0) {
            continue;
          }
          err = parse_operation(verb, op_obj, &curr_path->operations[valid_ops],
                                spec, 0, curr_path->route);
          if (err != 0) {
            return err;
          }
          if (curr_path->operations[valid_ops].verb != OA_VERB_UNKNOWN) {
            valid_ops++;
          }
        }
      }
      curr_path->n_operations = valid_ops;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the name in list operation.
 */
cdd_c_error_t name_in_list(const char *name, char **names, size_t count) {
  size_t i;
  if (!name || !names)
    return CDD_C_SUCCESS;
  for (i = 0; i < count; ++i) {
    if (names[i] && strcmp(names[i], name) == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Collects path template names.
 */
cdd_c_error_t collect_path_template_names(const char *route, char ***out_names,
                                          size_t *out_count) {
  size_t i;
  size_t cap = 0;
  size_t count = 0;
  char **names = NULL;

  if (!out_names || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_names = NULL;
  *out_count = 0;

  if (!route)
    return CDD_C_SUCCESS;

  for (i = 0; route[i]; ++i) {
    if (route[i] == '{') {
      size_t start = i + 1;
      size_t end = start;
      while (route[end] && route[end] != '}')
        ++end;
      if (!route[end]) {
        free_name_list(names, count);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      if (end == start) {
        free_name_list(names, count);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      {
        size_t len = end - start;
        char *name = (char *)(size_t)malloc(len + 1);
        if (!name) {
          free_name_list(names, count);
          return CDD_C_ERROR_MEMORY;
        }
        memcpy(name, route + start, len);
        name[len] = '\0';
        if (name_in_list(name, names, count)) {
          free(name);
          free_name_list(names, count);
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
        if (count == cap) {
          size_t new_cap = cap ? cap * 2 : 4;
          char **tmp = (char **)realloc(names, new_cap * sizeof(char *));
          if (!tmp) {
            free(name);
            free_name_list(names, count);
            return CDD_C_ERROR_MEMORY;
          }
          names = tmp;
          cap = new_cap;
        }
        names[count++] = name;
      }
      i = end;
    } else if (route[i] == '}') {
      free_name_list(names, count);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }

  *out_names = names;
  *out_count = count;
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the path param.
 */
cdd_c_error_t find_path_param(const struct OpenAPI_Parameter *params, size_t n,
                              const char *name,
                              struct OpenAPI_Parameter **_out_val) {
  size_t i;
  if (!params || !name) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < n; ++i) {
    if (params[i].in != OA_PARAM_IN_PATH)
      continue;
    if (!params[i].name)
      continue;
    if (strcmp(params[i].name, name) == 0) {
      *_out_val = (struct OpenAPI_Parameter *)(&params[i]);
      return CDD_C_SUCCESS;
    }
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the validate path params list operation.
 */
cdd_c_error_t validate_path_params_list(const struct OpenAPI_Parameter *params,
                                        size_t n_params, char **template_names,
                                        size_t n_template_names) {
  size_t i;
  if (!params)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_params; ++i) {
    const struct OpenAPI_Parameter *p = &params[i];
    if (p->in != OA_PARAM_IN_PATH)
      continue;
    if (!p->name || !name_in_list(p->name, template_names, n_template_names))
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (!p->required)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the validate path template for operation operation.
 */
cdd_c_error_t validate_path_template_for_operation(
    const struct OpenAPI_Path *path, const struct OpenAPI_Operation *op,
    char **template_names, size_t n_template_names) {
  struct OpenAPI_Parameter *_ast_find_path_param_81;
  struct OpenAPI_Parameter *_ast_find_path_param_82;
  size_t i;
  if (!path || !template_names || n_template_names == 0)
    return CDD_C_SUCCESS;

  if (validate_path_params_list(op ? op->parameters : NULL,
                                op ? op->n_parameters : 0, template_names,
                                n_template_names) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < n_template_names; ++i) {
    const struct OpenAPI_Parameter *p =
        op ? (find_path_param(op->parameters, op->n_parameters,
                              template_names[i], &_ast_find_path_param_81),
              _ast_find_path_param_81)
           : NULL;
    if (!p) {
      p = (find_path_param(path->parameters, path->n_parameters,
                           template_names[i], &_ast_find_path_param_82),
           _ast_find_path_param_82);
    }
    if (!p)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the validate path templates operation.
 */
cdd_c_error_t validate_path_templates(const struct OpenAPI_Path *paths,
                                      size_t n_paths) {
  size_t i;
  for (i = 0; i < n_paths; ++i) {
    const struct OpenAPI_Path *path = &paths[i];
    char **template_names = NULL;
    size_t n_template_names = 0;
    size_t op_idx;
    cdd_c_error_t rc;
    int has_ops;

    if (!path->route)
      continue;
    if (path->route[0] != '/')
      continue;
    if (path->ref)
      continue;

    rc = collect_path_template_names(path->route, &template_names,
                                     &n_template_names);
    if (rc != CDD_C_SUCCESS)
      return rc;

    rc = validate_path_params_list(path->parameters, path->n_parameters,
                                   template_names, n_template_names);
    if (rc != CDD_C_SUCCESS) {
      free_name_list(template_names, n_template_names);
      return rc;
    }

    has_ops = (path->n_operations + path->n_additional_operations) > 0;
    if (!has_ops) {
      free_name_list(template_names, n_template_names);
      continue;
    }

    for (op_idx = 0; op_idx < path->n_operations; ++op_idx) {
      if (validate_path_template_for_operation(path, &path->operations[op_idx],
                                               template_names,
                                               n_template_names) != 0) {
        free_name_list(template_names, n_template_names);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }

    for (op_idx = 0; op_idx < path->n_additional_operations; ++op_idx) {
      if (validate_path_template_for_operation(
              path, &path->additional_operations[op_idx], template_names,
              n_template_names) != 0) {
        free_name_list(template_names, n_template_names);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }

    free_name_list(template_names, n_template_names);
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the normalize path template route operation.
 */
cdd_c_error_t normalize_path_template_route(const char *route,
                                            char **_out_val) {
  size_t i = 0;
  size_t len = 0;
  char *out;
  size_t pos = 0;
  if (!route) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  while (route[i]) {
    if (route[i] == '{') {
      size_t j = i + 1;
      while (route[j] && route[j] != '}')
        ++j;
      if (!route[j]) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      len += 2;
      i = j + 1;
    } else {
      ++len;
      ++i;
    }
  }
  out = (char *)(size_t)calloc(len + 1, sizeof(char));
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  i = 0;
  while (route[i]) {
    if (route[i] == '{') {
      size_t j = i + 1;
      while (route[j] && route[j] != '}')
        ++j;
      if (!route[j]) {
        free(out);
        {
          *_out_val = NULL;
          return CDD_C_SUCCESS;
        }
      }
      out[pos++] = '{';
      out[pos++] = '}';
      i = j + 1;
    } else {
      out[pos++] = route[i++];
    }
  }
  out[pos] = '\0';
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the validate path template collisions operation.
 */
cdd_c_error_t
validate_path_template_collisions(const struct OpenAPI_Path *paths,
                                  size_t n_paths) {
  char *_ast_normalize_path_template_route_83 = NULL;
  char *_ast_normalize_path_template_route_84 = NULL;
  size_t i;
  if (!paths)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_paths; ++i) {
    const char *route_i = paths[i].route;
    char *norm_i;
    size_t j;
    if (!route_i || route_i[0] != '/' || !strchr(route_i, '{'))
      continue;
    norm_i = (normalize_path_template_route(
                  route_i, &_ast_normalize_path_template_route_83),
              _ast_normalize_path_template_route_83);
    if (!norm_i)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    for (j = i + 1; j < n_paths; ++j) {
      const char *route_j = paths[j].route;
      char *norm_j;
      if (!route_j || route_j[0] != '/' || !strchr(route_j, '{'))
        continue;
      norm_j = (normalize_path_template_route(
                    route_j, &_ast_normalize_path_template_route_84),
                _ast_normalize_path_template_route_84);
      if (!norm_j) {
        free(norm_i);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      if (strcmp(norm_i, norm_j) == 0 && strcmp(route_i, route_j) != 0) {
        free(norm_j);
        free(norm_i);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      free(norm_j);
    }
    free(norm_i);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses operation from the given input.
 */
cdd_c_error_t parse_operation(const char *verb_str, const JSON_Object *op_obj,
                              struct OpenAPI_Operation *out_op,
                              const struct OpenAPI_Spec *spec,
                              int is_additional, const char *route_hint) {
  enum OpenAPI_Verb _ast_parse_verb_79;
  char *_ast_strdup_262 = NULL;
  char *_ast_strdup_263 = NULL;
  char *_ast_strdup_264 = NULL;
  char *_ast_strdup_265 = NULL;
  char *_ast_strdup_266 = NULL;
  char *_ast_strdup_267 = NULL;
  char *_ast_strdup_268 = NULL;
  char *_ast_strdup_269 = NULL;
  const char *op_id;
  const JSON_Array *params;
  const JSON_Array *tags;
  const JSON_Object *req_body, *responses;
  const char *summary;
  const char *description;
  const JSON_Object *ext_docs;
  int deprecated_present;
  int deprecated_val;
  cdd_c_error_t rc_sec;

  if (!verb_str || !op_obj || !out_op)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  (void)route_hint;

  out_op->verb =
      (parse_verb(verb_str, &_ast_parse_verb_79), _ast_parse_verb_79);
  out_op->is_additional = is_additional;
  if (verb_str) {
    out_op->method =
        (c_cdd_strdup(verb_str, &_ast_strdup_262), _ast_strdup_262);
    if (!out_op->method)
      return CDD_C_ERROR_MEMORY;
  }
  if (out_op->verb == OA_VERB_UNKNOWN && !is_additional)
    return CDD_C_SUCCESS;

  op_id = json_object_get_string(op_obj, "operationId");
  out_op->operation_id =
      (c_cdd_strdup(op_id ? op_id : NULL, &_ast_strdup_263), _ast_strdup_263);
  summary = json_object_get_string(op_obj, "summary");
  if (summary) {
    out_op->summary =
        (c_cdd_strdup(summary, &_ast_strdup_264), _ast_strdup_264);
    if (!out_op->summary)
      return CDD_C_ERROR_MEMORY;
  }
  description = json_object_get_string(op_obj, "description");
  if (description) {
    out_op->description =
        (c_cdd_strdup(description, &_ast_strdup_265), _ast_strdup_265);
    if (!out_op->description)
      return CDD_C_ERROR_MEMORY;
  }
  ext_docs = json_object_get_object(op_obj, "externalDocs");
  if (ext_docs) {
    cdd_c_error_t rc = parse_external_docs(ext_docs, &out_op->external_docs);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  deprecated_present = json_object_has_value(op_obj, "deprecated");
  deprecated_val = json_object_get_boolean(op_obj, "deprecated");
  if (deprecated_present)
    out_op->deprecated = (deprecated_val == 1);
  rc_sec = parse_security_field(op_obj, "security", &out_op->security,
                                &out_op->n_security, &out_op->security_set);
  if (rc_sec != 0)
    return rc_sec;
  {
    cdd_c_error_t _rc = collect_extensions(op_obj, &out_op->extensions_json);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  /* 1. Parameters */
  params = json_object_get_array(op_obj, "parameters");
  {
    cdd_c_error_t rc = parse_parameters_array(params, &out_op->parameters,
                                              &out_op->n_parameters, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  /* 2. Request Body */
  req_body = json_object_get_object(op_obj, "requestBody");
  if (req_body) {
    cdd_c_error_t rc_req;
    struct OpenAPI_RequestBody rb;
    memset(&rb, 0, sizeof(rb));
    rc_req =
        parse_request_body_object(req_body, &rb, spec, 1, out_op->operation_id);
    if (rc_req != 0) {
      free_request_body(&rb);
      return rc_req;
    }
    if (rb.ref) {
      out_op->req_body_ref =
          (c_cdd_strdup(rb.ref, &_ast_strdup_266), _ast_strdup_266);
      if (!out_op->req_body_ref) {
        free_request_body(&rb);
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (rb.description) {
      out_op->req_body_description =
          (c_cdd_strdup(rb.description, &_ast_strdup_267), _ast_strdup_267);
      if (!out_op->req_body_description) {
        free_request_body(&rb);
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (rb.required_set) {
      out_op->req_body_required_set = 1;
      out_op->req_body_required = rb.required;
    }
    if (rb.extensions_json) {
      out_op->req_body_extensions_json =
          (c_cdd_strdup(rb.extensions_json, &_ast_strdup_268), _ast_strdup_268);
      if (!out_op->req_body_extensions_json) {
        free_request_body(&rb);
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (copy_schema_ref(&out_op->req_body, &rb.schema) != 0) {
      free_request_body(&rb);
      return CDD_C_ERROR_MEMORY;
    }
    if (rb.content_media_types && rb.n_content_media_types > 0) {
      if (copy_media_type_array(
              &out_op->req_body_media_types, &out_op->n_req_body_media_types,
              rb.content_media_types, rb.n_content_media_types) != 0) {
        free_request_body(&rb);
        return CDD_C_ERROR_MEMORY;
      }
    }
    free_request_body(&rb);
  }

  /* 3. Responses */
  responses = json_object_get_object(op_obj, "responses");
  {
    cdd_c_error_t rc =
        parse_responses(responses, out_op, spec, out_op->operation_id);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  /* 4. Callbacks */
  {
    const JSON_Object *callbacks_obj =
        json_object_get_object(op_obj, "callbacks");
    {
      cdd_c_error_t _rc = parse_callbacks_object(
          callbacks_obj, &out_op->callbacks, &out_op->n_callbacks, spec, 1);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  /* 5. Tags */
  tags = json_object_get_array(op_obj, "tags");
  if (tags) {
    size_t t_count = json_array_get_count(tags);
    out_op->n_tags = t_count;
    if (t_count > 0) {
      size_t k;
      out_op->tags = (char **)calloc(t_count, sizeof(char *));
      if (!out_op->tags)
        return CDD_C_ERROR_MEMORY;
      for (k = 0; k < t_count; ++k) {
        const char *t_val = json_array_get_string(tags, k);
        out_op->tags[k] = (c_cdd_strdup(t_val ? t_val : "", &_ast_strdup_269),
                           _ast_strdup_269);
        if (!out_op->tags[k])
          return CDD_C_ERROR_MEMORY;
      }
    }
  }

  {
    cdd_c_error_t _rc = parse_servers_array(op_obj, "servers", &out_op->servers,
                                            &out_op->n_servers);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses additional operations from the given input.
 */
cdd_c_error_t parse_additional_operations(const JSON_Object *path_obj,
                                          struct OpenAPI_Path *path,
                                          const struct OpenAPI_Spec *spec) {
  const JSON_Object *add_ops;
  size_t count, i;

  if (!path_obj || !path)
    return CDD_C_SUCCESS;

  add_ops = json_object_get_object(path_obj, "additionalOperations");
  if (!add_ops)
    return CDD_C_SUCCESS;

  count = json_object_get_count(add_ops);
  if (count == 0)
    return CDD_C_SUCCESS;

  path->additional_operations = (struct OpenAPI_Operation *)calloc(
      count, sizeof(struct OpenAPI_Operation));
  if (!path->additional_operations)
    return CDD_C_ERROR_MEMORY;
  path->n_additional_operations = count;

  for (i = 0; i < count; ++i) {
    const char *method = json_object_get_name(add_ops, i);
    const JSON_Object *op_obj =
        json_value_get_object(json_object_get_value_at(add_ops, i));
    struct OpenAPI_Operation *curr = &path->additional_operations[i];
    if (is_fixed_operation_method(method))
      return CDD_C_ERROR_INVALID_ARGUMENT;
    {
      cdd_c_error_t _rc =
          parse_operation(method, op_obj, curr, spec, 1, path->route);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  return CDD_C_SUCCESS;
}
