#ifndef C_CDD_TESTS_TEST_FUNCTION_H
#define C_CDD_TESTS_TEST_FUNCTION_H

#include <greatest.h>

#include <c_str_precondition_internal.h>

#include "tests/test_c_cdd_lib_export.h"
#include <c_cdd_utils.h>
#include <cst.h>

static const char sum_func_src[] = "int sum(int a, int b) { return a + b; }";

void az_PRECONDITION(int32_t destination_max_size) {
  if (!(destination_max_size > 0)) {
    az_precondition_failed_get_callback()();
  }
}

void span_to_str(char *destination, int32_t destination_max_size,
                 az_span source) {
  _az_PRECONDITION_NOT_NULL(destination);
  _az_PRECONDITION(destination_max_size > 0);

  // Implementations of memmove generally do the right thing when number of
  // bytes to move is 0, even if the ptr is null, but given the behavior is
  // documented to be undefined, we disallow it as a precondition.
  _az_PRECONDITION_VALID_SPAN(source, 0, false);

  {
    int32_t size_to_write = az_span_size(source);

    az_PRECONDITION(size_to_write < destination_max_size);

    // Even though the contract of this function is that the
    // destination_max_size must be larger than source to be able to copy all of
    // the source to the char buffer including an extra null terminating
    // character, cap the data move if the source is too large, to avoid memory
    // corruption.
    if (size_to_write >= destination_max_size) {
      // Leave enough space for the null terminator.
      size_to_write = destination_max_size - 1;

      // If destination_max_size was 0, we don't want size_to_write to be
      // negative and corrupt data before the destination pointer.
      if (size_to_write < 0) {
        size_to_write = 0;
      }
    }

    az_PRECONDITION(size_to_write >= 0);

    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    memmove((void *)destination, (void const *)az_span_ptr(source),
            (size_t)size_to_write);
    destination[size_to_write] = 0;
  }
}

TEST x_test_function_scanned(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  const struct az_span_list *scanned = scanner(sum_func_span);
  struct az_span_elem *iter;
  enum { n = 4 };
  size_t i = 0;
  static const char *scanned_str_l[n] = {"int sum(int a, int b)", " ",
                                         "{ return a + b;", " "};

  printf("scanned->size: %u\n\n", scanned->size);
  ASSERT_EQ(scanned->size, n);

  for (iter = (struct az_span_elem *)scanned->list; iter != NULL;
       iter = iter->next, i++) {
    const int32_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    az_span_to_str(iter_s, n, iter->span);
    print_escaped("iter_s", iter_s);
    print_escaped("scanned_str_l[i]", (char *)scanned_str_l[i]);
    putchar('\n');
    /*ASSERT_STR_EQ(iter_s, scanned_str_l[i++]);*/
    free(iter_s);
  }
  PASS();
}

TEST x_test_function_tokenizer(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  const struct az_span_list *scanned = scanner(sum_func_span);
  const struct az_span_elem *tokens = tokenizer(scanned->list);
  PASS();
}

TEST x_test_function_parsed(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  const struct az_span_list *scanned = scanner(sum_func_span);
  const struct CstNode **parsed = parser((struct az_span_elem *)scanned);
  //  static enum TypeSpecifier int_specifier[] = {INT};
  //  static const struct Declaration a_arg = {/* pos_start */ 0,
  //                                           /* scope */ NULL,
  //                                           /* value */ "int a",
  //                                           /* specifiers */ int_specifier,
  //                                           /* type */ NULL,
  //                                           /* name */ "a"};
  //  static const struct Declaration b_arg = {/* pos_start */ 0,
  //                                           /* scope */ NULL,
  //                                           /* value */ "int b",
  //                                           /* specifiers */ int_specifier,
  //                                           /* type */ NULL,
  //                                           /* name */ "b"};
  //  struct Declaration args[2];
  //  struct Return _return = {/* pos_start */ 0,
  //                           /* scope */ NULL,
  //                           /* value */ "return a + b;",
  //                           /* val */ "a + b"};
  //  struct CstNode return_cst_node = {Return};
  //  struct CstNode sum_func_body[1];
  //  return_cst_node.type._return = _return;
  //  sum_func_body[0] = return_cst_node;
  //
  //  args[0] = a_arg;
  //  args[1] = b_arg;
  //
  //  {
  //    struct Function sum_func = {/* pos_start */ 0,
  //                                /* scope */ NULL,
  //                                /* value */ sum_func_src,
  //                                /* specifiers */ int_specifier,
  //                                /* name */ "sum",
  //                                /* args */ NULL,
  //                                /* body */ NULL};
  //    sum_func.args = args;
  //    sum_func.body = sum_func_body;
  //
  //    ASSERT_EQ(parsed, NULL);
  //    /* TODO: loop through parsed, assert contents contain expected
  //    information
  //     */ }
  PASS();
}

extern TEST_C_CDD_LIB_EXPORT void cdd_precondition_failed(void) {
  fputs("cdd_precondition_failed", stderr);
}

SUITE(function_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(x_test_function_scanned);
  /*RUN_TEST(x_test_function_tokenizer);*/
  /*RUN_TEST(x_test_function_parsed);*/
}

#endif /* !C_CDD_TESTS_TEST_FUNCTION_H */
