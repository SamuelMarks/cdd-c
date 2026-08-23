#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
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
#include "cdd_c_error.h"
#include "classes/emit/struct.h"
/* clang-format on */

/**
 * @brief Writes a struct-to-form-urlencoded function.
 * @param fp Output file.
 * @param struct_name Name of the struct.
 * @param sf Struct fields list.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_to_form_urlencoded_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_FORM_H */

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
