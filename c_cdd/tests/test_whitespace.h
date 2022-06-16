#ifndef C_CDD_TESTS_TEST_WHITESPACE_H
#define C_CDD_TESTS_TEST_WHITESPACE_H

#include <greatest.h>

#include <c_str_precondition_internal.h>

#include <c_cdd_utils.h>
#include <cst.h>

#include <cdd_helpers.h>

static const char whitespace_src[] = "\n\r\v"
                                     "/* C comment 0 */"
                                     "\n"
                                     "/* C comment*\\/ fin */";

TEST x_test_whitespace_scanned(void) {
  const az_span whitespace_span =
      az_span_create_from_str((char *)whitespace_src);
  struct scan_az_span_list *const scanned = scanner(whitespace_span);
  struct scan_az_span_elem *iter;
  enum { n = 4 };
  size_t i;
  struct StrScannerKind scanned_l[n] = {{"\n\r\v", Whitespace},
                                        {"/* C comment 0 */", CComment},
                                        {"\n", Whitespace},
                                        {"/* C comment*\\/ fin */", CComment}};
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

SUITE(whitespace_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(x_test_whitespace_scanned);
}

#endif /* !C_CDD_TESTS_TEST_WHITESPACE_H */
