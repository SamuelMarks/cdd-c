/**
 * @file test_openapi_loader_common.h
 * @brief Common definitions, mocks, and helpers for OpenAPI loader unit tests.
 */

#ifndef TEST_OPENAPI_LOADER_COMMON_H
#define TEST_OPENAPI_LOADER_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "c_cdd/safe_crt.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <parson.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/struct.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

/* clang-format on */

extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_parameter(struct OpenAPI_Parameter *param);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_header(struct OpenAPI_Header *hdr);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_operation(struct OpenAPI_Operation *op);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_parameter_fields(
    struct OpenAPI_Parameter *dst, const struct OpenAPI_Parameter *src);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_header_fields(
    struct OpenAPI_Header *dst, const struct OpenAPI_Header *src);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_encoding_fields(
    struct OpenAPI_Encoding *dst, const struct OpenAPI_Encoding *src);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_media_type_fields(
    struct OpenAPI_MediaType *dst, const struct OpenAPI_MediaType *src);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_response_fields(
    struct OpenAPI_Response *dst, const struct OpenAPI_Response *src);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_operation_fields(
    struct OpenAPI_Operation *dst, const struct OpenAPI_Operation *src);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_schema_ref(
    struct OpenAPI_SchemaRef *dst, const struct OpenAPI_SchemaRef *src);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_example_fields(
    struct OpenAPI_Example *dst, const struct OpenAPI_Example *src);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_encoding(struct OpenAPI_Encoding *enc);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_media_type(struct OpenAPI_MediaType *mt);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_response(struct OpenAPI_Response *resp);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_request_body(struct OpenAPI_RequestBody *rb);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_any_value(struct OpenAPI_Any *val);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_free_link(struct OpenAPI_Link *link);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_security_requirement(struct OpenAPI_SecurityRequirement *req);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_path_item(struct OpenAPI_Path *p);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_callback(struct OpenAPI_Callback *cb);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_free_string_array(char **arr,
                                                             size_t n);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_example(struct OpenAPI_Example *ex);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_schema_ref_content(struct OpenAPI_SchemaRef *ref);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_path_fields(
    struct OpenAPI_Path *dst, const struct OpenAPI_Path *src);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_callback_fields(
    struct OpenAPI_Callback *dst, const struct OpenAPI_Callback *src);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_request_body_fields(
    struct OpenAPI_RequestBody *dst, const struct OpenAPI_RequestBody *src);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_server_object(
    struct OpenAPI_Server *dst, const struct OpenAPI_Server *src);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_link_fields(
    struct OpenAPI_Link *dst, const struct OpenAPI_Link *src);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_component_callback_is_referenced(
    const struct OpenAPI_Spec *spec, const char *name);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_media_type(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_MediaType **out_val);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_find_media_object_by_name(
    const JSON_Object *content, const char *media_name, JSON_Object **out_val);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_schema_ref(
    const JSON_Object *schema, struct OpenAPI_SchemaRef *out,
    const struct OpenAPI_Spec *spec);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_xml_node_type(
    const char *str, enum OpenAPI_XmlNodeType *out_val);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_schema_type(
    const JSON_Object *schema, int *out_nullable, char **out_val);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_json_pointer_unescape(const char *in, char **out_val);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_normalize_path(const char *path,
                                                          char **out_val);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_resolve_uri_reference(
    const char *base, const char *ref, char **out_val);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_ref_base_matches_self(
    const struct OpenAPI_Spec *spec, const char *ref, const char *hash);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_ref_name_from_prefix(const struct OpenAPI_Spec *spec, const char *ref,
                              const char *prefix, char **out_val);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_param_in(const char *in, enum OpenAPI_ParamIn *out_val);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_param_style(const char *s, enum OpenAPI_Style *out_val);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_validate_parameter_style(
    const struct OpenAPI_Parameter *p, int has_content);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_find_path_param(const struct OpenAPI_Parameter *params, size_t n,
                         const char *name, struct OpenAPI_Parameter **out_val);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_scan_querystring_usage(
    const struct OpenAPI_Parameter *params, size_t n_params, size_t *qs_count,
    int *has_query);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_normalize_path_template_route(const char *route, char **out_val);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_collect_schema_extras(const JSON_Object *obj, const char **skip_keys,
                               size_t skip_count, char **out_json);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_collect_extensions(const JSON_Object *obj, char **out_json);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_info(const JSON_Object *root_obj, struct OpenAPI_Spec *out);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_security_type(
    const char *type, enum OpenAPI_SecurityType *_out_val);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_security_in(const char *in, enum OpenAPI_SecurityIn *_out_val);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_media_type_key_map(const JSON_Object *obj);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_header_name_is_content_type(const char *name);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_param_type_is_primitive(const char *type);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_url_has_query_or_fragment(const char *url);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_openapi_version_supported(const char *version);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_example_fields_valid(const struct OpenAPI_Example *ex);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_object_has_example_and_examples(const JSON_Object *obj);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_uri_has_scheme_prefix(const char *uri, size_t len);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_root_is_schema_document(
    const JSON_Value *root, const JSON_Object *root_obj);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_root_has_openapi_fields(const JSON_Object *root_obj);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_tag_index_by_name(
    const struct OpenAPI_Spec *spec, const char *name, size_t *out_idx);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_detect_tag_cycle(
    const struct OpenAPI_Spec *spec, size_t idx, int *state);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_tag_parents(const struct OpenAPI_Spec *spec);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_server_variable_defined(
    const struct OpenAPI_Server *srv, const char *name);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_server_variable_seen(char **seen, size_t seen_count, const char *name);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_is_valid_response_code_key(const char *code);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_media_type_base_len(const char *name, size_t *_out_val);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_media_type_base_equal(const char *a,
                                                                 const char *b);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_media_type_is_json(const char *name);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_param_key_equals(
    const struct OpenAPI_Parameter *a, const struct OpenAPI_Parameter *b);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_schema_has_composition(const JSON_Object *schema_obj);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_is_fixed_operation_method(const char *method);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_validate_querystring_usage(
    const struct OpenAPI_Path *paths, size_t n_paths);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_schema_name_in_use(const struct OpenAPI_Spec *spec, const char *name);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_operation(const char *verb_str, const JSON_Object *op_obj,
                         struct OpenAPI_Operation *out_op,
                         const struct OpenAPI_Spec *spec, int is_additional);

static cdd_c_error_t load_spec_str(const char *json_str,
                                   struct OpenAPI_Spec *spec) {
  JSON_Value *dyn = json_parse_string(json_str);
  cdd_c_error_t rc;
  if (!dyn)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  openapi_spec_init(spec);
  rc = openapi_load_from_json(dyn, spec);
  json_value_free(dyn);
  return rc;
}

static cdd_c_error_t
load_spec_str_with_context(const char *json_str, const char *retrieval_uri,
                           struct OpenAPI_DocRegistry *registry,
                           struct OpenAPI_Spec *spec) {
  JSON_Value *dyn = json_parse_string(json_str);
  cdd_c_error_t rc;
  if (!dyn)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  openapi_spec_init(spec);
  rc = openapi_load_from_json_with_context(dyn, retrieval_uri, spec, registry);
  json_value_free(dyn);
  return rc;
}

static cdd_c_error_t find_raw_schema_index(const struct OpenAPI_Spec *spec,
                                           const char *name) {
  size_t i;
  if (!spec || !name)
    return -1;
  for (i = 0; i < spec->n_raw_schemas; ++i) {
    if (spec->raw_schema_names && spec->raw_schema_names[i] &&
        strcmp(spec->raw_schema_names[i], name) == 0) {
      return (int)i;
    }
  }
  return -1;
}

static cdd_c_error_t find_scheme(const struct OpenAPI_Spec *spec,
                                 const char *name,
                                 struct OpenAPI_SecurityScheme **_out_val) {
  size_t i;
  if (!spec || !name) {
    *_out_val = NULL;
    return 0;
  }
  for (i = 0; i < spec->n_security_schemes; ++i) {
    if (spec->security_schemes[i].name &&
        strcmp(spec->security_schemes[i].name, name) == 0) {
      {
        *_out_val = &spec->security_schemes[i];
        return 0;
      }
    }
  }
  {
    *_out_val = NULL;
    return 0;
  }
}

static cdd_c_error_t find_media_type(const struct OpenAPI_MediaType *mts,
                                     size_t n, const char *name,
                                     struct OpenAPI_MediaType **_out_val) {
  size_t i;
  if (!mts || !name) {
    *_out_val = NULL;
    return 0;
  }
  for (i = 0; i < n; ++i) {
    if (mts[i].name && strcmp(mts[i].name, name) == 0) {
      *_out_val = (struct OpenAPI_MediaType *)(size_t)&mts[i];
      return 0;
    }
  }
  {
    *_out_val = NULL;
    return 0;
  }
}

static cdd_c_error_t concat_chunks(const char **chunks, size_t n, char **out) {
  size_t total_len = 0;
  size_t i;
  char *buf;
  if (!chunks || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < n; ++i) {
    if (chunks[i])
      total_len += strlen(chunks[i]);
  }
  if (g_cdd_alloc_fail) {
    buf = NULL;
  } else {
    buf = (char *)malloc(total_len + 1);
  }
  if (!buf)
    return CDD_C_ERROR_MEMORY;
  buf[0] = '\0';
  for (i = 0; i < n; ++i) {
    if (chunks[i]) {
#if defined(_MSC_VER)
      strcat_s(buf, total_len + 1, chunks[i]);
#else
      strcat(buf, chunks[i]);
#endif
    }
  }
  *out = buf;
  return CDD_C_SUCCESS;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_COMMON_H */
