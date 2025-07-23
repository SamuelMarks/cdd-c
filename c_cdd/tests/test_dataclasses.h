#ifndef TEST_DATACLASSES_H
#define TEST_DATACLASSES_H

#include <greatest.h>
#include <mocks/simple_json.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#else
#include <sys/errno.h>
#endif

TEST test_FooE_default_deepcopy_eq_cleanup(void) {
  struct FooE *foo0 = NULL, *foo1 = NULL, *foo2 = NULL;
  int rc;

  rc = FooE_default(&foo0);
  ASSERT_EQ(0, rc);
  ASSERT(foo0 != NULL);

  rc = FooE_deepcopy(foo0, &foo1);
  ASSERT_EQ(0, rc);
  ASSERT(foo1 != NULL);

  ASSERT(FooE_eq(foo0, foo1));

  foo0->can = 53;
  ASSERT(!FooE_eq(foo0, foo1));

  rc = FooE_deepcopy(NULL, &foo2);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(NULL, foo2);

  FooE_cleanup(foo0);
  FooE_cleanup(foo1);
  PASS();
}

TEST test_HazE_default_deepcopy_eq_cleanup(void) {
  struct HazE *h0 = NULL, *h1 = NULL, *h2 = NULL;
  int rc;

  rc = HazE_default(&h0);
  ASSERT_EQ(0, rc);
  ASSERT(h0 != NULL);

  rc = HazE_deepcopy(h0, &h1);
  ASSERT_EQ(0, rc);
  ASSERT(h1 != NULL);

  ASSERT(HazE_eq(h0, h1));

  h0->tank = (h0->tank == BIG) ? SMALL : BIG;
  ASSERT(!HazE_eq(h0, h1));

  rc = HazE_deepcopy(NULL, &h2);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(NULL, h2);

  HazE_cleanup(h0);
  HazE_cleanup(h1);
  PASS();
}

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

TEST test_HazE_json_roundtrip(void) {
  const char *const json = "{\"bzr\": \"bzrval\", \"tank\": \"BIG\"}";
  struct HazE *haz_in = NULL;
  char *json_out = NULL;
  int rc;

  rc = HazE_from_json(json, &haz_in);
  ASSERT_EQ(0, rc);
  ASSERT(haz_in != NULL);
  rc = HazE_to_json(haz_in, &json_out);
  ASSERT_EQ(0, rc);
  ASSERT(json_out != NULL);
  {
    struct HazE *haz_out = NULL;
    rc = HazE_from_json(json_out, &haz_out);
    ASSERT_EQ(0, rc);
    ASSERT(haz_out != NULL);
    ASSERT(HazE_eq(haz_in, haz_out));
    HazE_cleanup(haz_out);
  }
  free(json_out);
  HazE_cleanup(haz_in);
  PASS();
}

TEST test_json_parsing_errors(void) {
  struct HazE *h = NULL;
  struct FooE *f = NULL;

  ASSERT_EQ(EINVAL, HazE_from_json("{", &h));
  ASSERT_EQ(EINVAL, FooE_from_json("{", &f));
  ASSERT_EQ(EINVAL, HazE_from_json("[]", &h));
  ASSERT_EQ(EINVAL, FooE_from_json("[]", &f));

  ASSERT_EQ(EINVAL, HazE_from_json("{\"bzr\": \"val\"}", &h));

  ASSERT_EQ(
      0, FooE_from_json(
             "{\"can\": 1, \"haz\": {\"bzr\": \"v\", \"tank\": \"BIG\"}}", &f));
  FooE_cleanup(f);
  f = NULL;

  ASSERT_EQ(0,
            FooE_from_json("{\"bar\": \"v\", \"can\": 1, \"haz\": null}", &f));
  FooE_cleanup(f);
  PASS();
}

TEST test_json_parsing_corner_cases(void) {
  struct HazE *h = NULL;
  struct FooE *f = NULL;
  int rc;

  /* Test HazE from JSON with missing "tank" field */
  rc = HazE_from_json("{\"bzr\": \"val\"}", &h);
  ASSERT_EQ(EINVAL, rc);
  ASSERT_EQ(NULL, h);

  /* Test FooE from JSON with haz being an invalid object (missing "tank") */
  rc = FooE_from_json("{\"bar\": \"v\", \"can\": 1, \"haz\": {\"bzr\": \"v\"}}",
                      &f);
  ASSERT_EQ(EINVAL, rc);
  ASSERT_EQ(NULL, f);

  /* Test FooE where bar is NULL */
  rc = FooE_from_json("{\"bar\": null, \"can\": 1, \"haz\": {\"bzr\": \"v\", "
                      "\"tank\": \"BIG\"}}",
                      &f);
  ASSERT_EQ(0, rc);
  ASSERT(f != NULL);
  ASSERT(f->bar == NULL);
  FooE_cleanup(f);

  PASS();
}

TEST test_null_args_and_errors(void) {
  char *str = NULL;
  struct HazE h = {"", BIG};
  struct FooE f = {"", 1, NULL};
  struct HazE *haz_e_ptr = &h;
  struct FooE *foo_e_ptr = &f;
  f.haz = haz_e_ptr;

  ASSERT_EQ(EINVAL, Tank_to_str(BIG, NULL));
  ASSERT_EQ(EINVAL, Tank_from_str("BIG", NULL));

  ASSERT_EQ(EINVAL, HazE_to_json(haz_e_ptr, NULL));
  ASSERT_EQ(0, HazE_to_json(NULL, &str));
  ASSERT_STR_EQ("null", str);
  free(str);
  str = NULL;

  ASSERT_EQ(EINVAL, FooE_to_json(foo_e_ptr, NULL));
  ASSERT_EQ(0, FooE_to_json(NULL, &str));
  ASSERT_STR_EQ("null", str);
  free(str);
  str = NULL;

  ASSERT_EQ(EINVAL, HazE_from_json("{}", NULL));
  ASSERT_EQ(EINVAL, FooE_from_json("{}", NULL));
  ASSERT_EQ(EINVAL, HazE_from_json(NULL, &haz_e_ptr));
  ASSERT_EQ(EINVAL, FooE_from_json(NULL, &foo_e_ptr));

  ASSERT_EQ(EINVAL, FooE_default(NULL));
  ASSERT_EQ(EINVAL, HazE_default(NULL));

  ASSERT_EQ(EINVAL, FooE_deepcopy(foo_e_ptr, NULL));
  ASSERT_EQ(EINVAL, HazE_deepcopy(haz_e_ptr, NULL));
  PASS();
}

TEST test_debug_and_display(void) {
  struct HazE *haz = NULL;
  struct FooE *foo = NULL;
  FooE_default(&foo);
  ASSERT_EQ(0, FooE_debug(foo, stdout));
  ASSERT_EQ(0, FooE_debug(NULL, stdout));
  ASSERT_EQ(0, FooE_display(foo, stdout));

  HazE_default(&haz);
  ASSERT_EQ(0, HazE_debug(haz, stdout));
  ASSERT_EQ(0, HazE_debug(NULL, stdout));
  ASSERT_EQ(0, HazE_display(haz, stdout));

  FooE_cleanup(foo);
  HazE_cleanup(haz);
  PASS();
}

TEST test_display_fail(void) {
  struct FooE *foo = NULL;
  struct HazE *haz = NULL;
  int rc;
  const char *const tmp_fname = "display_test.tmp";
  FILE *fh_w, *fh_r;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  errno_t err = fopen_s(&fh_w, tmp_fname, "w");
  if (err != 0 || fh_w == NULL) {
    fprintf(stderr, "Failed to open for writing %s\n", tmp_fname);
    free(fh_w);
    return;
  }
#else
  fh_w = fopen(tmp_fname, "w");
  ASSERT(fh_w != NULL);
#endif
  fputs("content", fh_w);
  fclose(fh_w);

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fh_r, tmp_fname, "r");
    if (err != 0 || fh_r == NULL) {
      fprintf(stderr, "Failed to open for reading %s\n", tmp_fname);
      free(fh_w);
      return;
    }
  }
#else
  fh_r = fopen(tmp_fname, "r");
  ASSERT(fh_r != NULL);
#endif
  ASSERT(fh_r != NULL);

  FooE_default(&foo);
  rc = FooE_display(foo, fh_r); /* Try to write to read-only stream */
  ASSERT(rc != 0);
  FooE_cleanup(foo);

  HazE_default(&haz);
  /*rc = HazE_display(haz, fh_r);*/
  ASSERT(rc != 0);
  HazE_cleanup(haz);

  fclose(fh_r);
  remove(tmp_fname);

  PASS();
}

TEST test_eq_null_cases(void) {
  struct FooE *f1 = NULL, *f2 = NULL;
  struct HazE *h1 = NULL, *h2 = NULL;

  FooE_default(&f1);
  HazE_default(&h1);
  FooE_default(&f2);
  HazE_default(&h2);

  ASSERT(FooE_eq(NULL, NULL));
  ASSERT(!FooE_eq(f1, NULL));
  ASSERT(!FooE_eq(NULL, f1));

  ASSERT(HazE_eq(NULL, NULL));
  ASSERT(!HazE_eq(h1, NULL));
  ASSERT(!HazE_eq(NULL, h1));

  free((void *)f1->bar);
  f1->bar = NULL;
  f2->bar = strdup("not null");
  ASSERT(!FooE_eq(f1, f2));

  free((void *)f2->bar);
  f2->bar = NULL;
  ASSERT(FooE_eq(f1, f2)); /* Both bars are null */

  FooE_cleanup(f1);
  FooE_cleanup(f2);
  HazE_cleanup(h1);
  HazE_cleanup(h2);
  PASS();
}

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

  rc = Tank_from_str("UNKNOWN", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(UNKNOWN, val);

  rc = Tank_from_str("foo", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(UNKNOWN, val);

  ASSERT_EQ(EINVAL, Tank_from_str("BIG", NULL));

  PASS();
}

TEST test_cleanup_null(void) {
  FooE_cleanup(NULL);
  HazE_cleanup(NULL);
  PASS();
}

TEST test_to_json_with_null_fields(void) {
  struct HazE haz = {NULL, BIG};
  struct FooE foo = {NULL, 12, NULL};
  char *json_out = NULL;
  int rc;

  foo.haz = &haz;

  rc = HazE_to_json(&haz, &json_out);
  ASSERT_EQ(0, rc);
  {
    JSON_Value *val = json_parse_string(json_out);
    JSON_Object *obj = json_value_get_object(val);
    ASSERT(obj != NULL);
    ASSERT_EQ(JSONNull, json_value_get_type(json_object_get_value(obj, "bzr")));
    ASSERT_STR_EQ("BIG", json_object_get_string(obj, "tank"));
    json_value_free(val);
  }
  free(json_out);
  json_out = NULL;

  rc = FooE_to_json(&foo, &json_out);
  ASSERT_EQ(0, rc);
  {
    JSON_Value *val = json_parse_string(json_out);
    JSON_Object *obj = json_value_get_object(val);
    ASSERT(obj != NULL);
    ASSERT_EQ(JSONNull, json_value_get_type(json_object_get_value(obj, "bar")));
    ASSERT_EQ(12, (int)json_object_get_number(obj, "can"));
    {
      JSON_Object *haz_obj = json_object_get_object(obj, "haz");
      ASSERT(haz_obj != NULL);
      ASSERT_EQ(JSONNull,
                json_value_get_type(json_object_get_value(haz_obj, "bzr")));
      ASSERT_STR_EQ("BIG", json_object_get_string(haz_obj, "tank"));
    }
    json_value_free(val);
  }
  free(json_out);
  json_out = NULL;

  PASS();
}

SUITE(dataclasses_suite) {
  RUN_TEST(test_FooE_default_deepcopy_eq_cleanup);
  RUN_TEST(test_HazE_default_deepcopy_eq_cleanup);
  RUN_TEST(test_FooE_json_roundtrip);
  RUN_TEST(test_HazE_json_roundtrip);
  RUN_TEST(test_json_parsing_errors);
  RUN_TEST(test_null_args_and_errors);
  RUN_TEST(test_json_parsing_corner_cases);
  RUN_TEST(test_display_fail);
  RUN_TEST(test_debug_and_display);
  RUN_TEST(test_eq_null_cases);
  RUN_TEST(test_Tank_to_str_from_str);
  RUN_TEST(test_cleanup_null);
  RUN_TEST(test_to_json_with_null_fields);
}

#endif /* TEST_DATACLASSES_H */
