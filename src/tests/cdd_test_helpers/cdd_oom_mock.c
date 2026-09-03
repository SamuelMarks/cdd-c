/**
 * @file cdd_oom_mock.c
 * @brief Implementation of OOM mocking utilities.
 */

#define CDD_OOM_MOCK_IMPL

/* clang-format off */
#include "cdd_oom_mock.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int g_mock_oom_countdown = -1;

void mock_oom_reset_cb(void (*cb)(void)) {
  (void)cb;
  g_mock_oom_countdown = -1;
}

void mock_set_oom_after_calls(int calls) { g_mock_oom_countdown = calls; }

static int check_oom_countdown(void) {
  if (g_mock_oom_countdown >= 0) {
    if (g_mock_oom_countdown == 0) {
      g_mock_oom_countdown--;
      return 1; /* Trigger failure */
    }
    g_mock_oom_countdown--;
  }
  return 0;
}

void *mock_malloc(size_t size) {
  if (check_oom_countdown()) {
    return NULL;
  }
  return malloc(size);
}

void *mock_calloc(size_t num, size_t size) {
  if (check_oom_countdown()) {
    return NULL;
  }
  return calloc(num, size);
}

void *mock_realloc(void *ptr, size_t size) {
  if (check_oom_countdown()) {
    return NULL;
  }
  return realloc(ptr, size);
}

#if defined(_WIN32)
char *mock_strdup(const char *s) {
  if (check_oom_countdown()) {
    return NULL;
  }
  return _strdup(s);
}
#else
char *mock_strdup(const char *s) {
  if (check_oom_countdown()) {
    return NULL;
  }
  return strdup(s);
}
#endif
