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

#include "c_cdd/json_dom.h"
#include "c_cdd/log.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include "openapi/emit/openapi.h"
/* clang-format on */

/**
 * @brief Writes an OpenAPI parameter object.
 *
 * @param[in,out] p_obj The parameter JSON object.
 * @param[in] p The parameter structure.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t
write_parameter_object(JSON_Object *p_obj, const struct OpenAPI_Parameter *p) {
  char *in_str = NULL;
  char *style_str = NULL;
  cdd_c_error_t rc;

  if (!p_obj || !p)
    return CDD_C_SUCCESS;

  if (p->ref) {
    rc = cdd_json_object_set_string(p_obj, "$ref", p->ref);
    if (p->summary)
      rc = cdd_json_object_set_string(p_obj, "summary", p->summary);
    if (p->description)
      rc = cdd_json_object_set_string(p_obj, "description", p->description);
    return rc;
  }

  rc = param_in_to_str_openapi(p->in, &in_str);
  rc = style_to_str_openapi(p->style, &style_str);

  if (p->name)
    rc = cdd_json_object_set_string(p_obj, "name", p->name);
  if (in_str)
    rc = cdd_json_object_set_string(p_obj, "in", in_str);
  if (p->required)
    rc = cdd_json_object_set_boolean(p_obj, "required", 1);
  if (p->description)
    rc = cdd_json_object_set_string(p_obj, "description", p->description);
  if (p->deprecated_set)
    rc =
        cdd_json_object_set_boolean(p_obj, "deprecated", p->deprecated ? 1 : 0);
  if (p->allow_empty_value_set && p->in == OA_PARAM_IN_QUERY)
    rc = cdd_json_object_set_boolean(p_obj, "allowEmptyValue",
                                     p->allow_empty_value ? 1 : 0);
  if (p->collection_format)
    rc = cdd_json_object_set_string(p_obj, "collectionFormat",
                                    p->collection_format);
  if (p->example_location == OA_EXAMPLE_LOC_OBJECT) {
    rc = write_example_fields(p_obj, &p->example, p->example_set, p->examples,
                              p->n_examples);
  }

  if (!(p->content_media_types && p->n_content_media_types > 0) &&
      !(p->content_type || p->content_ref ||
        p->in == OA_PARAM_IN_QUERYSTRING)) {
    if (style_str)
      rc = cdd_json_object_set_string(p_obj, "style", style_str);
    if (p->explode_set)
      rc = cdd_json_object_set_boolean(p_obj, "explode", p->explode ? 1 : 0);
    else if (p->explode)
      rc = cdd_json_object_set_boolean(p_obj, "explode", 1);
    if (p->allow_reserved_set)
      rc = cdd_json_object_set_boolean(p_obj, "allowReserved",
                                       p->allow_reserved ? 1 : 0);
  }

  if (p->content_media_types && p->n_content_media_types > 0) {
    rc = write_media_type_map(p_obj, "content", p->content_media_types,
                              p->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (p->content_ref) {
    JSON_Value *content_val = NULL;
    JSON_Object *content_obj = NULL;
    JSON_Value *media_val = NULL;
    JSON_Object *media_obj = NULL;
    const char *content_key = p->content_type
                                  ? p->content_type
                                  : (p->in == OA_PARAM_IN_QUERYSTRING
                                         ? "application/x-www-form-urlencoded"
                                         : "application/json");

    rc = cdd_json_value_init_object(&content_val);
    rc = cdd_json_value_get_object(content_val, &content_obj);
    rc = cdd_json_value_init_object(&media_val);
    rc = cdd_json_value_get_object(media_val, &media_obj);

    rc = cdd_json_object_set_string(media_obj, "$ref", p->content_ref);
    rc = cdd_json_object_set_value_safe(content_obj, content_key, media_val);
    rc = cdd_json_object_set_value_safe(p_obj, "content", content_val);
  } else if (p->content_type || p->in == OA_PARAM_IN_QUERYSTRING) {
    JSON_Value *content_val = NULL;
    JSON_Object *content_obj = NULL;
    JSON_Value *media_val = NULL;
    JSON_Object *media_obj = NULL;
    int has_data = 0;

    rc = cdd_json_value_init_object(&content_val);
    rc = cdd_json_value_get_object(content_val, &content_obj);
    rc = cdd_json_value_init_object(&media_val);
    rc = cdd_json_value_get_object(media_val, &media_obj);

    rc = schema_ref_has_data(&p->schema, &has_data);
    if (p->item_schema_set && has_data) {
      rc = write_schema_ref(media_obj, "itemSchema", &p->schema);
    } else if (p->schema_set && has_data) {
      rc = write_schema_ref(media_obj, "schema", &p->schema);
    } else if (p->type || p->is_array) {
      rc = write_schema_from_type_fields(
          media_obj, p->item_schema_set ? "itemSchema" : "schema",
          p->type ? p->type : "string", p->is_array, p->items_type);
    }
    if (p->example_location == OA_EXAMPLE_LOC_MEDIA) {
      rc = write_example_fields(media_obj, &p->example, p->example_set,
                                p->examples, p->n_examples);
    }

    rc = cdd_json_object_set_value_safe(
        content_obj,
        p->content_type ? p->content_type : "application/x-www-form-urlencoded",
        media_val);
    rc = cdd_json_object_set_value_safe(p_obj, "content", content_val);
  } else {
    int has_data = 0;
    rc = schema_ref_has_data(&p->schema, &has_data);
    if (p->schema_set && has_data) {
      rc = write_schema_ref(p_obj, "schema", &p->schema);
    } else if (p->is_array || p->type) {
      rc = write_schema_from_type_fields(p_obj, "schema",
                                         p->type ? p->type : "string",
                                         p->is_array, p->items_type);
    }
  }

  if (p->extensions_json)
    rc = merge_schema_extras_object_openapi(p_obj, p->extensions_json);
  return rc;
}

/**
 * @brief Writes an OpenAPI header object.
 *
 * @param[in,out] h_obj The header JSON object.
 * @param[in] h The header structure.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t write_header_object(JSON_Object *h_obj,
                                               const struct OpenAPI_Header *h) {
  char *style_str = NULL;
  cdd_c_error_t rc;

  if (!h_obj || !h)
    return CDD_C_SUCCESS;

  if (h->ref) {
    rc = cdd_json_object_set_string(h_obj, "$ref", h->ref);
    if (h->description)
      rc = cdd_json_object_set_string(h_obj, "description", h->description);
    return rc;
  }

  if (h->description)
    rc = cdd_json_object_set_string(h_obj, "description", h->description);
  if (h->required)
    rc = cdd_json_object_set_boolean(h_obj, "required", 1);
  if (h->deprecated_set)
    rc =
        cdd_json_object_set_boolean(h_obj, "deprecated", h->deprecated ? 1 : 0);
  if (h->style_set) {
    rc = style_to_str_openapi(h->style, &style_str);
    if (style_str)
      rc = cdd_json_object_set_string(h_obj, "style", style_str);
  }
  if (h->collection_format)
    rc = cdd_json_object_set_string(h_obj, "collectionFormat",
                                    h->collection_format);
  if (h->explode_set)
    rc = cdd_json_object_set_boolean(h_obj, "explode", h->explode ? 1 : 0);
  if (h->example_location == OA_EXAMPLE_LOC_OBJECT) {
    rc = write_example_fields(h_obj, &h->example, h->example_set, h->examples,
                              h->n_examples);
  }

  if (h->content_media_types && h->n_content_media_types > 0) {
    rc = write_media_type_map(h_obj, "content", h->content_media_types,
                              h->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (h->content_ref) {
    JSON_Value *content_val = NULL;
    JSON_Object *content_obj = NULL;
    JSON_Value *media_val = NULL;
    JSON_Object *media_obj = NULL;

    rc = cdd_json_value_init_object(&content_val);
    rc = cdd_json_value_get_object(content_val, &content_obj);
    rc = cdd_json_value_init_object(&media_val);
    rc = cdd_json_value_get_object(media_val, &media_obj);

    rc = cdd_json_object_set_string(media_obj, "$ref", h->content_ref);
    rc = cdd_json_object_set_value_safe(
        content_obj, h->content_type ? h->content_type : "application/json",
        media_val);
    rc = cdd_json_object_set_value_safe(h_obj, "content", content_val);
  } else if (h->content_type) {
    JSON_Value *content_val = NULL;
    JSON_Object *content_obj = NULL;
    JSON_Value *media_val = NULL;
    JSON_Object *media_obj = NULL;
    int has_data = 0;

    rc = cdd_json_value_init_object(&content_val);
    rc = cdd_json_value_get_object(content_val, &content_obj);
    rc = cdd_json_value_init_object(&media_val);
    rc = cdd_json_value_get_object(media_val, &media_obj);

    rc = schema_ref_has_data(&h->schema, &has_data);
    if (h->schema_set && has_data) {
      rc = write_schema_ref(media_obj, "schema", &h->schema);
    } else if (h->type || h->is_array) {
      rc = write_schema_from_type_fields(media_obj, "schema",
                                         h->type ? h->type : "string",
                                         h->is_array, h->items_type);
    }
    if (h->example_location == OA_EXAMPLE_LOC_MEDIA) {
      rc = write_example_fields(media_obj, &h->example, h->example_set,
                                h->examples, h->n_examples);
    }

    rc =
        cdd_json_object_set_value_safe(content_obj, h->content_type, media_val);
    rc = cdd_json_object_set_value_safe(h_obj, "content", content_val);
  } else {
    int has_data = 0;
    rc = schema_ref_has_data(&h->schema, &has_data);
    if (h->schema_set && has_data) {
      rc = write_schema_ref(h_obj, "schema", &h->schema);
    } else {
      rc = write_schema_from_type_fields(h_obj, "schema",
                                         h->type ? h->type : "string",
                                         h->is_array, h->items_type);
    }
  }

  if (h->extensions_json)
    rc = merge_schema_extras_object_openapi(h_obj, h->extensions_json);
  return rc;
}

/**
 * @brief Writes an encoding object.
 *
 * @param[in,out] enc_obj The encoding JSON object.
 * @param[in] enc The encoding structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_encoding_object(
    JSON_Object *enc_obj, const struct OpenAPI_Encoding *enc) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!enc_obj || !enc)
    return CDD_C_SUCCESS;

  if (enc->content_type)
    rc = cdd_json_object_set_string(enc_obj, "contentType", enc->content_type);
  if (enc->style_set) {
    char *style_str = NULL;
    rc = style_to_str_openapi(enc->style, &style_str);
    if (style_str)
      rc = cdd_json_object_set_string(enc_obj, "style", style_str);
  }
  if (enc->explode_set)
    rc = cdd_json_object_set_boolean(enc_obj, "explode", enc->explode ? 1 : 0);
  if (enc->allow_reserved_set)
    rc = cdd_json_object_set_boolean(enc_obj, "allowReserved",
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
    JSON_Value *item_val = NULL;
    JSON_Object *item_obj = NULL;
    rc = cdd_json_value_init_object(&item_val);
    if (!item_val) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_value_get_object(item_val, &item_obj);
    if (write_encoding_object(item_obj, enc->item_encoding) != 0) {
      rc = cdd_json_value_free(item_val);
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_object_set_value_safe(enc_obj, "itemEncoding", item_val);
  }

  if (enc->extensions_json)
    rc = merge_schema_extras_object_openapi(enc_obj, enc->extensions_json);

  return rc;
}

/**
 * @brief Writes encoding map to media type object.
 *
 * @param[in,out] media_obj The media type JSON object.
 * @param[in] encoding Array of encoding structures.
 * @param[in] n_encoding Number of encodings.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
write_encoding_map(JSON_Object *media_obj,
                   const struct OpenAPI_Encoding *encoding, size_t n_encoding) {
  JSON_Value *enc_val = NULL;
  JSON_Object *enc_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!media_obj || !encoding || n_encoding == 0)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&enc_val);
  if (!enc_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(enc_val, &enc_obj);

  for (i = 0; i < n_encoding; ++i) {
    const struct OpenAPI_Encoding *enc = &encoding[i];
    JSON_Value *e_val = NULL;
    JSON_Object *e_obj = NULL;
    const char *name = enc->name ? enc->name : "encoding";

    rc = cdd_json_value_init_object(&e_val);
    if (!e_val) {
      rc = cdd_json_value_free(enc_val);
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_value_get_object(e_val, &e_obj);
    if (write_encoding_object(e_obj, enc) != 0) {
      rc = cdd_json_value_free(e_val);
      rc = cdd_json_value_free(enc_val);
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_object_set_value_safe(enc_obj, name, e_val);
  }

  rc = cdd_json_object_set_value_safe(media_obj, "encoding", enc_val);
  return rc;
}

/**
 * @brief Writes encoding array to parent JSON object.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property key name.
 * @param[in] encoding Array of encoding structures.
 * @param[in] n_encoding Number of encodings.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_encoding_array(
    JSON_Object *parent, const char *key,
    const struct OpenAPI_Encoding *encoding, size_t n_encoding) {
  JSON_Value *arr_val = NULL;
  JSON_Array *arr = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!parent || !key || !encoding || n_encoding == 0)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_array(&arr_val);
  if (!arr_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_array(arr_val, &arr);

  for (i = 0; i < n_encoding; ++i) {
    JSON_Value *e_val = NULL;
    JSON_Object *e_obj = NULL;
    rc = cdd_json_value_init_object(&e_val);
    if (!e_val) {
      rc = cdd_json_value_free(arr_val);
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_value_get_object(e_val, &e_obj);
    if (write_encoding_object(e_obj, &encoding[i]) != 0) {
      rc = cdd_json_value_free(e_val);
      rc = cdd_json_value_free(arr_val);
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_array_append_value_safe(arr, e_val);
  }

  rc = cdd_json_object_set_value_safe(parent, key, arr_val);
  return rc;
}

/**
 * @brief Writes an OpenAPI media type object.
 *
 * @param[in,out] media_obj The media type JSON object.
 * @param[in] mt The media type structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_media_type_object(
    JSON_Object *media_obj, const struct OpenAPI_MediaType *mt) {
  int has_schema = 0;
  int has_item_schema = 0;
  cdd_c_error_t rc;
  if (!media_obj || !mt)
    return CDD_C_SUCCESS;
  if (mt->ref) {
    rc = cdd_json_object_set_string(media_obj, "$ref", mt->ref);
    return rc;
  }
  rc = schema_ref_has_data(&mt->schema, &has_schema);
  if (mt->schema_set || has_schema) {
    rc = write_schema_ref(media_obj, "schema", &mt->schema);
  }
  rc = schema_ref_has_data(&mt->item_schema, &has_item_schema);
  if (mt->item_schema_set || has_item_schema) {
    rc = write_schema_ref(media_obj, "itemSchema", &mt->item_schema);
  }
  rc = write_example_fields(media_obj, &mt->example, mt->example_set,
                            mt->examples, mt->n_examples);
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
    JSON_Value *item_val = NULL;
    JSON_Object *item_obj = NULL;
    rc = cdd_json_value_init_object(&item_val);
    if (!item_val) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_value_get_object(item_val, &item_obj);
    if (write_encoding_object(item_obj, mt->item_encoding) != 0) {
      rc = cdd_json_value_free(item_val);
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_object_set_value_safe(media_obj, "itemEncoding", item_val);
  }
  if (mt->extensions_json)
    rc = merge_schema_extras_object_openapi(media_obj, mt->extensions_json);
  return rc;
}

/**
 * @brief Writes media type map to parent JSON object.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property key name.
 * @param[in] mts Array of media type structures.
 * @param[in] n_mts Number of media types.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
write_media_type_map(JSON_Object *parent, const char *key,
                     const struct OpenAPI_MediaType *mts, size_t n_mts) {
  JSON_Value *content_val = NULL;
  JSON_Object *content_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!parent || !key || !mts || n_mts == 0)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&content_val);
  if (!content_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(content_val, &content_obj);

  for (i = 0; i < n_mts; ++i) {
    const struct OpenAPI_MediaType *mt = &mts[i];
    const char *name = mt->name ? mt->name : "application/json";
    JSON_Value *mt_val = NULL;
    JSON_Object *mt_obj = NULL;
    rc = cdd_json_value_init_object(&mt_val);
    rc = cdd_json_value_get_object(mt_val, &mt_obj);

    if (write_media_type_object(mt_obj, mt) != 0) {
      rc = cdd_json_value_free(mt_val);
      rc = cdd_json_value_free(content_val);
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_object_set_value_safe(content_obj, name, mt_val);
  }

  rc = cdd_json_object_set_value_safe(parent, key, content_val);
  return rc;
}

/**
 * @brief Writes an OpenAPI link object.
 *
 * @param[in,out] l_obj The link JSON object.
 * @param[in] link The link structure.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t write_link_object(JSON_Object *l_obj,
                                             const struct OpenAPI_Link *link) {
  cdd_c_error_t rc;
  if (!l_obj || !link)
    return CDD_C_SUCCESS;

  if (link->ref) {
    rc = cdd_json_object_set_string(l_obj, "$ref", link->ref);
    if (link->summary)
      rc = cdd_json_object_set_string(l_obj, "summary", link->summary);
    if (link->description)
      rc = cdd_json_object_set_string(l_obj, "description", link->description);
    return rc;
  }

  rc = openapi_emit_reflected_strings(l_obj, &OpenAPI_Link_type_info, link);

  if (link->n_parameters > 0 && link->parameters) {
    JSON_Value *params_val = NULL;
    JSON_Object *params_obj = NULL;
    size_t i;
    rc = cdd_json_value_init_object(&params_val);
    rc = cdd_json_value_get_object(params_val, &params_obj);
    for (i = 0; i < link->n_parameters; ++i) {
      const struct OpenAPI_LinkParam *param = &link->parameters[i];
      JSON_Value *val = NULL;
      rc = any_to_json_value(&param->value, &val);
      if (val && param->name) {
        rc = cdd_json_object_set_value_safe(params_obj, param->name, val);
      } else if (val) {
        rc = cdd_json_value_free(val);
      }
    }
    rc = cdd_json_object_set_value_safe(l_obj, "parameters", params_val);
  }

  if (link->request_body_set) {
    JSON_Value *rb_val = NULL;
    rc = any_to_json_value(&link->request_body, &rb_val);
    if (rb_val)
      rc = cdd_json_object_set_value_safe(l_obj, "requestBody", rb_val);
  }

  if (link->server_set && link->server) {
    JSON_Value *srv_val = NULL;
    JSON_Object *srv_obj = NULL;
    rc = cdd_json_value_init_object(&srv_val);
    rc = cdd_json_value_get_object(srv_val, &srv_obj);
    rc = write_server_object(srv_obj, link->server);
    rc = cdd_json_object_set_value_safe(l_obj, "server", srv_val);
  }

  if (link->extensions_json)
    rc = merge_schema_extras_object_openapi(l_obj, link->extensions_json);
  return rc;
}

/**
 * @brief Writes headers map to parent JSON object.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property key name.
 * @param[in] headers Array of header structures.
 * @param[in] n_headers Number of headers.
 * @param[in] ignore_content_type Flag to ignore Content-Type headers.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_headers_map(
    JSON_Object *parent, const char *key, const struct OpenAPI_Header *headers,
    size_t n_headers, int ignore_content_type) {
  JSON_Value *headers_val = NULL;
  JSON_Object *headers_obj = NULL;
  size_t i;
  size_t written = 0;
  cdd_c_error_t rc;

  if (!parent || !key || !headers || n_headers == 0)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&headers_val);
  if (!headers_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(headers_val, &headers_obj);

  for (i = 0; i < n_headers; ++i) {
    const struct OpenAPI_Header *h = &headers[i];
    const char *name = h->name ? h->name : "header";
    JSON_Value *h_val = NULL;
    JSON_Object *h_obj = NULL;
    int is_ct = 0;

    if (ignore_content_type) {
      rc = header_name_is_content_type_openapi(name, &is_ct);
      if (is_ct)
        continue;
    }
    rc = cdd_json_value_init_object(&h_val);
    rc = cdd_json_value_get_object(h_val, &h_obj);
    rc = write_header_object(h_obj, h);
    rc = cdd_json_object_set_value_safe(headers_obj, name, h_val);
    written++;
  }

  if (written == 0) {
    rc = cdd_json_value_free(headers_val);
    return rc;
  }
  rc = cdd_json_object_set_value_safe(parent, key, headers_val);
  return rc;
}

/**
 * @brief Writes headers for an OpenAPI response.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] resp Response structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_headers(JSON_Object *parent,
                                         const struct OpenAPI_Response *resp) {
  if (!parent || !resp || resp->n_headers == 0 || !resp->headers)
    return CDD_C_SUCCESS;
  return write_headers_map(parent, "headers", resp->headers, resp->n_headers,
                           1);
}

/**
 * @brief Writes links for an OpenAPI response.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] resp Response structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_links(JSON_Object *parent,
                                       const struct OpenAPI_Response *resp) {
  JSON_Value *links_val = NULL;
  JSON_Object *links_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!parent || !resp || resp->n_links == 0 || !resp->links)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_object(&links_val);
  if (!links_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(links_val, &links_obj);

  for (i = 0; i < resp->n_links; ++i) {
    const struct OpenAPI_Link *link = &resp->links[i];
    JSON_Value *l_val = NULL;
    JSON_Object *l_obj = NULL;
    const char *name = link->name ? link->name : "link";

    rc = cdd_json_value_init_object(&l_val);
    rc = cdd_json_value_get_object(l_val, &l_obj);
    rc = write_link_object(l_obj, link);
    rc = cdd_json_object_set_value_safe(links_obj, name, l_val);
  }

  rc = cdd_json_object_set_value_safe(parent, "links", links_val);
  return rc;
}

/**
 * @brief Writes an OpenAPI response object.
 *
 * @param[in,out] r_obj The response JSON object.
 * @param[in] resp The response structure.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t
write_response_object(JSON_Object *r_obj, const struct OpenAPI_Response *resp) {
  cdd_c_error_t rc;
  if (!r_obj || !resp)
    return CDD_C_SUCCESS;

  if (resp->ref) {
    rc = cdd_json_object_set_string(r_obj, "$ref", resp->ref);
    if (resp->summary)
      rc = cdd_json_object_set_string(r_obj, "summary", resp->summary);
    if (resp->description)
      rc = cdd_json_object_set_string(r_obj, "description", resp->description);
    return rc;
  }

  if (resp->summary)
    rc = cdd_json_object_set_string(r_obj, "summary", resp->summary);
  rc = cdd_json_object_set_string(r_obj, "description",
                                  resp->description ? resp->description : "");

  if (resp->headers && resp->n_headers > 0) {
    rc = write_headers(r_obj, resp);
  }
  if (resp->links && resp->n_links > 0) {
    rc = write_links(r_obj, resp);
  }

  if (resp->content_media_types && resp->n_content_media_types > 0) {
    rc = write_media_type_map(r_obj, "content", resp->content_media_types,
                              resp->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (resp->content_ref) {
    JSON_Value *cont_val = NULL;
    JSON_Object *cont_obj = NULL;
    JSON_Value *media_val = NULL;
    JSON_Object *media_obj = NULL;

    rc = cdd_json_value_init_object(&cont_val);
    rc = cdd_json_value_get_object(cont_val, &cont_obj);
    rc = cdd_json_value_init_object(&media_val);
    rc = cdd_json_value_get_object(media_val, &media_obj);

    rc = cdd_json_object_set_string(media_obj, "$ref", resp->content_ref);
    rc = cdd_json_object_set_value_safe(
        cont_obj, resp->content_type ? resp->content_type : "application/json",
        media_val);
    rc = cdd_json_object_set_value_safe(r_obj, "content", cont_val);
  } else {
    int has_data = 0;
    rc = schema_ref_has_data(&resp->schema, &has_data);
    if (has_data || resp->content_type) {
      JSON_Value *cont_val = NULL;
      JSON_Object *cont_obj = NULL;
      JSON_Value *media_val = NULL;
      JSON_Object *media_obj = NULL;

      rc = cdd_json_value_init_object(&cont_val);
      rc = cdd_json_value_get_object(cont_val, &cont_obj);
      rc = cdd_json_value_init_object(&media_val);
      rc = cdd_json_value_get_object(media_val, &media_obj);

      if (has_data) {
        rc = write_schema_ref(media_obj, "schema", &resp->schema);
      }
      rc = write_example_fields(media_obj, &resp->example, resp->example_set,
                                resp->examples, resp->n_examples);

      rc = cdd_json_object_set_value_safe(
          cont_obj,
          resp->content_type ? resp->content_type : "application/json",
          media_val);
      rc = cdd_json_object_set_value_safe(r_obj, "content", cont_val);
    }
  }

  if (resp->extensions_json)
    rc = merge_schema_extras_object_openapi(r_obj, resp->extensions_json);
  return rc;
}

/**
 * @brief Writes parameters array to parent JSON object.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] params Array of parameters.
 * @param[in] n_params Number of parameters.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
write_parameters(JSON_Object *parent, const struct OpenAPI_Parameter *params,
                 size_t n_params) {
  JSON_Value *arr_val = NULL;
  JSON_Array *arr = NULL;
  size_t i;
  size_t written = 0;
  cdd_c_error_t rc;

  if (!parent || !params || n_params == 0)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_array(&arr_val);
  if (arr_val == NULL) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_array(arr_val, &arr);

  for (i = 0; i < n_params; ++i) {
    const struct OpenAPI_Parameter *p = &params[i];
    JSON_Value *p_val = NULL;
    JSON_Object *p_obj = NULL;
    int is_res = 0;
    rc = cdd_json_value_init_object(&p_val);
    rc = cdd_json_value_get_object(p_val, &p_obj);
    rc = param_is_reserved_header_openapi(p, &is_res);
    if (is_res) {
      rc = cdd_json_value_free(p_val);
      continue;
    }
    rc = write_parameter_object(p_obj, p);

    rc = cdd_json_array_append_value_safe(arr, p_val);
    written++;
  }

  if (written == 0) {
    rc = cdd_json_value_free(arr_val);
    return rc;
  }
  rc = cdd_json_object_set_value_safe(parent, "parameters", arr_val);
  return rc;
}

/**
 * @brief Writes an OpenAPI request body object.
 *
 * @param[in,out] rb_obj The request body JSON object.
 * @param[in] rb The request body structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_request_body_object(
    JSON_Object *rb_obj, const struct OpenAPI_RequestBody *rb) {
  JSON_Value *content_val = NULL;
  JSON_Object *content_obj = NULL;
  JSON_Value *media_val = NULL;
  JSON_Object *media_obj = NULL;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!rb_obj || !rb)
    return CDD_C_SUCCESS;

  if (rb->content_media_types && rb->n_content_media_types > 0) {
    if (write_media_type_map(rb_obj, "content", rb->content_media_types,
                             rb->n_content_media_types) != 0)
      return CDD_C_ERROR_MEMORY;
  } else if (rb->content_ref) {

    rc = cdd_json_value_init_object(&content_val);
    if (!content_val) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_value_get_object(content_val, &content_obj);

    rc = cdd_json_value_init_object(&media_val);
    if (!media_val) {
      rc = cdd_json_value_free(content_val);
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_value_get_object(media_val, &media_obj);

    rc = cdd_json_object_set_string(media_obj, "$ref", rb->content_ref);
    rc = cdd_json_object_set_value_safe(
        content_obj,
        rb->schema.content_type ? rb->schema.content_type : "application/json",
        media_val);
    rc = cdd_json_object_set_value_safe(rb_obj, "content", content_val);
  } else {
    rc = cdd_json_value_init_object(&content_val);
    if (!content_val) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_value_get_object(content_val, &content_obj);

    rc = cdd_json_value_init_object(&media_val);
    if (!media_val) {
      rc = cdd_json_value_free(content_val);
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_value_get_object(media_val, &media_obj);

    rc = write_schema_ref(media_obj, "schema", &rb->schema);
    rc = write_example_fields(media_obj, &rb->example, rb->example_set,
                              rb->examples, rb->n_examples);

    rc = cdd_json_object_set_value_safe(
        content_obj,
        rb->schema.content_type ? rb->schema.content_type : "application/json",
        media_val);
    rc = cdd_json_object_set_value_safe(rb_obj, "content", content_val);
  }

  if (rb->description) {
    rc = cdd_json_object_set_string(rb_obj, "description", rb->description);
  }
  if (rb->required_set) {
    rc = cdd_json_object_set_boolean(rb_obj, "required", rb->required ? 1 : 0);
  }

  if (rb->extensions_json)
    rc = merge_schema_extras_object_openapi(rb_obj, rb->extensions_json);

  return rc;
}

/**
 * @brief Writes request body to operation JSON object.
 *
 * @param[in,out] op_obj Operation JSON object.
 * @param[in] op Operation structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
write_request_body(JSON_Object *op_obj, const struct OpenAPI_Operation *op) {
  JSON_Value *rb_val = NULL;
  JSON_Object *rb_obj = NULL;
  cdd_c_error_t rc;

  if (!op_obj || !op)
    return CDD_C_SUCCESS;

  if (op->req_body_ref) {
    rc = cdd_json_value_init_object(&rb_val);
    if (!rb_val) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    rc = cdd_json_value_get_object(rb_val, &rb_obj);
    rc = cdd_json_object_set_string(rb_obj, "$ref", op->req_body_ref);
    if (op->req_body_description)
      rc = cdd_json_object_set_string(rb_obj, "description",
                                      op->req_body_description);
    if (op->req_body_extensions_json)
      rc = merge_schema_extras_object_openapi(rb_obj,
                                              op->req_body_extensions_json);
    rc = cdd_json_object_set_value_safe(op_obj, "requestBody", rb_val);
    return rc;
  }

  /* If body is empty and no fields, skip */
  {
    int has_data = 0;
    rc = schema_ref_has_data(&op->req_body, &has_data);
    if (!has_data && op->req_body.content_type == NULL &&
        op->n_req_body_media_types == 0) {
      return CDD_C_SUCCESS;
    }
  }

  rc = cdd_json_value_init_object(&rb_val);
  if (!rb_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  rc = cdd_json_value_get_object(rb_val, &rb_obj);

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
      rc = cdd_json_value_free(rb_val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  rc = cdd_json_object_set_value_safe(op_obj, "requestBody", rb_val);
  return rc;
}
