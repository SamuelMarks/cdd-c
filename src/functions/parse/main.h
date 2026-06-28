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
extern C_CDD_EXPORT enum cdd_c_error cdd_main(int argc, char **argv);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
