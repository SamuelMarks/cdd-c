/**
 * @file client_body_media.c
 * @brief Media type detection, verbs, and predicates for client body
 * generation.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "functions/emit/client_body_internal.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_fail_is_primitive_type;
#endif

/**
 * @brief Maps an OpenAPI HTTP verb enum to its C HTTP method constant name.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t client_body_verb_to_enum_str(enum OpenAPI_Verb v,
                                           const char **_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (v) {
  case OA_VERB_GET: {
    *_out_val = "HTTP_GET";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_POST: {
    *_out_val = "HTTP_POST";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_PUT: {
    *_out_val = "HTTP_PUT";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_DELETE: {
    *_out_val = "HTTP_DELETE";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_HEAD: {
    *_out_val = "HTTP_HEAD";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_PATCH: {
    *_out_val = "HTTP_PATCH";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_OPTIONS: {
    *_out_val = "HTTP_OPTIONS";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_TRACE: {
    *_out_val = "HTTP_TRACE";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_QUERY: {
    *_out_val = "HTTP_QUERY";
    return CDD_C_SUCCESS;
  }
  default: {
    *_out_val = "HTTP_GET";
    return CDD_C_SUCCESS;
  }
  }
}

/**
 * @brief Maps an HTTP method string to its C HTTP method constant name.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t client_body_method_str_to_enum_str(const char *method,
                                                 const char **_out_val) {
  int eq = 0;
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!method) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  c_cdd_str_iequal(method, "get", &eq);
  if (eq) {
    *_out_val = "HTTP_GET";
    return CDD_C_SUCCESS;
  }
  c_cdd_str_iequal(method, "post", &eq);
  if (eq) {
    *_out_val = "HTTP_POST";
    return CDD_C_SUCCESS;
  }
  c_cdd_str_iequal(method, "put", &eq);
  if (eq) {
    *_out_val = "HTTP_PUT";
    return CDD_C_SUCCESS;
  }
  c_cdd_str_iequal(method, "delete", &eq);
  if (eq) {
    *_out_val = "HTTP_DELETE";
    return CDD_C_SUCCESS;
  }
  c_cdd_str_iequal(method, "patch", &eq);
  if (eq) {
    *_out_val = "HTTP_PATCH";
    return CDD_C_SUCCESS;
  }
  c_cdd_str_iequal(method, "head", &eq);
  if (eq) {
    *_out_val = "HTTP_HEAD";
    return CDD_C_SUCCESS;
  }
  c_cdd_str_iequal(method, "options", &eq);
  if (eq) {
    *_out_val = "HTTP_OPTIONS";
    return CDD_C_SUCCESS;
  }
  c_cdd_str_iequal(method, "trace", &eq);
  if (eq) {
    *_out_val = "HTTP_TRACE";
    return CDD_C_SUCCESS;
  }
  c_cdd_str_iequal(method, "query", &eq);
  if (eq) {
    *_out_val = "HTTP_QUERY";
    return CDD_C_SUCCESS;
  }
  c_cdd_str_iequal(method, "connect", &eq);
  if (eq) {
    *_out_val = "HTTP_CONNECT";
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Maps an HTTP status code to the corresponding cdd_c_error_t constant
 * name.
 *
 * @param[in] status Numeric HTTP status code.
 * @param[out] _out_val Pointer to receive the `cdd_c_error_t` constant string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t mapped_err_code(int status, const char **_out_val) {
  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (status == 400) {
    *_out_val = "CDD_C_ERROR_INVALID_ARGUMENT";
    return CDD_C_SUCCESS;
  }
  if (status == 401 || status == 403) {
    *_out_val = "CDD_C_ERROR_SYSTEM"; /* EACCES */
    return CDD_C_SUCCESS;
  }
  if (status == 404) {
    *_out_val = "CDD_C_ERROR_NOT_FOUND";
    return CDD_C_SUCCESS;
  }
  *_out_val = "CDD_C_ERROR_IO"; /* generic */
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the media type.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t find_media_type(const struct OpenAPI_MediaType *mts, size_t n,
                              const char *name,
                              const struct OpenAPI_MediaType **_out_val) {
  size_t i;
  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (!mts || !name) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < n; ++i) {
    if (mts[i].name && strcmp(mts[i].name, name) == 0) {
      *_out_val = &mts[i];
      return CDD_C_SUCCESS;
    }
  }
  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the encoding.
 *
 * @param[in] mt OpenAPI media type containing encoding rules.
 * @param[in] name Property name to look up encoding for.
 * @param[out] _out_val Pointer to receive the matching encoding descriptor.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t find_encoding(const struct OpenAPI_MediaType *mt,
                            const char *name,
                            struct OpenAPI_Encoding **_out_val) {
  size_t i;
  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (!mt || !name || !mt->encoding) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < mt->n_encoding; ++i) {
    if (mt->encoding[i].name && strcmp(mt->encoding[i].name, name) == 0) {
      *_out_val = &mt->encoding[i];
      return CDD_C_SUCCESS;
    }
  }
  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if primitive type.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t is_primitive_type(const char *oa_type, int *out) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_is_primitive_type) {
    g_cdd_fail_is_primitive_type = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  *out = 0;
  if (!oa_type) {
    return CDD_C_SUCCESS;
  }
  *out = (strcmp(oa_type, "string") == 0 || strcmp(oa_type, "integer") == 0 ||
          strcmp(oa_type, "number") == 0 || strcmp(oa_type, "boolean") == 0);
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if object ref type.
 *
 * @param[in] type OpenAPI type or schema reference name.
 * @param[out] out_is_obj Pointer to receive 1 if object reference, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t is_object_ref_type(const char *type, int *out_is_obj) {
  int is_prim = 0;
  cdd_c_error_t rc;
  if (!out_is_obj) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  *out_is_obj = 0;
  if (!type) {
    return CDD_C_SUCCESS;
  }
  rc = is_primitive_type(type, &is_prim);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (is_prim) {
    return CDD_C_SUCCESS;
  }
  if (strcmp(type, "object") == 0 || strcmp(type, "array") == 0 ||
      strcmp(type, "enum") == 0) {
    return CDD_C_SUCCESS;
  }
  *out_is_obj = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks whether all fields in a StructFields container have primitive
 * types.
 *
 * @param[in] sf `StructFields` container to inspect.
 * @param[out] out_all_prim Pointer to receive 1 if all fields are primitive, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t struct_fields_all_primitive(const struct StructFields *sf,
                                          int *out_all_prim) {
  size_t i;
  if (!out_all_prim)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_all_prim = 0;
  if (!sf)
    return CDD_C_SUCCESS;
  for (i = 0; i < sf->size; ++i) {
    int is_prim = 0;
    const char *t = sf->fields[i].type;
    cdd_c_error_t rc = is_primitive_type(t, &is_prim);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!is_prim)
      return CDD_C_SUCCESS;
  }
  *out_all_prim = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks whether an OpenAPI schema reference defines an inline type or
 * enum.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t schema_has_inline(const struct OpenAPI_SchemaRef *schema,
                                int *out) {
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = 0;
  if (!schema)
    return CDD_C_SUCCESS;
  *out = (schema->inline_type != NULL);
  return CDD_C_SUCCESS;
}

/**
 * @brief Computes the length of the essence (type/subtype) portion of a media
 * type string.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t client_body_media_type_base_len(const char *media_type,
                                              size_t *_out_val) {
  size_t i = 0;
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!media_type) {
    *_out_val = 0;
    return CDD_C_SUCCESS;
  }
  while (media_type[i] && media_type[i] != ';')
    ++i;
  *_out_val = i;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks case-insensitively whether a media type starts with a given
 * prefix.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t media_type_has_prefix(const char *media_type, const char *prefix,
                                    int *out) {
  size_t i;
  size_t len = 0;
  size_t pre_len;

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = 0;
  if (!media_type || !prefix)
    return CDD_C_SUCCESS;

  media_type_base_len(media_type, &len);

  pre_len = strlen(prefix);
  if (len < pre_len)
    return CDD_C_SUCCESS;

  for (i = 0; i < pre_len; ++i) {
    char a = media_type[i];
    char b = prefix[i];
    if (a >= 'A' && a <= 'Z')
      a = (char)(a - 'A' + 'a');
    if (b >= 'A' && b <= 'Z')
      b = (char)(b - 'A' + 'a');
    if (a != b)
      return CDD_C_SUCCESS;
  }
  *out = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks case-insensitively whether a media type base ends with a given
 * suffix.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t media_type_has_suffix(const char *media_type, const char *suffix,
                                    int *out) {
  size_t i;
  size_t len = 0;
  size_t suf_len;
  size_t start;

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = 0;
  if (!media_type || !suffix)
    return CDD_C_SUCCESS;

  media_type_base_len(media_type, &len);

  suf_len = strlen(suffix);
  if (len < suf_len)
    return CDD_C_SUCCESS;

  start = len - suf_len;
  for (i = 0; i < suf_len; ++i) {
    char a = media_type[start + i];
    char b = suffix[i];
    if (a >= 'A' && a <= 'Z')
      a = (char)(a - 'A' + 'a');
    if (b >= 'A' && b <= 'Z')
      b = (char)(b - 'A' + 'a');
    if (a != b)
      return CDD_C_SUCCESS;
  }
  *out = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Compares the base essence of a media type case-insensitively against
 * `expected`.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t media_type_ieq(const char *media_type, const char *expected,
                             int *out) {
  size_t i;
  size_t len = 0;
  size_t exp_len;

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = 0;
  if (!media_type || !expected)
    return CDD_C_SUCCESS;

  media_type_base_len(media_type, &len);

  exp_len = strlen(expected);
  if (len != exp_len)
    return CDD_C_SUCCESS;

  for (i = 0; i < len; ++i) {
    char a = media_type[i];
    char b = expected[i];
    if (a >= 'A' && a <= 'Z')
      a = (char)(a - 'A' + 'a');
    if (b >= 'A' && b <= 'Z')
      b = (char)(b - 'A' + 'a');
    if (a != b)
      return CDD_C_SUCCESS;
  }
  *out = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a media type is JSON (`application/json` or `*+json`).
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t media_type_is_json(const char *media_type, int *out) {
  int eq = 0;
  int suf = 0;

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = 0;
  if (!media_type)
    return CDD_C_SUCCESS;

  media_type_ieq(media_type, "application/json", &eq);
  if (eq) {
    *out = 1;
    return CDD_C_SUCCESS;
  }

  media_type_has_suffix(media_type, "+json", &suf);
  *out = suf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a media type is `application/x-www-form-urlencoded`.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t media_type_is_form(const char *media_type, int *out) {
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = 0;
  if (!media_type)
    return CDD_C_SUCCESS;
  return media_type_ieq(media_type, "application/x-www-form-urlencoded", out);
}

/**
 * @brief Checks if a media type is `text/plain`.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t media_type_is_text_plain(const char *media_type, int *out) {
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = 0;
  if (!media_type)
    return CDD_C_SUCCESS;
  return media_type_ieq(media_type, "text/plain", out);
}

/**
 * @brief Checks if a media type starts with `multipart/`.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t media_type_is_multipart(const char *media_type, int *out) {
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = 0;
  if (!media_type)
    return CDD_C_SUCCESS;
  return media_type_has_prefix(media_type, "multipart/", out);
}

/**
 * @brief Checks if a media type is `multipart/form-data`.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t media_type_is_multipart_form(const char *media_type, int *out) {
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = 0;
  if (!media_type)
    return CDD_C_SUCCESS;
  return media_type_ieq(media_type, "multipart/form-data", out);
}

/**
 * @brief Selects the primary media type entry from a Content-Type map.
 *
 * @param[in] content_type Comma-separated Content-Type header string.
 * @param[out] buf Scratch buffer to hold the extracted primary media type.
 * @param[in] buf_sz Capacity of `buf` in bytes.
 * @param[out] _out_val Pointer to receive `buf`.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t first_content_type_entry(const char *content_type, char *buf,
                                       size_t buf_sz, const char **_out_val) {
  size_t i = 0;
  size_t j = 0;
  if (!_out_val || !buf || buf_sz == 0 || !content_type)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  while (content_type[i] && isspace((unsigned char)content_type[i])) {
    ++i;
  }
  for (; content_type[i] && content_type[i] != ','; ++i) {
    if (j + 1 < buf_sz)
      buf[j++] = content_type[i];
  }
  buf[j] = '\0';
  *_out_val = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Sanitizes an arbitrary string into a valid C identifier.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t sanitize_ident(char *out, size_t outsz, const char *in) {
  size_t i = 0;
  size_t j = 0;
  if (!out || outsz == 0 || !in)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  out[0] = '\0';
  for (i = 0; in[i] && j + 1 < outsz; ++i) {
    const unsigned char c = (unsigned char)in[i];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9')) {
      out[j++] = (char)c;
    } else {
      out[j++] = '_';
    }
  }
  out[j] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Builds the C parameter name for a multipart part header.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t multipart_header_param_name(char *out, size_t outsz,
                                          const char *field,
                                          const char *header) {
  char hdr_sanitized[128];
  if (!out || outsz == 0 || !field || !header)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  out[0] = '\0';
  sanitize_ident(hdr_sanitized, sizeof(hdr_sanitized), header);
  CDD_SNPRINTF(out, outsz, "%s_hdr_%s", field, hdr_sanitized);
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks case-insensitively whether a header name is Content-Type.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t header_name_is_content_type(const char *name, int *out) {
  int eq = 0;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = 0;
  if (!name)
    return CDD_C_SUCCESS;
  c_cdd_str_iequal(name, "Content-Type", &eq);
  *out = eq;
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines whether a media type represents textual payload content.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t media_type_is_textual(const char *media_type, int *out) {
  int res = 0;

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = 0;
  if (!media_type)
    return CDD_C_SUCCESS;

  media_type_is_text_plain(media_type, &res);
  if (res) {
    *out = 1;
    return CDD_C_SUCCESS;
  }

  media_type_has_prefix(media_type, "text/", &res);
  if (res) {
    *out = 1;
    return CDD_C_SUCCESS;
  }

  media_type_ieq(media_type, "application/xml", &res);
  if (res) {
    *out = 1;
    return CDD_C_SUCCESS;
  }

  media_type_has_suffix(media_type, "+xml", &res);
  *out = res;
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines whether a media type represents raw binary payload content.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t media_type_is_binary(const char *media_type, int *out) {
  int res = 0;

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = 0;
  if (!media_type)
    return CDD_C_SUCCESS;

  media_type_is_json(media_type, &res);
  if (res)
    return CDD_C_SUCCESS;

  media_type_is_form(media_type, &res);
  if (res)
    return CDD_C_SUCCESS;

  media_type_is_multipart(media_type, &res);
  if (res)
    return CDD_C_SUCCESS;

  media_type_is_textual(media_type, &res);
  if (res)
    return CDD_C_SUCCESS;

  *out = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks whether an OpenAPI schema reference is an inline string type.
 *
 * @param[in] schema OpenAPI schema reference to check.
 * @param[out] out_is_string Pointer to receive 1 if inline non-array string, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t schema_inline_is_string(const struct OpenAPI_SchemaRef *schema,
                                      int *out_is_string) {
  if (!out_is_string)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_string = 0;
  if (!schema || schema->is_array || !schema->inline_type)
    return CDD_C_SUCCESS;
  *out_is_string = (strcmp(schema->inline_type, "string") == 0);
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines whether an OpenAPI response returns a plain text string
 * body.
 *
 * @param[in] resp OpenAPI response descriptor to inspect.
 * @param[out] out_is_textual_string Pointer to receive 1 if textual string
 * body, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t response_is_textual_string(const struct OpenAPI_Response *resp,
                                         int *out_is_textual_string) {
  int is_textual = 0;

  if (!out_is_textual_string)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_textual_string = 0;
  if (!resp || !resp->content_type)
    return CDD_C_SUCCESS;

  media_type_is_textual(resp->content_type, &is_textual);
  if (!is_textual)
    return CDD_C_SUCCESS;

  return schema_inline_is_string(&resp->schema, out_is_textual_string);
}

/**
 * @brief Determines whether an OpenAPI response returns a binary body.
 *
 * @param[in] resp OpenAPI response descriptor to inspect.
 * @param[out] out_is_binary Pointer to receive 1 if binary body, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t response_is_binary(const struct OpenAPI_Response *resp,
                                 int *out_is_binary) {
  if (!out_is_binary)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_binary = 0;
  if (!resp || !resp->content_type)
    return CDD_C_SUCCESS;
  return media_type_is_binary(resp->content_type, out_is_binary);
}

/**
 * @brief Checks whether an OpenAPI schema reference describes a non-empty
 * payload.
 *
 * @param[in] schema OpenAPI schema reference to inspect.
 * @param[out] out_has_payload Pointer to receive 1 if payload exists, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t schema_has_payload(const struct OpenAPI_SchemaRef *schema,
                                 int *out_has_payload) {
  int has_inline = 0;

  if (!out_has_payload)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has_payload = 0;
  if (!schema)
    return CDD_C_SUCCESS;
  if (schema->ref_name) {
    *out_has_payload = 1;
    return CDD_C_SUCCESS;
  }
  schema_has_inline(schema, &has_inline);
  if (has_inline) {
    *out_has_payload = 1;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}
