/* clang-format off */
#include <stdlib.h>
/* clang-format on */
extern int g_cdd_fail_alloc;

/* Mmm, we shouldn't define this here, we can't easily mock malloc since other
 * things use it, but wait! We can define a test specific cdd_cst_emit function
 * or just not hit 100% lines in cdd_cst_emit without doing a LD_PRELOAD.
 * Actually, I am missing 8 branches... */
