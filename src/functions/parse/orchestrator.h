/**
 * @file orchestrator.h
 * @brief High-level orchestration for automatic code refactoring.
 *
 * Manages the pipeline of parsing, analysis, dependency graph construction,
 * error propagation, and code rewriting.
 *
 * @author Samuel Marks
 */

#ifndef REFACTOR_ORCHESTRATOR_H
#define REFACTOR_ORCHESTRATOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <c_cdd_export.h>
#include "cdd_c_error.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/**
 * @brief Represents a function in the call graph.
 */
struct FuncNode {
  size_t node_idx;            /**< Index in the CST node list */
  char *name;                 /**< Name of the function */
  int returns_void;           /**< True if function currently returns void */
  int returns_ptr;            /**< True if function returns pointer/struct */
  char *original_return_type; /**< Original return type string */
  int is_main;                /**< Special handling for main() */
  int contains_allocs;        /**< True if body performs allocations */
  int marked_for_refactor;    /**< True if signature/body needs rewriting */
  size_t token_start;         /**< Func start index in token list */
  size_t body_start;          /**< Index of opening brace '{' */
  size_t token_end;           /**< Func end index (exclusive) */
  size_t *callers;            /**< Indices of functions calling this function */
  size_t num_callers;         /**< Count of callers */
  size_t alloc_callers;       /**< Capacity of callers array */
};

/**
 * @brief Container for the dependency graph.
 */
struct DependencyGraph {
  struct FuncNode *nodes; /**< Array of function nodes */
  size_t count;           /**< Total number of functions */
};

/**
 * @brief Context structure for directory walk in fix command.
 */
struct FixWalkContext {
  int in_place;                   /**< in_place */
  const char *single_output_file; /**< single_output_file */
  int error_count;                /**< error_count */
};

/**
 * @brief Apply the "fix" workflow to a single C source string.
 *
 * @param[in] source_code The null-terminated C source code string.
 * @param[out] out_code Pointer to store the newly allocated result string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t orchestrate_fix(const char *source_code,
                                                  char **out_code);

/**
 * @brief Command-line entry point for the fix functionality.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t fix_code_main(int argc, char **argv);

/**
 * @brief Extract a slice of tokens into a temporary view.
 *
 * @param[in] src Source token list.
 * @param[in] start Starting token index.
 * @param[in] end Ending token index (exclusive).
 * @param[out] dst Destination token list view.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on error.
 */
extern C_CDD_EXPORT cdd_c_error_t get_token_slice(const struct TokenList *src,
                                                  size_t start, size_t end,
                                                  struct TokenList *dst);

/**
 * @brief Retrieves the index of the first token of a specific kind in a range.
 *
 * @param[in] tokens The token list.
 * @param[in] start Start index.
 * @param[in] end End index.
 * @param[in] kind Token kind to find.
 * @param[out] _out_val Found index or end if not found.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on error.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_token_in_range(const struct TokenList *tokens, size_t start, size_t end,
                    enum TokenKind kind, size_t *_out_val);

/**
 * @brief Extracts the function name from a token range preceding a body.
 *
 * @param[in] tokens The token list.
 * @param[in] start Start index.
 * @param[in] body_start Body start index.
 * @param[out] _out_val Output function name (allocated or NULL).
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
extract_func_name(const struct TokenList *tokens, size_t start,
                  size_t body_start, char **_out_val);

/**
 * @brief Joins tokens in a range into a single allocated string.
 *
 * @param[in] tokens The token list.
 * @param[in] start Start index.
 * @param[in] end End index.
 * @param[out] _out_val Output joined string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t join_tokens_str(
    const struct TokenList *tokens, size_t start, size_t end, char **_out_val);

/**
 * @brief Analyzes return type tokens of a signature.
 *
 * @param[in] tokens The token list.
 * @param[in] start Start index.
 * @param[in] body_start Body start index.
 * @param[out] is_ptr Set to 1 if return type is pointer, 0 otherwise.
 * @param[out] is_void Set to 1 if return type is void, 0 otherwise.
 * @param[out] type_str Output allocated return type string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t analyze_signature_tokens(
    const struct TokenList *tokens, size_t start, size_t body_start,
    int *is_ptr, int *is_void, char **type_str);

/**
 * @brief Checks if a file path corresponds to a C source file (.c extension).
 *
 * @param[in] path File path.
 * @param[out] out_is_src Set to 1 if .c file, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on error.
 */
extern C_CDD_EXPORT cdd_c_error_t is_c_source(const char *path,
                                              int *out_is_src);

/**
 * @brief Directory walk callback for fixing individual files.
 *
 * @param[in] path File path.
 * @param[in,out] user_data Context pointer.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t fix_file_callback(const char *path,
                                                    void *user_data);

/**
 * @brief Checks if a token matches a literal string.
 *
 * @param[in] tok Token to check.
 * @param[in] s String to match.
 * @param[out] out_eq Set to 1 if equal, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on error.
 */
extern C_CDD_EXPORT cdd_c_error_t token_eq_str(const struct Token *tok,
                                               const char *s, int *out_eq);

/**
 * @brief Concatenates two strings with an optional delimiter.
 *
 * @param[in] s1 First string (can be NULL or empty).
 * @param[in] delim Delimiter string (can be NULL).
 * @param[in] s2 Second string (can be NULL or empty).
 * @param[out] out_str Pointer to store newly allocated concatenated string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 *         or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t concat_strings(const char *s1,
                                                 const char *delim,
                                                 const char *s2,
                                                 char **out_str);

/**
 * @brief Adds a node to the dependency graph.
 *
 * @param[in,out] g Dependency graph.
 * @param[in] idx Node index.
 * @param[in] name Function name.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t graph_add_node(struct DependencyGraph *g,
                                                 size_t idx, const char *name);

/**
 * @brief Adds a caller-callee directed edge to the dependency graph.
 *
 * @param[in,out] g Dependency graph.
 * @param[in] caller_idx Index of calling function.
 * @param[in] callee_idx Index of called function.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t graph_add_edge(struct DependencyGraph *g,
                                                 size_t caller_idx,
                                                 size_t callee_idx);

/**
 * @brief Frees all dynamically allocated memory in dependency graph.
 *
 * @param[in,out] g Dependency graph.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
graph_free_contents(struct DependencyGraph *g);

/**
 * @brief Recursively propagates refactoring requirements to all callers.
 *
 * @param[in,out] g Dependency graph.
 * @param[in] idx Function node index.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
propagate_refactor_mark(struct DependencyGraph *g, size_t idx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* REFACTOR_ORCHESTRATOR_H */
