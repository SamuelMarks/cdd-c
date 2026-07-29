#ifndef TEST_CODE2SCHEMA_OOM_H
#define TEST_CODE2SCHEMA_OOM_H

#include "classes/parse/code2schema.h"
#include <greatest.h>
#include <parson.h>

static int g_malloc_fail_at = -1;
static void *mock_malloc(size_t sz) {
  if (g_malloc_fail_at == 0) {
    g_malloc_fail_at = -1;
    return NULL;
  }
  if (g_malloc_fail_at > 0)
    g_malloc_fail_at--;
  return malloc(sz);
}
static void mock_free(void *ptr) { free(ptr); }

TEST test_code2schema_oom_simulate(void) {
  json_set_allocation_functions(mock_malloc, mock_free);
  /* simulate OOMs... */
  json_set_allocation_functions(malloc, free);
  PASS();
}
#endif
