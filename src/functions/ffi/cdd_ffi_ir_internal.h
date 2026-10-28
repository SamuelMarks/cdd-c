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

/** @brief Test fault-injection counter for FFI extractor allocations */
extern C_CDD_EXPORT int g_ffi_extractor_alloc_fail;

/**
 * @brief Allocates memory for FFI IR with test fault injection.
 * @param[in] sz Byte size to allocate.
 * @param[out] out_ptr Output allocated pointer.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_ffi_do_malloc(size_t sz, void **out_ptr);

/**
 * @brief Zero-allocates memory for FFI IR with test fault injection.
 * @param[in] n Number of elements.
 * @param[in] sz Element byte size.
 * @param[out] out_ptr Output allocated pointer.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_ffi_do_calloc(size_t n, size_t sz,
                                                    void **out_ptr);

/**
 * @brief Reallocates memory for FFI IR with test fault injection.
 * @param[in,out] ptr Existing pointer (or NULL).
 * @param[in] sz New byte size.
 * @param[out] out_ptr Output reallocated pointer.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_ffi_do_realloc(void *ptr, size_t sz,
                                                     void **out_ptr);

/**
 * @brief Duplicates a string for FFI IR with test fault injection.
 * @param[in] s Source string.
 * @param[out] out_str Output duplicated string pointer.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_ffi_do_strdup(const char *s,
                                                    char **out_str);

/** @brief Allocates memory via FFI test-aware allocator */
#define CDD_MALLOC(sz)                                                         \
  (cdd_ffi_do_malloc((sz), &g_cdd_last_alloc_ptr), g_cdd_last_alloc_ptr)
/** @brief Zero-allocates memory via FFI test-aware allocator */
#define CDD_CALLOC(n, sz)                                                      \
  (cdd_ffi_do_calloc((n), (sz), &g_cdd_last_alloc_ptr), g_cdd_last_alloc_ptr)
/** @brief Reallocates memory via FFI test-aware allocator */
#define CDD_REALLOC(ptr, sz)                                                   \
  (cdd_ffi_do_realloc((ptr), (sz), &g_cdd_last_alloc_ptr), g_cdd_last_alloc_ptr)
/** @brief Duplicates string via FFI test-aware allocator */
#define CDD_STRDUP(s)                                                          \
  (cdd_ffi_do_strdup((s), &g_cdd_last_strdup_ptr), g_cdd_last_strdup_ptr)

/**
 * @brief Maps a C type string to its corresponding FFI primitive kind.
 * @param[in] c_type C type string.
 * @param[out] out_kind Pointer to receive mapped primitive kind.
 * @return CDD_C_SUCCESS on success or error code.
 */
cdd_c_error_t map_c_type_to_ffi_kind(const char *c_type,
                                     cdd_ffi_primitive_kind_t *out_kind);

/**
 * @brief Parses a C++ template type string into an FFI type representation.
 *
 * @param[in] c_type C++ template type string.
 * @param[out] out_type Destination FFI type pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
cdd_c_error_t parse_template_type(const char *c_type, cdd_ffi_type_t *out_type);

/**
 * @brief Generate a flattened C-compatible FFI symbol from C++ namespace,
 * class, and method names.
 *
 * @param[in] ns_name Optional C++ namespace name (may be NULL).
 * @param[in] class_name Optional C++ class name (may be NULL).
 * @param[in] method_name Method or function identifier.
 * @param[out] out_mangled Newly allocated mangled symbol string.
 * @return 0 on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_mangle_cpp_name(const char *ns_name,
                                                   const char *class_name,
                                                   const char *method_name,
                                                   char **out_mangled);

/**
 * @brief Adds a new node to the FFI IR.
 *
 * @param[in,out] ir Pointer to FFI IR structure.
 * @param[in] kind Kind of node to add.
 * @param[in] name Name of the node.
 * @param[out] out_node Optional pointer to receive the created node.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t ir_add_node(cdd_ffi_ir_t *ir,
                                       cdd_ffi_node_kind_t kind,
                                       const char *name,
                                       cdd_ffi_ir_node_t **out_node);

/**
 * @brief Instantiates templated types discovered in the FFI IR.
 *
 * @param[in,out] ir Pointer to FFI IR structure.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
cdd_c_error_t instantiate_templates(cdd_ffi_ir_t *ir);

#ifdef CDD_BUILD_TESTS
/**
 * @brief Expose map_c_type_to_ffi_kind for testing.
 * @param[in] c_type Type string.
 * @param[out] out_kind Destination primitive kind pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_map_c_type_to_ffi_kind_test(
    const char *c_type, cdd_ffi_primitive_kind_t *out_kind);

/**
 * @brief Expose parse_template_type for testing.
 * @param[in] c_type Type string.
 * @param[out] out_type Destination FFI type pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_ffi_parse_template_type_test(const char *c_type, cdd_ffi_type_t *out_type);

/**
 * @brief Expose instantiate_templates for testing.
 * @param[in,out] ir Pointer to FFI IR structure.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_instantiate_templates_test(cdd_ffi_ir_t *ir);
#endif /* CDD_BUILD_TESTS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CDD_FFI_IR_INTERNAL_H */
