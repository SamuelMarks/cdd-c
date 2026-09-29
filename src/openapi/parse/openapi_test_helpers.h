/**
 * @file openapi_test_helpers.h
 * @brief Internal test helper declarations for OpenAPI parser unit tests.
 * @author Samuel Marks
 */

#ifndef OPENAPI_TEST_HELPERS_H
#define OPENAPI_TEST_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "openapi/parse/openapi_types.h"
#include <parson.h>
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS

/**
 * @brief cdd test collect extensions.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_collect_extensions(const JSON_Object *obj, char **out_json);

/**
 * @brief cdd test collect schema extras.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_collect_schema_extras(const JSON_Object *obj, const char **skip_keys,
                               size_t n_skip, char **out_json);

/**
 * @brief cdd test detect tag cycle.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_detect_tag_cycle(
    const struct OpenAPI_Spec *spec, size_t idx, int *state);

/**
 * @brief cdd test example fields valid.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_example_fields_valid(const struct OpenAPI_Example *ex);

/**
 * @brief cdd test find path param.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_find_path_param(const struct OpenAPI_Parameter *params, size_t n,
                         const char *name, struct OpenAPI_Parameter **out_val);

/**
 * @brief cdd test free header.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_header(struct OpenAPI_Header *hdr);

/**
 * @brief cdd test free operation.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_operation(struct OpenAPI_Operation *op);

/**
 * @brief cdd test free parameter.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_parameter(struct OpenAPI_Parameter *param);

/**
 * @brief cdd test header name is content type.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_header_name_is_content_type(const char *name);

/**
 * @brief cdd test is fixed operation method.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_is_fixed_operation_method(const char *method);

/**
 * @brief cdd test is valid response code key.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_is_valid_response_code_key(const char *code);

/**
 * @brief cdd test json pointer unescape.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_json_pointer_unescape(const char *in, char **out_val);

/**
 * @brief cdd test media type base equal.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_media_type_base_equal(const char *a,
                                                                 const char *b);

/**
 * @brief cdd test media type base len.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_media_type_base_len(const char *name,
                                                               size_t *out_val);

/**
 * @brief cdd test media type is json.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_media_type_is_json(const char *name);

/**
 * @brief cdd test normalize path.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_normalize_path(const char *path,
                                                          char **out_val);

/**
 * @brief cdd test normalize path template route.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_normalize_path_template_route(const char *route, char **out_val);

/**
 * @brief cdd test object has example and examples.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_object_has_example_and_examples(const JSON_Object *obj);

/**
 * @brief cdd test openapi version supported.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_openapi_version_supported(const char *version);

/**
 * @brief cdd test param key equals.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_param_key_equals(
    const struct OpenAPI_Parameter *a, const struct OpenAPI_Parameter *b);

/**
 * @brief cdd test param type is primitive.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_param_type_is_primitive(const char *type);

/**
 * @brief cdd test parse info.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_info(const JSON_Object *root_obj, struct OpenAPI_Spec *out);

/**
 * @brief cdd test parse operation.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_operation(const char *verb_str, const JSON_Object *op_obj,
                         struct OpenAPI_Operation *out_op,
                         const struct OpenAPI_Spec *spec, int is_additional);

/**
 * @brief cdd test parse param in.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_param_in(const char *in, enum OpenAPI_ParamIn *out_val);

/**
 * @brief cdd test parse param style.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_param_style(const char *s, enum OpenAPI_Style *out_val);

/**
 * @brief cdd test parse schema type.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_schema_type(
    const JSON_Object *schema, int *out_nullable, char **out_val);

/**
 * @brief cdd test parse security in.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_security_in(const char *in, enum OpenAPI_SecurityIn *out_val);

/**
 * @brief cdd test parse security type.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_security_type(
    const char *type, enum OpenAPI_SecurityType *out_val);

/**
 * @brief cdd test parse xml node type.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_xml_node_type(
    const char *node_type, enum OpenAPI_XmlNodeType *out_val);

/**
 * @brief cdd test ref base matches self.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_ref_base_matches_self(
    const struct OpenAPI_Spec *spec, const char *ref, const char *hash);

/**
 * @brief cdd test ref name from prefix.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_ref_name_from_prefix(const struct OpenAPI_Spec *spec, const char *ref,
                              const char *prefix, char **out_val);

/**
 * @brief cdd test resolve uri reference.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_resolve_uri_reference(
    const char *base_uri, const char *ref, char **out_val);

/**
 * @brief cdd test root has openapi fields.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_root_has_openapi_fields(const JSON_Object *root_obj);

/**
 * @brief cdd test root is schema document.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_root_is_schema_document(
    const JSON_Value *root, const JSON_Object *root_obj);

/**
 * @brief cdd test scan querystring usage.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_scan_querystring_usage(
    const struct OpenAPI_Parameter *params, size_t n_params,
    size_t *out_qs_count, int *out_has_query);

/**
 * @brief cdd test schema has composition.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_schema_has_composition(const JSON_Object *schema_obj);

/**
 * @brief cdd test schema name in use.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_schema_name_in_use(const struct OpenAPI_Spec *spec, const char *name);

/**
 * @brief cdd test server variable defined.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_server_variable_defined(
    const struct OpenAPI_Server *srv, const char *name);

/**
 * @brief cdd test server variable seen.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_server_variable_seen(char **seen, size_t seen_count, const char *name);

/**
 * @brief cdd test tag index by name.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_tag_index_by_name(
    const struct OpenAPI_Spec *spec, const char *name, size_t *out_idx);

/**
 * @brief cdd test uri has scheme prefix.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_uri_has_scheme_prefix(const char *uri, size_t len);

/**
 * @brief cdd test url has query or fragment.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_url_has_query_or_fragment(const char *url);

/**
 * @brief cdd test validate media type key map.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_media_type_key_map(const JSON_Object *obj);

/**
 * @brief cdd test validate parameter style.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_validate_parameter_style(
    const struct OpenAPI_Parameter *p, int has_content);

/**
 * @brief cdd test validate querystring usage.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_validate_querystring_usage(
    const struct OpenAPI_Path *paths, size_t n_paths);

/**
 * @brief cdd test validate tag parents.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_tag_parents(const struct OpenAPI_Spec *spec);

/**
 * @brief Test helper to copy parameter fields.
 *
 * @param[out] dst Destination parameter.
 * @param[in] src Source parameter.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_parameter_fields(
    struct OpenAPI_Parameter *dst, const struct OpenAPI_Parameter *src);

/**
 * @brief Test helper to copy header fields.
 *
 * @param[out] dst Destination header.
 * @param[in] src Source header.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_header_fields(
    struct OpenAPI_Header *dst, const struct OpenAPI_Header *src);

/**
 * @brief Test helper to copy encoding fields.
 *
 * @param[out] dst Destination encoding.
 * @param[in] src Source encoding.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_encoding_fields(
    struct OpenAPI_Encoding *dst, const struct OpenAPI_Encoding *src);

/**
 * @brief Test helper to copy media type fields.
 *
 * @param[out] dst Destination media type.
 * @param[in] src Source media type.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_media_type_fields(
    struct OpenAPI_MediaType *dst, const struct OpenAPI_MediaType *src);

/**
 * @brief Test helper to copy response fields.
 *
 * @param[out] dst Destination response.
 * @param[in] src Source response.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_response_fields(
    struct OpenAPI_Response *dst, const struct OpenAPI_Response *src);

/**
 * @brief Test helper to copy operation fields.
 *
 * @param[out] dst Destination operation.
 * @param[in] src Source operation.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_operation_fields(
    struct OpenAPI_Operation *dst, const struct OpenAPI_Operation *src);

/**
 * @brief Test helper to copy schema ref.
 *
 * @param[out] dst Destination schema ref.
 * @param[in] src Source schema ref.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_schema_ref(
    struct OpenAPI_SchemaRef *dst, const struct OpenAPI_SchemaRef *src);

/**
 * @brief Test helper to copy example fields.
 *
 * @param[out] dst Destination example.
 * @param[in] src Source example.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_example_fields(
    struct OpenAPI_Example *dst, const struct OpenAPI_Example *src);

/**
 * @brief Test helper to free encoding.
 *
 * @param[in,out] enc Encoding to free.
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_encoding(struct OpenAPI_Encoding *enc);

/**
 * @brief Test helper to free media type.
 *
 * @param[in,out] mt Media type to free.
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_media_type(struct OpenAPI_MediaType *mt);

/**
 * @brief Test helper to free response.
 *
 * @param[in,out] resp Response to free.
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_response(struct OpenAPI_Response *resp);

/**
 * @brief Test helper to free request body.
 *
 * @param[in,out] rb Request body to free.
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_request_body(struct OpenAPI_RequestBody *rb);

/**
 * @brief Test helper to free any value.
 *
 * @param[in,out] val Any value to free.
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_any_value(struct OpenAPI_Any *val);

/**
 * @brief Test helper to free link.
 *
 * @param[in,out] link Link to free.
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_free_link(struct OpenAPI_Link *link);

/**
 * @brief Test helper to free security requirement.
 *
 * @param[in,out] req Security requirement to free.
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_security_requirement(struct OpenAPI_SecurityRequirement *req);

/**
 * @brief Test helper to free path item.
 *
 * @param[in,out] p Path item to free.
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_path_item(struct OpenAPI_Path *p);

/**
 * @brief Test helper to free callback.
 *
 * @param[in,out] cb Callback to free.
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_callback(struct OpenAPI_Callback *cb);

/**
 * @brief Test helper to free string array.
 *
 * @param[in,out] arr String array to free.
 * @param[in] n Array size.
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_free_string_array(char **arr,
                                                             size_t n);

/**
 * @brief Test helper to free example.
 *
 * @param[in,out] ex Example to free.
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_example(struct OpenAPI_Example *ex);

/**
 * @brief Test helper to free schema ref content.
 *
 * @param[in,out] ref Schema ref to free.
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_free_schema_ref_content(struct OpenAPI_SchemaRef *ref);

/**
 * @brief Test helper to copy path fields.
 *
 * @param[out] dst Destination path.
 * @param[in] src Source path.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_path_fields(
    struct OpenAPI_Path *dst, const struct OpenAPI_Path *src);

/**
 * @brief Test helper to copy callback fields.
 *
 * @param[out] dst Destination callback.
 * @param[in] src Source callback.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_callback_fields(
    struct OpenAPI_Callback *dst, const struct OpenAPI_Callback *src);

/**
 * @brief Test helper to copy request body fields.
 *
 * @param[out] dst Destination request body.
 * @param[in] src Source request body.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_request_body_fields(
    struct OpenAPI_RequestBody *dst, const struct OpenAPI_RequestBody *src);

/**
 * @brief Test helper to copy server object.
 *
 * @param[out] dst Destination server.
 * @param[in] src Source server.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_server_object(
    struct OpenAPI_Server *dst, const struct OpenAPI_Server *src);

/**
 * @brief Test helper to copy link fields.
 *
 * @param[out] dst Destination link.
 * @param[in] src Source link.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_link_fields(
    struct OpenAPI_Link *dst, const struct OpenAPI_Link *src);

/**
 * @brief Test helper to check if component callback is referenced.
 *
 * @param[in] spec OpenAPI spec.
 * @param[in] name Callback name.
 * @return CDD_C_ERROR_UNKNOWN if referenced, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_component_callback_is_referenced(
    const struct OpenAPI_Spec *spec, const char *name);

/**
 * @brief Test helper to find component media type.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Media type reference.
 * @param[out] out_val Pointer to found media type.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_media_type(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_MediaType **out_val);

/**
 * @brief Test helper to find component parameter.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Parameter reference.
 * @param[out] out_val Pointer to found parameter.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_parameter(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_Parameter **out_val);

/**
 * @brief Test helper to find component response.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Response reference.
 * @param[out] out_val Pointer to found response.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_response(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_Response **out_val);

/**
 * @brief Test helper to find component header.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Header reference.
 * @param[out] out_val Pointer to found header.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_find_component_header(const struct OpenAPI_Spec *spec, const char *ref,
                               struct OpenAPI_Header **out_val);

/**
 * @brief Test helper to find component request body.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Request body reference.
 * @param[out] out_val Pointer to found request body.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_request_body(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_RequestBody **out_val);

/**
 * @brief Test helper to find component link.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Link reference.
 * @param[out] out_val Pointer to found link.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_find_component_link(const struct OpenAPI_Spec *spec, const char *ref,
                             struct OpenAPI_Link **out_val);

/**
 * @brief Test helper to find component callback.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Callback reference.
 * @param[out] out_val Pointer to found callback.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_callback(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_Callback **out_val);

/**
 * @brief Test helper to find component path item.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Path item reference.
 * @param[out] out_val Pointer to found path item.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_path_item(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_Path **out_val);

/**
 * @brief Test helper to find component example.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Example reference.
 * @param[out] out_val Pointer to found example.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_example(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_Example **out_val);

/**
 * @brief Test helper to find media object by name.
 *
 * @param[in] content JSON content object.
 * @param[in] media_name Media type name to search.
 * @param[out] out_val Pointer to found JSON object.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_find_media_object_by_name(
    const JSON_Object *content, const char *media_name, JSON_Object **out_val);

/**
 * @brief Test helper to parse schema ref.
 *
 * @param[in] schema JSON schema object.
 * @param[out] out SchemaRef structure to populate.
 * @param[in] spec OpenAPI spec context.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_schema_ref(
    const JSON_Object *schema, struct OpenAPI_SchemaRef *out,
    const struct OpenAPI_Spec *spec);

/**
 * @brief Test helper to free security requirement set.
 *
 * @param[in,out] set Security requirement set to free.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_free_security_requirement_set(
    struct OpenAPI_SecurityRequirementSet *set);

/**
 * @brief Test helper to copy any value.
 *
 * @param[out] dst Destination any value.
 * @param[in] src Source any value.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_copy_any_value(struct OpenAPI_Any *dst, const struct OpenAPI_Any *src);

/**
 * @brief Test helper to copy item schema as array.
 *
 * @param[out] dst Destination schema ref.
 * @param[in] item Source item schema ref.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_item_schema_as_array(
    struct OpenAPI_SchemaRef *dst, const struct OpenAPI_SchemaRef *item);

/**
 * @brief Test helper to copy security requirement sets.
 *
 * @param[out] dst Destination security requirement set array.
 * @param[out] dst_count Pointer to store destination count.
 * @param[in] src Source security requirement set array.
 * @param[in] src_count Source count.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_security_requirement_sets(
    struct OpenAPI_SecurityRequirementSet **dst, size_t *dst_count,
    const struct OpenAPI_SecurityRequirementSet *src, size_t src_count);

/**
 * @brief Test helper to copy media type array.
 *
 * @param[out] dst Destination media type array.
 * @param[out] dst_count Pointer to store destination count.
 * @param[in] src Source media type array.
 * @param[in] src_count Source count.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_media_type_array(
    struct OpenAPI_MediaType **dst, size_t *dst_count,
    const struct OpenAPI_MediaType *src, size_t src_count);

/**
 * @brief Test helper to parse security schemes.
 *
 * @param[in] components Components JSON object.
 * @param[out] out OpenAPI spec to populate.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_security_schemes(
    const JSON_Object *components, struct OpenAPI_Spec *out);

/**
 * @brief Test helper to parse security field.
 *
 * @param[in] obj JSON object containing security field.
 * @param[in] key Key name for security field.
 * @param[out] out Security requirement set array to populate.
 * @param[out] out_count Pointer to store count.
 * @param[out] out_set Flag indicating whether field was set.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_security_field(const JSON_Object *obj, const char *key,
                              struct OpenAPI_SecurityRequirementSet **out,
                              size_t *out_count, int *out_set);

/**
 * @brief Test helper to parse security requirements.
 *
 * @param[in] arr JSON array of security requirements.
 * @param[out] out Security requirement set array to populate.
 * @param[out] out_count Pointer to store count.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_security_requirements(
    const JSON_Array *arr, struct OpenAPI_SecurityRequirementSet **out,
    size_t *out_count);

/**
 * @brief Test helper to parse parameter object.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_parameter_object(
    const JSON_Object *p_obj, struct OpenAPI_Parameter *out_param,
    const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Test helper to parse parameters array.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_parameters_array(
    const JSON_Array *arr, struct OpenAPI_Parameter **out_params,
    size_t *out_count, const struct OpenAPI_Spec *spec);

/**
 * @brief Test helper to parse request body object.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_request_body_object(
    const JSON_Object *rb_obj, struct OpenAPI_RequestBody *out_rb,
    const struct OpenAPI_Spec *spec, int resolve_refs, const char *op_id);

/**
 * @brief Test helper to parse response object.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_response_object(
    const JSON_Object *resp_obj, struct OpenAPI_Response *out_resp,
    const struct OpenAPI_Spec *spec, int resolve_refs, const char *op_id,
    const char *resp_code);

/**
 * @brief Test helper to parse responses.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_responses(
    const JSON_Object *responses, struct OpenAPI_Operation *out_op,
    const struct OpenAPI_Spec *spec, const char *op_id);

/**
 * @brief Test helper to parse callback object.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_callback_object(
    const JSON_Object *cb_obj, struct OpenAPI_Callback *out_cb,
    const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Test helper to parse callbacks object.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_callbacks_object(
    const JSON_Object *callbacks, struct OpenAPI_Callback **out_callbacks,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Test helper for media_type_specificity.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_media_type_specificity(const char *name, int *out_spec);

/**
 * @brief Test helper for media_type_preference_rank.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_media_type_preference_rank(const char *name, int *out_rank);

/**
 * @brief Test helper for select_primary_media_type_index.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_select_primary_media_type_index(
    const struct OpenAPI_MediaType *mts, size_t n, int *out_idx);

/**
 * @brief Test helper for parse_media_type_object.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_media_type_object(
    const JSON_Object *media_obj, struct OpenAPI_MediaType *out,
    const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Test helper for parse_content_object.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_content_object(
    const JSON_Object *content, struct OpenAPI_MediaType **out,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Test helper for parse_schema_constraints.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_schema_constraints(
    const JSON_Object *schema, struct SchemaConstraintTarget *target);

/**
 * @brief Test helper for parse_string_enum_array.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_string_enum_array(
    const JSON_Array *arr, char ***out, size_t *out_count);

/**
 * @brief Test helper for copy_string_array.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_copy_string_array(char ***dst,
                                                             size_t *dst_count,
                                                             char **src,
                                                             size_t src_count);

/**
 * @brief Test helper for schema_is_string_enum_only.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_schema_is_string_enum_only(const JSON_Object *schema_obj);

/**
 * @brief Test helper for schema_is_struct_compatible.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_schema_is_struct_compatible(
    const JSON_Value *schema_val, const JSON_Object *schema_obj);

/**
 * @brief Test helper for parse_schema_array_ref.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_schema_array_ref(
    const JSON_Array *arr, struct OpenAPI_SchemaRef **out, size_t *out_count,
    const struct OpenAPI_Spec *spec);

/**
 * @brief Test helper for parse_schema_ref_ptr.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_schema_ref_ptr(
    const JSON_Object *obj, struct OpenAPI_SchemaRef **out,
    const struct OpenAPI_Spec *spec);

/**
 * @brief Test helper for parse_example_object.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_example_object(
    const JSON_Object *ex_obj, const char *name, struct OpenAPI_Example *out,
    const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Test helper for parse_examples_object.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_examples_object(
    const JSON_Object *examples, struct OpenAPI_Example **out,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Test helper for parse_media_examples.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_media_examples(
    const JSON_Object *media_obj, struct OpenAPI_Any *example, int *example_set,
    struct OpenAPI_Example **examples, size_t *n_examples,
    const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Test helper for parse_oauth_scopes.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_oauth_scopes(const JSON_Object *scopes_obj,
                            struct OpenAPI_OAuthScope **out, size_t *out_count);

/**
 * @brief Test helper for parse_oauth_flows.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_oauth_flows(
    const JSON_Object *flows_obj, struct OpenAPI_SecurityScheme *out);

/**
 * @brief Test helper for validate_querystring_usage_in_callbacks.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_querystring_usage_in_callbacks(
    const struct OpenAPI_Callback *callbacks, size_t n_callbacks);

/**
 * @brief Test helper for validate_querystring_usage_in_operations.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_querystring_usage_in_operations(
    const struct OpenAPI_Operation *ops, size_t n_ops);

/**
 * @brief Test helper for validate_querystring_usage_in_paths_callbacks.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_querystring_usage_in_paths_callbacks(
    const struct OpenAPI_Path *paths, size_t n_paths);

/**
 * @brief Test helper for validate_querystring_usage_in_component_callbacks.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_querystring_usage_in_component_callbacks(
    const struct OpenAPI_Spec *spec);

/**
 * @brief Test helper for add_unique_operation_id.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_add_unique_operation_id(
    char ***ids, size_t *count, size_t *cap, const char *op_id);

/**
 * @brief Test helper for collect_operation_ids.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_collect_operation_ids(const struct OpenAPI_Path *paths, size_t n_paths,
                               char ***ids, size_t *count, size_t *cap);

/**
 * @brief Test helper for path_item_ref_matches_component.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_path_item_ref_matches_component(
    const struct OpenAPI_Spec *spec, const char *ref, const char *name);

/**
 * @brief Test helper for component_path_item_is_referenced.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_component_path_item_is_referenced(
    const struct OpenAPI_Spec *spec, const char *name);

/**
 * @brief Test helper for callback_ref_matches_component.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_callback_ref_matches_component(
    const struct OpenAPI_Spec *spec, const char *ref, const char *name);

/**
 * @brief Test helper for component_callback_is_referenced_in_ops.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_component_callback_is_referenced_in_ops(
    const struct OpenAPI_Operation *ops, size_t n_ops,
    const struct OpenAPI_Spec *spec, const char *name);

/**
 * @brief Test helper for collect_callback_operation_ids_from_callbacks.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_collect_callback_operation_ids_from_callbacks(
    const struct OpenAPI_Callback *callbacks, size_t n_callbacks, char ***ids,
    size_t *count, size_t *cap);

/**
 * @brief Test helper for collect_callback_operation_ids_from_operations.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_collect_callback_operation_ids_from_operations(
    const struct OpenAPI_Operation *ops, size_t n_ops, char ***ids,
    size_t *count, size_t *cap);

/**
 * @brief Test helper for collect_callback_operation_ids_from_paths.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_collect_callback_operation_ids_from_paths(
    const struct OpenAPI_Path *paths, size_t n_paths, char ***ids,
    size_t *count, size_t *cap);

/**
 * @brief Test helper for validate_unique_operation_ids.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_unique_operation_ids(const struct OpenAPI_Spec *spec);

/**
 * @brief Test helper for parse_paths_object.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_paths_object(
    const JSON_Object *paths_obj, struct OpenAPI_Path **out_paths,
    size_t *out_n_paths, const struct OpenAPI_Spec *spec, int is_root_paths,
    int validate_refs);

/**
 * @brief Test helper for name_in_list.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_name_in_list(const char *name,
                                                        char **names,
                                                        size_t count);

/**
 * @brief Test helper for collect_path_template_names.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_collect_path_template_names(
    const char *route, char ***out_names, size_t *out_count);

/**
 * @brief Test helper for validate_path_params_list.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_validate_path_params_list(
    const struct OpenAPI_Parameter *params, size_t n_params,
    char **template_names, size_t n_template_names);

/**
 * @brief Test helper for validate_path_template_for_operation.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_validate_path_template_for_operation(
    const struct OpenAPI_Path *path, const struct OpenAPI_Operation *op,
    char **template_names, size_t n_template_names);

/**
 * @brief Test helper for validate_path_templates.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_validate_path_templates(
    const struct OpenAPI_Path *paths, size_t n_paths);

/**
 * @brief Test helper for validate_path_template_collisions.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_validate_path_template_collisions(
    const struct OpenAPI_Path *paths, size_t n_paths);

/**
 * @brief Test helper for parse_additional_operations.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_parse_additional_operations(
    const JSON_Object *path_obj, struct OpenAPI_Path *path,
    const struct OpenAPI_Spec *spec);

#endif /* CDD_BUILD_TESTS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENAPI_TEST_HELPERS_H */
