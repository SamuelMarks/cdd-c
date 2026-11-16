/**
 * @file bin_cdd.c
 * @brief Core bin cdd definitions and implementation.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/suppress_gui.h"

#include "cdd_c_error.h"
#ifdef CDD_C_TRANSFORMER_ONLY
#include "routes/parse/cli_cst.h"
#else
#include "functions/parse/main.h"
#endif
/* clang-format on */

/**
 * @brief Application entry point helper.
 *
 * Forwards arguments to the core command-line dispatcher (`cdd_main` or
 * `cli_transform_dispatch_main` when built with `CDD_C_TRANSFORMER_ONLY`)
 * after propagating initial GUI-suppression status.
 *
 * @param[in] init_rc Initial status returned by `cdd_suppress_gui_popups`.
 * @param[in] argc Argument count
 * @param[in] argv Argument values
 * @return CDD_C_SUCCESS on success, error code otherwise
 */
static cdd_c_error_t cdd_cli_main_internal(cdd_c_error_t init_rc, int argc,
                                           char **argv) {
  cdd_c_error_t rc;
#ifdef CDD_C_TRANSFORMER_ONLY
  rc = cli_transform_dispatch_main(argc, argv);
#else
  rc = cdd_main(argc, argv);
#endif
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  return init_rc;
}

/**
 * @brief Application entry point.
 *
 * @param[in] argc Argument count
 * @param[in] argv Argument values
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise
 */
int main(int argc, char **argv) {
  return cdd_cli_main_internal(cdd_suppress_gui_popups(), argc, argv) !=
                 CDD_C_SUCCESS
             ? 1
             : 0;
}
