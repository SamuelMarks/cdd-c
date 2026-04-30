#ifndef CDD_CST_NODE_H
#define CDD_CST_NODE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_token.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief CST Node Kinds
 */
enum cdd_cst_node_kind_t {
  CDD_CST_TRANSLATION_UNIT,
  CDD_CST_DECLARATION,
  CDD_CST_FUNCTION_DEFINITION,
  CDD_CST_STATEMENT,
  CDD_CST_EXPRESSION,
  CDD_CST_PREPROC_CONDITIONAL, /* #ifdef ... #else ... #endif */
  CDD_CST_PREPROC_DIRECTIVE,   /* #include, #define, etc */
  CDD_CST_TYPE_SPECIFIER,
  CDD_CST_IDENTIFIER,
  CDD_CST_LITERAL,
  CDD_CST_BINARY_EXPR,
  CDD_CST_UNARY_EXPR,
  CDD_CST_CALL_EXPR,
  CDD_CST_BLOCK,
  CDD_CST_ASM_STATEMENT,
  CDD_CST_UNKNOWN /* fallback */
};

typedef struct cdd_cst_node_t cdd_cst_node_t;

/**
 * @brief Represents a child of a CST node (either a Token or another Node).
 */
typedef struct cdd_cst_child_t cdd_cst_child_t;
enum cdd_cst_child_kind_t { CDD_CST_CHILD_TOKEN, CDD_CST_CHILD_NODE };

/** @brief Struct definition */
struct cdd_cst_child_t {
  /** @brief field */
  /** @brief field */
  enum cdd_cst_child_kind_t kind;
  /** @brief Struct definition */
  union {
    /** @brief field */
    /** @brief field */
    cdd_token_t *token;
    /** @brief field */
    /** @brief field */
    cdd_cst_node_t *node;
  } val;
};

/**
 * @brief A Concrete Syntax Tree Node.
 */
struct cdd_cst_node_t {
  /** @brief field */
  /** @brief field */
  enum cdd_cst_node_kind_t kind;
  /** @brief field */
  /** @brief field */
  cdd_cst_child_t *children;
  /** @brief field */
  /** @brief field */
  size_t num_children;
  /** @brief field */
  /** @brief field */
  size_t capacity;
  /** @brief field */
  /** @brief field */
  cdd_cst_node_t *parent;
};

/**
 * @brief Represents a fully parsed tree, owning the underlying tokens.
 */
typedef struct cdd_cst_tree_t cdd_cst_tree_t;
/** @brief Struct definition */
struct cdd_cst_tree_t {
  /** @brief field */
  /** @brief field */
  cdd_cst_node_t *root;
  cdd_token_list_t *base_tokens;    /**< The original tokens from lexer */
  cdd_token_t **synthesized_tokens; /**< Tokens synthesized during mutation */
  /** @brief field */
  /** @brief field */
  size_t num_synthesized;
  /** @brief field */
  /** @brief field */
  size_t synthesized_capacity;
  /** @brief string_pool field */
  char **string_pool;
  /** @brief string_capacity field */
  size_t num_strings;
  /** @brief string_capacity field */
  size_t string_capacity;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_NODE_H */
