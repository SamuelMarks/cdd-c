/**
 * @file oauth2_error.h
 * @brief OAuth2 Error Codegen definitions
 */

#ifndef C_CDD_CODEGEN_OAUTH2_ERROR_H
#define C_CDD_CODEGEN_OAUTH2_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "classes/emit/struct.h"
/* clang-format on */

/**
 * @brief Writes an OAuth2 Error parser function
 * @param fp The file pointer to write to
 * @param struct_name Name of the struct
 * @param sf Struct fields
 * @return 0 on success, error code otherwise
 */
extern C_CDD_EXPORT enum cdd_c_error
write_oauth2_error_parser_func(FILE *fp, const char *struct_name,
                               const struct StructFields *sf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_OAUTH2_ERROR_H */
