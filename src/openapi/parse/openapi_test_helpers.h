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

#endif /* CDD_BUILD_TESTS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENAPI_TEST_HELPERS_H */
