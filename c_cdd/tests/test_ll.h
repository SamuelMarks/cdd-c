#ifndef C_CDD_TESTS_TEST_LL_H
#define C_CDD_TESTS_TEST_LL_H

#include <greatest.h>

#include <ll.h>

TEST x_size_t_ll() {
  struct size_t_elem *full_ll = malloc(sizeof *full_ll);
  struct size_t_elem **ll_cur_ptr = &full_ll, *iter;

  struct size_t_list *ll = malloc(sizeof *ll);

  const size_t l[] = {5, 6, 10, 44};
  size_t i;

  ll->list = full_ll->next;

  for (i = 0; i < sizeof l / sizeof l[0]; i++)
    size_t_list_push(&ll->size, &ll_cur_ptr, l[i]);

  for (iter = (struct size_t_elem *)ll->list, i = 0; iter != NULL;
       iter = iter->next, i++)
    ASSERT_EQ(iter->lu, l[i]);

  PASS();
}

SUITE(ll_suite) { RUN_TEST(x_size_t_ll); }

#endif /* !C_CDD_TESTS_TEST_LL_H */
