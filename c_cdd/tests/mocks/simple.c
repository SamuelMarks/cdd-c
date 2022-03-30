#include <stdlib.h>

#include "simple.h"

void cleanup_Haz(struct Haz *haz) { free(haz); }

void cleanup_Foo(struct Foo *foo) {
  cleanup_Haz(foo->haz);
  free(foo);
}
