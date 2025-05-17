#ifndef TEST_FS_H
#define TEST_FS_H

#include <fs.h>
#include <greatest.h>

TEST test_get_basename(void) {
  const char *res;

  res = get_basename("/foo/bar/baz.txt");
  ASSERT_STR_EQ("baz.txt", res);

  res = get_basename("file.txt");
  ASSERT_STR_EQ("file.txt", res);

  res = get_basename("/foo/bar/");
  ASSERT_STR_EQ("", res);

  res = get_basename(NULL);
  ASSERT_EQ(NULL, res);

  PASS();
}

SUITE(fs_suite) { RUN_TEST(test_get_basename); }

#endif /* !TEST_FS_H */
