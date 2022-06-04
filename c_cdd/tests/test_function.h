#ifndef C_CDD_TESTS_TEST_FUNCTION_H
#define C_CDD_TESTS_TEST_FUNCTION_H

#include <greatest.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <c89stringutils_string_extras.h>
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */

#include <c_str_precondition_internal.h>

#include <c_cdd_utils.h>
#include <cst.h>

#include <cdd_helpers.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define NUM_LONG_FMT "z"
#else
#define NUM_LONG_FMT "l"
#endif

static const char sum_func_src[] = "int sum(int a, int b) { return a + b; }";

void az_PRECONDITION(int32_t destination_max_size) {
  if (destination_max_size <= 0)
    az_precondition_failed_get_callback()();
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

  printf("scanned->size         = %u\n\n", scanned->size);

  /*ASSERT_EQ(scanned->size, n);*/

  for (iter = (struct az_span_elem *)scanned->list; iter != NULL;
       iter = iter->next, i++) {
    const int32_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    char *s;
    az_span_to_str(iter_s, n, iter->span);
    asprintf(&s, "scanned_str_l[%" NUM_LONG_FMT "u]", i);
    print_escaped(s, (char *)scanned_str_l[i]);
    putchar('\n');
    /*ASSERT_STR_EQ(iter_s, scanned_str_l[i++]);*/
    free(s);
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

TEST x_size_t_ll() {
  struct size_t_elem *full_ll = malloc(sizeof *full_ll);
  struct size_t_elem **ll_cur_ptr = &full_ll, *iter;

  struct size_t_list *ll = malloc(sizeof *ll);

  const size_t l[] = {5, 6, 10, 44};
  size_t i;

  ll->list = full_ll;

  for (i = 0; i < sizeof l / sizeof l[0]; i++)
    size_t_list_push(&ll->size, &ll_cur_ptr, l[i]);

  for (iter = (struct size_t_elem *)ll->list, i = 0; iter != NULL;
       iter = iter->next, i++)
    printf("ll->list[%" NUM_LONG_FMT "u] = %" NUM_LONG_FMT "u\n", i, iter->lu);

  PASS();
}

SUITE(function_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(x_size_t_ll);
  /*RUN_TEST(x_test_function_scanned);*/
  /*RUN_TEST(x_test_function_tokenizer);*/
  /*RUN_TEST(x_test_function_parsed);*/
}

#endif /* !C_CDD_TESTS_TEST_FUNCTION_H */
