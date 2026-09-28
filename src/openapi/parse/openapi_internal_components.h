/**
 * @file openapi_internal_components.h
 * @brief Internal declarations for OpenAPI parser components, paths, and
 * loader.
 * @author Samuel Marks
 */

#ifndef OPENAPI_INTERNAL_COMPONENTS_H
#define OPENAPI_INTERNAL_COMPONENTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "openapi/parse/openapi_types.h"
#include <parson.h>
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */

/* --- openapi_schema_registry.c declarations --- */

/**
 * @brief Applies schema ref to param.
 */
extern cdd_c_error_t
apply_schema_ref_to_param(struct OpenAPI_Parameter *out_param,
                          const struct OpenAPI_SchemaRef *schema_ref);

/**
 * @brief Applies schema ref to header.
 */
extern cdd_c_error_t
apply_schema_ref_to_header(struct OpenAPI_Header *out_hdr,
                           const struct OpenAPI_SchemaRef *schema_ref);

/**
 * @brief Executes the schema name in use operation.
 */
extern cdd_c_error_t schema_name_in_use(const struct OpenAPI_Spec *spec,
                                        const char *name);

/**
 * @brief Executes the sanitize component name operation.
 */
extern cdd_c_error_t sanitize_component_name(const char *name, char **_out_val);

/**
 * @brief Executes the make unique schema name operation.
 */
extern cdd_c_error_t make_unique_schema_name(const struct OpenAPI_Spec *spec,
                                             const char *base, char **_out_val);

/**
 * @brief Executes the schema type array includes operation.
 */
extern cdd_c_error_t schema_type_array_includes(const JSON_Array *arr,
                                                const char *type);

/**
 * @brief Executes the schema object is object like operation.
 */
extern cdd_c_error_t
schema_object_is_object_like(const JSON_Object *schema_obj);

/**
 * @brief Executes the append defined schema operation.
 */
extern cdd_c_error_t append_defined_schema(struct OpenAPI_Spec *spec,
                                           char *schema_name,
                                           struct StructFields *schema_fields);

/**
 * @brief Executes the raw schema name exists operation.
 */
extern cdd_c_error_t raw_schema_name_exists(const struct OpenAPI_Spec *spec,
                                            const char *name);

/**
 * @brief Executes the append raw schema operation.
 */
extern cdd_c_error_t append_raw_schema(struct OpenAPI_Spec *spec,
                                       const char *name,
                                       const JSON_Value *schema_val);

/**
 * @brief Executes the register inline schema operation.
 */
extern cdd_c_error_t register_inline_schema(struct OpenAPI_Spec *spec,
                                            const char *base_name,
                                            const JSON_Object *schema_obj,
                                            const JSON_Value *schema_val,
                                            char **out_name);

/**
 * @brief Executes the assign schema ref name operation.
 */
extern cdd_c_error_t
assign_schema_ref_name(struct OpenAPI_SchemaRef *schema_ref, const char *name);

/**
 * @brief Executes the build inline request name operation.
 */
extern cdd_c_error_t build_inline_request_name(const char *op_id, int is_item,
                                               char **_out_val);

/**
 * @brief Executes the build inline response name operation.
 */
extern cdd_c_error_t build_inline_response_name(const char *op_id,
                                                const char *code, int is_item,
                                                char **_out_val);

/**
 * @brief Executes the build inline param name operation.
 */
extern cdd_c_error_t build_inline_param_name(const char *param_name,
                                             char **_out_val);

/* --- openapi_security.c declarations --- */

/**
 * @brief Parses servers from the given input.
 */
extern cdd_c_error_t parse_servers(const JSON_Object *root_obj,
                                   struct OpenAPI_Spec *out);

/**
 * @brief Parses security schemes from the given input.
 */
extern cdd_c_error_t parse_security_schemes(const JSON_Object *components,
                                            struct OpenAPI_Spec *out);

/**
 * @brief Parses security requirements from the given input.
 */
extern cdd_c_error_t
parse_security_requirements(const JSON_Array *arr,
                            struct OpenAPI_SecurityRequirementSet **out,
                            size_t *out_count);

/**
 * @brief Parses security field from the given input.
 */
extern cdd_c_error_t
parse_security_field(const JSON_Object *obj, const char *key,
                     struct OpenAPI_SecurityRequirementSet **out,
                     size_t *out_count, int *out_set);

/* --- openapi_headers_links.c declarations --- */

/**
 * @brief Parses header object from the given input.
 */
extern cdd_c_error_t parse_header_object(const JSON_Object *hdr_obj,
                                         struct OpenAPI_Header *out_hdr,
                                         const struct OpenAPI_Spec *spec,
                                         int resolve_refs);

/**
 * @brief Parses link parameters from the given input.
 */
extern cdd_c_error_t
parse_link_parameters(const JSON_Object *params_obj,
                      struct OpenAPI_LinkParam **out_params, size_t *out_count);

/**
 * @brief Parses link object from the given input.
 */
extern cdd_c_error_t parse_link_object(const JSON_Object *link_obj,
                                       struct OpenAPI_Link *out_link,
                                       const struct OpenAPI_Spec *spec,
                                       int resolve_refs);

/**
 * @brief Parses links object from the given input.
 */
extern cdd_c_error_t parse_links_object(const JSON_Object *links,
                                        struct OpenAPI_Link **out_links,
                                        size_t *out_count,
                                        const struct OpenAPI_Spec *spec,
                                        int resolve_refs);

/**
 * @brief Parses headers object from the given input.
 */
extern cdd_c_error_t parse_headers_object(const JSON_Object *headers,
                                          struct OpenAPI_Header **out_headers,
                                          size_t *out_count,
                                          const struct OpenAPI_Spec *spec,
                                          int resolve_refs,
                                          int ignore_content_type);

/**
 * @brief Parses encoding object from the given input.
 */
extern cdd_c_error_t parse_encoding_object(const JSON_Object *enc_obj,
                                           struct OpenAPI_Encoding *out,
                                           const struct OpenAPI_Spec *spec,
                                           int resolve_refs);

/**
 * @brief Parses encoding map from the given input.
 */
extern cdd_c_error_t parse_encoding_map(const JSON_Object *enc_obj,
                                        struct OpenAPI_Encoding **out,
                                        size_t *out_count,
                                        const struct OpenAPI_Spec *spec,
                                        int resolve_refs);

/**
 * @brief Parses encoding array from the given input.
 */
extern cdd_c_error_t parse_encoding_array(const JSON_Array *enc_arr,
                                          struct OpenAPI_Encoding **out,
                                          size_t *out_count,
                                          const struct OpenAPI_Spec *spec,
                                          int resolve_refs);

/* --- openapi_parameters.c declarations --- */

/**
 * @brief Parses parameter object from the given input.
 */
extern cdd_c_error_t parse_parameter_object(const JSON_Object *p_obj,
                                            struct OpenAPI_Parameter *out_param,
                                            const struct OpenAPI_Spec *spec,
                                            int resolve_refs);

/**
 * @brief Executes the param key equals operation.
 */
extern cdd_c_error_t param_key_equals(const struct OpenAPI_Parameter *a,
                                      const struct OpenAPI_Parameter *b);

/**
 * @brief Parses parameters array from the given input.
 */
extern cdd_c_error_t
parse_parameters_array(const JSON_Array *arr,
                       struct OpenAPI_Parameter **out_params, size_t *out_count,
                       const struct OpenAPI_Spec *spec);

/* --- openapi_media.c declarations --- */

/**
 * @brief Parses media type object from the given input.
 */
extern cdd_c_error_t parse_media_type_object(const JSON_Object *media_obj,
                                             struct OpenAPI_MediaType *out,
                                             const struct OpenAPI_Spec *spec,
                                             int resolve_refs);

/**
 * @brief Executes the media type base len operation.
 */
extern cdd_c_error_t media_type_base_len(const char *name, size_t *_out_val);

/**
 * @brief Executes the media type base equal operation.
 */
extern cdd_c_error_t media_type_base_equal(const char *a, const char *b);

/**
 * @brief Executes the media type is json operation.
 */
extern cdd_c_error_t media_type_is_json(const char *name);

/**
 * @brief Executes the media type specificity operation.
 */
extern cdd_c_error_t media_type_specificity(const char *name, int *out_spec);

/**
 * @brief Executes the media type preference rank operation.
 */
extern cdd_c_error_t media_type_preference_rank(const char *name,
                                                int *out_rank);

/**
 * @brief Executes the select primary media type index operation.
 */
extern cdd_c_error_t
select_primary_media_type_index(const struct OpenAPI_MediaType *mts, size_t n,
                                int *out_idx);

/**
 * @brief Retrieves the media object by name.
 */
extern cdd_c_error_t find_media_object_by_name(const JSON_Object *content,
                                               const char *media_name,
                                               JSON_Object **_out_val);

/**
 * @brief Parses content object from the given input.
 */
extern cdd_c_error_t parse_content_object(const JSON_Object *content,
                                          struct OpenAPI_MediaType **out,
                                          size_t *out_count,
                                          const struct OpenAPI_Spec *spec,
                                          int resolve_refs);

/* --- openapi_responses.c declarations --- */

/**
 * @brief Parses request body object from the given input.
 */
extern cdd_c_error_t parse_request_body_object(
    const JSON_Object *rb_obj, struct OpenAPI_RequestBody *out_rb,
    const struct OpenAPI_Spec *spec, int resolve_refs, const char *op_id);

/**
 * @brief Parses response object from the given input.
 */
extern cdd_c_error_t parse_response_object(const JSON_Object *resp_obj,
                                           struct OpenAPI_Response *out_resp,
                                           const struct OpenAPI_Spec *spec,
                                           int resolve_refs, const char *op_id,
                                           const char *resp_code);

/**
 * @brief Checks if valid response code key.
 */
extern cdd_c_error_t is_valid_response_code_key(const char *code);

/**
 * @brief Parses responses from the given input.
 */
extern cdd_c_error_t parse_responses(const JSON_Object *responses,
                                     struct OpenAPI_Operation *out_op,
                                     const struct OpenAPI_Spec *spec,
                                     const char *op_id);

/**
 * @brief Parses callback object from the given input.
 */
extern cdd_c_error_t parse_callback_object(const JSON_Object *cb_obj,
                                           struct OpenAPI_Callback *out_cb,
                                           const struct OpenAPI_Spec *spec,
                                           int resolve_refs);

/**
 * @brief Parses callbacks object from the given input.
 */
extern cdd_c_error_t parse_callbacks_object(
    const JSON_Object *callbacks, struct OpenAPI_Callback **out_callbacks,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs);

/* --- openapi_components.c declarations --- */

/**
 * @brief Parses component parameters from the given input.
 */
extern cdd_c_error_t parse_component_parameters(const JSON_Object *components,
                                                struct OpenAPI_Spec *out);

/**
 * @brief Parses component responses from the given input.
 */
extern cdd_c_error_t parse_component_responses(const JSON_Object *components,
                                               struct OpenAPI_Spec *out);

/**
 * @brief Parses component headers from the given input.
 */
extern cdd_c_error_t parse_component_headers(const JSON_Object *components,
                                             struct OpenAPI_Spec *out);

/**
 * @brief Parses component request bodies from the given input.
 */
extern cdd_c_error_t
parse_component_request_bodies(const JSON_Object *components,
                               struct OpenAPI_Spec *out);

/**
 * @brief Parses component media types from the given input.
 */
extern cdd_c_error_t parse_component_media_types(const JSON_Object *components,
                                                 struct OpenAPI_Spec *out);

/**
 * @brief Parses component examples from the given input.
 */
extern cdd_c_error_t parse_component_examples(const JSON_Object *components,
                                              struct OpenAPI_Spec *out);

/**
 * @brief Parses component links from the given input.
 */
extern cdd_c_error_t parse_component_links(const JSON_Object *components,
                                           struct OpenAPI_Spec *out);

/**
 * @brief Parses component callbacks from the given input.
 */
extern cdd_c_error_t parse_component_callbacks(const JSON_Object *components,
                                               struct OpenAPI_Spec *out);

/**
 * @brief Parses component path items from the given input.
 */
extern cdd_c_error_t parse_component_path_items(const JSON_Object *components,
                                                struct OpenAPI_Spec *out);

/**
 * @brief Parses components from the given input.
 */
extern cdd_c_error_t parse_components(const JSON_Object *components,
                                      struct OpenAPI_Spec *out);

/* --- openapi_paths.c declarations --- */

/**
 * @brief Parses paths object from the given input.
 */
extern cdd_c_error_t parse_paths_object(const JSON_Object *paths_obj,
                                        struct OpenAPI_Path **out_paths,
                                        size_t *out_count,
                                        const struct OpenAPI_Spec *spec,
                                        int require_leading_slash,
                                        int resolve_refs);

/**
 * @brief Executes the name in list operation.
 */
extern cdd_c_error_t name_in_list(const char *name, char **names, size_t count);

/**
 * @brief Collects path template names.
 */
extern cdd_c_error_t collect_path_template_names(const char *route,
                                                 char ***out_names,
                                                 size_t *out_count);

/**
 * @brief Retrieves the path param.
 */
extern cdd_c_error_t find_path_param(const struct OpenAPI_Parameter *params,
                                     size_t n, const char *name,
                                     struct OpenAPI_Parameter **_out_val);

/**
 * @brief Executes the validate path params list operation.
 */
extern cdd_c_error_t
validate_path_params_list(const struct OpenAPI_Parameter *params,
                          size_t n_params, char **template_names,
                          size_t n_template_names);

/**
 * @brief Executes the validate path template for operation operation.
 */
extern cdd_c_error_t validate_path_template_for_operation(
    const struct OpenAPI_Path *path, const struct OpenAPI_Operation *op,
    char **template_names, size_t n_template_names);

/**
 * @brief Executes the validate path templates operation.
 */
extern cdd_c_error_t validate_path_templates(const struct OpenAPI_Path *paths,
                                             size_t n_paths);

/**
 * @brief Executes the normalize path template route operation.
 */
extern cdd_c_error_t normalize_path_template_route(const char *route,
                                                   char **_out_val);

/**
 * @brief Executes the validate path template collisions operation.
 */
extern cdd_c_error_t
validate_path_template_collisions(const struct OpenAPI_Path *paths,
                                  size_t n_paths);

/**
 * @brief Parses operation from the given input.
 */
extern cdd_c_error_t parse_operation(const char *verb_str,
                                     const JSON_Object *op_obj,
                                     struct OpenAPI_Operation *out_op,
                                     const struct OpenAPI_Spec *spec,
                                     int is_additional, const char *route_hint);

/**
 * @brief Parses additional operations from the given input.
 */
extern cdd_c_error_t
parse_additional_operations(const JSON_Object *path_obj,
                            struct OpenAPI_Path *path,
                            const struct OpenAPI_Spec *spec);

/* --- openapi_validation.c declarations --- */

/**
 * @brief Executes the scan querystring usage operation.
 */
extern cdd_c_error_t
scan_querystring_usage(const struct OpenAPI_Parameter *params, size_t n_params,
                       size_t *qs_count, int *has_query);

/**
 * @brief Executes the validate querystring usage operation.
 */
extern cdd_c_error_t
validate_querystring_usage(const struct OpenAPI_Path *paths, size_t n_paths);

/**
 * @brief Executes the validate querystring usage in callbacks operation.
 */
extern cdd_c_error_t validate_querystring_usage_in_callbacks(
    const struct OpenAPI_Callback *callbacks, size_t n_callbacks);

/**
 * @brief Executes the validate querystring usage in operations
 * operation.
 */
extern cdd_c_error_t
validate_querystring_usage_in_operations(const struct OpenAPI_Operation *ops,
                                         size_t n_ops);

/**
 * @brief Executes the validate querystring usage in paths callbacks
 * operation.
 */
extern cdd_c_error_t
validate_querystring_usage_in_paths_callbacks(const struct OpenAPI_Path *paths,
                                              size_t n_paths);

/**
 * @brief Executes the validate querystring usage in component callbacks
 * operation.
 */
extern cdd_c_error_t validate_querystring_usage_in_component_callbacks(
    const struct OpenAPI_Spec *spec);

/**
 * @brief Adds or sets unique operation id.
 */
extern cdd_c_error_t add_unique_operation_id(char ***ids, size_t *count,
                                             size_t *cap, const char *op_id);

/**
 * @brief Collects operation ids.
 */
extern cdd_c_error_t collect_operation_ids(const struct OpenAPI_Path *paths,
                                           size_t n_paths, char ***ids,
                                           size_t *count, size_t *cap);

/**
 * @brief Executes the path item ref matches component operation.
 */
extern cdd_c_error_t
path_item_ref_matches_component(const struct OpenAPI_Spec *spec,
                                const char *ref, const char *name);

/**
 * @brief Executes the component path item is referenced operation.
 */
extern cdd_c_error_t
component_path_item_is_referenced(const struct OpenAPI_Spec *spec,
                                  const char *name);

/**
 * @brief Executes the callback ref matches component operation.
 */
extern cdd_c_error_t
callback_ref_matches_component(const struct OpenAPI_Spec *spec, const char *ref,
                               const char *name);

/**
 * @brief Executes the component callback is referenced in ops operation.
 */
extern cdd_c_error_t component_callback_is_referenced_in_ops(
    const struct OpenAPI_Operation *ops, size_t n_ops,
    const struct OpenAPI_Spec *spec, const char *name);

/**
 * @brief Executes the component callback is referenced operation.
 */
extern cdd_c_error_t
component_callback_is_referenced(const struct OpenAPI_Spec *spec,
                                 const char *name);

/**
 * @brief Collects callback operation ids from callbacks.
 */
extern cdd_c_error_t collect_callback_operation_ids_from_callbacks(
    const struct OpenAPI_Callback *callbacks, size_t n_callbacks, char ***ids,
    size_t *count, size_t *cap);

/**
 * @brief Collects callback operation ids from operations.
 */
extern cdd_c_error_t collect_callback_operation_ids_from_operations(
    const struct OpenAPI_Operation *ops, size_t n_ops, char ***ids,
    size_t *count, size_t *cap);

/**
 * @brief Collects callback operation ids from paths.
 */
extern cdd_c_error_t
collect_callback_operation_ids_from_paths(const struct OpenAPI_Path *paths,
                                          size_t n_paths, char ***ids,
                                          size_t *count, size_t *cap);

/**
 * @brief Executes the validate unique operation ids operation.
 */
extern cdd_c_error_t
validate_unique_operation_ids(const struct OpenAPI_Spec *spec);

/* --- openapi.c declarations --- */

/**
 * @brief Executes the openapi load from json internal operation.
 */
extern cdd_c_error_t openapi_load_from_json_internal(
    const JSON_Value *root, struct OpenAPI_Spec *out, const char *retrieval_uri,
    struct OpenAPI_DocRegistry *registry);

/**
 * @brief Executes the openapi load from json operation.
 */
extern cdd_c_error_t openapi_load_from_json(const JSON_Value *root,
                                            struct OpenAPI_Spec *out);

/**
 * @brief Executes the openapi load from json with context operation.
 */
extern cdd_c_error_t openapi_load_from_json_with_context(
    const JSON_Value *root, const char *retrieval_uri, struct OpenAPI_Spec *out,
    struct OpenAPI_DocRegistry *registry);

/**
 * @brief Executes the openapi spec find schema operation.
 */
extern cdd_c_error_t openapi_spec_find_schema(const struct OpenAPI_Spec *spec,
                                              const char *name,
                                              struct StructFields **_out_val);

/**
 * @brief Executes the openapi spec find schema by id operation.
 */
extern cdd_c_error_t
openapi_spec_find_schema_by_id(const struct OpenAPI_Spec *spec, const char *ref,
                               struct StructFields **_out_val);

/**
 * @brief Executes the openapi spec find schema by anchor operation.
 */
extern cdd_c_error_t
openapi_spec_find_schema_by_anchor(const struct OpenAPI_Spec *spec,
                                   const char *ref, int dynamic_anchor,
                                   struct StructFields **_out_val);

/**
 * @brief Executes the openapi spec find schema for ref operation.
 */
extern cdd_c_error_t
openapi_spec_find_schema_for_ref(const struct OpenAPI_Spec *spec,
                                 const struct OpenAPI_SchemaRef *ref,
                                 struct StructFields **_out_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENAPI_INTERNAL_COMPONENTS_H */
