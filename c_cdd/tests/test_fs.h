#ifndef TEST_FS_H
#define TEST_FS_H

#include <fs.h>
#include <greatest.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define PATH_SEP "\\"
#define PATH_SEP_C '\\'
#define strtok_r strtok_s
#else
#define PATH_SEP "/"
#define PATH_SEP_C '/'
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

TEST test_get_basename(void) {
  const char *res;

  res = get_basename(PATH_SEP "foo" PATH_SEP "bar" PATH_SEP "baz.txt");
  ASSERT_STR_EQ("baz.txt", res);

  res = get_basename("file.txt");
  ASSERT_STR_EQ("file.txt", res);

  res = get_basename(PATH_SEP "foo" PATH_SEP "bar" PATH_SEP);
  ASSERT_STR_EQ("", res);

  res = get_basename(NULL);
  ASSERT_EQ(NULL, res);

  PASS();
}

SUITE(fs_suite) { RUN_TEST(test_get_basename); }

#endif /* !TEST_FS_H */
