/**
 * @file openapi_copy_components.c
 * @brief Deep copy routines for components, parameters, and headers.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Creates a deep copy of parameter fields.
 *
 * @param[out] dst Destination OpenAPI_Parameter structure.
 * @param[in] src Source OpenAPI_Parameter structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_parameter_fields(struct OpenAPI_Parameter *dst,
                                    const struct OpenAPI_Parameter *src) {
  cdd_c_error_t rc;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  dst->in = src->in;
  dst->required = src->required;
  dst->deprecated = src->deprecated;
  dst->deprecated_set = src->deprecated_set;
  dst->is_array = src->is_array;
  dst->style = src->style;
  dst->explode = src->explode;
  dst->explode_set = src->explode_set;
  dst->allow_reserved = src->allow_reserved;
  dst->allow_reserved_set = src->allow_reserved_set;
  dst->allow_empty_value = src->allow_empty_value;
  dst->allow_empty_value_set = src->allow_empty_value_set;
  dst->example_location = src->example_location;
  if (src->name) {
    rc = c_cdd_strdup(src->name, &dst->name);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->type) {
    rc = c_cdd_strdup(src->type, &dst->type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->description) {
    rc = c_cdd_strdup(src->description, &dst->description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->content_type) {
    rc = c_cdd_strdup(src->content_type, &dst->content_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->content_ref) {
    rc = c_cdd_strdup(src->content_ref, &dst->content_ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->content_media_types && src->n_content_media_types > 0) {
    rc = copy_media_type_array(
        &dst->content_media_types, &dst->n_content_media_types,
        src->content_media_types, src->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->schema_set) {
    dst->schema_set = 1;
    rc = copy_schema_ref(&dst->schema, &src->schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->items_type) {
    rc = c_cdd_strdup(src->items_type, &dst->items_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->collection_format) {
    rc = c_cdd_strdup(src->collection_format, &dst->collection_format);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->example_set) {
    rc = copy_any_value(&dst->example, &src->example);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    size_t i;
    dst->examples = (struct OpenAPI_Example *)C_CDD_CALLOC(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return CDD_C_ERROR_MEMORY;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      rc = copy_example_fields(&dst->examples[i], &src->examples[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of header fields.
 *
 * @param[out] dst Destination OpenAPI_Header structure.
 * @param[in] src Source OpenAPI_Header structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_header_fields(struct OpenAPI_Header *dst,
                                 const struct OpenAPI_Header *src) {
  cdd_c_error_t rc;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  dst->required = src->required;
  dst->deprecated = src->deprecated;
  dst->deprecated_set = src->deprecated_set;
  dst->style = src->style;
  dst->style_set = src->style_set;
  dst->explode = src->explode;
  dst->explode_set = src->explode_set;
  dst->is_array = src->is_array;
  dst->example_location = src->example_location;
  if (src->description) {
    rc = c_cdd_strdup(src->description, &dst->description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->content_type) {
    rc = c_cdd_strdup(src->content_type, &dst->content_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->content_ref) {
    rc = c_cdd_strdup(src->content_ref, &dst->content_ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->content_media_types && src->n_content_media_types > 0) {
    rc = copy_media_type_array(
        &dst->content_media_types, &dst->n_content_media_types,
        src->content_media_types, src->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->schema_set) {
    dst->schema_set = 1;
    rc = copy_schema_ref(&dst->schema, &src->schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->type) {
    rc = c_cdd_strdup(src->type, &dst->type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->items_type) {
    rc = c_cdd_strdup(src->items_type, &dst->items_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->collection_format) {
    rc = c_cdd_strdup(src->collection_format, &dst->collection_format);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->example_set) {
    rc = copy_any_value(&dst->example, &src->example);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    size_t i;
    dst->examples = (struct OpenAPI_Example *)C_CDD_CALLOC(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return CDD_C_ERROR_MEMORY;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      rc = copy_example_fields(&dst->examples[i], &src->examples[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of encoding fields.
 *
 * @param[out] dst Destination OpenAPI_Encoding structure.
 * @param[in] src Source OpenAPI_Encoding structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_encoding_fields(struct OpenAPI_Encoding *dst,
                                   const struct OpenAPI_Encoding *src) {
  size_t i;
  cdd_c_error_t rc;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  dst->style = src->style;
  dst->style_set = src->style_set;
  dst->explode = src->explode;
  dst->explode_set = src->explode_set;
  dst->allow_reserved = src->allow_reserved;
  dst->allow_reserved_set = src->allow_reserved_set;
  if (src->name && !dst->name) {
    rc = c_cdd_strdup(src->name, &dst->name);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->content_type) {
    rc = c_cdd_strdup(src->content_type, &dst->content_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->headers && src->n_headers > 0) {
    dst->headers = (struct OpenAPI_Header *)C_CDD_CALLOC(
        src->n_headers, sizeof(struct OpenAPI_Header));
    if (!dst->headers)
      return CDD_C_ERROR_MEMORY;
    dst->n_headers = src->n_headers;
    for (i = 0; i < src->n_headers; ++i) {
      struct OpenAPI_Header *dst_hdr = &dst->headers[i];
      const struct OpenAPI_Header *src_hdr = &src->headers[i];
      if (src_hdr->name) {
        rc = c_cdd_strdup(src_hdr->name, &dst_hdr->name);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      rc = copy_header_fields(dst_hdr, src_hdr);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->encoding && src->n_encoding > 0) {
    dst->encoding = (struct OpenAPI_Encoding *)C_CDD_CALLOC(
        src->n_encoding, sizeof(struct OpenAPI_Encoding));
    if (!dst->encoding)
      return CDD_C_ERROR_MEMORY;
    dst->n_encoding = src->n_encoding;
    for (i = 0; i < src->n_encoding; ++i) {
      rc = copy_encoding_fields(&dst->encoding[i], &src->encoding[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->prefix_encoding && src->n_prefix_encoding > 0) {
    dst->prefix_encoding = (struct OpenAPI_Encoding *)C_CDD_CALLOC(
        src->n_prefix_encoding, sizeof(struct OpenAPI_Encoding));
    if (!dst->prefix_encoding)
      return CDD_C_ERROR_MEMORY;
    dst->n_prefix_encoding = src->n_prefix_encoding;
    for (i = 0; i < src->n_prefix_encoding; ++i) {
      rc = copy_encoding_fields(&dst->prefix_encoding[i],
                                &src->prefix_encoding[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->item_encoding) {
    dst->item_encoding = (struct OpenAPI_Encoding *)C_CDD_CALLOC(
        1, sizeof(struct OpenAPI_Encoding));
    if (!dst->item_encoding)
      return CDD_C_ERROR_MEMORY;
    dst->item_encoding_set = 1;
    rc = copy_encoding_fields(dst->item_encoding, src->item_encoding);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of media type fields.
 *
 * @param[out] dst Destination OpenAPI_MediaType structure.
 * @param[in] src Source OpenAPI_MediaType structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_media_type_fields(struct OpenAPI_MediaType *dst,
                                     const struct OpenAPI_MediaType *src) {
  size_t i;
  cdd_c_error_t rc;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  if (src->name && !dst->name) {
    rc = c_cdd_strdup(src->name, &dst->name);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->ref && !dst->ref) {
    rc = c_cdd_strdup(src->ref, &dst->ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->schema_set || src->schema.ref_name || src->schema.inline_type ||
      src->schema.is_array || src->schema.n_multipart_fields > 0) {
    rc = copy_schema_ref(&dst->schema, &src->schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->schema_set = 1;
  }
  if (src->item_schema_set || src->item_schema.ref_name ||
      src->item_schema.inline_type || src->item_schema.is_array ||
      src->item_schema.n_multipart_fields > 0) {
    rc = copy_schema_ref(&dst->item_schema, &src->item_schema);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->item_schema_set = 1;
  }
  if (src->example_set) {
    rc = copy_any_value(&dst->example, &src->example);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    dst->examples = (struct OpenAPI_Example *)C_CDD_CALLOC(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return CDD_C_ERROR_MEMORY;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      rc = copy_example_fields(&dst->examples[i], &src->examples[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->encoding && src->n_encoding > 0) {
    dst->encoding = (struct OpenAPI_Encoding *)C_CDD_CALLOC(
        src->n_encoding, sizeof(struct OpenAPI_Encoding));
    if (!dst->encoding)
      return CDD_C_ERROR_MEMORY;
    dst->n_encoding = src->n_encoding;
    for (i = 0; i < src->n_encoding; ++i) {
      rc = copy_encoding_fields(&dst->encoding[i], &src->encoding[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->prefix_encoding && src->n_prefix_encoding > 0) {
    dst->prefix_encoding = (struct OpenAPI_Encoding *)C_CDD_CALLOC(
        src->n_prefix_encoding, sizeof(struct OpenAPI_Encoding));
    if (!dst->prefix_encoding)
      return CDD_C_ERROR_MEMORY;
    dst->n_prefix_encoding = src->n_prefix_encoding;
    for (i = 0; i < src->n_prefix_encoding; ++i) {
      rc = copy_encoding_fields(&dst->prefix_encoding[i],
                                &src->prefix_encoding[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->item_encoding) {
    dst->item_encoding = (struct OpenAPI_Encoding *)C_CDD_CALLOC(
        1, sizeof(struct OpenAPI_Encoding));
    if (!dst->item_encoding)
      return CDD_C_ERROR_MEMORY;
    dst->item_encoding_set = 1;
    rc = copy_encoding_fields(dst->item_encoding, src->item_encoding);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of media type array.
 *
 * @param[out] dst Output array of OpenAPI_MediaType structures.
 * @param[out] dst_count Output count of copied media types.
 * @param[in] src Source array of OpenAPI_MediaType structures.
 * @param[in] src_count Number of elements in src.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_media_type_array(struct OpenAPI_MediaType **dst,
                                    size_t *dst_count,
                                    const struct OpenAPI_MediaType *src,
                                    size_t src_count) {
  size_t i;
  if (!dst || !dst_count)
    return CDD_C_SUCCESS;
  *dst = NULL;
  *dst_count = 0;
  if (!src || src_count == 0)
    return CDD_C_SUCCESS;
  *dst = (struct OpenAPI_MediaType *)C_CDD_CALLOC(
      src_count, sizeof(struct OpenAPI_MediaType));
  if (!*dst)
    return CDD_C_ERROR_MEMORY;
  *dst_count = src_count;
  for (i = 0; i < src_count; ++i) {
    cdd_c_error_t rc = copy_media_type_fields(&(*dst)[i], &src[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of response fields.
 *
 * @param[out] dst Destination OpenAPI_Response structure.
 * @param[in] src Source OpenAPI_Response structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t copy_response_fields(struct OpenAPI_Response *dst,
                                   const struct OpenAPI_Response *src) {
  size_t i;
  cdd_c_error_t rc;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  if (src->summary) {
    rc = c_cdd_strdup(src->summary, &dst->summary);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->description) {
    rc = c_cdd_strdup(src->description, &dst->description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->content_type) {
    rc = c_cdd_strdup(src->content_type, &dst->content_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->content_ref) {
    rc = c_cdd_strdup(src->content_ref, &dst->content_ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->content_media_types && src->n_content_media_types > 0) {
    rc = copy_media_type_array(
        &dst->content_media_types, &dst->n_content_media_types,
        src->content_media_types, src->n_content_media_types);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (src->example_set) {
    rc = copy_any_value(&dst->example, &src->example);
    if (rc != CDD_C_SUCCESS)
      return rc;
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    dst->examples = (struct OpenAPI_Example *)C_CDD_CALLOC(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return CDD_C_ERROR_MEMORY;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      rc = copy_example_fields(&dst->examples[i], &src->examples[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->n_headers > 0 && src->headers) {
    dst->headers = (struct OpenAPI_Header *)C_CDD_CALLOC(
        src->n_headers, sizeof(struct OpenAPI_Header));
    if (!dst->headers)
      return CDD_C_ERROR_MEMORY;
    dst->n_headers = src->n_headers;
    for (i = 0; i < src->n_headers; ++i) {
      struct OpenAPI_Header *dst_hdr = &dst->headers[i];
      const struct OpenAPI_Header *src_hdr = &src->headers[i];
      if (src_hdr->name) {
        rc = c_cdd_strdup(src_hdr->name, &dst_hdr->name);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      rc = copy_header_fields(dst_hdr, src_hdr);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (src->n_links > 0 && src->links) {
    dst->links = (struct OpenAPI_Link *)C_CDD_CALLOC(
        src->n_links, sizeof(struct OpenAPI_Link));
    if (!dst->links)
      return CDD_C_ERROR_MEMORY;
    dst->n_links = src->n_links;
    for (i = 0; i < src->n_links; ++i) {
      struct OpenAPI_Link *dst_link = &dst->links[i];
      const struct OpenAPI_Link *src_link = &src->links[i];
      if (src_link->name) {
        rc = c_cdd_strdup(src_link->name, &dst_link->name);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (src_link->ref) {
        rc = c_cdd_strdup(src_link->ref, &dst_link->ref);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      rc = copy_link_fields(dst_link, src_link);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return copy_schema_ref(&dst->schema, &src->schema);
}
