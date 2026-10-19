/**
 * @file code2schema_internal.h
 * @brief Internal declarations and utilities for code2schema modules.
 * @author Samuel Marks
 */

#ifndef CODE2SCHEMA_INTERNAL_H
#define CODE2SCHEMA_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../win_compat_sym.h"

#include <parson.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "classes/emit/struct.h"
#include "classes/parse/code2schema.h"
#include "classes/parse/mapping.h"
#include "classes/parse/numeric.h"
#include "functions/emit/codegen.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/**
 * @brief Parses union member tokens from a TokenList starting after '{' and
 * writes the resulting oneOf schema to schemas_obj.
 *
 * @param[in] tl TokenList containing union body tokens.
 * @param[in,out] idx Pointer to current token index (starts after '{', updated
 * to after '}').
 * @param[in,out] schemas_obj Target schemas JSON object.
 * @param[in] union_name Name of the union.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_parse_union_tokens_and_write(
    const struct TokenList *tl, size_t *idx, JSON_Object *schemas_obj,
    const char *union_name);

/**
 * @brief Copies up to 63 characters from a token into a 64-byte buffer.
 *
 * @param[out] dst Target 64-byte buffer.
 * @param[in] tok Source token.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t c2s_copy_token_ident(char *dst, const struct Token *tok);

/**
 * @brief Retrieves token kind at index or TOKEN_UNKNOWN if out of bounds.
 *
 * @param[in] tl Token list.
 * @param[in] idx Token index.
 * @param[out] out_kind Output token kind.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t c2s_tok_kind(const struct TokenList *tl, size_t idx,
                           enum TokenKind *out_kind);

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;
#endif

/** @brief MAX_LINE_LENGTH definition */
#define MAX_LINE_LENGTH 1024

/**
 * @brief Applies a `oneOf`/`anyOf` JSON array to `StructFields` with mutable
 * root support.
 *
 * @param[in] union_arr JSON array of union branch schemas.
 * @param[in,out] dest Destination `StructFields` to populate.
 * @param[in] root Read-only root OpenAPI/JSON Schema object.
 * @param[in,out] root_mut Optional mutable root object for hoisted schemas.
 * @param[in] schema_name Parent schema name.
 * @param[in] is_anyof Non-zero if `anyOf`, 0 if `oneOf`.
 * @param[in] schema_obj Optional enclosing schema object for discriminator.
 * @param[in] allow_inline Non-zero to allow inline union fields.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
extern C_CDD_EXPORT cdd_c_error_t apply_union_to_struct_fields_ex_mut(
    const JSON_Array *union_arr, struct StructFields *dest,
    const JSON_Object *root, JSON_Object *root_mut, const char *schema_name,
    int is_anyof, const JSON_Object *schema_obj, int allow_inline);

/**
 * @brief Converts a JSON Schema object into `StructFields` with mutable root
 * support.
 *
 * @param[in] o JSON Schema object to convert.
 * @param[in,out] f Destination `StructFields` structure.
 * @param[in] root Read-only root OpenAPI/JSON Schema object.
 * @param[in,out] root_mut Optional mutable root object for hoisted schemas.
 * @param[in] schema_name Name of the schema being converted.
 * @param[in] allow_inline_union Non-zero to allow inline union fields.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_json_object_to_struct_fields_internal_mut(
    const JSON_Object *o, struct StructFields *f, const JSON_Object *root,
    JSON_Object *root_mut, const char *schema_name, int allow_inline_union);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CODE2SCHEMA_INTERNAL_H */
