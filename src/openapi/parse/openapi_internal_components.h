/**
 * @file openapi_internal_components.h
 * @brief Internal component, operation, and metadata parsing declarations for
 * OpenAPI parser modules.
 * @author Samuel Marks
 */

#ifndef OPENAPI_INTERNAL_COMPONENTS_H
#define OPENAPI_INTERNAL_COMPONENTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <parson.h>
#include <stddef.h>
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "openapi/parse/openapi_types.h"
/* clang-format on */

/**
 * @brief Parses example object from the given input.
 *
 * @param[in] ex_obj The source JSON example object.
 * @param[in] name The example key name.
 * @param[out] out_ex Output OpenAPI_Example structure.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_example_object(
    const JSON_Object *ex_obj, const char *name, struct OpenAPI_Example *out_ex,
    const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses examples object from the given input.
 *
 * @param[in] examples_obj The source JSON examples map object.
 * @param[out] out Output array of OpenAPI_Example structures.
 * @param[out] out_count Output number of parsed examples.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_examples_object(
    const JSON_Object *examples_obj, struct OpenAPI_Example **out,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses media examples from the given input.
 *
 * @param[in] media_obj The source media type JSON object.
 * @param[out] example Output single example value.
 * @param[out] example_set Output flag set to 1 if single example is present.
 * @param[out] examples Output array of named examples.
 * @param[out] n_examples Output count of named examples.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_media_examples(
    const JSON_Object *media_obj, struct OpenAPI_Any *example, int *example_set,
    struct OpenAPI_Example **examples, size_t *n_examples,
    const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses external docs from the given input.
 *
 * @param[in] ext_obj The source externalDocs JSON object.
 * @param[out] out Output OpenAPI_ExternalDocs structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_external_docs(
    const JSON_Object *ext_obj, struct OpenAPI_ExternalDocs *out);

/**
 * @brief Parses discriminator object from the given input.
 *
 * @param[in] disc_obj The source discriminator JSON object.
 * @param[out] out Output OpenAPI_Discriminator structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_discriminator_object(
    const JSON_Object *disc_obj, struct OpenAPI_Discriminator *out);

/**
 * @brief Parses xml object from the given input.
 *
 * @param[in] xml_obj The source XML JSON object.
 * @param[out] out Output OpenAPI_Xml structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_xml_object(const JSON_Object *xml_obj,
                                                   struct OpenAPI_Xml *out);

/**
 * @brief Parses info from the given input.
 *
 * @param[in] root_obj The root OpenAPI JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_info(const JSON_Object *root_obj,
                                             struct OpenAPI_Spec *out);

/**
 * @brief Parses tags from the given input.
 *
 * @param[in] root_obj The root OpenAPI JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_tags(const JSON_Object *root_obj,
                                             struct OpenAPI_Spec *out);

/**
 * @brief Parses server object from the given input.
 *
 * @param[in] srv_obj The source server JSON object.
 * @param[out] out_srv Output OpenAPI_Server structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_server_object(const JSON_Object *srv_obj, struct OpenAPI_Server *out_srv);

/**
 * @brief Parses servers array from the given input.
 *
 * @param[in] parent The parent JSON object.
 * @param[in] key The servers array key name.
 * @param[out] out_servers Output array of OpenAPI_Server structures.
 * @param[out] out_count Output count of servers.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_servers_array(const JSON_Object *parent, const char *key,
                    struct OpenAPI_Server **out_servers, size_t *out_count);

/**
 * @brief Parses servers from the given input.
 *
 * @param[in] root_obj The root OpenAPI JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_servers(const JSON_Object *root_obj,
                                                struct OpenAPI_Spec *out);

/**
 * @brief Parses oauth scopes from the given input.
 *
 * @param[in] scopes_obj The source scopes JSON object.
 * @param[out] out Output array of OpenAPI_OAuthScope structures.
 * @param[out] out_count Output count of scopes.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_oauth_scopes(const JSON_Object *scopes_obj,
                   struct OpenAPI_OAuthScope **out, size_t *out_count);

/**
 * @brief Parses oauth flows from the given input.
 *
 * @param[in] flows_obj The source OAuth flows JSON object.
 * @param[in,out] out The target OpenAPI_SecurityScheme structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_oauth_flows(
    const JSON_Object *flows_obj, struct OpenAPI_SecurityScheme *out);

/**
 * @brief Parses security schemes from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_security_schemes(const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses security requirements from the given input.
 *
 * @param[in] arr The source security requirements JSON array.
 * @param[out] out Output array of OpenAPI_SecurityRequirementSet structures.
 * @param[out] out_count Output count of requirement sets.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_security_requirements(
    const JSON_Array *arr, struct OpenAPI_SecurityRequirementSet **out,
    size_t *out_count);

/**
 * @brief Parses security field from the given input.
 *
 * @param[in] parent The parent JSON object.
 * @param[in] key The security property key name.
 * @param[out] out Output array of OpenAPI_SecurityRequirementSet structures.
 * @param[out] out_count Output count of requirement sets.
 * @param[out] out_set Output flag set to 1 if security field was present.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_security_field(const JSON_Object *parent, const char *key,
                     struct OpenAPI_SecurityRequirementSet **out,
                     size_t *out_count, int *out_set);

/**
 * @brief Parses parameter object from the given input.
 *
 * @param[in] p_obj The source parameter JSON object.
 * @param[out] out_param Output OpenAPI_Parameter structure.
 * @param[in,out] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_parameter_object(
    const JSON_Object *p_obj, struct OpenAPI_Parameter *out_param,
    struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Checks whether two OpenAPI parameters have the same name and location.
 *
 * @param[in] a First OpenAPI parameter.
 * @param[in] b Second OpenAPI parameter.
 * @return Non-zero if equal, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t param_key_equals(
    const struct OpenAPI_Parameter *a, const struct OpenAPI_Parameter *b);

/**
 * @brief Parses parameters array from the given input.
 *
 * @param[in] arr The source parameters JSON array.
 * @param[out] out_params Output array of OpenAPI_Parameter structures.
 * @param[out] out_count Output count of parameters.
 * @param[in,out] spec The parent OpenAPI specification.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_parameters_array(
    const JSON_Array *arr, struct OpenAPI_Parameter **out_params,
    size_t *out_count, struct OpenAPI_Spec *spec);

/**
 * @brief Parses header object from the given input.
 *
 * @param[in] hdr_obj The source header JSON object.
 * @param[out] out_hdr Output OpenAPI_Header structure.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_header_object(const JSON_Object *hdr_obj, struct OpenAPI_Header *out_hdr,
                    const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses headers object from the given input.
 *
 * @param[in] headers The source headers map JSON object.
 * @param[out] out_headers Output array of OpenAPI_Header structures.
 * @param[out] out_count Output count of headers.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @param[in] ignore_content_type Non-zero to skip Content-Type headers.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_headers_object(
    const JSON_Object *headers, struct OpenAPI_Header **out_headers,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs,
    int ignore_content_type);

/**
 * @brief Parses link parameters from the given input.
 *
 * @param[in] params_obj The source link parameters JSON object.
 * @param[out] out_params Output array of OpenAPI_LinkParam structures.
 * @param[out] out_count Output count of link parameters.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_link_parameters(const JSON_Object *params_obj,
                      struct OpenAPI_LinkParam **out_params, size_t *out_count);

/**
 * @brief Parses link object from the given input.
 *
 * @param[in] link_obj The source link JSON object.
 * @param[out] out_link Output OpenAPI_Link structure.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_link_object(const JSON_Object *link_obj, struct OpenAPI_Link *out_link,
                  const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses links object from the given input.
 *
 * @param[in] links The source links map JSON object.
 * @param[out] out_links Output array of OpenAPI_Link structures.
 * @param[out] out_count Output count of links.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_links_object(
    const JSON_Object *links, struct OpenAPI_Link **out_links,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses request body object from the given input.
 *
 * @param[in] rb_obj The source request body JSON object.
 * @param[out] out_rb Output OpenAPI_RequestBody structure.
 * @param[in,out] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @param[in] op_id Optional operationId for inline schema naming.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_request_body_object(
    const JSON_Object *rb_obj, struct OpenAPI_RequestBody *out_rb,
    struct OpenAPI_Spec *spec, int resolve_refs, const char *op_id);

/**
 * @brief Parses response object from the given input.
 *
 * @param[in] resp_obj The source response JSON object.
 * @param[out] out_resp Output OpenAPI_Response structure.
 * @param[in,out] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @param[in] op_id Optional operationId for inline schema naming.
 * @param[in] resp_code Optional HTTP status code string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_response_object(
    const JSON_Object *resp_obj, struct OpenAPI_Response *out_resp,
    struct OpenAPI_Spec *spec, int resolve_refs, const char *op_id,
    const char *resp_code);

/**
 * @brief Parses responses from the given input.
 *
 * @param[in] responses The source responses map JSON object.
 * @param[in,out] out_op The target OpenAPI_Operation structure.
 * @param[in,out] spec The parent OpenAPI specification.
 * @param[in] op_id Optional operationId for inline schema naming.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_responses(const JSON_Object *responses, struct OpenAPI_Operation *out_op,
                struct OpenAPI_Spec *spec, const char *op_id);

/**
 * @brief Parses callback object from the given input.
 *
 * @param[in] cb_obj The source callback JSON object.
 * @param[out] out_cb Output OpenAPI_Callback structure.
 * @param[in,out] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_callback_object(
    const JSON_Object *cb_obj, struct OpenAPI_Callback *out_cb,
    struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses callbacks object from the given input.
 *
 * @param[in] callbacks The source callbacks map JSON object.
 * @param[out] out_callbacks Output array of OpenAPI_Callback structures.
 * @param[out] out_count Output count of callbacks.
 * @param[in,out] spec The parent OpenAPI specification.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_callbacks_object(
    const JSON_Object *callbacks, struct OpenAPI_Callback **out_callbacks,
    size_t *out_count, struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses operation from the given input.
 *
 * @param[in] verb_str The HTTP method string.
 * @param[in] op_obj The source operation JSON object.
 * @param[out] out_op Output OpenAPI_Operation structure.
 * @param[in,out] spec The parent OpenAPI specification.
 * @param[in] is_additional Non-zero if parsed from additionalOperations.
 * @param[in] route_hint Route path string used for fallback naming.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_operation(const char *verb_str, const JSON_Object *op_obj,
                struct OpenAPI_Operation *out_op, struct OpenAPI_Spec *spec,
                int is_additional, const char *route_hint);

/**
 * @brief Parses additional operations from the given input.
 *
 * @param[in] path_obj The source path item JSON object.
 * @param[in,out] path The target OpenAPI_Path structure.
 * @param[in,out] spec The parent OpenAPI specification.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_additional_operations(
    const JSON_Object *path_obj, struct OpenAPI_Path *path,
    struct OpenAPI_Spec *spec);

/**
 * @brief Parses path item object from the given input.
 *
 * @param[in] route The route path string.
 * @param[in] path_obj The source path item JSON object.
 * @param[out] curr Output OpenAPI_Path structure.
 * @param[in] spec The parent OpenAPI specification.
 * @param[in] require_leading_slash Non-zero if route must begin with '/'.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_path_item_object(
    const char *route, const JSON_Object *path_obj, struct OpenAPI_Path *curr,
    const struct OpenAPI_Spec *spec, int require_leading_slash,
    int resolve_refs);

/**
 * @brief Parses paths object from the given input.
 *
 * @param[in] paths_obj The source paths map JSON object.
 * @param[out] out_paths Output array of OpenAPI_Path structures.
 * @param[out] out_count Output count of paths.
 * @param[in,out] spec The parent OpenAPI specification.
 * @param[in] require_leading_slash Non-zero if routes must begin with '/'.
 * @param[in] resolve_refs Non-zero to resolve component references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_paths_object(
    const JSON_Object *paths_obj, struct OpenAPI_Path **out_paths,
    size_t *out_count, struct OpenAPI_Spec *spec, int require_leading_slash,
    int resolve_refs);

/**
 * @brief Parses component parameters from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_parameters(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component responses from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_responses(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component headers from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_headers(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component request bodies from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_request_bodies(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component media types from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_media_types(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component examples from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_examples(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component links from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_component_links(const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component callbacks from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_callbacks(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component path items from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_path_items(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses components from the given input.
 *
 * @param[in] components The components JSON object.
 * @param[in,out] out The target OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_components(const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses a Parson JSON object into an OpenAPI_Spec with document
 * context.
 *
 * @param[in] root The root Parson JSON_Value.
 * @param[out] out The target OpenAPI_Spec structure.
 * @param[in] retrieval_uri Optional document retrieval URI.
 * @param[in,out] registry Optional document registry for multi-file resolution.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t openapi_load_from_json_internal(
    const JSON_Value *root, struct OpenAPI_Spec *out, const char *retrieval_uri,
    struct OpenAPI_DocRegistry *registry);

/**
 * @brief Looks up a schema in an OpenAPI_Spec by its JSON Schema $id URI.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The $id reference string.
 * @param[out] _out_val Output pointer to the found StructFields.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
openapi_spec_find_schema_by_id(const struct OpenAPI_Spec *spec, const char *ref,
                               struct StructFields **_out_val);

/**
 * @brief Looks up a schema in an OpenAPI_Spec by its JSON Schema $anchor.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The anchor reference string.
 * @param[in] dynamic_anchor Non-zero to search $dynamicAnchor instead of
 * $anchor.
 * @param[out] _out_val Output pointer to the found StructFields.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t openapi_spec_find_schema_by_anchor(
    const struct OpenAPI_Spec *spec, const char *ref, int dynamic_anchor,
    struct StructFields **_out_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENAPI_INTERNAL_COMPONENTS_H */
