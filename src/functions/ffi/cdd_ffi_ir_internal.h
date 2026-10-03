/**
 * @file cdd_ffi_ir_internal.h
 * @brief Internal declarations for FFI IR extraction and templates.
 *
 * @author Samuel Marks
 */

#ifndef CDD_FFI_IR_INTERNAL_H
#define CDD_FFI_IR_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include <stddef.h>

#include "cdd_c_error.h"
#include "ffi/cdd_ffi_ir.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_ffi_extractor_alloc_fail;
void *cdd_ffi_malloc_impl(size_t sz);
void *cdd_ffi_calloc_impl(size_t n, size_t sz);
void *cdd_ffi_realloc_impl(void *ptr, size_t sz);
char *cdd_ffi_strdup_impl(const char *s);

#define CDD_MALLOC(sz) cdd_ffi_malloc_impl(sz)
#define CDD_CALLOC(n, sz) cdd_ffi_calloc_impl((n), (sz))
#define CDD_REALLOC(ptr, sz) cdd_ffi_realloc_impl((ptr), (sz))
#define CDD_STRDUP(s) cdd_ffi_strdup_impl(s)
#else
#define CDD_MALLOC(sz) malloc(sz)
#define CDD_CALLOC(n, sz) calloc(n, sz)
#define CDD_REALLOC(ptr, sz) realloc(ptr, sz)
#define CDD_STRDUP(s) strdup(s)
#endif

cdd_c_error_t map_c_type_to_ffi_kind(const char *c_type,
                                     cdd_ffi_primitive_kind_t *out_kind);

cdd_c_error_t parse_template_type(const char *c_type, cdd_ffi_type_t *out_type);

C_CDD_EXPORT cdd_c_error_t cdd_ffi_mangle_cpp_name(const char *ns_name,
                                                   const char *class_name,
                                                   const char *method_name,
                                                   char **out_mangled);

cdd_c_error_t ir_add_node(cdd_ffi_ir_t *ir, cdd_ffi_node_kind_t kind,
                          const char *name, cdd_ffi_ir_node_t **out_node);

cdd_c_error_t instantiate_templates(cdd_ffi_ir_t *ir);

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT cdd_c_error_t cdd_ffi_map_c_type_to_ffi_kind_test(
    const char *c_type, cdd_ffi_primitive_kind_t *out_kind);

C_CDD_EXPORT cdd_c_error_t
cdd_ffi_parse_template_type_test(const char *c_type, cdd_ffi_type_t *out_type);

C_CDD_EXPORT cdd_c_error_t cdd_ffi_instantiate_templates_test(cdd_ffi_ir_t *ir);
#endif /* CDD_BUILD_TESTS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CDD_FFI_IR_INTERNAL_H */
