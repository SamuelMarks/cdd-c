#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#else
#include <sys/errno.h>
#endif

#include "simple_json.h"

static const char *const haz_e_mock0 =
    "{\"bzr\": \"some_bzr\",\"tank\": \"SMALL\"}";
static const char *const foo_e_mock0 =
    "{\"bar\": \"some_bar\",\"can\": 5,\"haz\":{\"bzr\": "
    "\"some_bzr\",\"tank\": \"SMALL\"}}";

int main(void) {
  const enum Tank t = Tank_BIG;
  char *tank_as_str = NULL;
  struct HazE haz_e = {"some_bzr", Tank_SMALL};
  struct FooE foo_e = {"some_bar", 5, NULL};
  char *haz_e_json = NULL;
  char *foo_e_json = NULL;
  struct HazE *haz_e0 = malloc(sizeof(*haz_e0));
  struct FooE *foo_e0 = malloc(sizeof(*foo_e0));
  foo_e.haz = &haz_e;

  if (haz_e0 == NULL || foo_e0 == NULL)
    return ENOMEM;

  assert(Tank_to_str(t, &tank_as_str) == 0);
  assert(strcmp(tank_as_str, "BIG") == 0);
  free(tank_as_str);

  assert(HazE_to_json(&haz_e, &haz_e_json) == 0);
  assert(strcmp(haz_e_json, haz_e_mock0) == 0);
  free(haz_e_json);

  assert(HazE_from_json(haz_e_mock0, &haz_e0) == 0);
  assert(HazE_eq(haz_e0, &haz_e));
  free(haz_e0);

  assert(FooE_to_json(&foo_e, &foo_e_json) == 0);
  assert(strcmp(foo_e_json, foo_e_mock0) == 0);
  free(foo_e_json);

  assert(FooE_from_json(foo_e_mock0, &foo_e0) == 0);
  assert(FooE_eq(foo_e0, &foo_e));
  free(foo_e0);

  return EXIT_SUCCESS;
}
