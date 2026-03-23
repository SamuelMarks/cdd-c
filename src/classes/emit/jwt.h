#ifndef C_CDD_CODEGEN_JWT_H
#define C_CDD_CODEGEN_JWT_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include <stdio.h>
#include "c_cdd_export.h"
#include "classes/emit/struct.h"

/* clang-format on */
extern C_CDD_EXPORT int
write_struct_from_jwt_func(FILE *fp, const char *struct_name,
                           const struct StructFields *sf);

#ifdef __cplusplus
}
#endif

#endif /* C_CDD_CODEGEN_JWT_H */