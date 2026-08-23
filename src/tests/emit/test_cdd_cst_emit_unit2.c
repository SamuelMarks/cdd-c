#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
/* clang-format off */
#include "c_cdd_export.h"
#include <stdlib.h>
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern int g_cdd_alloc_fail;
/* extern int g_cdd_alloc_fail; (moved to global) */

/* Mmm, we shouldn't define this here, we can't easily mock malloc since other
 * things use it, but wait! We can define a test specific cdd_cst_emit function
 * or just not hit 100% lines in cdd_cst_emit without doing a LD_PRELOAD.
 * Actually, I am missing 8 branches... */

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
