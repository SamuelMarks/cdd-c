#ifndef TEST_DSL_INTEGRATION_H
#define TEST_DSL_INTEGRATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../src/cdd_api.h"
#include "../cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

TEST test_dsl_integration_basic(void) {
  cdd_c_error_t rc;
  const char *source = "/* comment */\nfunc my_func([in] *mut i32 arg1) {\n  "
                       "body %{ return 0; }%\n}";
  cdd_generate_bindings_config_t config = {0};

  config.output_dir = "out";
  config.target_langs = "dsl";

  rc = cdd_generate_bindings_from_dsl(source, &config);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  PASS();
}

TEST test_dsl_integration_fail(void) {
  cdd_c_error_t rc;
  cdd_generate_bindings_config_t config = {0};

  rc = cdd_generate_bindings_from_dsl(NULL, &config);
  ASSERT_EQ_FMT(CDD_C_ERROR_INVALID_ARGUMENT, rc, "%d");

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_INTEGRATION_H */
