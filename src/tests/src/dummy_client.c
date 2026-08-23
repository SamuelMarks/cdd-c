#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
/* clang-format off */
#include "cdd_c_error.h"
/* clang-format on */

cdd_c_error_t dummy_client(void);

cdd_c_error_t dummy_client(void) { return CDD_C_SUCCESS; }

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
