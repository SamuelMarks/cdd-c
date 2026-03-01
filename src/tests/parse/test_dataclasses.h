#ifndef TEST_DATACLASSES_H
#define TEST_DATACLASSES_H

#include <greatest.h>
#include <mocks/emit/simple_json.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
#else
#include <sys/errno.h>
#endif

#include "cdd_test_helpers/cdd_helpers.h"

/*
 * Mocks for strict recursive testing (Linked List).
 * Ideally these would be generated, but for testing the generator logic we
 * often verify against pre-generated code or mock logic that follows the
 * generated pattern. Here we define a new struct Node to test recursive
 * patterns specifically.
 */

struct Node {
  int value;
  struct Node *next;
};

/* Emulate Generated Code for Node to test the logic pattern */
void Node_cleanup(struct Node *const obj) {
  if (obj == NULL)
    return;
  if (obj->next) {
    Node_cleanup(obj->next);
    obj->next = NULL;
  }
  free(obj);
}

int Node_deepcopy(const struct Node *src, struct Node **dest) {
  if (!dest)
    return EINVAL;
  if (!src) {
    *dest = NULL;
    return 0;
  }

  *dest = malloc(sizeof(**dest));
  if (*dest == NULL)
    return ENOMEM;
  memset(*dest, 0, sizeof(**dest));

  (*dest)->value = src->value;

  if (src->next) {
    int rc = Node_deepcopy(src->next, &(*dest)->next);
    if (rc != 0) {
      Node_cleanup(*dest);
      *dest = NULL;
      return rc;
    }
  } else {
    (*dest)->next = NULL;
  }
  return 0;
}

int Node_eq(const struct Node *a, const struct Node *b) {
  if (a == NULL || b == NULL)
    return (a == b);
  if (a->value != b->value)
    return 0;
  return Node_eq(a->next, b->next);
}

/* --- Tests --- */

TEST test_recursive_cleanup(void) {
  struct Node *head = malloc(sizeof(struct Node));
  struct Node *next = malloc(sizeof(struct Node));
  ASSERT(head && next);

  head->value = 1;
  head->next = next;
  next->value = 2;
  next->next = NULL;

  /* Should recursively free 'next' without crashing */
  Node_cleanup(head);
  PASS();
}

TEST test_recursive_deepcopy(void) {
  struct Node *head = malloc(sizeof(struct Node));
  struct Node *next = malloc(sizeof(struct Node));
  struct Node *copy = NULL;
  int rc;

  ASSERT(head && next);
  head->value = 10;
  head->next = next;
  next->value = 20;
  next->next = NULL;

  rc = Node_deepcopy(head, &copy);
  ASSERT_EQ(0, rc);
  ASSERT(copy != NULL);
  ASSERT(copy != head);
  ASSERT_EQ(10, copy->value);
  ASSERT(copy->next != NULL);
  ASSERT(copy->next != next);
  ASSERT_EQ(20, copy->next->value);

  Node_cleanup(head);
  Node_cleanup(copy);
  PASS();
}

TEST test_recursive_eq(void) {
  struct Node *n1 = malloc(sizeof(struct Node));
  struct Node *n2 = malloc(sizeof(struct Node));
  struct Node n1_next, n2_next;

  n1->value = 1;
  n1->next = &n1_next;
  n2->value = 1;
  n2->next = &n2_next;

  n1_next.value = 2;
  n1_next.next = NULL;
  n2_next.value = 2;
  n2_next.next = NULL;

  ASSERT(Node_eq(n1, n2));

  n2_next.value = 3;
  ASSERT(!Node_eq(n1, n2));

  free(n1);
  free(n2);
  PASS();
}

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

  h0->tank = (h0->tank == Tank_BIG) ? Tank_SMALL : Tank_BIG;
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
  struct HazE h = {"", Tank_BIG};
  struct FooE f = {"", 1, NULL};
  struct HazE *haz_e_ptr = &h;
  struct FooE *foo_e_ptr = &f;
  f.haz = haz_e_ptr;

  ASSERT_EQ(EINVAL, Tank_to_str(Tank_BIG, NULL));
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
  FILE *fh;

  write_to_file(tmp_fname, "content");

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    const errno_t err = fopen_s(&fh, tmp_fname, "r, ccs=UTF-8");
    if (err != 0 || fh == NULL) {
      FAILm("Failed to read file");
    }
  }
#else
  fh = fopen(tmp_fname, "r");
  ASSERT(fh != NULL);
#endif

  FooE_default(&foo);
  rc = FooE_display(foo, fh); /* Try to write to read-only stream */
  ASSERT(rc != 0);
  FooE_cleanup(foo);

  HazE_default(&haz);
  rc = HazE_display(haz, fh);
  ASSERT(rc != 0);
  HazE_cleanup(haz);

  fclose(fh);
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

  rc = Tank_to_str(Tank_BIG, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("BIG", str);
  free(str);
  str = NULL;

  rc = Tank_to_str(Tank_SMALL, &str);
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
  ASSERT_EQ(Tank_BIG, val);

  rc = Tank_from_str("SMALL", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(Tank_SMALL, val);

  rc = Tank_from_str(NULL, &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(Tank_UNKNOWN, val);

  rc = Tank_from_str("UNKNOWN", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(Tank_UNKNOWN, val);

  rc = Tank_from_str("foo", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(Tank_UNKNOWN, val);

  ASSERT_EQ(EINVAL, Tank_from_str("BIG", NULL));

  PASS();
}

TEST test_cleanup_null(void) {
  FooE_cleanup(NULL);
  HazE_cleanup(NULL);
  PASS();
}

TEST test_to_json_with_null_fields(void) {
  struct HazE haz = {NULL, Tank_BIG};
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

TEST test_debug_fail(void) {
  struct FooE *foo = NULL;
  struct HazE *haz = NULL;
  int rc;
  const char *const tmp_fname = "debug_test.tmp";
  FILE *fh;

  write_to_file(tmp_fname, "content");

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    const errno_t err = fopen_s(&fh, tmp_fname, "r, ccs=UTF-8");
    if (err != 0 || fh == NULL) {
      FAILm("Failed to read file");
    }
  }
#else
  fh = fopen(tmp_fname, "r");
  ASSERT(fh != NULL);
#endif

  FooE_default(&foo);
  rc = FooE_debug(foo, fh); /* Try to write to read-only stream */
  ASSERT(rc != 0);
  FooE_cleanup(foo);

  rewind(fh); /* reset for next test */

  HazE_default(&haz);
  rc = HazE_debug(haz, fh); /* Try to write to read-only stream */
  ASSERT(rc != 0);
  HazE_cleanup(haz);

  fclose(fh);
  remove(tmp_fname);
  PASS();
}

TEST test_json_parsing_wrong_types(void) {
  struct FooE *f = NULL;
  struct HazE *h = NULL;

  /* tank is not a string */
  ASSERT_EQ(EINVAL, HazE_from_json("{\"bzr\": \"v\", \"tank\": 123}", &h));

  /* bar is not a string */
  ASSERT_EQ(0, FooE_from_json("{\"bar\": 123, \"can\": 1, \"haz\": {\"bzr\": "
                              "\"v\", \"tank\": \"BIG\"}}",
                              &f));
  ASSERT(f->bar == NULL);
  FooE_cleanup(f);
  f = NULL;

  /* can is not a number */
  ASSERT_EQ(0,
            FooE_from_json("{\"bar\": \"v\", \"can\": \"notanumber\", \"haz\": "
                           "{\"bzr\": \"v\", \"tank\": \"BIG\"}}",
                           &f));
  ASSERT(f->can == 0);
  FooE_cleanup(f);
  f = NULL;

  /* haz is not an object */
  ASSERT_EQ(0,
            FooE_from_json("{\"bar\": \"v\", \"can\": 1, \"haz\": 123}", &f));
  ASSERT(f->haz == NULL);
  FooE_cleanup(f);

  PASS();
}

TEST test_deepcopy_null_fields(void) {
  struct HazE haz_in = {NULL, Tank_BIG};
  struct HazE *haz_out = NULL;
  struct FooE foo_in = {NULL, 42, NULL};
  struct FooE *foo_out = NULL;
  int rc;

  /* Deepcopy HazE with NULL bzr */
  rc = HazE_deepcopy(&haz_in, &haz_out);
  ASSERT_EQ(0, rc);
  ASSERT(haz_out != NULL);
  ASSERT(haz_out->bzr == NULL);
  ASSERT(haz_out->tank == Tank_BIG);
  HazE_cleanup(haz_out);

  /* Deepcopy FooE with NULL bar and NULL haz */
  rc = FooE_deepcopy(&foo_in, &foo_out);
  ASSERT_EQ(0, rc);
  ASSERT(foo_out != NULL);
  ASSERT(foo_out->bar == NULL);
  ASSERT(foo_out->can == 42);
  ASSERT(foo_out->haz == NULL);
  FooE_cleanup(foo_out);

  PASS();
}

TEST test_json_parsing_missing_fields(void) {
  struct FooE *f = NULL;
  int rc;

  /* `bar` is optional and can be missing */
  rc = FooE_from_json(
      "{\"can\": 1, \"haz\": {\"bzr\": \"v\", \"tank\": \"BIG\"}}", &f);
  ASSERT_EQ(0, rc);
  ASSERT(f != NULL);
  ASSERT(f->bar == NULL);
  FooE_cleanup(f);
  f = NULL;

  /* `haz` being missing (null) is fine */
  rc = FooE_from_json("{\"bar\": \"v\", \"can\": 1}", &f);
  ASSERT_EQ(0, rc);
  ASSERT(f != NULL);
  ASSERT(f->haz == NULL);
  FooE_cleanup(f);
  f = NULL;

  /* `can` optional and can be missing */
  rc = FooE_from_json(
      "{\"bar\": \"v\", \"haz\": {\"bzr\": \"v\", \"tank\": \"BIG\"}}", &f);
  ASSERT_EQ(0, rc);
  ASSERT(f != NULL);
  ASSERT(f->can == 0);
  FooE_cleanup(f);

  PASS();
}

TEST test_debug_with_null_nested(void) {
  struct FooE *f = NULL;
  int rc;

  rc = FooE_from_json("{\"bar\": \"v\", \"can\": 1, \"haz\": null}", &f);
  ASSERT_EQ(0, rc);
  ASSERT(f != NULL);

  rc = FooE_debug(f, stdout);
  ASSERT_EQ(0, rc);

  FooE_cleanup(f);
  PASS();
}

TEST test_debug_with_empty_strings(void) {
  struct HazE haz = {"", Tank_SMALL};
  struct FooE foo = {"", 0, NULL};
  FILE *tmp = tmpfile();
  ASSERT(tmp != NULL);
  foo.haz = &haz;

  ASSERT_EQ(0, HazE_debug(&haz, tmp));
  ASSERT_EQ(0, FooE_debug(&foo, tmp));

  fclose(tmp);
  PASS();
}

TEST test_HazE_deepcopy_alloc_fail(void) {
  struct HazE haz_in = {"test", Tank_BIG};
  struct HazE *haz_out = NULL;

  const int rc = HazE_deepcopy(&haz_in, &haz_out);
  if (rc == ENOMEM) {
    ASSERT_EQ(NULL, haz_out);
  } else {
    ASSERT_EQ(0, rc);
    ASSERT(haz_out != NULL);
    HazE_cleanup(haz_out);
  }
  PASS();
}

TEST test_simple_json_HazE_more_eq_cases(void) {
  struct HazE *h1 = NULL, *h2 = NULL;
  HazE_default(&h1);
  HazE_default(&h2);

  /* Test bzr member inequality */
  free((void *)h1->bzr);
  h1->bzr = strdup("abc");
  free((void *)h2->bzr);
  h2->bzr = strdup("def");
  ASSERT(!HazE_eq(h1, h2));

  /* Test one-sided null bzr */
  free((void *)h1->bzr);
  h1->bzr = NULL;
  ASSERT(!HazE_eq(h1, h2));

  free((void *)h2->bzr);
  h2->bzr = strdup("abc");
  ASSERT(!HazE_eq(h2, h1));

  HazE_cleanup(h1);
  HazE_cleanup(h2);
  PASS();
}

TEST test_simple_json_more_eq_cases(void) {
  struct FooE *f1 = NULL, *f2 = NULL;
  FooE_default(&f1);
  FooE_default(&f2);

  /* Test can member inequality */
  f1->can = 1;
  f2->can = 2;
  ASSERT(!FooE_eq(f1, f2));
  f2->can = 1;
  ASSERT(FooE_eq(f1, f2)); /* Back to equal */

  /* Test one-sided null haz */
  HazE_cleanup(f1->haz);
  f1->haz = NULL;
  ASSERT(!FooE_eq(f1, f2));

  FooE_cleanup(f1);
  FooE_cleanup(f2);
  PASS();
}

TEST test_FooE_eq_nested_diff(void) {
  struct FooE *f1 = NULL, *f2 = NULL;
  FooE_default(&f1);
  FooE_default(&f2);

  /* Test haz member inequality */
  f1->haz->tank = Tank_BIG;
  f2->haz->tank = Tank_SMALL;
  ASSERT(!FooE_eq(f1, f2));

  f2->haz->tank = Tank_BIG;
  ASSERT(FooE_eq(f1, f2));

  FooE_cleanup(f1);
  FooE_cleanup(f2);
  PASS();
}

SUITE(dataclasses_suite) {
  RUN_TEST(test_recursive_cleanup);
  RUN_TEST(test_recursive_deepcopy);
  RUN_TEST(test_recursive_eq);
  RUN_TEST(test_FooE_default_deepcopy_eq_cleanup);
  RUN_TEST(test_HazE_default_deepcopy_eq_cleanup);
  RUN_TEST(test_FooE_json_roundtrip);
  RUN_TEST(test_HazE_json_roundtrip);
  RUN_TEST(test_json_parsing_errors);
  RUN_TEST(test_null_args_and_errors);
  RUN_TEST(test_json_parsing_corner_cases);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  /* TODO: Get them to work on MSVC */
#else
  RUN_TEST(test_display_fail);
  RUN_TEST(test_debug_fail);
#endif
  RUN_TEST(test_debug_and_display);
  RUN_TEST(test_eq_null_cases);
  RUN_TEST(test_Tank_to_str_from_str);
  RUN_TEST(test_cleanup_null);
  RUN_TEST(test_to_json_with_null_fields);
  RUN_TEST(test_json_parsing_wrong_types);
  RUN_TEST(test_deepcopy_null_fields);
  RUN_TEST(test_json_parsing_missing_fields);
  RUN_TEST(test_debug_with_null_nested);
  RUN_TEST(test_debug_with_empty_strings);
  RUN_TEST(test_HazE_deepcopy_alloc_fail);
  RUN_TEST(test_simple_json_more_eq_cases);
  RUN_TEST(test_simple_json_HazE_more_eq_cases);
  RUN_TEST(test_FooE_eq_nested_diff);
}

#endif /* TEST_DATACLASSES_H */
