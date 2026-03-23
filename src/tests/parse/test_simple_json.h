/* clang-format off */
#ifndef TEST_SIMPLE_JSON_H
#define TEST_SIMPLE_JSON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* clang-format off */
#include <greatest.h>

#include "../mocks/emit/simple.h"
/* clang-format on */
/* clang-format on */

TEST test_simple_cleanup_and_null(void) {
  Haz_cleanup(NULL); /* Safe to call on NULL */
  Foo_cleanup(NULL);

  {
    struct Haz *hz = (struct Haz *)malloc(sizeof(*hz));
    if (hz) {
      hz->bzr = "hello";
      Haz_cleanup(hz);
    }
  }

  {
    struct Foo *foo = (struct Foo *)calloc(1, sizeof(*foo));
    if (foo) {
      foo->bar = NULL;
      foo->can = 0;
      foo->haz = (struct Haz *)calloc(1, sizeof(*foo->haz));
      if (foo->haz)
        Foo_cleanup(foo); /* This will free haz and foo */
      else
        free(foo);
    }
  }

  PASS();
}

TEST test_foo_cleanup_with_null_haz(void) {
  struct Foo *foo = (struct Foo *)calloc(1, sizeof(*foo));
  if (foo) {
    foo->bar = NULL;
    foo->can = 0;
    foo->haz = NULL;  /* Haz is null */
    Foo_cleanup(foo); /* Should call Haz_cleanup(NULL) */
  }
  PASS();
}

SUITE(simple_mocks_suite) {
  RUN_TEST(test_simple_cleanup_and_null);
  RUN_TEST(test_foo_cleanup_with_null_haz);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_SIMPLE_JSON_H */
