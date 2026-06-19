#ifndef CDD_FFI_IR_EXTRACTOR_H
#define CDD_FFI_IR_EXTRACTOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
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
C_CDD_EXPORT int cdd_ffi_mangle_cpp_name(const char *ns_name,
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
C_CDD_EXPORT int
cdd_ffi_ir_extract_exports(const char *filename, const char *content,
                           const cdd_generate_bindings_config_t *config,
                           cdd_ffi_ir_t **out_ir);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_IR_EXTRACTOR_H */