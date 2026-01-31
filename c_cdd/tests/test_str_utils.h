/**
 * @file test_str_utils.h
 * @brief Unit tests for shared string utilities.
 * @author Samuel Marks
 */

#ifndef TEST_STR_UTILS_H
#define TEST_STR_UTILS_H

#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "str_utils.h"

/* --- strdup tests --- */

TEST test_c_cdd_strdup_basic(void) {
  const char *input = "hello world";
  char *dup = c_cdd_strdup(input);
  ASSERT(dup != NULL);
  ASSERT(dup != input); /* Pointers must check distinct */
  ASSERT_STR_EQ(input, dup);
  free(dup);
  PASS();
}

TEST test_c_cdd_strdup_null(void) {
  ASSERT(c_cdd_strdup(NULL) == NULL);
  PASS();
}

TEST test_c_cdd_strdup_empty(void) {
  char *dup = c_cdd_strdup("");
  ASSERT(dup != NULL);
  ASSERT_STR_EQ("", dup);
  free(dup);
  PASS();
}

/* --- str_starts_with tests --- */

TEST test_c_cdd_str_starts_with(void) {
  ASSERT(c_cdd_str_starts_with("prefix_string", "prefix"));
  ASSERT(!c_cdd_str_starts_with("string_prefix", "prefix"));
  ASSERT(!c_cdd_str_starts_with("pre", "prefix"));
  /* Exact */
  ASSERT(c_cdd_str_starts_with("foo", "foo"));
  /* Empty prefix matches everything */
  ASSERT(c_cdd_str_starts_with("anything", ""));
  PASS();
}

TEST test_c_cdd_str_starts_with_null(void) {
  ASSERT(!c_cdd_str_starts_with(NULL, "param"));
  ASSERT(!c_cdd_str_starts_with("param", NULL));
  ASSERT(!c_cdd_str_starts_with(NULL, NULL));
  PASS();
}

/* --- str_equal tests --- */

TEST test_c_cdd_str_equal(void) {
  ASSERT(c_cdd_str_equal("foo", "foo"));
  ASSERT(!c_cdd_str_equal("foo", "bar"));
  ASSERT(!c_cdd_str_equal("foo", "fo"));
  PASS();
}

TEST test_c_cdd_str_equal_nulls(void) {
  ASSERT(c_cdd_str_equal(NULL, NULL));
  ASSERT(!c_cdd_str_equal("foo", NULL));
  ASSERT(!c_cdd_str_equal(NULL, "foo"));
  PASS();
}

/* --- str_after_last tests --- */

TEST test_c_cdd_str_after_last(void) {
  /* Common path case */
  ASSERT_STR_EQ("Type", c_cdd_str_after_last("#/definitions/Type", '/'));
  /* Trailing slash returns empty string */
  ASSERT_STR_EQ("", c_cdd_str_after_last("/path/to/", '/'));
  /* No delimiter returns original */
  ASSERT_STR_EQ("NoSlash", c_cdd_str_after_last("NoSlash", '/'));
  /* Empty string */
  ASSERT_STR_EQ("", c_cdd_str_after_last("", '/'));
  PASS();
}

TEST test_c_cdd_str_after_last_null(void) {
  ASSERT_STR_EQ("", c_cdd_str_after_last(NULL, '/'));
  PASS();
}

/* --- ref_is_type tests --- */

TEST test_c_cdd_ref_is_type(void) {
  ASSERT(c_cdd_ref_is_type("#/components/schemas/Integer", "Integer"));
  ASSERT(
      !c_cdd_ref_is_type("#/components/schemas/Integer", "String")); /* Diff */
  ASSERT(!c_cdd_ref_is_type("JustName", "Other")); /* No slash, direct compar */
  ASSERT(c_cdd_ref_is_type("DirectMatch", "DirectMatch"));
  PASS();
}

/* --- trim_trailing tests --- */

TEST test_c_cdd_str_trim_trailing_whitespace(void) {
  char buf[32];

  /* Basic spaces */
  strcpy(buf, "hello   ");
  c_cdd_str_trim_trailing_whitespace(buf);
  ASSERT_STR_EQ("hello", buf);

  /* Mix tabs/newlines */
  strcpy(buf, "foo\t\n ");
  c_cdd_str_trim_trailing_whitespace(buf);
  ASSERT_STR_EQ("foo", buf);

  /* Nothing to trim */
  strcpy(buf, "bar");
  c_cdd_str_trim_trailing_whitespace(buf);
  ASSERT_STR_EQ("bar", buf);

  /* Empty */
  strcpy(buf, "");
  c_cdd_str_trim_trailing_whitespace(buf);
  ASSERT_STR_EQ("", buf);

  /* All whitespace */
  strcpy(buf, "   ");
  c_cdd_str_trim_trailing_whitespace(buf);
  ASSERT_STR_EQ("", buf);

  /* Internal whitespace preserved */
  strcpy(buf, "a b c  ");
  c_cdd_str_trim_trailing_whitespace(buf);
  ASSERT_STR_EQ("a b c", buf);

  PASS();
}

/* --- Suite definition --- */

SUITE(str_utils_suite) {
  RUN_TEST(test_c_cdd_strdup_basic);
  RUN_TEST(test_c_cdd_strdup_null);
  RUN_TEST(test_c_cdd_strdup_empty);

  RUN_TEST(test_c_cdd_str_starts_with);
  RUN_TEST(test_c_cdd_str_starts_with_null);

  RUN_TEST(test_c_cdd_str_equal);
  RUN_TEST(test_c_cdd_str_equal_nulls);

  RUN_TEST(test_c_cdd_str_after_last);
  RUN_TEST(test_c_cdd_str_after_last_null);

  RUN_TEST(test_c_cdd_ref_is_type);

  RUN_TEST(test_c_cdd_str_trim_trailing_whitespace);
}

#endif /* TEST_STR_UTILS_H */
