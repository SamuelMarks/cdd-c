/**
 * @file client_sig_types.c
 * @brief Type mapping and identifier sanitization for client signature
 * generation.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "c_cdd/safe_crt.h"
#include "cdd_c_error.h"
#include "functions/emit/client_sig.h"
#include "functions/emit/client_sig_types.h"
#include "functions/parse/str.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_fail_map_type_to_c_arg;
extern C_CDD_EXPORT int g_cdd_fail_is_primitive_type;
extern C_CDD_EXPORT int g_cdd_fail_map_array_item_type;
extern C_CDD_EXPORT int g_cdd_fail_sanitize_ident;
extern C_CDD_EXPORT int g_cdd_fail_multipart_header_param_name;
extern C_CDD_EXPORT int g_cdd_fail_header_name_is_content_type;
extern C_CDD_EXPORT int g_cdd_fail_c_cdd_str_iequal;
extern C_CDD_EXPORT int g_cdd_fail_map_type_to_c_out;
extern C_CDD_EXPORT int g_cdd_fail_map_array_item_type_out;
extern C_CDD_EXPORT int g_cdd_fail_schema_has_inline;
#endif

/**
 * @brief Maps an OpenAPI type string to a C function argument type.
 */
cdd_c_error_t map_type_to_c_arg(const char *oa_type, const char **_out_val) {
  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_map_type_to_c_arg) {
    g_cdd_fail_map_type_to_c_arg = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!oa_type) {
    *_out_val = "const void *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "integer") == 0) {
    *_out_val = "int ";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "string") == 0) {
    *_out_val = "const char *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "boolean") == 0) {
    *_out_val = "int ";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "number") == 0) {
    *_out_val = "double ";
    return CDD_C_SUCCESS;
  }
  *_out_val = "const void *";
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if an OpenAPI type is a primitive type.
 */
cdd_c_error_t is_primitive_type(const char *oa_type, int *out) {
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_is_primitive_type) {
    g_cdd_fail_is_primitive_type = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!oa_type) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  *out = (strcmp(oa_type, "integer") == 0 || strcmp(oa_type, "string") == 0 ||
          strcmp(oa_type, "boolean") == 0 || strcmp(oa_type, "number") == 0);
  return CDD_C_SUCCESS;
}

/**
 * @brief Maps an OpenAPI array item type to a C pointer type.
 */
cdd_c_error_t map_array_item_type(const char *oa_type, const char **_out_val) {
  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_map_array_item_type) {
    g_cdd_fail_map_array_item_type = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!oa_type) {
    *_out_val = "const void *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "integer") == 0) {
    *_out_val = "const int *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "boolean") == 0) {
    *_out_val = "const int *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "string") == 0) {
    *_out_val = "const char **";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "number") == 0) {
    *_out_val = "const double *";
    return CDD_C_SUCCESS;
  }
  *_out_val = "const void *";
  return CDD_C_SUCCESS;
}

/**
 * @brief Sanitizes an identifier to be a valid C identifier.
 */
cdd_c_error_t sanitize_ident(char *out, size_t outsz, const char *in) {
  size_t i = 0;
  size_t j = 0;
  if (!out || outsz == 0) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_sanitize_ident) {
    g_cdd_fail_sanitize_ident = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  out[0] = '\0';
  if (!in) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
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
  if (j > 0 && isdigit((unsigned char)out[0])) {
    if (j + 1 < outsz) {
      memmove(out + 1, out, j + 1);
      out[0] = '_';
    } else {
      out[0] = '_';
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates a sanitized multipart header parameter name.
 */
cdd_c_error_t multipart_header_param_name(char *out, size_t outsz,
                                          const char *field,
                                          const char *header) {
  char hdr_sanitized[128];
  cdd_c_error_t rc;
  if (!out || outsz == 0 || !field || !header) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_multipart_header_param_name) {
    g_cdd_fail_multipart_header_param_name = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  out[0] = '\0';
  rc = sanitize_ident(hdr_sanitized, sizeof(hdr_sanitized), header);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  CDD_SNPRINTF(out, outsz, "%s_hdr_%s", field, hdr_sanitized);
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a header name is Content-Type (case-insensitive).
 */
cdd_c_error_t header_name_is_content_type(const char *name, int *out) {
  int iequal = 0;
  cdd_c_error_t rc;
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_header_name_is_content_type) {
    g_cdd_fail_header_name_is_content_type = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *out = 0;
  if (!name) {
    return CDD_C_SUCCESS;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_c_cdd_str_iequal) {
    g_cdd_fail_c_cdd_str_iequal = 0;
    rc = CDD_C_ERROR_UNKNOWN;
  } else
#endif
    rc = c_cdd_str_iequal(name, "Content-Type", &iequal);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  *out = iequal;
  return CDD_C_SUCCESS;
}

/**
 * @brief Maps an OpenAPI type to a C output pointer parameter type.
 */
cdd_c_error_t map_type_to_c_out(const char *oa_type, const char **_out_val) {
  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_map_type_to_c_out) {
    g_cdd_fail_map_type_to_c_out = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!oa_type) {
    *_out_val = "void *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "integer") == 0) {
    *_out_val = "int *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "boolean") == 0) {
    *_out_val = "int *";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "string") == 0) {
    *_out_val = "char **";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "number") == 0) {
    *_out_val = "double *";
    return CDD_C_SUCCESS;
  }
  *_out_val = "void *";
  return CDD_C_SUCCESS;
}

/**
 * @brief Maps an OpenAPI array item type to a C array output pointer parameter
 * type.
 */
cdd_c_error_t map_array_item_type_out(const char *oa_type,
                                      const char **_out_val) {
  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_map_array_item_type_out) {
    g_cdd_fail_map_array_item_type_out = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  if (!oa_type) {
    *_out_val = "void **";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "integer") == 0) {
    *_out_val = "int **";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "boolean") == 0) {
    *_out_val = "int **";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "string") == 0) {
    *_out_val = "char ***";
    return CDD_C_SUCCESS;
  }
  if (strcmp(oa_type, "number") == 0) {
    *_out_val = "double **";
    return CDD_C_SUCCESS;
  }
  *_out_val = "void **";
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a schema reference contains an inline type definition.
 */
cdd_c_error_t schema_has_inline(const struct OpenAPI_SchemaRef *schema,
                                int *out) {
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_schema_has_inline) {
    if (--g_cdd_fail_schema_has_inline == 0) {
      return CDD_C_ERROR_UNKNOWN;
    }
  }
#endif
  if (!schema) {
    *out = 0;
    return CDD_C_SUCCESS;
  }
  *out = (schema->inline_type != NULL);
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
/**
 * @brief Test helper to map OpenAPI type to C argument string.
 */
cdd_c_error_t cdd_test_sig_map_type_to_c_arg(const char *oa_type,
                                             const char **out_val) {
  return map_type_to_c_arg(oa_type, out_val);
}

/**
 * @brief Test helper to check if type is primitive.
 */
cdd_c_error_t cdd_test_sig_is_primitive_type(const char *oa_type, int *out) {
  return is_primitive_type(oa_type, out);
}

/**
 * @brief Test helper to map OpenAPI array item type to C type.
 */
cdd_c_error_t cdd_test_sig_map_array_item_type(const char *oa_type,
                                               const char **out_val) {
  return map_array_item_type(oa_type, out_val);
}

/**
 * @brief Test helper to sanitize an identifier.
 */
cdd_c_error_t cdd_test_sig_sanitize_ident(char *out, size_t outsz,
                                          const char *in) {
  return sanitize_ident(out, outsz, in);
}

/**
 * @brief Test helper to construct multipart header param name.
 */
cdd_c_error_t cdd_test_sig_multipart_header_param_name(char *out, size_t outsz,
                                                       const char *field,
                                                       const char *header) {
  return multipart_header_param_name(out, outsz, field, header);
}

/**
 * @brief Test helper to check if header is Content-Type.
 */
cdd_c_error_t cdd_test_sig_header_name_is_content_type(const char *name,
                                                       int *out) {
  return header_name_is_content_type(name, out);
}

/**
 * @brief Test helper to map OpenAPI type to C output parameter type.
 */
cdd_c_error_t cdd_test_sig_map_type_to_c_out(const char *oa_type,
                                             const char **out_val) {
  return map_type_to_c_out(oa_type, out_val);
}

/**
 * @brief Test helper to map OpenAPI array item type to C output parameter type.
 */
cdd_c_error_t cdd_test_sig_map_array_item_type_out(const char *oa_type,
                                                   const char **out_val) {
  return map_array_item_type_out(oa_type, out_val);
}

/**
 * @brief Test helper to check if schema has inline type.
 */
cdd_c_error_t
cdd_test_sig_schema_has_inline(const struct OpenAPI_SchemaRef *schema,
                               int *out) {
  return schema_has_inline(schema, out);
}
#endif
