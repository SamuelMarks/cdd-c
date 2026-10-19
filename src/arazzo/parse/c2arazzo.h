/**
 * @file c2arazzo.h
 * @brief C Source/Header to Arazzo v1.1.0 Specification Extractor.
 *
 * Extracts Arazzo v1.1.0 workflows, steps, source descriptions, and reusable
 * components from Doxygen annotations (`@arazzo`, `@workflow`, `@step`, etc.),
 * C SDK workflow orchestrator functions (`<prefix>workflow_<workflowId>`),
 * CLI workflow dispatchers, and `greatest.h` E2E workflow tests
 * (`TEST test_workflow_<workflowId>`).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_C2ARAZZO_H
/** @brief Header guard for c2arazzo.h */
#define C_CDD_C2ARAZZO_H

/* clang-format off */
#include "arazzo/parse/arazzo_types.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "docstrings/parse/doc.h"
#include "c_cdd/no_discard.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Applies parsed Arazzo docstring metadata onto an `Arazzo_Spec`.
 *
 * @param[in] meta Parsed docstring metadata block.
 * @param[in,out] spec Target Arazzo specification structure.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` on NULL,
 * or `CDD_C_ERROR_MEMORY` on OOM.
 */
C_CDD_EXPORT cdd_c_error_t c2arazzo_apply_doc_metadata(
    const struct DocMetadata *meta, struct Arazzo_Spec *spec);

/**
 * @brief Extracts Arazzo v1.1.0 workflows and metadata from an in-memory C
 * source string.
 *
 * @param[in] source Null-terminated C source/header text.
 * @param[in,out] spec Target Arazzo specification structure.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t c2arazzo_parse_source(const char *source,
                                                 struct Arazzo_Spec *spec);

/**
 * @brief Extracts Arazzo v1.1.0 workflows and metadata from a C source or
 * header file on disk.
 *
 * @param[in] path Path to the `.c` or `.h` file.
 * @param[in,out] spec Target Arazzo specification structure.
 * @return `CDD_C_SUCCESS` on success, or error code on I/O or allocation
 * failure.
 */
C_CDD_EXPORT cdd_c_error_t c2arazzo_parse_file(const char *path,
                                               struct Arazzo_Spec *spec);

/**
 * @brief Recursively scans a directory (or single file) of C code and populates
 * an `Arazzo_Spec`.
 *
 * @param[in] dir_path Directory or file path to scan.
 * @param[in,out] spec Target Arazzo specification structure.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t c2arazzo_parse_dir(const char *dir_path,
                                              struct Arazzo_Spec *spec);

/**
 * @brief CLI entry point for `cdd-c to_arazzo` and `cdd-c c2arazzo`.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t c2arazzo_cli_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C2ARAZZO_H */
