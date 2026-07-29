/* clang-format off */
#include "c_cdd/memory.h"
#include <stdio.h>
/* clang-format on */
int g_cdd_alloc_fail_countdown = 0;

int g_cdd_alloc_fail_countdown_countdown = 0;

void *test_malloc(size_t size) {
  if (g_cdd_alloc_fail_countdown_countdown > 0) {
    if (--g_cdd_alloc_fail_countdown_countdown == 0) {
      printf("test_allocator failing from %s\n", __func__);
      return NULL;
    }
  }
  return malloc(size);
}

void *test_calloc(size_t count, size_t size) {
  if (g_cdd_alloc_fail_countdown_countdown > 0) {
    if (--g_cdd_alloc_fail_countdown_countdown == 0) {
      printf("test_allocator failing from %s\n", __func__);
      return NULL;
    }
  }
  return calloc(count, size);
}

void *test_realloc(void *ptr, size_t size) {
  if (g_cdd_alloc_fail_countdown_countdown > 0) {
    if (--g_cdd_alloc_fail_countdown_countdown == 0) {
      printf("test_allocator failing from %s\n", __func__);
      return NULL;
    }
  }
  return realloc(ptr, size);
}

#if defined(_MSC_VER)
char *test_strdup(const char *str) {
  if (g_cdd_alloc_fail_countdown_countdown > 0) {
    if (--g_cdd_alloc_fail_countdown_countdown == 0) {
      printf("test_allocator failing from %s\n", __func__);
      return NULL;
    }
  }
  return _strdup(str);
}
#else
char *test_strdup(const char *str) {
  if (g_cdd_alloc_fail_countdown_countdown > 0) {
    if (--g_cdd_alloc_fail_countdown_countdown == 0) {
      printf("test_allocator failing from %s\n", __func__);
      return NULL;
    }
  }
  return strdup(str);
}
#endif
