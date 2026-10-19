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
#include "openapi/parse/openapi_internal_components.h"
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
 *
 * @param[in] v The HTTP verb string.
 * @param[out] _out_val Output verb enum value.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_verb(const char *v,
                                             enum OpenAPI_Verb *_out_val);

/**
 * @brief Checks if fixed operation method.
 *
 * @param[in] method The HTTP method string to check.
 * @return Non-zero if fixed operation method, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t is_fixed_operation_method(const char *method);

/**
 * @brief Parses param in from the given input.
 *
 * @param[in] in The parameter location string.
 * @param[out] _out_val Output parameter location enum value.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_param_in(const char *in, enum OpenAPI_ParamIn *_out_val);

/**
 * @brief Parses param style from the given input.
 *
 * @param[in] s The parameter style string.
 * @param[out] _out_val Output parameter style enum value.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_param_style(const char *s, enum OpenAPI_Style *_out_val);

/**
 * @brief Checks whether an OpenAPI parameter type name is a scalar primitive.
 *
 * @param[in] type The parameter type string.
 * @return Non-zero if primitive, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t param_type_is_primitive(const char *type);

/**
 * @brief Checks whether an OpenAPI parameter represents an object or map
 * structure.
 *
 * @param[in] p The OpenAPI parameter structure.
 * @return Non-zero if object-like, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
param_type_is_object_like(const struct OpenAPI_Parameter *p);

/**
 * @brief Checks case-insensitively whether a header name is Content-Type.
 *
 * @param[in] name The header name string.
 * @return Non-zero if Content-Type, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t header_name_is_content_type(const char *name);

/**
 * @brief Checks whether an HTTP header name is reserved by OpenAPI (Accept,
 * Content-Type, Authorization).
 *
 * @param[in] param The OpenAPI parameter structure.
 * @return Non-zero if reserved header, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
header_param_is_reserved(const struct OpenAPI_Parameter *param);

/**
 * @brief Parses security type from the given input.
 *
 * @param[in] type The security type string.
 * @param[out] _out_val Output security type enum value.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_security_type(const char *type, enum OpenAPI_SecurityType *_out_val);

/**
 * @brief Parses security in from the given input.
 *
 * @param[in] in The security location string.
 * @param[out] _out_val Output security location enum value.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_security_in(const char *in, enum OpenAPI_SecurityIn *_out_val);

/**
 * @brief Parses oauth flow type from the given input.
 *
 * @param[in] flow The OAuth flow type string.
 * @param[out] _out_val Output OAuth flow type enum value.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_oauth_flow_type(const char *flow, enum OpenAPI_OAuthFlowType *_out_val);

/**
 * @brief Parses xml node type from the given input.
 *
 * @param[in] node_type The XML node type string.
 * @param[out] _out_val Output XML node type enum value.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_xml_node_type(const char *node_type, enum OpenAPI_XmlNodeType *_out_val);

/**
 * @brief Parses any value from the given input.
 *
 * @param[in] val The source JSON value.
 * @param[out] out Output OpenAPI_Any structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_any_value(const JSON_Value *val,
                                                  struct OpenAPI_Any *out);

/**
 * @brief Parses any field from the given input.
 *
 * @param[in] obj The source JSON object.
 * @param[in] key The field key name.
 * @param[out] out Output OpenAPI_Any structure.
 * @param[out] out_set Output flag set to 1 if field was present.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_any_field(const JSON_Object *obj,
                                                  const char *key,
                                                  struct OpenAPI_Any *out,
                                                  int *out_set);

/**
 * @brief Parses any array from the given input.
 *
 * @param[in] arr The source JSON array.
 * @param[out] out Output array of OpenAPI_Any structures.
 * @param[out] out_count Output count of parsed elements.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_any_array(const JSON_Array *arr,
                                                  struct OpenAPI_Any **out,
                                                  size_t *out_count);

/**
 * @brief Checks whether a string key already exists in a list of strings.
 *
 * @param[in] key The key string to search for.
 * @param[in] list Array of string keys.
 * @param[in] count Number of elements in list.
 * @return Non-zero if found, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t key_in_list(const char *key,
                                              const char **list, size_t count);

/**
 * @brief Checks if extension key.
 *
 * @param[in] key The property key name.
 * @return Non-zero if key starts with x-, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t is_extension_key(const char *key);

/**
 * @brief Deep-copies a Parson JSON_Value via serialization round-trip.
 */
extern C_CDD_EXPORT cdd_c_error_t clone_json_value(const JSON_Value *val,
                                                   JSON_Value **_out_val);

/**
 * @brief Collects schema extras.
 *
 * @param[in] obj The source JSON object.
 * @param[in] skip_keys Array of known keys to skip.
 * @param[in] skip_count Number of keys in skip_keys.
 * @param[out] out_json Output serialized JSON string of extra properties.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t collect_schema_extras(const JSON_Object *obj,
                                                        const char **skip_keys,
                                                        size_t skip_count,
                                                        char **out_json);

/**
 * @brief Collects extensions.
 *
 * @param[in] obj The source JSON object.
 * @param[out] out_json Output serialized JSON string of x- extensions.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t collect_extensions(const JSON_Object *obj,
                                                     char **out_json);

/**
 * @brief Frees the memory associated with any value.
 *
 * @param[in,out] val The OpenAPI_Any value to free.
 */
extern C_CDD_EXPORT void free_any_value(struct OpenAPI_Any *val);

/**
 * @brief Frees the memory associated with example.
 *
 * @param[in,out] ex The OpenAPI_Example structure to free.
 */
extern C_CDD_EXPORT void free_example(struct OpenAPI_Example *ex);

/**
 * @brief Frees the memory associated with link.
 *
 * @param[in,out] link The OpenAPI_Link structure to free.
 */
extern C_CDD_EXPORT void free_link(struct OpenAPI_Link *link);

/**
 * @brief Frees the memory associated with security requirement.
 *
 * @param[in,out] req The OpenAPI_SecurityRequirement structure to free.
 */
extern C_CDD_EXPORT void
free_security_requirement(struct OpenAPI_SecurityRequirement *req);

/**
 * @brief Frees the memory associated with security requirement set.
 *
 * @param[in,out] set The OpenAPI_SecurityRequirementSet structure to free.
 */
extern C_CDD_EXPORT void
free_security_requirement_set(struct OpenAPI_SecurityRequirementSet *set);

/**
 * @brief Frees the memory associated with schema ref content.
 *
 * @param[in,out] ref The OpenAPI_SchemaRef structure to free.
 */
extern C_CDD_EXPORT void free_schema_ref_content(struct OpenAPI_SchemaRef *ref);

/**
 * @brief Frees the memory associated with header.
 *
 * @param[in,out] hdr The OpenAPI_Header structure to free.
 */
extern C_CDD_EXPORT void free_header(struct OpenAPI_Header *hdr);

/**
 * @brief Frees the memory associated with encoding.
 *
 * @param[in,out] enc The OpenAPI_Encoding structure to free.
 */
extern C_CDD_EXPORT void free_encoding(struct OpenAPI_Encoding *enc);

/**
 * @brief Frees the memory associated with media type.
 *
 * @param[in,out] mt The OpenAPI_MediaType structure to free.
 */
extern C_CDD_EXPORT void free_media_type(struct OpenAPI_MediaType *mt);

/**
 * @brief Frees the memory associated with parameter.
 *
 * @param[in,out] param The OpenAPI_Parameter structure to free.
 */
extern C_CDD_EXPORT void free_parameter(struct OpenAPI_Parameter *param);

/**
 * @brief Frees the memory associated with response.
 *
 * @param[in,out] resp The OpenAPI_Response structure to free.
 */
extern C_CDD_EXPORT void free_response(struct OpenAPI_Response *resp);

/**
 * @brief Frees the memory associated with request body.
 *
 * @param[in,out] rb The OpenAPI_RequestBody structure to free.
 */
extern C_CDD_EXPORT void free_request_body(struct OpenAPI_RequestBody *rb);

/**
 * @brief Frees the memory associated with path item.
 *
 * @param[in,out] p The OpenAPI_Path structure to free.
 */
extern C_CDD_EXPORT void free_path_item(struct OpenAPI_Path *p);

/**
 * @brief Frees the memory associated with callback.
 *
 * @param[in,out] cb The OpenAPI_Callback structure to free.
 */
extern C_CDD_EXPORT void free_callback(struct OpenAPI_Callback *cb);

/**
 * @brief Frees the memory associated with operation.
 *
 * @param[in,out] op The OpenAPI_Operation structure to free.
 */
extern C_CDD_EXPORT void free_operation(struct OpenAPI_Operation *op);

/**
 * @brief Creates a deep copy of any value.
 *
 * @param[out] dst Destination OpenAPI_Any structure.
 * @param[in] src Source OpenAPI_Any structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_any_value(struct OpenAPI_Any *dst,
                                                 const struct OpenAPI_Any *src);

/**
 * @brief Creates a deep copy of example fields.
 *
 * @param[out] dst Destination OpenAPI_Example structure.
 * @param[in] src Source OpenAPI_Example structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_example_fields(
    struct OpenAPI_Example *dst, const struct OpenAPI_Example *src);

/**
 * @brief Creates a deep copy of schema ref.
 *
 * @param[out] dst Destination OpenAPI_SchemaRef structure.
 * @param[in] src Source OpenAPI_SchemaRef structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_schema_ref(
    struct OpenAPI_SchemaRef *dst, const struct OpenAPI_SchemaRef *src);

/**
 * @brief Creates a deep copy of item schema as array.
 *
 * @param[out] dst Destination array OpenAPI_SchemaRef structure.
 * @param[in] item Source item OpenAPI_SchemaRef structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_item_schema_as_array(
    struct OpenAPI_SchemaRef *dst, const struct OpenAPI_SchemaRef *item);

/**
 * @brief Creates a deep copy of header fields.
 *
 * @param[out] dst Destination OpenAPI_Header structure.
 * @param[in] src Source OpenAPI_Header structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_header_fields(
    struct OpenAPI_Header *dst, const struct OpenAPI_Header *src);

/**
 * @brief Creates a deep copy of encoding fields.
 *
 * @param[out] dst Destination OpenAPI_Encoding structure.
 * @param[in] src Source OpenAPI_Encoding structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_encoding_fields(
    struct OpenAPI_Encoding *dst, const struct OpenAPI_Encoding *src);

/**
 * @brief Creates a deep copy of media type fields.
 *
 * @param[out] dst Destination OpenAPI_MediaType structure.
 * @param[in] src Source OpenAPI_MediaType structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_media_type_fields(
    struct OpenAPI_MediaType *dst, const struct OpenAPI_MediaType *src);

/**
 * @brief Creates a deep copy of media type array.
 *
 * @param[out] dst Output array of OpenAPI_MediaType structures.
 * @param[out] dst_count Output count of copied media types.
 * @param[in] src Source array of OpenAPI_MediaType structures.
 * @param[in] src_count Number of elements in src.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
copy_media_type_array(struct OpenAPI_MediaType **dst, size_t *dst_count,
                      const struct OpenAPI_MediaType *src, size_t src_count);

/**
 * @brief Creates a deep copy of parameter fields.
 *
 * @param[out] dst Destination OpenAPI_Parameter structure.
 * @param[in] src Source OpenAPI_Parameter structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_parameter_fields(
    struct OpenAPI_Parameter *dst, const struct OpenAPI_Parameter *src);

/**
 * @brief Creates a deep copy of server object.
 *
 * @param[out] dst Destination OpenAPI_Server structure.
 * @param[in] src Source OpenAPI_Server structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_server_object(
    struct OpenAPI_Server *dst, const struct OpenAPI_Server *src);

/**
 * @brief Creates a deep copy of link fields.
 *
 * @param[out] dst Destination OpenAPI_Link structure.
 * @param[in] src Source OpenAPI_Link structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
copy_link_fields(struct OpenAPI_Link *dst, const struct OpenAPI_Link *src);

/**
 * @brief Creates a deep copy of response fields.
 *
 * @param[out] dst Destination OpenAPI_Response structure.
 * @param[in] src Source OpenAPI_Response structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_response_fields(
    struct OpenAPI_Response *dst, const struct OpenAPI_Response *src);

/**
 * @brief Creates a deep copy of request body fields.
 *
 * @param[out] dst Destination OpenAPI_RequestBody structure.
 * @param[in] src Source OpenAPI_RequestBody structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_request_body_fields(
    struct OpenAPI_RequestBody *dst, const struct OpenAPI_RequestBody *src);

/**
 * @brief Creates a deep copy of security requirement sets.
 *
 * @param[out] dst Output array of OpenAPI_SecurityRequirementSet structures.
 * @param[out] dst_count Output count of security requirement sets.
 * @param[in] src Source array of OpenAPI_SecurityRequirementSet structures.
 * @param[in] src_count Number of elements in src.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_security_requirement_sets(
    struct OpenAPI_SecurityRequirementSet **dst, size_t *dst_count,
    const struct OpenAPI_SecurityRequirementSet *src, size_t src_count);

/**
 * @brief Creates a deep copy of path fields.
 *
 * @param[out] dst Destination OpenAPI_Path structure.
 * @param[in] src Source OpenAPI_Path structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
copy_path_fields(struct OpenAPI_Path *dst, const struct OpenAPI_Path *src);

/**
 * @brief Creates a deep copy of callback fields.
 *
 * @param[out] dst Destination OpenAPI_Callback structure.
 * @param[in] src Source OpenAPI_Callback structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_callback_fields(
    struct OpenAPI_Callback *dst, const struct OpenAPI_Callback *src);

/**
 * @brief Creates a deep copy of operation fields.
 *
 * @param[out] dst Destination OpenAPI_Operation structure.
 * @param[in] src Source OpenAPI_Operation structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_operation_fields(
    struct OpenAPI_Operation *dst, const struct OpenAPI_Operation *src);

/**
 * @brief Retrieves the component parameter.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found parameter.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_parameter(const struct OpenAPI_Spec *spec, const char *ref,
                         struct OpenAPI_Parameter **_out_val);

/**
 * @brief Retrieves the component response.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found response.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_response(const struct OpenAPI_Spec *spec, const char *ref,
                        struct OpenAPI_Response **_out_val);

/**
 * @brief Retrieves the component header.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found header.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_header(const struct OpenAPI_Spec *spec, const char *ref,
                      struct OpenAPI_Header **_out_val);

/**
 * @brief Retrieves the component request body.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found request body.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_request_body(const struct OpenAPI_Spec *spec, const char *ref,
                            struct OpenAPI_RequestBody **_out_val);

/**
 * @brief Retrieves the component media type.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found media type.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_media_type(const struct OpenAPI_Spec *spec, const char *ref,
                          struct OpenAPI_MediaType **_out_val);

/**
 * @brief Retrieves the component example.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found example.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_example(const struct OpenAPI_Spec *spec, const char *ref,
                       struct OpenAPI_Example **_out_val);

/**
 * @brief Retrieves the component link.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found link.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_link(const struct OpenAPI_Spec *spec, const char *ref,
                    struct OpenAPI_Link **_out_val);

/**
 * @brief Retrieves the component callback.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found callback.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_callback(const struct OpenAPI_Spec *spec, const char *ref,
                        struct OpenAPI_Callback **_out_val);

/**
 * @brief Retrieves the component path item.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found path item.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_component_path_item(const struct OpenAPI_Spec *spec, const char *ref,
                         struct OpenAPI_Path **_out_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENAPI_INTERNAL_H */
