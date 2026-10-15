/**
 * @file cdd_helpers.h
 * @brief Helper functions for unit testing.
 * @author Samuel Marks
 */

#ifndef C_CDD_TESTS_CDD_HELPERS_H
#define C_CDD_TESTS_CDD_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "cdd_test_helpers_export.h"
#include <stdio.h>
/* clang-format on */

/**
 * @brief Callback for assertion failures in dependencies.
 * Usually stubbed out or prints to stderr.
 */
CDD_TEST_HELPERS_EXPORT void cdd_precondition_failed(void);

/**
 * @brief Create a temporary file in the test sandbox and return its stream.
 *
 * @return Open FILE pointer on success, or NULL on error.
 */
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);

/**
 * @brief Clean up all temporary files and directories created by tests.
 */
CDD_TEST_HELPERS_EXPORT void cdd_test_tmpfile_cleanup(void);

/**
 * @brief Write string content to a file.
 * Helper for setting up test conditions.
 *
 * @param[in] filename Path to the file to write.
 * @param[in] contents The data to write.
 * @return CDD_C_SUCCESS on success, or an error code on failure.
 */
CDD_TEST_HELPERS_EXPORT cdd_c_error_t write_to_file(const char *filename,
                                                    const char *contents);

/**
 * @brief Returns a mutable copy of a string in a rotating static test buffer.
 *
 * @param[in] str Input string.
 * @return Mutable pointer to a copy of str, or NULL if str is NULL.
 */
CDD_TEST_HELPERS_EXPORT char *cdd_test_mut_str(const char *str);

#ifndef C_CDD_STR_LIT
/** @brief Helper macro for mutable test string literals */
#define C_CDD_STR_LIT(s) cdd_test_mut_str(s)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_TESTS_CDD_HELPERS_H */
