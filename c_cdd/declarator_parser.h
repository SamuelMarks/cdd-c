/**
 * @file declarator_parser.h
 * @brief "Right-Left" (Spiral) parser for C declarations.
 *
 * Implements logic to parse complex C declarators into a structured type chain.
 * Correctly handles operator precedence (arrays/functions binding tighter than
 * pointers) and grouping parentheses.
 *
 * Usage:
 * - `int *(*f)(int)` -> Identifier: "f", Type: Pointer -> Function -> Pointer
 * -> Base(int)
 * - `struct S a[10]` -> Identifier: "a", Type: Array(10) -> Base(struct S)
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_DECLARATOR_PARSER_H
#define C_CDD_DECLARATOR_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "tokenizer.h"
#include <stddef.h>

/**
 * @brief Classification of a type node in the chain.
 */
enum DeclTypeKind {
  DECL_BASE,  /**< The fundamental type (int, struct S, typeof(x)) */
  DECL_PTR,   /**< Pointer (*) */
  DECL_ARRAY, /**< Array ([]) */
  DECL_FUNC   /**< Function (()) */
};

/**
 * @brief A node in the type chain description.
 *
 * The chain is ordered from outer-most wrapper to inner-most type.
 * e.g. `int *a[]` (Array of Pointers to Int) ->
 *      [ARRAY] -> [PTR] -> [BASE(int)]
 */
struct DeclType {
  enum DeclTypeKind kind;
  struct DeclType *inner; /**< The type being modified (next in logic) */

  union {
    struct {
      char *name; /**< Full text of base type (e.g. "const unsigned int") */
    } base;
    struct {
      char *qualifiers; /**< Pointer qualifiers (e.g. "const", "restrict") */
    } ptr;
    struct {
      char *size_expr; /**< Dimension expression (e.g. "10", "MAX"), or NULL if
                          empty "[]" */
    } array;
    struct {
      char *args_str; /**< Raw text of argument list (e.g. "int a, float b") */
    } func;
  } data;
};

/**
 * @brief Result of parsing a full declaration.
 */
struct DeclInfo {
  char *identifier;      /**< Name of the variable/function declared */
  struct DeclType *type; /**< Head of the type chain */
};

/**
 * @brief Initialize a DeclInfo structure.
 * @param[out] info The structure to zero.
 */
extern C_CDD_EXPORT void decl_info_init(struct DeclInfo *info);

/**
 * @brief Free resources in a DeclInfo structure.
 * Recursively frees the type chain.
 * @param[in] info The structure to free.
 */
extern C_CDD_EXPORT void decl_info_free(struct DeclInfo *info);

/**
 * @brief Parse a declaration token range.
 *
 * Deconstructs a C declaration using the Spiral Rule.
 * 1. Locates the identifier (pivot).
 * 2. Unwinds operators right (arrays/functions) and left (pointers) respecting
 * grouping.
 * 3. Captures remaining left-side tokens as the base specifier.
 *
 * @param[in] tokens The full token stream.
 * @param[in] start_idx Start index of the declaration statement.
 * @param[in] end_idx End index (exclusive), typically at semicolon or comma.
 * @param[out] out_info Pointer to destination structure.
 * @return 0 on success, EINVAL on syntax error, ENOMEM on alloc failure.
 */
extern C_CDD_EXPORT int parse_declaration(const struct TokenList *tokens,
                                          size_t start_idx, size_t end_idx,
                                          struct DeclInfo *out_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_DECLARATOR_PARSER_H */
