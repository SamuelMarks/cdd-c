#ifndef TEST_FS_H
#define TEST_FS_H

#include "cdd_test_helpers/cdd_helpers.h" 
#include <errno.h>
#include <fs.h>
#include <greatest.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <wchar.h>
#endif /* !_MSC_VER */ 

#ifndef _MSC_VER
#include <sys/stat.h>
#endif /* !_MSC_VER */ 

TEST test_get_basename(void) { 
  char *res = NULL; 
  int rc; 

  rc = get_basename(PATH_SEP "foo" PATH_SEP "bar" PATH_SEP "baz.txt", &res); 
  ASSERT_EQ(0, rc); 
  ASSERT_STR_EQ("baz.txt", res); 
  free(res); 

  rc = get_basename("file.txt", &res); 
  ASSERT_EQ(0, rc); 
  ASSERT_STR_EQ("file.txt", res); 
  free(res); 

  rc = get_basename(PATH_SEP "foo" PATH_SEP "bar" PATH_SEP, &res); 
  ASSERT_EQ(0, rc); 
  ASSERT_STR_EQ("bar", res); 
  free(res); 

  rc = get_basename(NULL, &res); 
  ASSERT_EQ(0, rc); 
  ASSERT_STR_EQ(".", res); 
  free(res); 

  PASS(); 
} 

TEST test_read_to_file_error(void) { 
  int rc; 
  size_t size = 0; 
  char *s = NULL; 

  rc = read_to_file("file_that_does_not_exist.xyz", "r", &s, &size); 
  ASSERT_EQ(ENOENT, rc); 
  ASSERT_EQ(NULL, s); 

  rc = read_to_file(NULL, "r", &s, &size); 
  ASSERT_EQ(EINVAL, rc); 

  PASS(); 
} 

static int mock_walk_cb(const char *path, void *user_data) { 
  int *count = (int *)user_data; 
  (*count)++; 
  (void)path; 
  return 0; 
} 

TEST test_walk_directory(void) { 
  /* Setup structure: 
     walk_test_root/ 
       a.txt
       sub/ 
         b.txt
  */ 
  char *sys_tmp = NULL; 
  char *root = NULL; 
  int count = 0; 
  int rc; 

  /* tempdir returns system temp path usually */ 
  rc = tempdir(&sys_tmp); 
  ASSERT_EQ(0, rc); 

  /* Create a unique subdirectory to avoid counting other temp files */ 
  if (asprintf(&root, "%s%swalk_test_%d", sys_tmp, PATH_SEP, rand()) == -1) { 
    free(sys_tmp); 
    FAILm("asprintf failed"); 
  } 
  free(sys_tmp); 

  rc = makedir(root); 
  ASSERT_EQ(0, rc); 

  { 
    char *sub = NULL; 
    char *f1 = NULL; 
    char *f2 = NULL; 

    asprintf(&sub, "%s%ssub", root, PATH_SEP); 
    makedir(sub); 

    asprintf(&f1, "%s%sa.txt", root, PATH_SEP); 
    write_to_file(f1, "a"); 

    asprintf(&f2, "%s%sb.txt", sub, PATH_SEP); 
    write_to_file(f2, "b"); 

    rc = walk_directory(root, mock_walk_cb, &count); 
    ASSERT_EQ(0, rc); 
    /* Expect 2 files */ 
    ASSERT_EQ(2, count); 

    remove(f2); 
    remove(f1); 
    rmdir(sub); 
    rmdir(root); 

    free(sub); 
    free(f1); 
    free(f2); 
  } 
  free(root); 
  PASS(); 
} 

TEST test_makedir_check(void) { 
  char *t = NULL; 
  char *p = NULL; 
  tempdir(&t); 
  asprintf(&p, "%s%snewdir", t, PATH_SEP); 
  ASSERT_EQ(0, makedir(p)); 
  rmdir(p); 
  /* Don't try to remove system temp dir (t) */ 
  /* rmdir(t); */ 
  free(p); 
  free(t); 
  PASS(); 
} 

SUITE(fs_suite) { 
  RUN_TEST(test_get_basename); 
  RUN_TEST(test_read_to_file_error); 
  RUN_TEST(test_walk_directory); 
  RUN_TEST(test_makedir_check); 
} 

#endif /* !TEST_FS_H */
