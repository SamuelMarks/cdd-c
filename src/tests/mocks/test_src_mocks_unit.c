/**
 * @file test_src_mocks_unit.c
 * @brief Exhaustive `greatest.h` + OOM + IO fault-injection runner for
 * `src/mocks/emit/simple.c` and `src/mocks/emit/simple_json.c`.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/suppress_gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/json_dom.h"
#include "c_cdd/memory.h"
#include "cdd_c_error.h"
#include "cdd_src_mocks_export.h"
#include "cdd_test_helpers/cdd_crt_bridge.h"
#include "../../mocks/emit/simple.h"
#include "../../mocks/emit/simple_json.h"
#include <greatest.h>
/* clang-format on */

/** @brief External test variable `g_simple_json_fail_alloc`. */
extern CDD_SRC_MOCKS_EXPORT int g_simple_json_fail_alloc;
/** @brief External test variable `g_simple_json_fail_io`. */
extern CDD_SRC_MOCKS_EXPORT int g_simple_json_fail_io;

/**
 * @brief Opens a scratch file for display/debug serialization tests.
 * @return Opened `FILE *` stream or NULL.
 */
static FILE *open_src_mocks_tmp(void) {
  FILE *f = NULL;
#if defined(_MSC_VER)
  if (fopen_s(&f, "src_mocks_tmp.txt", "w+") != 0)
    f = NULL;
#else
  f = fopen("src_mocks_tmp.txt", "w+");
#endif
  return f;
}

/**
 * @brief Tests `Haz_cleanup` and `Foo_cleanup` in `src/mocks/emit/simple.c`.
 * @return Greatest test result.
 */
TEST test_src_mocks_simple_cleanup(void) {
  struct Foo *f1;
  struct Foo *f2;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, Haz_cleanup(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, Foo_cleanup(NULL));

  f1 = (struct Foo *)calloc(1, sizeof(*f1));
  ASSERT(f1 != NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, Foo_cleanup(f1));
  free(f1);

  f2 = (struct Foo *)calloc(1, sizeof(*f2));
  ASSERT(f2 != NULL);
  f2->haz = (struct Haz *)calloc(1, sizeof(*f2->haz));
  ASSERT(f2->haz != NULL);
  ASSERT_EQ(CDD_C_SUCCESS, Foo_cleanup(f2));

  PASS();
}

/**
 * @brief Tests 100% of functions, lines, OOM paths, and IO failure paths in
 * `src/mocks/emit/simple_json.c`.
 * @return Greatest test result.
 */
TEST test_src_mocks_simple_json_exhaustive(void) {
  struct FooE *foo_e = NULL;
  struct FooE *foo_e2 = NULL;
  struct HazE *haz_e = NULL;
  struct HazE *haz_e2 = NULL;
  char *json = NULL;
  char *str = NULL;
  enum Tank t = Tank_UNKNOWN;
  FILE *f = NULL;
  int i;
  cdd_c_error_t rc;

  g_simple_json_fail_alloc = 0;
  g_simple_json_fail_io = 0;

  /* Tank helpers */
  ASSERT_EQ(CDD_C_SUCCESS, Tank_default(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, Tank_default(&t));
  ASSERT_EQ(Tank_BIG, t);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, Tank_to_str(Tank_BIG, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, Tank_to_str(Tank_BIG, &str));
  free(str);
  ASSERT_EQ(CDD_C_SUCCESS, Tank_to_str(Tank_SMALL, &str));
  free(str);
  ASSERT_EQ(CDD_C_SUCCESS, Tank_to_str(Tank_UNKNOWN, &str));
  free(str);
  ASSERT_EQ(CDD_C_SUCCESS, Tank_to_str((enum Tank)99, &str));
  free(str);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, Tank_from_str("BIG", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, Tank_from_str(NULL, &t));
  ASSERT_EQ(Tank_UNKNOWN, t);
  ASSERT_EQ(CDD_C_SUCCESS, Tank_from_str("BIG", &t));
  ASSERT_EQ(Tank_BIG, t);
  ASSERT_EQ(CDD_C_SUCCESS, Tank_from_str("SMALL", &t));
  ASSERT_EQ(Tank_SMALL, t);
  ASSERT_EQ(CDD_C_SUCCESS, Tank_from_str("UNKNOWN", &t));
  ASSERT_EQ(Tank_UNKNOWN, t);
  ASSERT_EQ(CDD_C_SUCCESS, Tank_from_str("OTHER", &t));
  ASSERT_EQ(Tank_UNKNOWN, t);

  /* HazE lifecycle, eq, json, display, debug */
  ASSERT_EQ(CDD_C_SUCCESS, HazE_cleanup(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_default(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, HazE_default(&haz_e));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_deepcopy(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, HazE_deepcopy(NULL, &haz_e2));
  ASSERT_EQ(NULL, haz_e2);
  ASSERT_EQ(CDD_C_SUCCESS, HazE_deepcopy(haz_e, &haz_e2));
  ASSERT_EQ(CDD_C_SUCCESS, HazE_eq(NULL, NULL));
  ASSERT_NEQ(CDD_C_SUCCESS, HazE_eq(haz_e, NULL));
  ASSERT_NEQ(CDD_C_SUCCESS, HazE_eq(NULL, haz_e));
  ASSERT_EQ(CDD_C_SUCCESS, HazE_eq(haz_e, haz_e2));
  haz_e2->tank = Tank_SMALL;
  ASSERT_NEQ(CDD_C_SUCCESS, HazE_eq(haz_e, haz_e2));
  haz_e2->tank = Tank_BIG;
  haz_e2->bzr = C_CDD_RAW_STRDUP("diff");
  ASSERT_NEQ(CDD_C_SUCCESS, HazE_eq(haz_e, haz_e2));
  ASSERT_NEQ(CDD_C_SUCCESS, HazE_eq(haz_e2, haz_e));
  haz_e->bzr = C_CDD_RAW_STRDUP("other");
  ASSERT_NEQ(CDD_C_SUCCESS, HazE_eq(haz_e, haz_e2));
  free(haz_e->bzr);
  haz_e->bzr = C_CDD_RAW_STRDUP("diff");
  ASSERT_EQ(CDD_C_SUCCESS, HazE_eq(haz_e, haz_e2));
  ASSERT_EQ(CDD_C_SUCCESS, HazE_cleanup(haz_e2));
  haz_e2 = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_to_json(haz_e, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, HazE_to_json(NULL, &json));
  free(json);
  json = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, HazE_to_json(haz_e, &json));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_from_json(json, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_from_json(NULL, &haz_e2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_from_json("{bad", &haz_e2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_from_json("[]", &haz_e2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_from_json("{}", &haz_e2));
  ASSERT_EQ(CDD_C_SUCCESS, HazE_from_json(json, &haz_e2));
  ASSERT_EQ(CDD_C_SUCCESS, HazE_cleanup(haz_e2));
  haz_e2 = NULL;
  free(json);
  json = NULL;

  free(haz_e->bzr);
  haz_e->bzr = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, HazE_to_json(haz_e, &json));
  free(json);
  json = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, HazE_from_json("{\"tank\":\"BIG\"}", &haz_e2));
  ASSERT_EQ(CDD_C_SUCCESS, HazE_cleanup(haz_e2));
  haz_e2 = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_from_jsonObject(NULL, &haz_e2));
  {
    JSON_Value *v = NULL;
    JSON_Object *vo = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("{\"tank\":\"BIG\"}", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_object(v, &vo));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_from_jsonObject(vo, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  f = open_src_mocks_tmp();
  ASSERT(f != NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_display(haz_e, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, HazE_debug(haz_e, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, HazE_display(haz_e, f));
  ASSERT_EQ(CDD_C_SUCCESS, HazE_debug(NULL, f));
  ASSERT_EQ(CDD_C_SUCCESS, HazE_debug(haz_e, f));
  haz_e->bzr = C_CDD_RAW_STRDUP("");
  ASSERT_EQ(CDD_C_SUCCESS, HazE_debug(haz_e, f));
  free(haz_e->bzr);
  haz_e->bzr = C_CDD_RAW_STRDUP("txt");
  ASSERT_EQ(CDD_C_SUCCESS, HazE_debug(haz_e, f));

  g_simple_json_fail_io = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, HazE_display(haz_e, f));
  g_simple_json_fail_io = 2;
  ASSERT_EQ(CDD_C_SUCCESS, HazE_debug(NULL, f));
  g_simple_json_fail_io = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, HazE_debug(NULL, f));
  g_simple_json_fail_io = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, HazE_debug(haz_e, f));

  /* FooE lifecycle, eq, json, display, debug */
  ASSERT_EQ(CDD_C_SUCCESS, FooE_cleanup(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_default(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_default(&foo_e));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_deepcopy(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_deepcopy(NULL, &foo_e2));
  ASSERT_EQ(NULL, foo_e2);
  ASSERT_EQ(CDD_C_SUCCESS, FooE_deepcopy(foo_e, &foo_e2));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_eq(NULL, NULL));
  ASSERT_NEQ(CDD_C_SUCCESS, FooE_eq(foo_e, NULL));
  ASSERT_NEQ(CDD_C_SUCCESS, FooE_eq(NULL, foo_e));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_eq(foo_e, foo_e2));
  foo_e2->can = 99;
  ASSERT_NEQ(CDD_C_SUCCESS, FooE_eq(foo_e, foo_e2));
  foo_e2->can = foo_e->can;
  foo_e2->bar = C_CDD_RAW_STRDUP("b");
  ASSERT_NEQ(CDD_C_SUCCESS, FooE_eq(foo_e, foo_e2));
  foo_e->bar = C_CDD_RAW_STRDUP("b");
  foo_e2->haz->tank = Tank_SMALL;
  ASSERT_NEQ(CDD_C_SUCCESS, FooE_eq(foo_e, foo_e2));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_cleanup(foo_e2));
  foo_e2 = NULL;

  /* FooE_deepcopy with haz == NULL */
  {
    struct FooE no_haz;
    memset(&no_haz, 0, sizeof(no_haz));
    no_haz.can = 3;
    ASSERT_EQ(CDD_C_SUCCESS, FooE_deepcopy(&no_haz, &foo_e2));
    ASSERT_EQ(CDD_C_SUCCESS, FooE_cleanup(foo_e2));
    foo_e2 = NULL;
  }

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_to_json(foo_e, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_to_json(NULL, &json));
  free(json);
  json = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, FooE_to_json(foo_e, &json));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_from_json(json, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_from_json(NULL, &foo_e2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_from_json("{bad", &foo_e2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_from_json("[]", &foo_e2));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_from_json("{}", &foo_e2));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_cleanup(foo_e2));
  foo_e2 = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            FooE_from_json("{\"haz\":{}}", &foo_e2));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_from_json(json, &foo_e2));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_cleanup(foo_e2));
  foo_e2 = NULL;
  free(json);
  json = NULL;

  free(foo_e->bar);
  foo_e->bar = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, FooE_to_json(foo_e, &json));
  free(json);
  json = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_from_jsonObject(NULL, &foo_e2));
  {
    JSON_Value *v = NULL;
    JSON_Object *vo = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("{\"can\":1}", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_object(v, &vo));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_from_jsonObject(vo, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_display(foo_e, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, FooE_debug(foo_e, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_display(foo_e, f));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_debug(NULL, f));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_debug(foo_e, f));
  foo_e->bar = C_CDD_RAW_STRDUP("bar");
  ASSERT_EQ(CDD_C_SUCCESS, FooE_debug(foo_e, f));

  g_simple_json_fail_io = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, FooE_display(foo_e, f));
  g_simple_json_fail_io = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, FooE_debug(NULL, f));
  g_simple_json_fail_io = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, FooE_debug(foo_e, f));
  g_simple_json_fail_io = 2;
  ASSERT_EQ(CDD_C_ERROR_IO, FooE_debug(foo_e, f));
  g_simple_json_fail_io = 3;
  ASSERT_EQ(CDD_C_SUCCESS, FooE_debug(foo_e, f));
  g_simple_json_fail_io = 0;

  /* OOM sweeps across all allocations in simple_json.c */
  for (i = 1; i < 60; i++) {
    g_simple_json_fail_alloc = i;
    rc = Tank_to_str(Tank_BIG, &str);
    if (rc == CDD_C_SUCCESS) {
      free(str);
      str = NULL;
    }

    g_simple_json_fail_alloc = i;
    rc = HazE_default(&haz_e2);
    if (rc == CDD_C_SUCCESS) {
      rc = HazE_cleanup(haz_e2);
      haz_e2 = NULL;
    }

    g_simple_json_fail_alloc = i;
    rc = HazE_deepcopy(haz_e, &haz_e2);
    if (rc == CDD_C_SUCCESS) {
      rc = HazE_cleanup(haz_e2);
      haz_e2 = NULL;
    }

    g_simple_json_fail_alloc = i;
    rc = HazE_display(haz_e, f);

    g_simple_json_fail_alloc = i;
    rc = HazE_debug(haz_e, f);
    {
      struct HazE null_bzr_haz;
      null_bzr_haz.bzr = NULL;
      null_bzr_haz.tank = Tank_BIG;
      g_simple_json_fail_alloc = i;
      rc = HazE_debug(&null_bzr_haz, f);
    }

    g_simple_json_fail_alloc = i;
    rc = HazE_to_json(haz_e, &json);
    if (rc == CDD_C_SUCCESS) {
      free(json);
      json = NULL;
    }

    g_simple_json_fail_alloc = i;
    rc = HazE_to_json(NULL, &json);
    if (rc == CDD_C_SUCCESS) {
      free(json);
      json = NULL;
    }

    {
      JSON_Value *v = NULL;
      JSON_Object *vo = NULL;
      g_simple_json_fail_alloc = 0;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_parse_string("{\"tank\":\"BIG\",\"bzr\":\"x\"}", &v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_object(v, &vo));
      g_simple_json_fail_alloc = i;
      rc = HazE_from_jsonObject(vo, &haz_e2);
      if (rc == CDD_C_SUCCESS) {
        rc = HazE_cleanup(haz_e2);
        haz_e2 = NULL;
      }
      g_simple_json_fail_alloc = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    }

    g_simple_json_fail_alloc = i;
    rc = FooE_default(&foo_e2);
    if (rc == CDD_C_SUCCESS) {
      rc = FooE_cleanup(foo_e2);
      foo_e2 = NULL;
    }

    g_simple_json_fail_alloc = i;
    rc = FooE_deepcopy(foo_e, &foo_e2);
    if (rc == CDD_C_SUCCESS) {
      rc = FooE_cleanup(foo_e2);
      foo_e2 = NULL;
    }

    g_simple_json_fail_alloc = i;
    rc = FooE_display(foo_e, f);

    g_simple_json_fail_alloc = i;
    rc = FooE_debug(foo_e, f);

    g_simple_json_fail_alloc = i;
    rc = FooE_to_json(foo_e, &json);
    if (rc == CDD_C_SUCCESS) {
      free(json);
      json = NULL;
    }

    g_simple_json_fail_alloc = i;
    rc = FooE_to_json(NULL, &json);
    if (rc == CDD_C_SUCCESS) {
      free(json);
      json = NULL;
    }

    {
      JSON_Value *v = NULL;
      JSON_Object *vo = NULL;
      g_simple_json_fail_alloc = 0;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_parse_string("{\"can\":1,\"bar\":\"b\",\"haz\":{"
                                      "\"tank\":\"BIG\",\"bzr\":\"z\"}}",
                                      &v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_object(v, &vo));
      g_simple_json_fail_alloc = i;
      rc = FooE_from_jsonObject(vo, &foo_e2);
      if (rc == CDD_C_SUCCESS) {
        rc = FooE_cleanup(foo_e2);
        foo_e2 = NULL;
      }
      g_simple_json_fail_alloc = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    }

    g_simple_json_fail_alloc = i;
    rc = run_mocks_test();
  }
  g_simple_json_fail_alloc = 0;
  ASSERT_EQ(CDD_C_SUCCESS, run_mocks_test());

  fclose(f);
  remove("src_mocks_tmp.txt");
  ASSERT_EQ(CDD_C_SUCCESS, HazE_cleanup(haz_e));
  ASSERT_EQ(CDD_C_SUCCESS, FooE_cleanup(foo_e));

  PASS();
}

/**
 * @brief Suite for `src/mocks/`.
 */
SUITE(src_mocks_unit_suite) {
  RUN_TEST(test_src_mocks_simple_cleanup);
  RUN_TEST(test_src_mocks_simple_json_exhaustive);
}

/** @brief Greatest test runner global state definitions. */
GREATEST_MAIN_DEFS();

/**
 * @brief Entry point for `test_src_mocks_unit`.
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return Process exit status.
 */
int main(int argc, char **argv) {
  if (cdd_suppress_gui_popups() != CDD_C_SUCCESS)
    return EXIT_FAILURE;
  if (c_cdd_crt_init_runtime() != CDD_C_SUCCESS)
    return EXIT_FAILURE;
#if defined(_MSC_VER)
  json_set_allocation_functions(test_crt_malloc, c_cdd_crt_free);
#endif
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(src_mocks_unit_suite);
  GREATEST_MAIN_END();
}
