#ifndef TEST_DATACLASSES_H
#define TEST_DATACLASSES_H

#include <greatest.h>

#include <mocks/simple_json.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#else
#include <sys/errno.h>
#endif

TEST test_FooE_default_deepcopy_eq_cleanup(void) {
  struct FooE *foo0 = NULL;
  struct FooE *foo1 = NULL;
  struct FooE *foo2 = NULL;
  int rc;

  rc = FooE_default(&foo0);
  ASSERT_EQ_FMT(0, rc, "%d");
  ASSERT(foo0 != NULL);

  rc = FooE_deepcopy(foo0, &foo1);
  ASSERT_EQ_FMT(0, rc, "%d");
  ASSERT(foo1 != NULL);

  ASSERT(FooE_eq(foo0, foo1));

  foo0->can = 53;
  ASSERT(!FooE_eq(foo0, foo1));

  rc = FooE_deepcopy(NULL, &foo2);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(NULL, foo2);

  FooE_cleanup(foo0);
  FooE_cleanup(foo1);
  FooE_cleanup(foo2);

  PASS();
}

TEST test_HazE_default_deepcopy_eq_cleanup(void) {
  struct HazE *h0 = NULL;
  struct HazE *h1 = NULL;
  struct HazE *h2 = NULL;
  int rc;

  rc = HazE_default(&h0);
  ASSERT_EQ_FMT(0, rc, "%d");
  ASSERT(h0 != NULL);

  rc = HazE_deepcopy(h0, &h1);
  ASSERT_EQ_FMT(0, rc, "%d");
  ASSERT(h1 != NULL);

  ASSERT(HazE_eq(h0, h1));

  h0->tank = (h0->tank == BIG) ? SMALL : BIG;
  ASSERT(!HazE_eq(h0, h1));

  rc = HazE_deepcopy(NULL, &h2);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(NULL, h2);

  HazE_cleanup(h0);
  HazE_cleanup(h1);
  HazE_cleanup(h2);

  PASS();
}

TEST test_json_roundtrip_and_errors(void) {
  const char *const bad_json = "{ \"bar\": \"hello\" "; /* Incomplete */
  const char *const haz_json_no_tank = "{\"bzr\": \"bzrval\"}";
  struct FooE *foo_out = NULL;
  struct HazE *haz_out = NULL;

  ASSERT_EQ(EINVAL, FooE_from_json(bad_json, &foo_out));
  ASSERT_EQ(NULL, foo_out);

  ASSERT_EQ(EINVAL, FooE_from_json(NULL, &foo_out));
  ASSERT_EQ(EINVAL, FooE_from_json("{}", NULL));

  ASSERT_EQ(EINVAL, HazE_from_json(haz_json_no_tank, &haz_out));
  ASSERT_EQ(NULL, haz_out);

  PASS();
}

/* Test FooE from_json / to_json roundtrip */
TEST test_FooE_json_roundtrip(void) {
  const char *const json = "{\"bar\": \"hello\", \"can\": 42, \"haz\": "
                           "{\"bzr\": \"bzrval\", \"tank\": \"SMALL\"}}";
  struct FooE *foo_in = NULL;
  char *json_out = NULL;
  int rc;

  rc = FooE_from_json(json, &foo_in);
  ASSERT_EQ_FMT(0, rc, "%d");
  ASSERT(foo_in != NULL);

  rc = FooE_to_json(foo_in, &json_out);
  ASSERT_EQ_FMT(0, rc, "%d");
  ASSERT(json_out != NULL);

  /* Deserialize again */
  {
    struct FooE *foo_out = NULL;
    rc = FooE_from_json(json_out, &foo_out);
    ASSERT_EQ_FMT(0, rc, "%d");
    ASSERT(foo_out != NULL);

    ASSERT(FooE_eq(foo_in, foo_out));

    FooE_cleanup(foo_out);
  }

  free(json_out);
  FooE_cleanup(foo_in);

  PASS();
}

/* Test HazE from_json / to_json roundtrip */
TEST test_HazE_json_roundtrip(void) {
  const char *const json = "{\"bzr\": \"bzrval\", \"tank\": \"BIG\"}";
  struct HazE *haz_in = NULL;
  char *json_out = NULL;
  int rc;

  rc = HazE_from_json(json, &haz_in);
  if (rc != 0 || haz_in == NULL)
    FAIL();

  rc = HazE_to_json(haz_in, &json_out);
  if (rc != 0 || json_out == NULL) {
    HazE_cleanup(haz_in);
    FAIL();
  }

  {
    struct HazE *haz_out = NULL;
    rc = HazE_from_json(json_out, &haz_out);
    if (rc != 0 || haz_out == NULL) {
      free(json_out);
      HazE_cleanup(haz_in);
      FAIL();
    }

    ASSERT(HazE_eq(haz_in, haz_out));

    HazE_cleanup(haz_out);
  }

  free(json_out);
  HazE_cleanup(haz_in);

  PASS();
}

/* Test enum Tank to_str/from_str */
TEST test_Tank_to_str_from_str(void) {
  char *str = NULL;
  enum Tank val;
  int rc;

  rc = Tank_to_str(BIG, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("BIG", str);
  free(str);
  str = NULL;

  rc = Tank_to_str(SMALL, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("SMALL", str);
  free(str);
  str = NULL;

  rc = Tank_to_str(-42, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("UNKNOWN", str);
  free(str);
  str = NULL;

  rc = Tank_from_str("BIG", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(BIG, val);

  rc = Tank_from_str(NULL, &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(UNKNOWN, val);

  rc = Tank_from_str("foo", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(UNKNOWN, val);

  ASSERT_EQ(EINVAL, Tank_from_str("BIG", NULL));

  PASS();
}

/* Test debug and display functions for FooE and HazE */
TEST test_debug_and_display(void) {
  struct FooE *foo = NULL;
  struct HazE *haz = NULL;
  int rc;
  rc = FooE_default(&foo);
  ASSERT_EQ(0, rc);
  rc = HazE_default(&haz);
  ASSERT_EQ(0, rc);

  rc = FooE_debug(foo, stdout);
  ASSERT_EQ(0, rc);
  rc = FooE_debug(NULL, stdout);
  ASSERT_EQ(0, rc);

  rc = FooE_display(foo, stdout);
  ASSERT_EQ(0, rc);

  rc = HazE_debug(haz, stdout);
  ASSERT_EQ(0, rc);

  FooE_cleanup(foo);
  HazE_cleanup(haz);

  PASS();
}

TEST test_eq_null_cases(void) {
  struct FooE *f1 = NULL, *f2 = NULL;

  FooE_default(&f1);
  FooE_default(&f2);

  ASSERT(FooE_eq(NULL, NULL));
  ASSERT(!FooE_eq(f1, NULL));
  ASSERT(!FooE_eq(NULL, f1));

  /* Test one string is NULL, other is not */
  free((void *)f1->bar);
  f1->bar = NULL;
  f2->bar = strdup("not null");
  ASSERT(!FooE_eq(f1, f2));
  /* And the other way */
  f1->bar = strdup("not null");
  free((void *)f2->bar);
  f2->bar = NULL;
  ASSERT(!FooE_eq(f1, f2));

  FooE_cleanup(f1);
  FooE_cleanup(f2);
  PASS();
}

/* Test cleanup handles NULL gracefully */
TEST test_cleanup_null(void) {
  FooE_cleanup(NULL);
  HazE_cleanup(NULL);

  PASS();
}

SUITE(dataclasses_suite) {
  RUN_TEST(test_FooE_default_deepcopy_eq_cleanup);
  RUN_TEST(test_HazE_default_deepcopy_eq_cleanup);
  RUN_TEST(test_FooE_json_roundtrip);
  RUN_TEST(test_HazE_json_roundtrip);
  RUN_TEST(test_json_roundtrip_and_errors);
  RUN_TEST(test_Tank_to_str_from_str);
  RUN_TEST(test_debug_and_display);
  RUN_TEST(test_cleanup_null);
  RUN_TEST(test_eq_null_cases);
}

#endif /* TEST_DATACLASSES_H */
