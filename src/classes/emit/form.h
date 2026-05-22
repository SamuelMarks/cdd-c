/**
 * @file form.h
 * @brief Generation of URL encoded forms.
 */

#ifndef C_CDD_CODEGEN_FORM_H
#define C_CDD_CODEGEN_FORM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>
#include "c_cdd_export.h"
#include "classes/emit/struct.h"
/* clang-format on */

/**
 * @brief Writes a struct-to-form-urlencoded function.
 * @param fp Output file.
 * @param struct_name Name of the struct.
 * @param sf Struct fields list.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_struct_to_form_urlencoded_func(FILE *fp, const char *struct_name,
                                     const struct StructFields *sf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_FORM_H */
