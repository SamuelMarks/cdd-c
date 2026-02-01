/**
 * @file str_utils.h
 * @brief Centralized string utilities and platform compatibility safeguards.
 *
 * Consolidates memory-safe string duplication, inspection, modifications,
 * and format specifier macros.
 *
 * Added `c_cdd_destringize` for _Pragma argument handling.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_STR_UTILS_H
#define C_CDD_STR_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdio.h>

#include "c_cdd_export.h"
#include "c_cdd_stdbool.h"

/* --- Platform Compatibility Macros --- */

#if defined(_BSD_SOURCE) || defined(_GNU_SOURCE) || defined(HAVE_ASPRINTF)
/* Standard libraries have asprintf */
#else
/* Fallback for systems without asprintf (e.g. MSVC, pure C89) */
#include <c89stringutils_string_extras.h>
#endif

/**
 * @brief Format specifier for size_t/ssize_t.
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define NUM_LONG_FMT "z"
#else
#define NUM_LONG_FMT "l"
#endif

/* --- Allocation Helpers --- */

/**
 * @brief Duplicate a string.
 *
 * @param[in] s The null-terminated string to duplicate.
 * @return A pointer to the new string, or NULL if allocation failed or input
 * was NULL.
 */
extern C_CDD_EXPORT char *c_cdd_strdup(const char *s);

/* --- Inspection Helpers --- */

/**
 * @brief Check if a string starts with a given prefix.
 *
 * @param[in] str The main string to check.
 * @param[in] prefix The prefix string to look for.
 * @return true if `str` begins with `prefix`, false otherwise.
 */
extern C_CDD_EXPORT bool c_cdd_str_starts_with(const char *str,
                                               const char *prefix);

/**
 * @brief Check if two strings are equal (content-wise).
 *
 * @param[in] a First string.
 * @param[in] b Second string.
 * @return true if strings match or both are NULL, false otherwise.
 */
extern C_CDD_EXPORT bool c_cdd_str_equal(const char *a, const char *b);

/**
 * @brief Find the substring after the last occurrence of a character.
 *
 * @param[in] str The string to search.
 * @param[in] delimiter The delimiter character (e.g., '/').
 * @return Pointer to character immediately following the last delimiter.
 */
extern C_CDD_EXPORT const char *c_cdd_str_after_last(const char *str,
                                                     int delimiter);

/**
 * @brief Check if a pointer reference matches a specific type name.
 *
 * @param[in] ref The reference path.
 * @param[in] type The simple type name.
 * @return true if the extracted name matches `type`.
 */
extern C_CDD_EXPORT bool c_cdd_ref_is_type(const char *ref, const char *type);

/* --- Modification Helpers --- */

/**
 * @brief Remove trailing whitespace from a string in-place.
 *
 * @param[in,out] str The string to modify.
 */
extern C_CDD_EXPORT void c_cdd_str_trim_trailing_whitespace(char *str);

/**
 * @brief Decode a string literal token for _Pragma usage.
 *
 * Performs "destringizing" (6.10.9) to convert a string literal token into
 * the corresponding preprocessing tokens text.
 * 1. Removes surrounding quotes.
 * 2. Unescapes `\"` to `"` and `\\` to `\`.
 *
 * @param[in] quoted The string literal (with quotes).
 * @return Allocated string containing the decoded content, or NULL on error.
 */
extern C_CDD_EXPORT char *c_cdd_destringize(const char *quoted);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_STR_UTILS_H */
