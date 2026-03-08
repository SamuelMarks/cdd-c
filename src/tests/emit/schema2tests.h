/**
 * @file schema2tests.h
 * @brief Logic for generating C test suites from JSON schemas.
 * @author Samuel Marks
 */

#ifndef SCHEMA2TESTS_H
#define SCHEMA2TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <c_cdd_export.h>

/**
 * @brief Entry point for generating tests from a JSON schema.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector. argv[0]=schema, argv[1]=header_to_test,
 * argv[2]=output_test_file.
 * @return 0 on success, non-zero code on failure.
 */
extern C_CDD_EXPORT int jsonschema2tests_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* !SCHEMA2TESTS_H */
