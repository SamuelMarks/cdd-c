/**
 * @file test_simple_json.h
 * @brief Unit tests for simple json parsing/generation logic.
 */

#ifndef TEST_SIMPLE_JSON_H
#define TEST_SIMPLE_JSON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>

#include "../mocks/emit/simple.h"
#include "../mocks/emit/simple_json.h"
/* clang-format on */

/**
 * @brief Tests basic null handling of cleanup.
 * @return TEST
 */
TEST test_simple_cleanup_and_null(void) {
  Haz_cleanup(NULL); /* Safe to call on NULL */
  Foo_cleanup(NULL);
  HazE_cleanup(NULL);
  FooE_cleanup(NULL);

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

/**
 * @brief Tests foo cleanup.
 * @return TEST
 */
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

/**
 * @brief test json serialization
 * @return TEST
 */
TEST test_foo_e_json(void) {
  struct FooE *foo_e = NULL;
  char *json = NULL;

  ASSERT_EQ(0, FooE_default(&foo_e));
  foo_e->can = 42;

  ASSERT_EQ(0, FooE_to_json(foo_e, &json));
  ASSERT(json != NULL);
  ASSERT(strstr(json, "\"can\": 42"));

  FooE_cleanup(foo_e);
  free(json);
  PASS();
}

/**
 * @brief Suite for simple mocks
 */
SUITE(simple_mocks_suite) {
  RUN_TEST(test_simple_cleanup_and_null);
  RUN_TEST(test_foo_cleanup_with_null_haz);
  RUN_TEST(test_foo_e_json);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_SIMPLE_JSON_H */
