#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

int main() {
  struct rlimit rl;
  if (getrlimit(RLIMIT_AS, &rl) != 0) {
    perror("getrlimit");
    return 1;
  }
  rl.rlim_cur = 1024 * 1024 * 10; /* 10 MB */
  if (setrlimit(RLIMIT_AS, &rl) == 0) {
    void *p = malloc(1024 * 1024 * 20);
    if (!p)
      printf("OOM worked\n");
    else
      printf("OOM failed\n");
  } else {
    perror("setrlimit");
  }
  return 0;
}
