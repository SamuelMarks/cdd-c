/* clang-format off */
/* clang-format on */
#ifdef __cplusplus
extern "C" {
#endif
#ifndef CDD_C_ERROR_H
#define CDD_C_ERROR_H

/**
 * @brief Global error enumeration for cdd-c project.
 */
enum cdd_c_error {
  CDD_C_SUCCESS = 0,
  CDD_C_ERROR_MEMORY = 1,
  CDD_C_ERROR_INVALID_ARGUMENT = 2,
  CDD_C_ERROR_IO = 3,
  CDD_C_ERROR_SYSTEM = 4,
  CDD_C_ERROR_NOT_FOUND = 5,
  CDD_C_ERROR_PARSE = 6,
  CDD_C_ERROR_UNKNOWN = 7
};

#endif /* CDD_C_ERROR_H */

#ifdef __cplusplus
}
#endif /* __cplusplus */
