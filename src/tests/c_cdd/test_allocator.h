#ifndef C_CDD_TEST_ALLOCATOR_H
#define C_CDD_TEST_ALLOCATOR_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int g_cdd_alloc_fail_countdown_countdown;

void *test_malloc(size_t size);
void *test_calloc(size_t count, size_t size);
void *test_realloc(void *ptr, size_t size);
char *test_strdup(const char *str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
