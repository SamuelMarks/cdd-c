
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/log.h"
#include "c_cdd_export.h"
#include <greatest.h>
/* clang-format on */
TEST test_log_basic(void) {
  ASSERT_EQ(0, c_cdd_log_debug("test"));
  PASS();
}
SUITE(log_suite) { RUN_TEST(test_log_basic); }

#ifdef __cplusplus
}
#endif /* __cplusplus */
