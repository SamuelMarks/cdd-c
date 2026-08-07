/* clang-format off */
#include "functions/parse/main.h"
#include "cdd_c_error.h"
/* clang-format on */

/**
 * @brief Application entry point helper.
 *
 * Forwards arguments to the core command-line dispatcher (`cdd_main`).
 *
 * @param[in] argc Argument count
 * @param[in] argv Argument values
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_UNKNOWN otherwise
 */
static cdd_c_error_t cdd_cli_main_internal(int argc, char **argv) {
  int rc;
  rc = cdd_main(argc, argv);
  if (rc != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Application entry point.
 *
 * @param[in] argc Argument count
 * @param[in] argv Argument values
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise
 */
int main(int argc, char **argv) {
  cdd_c_error_t rc;
  rc = cdd_cli_main_internal(argc, argv);
  if (rc != CDD_C_SUCCESS) {
    return 1;
  }
  return 0;
}
