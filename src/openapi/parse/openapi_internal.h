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
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/memory.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include "openapi/parse/openapi.h"
#include "openapi/parse/openapi_types.h"
#include "win_compat_sym.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
#undef malloc
#define malloc(sz) C_CDD_MALLOC(sz)
#undef realloc
#define realloc(ptr, sz) C_CDD_REALLOC(ptr, sz)
#undef calloc
#define calloc(n, sz) C_CDD_CALLOC(n, sz)
#endif

#define media_type_is_json openapi_media_type_is_json
#define media_type_base_len openapi_media_type_base_len
#define header_name_is_content_type openapi_header_name_is_content_type
#define clone_json_value openapi_parse_clone_json_value

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_openapi_spec_init_fail;
extern C_CDD_EXPORT int g_cdd_fail_find_schema_by_anchor;
extern C_CDD_EXPORT int g_cdd_fail_find_schema_by_id;
#endif

/** @brief ResolvedRefTarget structure */
struct ResolvedRefTarget {
  /** @brief spec */
  const struct OpenAPI_Spec *spec;
  /** @brief ref */
  const char *ref;
  /** @brief resolved_ref */
  char *resolved_ref;
};

/* --- openapi_init.c declarations --- */

/**
 * @brief Executes the openapi spec init operation.
 */
extern cdd_c_error_t openapi_spec_init(struct OpenAPI_Spec *spec);

/**
 * @brief Executes the openapi doc registry init operation.
 */
extern cdd_c_error_t
openapi_doc_registry_init(struct OpenAPI_DocRegistry *registry);

/**
 * @brief Executes the openapi doc registry free operation.
 */
extern void openapi_doc_registry_free(struct OpenAPI_DocRegistry *registry);

/**
 * @brief Executes the openapi doc registry add operation.
 */
extern cdd_c_error_t
openapi_doc_registry_add(struct OpenAPI_DocRegistry *registry,
                         struct OpenAPI_Spec *spec);

/* --- openapi_free_components.c declarations --- */

/**
 * @brief Frees the memory associated with schema ref content.
 */
extern void free_schema_ref_content(struct OpenAPI_SchemaRef *ref);

/**
 * @brief Frees the memory associated with encoding.
 */
extern void free_encoding(struct OpenAPI_Encoding *enc);

/**
 * @brief Frees the memory associated with media type.
 */
extern void free_media_type(struct OpenAPI_MediaType *mt);

/**
 * @brief Frees the memory associated with parameter.
 */
extern void free_parameter(struct OpenAPI_Parameter *param);

/**
 * @brief Frees the memory associated with header.
 */
extern void free_header(struct OpenAPI_Header *hdr);

/**
 * @brief Frees the memory associated with response.
 */
extern void free_response(struct OpenAPI_Response *resp);

/**
 * @brief Frees the memory associated with request body.
 */
extern void free_request_body(struct OpenAPI_RequestBody *rb);

/**
 * @brief Frees the memory associated with any value.
 */
extern void free_any_value(struct OpenAPI_Any *val);

/**
 * @brief Frees the memory associated with link.
 */
extern void free_link(struct OpenAPI_Link *link);

/**
 * @brief Frees the memory associated with security requirement.
 */
extern void free_security_requirement(struct OpenAPI_SecurityRequirement *req);

/**
 * @brief Frees the memory associated with security requirement set.
 */
extern void
free_security_requirement_set(struct OpenAPI_SecurityRequirementSet *set);

/**
 * @brief Frees the memory associated with example.
 */
extern void free_example(struct OpenAPI_Example *ex);

/* --- openapi_free.c declarations --- */

/**
 * @brief Free servers array.
 * @param servers Array of servers
 * @param n_servers Number of servers
 */
extern void openapi_free_servers_array(struct OpenAPI_Server *servers,
                                       size_t n_servers);

/**
 * @brief Frees the memory associated with path item.
 */
extern void free_path_item(struct OpenAPI_Path *p);

/**
 * @brief Frees the memory associated with callback.
 */
extern void free_callback(struct OpenAPI_Callback *cb);

/**
 * @brief Frees the memory associated with operation.
 */
extern void free_operation(struct OpenAPI_Operation *op);

/**
 * @brief Executes the openapi spec free operation.
 */
extern void openapi_spec_free(struct OpenAPI_Spec *spec);

/**
 * @brief Frees the memory associated with string array.
 */
extern void free_string_array(char **arr, size_t n);

/**
 * @brief Frees the memory associated with name list.
 */
extern void free_name_list(char **names, size_t count);

/* --- openapi_utils.c declarations --- */

/* --- Parsing Helpers --- */
extern C_CDD_EXPORT cdd_c_error_t parse_verb(const char *v,
                                             enum OpenAPI_Verb *_out_val);

/**
 * @brief Checks if fixed operation method.
 */
extern cdd_c_error_t is_fixed_operation_method(const char *method);

/**
 * @brief Parses param in from the given input.
 */
extern cdd_c_error_t parse_param_in(const char *in,
                                    enum OpenAPI_ParamIn *_out_val);

/**
 * @brief Parses param style from the given input.
 */
extern cdd_c_error_t parse_param_style(const char *s,
                                       enum OpenAPI_Style *_out_val);

/**
 * @brief Executes the param type is primitive operation.
 */
extern C_CDD_EXPORT cdd_c_error_t param_type_is_primitive(const char *type);

/**
 * @brief Executes the param type is object like operation.
 */
extern C_CDD_EXPORT cdd_c_error_t
param_type_is_object_like(const struct OpenAPI_Parameter *p);

/**
 * @brief Executes the validate parameter style operation.
 */
extern cdd_c_error_t validate_parameter_style(const struct OpenAPI_Parameter *p,
                                              int has_content);

/**
 * @brief Executes the component key is valid operation.
 */
extern C_CDD_EXPORT cdd_c_error_t component_key_is_valid(const char *name);

/**
 * @brief media type key is valid.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_key_is_valid(const char *name);

/**
 * @brief Executes the validate component key map operation.
 */
extern C_CDD_EXPORT cdd_c_error_t
validate_component_key_map(const JSON_Object *obj);

/**
 * @brief validate media type key map.
 */
extern cdd_c_error_t validate_media_type_key_map(const JSON_Object *obj);

/**
 * @brief Executes the header name is content type operation.
 */
extern cdd_c_error_t header_name_is_content_type(const char *name);

/**
 * @brief Executes the header param is reserved operation.
 */
extern C_CDD_EXPORT cdd_c_error_t
header_param_is_reserved(const struct OpenAPI_Parameter *param);

/**
 * @brief Parses security type from the given input.
 */
extern cdd_c_error_t parse_security_type(const char *type,
                                         enum OpenAPI_SecurityType *_out_val);

/**
 * @brief Parses security in from the given input.
 */
extern cdd_c_error_t parse_security_in(const char *in,
                                       enum OpenAPI_SecurityIn *_out_val);

/**
 * @brief Parses oauth flow type from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_oauth_flow_type(const char *flow, enum OpenAPI_OAuthFlowType *_out_val);

/**
 * @brief Parses xml node type from the given input.
 */
extern cdd_c_error_t parse_xml_node_type(const char *node_type,
                                         enum OpenAPI_XmlNodeType *_out_val);

/**
 * @brief Parses any value from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_any_value(const JSON_Value *val,
                                                  struct OpenAPI_Any *out);

/**
 * @brief Parses any field from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_any_field(const JSON_Object *obj,
                                                  const char *key,
                                                  struct OpenAPI_Any *out,
                                                  int *out_set);

/**
 * @brief Parses any array from the given input.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_any_array(const JSON_Array *arr,
                                                  struct OpenAPI_Any **out,
                                                  size_t *out_count);

/**
 * @brief Executes the key in list operation.
 */
extern C_CDD_EXPORT cdd_c_error_t key_in_list(const char *key,
                                              const char **list, size_t count);

/**
 * @brief Checks if extension key.
 */
extern C_CDD_EXPORT cdd_c_error_t is_extension_key(const char *key);

/**
 * @brief Executes the clone json value operation.
 */
extern cdd_c_error_t clone_json_value(const JSON_Value *val,
                                      JSON_Value **_out_val);

/**
 * @brief Collects schema extras.
 */
extern cdd_c_error_t collect_schema_extras(const JSON_Object *obj,
                                           const char **skip_keys,
                                           size_t skip_count, char **out_json);

/**
 * @brief Collects extensions.
 */
extern cdd_c_error_t collect_extensions(const JSON_Object *obj,
                                        char **out_json);

/**
 * @brief Executes the url has query or fragment operation.
 */
extern cdd_c_error_t url_has_query_or_fragment(const char *url);

/**
 * @brief Executes the openapi version supported operation.
 */
extern cdd_c_error_t openapi_version_supported(const char *version);

/**
 * @brief Executes the example fields valid operation.
 */
extern cdd_c_error_t example_fields_valid(const struct OpenAPI_Example *ex);

/**
 * @brief Executes the object has example and examples operation.
 */
extern cdd_c_error_t object_has_example_and_examples(const JSON_Object *obj);

/* --- openapi_examples.c declarations --- */

/**
 * @brief Creates a deep copy of example fields.
 */
extern cdd_c_error_t copy_example_fields(struct OpenAPI_Example *dst,
                                         const struct OpenAPI_Example *src);

/**
 * @brief Retrieves the component example.
 */
extern cdd_c_error_t find_component_example(const struct OpenAPI_Spec *spec,
                                            const char *ref,
                                            struct OpenAPI_Example **_out_val);

/**
 * @brief Parses example object from the given input.
 */
extern cdd_c_error_t parse_example_object(const JSON_Object *ex_obj,
                                          const char *name,
                                          struct OpenAPI_Example *out,
                                          const struct OpenAPI_Spec *spec,
                                          int resolve_refs);

/**
 * @brief Parses examples object from the given input.
 */
extern cdd_c_error_t parse_examples_object(const JSON_Object *examples,
                                           struct OpenAPI_Example **out,
                                           size_t *out_count,
                                           const struct OpenAPI_Spec *spec,
                                           int resolve_refs);

/**
 * @brief Parses media examples from the given input.
 */
extern cdd_c_error_t
parse_media_examples(const JSON_Object *media_obj, struct OpenAPI_Any *example,
                     int *example_set, struct OpenAPI_Example **examples,
                     size_t *n_examples, const struct OpenAPI_Spec *spec,
                     int resolve_refs);

/**
 * @brief Parses oauth scopes from the given input.
 */
extern cdd_c_error_t parse_oauth_scopes(const JSON_Object *scopes_obj,
                                        struct OpenAPI_OAuthScope **out,
                                        size_t *out_count);

/**
 * @brief Parses oauth flows from the given input.
 */
extern cdd_c_error_t parse_oauth_flows(const JSON_Object *flows_obj,
                                       struct OpenAPI_SecurityScheme *out);

/* --- openapi_uri.c declarations --- */

/**
 * @brief Executes the json pointer unescape operation.
 */
extern cdd_c_error_t json_pointer_unescape(const char *in, char **_out_val);

/**
 * @brief Executes the uri has scheme prefix operation.
 */
extern cdd_c_error_t uri_has_scheme_prefix(const char *uri, size_t len);

/**
 * @brief Executes the uri base len operation.
 */
extern cdd_c_error_t uri_base_len(const char *uri, size_t *_out_val);

/**
 * @brief Executes the uri scheme len operation.
 */
extern cdd_c_error_t uri_scheme_len(const char *uri, size_t len,
                                    size_t *_out_val);

/**
 * @brief Executes the dup substr operation.
 */
extern cdd_c_error_t dup_substr(const char *src, size_t len, char **_out_val);

/**
 * @brief Executes the normalize path operation.
 */
extern cdd_c_error_t normalize_path(const char *path, char **_out_val);

/**
 * @brief Executes the resolve uri reference operation.
 */
extern cdd_c_error_t resolve_uri_reference(const char *base_uri,
                                           const char *ref, char **_out_val);

/**
 * @brief Executes the compute document uri operation.
 */
extern cdd_c_error_t compute_document_uri(const char *self_uri,
                                          const char *retrieval_uri,
                                          char **_out_val);

/**
 * @brief Executes the root has openapi fields operation.
 */
extern cdd_c_error_t root_has_openapi_fields(const JSON_Object *root_obj);

/**
 * @brief Executes the root is schema document operation.
 */
extern cdd_c_error_t root_is_schema_document(const JSON_Value *root,
                                             const JSON_Object *root_obj);

/**
 * @brief Executes the store schema root json operation.
 */
extern cdd_c_error_t store_schema_root_json(struct OpenAPI_Spec *spec,
                                            const JSON_Value *root);

/**
 * @brief Executes the resolve ref target operation.
 */
extern cdd_c_error_t resolve_ref_target(const struct OpenAPI_Spec *spec,
                                        const char *ref,
                                        struct ResolvedRefTarget *_out_val);

/**
 * @brief Executes the ref base matches self operation.
 */
extern cdd_c_error_t ref_base_matches_self(const struct OpenAPI_Spec *spec,
                                           const char *ref, const char *hash);

/**
 * @brief Executes the ref name from prefix operation.
 */
extern cdd_c_error_t ref_name_from_prefix(const struct OpenAPI_Spec *spec,
                                          const char *ref, const char *prefix,
                                          char **_out_val);

/* --- openapi_find.c declarations --- */

/**
 * @brief Retrieves the component parameter.
 */
extern cdd_c_error_t
find_component_parameter(const struct OpenAPI_Spec *spec, const char *ref,
                         struct OpenAPI_Parameter **_out_val);

/**
 * @brief Retrieves the component response.
 */
extern cdd_c_error_t
find_component_response(const struct OpenAPI_Spec *spec, const char *ref,
                        struct OpenAPI_Response **_out_val);

/**
 * @brief Retrieves the component header.
 */
extern cdd_c_error_t find_component_header(const struct OpenAPI_Spec *spec,
                                           const char *ref,
                                           struct OpenAPI_Header **_out_val);

/**
 * @brief Retrieves the component request body.
 */
extern cdd_c_error_t
find_component_request_body(const struct OpenAPI_Spec *spec, const char *ref,
                            struct OpenAPI_RequestBody **_out_val);

/**
 * @brief Retrieves the component media type.
 */
extern cdd_c_error_t
find_component_media_type(const struct OpenAPI_Spec *spec, const char *ref,
                          struct OpenAPI_MediaType **_out_val);

/**
 * @brief Retrieves the component link.
 */
extern cdd_c_error_t find_component_link(const struct OpenAPI_Spec *spec,
                                         const char *ref,
                                         struct OpenAPI_Link **_out_val);

/**
 * @brief Retrieves the component callback.
 */
extern cdd_c_error_t
find_component_callback(const struct OpenAPI_Spec *spec, const char *ref,
                        struct OpenAPI_Callback **_out_val);

/**
 * @brief Retrieves the component path item.
 */
extern cdd_c_error_t find_component_path_item(const struct OpenAPI_Spec *spec,
                                              const char *ref,
                                              struct OpenAPI_Path **_out_val);

/* --- openapi_copy_schema.c declarations --- */

/**
 * @brief Creates a deep copy of schema ref.
 */
extern cdd_c_error_t copy_schema_ref(struct OpenAPI_SchemaRef *dst,
                                     const struct OpenAPI_SchemaRef *src);

/**
 * @brief Creates a deep copy of item schema as array.
 */
extern cdd_c_error_t
copy_item_schema_as_array(struct OpenAPI_SchemaRef *dst,
                          const struct OpenAPI_SchemaRef *item);

/**
 * @brief Creates a deep copy of any value.
 */
extern cdd_c_error_t copy_any_value(struct OpenAPI_Any *dst,
                                    const struct OpenAPI_Any *src);

/* --- openapi_copy_components.c declarations --- */

/**
 * @brief Creates a deep copy of parameter fields.
 */
extern cdd_c_error_t copy_parameter_fields(struct OpenAPI_Parameter *dst,
                                           const struct OpenAPI_Parameter *src);

/**
 * @brief Creates a deep copy of header fields.
 */
extern cdd_c_error_t copy_header_fields(struct OpenAPI_Header *dst,
                                        const struct OpenAPI_Header *src);

/**
 * @brief Creates a deep copy of encoding fields.
 */
extern cdd_c_error_t copy_encoding_fields(struct OpenAPI_Encoding *dst,
                                          const struct OpenAPI_Encoding *src);

/**
 * @brief Creates a deep copy of media type fields.
 */
extern cdd_c_error_t
copy_media_type_fields(struct OpenAPI_MediaType *dst,
                       const struct OpenAPI_MediaType *src);

/**
 * @brief Creates a deep copy of media type array.
 */
extern cdd_c_error_t copy_media_type_array(struct OpenAPI_MediaType **dst,
                                           size_t *dst_count,
                                           const struct OpenAPI_MediaType *src,
                                           size_t src_count);

/**
 * @brief Creates a deep copy of response fields.
 */
extern cdd_c_error_t copy_response_fields(struct OpenAPI_Response *dst,
                                          const struct OpenAPI_Response *src);

/* --- openapi_copy_operations.c declarations --- */

/**
 * @brief Creates a deep copy of server object.
 */
extern cdd_c_error_t copy_server_object(struct OpenAPI_Server *dst,
                                        const struct OpenAPI_Server *src);

/**
 * @brief Creates a deep copy of link fields.
 */
extern cdd_c_error_t copy_link_fields(struct OpenAPI_Link *dst,
                                      const struct OpenAPI_Link *src);

/**
 * @brief Creates a deep copy of security requirement sets.
 */
extern cdd_c_error_t copy_security_requirement_sets(
    struct OpenAPI_SecurityRequirementSet **dst, size_t *dst_count,
    const struct OpenAPI_SecurityRequirementSet *src, size_t src_count);

/**
 * @brief Creates a deep copy of callback fields.
 */
extern cdd_c_error_t copy_callback_fields(struct OpenAPI_Callback *dst,
                                          const struct OpenAPI_Callback *src);

/**
 * @brief Creates a deep copy of operation fields.
 */
extern cdd_c_error_t copy_operation_fields(struct OpenAPI_Operation *dst,
                                           const struct OpenAPI_Operation *src);

/**
 * @brief Creates a deep copy of path fields.
 */
extern cdd_c_error_t copy_path_fields(struct OpenAPI_Path *dst,
                                      const struct OpenAPI_Path *src);

/**
 * @brief Creates a deep copy of request body fields.
 */
extern cdd_c_error_t
copy_request_body_fields(struct OpenAPI_RequestBody *dst,
                         const struct OpenAPI_RequestBody *src);

/* --- openapi_metadata.c declarations --- */

/**
 * @brief Parses info from the given input.
 */
extern cdd_c_error_t parse_info(const JSON_Object *root_obj,
                                struct OpenAPI_Spec *out);

/**
 * @brief Parses external docs from the given input.
 */
extern cdd_c_error_t parse_external_docs(const JSON_Object *obj,
                                         struct OpenAPI_ExternalDocs *out);

/**
 * @brief Parses discriminator object from the given input.
 */
extern cdd_c_error_t
parse_discriminator_object(const JSON_Object *obj,
                           struct OpenAPI_Discriminator *out);

/**
 * @brief Parses xml object from the given input.
 */
extern cdd_c_error_t parse_xml_object(const JSON_Object *obj,
                                      struct OpenAPI_Xml *out);

/**
 * @brief Parses tags from the given input.
 */
extern cdd_c_error_t parse_tags(const JSON_Object *root_obj,
                                struct OpenAPI_Spec *out);

/**
 * @brief Executes the tag index by name operation.
 */
extern cdd_c_error_t tag_index_by_name(const struct OpenAPI_Spec *spec,
                                       const char *name, size_t *out_idx);

/**
 * @brief Executes the detect tag cycle operation.
 */
extern cdd_c_error_t detect_tag_cycle(const struct OpenAPI_Spec *spec,
                                      size_t idx, int *state);

/**
 * @brief Executes the validate tag parents operation.
 */
extern cdd_c_error_t validate_tag_parents(const struct OpenAPI_Spec *spec);

/**
 * @brief Executes the server variable defined operation.
 */
extern cdd_c_error_t server_variable_defined(const struct OpenAPI_Server *srv,
                                             const char *name);

/**
 * @brief Executes the server variable seen operation.
 */
extern cdd_c_error_t server_variable_seen(char **seen, size_t seen_count,
                                          const char *name);

/**
 * @brief Executes the validate server url variables operation.
 */
extern cdd_c_error_t
validate_server_url_variables(const struct OpenAPI_Server *srv);

/**
 * @brief Parses server object from the given input.
 */
extern cdd_c_error_t parse_server_object(const JSON_Object *srv_obj,
                                         struct OpenAPI_Server *out_srv);

/**
 * @brief Parses servers array from the given input.
 */
extern cdd_c_error_t parse_servers_array(const JSON_Object *parent,
                                         const char *key,
                                         struct OpenAPI_Server **out_servers,
                                         size_t *out_count);

/* --- openapi_schemas.c declarations --- */

/**
 * @brief Parses schema type from the given input.
 */
extern cdd_c_error_t parse_schema_type(const JSON_Object *schema,
                                       int *out_nullable, char **_out_val);

/**
 * @brief Parses schema constraints from the given input.
 */
extern cdd_c_error_t
parse_schema_constraints(const JSON_Object *schema,
                         struct SchemaConstraintTarget *target);

/**
 * @brief Parses string enum array from the given input.
 */
extern cdd_c_error_t parse_string_enum_array(const JSON_Array *arr, char ***out,
                                             size_t *out_count);

/**
 * @brief Creates a deep copy of string array.
 */
extern cdd_c_error_t copy_string_array(char ***dst, size_t *dst_count,
                                       char **src, size_t src_count);

/**
 * @brief Executes the schema is string enum only operation.
 */
extern cdd_c_error_t schema_is_string_enum_only(const JSON_Object *schema_obj);

/**
 * @brief Executes the schema is struct compatible operation.
 */
extern cdd_c_error_t schema_is_struct_compatible(const JSON_Value *schema_val,
                                                 const JSON_Object *schema_obj);

/**
 * @brief Executes the schema has composition operation.
 */
extern cdd_c_error_t schema_has_composition(const JSON_Object *schema_obj);

/**
 * @brief Parses schema array ref from the given input.
 */
extern cdd_c_error_t parse_schema_array_ref(const JSON_Array *arr,
                                            struct OpenAPI_SchemaRef **out,
                                            size_t *out_count,
                                            const struct OpenAPI_Spec *spec);

/**
 * @brief Parses schema ref ptr from the given input.
 */
extern cdd_c_error_t parse_schema_ref_ptr(const JSON_Object *obj,
                                          struct OpenAPI_SchemaRef **out,
                                          const struct OpenAPI_Spec *spec);

/* --- openapi_schema_ref.c declarations --- */

/**
 * @brief Parses schema ref from the given input.
 */
extern cdd_c_error_t parse_schema_ref(const JSON_Object *schema,
                                      struct OpenAPI_SchemaRef *out,
                                      const struct OpenAPI_Spec *spec);

#include "openapi/parse/openapi_internal_components.h"

#ifdef CDD_BUILD_TESTS
#include "openapi/parse/openapi_test_helpers.h"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENAPI_INTERNAL_H */
