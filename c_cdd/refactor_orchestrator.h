/**
 * @file refactor_orchestrator.h
 * @brief High-level orchestration for converting unsafe C code to strictly
 * checked safety-patterned C code.
 * @author Samuel Marks
 */

#ifndef REFACTOR_ORCHESTRATOR_H
#define REFACTOR_ORCHESTRATOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_cdd_export.h>

/**
 * @brief Apply the "fix" workflow to a single C source string.
 *
 * Workflow:
 * 1. Parse content into Tokens and CST (Concrete Syntax Tree).
 * 2. Analyze all memory allocations.
 * 3. Identify functions that return pointers/void and perform allocations.
 * 4. Refactor signatures (e.g., `char* f()` -> `int f(char **out)`).
 * 5. Refactor bodies (inject checks, rewrite returns).
 * 6. Refactor call-sites (propagate error handling).
 *
 * @param[in] source_code The null-terminated C source code string.
 * @param[out] out_code Pointer to char* where the allocated result string will
 * be stored.
 * @return 0 on success, error code (ENOMEM/EINVAL) on failure.
 */
extern C_CDD_EXPORT int orchestrate_fix(const char *source_code,
                                        char **out_code);

/**
 * @brief Command-line entry point for the fix functionality.
 * Reads input file, processes it, and writes to output file.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return EXIT_SUCCESS or EXIT_FAILURE.
 */
extern C_CDD_EXPORT int fix_code_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* REFACTOR_ORCHESTRATOR_H */
