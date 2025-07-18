#ifndef TEST_SIMPLE_JSON_H
#define TEST_SIMPLE_JSON_H

#include <greatest.h>

#include "simple.h"

TEST test_simple_cleanup_and_null(void) {
  Haz_cleanup(NULL); /* Safe to call on NULL */
  Foo_cleanup(NULL);

  {
    struct Haz *hz = malloc(sizeof(*hz));
    ASSERT(hz != NULL);
    hz->bzr = "hello";
    Haz_cleanup(hz);
  }

  {
    struct Foo *foo = calloc(sizeof(*foo), 1);
    ASSERT(foo != NULL);
    foo->bar = NULL;
    foo->can = 0;
    foo->haz = calloc(sizeof(*foo->haz), 1);
    ASSERT(foo->haz != NULL);
    Foo_cleanup(foo); /* This will free haz and foo */
  }

  PASS();
}

SUITE(simple_mocks_suite) { RUN_TEST(test_simple_cleanup_and_null); }

#endif /* !TEST_SIMPLE_JSON_H */
