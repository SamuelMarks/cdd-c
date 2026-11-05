/**
 * @file cdd_serde_internal.h
 * @brief Internal shared declarations for C89 Serde/CLI comment directive
 * lexing and parsing.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_SERDE_INTERNAL_H
#define C_CDD_SERDE_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/memory.h"
#include "classes/parse/cdd_serde.h"
#include "functions/parse/str.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief Maximum number of identifier words split during case conversion */
#define CDD_SERDE_MAX_WORDS 32

/** @brief Maximum length of a single identifier word */
#define CDD_SERDE_MAX_WORD_LEN 64

/**
 * @brief Active target scope of a `@cdd` / `@serde` / `@cli` directive block.
 */
enum CddDirectiveScope {
  CDD_SCOPE_GENERAL = 0,   /**< Applies across all targets */
  CDD_SCOPE_TO_JSON = 1,   /**< Applies only to JSON/form serialization */
  CDD_SCOPE_FROM_JSON = 2, /**< Applies only to JSON/form deserialization */
  CDD_SCOPE_CLI = 3,       /**< Applies to both CLI parsing and emission */
  CDD_SCOPE_TO_CLI = 4,    /**< Applies only to `to_cli_argv` emission */
  CDD_SCOPE_FROM_CLI = 5   /**< Applies only to `from_cli` parsing */
};

/**
 * @brief Safely copies a string into a fixed-size destination buffer with
 * truncation.
 *
 * @param[out] dst Destination buffer (must have capacity `dst_sz > 0`).
 * @param[in] dst_sz Capacity of `dst` in bytes.
 * @param[in] src Source null-terminated string.
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t cdd_serde_copy_str(char *dst, size_t dst_sz,
                                        const char *src);

/**
 * @brief Case-insensitive ASCII string comparison helper.
 *
 * @param[in] a First string.
 * @param[in] b Second string.
 * @param[out] out_eq Output flag set to 1 if equal, 0 otherwise.
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t cdd_serde_str_ieq(const char *a, const char *b,
                                       int *out_eq);

/**
 * @brief Advances pointer past whitespace, commas, and comment asterisks.
 *
 * @param[in,out] p Pointer to current character pointer.
 * @param[in] end End of slice.
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t cdd_serde_skip_delims(const char **p, const char *end);

/**
 * @brief Extracts a quoted string or bare identifier value starting at `*p`.
 *
 * @param[in,out] p Pointer to current character pointer.
 * @param[in] end End of slice.
 * @param[out] out_val Buffer receiving the extracted string.
 * @param[in] val_sz Capacity of `out_val`.
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t cdd_serde_parse_value(const char **p, const char *end,
                                           char *out_val, size_t val_sz);

/**
 * @brief Parses a replacement tuple `("<from>" -> "<to>")` or
 * `("<from>", "<to>")` starting at `*p`.
 *
 * @param[in,out] p Pointer to current character pointer.
 * @param[in] end End of slice.
 * @param[out] out_from Buffer for source pattern.
 * @param[in] from_sz Capacity of `out_from`.
 * @param[out] out_to Buffer for target pattern.
 * @param[in] to_sz Capacity of `out_to`.
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t
cdd_serde_parse_replace_tuple(const char **p, const char *end, char *out_from,
                              size_t from_sz, char *out_to, size_t to_sz);

/**
 * @brief Locates the next `@cdd`, `@serde`, `@cli`, `@to_json`, `@from_json`,
 * `@to_cli`, `@from_cli`, `#[...(`, or `[[...]]` directive block in a comment
 * string.
 *
 * @param[in] cur Current scan pointer.
 * @param[out] out_start Start of directive arguments.
 * @param[out] out_end End of directive block.
 * @param[out] out_next Pointer to resume scanning after this block.
 * @param[out] out_scope Output pointer receiving the initial
 * `CddDirectiveScope` (optional).
 * @return CDD_C_SUCCESS on success (sets `*out_start = NULL` if none found).
 */
extern cdd_c_error_t
cdd_serde_find_directive_ex(const char *cur, const char **out_start,
                            const char **out_end, const char **out_next,
                            enum CddDirectiveScope *out_scope);

/**
 * @brief Extracts the next directive key identifier within `[ *p, end )`.
 *
 * @param[in,out] p Pointer to current scan position.
 * @param[in] end End of directive slice.
 * @param[out] key_buf Buffer receiving the key identifier.
 * @param[in] key_sz Capacity of `key_buf`.
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t cdd_serde_next_key(const char **p, const char *end,
                                        char *key_buf, size_t key_sz);

/**
 * @brief Parses a boolean string (`"true"` / `"false"` / `"1"` / `"0"`) into
 * `0` or `1`.
 *
 * @param[in] val String value.
 * @param[out] out_bool Output integer (`0` or `1`).
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t cdd_serde_parse_bool_val(const char *val, int *out_bool);

/**
 * @brief Parses a `clap` `num_args` specification (e.g. `"2"`, `"1..=3"`,
 * `"1..4"`, `"1.."`, `"..=3"`, `".."`).
 *
 * @param[in] val Input `num_args` string.
 * @param[out] out_min Minimum number of arguments.
 * @param[out] out_max Maximum number of arguments (`-1` for unbounded).
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t cdd_serde_parse_num_args(const char *val, int *out_min,
                                              int *out_max);

/**
 * @brief Scans `comment_text` for Doxygen metadata (`@brief`, `@summary`,
 * `@details`, `@description`, `@version`, `@infoVersion`, `@author`,
 * `@contact`) and plain Doxygen docstrings.
 *
 * @param[in] comment_text Raw comment block.
 * @param[out] out_brief Buffer for brief/summary text.
 * @param[in] brief_sz Capacity of `out_brief`.
 * @param[out] out_details Buffer for details/description text.
 * @param[in] details_sz Capacity of `out_details`.
 * @param[out] out_version Buffer for version text (optional).
 * @param[in] version_sz Capacity of `out_version`.
 * @param[out] out_author Buffer for author text (optional).
 * @param[in] author_sz Capacity of `out_author`.
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t cdd_serde_extract_doxygen_fallbacks(
    const char *comment_text, char *out_brief, size_t brief_sz,
    char *out_details, size_t details_sz, char *out_version, size_t version_sz,
    char *out_author, size_t author_sz);

/**
 * @brief Matches a directive scope keyword (`to_json`, `from_json`, `cli`,
 * etc.) to its `enum CddDirectiveScope`.
 *
 * @param[in] key Directive keyword identifier.
 * @param[out] out_scope Output scope (`CDD_SCOPE_GENERAL` if not a scope key).
 * @return CDD_C_SUCCESS on success.
 */
extern cdd_c_error_t
cdd_serde_match_scope_key(const char *key, enum CddDirectiveScope *out_scope);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_SERDE_INTERNAL_H */
