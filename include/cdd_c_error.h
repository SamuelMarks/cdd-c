/**
 * @file cdd_c_error.h
 * @brief Core cdd c error definitions and implementation.
 */
#ifndef CDD_C_ERROR_H
#define CDD_C_ERROR_H

/* clang-format off */
#include "c_cdd/no_discard.h"
#ifndef C_CDD_EXPORT
#include "c_cdd_export.h"
#endif
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * @brief Global error enumeration for cdd-c project.
 */
typedef enum NO_DISCARD_TYPE cdd_c_error {
  /** @brief Force signed underlying integer type. */
  CDD_C_ERROR_FORCE_SIGNED = -1,
  /** @brief Operation succeeded. */
  CDD_C_SUCCESS = 0,
  /** @brief Out of memory. */
  CDD_C_ERROR_MEMORY = 1,
  /** @brief Invalid argument passed to function. */
  CDD_C_ERROR_INVALID_ARGUMENT = 2,
  /** @brief Input/output failure. */
  CDD_C_ERROR_IO = 3,
  /** @brief System or OS-level failure. */
  CDD_C_ERROR_SYSTEM = 4,
  /** @brief Requested item was not found. */
  CDD_C_ERROR_NOT_FOUND = 5,
  /** @brief Parsing failed. */
  CDD_C_ERROR_PARSE = 6,
  /** @brief Unknown or unspecified error. */
  CDD_C_ERROR_UNKNOWN = 7
} cdd_c_error_t NO_DISCARD_TYPEDEF;

/** @brief Alias for unsupported specification features or selectors. */
#define CDD_C_ERROR_NOT_SUPPORTED CDD_C_ERROR_UNKNOWN

/** @brief Scratch error code storing the status of the most recent operation */
extern C_CDD_EXPORT cdd_c_error_t g_cdd_last_alloc_err;

/**
 * @brief Converts a cdd_c_error_t to a string.
 *
 * @param[in] err The error code.
 * @param[out] out The output string pointer.
 * @return cdd_c_error_t CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT
 * otherwise.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_c_strerror(cdd_c_error_t err,
                                                     const char **out);

/**
 * @brief Preserves the first non-success error code across primary and cleanup
 * steps.
 *
 * @param[in] primary Primary operation status code.
 * @param[in] cleanup_rc Cleanup operation status code.
 * @return `primary` when `primary != CDD_C_SUCCESS`, otherwise `cleanup_rc`.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_c_keep_err(cdd_c_error_t primary,
                                                     cdd_c_error_t cleanup_rc);

#ifdef CDD_BUILD_TESTS
/** @brief Countdown counter for step-level error-percolation fault injection in
 * tests. */
extern C_CDD_EXPORT int g_cdd_step_fail;
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_C_ERROR_H */
