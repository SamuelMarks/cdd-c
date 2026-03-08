#ifndef TEST_FLEXIBLE_ARRAY_H
#define TEST_FLEXIBLE_ARRAY_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/struct.h"
#include "classes/parse/code2schema.h"

TEST test_parse_fam_basic(void) {
  struct StructFields sf;
  struct StructField *f;
  int rc;

  struct_fields_init(&sf);

  rc = parse_struct_member_line("char data[];", &sf);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, sf.size);

  f = &sf.fields[0];
  ASSERT_STR_EQ("data", f->name);
  ASSERT_STR_EQ("string",
                f->type); /* char[] maps to string type logic currently */
  ASSERT_EQ(1, f->is_flexible_array);

  struct_fields_free(&sf);
  PASS();
}

TEST test_parse_fam_int(void) {
  struct StructFields sf;
  struct StructField *f;
  int rc;

  struct_fields_init(&sf);

  /* int items[]; */
  rc = parse_struct_member_line("int items[];", &sf);
  ASSERT_EQ(0, rc);

  f = &sf.fields[0];
  ASSERT_STR_EQ("items", f->name);
  ASSERT_STR_EQ("array", f->type);
  ASSERT_EQ(1, f->is_flexible_array);

  struct_fields_free(&sf);
  PASS();
}

TEST test_parse_ptr_not_fam(void) {
  struct StructFields sf;
  struct StructField *f;
  int rc;

  struct_fields_init(&sf);

  rc = parse_struct_member_line("char *ptr;", &sf);
  ASSERT_EQ(0, rc);

  f = &sf.fields[0];
  ASSERT_STR_EQ("ptr", f->name);
  ASSERT_EQ(0, f->is_flexible_array);

  struct_fields_free(&sf);
  PASS();
}

TEST test_parse_fixed_array_not_fam(void) {
  struct StructFields sf;
  struct StructField *f;
  int rc;

  struct_fields_init(&sf);

  rc = parse_struct_member_line("char buf[10];", &sf);
  ASSERT_EQ(0, rc);

  f = &sf.fields[0];
  /* Current naive parser includes [10] in name likely, verify behavior */
  /* Logic: last space split. "char buf[10];" -> name="buf[10]" type="char" */
  /* Our FAM check name[len-1]==']' && name[len-2]=='['. */
  /* ']' at end? Yes. '[' at len-2? No, it's '0'. */

  /* NOTE: The naive parser keeps array brackets in the name currently. */
  ASSERT_STR_EQ("buf[10]", f->name);
  ASSERT_EQ(0, f->is_flexible_array);

  struct_fields_free(&sf);
  PASS();
}

TEST test_parse_fam_mixed_lines(void) {
  struct StructFields sf;
  int rc;

  struct_fields_init(&sf);

  rc = parse_struct_member_line("int len;", &sf);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, sf.fields[0].is_flexible_array);

  rc = parse_struct_member_line("double vals[];", &sf);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, sf.fields[1].is_flexible_array);
  ASSERT_STR_EQ("vals", sf.fields[1].name);

  struct_fields_free(&sf);
  PASS();
}

SUITE(flexible_array_suite) {
  RUN_TEST(test_parse_fam_basic);
  RUN_TEST(test_parse_fam_int);
  RUN_TEST(test_parse_ptr_not_fam);
  RUN_TEST(test_parse_fixed_array_not_fam);
  RUN_TEST(test_parse_fam_mixed_lines);
}

#endif /* TEST_FLEXIBLE_ARRAY_H */
