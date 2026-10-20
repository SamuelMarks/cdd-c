/**
 * @file openapi_parameters.c
 * @brief Parameter object and parameter array parsing.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Internal helper to parse an OpenAPI Parameter object.
 *
 * @param[in] p_obj The source parameter JSON object.
 * @param[out] out_param Output OpenAPI_Parameter structure.
 * @param[in,out] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
parse_parameter_object_inner(const JSON_Object *p_obj,
                             struct OpenAPI_Parameter *out_param,
                             struct OpenAPI_Spec *spec, int resolve_refs) {
  char *base = NULL;
  char *registered = NULL;
  const char *ref;
  const char *name;
  const char *in;
  const char *desc;
  int req;
  int deprecated_present;
  int deprecated_val;
  int allow_reserved_present;
  int allow_reserved_val;
  int allow_empty_present;
  int allow_empty_val;
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
  const char *style_str;
  int explode_present;
  int explode_val;
  cdd_c_error_t rc;

  if (!p_obj || !out_param)
    return CDD_C_SUCCESS;

  ref = json_object_get_string(p_obj, "$ref");
  if (ref) {
    c_cdd_strdup(ref, &out_param->ref);
    if (!out_param->ref)
      return CDD_C_ERROR_MEMORY;
    if (resolve_refs && spec) {
      struct OpenAPI_Parameter *comp = NULL;
      find_component_parameter(spec, ref, &comp);
      if (comp) {
        rc = copy_parameter_fields(out_param, comp);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    desc = json_object_get_string(p_obj, "description");
    if (desc) {
      C_CDD_FREE(out_param->description);
      out_param->description = NULL;
      c_cdd_strdup(desc, &out_param->description);
      if (!out_param->description)
        return CDD_C_ERROR_MEMORY;
    }
    return CDD_C_SUCCESS;
  }

  name = json_object_get_string(p_obj, "name");
  in = json_object_get_string(p_obj, "in");
  if (!name || !*name || !in)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  c_cdd_strdup(name, &out_param->name);
  if (!out_param->name)
    return CDD_C_ERROR_MEMORY;
  parse_param_in(in, &out_param->in);
  if (out_param->in == OA_PARAM_IN_UNKNOWN)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  desc = json_object_get_string(p_obj, "description");
  req = json_object_get_boolean(p_obj, "required");
  out_param->required = (req == 1);
  deprecated_present = json_object_has_value(p_obj, "deprecated");
  deprecated_val = json_object_get_boolean(p_obj, "deprecated");
  allow_reserved_present = json_object_has_value(p_obj, "allowReserved");
  allow_reserved_val = json_object_get_boolean(p_obj, "allowReserved");
  allow_empty_present = json_object_has_value(p_obj, "allowEmptyValue");
  allow_empty_val = json_object_get_boolean(p_obj, "allowEmptyValue");
  schema_val = json_object_get_value(p_obj, "schema");
  schema = schema_val ? json_value_get_object(schema_val) : NULL;
  content = json_object_get_object(p_obj, "content");
  if (content) {
    if (json_object_get_count(content) != 1)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = parse_content_object(content, &out_param->content_media_types,
                              &out_param->n_content_media_types, spec,
                              resolve_refs);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  effective_schema = schema;
  effective_schema_val = schema_val;
  if (!effective_schema && spec && spec->swagger_version &&
      out_param->in != OA_PARAM_IN_BODY &&
      json_object_has_value(p_obj, "type")) {
    effective_schema = p_obj;
    effective_schema_val = json_object_get_wrapping_value(p_obj);
  }
  media_obj = NULL;
  media_schema_val = NULL;
  media_type = NULL;
  media_ref = NULL;
  resolved_schema = NULL;
  type = NULL;
  parsed_schema_set = 0;
  style_str = json_object_get_string(p_obj, "style");
  explode_present = json_object_has_value(p_obj, "explode");
  explode_val = json_object_get_boolean(p_obj, "explode");

  {
    const int has_schema = (schema_val != NULL);
    int has_content = (content != NULL);
    if (has_schema && has_content)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (out_param->in == OA_PARAM_IN_QUERYSTRING && !has_content)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (allow_empty_present && out_param->in != OA_PARAM_IN_QUERY)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (content) {
    if (out_param->in == OA_PARAM_IN_QUERYSTRING) {
      media_obj =
          json_object_get_object(content, "application/x-www-form-urlencoded");
      if (media_obj) {
        media_type = "application/x-www-form-urlencoded";
      }
    }
    if (!media_obj) {
      media_type = json_object_get_name(content, 0);
      media_obj = json_object_get_object(content, media_type);
    }
    c_cdd_strdup(media_type, &out_param->content_type);
    if (!out_param->content_type)
      return CDD_C_ERROR_MEMORY;
    if (media_obj) {
      media_ref = json_object_get_string(media_obj, "$ref");
      if (media_ref) {
        c_cdd_strdup(media_ref, &out_param->content_ref);
        if (!out_param->content_ref)
          return CDD_C_ERROR_MEMORY;
        if (resolve_refs && spec) {
          struct OpenAPI_MediaType *mt = NULL;
          find_component_media_type(spec, media_ref, &mt);
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

  if (effective_schema)
    type = json_object_get_string(effective_schema, "type");

  if (desc) {
    c_cdd_strdup(desc, &out_param->description);
    if (!out_param->description)
      return CDD_C_ERROR_MEMORY;
  }

  if (deprecated_present) {
    out_param->deprecated_set = 1;
    out_param->deprecated = (deprecated_val == 1);
  }
  if (allow_reserved_present) {
    out_param->allow_reserved_set = 1;
    out_param->allow_reserved = (allow_reserved_val == 1);
  }
  if (allow_empty_present) {
    out_param->allow_empty_value_set = 1;
    out_param->allow_empty_value = (allow_empty_val == 1);
  }
  if (resolved_schema) {
    if (out_param->in != OA_PARAM_IN_QUERYSTRING) {
      rc = apply_schema_ref_to_param(out_param, resolved_schema);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    rc = copy_schema_ref(&out_param->schema, resolved_schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
    out_param->schema_set = 1;
  } else if (effective_schema_val &&
             json_value_get_type(effective_schema_val) == JSONBoolean) {
    out_param->schema.schema_is_boolean = 1;
    out_param->schema.schema_boolean_value =
        json_value_get_boolean(effective_schema_val);
    out_param->schema_set = 1;
  } else if (effective_schema) {
    rc = parse_schema_ref(effective_schema, &parsed_schema, spec);
    if (rc != CDD_C_SUCCESS)
      return rc;
    parsed_schema_set = 1;
    if (out_param->in != OA_PARAM_IN_QUERYSTRING) {
      if (apply_schema_ref_to_param(out_param, &parsed_schema) != 0) {
        free_schema_ref_content(&parsed_schema);
        return CDD_C_ERROR_MEMORY;
      }
    }
    if (copy_schema_ref(&out_param->schema, &parsed_schema) != 0) {
      free_schema_ref_content(&parsed_schema);
      return CDD_C_ERROR_MEMORY;
    }
    out_param->schema_set = 1;
  }

  if (spec && out_param->in == OA_PARAM_IN_QUERYSTRING &&
      media_type_is_json(out_param->content_type) && effective_schema &&
      schema_object_is_object_like(effective_schema) &&
      !out_param->schema.ref_name) {
    base = NULL;
    registered = NULL;
    rc = build_inline_param_name(out_param->name, &base);
    if (rc != CDD_C_SUCCESS) {
      free_schema_ref_content(&parsed_schema);
      return rc;
    }
    rc = register_inline_schema(spec, base, effective_schema,
                                effective_schema_val, &registered);
    if (rc == CDD_C_SUCCESS) {
      if (out_param->schema.inline_type) {
        C_CDD_FREE(out_param->schema.inline_type);
        out_param->schema.inline_type = NULL;
      }
      if (assign_schema_ref_name(&out_param->schema, registered) !=
          CDD_C_SUCCESS) {
        C_CDD_FREE(base);
        free_schema_ref_content(&parsed_schema);
        return CDD_C_ERROR_MEMORY;
      }
    }
    C_CDD_FREE(base);
  }

  if (!out_param->type) {
    c_cdd_strdup(type ? type : "string", &out_param->type);
    if (!out_param->type) {
      if (parsed_schema_set)
        free_schema_ref_content(&parsed_schema);
      return CDD_C_ERROR_MEMORY;
    }
  }

  if (style_str) {
    parse_param_style(style_str, &out_param->style);
    if (out_param->style == OA_STYLE_UNKNOWN) {
      if (parsed_schema_set)
        free_schema_ref_content(&parsed_schema);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  } else {
    if (out_param->in == OA_PARAM_IN_QUERY)
      out_param->style = OA_STYLE_FORM;
    else if (out_param->in == OA_PARAM_IN_PATH)
      out_param->style = OA_STYLE_SIMPLE;
    else if (out_param->in == OA_PARAM_IN_COOKIE)
      out_param->style = OA_STYLE_FORM;
    else
      out_param->style = OA_STYLE_SIMPLE;
  }

  if (explode_present) {
    out_param->explode_set = 1;
    out_param->explode = explode_val;
  } else {
    if (out_param->style == OA_STYLE_FORM)
      out_param->explode = 1;
    else
      out_param->explode = 0;
  }

  rc = validate_parameter_style(out_param, content != NULL);
  if (rc != CDD_C_SUCCESS) {
    if (parsed_schema_set)
      free_schema_ref_content(&parsed_schema);
    return rc;
  }

  if (object_has_example_and_examples(p_obj)) {
    if (parsed_schema_set)
      free_schema_ref_content(&parsed_schema);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = parse_examples_object(json_object_get_object(p_obj, "examples"),
                             &out_param->examples, &out_param->n_examples, spec,
                             resolve_refs);
  if (rc != CDD_C_SUCCESS) {
    if (parsed_schema_set)
      free_schema_ref_content(&parsed_schema);
    return rc;
  }
  if (out_param->n_examples == 0) {
    rc = parse_any_field(p_obj, "example", &out_param->example,
                         &out_param->example_set);
    if (rc != CDD_C_SUCCESS) {
      if (parsed_schema_set)
        free_schema_ref_content(&parsed_schema);
      return rc;
    }
  }
  if (out_param->example_set || out_param->n_examples > 0) {
    out_param->example_location = OA_EXAMPLE_LOC_OBJECT;
  } else if (media_obj && !media_ref) {
    rc = parse_media_examples(media_obj, &out_param->example,
                              &out_param->example_set, &out_param->examples,
                              &out_param->n_examples, spec, resolve_refs);
    if (rc != CDD_C_SUCCESS) {
      if (parsed_schema_set)
        free_schema_ref_content(&parsed_schema);
      return rc;
    }
    if (out_param->example_set || out_param->n_examples > 0) {
      out_param->example_location = OA_EXAMPLE_LOC_MEDIA;
    }
  }

  rc = collect_extensions(p_obj, &out_param->extensions_json);
  if (rc != CDD_C_SUCCESS) {
    if (parsed_schema_set)
      free_schema_ref_content(&parsed_schema);
    return rc;
  }

  if (parsed_schema_set)
    free_schema_ref_content(&parsed_schema);

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses parameter object from the given input.
 */
cdd_c_error_t parse_parameter_object(const JSON_Object *p_obj,
                                     struct OpenAPI_Parameter *out_param,
                                     struct OpenAPI_Spec *spec,
                                     int resolve_refs) {
  cdd_c_error_t rc =
      parse_parameter_object_inner(p_obj, out_param, spec, resolve_refs);
  if (rc != CDD_C_SUCCESS)
    free_parameter(out_param);
  return rc;
}

/**
 * @brief Checks whether two OpenAPI parameters have the same name and location.
 */
cdd_c_error_t param_key_equals(const struct OpenAPI_Parameter *a,
                               const struct OpenAPI_Parameter *b) {
  if (!a || !b || !a->name || !b->name)
    return CDD_C_SUCCESS;
  return (a->in == b->in) && (strcmp(a->name, b->name) == 0);
}

/**
 * @brief Parses parameters array from the given input.
 */
cdd_c_error_t parse_parameters_array(const JSON_Array *arr,
                                     struct OpenAPI_Parameter **out_params,
                                     size_t *out_count,
                                     struct OpenAPI_Spec *spec) {
  size_t i, count, valid = 0;
  if (!out_params || !out_count)
    return CDD_C_SUCCESS;

  *out_params = NULL;
  *out_count = 0;

  if (!arr)
    return CDD_C_SUCCESS;

  count = json_array_get_count(arr);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out_params = (struct OpenAPI_Parameter *)C_CDD_CALLOC(
      count, sizeof(struct OpenAPI_Parameter));
  if (!*out_params)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < count; ++i) {
    const JSON_Object *p_obj = json_array_get_object(arr, i);
    if (p_obj) {
      struct OpenAPI_Parameter tmp = {0};
      cdd_c_error_t rc = parse_parameter_object(p_obj, &tmp, spec, 1);
      if (rc != CDD_C_SUCCESS) {
        size_t j;
        free_parameter(&tmp);
        for (j = 0; j < valid; ++j)
          free_parameter(&(*out_params)[j]);
        C_CDD_FREE(*out_params);
        *out_params = NULL;
        *out_count = 0;
        return rc;
      }
      if (header_param_is_reserved(&tmp)) {
        free_parameter(&tmp);
        continue;
      }
      if (tmp.name) {
        size_t k;
        for (k = 0; k < valid; ++k) {
          if (param_key_equals(&tmp, &(*out_params)[k])) {
            size_t j;
            free_parameter(&tmp);
            for (j = 0; j < valid; ++j)
              free_parameter(&(*out_params)[j]);
            C_CDD_FREE(*out_params);
            *out_params = NULL;
            *out_count = 0;
            return CDD_C_ERROR_INVALID_ARGUMENT;
          }
        }
      }
      (*out_params)[valid++] = tmp;
    }
  }
  if (valid == 0) {
    C_CDD_FREE(*out_params);
    *out_params = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }
  if (valid < count) {
    struct OpenAPI_Parameter *tmp = (struct OpenAPI_Parameter *)C_CDD_REALLOC(
        *out_params, valid * sizeof(struct OpenAPI_Parameter));
    if (tmp)
      *out_params = tmp;
  }
  *out_count = valid;
  return CDD_C_SUCCESS;
}
