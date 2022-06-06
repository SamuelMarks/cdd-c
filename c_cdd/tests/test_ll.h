#ifndef C_CDD_TESTS_TEST_LL_H
#define C_CDD_TESTS_TEST_LL_H

#include <greatest.h>

#include <ll.h>

TEST x_size_t_ll() {
  struct size_t_elem *full_ll = calloc(1, sizeof *full_ll);
  struct size_t_elem **ll_cur_ptr = &full_ll, *iter;

  struct size_t_list *ll = malloc(sizeof *ll);

  const size_t l[] = {5, 6, 10, 44};
  size_t i;

  ll->list = full_ll->next, ll->size = 0;

  for (i = 0; i < sizeof l / sizeof l[0]; i++)
    size_t_list_push(&ll->size, &ll_cur_ptr, l[i]);

  for (iter = (struct size_t_elem *)ll->list, i = 0; iter != NULL;
       iter = iter->next, i++)
    ASSERT_EQ(iter->lu, l[i]);

  size_t_list_cleanup(ll);

  ASSERT_EQ(ll->list, NULL);
  ASSERT_EQ(ll->size, 0);

  PASS();
}

TEST x_az_span_ll() {
  struct az_span_elem *full_ll = calloc(1, sizeof *full_ll);
  struct az_span_elem **ll_cur_ptr = &full_ll, *iter;

  struct az_span_list *ll = malloc(sizeof *ll);

  const az_span l[] = {AZ_SPAN_FROM_STR("foo"), AZ_SPAN_FROM_STR("bar")};
  size_t i;

  ll->list = full_ll->next, ll->size = 0;

  for (i = 0; i < sizeof l / sizeof l[0]; i++)
    az_span_list_push(&ll->size, &ll_cur_ptr, l[i]);

  for (iter = (struct az_span_elem *)ll->list, i = 0; iter != NULL;
       iter = iter->next, i++)
    ASSERT_FALSE(!az_span_is_content_equal(iter->span, l[0]));

  az_span_list_cleanup(ll);

  ASSERT_EQ(ll->list, NULL);
  ASSERT_EQ(ll->size, 0);

  PASS();
}

SUITE(ll_suite) {
  RUN_TEST(x_size_t_ll);
  RUN_TEST(x_az_span_ll);
}

#endif /* !C_CDD_TESTS_TEST_LL_H */
