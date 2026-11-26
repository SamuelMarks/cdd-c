/**
 * @file mapping.c
 * @brief Implementation of C to OpenAPI type mapping.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "classes/parse/mapping.h"
#include "functions/parse/str.h"
/* clang-format on */

/*
 * @brief Initializes a C-to-OpenAPI type mapping table.
 *
 * @param[out] out Pointer to the OpenApiTypeMapping structure to initialize.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c_mapping_init(struct OpenApiTypeMapping *out) {
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_mapping_fail_init;
  if (g_mapping_fail_init) {
    g_mapping_fail_init = 0;
    return CDD_C_ERROR_MEMORY;
  }
#endif
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  memset(out, 0, sizeof(*out));
  out->kind = OA_TYPE_UNKNOWN;
  return CDD_C_SUCCESS;
}

/*
 * @brief Releases memory held by a C-to-OpenAPI type mapping table.
 *
 * @param[in,out] out Pointer to the OpenApiTypeMapping structure to free.
 * @return CDD_C_SUCCESS.
 */
cdd_c_error_t c_mapping_free(struct OpenApiTypeMapping *out) {
  if (out) {
    if (out->oa_type)
      C_CDD_FREE(out->oa_type);
    if (out->oa_format)
      C_CDD_FREE(out->oa_format);
    if (out->ref_name)
      C_CDD_FREE(out->ref_name);
    memset(out, 0, sizeof(*out));
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets primitive (promoting `kind` to `OA_TYPE_ARRAY` when
 * `is_array` is non-zero).
 *
 * @param[out] out Target `OpenApiTypeMapping` structure.
 * @param[in] type OpenAPI primitive type string.
 * @param[in] fmt Optional OpenAPI format string.
 * @param[in] is_array Non-zero if declarator is an array.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t set_primitive(struct OpenApiTypeMapping *out,
                                   const char *type, const char *fmt,
                                   int is_array) {
  cdd_c_error_t rc_str;
  out->kind = is_array ? OA_TYPE_ARRAY : OA_TYPE_PRIMITIVE;
  rc_str = c_cdd_strdup(type, &out->oa_type);
  if (rc_str != CDD_C_SUCCESS)
    return rc_str;
  if (fmt) {
    rc_str = c_cdd_strdup(fmt, &out->oa_format);
    if (rc_str != CDD_C_SUCCESS) {
      C_CDD_FREE(out->oa_type);
      out->oa_type = NULL;
      return rc_str;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets ref (promoting `kind` to `OA_TYPE_ARRAY` when `is_array`
 * is non-zero).
 *
 * @param[out] out Target `OpenApiTypeMapping` structure.
 * @param[in] ref Schema reference name.
 * @param[in] is_array Non-zero if declarator is an array.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t set_ref(struct OpenApiTypeMapping *out, const char *ref,
                             int is_array) {
  cdd_c_error_t rc = c_cdd_strdup(ref, &out->ref_name);
  if (rc != CDD_C_SUCCESS)
    return rc;
  out->kind = is_array ? OA_TYPE_ARRAY : OA_TYPE_OBJECT;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees a heap buffer `p` and returns `err`.
 *
 * @param[in,out] p Heap buffer to free.
 * @param[in] err Error code to return.
 * @return `err`.
 */
static cdd_c_error_t mapping_free_and_return(void *p, cdd_c_error_t err) {
  C_CDD_FREE(p);
  return err;
}

/**
 * @brief Strips leading C type qualifiers (`const`, `volatile`) and whitespace.
 *
 * @param[in] type Input C type string.
 * @param[out] _out_val Pointer to receive the unqualified substring pointer.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t skip_qualifiers(const char *type, const char **_out_val) {
  const char *p;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_skip_qualifiers;
  if (g_cdd_fail_skip_qualifiers && --g_cdd_fail_skip_qualifiers == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!type || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  p = type;
  while (*p) {
    while (isspace((unsigned char)*p))
      p++;
    if (strncmp(p, "const", 5) == 0 && isspace((unsigned char)p[5])) {
      p += 6;
      continue;
    }
    if (strncmp(p, "volatile", 8) == 0 && isspace((unsigned char)p[8])) {
      p += 9;
      continue;
    }
    break;
  }
  *_out_val = p;
  return CDD_C_SUCCESS;
}

/**
 * @brief Normalizes whitespace and qualifiers in a C type string.
 *
 * @param[in] in Input C type string.
 * @param[out] _out_val Pointer to receive the newly allocated cleaned type
 * string.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t clean_type_str(const char *in, char **_out_val) {
  char *p;
  char *buf = NULL;
  cdd_c_error_t rc;

  if (!in || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Remove pointer asterisk */
  rc = c_cdd_strdup(in, &buf);
  if (rc != CDD_C_SUCCESS)
    return rc;

  p = strchr(buf, '*');
  if (p)
    *p = '\0';

  rc = c_cdd_str_trim_trailing_whitespace(buf);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(buf);
    return rc;
  }
  *_out_val = buf;
  return CDD_C_SUCCESS;
}

/*
 * @brief Maps a C type declaration string to its corresponding OpenAPI type and
 * format.
 *
 * @param[in] c_type_in Raw C type specifier string.
 * @param[in] decl_name Optional declarator name (used for array `[...]`
 * detection).
 * @param[out] out Output OpenApiTypeMapping structure receiving the mapped
 * type.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c_mapping_map_type(const char *c_type_in, const char *decl_name,
                                 struct OpenApiTypeMapping *out) {
  char *clean = NULL;
  const char *c_type = NULL;
  int is_ptr = 0;
  int is_array = 0;
  cdd_c_error_t rc;

  if (!c_type_in)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = c_mapping_init(out);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = skip_qualifiers(c_type_in, &c_type);
  if (rc != CDD_C_SUCCESS)
    return rc;

  /* Pointer/Array detection works on raw type */
  if (strchr(c_type, '*'))
    is_ptr = 1;
  if (decl_name && strchr(decl_name, '['))
    is_array = 1;

  /* Check for templates */
  if (strchr(c_type, '<') && strchr(c_type, '>')) {
    /* Keep the exact type as object for templates so FFI extractor can parse it
     */
    return set_ref(out, c_type, 0);
  }

  /* Check for char* or char[] -> string */
  if (strstr(c_type, "char") && (is_ptr || is_array)) {
    /* Special case: strings are primitives in OpenAPI */
    return set_primitive(out, "string", NULL, 0);
  }

  /* Primitives */
  if (strstr(c_type, "int")) {
    if (strstr(c_type, "long")) {
      return set_primitive(out, "integer", "int64", is_array);
    }
    if (strstr(c_type, "short")) {
      return set_primitive(out, "integer", NULL, is_array);
    }
    return set_primitive(out, "integer", "int32", is_array);
  }
  if (strstr(c_type, "long") || strstr(c_type, "short")) {
    /* Handle long/short without "int" (e.g. "unsigned short") */
    if (strstr(c_type, "long")) {
      return set_primitive(out, "integer", "int64", is_array);
    }
    return set_primitive(out, "integer", NULL, is_array);
  }
  if (strstr(c_type, "float")) {
    return set_primitive(out, "number", "float", is_array);
  }
  if (strstr(c_type, "double")) {
    return set_primitive(out, "number", "double", is_array);
  }
  if (strstr(c_type, "bool") || strstr(c_type, "_Bool")) {
    return set_primitive(out, "boolean", NULL, is_array);
  }
  if (strstr(c_type, "size_t")) {
    return set_primitive(out, "integer", "int64", is_array);
  }
  if (strstr(c_type, "void")) {
    if (is_ptr) {
      return set_primitive(out, "string", "binary", is_array);
    }
    out->kind = is_array ? OA_TYPE_ARRAY : OA_TYPE_UNKNOWN;
    return CDD_C_SUCCESS;
  }

  /* Structs / Enums */
  {
    int starts1 = 0, starts2 = 0;
    rc = c_cdd_str_starts_with(c_type, "struct ", &starts1);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = c_cdd_str_starts_with(c_type, "enum ", &starts2);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (starts1 || starts2) {
      const char *start;
      rc = clean_type_str(c_type, &clean);
      if (rc != CDD_C_SUCCESS)
        return rc;

      start = clean;
      if (strncmp(start, "struct ", 7) == 0)
        start += 7;
      else
        start += 5;

      while (isspace((unsigned char)*start))
        start++;

      return mapping_free_and_return(clean, set_ref(out, start, is_array));
    }
  }

  if (strlen(c_type) == 1) {
    return set_ref(out, c_type, is_array);
  }
  return set_primitive(out, "string", NULL, is_array);
}

/**
 * @brief Internal self-test for mapping helper NULL-argument error paths.
 *
 * @return CDD_C_SUCCESS if all internal error checks succeed.
 */
C_CDD_EXPORT cdd_c_error_t test_mapping_internal_errors(void);

/**
 * @brief Internal self-test for mapping helper NULL-argument error paths.
 *
 * @return CDD_C_SUCCESS if all internal error checks succeed.
 */
C_CDD_EXPORT cdd_c_error_t test_mapping_internal_errors(void) {
  const char *out_str = NULL;
  char *out_buf = NULL;
  cdd_c_error_t err1 = skip_qualifiers(NULL, &out_str);
  cdd_c_error_t err2 = skip_qualifiers("int", NULL);
  cdd_c_error_t err3 = clean_type_str(NULL, &out_buf);
  cdd_c_error_t err4 = clean_type_str("int", NULL);
  return (cdd_c_error_t)((err1 ^ CDD_C_ERROR_INVALID_ARGUMENT) |
                         (err2 ^ CDD_C_ERROR_INVALID_ARGUMENT) |
                         (err3 ^ CDD_C_ERROR_INVALID_ARGUMENT) |
                         (err4 ^ CDD_C_ERROR_INVALID_ARGUMENT));
}
