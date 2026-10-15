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
cdd_c_error_t c2s_parse_union_tokens_and_write(const struct TokenList *tl,
                                               size_t *idx,
                                               JSON_Object *schemas_obj,
                                               const char *union_name);

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;
#endif

/** @brief MAX_LINE_LENGTH definition */
#define MAX_LINE_LENGTH 1024

extern C_CDD_EXPORT cdd_c_error_t apply_union_to_struct_fields_ex_mut(
    const JSON_Array *union_arr, struct StructFields *dest,
    const JSON_Object *root, JSON_Object *root_mut, const char *schema_name,
    int is_anyof, const JSON_Object *schema_obj, int allow_inline);

extern C_CDD_EXPORT cdd_c_error_t c2s_json_object_to_struct_fields_internal_mut(
    const JSON_Object *o, struct StructFields *f, const JSON_Object *root,
    JSON_Object *root_mut, const char *schema_name, int allow_inline_union);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CODE2SCHEMA_INTERNAL_H */
