/**
 * @file test_str_utils.h
 * @brief Unit tests for shared string utilities.
 * @author Samuel Marks
 */

#ifndef TEST_STR_UTILS_H
#define TEST_STR_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/str.h"
/* clang-format on */

/* --- strdup tests --- */

TEST test_c_cdd_strdup_basic(void) {
  char *_ast_strdup_0 = NULL;
  const char *input = "hello world";
  char *dup = (c_cdd_strdup(input, &_ast_strdup_0), _ast_strdup_0);
  ASSERT(dup != NULL);
  ASSERT(dup != input); /* Pointers must check distinct */
  ASSERT_STR_EQ(input, dup);
  free(dup);
  PASS();
}

TEST test_c_cdd_strdup_null(void) {
  char *_ast_strdup_1 = NULL;
  ASSERT((c_cdd_strdup(NULL, &_ast_strdup_1), _ast_strdup_1) == NULL);
  PASS();
}

TEST test_c_cdd_strdup_empty(void) {
  char *_ast_strdup_2 = NULL;
  char *dup = (c_cdd_strdup("", &_ast_strdup_2), _ast_strdup_2);
  ASSERT(dup != NULL);
  ASSERT_STR_EQ("", dup);
  free(dup);
  PASS();
}

/* --- str_starts_with tests --- */

TEST test_c_cdd_str_starts_with(void) {
  bool _ast_starts_with_3 = false;
  bool _ast_starts_with_4 = false;
  bool _ast_starts_with_5 = false;
  bool _ast_starts_with_6 = false;
  bool _ast_starts_with_7 = false;
  ASSERT((c_cdd_str_starts_with("prefix_string", "prefix", &_ast_starts_with_3),
          _ast_starts_with_3));
  ASSERT(
      !(c_cdd_str_starts_with("string_prefix", "prefix", &_ast_starts_with_4),
        _ast_starts_with_4));
  ASSERT(!(c_cdd_str_starts_with("pre", "prefix", &_ast_starts_with_5),
           _ast_starts_with_5));
  /* Exact */
  ASSERT((c_cdd_str_starts_with("foo", "foo", &_ast_starts_with_6),
          _ast_starts_with_6));
  /* Empty prefix matches everything */
  ASSERT((c_cdd_str_starts_with("anything", "", &_ast_starts_with_7),
          _ast_starts_with_7));
  PASS();
}

TEST test_c_cdd_str_starts_with_null(void) {
  bool _ast_starts_with_8 = false;
  bool _ast_starts_with_9 = false;
  bool _ast_starts_with_10 = false;
  ASSERT(!(c_cdd_str_starts_with(NULL, "param", &_ast_starts_with_8),
           _ast_starts_with_8));
  ASSERT(!(c_cdd_str_starts_with("param", NULL, &_ast_starts_with_9),
           _ast_starts_with_9));
  ASSERT(!(c_cdd_str_starts_with(NULL, NULL, &_ast_starts_with_10),
           _ast_starts_with_10));
  PASS();
}

/* --- str_equal tests --- */

TEST test_c_cdd_str_equal(void) {
  bool _ast_equal_11 = false;
  bool _ast_equal_12 = false;
  bool _ast_equal_13 = false;
  ASSERT((c_cdd_str_equal("foo", "foo", &_ast_equal_11), _ast_equal_11));
  ASSERT(!(c_cdd_str_equal("foo", "bar", &_ast_equal_12), _ast_equal_12));
  ASSERT(!(c_cdd_str_equal("foo", "fo", &_ast_equal_13), _ast_equal_13));
  PASS();
}

TEST test_c_cdd_str_equal_nulls(void) {
  bool _ast_equal_14 = false;
  bool _ast_equal_15 = false;
  bool _ast_equal_16 = false;
  ASSERT((c_cdd_str_equal(NULL, NULL, &_ast_equal_14), _ast_equal_14));
  ASSERT(!(c_cdd_str_equal("foo", NULL, &_ast_equal_15), _ast_equal_15));
  ASSERT(!(c_cdd_str_equal(NULL, "foo", &_ast_equal_16), _ast_equal_16));
  PASS();
}

/* --- str_iequal tests --- */

TEST test_c_cdd_str_iequal(void) {
  bool _ast_iequal_17 = false;
  bool _ast_iequal_18 = false;
  bool _ast_iequal_19 = false;
  bool _ast_iequal_20 = false;
  ASSERT((c_cdd_str_iequal("Foo", "foo", &_ast_iequal_17), _ast_iequal_17));
  ASSERT((c_cdd_str_iequal("Content-Type", "content-type", &_ast_iequal_18),
          _ast_iequal_18));
  ASSERT(!(c_cdd_str_iequal("Foo", "bar", &_ast_iequal_19), _ast_iequal_19));
  ASSERT(!(c_cdd_str_iequal("Foo", "fo", &_ast_iequal_20), _ast_iequal_20));
  PASS();
}

TEST test_c_cdd_str_iequal_nulls(void) {
  bool _ast_iequal_21 = false;
  bool _ast_iequal_22 = false;
  bool _ast_iequal_23 = false;
  ASSERT((c_cdd_str_iequal(NULL, NULL, &_ast_iequal_21), _ast_iequal_21));
  ASSERT(!(c_cdd_str_iequal("foo", NULL, &_ast_iequal_22), _ast_iequal_22));
  ASSERT(!(c_cdd_str_iequal(NULL, "foo", &_ast_iequal_23), _ast_iequal_23));
  PASS();
}

/* --- str_after_last tests --- */

TEST test_c_cdd_str_after_last(void) {
  const char *_ast_after_last_24 = NULL;
  const char *_ast_after_last_25 = NULL;
  const char *_ast_after_last_26 = NULL;
  const char *_ast_after_last_27 = NULL;
  /* Common path case */
  ASSERT_STR_EQ("Type", (c_cdd_str_after_last("#/definitions/Type", '/',
                                              &_ast_after_last_24),
                         _ast_after_last_24));
  /* Trailing slash returns empty string */
  ASSERT_STR_EQ("",
                (c_cdd_str_after_last("/path/to/", '/', &_ast_after_last_25),
                 _ast_after_last_25));
  /* No delimiter returns original */
  ASSERT_STR_EQ("NoSlash",
                (c_cdd_str_after_last("NoSlash", '/', &_ast_after_last_26),
                 _ast_after_last_26));
  /* Empty string */
  ASSERT_STR_EQ("", (c_cdd_str_after_last("", '/', &_ast_after_last_27),
                     _ast_after_last_27));
  PASS();
}

TEST test_c_cdd_str_after_last_null(void) {
  const char *_ast_after_last_28 = NULL;
  ASSERT_STR_EQ("", (c_cdd_str_after_last(NULL, '/', &_ast_after_last_28),
                     _ast_after_last_28));
  PASS();
}

/* --- ref_is_type tests --- */

TEST test_c_cdd_ref_is_type(void) {
  bool _ast_ref_is_type_29 = false;
  bool _ast_ref_is_type_30 = false;
  bool _ast_ref_is_type_31 = false;
  bool _ast_ref_is_type_32 = false;
  ASSERT((c_cdd_ref_is_type("#/components/schemas/Integer", "Integer",
                            &_ast_ref_is_type_29),
          _ast_ref_is_type_29));
  ASSERT(!(c_cdd_ref_is_type("#/components/schemas/Integer", "String",
                             &_ast_ref_is_type_30),
           _ast_ref_is_type_30)); /* Diff */
  ASSERT(!(c_cdd_ref_is_type("JustName", "Other", &_ast_ref_is_type_31),
           _ast_ref_is_type_31)); /* No slash, direct compar */
  ASSERT((c_cdd_ref_is_type("DirectMatch", "DirectMatch", &_ast_ref_is_type_32),
          _ast_ref_is_type_32));
  PASS();
}

/* --- trim_trailing tests --- */

TEST test_c_cdd_str_trim_trailing_whitespace(void) {
  char buf[32];

/* Basic spaces */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(buf, sizeof(buf), "hello   ");
#else
#if defined(_MSC_VER)
  strcpy_s(buf, sizeof(buf), "hello   ");
#else
#if defined(_MSC_VER)
  strcpy_s(buf, sizeof(buf), "hello   ");
#else
  strcpy(buf, "hello   ");
#endif
#endif
#endif
  c_cdd_str_trim_trailing_whitespace(buf);
  ASSERT_STR_EQ("hello", buf);

/* Mix tabs/newlines */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(buf, sizeof(buf), "foo\t\n ");
#else
#if defined(_MSC_VER)
  strcpy_s(buf, sizeof(buf), "foo\t\n ");
#else
#if defined(_MSC_VER)
  strcpy_s(buf, sizeof(buf), "foo\t\n ");
#else
  strcpy(buf, "foo\t\n ");
#endif
#endif
#endif
  c_cdd_str_trim_trailing_whitespace(buf);
  ASSERT_STR_EQ("foo", buf);

/* Nothing to trim */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(buf, sizeof(buf), "bar");
#else
#if defined(_MSC_VER)
  strcpy_s(buf, sizeof(buf), "bar");
#else
#if defined(_MSC_VER)
  strcpy_s(buf, sizeof(buf), "bar");
#else
  strcpy(buf, "bar");
#endif
#endif
#endif
  c_cdd_str_trim_trailing_whitespace(buf);
  ASSERT_STR_EQ("bar", buf);

/* Empty */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(buf, sizeof(buf), "");
#else
#if defined(_MSC_VER)
  strcpy_s(buf, sizeof(buf), "");
#else
#if defined(_MSC_VER)
  strcpy_s(buf, sizeof(buf), "");
#else
  strcpy(buf, "");
#endif
#endif
#endif
  c_cdd_str_trim_trailing_whitespace(buf);
  ASSERT_STR_EQ("", buf);

/* All whitespace */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(buf, sizeof(buf), "   ");
#else
#if defined(_MSC_VER)
  strcpy_s(buf, sizeof(buf), "   ");
#else
#if defined(_MSC_VER)
  strcpy_s(buf, sizeof(buf), "   ");
#else
  strcpy(buf, "   ");
#endif
#endif
#endif
  c_cdd_str_trim_trailing_whitespace(buf);
  ASSERT_STR_EQ("", buf);

/* Internal whitespace preserved */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(buf, sizeof(buf), "a b c  ");
#else
#if defined(_MSC_VER)
  strcpy_s(buf, sizeof(buf), "a b c  ");
#else
#if defined(_MSC_VER)
  strcpy_s(buf, sizeof(buf), "a b c  ");
#else
  strcpy(buf, "a b c  ");
#endif
#endif
#endif
  c_cdd_str_trim_trailing_whitespace(buf);
  ASSERT_STR_EQ("a b c", buf);

  PASS();
}

TEST test_c_cdd_ref_is_type_null(void) {
  bool _ast_ref_is_type_33 = false;
  bool _ast_ref_is_type_34 = false;
  bool _ast_ref_is_type_35 = false;
  ASSERT(!(c_cdd_ref_is_type(NULL, "Type", &_ast_ref_is_type_33),
           _ast_ref_is_type_33));
  ASSERT(!(c_cdd_ref_is_type("Ref", NULL, &_ast_ref_is_type_34),
           _ast_ref_is_type_34));
  ASSERT(!(c_cdd_ref_is_type(NULL, NULL, &_ast_ref_is_type_35),
           _ast_ref_is_type_35));
  PASS();
}

TEST test_c_cdd_str_trim_trailing_whitespace_null(void) {
  c_cdd_str_trim_trailing_whitespace(NULL);
  PASS();
}

TEST test_c_cdd_destringize(void) {
  char *_ast_destringize_36 = NULL;
  char *_ast_destringize_37 = NULL;
  char *_ast_destringize_38 = NULL;
  char *_ast_destringize_39 = NULL;
  char *_ast_destringize_40 = NULL;
  char *_ast_destringize_41 = NULL;
  char *_ast_destringize_42 = NULL;
  char *_ast_destringize_43 = NULL;
  char *_ast_destringize_44 = NULL;
  char *_ast_destringize_45 = NULL;
  char *res;

  /* Basic destringize */
  res = (c_cdd_destringize("\"hello\"", &_ast_destringize_36),
         _ast_destringize_36);
  ASSERT_STR_EQ("hello", res);
  free(res);

  /* Wide string destringize */
  res = (c_cdd_destringize("L\"wide\"", &_ast_destringize_37),
         _ast_destringize_37);
  ASSERT_STR_EQ("wide", res);
  free(res);

  /* Escaped quotes and backslashes */
  res = (c_cdd_destringize("\"a\\\"b\\\\c\"", &_ast_destringize_38),
         _ast_destringize_38);
  ASSERT_STR_EQ("a\"b\\c", res);
  free(res);

  /* Unhandled escapes, passed through literally */
  res = (c_cdd_destringize("\"\\t\\n\"", &_ast_destringize_39),
         _ast_destringize_39);
  ASSERT_STR_EQ("\\t\\n", res);
  free(res);

  /* Trailing backslash */
  res = (c_cdd_destringize("\"a\\\\\"", &_ast_destringize_40),
         _ast_destringize_40);
  ASSERT_STR_EQ("a\\", res);
  free(res);

  /* Actual trailing backslash (missing quote) */
  res =
      (c_cdd_destringize("\"a\\\\", &_ast_destringize_41), _ast_destringize_41);
  ASSERT_STR_EQ("a\\", res);
  free(res);

  /* Null and malformed */
  ASSERT((c_cdd_destringize(NULL, &_ast_destringize_42), _ast_destringize_42) ==
         NULL);
  ASSERT((c_cdd_destringize("", &_ast_destringize_43), _ast_destringize_43) ==
         NULL);
  ASSERT((c_cdd_destringize("x", &_ast_destringize_44), _ast_destringize_44) ==
         NULL);
  ASSERT((c_cdd_destringize("bad", &_ast_destringize_45),
          _ast_destringize_45) == NULL);

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
  RUN_TEST(test_c_cdd_str_iequal);
  RUN_TEST(test_c_cdd_str_iequal_nulls);

  RUN_TEST(test_c_cdd_str_after_last);
  RUN_TEST(test_c_cdd_str_after_last_null);

  RUN_TEST(test_c_cdd_ref_is_type);

  RUN_TEST(test_c_cdd_str_trim_trailing_whitespace);
  RUN_TEST(test_c_cdd_ref_is_type_null);
  RUN_TEST(test_c_cdd_str_trim_trailing_whitespace_null);
  RUN_TEST(test_c_cdd_destringize);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_STR_UTILS_H */
