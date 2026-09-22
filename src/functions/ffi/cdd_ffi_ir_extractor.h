#ifndef CDD_FFI_IR_EXTRACTOR_H
#define CDD_FFI_IR_EXTRACTOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#if defined(_MSC_VER) && (_MSC_VER < 1600)
#include "msvc/stdint.h"
#else
#include <stdint.h>
#endif
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "../../include/ffi/cdd_ffi_ir.h"
#include "../../src/cdd_api.h"
/* clang-format on */

/**
 * @file cdd_ffi_ir_extractor.h
 * @brief Logic for extracting exported symbols to the Universal FFI IR.
 */

/**
 * @brief Generate a flattened C-compatible FFI symbol from C++ namespace,
 * class, and method names.
 *
 * @param ns_name The namespace (can be NULL).
 * @param class_name The class name (can be NULL).
 * @param method_name The method name.
 * @param out_mangled Pointer to receive the allocated mangled name.
 * @return 0 on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_mangle_cpp_name(const char *ns_name,
                                                   const char *class_name,
                                                   const char *method_name,
                                                   char **out_mangled);

/**
 * @brief Extracts public exports into the FFI IR.
 * @param filename The path to the file.
 * @param content The text content of the file.
 * @param config Configuration options.
 * @param out_ir The pointer to the resulting FFI IR.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_ir_extract_exports(
    const char *filename, const char *content,
    const cdd_generate_bindings_config_t *config, cdd_ffi_ir_t **out_ir);

#ifdef CDD_BUILD_TESTS
/**
 * @brief Expose int64_to_str for testing.
 * @param val Value to convert.
 * @param buf Destination buffer.
 * @param buf_sz Size of destination buffer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_int64_to_str_test(int64_t val, char *buf,
                                                     size_t buf_sz);

/**
 * @brief Expose map_c_type_to_ffi_kind for testing.
 * @param c_type Type string.
 * @param out_kind Destination primitive kind pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_map_c_type_to_ffi_kind_test(
    const char *c_type, cdd_ffi_primitive_kind_t *out_kind);

/**
 * @brief Expose parse_template_type for testing.
 * @param c_type Type string.
 * @param out_type Destination FFI type pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_ffi_parse_template_type_test(const char *c_type, cdd_ffi_type_t *out_type);

/**
 * @brief Expose extract_single_file_exports for testing.
 * @param ir Pointer to FFI IR structure.
 * @param filename Name of file.
 * @param content String content.
 * @param config Pointer to generator configuration.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_extract_single_file_exports_test(
    cdd_ffi_ir_t *ir, const char *filename, const char *content,
    const cdd_generate_bindings_config_t *config);

/**
 * @brief Expose is_visited for testing.
 * @param ctx Pointer to include merge context.
 * @param path File path.
 * @param out_visited Pointer to output integer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_is_visited_test(void *ctx, const char *path,
                                                   int *out_visited);

/**
 * @brief Expose add_visited for testing.
 * @param ctx Pointer to include merge context.
 * @param path File path.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_add_visited_test(void *ctx,
                                                    const char *path);

/**
 * @brief Expose include_visitor for testing.
 * @param info Pointer to include information.
 * @param user_data Pointer to user context.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_include_visitor_test(const void *info,
                                                        void *user_data);

/**
 * @brief Expose extract_exports_recursive for testing.
 * @param filename File path.
 * @param content String content.
 * @param ctx Pointer to user context.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_extract_exports_recursive_test(
    const char *filename, const char *content, void *ctx);

/**
 * @brief Expose instantiate_templates for testing.
 * @param ir Pointer to FFI IR.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_instantiate_templates_test(cdd_ffi_ir_t *ir);

/**
 * @brief Expose ir_add_node for testing.
 * @param ir Pointer to FFI IR.
 * @param kind Node kind.
 * @param name Node name.
 * @param out_node Pointer to receive node.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_ffi_ir_add_node_test(cdd_ffi_ir_t *ir, cdd_ffi_node_kind_t kind,
                         const char *name, cdd_ffi_ir_node_t **out_node);

/**
 * @brief Expose malloc for testing.
 * @param sz Size to allocate.
 * @param out_p Destination pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_malloc_test(size_t sz, void **out_p);

/**
 * @brief Expose calloc for testing.
 * @param n Count.
 * @param sz Size of each element.
 * @param out_p Destination pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_calloc_test(size_t n, size_t sz,
                                               void **out_p);

/**
 * @brief Expose realloc for testing.
 * @param p Pointer to reallocate.
 * @param sz New size.
 * @param out_p Destination pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_realloc_test(void *p, size_t sz,
                                                void **out_p);

/**
 * @brief Expose strdup for testing.
 * @param s String to duplicate.
 * @param out_s Destination string pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_strdup_test(const char *s, char **out_s);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_IR_EXTRACTOR_H */
