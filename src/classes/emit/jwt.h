/**
 * @file jwt.h
 * @brief JWT codegen definitions
 */

#ifndef C_CDD_CODEGEN_JWT_H
#define C_CDD_CODEGEN_JWT_H

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
 * @brief Writes a JWT parsing function for a struct.
 * @param fp The file pointer to write to.
 * @param struct_name The name of the struct.
 * @param sf The fields of the struct.
 * @return 0 on success, error code otherwise.
 */
extern C_CDD_EXPORT enum cdd_c_error
write_struct_from_jwt_func(FILE *fp, const char *struct_name,
                           const struct StructFields *sf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_JWT_H */
