/**
 * @file safe_crt_internal.h
 * @brief Internal declarations for the Safe CRT transformer.
 *
 * @author Samuel Marks
 */

#ifndef CDD_TRANSFORMERS_SAFE_CRT_INTERNAL_H
#define CDD_TRANSFORMERS_SAFE_CRT_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/safe_crt.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_builder.h"
#include "classes/parse/cdd_cst_parser.h"
/* clang-format on */

/** @brief Forward declaration of safe_crt_arena_t */
typedef struct safe_crt_arena_t safe_crt_arena_t;

/** @brief Forward declaration of expr_t */
typedef struct expr_t expr_t;

/**
 * @brief Inferred buffer size metadata for safe CRT transformations.
 */
typedef struct inferred_size_t inferred_size_t;

/**
 * @brief Structure holding inferred buffer size metadata.
 */
struct inferred_size_t {
  int valid;                /**< 1 if size inference succeeded, 0 otherwise */
  int is_malloc;            /**< 1 if buffer was allocated via malloc */
  cdd_token_t *base_tok;    /**< Base token of the buffer identifier */
  expr_t *offset_expr;      /**< Pointer offset expression or NULL */
  expr_t *malloc_size_expr; /**< Size expression passed to malloc or NULL */
};

/** @brief Arena structure for allocating AST expressions */
struct safe_crt_arena_t {
  safe_crt_arena_t *next; /**< Pointer to next arena block */
  char data[1];           /**< Memory buffer payload */
};

/** @brief Expression AST node structure */
struct expr_t {
  int type; /**< 0: token, 1: call, 2: group, 3: fopen assign, 4: hidden */
  cdd_token_t *tok;       /**< Primary token */
  cdd_token_t *close_tok; /**< Closing token */
  expr_t *args[16];       /**< Expression arguments or children */
  size_t num_args;        /**< Number of arguments */
  expr_t *next;           /**< Next sibling expression */
};

/**
 * @brief Context flags tracking which helper buffers need emitting for MSC.
 */
typedef struct {
  int is_msc;            /**< Non-zero if compiling for MSC */
  int needs_errbuf;      /**< Non-zero if __errbuf is needed */
  int needs_wcserrbuf;   /**< Non-zero if __wcserrbuf is needed */
  int needs_strtokctx;   /**< Non-zero if __strtokctx is needed */
  int needs_wcstokctx;   /**< Non-zero if __wcstokctx is needed */
  int needs_mbstokctx;   /**< Non-zero if __mbstokctx is needed */
  int needs_ecvtbuf;     /**< Non-zero if __ecvtbuf is needed */
  int needs_fcvtbuf;     /**< Non-zero if __fcvtbuf is needed */
  int needs_getenv_ptr;  /**< Non-zero if __getenv_ptr is needed */
  int needs_wgetenv_ptr; /**< Non-zero if __wgetenv_ptr is needed */
} emit_ctx_t;

extern emit_ctx_t *g_msc_ctx;
extern cdd_cst_tree_t *safe_crt_current_tree;
#define current_tree safe_crt_current_tree

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_safe_crt_malloc_fail;
#endif

/**
 * @brief Allocates memory from the safe CRT arena.
 *
 * @param[in] len Length in bytes to allocate.
 * @param[out] out_ptr Pointer to receive the allocated memory.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t arena_alloc(size_t len, void **out_ptr);

/**
 * @brief Frees all memory allocated in the safe CRT arena.
 */
void arena_free_all(void);

/**
 * @brief Parses an expression AST from CST statement node tokens.
 *
 * @param[in] stmt CST statement node.
 * @param[in,out] idx Token index within stmt children.
 * @param[in] stop_at_comma Whether to stop parsing when encountering a comma.
 * @param[out] out_expr Pointer to receive parsed expression AST.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t parse_expr_ast(cdd_cst_node_t *stmt, size_t *idx,
                                          int stop_at_comma, expr_t **out_expr);

/**
 * @brief Finds and marks fopen assignments in the expression AST.
 *
 * @param[in] head Head of the expression AST list.
 * @param[out] out_found Pointer to receive 1 if fopen assignment found, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t find_and_mark_fopen(expr_t *head, int *out_found);

/**
 * @brief Checks for unsupported bare calls.
 *
 * @param[in] head Head of the expression AST list.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t check_unsupported_calls(expr_t *head);

/**
 * @brief Infers buffer size metadata for a given expression node.
 *
 * @param[in] node Target expression node.
 * @return Inferred size metadata structure.
 */
inferred_size_t infer_buffer_size(expr_t *node);

/**
 * @brief Checks if any call in the expression tree requires safe CRT
 * transformation.
 *
 * @param[in] head Head of the expression AST list.
 * @return CDD_C_SUCCESS if none, CDD_C_ERROR_PARSE if uninferrable size,
 * CDD_C_ERROR_UNKNOWN if transformation needed.
 */
C_CDD_EXPORT cdd_c_error_t check_needs_transform(expr_t *head);

/**
 * @brief Checks if an expression represents NULL or numeric 0.
 *
 * @param[in] node Target expression node.
 * @return CDD_C_ERROR_UNKNOWN if NULL or 0, CDD_C_SUCCESS otherwise.
 */
C_CDD_EXPORT cdd_c_error_t expr_is_null_or_zero(expr_t *node);

/**
 * @brief Safely pools a string in the CST tree's string pool.
 *
 * @param[in,out] tree Target CST tree.
 * @param[in] str String to duplicate and pool.
 * @return Pointer to pooled string, or NULL on failure.
 */
C_CDD_EXPORT const char *safe_crt_pool_string_safe(cdd_cst_tree_t *tree,
                                                   const char *str);

/**
 * @brief Clones a linked list of trivia nodes.
 *
 * @param[in] head Head of the trivia list.
 * @param[out] out_trivia Pointer to receive cloned trivia list head.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t clone_trivia(cdd_trivia_t *head,
                                        cdd_trivia_t **out_trivia);

/**
 * @brief Clones a CST token including its trivia.
 *
 * @param[in,out] tree Target CST tree.
 * @param[in] tok Token to clone.
 * @param[out] out_token Pointer to receive cloned token.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t clone_token(cdd_cst_tree_t *tree, cdd_token_t *tok,
                                       cdd_token_t **out_token);

/**
 * @brief Emits an expression AST with stripped leading trivia.
 *
 * @param[in] node Expression AST node to emit.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_ast_bld_strip(expr_t *node,
                                              cdd_cst_builder_t *bld,
                                              int is_msc);

/**
 * @brief Emits an expression AST stripping leading ampersand if present.
 *
 * @param[in] node Expression AST node to emit.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_ast_bld_strip_ampersand(expr_t *node,
                                                        cdd_cst_builder_t *bld,
                                                        int is_msc);

/**
 * @brief Emits the inferred buffer size expression into builder.
 *
 * @param[in,out] bld CST builder.
 * @param[in] dest Destination buffer expression.
 */
C_CDD_EXPORT cdd_c_error_t emit_inferred_size(cdd_cst_builder_t *bld,
                                              expr_t *dest);

/**
 * @brief Emits transformed AST tokens into the CST builder.
 *
 * @param[in] node Expression node.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @return Number of safe CRT changes emitted.
 */
cdd_c_error_t emit_ast_bld(expr_t *node, cdd_cst_builder_t *bld, int is_msc,
                           int *out_changes);

/**
 * @brief Extracts the indentation string from a token's leading trivia.
 *
 * @param[in] tok Target token.
 * @param[out] out_indent Buffer of at least 64 bytes to receive indentation
 * string.
 */
C_CDD_EXPORT void get_indent_string(cdd_token_t *tok, char *out_indent);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_TRANSFORMERS_SAFE_CRT_INTERNAL_H */
