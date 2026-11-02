/**
 * @file cdd_serde.h
 * @brief Serde-style C89 struct and enum reflection, case conversion, string
 * rewriting, and comment directive parser.
 *
 * Provides utilities to parse C89 reflection comments (`@cdd ...` or
 * `@serde ...`) atop C structs, struct fields, enums, and enum variants,
 * as well as deterministic string rewriting (`replace`, `rename_all`,
 * `uppercase`, `lowercase`, `PascalCase`, `camelCase`, `snake_case`,
 * `SCREAMING_SNAKE_CASE`, `kebab-case`, `SCREAMING-KEBAB-CASE`, `Train-Case`)
 * and effective wire-name resolution.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_SERDE_H
#define C_CDD_SERDE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "classes/emit/enum.h"
#include "classes/emit/struct.h"
/* clang-format on */

/**
 * @brief Parses a case transformation rule string into `enum CddSerdeCaseRule`.
 *
 * Recognizes `"lowercase"`, `"UPPERCASE"`, `"uppercase"`, `"PascalCase"`,
 * `"pascalcase"`, `"camelCase"`, `"camelcase"`, `"snake_case"`, `"snakecase"`,
 * `"SCREAMING_SNAKE_CASE"`, `"screaming_snakecase"`, `"kebab-case"`,
 * `"kebabcase"`, `"SCREAMING-KEBAB-CASE"`, `"screaming_kebabcase"`,
 * `"Train-Case"`, `"traincase"`, and `"none"`.
 *
 * @param[in] str Case rule identifier string.
 * @param[out] out_rule Output pointer receiving the parsed rule enum.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * unrecognized or NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_serde_parse_case_rule(const char *str, enum CddSerdeCaseRule *out_rule);

/**
 * @brief Converts an `enum CddSerdeCaseRule` value to its canonical string
 * representation.
 *
 * @param[in] rule Case transformation rule enum value.
 * @param[out] out_str Output pointer receiving the static canonical string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_serde_case_rule_to_str(enum CddSerdeCaseRule rule, const char **out_str);

/**
 * @brief Parses a CLI action string into `enum CddCliAction`.
 *
 * Recognizes `"auto"`, `"set"`, `"set_true"`, `"true"`, `"set_false"`,
 * `"false"`, `"count"`, `"append"`, `"help"`, and `"version"`.
 *
 * @param[in] str Action identifier string.
 * @param[out] out_action Output pointer receiving the parsed action enum.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * unrecognized or NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_cli_parse_action(const char *str, enum CddCliAction *out_action);

/**
 * @brief Converts an `enum CddCliAction` value to its canonical string
 * representation.
 *
 * @param[in] action CLI action enum value.
 * @param[out] out_str Output pointer receiving the static canonical string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_cli_action_to_str(enum CddCliAction action, const char **out_str);

/**
 * @brief Parses a CLI value hint string into `enum CddCliValueHint`.
 *
 * Recognizes `"none"`, `"file"`, `"file_path"`, `"dir"`, `"dir_path"`,
 * `"path"`, `"any_path"`, `"cmd"`, `"executable"`, `"url"`, `"hostname"`,
 * `"username"`, and `"email"`.
 *
 * @param[in] str Value hint identifier string.
 * @param[out] out_hint Output pointer receiving the parsed value hint enum.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * unrecognized or NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_cli_parse_value_hint(const char *str, enum CddCliValueHint *out_hint);

/**
 * @brief Converts an `enum CddCliValueHint` value to its canonical string
 * representation.
 *
 * @param[in] hint CLI value hint enum value.
 * @param[out] out_str Output pointer receiving the static canonical string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_cli_value_hint_to_str(enum CddCliValueHint hint, const char **out_str);

/**
 * @brief Transforms an identifier string according to a `CddSerdeCaseRule`.
 *
 * Splits `input` into words across `_`, `-`, whitespace, and `lower->UPPER` or
 * `UPPER->Upper+lower` boundaries, then formats into `out_buf`.
 *
 * @param[in] input Input C identifier or string.
 * @param[in] rule Case rule to apply.
 * @param[out] out_buf Destination buffer for the transformed string.
 * @param[in] out_sz Capacity of `out_buf` in bytes.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_serde_transform_case(const char *input, enum CddSerdeCaseRule rule,
                         char *out_buf, size_t out_sz);

/**
 * @brief Replaces all occurrences of `from_pat` with `to_pat` in `input`.
 *
 * @param[in] input Source string to rewrite.
 * @param[in] from_pat Substring pattern to search for (e.g. `"_"` or `"-"`).
 * @param[in] to_pat Replacement string (e.g. `"-"` or `"_"`).
 * @param[out] out_buf Destination buffer for the rewritten string.
 * @param[in] out_sz Capacity of `out_buf` in bytes.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_serde_replace_str(const char *input,
                                                        const char *from_pat,
                                                        const char *to_pat,
                                                        char *out_buf,
                                                        size_t out_sz);

/**
 * @brief Resolves the effective serialization (`out_ser_name`) and
 * deserialization (`out_de_name`) wire names for a struct field.
 *
 * Layers transformations in order:
 * 1. Base C identifier (`field->name`)
 * 2. Container `serde_prefix` / `serde_suffix`
 * 3. Container `serde_rename_all` (and `serde_rename_all_ser` / `_de`)
 * 4. Container `serde_replace_from` -> `serde_replace_to`
 * 5. Field `serde_case` override
 * 6. Field `serde_replace_from` -> `serde_replace_to` override
 * 7. Field explicit `serde_rename` override
 * 8. Field explicit `serde_rename_ser` / `serde_rename_de` overrides
 *
 * @param[in] sf Enclosing `StructFields` container.
 * @param[in] field_idx Index of the field within `sf->fields`.
 * @param[out] out_ser_name Optional buffer receiving the serialization wire
 * name (may be NULL).
 * @param[in] ser_sz Capacity of `out_ser_name`.
 * @param[out] out_de_name Optional buffer receiving the deserialization wire
 * name (may be NULL).
 * @param[in] de_sz Capacity of `out_de_name`.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_serde_resolve_field_names(
    const struct StructFields *sf, size_t field_idx, char *out_ser_name,
    size_t ser_sz, char *out_de_name, size_t de_sz);

/**
 * @brief Resolves the effective wire string for an enum variant.
 *
 * @param[in] em Enclosing `EnumMembers` container.
 * @param[in] member_idx Index of the enum variant within `em->members`.
 * @param[out] out_name Buffer receiving the resolved wire name.
 * @param[in] out_sz Capacity of `out_name`.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_serde_resolve_enum_member_name(
    const struct EnumMembers *em, size_t member_idx, char *out_name,
    size_t out_sz);

/**
 * @brief Parses container-level `@cdd` or `@serde`
 * directives into a `StructFields` container.
 *
 * @param[in] comment_text Raw C comment text (may contain multiple comments).
 * @param[in,out] sf Target `StructFields` container to update.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_serde_parse_container_comment(
    const char *comment_text, struct StructFields *sf);

/**
 * @brief Parses container-level `@cdd` or `@serde`
 * directives into an `EnumMembers` container.
 *
 * @param[in] comment_text Raw C comment text.
 * @param[in,out] em Target `EnumMembers` container to update.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_serde_parse_enum_comment(const char *comment_text, struct EnumMembers *em);

/**
 * @brief Parses field-level `@cdd` or `@serde` directives
 * into a `StructField` descriptor.
 *
 * @param[in] comment_text Raw C comment text.
 * @param[in,out] field Target `StructField` to update.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_serde_parse_field_comment(
    const char *comment_text, struct StructField *field);

/**
 * @brief Parses enum-variant-level `@cdd` or `@serde`
 * directives for a single enum constant.
 *
 * @param[in] comment_text Raw C comment text.
 * @param[out] out_rename Buffer receiving explicit variant rename (or empty).
 * @param[in] rename_sz Capacity of `out_rename`.
 * @param[out] out_skip Output flag set to 1 if `@cdd skip` is present.
 * @param[out] out_case Output case rule if a variant-level case flag is set.
 * @param[out] out_rep_from Buffer receiving variant-level replace source.
 * @param[in] rep_from_sz Capacity of `out_rep_from`.
 * @param[out] out_rep_to Buffer receiving variant-level replace target.
 * @param[in] rep_to_sz Capacity of `out_rep_to`.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_serde_parse_enum_variant_comment(
    const char *comment_text, char *out_rename, size_t rename_sz, int *out_skip,
    enum CddSerdeCaseRule *out_case, char *out_rep_from, size_t rep_from_sz,
    char *out_rep_to, size_t rep_to_sz);

/**
 * @brief Extended parser for enum-variant-level `@cdd` or `@serde` directives
 * including `alias` and `other`.
 *
 * @param[in] comment_text Raw C comment text.
 * @param[out] out_rename Buffer receiving explicit variant rename (or empty).
 * @param[in] rename_sz Capacity of `out_rename`.
 * @param[out] out_skip Output flag set to 1 if `@cdd skip` is present.
 * @param[out] out_case Output case rule if a variant-level case flag is set.
 * @param[out] out_rep_from Buffer receiving variant-level replace source.
 * @param[in] rep_from_sz Capacity of `out_rep_from`.
 * @param[out] out_rep_to Buffer receiving variant-level replace target.
 * @param[in] to_sz Capacity of `out_rep_to`.
 * @param[out] out_other Optional output flag set to 1 if `@cdd other` is
 * present.
 * @param[in,out] em_for_aliases Optional `EnumMembers` container to append
 * aliases to.
 * @param[in] member_idx Index of the variant in `em_for_aliases`.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_serde_parse_enum_variant_comment_ex(
    const char *comment_text, char *out_rename, size_t rename_sz, int *out_skip,
    enum CddSerdeCaseRule *out_case, char *out_rep_from, size_t rep_from_sz,
    char *out_rep_to, size_t to_sz, int *out_other,
    struct EnumMembers *em_for_aliases, size_t member_idx);

/**
 * @brief Escapes a raw wire string for safe inclusion inside generated C string
 * literals (`"..."`), handling `"`, `\`, `\b`, `\f`, `\n`, `\r`, `\t`, and
 * control characters `0x01..0x1F`.
 *
 * @param[in] raw Raw unescaped string.
 * @param[out] out Destination buffer.
 * @param[in] out_sz Capacity of `out` in bytes.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_escape_json_c_str(const char *raw,
                                                        char *out,
                                                        size_t out_sz);

/**
 * @brief Escapes a raw string for safe inclusion inside C89 block comment
 * directive arguments, escaping `"`, `\`, and preventing star-slash from
 * closing the block comment prematurely.
 *
 * @param[in] raw Raw unescaped string.
 * @param[out] out Destination buffer.
 * @param[in] out_sz Capacity of `out` in bytes.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_escape_comment_str(const char *raw,
                                                         char *out,
                                                         size_t out_sz);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_SERDE_H */
