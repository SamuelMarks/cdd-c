/**
 * @file mapping.c
 * @brief Implementation of C to OpenAPI type mapping.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "classes/parse/mapping.h"
#include "functions/parse/str.h"
#include "c_cdd/log.h"
/* clang-format on */

/**
 * @brief Executes the c mapping init operation.
 */
void c_mapping_init(struct OpenApiTypeMapping *out) {
  if (out) {
    memset(out, 0, sizeof(*out));
    out->kind = OA_TYPE_UNKNOWN;
  }
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
static enum cdd_c_error set_primitive(struct OpenApiTypeMapping *out,
                                      const char *type, const char *fmt) {
  out->kind = OA_TYPE_PRIMITIVE;
  c_cdd_strdup(type, &out->oa_type);
  if (!out->oa_type) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  if (fmt) {
    c_cdd_strdup(fmt, &out->oa_format);
    if (!out->oa_format) {
      free(out->oa_type);
      out->oa_type = NULL;
      return CDD_C_ERROR_MEMORY;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets ref.
 */
static enum cdd_c_error set_ref(struct OpenApiTypeMapping *out,
                                const char *ref) {
  out->kind = OA_TYPE_OBJECT;
  /* OpenAPI usually doesn't put "type": "object" alongside $ref,
     but for internal mapping representation we mark it.
     The ref_name holds the target. */
  c_cdd_strdup(ref, &out->ref_name);
  if (!out->ref_name) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  return CDD_C_SUCCESS;
}

/* Strip qualifiers like const, volatile, struct, enum */
static enum cdd_c_error skip_qualifiers(const char *type,
                                        const char **_out_val) {
  const char *p = type;
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
    /* struct/enum are handled separately in logic, but signed/unsigned are
     * qualifiers for primitives */
    /* Note: if we strip 'unsigned', 'unsigned int' becomes 'int' which maps to
       int32. Use care. Here we only strip CV qualifiers to find the core type
       start. */
    break;
  }
  {
    *_out_val = p;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the clean type str operation.
 */
static enum cdd_c_error clean_type_str(const char *in, char **_out_val) {
  char *p;
  char *buf = NULL;
  int rc;

  /* Remove pointer asterisk */
  rc = c_cdd_strdup(in, &buf);
  if (rc != 0)
    return rc;

  p = strchr(buf, '*');
  if (p)
    *p = '\0';

  c_cdd_str_trim_trailing_whitespace(buf);
  {
    *_out_val = buf;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the c mapping map type operation.
 */
enum cdd_c_error c_mapping_map_type(const char *c_type_in,
                                    const char *decl_name,
                                    struct OpenApiTypeMapping *out) {
  char *clean = NULL;
  const char *c_type = NULL;
  int is_ptr = 0;
  int is_array = 0;
  int rc = 0;
  skip_qualifiers(c_type_in, &c_type);
  printf("DEBUG c_mapping_map_type: c_type_in='%s', c_type='%s'\n", c_type_in,
         c_type);

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  c_mapping_init(out);

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
    c_cdd_str_starts_with(c_type, "struct ", &starts1);
    c_cdd_str_starts_with(c_type, "enum ", &starts2);
    if (starts1 || starts2) {
      clean = NULL;
      rc = clean_type_str(c_type, &clean);
      if (rc != 0) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        return CDD_C_ERROR_MEMORY;
      }

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
      if (strlen(c_type) == 1 || (strchr(c_type, '<') && strchr(c_type, '>'))) {
        rc = set_ref(out, c_type);
      } else {
        rc = set_primitive(out, "string", NULL);
      }
    }
  }

  if (rc != 0)
    return rc;

  /* Handle Arrays (Pointer to POD/Obj, but not char*) */
  if (is_array) {
    char *inner_ref = NULL;
    char *inner_type = NULL;
    /* It's an array of the resolved type */
    if (out->ref_name) {
      rc = c_cdd_strdup(out->ref_name, &inner_ref);
      if (rc != 0)
        return rc;
    }
    if (out->oa_type) {
      rc = c_cdd_strdup(out->oa_type, &inner_type);
      if (rc != 0)
        return rc;
    }
    /* Move current mapping to "items" logic? */
    /* The OpenApiTypeMapping struct is flat. We need to indicate it's an
       Array OF X. */
    /* We change kind to ARRAY. We reuse ref_name/oa_type to mean the ITEMS
     * type.
     */
    out->kind = OA_TYPE_ARRAY;
    /* Ensure we kept the item type info. It's already in oa_type/ref_name. */
    (void)inner_ref;
    (void)inner_type;
    if (inner_ref)
      free(inner_ref);
    if (inner_type)
      free(inner_type);
  }

  return CDD_C_SUCCESS;
}
