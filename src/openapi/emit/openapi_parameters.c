/**
 * @file openapi_parameters.c
 * @brief OpenAPI emitter parameters, headers, links, and responses generation.
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
 * @brief Writes a single-entry `content` map with `"$ref": content_ref` onto
 * `target_obj`.
 *
 * @param[in,out] target_obj Target parameter, header, response, or requestBody
 * object.
 * @param[in] content_key Media type key string.
 * @param[in] content_ref `$ref` string for the media type entry.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t write_single_content_ref(JSON_Object *target_obj,
                                              const char *content_key,
                                              const char *content_ref) {
  JSON_Value *content_val = NULL;
  JSON_Object *content_obj = NULL;
  JSON_Value *media_val = NULL;
  JSON_Object *media_obj = NULL;
  cdd_c_error_t rc;

  rc = oa_emit_new_object(&content_val, &content_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_emit_new_object(&media_val, &media_obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(content_val));
  }
  rc = cdd_json_object_set_string(media_obj, "$ref", content_ref);
  if (rc != CDD_C_SUCCESS) {
    cdd_c_error_t f1 = cdd_json_value_free(media_val);
    cdd_c_error_t f2 = cdd_json_value_free(content_val);
    return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
  }
  rc = cdd_json_object_set_value_safe(content_obj, content_key, media_val);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(content_val));
  }
  return cdd_json_object_set_value_safe(target_obj, "content", content_val);
}

/**
 * @brief Writes inline parameter `content` entry when `content_type` or
 * `querystring` location is used.
 *
 * @param[in,out] p_obj Target parameter JSON object.
 * @param[in] p Parameter definition.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t
write_param_inline_content(JSON_Object *p_obj,
                           const struct OpenAPI_Parameter *p) {
  JSON_Value *content_val = NULL;
  JSON_Object *content_obj = NULL;
  JSON_Value *media_val = NULL;
  JSON_Object *media_obj = NULL;
  int has_data = 0;
  cdd_c_error_t rc;

  rc = oa_emit_new_object(&content_val, &content_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_emit_new_object(&media_val, &media_obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(content_val));
  }

  rc = schema_ref_has_data(&p->schema, &has_data);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_param_media;
  if (p->item_schema_set && has_data) {
    rc = write_schema_ref(media_obj, "itemSchema", &p->schema);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_param_media;
  } else if (p->schema_set && has_data) {
    rc = write_schema_ref(media_obj, "schema", &p->schema);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_param_media;
  } else if (p->type || p->is_array) {
    rc = write_schema_from_type_fields(
        media_obj, p->item_schema_set ? "itemSchema" : "schema",
        p->type ? p->type : "string", p->is_array, p->items_type);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_param_media;
  }
  if (p->example_location == OA_EXAMPLE_LOC_MEDIA) {
    rc = write_example_fields(media_obj, &p->example, p->example_set,
                              p->examples, p->n_examples);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_param_media;
  }

  rc = cdd_json_object_set_value_safe(
      content_obj,
      p->content_type ? p->content_type : "application/x-www-form-urlencoded",
      media_val);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(content_val));
  }
  return cdd_json_object_set_value_safe(p_obj, "content", content_val);

cleanup_param_media: {
  cdd_c_error_t f1 = cdd_json_value_free(media_val);
  cdd_c_error_t f2 = cdd_json_value_free(content_val);
  return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
}
}

/**
 * @brief Writes inline header `content` entry when `content_type` is used.
 *
 * @param[in,out] h_obj Target header JSON object.
 * @param[in] h Header definition.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t
write_header_inline_content(JSON_Object *h_obj,
                            const struct OpenAPI_Header *h) {
  JSON_Value *content_val = NULL;
  JSON_Object *content_obj = NULL;
  JSON_Value *media_val = NULL;
  JSON_Object *media_obj = NULL;
  int has_data = 0;
  cdd_c_error_t rc;

  rc = oa_emit_new_object(&content_val, &content_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = oa_emit_new_object(&media_val, &media_obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(content_val));
  }

  rc = schema_ref_has_data(&h->schema, &has_data);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_header_media;
  if (h->schema_set && has_data) {
    rc = write_schema_ref(media_obj, "schema", &h->schema);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_header_media;
  } else if (h->type || h->is_array) {
    rc = write_schema_from_type_fields(media_obj, "schema",
                                       h->type ? h->type : "string",
                                       h->is_array, h->items_type);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_header_media;
  }
  if (h->example_location == OA_EXAMPLE_LOC_MEDIA) {
    rc = write_example_fields(media_obj, &h->example, h->example_set,
                              h->examples, h->n_examples);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_header_media;
  }

  rc = cdd_json_object_set_value_safe(content_obj, h->content_type, media_val);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(content_val));
  }
  return cdd_json_object_set_value_safe(h_obj, "content", content_val);

cleanup_header_media: {
  cdd_c_error_t f1 = cdd_json_value_free(media_val);
  cdd_c_error_t f2 = cdd_json_value_free(content_val);
  return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
}
}

/**
 * @brief Writes an OpenAPI parameter object.
 *
 * @param[in,out] p_obj The parameter JSON object.
 * @param[in] p The parameter structure.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t
write_parameter_object(JSON_Object *p_obj, const struct OpenAPI_Parameter *p) {
  const char *in_str = NULL;
  const char *style_str = NULL;
  cdd_c_error_t rc;

  if (!p_obj || !p)
    return CDD_C_SUCCESS;

  if (p->ref) {
    rc = cdd_json_object_set_string(p_obj, "$ref", p->ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (p->summary) {
      rc = cdd_json_object_set_string(p_obj, "summary", p->summary);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (p->description) {
      return cdd_json_object_set_string(p_obj, "description", p->description);
    }
    return CDD_C_SUCCESS;
  }

  rc = param_in_to_str_openapi(p->in, &in_str);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = style_to_str_openapi(p->style, &style_str);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (p->name) {
    rc = cdd_json_object_set_string(p_obj, "name", p->name);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (in_str) {
    rc = cdd_json_object_set_string(p_obj, "in", in_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (p->required) {
    rc = cdd_json_object_set_boolean(p_obj, "required", 1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (p->description) {
    rc = cdd_json_object_set_string(p_obj, "description", p->description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (p->deprecated_set) {
    rc =
        cdd_json_object_set_boolean(p_obj, "deprecated", p->deprecated ? 1 : 0);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (p->allow_empty_value_set && p->in == OA_PARAM_IN_QUERY) {
    rc = cdd_json_object_set_boolean(p_obj, "allowEmptyValue",
                                     p->allow_empty_value ? 1 : 0);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (p->collection_format) {
    rc = cdd_json_object_set_string(p_obj, "collectionFormat",
                                    p->collection_format);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (p->example_location == OA_EXAMPLE_LOC_OBJECT) {
    rc = write_example_fields(p_obj, &p->example, p->example_set, p->examples,
                              p->n_examples);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (!(p->content_media_types && p->n_content_media_types > 0) &&
      !(p->content_type || p->content_ref ||
        p->in == OA_PARAM_IN_QUERYSTRING)) {
    if (style_str) {
      rc = cdd_json_object_set_string(p_obj, "style", style_str);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (p->explode_set) {
      rc = cdd_json_object_set_boolean(p_obj, "explode", p->explode ? 1 : 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (p->explode) {
      rc = cdd_json_object_set_boolean(p_obj, "explode", 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (p->allow_reserved_set) {
      rc = cdd_json_object_set_boolean(p_obj, "allowReserved",
                                       p->allow_reserved ? 1 : 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (p->content_media_types && p->n_content_media_types > 0) {
    rc = write_media_type_map(p_obj, "content", p->content_media_types,
                              p->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (p->content_ref) {
    const char *content_key = p->content_type
                                  ? p->content_type
                                  : (p->in == OA_PARAM_IN_QUERYSTRING
                                         ? "application/x-www-form-urlencoded"
                                         : "application/json");
    rc = write_single_content_ref(p_obj, content_key, p->content_ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (p->content_type || p->in == OA_PARAM_IN_QUERYSTRING) {
    rc = write_param_inline_content(p_obj, p);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    int has_data = 0;
    rc = schema_ref_has_data(&p->schema, &has_data);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (p->schema_set && has_data) {
      rc = write_schema_ref(p_obj, "schema", &p->schema);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (p->is_array || p->type) {
      rc = write_schema_from_type_fields(p_obj, "schema",
                                         p->type ? p->type : "string",
                                         p->is_array, p->items_type);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (p->extensions_json) {
    return merge_schema_extras_object_openapi(p_obj, p->extensions_json);
  }
  return CDD_C_SUCCESS;
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
  const char *style_str = NULL;
  cdd_c_error_t rc;

  if (!h_obj || !h)
    return CDD_C_SUCCESS;

  if (h->ref) {
    rc = cdd_json_object_set_string(h_obj, "$ref", h->ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (h->description) {
      return cdd_json_object_set_string(h_obj, "description", h->description);
    }
    return CDD_C_SUCCESS;
  }

  if (h->description) {
    rc = cdd_json_object_set_string(h_obj, "description", h->description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (h->required) {
    rc = cdd_json_object_set_boolean(h_obj, "required", 1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (h->deprecated_set) {
    rc =
        cdd_json_object_set_boolean(h_obj, "deprecated", h->deprecated ? 1 : 0);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (h->style_set) {
    rc = style_to_str_openapi(h->style, &style_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (style_str) {
      rc = cdd_json_object_set_string(h_obj, "style", style_str);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (h->collection_format) {
    rc = cdd_json_object_set_string(h_obj, "collectionFormat",
                                    h->collection_format);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (h->explode_set) {
    rc = cdd_json_object_set_boolean(h_obj, "explode", h->explode ? 1 : 0);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (h->example_location == OA_EXAMPLE_LOC_OBJECT) {
    rc = write_example_fields(h_obj, &h->example, h->example_set, h->examples,
                              h->n_examples);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (h->content_media_types && h->n_content_media_types > 0) {
    rc = write_media_type_map(h_obj, "content", h->content_media_types,
                              h->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (h->content_ref) {
    rc = write_single_content_ref(
        h_obj, h->content_type ? h->content_type : "application/json",
        h->content_ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (h->content_type) {
    rc = write_header_inline_content(h_obj, h);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    int has_data = 0;
    rc = schema_ref_has_data(&h->schema, &has_data);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (h->schema_set && has_data) {
      rc = write_schema_ref(h_obj, "schema", &h->schema);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      rc = write_schema_from_type_fields(h_obj, "schema",
                                         h->type ? h->type : "string",
                                         h->is_array, h->items_type);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (h->extensions_json) {
    return merge_schema_extras_object_openapi(h_obj, h->extensions_json);
  }
  return CDD_C_SUCCESS;
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
  cdd_c_error_t rc;
  if (!enc_obj || !enc)
    return CDD_C_SUCCESS;

  if (enc->content_type) {
    rc = cdd_json_object_set_string(enc_obj, "contentType", enc->content_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (enc->style_set) {
    const char *style_str = NULL;
    rc = style_to_str_openapi(enc->style, &style_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (style_str) {
      rc = cdd_json_object_set_string(enc_obj, "style", style_str);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (enc->explode_set) {
    rc = cdd_json_object_set_boolean(enc_obj, "explode", enc->explode ? 1 : 0);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (enc->allow_reserved_set) {
    rc = cdd_json_object_set_boolean(enc_obj, "allowReserved",
                                     enc->allow_reserved ? 1 : 0);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (enc->headers && enc->n_headers > 0) {
    rc = write_headers_map(enc_obj, "headers", enc->headers, enc->n_headers, 1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (enc->encoding && enc->n_encoding > 0) {
    rc = write_encoding_map(enc_obj, enc->encoding, enc->n_encoding);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (enc->prefix_encoding && enc->n_prefix_encoding > 0) {
    rc = write_encoding_array(enc_obj, "prefixEncoding", enc->prefix_encoding,
                              enc->n_prefix_encoding);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (enc->item_encoding && enc->item_encoding_set) {
    JSON_Value *item_val = NULL;
    JSON_Object *item_obj = NULL;
    rc = oa_emit_new_object(&item_val, &item_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = write_encoding_object(item_obj, enc->item_encoding);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(item_val));
    }
    rc = cdd_json_object_set_value_safe(enc_obj, "itemEncoding", item_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (enc->extensions_json) {
    return merge_schema_extras_object_openapi(enc_obj, enc->extensions_json);
  }

  return CDD_C_SUCCESS;
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

  rc = oa_emit_new_object(&enc_val, &enc_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < n_encoding; ++i) {
    const struct OpenAPI_Encoding *enc = &encoding[i];
    JSON_Value *e_val = NULL;
    JSON_Object *e_obj = NULL;
    const char *name = enc->name ? enc->name : "encoding";

    rc = oa_emit_new_object(&e_val, &e_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(enc_val));
    }
    rc = write_encoding_object(e_obj, enc);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(e_val);
      cdd_c_error_t f2 = cdd_json_value_free(enc_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }
    rc = cdd_json_object_set_value_safe(enc_obj, name, e_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(enc_val));
    }
  }

  return cdd_json_object_set_value_safe(media_obj, "encoding", enc_val);
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

  rc = oa_emit_new_array(&arr_val, &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < n_encoding; ++i) {
    JSON_Value *e_val = NULL;
    JSON_Object *e_obj = NULL;
    rc = oa_emit_new_object(&e_val, &e_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
    }
    rc = write_encoding_object(e_obj, &encoding[i]);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(e_val);
      cdd_c_error_t f2 = cdd_json_value_free(arr_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }
    rc = cdd_json_array_append_value_safe(arr, e_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
    }
  }

  return cdd_json_object_set_value_safe(parent, key, arr_val);
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
    return cdd_json_object_set_string(media_obj, "$ref", mt->ref);
  }
  rc = schema_ref_has_data(&mt->schema, &has_schema);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (mt->schema_set || has_schema) {
    rc = write_schema_ref(media_obj, "schema", &mt->schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  rc = schema_ref_has_data(&mt->item_schema, &has_item_schema);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (mt->item_schema_set || has_item_schema) {
    rc = write_schema_ref(media_obj, "itemSchema", &mt->item_schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  rc = write_example_fields(media_obj, &mt->example, mt->example_set,
                            mt->examples, mt->n_examples);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (mt->encoding && mt->n_encoding > 0) {
    rc = write_encoding_map(media_obj, mt->encoding, mt->n_encoding);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (mt->prefix_encoding && mt->n_prefix_encoding > 0) {
    rc = write_encoding_array(media_obj, "prefixEncoding", mt->prefix_encoding,
                              mt->n_prefix_encoding);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (mt->item_encoding && mt->item_encoding_set) {
    JSON_Value *item_val = NULL;
    JSON_Object *item_obj = NULL;
    rc = oa_emit_new_object(&item_val, &item_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = write_encoding_object(item_obj, mt->item_encoding);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(item_val));
    }
    rc = cdd_json_object_set_value_safe(media_obj, "itemEncoding", item_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (mt->extensions_json) {
    return merge_schema_extras_object_openapi(media_obj, mt->extensions_json);
  }
  return CDD_C_SUCCESS;
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

  rc = oa_emit_new_object(&content_val, &content_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < n_mts; ++i) {
    const struct OpenAPI_MediaType *mt = &mts[i];
    const char *name = mt->name ? mt->name : "application/json";
    JSON_Value *mt_val = NULL;
    JSON_Object *mt_obj = NULL;
    rc = oa_emit_new_object(&mt_val, &mt_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(content_val));
    }

    rc = write_media_type_object(mt_obj, mt);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(mt_val);
      cdd_c_error_t f2 = cdd_json_value_free(content_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }
    rc = cdd_json_object_set_value_safe(content_obj, name, mt_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(content_val));
    }
  }

  return cdd_json_object_set_value_safe(parent, key, content_val);
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
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (link->summary) {
      rc = cdd_json_object_set_string(l_obj, "summary", link->summary);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (link->description) {
      rc = cdd_json_object_set_string(l_obj, "description", link->description);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return CDD_C_SUCCESS;
  }

  rc = openapi_emit_reflected_strings(l_obj, &OpenAPI_Link_type_info, link);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (link->n_parameters > 0 && link->parameters) {
    JSON_Value *params_val = NULL;
    JSON_Object *params_obj = NULL;
    size_t i;
    rc = oa_emit_new_object(&params_val, &params_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    for (i = 0; i < link->n_parameters; ++i) {
      const struct OpenAPI_LinkParam *param = &link->parameters[i];
      JSON_Value *val = NULL;
      rc = any_to_json_value(&param->value, &val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(params_val));
      }
      if (val && param->name) {
        rc = cdd_json_object_set_value_safe(params_obj, param->name, val);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(rc, cdd_json_value_free(params_val));
        }
      } else if (val) {
        rc = cdd_json_value_free(val);
      }
    }
    rc = cdd_json_object_set_value_safe(l_obj, "parameters", params_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (link->request_body_set) {
    JSON_Value *rb_val = NULL;
    rc = any_to_json_value(&link->request_body, &rb_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (rb_val) {
      rc = cdd_json_object_set_value_safe(l_obj, "requestBody", rb_val);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (link->server_set && link->server) {
    JSON_Value *srv_val = NULL;
    JSON_Object *srv_obj = NULL;
    rc = oa_emit_new_object(&srv_val, &srv_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = write_server_object(srv_obj, link->server);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(srv_val));
    }
    rc = cdd_json_object_set_value_safe(l_obj, "server", srv_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (link->extensions_json) {
    return merge_schema_extras_object_openapi(l_obj, link->extensions_json);
  }
  return CDD_C_SUCCESS;
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

  rc = oa_emit_new_object(&headers_val, &headers_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < n_headers; ++i) {
    const struct OpenAPI_Header *h = &headers[i];
    const char *name = h->name ? h->name : "header";
    JSON_Value *h_val = NULL;
    JSON_Object *h_obj = NULL;
    int is_ct = 0;

    if (ignore_content_type) {
      rc = header_name_is_content_type_openapi(name, &is_ct);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(headers_val));
      }
      if (is_ct)
        continue;
    }
    rc = oa_emit_new_object(&h_val, &h_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(headers_val));
    }
    rc = write_header_object(h_obj, h);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(h_val);
      cdd_c_error_t f2 = cdd_json_value_free(headers_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }
    rc = cdd_json_object_set_value_safe(headers_obj, name, h_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(headers_val));
    }
    written++;
  }

  if (written == 0) {
    return cdd_json_value_free(headers_val);
  }
  return cdd_json_object_set_value_safe(parent, key, headers_val);
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

  rc = oa_emit_new_object(&links_val, &links_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < resp->n_links; ++i) {
    const struct OpenAPI_Link *link = &resp->links[i];
    JSON_Value *l_val = NULL;
    JSON_Object *l_obj = NULL;
    const char *name = link->name ? link->name : "link";

    rc = oa_emit_new_object(&l_val, &l_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(links_val));
    }
    rc = write_link_object(l_obj, link);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(l_val);
      cdd_c_error_t f2 = cdd_json_value_free(links_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }
    rc = cdd_json_object_set_value_safe(links_obj, name, l_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(links_val));
    }
  }

  return cdd_json_object_set_value_safe(parent, "links", links_val);
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
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (resp->summary) {
      rc = cdd_json_object_set_string(r_obj, "summary", resp->summary);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (resp->description) {
      rc = cdd_json_object_set_string(r_obj, "description", resp->description);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return CDD_C_SUCCESS;
  }

  if (resp->summary) {
    rc = cdd_json_object_set_string(r_obj, "summary", resp->summary);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  rc = cdd_json_object_set_string(r_obj, "description",
                                  resp->description ? resp->description : "");
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (resp->headers && resp->n_headers > 0) {
    rc = write_headers(r_obj, resp);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (resp->links && resp->n_links > 0) {
    rc = write_links(r_obj, resp);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (resp->content_media_types && resp->n_content_media_types > 0) {
    rc = write_media_type_map(r_obj, "content", resp->content_media_types,
                              resp->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (resp->content_ref) {
    rc = write_single_content_ref(
        r_obj, resp->content_type ? resp->content_type : "application/json",
        resp->content_ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    int has_data = 0;
    rc = schema_ref_has_data(&resp->schema, &has_data);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (has_data || resp->content_type) {
      JSON_Value *cont_val = NULL;
      JSON_Object *cont_obj = NULL;
      JSON_Value *media_val = NULL;
      JSON_Object *media_obj = NULL;

      rc = oa_emit_new_object(&cont_val, &cont_obj);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = oa_emit_new_object(&media_val, &media_obj);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(cont_val));
      }

      if (has_data) {
        rc = write_schema_ref(media_obj, "schema", &resp->schema);
        if (rc != CDD_C_SUCCESS) {
          cdd_c_error_t f1 = cdd_json_value_free(media_val);
          cdd_c_error_t f2 = cdd_json_value_free(cont_val);
          return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
        }
      }
      rc = write_example_fields(media_obj, &resp->example, resp->example_set,
                                resp->examples, resp->n_examples);
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t f1 = cdd_json_value_free(media_val);
        cdd_c_error_t f2 = cdd_json_value_free(cont_val);
        return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
      }

      rc = cdd_json_object_set_value_safe(
          cont_obj,
          resp->content_type ? resp->content_type : "application/json",
          media_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(cont_val));
      }
      rc = cdd_json_object_set_value_safe(r_obj, "content", cont_val);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (resp->extensions_json) {
    return merge_schema_extras_object_openapi(r_obj, resp->extensions_json);
  }
  return CDD_C_SUCCESS;
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

  rc = oa_emit_new_array(&arr_val, &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < n_params; ++i) {
    const struct OpenAPI_Parameter *p = &params[i];
    JSON_Value *p_val = NULL;
    JSON_Object *p_obj = NULL;
    int is_res = 0;
    rc = param_is_reserved_header_openapi(p, &is_res);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
    }
    if (is_res)
      continue;
    rc = oa_emit_new_object(&p_val, &p_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
    }
    rc = write_parameter_object(p_obj, p);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(p_val);
      cdd_c_error_t f2 = cdd_json_value_free(arr_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }

    rc = cdd_json_array_append_value_safe(arr, p_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
    }
    written++;
  }

  if (written == 0) {
    return cdd_json_value_free(arr_val);
  }
  return cdd_json_object_set_value_safe(parent, "parameters", arr_val);
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
  cdd_c_error_t rc;

  if (!rb_obj || !rb)
    return CDD_C_SUCCESS;

  if (rb->content_media_types && rb->n_content_media_types > 0) {
    rc = write_media_type_map(rb_obj, "content", rb->content_media_types,
                              rb->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (rb->content_ref) {
    rc = write_single_content_ref(
        rb_obj,
        rb->schema.content_type ? rb->schema.content_type : "application/json",
        rb->content_ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    rc = oa_emit_new_object(&content_val, &content_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = oa_emit_new_object(&media_val, &media_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(content_val));
    }

    rc = write_schema_ref(media_obj, "schema", &rb->schema);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(media_val);
      cdd_c_error_t f2 = cdd_json_value_free(content_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }
    rc = write_example_fields(media_obj, &rb->example, rb->example_set,
                              rb->examples, rb->n_examples);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(media_val);
      cdd_c_error_t f2 = cdd_json_value_free(content_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }

    rc = cdd_json_object_set_value_safe(
        content_obj,
        rb->schema.content_type ? rb->schema.content_type : "application/json",
        media_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(content_val));
    }
    rc = cdd_json_object_set_value_safe(rb_obj, "content", content_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (rb->description) {
    rc = cdd_json_object_set_string(rb_obj, "description", rb->description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (rb->required_set) {
    rc = cdd_json_object_set_boolean(rb_obj, "required", rb->required ? 1 : 0);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (rb->extensions_json) {
    return merge_schema_extras_object_openapi(rb_obj, rb->extensions_json);
  }

  return CDD_C_SUCCESS;
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
    rc = oa_emit_new_object(&rb_val, &rb_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_object_set_string(rb_obj, "$ref", op->req_body_ref);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(rb_val));
    }
    if (op->req_body_description) {
      rc = cdd_json_object_set_string(rb_obj, "description",
                                      op->req_body_description);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(rb_val));
      }
    }
    if (op->req_body_extensions_json) {
      rc = merge_schema_extras_object_openapi(rb_obj,
                                              op->req_body_extensions_json);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(rb_val));
      }
    }
    return cdd_json_object_set_value_safe(op_obj, "requestBody", rb_val);
  }

  /* If body is empty and no fields, skip */
  {
    int has_data = 0;
    rc = schema_ref_has_data(&op->req_body, &has_data);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!has_data && op->req_body.content_type == NULL &&
        op->n_req_body_media_types == 0) {
      return CDD_C_SUCCESS;
    }
  }

  rc = oa_emit_new_object(&rb_val, &rb_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  {
    struct OpenAPI_RequestBody rb = {0};
    rb.description = op->req_body_description;
    rb.required = op->req_body_required;
    rb.required_set = op->req_body_required_set;
    rb.schema = op->req_body;
    rb.content_media_types = op->req_body_media_types;
    rb.n_content_media_types = op->n_req_body_media_types;
    rb.extensions_json = op->req_body_extensions_json;
    rc = write_request_body_object(rb_obj, &rb);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(rb_val));
    }
  }

  return cdd_json_object_set_value_safe(op_obj, "requestBody", rb_val);
}
