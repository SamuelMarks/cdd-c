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
 */
cdd_c_error_t copy_parameter_fields(struct OpenAPI_Parameter *dst,
                                    const struct OpenAPI_Parameter *src) {
  char *_ast_strdup_98 = NULL;
  char *_ast_strdup_99 = NULL;
  char *_ast_strdup_100 = NULL;
  char *_ast_strdup_101 = NULL;
  char *_ast_strdup_102 = NULL;
  char *_ast_strdup_103 = NULL;
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
    dst->name = (c_cdd_strdup(src->name, &_ast_strdup_98), _ast_strdup_98);
    if (!dst->name)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->type) {
    dst->type = (c_cdd_strdup(src->type, &_ast_strdup_99), _ast_strdup_99);
    if (!dst->type)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->description) {
    dst->description =
        (c_cdd_strdup(src->description, &_ast_strdup_100), _ast_strdup_100);
    if (!dst->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_type) {
    dst->content_type =
        (c_cdd_strdup(src->content_type, &_ast_strdup_101), _ast_strdup_101);
    if (!dst->content_type)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_ref) {
    dst->content_ref =
        (c_cdd_strdup(src->content_ref, &_ast_strdup_102), _ast_strdup_102);
    if (!dst->content_ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_media_types && src->n_content_media_types > 0) {
    {
      cdd_c_error_t _rc = copy_media_type_array(
          &dst->content_media_types, &dst->n_content_media_types,
          src->content_media_types, src->n_content_media_types);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }
  if (src->schema_set) {
    dst->schema_set = 1;
    {
      cdd_c_error_t _rc = copy_schema_ref(&dst->schema, &src->schema);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }
  if (src->items_type) {
    dst->items_type =
        (c_cdd_strdup(src->items_type, &_ast_strdup_103), _ast_strdup_103);
    if (!dst->items_type)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->example_set) {
    {
      cdd_c_error_t _rc = copy_any_value(&dst->example, &src->example);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    size_t i;
    dst->examples = (struct OpenAPI_Example *)calloc(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return CDD_C_ERROR_MEMORY;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      {
        cdd_c_error_t _rc =
            copy_example_fields(&dst->examples[i], &src->examples[i]);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of header fields.
 */
cdd_c_error_t copy_header_fields(struct OpenAPI_Header *dst,
                                 const struct OpenAPI_Header *src) {
  char *_ast_strdup_104 = NULL;
  char *_ast_strdup_105 = NULL;
  char *_ast_strdup_106 = NULL;
  char *_ast_strdup_107 = NULL;
  char *_ast_strdup_108 = NULL;
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
    dst->description =
        (c_cdd_strdup(src->description, &_ast_strdup_104), _ast_strdup_104);
    if (!dst->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_type) {
    dst->content_type =
        (c_cdd_strdup(src->content_type, &_ast_strdup_105), _ast_strdup_105);
    if (!dst->content_type)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_ref) {
    dst->content_ref =
        (c_cdd_strdup(src->content_ref, &_ast_strdup_106), _ast_strdup_106);
    if (!dst->content_ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_media_types && src->n_content_media_types > 0) {
    {
      cdd_c_error_t _rc = copy_media_type_array(
          &dst->content_media_types, &dst->n_content_media_types,
          src->content_media_types, src->n_content_media_types);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }
  if (src->schema_set) {
    dst->schema_set = 1;
    {
      cdd_c_error_t _rc = copy_schema_ref(&dst->schema, &src->schema);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }
  if (src->type) {
    dst->type = (c_cdd_strdup(src->type, &_ast_strdup_107), _ast_strdup_107);
    if (!dst->type)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->items_type) {
    dst->items_type =
        (c_cdd_strdup(src->items_type, &_ast_strdup_108), _ast_strdup_108);
    if (!dst->items_type)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->example_set) {
    {
      cdd_c_error_t _rc = copy_any_value(&dst->example, &src->example);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    size_t i;
    dst->examples = (struct OpenAPI_Example *)calloc(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return CDD_C_ERROR_MEMORY;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      {
        cdd_c_error_t _rc =
            copy_example_fields(&dst->examples[i], &src->examples[i]);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of encoding fields.
 */
cdd_c_error_t copy_encoding_fields(struct OpenAPI_Encoding *dst,
                                   const struct OpenAPI_Encoding *src) {
  char *_ast_strdup_109 = NULL;
  char *_ast_strdup_110 = NULL;
  char *_ast_strdup_111 = NULL;
  size_t i;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  dst->style = src->style;
  dst->style_set = src->style_set;
  dst->explode = src->explode;
  dst->explode_set = src->explode_set;
  dst->allow_reserved = src->allow_reserved;
  dst->allow_reserved_set = src->allow_reserved_set;
  if (src->name && !dst->name) {
    dst->name = (c_cdd_strdup(src->name, &_ast_strdup_109), _ast_strdup_109);
    if (!dst->name)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_type) {
    dst->content_type =
        (c_cdd_strdup(src->content_type, &_ast_strdup_110), _ast_strdup_110);
    if (!dst->content_type)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->headers && src->n_headers > 0) {
    dst->headers = (struct OpenAPI_Header *)calloc(
        src->n_headers, sizeof(struct OpenAPI_Header));
    if (!dst->headers)
      return CDD_C_ERROR_MEMORY;
    dst->n_headers = src->n_headers;
    for (i = 0; i < src->n_headers; ++i) {
      struct OpenAPI_Header *dst_hdr = &dst->headers[i];
      const struct OpenAPI_Header *src_hdr = &src->headers[i];
      if (src_hdr->name) {
        dst_hdr->name =
            (c_cdd_strdup(src_hdr->name, &_ast_strdup_111), _ast_strdup_111);
        if (!dst_hdr->name)
          return CDD_C_ERROR_MEMORY;
      }
      {
        cdd_c_error_t _rc = copy_header_fields(dst_hdr, src_hdr);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }
  if (src->encoding && src->n_encoding > 0) {
    dst->encoding = (struct OpenAPI_Encoding *)calloc(
        src->n_encoding, sizeof(struct OpenAPI_Encoding));
    if (!dst->encoding)
      return CDD_C_ERROR_MEMORY;
    dst->n_encoding = src->n_encoding;
    for (i = 0; i < src->n_encoding; ++i) {
      {
        cdd_c_error_t _rc =
            copy_encoding_fields(&dst->encoding[i], &src->encoding[i]);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }
  if (src->prefix_encoding && src->n_prefix_encoding > 0) {
    dst->prefix_encoding = (struct OpenAPI_Encoding *)calloc(
        src->n_prefix_encoding, sizeof(struct OpenAPI_Encoding));
    if (!dst->prefix_encoding)
      return CDD_C_ERROR_MEMORY;
    dst->n_prefix_encoding = src->n_prefix_encoding;
    for (i = 0; i < src->n_prefix_encoding; ++i) {
      {
        cdd_c_error_t _rc = copy_encoding_fields(&dst->prefix_encoding[i],
                                                 &src->prefix_encoding[i]);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }
  if (src->item_encoding) {
    dst->item_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    if (!dst->item_encoding)
      return CDD_C_ERROR_MEMORY;
    dst->item_encoding_set = 1;
    {
      cdd_c_error_t _rc =
          copy_encoding_fields(dst->item_encoding, src->item_encoding);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of media type fields.
 */
cdd_c_error_t copy_media_type_fields(struct OpenAPI_MediaType *dst,
                                     const struct OpenAPI_MediaType *src) {
  char *_ast_strdup_112 = NULL;
  char *_ast_strdup_113 = NULL;
  size_t i;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  if (src->name && !dst->name) {
    dst->name = (c_cdd_strdup(src->name, &_ast_strdup_112), _ast_strdup_112);
    if (!dst->name)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->ref && !dst->ref) {
    dst->ref = (c_cdd_strdup(src->ref, &_ast_strdup_113), _ast_strdup_113);
    if (!dst->ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->schema_set || src->schema.ref_name || src->schema.inline_type ||
      src->schema.is_array || src->schema.n_multipart_fields > 0) {
    {
      cdd_c_error_t _rc = copy_schema_ref(&dst->schema, &src->schema);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    dst->schema_set = 1;
  }
  if (src->item_schema_set || src->item_schema.ref_name ||
      src->item_schema.inline_type || src->item_schema.is_array ||
      src->item_schema.n_multipart_fields > 0) {
    {
      cdd_c_error_t _rc = copy_schema_ref(&dst->item_schema, &src->item_schema);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    dst->item_schema_set = 1;
  }
  if (src->example_set) {
    {
      cdd_c_error_t _rc = copy_any_value(&dst->example, &src->example);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    dst->examples = (struct OpenAPI_Example *)calloc(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return CDD_C_ERROR_MEMORY;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      {
        cdd_c_error_t _rc =
            copy_example_fields(&dst->examples[i], &src->examples[i]);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }
  if (src->encoding && src->n_encoding > 0) {
    dst->encoding = (struct OpenAPI_Encoding *)calloc(
        src->n_encoding, sizeof(struct OpenAPI_Encoding));
    if (!dst->encoding)
      return CDD_C_ERROR_MEMORY;
    dst->n_encoding = src->n_encoding;
    for (i = 0; i < src->n_encoding; ++i) {
      {
        cdd_c_error_t _rc =
            copy_encoding_fields(&dst->encoding[i], &src->encoding[i]);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }
  if (src->prefix_encoding && src->n_prefix_encoding > 0) {
    dst->prefix_encoding = (struct OpenAPI_Encoding *)calloc(
        src->n_prefix_encoding, sizeof(struct OpenAPI_Encoding));
    if (!dst->prefix_encoding)
      return CDD_C_ERROR_MEMORY;
    dst->n_prefix_encoding = src->n_prefix_encoding;
    for (i = 0; i < src->n_prefix_encoding; ++i) {
      {
        cdd_c_error_t _rc = copy_encoding_fields(&dst->prefix_encoding[i],
                                                 &src->prefix_encoding[i]);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }
  if (src->item_encoding) {
    dst->item_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    if (!dst->item_encoding)
      return CDD_C_ERROR_MEMORY;
    dst->item_encoding_set = 1;
    {
      cdd_c_error_t _rc =
          copy_encoding_fields(dst->item_encoding, src->item_encoding);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of media type array.
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
  *dst = (struct OpenAPI_MediaType *)calloc(src_count,
                                            sizeof(struct OpenAPI_MediaType));
  if (!*dst)
    return CDD_C_ERROR_MEMORY;
  *dst_count = src_count;
  for (i = 0; i < src_count; ++i) {
    if (copy_media_type_fields(&(*dst)[i], &src[i]) != 0)
      return CDD_C_ERROR_MEMORY;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a deep copy of response fields.
 */
cdd_c_error_t copy_response_fields(struct OpenAPI_Response *dst,
                                   const struct OpenAPI_Response *src) {
  char *_ast_strdup_114 = NULL;
  char *_ast_strdup_115 = NULL;
  char *_ast_strdup_116 = NULL;
  char *_ast_strdup_117 = NULL;
  char *_ast_strdup_118 = NULL;
  char *_ast_strdup_119 = NULL;
  char *_ast_strdup_120 = NULL;
  size_t i;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  if (src->summary) {
    dst->summary =
        (c_cdd_strdup(src->summary, &_ast_strdup_114), _ast_strdup_114);
    if (!dst->summary)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->description) {
    dst->description =
        (c_cdd_strdup(src->description, &_ast_strdup_115), _ast_strdup_115);
    if (!dst->description)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_type) {
    dst->content_type =
        (c_cdd_strdup(src->content_type, &_ast_strdup_116), _ast_strdup_116);
    if (!dst->content_type)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_ref) {
    dst->content_ref =
        (c_cdd_strdup(src->content_ref, &_ast_strdup_117), _ast_strdup_117);
    if (!dst->content_ref)
      return CDD_C_ERROR_MEMORY;
  }
  if (src->content_media_types && src->n_content_media_types > 0) {
    {
      cdd_c_error_t _rc = copy_media_type_array(
          &dst->content_media_types, &dst->n_content_media_types,
          src->content_media_types, src->n_content_media_types);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }
  if (src->example_set) {
    {
      cdd_c_error_t _rc = copy_any_value(&dst->example, &src->example);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    dst->examples = (struct OpenAPI_Example *)calloc(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return CDD_C_ERROR_MEMORY;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      {
        cdd_c_error_t _rc =
            copy_example_fields(&dst->examples[i], &src->examples[i]);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }
  if (src->n_headers > 0 && src->headers) {
    dst->headers = (struct OpenAPI_Header *)calloc(
        src->n_headers, sizeof(struct OpenAPI_Header));
    if (!dst->headers)
      return CDD_C_ERROR_MEMORY;
    dst->n_headers = src->n_headers;
    for (i = 0; i < src->n_headers; ++i) {
      struct OpenAPI_Header *dst_hdr = &dst->headers[i];
      const struct OpenAPI_Header *src_hdr = &src->headers[i];
      if (src_hdr->name) {
        dst_hdr->name =
            (c_cdd_strdup(src_hdr->name, &_ast_strdup_118), _ast_strdup_118);
        if (!dst_hdr->name)
          return CDD_C_ERROR_MEMORY;
      }
      {
        cdd_c_error_t _rc = copy_header_fields(dst_hdr, src_hdr);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }
  if (src->n_links > 0 && src->links) {
    dst->links = (struct OpenAPI_Link *)calloc(src->n_links,
                                               sizeof(struct OpenAPI_Link));
    if (!dst->links)
      return CDD_C_ERROR_MEMORY;
    dst->n_links = src->n_links;
    for (i = 0; i < src->n_links; ++i) {
      struct OpenAPI_Link *dst_link = &dst->links[i];
      const struct OpenAPI_Link *src_link = &src->links[i];
      if (src_link->name) {
        dst_link->name =
            (c_cdd_strdup(src_link->name, &_ast_strdup_119), _ast_strdup_119);
        if (!dst_link->name)
          return CDD_C_ERROR_MEMORY;
      }
      if (src_link->ref) {
        dst_link->ref =
            (c_cdd_strdup(src_link->ref, &_ast_strdup_120), _ast_strdup_120);
        if (!dst_link->ref)
          return CDD_C_ERROR_MEMORY;
      }
      {
        cdd_c_error_t _rc = copy_link_fields(dst_link, src_link);
        if (_rc != CDD_C_SUCCESS)
          return _rc;
      }
    }
  }
  return copy_schema_ref(&dst->schema, &src->schema);
}
