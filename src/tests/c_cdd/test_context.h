/**
 * @file test_context.h
 * @brief Unit tests for `cdd_context_t` allocator, VFS, and socket VTables.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CONTEXT_H
#define TEST_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/context.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse/fs.h"
/* clang-format on */

/**
 * @brief Tests `cdd_context_init_default`, `cdd_context_set_global`,
 * `cdd_context_get_global`, and VFS/allocator VTable callbacks.
 * @return Test or helper result.
 */
TEST test_cdd_context_lifecycle_and_vtables(void) {
  cdd_context_t ctx;
  const cdd_context_t *active = NULL;
  void *ptr = NULL;
  char *dup = NULL;
  FILE *fp = NULL;
  char buf[16];
  size_t n_io = 0;
  int is_dir = 0;
  void *dir_handle = NULL;
  const char *tmp_file = "test_build_dir/cdd_ctx_test.txt";

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_context_init_default(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_context_get_global(NULL));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_context_init_default(&ctx));
  ctx.user_data = &ctx;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_context_set_global(&ctx));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_context_get_global(&active));
  ASSERT_EQ(&ctx, active);
  ASSERT_EQ(&ctx, active->user_data);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_context_set_global(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_context_get_global(&active));
  ASSERT(active != NULL);
  ASSERT_EQ(NULL, active->user_data);

  /* Allocator VTable */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            active->allocator.malloc_fn(16, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            active->allocator.calloc_fn(4, 4, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            active->allocator.realloc_fn(NULL, 16, NULL));

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, active->allocator.malloc_fn(16, &ptr));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, active->allocator.calloc_fn(4, 4, &ptr));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, active->allocator.realloc_fn(NULL, 16, &ptr));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, active->allocator.malloc_fn(16, &ptr));
  ASSERT(ptr != NULL);
  ASSERT_EQ(CDD_C_SUCCESS, active->allocator.realloc_fn(ptr, 32, &ptr));
  ASSERT(ptr != NULL);
  active->allocator.free_fn(ptr);

  ptr = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, active->allocator.calloc_fn(4, 4, &ptr));
  ASSERT(ptr != NULL);
  active->allocator.free_fn(ptr);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            active->allocator.strdup_fn(NULL, &dup));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            active->allocator.strdup_fn("hello", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, active->allocator.strdup_fn("hello", &dup));
  ASSERT_STR_EQ("hello", dup);
  active->allocator.free_fn(dup);

  /* VFS VTable error and success paths */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, active->vfs.fopen_fn(NULL, "r", &fp));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, active->vfs.fopen_fn("a", NULL, &fp));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, active->vfs.fopen_fn("a", "r", NULL));
  ASSERT_EQ(CDD_C_ERROR_IO,
            active->vfs.fopen_fn("/nonexistent_dir_xyz/file.txt", "r", &fp));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            active->vfs.fread_fn(NULL, 1, 1, fp, &n_io));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            active->vfs.fwrite_fn(NULL, 1, 1, fp, &n_io));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, active->vfs.fclose_fn(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, active->vfs.stat_fn(NULL, &is_dir));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, active->vfs.stat_fn(".", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            active->vfs.opendir_fn(NULL, &dir_handle));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, active->vfs.opendir_fn(".", NULL));

  ASSERT_EQ(CDD_C_SUCCESS, makedirs("test_build_dir"));

  ASSERT_EQ(CDD_C_SUCCESS, active->vfs.fopen_fn(tmp_file, "wb", &fp));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            active->vfs.fread_fn(buf, 1, 1, NULL, &n_io));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            active->vfs.fread_fn(buf, 1, 1, fp, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            active->vfs.fwrite_fn("abc", 1, 3, NULL, &n_io));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            active->vfs.fwrite_fn("abc", 1, 3, fp, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, active->vfs.fwrite_fn("abc", 1, 3, fp, &n_io));
  ASSERT_EQ(3, n_io);
  ASSERT_EQ(CDD_C_SUCCESS, active->vfs.fclose_fn(fp));

  ASSERT_EQ(CDD_C_SUCCESS, active->vfs.fopen_fn(tmp_file, "rb", &fp));
  /* Writing to a read-only stream fails */
  ASSERT_EQ(CDD_C_ERROR_IO, active->vfs.fwrite_fn("abc", 1, 3, fp, &n_io));
  memset(buf, 0, sizeof(buf));
  ASSERT_EQ(CDD_C_SUCCESS, active->vfs.fread_fn(buf, 1, 3, fp, &n_io));
  ASSERT_EQ(3, n_io);
  ASSERT_STR_EQ("abc", buf);
  ASSERT_EQ(CDD_C_SUCCESS, active->vfs.fclose_fn(fp));

  ASSERT_EQ(CDD_C_SUCCESS, active->vfs.stat_fn("test_build_dir", &is_dir));
  ASSERT_EQ(1, is_dir);
  ASSERT_EQ(CDD_C_SUCCESS,
            active->vfs.opendir_fn("test_build_dir", &dir_handle));
  ASSERT(dir_handle != NULL);
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            active->vfs.opendir_fn(tmp_file, &dir_handle));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            active->vfs.opendir_fn("/nonexistent_dir_xyz", &dir_handle));

  remove(tmp_file);
  PASS();
}

/**
 * @brief Executes the context_suite test suite.
 */
SUITE(context_suite) { RUN_TEST(test_cdd_context_lifecycle_and_vtables); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CONTEXT_H */
