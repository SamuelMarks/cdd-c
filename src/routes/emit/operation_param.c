/**
 * @file operation_param.c
 * @brief Implementation of operation parameter helpers.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "win_compat_sym.h"

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "classes/parse/mapping.h"
#include "functions/parse/str.h"
#include "routes/emit/operation.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
#undef malloc
#define malloc(sz) C_CDD_MALLOC(sz)
#undef realloc
#define realloc(ptr, sz) C_CDD_REALLOC(ptr, sz)
#undef calloc
#define calloc(n, sz) C_CDD_CALLOC(n, sz)
extern C_CDD_EXPORT int g_cdd_fail_schema_ref_has_data;
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;
extern C_CDD_EXPORT int g_cdd_fail_apply_format;
#endif

/**
 * @brief Retrieves the doc param.
 */
cdd_c_error_t find_doc_param(const struct DocMetadata *doc, const char *name,
                             struct DocParam **_out_val) {
  size_t i;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_op_fail_find_doc_param;
  if (g_op_fail_find_doc_param) {
    g_op_fail_find_doc_param = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;
  if (!doc || !name)
    return CDD_C_SUCCESS;
  for (i = 0; i < doc->n_params; ++i) {
    if (doc->params[i].name && strcmp(doc->params[i].name, name) == 0) {
      *_out_val = &doc->params[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if path param.
 */
cdd_c_error_t is_path_param(const char *route, const char *name,
                            int *out_is_path) {
  char tmpl[128];
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_op_fail_is_path_param;
  if (g_op_fail_is_path_param) {
    g_op_fail_is_path_param = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (out_is_path)
    *out_is_path = 0;
  if (!route || !name || !out_is_path)
    return CDD_C_SUCCESS;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(tmpl, sizeof(tmpl), "{%s}", name);
#else
  snprintf(tmpl, sizeof(tmpl), "{%s}", name);
#endif
  *out_is_path = (strstr(route, tmpl) != NULL) ? 1 : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees dynamically allocated fields of an OpenAPI Encoding.
 */
cdd_c_error_t free_encoding_fields(struct OpenAPI_Encoding *enc) {
  if (!enc)
    return CDD_C_SUCCESS;
  if (enc->name) {
    free(enc->name);
    enc->name = NULL;
  }
  if (enc->content_type) {
    free(enc->content_type);
    enc->content_type = NULL;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets param to op.
 */
cdd_c_error_t add_param_to_op(struct OpenAPI_Operation *op,
                              struct OpenAPI_Parameter *p) {
  struct OpenAPI_Parameter *new_arr;
  size_t new_count;

  if (!op || !p)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  new_count = op->n_parameters + 1;
  new_arr = (struct OpenAPI_Parameter *)realloc(
      op->parameters, new_count * sizeof(struct OpenAPI_Parameter));
  if (!new_arr) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  op->parameters = new_arr;
  op->parameters[op->n_parameters] = *p; /* Copy struct */
  op->n_parameters = new_count;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees dynamically allocated fields of an OpenAPI Parameter.
 */
cdd_c_error_t free_param_fields(struct OpenAPI_Parameter *p) {
  if (!p)
    return CDD_C_SUCCESS;
  if (p->name) {
    free(p->name);
    p->name = NULL;
  }
  if (p->type) {
    free(p->type);
    p->type = NULL;
  }
  if (p->description) {
    free(p->description);
    p->description = NULL;
  }
  if (p->items_type) {
    free(p->items_type);
    p->items_type = NULL;
  }
  if (p->content_type) {
    free(p->content_type);
    p->content_type = NULL;
  }
  if (p->schema.ref_name) {
    free(p->schema.ref_name);
    p->schema.ref_name = NULL;
  }
  if (p->schema.ref) {
    free(p->schema.ref);
    p->schema.ref = NULL;
  }
  if (p->schema.inline_type) {
    free(p->schema.inline_type);
    p->schema.inline_type = NULL;
  }
  if (p->schema.items_ref) {
    free(p->schema.items_ref);
    p->schema.items_ref = NULL;
  }
  if (p->schema.format) {
    free(p->schema.format);
    p->schema.format = NULL;
  }
  if (p->schema.items_format) {
    free(p->schema.items_format);
    p->schema.items_format = NULL;
  }
  if (p->schema.content_media_type) {
    free(p->schema.content_media_type);
    p->schema.content_media_type = NULL;
  }
  if (p->schema.content_encoding) {
    free(p->schema.content_encoding);
    p->schema.content_encoding = NULL;
  }
  if (p->schema.items_content_media_type) {
    free(p->schema.items_content_media_type);
    p->schema.items_content_media_type = NULL;
  }
  if (p->schema.items_content_encoding) {
    free(p->schema.items_content_encoding);
    p->schema.items_content_encoding = NULL;
  }
  if (p->example_set) {
    free_any_value_local(&p->example);
    p->example_set = 0;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets querystring schema from type map.
 */
cdd_c_error_t set_querystring_schema_from_type_map(
    struct OpenAPI_Parameter *param,
    const struct OpenApiTypeMapping *type_map) {
  cdd_c_error_t rc;
  if (!param || !type_map)
    return CDD_C_SUCCESS;
  if (type_map->ref_name) {
    param->schema_set = 1;
    param->schema.is_array = (type_map->kind == OA_TYPE_ARRAY);
    rc = c_cdd_strdup(type_map->ref_name, &param->schema.ref_name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  }
  if (type_map->kind == OA_TYPE_ARRAY) {
    param->is_array = 1;
    rc = c_cdd_strdup("array", &param->type);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (type_map->oa_type) {
      rc = c_cdd_strdup(type_map->oa_type, &param->items_type);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return CDD_C_SUCCESS;
  }
  if (type_map->oa_type) {
    rc = c_cdd_strdup(type_map->oa_type, &param->type);
  } else {
    rc = c_cdd_strdup("string", &param->type);
  }
  return rc;
}

/**
 * @brief Checks if an OpenAPI type name is a primitive type.
 *
 * @param[in] type Type name string.
 * @param[out] out_is_primitive Pointer to int receiving 1 if primitive, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
cdd_c_error_t oa_type_is_primitive(const char *type, int *out_is_primitive) {
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_op_fail_oa_type_is_primitive;
  if (g_op_fail_oa_type_is_primitive) {
    g_op_fail_oa_type_is_primitive = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (out_is_primitive)
    *out_is_primitive = 0;
  if (!type || !out_is_primitive)
    return CDD_C_SUCCESS;
  *out_is_primitive =
      (strcmp(type, "integer") == 0 || strcmp(type, "number") == 0 ||
       strcmp(type, "string") == 0 || strcmp(type, "boolean") == 0)
          ? 1
          : 0;
  return CDD_C_SUCCESS;
}

/* Apply format from type mapping (or override) to a SchemaRef.
 * Returns: 1 if applied, 0 if not applicable, or ENOMEM on allocation failure.
 */
/**
 * @brief Applies format to schema ref.
 */
cdd_c_error_t apply_format_to_schema_ref(struct OpenAPI_SchemaRef *schema,
                                         const struct OpenApiTypeMapping *map,
                                         const char *override_format,
                                         int *out_applied) {
  const char *fmt;
  int is_prim = 0;
  cdd_c_error_t rc;

  if (out_applied)
    *out_applied = 0;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_apply_format) {
    g_cdd_fail_apply_format = 0;
    return CDD_C_ERROR_MEMORY;
  }
#endif
  if (!schema || !map)
    return CDD_C_SUCCESS;
  fmt =
      (override_format && *override_format) ? override_format : map->oa_format;
  if (!fmt || !*fmt)
    return CDD_C_SUCCESS;
  if (!map->oa_type)
    return CDD_C_SUCCESS;
  rc = oa_type_is_primitive(map->oa_type, &is_prim);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!is_prim)
    return CDD_C_SUCCESS;

  if (map->kind == OA_TYPE_ARRAY) {
    schema->is_array = 1;
    if (!schema->inline_type) {
      rc = c_cdd_strdup(map->oa_type, &schema->inline_type);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (schema->items_format) {
      free(schema->items_format);
      schema->items_format = NULL;
    }
    rc = c_cdd_strdup(fmt, &schema->items_format);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    if (!schema->inline_type) {
      rc = c_cdd_strdup(map->oa_type, &schema->inline_type);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (schema->format) {
      free(schema->format);
      schema->format = NULL;
    }
    rc = c_cdd_strdup(fmt, &schema->format);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (out_applied)
    *out_applied = 1;
  return CDD_C_SUCCESS;
}

/* --- Type Analysis --- */

/**
 * @brief Determine if a type is a struct pointer eligible for Body.
 * Heuristic: Contains "struct", ends with "*" or "**".
 */
/**
 * @brief Checks if struct pointer.
 */
cdd_c_error_t is_struct_pointer(const char *type, int *is_double_ptr,
                                int *out_is_struct_ptr) {
  const char *p;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_op_fail_is_struct_pointer;
  if (g_op_fail_is_struct_pointer) {
    g_op_fail_is_struct_pointer = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (out_is_struct_ptr)
    *out_is_struct_ptr = 0;
  if (!type)
    return CDD_C_SUCCESS;
  if (!strstr(type, "struct "))
    return CDD_C_SUCCESS;

  p = strrchr(type, '*');
  if (!p)
    return CDD_C_SUCCESS;

  if (is_double_ptr)
    *is_double_ptr = (p > type && *(p - 1) == '*') ? 1 : 0;

  if (out_is_struct_ptr)
    *out_is_struct_ptr = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the doc style to openapi operation.
 */
cdd_c_error_t doc_style_to_openapi(enum DocParamStyle style,
                                   enum OpenAPI_Style *_out_val) {
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_op_fail_doc_style_to_openapi;
  if (g_op_fail_doc_style_to_openapi) {
    g_op_fail_doc_style_to_openapi = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (style) {
  case DOC_PARAM_STYLE_FORM: {
    *_out_val = OA_STYLE_FORM;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_SIMPLE: {
    *_out_val = OA_STYLE_SIMPLE;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_MATRIX: {
    *_out_val = OA_STYLE_MATRIX;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_LABEL: {
    *_out_val = OA_STYLE_LABEL;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_SPACE_DELIMITED: {
    *_out_val = OA_STYLE_SPACE_DELIMITED;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_PIPE_DELIMITED: {
    *_out_val = OA_STYLE_PIPE_DELIMITED;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_DEEP_OBJECT: {
    *_out_val = OA_STYLE_DEEP_OBJECT;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_COOKIE: {
    *_out_val = OA_STYLE_COOKIE;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_UNSET:
  default: {
    *_out_val = OA_STYLE_UNKNOWN;
    return CDD_C_SUCCESS;
  }
  }
}

/* --- Core Logic --- */
