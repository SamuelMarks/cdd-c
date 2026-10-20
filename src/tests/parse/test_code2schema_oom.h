/**
 * @file test_code2schema_oom.h
 * @brief Unit tests and mocks in test_code2schema_oom.h.
 */
#ifndef TEST_CODE2SCHEMA_OOM_H
#define TEST_CODE2SCHEMA_OOM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "classes/parse/code2schema.h"
#include <greatest.h>
#include <parson.h>
/* clang-format on */

/** @brief Test definition g_malloc_fail_at. */
static int g_malloc_fail_at = -1;
/**
 * @brief Helper function mock_malloc.
 * @param[in] sz Parameter sz.
 * @return Result status or value.
 */
static void *mock_malloc(size_t sz) {
  if (g_malloc_fail_at == 0) {
    g_malloc_fail_at = -1;
    return NULL;
  }
  if (g_malloc_fail_at > 0)
    g_malloc_fail_at--;
  return malloc(sz);
}
/**
 * @brief Helper function mock_free.
 * @param[in,out] ptr Parameter ptr.
 */
static void mock_free(void *ptr) { free(ptr); }

/**
 * @brief Unit test test_code2schema_oom_simulate.
 * @return Greatest test result.
 */
TEST test_code2schema_oom_simulate(void) {
  json_set_allocation_functions(mock_malloc, mock_free);
  /* simulate OOMs... */
  json_set_allocation_functions(malloc, free);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
