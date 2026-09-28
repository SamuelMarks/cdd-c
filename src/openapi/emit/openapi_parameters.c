/**
 * @file openapi_parameters.c
 * @brief OpenAPI emitter parameter and request body generation.
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
 * @brief Generates C code for write parameter object.
 */
C_CDD_EXPORT void write_parameter_object(JSON_Object *p_obj,
                                         const struct OpenAPI_Parameter *p) {
  char *_ast_param_in_to_str_14 = NULL;
  char *_ast_style_to_str_15 = NULL;
  const char *in_str;
  const char *style_str;
  cdd_c_error_t rc;

  if (!p_obj || !p)
    return;

  if (p->ref) {
    json_object_set_string(p_obj, "$ref", p->ref);
    if (p->summary)
      json_object_set_string(p_obj, "summary", p->summary);
    if (p->description)
      json_object_set_string(p_obj, "description", p->description);
    return;
  }

  in_str = (param_in_to_str_openapi(p->in, &_ast_param_in_to_str_14),
            _ast_param_in_to_str_14);
  style_str = (style_to_str_openapi(p->style, &_ast_style_to_str_15),
               _ast_style_to_str_15);

  if (p->name)
    json_object_set_string(p_obj, "name", p->name);
  if (in_str)
    json_object_set_string(p_obj, "in", in_str);
  if (p->required)
    json_object_set_boolean(p_obj, "required", 1);
  if (p->description)
    json_object_set_string(p_obj, "description", p->description);
  if (p->deprecated_set)
    json_object_set_boolean(p_obj, "deprecated", p->deprecated ? 1 : 0);
  if (p->allow_empty_value_set && p->in == OA_PARAM_IN_QUERY)
    json_object_set_boolean(p_obj, "allowEmptyValue",
                            p->allow_empty_value ? 1 : 0);
  if (p->example_location == OA_EXAMPLE_LOC_OBJECT) {
    write_example_fields(p_obj, &p->example, p->example_set, p->examples,
                         p->n_examples);
  }

  if (!(p->content_media_types && p->n_content_media_types > 0) &&
      !(p->content_type || p->content_ref ||
        p->in == OA_PARAM_IN_QUERYSTRING)) {
    if (style_str)
      json_object_set_string(p_obj, "style", style_str);
    if (p->explode_set)
      json_object_set_boolean(p_obj, "explode", p->explode ? 1 : 0);
    else if (p->explode)
      json_object_set_boolean(p_obj, "explode", 1);
    if (p->allow_reserved_set)
      json_object_set_boolean(p_obj, "allowReserved",
                              p->allow_reserved ? 1 : 0);
  }

  if (p->content_media_types && p->n_content_media_types > 0) {
    rc = write_media_type_map(p_obj, "content", p->content_media_types,
                              p->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return;
  } else if (p->content_ref) {
    JSON_Value *content_val = json_value_init_object();
    JSON_Object *content_obj = json_value_get_object(content_val);
    JSON_Value *media_val = json_value_init_object();
    JSON_Object *media_obj = json_value_get_object(media_val);
    const char *content_key = p->content_type
                                  ? p->content_type
                                  : (p->in == OA_PARAM_IN_QUERYSTRING
                                         ? "application/x-www-form-urlencoded"
                                         : "application/json");

    json_object_set_string(media_obj, "$ref", p->content_ref);
    json_object_set_value(content_obj, content_key, media_val);
    json_object_set_value(p_obj, "content", content_val);
  } else if (p->content_type || p->in == OA_PARAM_IN_QUERYSTRING) {
    JSON_Value *content_val = json_value_init_object();
    JSON_Object *content_obj = json_value_get_object(content_val);
    JSON_Value *media_val = json_value_init_object();
    JSON_Object *media_obj = json_value_get_object(media_val);

    if (p->item_schema_set && schema_ref_has_data(&p->schema)) {
      write_schema_ref(media_obj, "itemSchema", &p->schema);
    } else if (p->schema_set && schema_ref_has_data(&p->schema)) {
      write_schema_ref(media_obj, "schema", &p->schema);
    } else if (p->type || p->is_array) {
      write_schema_from_type_fields(
          media_obj, p->item_schema_set ? "itemSchema" : "schema",
          p->type ? p->type : "string", p->is_array, p->items_type);
    }
    if (p->example_location == OA_EXAMPLE_LOC_MEDIA) {
      write_example_fields(media_obj, &p->example, p->example_set, p->examples,
                           p->n_examples);
    }

    json_object_set_value(content_obj,
                          p->content_type ? p->content_type
                                          : "application/x-www-form-urlencoded",
                          media_val);
    json_object_set_value(p_obj, "content", content_val);
  } else if (p->schema_set && schema_ref_has_data(&p->schema)) {
    write_schema_ref(p_obj, "schema", &p->schema);
  } else if (p->is_array || p->type) {
    write_schema_from_type_fields(p_obj, "schema", p->type ? p->type : "string",
                                  p->is_array, p->items_type);
  }

  if (p->extensions_json)
    merge_schema_extras_object_openapi(p_obj, p->extensions_json);
}

/**
 * @brief Generates C code for write header object.
 */
C_CDD_EXPORT void write_header_object(JSON_Object *h_obj,
                                      const struct OpenAPI_Header *h) {
  char *_ast_style_to_str_16 = NULL;
  const char *style_str;
  cdd_c_error_t rc;

  if (!h_obj || !h)
    return;

  if (h->ref) {
    json_object_set_string(h_obj, "$ref", h->ref);
    if (h->description)
      json_object_set_string(h_obj, "description", h->description);
    return;
  }

  if (h->description)
    json_object_set_string(h_obj, "description", h->description);
  if (h->required)
    json_object_set_boolean(h_obj, "required", 1);
  if (h->deprecated_set)
    json_object_set_boolean(h_obj, "deprecated", h->deprecated ? 1 : 0);
  if (h->style_set) {
    style_str = (style_to_str_openapi(h->style, &_ast_style_to_str_16),
                 _ast_style_to_str_16);
    if (style_str)
      json_object_set_string(h_obj, "style", style_str);
  }
  if (h->explode_set)
    json_object_set_boolean(h_obj, "explode", h->explode ? 1 : 0);
  if (h->example_location == OA_EXAMPLE_LOC_OBJECT) {
    write_example_fields(h_obj, &h->example, h->example_set, h->examples,
                         h->n_examples);
  }

  if (h->content_media_types && h->n_content_media_types > 0) {
    rc = write_media_type_map(h_obj, "content", h->content_media_types,
                              h->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return;
  } else if (h->content_ref) {
    JSON_Value *content_val = json_value_init_object();
    JSON_Object *content_obj = json_value_get_object(content_val);
    JSON_Value *media_val = json_value_init_object();
    JSON_Object *media_obj = json_value_get_object(media_val);

    json_object_set_string(media_obj, "$ref", h->content_ref);
    json_object_set_value(
        content_obj, h->content_type ? h->content_type : "application/json",
        media_val);
    json_object_set_value(h_obj, "content", content_val);
  } else if (h->content_type) {
    JSON_Value *content_val = json_value_init_object();
    JSON_Object *content_obj = json_value_get_object(content_val);
    JSON_Value *media_val = json_value_init_object();
    JSON_Object *media_obj = json_value_get_object(media_val);

    if (h->schema_set && schema_ref_has_data(&h->schema)) {
      write_schema_ref(media_obj, "schema", &h->schema);
    } else if (h->type || h->is_array) {
      write_schema_from_type_fields(media_obj, "schema",
                                    h->type ? h->type : "string", h->is_array,
                                    h->items_type);
    }
    if (h->example_location == OA_EXAMPLE_LOC_MEDIA) {
      write_example_fields(media_obj, &h->example, h->example_set, h->examples,
                           h->n_examples);
    }

    json_object_set_value(content_obj, h->content_type, media_val);
    json_object_set_value(h_obj, "content", content_val);
  } else if (h->schema_set && schema_ref_has_data(&h->schema)) {
    write_schema_ref(h_obj, "schema", &h->schema);
  } else {
    write_schema_from_type_fields(h_obj, "schema", h->type ? h->type : "string",
                                  h->is_array, h->items_type);
  }

  if (h->extensions_json)
    merge_schema_extras_object_openapi(h_obj, h->extensions_json);
}

/**
 * @brief Generates C code for write encoding object.
 */
C_CDD_EXPORT cdd_c_error_t write_encoding_object(
    JSON_Object *enc_obj, const struct OpenAPI_Encoding *enc) {
  char *_ast_style_to_str_17 = NULL;
  cdd_c_error_t rc;
  if (!enc_obj || !enc)
    return CDD_C_SUCCESS;

  if (enc->content_type)
    json_object_set_string(enc_obj, "contentType", enc->content_type);
  if (enc->style_set) {
    const char *style_str =
        (style_to_str_openapi(enc->style, &_ast_style_to_str_17),
         _ast_style_to_str_17);
    if (style_str)
      json_object_set_string(enc_obj, "style", style_str);
  }
  if (enc->explode_set)
    json_object_set_boolean(enc_obj, "explode", enc->explode ? 1 : 0);
  if (enc->allow_reserved_set)
    json_object_set_boolean(enc_obj, "allowReserved",
                            enc->allow_reserved ? 1 : 0);
  if (enc->headers && enc->n_headers > 0) {
    rc = write_headers_map(enc_obj, "headers", enc->headers, enc->n_headers, 1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (enc->encoding && enc->n_encoding > 0) {
    if (write_encoding_map(enc_obj, enc->encoding, enc->n_encoding) != 0)
      return CDD_C_ERROR_MEMORY;
  }
  if (enc->prefix_encoding && enc->n_prefix_encoding > 0) {
    if (write_encoding_array(enc_obj, "prefixEncoding", enc->prefix_encoding,
                             enc->n_prefix_encoding) != 0)
      return CDD_C_ERROR_MEMORY;
  }
  if (enc->item_encoding && enc->item_encoding_set) {
    JSON_Value *item_val = json_value_init_object();
    JSON_Object *item_obj = json_value_get_object(item_val);
    if (!item_val) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    if (write_encoding_object(item_obj, enc->item_encoding) != 0) {
      json_value_free(item_val);
      return CDD_C_ERROR_MEMORY;
    }
    json_object_set_value(enc_obj, "itemEncoding", item_val);
  }

  if (enc->extensions_json)
    merge_schema_extras_object_openapi(enc_obj, enc->extensions_json);

  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write encoding map.
 */
C_CDD_EXPORT cdd_c_error_t
write_encoding_map(JSON_Object *media_obj,
                   const struct OpenAPI_Encoding *encoding, size_t n_encoding) {
  JSON_Value *enc_val;
  JSON_Object *enc_obj;
  size_t i;

  if (!media_obj || !encoding || n_encoding == 0)
    return CDD_C_SUCCESS;

  enc_val = json_value_init_object();
  if (!enc_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  enc_obj = json_value_get_object(enc_val);

  for (i = 0; i < n_encoding; ++i) {
    const struct OpenAPI_Encoding *enc = &encoding[i];
    JSON_Value *e_val = json_value_init_object();
    JSON_Object *e_obj = json_value_get_object(e_val);
    const char *name = enc->name ? enc->name : "encoding";

    if (!e_val) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    if (write_encoding_object(e_obj, enc) != 0) {
      json_value_free(e_val);
      return CDD_C_ERROR_MEMORY;
    }
    json_object_set_value(enc_obj, name, e_val);
  }

  json_object_set_value(media_obj, "encoding", enc_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write encoding array.
 */
C_CDD_EXPORT cdd_c_error_t write_encoding_array(
    JSON_Object *parent, const char *key,
    const struct OpenAPI_Encoding *encoding, size_t n_encoding) {
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;

  if (!parent || !key || !encoding || n_encoding == 0)
    return CDD_C_SUCCESS;

  arr_val = json_value_init_array();
  if (!arr_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  arr = json_value_get_array(arr_val);

  for (i = 0; i < n_encoding; ++i) {
    JSON_Value *e_val = json_value_init_object();
    JSON_Object *e_obj = json_value_get_object(e_val);
    if (!e_val) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    if (write_encoding_object(e_obj, &encoding[i]) != 0) {
      json_value_free(e_val);
      return CDD_C_ERROR_MEMORY;
    }
    json_array_append_value(arr, e_val);
  }

  json_object_set_value(parent, key, arr_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write media type object.
 */
C_CDD_EXPORT cdd_c_error_t write_media_type_object(
    JSON_Object *media_obj, const struct OpenAPI_MediaType *mt) {
  if (!media_obj || !mt)
    return CDD_C_SUCCESS;
  if (mt->ref) {
    json_object_set_string(media_obj, "$ref", mt->ref);
    return CDD_C_SUCCESS;
  }
  if (mt->schema_set || schema_ref_has_data(&mt->schema)) {
    write_schema_ref(media_obj, "schema", &mt->schema);
  }
  if (mt->item_schema_set || schema_ref_has_data(&mt->item_schema)) {
    write_schema_ref(media_obj, "itemSchema", &mt->item_schema);
  }
  write_example_fields(media_obj, &mt->example, mt->example_set, mt->examples,
                       mt->n_examples);
  if (mt->encoding && mt->n_encoding > 0) {
    if (write_encoding_map(media_obj, mt->encoding, mt->n_encoding) != 0)
      return CDD_C_ERROR_MEMORY;
  }
  if (mt->prefix_encoding && mt->n_prefix_encoding > 0) {
    if (write_encoding_array(media_obj, "prefixEncoding", mt->prefix_encoding,
                             mt->n_prefix_encoding) != 0)
      return CDD_C_ERROR_MEMORY;
  }
  if (mt->item_encoding && mt->item_encoding_set) {
    JSON_Value *item_val = json_value_init_object();
    JSON_Object *item_obj = json_value_get_object(item_val);
    if (!item_val) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    if (write_encoding_object(item_obj, mt->item_encoding) != 0) {
      json_value_free(item_val);
      return CDD_C_ERROR_MEMORY;
    }
    json_object_set_value(media_obj, "itemEncoding", item_val);
  }
  if (mt->extensions_json)
    merge_schema_extras_object_openapi(media_obj, mt->extensions_json);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write media type map.
 */
C_CDD_EXPORT cdd_c_error_t
write_media_type_map(JSON_Object *parent, const char *key,
                     const struct OpenAPI_MediaType *mts, size_t n_mts) {
  JSON_Value *content_val;
  JSON_Object *content_obj;
  size_t i;

  if (!parent || !key || !mts || n_mts == 0)
    return CDD_C_SUCCESS;

  content_val = json_value_init_object();
  if (!content_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  content_obj = json_value_get_object(content_val);

  for (i = 0; i < n_mts; ++i) {
    const struct OpenAPI_MediaType *mt = &mts[i];
    const char *name = mt->name ? mt->name : "application/json";
    JSON_Value *mt_val = json_value_init_object();
    JSON_Object *mt_obj = json_value_get_object(mt_val);

    if (write_media_type_object(mt_obj, mt) != 0) {
      json_value_free(mt_val);
      json_value_free(content_val);
      return CDD_C_ERROR_MEMORY;
    }
    json_object_set_value(content_obj, name, mt_val);
  }

  json_object_set_value(parent, key, content_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write link object.
 */
C_CDD_EXPORT void write_link_object(JSON_Object *l_obj,
                                    const struct OpenAPI_Link *link) {
  JSON_Value *_ast_any_to_json_value_18;
  JSON_Value *_ast_any_to_json_value_19;
  if (!l_obj || !link)
    return;

  if (link->ref) {
    json_object_set_string(l_obj, "$ref", link->ref);
    if (link->summary)
      json_object_set_string(l_obj, "summary", link->summary);
    if (link->description)
      json_object_set_string(l_obj, "description", link->description);
    return;
  }

  if (link->operation_ref)
    json_object_set_string(l_obj, "operationRef", link->operation_ref);
  if (link->operation_id)
    json_object_set_string(l_obj, "operationId", link->operation_id);
  if (link->description)
    json_object_set_string(l_obj, "description", link->description);

  if (link->n_parameters > 0 && link->parameters) {
    JSON_Value *params_val = json_value_init_object();
    JSON_Object *params_obj = json_value_get_object(params_val);
    size_t i;
    for (i = 0; i < link->n_parameters; ++i) {
      const struct OpenAPI_LinkParam *param = &link->parameters[i];
      JSON_Value *val =
          (any_to_json_value(&param->value, &_ast_any_to_json_value_18),
           _ast_any_to_json_value_18);
      if (val && param->name) {
        json_object_set_value(params_obj, param->name, val);
      } else if (val) {
        json_value_free(val);
      }
    }
    json_object_set_value(l_obj, "parameters", params_val);
  }

  if (link->request_body_set) {
    JSON_Value *rb_val =
        (any_to_json_value(&link->request_body, &_ast_any_to_json_value_19),
         _ast_any_to_json_value_19);
    if (rb_val)
      json_object_set_value(l_obj, "requestBody", rb_val);
  }

  if (link->server_set && link->server) {
    JSON_Value *srv_val = json_value_init_object();
    JSON_Object *srv_obj = json_value_get_object(srv_val);
    write_server_object(srv_obj, link->server);
    json_object_set_value(l_obj, "server", srv_val);
  }

  if (link->extensions_json)
    merge_schema_extras_object_openapi(l_obj, link->extensions_json);
}

/**
 * @brief Generates C code for write headers map.
 */
C_CDD_EXPORT cdd_c_error_t write_headers_map(
    JSON_Object *parent, const char *key, const struct OpenAPI_Header *headers,
    size_t n_headers, int ignore_content_type) {
  JSON_Value *headers_val;
  JSON_Object *headers_obj;
  size_t i;
  size_t written = 0;

  if (!parent || !key || !headers || n_headers == 0)
    return CDD_C_SUCCESS;

  headers_val = json_value_init_object();
  if (!headers_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  headers_obj = json_value_get_object(headers_val);

  for (i = 0; i < n_headers; ++i) {
    const struct OpenAPI_Header *h = &headers[i];
    const char *name = h->name ? h->name : "header";
    JSON_Value *h_val;
    JSON_Object *h_obj;

    if (ignore_content_type && header_name_is_content_type_openapi(name))
      continue;
    h_val = json_value_init_object();
    h_obj = json_value_get_object(h_val);
    write_header_object(h_obj, h);
    json_object_set_value(headers_obj, name, h_val);
    written++;
  }

  if (written == 0) {
    json_value_free(headers_val);
    return CDD_C_SUCCESS;
  }
  json_object_set_value(parent, key, headers_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write headers.
 */
C_CDD_EXPORT cdd_c_error_t write_headers(JSON_Object *parent,
                                         const struct OpenAPI_Response *resp) {
  if (!parent || !resp || resp->n_headers == 0 || !resp->headers)
    return CDD_C_SUCCESS;
  return write_headers_map(parent, "headers", resp->headers, resp->n_headers,
                           1);
}

/**
 * @brief Generates C code for write links.
 */
C_CDD_EXPORT cdd_c_error_t write_links(JSON_Object *parent,
                                       const struct OpenAPI_Response *resp) {
  JSON_Value *links_val;
  JSON_Object *links_obj;
  size_t i;

  if (!parent || !resp || resp->n_links == 0 || !resp->links)
    return CDD_C_SUCCESS;

  links_val = json_value_init_object();
  if (!links_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  links_obj = json_value_get_object(links_val);

  for (i = 0; i < resp->n_links; ++i) {
    const struct OpenAPI_Link *link = &resp->links[i];
    JSON_Value *l_val = json_value_init_object();
    JSON_Object *l_obj = json_value_get_object(l_val);
    const char *name = link->name ? link->name : "link";

    write_link_object(l_obj, link);
    json_object_set_value(links_obj, name, l_val);
  }

  json_object_set_value(parent, "links", links_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write response object.
 */
C_CDD_EXPORT void write_response_object(JSON_Object *r_obj,
                                        const struct OpenAPI_Response *resp) {
  cdd_c_error_t rc;
  if (!r_obj || !resp)
    return;

  if (resp->ref) {
    json_object_set_string(r_obj, "$ref", resp->ref);
    if (resp->summary)
      json_object_set_string(r_obj, "summary", resp->summary);
    if (resp->description)
      json_object_set_string(r_obj, "description", resp->description);
    return;
  }

  if (resp->summary)
    json_object_set_string(r_obj, "summary", resp->summary);
  json_object_set_string(r_obj, "description",
                         resp->description ? resp->description : "");

  if (resp->headers && resp->n_headers > 0) {
    write_headers(r_obj, resp);
  }
  if (resp->links && resp->n_links > 0) {
    write_links(r_obj, resp);
  }

  if (resp->content_media_types && resp->n_content_media_types > 0) {
    rc = write_media_type_map(r_obj, "content", resp->content_media_types,
                              resp->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return;
  } else if (resp->content_ref) {
    JSON_Value *cont_val = json_value_init_object();
    JSON_Object *cont_obj = json_value_get_object(cont_val);
    JSON_Value *media_val = json_value_init_object();
    JSON_Object *media_obj = json_value_get_object(media_val);

    json_object_set_string(media_obj, "$ref", resp->content_ref);
    json_object_set_value(
        cont_obj, resp->content_type ? resp->content_type : "application/json",
        media_val);
    json_object_set_value(r_obj, "content", cont_val);
  } else if (schema_ref_has_data(&resp->schema) || resp->content_type) {
    JSON_Value *cont_val = json_value_init_object();
    JSON_Object *cont_obj = json_value_get_object(cont_val);
    JSON_Value *media_val = json_value_init_object();
    JSON_Object *media_obj = json_value_get_object(media_val);

    if (schema_ref_has_data(&resp->schema)) {
      write_schema_ref(media_obj, "schema", &resp->schema);
    }
    write_example_fields(media_obj, &resp->example, resp->example_set,
                         resp->examples, resp->n_examples);

    json_object_set_value(
        cont_obj, resp->content_type ? resp->content_type : "application/json",
        media_val);
    json_object_set_value(r_obj, "content", cont_val);
  }

  if (resp->extensions_json)
    merge_schema_extras_object_openapi(r_obj, resp->extensions_json);
}

/**
 * @brief Generates C code for write parameters.
 */
C_CDD_EXPORT cdd_c_error_t
write_parameters(JSON_Object *parent, const struct OpenAPI_Parameter *params,
                 size_t n_params) {
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;
  size_t written = 0;

  if (!parent || !params || n_params == 0)
    return CDD_C_SUCCESS;

  arr_val = json_value_init_array();
  if (arr_val == NULL) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  arr = json_value_get_array(arr_val);

  for (i = 0; i < n_params; ++i) {
    const struct OpenAPI_Parameter *p = &params[i];
    JSON_Value *p_val = json_value_init_object();
    JSON_Object *p_obj = json_value_get_object(p_val);
    if (param_is_reserved_header_openapi(p)) {
      json_value_free(p_val);
      continue;
    }
    write_parameter_object(p_obj, p);

    json_array_append_value(arr, p_val);
    written++;
  }

  if (written == 0) {
    json_value_free(arr_val);
    return CDD_C_SUCCESS;
  }
  json_object_set_value(parent, "parameters", arr_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write request body object.
 */
C_CDD_EXPORT cdd_c_error_t write_request_body_object(
    JSON_Object *rb_obj, const struct OpenAPI_RequestBody *rb) {
  JSON_Value *content_val;
  JSON_Object *content_obj;
  JSON_Value *media_val;
  JSON_Object *media_obj;

  if (!rb_obj || !rb)
    return CDD_C_SUCCESS;

  if (rb->content_media_types && rb->n_content_media_types > 0) {
    if (write_media_type_map(rb_obj, "content", rb->content_media_types,
                             rb->n_content_media_types) != 0)
      return CDD_C_ERROR_MEMORY;
  } else if (rb->content_ref) {

    content_val = json_value_init_object();
    if (!content_val) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    content_obj = json_value_get_object(content_val);

    media_val = json_value_init_object();
    if (!media_val) {
      json_value_free(content_val);
      return CDD_C_ERROR_MEMORY;
    }
    media_obj = json_value_get_object(media_val);

    json_object_set_string(media_obj, "$ref", rb->content_ref);
    json_object_set_value(content_obj,
                          rb->schema.content_type ? rb->schema.content_type
                                                  : "application/json",
                          media_val);
    json_object_set_value(rb_obj, "content", content_val);
  } else {
    content_val = json_value_init_object();
    if (!content_val) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    content_obj = json_value_get_object(content_val);

    media_val = json_value_init_object();
    if (!media_val) {
      json_value_free(content_val);
      return CDD_C_ERROR_MEMORY;
    }
    media_obj = json_value_get_object(media_val);

    write_schema_ref(media_obj, "schema", &rb->schema);
    write_example_fields(media_obj, &rb->example, rb->example_set, rb->examples,
                         rb->n_examples);

    json_object_set_value(content_obj,
                          rb->schema.content_type ? rb->schema.content_type
                                                  : "application/json",
                          media_val);
    json_object_set_value(rb_obj, "content", content_val);
  }

  if (rb->description) {
    json_object_set_string(rb_obj, "description", rb->description);
  }
  if (rb->required_set) {
    json_object_set_boolean(rb_obj, "required", rb->required ? 1 : 0);
  }

  if (rb->extensions_json)
    merge_schema_extras_object_openapi(rb_obj, rb->extensions_json);

  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write request body.
 */
C_CDD_EXPORT cdd_c_error_t
write_request_body(JSON_Object *op_obj, const struct OpenAPI_Operation *op) {
  JSON_Value *rb_val;
  JSON_Object *rb_obj;

  if (!op_obj || !op)
    return CDD_C_SUCCESS;

  if (op->req_body_ref) {
    rb_val = json_value_init_object();
    if (!rb_val) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    rb_obj = json_value_get_object(rb_val);
    json_object_set_string(rb_obj, "$ref", op->req_body_ref);
    if (op->req_body_description)
      json_object_set_string(rb_obj, "description", op->req_body_description);
    if (op->req_body_extensions_json)
      merge_schema_extras_object_openapi(rb_obj, op->req_body_extensions_json);
    json_object_set_value(op_obj, "requestBody", rb_val);
    return CDD_C_SUCCESS;
  }

  /* If body is empty and no fields, skip */
  if (!schema_ref_has_data(&op->req_body) &&
      op->req_body.content_type == NULL && op->n_req_body_media_types == 0) {
    return CDD_C_SUCCESS;
  }

  rb_val = json_value_init_object();
  if (!rb_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rb_obj = json_value_get_object(rb_val);

  {
    struct OpenAPI_RequestBody rb = {0};
    rb.description = op->req_body_description;
    rb.required = op->req_body_required;
    rb.required_set = op->req_body_required_set;
    rb.schema = op->req_body;
    rb.content_media_types = op->req_body_media_types;
    rb.n_content_media_types = op->n_req_body_media_types;
    rb.extensions_json = op->req_body_extensions_json;
    if (write_request_body_object(rb_obj, &rb) != 0) {
      json_value_free(rb_val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  json_object_set_value(op_obj, "requestBody", rb_val);
  return CDD_C_SUCCESS;
}
