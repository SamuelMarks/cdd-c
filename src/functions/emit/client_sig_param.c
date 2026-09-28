/**
 * @file client_sig_param.c
 * @brief Parameter and query string inspection for client signature
 * generation.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <stddef.h>
#include <string.h>

#include "cdd_c_error.h"
#include "functions/emit/client_sig.h"
#include "functions/emit/client_sig_media.h"
#include "functions/emit/client_sig_param.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_fail_param_is_object_kv;
extern C_CDD_EXPORT int g_cdd_fail_qs_form_obj;
extern C_CDD_EXPORT int g_cdd_fail_qs_json_ref;
extern C_CDD_EXPORT int g_cdd_fail_qs_json_prim;
extern C_CDD_EXPORT int g_cdd_fail_qs_json_array_item_type;
extern C_CDD_EXPORT int g_cdd_fail_qs_json_array_item_ref;
extern C_CDD_EXPORT int g_cdd_fail_qs_raw;
#endif

/**
 * @brief Checks if a parameter represents an object Key-Value structure.
 */
cdd_c_error_t param_is_object_kv(const struct OpenAPI_Parameter *p, int *out) {
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_param_is_object_kv) {
    g_cdd_fail_param_is_object_kv = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!p) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  *out = 0;
  if (!p->is_array && p->type && strcmp(p->type, "object") == 0) {
    *out = (p->in == OA_PARAM_IN_QUERY || p->in == OA_PARAM_IN_PATH ||
            p->in == OA_PARAM_IN_HEADER || p->in == OA_PARAM_IN_COOKIE);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a querystring parameter is a form object for client
 * signatures.
 */
cdd_c_error_t
sig_querystring_param_is_form_object(const struct OpenAPI_Parameter *p,
                                     int *out) {
  int is_json = 0;
  cdd_c_error_t rc;
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_qs_form_obj) {
    g_cdd_fail_qs_form_obj = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!p) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  if (p->content_type) {
    rc = media_type_is_json(p->content_type, &is_json);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    if (is_json) {
      *out = 0;
      return CDD_C_SUCCESS;
    }
  }
  *out = (p->schema.ref_name != NULL);
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a querystring parameter is a JSON schema reference for
 * client signatures.
 */
cdd_c_error_t
sig_querystring_param_is_json_ref(const struct OpenAPI_Parameter *p, int *out) {
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_qs_json_ref) {
    g_cdd_fail_qs_json_ref = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!p) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  if (p->schema.is_array || (p->type && strcmp(p->type, "array") == 0)) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  *out = (p->schema.ref_name != NULL);
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines the JSON primitive type of a querystring parameter for
 * client signatures.
 */
cdd_c_error_t
sig_querystring_param_json_primitive_type(const struct OpenAPI_Parameter *p,
                                          const char **_out_val) {
  const char *type = NULL;
  int is_json = 0;
  cdd_c_error_t rc;

  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_qs_json_prim) {
    g_cdd_fail_qs_json_prim = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!p) {
    return CDD_C_SUCCESS;
  }

  if (p->in != OA_PARAM_IN_QUERYSTRING) {
    return CDD_C_SUCCESS;
  }

  rc = media_type_is_json(p->content_type, &is_json);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (!is_json) {
    return CDD_C_SUCCESS;
  }

  if (p->schema.is_array || (p->type && strcmp(p->type, "array") == 0)) {
    return CDD_C_SUCCESS;
  }

  if (p->schema.inline_type) {
    type = p->schema.inline_type;
  } else if (p->type) {
    type = p->type;
  }

  if (!type) {
    return CDD_C_SUCCESS;
  }

  if (strcmp(type, "string") == 0 || strcmp(type, "integer") == 0 ||
      strcmp(type, "number") == 0 || strcmp(type, "boolean") == 0) {
    *_out_val = type;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines the JSON array item type of a querystring parameter for
 * client signatures.
 */
cdd_c_error_t
sig_querystring_param_json_array_item_type(const struct OpenAPI_Parameter *p,
                                           const char **_out_val) {
  const char *item_type = NULL;
  int is_json = 0;
  cdd_c_error_t rc;

  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_qs_json_array_item_type) {
    g_cdd_fail_qs_json_array_item_type = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!p) {
    return CDD_C_SUCCESS;
  }

  if (p->in != OA_PARAM_IN_QUERYSTRING) {
    return CDD_C_SUCCESS;
  }

  rc = media_type_is_json(p->content_type, &is_json);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (!is_json) {
    return CDD_C_SUCCESS;
  }

  if (!(p->schema.is_array || (p->type && strcmp(p->type, "array") == 0) ||
        p->is_array)) {
    return CDD_C_SUCCESS;
  }

  if (p->schema.inline_type) {
    item_type = p->schema.inline_type;
  } else if (p->items_type) {
    item_type = p->items_type;
  }

  if (!item_type) {
    return CDD_C_SUCCESS;
  }

  if (strcmp(item_type, "string") == 0 || strcmp(item_type, "integer") == 0 ||
      strcmp(item_type, "number") == 0 || strcmp(item_type, "boolean") == 0) {
    *_out_val = item_type;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines the JSON array item reference of a querystring parameter
 * for client signatures.
 */
cdd_c_error_t
sig_querystring_param_json_array_item_ref(const struct OpenAPI_Parameter *p,
                                          const char **_out_val) {
  const char *item_type = NULL;
  int is_json = 0;
  cdd_c_error_t rc;

  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_qs_json_array_item_ref) {
    g_cdd_fail_qs_json_array_item_ref = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!p) {
    return CDD_C_SUCCESS;
  }

  if (p->in != OA_PARAM_IN_QUERYSTRING) {
    return CDD_C_SUCCESS;
  }

  rc = media_type_is_json(p->content_type, &is_json);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (!is_json) {
    return CDD_C_SUCCESS;
  }

  if (!(p->schema.is_array || (p->type && strcmp(p->type, "array") == 0) ||
        p->is_array)) {
    return CDD_C_SUCCESS;
  }

  if (p->schema.inline_type) {
    item_type = p->schema.inline_type;
  } else if (p->items_type) {
    item_type = p->items_type;
  }

  if (!item_type) {
    return CDD_C_SUCCESS;
  }

  if (strcmp(item_type, "string") == 0 || strcmp(item_type, "integer") == 0 ||
      strcmp(item_type, "number") == 0 || strcmp(item_type, "boolean") == 0) {
    return CDD_C_SUCCESS;
  }

  if (strcmp(item_type, "object") == 0) {
    return CDD_C_SUCCESS;
  }

  *_out_val = item_type;
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines raw primitive type for non-JSON/form querystring
 * parameters for client signatures.
 */
cdd_c_error_t
sig_querystring_param_raw_primitive_type(const struct OpenAPI_Parameter *p,
                                         const char **_out_val) {
  const char *type = NULL;
  int is_json = 0;
  int is_form = 0;
  cdd_c_error_t rc;

  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_qs_raw) {
    g_cdd_fail_qs_raw = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!p) {
    return CDD_C_SUCCESS;
  }

  if (p->in != OA_PARAM_IN_QUERYSTRING || !p->content_type) {
    return CDD_C_SUCCESS;
  }

  rc = media_type_is_json(p->content_type, &is_json);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (is_json) {
    return CDD_C_SUCCESS;
  }

  rc = media_type_is_form(p->content_type, &is_form);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (is_form) {
    return CDD_C_SUCCESS;
  }

  if (p->schema.inline_type) {
    type = p->schema.inline_type;
  } else if (p->type) {
    type = p->type;
  }

  if (!type) {
    *_out_val = "string";
    return CDD_C_SUCCESS;
  }
  if (strcmp(type, "string") == 0 || strcmp(type, "integer") == 0 ||
      strcmp(type, "number") == 0 || strcmp(type, "boolean") == 0) {
    *_out_val = type;
    return CDD_C_SUCCESS;
  }
  *_out_val = "string";
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
/**
 * @brief Test helper to check if param is object KV.
 */
cdd_c_error_t cdd_test_sig_param_is_object_kv(const struct OpenAPI_Parameter *p,
                                              int *out) {
  return param_is_object_kv(p, out);
}

/**
 * @brief Test helper to check if querystring param is form object.
 */
cdd_c_error_t
cdd_test_sig_querystring_param_is_form_object(const struct OpenAPI_Parameter *p,
                                              int *out) {
  return sig_querystring_param_is_form_object(p, out);
}

/**
 * @brief Test helper to check if querystring param is JSON ref.
 */
cdd_c_error_t
cdd_test_sig_querystring_param_is_json_ref(const struct OpenAPI_Parameter *p,
                                           int *out) {
  return sig_querystring_param_is_json_ref(p, out);
}

/**
 * @brief Test helper to determine querystring param JSON primitive type.
 */
cdd_c_error_t cdd_test_sig_querystring_param_json_primitive_type(
    const struct OpenAPI_Parameter *p, const char **out_val) {
  return sig_querystring_param_json_primitive_type(p, out_val);
}

/**
 * @brief Test helper to determine querystring param JSON array item type.
 */
cdd_c_error_t cdd_test_sig_querystring_param_json_array_item_type(
    const struct OpenAPI_Parameter *p, const char **out_val) {
  return sig_querystring_param_json_array_item_type(p, out_val);
}

/**
 * @brief Test helper to determine querystring param JSON array item ref.
 */
cdd_c_error_t cdd_test_sig_querystring_param_json_array_item_ref(
    const struct OpenAPI_Parameter *p, const char **out_val) {
  return sig_querystring_param_json_array_item_ref(p, out_val);
}

/**
 * @brief Test helper to determine querystring param raw primitive type.
 */
cdd_c_error_t cdd_test_sig_querystring_param_raw_primitive_type(
    const struct OpenAPI_Parameter *p, const char **out_val) {
  return sig_querystring_param_raw_primitive_type(p, out_val);
}
#endif
