/**
 * @file simple.h
 * @brief Unit tests and mocks in simple.h.
 */
#ifndef C_CDD_TESTS_MOCKS_SIMPLE_H
#define C_CDD_TESTS_MOCKS_SIMPLE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_c_error.h"
#include "simple_mocks_export.h"
/* clang-format on */

/** @brief Test definition Haz. */
struct Haz {
  /** @brief Test definition bzr. */
  char *bzr;
};

/**
 * @brief Releases memory owned by a Haz instance.
 * @param[in,out] haz Parameter haz.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error Haz_cleanup(struct Haz *haz);

/** @brief Test definition Foo. */
struct Foo {
  /** @brief Test definition bar. */
  char *bar;
  /** @brief Test definition can. */
  int can;
  /** @brief Test definition haz. */
  struct Haz *haz;
};

/**
 * @brief Releases memory owned by a Foo instance.
 * @param[in,out] foo Parameter foo.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error Foo_cleanup(struct Foo *foo);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_TESTS_MOCKS_SIMPLE_H */
