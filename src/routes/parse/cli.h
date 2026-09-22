/**
 * @file cli.h
 * @brief C to OpenAPI CLI routes parser and generator.
 *
 * Provides command line interfaces and helper functions for:
 * - Parsing C source/header files into OpenAPI 3.x specifications
 * (`c2openapi`).
 * - Emitting documentation JSON structures (`to_docs_json`).
 * - Generating multi-language bindings (`cdd-c bind`).
 * - Converting between C code signatures, docstrings, and OpenAPI models.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_C2OPENAPI_CLI_H
#define C_CDD_C2OPENAPI_CLI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "docstrings/parse/doc.h"
#include "openapi/emit/openapi.h"
#include "routes/emit/operation.h"
/* clang-format on */

struct TypeDefList;

/**
 * @brief Registers parsed C types into an OpenAPI specification schemas list.
 *
 * @param[in,out] spec OpenAPI specification to populate.
 * @param[in] types List of parsed C type definitions.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_register_types(
    struct OpenAPI_Spec *spec, const struct TypeDefList *types);

/**
 * @brief Main CLI entry point for c2openapi command.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_cli_main(int argc, char **argv);

/**
 * @brief Main CLI entry point for to_docs_json command.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t to_docs_json_cli_main(int argc, char **argv);

/**
 * @brief Main CLI entry point for to_openapi command.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t to_openapi_cli_main(int argc, char **argv);

/**
 * @brief Main CLI entry point for from_openapi command.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t from_openapi_cli_main(int argc, char **argv);

/**
 * @brief Main CLI entry point for generate_bindings command.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t generate_bindings_cli_main(int argc,
                                                             char **argv);

/**
 * @brief Checks if a path points to a C source or header file.
 *
 * @param[in] path File path.
 * @param[out] out_is_source Pointer to store 1 if source/header, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * out_is_source.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_is_source_file(const char *path,
                                                           int *out_is_source);

/**
 * @brief Checks if an OpenAPI specification already contains a tag with the
 * given name.
 *
 * @param[in] spec OpenAPI spec.
 * @param[in] name Tag name.
 * @param[out] out_has_tag Pointer to store 1 if found, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * out_has_tag.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_spec_has_tag(
    const struct OpenAPI_Spec *spec, const char *name, int *out_has_tag);

/**
 * @brief Adds a tag to an OpenAPI specification if not already present.
 *
 * @param[in,out] spec OpenAPI spec.
 * @param[in] name Tag name.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
c2openapi_spec_add_tag(struct OpenAPI_Spec *spec, const char *name);

/**
 * @brief Finds a tag by name in an OpenAPI specification.
 *
 * @param[in] spec OpenAPI spec.
 * @param[in] name Tag name.
 * @param[out] out_val Pointer to store matching tag pointer, or NULL.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_UNKNOWN if not found, or error
 * code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_spec_find_tag(
    struct OpenAPI_Spec *spec, const char *name, struct OpenAPI_Tag **out_val);

/**
 * @brief Maps DocSecurityType enumeration to OpenAPI_SecurityType.
 *
 * @param[in] type Input doc security type.
 * @param[out] out_val Output OpenAPI security type.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_map_doc_security_type(
    enum DocSecurityType type, enum OpenAPI_SecurityType *out_val);

/**
 * @brief Maps DocSecurityIn enumeration to OpenAPI_SecurityIn.
 *
 * @param[in] in Input doc security location.
 * @param[out] out_val Output OpenAPI security location.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on invalid
 * input.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_map_doc_security_in(
    enum DocSecurityIn in, enum OpenAPI_SecurityIn *out_val);

/**
 * @brief Maps DocOAuthFlowType enumeration to OpenAPI_OAuthFlowType.
 *
 * @param[in] type Input doc oauth flow type.
 * @param[out] out_val Output OpenAPI oauth flow type.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_UNKNOWN on invalid/unset type.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_map_doc_flow_type(
    enum DocOAuthFlowType type, enum OpenAPI_OAuthFlowType *out_val);

/**
 * @brief Deep-copies server variables from a doc server to an OpenAPI server
 * object.
 *
 * @param[in,out] dst Destination OpenAPI server.
 * @param[in] src Source doc server.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_copy_doc_server_variables(
    struct OpenAPI_Server *dst, const struct DocServer *src);

/**
 * @brief Frees heap allocations in an OpenAPI server's variables array.
 *
 * @param[in,out] srv OpenAPI server to clean.
 */
extern C_CDD_EXPORT void
c2openapi_free_openapi_server_variables(struct OpenAPI_Server *srv);

/**
 * @brief Merges scopes into an existing OpenAPI OAuth flow object.
 *
 * @param[in,out] dst Destination OpenAPI OAuth flow.
 * @param[in] src Source doc OAuth flow.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_merge_scopes(
    struct OpenAPI_OAuthFlow *dst, const struct DocOAuthFlow *src);

/**
 * @brief Finds an existing OAuth flow of a given type in a security scheme.
 *
 * @param[in] scheme Security scheme.
 * @param[in] type OAuth flow type.
 * @param[out] out_val Pointer to store matching flow pointer, or NULL.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_find_oauth_flow(
    struct OpenAPI_SecurityScheme *scheme, enum OpenAPI_OAuthFlowType type,
    struct OpenAPI_OAuthFlow **out_val);

/**
 * @brief Merges OAuth flow URLs and scopes into an existing flow.
 *
 * @param[in,out] dst Destination OAuth flow.
 * @param[in] src Source doc OAuth flow.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_merge_oauth_flow(
    struct OpenAPI_OAuthFlow *dst, const struct DocOAuthFlow *src);

/**
 * @brief Validates that required URLs are present for a given OAuth flow type.
 *
 * @param[in] flow Doc OAuth flow.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on
 * invalid/missing fields.
 */
extern C_CDD_EXPORT cdd_c_error_t
c2openapi_validate_doc_oauth_flow(const struct DocOAuthFlow *flow);

/**
 * @brief Adds a security scheme definition from doc comments to an OpenAPI
 * specification.
 *
 * @param[in,out] spec OpenAPI specification.
 * @param[in] doc Doc security scheme definition.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_spec_add_security_scheme(
    struct OpenAPI_Spec *spec, const struct DocSecurityScheme *doc);

/**
 * @brief Applies all security schemes in doc metadata to an OpenAPI
 * specification.
 *
 * @param[in,out] spec OpenAPI specification.
 * @param[in] meta Doc metadata containing security schemes.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_apply_doc_security_schemes(
    struct OpenAPI_Spec *spec, const struct DocMetadata *meta);

/**
 * @brief Adds or merges OAuth flows into an OpenAPI security scheme.
 *
 * @param[in,out] scheme OpenAPI security scheme.
 * @param[in] doc Source doc security scheme.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_add_oauth_flows(
    struct OpenAPI_SecurityScheme *scheme, const struct DocSecurityScheme *doc);

/**
 * @brief Appends root-level security requirements from doc metadata to an
 * OpenAPI spec.
 *
 * @param[in,out] spec OpenAPI specification.
 * @param[in] meta Doc metadata.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_append_root_security(
    struct OpenAPI_Spec *spec, const struct DocMetadata *meta);

/**
 * @brief Appends root-level server definitions from doc metadata to an OpenAPI
 * spec.
 *
 * @param[in,out] spec OpenAPI specification.
 * @param[in] meta Doc metadata.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_append_root_servers(
    struct OpenAPI_Spec *spec, const struct DocMetadata *meta);

/**
 * @brief Applies global metadata (info, license, externalDocs, servers,
 * security) to spec.
 *
 * @param[in,out] spec OpenAPI specification.
 * @param[in] meta Doc metadata.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_apply_doc_global_meta(
    struct OpenAPI_Spec *spec, const struct DocMetadata *meta);

/**
 * @brief Applies tag metadata (summary, description, externalDocs) to a spec
 * tag.
 *
 * @param[in,out] spec OpenAPI specification.
 * @param[in] meta Doc tag metadata.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_spec_apply_tag_meta(
    struct OpenAPI_Spec *spec, const struct DocTagMeta *meta);

/**
 * @brief Applies all tag metadata entries in doc metadata to an OpenAPI spec.
 *
 * @param[in,out] spec OpenAPI specification.
 * @param[in] meta Doc metadata.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_apply_doc_tag_meta(
    struct OpenAPI_Spec *spec, const struct DocMetadata *meta);

/**
 * @brief Collects tags from an operation and ensures they exist in spec->tags.
 *
 * @param[in,out] spec OpenAPI specification.
 * @param[in] op OpenAPI operation.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_collect_tags_from_op(
    struct OpenAPI_Spec *spec, const struct OpenAPI_Operation *op);

/**
 * @brief Collects tags from an array of OpenAPI paths.
 *
 * @param[in,out] spec OpenAPI specification.
 * @param[in] paths Array of OpenAPI paths.
 * @param[in] n_paths Number of paths.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_collect_tags_from_paths(
    struct OpenAPI_Spec *spec, const struct OpenAPI_Path *paths,
    size_t n_paths);

/**
 * @brief Collects all tags across all paths and webhooks in an OpenAPI spec.
 *
 * @param[in,out] spec OpenAPI specification.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
c2openapi_collect_spec_tags(struct OpenAPI_Spec *spec);

/**
 * @brief Parses a C function signature string into function name, return type,
 * and arguments.
 *
 * @param[in] sig_str C function signature string (e.g. "void foo(int a, char
 * *b)").
 * @param[out] out Parsed signature structure.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_parse_c_signature_string(
    const char *sig_str, struct C2OpenAPI_ParsedSig *out);

/**
 * @brief Frees memory allocated within a C2OpenAPI_ParsedSig structure.
 *
 * @param[in,out] sig Parsed signature structure to clean.
 */
extern C_CDD_EXPORT void
c2openapi_free_parsed_sig(struct C2OpenAPI_ParsedSig *sig);

/**
 * @brief Parses a single C source/header file and populates the OpenAPI spec
 * with routes and types.
 *
 * @param[in] path Path to C source/header file.
 * @param[in,out] spec OpenAPI specification to populate.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
c2openapi_process_file(const char *path, struct OpenAPI_Spec *spec);

/**
 * @brief File walker callback for directory traversal.
 *
 * @param[in] path File path encountered during walk.
 * @param[in,out] user_data Pointer to OpenAPI_Spec structure.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_walker_cb(const char *path,
                                                      void *user_data);

/**
 * @brief Loads a base OpenAPI JSON specification from disk.
 *
 * @param[in] path Path to JSON file.
 * @param[out] spec OpenAPI specification to populate.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
c2openapi_load_base_spec(const char *path, struct OpenAPI_Spec *spec);

/** @brief Alias for c2openapi_is_source_file. */
#define is_source_file c2openapi_is_source_file
/** @brief Alias for c2openapi_spec_has_tag. */
#define spec_has_tag c2openapi_spec_has_tag
/** @brief Alias for c2openapi_spec_add_tag. */
#define spec_add_tag c2openapi_spec_add_tag
/** @brief Alias for c2openapi_spec_find_tag. */
#define spec_find_tag c2openapi_spec_find_tag
/** @brief Alias for c2openapi_map_doc_security_in. */
#define map_doc_security_in c2openapi_map_doc_security_in
/** @brief Alias for c2openapi_map_doc_flow_type. */
#define map_doc_flow_type c2openapi_map_doc_flow_type
/** @brief Alias for c2openapi_copy_doc_server_variables. */
#define copy_doc_server_variables c2openapi_copy_doc_server_variables
/** @brief Alias for c2openapi_free_openapi_server_variables. */
#define free_openapi_server_variables c2openapi_free_openapi_server_variables
/** @brief Alias for c2openapi_merge_scopes. */
#define merge_scopes c2openapi_merge_scopes
/** @brief Alias for c2openapi_find_oauth_flow. */
#define find_oauth_flow c2openapi_find_oauth_flow
/** @brief Alias for c2openapi_merge_oauth_flow. */
#define merge_oauth_flow c2openapi_merge_oauth_flow
/** @brief Alias for c2openapi_validate_doc_oauth_flow. */
#define validate_doc_oauth_flow c2openapi_validate_doc_oauth_flow
/** @brief Alias for c2openapi_spec_add_security_scheme. */
#define spec_add_security_scheme c2openapi_spec_add_security_scheme
/** @brief Alias for c2openapi_apply_doc_security_schemes. */
#define apply_doc_security_schemes c2openapi_apply_doc_security_schemes
/** @brief Alias for c2openapi_add_oauth_flows. */
#define add_oauth_flows c2openapi_add_oauth_flows
/** @brief Alias for c2openapi_append_root_security. */
#define append_root_security c2openapi_append_root_security
/** @brief Alias for c2openapi_append_root_servers. */
#define append_root_servers c2openapi_append_root_servers
/** @brief Alias for c2openapi_apply_doc_global_meta. */
#define apply_doc_global_meta c2openapi_apply_doc_global_meta
/** @brief Alias for c2openapi_spec_apply_tag_meta. */
#define spec_apply_tag_meta c2openapi_spec_apply_tag_meta
/** @brief Alias for c2openapi_apply_doc_tag_meta. */
#define apply_doc_tag_meta c2openapi_apply_doc_tag_meta
/** @brief Alias for c2openapi_collect_tags_from_op. */
#define collect_tags_from_op c2openapi_collect_tags_from_op
/** @brief Alias for c2openapi_collect_tags_from_paths. */
#define collect_tags_from_paths c2openapi_collect_tags_from_paths
/** @brief Alias for c2openapi_collect_spec_tags. */
#define collect_spec_tags c2openapi_collect_spec_tags
/** @brief Alias for c2openapi_parse_c_signature_string. */
#define parse_c_signature_string c2openapi_parse_c_signature_string
/** @brief Alias for c2openapi_free_parsed_sig. */
#define free_parsed_sig c2openapi_free_parsed_sig
/** @brief Alias for c2openapi_process_file. */
#define process_file c2openapi_process_file
/** @brief Alias for c2openapi_walker_cb. */
#define walker_cb c2openapi_walker_cb
/** @brief Alias for c2openapi_load_base_spec. */
#define load_base_spec c2openapi_load_base_spec
/** @brief Alias for c2openapi_map_doc_security_type. */
#define map_doc_security_type c2openapi_map_doc_security_type
/** @brief Alias for c2openapi_apply_all_doc_meta. */
#define apply_all_doc_meta c2openapi_apply_all_doc_meta
/** @brief Alias for c2openapi_set_json_allocators. */
#define set_json_allocators c2openapi_set_json_allocators

/**
 * @brief Sets custom JSON memory allocation functions in the library.
 *
 * @param[in] malloc_fun Custom memory allocation function.
 * @param[in] free_fun Custom memory deallocation function.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_set_json_allocators(
    void *(*malloc_fun)(size_t), void (*free_fun)(void *));

/**
 * @brief Applies all doc metadata (tags, security schemes, and global).
 *
 * @param[in,out] spec OpenAPI specification.
 * @param[in] meta Doc metadata.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t c2openapi_apply_all_doc_meta(
    struct OpenAPI_Spec *spec, const struct DocMetadata *meta);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C2OPENAPI_CLI_H */
