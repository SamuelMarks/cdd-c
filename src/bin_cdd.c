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
