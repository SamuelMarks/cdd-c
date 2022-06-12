#ifndef C_CDD_TESTS_TEST_STRUCT_H
#define C_CDD_TESTS_TEST_STRUCT_H

#include <greatest.h>

#include "cst.h"

static const char one_structs_src[] = "struct Haz {\n"
                                      "  const char *bzr;\n"
                                      "};\n";

static const char two_structs_src[] = "struct Haz {\n"
                                      "  const char *bzr;\n"
                                      "};\n"
                                      "\n"
                                      "struct Foo {\n"
                                      "  const char *bar;\n"
                                      "  int can;\n"
                                      "  struct Haz *haz;\n"
                                      "};\n";

TEST x_test_one_structs_scanned(void) {
  const az_span one_structs_span =
      az_span_create_from_str((char *)sum_func_src);
  struct scan_az_span_elem *scanned =
      (struct scan_az_span_elem *)scanner(one_structs_span);
  struct scan_az_span_elem *iter;
  enum { n = 5 };
  size_t i = 0;
  static const char *scanned_str_l[n] = {
      "struct Haz ", "{", "\012  const char *bzr;", "\012}", ";"};
  static enum ScannerKind scanned_kind_l[n] = {};

  for (iter = scanned; iter != NULL; iter = iter->next) {
    const size_t n = az_span_size(iter->span);
    char *iter_s = malloc(n);
    az_span_to_str(iter_s, (int32_t)n, iter->span);
    ASSERT_STR_EQ(scanned_str_l[i++], iter_s);
    ASSERT_EQ(iter->kind, scanned_kind_l[i]);
    free(iter_s);
  }
  PASS();
}

TEST x_test_two_structs_scanned(void) {
  const az_span two_structs_span =
      az_span_create_from_str((char *)two_structs_src);
  struct scan_az_span_elem *scanned =
      (struct scan_az_span_elem *)scanner(two_structs_span);
  struct scan_az_span_elem *iter;
  enum { n = 12 };
  size_t i = 0;
  static const char *scanned_str_l[n] = {"struct Haz ",
                                         "{",
                                         "\012  const char *bzr;",
                                         "\012}",
                                         ";",
                                         "\012\012struct Foo ",
                                         "{",
                                         "\012  const char *bar;",
                                         "\012  int can;",
                                         "\012  struct Haz *haz;",
                                         "\012}",
                                         ";"};
  static enum ScannerKind scanned_kind_l[n] = {/*TODO*/};

  for (iter = scanned; iter != NULL; iter = iter->next) {
    const size_t n = az_span_size(iter->span);
    char *iter_s = malloc(n);
    az_span_to_str(iter_s, (int32_t)n, iter->span);
    ASSERT_STR_EQ(scanned_str_l[i++], iter_s);
    ASSERT_EQ(iter->kind, scanned_kind_l[i]);
    free(iter_s);
  }
  PASS();
}

SUITE(struct_suite) {
  RUN_TEST(x_test_one_structs_scanned);
  RUN_TEST(x_test_two_structs_scanned);
}

#endif /* !C_CDD_TESTS_TEST_STRUCT_H */
