#ifndef C_CDD_TESTS_MOCKS_SIMPLE_H
#define C_CDD_TESTS_MOCKS_SIMPLE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "simple_mocks_export.h"

struct Haz {
  const char *bzr;
};

extern SIMPLE_MOCKS_EXPORT void Haz_cleanup(struct Haz *);

struct Foo {
  const char *bar;
  int can;
  struct Haz *haz;
};

extern SIMPLE_MOCKS_EXPORT void Foo_cleanup(struct Foo *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_TESTS_MOCKS_SIMPLE_H */
