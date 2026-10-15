#ifndef C_CDD_TESTS_MOCKS_SIMPLE_H
#define C_CDD_TESTS_MOCKS_SIMPLE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_c_error.h"
#include "simple_mocks_export.h"
/* clang-format on */

struct Haz {
  char *bzr;
};

struct Foo {
  char *bar;
  int can;
  struct Haz *haz;
};

/**
 * @brief Releases memory owned by a Foo instance.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error Foo_cleanup(struct Foo *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_TESTS_MOCKS_SIMPLE_H */
