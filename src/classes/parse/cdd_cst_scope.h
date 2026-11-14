/**
 * @file cdd_cst_scope.h
 * @brief Concrete Syntax Tree (CST) scope implementation and utilities.
 */
#ifndef CDD_CST_SCOPE_H
#define CDD_CST_SCOPE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "cdd_cst_node.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Scope kinds for lexical tracking
 */
enum cdd_cst_scope_kind_t {
  CDD_CST_SCOPE_FILE,
  CDD_CST_SCOPE_FUNCTION,
  CDD_CST_SCOPE_BLOCK,
  CDD_CST_SCOPE_PROTOTYPE,
  CDD_CST_SCOPE_NAMESPACE
};

/**
 * @brief Symbol kinds
 */
enum cdd_cst_symbol_kind_t {
  CDD_CST_SYMBOL_VARIABLE,
  CDD_CST_SYMBOL_TYPEDEF,
  CDD_CST_SYMBOL_STRUCT_TAG,
  CDD_CST_SYMBOL_UNION_TAG,
  CDD_CST_SYMBOL_ENUM_TAG,
  CDD_CST_SYMBOL_ENUM_CONSTANT,
  CDD_CST_SYMBOL_FUNCTION
};

/** @brief Struct/union member layout entry typedef */
typedef struct cdd_cst_field_info_t cdd_cst_field_info_t;
/** @brief Member layout metadata for struct and union tag symbols */
struct cdd_cst_field_info_t {
  char *name;                 /**< Member field name (or empty for unnamed) */
  char *type_str;             /**< Canonical C type string of the member */
  size_t array_len;           /**< Constant array element count (0 if scalar) */
  int is_bitfield;            /**< 1 if declared as a bit-field */
  cdd_cst_field_info_t *next; /**< Next member in declaration order */
};

/** @brief Lexical scope symbol entry typedef */
typedef struct cdd_cst_symbol_t cdd_cst_symbol_t;
/** @brief Struct definition */
struct cdd_cst_symbol_t {
  char *name;                      /**< The name of the symbol */
  enum cdd_cst_symbol_kind_t kind; /**< The kind of the symbol */
  cdd_cst_node_t *decl_node;       /**< Pointer to its declaration node */
  char *type_str;                  /**< Resolved C type or return type string */
  long enum_value;                 /**< Evaluated enum constant value */
  int has_enum_value;              /**< 1 if enum_value is valid */
  cdd_cst_field_info_t *fields;    /**< Struct/union member layout list */
  cdd_cst_symbol_t *next;          /**< Next symbol in linked list */
};

/** @brief Lexical scope node typedef */
typedef struct cdd_cst_scope_t cdd_cst_scope_t;
/** @brief Struct definition */
struct cdd_cst_scope_t {
  enum cdd_cst_scope_kind_t kind; /**< Kind of scope */
  cdd_cst_scope_t *parent;        /**< Parent scope */
  cdd_cst_scope_t **children;     /**< Children scopes */
  size_t num_children;            /**< Number of children scopes */
  size_t capacity;                /**< Capacity of children scopes array */
  cdd_cst_symbol_t *symbols;      /**< Symbols defined in this scope */
  cdd_cst_node_t *ast_node;       /**< Associated CST block/function node */
};

/** @brief Lexical scope environment typedef */
typedef struct cdd_cst_scope_env_t cdd_cst_scope_env_t;
/** @brief Struct definition */
struct cdd_cst_scope_env_t {
  cdd_cst_scope_t *global_scope;  /**< The global file scope */
  cdd_cst_scope_t *current_scope; /**< The active scope during tracking */
  cdd_cst_tree_t *bound_tree;     /**< Optional CST tree bound to this env */
};

/**
 * @brief Initialize a scope environment.
 * @param[out] out_env Pointer to the env to initialize.
 * @return 0 on success.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_cst_scope_env_init(cdd_cst_scope_env_t **out_env);

/**
 * @brief Free a scope environment.
 * @param[in,out] env The environment to free.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_scope_env_free(cdd_cst_scope_env_t *env);

/**
 * @brief Push a new scope.
 * @param[in,out] env The environment.
 * @param[in] kind The kind of scope.
 * @return 0 on success.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_scope_enter(cdd_cst_scope_env_t *env,
                                               enum cdd_cst_scope_kind_t kind);

/**
 * @brief Pop the current scope.
 * @param[in,out] env The environment.
 * @return 0 on success.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_scope_leave(cdd_cst_scope_env_t *env);

/**
 * @brief Add a symbol to the current scope.
 * @param[in,out] env The environment.
 * @param[in] name The symbol name.
 * @param[in] kind The symbol kind.
 * @param[in,out] decl_node The associated declaration node.
 * @return 0 on success.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_scope_add_symbol(
    cdd_cst_scope_env_t *env, const char *name, enum cdd_cst_symbol_kind_t kind,
    cdd_cst_node_t *decl_node);

/**
 * @brief Lookup a symbol by name in the current and parent scopes.
 * @param[in,out] env The environment.
 * @param[in] name The symbol name.
 * @param[in] kind The symbol kind (or use a special "any" kind in logic).
 * @param[out] out_symbol Pointer to store the found symbol.
 * @return 0 on success (found), or ENOENT if not found.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_scope_lookup_symbol(
    cdd_cst_scope_env_t *env, const char *name, enum cdd_cst_symbol_kind_t kind,
    cdd_cst_symbol_t **out_symbol);

/**
 * @brief Check if a symbol kind is in the tag namespace (struct, union, enum).
 * @param[in] kind The symbol kind.
 * @param[out] out_is_tag Pointer to receive 1 if it is a tag, 0 otherwise.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_cst_symbol_is_tag(enum cdd_cst_symbol_kind_t kind, int *out_is_tag);

/**
 * @brief Attaches a canonical C type string to a symbol entry.
 * @param[in,out] sym Target symbol entry.
 * @param[in] type_str Null-terminated C type string.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_symbol_set_type(cdd_cst_symbol_t *sym,
                                                   const char *type_str);

/**
 * @brief Appends a struct or union member layout entry to a tag symbol.
 * @param[in,out] tag_sym Target struct or union tag symbol.
 * @param[in] field_name Member identifier (empty string if unnamed).
 * @param[in] type_str Canonical C type string of the member.
 * @param[in] array_len Constant array element count (0 if scalar/pointer).
 * @param[in] is_bitfield Non-zero if declared as a bit-field.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_symbol_add_field(cdd_cst_symbol_t *tag_sym,
                                                    const char *field_name,
                                                    const char *type_str,
                                                    size_t array_len,
                                                    int is_bitfield);

/**
 * @brief Looks up a symbol starting from an explicit lexical scope outward.
 * @param[in] scope Starting lexical scope.
 * @param[in] name Symbol identifier name.
 * @param[in] kind Symbol namespace/kind to match.
 * @param[out] out_symbol Output pointer receiving the resolved symbol.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or error.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_scope_lookup_in_scope(
    const cdd_cst_scope_t *scope, const char *name,
    enum cdd_cst_symbol_kind_t kind, cdd_cst_symbol_t **out_symbol);

/**
 * @brief Finds the innermost lexical scope associated with a CST node or its
 * enclosing block/function.
 * @param[in] env Active scope environment.
 * @param[in] node CST node to locate scope for (NULL returns global scope).
 * @param[out] out_scope Output pointer receiving the resolved scope.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_scope_find_for_node(
    const cdd_cst_scope_env_t *env, const cdd_cst_node_t *node,
    cdd_cst_scope_t **out_scope);

/**
 * @brief Updates any `scope->ast_node` pointers matching `old_node` to
 * `new_node` when a CST node is spliced/replaced.
 * @param[in,out] env Active scope environment.
 * @param[in] old_node Previous CST node pointer being replaced.
 * @param[in] new_node Replacement CST node pointer.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_scope_remap_ast_node(
    cdd_cst_scope_env_t *env, const cdd_cst_node_t *old_node,
    cdd_cst_node_t *new_node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_SCOPE_H */
