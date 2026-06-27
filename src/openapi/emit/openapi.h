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
extern C_CDD_EXPORT int
openapi_write_spec_to_json(const struct OpenAPI_Spec *spec, char **json_out);

/**
 * @brief Converts verb to string.
 *
 * @param verb The verb.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int verb_to_str_openapi(enum OpenAPI_Verb verb,
                                            char **_out_val);

/**
 * @brief Converts parameter in to string.
 *
 * @param in The parameter in.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int param_in_to_str_openapi(enum OpenAPI_ParamIn in,
                                                char **_out_val);

/**
 * @brief Converts style to string.
 *
 * @param s The style.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int style_to_str_openapi(enum OpenAPI_Style s,
                                             char **_out_val);

/**
 * @brief Converts xml node type to string.
 *
 * @param t The xml node type.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int xml_node_type_to_str_openapi(enum OpenAPI_XmlNodeType t,
                                                     char **_out_val);

/**
 * @brief Checks if header name is content type.
 *
 * @param name The name.
 * @return 1 if true, 0 otherwise.
 */
extern C_CDD_EXPORT int header_name_is_content_type_openapi(const char *name);

/**
 * @brief Checks if parameter is a reserved header.
 *
 * @param p The parameter.
 * @return 1 if true, 0 otherwise.
 */
extern C_CDD_EXPORT int
param_is_reserved_header_openapi(const struct OpenAPI_Parameter *p);

/**
 * @brief Converts oauth flow type to string.
 *
 * @param t The oauth flow type.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
extern int oauth_flow_type_to_str_openapi(enum OpenAPI_OAuthFlowType t,
                                          char **_out_val);
/**
 * @brief Checks if a schema type is primitive.
 *
 * @param type The schema type string.
 * @return 1 if true, 0 otherwise.
 */
extern int is_schema_primitive_openapi(const char *type);

/**
 * @brief Merges extra schema object properties.
 *
 * @param target The target json object.
 * @param extras_json The extra json string.
 * @return 0 on success.
 */
extern int merge_schema_extras_object_openapi(JSON_Object *target,
                                              const char *extras_json);

#ifdef __cplusplus
}

#endif /* __cplusplus */

#endif /* C_CDD_OPENAPI_WRITER_H */
