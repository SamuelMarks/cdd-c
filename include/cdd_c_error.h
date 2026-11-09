/**
 * @file cdd_c_error.h
 * @brief Core cdd c error definitions and implementation.
 */
/* clang-format off */
#include "c_cdd/no_discard.h"
#include "c_cdd_export.h"
/* clang-format on */
#ifndef CDD_C_ERROR_H
#define CDD_C_ERROR_H

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
} cdd_c_error_t;

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
                                                     char **out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_C_ERROR_H */
