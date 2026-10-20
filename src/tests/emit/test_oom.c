/**
 * @file test_oom.c
 * @brief Unit tests and mocks in test_oom.c.
 */
/* clang-format off */#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
/* clang-format on */
/**
 * @brief Helper function main.
 * @return Result status or value.
 */
cdd_c_error_t main(void) {
  struct rlimit rl;
  if (getrlimit(RLIMIT_AS, &rl) != 0) {
    perror("getrlimit");
    return 1;
  }
  rl.rlim_cur = 1024 * 1024 * 10; /* 10 MB */
  if (setrlimit(RLIMIT_AS, &rl) == 0) {
    void *p = malloc(1024 * 1024 * 20);
    if (!p) {
      printf("OOM worked\n");
    } else {
      printf("OOM failed\n");
      free(p);
    }
  } else {
    perror("setrlimit");
  }
  return 0;
}
