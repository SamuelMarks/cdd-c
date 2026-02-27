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
#include "cdd_test_helpers_export.h"
#include <stdio.h>

/**
 * @brief Callback for assertion failures in dependencies.
 * Usually stubbed out or prints to stderr.
 */
extern CDD_TEST_HELPERS_EXPORT void cdd_precondition_failed(void);

/**
 * @brief Write string content to a file.
 * Helper for setting up test conditions.
 *
 * @param[in] filename Path to the file to write.
 * @param[in] contents The data to write.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
extern CDD_TEST_HELPERS_EXPORT int write_to_file(const char *filename,
                                                 const char *contents);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_TESTS_CDD_HELPERS_H */
