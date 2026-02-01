/**
 * @file preprocessor.h
 * @brief Simplistic C Preprocessor logic for include resolution and macro
 * indexing.
 *
 * Provides functionalities to:
 * - Manage include search paths.
 * - Scan source files for `#include` directives.
 * - Scan and index `#define` macros, including function-like and variadic
 * macros.
 * - Reassemble fragmented path tokens (e.g. < sys / stat . h >).
 * - Resolve relative and system paths against the search context.
 * - Evaluate preprocessor conditional expressions (#if, defined, etc.).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_PREPROCESSOR_H
#define C_CDD_PREPROCESSOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>

#include "c_cdd_export.h"
#include "tokenizer.h"

/**
 * @brief Type definition for the include visitor callback.
 *
 * Invoked for each resolved include found in a scanned file.
 *
 * @param[in] resolved_path Absolute or relative path to the included file.
 * @param[in] user_data Opaque pointer passed by caller.
 * @return 0 to continue scanning, non-zero to stop.
 */
typedef int (*pp_visitor_cb)(const char *resolved_path, void *user_data);

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
};

/**
 * @brief Initialize a preprocessor context.
 *
 * @param[out] ctx Pointer to the context structure.
 * @return 0 on success, EINVAL or ENOMEM on failure.
 */
extern C_CDD_EXPORT int pp_context_init(struct PreprocessorContext *ctx);

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
 * @return 0 on success, ENOMEM on allocation failure.
 */
extern C_CDD_EXPORT int pp_add_search_path(struct PreprocessorContext *ctx,
                                           const char *path);

/**
 * @brief Add a macro definition manually to the context.
 * Useful for seeding configuration macros (e.g., -DDEBUG).
 *
 * @param[in,out] ctx The context.
 * @param[in] name Macro name.
 * @param[in] value Macro value text (can be NULL for empty define).
 * @return 0 on success, ENOMEM on failure.
 */
extern C_CDD_EXPORT int pp_add_macro(struct PreprocessorContext *ctx,
                                     const char *name, const char *value);

/**
 * @brief Scan a file for #include directives and resolve them.
 *
 * Reads the file at `filename`, tokenizes it, identifies `#include` lines,
 * reconstructs the path arguments, resolves them using the context,
 * and triggers `cb` for valid files.
 *
 * Respects conditional compilation directives (#if, #ifdef, #else, #endif).
 * Only includes within active blocks are reported.
 *
 * @param[in] filename Path to the source file to scan.
 * @param[in] ctx Preprocessor context containing search paths.
 * @param[in] cb Callback function for found includes.
 * @param[in] user_data Opaque data passed to callback.
 * @return 0 on success, error code on failure (IO, memory, or parsing).
 */
extern C_CDD_EXPORT int pp_scan_includes(const char *filename,
                                         const struct PreprocessorContext *ctx,
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
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int pp_scan_defines(struct PreprocessorContext *ctx,
                                        const char *filename);

/**
 * @brief Evaluate a preprocessor constant expression.
 *
 * Implements a recursive descent parser for integer constant expressions
 * as defined in C standard logic (6.10.1).
 * Supports:
 * - Arithmetic: +, -, *, /, %
 * - Logical: ||, &&, !
 * - Bitwise: |, &, ^, ~, <<, >>
 * - Comparison: ==, !=, <, >, <=, >=
 * - 'defined' operator
 * - Identifiers (resolves to 0 if not defined)
 *
 * @param[in] tokens The token list containing the expression.
 * @param[in] start_idx Index of the first token of the expression.
 * @param[in] end_idx Index of the first token AFTER the expression (exclusive).
 * @param[in] ctx Context for looking up 'defined' macros.
 * @param[out] result Result of the evaluation (1 or 0 usually).
 * @return 0 on success, EINVAL on syntax error.
 */
extern C_CDD_EXPORT int
pp_eval_expression(const struct TokenList *tokens, size_t start_idx,
                   size_t end_idx, const struct PreprocessorContext *ctx,
                   long *result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_PREPROCESSOR_H */
