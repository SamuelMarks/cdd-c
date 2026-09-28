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

#endif /* CDD_BUILD_TESTS */
