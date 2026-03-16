/**
 * @file test_desig_init.h
 * @brief Unit tests for designated initializer analysis.
 */

/* clang-format off */
#ifndef TEST_DESIG_INIT_H
#define TEST_DESIG_INIT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* clang-format off */
#include <greatest.h>
#include <string.h>

#include "functions/parse/desig_init.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */
/* clang-format on */

TEST test_scan_for_designated_initializers_basic(void) {
  struct TokenList *tokens = NULL;
  struct DesigInitList list;
  const char *src = "struct S my_s = {
                        .x = 10,
             .y = \"hello\",
                      .z = {.inner = 5}
};
";

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
ASSERT_STR_EQ("{ .inner = 5 }", list.sites[2].value_expr);

desig_init_list_free(&list);
free_token_list(tokens);
PASS();
}

SUITE(desig_init_suite) {
  RUN_TEST(test_scan_for_designated_initializers_basic);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DESIG_INIT_H */
