/**
 * @file simple.h
 * @brief Mock data structures and cleanup declarations in simple.h.
 */
#ifndef C_CDD_SRC_MOCKS_SIMPLE_H
#define C_CDD_SRC_MOCKS_SIMPLE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_c_error.h"
#include "cdd_src_mocks_export.h"
/* clang-format on */

/** @brief Mock structure Haz. */
struct Haz {
  /** @brief String member bzr. */
  char *bzr;
};

/**
 * @brief Releases memory owned by a Haz instance.
 * @param[in,out] haz Pointer to Haz instance to free.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT`.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t Haz_cleanup(struct Haz *haz);

/** @brief Mock structure Foo. */
struct Foo {
  /** @brief String member bar. */
  char *bar;
  /** @brief Integer member can. */
  int can;
  /** @brief Nested Haz pointer haz. */
  struct Haz *haz;
};

/**
 * @brief Releases memory owned by a Foo instance.
 * @param[in,out] foo Pointer to Foo instance to free.
 * @return `CDD_C_SUCCESS` on success, or error code from nested cleanup.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t Foo_cleanup(struct Foo *foo);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_SRC_MOCKS_SIMPLE_H */
