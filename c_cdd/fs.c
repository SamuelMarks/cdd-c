#include <stdio.h>
#include <string.h>

#include "fs.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define PATH_SEP "\\"
#define PATH_SEP_C '\'
#define strtok_r strtok_s
#else
#define PATH_SEP "/"
#define PATH_SEP_C '/'
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

const char *get_basename(const char *const path) {
  /*char *pch;
  char *ans;
  pch = strtok_r(path, PATH_SEP, &ans);
  if (pch == NULL)
    ans = path;
  while (pch != NULL) {
    ans = pch;
    pch = strtok_r(NULL, PATH_SEP, &ans);
  }
  return ans;*/

  const char *last_slash;
  if (path == NULL)
    return path;

  last_slash = strrchr(path, PATH_SEP_C);

  if (last_slash != NULL) {
    return last_slash + 1;
  } else {
    return path;
  }
}
