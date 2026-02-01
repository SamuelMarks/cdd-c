/**
 * @file initializer_parser.h
 * @brief Parser for C Initializer lists (Brace-enclosed lists).
 *
 * Handles parsing of standard and designated initializers used in variable
 * declarations and compound literals.
 * Supports:
 * - Positional initialization: `{ 1, 2, 3 }`
 * - Designated initialization: `{ .x = 1, [0] = 5 }`
 * - Nested structures: `{ .pt = { 1, 2 } }`
 * - Mixed formats.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_INITIALIZER_PARSER_H
#define C_CDD_INITIALIZER_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "tokenizer.h"
#include <stddef.h>

/**
 * @brief Discriminator for initializer value type.
 */
enum InitKind {
  INIT_KIND_SCALAR,   /**< A simple expression string (e.g. "5", "x + 1") */
  INIT_KIND_COMPOUND, /**< A nested brace-enclosed list (e.g. "{ .x=1 }") */
  INIT_KIND_NONE      /**< Empty/Error state */
};

/* Forward declaration for recursion */
struct InitList;

/**
 * @brief Represents a single value in an initializer (Scalar or Compound).
 */
struct InitValue {
  enum InitKind kind; /**< Type of value */
  union {
    char *scalar;              /**< Text of expression if SCALAR */
    struct InitList *compound; /**< Nested list if COMPOUND */
  } data;
};

/**
 * @brief Represents one item in an initializer list.
 * E.g., `.x = 5` or just `5`.
 */
struct InitItem {
  char *designator; /**< Designator string (e.g. ".x", "[0]", ".a[1]"), or NULL
                       if positional */
  struct InitValue *value; /**< The value assigned */
};

/**
 * @brief A container for a sequence of initializer items.
 * Represents the content between braces `{ ... }`.
 */
struct InitList {
  struct InitItem *items; /**< Array of items */
  size_t count;           /**< Number of items */
  size_t capacity;        /**< Internal capacity */
};

/**
 * @brief Initialize an InitList structure.
 *
 * @param[out] list Pointer to the list to init.
 * @return 0 on success, EINVAL if list is NULL.
 */
extern C_CDD_EXPORT int init_list_init(struct InitList *list);

/**
 * @brief Free an InitList and all its contents (recursive).
 *
 * @param[in] list Pointer to the list to free.
 */
extern C_CDD_EXPORT void init_list_free(struct InitList *list);

/**
 * @brief Parse a token range representing an initializer.
 *
 * The range should typically start with `TOKEN_LBRACE` ('{').
 * The function parses until the matching `TOKEN_RBRACE` ('}').
 *
 * @param[in] tokens The full token stream.
 * @param[in] start_idx Index of the opening brace '{'.
 * @param[in] end_idx Index limit (exclusive) for safety.
 * @param[out] out Pointer to an empty InitList structure to populate.
 * @param[out] consumed Optional pointer to receive number of tokens consumed
 * (including empty braces).
 * @return 0 on success, EINVAL on syntax error, ENOMEM on alloc failure.
 */
extern C_CDD_EXPORT int parse_initializer(const struct TokenList *tokens,
                                          size_t start_idx, size_t end_idx,
                                          struct InitList *out,
                                          size_t *consumed);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_INITIALIZER_PARSER_H */
