/**
 * @file preprocessor.h
 * @brief Simplistic C Preprocessor logic for include resolution and macro
 * indexing.
 *
 * Provides functionalities to:
 * - Manage include search paths.
 * - Scan source files for `\#include` directives.
 * - Scan and index `#define` macros, including function-like and variadic
 * macros.
 * - Reassemble fragmented path tokens (e.g. < sys / stat . h >).
 * - Resolve relative and system paths against the search context.
 * - Evaluate preprocessor conditional expressions (\#if, defined, etc.).
 * - Support C23 introspection macros: `__has_include`, `__has_embed`,
 * `__has_c_attribute`.
 * - Parse `\#embed` parameters (`limit`, `prefix`, `suffix`, `if_empty`).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_PREPROCESSOR_H
#define C_CDD_PREPROCESSOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/**
 * @brief Enumeration of supported directives scanned by path logic.
 */
enum PpDirectiveKind {
  PP_DIR_INCLUDE, /**< \#include ... */
  PP_DIR_EMBED    /**< \#embed ... */
};

/**
 * @brief Preprocessor conditional evaluation state.
 */
enum CondState {
  COND_ACTIVE,    /**< Active branch, statements evaluated */
  COND_SKIPPING,  /**< Skipping branch until elif/else/endif */
  COND_SATISFIED, /**< Prior condition satisfied, skip remaining elif/else */
  COND_ELSE_SEEN  /**< Else block seen, skip further directives */
};

/**
 * @brief Stack for tracking nested preprocessor conditionals.
 */
struct ConditionalStack {
  enum CondState states[32]; /**< Stack states */
  int top;                   /**< Index of topmost state (-1 when empty) */
};

/**
 * @brief Container for C23 \#embed parameters.
 */
struct EmbedParams {
  long limit;     /**< The value of 'limit(...)', -1 if unspecified */
  char *prefix;   /**< The raw text content of 'prefix(...)', or NULL */
  char *suffix;   /**< The raw text content of 'suffix(...)', or NULL */
  char *if_empty; /**< The raw text content of 'if_empty(...)', or NULL */
};

/**
 * @brief Information passed to the visitor callback.
 */
struct IncludeInfo {
  enum PpDirectiveKind kind; /**< Type of directive encountered */
  const char *resolved_path; /**< The resolved absolute/relative path on disk */
  const char *raw_path; /**< The raw path string as it appeared in source */
  int is_system;        /**< 1 if angle brackets <>, 0 if quoted "" */
  int is_next;          /**< 1 if it is `#include_next`, 0 otherwise */
  struct EmbedParams params; /**< Embed parameters (zeroed for \#include) */
};

/**
 * @brief Type definition for the include visitor callback.
 *
 * Invoked for each resolved include found in a scanned file.
 *
 * @param[in] info Details about the directive and resolved file.
 * @param[in] user_data Opaque pointer passed by caller.
 * @return 0 to continue scanning, non-zero to stop.
 */
typedef cdd_c_error_t (*pp_visitor_cb)(const struct IncludeInfo *info,
                                       void *user_data);

/**
 * @brief Represents a single definition found in source code.
 */
struct MacroDef {
  char *name;           /**< Macro identifier */
  int is_function_like; /**< True if defined as MACRO(...) */
  int is_variadic;      /**< True if arguments end in ... */
  char **args;          /**< Array of argument names (excluding .../VA_ARGS) */
  size_t arg_count;     /**< Count of explicit arguments */
  char *value;          /**< Raw text value of the macro (for object-like) */
};

/**
 * @brief Context holding configuration for the preprocessor.
 * Maintains a list of search paths (e.g., -I folders) and found definitions.
 */
struct PreprocessorContext {
  char **search_paths; /**< Dynamic array of search directory paths */
  size_t size;         /**< Number of search paths */
  size_t capacity;     /**< Capacity of the search path array */

  struct MacroDef *macros; /**< Dynamic array of discovered macros */
  size_t macro_count;      /**< Number of macros */
  size_t macro_capacity;   /**< Capacity of macro array */

  /* Introspection context (current file path for relative lookups) */
  const char *current_file_dir; /**< Directory of the file being processed, used
                                   for relative include resolution. */
};

/**
 * @brief Initialize a preprocessor context.
 *
 * @param[out] ctx Pointer to the context structure.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
pp_context_init(struct PreprocessorContext *ctx);

/**
 * @brief Free resources associated with the context.
 *
 * @param[in] ctx Pointer to the context.
 */
extern C_CDD_EXPORT void pp_context_free(struct PreprocessorContext *ctx);

/**
 * @brief Add a search path to the context.
 *
 * @param[in,out] ctx The context.
 * @param[in] path Directory path to add (copied internally).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 *         or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
pp_add_search_path(struct PreprocessorContext *ctx, const char *path);

/**
 * @brief Add a macro definition manually to the context.
 * Useful for seeding configuration macros (e.g., -DDEBUG).
 *
 * @param[in,out] ctx The context.
 * @param[in] name Macro name.
 * @param[in] value Macro value text (can be NULL for empty define).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on failure,
 *         or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_add_macro(struct PreprocessorContext *ctx,
                                               const char *name,
                                               const char *value);

/**
 * @brief Scan a file for \#include directives and resolve them.
 *
 * Reads the file at `filename`, tokenizes it, identifies `\#include` lines,
 * reconstructs the path arguments, resolves them using the context,
 * and triggers `cb` for valid files.
 *
 * Respects conditional compilation directives (\#if, \#ifdef, \#else, \#endif).
 * Only includes within active blocks are reported.
 *
 * @param[in] filename Path to the source file to scan.
 * @param[in] ctx Preprocessor context containing search paths.
 * @param[in] cb Callback function for found includes.
 * @param[in] user_data Opaque data passed to callback.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
pp_scan_includes(const char *filename, struct PreprocessorContext *ctx,
                 pp_visitor_cb cb, void *user_data);

/**
 * @brief Scan a file token stream for macro definitions and populate the
 * context.
 *
 * Parses `#define` lines to extract macro signatures.
 * correctly identifies `NAME`, `NAME(a, b)`, and `NAME(a, ...)` forms.
 *
 * @param[in,out] ctx The context to populate with found macros.
 * @param[in] filename Path to the file to parse.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
pp_scan_defines(struct PreprocessorContext *ctx, const char *filename);

/**
 * @brief Evaluate a preprocessor constant expression.
 *
 * Implements a recursive descent parser for integer constant expressions
 * as defined in C standard logic (6.10.1).
 *
 * @param[in] tokens The token list containing the expression.
 * @param[in] start_idx Index of the first token of the expression.
 * @param[in] end_idx Index of the first token AFTER the expression (exclusive).
 * @param[in] ctx Context for looking up identifiers and resolving paths.
 * @param[out] result Result of the evaluation (1 or 0 usually).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on syntax
 * error.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_eval_expression(
    const struct TokenList *tokens, size_t start_idx, size_t end_idx,
    const struct PreprocessorContext *ctx, long *result);

/**
 * @brief Release memory within EmbedParams structure.
 *
 * @param[in] params Pointer to struct to clean.
 */
extern C_CDD_EXPORT void pp_embed_params_free(struct EmbedParams *params);

/**
 * @brief Joins a directory and file name into a path string.
 *
 * @param[in] dir Directory path.
 * @param[in] file File name or relative path.
 * @param[out] out_val Pointer to store newly allocated joined path.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 *         or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_join_path(const char *dir,
                                               const char *file,
                                               char **out_val);

/**
 * @brief Checks if a file exists on disk.
 *
 * @param[in] path File path to check.
 * @param[out] out_exists Pointer to store 1 if file exists, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_file_exists(const char *path,
                                                 int *out_exists);

/**
 * @brief Converts a token slice to a null-terminated string.
 *
 * @param[in] t Token to convert.
 * @param[out] out_val Pointer to store newly allocated string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 *         or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_token_to_string(const struct Token *t,
                                                     char **out_val);

/**
 * @brief Resolves an include path against search context and current file
 * directory.
 *
 * @param[in] ctx Preprocessor context.
 * @param[in] current_dir Current file directory (or NULL).
 * @param[in] include_path Include path to resolve.
 * @param[in] is_system 1 if system angle bracket include, 0 if quoted.
 * @param[out] out_val Pointer to store newly allocated resolved path or NULL.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 *         or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
pp_resolve_path(const struct PreprocessorContext *ctx, const char *current_dir,
                const char *include_path, int is_system, char **out_val);

/**
 * @brief Reconstructs a path string from a sequence of tokens.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start index in token list.
 * @param[in] end End index in token list (exclusive).
 * @param[out] out_val Pointer to store newly allocated reconstructed path.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 *         or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_reconstruct_path(
    const struct TokenList *tokens, size_t start, size_t end, char **out_val);

/**
 * @brief Parses parameters for a C23 \#embed directive.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] end End index.
 * @param[in,out] ctx Preprocessor context.
 * @param[out] out_params Pointer to EmbedParams structure to populate.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_parse_embed_params(
    const struct TokenList *tokens, size_t start, size_t end,
    struct PreprocessorContext *ctx, struct EmbedParams *out_params);

/**
 * @brief Checks if a token matches any defined macro in the context.
 *
 * @param[in] ctx Preprocessor context.
 * @param[in] tok Token to check.
 * @param[out] out_val Pointer to store 1 if defined, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
pp_is_defined_macro(const struct PreprocessorContext *ctx,
                    const struct Token *tok, int *out_val);

/**
 * @brief Pushes a conditional state onto the stack.
 *
 * @param[in,out] st Stack.
 * @param[in] s State to push.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on error.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_stack_push(struct ConditionalStack *st,
                                                enum CondState s);

/**
 * @brief Pops the topmost conditional state from the stack.
 *
 * @param[in,out] st Stack.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on error.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_stack_pop(struct ConditionalStack *st);

/**
 * @brief Peeks at the topmost conditional state on the stack.
 *
 * @param[in] st Stack.
 * @param[out] out_val Output conditional state (COND_ACTIVE if empty).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on error.
 */
extern C_CDD_EXPORT cdd_c_error_t
pp_stack_peek(const struct ConditionalStack *st, enum CondState *out_val);

/**
 * @brief Checks if the current preprocessor conditional nesting is actively
 * enabled.
 *
 * @param[in] st Stack.
 * @param[out] out_val 1 if code is actively enabled, 0 if skipped.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on error.
 */
extern C_CDD_EXPORT cdd_c_error_t
pp_is_enabled(const struct ConditionalStack *st, int *out_val);

/**
 * @brief State container for parsing preprocessor expressions.
 */
struct ExprState {
  const struct TokenList *tokens;        /**< Token sequence */
  size_t pos;                            /**< Current position */
  size_t end;                            /**< End position (exclusive) */
  const struct PreprocessorContext *ctx; /**< Context for macro/file lookups */
  int error;                             /**< Error flag */
};

/**
 * @brief Peeks at the next token in an expression state.
 *
 * @param[in,out] s Expression state.
 * @param[out] out_val Pointer to store token kind.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on error.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_preprocessor_peek(struct ExprState *s,
                                                       enum TokenKind *out_val);

/**
 * @brief Free heap allocations inside a MacroDef structure.
 *
 * @param[in,out] def Pointer to macro definition to clean.
 */
extern C_CDD_EXPORT void pp_free_macro_def(struct MacroDef *def);

/**
 * @brief Adds a macro definition internally to the preprocessor context.
 *
 * @param[in,out] ctx Preprocessor context.
 * @param[in] def Macro definition to store.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 *         or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_add_macro_internal(
    struct PreprocessorContext *ctx, const struct MacroDef *def);

/** @brief Alias for pp_free_macro_def. */
#define free_macro_def pp_free_macro_def
/** @brief Alias for pp_add_macro_internal. */
#define add_macro_internal pp_add_macro_internal
/** @brief Alias for pp_join_path. */
#define join_path pp_join_path
/** @brief Alias for pp_file_exists. */
#define file_exists pp_file_exists
/** @brief Alias for pp_token_to_string. */
#define token_to_string pp_token_to_string
/** @brief Alias for pp_resolve_path. */
#define resolve_path pp_resolve_path
/** @brief Alias for pp_reconstruct_path. */
#define reconstruct_path pp_reconstruct_path
/** @brief Alias for pp_parse_embed_params. */
#define parse_embed_params pp_parse_embed_params
/** @brief Alias for pp_is_defined_macro. */
#define is_defined_macro pp_is_defined_macro
/** @brief Alias for pp_stack_push. */
#define stack_push pp_stack_push
/** @brief Alias for pp_stack_pop. */
#define stack_pop pp_stack_pop
/** @brief Alias for pp_stack_peek. */
#define stack_preprocessor_peek pp_stack_peek
/** @brief Alias for pp_preprocessor_peek. */
#define preprocessor_peek pp_preprocessor_peek

#ifdef CDD_BUILD_TESTS
/** @brief Test hook to fail pp_file_exists. */
extern C_CDD_EXPORT int g_cdd_pp_file_exists_fail;
/** @brief Test hook to fail match. */
extern C_CDD_EXPORT int g_cdd_pp_match_fail;
/** @brief Test hook to fail skip_ws. */
extern C_CDD_EXPORT int g_cdd_pp_skip_ws_fail;
/** @brief Test hook to fail pp_resolve_path. */
extern C_CDD_EXPORT int g_cdd_pp_resolve_path_fail;
/** @brief Test hook to fail pp_eval_expression. */
extern C_CDD_EXPORT int g_cdd_pp_eval_expr_fail;
/** @brief Test hook to fail pp_token_to_string. */
extern C_CDD_EXPORT int g_cdd_pp_token_to_string_fail;
/** @brief Test hook to fail pp_is_defined_macro. */
extern C_CDD_EXPORT int g_cdd_pp_is_defined_macro_fail;
/** @brief Test hook to fail pp_preprocessor_peek. */
extern C_CDD_EXPORT int g_cdd_pp_peek_fail;
/** @brief Test hook to fail parse_primary. */
extern C_CDD_EXPORT int g_cdd_pp_primary_fail;
/** @brief Test hook to fail parse_unary. */
extern C_CDD_EXPORT int g_cdd_pp_unary_fail;
/** @brief Test hook to fail parse_multiplicative. */
extern C_CDD_EXPORT int g_cdd_pp_multiplicative_fail;
/** @brief Test hook to fail parse_additive. */
extern C_CDD_EXPORT int g_cdd_pp_additive_fail;
/** @brief Test hook to fail parse_shift. */
extern C_CDD_EXPORT int g_cdd_pp_shift_fail;
/** @brief Test hook to fail parse_relational. */
extern C_CDD_EXPORT int g_cdd_pp_relational_fail;
/** @brief Test hook to fail parse_equality. */
extern C_CDD_EXPORT int g_cdd_pp_equality_fail;
/** @brief Test hook to fail parse_logic_and. */
extern C_CDD_EXPORT int g_cdd_pp_logic_and_fail;

/**
 * @brief Skips whitespace in expression state (test export).
 * @param[in,out] s Expression state.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on error.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_expr_skip_ws(struct ExprState *s);

/**
 * @brief Matches a token kind in expression state (test export).
 * @param[in,out] s Expression state.
 * @param[in] kind Token kind to match.
 * @param[out] out_val Pointer to store 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on error.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_expr_match(struct ExprState *s,
                                                enum TokenKind kind,
                                                int *out_val);

/**
 * @brief Handles __has_include and __has_embed (test export).
 * @param[in,out] s Expression state.
 * @param[out] out_val Result pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
pp_handle_has_include_embed(struct ExprState *s, long *out_val);

/**
 * @brief Handles __has_c_attribute (test export).
 * @param[in,out] s Expression state.
 * @param[out] out_val Result pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_handle_has_c_attribute(struct ExprState *s,
                                                            long *out_val);

/**
 * @brief Parses primary expression (test export).
 * @param[in,out] s Expression state.
 * @param[out] out_val Result pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_parse_primary(struct ExprState *s,
                                                   long *out_val);

/**
 * @brief Parses unary expression (test export).
 * @param[in,out] s Expression state.
 * @param[out] out_val Result pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_parse_unary(struct ExprState *s,
                                                 long *out_val);

/**
 * @brief Parses multiplicative expression (test export).
 * @param[in,out] s Expression state.
 * @param[out] out_val Result pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_parse_multiplicative(struct ExprState *s,
                                                          long *out_val);

/**
 * @brief Parses additive expression (test export).
 * @param[in,out] s Expression state.
 * @param[out] out_val Result pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_parse_additive(struct ExprState *s,
                                                    long *out_val);

/**
 * @brief Parses shift expression (test export).
 * @param[in,out] s Expression state.
 * @param[out] out_val Result pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_parse_shift(struct ExprState *s,
                                                 long *out_val);

/**
 * @brief Parses relational expression (test export).
 * @param[in,out] s Expression state.
 * @param[out] out_val Result pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_parse_relational(struct ExprState *s,
                                                      long *out_val);

/**
 * @brief Parses equality expression (test export).
 * @param[in,out] s Expression state.
 * @param[out] out_val Result pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_parse_equality(struct ExprState *s,
                                                    long *out_val);

/**
 * @brief Parses logical and expression (test export).
 * @param[in,out] s Expression state.
 * @param[out] out_val Result pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_parse_logic_and(struct ExprState *s,
                                                     long *out_val);

/**
 * @brief Parses logical or expression (test export).
 * @param[in,out] s Expression state.
 * @param[out] out_val Result pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_parse_logic_or(struct ExprState *s,
                                                    long *out_val);

/**
 * @brief Parses full expression (test export).
 * @param[in,out] s Expression state.
 * @param[out] out_val Result pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t pp_parse_expr(struct ExprState *s,
                                                long *out_val);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_PREPROCESSOR_H */
