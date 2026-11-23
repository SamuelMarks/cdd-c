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

#include "win_compat_sym.h"

#include "c_cdd/json_dom.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd/reflect.h"
#include "c_cdd/safe_crt.h"
#include "classes/emit/cli_struct.h"
#include "classes/emit/struct.h"
#include "classes/parse/cdd_serde.h"
#include "classes/parse/cdd_serde_internal.h"
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

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;
extern C_CDD_EXPORT int g_cdd_c2s_fail_step;
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

/**
 * @brief Frees a dynamically allocated string array (`*_free` destructor
 * alias).
 *
 * @param[in,out] arr Array of strings to free.
 * @param[in] n Number of elements in `arr`.
 * @return CDD_C_SUCCESS.
 */
cdd_c_error_t c2s_string_array_free(char **arr, size_t n);

/**
 * @brief Looks up an optional string property on a JSON_Object, setting
 * `*out_str` to NULL when absent or non-string.
 *
 * @param[in] obj JSON_Object to inspect.
 * @param[in] key Property name.
 * @param[out] out_str Output pointer receiving the string or NULL.
 * @return CDD_C_SUCCESS on success, or error code.
 */
cdd_c_error_t c2s_json_opt_string(const JSON_Object *obj, const char *key,
                                  const char **out_str);

/**
 * @brief Looks up an optional object property on a JSON_Object, setting
 * `*out_obj` to NULL when absent or non-object.
 *
 * @param[in] obj JSON_Object to inspect.
 * @param[in] key Property name.
 * @param[out] out_obj Output pointer receiving the child JSON_Object or NULL.
 * @return CDD_C_SUCCESS on success, or error code.
 */
cdd_c_error_t c2s_json_opt_object(const JSON_Object *obj, const char *key,
                                  JSON_Object **out_obj);

/**
 * @brief Looks up an optional array property on a JSON_Object, setting
 * `*out_arr` to NULL when absent or non-array.
 *
 * @param[in] obj JSON_Object to inspect.
 * @param[in] key Property name.
 * @param[out] out_arr Output pointer receiving the child JSON_Array or NULL.
 * @return CDD_C_SUCCESS on success, or error code.
 */
cdd_c_error_t c2s_json_opt_array(const JSON_Object *obj, const char *key,
                                 JSON_Array **out_arr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CODE2SCHEMA_INTERNAL_H */
