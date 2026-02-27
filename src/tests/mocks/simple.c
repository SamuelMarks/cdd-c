#include <stdlib.h>

#include "simple.h"

void Haz_cleanup(struct Haz *const haz) { free(haz); }

void Foo_cleanup(struct Foo *const foo) {
  if (foo == NULL)
    return;
  Haz_cleanup(foo->haz);
  free(foo);
}
