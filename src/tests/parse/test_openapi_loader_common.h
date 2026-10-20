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
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Helper function load_spec_str.
 * @param[in] json_str Parameter json_str.
 * @param[in,out] spec Parameter spec.
 * @return Result status or value.
 */
static cdd_c_error_t load_spec_str(const char *json_str,
                                   struct OpenAPI_Spec *spec) {
  JSON_Value *dyn = json_parse_string(json_str);
  cdd_c_error_t rc = 0;
  if (!dyn)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  openapi_spec_init(spec);
  rc = openapi_load_from_json(dyn, spec);
  if (rc != CDD_C_SUCCESS)
    openapi_spec_free(spec);
  json_value_free(dyn);
  return rc;
}

static cdd_c_error_t
/**
 * @brief Helper function load_spec_str_with_context.
 * @param[in] json_str Parameter json_str.
 * @param[in] retrieval_uri Parameter retrieval_uri.
 * @param[in,out] registry Parameter registry.
 * @param[in,out] spec Parameter spec.
 * @return Result status or value.
 */
load_spec_str_with_context(const char *json_str, const char *retrieval_uri,
                           struct OpenAPI_DocRegistry *registry,
                           struct OpenAPI_Spec *spec) {
  JSON_Value *dyn = json_parse_string(json_str);
  cdd_c_error_t rc = 0;
  if (!dyn)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  openapi_spec_init(spec);
  rc = openapi_load_from_json_with_context(dyn, retrieval_uri, spec, registry);
  if (rc != CDD_C_SUCCESS)
    openapi_spec_free(spec);
  json_value_free(dyn);
  return rc;
}

/**
 * @brief Helper function test_wrap_parse_header_object.
 * @param[in] hdr_obj Parameter hdr_obj.
 * @param[out] out_hdr Parameter out_hdr.
 * @param[in] spec Parameter spec.
 * @param[in] resolve_refs Parameter resolve_refs.
 * @return Result status or value.
 */
static cdd_c_error_t test_wrap_parse_header_object(
    const JSON_Object *hdr_obj, struct OpenAPI_Header *out_hdr,
    const struct OpenAPI_Spec *spec, int resolve_refs) {
  cdd_c_error_t rc =
      (parse_header_object)(hdr_obj, out_hdr, spec, resolve_refs);
  if (rc != CDD_C_SUCCESS && out_hdr)
    free_header(out_hdr);
  return rc;
}
/**
 * @brief Macro parse_header_object.
 */
#define parse_header_object test_wrap_parse_header_object

/**
 * @brief Helper function test_wrap_parse_headers_object.
 * @param[in] headers Parameter headers.
 * @param[out] out_headers Parameter out_headers.
 * @param[out] out_count Parameter out_count.
 * @param[in] spec Parameter spec.
 * @param[in] resolve_refs Parameter resolve_refs.
 * @param[in] ignore_content_type Parameter ignore_content_type.
 * @return Result status or value.
 */
static cdd_c_error_t test_wrap_parse_headers_object(
    const JSON_Object *headers, struct OpenAPI_Header **out_headers,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs,
    int ignore_content_type) {
  cdd_c_error_t rc =
      (parse_headers_object)(headers, out_headers, out_count, spec,
                             resolve_refs, ignore_content_type);
  if (rc != CDD_C_SUCCESS && out_headers && *out_headers) {
    size_t i;
    for (i = 0; out_count && i < *out_count; ++i)
      free_header(&(*out_headers)[i]);
    free(*out_headers);
    *out_headers = NULL;
    if (out_count)
      *out_count = 0;
  }
  return rc;
}
/**
 * @brief Macro parse_headers_object.
 */
#define parse_headers_object test_wrap_parse_headers_object

static cdd_c_error_t
/**
 * @brief Helper function test_wrap_parse_link_object.
 * @param[in] link_obj Parameter link_obj.
 * @param[out] out_link Parameter out_link.
 * @param[in] spec Parameter spec.
 * @param[in] resolve_refs Parameter resolve_refs.
 * @return Result status or value.
 */
test_wrap_parse_link_object(const JSON_Object *link_obj,
                            struct OpenAPI_Link *out_link,
                            const struct OpenAPI_Spec *spec, int resolve_refs) {
  cdd_c_error_t rc =
      (parse_link_object)(link_obj, out_link, spec, resolve_refs);
  if (rc != CDD_C_SUCCESS && out_link)
    free_link(out_link);
  return rc;
}
/**
 * @brief Macro parse_link_object.
 */
#define parse_link_object test_wrap_parse_link_object

/**
 * @brief Helper function test_wrap_parse_links_object.
 * @param[in] links Parameter links.
 * @param[out] out_links Parameter out_links.
 * @param[out] out_count Parameter out_count.
 * @param[in] spec Parameter spec.
 * @param[in] resolve_refs Parameter resolve_refs.
 * @return Result status or value.
 */
static cdd_c_error_t test_wrap_parse_links_object(
    const JSON_Object *links, struct OpenAPI_Link **out_links,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs) {
  cdd_c_error_t rc =
      (parse_links_object)(links, out_links, out_count, spec, resolve_refs);
  if (rc != CDD_C_SUCCESS && out_links && *out_links) {
    size_t i;
    for (i = 0; out_count && i < *out_count; ++i)
      free_link(&(*out_links)[i]);
    free(*out_links);
    *out_links = NULL;
    if (out_count)
      *out_count = 0;
  }
  return rc;
}
/**
 * @brief Macro parse_links_object.
 */
#define parse_links_object test_wrap_parse_links_object

static cdd_c_error_t
/**
 * @brief Helper function test_wrap_parse_link_parameters.
 * @param[in] params_obj Parameter params_obj.
 * @param[out] out_params Parameter out_params.
 * @param[out] out_count Parameter out_count.
 * @return Result status or value.
 */
test_wrap_parse_link_parameters(const JSON_Object *params_obj,
                                struct OpenAPI_LinkParam **out_params,
                                size_t *out_count) {
  cdd_c_error_t rc = (parse_link_parameters)(params_obj, out_params, out_count);
  if (rc != CDD_C_SUCCESS && out_params && *out_params) {
    size_t i;
    for (i = 0; out_count && i < *out_count; ++i) {
      free((*out_params)[i].name);
      free_any_value(&(*out_params)[i].value);
    }
    free(*out_params);
    *out_params = NULL;
    if (out_count)
      *out_count = 0;
  }
  return rc;
}
/**
 * @brief Macro parse_link_parameters.
 */
#define parse_link_parameters test_wrap_parse_link_parameters

/**
 * @brief Helper function test_wrap_parse_encoding_object.
 * @param[in] enc_obj Parameter enc_obj.
 * @param[out] out Parameter out.
 * @param[in] spec Parameter spec.
 * @param[in] resolve_refs Parameter resolve_refs.
 * @return Result status or value.
 */
static cdd_c_error_t test_wrap_parse_encoding_object(
    const JSON_Object *enc_obj, struct OpenAPI_Encoding *out,
    const struct OpenAPI_Spec *spec, int resolve_refs) {
  cdd_c_error_t rc = (parse_encoding_object)(enc_obj, out, spec, resolve_refs);
  if (rc != CDD_C_SUCCESS && out)
    free_encoding(out);
  return rc;
}
/**
 * @brief Macro parse_encoding_object.
 */
#define parse_encoding_object test_wrap_parse_encoding_object

/**
 * @brief Helper function test_wrap_parse_encoding_map.
 * @param[in] enc_obj Parameter enc_obj.
 * @param[out] out Parameter out.
 * @param[out] out_count Parameter out_count.
 * @param[in] spec Parameter spec.
 * @param[in] resolve_refs Parameter resolve_refs.
 * @return Result status or value.
 */
static cdd_c_error_t test_wrap_parse_encoding_map(
    const JSON_Object *enc_obj, struct OpenAPI_Encoding **out,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs) {
  cdd_c_error_t rc =
      (parse_encoding_map)(enc_obj, out, out_count, spec, resolve_refs);
  if (rc != CDD_C_SUCCESS && out && *out) {
    size_t i;
    for (i = 0; out_count && i < *out_count; ++i)
      free_encoding(&(*out)[i]);
    free(*out);
    *out = NULL;
    if (out_count)
      *out_count = 0;
  }
  return rc;
}
/**
 * @brief Macro parse_encoding_map.
 */
#define parse_encoding_map test_wrap_parse_encoding_map

/**
 * @brief Helper function test_wrap_parse_encoding_array.
 * @param[in] enc_arr Parameter enc_arr.
 * @param[out] out Parameter out.
 * @param[out] out_count Parameter out_count.
 * @param[in] spec Parameter spec.
 * @param[in] resolve_refs Parameter resolve_refs.
 * @return Result status or value.
 */
static cdd_c_error_t test_wrap_parse_encoding_array(
    const JSON_Array *enc_arr, struct OpenAPI_Encoding **out, size_t *out_count,
    const struct OpenAPI_Spec *spec, int resolve_refs) {
  cdd_c_error_t rc =
      (parse_encoding_array)(enc_arr, out, out_count, spec, resolve_refs);
  if (rc != CDD_C_SUCCESS && out && *out) {
    size_t i;
    for (i = 0; out_count && i < *out_count; ++i)
      free_encoding(&(*out)[i]);
    free(*out);
    *out = NULL;
    if (out_count)
      *out_count = 0;
  }
  return rc;
}
/**
 * @brief Macro parse_encoding_array.
 */
#define parse_encoding_array test_wrap_parse_encoding_array

static cdd_c_error_t
/**
 * @brief Helper function test_wrap_parse_component_parameters.
 * @param[in] components Parameter components.
 * @param[out] out Parameter out.
 * @return Result status or value.
 */
test_wrap_parse_component_parameters(const JSON_Object *components,
                                     struct OpenAPI_Spec *out) {
  cdd_c_error_t rc = (parse_component_parameters)(components, out);
  if (rc != CDD_C_SUCCESS && out)
    openapi_spec_free(out);
  return rc;
}
/**
 * @brief Macro parse_component_parameters.
 */
#define parse_component_parameters test_wrap_parse_component_parameters

static cdd_c_error_t
/**
 * @brief Helper function test_wrap_parse_component_responses.
 * @param[in] components Parameter components.
 * @param[out] out Parameter out.
 * @return Result status or value.
 */
test_wrap_parse_component_responses(const JSON_Object *components,
                                    struct OpenAPI_Spec *out) {
  cdd_c_error_t rc = (parse_component_responses)(components, out);
  if (rc != CDD_C_SUCCESS && out)
    openapi_spec_free(out);
  return rc;
}
/**
 * @brief Macro parse_component_responses.
 */
#define parse_component_responses test_wrap_parse_component_responses

static cdd_c_error_t
/**
 * @brief Helper function test_wrap_parse_component_headers.
 * @param[in] components Parameter components.
 * @param[out] out Parameter out.
 * @return Result status or value.
 */
test_wrap_parse_component_headers(const JSON_Object *components,
                                  struct OpenAPI_Spec *out) {
  cdd_c_error_t rc = (parse_component_headers)(components, out);
  if (rc != CDD_C_SUCCESS && out)
    openapi_spec_free(out);
  return rc;
}
/**
 * @brief Macro parse_component_headers.
 */
#define parse_component_headers test_wrap_parse_component_headers

static cdd_c_error_t
/**
 * @brief Helper function test_wrap_parse_component_request_bodies.
 * @param[in] components Parameter components.
 * @param[out] out Parameter out.
 * @return Result status or value.
 */
test_wrap_parse_component_request_bodies(const JSON_Object *components,
                                         struct OpenAPI_Spec *out) {
  cdd_c_error_t rc = (parse_component_request_bodies)(components, out);
  if (rc != CDD_C_SUCCESS && out)
    openapi_spec_free(out);
  return rc;
}
/**
 * @brief Macro parse_component_request_bodies.
 */
#define parse_component_request_bodies test_wrap_parse_component_request_bodies

static cdd_c_error_t
/**
 * @brief Helper function test_wrap_parse_component_media_types.
 * @param[in] components Parameter components.
 * @param[out] out Parameter out.
 * @return Result status or value.
 */
test_wrap_parse_component_media_types(const JSON_Object *components,
                                      struct OpenAPI_Spec *out) {
  cdd_c_error_t rc = (parse_component_media_types)(components, out);
  if (rc != CDD_C_SUCCESS && out)
    openapi_spec_free(out);
  return rc;
}
/**
 * @brief Macro parse_component_media_types.
 */
#define parse_component_media_types test_wrap_parse_component_media_types

static cdd_c_error_t
/**
 * @brief Helper function test_wrap_parse_component_examples.
 * @param[in] components Parameter components.
 * @param[out] out Parameter out.
 * @return Result status or value.
 */
test_wrap_parse_component_examples(const JSON_Object *components,
                                   struct OpenAPI_Spec *out) {
  cdd_c_error_t rc = (parse_component_examples)(components, out);
  if (rc != CDD_C_SUCCESS && out)
    openapi_spec_free(out);
  return rc;
}
/**
 * @brief Macro parse_component_examples.
 */
#define parse_component_examples test_wrap_parse_component_examples

static cdd_c_error_t
/**
 * @brief Helper function test_wrap_parse_component_links.
 * @param[in] components Parameter components.
 * @param[out] out Parameter out.
 * @return Result status or value.
 */
test_wrap_parse_component_links(const JSON_Object *components,
                                struct OpenAPI_Spec *out) {
  cdd_c_error_t rc = (parse_component_links)(components, out);
  if (rc != CDD_C_SUCCESS && out)
    openapi_spec_free(out);
  return rc;
}
/**
 * @brief Macro parse_component_links.
 */
#define parse_component_links test_wrap_parse_component_links

static cdd_c_error_t
/**
 * @brief Helper function test_wrap_parse_component_callbacks.
 * @param[in] components Parameter components.
 * @param[out] out Parameter out.
 * @return Result status or value.
 */
test_wrap_parse_component_callbacks(const JSON_Object *components,
                                    struct OpenAPI_Spec *out) {
  cdd_c_error_t rc = (parse_component_callbacks)(components, out);
  if (rc != CDD_C_SUCCESS && out)
    openapi_spec_free(out);
  return rc;
}
/**
 * @brief Macro parse_component_callbacks.
 */
#define parse_component_callbacks test_wrap_parse_component_callbacks

static cdd_c_error_t
/**
 * @brief Helper function test_wrap_parse_component_path_items.
 * @param[in] components Parameter components.
 * @param[out] out Parameter out.
 * @return Result status or value.
 */
test_wrap_parse_component_path_items(const JSON_Object *components,
                                     struct OpenAPI_Spec *out) {
  cdd_c_error_t rc = (parse_component_path_items)(components, out);
  if (rc != CDD_C_SUCCESS && out)
    openapi_spec_free(out);
  return rc;
}
/**
 * @brief Macro parse_component_path_items.
 */
#define parse_component_path_items test_wrap_parse_component_path_items

/**
 * @brief Helper function test_wrap_parse_components.
 * @param[in] components Parameter components.
 * @param[out] out Parameter out.
 * @return Result status or value.
 */
static cdd_c_error_t test_wrap_parse_components(const JSON_Object *components,
                                                struct OpenAPI_Spec *out) {
  cdd_c_error_t rc = (parse_components)(components, out);
  if (rc != CDD_C_SUCCESS && out)
    openapi_spec_free(out);
  return rc;
}
/**
 * @brief Macro parse_components.
 */
#define parse_components test_wrap_parse_components

/**
 * @brief Helper function find_raw_schema_index.
 * @param[in] spec Parameter spec.
 * @param[in] name Parameter name.
 * @return Result status or value.
 */
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

/**
 * @brief Helper function find_scheme.
 * @param[in] spec Parameter spec.
 * @param[in] name Parameter name.
 * @param[in,out] _out_val Parameter _out_val.
 * @return Result status or value.
 */
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

/**
 * @brief Helper function find_media_type.
 * @param[in] mts Parameter mts.
 * @param[in] n Parameter n.
 * @param[in] name Parameter name.
 * @param[in,out] _out_val Parameter _out_val.
 * @return Result status or value.
 */
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

/**
 * @brief Helper function concat_chunks.
 * @param[in] chunks Parameter chunks.
 * @param[in] n Parameter n.
 * @param[out] out Parameter out.
 * @return Result status or value.
 */
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
