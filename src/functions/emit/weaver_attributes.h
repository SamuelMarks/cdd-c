/**
 * @file weaver_attributes.h
 * @brief Core weaver attributes definitions and implementation.
 */
#ifndef C_CDD_WEAVER_ATTRIBUTES_H
#define C_CDD_WEAVER_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "functions/emit/patcher.h"
#include "functions/parse/cst.h"
#include "functions/parse/tokenizer.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Translates GNU C `__attribute__` annotations into cross-platform
 * MSVC/GCC preprocessor conditional patches.
 *
 * @param[in,out] patches Patch list to populate with replacement patches.
 * @param[in] tokens Token stream corresponding to the parsed source file.
 * @param[in] cst Concrete Syntax Tree node list to inspect for attributes.
 * @return CDD_C_SUCCESS on success, or an error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t weaver_translate_gcc_attributes(
    struct PatchList *patches, const struct TokenList *tokens,
    const struct CstNodeList *cst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
