#ifndef C_CDD_TESTS_TEST_MACRO_H
#define C_CDD_TESTS_TEST_MACRO_H

#include <greatest.h>

#include <c_str_precondition_internal.h>

#include <c_cdd_utils.h>
#include <cst.h>

#include <cdd_helpers.h>

static const char macro_src[] = "# define foo bar\n"
                                "#ifdef FOO\n"
                                "# define CAT(bar,foo)(bar ## foo)\n"
                                "#define HAZ\\\nFOO\n";

x_test_macro_scanned(void) {
  const az_span macro_span = az_span_create_from_str((char *)macro_src);
  const struct scan_az_span_list *const scanned = scanner(macro_span);
  struct scan_az_span_elem *iter;
  enum { n = 4 };
  size_t i;
  struct StrScannerKind scanned_l[n] = {
      {"# define foo bar\n", Macro},
      {"#ifdef FOO\n", Macro},
      {"# define CAT(bar,foo)(bar ## foo)\n", Macro},
      {"#define HAZ\\\nFOO\n", Macro}};

  debug_scanned_with_mock(scanned, &i, scanned_l, n);
  ASSERT_GT(i, 0);

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

SUITE(macro_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(x_test_macro_scanned);
}

#endif /* !C_CDD_TESTS_TEST_MACRO_H */
