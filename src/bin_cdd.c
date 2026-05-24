/* clang-format off */
#include "functions/parse/main.h"
/* clang-format on */
/**
 * @brief Application entry point.
 *
 * Forwards arguments to the core command-line dispatcher (`cdd_main`).
 *
 * @param[in] argc Argument count
 * @param[in] argv Argument values
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise
 */
int main(int argc, char **argv) { return cdd_main(argc, argv); }

#ifdef CDD_BUILD_TESTS
int g_cdd_fail_alloc = 0;
int g_cdd_fail_alloc_audit = 0;
int g_cdd_fprintf_fail = 0;
int g_cdd_mock_dlopen_success = 0;
#endif
