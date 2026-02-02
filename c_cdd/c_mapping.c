/**
 * @file c_mapping.c
 * @brief Implementation of C to OpenAPI type mapping.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_mapping.h"
#include "str_utils.h"

void c_mapping_init(struct OpenApiTypeMapping *const out) {
  if (out) {
    memset(out, 0, sizeof(*out));
    out->kind = OA_TYPE_UNKNOWN;
  }
}

void c_mapping_free(struct OpenApiTypeMapping *const out) {
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

static int set_primitive(struct OpenApiTypeMapping *out, const char *type,
                         const char *fmt) {
  out->kind = OA_TYPE_PRIMITIVE;
  out->oa_type = c_cdd_strdup(type);
  if (!out->oa_type)
    return ENOMEM;
  if (fmt) {
    out->oa_format = c_cdd_strdup(fmt);
    if (!out->oa_format) {
      free(out->oa_type);
      out->oa_type = NULL;
      return ENOMEM;
    }
  }
  return 0;
}

static int set_ref(struct OpenApiTypeMapping *out, const char *ref) {
  out->kind = OA_TYPE_OBJECT;
  /* OpenAPI usually doesn't put "type": "object" alongside $ref,
     but for internal mapping representation we mark it.
     The ref_name holds the target. */
  out->ref_name = c_cdd_strdup(ref);
  if (!out->ref_name)
    return ENOMEM;
  return 0;
}

/* Strip qualifiers like const, volatile, struct, enum */
static const char *skip_qualifiers(const char *type) {
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
  return p;
}

static char *clean_type_str(const char *in) {
  char *buf = c_cdd_strdup(in);
  char *p;
  if (!buf)
    return NULL;

  /* Remove pointer asterisk */
  p = strchr(buf, '*');
  if (p)
    *p = '\0';

  c_cdd_str_trim_trailing_whitespace(buf);
  return buf;
}

int c_mapping_map_type(const char *const c_type_in, const char *const decl_name,
                       struct OpenApiTypeMapping *const out) {
  char *clean = NULL;
  const char *c_type = skip_qualifiers(c_type_in);
  int is_ptr = 0;
  int is_array = 0;
  int rc = 0;

  if (!c_type_in || !out)
    return EINVAL;

  c_mapping_init(out);

  /* Pointer/Array detection works on raw type */
  if (strchr(c_type, '*'))
    is_ptr = 1;
  if (decl_name && strchr(decl_name, '['))
    is_array = 1;

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
  else if (c_cdd_str_starts_with(c_type, "struct ") ||
           c_cdd_str_starts_with(c_type, "enum ")) {
    clean = clean_type_str(c_type);
    if (!clean)
      return ENOMEM;

    /* Skip "struct " (7 chars) or "enum " (5 chars) */
    {
      const char *start = clean;
      if (strncmp(start, "struct ", 7) == 0)
        start += 7;
      else if (strncmp(start, "enum ", 5) == 0)
        start += 5;

      while (*start && isspace((unsigned char)*start))
        start++; /* Trim logic */

      rc = set_ref(out, start);
    }
    free(clean);
  } else {
    /* Fallback: Unknown type (defaults to string usually in schemas?) */
    /* Let's set primitive string for safety in generation unless unknown */
    rc = set_primitive(out, "string", NULL);
  }

  if (rc != 0)
    return rc;

  /* Handle Arrays (Pointer to POD/Obj, but not char*) */
  if (is_array) {
    /* It's an array of the resolved type */
    char *inner_ref = out->ref_name ? c_cdd_strdup(out->ref_name) : NULL;
    char *inner_type = out->oa_type ? c_cdd_strdup(out->oa_type) : NULL;
    /* Move current mapping to "items" logic? */
    /* The OpenApiTypeMapping struct is flat. We need to indicate it's an Array
       OF X. */
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

  return 0;
}
