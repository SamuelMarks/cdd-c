/**
 * @file url.c
 * @brief Implementation of URL generation helpers and media type utilities.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "functions/parse/str.h"
#include "routes/emit/url.h"
#include "win_compat_sym.h"
/* clang-format on */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

/**
 * @brief Checks if a given OpenAPI type string represents a primitive
 * type.
 */
cdd_c_error_t is_primitive_type_url(const char *type) {
  if (!type)
    return CDD_C_SUCCESS;
  return strcmp(type, "integer") == 0 || strcmp(type, "string") == 0 ||
         strcmp(type, "boolean") == 0 || strcmp(type, "number") == 0;
}

/**
 * @brief Determines if an OpenAPI parameter represents an object
 * serialized as key-value pairs.
 */
cdd_c_error_t param_is_object_kv_url(const struct OpenAPI_Parameter *p) {
  if (!p)
    return CDD_C_SUCCESS;
  if (p->is_array)
    return CDD_C_SUCCESS;
  if (p->in != OA_PARAM_IN_QUERY)
    return CDD_C_SUCCESS;
  if (!p->type)
    return CDD_C_SUCCESS;
  return !is_primitive_type_url(p->type);
}

/**
 * @brief Computes the base length of a media type string, ignoring
 * parameters like charset.
 */
cdd_c_error_t media_type_base_len_url(const char *media_type,
                                      size_t *_out_val) {
  size_t i = 0;
  if (!media_type) {
    *_out_val = 0;
    return CDD_C_SUCCESS;
  }
  while (media_type[i] && media_type[i] != ';')
    ++i;
  {
    *_out_val = i;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Performs a case-insensitive comparison of a media type against
 * an expected string.
 */
cdd_c_error_t media_type_ieq_url(const char *media_type, const char *expected) {
  size_t _ast_media_type_base_len_0 = 0;
  size_t i;
  size_t len;
  size_t exp_len;
  if (!media_type || !expected)
    return CDD_C_SUCCESS;
  len = (media_type_base_len_url(media_type, &_ast_media_type_base_len_0),
         _ast_media_type_base_len_0);
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
  return CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief Checks if a given media type string represents JSON.
 */
cdd_c_error_t media_type_is_json_url(const char *media_type) {
  size_t _ast_media_type_base_len_1 = 0;
  size_t len;
  if (!media_type)
    return CDD_C_SUCCESS;
  if (media_type_ieq_url(media_type, "application/json"))
    return CDD_C_ERROR_UNKNOWN;
  len = (media_type_base_len_url(media_type, &_ast_media_type_base_len_1),
         _ast_media_type_base_len_1);
  if (len < 5)
    return CDD_C_SUCCESS;
  {
    const char *suffix = "+json";
    size_t start = len - 5;
    size_t i;
    for (i = 0; i < 5; ++i) {
      char a = media_type[start + i];
      char b = suffix[i];
      if (a >= 'A' && a <= 'Z')
        a = (char)(a - 'A' + 'a');
      /* suffix is constant lowercase, b is never uppercase */
      if (a != b)
        return CDD_C_SUCCESS;
    }
  }
  return CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief Checks if a given media type string represents urlencoded form
 * data.
 */
cdd_c_error_t media_type_is_form_url(const char *media_type) {
  return media_type_ieq_url(media_type, "application/x-www-form-urlencoded");
}

/**
 * @brief Determines if a query parameter represents a form-encoded
 * object.
 */
cdd_c_error_t
querystring_param_is_form_object(const struct OpenAPI_Parameter *p) {
  if (!p)
    return CDD_C_SUCCESS;
  if (p->in != OA_PARAM_IN_QUERYSTRING)
    return CDD_C_SUCCESS;
  if (!media_type_is_form_url(p->content_type))
    return CDD_C_SUCCESS;
  if (p->schema.ref_name) {
    return CDD_C_ERROR_UNKNOWN;
  }
  if (p->schema.inline_type && strcmp(p->schema.inline_type, "object") == 0)
    return CDD_C_ERROR_UNKNOWN;
  if (p->type && strcmp(p->type, "object") == 0)
    return CDD_C_ERROR_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a query parameter schema is a JSON reference.
 */
cdd_c_error_t querystring_param_is_json_ref(const struct OpenAPI_Parameter *p) {
  if (!p)
    return CDD_C_SUCCESS;
  if (p->in != OA_PARAM_IN_QUERYSTRING)
    return CDD_C_SUCCESS;
  if (!media_type_is_json_url(p->content_type))
    return CDD_C_SUCCESS;
  if (p->schema.is_array || (p->type && strcmp(p->type, "array") == 0))
    return CDD_C_SUCCESS;
  return p->schema.ref_name != NULL;
}

/**
 * @brief Retrieves the primitive type of a JSON query parameter.
 */
cdd_c_error_t
querystring_param_json_primitive_type(const struct OpenAPI_Parameter *p,
                                      const char **_out_val) {
  const char *type = NULL;
  if (!p) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (p->in != OA_PARAM_IN_QUERYSTRING) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (!media_type_is_json_url(p->content_type)) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (p->schema.is_array || (p->type && strcmp(p->type, "array") == 0)) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (p->schema.inline_type)
    type = p->schema.inline_type;
  else if (p->type)
    type = p->type;
  if (!type) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (strcmp(type, "string") == 0 || strcmp(type, "integer") == 0 ||
      strcmp(type, "number") == 0 || strcmp(type, "boolean") == 0) {
    *_out_val = type;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Retrieves the primitive type of items within a JSON array query
 * parameter.
 */
cdd_c_error_t
querystring_param_json_array_item_type(const struct OpenAPI_Parameter *p,
                                       const char **_out_val) {
  const char *item_type = NULL;
  if (!p) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (p->in != OA_PARAM_IN_QUERYSTRING) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (!media_type_is_json_url(p->content_type)) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (!(p->schema.is_array || (p->type && strcmp(p->type, "array") == 0) ||
        p->is_array)) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (p->schema.inline_type)
    item_type = p->schema.inline_type;
  else if (p->items_type)
    item_type = p->items_type;
  if (!item_type) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (strcmp(item_type, "string") == 0 || strcmp(item_type, "integer") == 0 ||
      strcmp(item_type, "number") == 0 || strcmp(item_type, "boolean") == 0) {
    *_out_val = item_type;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Retrieves the reference target of items within a JSON array
 * query parameter.
 */
cdd_c_error_t
querystring_param_json_array_item_ref(const struct OpenAPI_Parameter *p,
                                      const char **_out_val) {
  const char *item_type = NULL;
  if (!p) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (p->in != OA_PARAM_IN_QUERYSTRING) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (!media_type_is_json_url(p->content_type)) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (!(p->schema.is_array || (p->type && strcmp(p->type, "array") == 0) ||
        p->is_array)) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (p->schema.inline_type)
    item_type = p->schema.inline_type;
  else if (p->items_type)
    item_type = p->items_type;
  if (!item_type) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (strcmp(item_type, "string") == 0 || strcmp(item_type, "integer") == 0 ||
      strcmp(item_type, "number") == 0 || strcmp(item_type, "boolean") == 0) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (strcmp(item_type, "object") == 0) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = item_type;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Retrieves the raw primitive type of a query parameter.
 */
C_CDD_EXPORT cdd_c_error_t querystring_param_raw_primitive_type(
    const struct OpenAPI_Parameter *p, const char **_out_val) {
  const char *type = NULL;
  if (!p) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (p->in != OA_PARAM_IN_QUERYSTRING) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (!p->content_type) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (media_type_is_json_url(p->content_type)) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (media_type_is_form_url(p->content_type)) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  if (p->schema.inline_type)
    type = p->schema.inline_type;
  else if (p->type)
    type = p->type;
  if (!type) {
    *_out_val = "string";
    return CDD_C_SUCCESS;
  }
  if (strcmp(type, "string") == 0 || strcmp(type, "integer") == 0 ||
      strcmp(type, "number") == 0 || strcmp(type, "boolean") == 0) {
    *_out_val = type;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = "string";
    return CDD_C_SUCCESS;
  }
}
