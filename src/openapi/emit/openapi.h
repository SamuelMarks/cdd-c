/**
 * @file openapi.h
 * @brief Writer module for OpenAPI v3.2 definitions.
 *
 * Provides functionality to serialize an in-memory `OpenAPI_Spec` structure
 * into a JSON string. This acts as the inverse of `openapi_loader`.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_OPENAPI_WRITER_H
#define C_CDD_OPENAPI_WRITER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "openapi/parse/openapi.h"
#include "parson.h"
/* clang-format on */

/**
 * @brief Serialize an OpenAPI Spec structure to a JSON string.
 *
 * Converts the full specification tree (Paths, Operations, Parameters,
 * Responses, and Components) into a formatted JSON string.
 *
 * The caller is responsible for freeing the output string.
 *
 * @param[in] spec The specification structure to serialize.
 * @param[out] json_out Pointer to a char* where the allocated JSON string will
 * be stored.
 * @return 0 on success, EINVAL if inputs are invalid, ENOMEM on allocation
 * failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
openapi_write_spec_to_json(const struct OpenAPI_Spec *spec, char **json_out);

/**
 * @brief Converts verb to string.
 *
 * @param verb The verb.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t verb_to_str_openapi(enum OpenAPI_Verb verb,
                                                      char **_out_val);

/**
 * @brief Converts parameter in to string.
 *
 * @param in The parameter in.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
param_in_to_str_openapi(enum OpenAPI_ParamIn in, char **_out_val);

/**
 * @brief Converts style to string.
 *
 * @param s The style.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t style_to_str_openapi(enum OpenAPI_Style s,
                                                       char **_out_val);

/**
 * @brief Converts xml node type to string.
 *
 * @param t The xml node type.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
xml_node_type_to_str_openapi(enum OpenAPI_XmlNodeType t, char **_out_val);

/**
 * @brief Checks if header name is content type.
 *
 * @param name The name.
 * @return 1 if true, 0 otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
header_name_is_content_type_openapi(const char *name);

/**
 * @brief Checks if parameter is a reserved header.
 *
 * @param p The parameter.
 * @return 1 if true, 0 otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
param_is_reserved_header_openapi(const struct OpenAPI_Parameter *p);

/**
 * @brief Converts oauth flow type to string.
 *
 * @param t The oauth flow type.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
oauth_flow_type_to_str_openapi(enum OpenAPI_OAuthFlowType t, char **_out_val);
/**
 * @brief Checks if a schema type is primitive.
 *
 * @param type The schema type string.
 * @return 1 if true, 0 otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t is_schema_primitive_openapi(const char *type);

/**
 * @brief Merges extra schema object properties.
 *
 * @param target The target json object.
 * @param extras_json The extra json string.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t merge_schema_extras_object_openapi(
    JSON_Object *target, const char *extras_json);

/**
 * @brief Checks if license fields are invalid.
 *
 * @param[in] lic The license structure to check.
 * @return CDD_C_SUCCESS if valid, error code if invalid.
 */
extern C_CDD_EXPORT cdd_c_error_t
license_fields_invalid(const struct OpenAPI_License *lic);

/**
 * @brief Checks if server URL contains query or fragment parts.
 *
 * @param[in] url The server URL to validate.
 * @return CDD_C_SUCCESS if valid, error code if invalid.
 */
extern C_CDD_EXPORT cdd_c_error_t
server_url_has_query_or_fragment(const char *url);

/**
 * @brief Clones a parson JSON value.
 *
 * @param[in] val The source JSON value to clone.
 * @param[out] _out_val Pointer to the cloned JSON value output.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t clone_json_value(const JSON_Value *val,
                                                   JSON_Value **_out_val);

/**
 * @brief Checks if a schema reference structure contains any data.
 *
 * @param[in] ref The schema reference to check.
 * @return CDD_C_SUCCESS if empty, non-zero error/status if populated.
 */
extern C_CDD_EXPORT cdd_c_error_t
schema_ref_has_data(const struct OpenAPI_SchemaRef *ref);

/**
 * @brief Resolves the schema ref keyword string ($ref or $dynamicRef).
 *
 * @param[in] is_dynamic Flag indicating if dynamic ref is used.
 * @param[out] _out_val Pointer to the keyword string output.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t schema_ref_keyword(int is_dynamic,
                                                     char **_out_val);

/**
 * @brief Converts an OpenAPI_Any value to a Parson JSON_Value.
 *
 * @param[in] val The OpenAPI_Any value.
 * @param[out] _out_val Pointer to the allocated JSON_Value.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
any_to_json_value(const struct OpenAPI_Any *val, JSON_Value **_out_val);

/**
 * @brief Writes schema type field to JSON object.
 *
 * @param[in,out] obj The JSON object to populate.
 * @param[in] type The type name string.
 * @param[in] nullable Non-zero if the type is nullable.
 */
extern C_CDD_EXPORT void write_schema_type(JSON_Object *obj, const char *type,
                                           int nullable);

/**
 * @brief Checks if a type union contains a specific string value.
 *
 * @param[in] types Array of type strings.
 * @param[in] n_types Number of elements in types array.
 * @param[in] value The string value to search for.
 * @return CDD_C_ERROR_UNKNOWN if found, CDD_C_SUCCESS if not found.
 */
extern C_CDD_EXPORT cdd_c_error_t type_union_contains(char **types,
                                                      size_t n_types,
                                                      const char *value);

/**
 * @brief Writes schema type union field to JSON object.
 *
 * @param[in,out] obj The JSON object to populate.
 * @param[in] type The default type name string.
 * @param[in] nullable Non-zero if the type is nullable.
 * @param[in] type_union Array of union type strings.
 * @param[in] n_type_union Number of elements in type_union array.
 */
extern C_CDD_EXPORT void write_schema_type_union(JSON_Object *obj,
                                                 const char *type, int nullable,
                                                 char **type_union,
                                                 size_t n_type_union);

/**
 * @brief Writes enum any values array to JSON object.
 *
 * @param[in,out] obj The JSON object to populate.
 * @param[in] key The JSON key name.
 * @param[in] values Array of any values.
 * @param[in] n_values Number of values.
 */
extern C_CDD_EXPORT void write_enum_any_values(JSON_Object *obj,
                                               const char *key,
                                               const struct OpenAPI_Any *values,
                                               size_t n_values);

/**
 * @brief Writes any array values to JSON object.
 *
 * @param[in,out] obj The JSON object to populate.
 * @param[in] key The JSON key name.
 * @param[in] values Array of any values.
 * @param[in] n_values Number of values.
 */
extern C_CDD_EXPORT void
write_any_array_values(JSON_Object *obj, const char *key,
                       const struct OpenAPI_Any *values, size_t n_values);

/**
 * @brief Writes an OpenAPI_Example to a JSON object.
 *
 * @param[in,out] ex_obj The JSON object to populate.
 * @param[in] ex The example structure to write.
 */
extern C_CDD_EXPORT void write_example_object(JSON_Object *ex_obj,
                                              const struct OpenAPI_Example *ex);

/**
 * @brief Writes an examples dictionary to parent JSON object.
 *
 * @param[in,out] parent The parent JSON object.
 * @param[in] key The property key name.
 * @param[in] examples Array of example structures.
 * @param[in] n_examples Number of examples in the array.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_examples_object(
    JSON_Object *parent, const char *key,
    const struct OpenAPI_Example *examples, size_t n_examples);

/**
 * @brief Writes example or examples fields to a JSON object.
 *
 * @param[in,out] parent The parent JSON object.
 * @param[in] example Single example structure.
 * @param[in] example_set Non-zero if single example is set.
 * @param[in] examples Array of multiple examples.
 * @param[in] n_examples Count of multiple examples.
 */
extern C_CDD_EXPORT void
write_example_fields(JSON_Object *parent, const struct OpenAPI_Any *example,
                     int example_set, const struct OpenAPI_Example *examples,
                     size_t n_examples);

/**
 * @brief Writes external documentation object to parent JSON object.
 *
 * @param[in,out] parent The parent JSON object.
 * @param[in] key The property key name.
 * @param[in] docs The external documentation structure.
 */
extern C_CDD_EXPORT void
write_external_docs(JSON_Object *parent, const char *key,
                    const struct OpenAPI_ExternalDocs *docs);

/**
 * @brief Writes discriminator object to parent JSON object.
 *
 * @param[in,out] parent The parent JSON object.
 * @param[in] disc The discriminator structure.
 * @param[in] disc_set Non-zero if discriminator is set.
 */
extern C_CDD_EXPORT void
write_discriminator_object(JSON_Object *parent,
                           const struct OpenAPI_Discriminator *disc,
                           int disc_set);

/**
 * @brief Writes XML object to parent JSON object.
 *
 * @param[in,out] parent The parent JSON object.
 * @param[in] xml The XML structure.
 * @param[in] xml_set Non-zero if XML structure is set.
 */
extern C_CDD_EXPORT void write_xml_object(JSON_Object *parent,
                                          const struct OpenAPI_Xml *xml,
                                          int xml_set);

/**
 * @brief Writes info metadata object to root specification object.
 *
 * @param[in,out] root_obj The root JSON object.
 * @param[in] spec The OpenAPI specification structure.
 */
extern C_CDD_EXPORT void write_info(JSON_Object *root_obj,
                                    const struct OpenAPI_Spec *spec);

/**
 * @brief Writes server object fields to JSON object.
 *
 * @param[in,out] srv_obj The server JSON object.
 * @param[in] srv The OpenAPI server structure.
 */
extern C_CDD_EXPORT void write_server_object(JSON_Object *srv_obj,
                                             const struct OpenAPI_Server *srv);

/**
 * @brief Writes numeric constraint fields to a JSON schema object.
 *
 * @param[in,out] obj The JSON schema object.
 * @param[in] has_min Non-zero if minimum is set.
 * @param[in] min_val Minimum numeric value.
 * @param[in] exclusive_min Non-zero if minimum is exclusive.
 * @param[in] has_max Non-zero if maximum is set.
 * @param[in] max_val Maximum numeric value.
 * @param[in] exclusive_max Non-zero if maximum is exclusive.
 */
extern C_CDD_EXPORT void write_numeric_constraints(JSON_Object *obj,
                                                   int has_min, double min_val,
                                                   int exclusive_min,
                                                   int has_max, double max_val,
                                                   int exclusive_max);

/**
 * @brief Writes string constraint fields to a JSON schema object.
 *
 * @param[in,out] obj The JSON schema object.
 * @param[in] has_min_len Non-zero if minLength is set.
 * @param[in] min_len Minimum string length.
 * @param[in] has_max_len Non-zero if maxLength is set.
 * @param[in] max_len Maximum string length.
 * @param[in] pattern Optional regex pattern string.
 */
extern C_CDD_EXPORT void
write_string_constraints(JSON_Object *obj, int has_min_len, size_t min_len,
                         int has_max_len, size_t max_len, const char *pattern);

/**
 * @brief Writes array constraint fields to a JSON schema object.
 *
 * @param[in,out] obj The JSON schema object.
 * @param[in] has_min_items Non-zero if minItems is set.
 * @param[in] min_items Minimum item count.
 * @param[in] has_max_items Non-zero if maxItems is set.
 * @param[in] max_items Maximum item count.
 * @param[in] unique_items Non-zero if uniqueItems is true.
 */
extern C_CDD_EXPORT void
write_array_constraints(JSON_Object *obj, int has_min_items, size_t min_items,
                        int has_max_items, size_t max_items, int unique_items);

/**
 * @brief Writes items schema fields for an array schema.
 *
 * @param[in,out] item_obj The items JSON schema object.
 * @param[in] ref The OpenAPI schema reference holding item constraints.
 */
extern C_CDD_EXPORT void
write_items_schema_fields(JSON_Object *item_obj,
                          const struct OpenAPI_SchemaRef *ref);

/**
 * @brief Writes schema reference or inline definition to parent object.
 *
 * @param[in,out] parent The parent JSON object.
 * @param[in] key Property key name.
 * @param[in] ref The schema reference structure.
 */
extern C_CDD_EXPORT void write_schema_ref(JSON_Object *parent, const char *key,
                                          const struct OpenAPI_SchemaRef *ref);

/**
 * @brief Constructs and writes schema from simple type and items type strings.
 *
 * @param[in,out] parent The parent JSON object.
 * @param[in] key Property key name.
 * @param[in] type Type name string.
 * @param[in] is_array Non-zero if array type.
 * @param[in] items_type Items type string for arrays.
 */
extern C_CDD_EXPORT void write_schema_from_type_fields(JSON_Object *parent,
                                                       const char *key,
                                                       const char *type,
                                                       int is_array,
                                                       const char *items_type);

/**
 * @brief Writes an OpenAPI parameter object.
 *
 * @param[in,out] p_obj The parameter JSON object.
 * @param[in] p The parameter structure.
 */
extern C_CDD_EXPORT void
write_parameter_object(JSON_Object *p_obj, const struct OpenAPI_Parameter *p);

/**
 * @brief Writes an OpenAPI header object.
 *
 * @param[in,out] h_obj The header JSON object.
 * @param[in] h The header structure.
 */
extern C_CDD_EXPORT void write_header_object(JSON_Object *h_obj,
                                             const struct OpenAPI_Header *h);

/**
 * @brief Writes an encoding object.
 *
 * @param[in,out] enc_obj The encoding JSON object.
 * @param[in] enc The encoding structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_encoding_object(JSON_Object *enc_obj, const struct OpenAPI_Encoding *enc);

/**
 * @brief Writes encoding map to media type object.
 *
 * @param[in,out] media_obj The media type JSON object.
 * @param[in] encoding Array of encoding structures.
 * @param[in] n_encoding Number of encodings.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_encoding_map(JSON_Object *media_obj,
                   const struct OpenAPI_Encoding *encoding, size_t n_encoding);

/**
 * @brief Writes an OpenAPI link object.
 *
 * @param[in,out] l_obj The link JSON object.
 * @param[in] link The link structure.
 */
extern C_CDD_EXPORT void write_link_object(JSON_Object *l_obj,
                                           const struct OpenAPI_Link *link);

/**
 * @brief Writes an OpenAPI response object.
 *
 * @param[in,out] r_obj The response JSON object.
 * @param[in] resp The response structure.
 */
extern C_CDD_EXPORT void
write_response_object(JSON_Object *r_obj, const struct OpenAPI_Response *resp);

/**
 * @brief Writes an OpenAPI request body object.
 *
 * @param[in,out] rb_obj The request body JSON object.
 * @param[in] rb The request body structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_request_body_object(
    JSON_Object *rb_obj, const struct OpenAPI_RequestBody *rb);

/**
 * @brief Writes an OpenAPI callback object.
 *
 * @param[in,out] cb_obj The callback JSON object.
 * @param[in] cb The callback structure.
 */
extern C_CDD_EXPORT void
write_callback_object(JSON_Object *cb_obj, const struct OpenAPI_Callback *cb);

/**
 * @brief Writes an OpenAPI media type object.
 *
 * @param[in,out] media_obj The media type JSON object.
 * @param[in] mt The media type structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_media_type_object(
    JSON_Object *media_obj, const struct OpenAPI_MediaType *mt);

/**
 * @brief Writes headers map to parent JSON object.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property key name.
 * @param[in] headers Array of header structures.
 * @param[in] n_headers Number of headers.
 * @param[in] ignore_content_type Flag to ignore Content-Type headers.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_headers_map(
    JSON_Object *parent, const char *key, const struct OpenAPI_Header *headers,
    size_t n_headers, int ignore_content_type);

/**
 * @brief Writes headers for an OpenAPI response.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] resp Response structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_headers(JSON_Object *parent, const struct OpenAPI_Response *resp);

/**
 * @brief Writes links for an OpenAPI response.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] resp Response structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_links(JSON_Object *parent, const struct OpenAPI_Response *resp);

/**
 * @brief Writes media type map to parent JSON object.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property key name.
 * @param[in] mts Array of media type structures.
 * @param[in] n_mts Number of media types.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_media_type_map(JSON_Object *parent, const char *key,
                     const struct OpenAPI_MediaType *mts, size_t n_mts);

/**
 * @brief Writes encoding array to parent JSON object.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property key name.
 * @param[in] encoding Array of encoding structures.
 * @param[in] n_encoding Number of encodings.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_encoding_array(
    JSON_Object *parent, const char *key,
    const struct OpenAPI_Encoding *encoding, size_t n_encoding);

/**
 * @brief Writes operation object fields.
 *
 * @param[in,out] op_obj Operation JSON object.
 * @param[in] op Operation structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_operation_object(JSON_Object *op_obj, const struct OpenAPI_Operation *op);

/**
 * @brief Writes parameters array to parent JSON object.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] params Array of parameters.
 * @param[in] n_params Number of parameters.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_parameters(JSON_Object *parent, const struct OpenAPI_Parameter *params,
                 size_t n_params);

/**
 * @brief Writes responses map to operation JSON object.
 *
 * @param[in,out] op_obj Operation JSON object.
 * @param[in] op Operation structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_responses(JSON_Object *op_obj, const struct OpenAPI_Operation *op);

/**
 * @brief Writes request body to operation JSON object.
 *
 * @param[in,out] op_obj Operation JSON object.
 * @param[in] op Operation structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_request_body(JSON_Object *op_obj, const struct OpenAPI_Operation *op);

/**
 * @brief Writes callbacks map to operation JSON object.
 *
 * @param[in,out] op_obj Operation JSON object.
 * @param[in] op Operation structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_callbacks(JSON_Object *op_obj, const struct OpenAPI_Operation *op);

/**
 * @brief Writes operations to path item JSON object.
 *
 * @param[in,out] path_item Path item JSON object.
 * @param[in] path Path structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_operations(JSON_Object *path_item, const struct OpenAPI_Path *path);

/**
 * @brief Writes additional operations to path item JSON object.
 *
 * @param[in,out] path_item Path item JSON object.
 * @param[in] path Path structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_additional_operations(
    JSON_Object *path_item, const struct OpenAPI_Path *path);

/**
 * @brief Writes path item object.
 *
 * @param[in,out] item_obj Path item JSON object.
 * @param[in] path Path structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_path_item_object(JSON_Object *item_obj, const struct OpenAPI_Path *path);

/**
 * @brief Writes paths object to root JSON object.
 *
 * @param[in,out] root_obj Root JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_paths(JSON_Object *root_obj,
                                              const struct OpenAPI_Spec *spec);

/**
 * @brief Writes servers array to root JSON object.
 *
 * @param[in,out] root_obj Root JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_servers(JSON_Object *root_obj, const struct OpenAPI_Spec *spec);

/**
 * @brief Writes server array to parent JSON object.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property key name.
 * @param[in] servers Array of server structures.
 * @param[in] n_servers Number of servers.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_server_array(JSON_Object *parent, const char *key,
                   const struct OpenAPI_Server *servers, size_t n_servers);

/**
 * @brief Writes security requirements array to parent JSON object.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property key name.
 * @param[in] sets Array of security requirement sets.
 * @param[in] count Number of sets.
 * @param[in] set_flag Flag indicating if security requirements are set.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_security_requirements(JSON_Object *parent, const char *key,
                            const struct OpenAPI_SecurityRequirementSet *sets,
                            size_t count, int set_flag);

/**
 * @brief Writes security schemes to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_security_schemes(
    JSON_Object *components, const struct OpenAPI_Spec *spec);

/**
 * @brief Writes component parameters to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_component_parameters(
    JSON_Object *components, const struct OpenAPI_Spec *spec);

/**
 * @brief Writes component responses to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_component_responses(
    JSON_Object *components, const struct OpenAPI_Spec *spec);

/**
 * @brief Writes component headers to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_component_headers(
    JSON_Object *components, const struct OpenAPI_Spec *spec);

/**
 * @brief Writes component request bodies to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_component_request_bodies(
    JSON_Object *components, const struct OpenAPI_Spec *spec);

/**
 * @brief Writes component media types to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_component_media_types(
    JSON_Object *components, const struct OpenAPI_Spec *spec);

/**
 * @brief Writes component examples to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_component_examples(
    JSON_Object *components, const struct OpenAPI_Spec *spec);

/**
 * @brief Writes component links to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_component_links(JSON_Object *components, const struct OpenAPI_Spec *spec);

/**
 * @brief Writes component callbacks to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_component_callbacks(
    JSON_Object *components, const struct OpenAPI_Spec *spec);

/**
 * @brief Writes component path items to components JSON object.
 *
 * @param[in,out] components Components JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_component_path_items(
    JSON_Object *components, const struct OpenAPI_Spec *spec);

/**
 * @brief Writes components object to root JSON object.
 *
 * @param[in,out] root_obj Root JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_components(JSON_Object *root_obj, const struct OpenAPI_Spec *spec);

/**
 * @brief Writes tags array to root JSON object.
 *
 * @param[in,out] root_obj Root JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_tags(JSON_Object *root_obj,
                                             const struct OpenAPI_Spec *spec);

/**
 * @brief Writes webhooks object to root JSON object.
 *
 * @param[in,out] root_obj Root JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_webhooks(JSON_Object *root_obj, const struct OpenAPI_Spec *spec);

#ifdef __cplusplus
}

#endif /* __cplusplus */

#endif /* C_CDD_OPENAPI_WRITER_H */
