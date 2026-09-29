/**
 * @file openapi_test_helpers.c
 * @brief Test helper implementations for OpenAPI parser unit tests.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
/**
 * @brief cdd test collect extensions.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_collect_extensions(const JSON_Object *obj,
                                                       char **out_json) {
  return collect_extensions(obj, out_json);
}

/**
 * @brief cdd test collect schema extras.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_collect_schema_extras(const JSON_Object *obj, const char **skip_keys,
                               size_t n_skip, char **out_json) {
  return collect_schema_extras(obj, skip_keys, n_skip, out_json);
}

/**
 * @brief cdd test detect tag cycle.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_detect_tag_cycle(
    const struct OpenAPI_Spec *spec, size_t idx, int *state) {
  return detect_tag_cycle(spec, idx, state);
}

/**
 * @brief cdd test example fields valid.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_example_fields_valid(const struct OpenAPI_Example *ex) {
  return example_fields_valid(ex);
}

/**
 * @brief cdd test find path param.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_find_path_param(const struct OpenAPI_Parameter *params, size_t n,
                         const char *name, struct OpenAPI_Parameter **out_val) {
  return find_path_param(params, n, name, out_val);
}

/**
 * @brief cdd test free header.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_free_header(struct OpenAPI_Header *hdr) {
  free_header(hdr);
  return CDD_C_SUCCESS;
}

/**
 * @brief cdd test free operation.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_free_operation(struct OpenAPI_Operation *op) {
  free_operation(op);
  return CDD_C_SUCCESS;
}

/**
 * @brief cdd test free parameter.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_free_parameter(struct OpenAPI_Parameter *param) {
  free_parameter(param);
  return CDD_C_SUCCESS;
}

/**
 * @brief cdd test header name is content type.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_header_name_is_content_type(const char *name) {
  return header_name_is_content_type(name);
}

/**
 * @brief cdd test is fixed operation method.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_is_fixed_operation_method(const char *method) {
  return is_fixed_operation_method(method);
}

/**
 * @brief cdd test is valid response code key.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_is_valid_response_code_key(const char *code) {
  return is_valid_response_code_key(code);
}

/**
 * @brief cdd test json pointer unescape.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_json_pointer_unescape(const char *in,
                                                          char **out_val) {
  return json_pointer_unescape(in, out_val);
}

/**
 * @brief cdd test media type base equal.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_media_type_base_equal(const char *a,
                                                          const char *b) {
  return media_type_base_equal(a, b);
}

/**
 * @brief cdd test media type base len.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_media_type_base_len(const char *name,
                                                        size_t *out_val) {
  return media_type_base_len(name, out_val);
}

/**
 * @brief cdd test media type is json.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_media_type_is_json(const char *name) {
  return media_type_is_json(name);
}

/**
 * @brief cdd test normalize path.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_normalize_path(const char *path,
                                                   char **out_val) {
  return normalize_path(path, out_val);
}

/**
 * @brief cdd test normalize path template route.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_normalize_path_template_route(const char *route, char **out_val) {
  return normalize_path_template_route(route, out_val);
}

/**
 * @brief cdd test object has example and examples.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_object_has_example_and_examples(const JSON_Object *obj) {
  return object_has_example_and_examples(obj);
}

/**
 * @brief cdd test openapi version supported.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_openapi_version_supported(const char *version) {
  return openapi_version_supported(version);
}

/**
 * @brief cdd test param key equals.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_param_key_equals(
    const struct OpenAPI_Parameter *a, const struct OpenAPI_Parameter *b) {
  return param_key_equals(a, b);
}

/**
 * @brief cdd test param type is primitive.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_param_type_is_primitive(const char *type) {
  return param_type_is_primitive(type);
}

/**
 * @brief cdd test parse info.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_info(const JSON_Object *root_obj,
                                               struct OpenAPI_Spec *out) {
  return parse_info(root_obj, out);
}

/**
 * @brief cdd test parse operation.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_operation(const char *verb_str, const JSON_Object *op_obj,
                         struct OpenAPI_Operation *out_op,
                         const struct OpenAPI_Spec *spec, int is_additional) {
  return parse_operation(verb_str, op_obj, out_op, spec, is_additional, NULL);
}

/**
 * @brief cdd test parse param in.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_param_in(const char *in, enum OpenAPI_ParamIn *out_val) {
  return parse_param_in(in, out_val);
}

/**
 * @brief cdd test parse param style.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_param_style(const char *s, enum OpenAPI_Style *out_val) {
  return parse_param_style(s, out_val);
}

/**
 * @brief cdd test parse schema type.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_schema_type(const JSON_Object *schema,
                                                      int *out_nullable,
                                                      char **out_val) {
  return parse_schema_type(schema, out_nullable, out_val);
}

/**
 * @brief cdd test parse security in.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_security_in(const char *in, enum OpenAPI_SecurityIn *out_val) {
  return parse_security_in(in, out_val);
}

/**
 * @brief cdd test parse security type.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_security_type(
    const char *type, enum OpenAPI_SecurityType *out_val) {
  return parse_security_type(type, out_val);
}

/**
 * @brief cdd test parse xml node type.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_xml_node_type(
    const char *node_type, enum OpenAPI_XmlNodeType *out_val) {
  return parse_xml_node_type(node_type, out_val);
}

/**
 * @brief cdd test ref base matches self.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_ref_base_matches_self(
    const struct OpenAPI_Spec *spec, const char *ref, const char *hash) {
  return ref_base_matches_self(spec, ref, hash);
}

/**
 * @brief cdd test ref name from prefix.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_ref_name_from_prefix(const struct OpenAPI_Spec *spec, const char *ref,
                              const char *prefix, char **out_val) {
  return ref_name_from_prefix(spec, ref, prefix, out_val);
}

/**
 * @brief cdd test resolve uri reference.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_resolve_uri_reference(const char *base_uri,
                                                          const char *ref,
                                                          char **out_val) {
  return resolve_uri_reference(base_uri, ref, out_val);
}

/**
 * @brief cdd test root has openapi fields.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_root_has_openapi_fields(const JSON_Object *root_obj) {
  return root_has_openapi_fields(root_obj);
}

/**
 * @brief cdd test root is schema document.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_root_is_schema_document(
    const JSON_Value *root, const JSON_Object *root_obj) {
  return root_is_schema_document(root, root_obj);
}

/**
 * @brief cdd test scan querystring usage.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_scan_querystring_usage(
    const struct OpenAPI_Parameter *params, size_t n_params,
    size_t *out_qs_count, int *out_has_query) {
  return scan_querystring_usage(params, n_params, out_qs_count, out_has_query);
}

/**
 * @brief cdd test schema has composition.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_schema_has_composition(const JSON_Object *schema_obj) {
  return schema_has_composition(schema_obj);
}

/**
 * @brief cdd test schema name in use.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_schema_name_in_use(const struct OpenAPI_Spec *spec, const char *name) {
  return schema_name_in_use(spec, name);
}

/**
 * @brief cdd test server variable defined.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_server_variable_defined(
    const struct OpenAPI_Server *srv, const char *name) {
  return server_variable_defined(srv, name);
}

/**
 * @brief cdd test server variable seen.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_server_variable_seen(char **seen,
                                                         size_t seen_count,
                                                         const char *name) {
  return server_variable_seen(seen, seen_count, name);
}

/**
 * @brief cdd test tag index by name.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_tag_index_by_name(
    const struct OpenAPI_Spec *spec, const char *name, size_t *out_idx) {
  return tag_index_by_name(spec, name, out_idx);
}

/**
 * @brief cdd test uri has scheme prefix.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_uri_has_scheme_prefix(const char *uri,
                                                          size_t len) {
  return uri_has_scheme_prefix(uri, len);
}

/**
 * @brief cdd test url has query or fragment.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_url_has_query_or_fragment(const char *url) {
  return url_has_query_or_fragment(url);
}

/**
 * @brief cdd test validate media type key map.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_media_type_key_map(const JSON_Object *obj) {
  return validate_media_type_key_map(obj);
}

/**
 * @brief cdd test validate parameter style.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_validate_parameter_style(
    const struct OpenAPI_Parameter *p, int has_content) {
  return validate_parameter_style(p, has_content);
}

/**
 * @brief cdd test validate querystring usage.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_validate_querystring_usage(
    const struct OpenAPI_Path *paths, size_t n_paths) {
  return validate_querystring_usage(paths, n_paths);
}

/**
 * @brief cdd test validate tag parents.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_tag_parents(const struct OpenAPI_Spec *spec) {
  return validate_tag_parents(spec);
}

/**
 * @brief Test helper to copy parameter fields.
 *
 * @param[out] dst Destination parameter.
 * @param[in] src Source parameter.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_parameter_fields(
    struct OpenAPI_Parameter *dst, const struct OpenAPI_Parameter *src) {
  return copy_parameter_fields(dst, src);
}

/**
 * @brief Test helper to copy header fields.
 *
 * @param[out] dst Destination header.
 * @param[in] src Source header.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_header_fields(
    struct OpenAPI_Header *dst, const struct OpenAPI_Header *src) {
  return copy_header_fields(dst, src);
}

/**
 * @brief Test helper to copy encoding fields.
 *
 * @param[out] dst Destination encoding.
 * @param[in] src Source encoding.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_encoding_fields(
    struct OpenAPI_Encoding *dst, const struct OpenAPI_Encoding *src) {
  return copy_encoding_fields(dst, src);
}

/**
 * @brief Test helper to copy media type fields.
 *
 * @param[out] dst Destination media type.
 * @param[in] src Source media type.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_media_type_fields(
    struct OpenAPI_MediaType *dst, const struct OpenAPI_MediaType *src) {
  return copy_media_type_fields(dst, src);
}

/**
 * @brief Test helper to copy response fields.
 *
 * @param[out] dst Destination response.
 * @param[in] src Source response.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_response_fields(
    struct OpenAPI_Response *dst, const struct OpenAPI_Response *src) {
  return copy_response_fields(dst, src);
}

/**
 * @brief Test helper to copy operation fields.
 *
 * @param[out] dst Destination operation.
 * @param[in] src Source operation.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_operation_fields(
    struct OpenAPI_Operation *dst, const struct OpenAPI_Operation *src) {
  return copy_operation_fields(dst, src);
}

/**
 * @brief Test helper to copy schema ref.
 *
 * @param[out] dst Destination schema ref.
 * @param[in] src Source schema ref.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_schema_ref(
    struct OpenAPI_SchemaRef *dst, const struct OpenAPI_SchemaRef *src) {
  return copy_schema_ref(dst, src);
}

/**
 * @brief Test helper to copy example fields.
 *
 * @param[out] dst Destination example.
 * @param[in] src Source example.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_example_fields(
    struct OpenAPI_Example *dst, const struct OpenAPI_Example *src) {
  return copy_example_fields(dst, src);
}

/**
 * @brief Test helper to free encoding.
 *
 * @param[in,out] enc Encoding to free.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_free_encoding(struct OpenAPI_Encoding *enc) {
  free_encoding(enc);
  return CDD_C_SUCCESS;
}

/**
 * @brief Test helper to free media type.
 *
 * @param[in,out] mt Media type to free.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_free_media_type(struct OpenAPI_MediaType *mt) {
  free_media_type(mt);
  return CDD_C_SUCCESS;
}

/**
 * @brief Test helper to free response.
 *
 * @param[in,out] resp Response to free.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_free_response(struct OpenAPI_Response *resp) {
  free_response(resp);
  return CDD_C_SUCCESS;
}

/**
 * @brief Test helper to free request body.
 *
 * @param[in,out] rb Request body to free.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_free_request_body(struct OpenAPI_RequestBody *rb) {
  free_request_body(rb);
  return CDD_C_SUCCESS;
}

/**
 * @brief Test helper to free any value.
 *
 * @param[in,out] val Any value to free.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_free_any_value(struct OpenAPI_Any *val) {
  free_any_value(val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Test helper to free link.
 *
 * @param[in,out] link Link to free.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_free_link(struct OpenAPI_Link *link) {
  free_link(link);
  return CDD_C_SUCCESS;
}

/**
 * @brief Test helper to free security requirement.
 *
 * @param[in,out] req Security requirement to free.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_free_security_requirement(struct OpenAPI_SecurityRequirement *req) {
  free_security_requirement(req);
  return CDD_C_SUCCESS;
}

/**
 * @brief Test helper to free path item.
 *
 * @param[in,out] p Path item to free.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_free_path_item(struct OpenAPI_Path *p) {
  free_path_item(p);
  return CDD_C_SUCCESS;
}

/**
 * @brief Test helper to free callback.
 *
 * @param[in,out] cb Callback to free.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_free_callback(struct OpenAPI_Callback *cb) {
  free_callback(cb);
  return CDD_C_SUCCESS;
}

/**
 * @brief Test helper to free string array.
 *
 * @param[in,out] arr String array to free.
 * @param[in] n Array size.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_free_string_array(char **arr, size_t n) {
  free_string_array(arr, n);
  return CDD_C_SUCCESS;
}

/**
 * @brief Test helper to free example.
 *
 * @param[in,out] ex Example to free.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_free_example(struct OpenAPI_Example *ex) {
  free_example(ex);
  return CDD_C_SUCCESS;
}

/**
 * @brief Test helper to free schema ref content.
 *
 * @param[in,out] ref Schema ref to free.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_free_schema_ref_content(struct OpenAPI_SchemaRef *ref) {
  free_schema_ref_content(ref);
  return CDD_C_SUCCESS;
}

/**
 * @brief Test helper to copy path fields.
 *
 * @param[out] dst Destination path.
 * @param[in] src Source path.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_path_fields(
    struct OpenAPI_Path *dst, const struct OpenAPI_Path *src) {
  return copy_path_fields(dst, src);
}

/**
 * @brief Test helper to copy callback fields.
 *
 * @param[out] dst Destination callback.
 * @param[in] src Source callback.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_callback_fields(
    struct OpenAPI_Callback *dst, const struct OpenAPI_Callback *src) {
  return copy_callback_fields(dst, src);
}

/**
 * @brief Test helper to copy request body fields.
 *
 * @param[out] dst Destination request body.
 * @param[in] src Source request body.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_request_body_fields(
    struct OpenAPI_RequestBody *dst, const struct OpenAPI_RequestBody *src) {
  return copy_request_body_fields(dst, src);
}

/**
 * @brief Test helper to copy server object.
 *
 * @param[out] dst Destination server.
 * @param[in] src Source server.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_server_object(
    struct OpenAPI_Server *dst, const struct OpenAPI_Server *src) {
  return copy_server_object(dst, src);
}

/**
 * @brief Test helper to copy link fields.
 *
 * @param[out] dst Destination link.
 * @param[in] src Source link.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_link_fields(
    struct OpenAPI_Link *dst, const struct OpenAPI_Link *src) {
  return copy_link_fields(dst, src);
}

/**
 * @brief Test helper to check if component callback is referenced.
 *
 * @param[in] spec OpenAPI spec.
 * @param[in] name Callback name.
 * @return CDD_C_ERROR_UNKNOWN if referenced, CDD_C_SUCCESS otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_component_callback_is_referenced(
    const struct OpenAPI_Spec *spec, const char *name) {
  return component_callback_is_referenced(spec, name);
}

/**
 * @brief Test helper to find component media type.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Media type reference.
 * @param[out] out_val Pointer to found media type.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_media_type(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_MediaType **out_val) {
  return find_component_media_type(spec, ref, out_val);
}

/**
 * @brief Test helper to find component parameter.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Parameter reference.
 * @param[out] out_val Pointer to found parameter.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_parameter(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_Parameter **out_val) {
  return find_component_parameter(spec, ref, out_val);
}

/**
 * @brief Test helper to find component response.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Response reference.
 * @param[out] out_val Pointer to found response.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_response(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_Response **out_val) {
  return find_component_response(spec, ref, out_val);
}

/**
 * @brief Test helper to find component header.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Header reference.
 * @param[out] out_val Pointer to found header.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_find_component_header(const struct OpenAPI_Spec *spec, const char *ref,
                               struct OpenAPI_Header **out_val) {
  return find_component_header(spec, ref, out_val);
}

/**
 * @brief Test helper to find component request body.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Request body reference.
 * @param[out] out_val Pointer to found request body.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_request_body(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_RequestBody **out_val) {
  return find_component_request_body(spec, ref, out_val);
}

/**
 * @brief Test helper to find component link.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Link reference.
 * @param[out] out_val Pointer to found link.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_find_component_link(const struct OpenAPI_Spec *spec, const char *ref,
                             struct OpenAPI_Link **out_val) {
  return find_component_link(spec, ref, out_val);
}

/**
 * @brief Test helper to find component callback.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Callback reference.
 * @param[out] out_val Pointer to found callback.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_callback(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_Callback **out_val) {
  return find_component_callback(spec, ref, out_val);
}

/**
 * @brief Test helper to find component path item.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Path item reference.
 * @param[out] out_val Pointer to found path item.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_path_item(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_Path **out_val) {
  return find_component_path_item(spec, ref, out_val);
}

/**
 * @brief Test helper to find component example.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] ref Example reference.
 * @param[out] out_val Pointer to found example.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_find_component_example(
    const struct OpenAPI_Spec *spec, const char *ref,
    struct OpenAPI_Example **out_val) {
  return find_component_example(spec, ref, out_val);
}

/**
 * @brief Test helper to find media object by name.
 *
 * @param[in] content JSON content object.
 * @param[in] media_name Media type name to search.
 * @param[out] out_val Pointer to found JSON object.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_find_media_object_by_name(
    const JSON_Object *content, const char *media_name, JSON_Object **out_val) {
  return find_media_object_by_name(content, media_name, out_val);
}

/**
 * @brief Test helper to parse schema ref.
 *
 * @param[in] schema JSON schema object.
 * @param[out] out SchemaRef structure to populate.
 * @param[in] spec OpenAPI spec context.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_schema_ref(
    const JSON_Object *schema, struct OpenAPI_SchemaRef *out,
    const struct OpenAPI_Spec *spec) {
  return parse_schema_ref(schema, out, spec);
}

/**
 * @brief Test helper to free security requirement set.
 *
 * @param[in,out] set Security requirement set to free.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_free_security_requirement_set(
    struct OpenAPI_SecurityRequirementSet *set) {
  free_security_requirement_set(set);
  return CDD_C_SUCCESS;
}

/**
 * @brief Test helper to copy any value.
 *
 * @param[out] dst Destination any value.
 * @param[in] src Source any value.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_any_value(
    struct OpenAPI_Any *dst, const struct OpenAPI_Any *src) {
  return copy_any_value(dst, src);
}

/**
 * @brief Test helper to copy item schema as array.
 *
 * @param[out] dst Destination schema ref.
 * @param[in] item Source item schema ref.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_item_schema_as_array(
    struct OpenAPI_SchemaRef *dst, const struct OpenAPI_SchemaRef *item) {
  return copy_item_schema_as_array(dst, item);
}

/**
 * @brief Test helper to copy security requirement sets.
 *
 * @param[out] dst Destination security requirement set array.
 * @param[out] dst_count Pointer to store destination count.
 * @param[in] src Source security requirement set array.
 * @param[in] src_count Source count.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_security_requirement_sets(
    struct OpenAPI_SecurityRequirementSet **dst, size_t *dst_count,
    const struct OpenAPI_SecurityRequirementSet *src, size_t src_count) {
  return copy_security_requirement_sets(dst, dst_count, src, src_count);
}

/**
 * @brief Test helper to copy media type array.
 *
 * @param[out] dst Destination media type array.
 * @param[out] dst_count Pointer to store destination count.
 * @param[in] src Source media type array.
 * @param[in] src_count Source count.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_media_type_array(
    struct OpenAPI_MediaType **dst, size_t *dst_count,
    const struct OpenAPI_MediaType *src, size_t src_count) {
  return copy_media_type_array(dst, dst_count, src, src_count);
}

/**
 * @brief Test helper to parse security schemes.
 *
 * @param[in] components Components JSON object.
 * @param[out] out OpenAPI spec to populate.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_security_schemes(
    const JSON_Object *components, struct OpenAPI_Spec *out) {
  return parse_security_schemes(components, out);
}

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
C_CDD_EXPORT cdd_c_error_t
cdd_test_parse_security_field(const JSON_Object *obj, const char *key,
                              struct OpenAPI_SecurityRequirementSet **out,
                              size_t *out_count, int *out_set) {
  return parse_security_field(obj, key, out, out_count, out_set);
}

/**
 * @brief Test helper to parse security requirements.
 *
 * @param[in] arr JSON array of security requirements.
 * @param[out] out Security requirement set array to populate.
 * @param[out] out_count Pointer to store count.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_security_requirements(
    const JSON_Array *arr, struct OpenAPI_SecurityRequirementSet **out,
    size_t *out_count) {
  return parse_security_requirements(arr, out, out_count);
}

/**
 * @brief Test helper to parse parameter object.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_parameter_object(
    const JSON_Object *p_obj, struct OpenAPI_Parameter *out_param,
    const struct OpenAPI_Spec *spec, int resolve_refs) {
  return parse_parameter_object(p_obj, out_param, spec, resolve_refs);
}

/**
 * @brief Test helper to parse parameters array.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_parameters_array(
    const JSON_Array *arr, struct OpenAPI_Parameter **out_params,
    size_t *out_count, const struct OpenAPI_Spec *spec) {
  return parse_parameters_array(arr, out_params, out_count, spec);
}

/**
 * @brief Test helper to parse request body object.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_request_body_object(
    const JSON_Object *rb_obj, struct OpenAPI_RequestBody *out_rb,
    const struct OpenAPI_Spec *spec, int resolve_refs, const char *op_id) {
  return parse_request_body_object(rb_obj, out_rb, spec, resolve_refs, op_id);
}

/**
 * @brief Test helper to parse response object.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_response_object(
    const JSON_Object *resp_obj, struct OpenAPI_Response *out_resp,
    const struct OpenAPI_Spec *spec, int resolve_refs, const char *op_id,
    const char *resp_code) {
  return parse_response_object(resp_obj, out_resp, spec, resolve_refs, op_id,
                               resp_code);
}

/**
 * @brief Test helper to parse responses.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_responses(
    const JSON_Object *responses, struct OpenAPI_Operation *out_op,
    const struct OpenAPI_Spec *spec, const char *op_id) {
  return parse_responses(responses, out_op, spec, op_id);
}

/**
 * @brief Test helper to parse callback object.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_callback_object(
    const JSON_Object *cb_obj, struct OpenAPI_Callback *out_cb,
    const struct OpenAPI_Spec *spec, int resolve_refs) {
  return parse_callback_object(cb_obj, out_cb, spec, resolve_refs);
}

/**
 * @brief Test helper to parse callbacks object.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_callbacks_object(
    const JSON_Object *callbacks, struct OpenAPI_Callback **out_callbacks,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs) {
  return parse_callbacks_object(callbacks, out_callbacks, out_count, spec,
                                resolve_refs);
}

/**
 * @brief Test helper for media_type_specificity.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_media_type_specificity(const char *name,
                                                           int *out_spec) {
  return media_type_specificity(name, out_spec);
}

/**
 * @brief Test helper for media_type_preference_rank.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_media_type_preference_rank(const char *name,
                                                               int *out_rank) {
  return media_type_preference_rank(name, out_rank);
}

/**
 * @brief Test helper for select_primary_media_type_index.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_select_primary_media_type_index(
    const struct OpenAPI_MediaType *mts, size_t n, int *out_idx) {
  return select_primary_media_type_index(mts, n, out_idx);
}

/**
 * @brief Test helper for parse_media_type_object.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_media_type_object(
    const JSON_Object *media_obj, struct OpenAPI_MediaType *out,
    const struct OpenAPI_Spec *spec, int resolve_refs) {
  return parse_media_type_object(media_obj, out, spec, resolve_refs);
}

/**
 * @brief Test helper for parse_content_object.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_content_object(
    const JSON_Object *content, struct OpenAPI_MediaType **out,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs) {
  return parse_content_object(content, out, out_count, spec, resolve_refs);
}

/**
 * @brief Test helper for parse_schema_constraints.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_schema_constraints(
    const JSON_Object *schema, struct SchemaConstraintTarget *target) {
  return parse_schema_constraints(schema, target);
}

/**
 * @brief Test helper for parse_string_enum_array.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_string_enum_array(
    const JSON_Array *arr, char ***out, size_t *out_count) {
  return parse_string_enum_array(arr, out, out_count);
}

/**
 * @brief Test helper for copy_string_array.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_copy_string_array(char ***dst,
                                                      size_t *dst_count,
                                                      char **src,
                                                      size_t src_count) {
  return copy_string_array(dst, dst_count, src, src_count);
}

/**
 * @brief Test helper for schema_is_string_enum_only.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_schema_is_string_enum_only(const JSON_Object *schema_obj) {
  return schema_is_string_enum_only(schema_obj);
}

/**
 * @brief Test helper for schema_is_struct_compatible.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_schema_is_struct_compatible(
    const JSON_Value *schema_val, const JSON_Object *schema_obj) {
  return schema_is_struct_compatible(schema_val, schema_obj);
}

/**
 * @brief Test helper for parse_schema_array_ref.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_schema_array_ref(
    const JSON_Array *arr, struct OpenAPI_SchemaRef **out, size_t *out_count,
    const struct OpenAPI_Spec *spec) {
  return parse_schema_array_ref(arr, out, out_count, spec);
}

/**
 * @brief Test helper for parse_schema_ref_ptr.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_schema_ref_ptr(
    const JSON_Object *obj, struct OpenAPI_SchemaRef **out,
    const struct OpenAPI_Spec *spec) {
  return parse_schema_ref_ptr(obj, out, spec);
}

/**
 * @brief Test helper for parse_example_object.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_example_object(
    const JSON_Object *ex_obj, const char *name, struct OpenAPI_Example *out,
    const struct OpenAPI_Spec *spec, int resolve_refs) {
  return parse_example_object(ex_obj, name, out, spec, resolve_refs);
}

/**
 * @brief Test helper for parse_examples_object.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_examples_object(
    const JSON_Object *examples, struct OpenAPI_Example **out,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs) {
  return parse_examples_object(examples, out, out_count, spec, resolve_refs);
}

/**
 * @brief Test helper for parse_media_examples.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_media_examples(
    const JSON_Object *media_obj, struct OpenAPI_Any *example, int *example_set,
    struct OpenAPI_Example **examples, size_t *n_examples,
    const struct OpenAPI_Spec *spec, int resolve_refs) {
  return parse_media_examples(media_obj, example, example_set, examples,
                              n_examples, spec, resolve_refs);
}

/**
 * @brief Test helper for parse_oauth_scopes.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_oauth_scopes(
    const JSON_Object *scopes_obj, struct OpenAPI_OAuthScope **out,
    size_t *out_count) {
  return parse_oauth_scopes(scopes_obj, out, out_count);
}

/**
 * @brief Test helper for parse_oauth_flows.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_oauth_flows(
    const JSON_Object *flows_obj, struct OpenAPI_SecurityScheme *out) {
  return parse_oauth_flows(flows_obj, out);
}

/**
 * @brief Test helper for validate_querystring_usage_in_callbacks.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_validate_querystring_usage_in_callbacks(
    const struct OpenAPI_Callback *callbacks, size_t n_callbacks) {
  return validate_querystring_usage_in_callbacks(callbacks, n_callbacks);
}

/**
 * @brief Test helper for validate_querystring_usage_in_operations.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_validate_querystring_usage_in_operations(
    const struct OpenAPI_Operation *ops, size_t n_ops) {
  return validate_querystring_usage_in_operations(ops, n_ops);
}

/**
 * @brief Test helper for validate_querystring_usage_in_paths_callbacks.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_querystring_usage_in_paths_callbacks(
    const struct OpenAPI_Path *paths, size_t n_paths) {
  return validate_querystring_usage_in_paths_callbacks(paths, n_paths);
}

/**
 * @brief Test helper for validate_querystring_usage_in_component_callbacks.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_querystring_usage_in_component_callbacks(
    const struct OpenAPI_Spec *spec) {
  return validate_querystring_usage_in_component_callbacks(spec);
}

/**
 * @brief Test helper for add_unique_operation_id.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_add_unique_operation_id(char ***ids,
                                                            size_t *count,
                                                            size_t *cap,
                                                            const char *op_id) {
  return add_unique_operation_id(ids, count, cap, op_id);
}

/**
 * @brief Test helper for collect_operation_ids.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_collect_operation_ids(const struct OpenAPI_Path *paths, size_t n_paths,
                               char ***ids, size_t *count, size_t *cap) {
  return collect_operation_ids(paths, n_paths, ids, count, cap);
}

/**
 * @brief Test helper for path_item_ref_matches_component.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_path_item_ref_matches_component(
    const struct OpenAPI_Spec *spec, const char *ref, const char *name) {
  return path_item_ref_matches_component(spec, ref, name);
}

/**
 * @brief Test helper for component_path_item_is_referenced.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_component_path_item_is_referenced(
    const struct OpenAPI_Spec *spec, const char *name) {
  return component_path_item_is_referenced(spec, name);
}

/**
 * @brief Test helper for callback_ref_matches_component.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_callback_ref_matches_component(
    const struct OpenAPI_Spec *spec, const char *ref, const char *name) {
  return callback_ref_matches_component(spec, ref, name);
}

/**
 * @brief Test helper for component_callback_is_referenced_in_ops.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_component_callback_is_referenced_in_ops(
    const struct OpenAPI_Operation *ops, size_t n_ops,
    const struct OpenAPI_Spec *spec, const char *name) {
  return component_callback_is_referenced_in_ops(ops, n_ops, spec, name);
}

/**
 * @brief Test helper for collect_callback_operation_ids_from_callbacks.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_collect_callback_operation_ids_from_callbacks(
    const struct OpenAPI_Callback *callbacks, size_t n_callbacks, char ***ids,
    size_t *count, size_t *cap) {
  return collect_callback_operation_ids_from_callbacks(callbacks, n_callbacks,
                                                       ids, count, cap);
}

/**
 * @brief Test helper for collect_callback_operation_ids_from_operations.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_collect_callback_operation_ids_from_operations(
    const struct OpenAPI_Operation *ops, size_t n_ops, char ***ids,
    size_t *count, size_t *cap) {
  return collect_callback_operation_ids_from_operations(ops, n_ops, ids, count,
                                                        cap);
}

/**
 * @brief Test helper for collect_callback_operation_ids_from_paths.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_collect_callback_operation_ids_from_paths(
    const struct OpenAPI_Path *paths, size_t n_paths, char ***ids,
    size_t *count, size_t *cap) {
  return collect_callback_operation_ids_from_paths(paths, n_paths, ids, count,
                                                   cap);
}

/**
 * @brief Test helper for validate_unique_operation_ids.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_test_validate_unique_operation_ids(const struct OpenAPI_Spec *spec) {
  return validate_unique_operation_ids(spec);
}

/**
 * @brief Test helper for parse_paths_object.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_paths_object(
    const JSON_Object *paths_obj, struct OpenAPI_Path **out_paths,
    size_t *out_n_paths, const struct OpenAPI_Spec *spec, int is_root_paths,
    int validate_refs) {
  return parse_paths_object(paths_obj, out_paths, out_n_paths, spec,
                            is_root_paths, validate_refs);
}

/**
 * @brief Test helper for name_in_list.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_name_in_list(const char *name, char **names,
                                                 size_t count) {
  return name_in_list(name, names, count);
}

/**
 * @brief Test helper for collect_path_template_names.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_collect_path_template_names(
    const char *route, char ***out_names, size_t *out_count) {
  return collect_path_template_names(route, out_names, out_count);
}

/**
 * @brief Test helper for validate_path_params_list.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_validate_path_params_list(
    const struct OpenAPI_Parameter *params, size_t n_params,
    char **template_names, size_t n_template_names) {
  return validate_path_params_list(params, n_params, template_names,
                                   n_template_names);
}

/**
 * @brief Test helper for validate_path_template_for_operation.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_validate_path_template_for_operation(
    const struct OpenAPI_Path *path, const struct OpenAPI_Operation *op,
    char **template_names, size_t n_template_names) {
  return validate_path_template_for_operation(path, op, template_names,
                                              n_template_names);
}

/**
 * @brief Test helper for validate_path_templates.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_validate_path_templates(
    const struct OpenAPI_Path *paths, size_t n_paths) {
  return validate_path_templates(paths, n_paths);
}

/**
 * @brief Test helper for validate_path_template_collisions.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_validate_path_template_collisions(
    const struct OpenAPI_Path *paths, size_t n_paths) {
  return validate_path_template_collisions(paths, n_paths);
}

/**
 * @brief Test helper for parse_additional_operations.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_parse_additional_operations(
    const JSON_Object *path_obj, struct OpenAPI_Path *path,
    const struct OpenAPI_Spec *spec) {
  return parse_additional_operations(path_obj, path, spec);
}

#endif /* CDD_BUILD_TESTS */
