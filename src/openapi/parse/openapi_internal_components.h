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
 */
extern C_CDD_EXPORT cdd_c_error_t parse_example_object(
    const JSON_Object *ex_obj, const char *name, struct OpenAPI_Example *out_ex,
    const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses examples object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_examples_object(
    const JSON_Object *examples_obj, struct OpenAPI_Example **out,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses media examples from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_media_examples(
    const JSON_Object *media_obj, struct OpenAPI_Any *example, int *example_set,
    struct OpenAPI_Example **examples, size_t *n_examples,
    const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses external docs from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_external_docs(
    const JSON_Object *ext_obj, struct OpenAPI_ExternalDocs *out);

/**
 * @brief Parses discriminator object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_discriminator_object(
    const JSON_Object *disc_obj, struct OpenAPI_Discriminator *out);

/**
 * @brief Parses xml object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_xml_object(const JSON_Object *xml_obj,
                                                   struct OpenAPI_Xml *out);

/**
 * @brief Parses info from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_info(const JSON_Object *root_obj,
                                             struct OpenAPI_Spec *out);

/**
 * @brief Parses tags from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_tags(const JSON_Object *root_obj,
                                             struct OpenAPI_Spec *out);

/**
 * @brief Parses server object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_server_object(const JSON_Object *srv_obj, struct OpenAPI_Server *out_srv);

/**
 * @brief Parses servers array from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_servers_array(const JSON_Object *parent, const char *key,
                    struct OpenAPI_Server **out_servers, size_t *out_count);

/**
 * @brief Parses servers from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_servers(const JSON_Object *root_obj,
                                                struct OpenAPI_Spec *out);

/**
 * @brief Parses oauth scopes from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_oauth_scopes(const JSON_Object *scopes_obj,
                   struct OpenAPI_OAuthScope **out, size_t *out_count);

/**
 * @brief Parses oauth flows from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_oauth_flows(
    const JSON_Object *flows_obj, struct OpenAPI_SecurityScheme *out);

/**
 * @brief Parses security schemes from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_security_schemes(const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses security requirements from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_security_requirements(
    const JSON_Array *arr, struct OpenAPI_SecurityRequirementSet **out,
    size_t *out_count);

/**
 * @brief Parses security field from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_security_field(const JSON_Object *parent, const char *key,
                     struct OpenAPI_SecurityRequirementSet **out,
                     size_t *out_count, int *out_set);

/**
 * @brief Parses parameter object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_parameter_object(
    const JSON_Object *p_obj, struct OpenAPI_Parameter *out_param,
    struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Checks whether two OpenAPI parameters have the same name and location.
 */
extern C_CDD_EXPORT cdd_c_error_t param_key_equals(
    const struct OpenAPI_Parameter *a, const struct OpenAPI_Parameter *b);

/**
 * @brief Parses parameters array from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_parameters_array(
    const JSON_Array *arr, struct OpenAPI_Parameter **out_params,
    size_t *out_count, struct OpenAPI_Spec *spec);

/**
 * @brief Parses header object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_header_object(const JSON_Object *hdr_obj, struct OpenAPI_Header *out_hdr,
                    const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses headers object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_headers_object(
    const JSON_Object *headers, struct OpenAPI_Header **out_headers,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs,
    int ignore_content_type);

/**
 * @brief Parses link parameters from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_link_parameters(const JSON_Object *params_obj,
                      struct OpenAPI_LinkParam **out_params, size_t *out_count);

/**
 * @brief Parses link object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_link_object(const JSON_Object *link_obj, struct OpenAPI_Link *out_link,
                  const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses links object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_links_object(
    const JSON_Object *links, struct OpenAPI_Link **out_links,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses request body object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_request_body_object(
    const JSON_Object *rb_obj, struct OpenAPI_RequestBody *out_rb,
    struct OpenAPI_Spec *spec, int resolve_refs, const char *op_id);

/**
 * @brief Parses response object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_response_object(
    const JSON_Object *resp_obj, struct OpenAPI_Response *out_resp,
    struct OpenAPI_Spec *spec, int resolve_refs, const char *op_id,
    const char *resp_code);

/**
 * @brief Parses responses from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_responses(const JSON_Object *responses, struct OpenAPI_Operation *out_op,
                struct OpenAPI_Spec *spec, const char *op_id);

/**
 * @brief Parses callback object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_callback_object(
    const JSON_Object *cb_obj, struct OpenAPI_Callback *out_cb,
    struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses callbacks object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_callbacks_object(
    const JSON_Object *callbacks, struct OpenAPI_Callback **out_callbacks,
    size_t *out_count, struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses operation from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_operation(const char *verb_str, const JSON_Object *op_obj,
                struct OpenAPI_Operation *out_op, struct OpenAPI_Spec *spec,
                int is_additional, const char *route_hint);

/**
 * @brief Parses additional operations from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_additional_operations(
    const JSON_Object *path_obj, struct OpenAPI_Path *path,
    struct OpenAPI_Spec *spec);

/**
 * @brief Parses path item object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_path_item_object(
    const char *route, const JSON_Object *path_obj, struct OpenAPI_Path *curr,
    const struct OpenAPI_Spec *spec, int require_leading_slash,
    int resolve_refs);

/**
 * @brief Parses paths object from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_paths_object(
    const JSON_Object *paths_obj, struct OpenAPI_Path **out_paths,
    size_t *out_count, struct OpenAPI_Spec *spec, int require_leading_slash,
    int resolve_refs);

/**
 * @brief Parses component parameters from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_parameters(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component responses from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_responses(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component headers from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_headers(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component request bodies from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_request_bodies(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component media types from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_media_types(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component examples from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_examples(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component links from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_component_links(const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component callbacks from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_callbacks(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses component path items from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_component_path_items(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses components from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_components(const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Parses a Parson JSON object into an OpenAPI_Spec with document
 * context.
 */
extern C_CDD_EXPORT cdd_c_error_t openapi_load_from_json_internal(
    const JSON_Value *root, struct OpenAPI_Spec *out, const char *retrieval_uri,
    struct OpenAPI_DocRegistry *registry);

/**
 * @brief Looks up a schema in an OpenAPI_Spec by its JSON Schema $id URI.
 */
extern C_CDD_EXPORT cdd_c_error_t
openapi_spec_find_schema_by_id(const struct OpenAPI_Spec *spec, const char *ref,
                               struct StructFields **_out_val);

/**
 * @brief Looks up a schema in an OpenAPI_Spec by its JSON Schema $anchor.
 */
extern C_CDD_EXPORT cdd_c_error_t openapi_spec_find_schema_by_anchor(
    const struct OpenAPI_Spec *spec, const char *ref, int dynamic_anchor,
    struct StructFields **_out_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENAPI_INTERNAL_COMPONENTS_H */
