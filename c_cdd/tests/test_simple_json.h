#ifndef TEST_SIMPLE_JSON_H
#define TEST_SIMPLE_JSON_H

#include <greatest.h>

#include "simple.h"

TEST test_simple_cleanup_and_null(void) {
  Haz_cleanup(NULL); /* Safe to call on NULL */
  Foo_cleanup(NULL);

  {
    struct Haz *hz = malloc(sizeof(*hz));
    if (hz) {
      hz->bzr = "hello";
      Haz_cleanup(hz);
    }
  }

  {
    struct Foo *foo = calloc(sizeof(*foo), 1);
    if (foo) {
      foo->bar = NULL;
      foo->can = 0;
      foo->haz = calloc(sizeof(*foo->haz), 1);
      if (foo->haz)
        Foo_cleanup(foo); /* This will free haz and foo */
      else
        free(foo);
    }
  }

  PASS();
}

SUITE(simple_mocks_suite) { RUN_TEST(test_simple_cleanup_and_null); }

#endif /* !TEST_SIMPLE_JSON_H */
