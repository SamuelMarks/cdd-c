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
#include "classes/parse/mapping.h"
#include "functions/parse/str.h"
/* clang-format on */

/**
 * @brief Executes the c mapping init operation.
 */
cdd_c_error_t c_mapping_init(struct OpenApiTypeMapping *out) {
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  memset(out, 0, sizeof(*out));
  out->kind = OA_TYPE_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the c mapping free operation.
 */
void c_mapping_free(struct OpenApiTypeMapping *out) {
  if (out) {
    if (out->oa_type)
      free(out->oa_type);
    if (out->oa_format)
      free(out->oa_format);
    if (out->ref_name)
      free(out->ref_name);
    memset(out, 0, sizeof(*out));
  }
}

/**
 * @brief Adds or sets primitive.
 */
static cdd_c_error_t set_primitive(struct OpenApiTypeMapping *out,
                                   const char *type, const char *fmt) {
  cdd_c_error_t rc_str;
  out->kind = OA_TYPE_PRIMITIVE;
  rc_str = c_cdd_strdup(type, &out->oa_type);
  if (rc_str != CDD_C_SUCCESS)
    return rc_str;
  if (fmt) {
    rc_str = c_cdd_strdup(fmt, &out->oa_format);
    if (rc_str != CDD_C_SUCCESS) {
      free(out->oa_type);
      out->oa_type = NULL;
      return rc_str;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets ref.
 */
static cdd_c_error_t set_ref(struct OpenApiTypeMapping *out, const char *ref) {
  out->kind = OA_TYPE_OBJECT;
  return c_cdd_strdup(ref, &out->ref_name);
}

/* Strip qualifiers like const, volatile, struct, enum */
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
 * @brief Executes the clean type str operation.
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

  c_cdd_str_trim_trailing_whitespace(buf);
  *_out_val = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the c mapping map type operation.
 */
cdd_c_error_t c_mapping_map_type(const char *c_type_in, const char *decl_name,
                                 struct OpenApiTypeMapping *out) {
  char *clean = NULL;
  const char *c_type = NULL;
  int is_ptr = 0;
  int is_array = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!out || !c_type_in)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = skip_qualifiers(c_type_in, &c_type);
  if (rc != CDD_C_SUCCESS)
    return rc;

  (void)c_mapping_init(out);

  /* Pointer/Array detection works on raw type */
  if (strchr(c_type, '*'))
    is_ptr = 1;
  if (decl_name && strchr(decl_name, '['))
    is_array = 1;

  /* Check for templates */
  if (strchr(c_type, '<') && strchr(c_type, '>')) {
    /* Keep the exact type as object for templates so FFI extractor can parse it
     */
    return set_ref(out, c_type);
  }

  /* Check for char* or char[] -> string */
  if (strstr(c_type, "char") && (is_ptr || is_array)) {
    /* Special case: strings are primitives in OpenAPI */
    return set_primitive(out, "string", NULL);
  }

  /* Primitives */
  if (strstr(c_type, "int")) {
    if (strstr(c_type, "long"))
      rc = set_primitive(out, "integer", "int64");
    else if (strstr(c_type, "short"))
      rc = set_primitive(out, "integer", NULL);
    else
      rc = set_primitive(out, "integer", "int32");
  } else if (strstr(c_type, "long") || strstr(c_type, "short")) {
    /* Handle long/short without "int" (e.g. "unsigned short") */
    if (strstr(c_type, "long"))
      rc = set_primitive(out, "integer", "int64");
    else
      rc = set_primitive(out, "integer", NULL);
  } else if (strstr(c_type, "float")) {
    rc = set_primitive(out, "number", "float");
  } else if (strstr(c_type, "double")) {
    rc = set_primitive(out, "number", "double");
  } else if (strstr(c_type, "bool") || strstr(c_type, "_Bool")) {
    rc = set_primitive(out, "boolean", NULL);
  } else if (strstr(c_type, "size_t")) {
    rc = set_primitive(out, "integer", "int64"); /* Usually safe assumption */
  } else if (strstr(c_type, "void")) {
    /* void * usually maps to something generic or string, but void return is
     * null */
    /* If pointer: binary string? */
    if (is_ptr)
      rc = set_primitive(out, "string", "binary");
    else
      out->kind = OA_TYPE_UNKNOWN;
  }
  /* Structs / Enums */
  else {
    int starts1 = false, starts2 = false;
    rc = c_cdd_str_starts_with(c_type, "struct ", &starts1);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = c_cdd_str_starts_with(c_type, "enum ", &starts2);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (starts1 || starts2) {
      clean = NULL;
      rc = clean_type_str(c_type, &clean);
      if (rc != CDD_C_SUCCESS)
        return rc;

      /* Skip "struct " (7 chars) or "enum " (5 chars) */
      {
        const char *start = clean;
        if (strncmp(start, "struct ", 7) == 0)
          start += 7;
        else
          start += 5;

        while (isspace((unsigned char)*start))
          start++; /* Trim logic */

        rc = set_ref(out, start);
      }
      free(clean);
    } else {
      /* Fallback: Unknown type (defaults to string usually in schemas?) */
      /* Keep generic T as object for templates */
      if (strlen(c_type) == 1) {
        rc = set_ref(out, c_type);
      } else {
        rc = set_primitive(out, "string", NULL);
      }
    }
  }

  if (rc != CDD_C_SUCCESS)
    return rc;

  /* Handle Arrays (Pointer to POD/Obj, but not char*) */
  if (is_array) {
    out->kind = OA_TYPE_ARRAY;
  }

  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT cdd_c_error_t test_mapping_internal_errors(void);
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
#endif
