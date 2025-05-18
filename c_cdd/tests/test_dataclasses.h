#ifndef TEST_DATACLASSES_H
#define TEST_DATACLASSES_H

#include <mocks/simple_json.h>

#include <greatest.h>

TEST test_FooE_default_deepcopy_eq_cleanup(void) {
  struct FooE *foo0 = NULL;
  struct FooE *foo1 = NULL;
  int rc;

  rc = FooE_default(&foo0);
  if (rc != 0 || foo0 == NULL)
    FAIL();

  rc = FooE_deepcopy(foo0, &foo1);
  if (rc != 0 || foo1 == NULL) {
    FooE_cleanup(foo0);
    FAIL();
  }

  puts("foo0");
  FooE_display(foo0, stdout);
  puts("##########");
  puts("foo1");
  FooE_display(foo1, stdout);
  puts("----------");
  ASSERT(FooE_eq(foo0, foo1));

  foo0->can = 53;
  ASSERT(!FooE_eq(foo0, foo1));

  FooE_cleanup(foo0);
  FooE_cleanup(foo1);

  PASS();
}

TEST test_HazE_default_deepcopy_eq_cleanup(void) {
  struct HazE *h0 = NULL;
  struct HazE *h1 = NULL;
  int rc;

  rc = HazE_default(&h0);
  if (rc != 0 || h0 == NULL)
    FAIL();

  rc = HazE_deepcopy(h0, &h1);
  if (rc != 0 || h1 == NULL) {
    HazE_cleanup(h0);
    FAIL();
  }

  ASSERT(HazE_eq(h0, h1));

  /* Modifying h0 invalidates equality */
  h0->tank = (h0->tank == BIG) ? SMALL : BIG;
  ASSERT(!HazE_eq(h0, h1));

  HazE_cleanup(h0);
  HazE_cleanup(h1);

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
  if (rc != 0 || foo_in == NULL)
    FAIL();

  rc = FooE_to_json(foo_in, &json_out);
  if (rc != 0 || json_out == NULL) {
    FooE_cleanup(foo_in);
    FAIL();
  }

  /* Deserialize again */
  {
    struct FooE *foo_out = NULL;
    rc = FooE_from_json(json_out, &foo_out);
    if (rc != 0 || foo_out == NULL) {
      free(json_out);
      FooE_cleanup(foo_in);
      FAIL();
    }

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

  rc = Tank_to_str(SMALL, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("SMALL", str);
  free(str);

  /* Unknown value */
  rc = Tank_to_str(-42, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("UNKNOWN", str);
  free(str);

  rc = Tank_from_str("BIG", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(BIG, val);

  rc = Tank_from_str("SMALL", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(SMALL, val);

  rc = Tank_from_str("foo", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(UNKNOWN, val);

  rc = Tank_from_str(NULL, &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(UNKNOWN, val);

  rc = Tank_from_str("UNKNOWN", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(UNKNOWN, val);

  PASS();
}

/* Test debug and display functions for FooE and HazE */
TEST test_debug_and_display(void) {
  struct FooE *foo = NULL;
  struct HazE *haz = NULL;
  int rc;
  rc = FooE_default(&foo);
  if (rc != 0 || foo == NULL)
    FAIL();

  rc = HazE_default(&haz);
  if (rc != 0 || haz == NULL) {
    FooE_cleanup(foo);
    FAIL();
  }

  /* Redirect stdout to buffer or use FILE* memory buffer for testing?
     For simplicity, just test the functions do not return errors */

  rc = FooE_debug(foo, stdout);
  ASSERT_EQ(0, rc);

  rc = FooE_display(foo, stdout);
  ASSERT_EQ(0, rc);

  rc = HazE_debug(haz, stdout);
  ASSERT_EQ(0, rc);

  /*rc = HazE_display(haz, stdout);
  ASSERT_EQ(0, rc);*/

  FooE_cleanup(foo);
  HazE_cleanup(haz);

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
  RUN_TEST(test_Tank_to_str_from_str);
  RUN_TEST(test_debug_and_display);
  RUN_TEST(test_cleanup_null);
}

#endif /* TEST_DATACLASSES_H */
