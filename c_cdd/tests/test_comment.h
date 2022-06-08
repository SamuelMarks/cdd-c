#ifndef C_CDD_TESTS_TEST_COMMENT_H
#define C_CDD_TESTS_TEST_COMMENT_H

#include <greatest.h>

#include <c_str_precondition_internal.h>

#include <c_cdd_utils.h>
#include <cst.h>

#include <cdd_helpers.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define NUM_LONG_FMT "z"
#else
#define NUM_LONG_FMT "l"
#endif

static const char comment_src[] = "// C++ comment\n"
                                  "/* C comment 0 */"
                                  "/* C comment 1 */"
                                  "/* C comment*\\/ fin */";

TEST x_test_comment_scanned(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)comment_src);
  struct az_span_list *scanned = scanner(sum_func_span);
  struct az_span_elem *iter;
  enum { n = 4 };
  size_t i;
  static const char *scanned_str_l[n] = {
      "// C++ comment\n", "/* C comment 0 */", "/* C comment 1 */",
      "/* C comment*\\/ fin */"};

  ASSERT_EQ(scanned->size, n);

  for (iter = (struct az_span_elem *)scanned->list, i = 0; iter != NULL;
       iter = iter->next, i++) {
    const int32_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    az_span_to_str(iter_s, n, iter->span);
    ASSERT_STR_EQ(iter_s, scanned_str_l[i]);
    free(iter_s);
  }
  az_span_list_cleanup(scanned);
  ASSERT_EQ(scanned->size, 0);
  ASSERT_EQ(scanned->list, NULL);
  PASS();
}

SUITE(comment_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(x_test_comment_scanned);
}

#endif /* !C_CDD_TESTS_TEST_COMMENT_H */