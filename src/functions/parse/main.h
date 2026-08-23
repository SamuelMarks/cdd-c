#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#ifndef C_CDD_MAIN_H
#define C_CDD_MAIN_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */
/** @brief Main entry point function */
extern C_CDD_EXPORT cdd_c_error_t cdd_main(int argc, char **argv);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
