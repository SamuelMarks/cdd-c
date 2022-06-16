#ifndef C_CDD_TESTS_TEST_COMMENT_H
#define C_CDD_TESTS_TEST_COMMENT_H

#include <greatest.h>

#include <c_str_precondition_internal.h>

#include <c_cdd_utils.h>
#include <cst.h>

#include <cdd_helpers.h>

static const char comment_src[] = "// C++ comment\n"
                                  "/* C comment 0 */"
                                  "/* C comment 1 */"
                                  "/* C comment*\\/ fin */";

TEST x_test_comment_scanned(void) {
  const az_span comment_span = az_span_create_from_str((char *)comment_src);
  struct scan_az_span_list *const scanned = scanner(comment_span);
  struct scan_az_span_elem *iter;
  enum { n = 4 };
  size_t i;
  struct StrScannerKind scanned_l[n] = {{"// C++ comment\n", CppComment},
                                        {"/* C comment 0 */", CComment},
                                        {"/* C comment 1 */", CComment},
                                        {"/* C comment*\\/ fin */", CComment}};

  debug_scanned_with_mock(scanned, &i, scanned_l, n);

  ASSERT_EQ(scanned->size, n);

  for (iter = (struct scan_az_span_elem *)scanned->list, i = 0; iter != NULL;
       iter = iter->next, i++) {
    const size_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    az_span_to_str(iter_s, n, iter->span);
    ASSERT_STR_EQ(scanned_l[i].s, iter_s);
    ASSERT_EQ(scanned_l[i].kind, iter->kind);
    free(iter_s);
  }
  ASSERT_EQ(scanned->size, i);
  scan_az_span_list_cleanup(scanned);
  ASSERT_EQ(scanned->size, 0);
  ASSERT_EQ(scanned->list, NULL);
  PASS();
}

SUITE(comment_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(x_test_comment_scanned);
}

#endif /* !C_CDD_TESTS_TEST_COMMENT_H */
