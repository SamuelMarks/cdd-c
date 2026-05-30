extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_desig_init.h
 * @brief Unit tests for designated initializer analysis.
 */

#ifndef TEST_DESIG_INIT_H
#define TEST_DESIG_INIT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>

#include "functions/parse/desig_init.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/**
 * @brief Tests basic functionality of designated initializer scanning.
 *
 * @return The result of the test.
 */
TEST test_scan_for_designated_initializers_basic(void) {
  struct TokenList *tokens = NULL;
  struct DesigInitList list;
  const char *src = "struct S my_s = {\n"
                    "  .x = 10,\n"
                    "  .y = \"hello\",\n"
                    "  .z = {.inner = 5}\n"
                    "};\n";

  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src), &tokens));
  desig_init_list_init(&list);

  ASSERT_EQ(0, scan_for_designated_initializers(tokens, &list));

  /* Should find 3 designated initializers */
  ASSERT_EQ(3, list.count);

  ASSERT_STR_EQ("x", list.sites[0].field_name);
  ASSERT_STR_EQ("10", list.sites[0].value_expr);

  ASSERT_STR_EQ("y", list.sites[1].field_name);
  ASSERT_STR_EQ("\"hello\"", list.sites[1].value_expr);

  ASSERT_STR_EQ("z", list.sites[2].field_name);
  ASSERT_STR_EQ("{.inner = 5}", list.sites[2].value_expr);

  desig_init_list_free(&list);
  free_token_list(tokens);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests error handling of designated initializer scanning.
 *
 * @return The result of the test.
 */
TEST test_scan_for_designated_initializers_errors(void) {
  struct TokenList *tl = NULL;
  struct DesigInitList list;
  desig_init_list_init(&list);
  ASSERT_EQ(0, tokenize(az_span_create_from_str("int x = 1;"), &tl));

  ASSERT_EQ(EINVAL, scan_for_designated_initializers(NULL, &list));
  ASSERT_EQ(EINVAL, scan_for_designated_initializers(tl, NULL));

  desig_init_list_free(&list);
  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Designated initializer test suite.
 */
SUITE(desig_init_suite) {
  RUN_TEST(test_scan_for_designated_initializers_basic);
  RUN_TEST(test_scan_for_designated_initializers_errors);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DESIG_INIT_H */
