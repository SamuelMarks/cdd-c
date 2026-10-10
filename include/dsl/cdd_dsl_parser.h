#ifndef CDD_DSL_PARSER_H
#define CDD_DSL_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "dsl/cdd_dsl_tokenizer.h"
#include "ffi/cdd_ffi_ir.h"
/* clang-format on */

/**
 * @brief Parses a tokenized FFI IR DSL script into an IR AST.
 *
 * @param tokens The parsed tokens.
 * @param out_ir The FFI IR object to populate.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_dsl_parse(const cdd_dsl_token_list_t *tokens,
                                         cdd_ffi_ir_t **out_ir);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_DSL_PARSER_H */
