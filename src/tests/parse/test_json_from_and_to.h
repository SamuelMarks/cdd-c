extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_json_from_and_to.h
 * @brief Unit tests for JSON to_json and from_json mock functions.
 */

#ifndef TEST_JSON_FROM_AND_TO_H
#define TEST_JSON_FROM_AND_TO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <mocks/emit/simple_json.h>

#include <greatest.h>
/* clang-format on */

/**
 * @brief Tests basic from_str and to_str functionality for Tank enum.
 *
 * @return The result of the test.
 */
TEST test_enum_tank_to_str_and_from_str(void) {
  char *str = NULL;
  enum Tank tank_val;

  int rc = Tank_to_str(Tank_BIG, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("BIG", str);
  free(str);

  rc = Tank_from_str("SMALL", &tank_val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(Tank_SMALL, tank_val);

  rc = Tank_from_str("INVALID", &tank_val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(Tank_UNKNOWN, tank_val);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests struct HazE to_json/from_json roundtrip.
 *
 * @return The result of the test.
 */
TEST test_HazE_to_json_and_from_json(void) {
  struct HazE haz = {"example", Tank_BIG};
  char *json_str = NULL;
  struct HazE *haz_out = NULL;

  int rc = HazE_to_json(&haz, &json_str);
  ASSERT_EQ(0, rc);
  ASSERT(json_str != NULL);

  rc = HazE_from_json(json_str, &haz_out);
  ASSERT_EQ(0, rc);
  ASSERT(haz_out != NULL);

  ASSERT(HazE_eq(&haz, haz_out) == 0);

  HazE_cleanup(haz_out);
  free(json_str);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests struct FooE to_json/from_json roundtrip with null member.
 *
 * @return The result of the test.
 */
TEST test_FooE_to_json_and_from_json_with_null_haz(void) {
  struct FooE foo = {"barval", 42, NULL};
  char *json_str = NULL;
  struct FooE *foo_out = NULL;

  int rc = FooE_to_json(&foo, &json_str);
  ASSERT_EQ(0, rc);
  ASSERT(json_str != NULL);

  rc = FooE_from_json(json_str, &foo_out);
  ASSERT_EQ(0, rc);
  ASSERT(foo_out != NULL);

  ASSERT(FooE_eq(&foo, foo_out) == 0);

  FooE_cleanup(foo_out);
  free(json_str);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests struct FooE to_json/from_json roundtrip with non-null member.
 *
 * @return The result of the test.
 */
TEST test_FooE_to_json_and_from_json_non_null_haz(void) {
  struct HazE haz_in = {"bzr_data_here", Tank_BIG};
  struct FooE foo_in = {"bar_data_here", 777, NULL};
  char *json_str = NULL;
  struct FooE *foo_out = NULL;
  int rc;

  foo_in.haz = &haz_in;

  rc = FooE_to_json(&foo_in, &json_str);
  ASSERT_EQ(0, rc);
  ASSERT(json_str != NULL);

  rc = FooE_from_json(json_str, &foo_out);
  ASSERT_EQ(0, rc);
  ASSERT(foo_out != NULL);

  ASSERT(FooE_eq(&foo_in, foo_out) == 0);

  FooE_cleanup(foo_out);
  free(json_str);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief JSON mock serialization test suite.
 */
SUITE(json_from_and_to_suite) {
  RUN_TEST(test_enum_tank_to_str_and_from_str);

  RUN_TEST(test_HazE_to_json_and_from_json);

  RUN_TEST(test_FooE_to_json_and_from_json_with_null_haz);

  RUN_TEST(test_FooE_to_json_and_from_json_non_null_haz);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_JSON_FROM_AND_TO_H */
