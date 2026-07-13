#include "cdd_c_error.h"
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
/* LCOV_EXCL_START */
TEST test_simple_cleanup_and_null(void) {
  /* LCOV_EXCL_STOP */
  Haz_cleanup(NULL); /* Safe to call on NULL */
                     /* LCOV_EXCL_START */
  Foo_cleanup(NULL);
  HazE_cleanup(NULL);
  FooE_cleanup(NULL);
  /* LCOV_EXCL_STOP */

  {
    /* LCOV_EXCL_START */
    struct Haz *hz = (struct Haz *)malloc(sizeof(*hz));
    if (hz) {
      hz->bzr = strdup("hello");
      free((char *)hz->bzr);
      Haz_cleanup(hz);
      /* LCOV_EXCL_STOP */
    }
  }

  {
    /* LCOV_EXCL_START */
    struct Foo *foo = (struct Foo *)calloc(1, sizeof(*foo));
    if (foo) {
      foo->bar = NULL;
      foo->can = 0;
      foo->haz = (struct Haz *)calloc(1, sizeof(*foo->haz));
      if (foo->haz)
        /* LCOV_EXCL_STOP */
        Foo_cleanup(foo); /* This will free haz and foo */
      else
        /* LCOV_EXCL_START */
        free(foo);
      /* LCOV_EXCL_STOP */
    }
  }

  /* LCOV_EXCL_START */
  PASS();
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Tests foo cleanup.
 * @return TEST
 */
/* LCOV_EXCL_START */
TEST test_foo_cleanup_with_null_haz(void) {
  struct Foo *foo = (struct Foo *)calloc(1, sizeof(*foo));
  if (foo) {
    foo->bar = NULL;
    foo->can = 0;
    /* LCOV_EXCL_STOP */
    foo->haz = NULL;  /* Haz is null */
    Foo_cleanup(foo); /* Should call Haz_cleanup(NULL) */
  }
  /* LCOV_EXCL_START */
  PASS();
  /* LCOV_EXCL_STOP */
}

/**
 * @brief test json serialization
 * @return TEST
 */
/* LCOV_EXCL_START */
TEST test_foo_e_json(void) {
  struct FooE *foo_e = NULL;
  char *json = NULL;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ((int)CDD_C_SUCCESS, (int)FooE_default(&foo_e));
  foo_e->can = 42;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ((int)CDD_C_SUCCESS, (int)FooE_to_json(foo_e, &json));
  ASSERT(json != NULL);
  ASSERT(strstr(json, "\"can\": 42"));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  FooE_cleanup(foo_e);
  free(json);
  PASS();
  /* LCOV_EXCL_STOP */
}

#ifdef CDD_BUILD_TESTS
#ifndef CDD_BUILD_TESTS
#error "CDD_BUILD_TESTS is NOT defined!"
#endif
#include "simple_mocks_export.h"
extern SIMPLE_MOCKS_EXPORT int g_simple_json_fail_alloc;
#endif

/* LCOV_EXCL_START */
TEST test_foo_e_full_coverage(void) {
  struct FooE *foo_e = NULL;
  struct FooE *foo_e2 = NULL;
  struct HazE *haz_e = NULL;
  struct HazE *haz_e2 = NULL;
  char *json = NULL;
  char *str = NULL;
  /* LCOV_EXCL_STOP */
  enum Tank t;
  FILE *f;
#ifdef CDD_BUILD_TESTS
  int i;
  int rc;
#endif

  /* LCOV_EXCL_START */
  printf("DEBUG: test_foo_e_full_coverage started!\n");
  fflush(stdout);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_default(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_default(NULL));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, HazE_default(&haz_e));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_deepcopy(NULL, NULL));
  ASSERT_EQ(0, HazE_deepcopy(NULL, &haz_e2));
  ASSERT_EQ(NULL, haz_e2);
  ASSERT_EQ(0, HazE_deepcopy(haz_e, &haz_e2));
  ASSERT_EQ(0, HazE_eq(haz_e, haz_e2));
  ASSERT_EQ(0, HazE_eq(NULL, NULL));
  ASSERT_EQ(1, HazE_eq(haz_e, NULL));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, Tank_to_str(Tank_BIG, NULL));
  ASSERT_EQ(0, Tank_to_str(Tank_BIG, &str));
  free(str);
  ASSERT_EQ(0, Tank_to_str(Tank_SMALL, &str));
  free(str);
  ASSERT_EQ(0, Tank_to_str(Tank_UNKNOWN, &str));
  free(str);
  ASSERT_EQ(0, Tank_to_str((enum Tank)999, &str));
  free(str);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, Tank_from_str("BIG", NULL));
  ASSERT_EQ(0, Tank_from_str("BIG", &t));
  ASSERT_EQ(Tank_BIG, t);
  ASSERT_EQ(0, Tank_from_str("SMALL", &t));
  ASSERT_EQ(Tank_SMALL, t);
  ASSERT_EQ(0, Tank_from_str("UNKNOWN", &t));
  ASSERT_EQ(Tank_UNKNOWN, t);
  ASSERT_EQ(0, Tank_from_str("FOO", &t));
  ASSERT_EQ(Tank_UNKNOWN, t);
  ASSERT_EQ(0, Tank_from_str(NULL, &t));
  ASSERT_EQ(Tank_UNKNOWN, t);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, Tank_default(NULL));
  ASSERT_EQ(0, Tank_default(&t));
  ASSERT_EQ(Tank_BIG, t);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_to_json(haz_e, NULL));
  ASSERT_EQ(0, HazE_to_json(NULL, &json));
  ASSERT_STR_EQ("null", json);
  free(json);
  json = NULL;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, HazE_to_json(haz_e, &json));
  free(json);
  json = NULL;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  haz_e->bzr = strdup("test");
  ASSERT_EQ(0, HazE_to_json(haz_e, &json));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_from_json(json, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_from_json(NULL, &haz_e2));
  HazE_cleanup(haz_e2);
  ASSERT_EQ(0, HazE_from_json(json, &haz_e2));
  free(json);
  json = NULL;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_from_json("invalid", &haz_e2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_from_json("[]", &haz_e2));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  f = tmpfile();
  ASSERT(f);
  ASSERT(f);
  ASSERT_EQ(0, HazE_display(haz_e, f));
  ASSERT_EQ(0, HazE_debug(haz_e, f));
  ASSERT_EQ(0, HazE_debug(NULL, f));
  free((char *)haz_e->bzr);
  haz_e->bzr = NULL;
  ASSERT_EQ(0, HazE_debug(haz_e, f));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, FooE_default(&foo_e));
  printf("DEBUG: before load 1\n");
  fflush(stdout);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_deepcopy(NULL, NULL));
  printf("DEBUG: after load 1\n");
  fflush(stdout);
  ASSERT_EQ(0, FooE_deepcopy(NULL, &foo_e2));
  ASSERT_EQ(NULL, foo_e2);
  ASSERT_EQ(0, FooE_deepcopy(foo_e, &foo_e2));
  ASSERT_EQ(0, FooE_eq(foo_e, foo_e2));
  ASSERT_EQ(0, FooE_eq(NULL, NULL));
  ASSERT_EQ(1, FooE_eq(foo_e, NULL));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_to_json(foo_e, NULL));
  ASSERT_EQ(0, FooE_to_json(NULL, &json));
  ASSERT_STR_EQ("null", json);
  free(json);
  json = NULL;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, FooE_to_json(foo_e, &json));
  free(json);
  json = NULL;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  foo_e->bar = strdup("test");
  ASSERT_EQ(0, FooE_to_json(foo_e, &json));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_from_json(json, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_from_json(NULL, &foo_e2));
  FooE_cleanup(foo_e2);
  ASSERT_EQ(0, FooE_from_json(json, &foo_e2));
  free(json);
  json = NULL;
  printf("DEBUG: before FooE_from_json invalid\n");
  fflush(stdout);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_from_json("invalid", &foo_e2));
  printf("DEBUG: after FooE_from_json invalid\n");
  fflush(stdout);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_from_json("[]", &foo_e2));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT_EQ(0, FooE_display(foo_e, f));
  ASSERT_EQ(0, FooE_debug(foo_e, f));
  ASSERT_EQ(0, FooE_debug(NULL, f));
  free((char *)foo_e->bar);
  foo_e->bar = NULL;
  ASSERT_EQ(0, FooE_debug(foo_e, f));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  printf("DEBUG: FooE_cleanup(foo_e2) at %d\n", __LINE__);
  fflush(stdout);
  FooE_cleanup(foo_e2);
  foo_e2 = NULL;
  printf("DEBUG: FooE_cleanup(foo_e) at %d\n", __LINE__);
  fflush(stdout);
  FooE_cleanup(foo_e);
  foo_e = NULL;
  printf("DEBUG: HazE_cleanup(haz_e2) at %d\n", __LINE__);
  fflush(stdout);
  HazE_cleanup(haz_e2);
  /* LCOV_EXCL_STOP */
  /* Test HazE_from_jsonObject */
  /* LCOV_EXCL_START */
  HazE_cleanup(haz_e);
  haz_e = NULL;
  /* LCOV_EXCL_STOP */
  {
    /* LCOV_EXCL_START */
    JSON_Value *v = json_parse_string("{\"tank\": \"BIG\"}");
    ASSERT_EQ(0, HazE_from_jsonObject(json_value_get_object(v), &haz_e));
    HazE_cleanup(haz_e);
    haz_e = NULL;
    json_value_free(v);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_from_jsonObject(NULL, &haz_e));
    v = json_parse_string("{}");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              /* LCOV_EXCL_STOP */
              HazE_from_jsonObject(json_value_get_object(v), &haz_e));
    /* LCOV_EXCL_START */
    json_value_free(v);
    /* LCOV_EXCL_STOP */
  }

  /* Test FooE_from_jsonObject */
  /* LCOV_EXCL_START */
  FooE_cleanup(foo_e);
  foo_e = NULL;
  /* LCOV_EXCL_STOP */
  {
    /* LCOV_EXCL_START */
    JSON_Value *v = json_parse_string("{\"can\": 1}");
    ASSERT_EQ(0, FooE_from_jsonObject(json_value_get_object(v), &foo_e));
    FooE_cleanup(foo_e);
    json_value_free(v);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_from_jsonObject(NULL, &foo_e));
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  fclose(f);
  /* LCOV_EXCL_STOP */

#ifdef CDD_BUILD_TESTS
  /* LCOV_EXCL_START */
  for (i = 1; i < 150; i++) {
    g_simple_json_fail_alloc = i;
    rc = Tank_to_str(Tank_BIG, &str);
    if (str) {
      free(str);
      str = NULL;
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = i;
    rc = Tank_to_str(Tank_SMALL, &str);
    if (str) {
      free(str);
      str = NULL;
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = i;
    rc = Tank_to_str(Tank_UNKNOWN, &str);
    if (str) {
      free(str);
      str = NULL;
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = 0;
    HazE_default(&haz_e);
    g_simple_json_fail_alloc = 0;
    haz_e->bzr = malloc(4);
    strcpy((char *)haz_e->bzr, "foo");
    printf("DEBUG: haz_e->bzr = %p, literal foo = %p\n", (void *)haz_e->bzr,
           /* LCOV_EXCL_STOP */
           (void *)"foo");
    /* LCOV_EXCL_START */
    fflush(stdout);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = i;
    rc = HazE_default(&haz_e2);
    if (haz_e2) {
      HazE_cleanup(haz_e2);
      haz_e2 = NULL;
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = i;
    rc = HazE_deepcopy(haz_e, &haz_e2);
    if (haz_e2) {
      HazE_cleanup(haz_e2);
      haz_e2 = NULL;
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    free((void *)haz_e->bzr);
    haz_e->bzr = NULL;
    g_simple_json_fail_alloc = i;
    rc = HazE_deepcopy(haz_e, &haz_e2);
    if (haz_e2) {
      HazE_cleanup(haz_e2);
      haz_e2 = NULL;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = 0;
    haz_e->bzr = malloc(4);
    strcpy((char *)haz_e->bzr, "foo");
    printf("DEBUG: haz_e->bzr = %p, literal foo = %p\n", (void *)haz_e->bzr,
           /* LCOV_EXCL_STOP */
           (void *)"foo");
    /* LCOV_EXCL_START */
    fflush(stdout);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    f = tmpfile();
    ASSERT(f);
    g_simple_json_fail_alloc = i;
    HazE_display(haz_e, f);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = i;
    HazE_debug(haz_e, f);
    fclose(f);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = i;
    rc = HazE_to_json(haz_e, &json);
    if (json) {
      free(json);
      json = NULL;
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = i;
    rc = HazE_to_json(NULL, &json);
    if (json) {
      free(json);
      json = NULL;
      /* LCOV_EXCL_STOP */
    }

    {
      JSON_Value *v =
          /* LCOV_EXCL_START */
          json_parse_string("{\"tank\": \"BIG\", \"bzr\": \"test\"}");
      g_simple_json_fail_alloc = i;
      rc = HazE_from_jsonObject(json_value_get_object(v), &haz_e2);
      if (haz_e2) {
        HazE_cleanup(haz_e2);
        haz_e2 = NULL;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
      json_value_free(v);
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = 0;
    FooE_default(&foo_e);
    foo_e->bar = malloc(5);
    strcpy((char *)foo_e->bar, "test");
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = i;
    rc = FooE_default(&foo_e2);
    if (foo_e2) {
      FooE_cleanup(foo_e2);
      foo_e2 = NULL;
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = i;
    rc = FooE_deepcopy(foo_e, &foo_e2);
    if (foo_e2) {
      FooE_cleanup(foo_e2);
      foo_e2 = NULL;
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    f = tmpfile();
    ASSERT(f);
    g_simple_json_fail_alloc = i;
    FooE_display(foo_e, f);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = i;
    FooE_debug(foo_e, f);
    fclose(f);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = i;
    rc = FooE_to_json(foo_e, &json);
    if (json) {
      free(json);
      json = NULL;
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    g_simple_json_fail_alloc = i;
    rc = FooE_to_json(NULL, &json);
    if (json) {
      free(json);
      json = NULL;
      /* LCOV_EXCL_STOP */
    }

    {
      /* LCOV_EXCL_START */
      JSON_Value *v = json_parse_string(
          /* LCOV_EXCL_STOP */
          "{\"can\": 1, \"bar\": \"test\", \"haz\": {\"tank\": \"BIG\"}}");
      /* LCOV_EXCL_START */
      g_simple_json_fail_alloc = i;
      rc = FooE_from_jsonObject(json_value_get_object(v), &foo_e2);
      if (foo_e2) {
        FooE_cleanup(foo_e2);
        foo_e2 = NULL;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_START */
      json_value_free(v);
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    HazE_cleanup(haz_e);
    haz_e = NULL;
    FooE_cleanup(foo_e);
    foo_e = NULL;
    /* LCOV_EXCL_STOP */
  }
  (void)rc;
  /* LCOV_EXCL_START */
  g_simple_json_fail_alloc = 0;
/* LCOV_EXCL_STOP */
#endif

  /* LCOV_EXCL_START */
  PASS();
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Suite for simple mocks
 */
/* LCOV_EXCL_START */
SUITE(simple_mocks_suite) {
  RUN_TEST(test_simple_cleanup_and_null);
  RUN_TEST(test_foo_cleanup_with_null_haz);
  RUN_TEST(test_foo_e_json);
  RUN_TEST(test_foo_e_full_coverage);
}
/* LCOV_EXCL_STOP */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_SIMPLE_JSON_H */
