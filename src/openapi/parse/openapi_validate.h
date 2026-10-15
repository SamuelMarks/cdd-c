/**
 * @file openapi_validate.h
 * @brief Internal OpenAPI structural, parameter, path template, and cycle
 * validation declarations.
 * @author Samuel Marks
 */

#ifndef OPENAPI_VALIDATE_H
#define OPENAPI_VALIDATE_H

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
 * @brief Validates that a parameter's style and type combination is legal.
 *
 * @param[in] p Parameter to validate.
 * @param[in] has_content Non-zero if parameter uses `content` instead of
 * `schema`.
 * @return CDD_C_SUCCESS on valid style, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
validate_parameter_style(const struct OpenAPI_Parameter *p, int has_content);

/**
 * @brief Checks if a component map key matches `^[a-zA-Z0-9._-]+$`.
 *
 * @param[in] name Component key string.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if valid, CDD_C_SUCCESS (0) if
 * invalid.
 */
extern C_CDD_EXPORT cdd_c_error_t component_key_is_valid(const char *name);

/**
 * @brief Checks if a media type component key is valid.
 *
 * @param[in] name Media type key string.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if valid, CDD_C_SUCCESS (0) if
 * invalid.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_key_is_valid(const char *name);

/**
 * @brief Validates all keys in a components sub-map.
 *
 * @param[in] obj JSON object representing a component map.
 * @return CDD_C_SUCCESS if all keys are valid, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
validate_component_key_map(const JSON_Object *obj);

/**
 * @brief Validates all keys in `components.mediaTypes`.
 *
 * @param[in] obj JSON object representing the `mediaTypes` map.
 * @return CDD_C_SUCCESS if all keys are valid, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
validate_media_type_key_map(const JSON_Object *obj);

/**
 * @brief Checks if an OpenAPI version string (`3.0.x`, `3.1.x`, `3.2.x`) is
 * supported.
 *
 * @param[in] version Version string.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if supported, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
openapi_version_supported(const char *version);

/**
 * @brief Checks if an Example object's mutual exclusion rules are satisfied.
 *
 * @param[in] ex Example structure to check.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if valid, CDD_C_SUCCESS (0) if
 * invalid.
 */
extern C_CDD_EXPORT cdd_c_error_t
example_fields_valid(const struct OpenAPI_Example *ex);

/**
 * @brief Checks if a JSON object contains both `example` and `examples`.
 *
 * @param[in] obj JSON object to inspect.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if both are present, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
object_has_example_and_examples(const JSON_Object *obj);

/**
 * @brief Checks if a server variable name is defined in `srv->variables`.
 *
 * @param[in] srv Server structure.
 * @param[in] name Variable name to check.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if defined, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
server_variable_defined(const struct OpenAPI_Server *srv, const char *name);

/**
 * @brief Checks if a server variable name has already been seen in a list.
 *
 * @param[in] seen Array of seen variable names.
 * @param[in] seen_count Number of elements in `seen`.
 * @param[in] name Variable name to check.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if seen, CDD_C_SUCCESS (0) otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t server_variable_seen(char **seen,
                                                       size_t seen_count,
                                                       const char *name);

/**
 * @brief Validates `{var}` placeholders in a Server URL against its `variables`
 * map.
 *
 * @param[in] srv Server structure to validate.
 * @return CDD_C_SUCCESS if valid, or error code on mismatch/OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t
validate_server_url_variables(const struct OpenAPI_Server *srv);

/**
 * @brief Finds the index of a Tag by name in `spec->tags`.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] name Tag name to search for.
 * @param[out] out_idx Index of the tag.
 * @return CDD_C_SUCCESS on success, or error code if not found/invalid arg.
 */
extern C_CDD_EXPORT cdd_c_error_t tag_index_by_name(
    const struct OpenAPI_Spec *spec, const char *name, size_t *out_idx);

/**
 * @brief Detects parent cycles in `spec->tags` starting from `idx`.
 *
 * @param[in] spec OpenAPI specification.
 * @param[in] idx Starting tag index.
 * @param[in,out] state Visited state array (`0` unvisited, `1` visiting, `2`
 * done).
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if cycle detected, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
detect_tag_cycle(const struct OpenAPI_Spec *spec, size_t idx, int *state);

/**
 * @brief Validates that no tag parent cycles exist in `spec->tags`.
 *
 * @param[in] spec OpenAPI specification.
 * @return CDD_C_SUCCESS if valid, or error code on cycle/OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t
validate_tag_parents(const struct OpenAPI_Spec *spec);

/**
 * @brief Checks if a response status code key (`"default"`, `"200"`, `"2XX"`)
 * is valid.
 *
 * @param[in] code Status code key string.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if valid, CDD_C_SUCCESS (0) if
 * invalid.
 */
extern C_CDD_EXPORT cdd_c_error_t is_valid_response_code_key(const char *code);

/**
 * @brief Frees a list of extracted path template parameter names.
 *
 * @param[in] names Array of allocated name strings.
 * @param[in] count Number of elements.
 */
extern C_CDD_EXPORT void free_name_list(char **names, size_t count);

/**
 * @brief Checks if `name` is present in a string array `names`.
 *
 * @param[in] name Target string to find.
 * @param[in] names Array of strings.
 * @param[in] count Number of strings.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if present, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t name_in_list(const char *name, char **names,
                                               size_t count);

/**
 * @brief Extracts `{param}` template parameter names from a route path.
 *
 * @param[in] route Route path template string.
 * @param[out] out_names Output array of allocated parameter names.
 * @param[out] out_count Output number of extracted names.
 * @return CDD_C_SUCCESS on success, or error code on malformed template/OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t collect_path_template_names(
    const char *route, char ***out_names, size_t *out_count);

/**
 * @brief Finds a path parameter (`in: path`) by name in a parameter array.
 *
 * @param[in] params Array of parameters.
 * @param[in] n Number of parameters.
 * @param[in] name Path parameter name to look for.
 * @param[out] _out_val Matching parameter pointer, or NULL if not found.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_path_param(const struct OpenAPI_Parameter *params, size_t n,
                const char *name, const struct OpenAPI_Parameter **_out_val);

/**
 * @brief Validates that all `in: path` parameters in `params` are required and
 * appear in `template_names`.
 *
 * @param[in] params Array of parameters.
 * @param[in] n_params Number of parameters.
 * @param[in] template_names Extracted template parameter names.
 * @param[in] n_template_names Number of template parameter names.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t validate_path_params_list(
    const struct OpenAPI_Parameter *params, size_t n_params,
    char **template_names, size_t n_template_names);

/**
 * @brief Validates path template parameter coverage for a single operation.
 *
 * @param[in] path Parent path item.
 * @param[in] op Operation to validate.
 * @param[in] template_names Extracted template parameter names.
 * @param[in] n_template_names Number of template parameter names.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t validate_path_template_for_operation(
    const struct OpenAPI_Path *path, const struct OpenAPI_Operation *op,
    char **template_names, size_t n_template_names);

/**
 * @brief Validates path template syntax and parameter definitions across
 * `paths`.
 *
 * @param[in] paths Array of path items.
 * @param[in] n_paths Number of path items.
 * @return CDD_C_SUCCESS if valid, or error code on invalid template/missing
 * param.
 */
extern C_CDD_EXPORT cdd_c_error_t
validate_path_templates(const struct OpenAPI_Path *paths, size_t n_paths);

/**
 * @brief Normalizes a path template by replacing `{...}` segments with `{}` for
 * collision detection.
 *
 * @param[in] route Route path string.
 * @param[out] _out_val Newly allocated normalized signature string.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t
normalize_path_template_route(const char *route, char **_out_val);

/**
 * @brief Validates that no path template collisions exist across `paths`.
 *
 * @param[in] paths Array of path items.
 * @param[in] n_paths Number of path items.
 * @return CDD_C_SUCCESS if valid, or error code on collision/OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t validate_path_template_collisions(
    const struct OpenAPI_Path *paths, size_t n_paths);

/**
 * @brief Scans a parameter array for `in: querystring` and `in: query` usage.
 *
 * @param[in] params Array of parameters.
 * @param[in] n_params Number of parameters.
 * @param[in,out] qs_count Incremented for each `in: querystring` parameter.
 * @param[in,out] has_query Set to 1 if any `in: query` parameter is present.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
scan_querystring_usage(const struct OpenAPI_Parameter *params, size_t n_params,
                       size_t *qs_count, int *has_query);

/**
 * @brief Validates `in: querystring` parameter constraints across `paths`.
 *
 * @param[in] paths Array of path items.
 * @param[in] n_paths Number of path items.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
validate_querystring_usage(const struct OpenAPI_Path *paths, size_t n_paths);

/**
 * @brief Validates `in: querystring` usage inside callback objects.
 *
 * @param[in] callbacks Array of callback objects.
 * @param[in] n_callbacks Number of callback objects.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t validate_querystring_usage_in_callbacks(
    const struct OpenAPI_Callback *callbacks, size_t n_callbacks);

/**
 * @brief Validates `in: querystring` usage inside operations' callbacks.
 *
 * @param[in] ops Array of operations.
 * @param[in] n_ops Number of operations.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t validate_querystring_usage_in_operations(
    const struct OpenAPI_Operation *ops, size_t n_ops);

/**
 * @brief Validates `in: querystring` usage in callbacks nested under `paths`.
 *
 * @param[in] paths Array of path items.
 * @param[in] n_paths Number of path items.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t validate_querystring_usage_in_paths_callbacks(
    const struct OpenAPI_Path *paths, size_t n_paths);

/**
 * @brief Validates `in: querystring` usage in `spec->component_callbacks`.
 *
 * @param[in] spec OpenAPI specification.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
validate_querystring_usage_in_component_callbacks(
    const struct OpenAPI_Spec *spec);

/**
 * @brief Adds an `operationId` into a seen list, checking for duplicates.
 *
 * @param[in,out] ids Pointer to array of seen operation ID strings.
 * @param[in,out] count Pointer to count of seen operation IDs.
 * @param[in,out] cap Pointer to capacity of `ids` array.
 * @param[in] op_id Operation ID string.
 * @return CDD_C_SUCCESS if unique, CDD_C_ERROR_INVALID_ARGUMENT if duplicate,
 * or CDD_C_ERROR_MEMORY on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t add_unique_operation_id(char ***ids,
                                                          size_t *count,
                                                          size_t *cap,
                                                          const char *op_id);

/**
 * @brief Collects and validates uniqueness of `operationId`s across `paths`.
 *
 * @param[in] paths Array of path items.
 * @param[in] n_paths Number of path items.
 * @param[in,out] ids Pointer to array of seen operation IDs.
 * @param[in,out] count Pointer to count of seen operation IDs.
 * @param[in,out] cap Pointer to capacity of `ids` array.
 * @return CDD_C_SUCCESS on success, or error code on duplicate/OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t
collect_operation_ids(const struct OpenAPI_Path *paths, size_t n_paths,
                      char ***ids, size_t *count, size_t *cap);

/**
 * @brief Checks if a `$ref` matches a specific component path item name.
 *
 * @param[in] spec OpenAPI specification context.
 * @param[in] ref Reference string.
 * @param[in] name Component path item name.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if matches, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t path_item_ref_matches_component(
    const struct OpenAPI_Spec *spec, const char *ref, const char *name);

/**
 * @brief Checks if a component path item is referenced anywhere in `spec`.
 *
 * @param[in] spec OpenAPI specification context.
 * @param[in] name Component path item name.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if referenced, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t component_path_item_is_referenced(
    const struct OpenAPI_Spec *spec, const char *name);

/**
 * @brief Checks if a `$ref` matches a specific component callback name.
 *
 * @param[in] spec OpenAPI specification context.
 * @param[in] ref Reference string.
 * @param[in] name Component callback name.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if matches, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t callback_ref_matches_component(
    const struct OpenAPI_Spec *spec, const char *ref, const char *name);

/**
 * @brief Checks if a component callback is referenced in an array of
 * operations.
 *
 * @param[in] ops Array of operations.
 * @param[in] n_ops Number of operations.
 * @param[in] spec OpenAPI specification context.
 * @param[in] name Component callback name.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if referenced, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t component_callback_is_referenced_in_ops(
    const struct OpenAPI_Operation *ops, size_t n_ops,
    const struct OpenAPI_Spec *spec, const char *name);

/**
 * @brief Checks if a component callback is referenced anywhere in `spec`.
 *
 * @param[in] spec OpenAPI specification context.
 * @param[in] name Component callback name.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if referenced, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t component_callback_is_referenced(
    const struct OpenAPI_Spec *spec, const char *name);

/**
 * @brief Collects and validates uniqueness of `operationId`s across callbacks.
 *
 * @param[in] callbacks Array of callback objects.
 * @param[in] n_callbacks Number of callback objects.
 * @param[in,out] ids Pointer to array of seen operation IDs.
 * @param[in,out] count Pointer to count of seen operation IDs.
 * @param[in,out] cap Pointer to capacity of `ids` array.
 * @return CDD_C_SUCCESS on success, or error code on duplicate/OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t collect_callback_operation_ids_from_callbacks(
    const struct OpenAPI_Callback *callbacks, size_t n_callbacks, char ***ids,
    size_t *count, size_t *cap);

/**
 * @brief Collects and validates uniqueness of `operationId`s across operations'
 * callbacks.
 *
 * @param[in] ops Array of operations.
 * @param[in] n_ops Number of operations.
 * @param[in,out] ids Pointer to array of seen operation IDs.
 * @param[in,out] count Pointer to count of seen operation IDs.
 * @param[in,out] cap Pointer to capacity of `ids` array.
 * @return CDD_C_SUCCESS on success, or error code on duplicate/OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t
collect_callback_operation_ids_from_operations(
    const struct OpenAPI_Operation *ops, size_t n_ops, char ***ids,
    size_t *count, size_t *cap);

/**
 * @brief Collects and validates uniqueness of `operationId`s across callbacks
 * nested under `paths`.
 *
 * @param[in] paths Array of path items.
 * @param[in] n_paths Number of path items.
 * @param[in,out] ids Pointer to array of seen operation IDs.
 * @param[in,out] count Pointer to count of seen operation IDs.
 * @param[in,out] cap Pointer to capacity of `ids` array.
 * @return CDD_C_SUCCESS on success, or error code on duplicate/OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t collect_callback_operation_ids_from_paths(
    const struct OpenAPI_Path *paths, size_t n_paths, char ***ids,
    size_t *count, size_t *cap);

/**
 * @brief Validates that all `operationId` values across the entire `spec` are
 * unique.
 *
 * @param[in] spec OpenAPI specification.
 * @return CDD_C_SUCCESS if all operationIds are unique, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
validate_unique_operation_ids(const struct OpenAPI_Spec *spec);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENAPI_VALIDATE_H */
