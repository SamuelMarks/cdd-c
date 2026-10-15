/**
 * @file openapi_internal.h
 * @brief Internal declarations and utilities for OpenAPI parser modules.
 * @author Samuel Marks
 */

#ifndef OPENAPI_INTERNAL_H
#define OPENAPI_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <ctype.h>
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "classes/emit/enum.h"
#include "classes/emit/struct.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include "functions/str_includes.h"
#include "openapi/emit/openapi.h"
#include "openapi/parse/openapi.h"
#include "openapi/parse/openapi_media.h"
#include "openapi/parse/openapi_schema.h"
#include "openapi/parse/openapi_uri.h"
#include "openapi/parse/openapi_validate.h"
/* clang-format on */

#undef C_CDD_MALLOC
/** @brief OpenAPI parser malloc macro */
#define C_CDD_MALLOC(sz) c_cdd_malloc(sz)
#undef C_CDD_CALLOC
/** @brief OpenAPI parser calloc macro */
#define C_CDD_CALLOC(count, sz) c_cdd_calloc(count, sz)
#undef C_CDD_REALLOC
/** @brief OpenAPI parser realloc macro */
#define C_CDD_REALLOC(ptr, sz) c_cdd_realloc(ptr, sz)

/** @brief Symbol alias for openapi_header_name_is_content_type */
#define header_name_is_content_type openapi_header_name_is_content_type

#ifdef CDD_BUILD_TESTS
/** @brief Fault injection counter for clone_json_value serialize */
extern C_CDD_EXPORT int g_cdd_fail_clone_json_serialize;
/** @brief Fault injection counter for clone_json_value parse */
extern C_CDD_EXPORT int g_cdd_fail_clone_json_parse;
/** @brief Fault injection counter for collect_schema_extras init */
extern C_CDD_EXPORT int g_cdd_fail_extras_init;
/** @brief Fault injection counter for collect_schema_extras serialize */
extern C_CDD_EXPORT int g_cdd_fail_extras_serialize;
/** @brief Fault injection counter for collect_extensions init */
extern C_CDD_EXPORT int g_cdd_fail_extensions_init;
/** @brief Fault injection counter for collect_extensions serialize */
extern C_CDD_EXPORT int g_cdd_fail_extensions_serialize;
/** @brief Fault injection counter for parse_any_value serialize */
extern C_CDD_EXPORT int g_cdd_fail_any_serialize;
/** @brief Fault injection counter for append_raw_schema serialize */
extern C_CDD_EXPORT int g_cdd_fail_raw_schema_serialize;
/** @brief Fault injection counter for store_schema_root_json serialize */
extern C_CDD_EXPORT int g_cdd_fail_root_schema_serialize;
/** @brief Fault injection counter for assign_schema_ref_name */
extern C_CDD_EXPORT int g_cdd_fail_assign_ref_name;
/** @brief Fault injection counter for openapi_spec_find_schema_by_id */
extern C_CDD_EXPORT int g_cdd_fail_find_schema_by_id;
/** @brief Fault injection counter for openapi_spec_find_schema_by_anchor */
extern C_CDD_EXPORT int g_cdd_fail_find_schema_by_anchor;
#endif

/**
 * @brief Parses HTTP verb string to enum.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_verb(const char *v,
                                             enum OpenAPI_Verb *_out_val);

/**
 * @brief Checks if fixed operation method.
 */
extern C_CDD_EXPORT cdd_c_error_t is_fixed_operation_method(const char *method);

/**
 * @brief Parses param in from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_param_in(const char *in, enum OpenAPI_ParamIn *_out_val);

/**
 * @brief Parses param style from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_param_style(const char *s, enum OpenAPI_Style *_out_val);

/**
 * @brief Checks whether an OpenAPI parameter type name is a scalar primitive.
 */
extern C_CDD_EXPORT cdd_c_error_t param_type_is_primitive(const char *type);

/**
 * @brief Checks whether an OpenAPI parameter represents an object or map
 * structure.
 */
extern C_CDD_EXPORT cdd_c_error_t
param_type_is_object_like(const struct OpenAPI_Parameter *p);

/**
 * @brief Checks case-insensitively whether a header name is Content-Type.
 */
extern C_CDD_EXPORT cdd_c_error_t header_name_is_content_type(const char *name);

/**
 * @brief Checks whether an HTTP header name is reserved by OpenAPI (Accept,
 * Content-Type, Authorization).
 */
extern C_CDD_EXPORT cdd_c_error_t
header_param_is_reserved(const struct OpenAPI_Parameter *param);

/**
 * @brief Parses security type from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_security_type(const char *type, enum OpenAPI_SecurityType *_out_val);

/**
 * @brief Parses security in from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_security_in(const char *in, enum OpenAPI_SecurityIn *_out_val);

/**
 * @brief Parses oauth flow type from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_oauth_flow_type(const char *flow, enum OpenAPI_OAuthFlowType *_out_val);

/**
 * @brief Parses xml node type from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_xml_node_type(const char *node_type, enum OpenAPI_XmlNodeType *_out_val);

/**
 * @brief Parses any value from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_any_value(const JSON_Value *val,
                                                  struct OpenAPI_Any *out);

/**
 * @brief Parses any field from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_any_field(const JSON_Object *obj,
                                                  const char *key,
                                                  struct OpenAPI_Any *out,
                                                  int *out_set);

/**
 * @brief Parses any array from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_any_array(const JSON_Array *arr,
                                                  struct OpenAPI_Any **out,
                                                  size_t *out_count);

/**
 * @brief Checks whether a string key already exists in a list of strings.
 */
extern C_CDD_EXPORT cdd_c_error_t key_in_list(const char *key,
                                              const char **list, size_t count);

/**
 * @brief Checks if extension key.
 */
extern C_CDD_EXPORT cdd_c_error_t is_extension_key(const char *key);

/**
 * @brief Deep-copies a Parson JSON_Value via serialization round-trip.
 */
extern C_CDD_EXPORT cdd_c_error_t clone_json_value(const JSON_Value *val,
                                                   JSON_Value **_out_val);

/**
 * @brief Collects schema extras.
 */
extern C_CDD_EXPORT cdd_c_error_t collect_schema_extras(const JSON_Object *obj,
                                                        const char **skip_keys,
                                                        size_t skip_count,
                                                        char **out_json);

/**
 * @brief Collects extensions.
 */
extern C_CDD_EXPORT cdd_c_error_t collect_extensions(const JSON_Object *obj,
                                                     char **out_json);

/**
 * @brief Frees the memory associated with any value.
 */
extern C_CDD_EXPORT void free_any_value(struct OpenAPI_Any *val);

/**
 * @brief Frees the memory associated with example.
 */
extern C_CDD_EXPORT void free_example(struct OpenAPI_Example *ex);

/**
 * @brief Frees the memory associated with link.
 */
extern C_CDD_EXPORT void free_link(struct OpenAPI_Link *link);

/**
 * @brief Frees the memory associated with security requirement.
 */
extern C_CDD_EXPORT void
free_security_requirement(struct OpenAPI_SecurityRequirement *req);

/**
 * @brief Frees the memory associated with security requirement set.
 */
extern C_CDD_EXPORT void
free_security_requirement_set(struct OpenAPI_SecurityRequirementSet *set);

/**
 * @brief Frees the memory associated with schema ref content.
 */
extern C_CDD_EXPORT void free_schema_ref_content(struct OpenAPI_SchemaRef *ref);

/**
 * @brief Frees the memory associated with header.
 */
extern C_CDD_EXPORT void free_header(struct OpenAPI_Header *hdr);

/**
 * @brief Frees the memory associated with encoding.
 */
extern C_CDD_EXPORT void free_encoding(struct OpenAPI_Encoding *enc);

/**
 * @brief Frees the memory associated with media type.
 */
extern C_CDD_EXPORT void free_media_type(struct OpenAPI_MediaType *mt);

/**
 * @brief Frees the memory associated with parameter.
 */
extern C_CDD_EXPORT void free_parameter(struct OpenAPI_Parameter *param);

/**
 * @brief Frees the memory associated with response.
 */
extern C_CDD_EXPORT void free_response(struct OpenAPI_Response *resp);

/**
 * @brief Frees the memory associated with request body.
 */
extern C_CDD_EXPORT void free_request_body(struct OpenAPI_RequestBody *rb);

/**
 * @brief Frees the memory associated with path item.
 */
extern C_CDD_EXPORT void free_path_item(struct OpenAPI_Path *p);

/**
 * @brief Frees the memory associated with callback.
 */
extern C_CDD_EXPORT void free_callback(struct OpenAPI_Callback *cb);

/**
 * @brief Frees the memory associated with operation.
 */
extern C_CDD_EXPORT void free_operation(struct OpenAPI_Operation *op);

/**
 * @brief Creates a deep copy of any value.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_any_value(struct OpenAPI_Any *dst,
                                                 const struct OpenAPI_Any *src);

/**
 * @brief Creates a deep copy of example fields.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_example_fields(
    struct OpenAPI_Example *dst, const struct OpenAPI_Example *src);

/**
 * @brief Creates a deep copy of schema ref.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_schema_ref(
    struct OpenAPI_SchemaRef *dst, const struct OpenAPI_SchemaRef *src);

/**
 * @brief Creates a deep copy of item schema as array.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_item_schema_as_array(
    struct OpenAPI_SchemaRef *dst, const struct OpenAPI_SchemaRef *item);

/**
 * @brief Creates a deep copy of header fields.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_header_fields(
    struct OpenAPI_Header *dst, const struct OpenAPI_Header *src);

/**
 * @brief Creates a deep copy of encoding fields.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_encoding_fields(
    struct OpenAPI_Encoding *dst, const struct OpenAPI_Encoding *src);

/**
 * @brief Creates a deep copy of media type fields.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_media_type_fields(
    struct OpenAPI_MediaType *dst, const struct OpenAPI_MediaType *src);

/**
 * @brief Creates a deep copy of media type array.
 */
extern C_CDD_EXPORT cdd_c_error_t
copy_media_type_array(struct OpenAPI_MediaType **dst, size_t *dst_count,
                      const struct OpenAPI_MediaType *src, size_t src_count);

/**
 * @brief Creates a deep copy of parameter fields.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_parameter_fields(
    struct OpenAPI_Parameter *dst, const struct OpenAPI_Parameter *src);

/**
 * @brief Creates a deep copy of server object.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_server_object(
    struct OpenAPI_Server *dst, const struct OpenAPI_Server *src);

/**
 * @brief Creates a deep copy of link fields.
 */
extern C_CDD_EXPORT cdd_c_error_t
copy_link_fields(struct OpenAPI_Link *dst, const struct OpenAPI_Link *src);

/**
 * @brief Creates a deep copy of response fields.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_response_fields(
    struct OpenAPI_Response *dst, const struct OpenAPI_Response *src);

/**
 * @brief Creates a deep copy of request body fields.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_request_body_fields(
    struct OpenAPI_RequestBody *dst, const struct OpenAPI_RequestBody *src);

/**
 * @brief Creates a deep copy of security requirement sets.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_security_requirement_sets(
    struct OpenAPI_SecurityRequirementSet **dst, size_t *dst_count,
    const struct OpenAPI_SecurityRequirementSet *src, size_t src_count);

/**
 * @brief Creates a deep copy of path fields.
 */
extern C_CDD_EXPORT cdd_c_error_t
copy_path_fields(struct OpenAPI_Path *dst, const struct OpenAPI_Path *src);

/**
 * @brief Creates a deep copy of callback fields.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_callback_fields(
    struct OpenAPI_Callback *dst, const struct OpenAPI_Callback *src);

/**
 * @brief Creates a deep copy of operation fields.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_operation_fields(
    struct OpenAPI_Operation *dst, const struct OpenAPI_Operation *src);

/**
 * @brief Retrieves the component parameter.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_parameter(const struct OpenAPI_Spec *spec, const char *ref,
                         struct OpenAPI_Parameter **_out_val);

/**
 * @brief Retrieves the component response.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_response(const struct OpenAPI_Spec *spec, const char *ref,
                        struct OpenAPI_Response **_out_val);

/**
 * @brief Retrieves the component header.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_header(const struct OpenAPI_Spec *spec, const char *ref,
                      struct OpenAPI_Header **_out_val);

/**
 * @brief Retrieves the component request body.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_request_body(const struct OpenAPI_Spec *spec, const char *ref,
                            struct OpenAPI_RequestBody **_out_val);

/**
 * @brief Retrieves the component media type.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_media_type(const struct OpenAPI_Spec *spec, const char *ref,
                          struct OpenAPI_MediaType **_out_val);

/**
 * @brief Retrieves the component example.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_example(const struct OpenAPI_Spec *spec, const char *ref,
                       struct OpenAPI_Example **_out_val);

/**
 * @brief Retrieves the component link.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_link(const struct OpenAPI_Spec *spec, const char *ref,
                    struct OpenAPI_Link **_out_val);

/**
 * @brief Retrieves the component callback.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_callback(const struct OpenAPI_Spec *spec, const char *ref,
                        struct OpenAPI_Callback **_out_val);

/**
 * @brief Retrieves the component path item.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_path_item(const struct OpenAPI_Spec *spec, const char *ref,
                         struct OpenAPI_Path **_out_val);

#include "openapi/parse/openapi_internal_components.h"

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENAPI_INTERNAL_H */
