#ifndef CDD_MACRO_EVALUATOR_H
#define CDD_MACRO_EVALUATOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "preprocessor.h"
#include <stddef.h>
/* clang-format on */

/**
 * @enum cdd_macro_eval_type_t
 * @brief Represents the inferred type of a evaluated macro expression.
 */
typedef enum cdd_macro_eval_type_t {
  MACRO_EVAL_TYPE_UNKNOWN,
  MACRO_EVAL_TYPE_INT,
  MACRO_EVAL_TYPE_FLOAT,
  MACRO_EVAL_TYPE_STRING
} cdd_macro_eval_type_t;

/**
 * @struct cdd_macro_eval_result_t
 * @brief Result of a macro expression evaluation.
 */
typedef struct cdd_macro_eval_result_t {
  /** @brief Type of the result */
  cdd_macro_eval_type_t type;
  /** @brief Integer value */
  long long int_val;
  /** @brief Float value */
  double float_val;
  /** @brief String value (dynamically allocated) */
  char *str_val;
} cdd_macro_eval_result_t;

/**
 * @brief Evaluates a C preprocessor constant expression.
 * @param ctx The preprocessor context (for looking up other macros).
 * @param expression The right-hand side string of the macro to evaluate.
 * @param out_result Pointer to store the evaluated result.
 * @return 0 on success, non-zero on error (e.g. invalid expression or unknown
 * identifier).
 */
C_CDD_EXPORT enum cdd_c_error
cdd_macro_evaluate(struct PreprocessorContext *ctx, const char *expression,
                   cdd_macro_eval_result_t *out_result);

/**
 * @brief Frees any allocated memory in the evaluation result.
 * @param result The result to free.
 */
C_CDD_EXPORT void cdd_macro_eval_result_free(cdd_macro_eval_result_t *result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_MACRO_EVALUATOR_H */
