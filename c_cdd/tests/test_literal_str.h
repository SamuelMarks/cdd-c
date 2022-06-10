#ifndef C_CDD_TESTS_TEST_LITERAL_STR_H
#define C_CDD_TESTS_TEST_LITERAL_STR_H

#include <greatest.h>

#include <c_str_precondition_internal.h>

#include <c_cdd_utils.h>
#include <cst.h>

#include <assert.h>
#include <cdd_helpers.h>

TEST x_test_double_literal_str_scanned(void) {
  static const char literal_str_src[] = "\"foo\";\n"
                                        "\"bar can\";\n";
  const az_span literal_str_span =
      az_span_create_from_str((char *)literal_str_src);
  struct scan_az_span_list *scanned = scanner(literal_str_span);
  struct scan_az_span_elem *iter;
  enum { n = 2 };
  size_t i;
  static const char *scanned_str_l[n] = {"\"foo\"", "\"bar can\""};
  static enum ScannerKind scanned_kind_l[n] = {DoubleQuoted, DoubleQuoted};

  ASSERT_EQ(scanned->size, n);

  for (iter = (struct scan_az_span_elem *)scanned->list, i = 0; iter != NULL;
       iter = iter->next, i++) {
    const int32_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    az_span_to_str(iter_s, n, iter->span);
    ASSERT_STR_EQ(iter_s, scanned_str_l[i]);
    ASSERT_EQ(iter->kind, scanned_kind_l[i]);
    free(iter_s);
  }
  ASSERT_EQ(scanned->size, i);
  scan_az_span_list_cleanup(scanned);
  ASSERT_EQ(scanned->size, 0);
  ASSERT_EQ(scanned->list, NULL);
  PASS();
}

TEST x_test_single_literal_str_scanned(void) {
  static const char literal_str_src[] = "'a';\n"
                                        "'\\n';\n"
                                        "'\\'\n";
  const az_span literal_str_span =
      az_span_create_from_str((char *)literal_str_src);
  struct scan_az_span_list *scanned = scanner(literal_str_span);
  struct scan_az_span_elem *iter;
  enum { n = 3 };
  size_t i;
  static const char *scanned_str_l[n] = {"'a'", "'\\n'", "'\\'"};
  static enum ScannerKind scanned_kind_l[n] = {SingleQuoted, SingleQuoted,
                                               SingleQuoted};

  ASSERT_EQ(scanned->size, n);

  for (iter = (struct scan_az_span_elem *)scanned->list, i = 0; iter != NULL;
       iter = iter->next, i++) {
    const int32_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    const char *scanned_str = scanned_str_l[i];
    az_span_to_str(iter_s, n, iter->span);
    ASSERT_STR_EQ(iter_s, scanned_str);
    assert(strcmp(iter_s, scanned_str) == 0);
    ASSERT_EQ(iter->kind, scanned_kind_l[i]);
    free(iter_s);
  }
  ASSERT_EQ(scanned->size, i);
  scan_az_span_list_cleanup(scanned);
  ASSERT_EQ(scanned->size, 0);
  ASSERT_EQ(scanned->list, NULL);
  PASS();
}

TEST x_test_literal_str_scanned(void) {
  static const char literal_str_src[] = "\"foo\";\n"
                                        "'a';\n"
                                        "'\\n';\n"
                                        "'\\'\n"
                                        "\"bar can\";\n"
                                        "\"cat\" \"cat\"\n"
                                        "\"catt\"\"catt\"\n"
                                        "\"cut\"\n\"cut\"\n";
  const az_span literal_str_span =
      az_span_create_from_str((char *)literal_str_src);
  struct scan_az_span_list *scanned = scanner(literal_str_span);
  struct scan_az_span_elem *iter;
  enum { n = 11 };
  size_t i;
  static const char *scanned_str_l[n] = {
      "\"foo\"", "'a'",      "'\\n'",    "'\\'",    "\"bar can\"", "\"cat\"",
      "\"cat\"", "\"catt\"", "\"catt\"", "\"cut\"", "\"cut\""};
  static enum ScannerKind scanned_kind_l[n] = {
      DoubleQuoted, SingleQuoted, SingleQuoted, SingleQuoted,
      DoubleQuoted, DoubleQuoted, DoubleQuoted, DoubleQuoted,
      DoubleQuoted, DoubleQuoted, DoubleQuoted};

  SKIP();

  ASSERT_EQ(scanned->size, n);

  for (iter = (struct scan_az_span_elem *)scanned->list, i = 0; iter != NULL;
       iter = iter->next, i++) {
    const int32_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    az_span_to_str(iter_s, n, iter->span);
    ASSERT_STR_EQ(iter_s, scanned_str_l[i]);
    ASSERT_EQ(iter->kind, scanned_kind_l[i]);
    free(iter_s);
  }
  ASSERT_EQ(scanned->size, i);
  scan_az_span_list_cleanup(scanned);
  ASSERT_EQ(scanned->size, 0);
  ASSERT_EQ(scanned->list, NULL);
  PASS();
}

SUITE(literal_str_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(x_test_double_literal_str_scanned);
  RUN_TEST(x_test_single_literal_str_scanned);
  RUN_TEST(x_test_literal_str_scanned);
}

#endif /* !C_CDD_TESTS_TEST_LITERAL_STR_H */
